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
	
	Contains the finding of the visibility tree for rendering; from render.c
	
	Made [view_data *view] a member and removed it as an argument

Sep. 15, 2000 (Loren Petrich):
	Fixed stale-pointer bug in cast_render_ray() by using index instead of pointer to parent node.
	Added some code to keep parent and ParentIndex in sync.
	
Oct 13, 2000
	LP: replaced GrowableLists and ResizableLists with STL vectors
*/

#include "cseries.h"

#include "map.h"
#include "RenderVisTree.h"


// LP: "recommended" sizes of stuff in growable lists
#define POLYGON_QUEUE_SIZE 256
#define MAXIMUM_NODES 512
#define MAXIMUM_LINE_CLIPS 256
#define MAXIMUM_ENDPOINT_CLIPS 128
#define MAXIMUM_CLIPPING_WINDOWS 192



enum /* cast_render_ray() flags */
{
	_split_render_ray= 0x8000
};


enum /* cast_render_ray(), next_polygon_along_line() biases */
{
	_no_bias, /* will split at the given endpoint or travel clockwise otherwise */
	_clockwise_bias, /* cross the line clockwise from this endpoint */
	_counterclockwise_bias /* cross the line counterclockwise from this endpoint */
};


// Turned a preprocessor macro into an inline function
inline void INITIALIZE_NODE(node_data *node, short node_polygon_index, uint16 node_flags,
	node_data *node_parent, node_data **node_reference)
{
	node->flags= node_flags;
	node->polygon_index= node_polygon_index;
	node->clipping_endpoint_count= 0;
	node->clipping_line_count= 0;
	node->parent= node_parent;
	node->reference= node_reference;
	node->siblings= NULL;
	node->children= NULL;
	node->PS_Greater = NULL;
	node->PS_Less = NULL;
	node->PS_Shared = NULL;
	// LP addition above: polygon-sort tree data
}


// Inits everything
RenderVisTreeClass::RenderVisTreeClass():
	view(NULL), mark_as_explored(false), add_to_automap(true)
{
	PolygonQueue.reserve(POLYGON_QUEUE_SIZE);
	EndpointClips.reserve(MAXIMUM_ENDPOINT_CLIPS);
	LineClips.reserve(MAXIMUM_LINE_CLIPS);
	ClippingWindows.reserve(MAXIMUM_CLIPPING_WINDOWS);
}


// Resizes all the objects defined inside
void RenderVisTreeClass::Resize(size_t NumEndpoints, size_t NumLines)
{
	endpoint_x_coordinates.resize(NumEndpoints);
	line_clip_indexes.resize(NumLines);
}

// Add a polygon to the polygon queue
void RenderVisTreeClass::PUSH_POLYGON_INDEX(short polygon_index)
{
	if (!TEST_RENDER_FLAG(polygon_index, _polygon_is_visible))
	{
		// Grow the list only if necessary
		if (polygon_queue_size < PolygonQueue.size())
			PolygonQueue[polygon_queue_size]= polygon_index;
		else
			PolygonQueue.push_back(polygon_index);
		polygon_queue_size++;
		
		// polygon_queue[polygon_queue_size++]= polygon_index;
		SET_RENDER_FLAG(polygon_index, _polygon_is_visible);
	}
}

