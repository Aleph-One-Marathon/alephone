/*
	
	Rendering Polygon-Sorting Class
	by Loren Petrich,
	August 6, 2000
	
	Contains the sorting of polygons into depth order; from render.c
	
	Made [view_data *view] a member and removed it as an argument
*/

#include "cseries.h"

#include "map.h"
#include "RenderSortPoly.h"

#include <string.h>


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
	SortedNodes(MAXIMUM_SORTED_NODES),
	AccumulatedEndpointClips(MAXIMUM_CLIPS_PER_NODE),
	AccumulatedLineClips(MAXIMUM_CLIPS_PER_NODE),
	view(NULL),	// Idiot-proofing
	RVPtr(NULL)
{}


// Resizes all the objects defined inside
void RenderSortPolyClass::Resize(int NumPolygons)
{
	assert(polygon_index_to_sorted_node.SetLength(NumPolygons));
}


/* ---------- sorting (decomposing) the render tree */

void RenderSortPolyClass::initialize_sorted_render_tree()
{
	// LP change: sorted nodes a growable list
	SortedNodes.ResetLength();
	/*
	sorted_node_count= 0;
	next_sorted_node= sorted_nodes;
	*/
	
	return;
}

/*
tree decomposition:

pick a leaf polygon
	make sure the polygon is everywhere a leaf (don’t walk the tree, search it linearly) 
		if it’s not, pick the node which obstructed it to test next
		if it is, pull it off the tree (destructively) and accumulate it’s clipping information
			pick the one of the node’s siblings (or it’s parent if it has none) to handle next
*/

// LP change: this is the source of transparent-line errors;
// this has been increased to 32 (the value in the Win32 version)
// Feb 1, 2000: Suppressed because of use of growable-list
// MacOS-version value:
// #define MAXIMUM_NODE_ALIASES 20
// Win32-version value:
// #define MAXIMUM_NODE_ALIASES 32

void RenderSortPolyClass::sort_render_tree()
{
	assert(view);	// Idiot-proofing
	assert(RVPtr);
	node_data *leaf, *last_leaf;
	// LP: reference to simplify the code
	GrowableList<node_data>& Nodes = RVPtr->Nodes;

	initialize_sorted_render_tree();
	
	leaf= NULL;
	do
	{
		// LP change: no more growable list of aliases,
		// due to the sorted-polygon-tree structure of the nodes.
		/*
		short alias_count= 0;
		struct node_data *aliases[MAXIMUM_NODE_ALIASES];
		*/
		boolean leaf_has_children= FALSE; /* i.e., it’s not a leaf */
		node_data *node;

		/* if we don’t have a leaf, find one */
		if (!leaf)
			// LP change:
			for (leaf= Nodes.Begin(); leaf->children; leaf= leaf->children)
			// for (leaf= nodes; leaf->children; leaf= leaf->children)
				;
		last_leaf= leaf;
		
		/* does the current leaf have any children anywhere in the tree? */
		// LP change: Replaced all this code with binary-search code for polygon value,
		// followed by building of node-alias list
		short PolygonToFind = leaf->polygon_index;
		
		// Look for the first node with that polygon index;
		// start search off with the hypothesis of failure
		node_data *FoundNode = NULL;
		node_data *CurrNode = Nodes.Begin();
		while(true)
		{
			long PolyDiff = long(PolygonToFind) - long(CurrNode->polygon_index);
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
					leaf_has_children= TRUE;
					break;
				}
			}
		}
		
		/*
		// LP change: don't need this linear-search code anymore,
		// thanx to Rhys Hill's insights on how to construct self-sorting objects.
		for (node= Nodes.Begin(); node<Nodes.End(); ++node)
		// for (node= nodes; node<next_node; ++node)
		{
			if (node->polygon_index==leaf->polygon_index)
			{
				// LP change: using node-alias growable list				
				assert(NodeAliases.Add(node));
				
				if (node->children)
				{
					leaf_has_children= TRUE;
					break;
				}
				if (alias_count<MAXIMUM_NODE_ALIASES)
				{
					aliases[alias_count++]= node;
					
					if (node->children)
					{
						leaf_has_children= TRUE;
						break;
					}
				}
#ifdef DEBUG
				else
				{
					dprintf("exceeded MAXIMUM_NODE_ALIASES; this sucks, Beavis.");
					return;
				}
// LP: Added alternative way of handling this
#else
				else
				{
					assert(alias_count<MAXIMUM_NODE_ALIASES);
					return;
				}
#endif
			}
		}
		*/
		
		if (leaf_has_children) /* something was in our way; see if we can take it out instead */
		{
			leaf= node->children;
//			dprintf("polygon #%d is in the way of polygon #%d", node->polygon_index, leaf->polygon_index);
		}
		else /* this is a leaf, and we can remove it from the tree */
		{
			sorted_node_data *sorted_node;
			short alias;
			
//			dprintf("removed polygon #%d (#%d aliases)", leaf->polygon_index, alias_count);
			
			// LP change:
			int Length = SortedNodes.GetLength();
			// Will memory get swapped?
			bool DoSwap = Length >= SortedNodes.GetCapacity();
			assert(SortedNodes.Add());
			// Update dependent quantities; Length is original length
			if (DoSwap) {
				for (int k=0; k<Length; k++) {
					sorted_node = &SortedNodes[k];
					polygon_index_to_sorted_node[sorted_node->polygon_index]= sorted_node;
				}
			}
			sorted_node = &SortedNodes[Length];
			/*
			assert(sorted_node_count++<MAXIMUM_SORTED_NODES);
			sorted_node= next_sorted_node++;
			*/
			
			sorted_node->polygon_index= leaf->polygon_index;
			sorted_node->interior_objects= NULL;
			sorted_node->exterior_objects= NULL;
			// LP change: using polygon-sorted node chain
			sorted_node->clipping_windows= build_clipping_windows(FoundNode);
			/*
			sorted_node->clipping_windows= build_clipping_windows(view, aliases, alias_count);
			*/
			
			/* remember which sorted nodes correspond to which polygons (only valid if
				_polygon_is_visible) */
			polygon_index_to_sorted_node[sorted_node->polygon_index]= sorted_node;
			
			/* walk this node’s alias list, removing each from the tree */
			// LP change: move down the chain of polygon-sharing nodes
			for (node_data *Alias = FoundNode; Alias; Alias = Alias->PS_Shared)
			/*
			for (alias= 0;alias<alias_count;++alias)
			*/
			{
				// LP change: remember what the node was for when we break out
				node = Alias;
				// node= aliases[alias];

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
			leaf= node->siblings;
		}
	}
	// LP change:
	while (last_leaf!=Nodes.Begin()); /* continue until we remove the root */
	// while (last_leaf!=nodes); /* continue until we remove the root */
	
	return;
}

