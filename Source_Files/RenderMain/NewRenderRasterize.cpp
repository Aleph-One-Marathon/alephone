/*
	
	Rendering Clipping/Rasterization Class
	by Loren Petrich,
	August 7, 2000
	
	Contains the calculation of clipping and rasterization; from render.c
	
	Made [view_data *view] a member and removed it as an argument
	Also removed [bitmap_definition *destination] as superfluous,
		now that there is a special rasterizer object that can contain it.

Sep 2, 2000 (Loren Petrich):
	Added some idiot-proofing, since the shapes accessor now returns NULL for nonexistent bitmaps
	
	If a polygon has a "full_side" texture, and there is another polgyon on the other side,
	then suppress the void for the boundary's texture.

Aug 12, 2001 (Ian Rickard):
	Various changes relating to B&B mostly.  I didn't do substantial change commenting becase
	This Gets completely re-vamped for B&B.
*/

#include "cseries.h"

#include "map.h"
#include "lightsource.h"
#include "media.h"
#include "NewRenderRasterize.h"
#include "AnimatedTextures.h"
#include "OGL_Setup.h"

#include <string.h>


/* maximum number of vertices a polygon can be world-clipped into (one per clip line) */
#define MAXIMUM_VERTICES_PER_WORLD_POLYGON (MAXIMUM_VERTICES_PER_POLYGON+4)

#define MAXIMUM_RASTERIZE_WINDOWS 100 // can't have too many of these, only 4 bytes each

static rasterize_window rasterize_windows[MAXIMUM_RASTERIZE_WINDOWS];

NewRenderRasterizer::NewRenderRasterizer() {
	nodes = NULL;
	portalViews = NULL;
	windows = NULL;
	endpoints = NULL;
	objects = NULL;
	
	nodeCount = portalViewCount = windowCount = endpointCount = objectCount = 0;
	
	rast = NULL;
	GlobalView = NULL;
}


/* ---------- rendering the tree */

void NewRenderRasterizer::render_tree(NewVisTree *tree, RenderPlaceObjsClass *objs)
{
	assert(rast);
	assert(GlobalView);
	// LP: reference to simplify the code
	sorted_node_data *sortedNodes = &tree->SortedNodes[0];
	int sortedNodeCount = tree->SortedNodes.size();
	
	nodes = &tree->Nodes[0];
	nodeCount = tree->Nodes.size();
	portalViews = &tree->PortalViews[0];
	portalViewCount = tree->PortalViews.size();
	windows = &tree->ClippingWindows[0];
	windowCount = tree->ClippingWindows.size();
	endpoints = &tree->Endpoints[0];
	endpointCount = tree->Endpoints.size();
	objects = NULL;
	objectCount = 0;


	// LP change: added support for semitransparent liquids
	// bool SeeThruLiquids = TEST_FLAG(Get_OGL_ConfigureData().Flags,OGL_Flag_LiqSeeThru) != 0;
	// this is always on for now.  It will later be replaced by horizontal membrane opacity and handled in the vis process.
	
	/* walls, ceilings, interior objects, floors, exterior objects for all nodes, back to front */
	// LP change:
	for (int nodeIndex=0 ; nodeIndex < sortedNodeCount ; nodeIndex++)
	// for (node= sorted_nodes; node<next_sorted_node; ++node)
	{
		render_node_data *node = sortedNodes[nodeIndex].node;
		polygon_data *polygon= node->polygon;
		checkPortalViewIndex(node->portal_view);
		portal_view_data *portalView = portalViews + node->portal_view;
		//render_object_data *object;

		// LP change: moved this stuff out here because it only has to be calculated
		// once per polygon.
		// IR change: made 'em pointers
		horizontal_surface_data *floor_surface, *ceiling_surface;
		short i;
		
		// IR change: copy obsoleted by inclusion of horizontal_surface_data in polygon_data, now just copy pointer
		ceiling_surface = &polygon->ceiling_surface;
		floor_surface = &polygon->floor_surface;
		
		// The "continue" conditions are OK to move out here, because a non-drawn polygon's
		// inhabitants will be clipped away.
		
		// LP: get liquid data here for convenience;
		// pointer to it being NULL means no liquid in the polygon
		media_data *media = NULL;
		if (polygon->media_index!=NONE)
			media = get_media_data(polygon->media_index);
		// IR addition: so we can access it indirectly after the if.
		horizontal_surface_data media_surface;
		// IR change start: made lots of changes relating to B&B prep from here...		
		
		// calculate the extents of the clipping windows and build the rasterize windows
		clipping_window_data window_extents;
		window_extents = node->first_clip_window; // load up the first window, and build on that.
		
		rasterize_windows[0] = node->first_clip_window; // clip windows inherit rasterize windows, isn't OO nice?
		int rasterize_windows_used = 1;
		{
			uint16 nextWindow = node->first_clip_window.next_window; // start with the second window, since we just loaded up the first directly into it.
			while (nextWindow != UNONE)
			{
				checkWindowIndex(nextWindow);
				clipping_window_data *thisWindow = windows+nextWindow;
				
				if (thisWindow->x0 < window_extents.x0)
					window_extents.left = thisWindow->left, window_extents.x0 = thisWindow->x0;
				
				if (thisWindow->x1 > window_extents.x1)
					window_extents.right = thisWindow->right, window_extents.x1 = thisWindow->x1;
				
				if (thisWindow->y0 < window_extents.y0)
					window_extents.top = thisWindow->top, window_extents.y0 = thisWindow->y0;
				
				if (thisWindow->y1 > window_extents.y1)
					window_extents.bottom = thisWindow->bottom, window_extents.y1 = thisWindow->y1;
				
				if (rasterize_windows_used < MAXIMUM_RASTERIZE_WINDOWS) {
					rasterize_windows[rasterize_windows_used] = *thisWindow; // clip windows inherit rasterize windows, isn't OO nice?
					rasterize_windows_used++;
				} else {
					vassert(false, "exceeded maximum rasterize windows for a node.");
				}
				
				nextWindow = thisWindow->next_window;
			}
		}
		
		// First, render the sides.  This is changed from the old ceiling-first model because new sloped side inserts will not be clipped to floor/ceilings.
		if (ceiling_surface->height > floor_surface->height)
		{
			
			/* render visible sides */
			for (i= 0; i<polygon->vertex_count; ++i)
			{
				short side_index= polygon->side_indexes[i];
				
				if (side_index==NONE /*|| !TEST_RENDER_FLAG(side_index, _side_is_visible)*/) continue;
				
				line_data *line= get_line_data(polygon->line_indexes[i]);
				side_data *side= get_side_data(side_index);
				vertical_surface_data surface;
				
				surface.length= line->length;
				
				endpoint_data *endpoint0 = get_endpoint_data(polygon->endpoint_indexes[i]);
				
				surface.p0 = long_vector2d(endpoint0->transformedL);
				
				endpoint_data *endpoint1 = get_endpoint_data(polygon->endpoint_indexes[WRAP_HIGH(i, polygon->vertex_count-1)]);
				
				surface.p1 = long_vector2d(endpoint1->transformedL);
				
				surface.ambient_delta= side->ambient_delta;
				
				// LP change: indicate in all cases whether the void is on the other side;
				// added a workaround for full-side textures with a polygon on the other side
				bool void_present;
				
				switch (side->type)
				{
					case _full_side:
						void_present = true;
						// Suppress the void if there is a polygon on the other side.
						// testing transparency should fix a few problems with old maps.
						if (polygon->adjacent_polygon_indexes[i] != NONE && line->is_transparent()) void_present = false;
						
						surface.h0= floor_surface->height - portalView->origin.z;
						surface.hmax= ceiling_surface->height - portalView->origin.z;
						surface.h1= polygon->ceiling_surface.height - portalView->origin.z;
						surface.texture_definition= &side->primary_texture;
						
						render_node_side(&window_extents, rasterize_windows_used, &surface, void_present, portalView);
						break;
					case _split_side: /* render _low_side first */
						surface.h0= floor_surface->height - portalView->origin.z;
						surface.h1= MAX(line->highest_floor(), floor_surface->height) - portalView->origin.z;
						surface.hmax= ceiling_surface->height - portalView->origin.z;
						surface.texture_definition= &side->secondary_texture;
						
						render_node_side(&window_extents, rasterize_windows_used, &surface, true, portalView);
						
						/* fall through and render high side */
					case _high_side:
						surface.h0= MIN(line->lowest_ceiling(), ceiling_surface->height) - portalView->origin.z;
						surface.hmax= ceiling_surface->height - portalView->origin.z;
						surface.h1= polygon->ceiling_surface.height - portalView->origin.z;
						surface.texture_definition= &side->primary_texture;
						
						render_node_side(&window_extents, rasterize_windows_used, &surface, true, portalView);
						break;
					case _low_side:
						surface.h0= floor_surface->height - portalView->origin.z;
						surface.h1= MAX(line->highest_floor(), floor_surface->height) - portalView->origin.z;
						surface.hmax= ceiling_surface->height - portalView->origin.z;
						surface.texture_definition= &side->primary_texture;
						
						render_node_side(&window_extents, rasterize_windows_used, &surface, true, portalView);
						break;
					case _empty_side:
						// no nothing, might be here for just a transparent side or for clipping or something.
						break;
					
					default:
						// LP change:
						assert(false);
						// halt();
				}
				
				if (side->transparent_texture.texture!=NONE)
				{
					surface.h0= MAX(line->highest_floor(), floor_surface->height) - portalView->origin.z;
					surface.h1= line->lowest_ceiling() - portalView->origin.z;
					surface.hmax= ceiling_surface->height - portalView->origin.z;
					surface.texture_definition= &side->transparent_texture;
					
					render_node_side(&window_extents, rasterize_windows_used, &surface, false, portalView);
				}
			}
			
			// now that the sides are drawn, overlay the floor and ceiling.
			
			/* render ceiling if above viewer */
			if (ceiling_surface->height > portalView->origin.z)
			{
				render_node_floor_or_ceiling(&window_extents, rasterize_windows_used, polygon, ceiling_surface, true, portalView);
			}
			
			/* render floor if below viewer */
			if (floor_surface->height<portalView->origin.z)
			{
				render_node_floor_or_ceiling(&window_extents, rasterize_windows_used, polygon, floor_surface, true, portalView);
			}
		}

		// IR note: this code dies soon when seperate nodes for above/below liquid fall into place.
		// LP: this is for objects on the other side of the liquids;
		// render them out here if one can see through the liquids
		/* render exterior objects (with their own clipping windows) */
	//	for (object= node->exterior_objects; object; object= object->next_object)
	//	{
	//		render_node_object(object, true);
	//	}
		
		// IR note: this code dies soon when seperate nodes for above/below liquid fall into place.
		// LP: render the liquid surface after the walls and the stuff behind it
		// and before the stuff before it.
		if (media)
		{
			
			// Render only if between the floor and the ceiling:
			if (media->height > polygon->floor_surface.height && media->height < polygon->ceiling_surface.height)
			{
			
				// Render the liquids
				horizontal_surface_data LiquidSurface;
				
				LiquidSurface.origin= media->origin;
				LiquidSurface.height= media->height;
				LiquidSurface.texture= media->texture;
				LiquidSurface.lightsource_index= polygon->media_lightsource_index;
				LiquidSurface.transfer_mode= media->transfer_mode;
				LiquidSurface.transfer_mode_data= 0;
				
				render_node_floor_or_ceiling(&window_extents, rasterize_windows_used, polygon, &LiquidSurface, false, portalView);
			}
		}
		extern bool gDebugKey;
		if (gDebugKey)
		render_clip_debug_lines(&node->first_clip_window);
		
		// LP: this is for objects on the view side of the liquids
		/* render exterior objects (with their own clipping windows) */
	//	for (object= node->exterior_objects; object; object= object->next_object)
	//	{
	//		render_node_object(object, false);
	//	}
	}
	
	return;
}

