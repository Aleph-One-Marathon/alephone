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
	
	Rendering Polygon-Sorting Class
	by Loren Petrich,
	August 6, 2000
	
	Contains the sorting of polygons into depth order; from render.c
	
	Made [view_data *view] a member and removed it as an argument

Sept. 15, 2000 (Loren Petrich)
	Changed a dprintf/assert to a vassert in build_clipping_windows()
	
Oct 13, 2000
	LP: replaced GrowableLists and ResizableLists with STL vectors
*/

#include "cseries.h"

#include "map.h"
#include "RenderSortPoly.h"

#include <string.h>
#include <limits.h>


// LP: "recommended" sizes of stuff in growable lists
#define MAXIMUM_SORTED_NODES 128
#define MAXIMUM_CLIPS_PER_NODE 64

enum /* build_clipping_window() window states */
{
	_looking_for_left_clip, /* ignore right clips (we just passed one) */
	_looking_for_right_clip,
	_building_clip_window /* found valid left and right clip, build a window */
};


RenderSortPolyClass::RenderSortPolyClass():
	view(NULL),	// Idiot-proofing
	RVPtr(NULL)
{
	SortedNodes.reserve(MAXIMUM_SORTED_NODES);
	AccumulatedEndpointClips.reserve(MAXIMUM_CLIPS_PER_NODE);
	AccumulatedLineClips.reserve(MAXIMUM_CLIPS_PER_NODE);
}


// Resizes all the objects defined inside
void RenderSortPolyClass::Resize(size_t NumPolygons)
{
	polygon_index_to_sorted_node.resize(NumPolygons);
}


/* ---------- sorting (decomposing) the render tree */

void RenderSortPolyClass::initialize_sorted_render_tree()
{
	// LP change: sorted nodes a growable list
	SortedNodes.clear();
}

/*
tree decomposition:

pick a leaf polygon
	make sure the polygon is everywhere a leaf (don’t walk the tree, search it linearly) 
		if it’s not, pick the node which obstructed it to test next
		if it is, pull it off the tree (destructively) and accumulate it’s clipping information
			pick the one of the node’s siblings (or it’s parent if it has none) to handle next
*/

void RenderSortPolyClass::sort_render_tree()
{
	assert(view);	// Idiot-proofing
	assert(RVPtr);
	node_data *leaf, *last_leaf;
	// LP: reference to simplify the code
	RenderVisTreeClass::NodeList& Nodes = RVPtr->Nodes;

	initialize_sorted_render_tree();
	
	leaf= NULL;
	do
	{
		// LP change: no more growable list of aliases,
		// due to the sorted-polygon-tree structure of the nodes.
		bool leaf_has_children= false; /* i.e., it’s not a leaf */
		node_data *node = NULL;

		/* if we don’t have a leaf, find one */
		if (!leaf)
			for (leaf= &Nodes.front(); leaf->children; leaf= leaf->children)
				;
		last_leaf= leaf;
		
		/* does the current leaf have any children anywhere in the tree? */
		// LP change: Replaced all this code with binary-search code for polygon value,
		// followed by building of node-alias list
		short PolygonToFind = leaf->polygon_index;
		
		// Look for the first node with that polygon index;
		// start search off with the hypothesis of failure
		node_data *FoundNode = NULL;
		node_data *CurrNode = &Nodes.front();
		while(true)
		{
			int32 PolyDiff = int32(PolygonToFind) - int32(CurrNode->polygon_index);
			if (PolyDiff > 0)
			{
				node_data *NextNode = CurrNode->PS_Greater;
				if (NextNode)
					// Advance
					CurrNode = NextNode;
				else
					// Failed
					break;
			}
			else if (PolyDiff < 0)
			{
				node_data *NextNode = CurrNode->PS_Less;
				if (NextNode)
					// Advance
					CurrNode = NextNode;
				else
					// Failed
					break;
			}
			else // Equal: the search was a success
			{
				FoundNode = CurrNode;
				break;
			}
		}
		
		// Now load up the node aliases and check for children
		if (FoundNode)
		{
			// Search along node chain
			for (node = FoundNode; node; node = node->PS_Shared)
			{
				assert(node->polygon_index == PolygonToFind);
				if (node->children)
				{
					leaf_has_children= true;
					break;
				}
			}
		}
		
		if (leaf_has_children) /* something was in our way; see if we can take it out instead */
		{
			leaf= node->children;
//			dprintf("polygon #%d is in the way of polygon #%d", node->polygon_index, leaf->polygon_index);
		}
		else /* this is a leaf, and we can remove it from the tree */
		{
			sorted_node_data *sorted_node;
			
//			dprintf("removed polygon #%d (#%d aliases)", leaf->polygon_index, alias_count);
			
			size_t Length = SortedNodes.size();
			POINTER_DATA OldSNPointer = POINTER_CAST(SortedNodes.data());
				
			// Add a dummy object and check if the pointer got changed
			sorted_node_data Dummy;
			Dummy.polygon_index = NONE;			// Fake initialization to shut up CW
			SortedNodes.push_back(Dummy);
			POINTER_DATA NewSNPointer = POINTER_CAST(SortedNodes.data());
				
			if (NewSNPointer != OldSNPointer)
			{
				// Update what uses the sorted-node pointers
				for (size_t k=0; k<Length; k++) {
					sorted_node = &SortedNodes[k];
					polygon_index_to_sorted_node[sorted_node->polygon_index]= sorted_node;
				}
			}
			sorted_node = &SortedNodes[Length];
			
			sorted_node->polygon_index= leaf->polygon_index;
			sorted_node->interior_objects= NULL;
			sorted_node->exterior_objects= NULL;
			
			// Put SortedNodes in a valid state before we call build_clipping_windows()
			// (build_clipping_windows() updates SortedNodes if the clipping windows are reallocated)
			sorted_node->clipping_windows = nullptr;
			
			// LP change: using polygon-sorted node chain
			sorted_node->clipping_windows= build_clipping_windows(FoundNode);
			
			/* remember which sorted nodes correspond to which polygons (only valid if
				_polygon_is_visible) */
			polygon_index_to_sorted_node[sorted_node->polygon_index]= sorted_node;
			
			/* walk this node’s alias list, removing each from the tree */
			// LP change: move down the chain of polygon-sharing nodes
			for (node_data *Alias = FoundNode; Alias; Alias = Alias->PS_Shared)
			{
				// LP change: remember what the node was for when we break out
				node = Alias;

				/* remove this node and update the next node’s reference (if there is a
					reference and if there is a next node) */
				if (node->reference)
				{
					*(node->reference)= node->siblings;
					if (node->siblings) (node->siblings)->reference= node->reference;
				}
			}

			/* try to handle this node’s siblings next (if there aren’t any, then a ‘random’
				node will be chosen) */
			leaf= node ? node->siblings : NULL;
		}
	}

	while (last_leaf != &Nodes.front()); /* continue until we remove the root */
}