// Main routine
void RenderVisTreeClass::build_render_tree()
{
	assert(view);	// Idiot-proofing

	/* initialize the queue where we remember polygons we need to fire at */
	initialize_polygon_queue();

	/* initialize our node list to contain the root, etc. */
	initialize_render_tree();
	
	/* reset clipping buffers */
	initialize_clip_data();
	
	// LP change:
	// Adjusted for long-vector handling
	// Using start index of list of nodes: 0
	long_vector2d view_edge;
	short_to_long_2d(view->left_edge,view_edge);
	cast_render_ray(&view_edge, NONE, &Nodes.front(), _counterclockwise_bias);
	short_to_long_2d(view->right_edge,view_edge);
	cast_render_ray(&view_edge, NONE, &Nodes.front(), _clockwise_bias);
	
	/* pull polygons off the queue, fire at all their new endpoints, building the tree as we go */
	while (polygon_queue_size)
	{
		short vertex_index;
		short polygon_index= PolygonQueue[--polygon_queue_size];
		polygon_data *polygon= get_polygon_data(polygon_index);
		
		assert(!POLYGON_IS_DETACHED(polygon));
		
		for (vertex_index=0;vertex_index<polygon->vertex_count;++vertex_index)
		{
			short endpoint_index= polygon->endpoint_indexes[vertex_index];
			endpoint_data *endpoint= get_endpoint_data(endpoint_index);
			
			if (!TEST_RENDER_FLAG(endpoint_index, _endpoint_has_been_visited))
			{
				// LP change: move toward correct handling of long distances
				long_vector2d _vector;
				
				/* transform all visited endpoints */
				endpoint->transformed= endpoint->vertex;
				transform_overflow_point2d(&endpoint->transformed, (world_point2d *) &view->origin, view->yaw, &endpoint->flags);

				/* calculate an outbound vector to this endpoint */
				// LP: changed to do long distance correctly.	
				_vector.i= int32(endpoint->vertex.x)-int32(view->origin.x);
				_vector.j= int32(endpoint->vertex.y)-int32(view->origin.y);
				
				// LP change: compose a true transformed point to replace endpoint->transformed,
				// and use it in the upcoming code
				long_vector2d transformed_endpoint;
				overflow_short_to_long_2d(endpoint->transformed,endpoint->flags,transformed_endpoint);
				
				if (transformed_endpoint.i>0)
				{
					int32 x= view->half_screen_width + (transformed_endpoint.j*view->world_to_screen_x)/transformed_endpoint.i;
					
					endpoint_x_coordinates[endpoint_index]= static_cast<int16>(PIN(x, INT16_MIN, INT16_MAX));
					SET_RENDER_FLAG(endpoint_index, _endpoint_has_been_transformed);
				}
				
				/* do two cross products to determine whether this endpoint is in our view cone or not
					(we don’t have to cast at points outside the cone) */
				if ((view->right_edge.i*_vector.j - view->right_edge.j*_vector.i)<=0 && (view->left_edge.i*_vector.j - view->left_edge.j*_vector.i)>=0)
				{
					cast_render_ray(&_vector, ENDPOINT_IS_TRANSPARENT(endpoint) ? NONE : endpoint_index, &Nodes.front(), _no_bias);
				}
				
				SET_RENDER_FLAG(endpoint_index, _endpoint_has_been_visited);
			}
		}
	}
}

/* ---------- building the render tree */