void NewRenderRasterizer::fake_render_tree(NewVisTree *tree)
{
	assert(GlobalView);
	// LP: reference to simplify the code
	sorted_node_data *sortedNodes = &tree->SortedNodes[0];
	int sortedNodeCount = tree->SortedNodes.size();
	
	nodes = &tree->Nodes[0];
	nodeCount = tree->Nodes.size();
	portalViews = &tree->PortalViews[0];
	portalViewCount = tree->PortalViews.size();
	windows = &tree->ClippingWindows[0];
	windowCount = tree->ClippingWindows.size();
	endpoints = &tree->Endpoints[0];
	endpointCount = tree->Endpoints.size();


	// LP change: added support for semitransparent liquids
	// bool SeeThruLiquids = TEST_FLAG(Get_OGL_ConfigureData().Flags,OGL_Flag_LiqSeeThru) != 0;
	// this is always on for now.  It will later be replaced by horizontal membrane opacity and handled in the vis process.
	
	/* walls, ceilings, interior objects, floors, exterior objects for all nodes, back to front */
	// LP change:
	for (int nodeIndex=0 ; nodeIndex < sortedNodeCount ; nodeIndex++)
	// for (node= sorted_nodes; node<next_sorted_node; ++node)
	{
		render_node_data *node = sortedNodes[nodeIndex].node;
		polygon_data *polygon= node->polygon;
		checkPortalViewIndex(node->portal_view);
		portal_view_data *portalView = portalViews + node->portal_view;
		
		ADD_POLYGON_TO_AUTOMAP((int)node->polygon.index());
		
		
		if (polygon->ceiling_surface.height > polygon->floor_surface.height)
		{
			
			/* render visible sides */
			for (int i= 0; i<polygon->vertex_count; ++i)
			{
				short side_index= polygon->side_indexes[i];
				
				if (side_index==NONE /*|| !TEST_RENDER_FLAG(side_index, _side_is_visible)*/) continue;
				
				line_data *line= get_line_data(polygon->line_indexes[i]);
				side_data *side= get_side_data(side_index);
				
				endpoint_data *endpoint0 = get_endpoint_data(polygon->endpoint_indexes[i]);
				endpoint_data *endpoint1 = get_endpoint_data(polygon->endpoint_indexes[WRAP_HIGH(i, polygon->vertex_count-1)]);
				
				ADD_LINE_TO_AUTOMAP(polygon->line_indexes[i]);
			}
		}
	}
	
	return;
}

/* ---------- rendering ceilings and floors */

#define SIDE_VERTICAL_GAP_SKIP 10 // draw up to 10 unnecesary rows of pixels between windows
#define SIDE_HORIZONTAL_GAP_SKIP 4 // draw up to 4 unnecesary columns of pixels between windows

