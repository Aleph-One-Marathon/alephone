#ifndef _RENDER_PLACE_OBJECTS_CLASS_
#define _RENDER_PLACE_OBJECTS_CLASS_
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
#include "NewRenderVisTree.h"


/* ---------- render objects */

struct render_object_data
{
	struct sorted_node_data *node; // node we are being drawn inside.  We generate these after tree is finalized so pointers are safe
	struct clipping_window_data clipping_window; // our privately calculated clipping window
	
	uint16 next_object; // the next object in this chain
	
	struct rectangle_definition rectangle;
	
	int16 ymedia;
};


class RenderPlaceObjsClass
{
	// Auxiliary data and routines:

	void initialize_render_object_list();
	
	uint16 build_render_object(long_point3d *origin,
		_fixed floor_intensity, _fixed ceiling_intensity,
		sorted_node_data **base_nodes, short *base_node_count,
		short object_index, portal_view_data *view);
	
	void sort_render_object_into_tree(uint16 new_object_index,
		sorted_node_data **base_nodes, short base_node_count);

	short build_base_node_list(short origin_polygon_index,
		world_point3d *origin, world_distance left_distance, world_distance right_distance,
		sorted_node_data **base_nodes, portal_view_data *view);
	
	void build_aggregate_render_object_clipping_window(uint16 new_object_index,
		sorted_node_data **base_nodes, short base_node_count);
		
	shape_information_data *rescale_shape_information(shape_information_data *unscaled,
		shape_information_data *scaled, uint16 flags);
	
	// Pointers to view and calculated visibility tree and sorted polygons
	view_data *GlobalView;
	
	render_node_data *Nodes;
	int NodeCount;
	sorted_node_data *SortedNodes;
	int SortedNodeCount;
	
	render_object_data *Object(uint16 i) {return (i<RenderObjects.size() && i>=0)?&RenderObjects[i]:NULL;}
	
public:

	// LP additions: growable list of render objects; these are all the inhabitants
	// Length changed in build_render_object()
	// keep SortedNodes in sync
	vector<render_object_data> RenderObjects;
	
	void SetView(view_data *view) {GlobalView = view;}
	
	void build_render_object_list(NewVisTree *visTree);
	
  	// Inits everything
 	RenderPlaceObjsClass();
};

#endif
