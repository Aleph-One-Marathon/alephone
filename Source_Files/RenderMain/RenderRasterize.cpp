/*

	Copyright (C) 1991-2001 and beyond by Bungie Studios, Inc.
	and the "Aleph One" developers.
 
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	This license is contained in the file "COPYING",
	which is included with this source code; it is available online at
	http://www.gnu.org/licenses/gpl.html
	
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
*/

#include "cseries.h"

#include "map.h"
#include "lightsource.h"
#include "media.h"
#include "RenderRasterize.h"
#include "AnimatedTextures.h"
#include "OGL_Setup.h"
#include "preferences.h"
#include "screen.h"
#include "platforms.h"

#include <string.h>


/* maximum number of vertices a polygon can be world-clipped into (one per clip line) */
#define MAXIMUM_VERTICES_PER_WORLD_POLYGON (MAXIMUM_VERTICES_PER_POLYGON+4)


RenderRasterizerClass::RenderRasterizerClass():
	view(NULL),	// Idiot-proofing
	RSPtr(NULL),
	RasPtr(NULL)
{}


/* ---------- rendering the tree */

void RenderRasterizerClass::render_tree()
{
	render_tree(kDiffuse);
}

void RenderRasterizerClass::render_tree(RenderStep renderStep)
{
	assert(view);	// Idiot-proofing
	assert(RSPtr);
	assert(RasPtr);
	vector<sorted_node_data>::iterator node;
	// LP: reference to simplify the code
	vector<sorted_node_data>& SortedNodes = RSPtr->SortedNodes;
	
	// LP change: added support for semitransparent liquids
	bool SeeThruLiquids = get_screen_mode()->acceleration != _no_acceleration ? TEST_FLAG(Get_OGL_ConfigureData().Flags,OGL_Flag_LiqSeeThru) : graphics_preferences->software_alpha_blending != _sw_alpha_off;
	
	/* walls, ceilings, interior objects, floors, exterior objects for all nodes, back to front */
	for (node= SortedNodes.begin(); node != SortedNodes.end(); ++node)
		render_node(&*node, SeeThruLiquids, renderStep);
}