// LP change: added "void present on other side" flag
void NewRenderRasterizer::render_node_floor_or_ceiling(
	clipping_window_data *vis_extents,
	int rasterize_windows_used,
	polygon_data *polygon,
	horizontal_surface_data *surface,
	bool void_present,
	portal_view_data *portalView)
{
	// LP addition: animated-texture support
	// Extra variable defined so as not to edit the original texture
	shape_descriptor Texture = AnimTxtr_Translate(surface->texture);
	
	if (Texture==NONE) return;
	
	struct polygon_definition textured_polygon;
	flagged_world_point2d vertices[MAXIMUM_VERTICES_PER_WORLD_POLYGON];
	world_distance adjusted_height = surface->height - portalView->origin.z;
	int32 transformed_height = adjusted_height * GlobalView->world_to_screen_y;
	short vertex_count;
	short i;

	/* build transformed vertex list */
	vertex_count= polygon->vertex_count;
	for (i=0;i<vertex_count;++i)
	{
		endpoint_data *endpoint = get_endpoint_data(polygon->endpoint_indexes[i]);
		
		vertices[i] = endpoint->transformedL;
		
		vertices[i].flags= 0;
	}
	
	/* reversing the order in which these two were clipped caused weird problems */
	
	/* clip to left and right sides of the window */
	vertex_count= xy_clip_horizontal_polygon(vertices, vertex_count, &vis_extents->left, _clip_left);
	vertex_count= xy_clip_horizontal_polygon(vertices, vertex_count, &vis_extents->right, _clip_right);
	
	/* clip to top and bottom sides of the window */
	vertex_count= z_clip_horizontal_polygon(vertices, vertex_count, &vis_extents->top, adjusted_height, _clip_up);
	vertex_count= z_clip_horizontal_polygon(vertices, vertex_count, &vis_extents->bottom, adjusted_height, _clip_down);

	if (vertex_count <= 0) return;
	
	/* transform the points we have into screen-space (backwards for ceiling polygons) */
	for (i=0;i<vertex_count;++i)
	{
		flagged_world_point2d *world= vertices + (adjusted_height>0 ? vertex_count-i-1 : i);
		point2d *screen= textured_polygon.vertices + i;
		
		switch (world->flags&(_clip_left|_clip_right))
		{
			case 0:
				{
				int32 screen_x= GlobalView->half_screen_width + (world->y*GlobalView->world_to_screen_x)/world->x;
				screen->x= PIN(screen_x, 0, GlobalView->screen_width);
				}
				break;
			case _clip_left: screen->x= vis_extents->x0; break;
			case _clip_right: screen->x= vis_extents->x1; break;
			default:
				// LP change: suppressing
				// if (window->x1-window->x0>1) dprintf("ambiguous clip flags for window [%d,%d];g;", window->x0, window->x1);
				screen->x= vis_extents->x0;
				break;
		}
		
		switch (world->flags&(_clip_up|_clip_down))
		{
			case 0:
				{
				int32 screen_y= GlobalView->half_screen_height - transformed_height/world->x + portalView->dtanpitch;
				screen->y= PIN(screen_y, 0, GlobalView->screen_height);
				}
				break;
			case _clip_up: screen->y= 0; break;
			case _clip_down: screen->y= GlobalView->screen_height; break;
			default:
				// LP change: suppressing
				// if (window->y1-window->y0>1) dprintf("ambiguous clip flags for window [%d,%d];g;", window->y0, window->y1);
				screen->y= 0;
				break;
		}
		// vassert(screen->y>=0&&screen->y<=view->screen_height, csprintf(temporary, "horizontal: flags==%x, window @ %p", world->flags, window));
	}
	
	/* setup the other parameters of the textured polygon */
	textured_polygon.flags= 0;
	textured_polygon.origin.x= portalView->origin.x + surface->origin.x;
	textured_polygon.origin.y= portalView->origin.y + surface->origin.y;
	textured_polygon.origin.z= adjusted_height;
	
	get_shape_bitmap_and_shading_table(Texture, &textured_polygon.texture, &textured_polygon.shading_tables, GlobalView->shading_mode);
	if (!textured_polygon.texture) return;
	
	textured_polygon.ShapeDesc = Texture;
	
	textured_polygon.ambient_shade= get_light_intensity(surface->lightsource_index);
	textured_polygon.vertex_count= vertex_count;
	instantiate_polygon_transfer_mode(GlobalView, &textured_polygon, surface->transfer_mode, GlobalView->tick_count + 16*(polygon-map_polygons), true);
	if (GlobalView->shading_mode==_shading_infravision) textured_polygon.flags|= _SHADELESS_BIT;
	
	// and, finally, map it
	textured_polygon.VoidPresent = void_present;
	rast->texture_horizontal_polygon(textured_polygon, rasterize_area_spec(rasterize_windows, rasterize_windows_used));

	return;
}

/* ---------- rendering sides (walls) */