// LP change: make it better able to do long-distance views
// Using parent index instead of pointer to avoid stale-pointer bug
void RenderVisTreeClass::cast_render_ray(
	long_vector2d *_vector, // world_vector2d *vector,
	short endpoint_index,
	node_data* parent,
	short bias) /* _clockwise or _counterclockwise for walking endpoints */
{
	short polygon_index= parent->polygon_index;

//	dprintf("shooting at e#%d of p#%d", endpoint_index, polygon_index);
	
	do
	{
		short clipping_endpoint_index= endpoint_index;
		short clipping_line_index;
		uint16 clip_flags= next_polygon_along_line(&polygon_index, (world_point2d *) &view->origin, _vector, &clipping_endpoint_index, &clipping_line_index, bias);
		
		if (polygon_index==NONE)
		{
			if (clip_flags&_split_render_ray)
			{
				cast_render_ray(_vector, endpoint_index, parent, _clockwise_bias);
				cast_render_ray(_vector, endpoint_index, parent, _counterclockwise_bias);
			}
		}
		else
		{
			node_data **node_reference, *node;
			
			/* find the old node referencing this polygon transition or build one */
			for (node_reference= &parent->children;
					*node_reference && (*node_reference)->polygon_index!=polygon_index;
					node_reference= &(*node_reference)->siblings)
				;
			node= *node_reference;
			if (!node)
			{
				// LP change: using growable list
				// Contents get swapped when the length starts to exceed the capacity.
				// When they are not NULL,
				// "parent", "siblings" and "children" are pointers to members,
				// "reference" is a pointer to a member with an offset.
				// Cast the pointers to whatever size of integer the system uses.
				size_t Length = Nodes.size();
				
				node_data Dummy;
				Dummy.flags = 0;				// Fake initialization to shut up CW
				Nodes.push_back(Dummy);
				node = &Nodes[Length];		// The length here is the "old" length
				
				*node_reference= node;
				INITIALIZE_NODE(node, polygon_index, 0, parent, node_reference);
				
				// Place new node in tree if it has gotten rooted
				if (Length > 0)
				{
					node_data *CurrNode = &Nodes.front();
				while(true)
				{
					int32 PolyDiff = int32(polygon_index) - int32(CurrNode->polygon_index);
					if (PolyDiff > 0)
					{
						node_data *NextNode = CurrNode->PS_Greater;
						if (NextNode)
							// Advance
							CurrNode = NextNode;
						else
						{
							// Attach to end
							CurrNode->PS_Greater = node;
							break;
						}
					}
					else if (PolyDiff < 0)
					{
						node_data *NextNode = CurrNode->PS_Less;
						if (NextNode)
							// Advance
							CurrNode = NextNode;
						else
						{
							// Attach to end
							CurrNode->PS_Less = node;
							break;
						}
					}
					else // Equal
					{
						node_data *NextNode = CurrNode->PS_Shared;
						if (NextNode)
							// Splice node into shared-polygon chain
							node->PS_Shared = NextNode;
						CurrNode->PS_Shared = node;
						break;
					}
				}
				}
			}

			/* update the line clipping information, if necessary, for this node (don’t add
				duplicates */
			if (clipping_line_index!=NONE)
			{
				short i;
				
				if (!TEST_RENDER_FLAG(clipping_line_index, _line_has_clip_data))
					calculate_line_clipping_information(clipping_line_index, clip_flags);
				clipping_line_index= line_clip_indexes[clipping_line_index];
				
				for (i=0;
						i<node->clipping_line_count&&node->clipping_lines[i]!=clipping_line_index;
						++i)
					;
				if (i==node->clipping_line_count)
				{
					assert(node->clipping_line_count<MAXIMUM_CLIPPING_LINES_PER_NODE);
					node->clipping_lines[node->clipping_line_count++]= clipping_line_index;
				}
			}
			
			/* update endpoint clipping information for this node if we have a valid endpoint with clip */
			if (clipping_endpoint_index!=NONE && (clip_flags&(_clip_left|_clip_right)))
			{
				clipping_endpoint_index= calculate_endpoint_clipping_information(clipping_endpoint_index, clip_flags);
				
				// Be sure it's valid
				if (clipping_endpoint_index != NONE)
				{
					if (node->clipping_endpoint_count<MAXIMUM_CLIPPING_ENDPOINTS_PER_NODE)
						node->clipping_endpoints[node->clipping_endpoint_count++]= clipping_endpoint_index;
				}
			}
			
			parent= node;
		}
	}
	while (polygon_index!=NONE);
}

void RenderVisTreeClass::initialize_polygon_queue()
{
	polygon_queue_size= 0;
}