void RenderRasterizerClass::render_node(
	sorted_node_data *node,
	bool SeeThruLiquids,
	RenderStep renderStep)
{
	polygon_data *polygon= get_polygon_data(node->polygon_index);
	clipping_window_data *window;
	render_object_data *object;

	// LP change: moved this stuff out here because it only has to be calculated
	// once per polygon.
	horizontal_surface_data floor_surface, ceiling_surface;
	short i;
	
	ceiling_surface.origin= polygon->ceiling_origin;
	ceiling_surface.height= polygon->ceiling_height;
	ceiling_surface.texture= polygon->ceiling_texture;
	ceiling_surface.lightsource_index= polygon->ceiling_lightsource_index;
	ceiling_surface.transfer_mode= polygon->ceiling_transfer_mode;
	ceiling_surface.transfer_mode_data= 0;
	
	floor_surface.origin= polygon->floor_origin;
	floor_surface.height= polygon->floor_height;
	floor_surface.texture= polygon->floor_texture;
	floor_surface.lightsource_index= polygon->floor_lightsource_index;
	floor_surface.transfer_mode= polygon->floor_transfer_mode;
	floor_surface.transfer_mode_data= 0;
	
	// Disguise flooded platforms
	if (polygon->type == _polygon_is_platform)
	{
		platform_data *platform = get_platform_data(polygon->permutation);
		if (platform && PLATFORM_IS_FLOODED(platform))
		{
			short adj_index = find_flooding_polygon(node->polygon_index);
			if (adj_index != NONE)
			{
				struct polygon_data *adjacent_polygon= get_polygon_data(adj_index);
				floor_surface.origin= adjacent_polygon->floor_origin;
				floor_surface.height= adjacent_polygon->floor_height;
				floor_surface.texture= adjacent_polygon->floor_texture;
				floor_surface.lightsource_index= adjacent_polygon->floor_lightsource_index;
				floor_surface.transfer_mode= adjacent_polygon->floor_transfer_mode;
				floor_surface.transfer_mode_data= 0;
			}
		}
	}
	
	// The "continue" conditions are OK to move out here, because a non-drawn polygon's
	// inhabitants will be clipped away.
	
	// LP: get liquid data here for convenience;
	// pointer to it being NULL means no liquid in the polygon
	media_data *media = NULL;
	if (polygon->media_index!=NONE)
		media = get_media_data(polygon->media_index);
	
	/* if necessary, replace the ceiling or floor surfaces with the media surface */
	// LP change: don't do this step if liquids are semitransparent
	if (media && !SeeThruLiquids)
	{
		// LP change: moved this get upwards
		// struct media_data *media= get_media_data(polygon->media_index);
		horizontal_surface_data *media_surface= NULL;
				
		if (view->under_media_boundary)
		{
			// LP change: skip if high and dry
			if (media->height <= polygon->floor_height)
				return;
			
			if (media->height<polygon->ceiling_height) media_surface= &ceiling_surface;
		}
		else
		{
			// LP change: skip if submerged
			if (media->height >= polygon->ceiling_height)
				return;
			
			if (media->height>polygon->floor_height) media_surface= &floor_surface;
		}
		
		if (media_surface)
		{
			media_surface->origin= media->origin;
			media_surface->height= media->height;
			media_surface->texture= media->texture;
			media_surface->lightsource_index= polygon->media_lightsource_index;
			media_surface->transfer_mode= media->transfer_mode;
			media_surface->transfer_mode_data= 0;
		}
	}
	// LP change: always render liquids that are semitransparent
	else if (!SeeThruLiquids)
	{
		// if we’re trying to draw a polygon without media from under a polygon with media, don’t
		if (view->under_media_boundary) return;
	}
	
	// LP: this loop renders the walls
	for (window= node->clipping_windows; window; window= window->next_window)
	{
		if (ceiling_surface.height>floor_surface.height)
		{
			/* render ceiling if above viewer */
			if (ceiling_surface.height>view->origin.z)
			{
				// LP change: indicated that the void is on other side
				render_node_floor_or_ceiling(window, polygon, &ceiling_surface, true, true, renderStep);
			}
			
			/* render visible sides */
			for (i= 0; i<polygon->vertex_count; ++i)
			{
				short side_index= polygon->side_indexes[i];
				
				if (side_index!=NONE && TEST_RENDER_FLAG(side_index, _side_is_visible))
				{
					line_data *line= get_line_data(polygon->line_indexes[i]);
					side_data *side= get_side_data(side_index);
					vertical_surface_data surface;
					
					surface.length= line->length;
					store_endpoint(get_endpoint_data(polygon->endpoint_indexes[i]), surface.p0);
					store_endpoint(get_endpoint_data(polygon->endpoint_indexes[WRAP_HIGH(i, polygon->vertex_count-1)]), surface.p1);
					surface.ambient_delta= side->ambient_delta;
					
					// LP change: indicate in all cases whether the void is on the other side;
					// added a workaround for full-side textures with a polygon on the other side
					bool void_present;
					
					switch (side->type)
					{
						case _full_side:
							void_present = true;
							// Suppress the void if there is a polygon on the other side.
							if (polygon->adjacent_polygon_indexes[i] != NONE) void_present = false;
							
							surface.lightsource_index= side->primary_lightsource_index;
							surface.h0= floor_surface.height - view->origin.z;
							surface.hmax= ceiling_surface.height - view->origin.z;
							surface.h1= polygon->ceiling_height - view->origin.z;
							surface.texture_definition= &side->primary_texture;
							surface.transfer_mode= side->primary_transfer_mode;
							render_node_side(window, &surface, void_present, renderStep);
							break;
						case _split_side: /* render _low_side first */
							surface.lightsource_index= side->secondary_lightsource_index;
							surface.h0= floor_surface.height - view->origin.z;
							surface.h1= MAX(line->highest_adjacent_floor, floor_surface.height) - view->origin.z;
							surface.hmax= ceiling_surface.height - view->origin.z;
							surface.texture_definition= &side->secondary_texture;
							surface.transfer_mode= side->secondary_transfer_mode;
							render_node_side(window, &surface, true, renderStep);
							/* fall through and render high side */
						case _high_side:
							surface.lightsource_index= side->primary_lightsource_index;
							surface.h0= MIN(line->lowest_adjacent_ceiling, ceiling_surface.height) - view->origin.z;
							surface.hmax= ceiling_surface.height - view->origin.z;
							surface.h1= polygon->ceiling_height - view->origin.z;
							surface.texture_definition= &side->primary_texture;
							surface.transfer_mode= side->primary_transfer_mode;
							render_node_side(window, &surface, true, renderStep);
							// render_node_side(view, destination, window, &surface);
							break;
						case _low_side:
							surface.lightsource_index= side->primary_lightsource_index;
							surface.h0= floor_surface.height - view->origin.z;
							surface.h1= MAX(line->highest_adjacent_floor, floor_surface.height) - view->origin.z;
							surface.hmax= ceiling_surface.height - view->origin.z;
							surface.texture_definition= &side->primary_texture;
							surface.transfer_mode= side->primary_transfer_mode;
							render_node_side(window, &surface, true, renderStep);
							// render_node_side(view, destination, window, &surface);
							break;
						
						default:
							assert(false);
							break;
					}
					
					if (side->transparent_texture.texture!=UNONE)
					{
						surface.lightsource_index= side->transparent_lightsource_index;
						surface.h0= MAX(line->highest_adjacent_floor, floor_surface.height) - view->origin.z;
						surface.h1= line->lowest_adjacent_ceiling - view->origin.z;
						surface.hmax= ceiling_surface.height - view->origin.z;
						surface.texture_definition= &side->transparent_texture;
						surface.transfer_mode= side->transparent_transfer_mode;
						render_node_side(window, &surface, false, renderStep);
					}
				}
			}
			
			/* render floor if below viewer */
			if (floor_surface.height<view->origin.z)
			{
				// LP change: indicated that the void is on other side
				render_node_floor_or_ceiling(window, polygon, &floor_surface, true, false, renderStep);
			}
		}
	}

	// LP: this is for objects on the other side of the liquids;
	// render them out here if one can see through the liquids
	if (SeeThruLiquids)
	{
		/* render exterior objects (with their own clipping windows) */
		for (object= node->exterior_objects; object; object= object->next_object)
		{
			render_node_object(object, true, renderStep);
		}
	}
	
	// LP: render the liquid surface after the walls and the stuff behind it
	// and before the stuff before it.
	if (media && SeeThruLiquids)
	{
		
		// Render only if between the floor and the ceiling:
		if (media->height > polygon->floor_height && media->height < polygon->ceiling_height)
		{
		
			// Render the liquids
			bool ceil = (media->height > view->origin.z);
			horizontal_surface_data LiquidSurface;
			
			LiquidSurface.origin= media->origin;
			LiquidSurface.height= media->height;
			LiquidSurface.texture= media->texture;
			LiquidSurface.lightsource_index= polygon->media_lightsource_index;
			LiquidSurface.transfer_mode= media->transfer_mode;
			LiquidSurface.transfer_mode_data= 0;
			
			for (window= node->clipping_windows; window; window= window->next_window)
			{
				render_node_floor_or_ceiling(window, polygon, &LiquidSurface, false, ceil, renderStep);
			}
		}
	}
	
	// LP: this is for objects on the view side of the liquids
	/* render exterior objects (with their own clipping windows) */
	for (object= node->exterior_objects; object; object= object->next_object)
	{
		render_node_object(object, false, renderStep);
	}
}