// LP change: added "void present on other side" flag
void NewRenderRasterizer::render_node_side(
	clipping_window_data *vis_extents,
	int rasterize_windows_used,
	vertical_surface_data *surface,
	bool void_present,
	portal_view_data *portalView)
{
	world_distance h= MIN(surface->h1, surface->hmax);
	
	// LP addition: animated-texture support
	// Extra variable defined so as not to edit the original texture
	shape_descriptor Texture = AnimTxtr_Translate(surface->texture_definition->texture);
	if (h <= surface->h0 || Texture==NONE) return;
	
	
	// first, make sure the surface is in front of the camera
	if ((surface->p0.i * surface->p1.j) - (surface->p1.i * surface->p0.j) <= 0) return;
	// that checks that the second endpoint is clockwise from the first endpoint relative to the origin.
	// also takes care of lines with endpoints AT the origin, or that go through the origin, which have been known to cause problems.

	short vertex_count;
	flagged_world_point2d posts[2];
	
	// initialize the two posts of our trapezoid
	vertex_count= 2;
	posts[0].x= surface->p0.i, posts[0].y= surface->p0.j, posts[0].flags= 0;
	posts[1].x= surface->p1.i, posts[1].y= surface->p1.j, posts[1].flags= 0;
	
	// clip to left and right sides of the cone (must happen in the same order as horizontal polygons)
	vertex_count= xy_clip_line(posts, vertex_count, &vis_extents->left, _clip_left);
	vertex_count= xy_clip_line(posts, vertex_count, &vis_extents->right, _clip_right);
	
	if (vertex_count <= 0) return; // cliped out of existance

	flagged_world_point3d vertices[MAXIMUM_VERTICES_PER_WORLD_POLYGON];
	
	// build a polygon out of the two posts
	vertex_count= 4;// #0 = top left, continue clockwise.
	
	vertices[0].x= posts[0].x;  vertices[0].y= posts[0].y;  vertices[0].z= h;            vertices[0].flags= posts[0].flags; // top-left
	vertices[1].x= posts[1].x;  vertices[1].y= posts[1].y;  vertices[1].z= h;            vertices[1].flags= posts[1].flags; // top-right
	vertices[2].x= posts[1].x;  vertices[2].y= posts[1].y;  vertices[2].z= surface->h0;  vertices[2].flags= posts[1].flags; // bot-right
	vertices[3].x= posts[0].x;  vertices[3].y= posts[0].y;  vertices[3].z= surface->h0;  vertices[3].flags= posts[0].flags; // bot-left
	
	/* clip to top and bottom sides of the window; because xz_clip_vertical_polygon accepts
		vertices in clockwise or counterclockwise order, we can set our vertex list up to be
		clockwise on the screen and never worry about it after that */
	vertex_count= xz_clip_vertical_polygon(vertices, vertex_count, &vis_extents->top, _clip_up);
	vertex_count= xz_clip_vertical_polygon(vertices, vertex_count, &vis_extents->bottom, _clip_down);
	
	if (vertex_count <= 0) return; // cliped out of existance
	
	// LP change:
	world_distance dx= surface->p1.i - surface->p0.i;
	world_distance dy= surface->p1.j - surface->p0.j;
	/*
	world_distance dx= surface->p1.x - surface->p0.x;
	world_distance dy= surface->p1.y - surface->p0.y;
	*/
	// IR change: side effect of B&B prep
	world_distance x0= WORLD_FRACTIONAL_PART(surface->texture_definition->origin.x);
	world_distance y0= WORLD_FRACTIONAL_PART(surface->texture_definition->origin.y);
	
	struct polygon_definition textured_polygon;
	
	/* calculate texture origin and direction */	
	world_distance divisor = surface->length;
	if (divisor == 0)
		divisor = 1;
	textured_polygon.vector.i= (WORLD_ONE*dx)/divisor;
	textured_polygon.vector.j= (WORLD_ONE*dy)/divisor;
	textured_polygon.vector.k= -WORLD_ONE;
	// LP change:
	textured_polygon.origin.x= surface->p0.i - (x0*dx)/divisor;
	textured_polygon.origin.y= surface->p0.j - (x0*dy)/divisor;
	/*
	textured_polygon.origin.x= surface->p0.x - (x0*dx)/divisor;
	textured_polygon.origin.y= surface->p0.y - (x0*dy)/divisor;
	*/
	textured_polygon.origin.z= surface->h1 + y0;

	short i;
	
	/* transform the points we have into screen-space */
	for (i=0;i<vertex_count;++i)
	{
		flagged_world_point3d *world= vertices + i;
		// LP change:
		point2d *screen= textured_polygon.vertices + i;
		
		switch (world->flags&(_clip_left|_clip_right))
		{
			case 0:
				// LP change: making it long-distance friendly
				{
				int32 screen_x= GlobalView->half_screen_width + (world->y*GlobalView->world_to_screen_x)/world->x;
				screen->x= PIN(screen_x, 0, GlobalView->screen_width);
				}
				break;
			case _clip_left: screen->x= vis_extents->x0; break;
			case _clip_right: screen->x= vis_extents->x1; break;
			default:
				// LP change: suppressing
				// if (window->x1-window->x0>1) dprintf("ambiguous clip flags for window [%d,%d];g;", window->x0, window->x1);
				screen->x= vis_extents->x0;
				break;
		}
		
		switch (world->flags&(_clip_up|_clip_down))
		{
			case 0:
				// LP change: making it long-distance friendly
				{
				int32 screen_y= GlobalView->half_screen_height - (world->z*GlobalView->world_to_screen_y)/world->x + portalView->dtanpitch;
				screen->y= PIN(screen_y, 0, GlobalView->screen_height);
				}
				break;
			case _clip_up: screen->y= 0; break;
			case _clip_down: screen->y= GlobalView->screen_height; break;
			default:
				// LP change: suppressing
				// if (window->y1-window->y0>1) dprintf("ambiguous clip flags for window [%d,%d];g;", window->y0, window->y1);
				screen->y= vis_extents->y0;
				break;
		}
		// vassert(screen->y>=0&&screen->y<=view->screen_height, csprintf(temporary, "#%d!in[#0,#%d]: flags==%x, wind@%p #%d w@%p s@%p", screen->y, view->screen_height, world->flags, window, vertex_count, world, screen));
	}
	
	/* setup the other parameters of the textured polygon */
	textured_polygon.flags= 0;
	
	get_shape_bitmap_and_shading_table(Texture, &textured_polygon.texture, &textured_polygon.shading_tables, GlobalView->shading_mode);
	if (!textured_polygon.texture) return;

	textured_polygon.ShapeDesc = Texture;
	
	textured_polygon.ambient_shade= get_light_intensity(surface->texture_definition->lightsource_index) + surface->ambient_delta;
	textured_polygon.ambient_shade= PIN(textured_polygon.ambient_shade, 0, FIXED_ONE);
	textured_polygon.vertex_count= vertex_count;
	
	instantiate_polygon_transfer_mode(GlobalView, &textured_polygon, surface->texture_definition->transfer_mode, GlobalView->tick_count + (int32)windows, false);
	if (GlobalView->shading_mode==_shading_infravision) textured_polygon.flags|= _SHADELESS_BIT;
	
	// and, finally, map it
	textured_polygon.VoidPresent = void_present;
	rast->texture_vertical_polygon(textured_polygon, rasterize_area_spec(rasterize_windows, rasterize_windows_used));

	return;
}


// code I wrote for clipping but never used:
/*	if ((GlobalView->untransformed_left_edge->i*surface->p0.j) - (GlobalView->untransformed_left_edge->j*surface->p0.i) < 0)
	{ // endpoint outside window
		object_bounds.left = GlobalView->untransformed_left_edge;
		object_bounds.x0 = 0;
	} else { // endpoint inside window
		object_bounds.left = surface->p0;
		object_bounds.x0 = GlobalView->half_screen_width + (surface->p0.j*GlobalView->world_to_screen_x)/surface->p0.i;
	}
	
	if ((GlobalView->untransformed_right_edge->i*surface->p1.j) - (GlobalView->untransformed_right_edge->j*surface->p1.i) < 0)
	{ // endpoint outside window
		object_bounds.right = GlobalView->untransformed_right_edge;
		object_bounds.x1 = GlobalView->screen_width;
	} else { // endpoint inside window
		object_bounds.right = surface->p1;
		object_bounds.x1 = GlobalView->half_screen_width + (surface->p1.j*GlobalView->world_to_screen_x)/surface->p1.i;
	}
	
	{
		world_vector2d *extent, *closer, *farther;
		if (surface->p0.i < surface->p1.i) // first post is closer
		{
			closer = &surface->p0;
			farther = &surface->p1;
		} else {
			closer = &surface->p1;
			farther = &surface->p0;
		}
		
		// calculate the top edge
		if (h > 0) // top is above viewer
		{
			extent = closer;
		} else {
			extent = farther;
		}
		
		if ((GlobalView->top_edge.i * h) - (GlobalView->top_edge->j * extent->i))
		{ // line extends out top of window
			object_bounds.top = GlobalView->top_edge;
			object_bounds.y0 = 0;
		} else {
			object_bounds.top.i = extent->i;
			object_bounds.top.j = h;
			object_bounds.y0 = GlobalView->half_screen_height - (h * GlobalView->world_to_screen_y) / extent->i + portalView->dtanpitch;
		}
		
		// calculate the bottom edge
		if (h0 < 0) // bottom is below viewer
		{
			extent = closer;
		} else {
			extent = farther;
		}
		
		if ((GlobalView->bottom_edge.i * h) - (GlobalView->bottom_edge->j * extent->i))
		{ // line extends out bottom of window
			object_bounds.top = GlobalView->bottom_edge;
			object_bounds.y0 = 0;
		} else {
			object_bounds.top.i = extent->i;
			object_bounds.top.j = h0;
			object_bounds.y0 = GlobalView->half_screen_height - (h0 * GlobalView->world_to_screen_y) / extent->i + portalView->dtanpitch;
		}
	}*/

void NewRenderRasterizer::render_clip_debug_lines(clipping_window_data *window) {
	while (window) {
		rast->debug_line_v(window->x0, window->y0, window->y1-1);
		rast->debug_line_h(window->y0, window->x0, window->x1-1);
		rast->debug_line_v(window->x1-1, window->y0, window->y1-1);
		rast->debug_line_h(window->y1-1, window->x0, window->x1-1);
		
		if (window->next_window != UNONE) {
			checkWindowIndex(window->next_window);
			window = windows + window->next_window;
		} else {
			window = NULL;
		}
	}
}