/* ---------- initializing and calculating clip data */

/* be sure to examine all of a node’s parents for clipping information (gak!) */
clipping_window_data *RenderSortPolyClass::build_clipping_windows(
	// LP change: using node chain instead
	node_data *ChainBegin)
	// struct node_data **node_list,
	// short node_count)
{
	// LP change: growable lists
	AccumulatedLineClips.ResetLength();
	AccumulatedEndpointClips.ResetLength();
	// short accumulated_line_clip_count= 0, accumulated_endpoint_clip_count= 0;
	// struct line_clip_data *accumulated_line_clips[MAXIMUM_CLIPS_PER_NODE];
	// struct endpoint_clip_data *accumulated_endpoint_clips[MAXIMUM_CLIPS_PER_NODE];
	clipping_window_data *first_clipping_window= NULL;
	clipping_window_data *last_clipping_window;
	endpoint_clip_data *endpoint;
	line_clip_data *line;
	short x0, x1; /* ignoring what clipping parameters we’ve gotten, this is the left and right borders of this node on the screen */
	short i, j, k;
	// LP: references to simplify the code
	GrowableList<endpoint_clip_data>& EndpointClips = RVPtr->EndpointClips;
	GrowableList<line_clip_data>& LineClips = RVPtr->LineClips;
	GrowableList<clipping_window_data>& ClippingWindows = RVPtr->ClippingWindows;
	ResizableList<short>& endpoint_x_coordinates = RVPtr->endpoint_x_coordinates;
	
	/* calculate x0,x1 (real left and right borders of this node) in case the left and right borders
		of the window are sloppy */
	{
		// LP change: look at beginning of chain
		polygon_data *polygon= get_polygon_data(ChainBegin->polygon_index); /* all these nodes should be the same */
		// struct polygon_data *polygon= get_polygon_data((*node_list)->polygon_index); /* all these nodes should be the same */
		
		x0= SHORT_MAX, x1= SHORT_MIN;
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
				x0= SHORT_MIN, x1= SHORT_MAX;
				break;
			}
		}
	}
	
	/* add left, top and bottom of screen */
	// LP change:
	endpoint_clip_data *EndpointClipPtr = &EndpointClips[indexLEFT_SIDE_OF_SCREEN];
	assert(AccumulatedEndpointClips.Add(EndpointClipPtr));
	line_clip_data *LineClipPtr = &LineClips[indexTOP_AND_BOTTOM_OF_SCREEN];
	assert(AccumulatedLineClips.Add(LineClipPtr));
	/*
	accumulated_endpoint_clips[accumulated_endpoint_clip_count++]= endpoint_clips + indexLEFT_SIDE_OF_SCREEN;
	accumulated_line_clips[accumulated_line_clip_count++]= line_clips + indexTOP_AND_BOTTOM_OF_SCREEN;
	*/

	/* accumulate clipping information, left to right, into local arrays */
	// Move along chain
	for (node_data *ChainNode = ChainBegin; ChainNode; ChainNode = ChainNode->PS_Shared)
	// for (k= 0;k<node_count;++k)
	{
		node_data *node;
		
		// LP change: use chain node as starting point
		for (node= ChainNode;node;node= node->parent) /* examine this node and all parents! */
		// for (node= node_list[k];node;node= node->parent) /* examine this node and all parents! */
		{
			/* sort in endpoint clips (left to right) */
			for (i= 0;i<node->clipping_endpoint_count;++i)
			{
				// LP change:
				endpoint= &EndpointClips[node->clipping_endpoints[i]];
				// endpoint= endpoint_clips + node->clipping_endpoints[i];
				
				// LP change:
				for (j= 0;j<AccumulatedEndpointClips.GetLength();++j)
				// for (j= 0;j<accumulated_endpoint_clip_count;++j)
				{
					// LP change:
					if (AccumulatedEndpointClips[j]==endpoint) { j= NONE; break; } /* found duplicate */
					if ((AccumulatedEndpointClips[j]->x==endpoint->x&&endpoint->flags==_clip_left) ||
						AccumulatedEndpointClips[j]->x>endpoint->x)
						/*
					if (accumulated_endpoint_clips[j]==endpoint) { j= NONE; break; } *//* found duplicate *//*
					if ((accumulated_endpoint_clips[j]->x==endpoint->x&&endpoint->flags==_clip_left) ||
						accumulated_endpoint_clips[j]->x>endpoint->x)
					*/
					{
						break; /* found sorting position if x is greater or x is equal and this is a left clip */
					}
				}
				
				if (j!=NONE) /* if the endpoint was not a duplicate */
				{
					/* expand the array, if necessary, and add the new endpoint */
					// LP change:
					int Length = AccumulatedEndpointClips.GetLength();
					assert(AccumulatedEndpointClips.Add());
					assert(AccumulatedEndpointClips.GetLength() <= 32767);		// Originally a short value
					if (j!=Length) memmove(&AccumulatedEndpointClips[j+1], &AccumulatedEndpointClips[j],
						(Length-j)*sizeof(endpoint_clip_data *));
					AccumulatedEndpointClips[j]= endpoint;
					/*
					assert(accumulated_endpoint_clip_count<MAXIMUM_CLIPS_PER_NODE);
					if (j!=accumulated_endpoint_clip_count) memmove(accumulated_endpoint_clips+j+1, accumulated_endpoint_clips+j,
						(accumulated_endpoint_clip_count-j)*sizeof(struct endpoint_clip_data *));
					accumulated_endpoint_clips[j]= endpoint;
					accumulated_endpoint_clip_count+= 1;
					*/
				}
			}

			/* sort in line clips, avoiding redundancies;  calculate_vertical_line_clip_data(),
				the function which deals with these, does not depend on them being sorted */
			for (i= 0;i<node->clipping_line_count;++i)
			{
				// LP change:
				line= &LineClips[node->clipping_lines[i]];
				// line= line_clips + node->clipping_lines[i];
				
				// LP change:
				for (j= 0;j<AccumulatedLineClips.GetLength();++j) if (AccumulatedLineClips[j]==line) break; /* found duplicate */
				if (j==AccumulatedLineClips.GetLength()) /* if the line was not a duplicate */
				{
					assert(AccumulatedLineClips.Add(line));
					assert(AccumulatedLineClips.GetLength() <= 32767);		// Originally a short value
				}
				/*
				for (j= 0;j<accumulated_line_clip_count;++j) if (accumulated_line_clips[j]==line) break; *//* found duplicate *//*
				if (j==accumulated_line_clip_count) *//* if the line was not a duplicate *//*
				{
					assert(accumulated_line_clip_count<MAXIMUM_CLIPS_PER_NODE);
					accumulated_line_clips[accumulated_line_clip_count++]= line;
				}
				*/
			}
		}
	}
	