void RenderRasterizerClass::store_endpoint(
	endpoint_data *endpoint,
	long_vector2d& p)
{
	overflow_short_to_long_2d(endpoint->transformed, endpoint->flags, p);
}


/* ---------- rendering ceilings and floors */

// LP change: added "void present on other side" flag
void RenderRasterizerClass::render_node_floor_or_ceiling(
	clipping_window_data *window,
	polygon_data *polygon,
	horizontal_surface_data *surface,
	bool void_present,
	bool ceil,
	RenderStep renderStep)
{
	// LP addition: animated-texture support
	// Extra variable defined so as not to edit the original texture
	shape_descriptor Texture = AnimTxtr_Translate(surface->texture);
	if (Texture!=UNONE)
	{
		struct polygon_definition textured_polygon;
		flagged_world_point2d vertices[MAXIMUM_VERTICES_PER_WORLD_POLYGON];
		world_distance adjusted_height= surface->height-view->origin.z;
		int32 transformed_height= adjusted_height*view->world_to_screen_y;
		short vertex_count;
		short i;
	
		/* build transformed vertex list */
		vertex_count= polygon->vertex_count;
		for (i=0;i<vertex_count;++i)
		{
			// LP change: expanded the transformed-endpoint access
			long_vector2d temp_vertex;
			endpoint_data *endpoint = get_endpoint_data(polygon->endpoint_indexes[i]);
			overflow_short_to_long_2d(endpoint->transformed,endpoint->flags,temp_vertex);
			vertices[i].x = temp_vertex.i;
			vertices[i].y = temp_vertex.j;
			vertices[i].flags= 0;
		}
		
		/* reversing the order in which these two were clipped caused weird problems */
		
		/* clip to left and right sides of the window */
		vertex_count= xy_clip_horizontal_polygon(vertices, vertex_count, &window->left, _clip_left);
		vertex_count= xy_clip_horizontal_polygon(vertices, vertex_count, &window->right, _clip_right);
		
		/* clip to top and bottom sides of the window */
		vertex_count= z_clip_horizontal_polygon(vertices, vertex_count, &window->top, adjusted_height, _clip_up);
		vertex_count= z_clip_horizontal_polygon(vertices, vertex_count, &window->bottom, adjusted_height, _clip_down);
	
		if (vertex_count)
		{
			/* transform the points we have into screen-space (backwards for ceiling polygons) */
			for (i=0;i<vertex_count;++i)
			{
				flagged_world_point2d *world= vertices + (adjusted_height>0 ? vertex_count-i-1 : i);
				point2d *screen= textured_polygon.vertices + i;
				
				switch (world->flags&(_clip_left|_clip_right))
				{
					case 0:
						// LP change: making it long-distance friendly
						{
						int32 world_x = world->x ? world->x : 1; // fix for division by zero error
						int32 screen_x= view->half_screen_width + (world->y*view->world_to_screen_x)/world_x;
						screen->x= PIN(screen_x, 0, view->screen_width);
						}
						break;
					case _clip_left: screen->x= window->x0; break;
					case _clip_right: screen->x= window->x1; break;
					default:
						screen->x= window->x0;
						break;
				}
				
				switch (world->flags&(_clip_up|_clip_down))
				{
					case 0:
						// LP change: making it long-distance friendly
						{
						int32 world_x = world->x ? world->x : 1; // fix for division by zero error
						int32 screen_y= view->half_screen_height - transformed_height/world_x + view->dtanpitch;
						screen->y= PIN(screen_y, 0, view->screen_height);
						}
						break;
					case _clip_up: screen->y= window->y0; break;
					case _clip_down: screen->y= window->y1; break;
					default:
						screen->y= window->y0;
						break;
				}
				// vassert(screen->y>=0&&screen->y<=view->screen_height, csprintf(temporary, "horizontal: flags==%x, window @ %p", world->flags, window));
			}
			
			/* setup the other parameters of the textured polygon */
			textured_polygon.flags= 0;
			textured_polygon.origin.x= view->origin.x + surface->origin.x;
			textured_polygon.origin.y= view->origin.y + surface->origin.y;
			textured_polygon.origin.z= adjusted_height;
			get_shape_bitmap_and_shading_table(Texture, &textured_polygon.texture, &textured_polygon.shading_tables, view->shading_mode);
			// Bug out if bitmap is nonexistent
			if (!textured_polygon.texture) return;
			
			textured_polygon.ShapeDesc = Texture;
			textured_polygon.ambient_shade= get_light_intensity(surface->lightsource_index);
			textured_polygon.vertex_count= vertex_count;
			instantiate_polygon_transfer_mode(view, &textured_polygon, surface->transfer_mode, true);
			if (view->shading_mode==_shading_infravision) textured_polygon.flags|= _SHADELESS_BIT;
			
			/* and, finally, map it */
			// LP: added OpenGL support; also presence of void on other side
			textured_polygon.VoidPresent = void_present;
			// LP: using rasterizer object
			RasPtr->texture_horizontal_polygon(textured_polygon);
		}
	}
}

