/*
	
	Rendering Object-Placement Class
	by Loren Petrich,
	August 6, 2000
	
	Contains the placement of inhabitant objects into the sorted polygons; from render.c
*/

#include <string.h>
#include "map.h"
#include "interface.h"
#include "lightsource.h"
#include "media.h"
#include "RenderPlaceObjs.h"

// Externed from render.c
extern void instantiate_rectangle_transfer_mode(view_data *view,
	rectangle_definition *rectangle, short transfer_mode, fixed transfer_phase);

// LP: "recommended" sizes of stuff in growable lists
#define MAXIMUM_RENDER_OBJECTS 72
#define MAXIMUM_OBJECT_BASE_NODES 6

// Inits everything
RenderPlaceObjsClass::RenderPlaceObjsClass():
	RenderObjects(MAXIMUM_RENDER_OBJECTS),
	RVPtr(NULL),	// Idiot-proofing
	RSPtr(NULL)
{}


/* ---------- initializing, building and sorting the object list */

void RenderPlaceObjsClass::initialize_render_object_list()
{
	// LP change: using growable list
	RenderObjects.ResetLength();
	/*
	render_object_count= 0;
	next_render_object= render_objects;
	*/
	
	return;
}


/* walk our sorted polygon lists, adding every object in every polygon to the render_object list,
	in depth order */
void RenderPlaceObjsClass::build_render_object_list(
	view_data *view)
{
	assert(RVPtr);	// Idiot-proofing
	assert(RSPtr);
	struct sorted_node_data *sorted_node;
	// LP: reference to simplify the code
	GrowableList<sorted_node_data>& SortedNodes = RSPtr->SortedNodes;

	initialize_render_object_list();
	
	// LP change:
	for (sorted_node= SortedNodes.RevBegin();sorted_node>SortedNodes.RevEnd();--sorted_node)
	// for (sorted_node= next_sorted_node-1;sorted_node>=sorted_nodes;--sorted_node)
	{
		struct polygon_data *polygon= get_polygon_data(sorted_node->polygon_index);
		fixed ambient_intensity= get_light_intensity(polygon->floor_lightsource_index);
		short object_index= polygon->first_object;
		
		while (object_index!=NONE)
		{
			short base_node_count;
			struct sorted_node_data *base_nodes[MAXIMUM_OBJECT_BASE_NODES];
			// LP change:
			struct render_object_data *render_object= build_render_object(view, (long_point3d *) NULL, ambient_intensity, base_nodes, &base_node_count, object_index);
			// struct render_object_data *render_object= build_render_object(view, (world_point3d *) NULL, ambient_intensity, base_nodes, &base_node_count, object_index);
			
			if (render_object)
			{
				build_aggregate_render_object_clipping_window(render_object, base_nodes, base_node_count);
				sort_render_object_into_tree(render_object, base_nodes, base_node_count);
			}
			
			object_index= get_object_data(object_index)->next_object;
		}
	}

	return;
}