// LP change: make it better able to do long-distance views
uint16 RenderVisTreeClass::next_polygon_along_line(
	short *polygon_index,
	world_point2d *origin, /* not necessairly in polygon_index */
	long_vector2d *_vector, // world_vector2d *vector,
	short *clipping_endpoint_index, /* if non-NONE on entry this is the solid endpoint we’re shooting for */
	short *clipping_line_index, /* NONE on exit if this polygon transition wasn’t accross an elevation line */
	short bias)
{
	polygon_data *polygon= get_polygon_data(*polygon_index);
	short next_polygon_index, crossed_line_index, crossed_side_index;
	bool passed_through_solid_vertex= false;
	short vertex_index, vertex_delta;
	uint16 clip_flags= 0;
	short state;

	if (add_to_automap) ADD_POLYGON_TO_AUTOMAP(*polygon_index);
	if (mark_as_explored && polygon->type == _polygon_must_be_explored)
        polygon->type = _polygon_is_normal;
	PUSH_POLYGON_INDEX(*polygon_index);

	state= _looking_for_first_nonzero_vertex;
	vertex_index= 0, vertex_delta= 1; /* start searching clockwise from vertex zero */
	// LP change: added test for looping around:
	// will remember the first vertex examined when the state has changed
	short initial_vertex_index = vertex_index;
	bool changed_state = true;
	do
	{
		// Jump out of loop?
		if (changed_state)
			changed_state = false;
		else if (vertex_index == initial_vertex_index)
		{
			// Attempt to idiot-proof it by returning nothing
			next_polygon_index = NONE;
			crossed_line_index = NONE;
			crossed_side_index = NONE;
			break;
		}
			
		short endpoint_index= polygon->endpoint_indexes[vertex_index];
		world_point2d *vertex= &get_endpoint_data(endpoint_index)->vertex;
		// LP change to make it more long-distance-friendly
		CROSSPROD_TYPE cross_product= CROSSPROD_TYPE(int32(vertex->x)-int32(origin->x))*_vector->j - CROSSPROD_TYPE(int32(vertex->y)-int32(origin->y))*_vector->i;
		
//		dprintf("p#%d, e#%d:#%d, SGN(cp)=#%d, state=#%d", *polygon_index, vertex_index, polygon->endpoint_indexes[vertex_index], SGN(cross_product), state);
		if (cross_product < 0)
		{
		    switch (state)
		    {
			case _looking_for_first_nonzero_vertex:
			    /* search counterclockwise for transition (right to left) */
			    state= _looking_counterclockwise_for_left_vertex;
			    vertex_delta= -1;
			    // LP change: resetting loop test
			    initial_vertex_index = vertex_index;
			    changed_state = true;
			    break;

			case _looking_clockwise_for_right_vertex: /* found the transition we were looking for */
			{
			    short i= WRAP_LOW(vertex_index, polygon->vertex_count-1);
			    next_polygon_index= polygon->adjacent_polygon_indexes[i];
			    crossed_line_index= polygon->line_indexes[i];
			    crossed_side_index= polygon->side_indexes[i];
			}
			case _looking_for_next_nonzero_vertex: /* next_polygon_index already set */
			    state= NONE;
			    break;
		    }
		} else if (cross_product > 0)
		{
		    switch (state)
		    {
			case _looking_for_first_nonzero_vertex:
			    /* search clockwise for transition (left to right) */
			    state= _looking_clockwise_for_right_vertex;
			    // LP change: resetting loop test
			    initial_vertex_index = vertex_index;
			    changed_state = true;
			    break;

			case _looking_counterclockwise_for_left_vertex: /* found the transition we were looking for */
			    next_polygon_index= polygon->adjacent_polygon_indexes[vertex_index];
			    crossed_line_index= polygon->line_indexes[vertex_index];
			    crossed_side_index= polygon->side_indexes[vertex_index];
			case _looking_for_next_nonzero_vertex: /* next_polygon_index already set */
			    state= NONE;
			    break;
		    }
		} else
		{
		    if (state!=_looking_for_first_nonzero_vertex)
		    {
			if (endpoint_index==*clipping_endpoint_index) passed_through_solid_vertex= true;

			/* if we think we know what’s on the other side of this zero (these zeros)
			change the state: if we don’t find what we’re looking for then the polygon
			is entirely on one side of the line or the other (except for this vertex),
			in any case we need to call decide_where_vertex_leads() to find out what’s
			on the other side of this vertex */
			switch (state)
			{
			    case _looking_clockwise_for_right_vertex:
			    case _looking_counterclockwise_for_left_vertex:
				next_polygon_index= *polygon_index;
				clip_flags|= decide_where_vertex_leads(&next_polygon_index, &crossed_line_index, &crossed_side_index,
					   vertex_index, origin, _vector, clip_flags, bias);
				state= _looking_for_next_nonzero_vertex;
				// LP change: resetting loop test
				initial_vertex_index = vertex_index;
				changed_state = true;
				break;
			}
		    }
		}
		/* adjust vertex_index (clockwise or counterclockwise, depending on vertex_delta) */
		vertex_index= (vertex_delta<0) ? WRAP_LOW(vertex_index, polygon->vertex_count-1) :
			WRAP_HIGH(vertex_index, polygon->vertex_count-1);
	}
	while (state!=NONE);

//	dprintf("exiting, cli=#%d, npi=#%d", crossed_line_index, next_polygon_index);

	/* if we didn’t pass through the solid vertex we were aiming for, set clipping_endpoint_index to NONE,
		we assume the line we passed through doesn’t clip, and set clipping_line_index to NONE
		(this will be corrected in a few moments if we chose poorly) */
	if (!passed_through_solid_vertex) *clipping_endpoint_index= NONE;
	*clipping_line_index= NONE;
	
	if (crossed_line_index!=NONE)
	{
		line_data *line= get_line_data(crossed_line_index);

		/* add the line we crossed to the automap */
		if (add_to_automap) ADD_LINE_TO_AUTOMAP(crossed_line_index);

		/* if the line has a side facing this polygon, mark the side as visible */
		if (crossed_side_index!=NONE) SET_RENDER_FLAG(crossed_side_index, _side_is_visible);

		/* if this line is transparent we need to check for a change in elevation for clipping,
			if it’s not transparent then we can’t pass through it */
		// LP change: added test for there being a polygon on the other side
		if (LINE_IS_TRANSPARENT(line) && next_polygon_index != NONE)
		{
			polygon_data *next_polygon= get_polygon_data(next_polygon_index);
			
			if (line->highest_adjacent_floor>next_polygon->floor_height ||
				line->highest_adjacent_floor>polygon->floor_height) clip_flags|= _clip_down; /* next polygon floor is lower */
			if (line->lowest_adjacent_ceiling<next_polygon->ceiling_height ||
				line->lowest_adjacent_ceiling<polygon->ceiling_height) clip_flags|= _clip_up; /* next polygon ceiling is higher */
			if (clip_flags&(_clip_up|_clip_down)) *clipping_line_index= crossed_line_index;
		}
		else
		{
			next_polygon_index= NONE;
		}
	}

	/* tell the caller what polygon we ended up in */
	*polygon_index= next_polygon_index;
	
	return clip_flags;
}

