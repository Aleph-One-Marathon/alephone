#ifndef _RENDER_RASTERIZER_CLASS_
#define _RENDER_RASTERIZER_CLASS_
/*

	Copyright (C) 1991-2001 and beyond by Bungie Studios, Inc.
	and the "Aleph One" developers.
 
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
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
#include "Rasterizer.h"


/* ---------- flagged world points */

// IR change: why not inherit long_point2d?
struct flagged_world_point2d : public long_point2d /* for floors */
{
	// LP change: made this more long-distance friendly
<<<<<<< RenderRasterize.h
	// IR change: made it inherit long_point2d, so don't need these.
	//int32 x, y;
	// world_distance x, y;
=======
	int32 x, y;
>>>>>>> 1.11
	uint16 flags; /* _clip_left, _clip_right, _clip_top, _clip_bottom are valid */
	// IR addition:
	flagged_world_point2d() {}
	flagged_world_point2d(const long_point2d& it) : long_point2d(it) {}
};

// IR change: why not inherit flagged_world_point2d?
struct flagged_world_point3d : public flagged_world_point2d /* for ceilings */
{
	// LP change: made this more long-distance friendly
	// IR change: made it inherit flagged_world_point2d, so don't need these.
	//int32 x, y;
	world_distance z;
<<<<<<< RenderRasterize.h
	// world_distance x, y, z;
	// IR removed: don't need this either, its inherited
	//uint16 flags; /* _clip_left, _clip_right, _clip_top, _clip_bottom are valid */
	flagged_world_point3d() {}
	flagged_world_point3d(const long_point2d& it) : flagged_world_point2d(it) {}
	flagged_world_point3d(const flagged_world_point2d& it) : flagged_world_point2d(it) {}
=======
	uint16 flags; /* _clip_left, _clip_right, _clip_top, _clip_bottom are valid */
>>>>>>> 1.11
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
	
	struct side_texture_definition *texture_definition;
// IR removed: noew in side_texture_definition
//	short transfer_mode;
};


class RenderRasterizerClass
{
	// Auxiliary data and routines:
	
	// LP change: indicate whether the void is present on one side;
	// useful for suppressing semitransparency to the void
	void render_node_floor_or_ceiling(
		clipping_window_data *window, polygon_data *polygon, horizontal_surface_data *surface,
		bool void_present);
	void render_node_side(
		clipping_window_data *window, vertical_surface_data *surface,
		bool void_present);
	void render_clip_debug_lines(clipping_window_data *window);

	// LP change: add "other side of media" flag, to indicate that the sprite will be rendered
	// on the opposite side of the liquid surface from the viewpoint, instead of the same side.
	void render_node_object(render_object_data *object, bool other_side_of_media);

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
	
public:
	
	// Pointers to view and sorted polygons
	view_data *view;
	RenderSortPolyClass *RSPtr;
	RasterizerClass *RasPtr;
	
	void render_tree();
	
  	// Inits everything
 	RenderRasterizerClass();
};


#endif