// LP change: make it better able to do long-distance views
render_object_data *RenderPlaceObjsClass::build_render_object(
	view_data *view,
	long_point3d *origin, // world_point3d *origin,
	fixed ambient_intensity,
	sorted_node_data **base_nodes,
	short *base_node_count,
	short object_index)
{
	struct render_object_data *render_object= (struct render_object_data *) NULL;
	struct object_data *object= get_object_data(object_index);
	// LP: reference to simplify the code
	GrowableList<sorted_node_data>& SortedNodes = RSPtr->SortedNodes;
	
	// LP change: removed upper limit on number (restored it later)
	// if (!OBJECT_IS_INVISIBLE(object))
	if (!OBJECT_IS_INVISIBLE(object) && RenderObjects.GetLength()<get_dynamic_limit(_dynamic_limit_rendered))
	{
		// LP change: made this more long-distance-friendly
		long_point3d transformed_origin;
		// world_point3d transformed_origin;
		
		if (origin)
		{
			// LP change:
			transformed_origin.x = origin->x;
			transformed_origin.y = origin->y;
			transformed_origin.z = origin->z;
			// transformed_origin= *origin;
		}
		else
		{
			// LP change:
			world_point2d temp_tfm_origin;
			temp_tfm_origin.x = object->location.x;
			temp_tfm_origin.y = object->location.y;
			transformed_origin.z = object->location.z - view->origin.z;
			word tfm_origin_flags;
			transform_overflow_point2d(&temp_tfm_origin, (world_point2d *)&view->origin, view->yaw, &tfm_origin_flags);
			long_vector2d *tfm_origin_ptr = (long_vector2d *)(&transformed_origin);
			overflow_short_to_long_2d(temp_tfm_origin,tfm_origin_flags,*tfm_origin_ptr);
			/*
			transformed_origin= object->location;
			transformed_origin.z-= view->origin.z;
			transform_point2d((world_point2d *) &transformed_origin, (world_point2d *)&view->origin, view->yaw);
			*/
		}

		if (transformed_origin.x>MINIMUM_OBJECT_DISTANCE)
		{
			short x0, x1, y0, y1;
			struct shape_and_transfer_mode data;
			struct shape_information_data *shape_information;
			struct shape_information_data scaled_shape_information; // if necessary
			
			get_object_shape_and_transfer_mode(&view->origin, object_index, &data);
			shape_information= rescale_shape_information(
				extended_get_shape_information(data.collection_code, data.low_level_shape_index),
				&scaled_shape_information, GET_OBJECT_SCALE_FLAGS(object));

			/* if the caller wants it, give him the left and right extents of this shape */
			if (base_nodes)
			{
				*base_node_count= build_base_node_list(view, object->polygon, &object->location,
					shape_information->world_left, shape_information->world_right, base_nodes);
			}
			
			x0= view->half_screen_width + ((transformed_origin.y+shape_information->world_left)*view->world_to_screen_x)/transformed_origin.x;
			x1= view->half_screen_width + ((transformed_origin.y+shape_information->world_right)*view->world_to_screen_x)/transformed_origin.x;
			y0=	view->half_screen_height - (view->world_to_screen_y*(transformed_origin.z+shape_information->world_top))/transformed_origin.x + view->dtanpitch;
			y1= view->half_screen_height - (view->world_to_screen_y*(transformed_origin.z+shape_information->world_bottom))/transformed_origin.x + view->dtanpitch;
			if (x0<x1 && y0<y1)
			{
				// LP Change:
				int Length = RenderObjects.GetLength();
				POINTER_DATA OldROPointer;
				// Will memory get swapped?
				bool DoSwap = Length >= RenderObjects.GetCapacity();
				if (DoSwap) OldROPointer = POINTER_CAST(RenderObjects.Begin());
				assert(RenderObjects.Add());
				if (DoSwap)
				{
					// Get the render objects and sorted nodes into sync
					POINTER_DATA NewROPointer = POINTER_CAST(RenderObjects.Begin());
					for (int k=0; k<RenderObjects.GetLength(); k++)
					{
						render_object_data &RenderObject = RenderObjects[k];
						if (RenderObject.next_object != NULL)
							RenderObject.next_object = (render_object_data *)(NewROPointer + (POINTER_CAST(RenderObject.next_object) - OldROPointer));
					}
					for (int k=0; k<SortedNodes.GetLength(); k++)
					{
						sorted_node_data &SortedNode = SortedNodes[k];
						if (SortedNode.interior_objects != NULL)
							SortedNode.interior_objects = (render_object_data *)(NewROPointer + (POINTER_CAST(SortedNode.interior_objects) - OldROPointer));
						if (SortedNode.exterior_objects != NULL)
							SortedNode.exterior_objects = (render_object_data *)(NewROPointer + (POINTER_CAST(SortedNode.exterior_objects) - OldROPointer));
					}
				}
				render_object= &RenderObjects[Length];
				/*
				render_object= next_render_object++;
				render_object_count+= 1;
				*/
				
				render_object->rectangle.flags= 0;
				
				render_object->rectangle.x0= x0;
				render_object->rectangle.x1= x1;
				render_object->rectangle.y0= y0;
				render_object->rectangle.y1= y1;

				{
					// LP change: doing media handling more correctly here:
					short media_index = get_polygon_data(object->polygon)->media_index;
					media_data *media = (media_index != NONE) ? get_media_data(media_index) : NULL;
					
					// LP: the media splashes are clipped as if there was no liquid
					if (media && !OBJECT_IS_MEDIA_EFFECT(object))
					{
						render_object->ymedia= view->half_screen_height - (view->world_to_screen_y*(media->height-view->origin.z))/transformed_origin.x + view->dtanpitch;
					}
					else
					{
						// All the way down
						render_object->ymedia= SHORT_MAX;
					}
					
					/*
					short media_index= view->under_media_boundary ? view->under_media_index : get_polygon_data(object->polygon)->media_index;
					
					if (media_index!=NONE && !OBJECT_IS_MEDIA_EFFECT(object))
					{
						render_object->ymedia= view->half_screen_height - (view->world_to_screen_y*(media->height-view->origin.z))/transformed_origin.x + view->dtanpitch;
					}
					else
					{
						render_object->ymedia= SHORT_MAX;
					}
					*/
				}
				
				extended_get_shape_bitmap_and_shading_table(data.collection_code, data.low_level_shape_index,
					&render_object->rectangle.texture, &render_object->rectangle.shading_tables, view->shading_mode);
		
				// LP change: for the convenience of the OpenGL renderer
				render_object->rectangle.ShapeDesc = BUILD_DESCRIPTOR(data.collection_code,data.low_level_shape_index);
				
				render_object->rectangle.flip_vertical= FALSE;
				render_object->rectangle.flip_horizontal= (shape_information->flags&_X_MIRRORED_BIT) ? TRUE : FALSE;
				
				render_object->rectangle.depth= transformed_origin.x;
				instantiate_rectangle_transfer_mode(view, &render_object->rectangle, data.transfer_mode, data.transfer_phase);
				
				render_object->rectangle.ambient_shade= MAX(shape_information->minimum_light_intensity, ambient_intensity);

				if (view->shading_mode==_shading_infravision) render_object->rectangle.flags|= _SHADELESS_BIT;
				
				render_object->next_object= (struct render_object_data *) NULL;
				if (object->parasitic_object!=NONE)
				{
					struct render_object_data *parasitic_render_object;
					// LP change:
					long_point3d parasitic_origin= transformed_origin;
					// world_point3d parasitic_origin= transformed_origin;
					
					parasitic_origin.z+= shape_information->world_y0;
					parasitic_origin.y+= shape_information->world_x0;
					parasitic_render_object= build_render_object(view, &parasitic_origin, ambient_intensity,
						(struct sorted_node_data **) NULL, (short *) NULL, object->parasitic_object);
					
					if (parasitic_render_object)
					{
						assert(!parasitic_render_object->next_object); /* one parasite only, please */
	
						/* take the maximum intensity of the host and parasite as the intensity of the
							aggregate (does not handle multiple parasites correctly) */
						parasitic_render_object->rectangle.ambient_shade= render_object->rectangle.ambient_shade=
							MAX(parasitic_render_object->rectangle.ambient_shade, render_object->rectangle.ambient_shade);
						
						if (shape_information->flags&_KEYPOINT_OBSCURED_BIT) /* host obscures parasite */
						{
							render_object->next_object= parasitic_render_object;
						}
						else /* parasite obscures host; does not properly handle multiple parasites */
						{
							parasitic_render_object->next_object= render_object;
							render_object= parasitic_render_object;
						}
					}
				}
			}
		}
	}
	
	return render_object;
}