//	dprintf("#%d accumulated points @ %p", accumulated_endpoint_clip_count, accumulated_endpoint_clips);
//	dprintf("#%d accumulated lines @ %p", accumulated_line_clip_count, accumulated_line_clips);

	/* add right side of screen */
	// LP change:
	EndpointClipPtr = &EndpointClips[indexRIGHT_SIDE_OF_SCREEN];
	assert(AccumulatedEndpointClips.Add(EndpointClipPtr));
	// assert(accumulated_endpoint_clip_count<MAXIMUM_CLIPS_PER_NODE);
	// accumulated_endpoint_clips[accumulated_endpoint_clip_count++]= endpoint_clips + indexRIGHT_SIDE_OF_SCREEN;

	/* build the clipping windows */
	{
		short state= _looking_for_left_clip;
		endpoint_clip_data *left_clip, *right_clip;

		// LP change:
		for (i= 0;i<AccumulatedEndpointClips.GetLength();++i)
		// for (i= 0;i<accumulated_endpoint_clip_count;++i)
		{
			// LP change:
			endpoint= AccumulatedEndpointClips[i];
			// endpoint= accumulated_endpoint_clips[i];
	
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
					// LP change:
					dprintf("render.c: build_clipping_windows(): bad state: %d",state);
					assert(false);
					// halt();
			}

			if (state==_building_clip_window)
			{
				if (left_clip->x<view->screen_width && right_clip->x>0 && left_clip->x<right_clip->x)
				{
					// LP change: clipping windows are in growable list
					int Length = ClippingWindows.GetLength();
					POINTER_DATA OldCWPointer;
					// Will memory get swapped?
					bool DoSwap = Length >= ClippingWindows.GetCapacity();
					if (DoSwap) OldCWPointer = POINTER_CAST(ClippingWindows.Begin());
					assert(ClippingWindows.Add());
					if (DoSwap)
					{
						// Get the clipping windows and sorted nodes into sync; no render objects yet
						POINTER_DATA NewCWPointer = POINTER_CAST(ClippingWindows.Begin());
						for (int k=0; k<ClippingWindows.GetLength(); k++)
						{
							clipping_window_data &ClippingWindow = ClippingWindows[k];
							if (ClippingWindow.next_window != NULL)
								ClippingWindow.next_window = (clipping_window_data *)(NewCWPointer + (POINTER_CAST(ClippingWindow.next_window) - OldCWPointer));
						}
						for (int k=0; k<SortedNodes.GetLength(); k++)
						{
							sorted_node_data &SortedNode = SortedNodes[k];
							if (SortedNode.clipping_windows != NULL)
								SortedNode.clipping_windows = (clipping_window_data *)(NewCWPointer + (POINTER_CAST(SortedNode.clipping_windows) - OldCWPointer));
						}
					}
					clipping_window_data *window= &ClippingWindows[Length];
					// struct clipping_window_data *window= next_clipping_window++;
					
					/* handle maintaining the linked list of clipping windows */
					// assert(next_clipping_window_index++<MAXIMUM_CLIPPING_WINDOWS);
					if (!first_clipping_window)
					{
						first_clipping_window= last_clipping_window= window;
					}
					else
					{
						last_clipping_window->next_window= window;
						last_clipping_window= window;
					}
					
					window->x0= left_clip->x, window->x1= right_clip->x;
					window->left= left_clip->vector;
					window->right= right_clip->vector;
					calculate_vertical_clip_data(AccumulatedLineClips.Begin(), AccumulatedLineClips.GetLength(), window,
						MAX(x0, window->x0), MIN(x1, window->x1));
					/*
					calculate_vertical_clip_data(accumulated_line_clips, accumulated_line_clip_count, window,
						MAX(x0, window->x0), MIN(x1, window->x1));
					*/
					window->next_window= NULL;
				}
				
				state= _looking_for_left_clip;
			}
		}
	}

	return first_clipping_window;
}