/* ---------- rendering sides (walls) */

// LP change: added "void present on other side" flag
void RenderRasterizerClass::render_node_side(
	clipping_window_data *window,
	vertical_surface_data *surface,
	bool void_present,
	RenderStep renderStep)
{
	world_distance h= MIN(surface->h1, surface->hmax);
	
	// LP addition: animated-texture support
	// Extra variable defined so as not to edit the original texture
	shape_descriptor Texture = AnimTxtr_Translate(surface->texture_definition->texture);
	if (h>surface->h0 && Texture!=UNONE)
	{
		struct polygon_definition textured_polygon;
		flagged_world_point2d posts[2];
		flagged_world_point3d vertices[MAXIMUM_VERTICES_PER_WORLD_POLYGON];
		short vertex_count;
		short i;
	
		/* initialize the two posts of our trapezoid */
		vertex_count= 2;
		posts[0].x= surface->p0.i; posts[0].y= surface->p0.j; posts[0].flags= 0;
		posts[1].x= surface->p1.i; posts[1].y= surface->p1.j; posts[1].flags= 0;
	
		/* clip to left and right sides of the cone (must happen in the same order as horizontal polygons) */
		vertex_count= xy_clip_line(posts, vertex_count, &window->left, _clip_left);
		vertex_count= xy_clip_line(posts, vertex_count, &window->right, _clip_right);
		
		if (vertex_count)
		{
			/* build a polygon out of the two posts */
			vertex_count= 4;
			vertices[0].z= vertices[1].z= h;
			vertices[2].z= vertices[3].z= surface->h0;
			vertices[0].x= vertices[3].x= posts[0].x, vertices[0].y= vertices[3].y= posts[0].y;
			vertices[1].x= vertices[2].x= posts[1].x, vertices[1].y= vertices[2].y= posts[1].y;
			vertices[0].flags= vertices[3].flags= posts[0].flags;
			vertices[1].flags= vertices[2].flags= posts[1].flags;
		
			/* clip to top and bottom sides of the window; because xz_clip_vertical_polygon accepts
				vertices in clockwise or counterclockwise order, we can set our vertex list up to be
				clockwise on the screen and never worry about it after that */
			vertex_count= xz_clip_vertical_polygon(vertices, vertex_count, &window->top, _clip_up);
			vertex_count= xz_clip_vertical_polygon(vertices, vertex_count, &window->bottom, _clip_down);
			
			if (vertex_count)
			{
				world_distance dx= surface->p1.i - surface->p0.i;
				world_distance dy= surface->p1.j - surface->p0.j;
				world_distance x0= WORLD_FRACTIONAL_PART(surface->texture_definition->x0);
				world_distance y0= WORLD_FRACTIONAL_PART(surface->texture_definition->y0);
				
				/* calculate texture origin and direction */	
				world_distance divisor = surface->length;
				if (divisor == 0)
					divisor = 1;
				textured_polygon.vector.i= (WORLD_ONE*dx)/divisor;
				textured_polygon.vector.j= (WORLD_ONE*dy)/divisor;
				textured_polygon.vector.k= -WORLD_ONE;
				textured_polygon.origin.x= surface->p0.i - (x0*dx)/divisor;
				textured_polygon.origin.y= surface->p0.j - (x0*dy)/divisor;
				textured_polygon.origin.z= surface->h1 + y0;
	
				/* transform the points we have into screen-space */
				for (i=0;i<vertex_count;++i)
				{
					flagged_world_point3d *world= vertices + i;
					point2d *screen= textured_polygon.vertices + i;
					
					switch (world->flags&(_clip_left|_clip_right))
					{
						case 0:
							// LP change: making it long-distance friendly
							{
								int32 screen_x= view->half_screen_width + (world->x ? (world->y*view->world_to_screen_x)/world->x : 0);
							screen->x= PIN(screen_x, 0, view->screen_width);
							}
							break;
						case _clip_left: screen->x= window->x0; break;
						case _clip_right: screen->x= window->x1; break;
						default:
							screen->x= window->x0;
							break;
					}
					
					switch (world->flags&(_clip_up|_clip_down))
					{
						case 0:
							// LP change: making it long-distance friendly
							{
								int32 screen_y= view->half_screen_height - (world->x ? (world->z*view->world_to_screen_y)/world->x : 0) + view->dtanpitch;
							screen->y= PIN(screen_y, 0, view->screen_height);
							}
							break;
						case _clip_up: screen->y= window->y0; break;
						case _clip_down: screen->y= window->y1; break;
						default:
							screen->y= window->y0;
							break;
					}
					// vassert(screen->y>=0&&screen->y<=view->screen_height, csprintf(temporary, "#%d!in[#0,#%d]: flags==%x, wind@%p #%d w@%p s@%p", screen->y, view->screen_height, world->flags, window, vertex_count, world, screen));
				}
				
				/* setup the other parameters of the textured polygon */
				textured_polygon.flags= 0;
				get_shape_bitmap_and_shading_table(Texture, &textured_polygon.texture, &textured_polygon.shading_tables, view->shading_mode);
				// Bug out if bitmap is nonexistent
				if (!textured_polygon.texture) return;

				textured_polygon.ShapeDesc = Texture;
				textured_polygon.ambient_shade= get_light_intensity(surface->lightsource_index) + surface->ambient_delta;
				textured_polygon.ambient_shade= PIN(textured_polygon.ambient_shade, 0, FIXED_ONE);
				textured_polygon.vertex_count= vertex_count;
				instantiate_polygon_transfer_mode(view, &textured_polygon, surface->transfer_mode, false);
				if (view->shading_mode==_shading_infravision) textured_polygon.flags|= _SHADELESS_BIT;
				
				/* and, finally, map it */
				// LP: added OpenGL support; also presence of void on other side
				textured_polygon.VoidPresent = void_present;
				// LP: using rasterizer object
				RasPtr->texture_vertical_polygon(textured_polygon);
			}
		}
	}
}