/* ---------- rendering objects */
#if 0
void NewRenderRasterizer::render_node_object(
	render_object_data *object,
	bool other_side_of_media)
	// struct render_object_data *object)
{
	struct clipping_window_data *window;
	
	for (window= object->clipping_windows; window; window= window->next_window)
	{
		object->rectangle.clip_left= window->x0;
		object->rectangle.clip_right= window->x1;
		object->rectangle.clip_top= window->y0;
		object->rectangle.clip_bottom= window->y1;
		
		// Models will have their own liquid-surface clipping,
		// so don't edit their clip rects
		// This is bitwise XOR, but is presumably OK here
		if (view->under_media_boundary ^ other_side_of_media)
		// if (view->under_media_boundary)
		{
			// Clipping: below a liquid surface
			if (object->rectangle.ModelPtr)
				object->rectangle.BelowLiquid = true;
			else
				object->rectangle.clip_top= MAX(object->rectangle.clip_top, object->ymedia);
		}
		else
		{
			// Clipping: above a liquid surface
			if (object->rectangle.ModelPtr)
				object->rectangle.BelowLiquid = false;
			else
				object->rectangle.clip_bottom= MIN(object->rectangle.clip_bottom, object->ymedia);
		}
		
		// LP: added OpenGL support
		// LP: using rasterizer object
		RasPtr->texture_rectangle(object->rectangle);
		/*
		if (!OGL_RenderSprite(object->rectangle))
			texture_rectangle(&object->rectangle, destination, view);
		*/
	}
	
	return;
}
#endif


/* ---------- horizontal polygon clipping */

enum /* xy_clip_horizontal_polygon() states */
{
	_testing_first_vertex, /* are we in or out? */
	_searching_cw_for_in_out_transition,
	_searching_ccw_for_out_in_transition,
	_searching_cw_for_out_in_transition
};

// LP change: make it better able to do long-distance views
short NewRenderRasterizer::xy_clip_horizontal_polygon(
	flagged_world_point2d *vertices,
	short vertex_count,
	long_vector2d *line, // world_vector2d *line,
	uint16 flag)
{
#ifdef QUICKDRAW_DEBUG
	debug_flagged_points(vertices, vertex_count);
	debug_vector(line);
#endif
//	dprintf("clipping %p (#%d vertices) to vector %x,%x (slope==%x)", vertices, vertex_count, line->i, line->j, slope);
	
	if (vertex_count)
	{
		short state= _testing_first_vertex;
		short vertex_index= 0, vertex_delta= 1, first_vertex= 0;
		short entrance_vertex= NONE, exit_vertex= NONE; /* exiting the clipped area and entering the clipped area */
		bool clipped_exit_vertex= true, clipped_entrance_vertex= true; /* will be false if these points lie directly on a vertex */
		
		do
		{
			// LP change:
			CROSSPROD_TYPE cross_product= CROSSPROD_TYPE(line->i)*vertices[vertex_index].y - CROSSPROD_TYPE(line->j)*vertices[vertex_index].x;
			// int32 cross_product= line->i*vertices[vertex_index].y - line->j*vertices[vertex_index].x;
			
			switch (SGN(cross_product))
			{
				case -1: /* inside (i.e., will be clipped) */
//					dprintf("vertex#%d is inside s==#%d", vertex_index, state);
					switch (state)
					{
						case _testing_first_vertex:
							first_vertex= vertex_index;
							state= _searching_cw_for_in_out_transition; /* the exit point from the clip area */
							break;
						
						case _searching_ccw_for_out_in_transition: /* found exit point from clip area */
							state= _searching_cw_for_out_in_transition; /* the entrance point to the clipped area */
							vertex_delta= 1;
							if (exit_vertex==NONE) exit_vertex= WRAP_HIGH(vertex_index, vertex_count-1);
							vertex_index= first_vertex; /* skip vertices we already know are out */
							break;
						
						case _searching_cw_for_out_in_transition: /* found entrance point to clipped area */
							if (entrance_vertex==NONE) entrance_vertex= vertex_index;
							state= NONE;
							break;
					}
					break;
				
				case 0: /* clip line passed directly through a vertex */
//					dprintf("vertex#%d is on the clip line s==#%d", vertex_index, state);
					switch (state)
					{
						/* if we’re testing the first vertex, this tells us nothing */
						
						case _searching_cw_for_out_in_transition:
							entrance_vertex= vertex_index;
							clipped_entrance_vertex= false; /* remember if this passes through vertex */
							break;
						
						case _searching_cw_for_in_out_transition:
							exit_vertex= WRAP_HIGH(vertex_index, vertex_count-1);
							clipped_exit_vertex= false;
							break;
						case _searching_ccw_for_out_in_transition:
							exit_vertex= WRAP_HIGH(vertex_index, vertex_count-1);
							clipped_exit_vertex= false; /* remember if this passes through vertex */
							break;
					}
					break;
				
				case 1: /* outside (i.e., will not be clipped) */
//					dprintf("vertex#%d is outside s==#%d", vertex_index, state);
					switch (state)
					{
						case _testing_first_vertex:
							first_vertex= vertex_index;
							state= _searching_ccw_for_out_in_transition; /* the exit point from the clipped area */
							vertex_delta= -1;
							break;
						
						case _searching_cw_for_in_out_transition: /* found exit point from clipped area */
							state= _searching_cw_for_out_in_transition; /* the entrance point to the clipped area */
							if (exit_vertex==NONE) exit_vertex= vertex_index;
							break;
					}
					break;
			}

			/* adjust vertex_index (clockwise or counterclockwise, depending on vertex_delta)
				if we’ve come back to the first vertex without finding an entrance point we’re
				either all the way in or all the way out */
			vertex_index= (vertex_delta<0) ? WRAP_LOW(vertex_index, vertex_count-1) :
				WRAP_HIGH(vertex_index, vertex_count-1);
			if (vertex_index==first_vertex) /* we came full-circle without hitting anything */
			{
				switch (state)
				{
					case _searching_cw_for_in_out_transition: /* never found a way out: clipped into nothing */
						vertex_count= 0;
					case _testing_first_vertex: /* is this the right thing to do? */
					case _searching_ccw_for_out_in_transition: /* never found a way in: no clipping */
						exit_vertex= NONE;
						state= NONE;
						break;
				}
			}
		}
		while (state!=NONE);
		
		if (exit_vertex!=NONE) /* we’ve got clipping to do */
		{
			flagged_world_point2d new_entrance_point, new_exit_point;
			
//			dprintf("entrance_vertex==#%d (%s), exit_vertex==#%d (%s)", entrance_vertex, clipped_entrance_vertex ? "clipped" : "unclipped", exit_vertex, clipped_exit_vertex ? "clipped" : "unclipped");
			
			/* clip the entrance to the clipped area */
			if (clipped_entrance_vertex)
			{
				xy_clip_flagged_world_points(vertices + WRAP_LOW(entrance_vertex, vertex_count-1), vertices + entrance_vertex,
					&new_entrance_point, line);
			}
			else
			{
				new_entrance_point= vertices[entrance_vertex];
			}
			new_entrance_point.flags|= flag;
			
			/* clip the exit from the clipped area */
			if (clipped_exit_vertex)
			{
				xy_clip_flagged_world_points(vertices + WRAP_LOW(exit_vertex, vertex_count-1), vertices + exit_vertex,
					&new_exit_point, line);
			}
			else
			{
				new_exit_point= vertices[WRAP_LOW(exit_vertex, vertex_count-1)];
			}
			new_exit_point.flags|= flag;
			
			/* adjust for the change in number of vertices */
			vertex_delta= entrance_vertex - exit_vertex;
			if (vertex_delta<0)
			{
				if (vertex_delta!=-2) memmove(vertices+entrance_vertex+2, vertices+exit_vertex, (vertex_count-exit_vertex)*sizeof(flagged_world_point2d));
				vertex_delta= vertex_count+vertex_delta;
			}
			else
			{
				/* move down by exit_vertex, add new vertices to end */
				assert(vertex_delta);
				if (exit_vertex)
				{
					memmove(vertices, vertices+exit_vertex, vertex_delta*sizeof(flagged_world_point2d));
					entrance_vertex-= exit_vertex;
				}
			}
			
			vertex_count= vertex_delta+2;
			vwarn(vertex_count>=3 && vertex_count<=MAXIMUM_VERTICES_PER_WORLD_POLYGON,
				csprintf(temporary, "vertex overflow or underflow (#%d);g;", vertex_count));
				
			if (vertex_count<3 || vertex_count>MAXIMUM_VERTICES_PER_WORLD_POLYGON)
			{
				vertex_count= 0;
			}
			else
			{
				/* and, finally, add the new vertices */
				vertices[entrance_vertex]= new_entrance_point;
				vertices[entrance_vertex+1]= new_exit_point;
			}
		}
	}

#ifdef QUICKDRAW_DEBUG
	debug_flagged_points(vertices, vertex_count);
	debug_vector(line);
#endif
//	dprintf("result == %p (#%d vertices)", vertices, vertex_count);

	return vertex_count;
}