void RenderPlaceObjsClass::sort_render_object_into_tree(
	render_object_data *new_render_object, /* null-terminated linked list */
	sorted_node_data **base_nodes,
	short base_node_count)
{
	struct render_object_data *render_object, *last_new_render_object;
	struct render_object_data *deep_render_object= (struct render_object_data *) NULL;
	struct render_object_data *shallow_render_object= (struct render_object_data *) NULL;
	struct sorted_node_data *desired_node;
	short i;
	// LP: reference to simplify the code
	GrowableList<sorted_node_data>& SortedNodes = RSPtr->SortedNodes;

	/* find the last render_object in the given list of new objects */
	for (last_new_render_object= new_render_object;
			last_new_render_object->next_object;
			last_new_render_object= last_new_render_object->next_object)
		;

	/* find the two objects we must be lie between */
	// LP change:
	for (render_object= RenderObjects.Begin(); render_object<RenderObjects.End(); ++render_object)
	// for (render_object= render_objects; render_object<new_render_object; ++render_object)
	{
		/* if these two objects intersect... */
		if (render_object->rectangle.x1>new_render_object->rectangle.x0 && render_object->rectangle.x0<new_render_object->rectangle.x1 &&
			render_object->rectangle.y1>new_render_object->rectangle.y0 && render_object->rectangle.y0<new_render_object->rectangle.y1)
		{
			/* update our closest and farthest matches */
			if (render_object->rectangle.depth>new_render_object->rectangle.depth) /* found deeper intersecting object */
			{
				if (!deep_render_object || deep_render_object->rectangle.depth>render_object->rectangle.depth)
				{
					deep_render_object= render_object;
				}
			}
			else
			{
				if (render_object->rectangle.depth<new_render_object->rectangle.depth) /* found shallower intersecting object */
				{
					if (!shallow_render_object || shallow_render_object->rectangle.depth<=render_object->rectangle.depth)
					{
						shallow_render_object= render_object;
					}
				}
			}
		}
	}

	/* find the node we’d like to be in (that is, the node closest to the viewer of all the nodes
		we cross and therefore the latest one in the sorted node list) */
	desired_node= base_nodes[0];
	for (i= 1; i<base_node_count; ++i) if (base_nodes[i]>desired_node) desired_node= base_nodes[i];
	assert(desired_node>=SortedNodes.Begin() && desired_node<SortedNodes.End());
	// assert(desired_node>=sorted_nodes && desired_node<next_sorted_node);
	
	/* adjust desired node based on the nodes of the deep and shallow render object; only
		one of deep_render_object and shallow_render_object will be non-null after this if
		block.  the current object must be sorted with respect to this non-null object inside
		the object list of the desired_node */
	if (shallow_render_object && desired_node>=shallow_render_object->node)
	{
		/* we tried to sort too close to the front of the node list */
		desired_node= shallow_render_object->node;
		deep_render_object= (struct render_object_data *) NULL;
	}
	else
	{
		if (deep_render_object && desired_node<=deep_render_object->node)
		{
			/* we tried to sort too close to the back of the node list */
			desired_node= deep_render_object->node;
			shallow_render_object= (struct render_object_data *) NULL;
		}
		else
		{
			deep_render_object= shallow_render_object= (struct render_object_data *) NULL;
		}
	}
	
	/* update the .node fields of all the objects we’re about to add to reflect their new
		location in the sorted node list */
	for (render_object= new_render_object; render_object; render_object= render_object->next_object)
	{
		render_object->node= desired_node;
	}
	
	if (deep_render_object)
	{
		/* if it turns out that the object after deep_render_object (which we think we should be
			drawn in front of) is also deeper than us, make it the new deep_render_object */
		while ((render_object= deep_render_object->next_object)!=NULL
				&& render_object->rectangle.depth>new_render_object->rectangle.depth)
			deep_render_object= render_object;

		/* sort after deep_render_object object in the given node (so we are drawn in front of it) */
		last_new_render_object->next_object= deep_render_object->next_object;
		deep_render_object->next_object= new_render_object;
	}
	else
	{
//		if (shallow_render_object)
		{
			struct render_object_data **reference;
			
			/* find the reference to the shallow_render_object in the node list first (or the
				first object which is closer than new_render_object) */
			for (reference= &desired_node->exterior_objects;
					*reference!=shallow_render_object && *reference && (*reference)->rectangle.depth>new_render_object->rectangle.depth;
					reference= &(*reference)->next_object)
				;
			assert(!shallow_render_object || *reference);
			
			/* sort before this object in the given node (so we are drawn behind it) */
			last_new_render_object->next_object= *reference;
			*reference= new_render_object;
		}
//		else
//		{
//			/* sort anywhere in the node */
//			last_new_render_object->next_object= desired_node->exterior_objects;
//			desired_node->exterior_objects= new_render_object;
//		}
	}

	return;
}