// LP change: make it better able to do long-distance views
uint16 RenderVisTreeClass::decide_where_vertex_leads(
	short *polygon_index,
	short *line_index,
	short *side_index,
	short endpoint_index_in_polygon_list,
	world_point2d *origin,
	long_vector2d *_vector, // world_vector2d *vector,
	uint16 clip_flags,
	short bias)
{
	polygon_data *polygon= get_polygon_data(*polygon_index);
	short endpoint_index= polygon->endpoint_indexes[endpoint_index_in_polygon_list];
	short index;
	
	switch (bias)
	{
		case _no_bias:
//			dprintf("splitting at endpoint #%d", endpoint_index);
			clip_flags|= _split_render_ray;
			*polygon_index= *line_index= *side_index= NONE;
			index= NONE;
			break;
		
		case _clockwise_bias:
			index= endpoint_index_in_polygon_list;
			break;
		
		case _counterclockwise_bias:
			index= WRAP_LOW(endpoint_index_in_polygon_list, polygon->vertex_count-1);
			break;
		
		default:
			// LP change:
			assert(false);
			// halt();
	}
	
	if (index!=NONE)
	{
		line_data *line;
		world_point2d *vertex;
		CROSSPROD_TYPE cross_product;

		*line_index= polygon->line_indexes[index];
		*side_index= polygon->side_indexes[index];
		*polygon_index= polygon->adjacent_polygon_indexes[index];
		
		line= get_line_data(*line_index);
		if (*polygon_index!=NONE && LINE_IS_TRANSPARENT(line))
		{
			polygon= get_polygon_data(*polygon_index);
			
			/* locate our endpoint in this polygon */
			for (index=0;
					index<polygon->vertex_count && polygon->endpoint_indexes[index]!=endpoint_index;
					++index)
				;
			vassert(index!=polygon->vertex_count, csprintf(temporary, "endpoint #%d not in polygon #%d", endpoint_index, *polygon_index));
	
			switch (bias)
			{
				case _clockwise_bias: index= WRAP_HIGH(index, polygon->vertex_count-1); break;
				case _counterclockwise_bias: index= WRAP_LOW(index, polygon->vertex_count-1); break;
				default:
					assert(false);
					break;
			}
			
			vertex= &get_endpoint_data(polygon->endpoint_indexes[index])->vertex;
			// LP change: made more long-distance-friendly
			cross_product= CROSSPROD_TYPE(int32(vertex->x)-int32(origin->x))*_vector->j - CROSSPROD_TYPE(int32(vertex->y)-int32(origin->y))*_vector->i;
			
			if ((bias==_clockwise_bias&&cross_product>=0) || (bias==_counterclockwise_bias&&cross_product<=0))
			{
				/* we’re leaving this endpoint, set clip flag in case it’s solid */
				clip_flags|= (bias==_clockwise_bias) ? _clip_left : _clip_right;
			}
		}

//		dprintf("left endpoint #%d via line #%d to polygon #%d (bias==#%d)", endpoint_index, *line_index, *polygon_index, bias);
	}

	return clip_flags;
}