/* sort points before clipping to assure consistency; there is a way to make this more accurate
	but it requires the downshifting game, as played in SCOTTISH_TEXTURES.C.  it’s tempting to
	think that having a smaller scale for our world coordinates would help here (i.e., less bits
	per distance) but then wouldn’t we be screwed when we tried to rotate? */
// LP change: make it better able to do long-distance views
void NewRenderRasterizer::xy_clip_flagged_world_points(
	flagged_world_point2d *p0,
	flagged_world_point2d *p1,
	flagged_world_point2d *clipped,
	long_vector2d *line) // world_vector2d *line)
{
	bool swap= (p1->y>p0->y) ? false : ((p0->y==p1->y) ? (p1->x<p0->x) : true);
	flagged_world_point2d *local_p0= swap ? p1 : p0;
	flagged_world_point2d *local_p1= swap ? p0 : p1;
	world_distance dx= local_p1->x - local_p0->x;
	world_distance dy= local_p1->y - local_p0->y;
	int32 numerator= line->j*local_p0->x - line->i*local_p0->y;
	int32 denominator= line->i*dy - line->j*dx;
	short shift_count= FIXED_FRACTIONAL_BITS;
	_fixed t;

	/* give numerator 16 significant bits over denominator and then calculate t==n/d;  MPW’s PPCC
		didn’t seem to like (INT32_MIN>>1) and i had to substitute 0xc0000000 instead (hmmm) */
	while (numerator<=(int32)0x3fffffff && numerator>=(int32)0xc0000000 && shift_count--) numerator<<= 1;
	if (shift_count>0) denominator>>= shift_count;
	t= numerator/denominator;

	/* calculate the clipped point */
	clipped->x= local_p0->x + FIXED_INTEGERAL_PART(t*dx);
	clipped->y= local_p0->y + FIXED_INTEGERAL_PART(t*dy);
	clipped->flags= local_p0->flags&local_p1->flags;

	return;
}

/* almost wholly identical to xz_clip_vertical_polygon() except that this works off 2d points
	in the xy-plane and a height */
// LP change: make it better able to do long-distance views
short NewRenderRasterizer::z_clip_horizontal_polygon(
	flagged_world_point2d *vertices,
	short vertex_count,
	long_vector2d *line, // world_vector2d *line,/* i==x, j==z */
	world_distance height,
	uint16 flag)
{
	// LP change:
	CROSSPROD_TYPE heighti= CROSSPROD_TYPE(line->i)*height;
	// int32 heighti= line->i*height;
	
#ifdef QUICKDRAW_DEBUG
	debug_flagged_points(vertices, vertex_count);
	debug_x_line(line->j ? (line->i*height)/line->j : (height<0 ? INT32_MIN : INT32_MAX));
#endif
//	dprintf("clipping %p (#%d vertices) to vector %x,%x", vertices, vertex_count, line->i, line->j);
	
	if (vertex_count)
	{
		short state= _testing_first_vertex;
		short vertex_index= 0, vertex_delta= 1, first_vertex= 0;
		short entrance_vertex= NONE, exit_vertex= NONE; /* exiting the clipped area and entering the clipped area */
		bool clipped_exit_vertex= true, clipped_entrance_vertex= true; /* will be false if these points lie directly on a vertex */
		
		do
		{
			// LP change:
			CROSSPROD_TYPE cross_product= heighti - CROSSPROD_TYPE(line->j)*vertices[vertex_index].x;
			// int32 cross_product= heighti - line->j*vertices[vertex_index].x;

			if (cross_product<0) /* inside (i.e., will be clipped) */
			{
				switch (state)
				{
					case _testing_first_vertex:
						first_vertex= vertex_index;
						state= _searching_cw_for_in_out_transition; /* the exit point from the clip area */
						break;
					
					case _searching_ccw_for_out_in_transition: /* found exit point from clip area */
						state= _searching_cw_for_out_in_transition; /* the entrance point to the clipped area */
						vertex_delta= 1;
						if (exit_vertex==NONE) exit_vertex= WRAP_HIGH(vertex_index, vertex_count-1);
						vertex_index= first_vertex; /* skip vertices we already know are out */
						break;
					
					case _searching_cw_for_out_in_transition: /* found entrance point to clipped area */
						if (entrance_vertex==NONE) entrance_vertex= vertex_index;
						state= NONE;
						break;
				}
			}
			else
			{
				if (cross_product>0) /* outside (i.e., will not be clipped) */
				{
					switch (state)
					{
						case _testing_first_vertex:
							first_vertex= vertex_index;
							state= _searching_ccw_for_out_in_transition; /* the exit point from the clipped area */
							vertex_delta= -1;
							break;
						
						case _searching_cw_for_in_out_transition: /* found exit point from clipped area */
							state= _searching_cw_for_out_in_transition; /* the entrance point to the clipped area */
							if (exit_vertex==NONE) exit_vertex= vertex_index;
							break;
					}
				}
				else /* clip line passed directly through a vertex */
				{
					switch (state)
					{
						/* if we’re testing the first vertex (_testing_first_vertex), this tells us nothing */
						
						case _searching_cw_for_out_in_transition:
							entrance_vertex= vertex_index;
							clipped_entrance_vertex= false; /* remember if this passes through vertex */
							break;
						
						case _searching_cw_for_in_out_transition:
							exit_vertex= WRAP_HIGH(vertex_index, vertex_count-1);
							clipped_exit_vertex= false;
							break;
						case _searching_ccw_for_out_in_transition:
							exit_vertex= WRAP_HIGH(vertex_index, vertex_count-1);
							clipped_exit_vertex= false; /* remember if this passes through vertex */
							break;
					}
				}
			}

			/* adjust vertex_index (clockwise or counterclockwise, depending on vertex_delta)
				if we’ve come back to the first vertex without finding an entrance point we’re
				either all the way in or all the way out */
			vertex_index= (vertex_delta<0) ? WRAP_LOW(vertex_index, vertex_count-1) :
				WRAP_HIGH(vertex_index, vertex_count-1);
			if (vertex_index==first_vertex) /* we came full-circle without hitting anything */
			{
				switch (state)
				{
					case _searching_cw_for_in_out_transition: /* never found a way out: clipped into nothing */
						vertex_count= 0;
					case _testing_first_vertex: /* is this the right thing to do? */
					case _searching_ccw_for_out_in_transition: /* never found a way in: no clipping */
						exit_vertex= NONE;
						state= NONE;
						break;
				}
			}
		}
		while (state!=NONE);
		
		if (exit_vertex!=NONE) /* we’ve got clipping to do */
		{
			flagged_world_point2d new_entrance_point, new_exit_point;
			
//			dprintf("entrance_vertex==#%d (%s), exit_vertex==#%d (%s)", entrance_vertex, clipped_entrance_vertex ? "clipped" : "unclipped", exit_vertex, clipped_exit_vertex ? "clipped" : "unclipped");
			
			/* clip the entrance to the clipped area */
			if (clipped_entrance_vertex)
			{
				z_clip_flagged_world_points(vertices + WRAP_LOW(entrance_vertex, vertex_count-1), vertices + entrance_vertex,
					height, &new_entrance_point, line);
			}
			else
			{
				new_entrance_point= vertices[entrance_vertex];
			}
			new_entrance_point.flags|= flag;
			
			/* clip the exit from the clipped area */
			if (clipped_exit_vertex)
			{
				z_clip_flagged_world_points(vertices + WRAP_LOW(exit_vertex, vertex_count-1), vertices + exit_vertex,
					height, &new_exit_point, line);
			}
			else
			{
				new_exit_point= vertices[WRAP_LOW(exit_vertex, vertex_count-1)];
			}
			new_exit_point.flags|= flag;
			
			/* adjust for the change in number of vertices */
			vertex_delta= entrance_vertex - exit_vertex;
			if (vertex_delta<0)
			{
				if (vertex_delta!=-2) memmove(vertices+entrance_vertex+2, vertices+exit_vertex, (vertex_count-exit_vertex)*sizeof(flagged_world_point2d));
				vertex_delta= vertex_count+vertex_delta;
			}
			else
			{
				/* move down by exit_vertex, add new vertices to end */
				assert(vertex_delta);
				if (exit_vertex)
				{
					memmove(vertices, vertices+exit_vertex, vertex_delta*sizeof(flagged_world_point2d));
					entrance_vertex-= exit_vertex;
				}
			}
			vertex_count= vertex_delta+2;

			vwarn(vertex_count>=3 && vertex_count<=MAXIMUM_VERTICES_PER_WORLD_POLYGON,
				csprintf(temporary, "vertex overflow or underflow (#%d);g;", vertex_count));
				
			if (vertex_count<3 || vertex_count>MAXIMUM_VERTICES_PER_WORLD_POLYGON)
			{
				vertex_count= 0;
			}
			else
			{
				/* and, finally, add the new vertices */
				vertices[entrance_vertex]= new_entrance_point;
				vertices[entrance_vertex+1]= new_exit_point;
			}
		}
	}

#ifdef QUICKDRAW_DEBUG
	debug_flagged_points(vertices, vertex_count);
	debug_x_line(line->j ? (line->i*height)/line->j : (height<0 ? INT32_MIN : INT32_MAX));
#endif
//	dprintf("result == %p (#%d vertices)", vertices, vertex_count);

	return vertex_count;
}