enum /* build_base_node_list() states */
{
	_casting_left,
	_casting_right
};

/* we once thought it would be a clever idea to use the transformed endpoints, but, not.  we
	now bail if we can’t find a way out of the polygon we are given; usually this happens
	when we’re moving along gridlines */
short RenderPlaceObjsClass::build_base_node_list(
	view_data *view,
	short origin_polygon_index,
	world_point3d *origin,
	world_distance left_distance,
	world_distance right_distance,
	sorted_node_data **base_nodes)
{
	short cast_state;
	short base_node_count;
	world_distance origin_polygon_floor_height= get_polygon_data(origin_polygon_index)->floor_height;
	// LP: reference to simplify the code
	ResizableList<sorted_node_data *>& polygon_index_to_sorted_node = RSPtr->polygon_index_to_sorted_node;
	
	base_node_count= 1;
	base_nodes[0]= polygon_index_to_sorted_node[origin_polygon_index];

	cast_state= _casting_left;
	do
	{
		world_point2d destination= *((world_point2d *)origin);
		short polygon_index= origin_polygon_index;
		world_vector2d vector;
		
		switch (cast_state)
		{
			case _casting_left:
				translate_point2d(&destination, right_distance, NORMALIZE_ANGLE(view->yaw-QUARTER_CIRCLE));
//				dprintf("%s: (#%d,#%d)==>(#%d,#%d) (by #%d)", cast_state==_casting_left ? "left" : "right", origin->x, origin->y, destination.x, destination.y, cast_state==_casting_left ? left_distance : right_distance);
				cast_state= _casting_right;
				break;
			case _casting_right:
				translate_point2d(&destination, left_distance, NORMALIZE_ANGLE(view->yaw-QUARTER_CIRCLE));
//				dprintf("%s: (#%d,#%d)==>(#%d,#%d) (by #%d)", cast_state==_casting_left ? "left" : "right", origin->x, origin->y, destination.x, destination.y, cast_state==_casting_left ? left_distance : right_distance);
				cast_state= NONE;
				break;
			
			default:
				// LP change:
				assert(false);
				// halt();
		}

		vector.i= destination.x - origin->x;
		vector.j= destination.y - origin->y;
		
		/* move toward the given destination accumulating polygon indexes */
		do
		{
			struct polygon_data *polygon= get_polygon_data(polygon_index);
			short state= _looking_for_first_nonzero_vertex; /* really: testing first vertex state (we don’t have zero vertices) */
			short vertex_index= 0, vertex_delta= 1; /* start searching clockwise from vertex zero */
			world_point2d *vertex, *next_vertex;
			
			do
			{
				vertex= &get_endpoint_data(polygon->endpoint_indexes[vertex_index])->vertex;
		
				if ((vertex->x-origin->x)*vector.j - (vertex->y-origin->y)*vector.i >= 0)
				{
					/* endpoint is on the left side of our vector */
					switch (state)
					{
						case _looking_for_first_nonzero_vertex:
							/* search clockwise for transition (left to right) */
							state= _looking_clockwise_for_right_vertex;
							break;
						
						case _looking_counterclockwise_for_left_vertex: /* found the transition we were looking for */
							next_vertex= &get_endpoint_data(polygon->endpoint_indexes[WRAP_HIGH(vertex_index, polygon->vertex_count-1)])->vertex;
							polygon_index= polygon->adjacent_polygon_indexes[vertex_index];
							state= NONE;
							break;
					}
				}
				else
				{
					/* endpoint is on the right side of our vector */
					switch (state)
					{
						case _looking_for_first_nonzero_vertex:
							/* search counterclockwise for transition (right to left) */
							state= _looking_counterclockwise_for_left_vertex;
							vertex_delta= -1;
							break;
						
						case _looking_clockwise_for_right_vertex: /* found the transition we were looking for */
							next_vertex= vertex;
							vertex= &get_endpoint_data(polygon->endpoint_indexes[WRAP_LOW(vertex_index, polygon->vertex_count-1)])->vertex;
							polygon_index= polygon->adjacent_polygon_indexes[WRAP_LOW(vertex_index, polygon->vertex_count-1)];
							state= NONE;
							break;
					}
				}
				
				/* adjust vertex_index (clockwise or counterclockwise, depending on vertex_delta) */
				vertex_index= (vertex_delta<0) ? WRAP_LOW(vertex_index, polygon->vertex_count-1) :
					WRAP_HIGH(vertex_index, polygon->vertex_count-1);
				if (state!=NONE&&!vertex_index) polygon_index= state= NONE; /* we can’t find a way out; give up */
			}
			while (state!=NONE);
			
			if (polygon_index!=NONE)
			{
				polygon= get_polygon_data(polygon_index);
				
				/* can’t do above clipping (see note in change history) */
				if ((view->origin.z<origin->z && polygon->floor_height<origin_polygon_floor_height) ||
					(view->origin.z>origin->z && origin->z+WORLD_ONE_HALF<polygon->floor_height && polygon->floor_height>origin_polygon_floor_height))
				{
					/* if we’re above the viewer and going into a lower polygon or below the viewer and going
						into a higher polygon, don’t */
//					dprintf("discarding polygon #%d by height", polygon_index);
					polygon_index= NONE;
				}
				else
				{
//					dprintf("  into polygon #%d", polygon_index);
					if (!TEST_RENDER_FLAG(polygon_index, _polygon_is_visible)) polygon_index= NONE; /* don’t have transformed data, don’t even try! */
					if ((destination.x-vertex->x)*(next_vertex->y-vertex->y) - (destination.y-vertex->y)*(next_vertex->x-vertex->x) <= 0) polygon_index= NONE;
					if (polygon_index!=NONE && base_node_count<MAXIMUM_OBJECT_BASE_NODES) base_nodes[base_node_count++]= polygon_index_to_sorted_node[polygon_index];
				}
			}
		}
		while (polygon_index!=NONE);
	}
	while (cast_state!=NONE);

//	dprintf("found #%d polygons @ %p;dm %x %d;", base_polygon_count, base_polygon_indexes, base_polygon_indexes, base_polygon_count*sizeof(short));
	
	return base_node_count;
}