void RenderVisTreeClass::initialize_render_tree()
{
	// LP change: using growable list
	Nodes.clear();
	node_data Dummy;
	Dummy.flags = 0;				// Fake initialization to shut up CW
	Nodes.push_back(Dummy);
	INITIALIZE_NODE(&Nodes[0], view->origin_polygon_index, 0, NULL, NULL);
}

/* ---------- initializing and calculating clip data */

void RenderVisTreeClass::initialize_clip_data()
{
	ResetEndpointClips();
	
	/* set two default endpoint clips (left and right sides of screen) */
	{
		endpoint_clip_data *endpoint;
		
		endpoint= &EndpointClips[indexLEFT_SIDE_OF_SCREEN];
		endpoint->flags= _clip_left;
		short_to_long_2d(view->untransformed_left_edge,endpoint->vector);
		endpoint->x= 0;

		endpoint= &EndpointClips[indexRIGHT_SIDE_OF_SCREEN];
		endpoint->flags= _clip_right;
		short_to_long_2d(view->untransformed_right_edge,endpoint->vector);
		endpoint->x= view->screen_width;
	}
	
	ResetLineClips();

	/* set default line clip (top and bottom of screen) */
	{
		line_clip_data *line= &LineClips[indexTOP_AND_BOTTOM_OF_SCREEN];

		line->flags= _clip_up|_clip_down;
		line->x0= 0;
		line->x1= view->screen_width;
		line->top_y= 0; short_to_long_2d(view->top_edge,line->top_vector);
		line->bottom_y= view->screen_height; short_to_long_2d(view->bottom_edge,line->bottom_vector);
	}

	// LP change:
	ClippingWindows.clear();
}