/* ---------- initializing and calculating clip data */

/* be sure to examine all of a node’s parents for clipping information (gak!) */
clipping_window_data *RenderSortPolyClass::build_clipping_windows(
	node_data *ChainBegin)
{
	// LP change: growable lists
	AccumulatedLineClips.clear();
	AccumulatedEndpointClips.clear();
	endpoint_clip_data *endpoint;
	line_clip_data *line;
	short x0, x1; /* ignoring what clipping parameters we’ve gotten, this is the left and right borders of this node on the screen */
	short i, j;

	// LP: references to simplify the code
	vector<endpoint_clip_data>& EndpointClips = RVPtr->EndpointClips;
	vector<line_clip_data>& LineClips = RVPtr->LineClips;
	vector<clipping_window_data>& ClippingWindows = RVPtr->ClippingWindows;
	vector<short>& endpoint_x_coordinates = RVPtr->endpoint_x_coordinates;
	
	/* calculate x0,x1 (real left and right borders of this node) in case the left and right borders
		of the window are sloppy */
	{
		// LP change: look at beginning of chain
		polygon_data *polygon= get_polygon_data(ChainBegin->polygon_index); /* all these nodes should be the same */
		
		x0= SHRT_MAX, x1= SHRT_MIN;
		for (i= 0;i<polygon->vertex_count;++i)
		{
			short endpoint_index= polygon->endpoint_indexes[i];
			
			if (TEST_RENDER_FLAG(endpoint_index, _endpoint_has_been_transformed))
			{
				short x= endpoint_x_coordinates[endpoint_index];
				
				if (x<x0) x0= x;
				if (x>x1) x1= x;
			}
			else
			{
				x0= SHRT_MIN, x1= SHRT_MAX;
				break;
			}
		}
	}
	
	/* add left, top and bottom of screen */
	endpoint_clip_data *EndpointClipPtr = &EndpointClips[indexLEFT_SIDE_OF_SCREEN];
	AccumulatedEndpointClips.push_back(EndpointClipPtr);
	line_clip_data *LineClipPtr = &LineClips[indexTOP_AND_BOTTOM_OF_SCREEN];
	AccumulatedLineClips.push_back(LineClipPtr);

	/* accumulate clipping information, left to right, into local arrays */
	// Move along chain
	for (node_data *ChainNode = ChainBegin; ChainNode; ChainNode = ChainNode->PS_Shared)
	{
		node_data *node;
		
		// LP change: use chain node as starting point
		for (node= ChainNode;node;node= node->parent) /* examine this node and all parents! */
		{
			/* sort in endpoint clips (left to right) */
			for (i= 0;i<node->clipping_endpoint_count;++i)
			{
				endpoint= &EndpointClips[node->clipping_endpoints[i]];
				
				short size = AccumulatedEndpointClips.size();
				for (j= 0;j<size;++j)
				{
					if (AccumulatedEndpointClips[j]==endpoint) { j= NONE; break; } /* found duplicate */
					if ((AccumulatedEndpointClips[j]->x==endpoint->x&&endpoint->flags==_clip_left) ||
						AccumulatedEndpointClips[j]->x>endpoint->x)
					{
						break; /* found sorting position if x is greater or x is equal and this is a left clip */
					}
				}
				
				if (j!=NONE) /* if the endpoint was not a duplicate */
				{
					/* expand the array, if necessary, and add the new endpoint */
					int Length = AccumulatedEndpointClips.size();
					AccumulatedEndpointClips.push_back(NULL);
//					assert(AccumulatedEndpointClips.size() <= 32767);		// Originally a short value
					if (j!=Length) memmove(&AccumulatedEndpointClips[j+1], &AccumulatedEndpointClips[j],
						(Length-j)*sizeof(endpoint_clip_data *));
					AccumulatedEndpointClips[j]= endpoint;
				}
			}

			/* sort in line clips, avoiding redundancies;  calculate_vertical_line_clip_data(),
				the function which deals with these, does not depend on them being sorted */
			for (i= 0;i<node->clipping_line_count;++i)
			{
				line= &LineClips[node->clipping_lines[i]];
				
				short size = AccumulatedLineClips.size();
				for (j= 0;j<size;++j) if (AccumulatedLineClips[j]==line) break; /* found duplicate */
				if (j==short(AccumulatedLineClips.size())) /* if the line was not a duplicate */
				{
					AccumulatedLineClips.push_back(line);
//					assert(AccumulatedLineClips.size() <= 32767);		// Originally a short value
				}
			}
		}
	}
	
//	dprintf("#%d accumulated points @ %p", accumulated_endpoint_clip_count, accumulated_endpoint_clips);
//	dprintf("#%d accumulated lines @ %p", accumulated_line_clip_count, accumulated_line_clips);

	/* add right side of screen */
	EndpointClipPtr = &EndpointClips[indexRIGHT_SIDE_OF_SCREEN];
	AccumulatedEndpointClips.push_back(EndpointClipPtr);

	const auto initial_cw_count = ClippingWindows.size();
	
	/* build the clipping windows */
	{
		short state= _looking_for_left_clip;
		endpoint_clip_data *left_clip = NULL, *right_clip = NULL;

		for (i= 0;i<short(AccumulatedEndpointClips.size());++i)
		{
			endpoint= AccumulatedEndpointClips[i];
	
			switch (endpoint->flags)
			{
				case _clip_left:
					switch (state)
					{
						case _looking_for_left_clip:
							left_clip= endpoint;
							state= _looking_for_right_clip;
							break;
						case _looking_for_right_clip:
							left_clip= endpoint; /* found more strict clipping point, use it instead */
							break;
					}
					break;
				
				case _clip_right:
					switch (state)
					{
						case _looking_for_right_clip:
							right_clip= endpoint;
							state= _building_clip_window;
							break;
						
						/* ignore _left_clips */
					}
					break;
				
				default:
					vassert(false,csprintf(temporary,"RenderSortPoly.cpp: build_clipping_windows(): bad state: %d",state));
					break;
			}

			if (state==_building_clip_window)
			{
				if (left_clip->x<view->screen_width && right_clip->x>0 && left_clip->x<right_clip->x)
				{
					// LP change: clipping windows are in growable list
					size_t Length = ClippingWindows.size();
					POINTER_DATA OldCWPointer = POINTER_CAST(ClippingWindows.data());
					
					// Add a dummy object and check if the pointer got changed
					clipping_window_data Dummy;
					Dummy.next_window = NULL;			// Fake initialization to shut up CW
					ClippingWindows.push_back(Dummy);
					POINTER_DATA NewCWPointer = POINTER_CAST(ClippingWindows.data());
				
					if (NewCWPointer != OldCWPointer)
					{
						// Get the clipping windows and sorted nodes into sync; no render objects yet
						for (size_t k=0; k<Length; k++)
						{
							clipping_window_data &ClippingWindow = ClippingWindows[k];
							if (ClippingWindow.next_window != NULL)
								ClippingWindow.next_window = (clipping_window_data *)(NewCWPointer + (POINTER_CAST(ClippingWindow.next_window) - OldCWPointer));
						}
						for (size_t k=0; k<SortedNodes.size(); k++)
						{
							sorted_node_data &SortedNode = SortedNodes[k];
							if (SortedNode.clipping_windows != NULL)
								SortedNode.clipping_windows = (clipping_window_data *)(NewCWPointer + (POINTER_CAST(SortedNode.clipping_windows) - OldCWPointer));
						}
					}
					clipping_window_data *window= &ClippingWindows[Length];
					
					/* handle maintaining the linked list of clipping windows */
					if (Length > initial_cw_count)
						ClippingWindows[Length-1].next_window = window;
					
					window->x0= left_clip->x, window->x1= right_clip->x;
					window->left= left_clip->vector;
					window->right= right_clip->vector;
					calculate_vertical_clip_data(&AccumulatedLineClips.front(), AccumulatedLineClips.size(), window,
						MAX(x0, window->x0), MIN(x1, window->x1));
					window->next_window= NULL;
				}
				
				state= _looking_for_left_clip;
			}
		}
	}

	// Return the front of the linked list we made (if any)
	return ClippingWindows.size() > initial_cw_count ? &ClippingWindows[initial_cw_count] : nullptr;
}

