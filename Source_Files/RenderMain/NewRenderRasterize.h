#ifndef _RENDER_RASTERIZER_CLASS_
#define _RENDER_RASTERIZER_CLASS_
/*
	
	Rendering Clipping/Rasterization Class
	by Loren Petrich,
	August 7, 2000
	
	Defines a class for doing rasterization from the prepared lists of objects; from render.c
	
	Made [view_data *view] a member and removed it as an argument
	Doing the setup and rasterization of each object through a RasterizerClass object
	Also removed [bitmap_definition *destination] as superfluous,
		now that there is a special rasterizer object that can contain it.
	
Oct 13, 2000
	LP: replaced ResizableList with STL vector class

Aug 12, 2001 (Ian Rickard):
	Tweak for some structure rearranging
*/

#include <vector>
#include "world.h"
#include "render.h"
//#include "RenderSortPoly.h"
//#include "RenderPlaceObjs.h"
#include "NewRenderVisTree.h"
#include "RenderPlaceObjs.h"
#include "Rasterizer.h"


/* ---------- flagged world points */

struct flagged_world_point2d : public long_point2d /* for floors? */
{
	/* inherited: 
	int32 x, y;
	*/
	
	uint16 flags; // _clip_left, _clip_right, _clip_top, _clip_bottom are valid
	// IR addition:
	flagged_world_point2d() {}
	flagged_world_point2d(const long_point2d& it) : long_point2d(it) {}
};

struct flagged_world_point3d : public flagged_world_point2d /* for ceilings? */
{
	/* inherited: 
	int32 x, y;
	uint16 flags; // _clip_left, _clip_right, _clip_top, _clip_bottom are valid
	*/

	world_distance z;

	flagged_world_point3d() {}
	flagged_world_point3d(const long_point2d& it) : flagged_world_point2d(it) {}
	flagged_world_point3d(const flagged_world_point2d& it) : flagged_world_point2d(it) {}
};


/* ---------- vertical surface definition */

/* it’s not worth putting this into the side_data structure, although the transfer mode should
	be in the side_texture_definition structure */
struct vertical_surface_data
{
// IR removed: noew in side_texture_definition
//	short lightsource_index;
	_fixed ambient_delta; /* a delta to the lightsource’s intensity, then pinned to [0,FIXED_ONE] */
	
	world_distance length;
	world_distance h0, h1, hmax; /* h0<h1; hmax<=h1 and is the height where this wall side meets the ceiling */
	// LP change: made this more long-distance friendly
	long_vector2d p0, p1; /* will transform into left, right points on the screen (respectively) */
	// world_point2d p0, p1; /* will transform into left, right points on the screen (respectively) */
	
	struct side_texture_definition *texture_definition;
// IR removed: noew in side_texture_definition
//	short transfer_mode;
};


class NewRenderRasterizer
{
	// Auxiliary data and routines:
	
	// LP change: indicate whether the void is present on one side;
	// useful for suppressing semitransparency to the void
	void render_node_floor_or_ceiling(
		clipping_window_data *vis_extents, int rasterize_windows_used,
		polygon_data *polygon, horizontal_surface_data *surface,
		bool void_present, portal_view_data *portalView);
	void render_node_side(
		clipping_window_data *vis_extents, int rasterize_windows_used,
		vertical_surface_data *surface, bool void_present, portal_view_data *portalView);
	void render_clip_debug_lines(clipping_window_data *windows);

	// LP change: add "other side of media" flag, to indicate that the sprite will be rendered
	// on the opposite side of the liquid surface from the viewpoint, instead of the same side.
	void render_node_object(
		clipping_window_data *vis_extents, int rasterize_windows_used,
		render_object_data *object, world_distance clip_high,
		world_distance clip_low, portal_view_data *portalView);

	// LP changes for better long-distance support
	
	short xy_clip_horizontal_polygon(flagged_world_point2d *vertices, short vertex_count,
		long_vector2d *line, uint16 flag);
	
	void xy_clip_flagged_world_points(flagged_world_point2d *p0, flagged_world_point2d *p1,
		flagged_world_point2d *clipped, long_vector2d *line);
	
	short z_clip_horizontal_polygon(flagged_world_point2d *vertices, short vertex_count,
		long_vector2d *line, world_distance height, uint16 flag);
	
	void z_clip_flagged_world_points(flagged_world_point2d *p0, flagged_world_point2d *p1,
		world_distance height, flagged_world_point2d *clipped, long_vector2d *line);
	
	short xz_clip_vertical_polygon(flagged_world_point3d *vertices, short vertex_count,
		long_vector2d *line, uint16 flag);
	
	void xz_clip_flagged_world_points(flagged_world_point3d *p0, flagged_world_point3d *p1,
		flagged_world_point3d *clipped, long_vector2d *line);
	
	short xy_clip_line(flagged_world_point2d *posts, short vertex_count,
		long_vector2d *line, uint16 flag);
	
	render_node_data* nodes;
	portal_view_data* portalViews;
	clipping_window_data *windows;
	translated_endpoint_data *endpoints;
	render_object_data *objects;
	
	int nodeCount, portalViewCount, windowCount, endpointCount, objectCount;
	
	void checkNodeIndex(uint32 index)       {assert (index>=0 && index<nodeCount);}
	void checkPortalViewIndex(uint32 index) {assert (index>=0 && index<portalViewCount);}
	void checkWindowIndex(uint32 index)     {assert (index>=0 && index<windowCount);}
	void checkEndpointIndex(uint32 index)   {assert (index>=0 && index<endpointCount);}
	void checkObjectIndex(uint32 index)     {assert (index>=0 && index<objectCount);}
	
	RasterizerClass *rast;
	view_data *GlobalView;

public:
	
	// Pointers to view and sorted polygons
	
	void SetRasterizer(RasterizerClass *Ras) {rast = Ras;}
	void SetView(view_data *View) {GlobalView = View;}
	
	void render_tree(NewVisTree *tree, RenderPlaceObjsClass *objs);
	void fake_render_tree(NewVisTree *tree); // just generates automap.
	
  	// nulls out the pointers
 	NewRenderRasterizer();
};


#endif