void RenderVisTreeClass::calculate_line_clipping_information(
	short line_index,
	uint16 clip_flags)
{
	// LP addition: extend the line-clip list
	line_clip_data Dummy;
	Dummy.flags = 0;			// Fake initialization to shut up CW
	LineClips.push_back(Dummy);
	size_t Length = LineClips.size();
	assert(Length <= 32767);
	assert(Length >= 1);
	size_t LastIndex = Length-1;
	
	line_data *line= get_line_data(line_index);
	// LP change: relabeling p0 and p1 so as not to conflict with later use
	world_point2d p0_orig= get_endpoint_data(line->endpoint_indexes[0])->vertex;
	world_point2d p1_orig= get_endpoint_data(line->endpoint_indexes[1])->vertex;
	// LP addition: place for new line data
	line_clip_data *data= &LineClips[LastIndex];

	/* it’s possible (in fact, likely) that this line’s endpoints have not been transformed yet,
		so we have to do it ourselves */
	// LP change: making the operation long-distance friendly
	uint16 p0_flags = 0, p1_flags = 0;
	transform_overflow_point2d(&p0_orig, (world_point2d *) &view->origin, view->yaw, &p0_flags);
	transform_overflow_point2d(&p1_orig, (world_point2d *) &view->origin, view->yaw, &p1_flags);
	
	// Defining long versions here and copying over
	long_point2d p0, p1;
	long_vector2d *pv0ptr = (long_vector2d*)(&p0), *pv1ptr = (long_vector2d*)(&p1);
	overflow_short_to_long_2d(p0_orig,p0_flags,*pv0ptr);
	overflow_short_to_long_2d(p1_orig,p1_flags,*pv1ptr);
	
	clip_flags&= _clip_up|_clip_down;	
	assert(clip_flags&(_clip_up|_clip_down));
	assert(!TEST_RENDER_FLAG(line_index, _line_has_clip_data));

	SET_RENDER_FLAG(line_index, _line_has_clip_data);
	line_clip_indexes[line_index]= static_cast<vector<size_t>::value_type>(LastIndex);
	
	data->flags= 0;

	if (p0.x>0 && p1.x>0)
	{
		// LP change:
		long_point2d *p;
		world_distance z;
		int32 transformed_z;
		int32 y, y0, y1;
		int32 x0= view->half_screen_width + (p0.y*view->world_to_screen_x)/p0.x;
		int32 x1= view->half_screen_width + (p1.y*view->world_to_screen_x)/p1.x;
	
		data->x0= (short)PIN(x0, 0, view->screen_width);
		data->x1= (short)PIN(x1, 0, view->screen_width);
		if (data->x1<data->x0) SWAP(data->x0, data->x1);
		if (data->x1>data->x0)
		{
			if (clip_flags&_clip_up)
			{
				/* precalculate z and transformed_z */
				z= line->lowest_adjacent_ceiling-view->origin.z;
				transformed_z= z*view->world_to_screen_y;
				
				/* calculate and clip y0 and y1 (screen y-coordinates of each side of the line) */
				y0= (p0.x>0) ? (view->half_screen_height - transformed_z/p0.x + view->dtanpitch) : 0;
				y1= (p1.x>0) ? (view->half_screen_height - transformed_z/p1.x + view->dtanpitch) : 0;
		
				/* pick the highest (closest to zero) and pin it to the screen */
				if (y0<y1) y= y0, p= &p0; else y= y1, p= &p1;
				y= PIN(y, 0, view->screen_height);
				
				/* if we’re not useless (clipping up off the top of the screen) set up top-clip information) */
				if (y<=0)
				{
					clip_flags&= ~_clip_up;
				}
				else
				{
					data->top_vector.i= - p->x, data->top_vector.j= - z;
					data->top_y= y;
				}
			}
			
			if (clip_flags&_clip_down)
			{
				z= line->highest_adjacent_floor - view->origin.z;
				transformed_z= z*view->world_to_screen_y;
				
				/* calculate and clip y0 and y1 (screen y-coordinates of each side of the line) */
				y0= (p0.x>0) ? (view->half_screen_height - transformed_z/p0.x + view->dtanpitch) : view->screen_height;
				y1= (p1.x>0) ? (view->half_screen_height - transformed_z/p1.x + view->dtanpitch) : view->screen_height;
				
				/* pick the highest (closest to zero screen_height) and pin it to the screen */
				if (y0>y1) y= y0, p= &p0; else y= y1, p= &p1;
				y= PIN(y, 0, view->screen_height);
				
				/* if we’re not useless (clipping up off the bottom of the screen) set up top-clip information) */
				if (y>=view->screen_height)
				{
					clip_flags&= ~_clip_down;
				}
				else
				{
					data->bottom_vector.i= p->x,  data->bottom_vector.j= z;
					data->bottom_y= y;
				}
			}
	
			data->flags= clip_flags;
//			dprintf("line #%d clips %x @ %p", line_index, clip_flags, data);
		}
	}
}

