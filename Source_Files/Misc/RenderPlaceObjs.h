#ifndef _RENDER_PLACE_OBJECTS_CLASS_
#define _RENDER_PLACE_OBJECTS_CLASS_
/*
	
	Rendering Object-Placement Class
	by Loren Petrich,
	August 6, 2000
	
	Defines a class for placing inhabitants in appropriate rendering order; from render.c
	Works from RenderSortPoly stuff.
	
	Made [view_data *view] a member and removed it as an argument
*/

#include "GrowableList.h"
#include "ResizableList.h"
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
	
	short ymedia;
};


class RenderPlaceObjsClass
{
	// Auxiliary data and routines:

	void initialize_render_object_list();
	
	render_object_data *build_render_object(long_point3d *origin,
		fixed ambient_intensity, sorted_node_data **base_nodes, short *base_node_count,
		short object_index);
	
	void sort_render_object_into_tree(render_object_data *new_render_object,
		sorted_node_data **base_nodes, short base_node_count);

	short build_base_node_list(short origin_polygon_index,
		world_point3d *origin, world_distance left_distance, world_distance right_distance,
		sorted_node_data **base_nodes);
	
	void build_aggregate_render_object_clipping_window(render_object_data *render_object,
		sorted_node_data **base_nodes, short base_node_count);
		
	shape_information_data *rescale_shape_information(shape_information_data *unscaled,
		shape_information_data *scaled, word flags);

public:

	// LP additions: growable list of render objects; these are all the inhabitants
	// Length changed in build_render_object()
	// keep SortedNodes in sync
	GrowableList<render_object_data> RenderObjects;
	
	// Pointers to view and calculated visibility tree and sorted polygons
	view_data *view;
	RenderVisTreeClass *RVPtr;
	RenderSortPolyClass *RSPtr;
	
	void build_render_object_list();
	
  	// Inits everything
 	RenderPlaceObjsClass();
};

// Historical note: cause of too-many-transparent-line errors
// LP addition: node-alias growable list
// Only used in sort_render_tree()
// Suppressed as unnecessary because of node_data polygon-sorted-tree structure
// static GrowableList<node_data *> NodeAliases(32);


#endif