/* sort points before clipping to assure consistency; this is almost identical to xz_clip…()
	except that it clips 2d points in the xy-plane at the given height. */
// LP change: make it better able to do long-distance views
void NewRenderRasterizer::z_clip_flagged_world_points(
	flagged_world_point2d *p0,
	flagged_world_point2d *p1,
	world_distance height,
	flagged_world_point2d *clipped,
	long_vector2d *line) // world_vector2d *line)
{
	bool swap= (p1->y>p0->y) ? false : ((p0->y==p1->y) ? (p1->x<p0->x) : true);
	flagged_world_point2d *local_p0= swap ? p1 : p0;
	flagged_world_point2d *local_p1= swap ? p0 : p1;
	world_distance dx= local_p1->x - local_p0->x;
	world_distance dy= local_p1->y - local_p0->y;
	int32 numerator= line->j*local_p0->x - line->i*height;
	int32 denominator= - line->j*dx;
	short shift_count= FIXED_FRACTIONAL_BITS;
	_fixed t;

	/* give numerator 16 significant bits over denominator and then calculate t==n/d;  MPW’s PPCC
		didn’t seem to like (INT32_MIN>>1) and i had to substitute 0xc0000000 instead (hmmm) */
	while (numerator<=(int32)0x3fffffff && numerator>=(int32)0xc0000000 && shift_count--) numerator<<= 1;
	if (shift_count>0) denominator>>= shift_count;
	t= numerator/denominator;

	/* calculate the clipped point */
	clipped->x= local_p0->x + FIXED_INTEGERAL_PART(t*dx);
	clipped->y= local_p0->y + FIXED_INTEGERAL_PART(t*dy);
	clipped->flags= local_p0->flags&local_p1->flags;

	return;
}

/* ---------- vertical polygon clipping */

// LP change: make it better able to do long-distance views
short NewRenderRasterizer::xy_clip_line(
	flagged_world_point2d *posts,
	short vertex_count,
	long_vector2d *line, // world_vector2d *line,
	uint16 flag)
{
#ifdef QUICKDRAW_DEBUG
//	debug_flagged_points(posts, vertex_count);
//	debug_vector(line);
#endif
//	dprintf("clipping %p (#%d) to line (%d,%d)", posts, vertex_count, line->i, line->j);
	
	if (vertex_count)
	{
		// LP change:
		CROSSPROD_TYPE cross_product0= CROSSPROD_TYPE(line->i)*posts[0].y - CROSSPROD_TYPE(line->j)*posts[0].x;
		CROSSPROD_TYPE cross_product1= CROSSPROD_TYPE(line->i)*posts[1].y - CROSSPROD_TYPE(line->j)*posts[1].x;
		/*
		int32 cross_product0= line->i*posts[0].y - line->j*posts[0].x;
		int32 cross_product1= line->i*posts[1].y - line->j*posts[1].x;
		*/
		
		if (cross_product0<0)
		{
			if (cross_product1<0) /* clipped out of existence */
			{
				vertex_count= 0;
			}
			else
			{
				xy_clip_flagged_world_points(&posts[0], &posts[1], &posts[0], line);
				posts[0].flags|= flag;
			}
		}
		else
		{
			if (cross_product1<0)
			{
				xy_clip_flagged_world_points(&posts[0], &posts[1], &posts[1], line);
				posts[1].flags|= flag;
			}
		}
	}

#ifdef QUICKDRAW_DEBUG
//	debug_flagged_points(posts, vertex_count);
//	debug_vector(line);
#endif
//	dprintf("result #%d vertices", vertex_count);
	
	return vertex_count;
}