/* ---------- rendering objects */

void RenderRasterizerClass::render_node_object(
	render_object_data *object,
	bool other_side_of_media,
	RenderStep renderStep)
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
	}
}


/* ---------- horizontal polygon clipping */

enum /* xy_clip_horizontal_polygon() states */
{
	_testing_first_vertex, /* are we in or out? */
	_searching_cw_for_in_out_transition,
	_searching_ccw_for_out_in_transition,
	_searching_cw_for_out_in_transition
};

// LP change: make it better able to do long-distance views
short RenderRasterizerClass::xy_clip_horizontal_polygon(
	flagged_world_point2d *vertices,
	short vertex_count,
	long_vector2d *line,
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
void RenderRasterizerClass::xy_clip_flagged_world_points(
	flagged_world_point2d *p0,
	flagged_world_point2d *p1,
	flagged_world_point2d *clipped,
	long_vector2d *line)
{
	bool swap= (p1->y>p0->y) ? false : ((p0->y==p1->y) ? (p1->x<p0->x) : true);
	flagged_world_point2d *local_p0= swap ? p1 : p0;
	flagged_world_point2d *local_p1= swap ? p0 : p1;
	world_distance dx= local_p1->x - local_p0->x;
	world_distance dy= local_p1->y - local_p0->y;
	int32 numerator = int32(1LL*line->j*local_p0->x - 1LL*line->i*local_p0->y);
	int32 denominator = int32(1LL*line->i*dy - 1LL*line->j*dx);
	short shift_count= FIXED_FRACTIONAL_BITS;
	_fixed t;

	/* give numerator 16 significant bits over denominator and then calculate t==n/d;  MPW’s PPCC
		didn’t seem to like (INT32_MIN>>1) and i had to substitute 0xc0000000 instead (hmmm) */
	while (numerator<=(int32)0x3fffffff && numerator>=(int32)0xc0000000 && shift_count--) numerator<<= 1;
	if (shift_count>0) denominator>>= shift_count;
	t= numerator;
	if (denominator) 
		t /= denominator;

	/* calculate the clipped point */
	clipped->x = local_p0->x + FIXED_INTEGERAL_PART(int32(1LL*t*dx));
	clipped->y = local_p0->y + FIXED_INTEGERAL_PART(int32(1LL*t*dy));
	clipped->flags= local_p0->flags&local_p1->flags;
}

/* almost wholly identical to xz_clip_vertical_polygon() except that this works off 2d points
	in the xy-plane and a height */
// LP change: make it better able to do long-distance views
short RenderRasterizerClass::z_clip_horizontal_polygon(
	flagged_world_point2d *vertices,
	short vertex_count,
	long_vector2d *line, /* i==x, j==z */
	world_distance height,
	uint16 flag)
{
	CROSSPROD_TYPE heighti= CROSSPROD_TYPE(line->i)*height;
	
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
			CROSSPROD_TYPE cross_product= heighti - CROSSPROD_TYPE(line->j)*vertices[vertex_index].x;

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
void RenderRasterizerClass::z_clip_flagged_world_points(
	flagged_world_point2d *p0,
	flagged_world_point2d *p1,
	world_distance height,
	flagged_world_point2d *clipped,
	long_vector2d *line)
{
	bool swap= (p1->y>p0->y) ? false : ((p0->y==p1->y) ? (p1->x<p0->x) : true);
	flagged_world_point2d *local_p0= swap ? p1 : p0;
	flagged_world_point2d *local_p1= swap ? p0 : p1;
	world_distance dx= local_p1->x - local_p0->x;
	world_distance dy= local_p1->y - local_p0->y;
	int32 numerator = int32(1LL*line->j*local_p0->x - 1LL*line->i*height);
	int32 denominator = int32(-1LL*line->j*dx);
	short shift_count= FIXED_FRACTIONAL_BITS;
	_fixed t;

	/* give numerator 16 significant bits over denominator and then calculate t==n/d;  MPW’s PPCC
		didn’t seem to like (INT32_MIN>>1) and i had to substitute 0xc0000000 instead (hmmm) */
	while (numerator<=(int32)0x3fffffff && numerator>=(int32)0xc0000000 && shift_count--) numerator<<= 1;
	if (shift_count>0) denominator>>= shift_count;
	t= numerator;
	if (denominator) 
		t /= denominator;

	/* calculate the clipped point */
	clipped->x = local_p0->x + FIXED_INTEGERAL_PART(int32(1LL*t*dx));
	clipped->y = local_p0->y + FIXED_INTEGERAL_PART(int32(1LL*t*dy));
	clipped->flags= local_p0->flags&local_p1->flags;
}

/* ---------- vertical polygon clipping */

// LP change: make it better able to do long-distance views
short RenderRasterizerClass::xy_clip_line(
	flagged_world_point2d *posts,
	short vertex_count,
	long_vector2d *line,
	uint16 flag)
{
#ifdef QUICKDRAW_DEBUG
//	debug_flagged_points(posts, vertex_count);
//	debug_vector(line);
#endif
//	dprintf("clipping %p (#%d) to line (%d,%d)", posts, vertex_count, line->i, line->j);
	
	if (vertex_count)
	{
		CROSSPROD_TYPE cross_product0= CROSSPROD_TYPE(line->i)*posts[0].y - CROSSPROD_TYPE(line->j)*posts[0].x;
		CROSSPROD_TYPE cross_product1= CROSSPROD_TYPE(line->i)*posts[1].y - CROSSPROD_TYPE(line->j)*posts[1].x;
		
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
short RenderRasterizerClass::xz_clip_vertical_polygon(
	flagged_world_point3d *vertices,
	short vertex_count,
	long_vector2d *line, /* i==x, j==z */
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
			CROSSPROD_TYPE cross_product= CROSSPROD_TYPE(line->i)*vertices[vertex_index].z - CROSSPROD_TYPE(line->j)*vertices[vertex_index].x;
			
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
void RenderRasterizerClass::xz_clip_flagged_world_points(
	flagged_world_point3d *p0,
	flagged_world_point3d *p1,
	flagged_world_point3d *clipped,
	long_vector2d *line)
{
	bool swap= (p1->y>p0->y) ? false : ((p0->y==p1->y) ? (p1->x<p0->x) : true);
	flagged_world_point3d *local_p0= swap ? p1 : p0;
	flagged_world_point3d *local_p1= swap ? p0 : p1;
	world_distance dx= local_p1->x - local_p0->x;
	world_distance dy= local_p1->y - local_p0->y;
	world_distance dz= local_p1->z - local_p0->z;
	int32 numerator = int32(1LL*line->j*local_p0->x - 1LL*line->i*local_p0->z);
	int32 denominator = int32(1LL*line->i*dz - 1LL*line->j*dx);
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
	clipped->x = local_p0->x + FIXED_INTEGERAL_PART(int32(1LL*t*dx));
	clipped->y = local_p0->y + FIXED_INTEGERAL_PART(int32(1LL*t*dy));
	clipped->z = local_p0->z + FIXED_INTEGERAL_PART(int32(1LL*t*dz));
	clipped->flags= local_p0->flags&local_p1->flags;
}
