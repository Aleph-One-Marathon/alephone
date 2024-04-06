#ifndef _RENDER_PLACE_OBJECTS_CLASS_
#define _RENDER_PLACE_OBJECTS_CLASS_
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
	
	Rendering Object-Placement Class
	by Loren Petrich,
	August 6, 2000
	
	Defines a class for placing inhabitants in appropriate rendering order; from render.c
	Works from RenderSortPoly stuff.
	
	Made [view_data *view] a member and removed it as an argument
	
Oct 13, 2000
	LP: replaced GrowableLists and ResizableLists with STL vectors
*/

#include <vector>
#include "world.h"
#include "interface.h"
#include "render.h"
#include "RenderSortPoly.h"


/* ---------- render objects */

struct render_object_data
{
	struct sorted_node_data *node; /* node we are being drawn inside */
	struct clipping_window_data *clipping_windows; /* our privately calculated clipping window */
	
	struct render_object_data *next_object; /* the next object in this chain */
	
	struct rectangle_definition rectangle;
	
	int16 ymedia;
};


class RenderPlaceObjsClass
{
	struct span_data;

	void initialize_render_object_list();
	
	render_object_data* build_render_object(
		object_data* object,
		_fixed floor_intensity,
		_fixed ceiling_intensity,
		float Opacity,
		long_point3d* origin,
		long_point3d* rel_origin);
	
	void sort_render_object_into_tree(render_object_data* new_render_object, const span_data& span);

	span_data build_base_node_list(const render_object_data* render_object, short origin_polygon_index);
	
	void build_aggregate_render_object_clipping_window(render_object_data* render_object, const span_data& span);
		
	bool add_object_to_sorted_nodes(
		object_data* object,
		_fixed floor_intensity,
		_fixed ceiling_intensity,
		float Opacity);
	
	shape_information_data *rescale_shape_information(shape_information_data *unscaled,
		shape_information_data *scaled, uint16 flags);

public:

	// LP additions: growable list of render objects; these are all the inhabitants
	// Length changed in build_render_object()
	// keep SortedNodes in sync
	vector<render_object_data> RenderObjects;
	
	// Pointers to view and calculated visibility tree and sorted polygons
	view_data *view;
	RenderVisTreeClass *RVPtr;
	RenderSortPolyClass *RSPtr;
	
	void build_render_object_list();
	
  	// Inits everything
 	RenderPlaceObjsClass();
};

#endif