// LP change: make it better able to do long-distance views
short NewRenderRasterizer::xz_clip_vertical_polygon(
	flagged_world_point3d *vertices,
	short vertex_count,
	long_vector2d *line, // world_vector2d *line, /* i==x, j==z */
	uint16 flag)
{
#ifdef QUICKDRAW_DEBUG
//	debug_flagged_points3d(vertices, vertex_count);
//	debug_vector(line);
#endif
//	dprintf("clipping %p (#%d vertices) to vector %x,%x", vertices, vertex_count, line->i, line->j);
	
	if (vertex_count)
	{
		short state= _testing_first_vertex;
		short vertex_index= 0, vertex_delta= 1, first_vertex= 0;
		short entrance_vertex= NONE, exit_vertex= NONE; /* exiting the clipped area and entering the clipped area */
		bool clipped_exit_vertex= true, clipped_entrance_vertex= true; /* will be false if these points lie directly on a vertex */
		
		do
		{	
			// LP change:
			CROSSPROD_TYPE cross_product= CROSSPROD_TYPE(line->i)*vertices[vertex_index].z - CROSSPROD_TYPE(line->j)*vertices[vertex_index].x;
			// int32 cross_product= line->i*vertices[vertex_index].z - line->j*vertices[vertex_index].x;
			
			switch (SGN(cross_product))
			{
				case -1: /* inside (i.e., will be clipped) */
//					dprintf("vertex#%d is inside s==#%d", vertex_index, state);
					switch (state)
					{
						case _testing_first_vertex:
							first_vertex= vertex_index;
							state= _searching_cw_for_in_out_transition; /* the exit point from the clip area */
							break;
						
						case _searching_ccw_for_out_in_transition: /* found exit point from clip area */
							state= _searching_cw_for_out_in_transition; /* the entrance point to the clipped area */
							vertex_delta= 1;
							if (exit_vertex==NONE) exit_vertex= WRAP_HIGH(vertex_index, vertex_count-1);
							vertex_index= first_vertex; /* skip vertices we already know are out */
							break;
						
						case _searching_cw_for_out_in_transition: /* found entrance point to clipped area */
							if (entrance_vertex==NONE) entrance_vertex= vertex_index;
							state= NONE;
							break;
					}
					break;
				
				case 0: /* clip line passed directly through a vertex */
//					dprintf("vertex#%d is on the clip line s==#%d", vertex_index, state);
					switch (state)
					{
						/* if we’re testing the first vertex, this tells us nothing */
						
						case _searching_cw_for_out_in_transition:
							entrance_vertex= vertex_index;
							clipped_entrance_vertex= false; /* remember if this passes through vertex */
							break;
						
						case _searching_cw_for_in_out_transition:
							exit_vertex= WRAP_HIGH(vertex_index, vertex_count-1);
							clipped_exit_vertex= false;
							break;
						case _searching_ccw_for_out_in_transition:
							exit_vertex= WRAP_HIGH(vertex_index, vertex_count-1);
							clipped_exit_vertex= false; /* remember if this passes through vertex */
							break;
					}
					break;
				
				case 1: /* outside (i.e., will not be clipped) */
//					dprintf("vertex#%d is outside s==#%d", vertex_index, state);
					switch (state)
					{
						case _testing_first_vertex:
							first_vertex= vertex_index;
							state= _searching_ccw_for_out_in_transition; /* the exit point from the clipped area */
							vertex_delta= -1;
							break;
						
						case _searching_cw_for_in_out_transition: /* found exit point from clipped area */
							state= _searching_cw_for_out_in_transition; /* the entrance point to the clipped area */
							if (exit_vertex==NONE) exit_vertex= vertex_index;
							break;
					}
					break;
			}

			/* adjust vertex_index (clockwise or counterclockwise, depending on vertex_delta)
				if we’ve come back to the first vertex without finding an entrance point we’re
				either all the way in or all the way out */
			vertex_index= (vertex_delta<0) ? WRAP_LOW(vertex_index, vertex_count-1) :
				WRAP_HIGH(vertex_index, vertex_count-1);
			if (vertex_index==first_vertex) /* we came full-circle without hitting anything */
			{
				switch (state)
				{
					case _searching_cw_for_in_out_transition: /* never found a way out: clipped into nothing */
						vertex_count= 0;
					case _testing_first_vertex: /* is this the right thing to do? */
					case _searching_ccw_for_out_in_transition: /* never found a way in: no clipping */
						exit_vertex= NONE;
						state= NONE;
						break;
				}
			}
		}
		while (state!=NONE);
		
		if (exit_vertex!=NONE) /* we’ve got clipping to do */
		{
			flagged_world_point3d new_entrance_point, new_exit_point;
			
//			dprintf("entrance_vertex==#%d (%s), exit_vertex==#%d (%s)", entrance_vertex, clipped_entrance_vertex ? "clipped" : "unclipped", exit_vertex, clipped_exit_vertex ? "clipped" : "unclipped");
			
			/* clip the entrance to the clipped area */
			if (clipped_entrance_vertex)
			{
				xz_clip_flagged_world_points(vertices + WRAP_LOW(entrance_vertex, vertex_count-1), vertices + entrance_vertex,
					&new_entrance_point, line);
			}
			else
			{
				new_entrance_point= vertices[entrance_vertex];
			}
			new_entrance_point.flags|= flag;
			
			/* clip the exit from the clipped area */
			if (clipped_exit_vertex)
			{
				xz_clip_flagged_world_points(vertices + WRAP_LOW(exit_vertex, vertex_count-1), vertices + exit_vertex,
					&new_exit_point, line);
			}
			else
			{
				new_exit_point= vertices[WRAP_LOW(exit_vertex, vertex_count-1)];
			}
			new_exit_point.flags|= flag;
			
			/* adjust for the change in number of vertices */
			vertex_delta= entrance_vertex - exit_vertex;
			if (vertex_delta<0)
			{
				if (vertex_delta!=-2) memmove(vertices+entrance_vertex+2, vertices+exit_vertex, (vertex_count-exit_vertex)*sizeof(flagged_world_point3d));
				vertex_delta= vertex_count+vertex_delta;
			}
			else
			{
				/* move down by exit_vertex, add new vertices to end */
				assert(vertex_delta);
				if (exit_vertex)
				{
					memmove(vertices, vertices+exit_vertex, vertex_delta*sizeof(flagged_world_point3d));
					entrance_vertex-= exit_vertex;
				}
			}
			vertex_count= vertex_delta+2;

			vwarn(vertex_count>=3 && vertex_count<=MAXIMUM_VERTICES_PER_WORLD_POLYGON,
				csprintf(temporary, "vertex overflow or underflow (#%d);g;", vertex_count));
				
			if (vertex_count<3 || vertex_count>MAXIMUM_VERTICES_PER_WORLD_POLYGON)
			{
				vertex_count= 0;
			}
			else
			{
				/* and, finally, add the new vertices */
				vertices[entrance_vertex]= new_entrance_point;
				vertices[entrance_vertex+1]= new_exit_point;
			}
		}
	}

#ifdef QUICKDRAW_DEBUG
//	debug_flagged_points3d(vertices, vertex_count);
//	debug_vector(line);
#endif
//	dprintf("result == %p (#%d vertices)", vertices, vertex_count);

	return vertex_count;
}

/* sort points before clipping to assure consistency */
// LP change: make it better able to do long-distance views
void NewRenderRasterizer::xz_clip_flagged_world_points(
	flagged_world_point3d *p0,
	flagged_world_point3d *p1,
	flagged_world_point3d *clipped,
	long_vector2d *line) // world_vector2d *line)
{
	bool swap= (p1->y>p0->y) ? false : ((p0->y==p1->y) ? (p1->x<p0->x) : true);
	flagged_world_point3d *local_p0= swap ? p1 : p0;
	flagged_world_point3d *local_p1= swap ? p0 : p1;
	world_distance dx= local_p1->x - local_p0->x;
	world_distance dy= local_p1->y - local_p0->y;
	world_distance dz= local_p1->z - local_p0->z;
	int32 numerator= line->j*local_p0->x - line->i*local_p0->z;
	int32 denominator= line->i*dz - line->j*dx;
	short shift_count= FIXED_FRACTIONAL_BITS;
	_fixed t;

	/* give numerator 16 significant bits over denominator and then calculate t==n/d;  MPW’s PPCC
		didn’t seem to like (INT32_MIN>>1) and i had to substitute 0xc0000000 instead (hmmm) */
	while (numerator<=(int32)0x3fffffff && numerator>=(int32)0xc0000000 && shift_count--) numerator<<= 1;
	if (shift_count>0) denominator>>= shift_count;
	t = numerator;
	if (denominator)
		t /= denominator;

	/* calculate the clipped point */
	clipped->x= local_p0->x + FIXED_INTEGERAL_PART(t*dx);
	clipped->y= local_p0->y + FIXED_INTEGERAL_PART(t*dy);
	clipped->z= local_p0->z + FIXED_INTEGERAL_PART(t*dz);
	clipped->flags= local_p0->flags&local_p1->flags;

	return;
}