/* find the lowest bottom clip and the highest top clip of all nodes this object crosses.  then
	locate all left and right sides and compile them into one (or several) aggregate windows with
	the same top and bottom */
void RenderPlaceObjsClass::build_aggregate_render_object_clipping_window(
	render_object_data *render_object,
	sorted_node_data **base_nodes,
	short base_node_count)
{
	struct clipping_window_data *first_window= (struct clipping_window_data *) NULL;
	// LP: references to simplify the code
	GrowableList<clipping_window_data>& ClippingWindows = RVPtr->ClippingWindows;
	GrowableList<sorted_node_data>& SortedNodes = RSPtr->SortedNodes;
	
	if (base_node_count==1)
	{
		/* trivial case of one source window */
		first_window= base_nodes[0]->clipping_windows;
	}
	else
	{
		short i;
		short y0, y1;
		short left, right, left_count, right_count;
		short x0[MAXIMUM_OBJECT_BASE_NODES], x1[MAXIMUM_OBJECT_BASE_NODES]; /* sorted, left to right */
		struct clipping_window_data *window;
		world_distance depth= render_object->rectangle.depth;
		
		/* find the upper and lower bounds of the windows; we could do a better job than this by
			doing the same thing we do when the windows are originally built (i.e., calculating a
			new top/bottom for every window.  but screw that.  */
		left_count= right_count= 0;
		y0= SHORT_MAX, y1= SHORT_MIN;
		for (i= 0; i<base_node_count; ++i)
		{
			short j, k;
			
			window= base_nodes[i]->clipping_windows;
			
			/* update the top and bottom clipping bounds */
			if (window->y0<y0) y0= window->y0;
			if (window->y1>y1) y1= window->y1;
 			
			/* sort in the left side of this window */
			if (ABS(window->left.i)<depth)
			{
				for (j= 0; j<left_count && window->x0>=x0[j]; ++j)
					;
				for (k= j; k<left_count; ++k)
					x0[k+1]= x0[k];
				x0[j]= window->x0;
				left_count+= 1;
			}
			
			/* sort in the right side of this window */
			if (ABS(window->right.i)<depth)
			{
				for (j= 0; j<right_count && window->x1>=x1[j]; ++j)
					;
				for (k= j; k<right_count; ++k)
					x1[k+1]= x1[k];
				x1[j]= window->x1;
				right_count+= 1;
			}
		}
		
		/* build the windows, left to right */
		for (left= 0, right= 0; left<left_count && right<right_count; )
		{
			if (left==left_count-1 || x0[left+1]>x1[right])
			{
				if (x0[left]<x1[right]) /* found one between x0[left] and x1[right] */
				{
					/* allocate it */
					// LP Change:
					int Length = ClippingWindows.GetLength();
					POINTER_DATA OldCWPointer;
					// Will memory get swapped?
					bool DoSwap = Length >= ClippingWindows.GetCapacity();
					if (DoSwap) OldCWPointer = POINTER_CAST(ClippingWindows.Begin());
					assert(ClippingWindows.Add());
					if (DoSwap)
					{
						// Get the sorted nodes into sync
						// Also, the render objects and the parent window
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
						for (int k=0; k<RenderObjects.GetLength(); k++)
						{
							render_object_data &RenderObject = RenderObjects[k];
							if (RenderObject.clipping_windows != NULL)
								RenderObject.clipping_windows = (clipping_window_data *)(NewCWPointer + (POINTER_CAST(RenderObject.clipping_windows) - OldCWPointer));
						}
						if (first_window != NULL)
							first_window = (clipping_window_data *)(NewCWPointer + (POINTER_CAST(first_window) - OldCWPointer));
					}
					window= &ClippingWindows[Length];
					/*
					assert(next_clipping_window_index++<MAXIMUM_CLIPPING_WINDOWS);
					window= next_clipping_window++;
					*/
					
					/* build it */
					window->x0= x0[left], window->x1= x1[right];
					window->y0= y0, window->y1= y1;
					
					/* link it */
					window->next_window= first_window;
					first_window= window;
				}
				
				/* advance left by one, then advance right until it’s greater than left */
				if (++left<left_count) while (x0[left]>x1[right] && right<right_count) ++right;
			}
			else
			{
				left+= 1;
			}
		}
	}
	
	/* stuff our windows in all objects hanging off our first object (i.e., all parasites) */	
	for (; render_object; render_object= render_object->next_object) render_object->clipping_windows= first_window;

	return;
}

#define NUMBER_OF_SCALED_VALUES 6

shape_information_data *RenderPlaceObjsClass::rescale_shape_information(
	shape_information_data *unscaled,
	shape_information_data *scaled,
	word flags)
{
	if (flags)
	{
		world_distance *scaled_values= &scaled->world_left;
		world_distance *unscaled_values= &unscaled->world_left;
		short i;
		
		scaled->flags= unscaled->flags;
		scaled->minimum_light_intensity= unscaled->minimum_light_intensity;
		
		if (flags&_object_is_enlarged)
		{
			for (i= 0; i<NUMBER_OF_SCALED_VALUES; ++i)
			{
				*scaled_values++= *unscaled_values + (*unscaled_values>>2), unscaled_values+= 1;
			}
		}
		else
		{
			if (flags&_object_is_tiny)
			{
				for (i= 0; i<NUMBER_OF_SCALED_VALUES; ++i)
				{
					*scaled_values++= (*unscaled_values>>1), unscaled_values+= 1;
				}
			}
		}
	}
	else
	{
		scaled= unscaled;
	}
	
	return scaled;
}