/* does not care if the given line_clips are sorted or not */
void RenderSortPolyClass::calculate_vertical_clip_data(
	line_clip_data **accumulated_line_clips,
	size_t accumulated_line_clip_count,
	clipping_window_data *window,
	short x0,
	short x1)
{
	if (x0<x1)
	{
		short x;
		line_clip_data *highest_line, *locally_highest_line, *line;
	
		/* get the highest top clip covering the requested horizontal run */		
		x= x0;
		highest_line= NULL;
		do
		{
			locally_highest_line= NULL;
			
			for (size_t i= 0;i<accumulated_line_clip_count;++i)
			{
				line= accumulated_line_clips[i];
				
				if ((line->flags&_clip_up) && x>=line->x0 && x<line->x1 &&
					(!locally_highest_line || locally_highest_line->top_y<line->top_y))
				{
					locally_highest_line= line;
				}
			}
			vassert(locally_highest_line, csprintf(temporary, "didn't find diddly at #%d [#%d,#%d]", x, x0, x1));
				
			if (!highest_line || locally_highest_line->top_y<highest_line->top_y)
			{
				highest_line= locally_highest_line;
//				dprintf("%p [%d,%d] is new highest top clip line for window [%d,%d]", highest_line, highest_line->x0, highest_line->x1, x0, x1);
			}
			
			x= locally_highest_line->x1;
		}
		while (x<x1);
		
		assert(highest_line);
//		dprintf("%p [%d,%d] is highest top clip line for window [%d,%d]", highest_line, highest_line->x0, highest_line->x1, x0, x1);
		window->top= highest_line->top_vector;
		window->y0= highest_line->top_y;
	
		/* get the lowest bottom clip covering the requested horizontal run */	
		x= x0;
		highest_line= NULL;
		do
		{
			locally_highest_line= NULL; /* means lowest */
			
			for (size_t i= 0;i<accumulated_line_clip_count;++i)
			{
				line= accumulated_line_clips[i];
				
				if ((line->flags&_clip_down) && x>=line->x0 && x<line->x1 &&
					(!locally_highest_line || locally_highest_line->bottom_y>line->bottom_y))
				{
					locally_highest_line= line;
				}
			}
			vassert(locally_highest_line, csprintf(temporary, "didn't find diddly at #%d [#%d,#%d]", x, x0, x1));
				
			if (!highest_line || locally_highest_line->bottom_y>highest_line->bottom_y)
			{
				highest_line= locally_highest_line; 
//				dprintf("%p [%d,%d] is new lowest bottom clip line for window [%d,%d]", highest_line, highest_line->x0, highest_line->x1, x0, x1);
			}
			
			x= locally_highest_line->x1;
		}
		while (x<x1);
		
		assert(highest_line);
//		dprintf("%p [%d,%d] is lowest bottom clip line for window [%d,%d]", highest_line, highest_line->x0, highest_line->x1, x0, x1);
		window->bottom= highest_line->bottom_vector;
		window->y1= highest_line->bottom_y;
	}
}