/* we can actually rely on the given endpoint being transformed because we only set clipping
	information for endpoints we’re aiming at, and we transform endpoints before firing at them */
// Returns NONE of it does not have valid clip info
short RenderVisTreeClass::calculate_endpoint_clipping_information(
	short endpoint_index,
	uint16 clip_flags)
{
	// If this endpoint was not transformed, then don't do anything with it,
	// and indicate that it's not a valid endpoint
	if (!TEST_RENDER_FLAG(endpoint_index, _endpoint_has_been_transformed))
		return NONE;
	
	// LP addition: extend the endpoint-clip list
	endpoint_clip_data Dummy;
	Dummy.flags = 0;			// Fake initialization to shut up CW
	EndpointClips.push_back(Dummy);
	size_t Length = EndpointClips.size();
	assert(Length <= 32767);
	assert(Length >= 1);
	size_t LastIndex = Length-1;

	endpoint_data *endpoint= get_endpoint_data(endpoint_index);
	endpoint_clip_data *data= &EndpointClips[LastIndex];
	int32 x;

	assert((clip_flags&(_clip_left|_clip_right))); /* must have a clip flag */
	assert((clip_flags&(_clip_left|_clip_right))!=(_clip_left|_clip_right)); /* but can’t have both */
	assert(!TEST_RENDER_FLAG(endpoint_index, _endpoint_has_clip_data));
	
	// LP change: compose a true transformed point to replace endpoint->transformed,
	// and use it in the upcoming code
	long_vector2d transformed_endpoint;
	overflow_short_to_long_2d(endpoint->transformed,endpoint->flags,transformed_endpoint);
	
	data->flags= clip_flags&(_clip_left|_clip_right);
	switch (data->flags)
	{
		case _clip_left:
			data->vector.i= transformed_endpoint.i;
			data->vector.j= transformed_endpoint.j;
			break;
		case _clip_right: /* negatives so we clip to the correct side */
			data->vector.i= -transformed_endpoint.i;
			data->vector.j= -transformed_endpoint.j;
			break;
	}
	// warn(data->vector.i);
	
	// assert(TEST_RENDER_FLAG(endpoint_index, _endpoint_has_been_transformed));
	x= endpoint_x_coordinates[endpoint_index];

	data->x= (short)PIN(x, 0, view->screen_width);
	
	return (short)LastIndex;
}

// LP addition: resetters for some of the lists:
void RenderVisTreeClass::ResetEndpointClips(void)
{
	EndpointClips.clear();
	endpoint_clip_data Dummy;
	Dummy.flags = 0;			// Fake initialization to shut up CW
	for (int k=0; k<NUMBER_OF_INITIAL_ENDPOINT_CLIPS; k++)
		EndpointClips.push_back(Dummy);
}

void RenderVisTreeClass::ResetLineClips(void)
{
	LineClips.clear();
	line_clip_data Dummy;
	Dummy.flags = 0;			// Fake initialization to shut up CW
	for (int k=0; k<NUMBER_OF_INITIAL_LINE_CLIPS; k++)
		LineClips.push_back(Dummy);
}