/* does not care if the given line_clips are sorted or not */
void RenderSortPolyClass::calculate_vertical_clip_data(
	line_clip_data **accumulated_line_clips,
	short accumulated_line_clip_count,
	clipping_window_data *window,
	short x0,
	short x1)
{
	if (x0<x1)
	{
		short i, x;
		line_clip_data *highest_line, *locally_highest_line, *line;
	
		/* get the highest top clip covering the requested horizontal run */		
		x= x0;
		highest_line= NULL;
		do
		{
			locally_highest_line= NULL;
			
			for (i= 0;i<accumulated_line_clip_count;++i)
			{
				line= accumulated_line_clips[i];
				
				if ((line->flags&_clip_up) && x>=line->x0 && x<line->x1 &&
					(!locally_highest_line || locally_highest_line->top_y<line->top_y))
				{
					locally_highest_line= line;
				}
			}
			vassert(locally_highest_line, csprintf(temporary, "didn’t find diddly at #%d [#%d,#%d]", x, x0, x1));
				
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
			
			for (i= 0;i<accumulated_line_clip_count;++i)
			{
				line= accumulated_line_clips[i];
				
				if ((line->flags&_clip_down) && x>=line->x0 && x<line->x1 &&
					(!locally_highest_line || locally_highest_line->bottom_y>line->bottom_y))
				{
					locally_highest_line= line;
				}
			}
			vassert(locally_highest_line, csprintf(temporary, "didn’t find diddly at #%d [#%d,#%d]", x, x0, x1));
				
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
	
	return;
}
