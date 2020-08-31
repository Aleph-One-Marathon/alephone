#ifndef _RENDER_VISIBILITY_TREE_CLASS_
#define _RENDER_VISIBILITY_TREE_CLASS_
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
	
	Rendering Visibility-Tree Class
	by Loren Petrich,
	August 6, 2000
	
	Defines a class for doing rendering visibility; from render.c
	
	Made [view_data *view] a member and removed it as an argument
	
Sep. 11, 2000 (Loren Petrich):
	Made POINTER_DATA more general -- byte *, so one can do pointer arithmetic on it correctly.

Sep. 15, 2000 (Loren Petrich):
	Fixed stale-pointer bug in cast_render_ray() by using index instead of pointer to parent node
	
Oct 13, 2000
	LP: replaced GrowableLists and ResizableLists with STL vectors
*/

#include <deque>
#include <vector>
#include "map.h"
#include "render.h"


// Made pointers more general
typedef byte *POINTER_DATA;
#define POINTER_CAST(x) ((POINTER_DATA)(x))


// LP addition: typecast for cross-products, since these can be large
typedef double CROSSPROD_TYPE;


// Macros
#define WRAP_LOW(n,max) ((n)?((n)-1):(max))
#define WRAP_HIGH(n,max) (((n)==(max))?0:((n)+1))


// LP: put here since it also used in build_base_node_list()
enum /* next_polygon_along_line() states */
{
	_looking_for_first_nonzero_vertex,
	_looking_clockwise_for_right_vertex,
	_looking_counterclockwise_for_left_vertex,
	_looking_for_next_nonzero_vertex
};


/* ---------- clipping data */

enum /* É_clip_data flags */
{
	_clip_left= 0x0001,
	_clip_right= 0x0002,
	_clip_up= 0x0004,
	_clip_down= 0x0008
};

enum /* left and right sides of screen */
{
	indexLEFT_SIDE_OF_SCREEN,
	indexRIGHT_SIDE_OF_SCREEN,
	NUMBER_OF_INITIAL_ENDPOINT_CLIPS
};

enum /* top and bottom sides of screen */
{
	indexTOP_AND_BOTTOM_OF_SCREEN,
	NUMBER_OF_INITIAL_LINE_CLIPS
};

struct line_clip_data
{
	uint16 flags;
	short x0, x1; /* clipping bounds */
	
	// LP change: made more long-distance friendly
	long_vector2d top_vector, bottom_vector;
	short top_y, bottom_y; /* screen-space */
};

struct endpoint_clip_data
{
	uint16 flags;
	short x; /* screen-space */
	
	// LP change: made into a long vector for long views
	long_vector2d vector;
};

struct clipping_window_data
{
	// LP change: split into two sets of vectors: left-right and top-bottom
	long_vector2d left, right;
	long_vector2d top, bottom;
	short x0, x1, y0, y1;
	
	struct clipping_window_data *next_window;
};


/* ---------- nodes */

#define MAXIMUM_CLIPPING_ENDPOINTS_PER_NODE 4
#define MAXIMUM_CLIPPING_LINES_PER_NODE (MAXIMUM_VERTICES_PER_POLYGON-2)

struct node_data
{
	uint16 flags;
	short polygon_index;
	
	/* clipping effects all children (and was caused by crossing from parent) */
	short clipping_endpoint_count;
	short clipping_endpoints[MAXIMUM_CLIPPING_ENDPOINTS_PER_NODE];
	short clipping_line_count;
	short clipping_lines[MAXIMUM_CLIPPING_LINES_PER_NODE];

	node_data *parent; /* a pointer to our parent node (NULL for the root) */	
	node_data **reference; /* a pointer to the pointer which references us (NULL for the root) */
	node_data *siblings; /* pointer to the next sibling */
	node_data *children; /* pointer to the head of the child array */
	
	// LP addition: this is inspired by Rhys Hill's tree structures;
	// this is for membership in a polygon-sort tree.
	// As constructed, its root will be the first node in the list, the viewpoint node.
	node_data *PS_Greater;	// To nodes with higher polygon indices
	node_data *PS_Less;		// To nodes with lower polygon indices
	node_data *PS_Shared;	// To other nodes at the same polygon
};


class RenderVisTreeClass
{
	// Auxiliary data and routines:
	
	// Polygon queue now a growable list; its working size is maintained separately
	vector<short> PolygonQueue;
	size_t polygon_queue_size;
	
	/* translates from map indexes to clip indexes, only valid if appropriate render flag is set */
	vector<size_t> line_clip_indexes;
	
	// Turned preprocessor macro into function
	void PUSH_POLYGON_INDEX(short polygon_index);
	
	void initialize_polygon_queue();
	
	void initialize_render_tree();
	
	void initialize_clip_data();
	
	uint16 next_polygon_along_line(short *polygon_index, world_point2d *origin, long_vector2d *_vector,
		short *clipping_endpoint_index, short *clipping_line_index, short bias);
	
	// LP: referring to parent node by index instead of by pointer, to avoid stale-pointer bug
	void cast_render_ray(long_vector2d *_vector, short endpoint_index,
		node_data* parent, short bias);
	
	uint16 decide_where_vertex_leads(short *polygon_index, short *line_index, short *side_index, short endpoint_index_in_polygon_list,
		world_point2d *origin, long_vector2d *_vector, uint16 clip_flags, short bias);

	void calculate_line_clipping_information(short line_index, uint16 clip_flags);
	
	short calculate_endpoint_clipping_information(short endpoint_index, uint16 clip_flags);
	
	void ResetEndpointClips(void);
	
	void ResetLineClips();
	
public:

	/* gives screen x-coordinates for a map endpoint (only valid if _endpoint_is_visible) */
	vector<short> endpoint_x_coordinates;
	
	/* every time we find a unique endpoint which clips something, we build one of these for it */
	// LP addition: growable list
	// Length changed in calculate_endpoint_clipping_information() and ResetEndpointClips()
	vector<endpoint_clip_data> EndpointClips;

	/* every time we find a unique line which clips something, we build one of these for it (notice
		the translation table from line_indexes on the map to line_clip_indexes in our table for when
		we cross the same clip line again */
	// LP addition: growable list
	// Length changed in calculate_line_clipping_information() and ResetLineClips()
	vector<line_clip_data> LineClips;

	// Growable list of clipping windows
	// Length changed in build_clipping_windows(), initialize_clip_data(),
	// and build_aggregate_render_object_clipping_window();
	// keep sorted-node clipping-window pointers in sync with render-object ones
	vector<clipping_window_data> ClippingWindows;
	
	// Growable list of node_data values
	// Length changed in cast_render_ray() and initialize_render_tree()
	typedef std::deque<node_data> NodeList;
	NodeList Nodes;
	
	// Pointer to view
	view_data *view;
	
	// If true, the render tree will disable exploration
	// polygons (for the M1-style exploration goal).
	bool mark_as_explored;
	
	// If true (default), the render tree will fill out
	// the automap.
	bool add_to_automap;
	
	// Resizes all the objects defined inside;
	// the resizing is lazy
	void Resize(size_t NumEndpoints, size_t NumLines);
	
	// Builds the visibility tree
 	void build_render_tree();
 	
  	// Inits everything
 	RenderVisTreeClass();
};


#endif
