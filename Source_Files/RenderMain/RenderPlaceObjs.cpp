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
	
	Contains the placement of inhabitant objects into the sorted polygons; from render.c
	
	Made [view_data *view] a member and removed it as an argument

Sep 2, 2000 (Loren Petrich):
	Added some idiot-proofing, since the shapes accessor now returns NULL for nonexistent bitmaps
	
Oct 13, 2000 (Loren Petrich):
	Replaced GrowableLists and ResizableLists with STL vectors

Oct 19, 2000 (Loren Petrich):
	Added graceful escape in case of nonexistent shape in build_render_object().

Jan 17, 2001 (Loren Petrich):
	Added vertical flipping

Sept 11, 2001 (Loren Petrich):
	Added 3D-model support, including calculation of a projected bounding box and the miner's-light distance and direction

Feb 3, 2003 (Loren Petrich):
	Added chase-cam semitransparency

May 3, 2003 (Br'fin (Jeremy Parsons))
	Added LowLevelShape workaround for passing LowLevelShape info of sprites
	instead of abusing/overflowing shape_descriptors
*/

#include "cseries.h"

#include "map.h"
#include "lightsource.h"
#include "media.h"
#include "RenderPlaceObjs.h"
#include "OGL_Setup.h"
#include "ChaseCam.h"
#include "player.h"
#include "ephemera.h"
#include "preferences.h"

#include <string.h>
#include <limits.h>
#include <algorithm>
#include <boost/container/small_vector.hpp>


// LP: "recommended" sizes of stuff in growable lists
#define MAXIMUM_RENDER_OBJECTS 72

// The base nodes of an object (contiguous nodes it crosses and should draw over), the crossover points, and total span
struct RenderPlaceObjsClass::span_data
{
	struct base_node_data
	{
		sorted_node_data* node;
		long_point2d right_pt; // right edge of object's span in this node, or span_data::right_pt if rightmost node
	};	                       // (even if that point lies outside)
	
	boost::container::small_vector<base_node_data, 6> base_nodes; // left-to-right
	
	// Total span (can be off-map: coords are within +/- 2^16 instead of 2^15)
	long_point2d left_pt;
	long_point2d right_pt;
};

// For finding the 2D projection of the bounding box;
// also finds other useful info
static void FindProjectedBoundingBox(GLfloat BoundingBox[2][3],
	long_point3d& TransformedPosition,
	GLfloat Scale,
	short RelativeAngle,
	shape_information_data& ShapeInfo,
	short DepthType,
	int& Farthest,
	int& ProjDistance,
	int& DistanceRef,
	int& LightDepth,
	GLfloat *Direction
);


// Inits everything
RenderPlaceObjsClass::RenderPlaceObjsClass():
	view(NULL),	// Idiot-proofing
	RVPtr(NULL),
	RSPtr(NULL)
{RenderObjects.reserve(MAXIMUM_RENDER_OBJECTS);}


/* ---------- initializing, building and sorting the object list */

void RenderPlaceObjsClass::initialize_render_object_list()
{
	// LP change: using growable list
	RenderObjects.clear();
}

/* walk our sorted polygon lists, adding every object in every polygon to the render_object list,
	in depth order */
void RenderPlaceObjsClass::build_render_object_list()
{
	assert(view);	// Idiot-proofing
	assert(RVPtr);
	assert(RSPtr);
	sorted_node_data *sorted_node;
	// LP: reference to simplify the code
	vector<sorted_node_data>& SortedNodes = RSPtr->SortedNodes;
	
	// What's the object index of oneself in the game?
	short self_index = current_player->object_index;

	initialize_render_object_list();
	
	for (sorted_node = &SortedNodes.back(); sorted_node >= &SortedNodes.front(); --sorted_node)
	{
		polygon_data *polygon= get_polygon_data(sorted_node->polygon_index);
		_fixed floor_intensity= get_light_intensity(polygon->floor_lightsource_index);
		_fixed ceiling_intensity = get_light_intensity(polygon->ceiling_lightsource_index);
		short object_index= polygon->first_object;
		
		while (object_index!=NONE)
		{
			float Opacity = (object_index == self_index) ? GetChaseCamData().Opacity : 1;
			add_object_to_sorted_nodes(get_object_data(object_index), floor_intensity, ceiling_intensity, Opacity);
			object_index= get_object_data(object_index)->next_object;
		}

		if (graphics_preferences->ephemera_quality != _ephemera_off)
		{
			auto ephemera_index = get_polygon_ephemera(sorted_node->polygon_index);
			while (ephemera_index != NONE)
			{
				add_object_to_sorted_nodes(get_ephemera_data(ephemera_index), floor_intensity, ceiling_intensity, 1);
				ephemera_index = get_ephemera_data(ephemera_index)->next_object;
			}
		}
	}
}

// Return a linked list of new render objects (or null) for an object and any parasites, in draw order (back-to-front),
// without clipping windows, and unattached to any sorted node
render_object_data *RenderPlaceObjsClass::build_render_object(
	object_data* object,
	_fixed floor_intensity,
	_fixed ceiling_intensity,
	float Opacity,
	long_point3d* origin,
	long_point3d* rel_origin)
{
	render_object_data *render_object= NULL;
	// LP: reference to simplify the code
	vector<sorted_node_data>& SortedNodes = RSPtr->SortedNodes;
	
	// LP change: removed upper limit on number (restored it later)
	if (!OBJECT_IS_INVISIBLE(object) && int(RenderObjects.size())<get_dynamic_limit(_dynamic_limit_rendered))
	{
		// LP change: made this more long-distance-friendly
		long_point3d transformed_origin;
		
		if (origin)
		{
			transformed_origin.x = origin->x;
			transformed_origin.y = origin->y;
			transformed_origin.z = origin->z;
		}
		else
		{
			world_point2d temp_tfm_origin;
			temp_tfm_origin.x = object->location.x;
			temp_tfm_origin.y = object->location.y;
			transformed_origin.z = object->location.z - view->origin.z;
			uint16 tfm_origin_flags;
			transform_overflow_point2d(&temp_tfm_origin, (world_point2d *)&view->origin, view->yaw, &tfm_origin_flags);
			long_vector2d *tfm_origin_ptr = (long_vector2d *)(&transformed_origin);
			overflow_short_to_long_2d(temp_tfm_origin,tfm_origin_flags,*tfm_origin_ptr);
		}
		
		// May do some calculation on spries that are behind the view position,
		// but that is necessary for correctly rendering models that are behind the viewpoint,
		// but which extend to in fron of the viewpoint.
		// if (transformed_origin.x>MINIMUM_OBJECT_DISTANCE)
		{
			int x0, x1, y0, y1;	// Need the extra precision here
			shape_and_transfer_mode data;
			shape_information_data *shape_information;
			shape_information_data scaled_shape_information; // if necessary
			shape_information_data model_shape_information;	// also if necessary
			
			// Maximum distance of object parts (use position if a sprite)
			int Farthest = transformed_origin.x;
			
			// Projected distance of centroid
			int ProjDistance = transformed_origin.x;
			
			// Reference distance for frame calculation (use position if a sprite)
			int DistanceRef = transformed_origin.x;
			
			// For the convenience of the 3D-model renderer
			int LightDepth = transformed_origin.x;
			GLfloat LightDirection[3];

			get_object_shape_and_transfer_mode(&view->origin, object, &data);
			// Nonexistent shape: skip
			if (data.collection_code == NONE) return NULL;
			
			OGL_ModelData* ModelPtr = nullptr;
			
#ifdef HAVE_OPENGL
			// Find which 3D model will take the place of this sprite, if any
			short ModelSequence;
			ModelPtr = OGL_GetModelData(
				GET_COLLECTION(data.collection_code),
				GET_DESCRIPTOR_SHAPE(object->shape),
				ModelSequence);
#endif
			shape_information= rescale_shape_information(
				extended_get_shape_information(data.collection_code, data.low_level_shape_index),
				&scaled_shape_information, GET_OBJECT_SCALE_FLAGS(object));
			// Nonexistent frame: skip
			if (!shape_information) return NULL;
			
			// Create a fake sprite rectangle using the model's bounding box
			float Scale = 1;
#ifdef HAVE_OPENGL
			if (ModelPtr)
			{
				// Copy over
				model_shape_information = *shape_information;
				
				// Set up scaling and return pointer
				if (TEST_FLAG(object->flags,_object_is_enlarged)) Scale = 1.25;
				else if (TEST_FLAG(object->flags,_object_is_tiny)) Scale = 0.5;
				
				FindProjectedBoundingBox(ModelPtr->Model.BoundingBox,
					transformed_origin, Scale, object->facing - view->yaw,
					model_shape_information, ModelPtr->DepthType,
					Farthest, ProjDistance, DistanceRef, LightDepth, LightDirection);
				
				// Set pointer back
				shape_information = &model_shape_information;
			}
#endif
			// Too close?
			if (Farthest < MINIMUM_OBJECT_DISTANCE) return NULL;
			
			assert(DistanceRef > 0);
			
			{
				// Doing this with full-integer arithmetic to avoid mis-clipping;
				x0= view->half_screen_width + (int(transformed_origin.y+shape_information->world_left)*view->world_to_screen_x)/DistanceRef;
				x1= view->half_screen_width + (int(transformed_origin.y+shape_information->world_right)*view->world_to_screen_x)/DistanceRef;
				y0=	view->half_screen_height - (view->world_to_screen_y*int(transformed_origin.z+shape_information->world_top))/DistanceRef + view->dtanpitch;
				y1= view->half_screen_height - (view->world_to_screen_y*int(transformed_origin.z+shape_information->world_bottom))/DistanceRef + view->dtanpitch;
			
				size_t Length = RenderObjects.size();
				POINTER_DATA OldROPointer = POINTER_CAST(RenderObjects.data());
				
				// Add a dummy object and check if the pointer got changed
				render_object = &RenderObjects.emplace_back();
				POINTER_DATA NewROPointer = POINTER_CAST(RenderObjects.data());
				
				if (NewROPointer != OldROPointer)
				{
					// Get the render objects and sorted nodes into sync
					for (size_t k=0; k<Length; k++)
					{
						render_object_data &RenderObject = RenderObjects[k];
						if (RenderObject.next_object != NULL)
							RenderObject.next_object = (render_object_data *)(NewROPointer + (POINTER_CAST(RenderObject.next_object) - OldROPointer));
					}
					for (size_t k=0; k<SortedNodes.size(); k++)
					{
						sorted_node_data &SortedNode = SortedNodes[k];
						if (SortedNode.interior_objects != NULL)
							SortedNode.interior_objects = (render_object_data *)(NewROPointer + (POINTER_CAST(SortedNode.interior_objects) - OldROPointer));
						if (SortedNode.exterior_objects != NULL)
							SortedNode.exterior_objects = (render_object_data *)(NewROPointer + (POINTER_CAST(SortedNode.exterior_objects) - OldROPointer));
					}
				}
				
				render_object->clipping_windows = nullptr;
				render_object->rectangle.flags= 0;
				
				// Clamp to short values
				render_object->rectangle.x0= PIN(x0,SHRT_MIN,SHRT_MAX);
				render_object->rectangle.x1= PIN(x1,SHRT_MIN,SHRT_MAX);
				render_object->rectangle.y0= PIN(y0,SHRT_MIN,SHRT_MAX);
				render_object->rectangle.y1= PIN(y1,SHRT_MIN,SHRT_MAX);

				{
					// LP change: doing media handling more correctly here:
					short media_index = get_polygon_data(object->polygon)->media_index;
					media_data *media = (media_index != NONE) ? get_media_data(media_index) : NULL;
					
					// LP: the media splashes are clipped as if there was no liquid
					// And the 3D-model info needs the object-relative liquid height
					if (media && !OBJECT_IS_MEDIA_EFFECT(object))
					{
						render_object->rectangle.LiquidRelHeight = PIN(media->height - object->location.z, SHRT_MIN,SHRT_MAX);
						int ProjLiquidHeight = view->half_screen_height - (view->world_to_screen_y*(media->height-view->origin.z))/DistanceRef + view->dtanpitch;
						render_object->ymedia= PIN(ProjLiquidHeight,SHRT_MIN,SHRT_MAX);
					}
					else
					{
						// All the way down
						render_object->rectangle.LiquidRelHeight = SHRT_MIN;
						render_object->ymedia= SHRT_MAX;
					}
				}
				
				extended_get_shape_bitmap_and_shading_table(data.collection_code, data.low_level_shape_index,
					&render_object->rectangle.texture, &render_object->rectangle.shading_tables, view->shading_mode);
				
				// LP: not sure how to handle nonexistent sprites here
				assert(render_object->rectangle.texture);
				
				// LP change: for the convenience of the OpenGL renderer
				render_object->rectangle.Opacity = Opacity;
				render_object->rectangle.ShapeDesc = BUILD_DESCRIPTOR(data.collection_code,0);
				render_object->rectangle.LowLevelShape = data.low_level_shape_index;
				render_object->rectangle.ModelPtr = ModelPtr;
#ifdef HAVE_OPENGL
				if (ModelPtr)
				{
					render_object->rectangle.ModelSequence = ModelSequence;
					render_object->rectangle.ModelFrame = data.Frame;
					render_object->rectangle.NextModelFrame = data.NextFrame;
					render_object->rectangle.MixFrac = data.Ticks > 0 ?
						float(data.Phase)/float(data.Ticks) : 0;
					render_object->rectangle.Azimuth = object->facing;
					render_object->rectangle.LightDepth = LightDepth;
					objlist_copy(render_object->rectangle.LightDirection,LightDirection,3);
				}
#endif
				// need this for new rendering pipeline
				render_object->rectangle.WorldLeft = shape_information->world_left;
				render_object->rectangle.WorldBottom = shape_information->world_bottom;
				render_object->rectangle.WorldRight = shape_information->world_right;
				render_object->rectangle.WorldTop = shape_information->world_top;
				render_object->rectangle.Position = object->location;
				if(rel_origin) {
					render_object->rectangle.WorldLeft += rel_origin->x;
					render_object->rectangle.WorldRight += rel_origin->x;
					render_object->rectangle.WorldTop += rel_origin->z; 
					render_object->rectangle.WorldBottom += rel_origin->z; 
				}
				render_object->rectangle.Scale = Scale;
					
				render_object->rectangle.flip_vertical= (shape_information->flags&_Y_MIRRORED_BIT) ? true : false;
				render_object->rectangle.flip_horizontal= (shape_information->flags&_X_MIRRORED_BIT) ? true : false;
				
				// Calculate the object's horizontal position
				// for the convenience of doing teleport-in/teleport-out
				switch(data.transfer_mode)
				{
				case _xfer_fold_in:
				case _xfer_fold_out:
					render_object->rectangle.xc =
						view->half_screen_width + (int(transformed_origin.y)*view->world_to_screen_x)/DistanceRef;
				}
				
				render_object->rectangle.ProjDistance = PIN(ProjDistance,SHRT_MIN,SHRT_MAX);			
				render_object->rectangle.depth= DistanceRef;
				instantiate_rectangle_transfer_mode(view, &render_object->rectangle, data.transfer_mode, data.transfer_phase);
				
				render_object->rectangle.ambient_shade= MAX(shape_information->minimum_light_intensity, floor_intensity);
				render_object->rectangle.ceiling_light= MAX(shape_information->minimum_light_intensity, ceiling_intensity);

				if (view->shading_mode==_shading_infravision) render_object->rectangle.flags|= _SHADELESS_BIT;
				
				render_object->next_object= NULL;
				if (object->parasitic_object!=NONE)
				{
					render_object_data *parasitic_render_object;
					long_point3d parasitic_origin= transformed_origin;
					long_point3d parasitic_rel_origin;
					parasitic_rel_origin.z = shape_information->world_y0;
					parasitic_rel_origin.x = shape_information->world_x0;
					parasitic_origin.z+= shape_information->world_y0;
					parasitic_origin.y+= shape_information->world_x0;
					
					const auto render_object_index = render_object - RenderObjects.data();
					
					parasitic_render_object = build_render_object(
						get_object_data(object->parasitic_object),
						floor_intensity,
						ceiling_intensity,
						Opacity,
						&parasitic_origin,
						&parasitic_rel_origin);
					
					// Recover our pointer after build_render_object() potentially invalidated it
					render_object = &RenderObjects[render_object_index];
					
					if (parasitic_render_object)
					{
						// Code will now allow multiple parasites
						// assert(!parasitic_render_object->next_object); /* one parasite only, please */
	
						/* take the maximum intensity of the host and parasite as the intensity of the
							aggregate (does not handle multiple parasites correctly) */
						// Suppressed as possibly confusing for models
						/*
						parasitic_render_object->rectangle.ambient_shade= render_object->rectangle.ambient_shade=
							MAX(parasitic_render_object->rectangle.ambient_shade, render_object->rectangle.ambient_shade);
						parasitic_render_object->rectangle.ceiling_light= render_object->rectangle.ceiling_light=
							MAX(parasitic_render_object->rectangle.ceiling_light, render_object->rectangle.ceiling_light);
						*/
						
						if (shape_information->flags&_KEYPOINT_OBSCURED_BIT) /* host obscured by parasite */
						{
							render_object->next_object= parasitic_render_object;
						}
						else /* parasite obscured by host; does not properly handle multiple parasites */
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
	const span_data& span)
{
	render_object_data *render_object, *last_new_render_object;
	render_object_data *deep_render_object= NULL;
	render_object_data *shallow_render_object= NULL;

	/* find the last render_object in the given list of new objects */
	for (last_new_render_object= new_render_object;
			last_new_render_object->next_object;
			last_new_render_object= last_new_render_object->next_object)
		;

	/* find the two objects we must be lie between */
	for (render_object = &RenderObjects.front(); render_object <= &RenderObjects.back(); ++render_object)
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
	sorted_node_data* desired_node = span.base_nodes[0].node;
	for (int i = 1; i < span.base_nodes.size(); ++i)
		desired_node = std::max(desired_node, span.base_nodes[i].node);
	
	/* adjust desired node based on the nodes of the deep and shallow render object; only
		one of deep_render_object and shallow_render_object will be non-null after this if
		block.  the current object must be sorted with respect to this non-null object inside
		the object list of the desired_node */
	if (shallow_render_object && desired_node>=shallow_render_object->node)
	{
		/* we tried to sort too close to the front of the node list */
		desired_node= shallow_render_object->node;
		deep_render_object= NULL;
	}
	else
	{
		if (deep_render_object && desired_node<=deep_render_object->node)
		{
			/* we tried to sort too close to the back of the node list */
			desired_node= deep_render_object->node;
			shallow_render_object= NULL;
		}
		else
		{
			deep_render_object= shallow_render_object= NULL;
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
			render_object_data **reference;
			
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
}

// Return span and base node info for a render object (or list of such)
auto RenderPlaceObjsClass::build_base_node_list(
	const render_object_data* render_object,
	short origin_polygon_index) -> span_data
{
	assert(render_object);
	
	span_data result;
	const auto origin = render_object->rectangle.Position;
	world_distance origin_polygon_floor_height= get_polygon_data(origin_polygon_index)->floor_height;
	// LP: reference to simplify the code
	vector<sorted_node_data *>& polygon_index_to_sorted_node = RSPtr->polygon_index_to_sorted_node;
	
	// Add nodes to result.base_nodes in order found (updating the previous node's .right_pt if scanning rightward)
	auto scan_toward = [&](long_point2d destination, bool scanning_rightward)
	{
		short polygon_index= origin_polygon_index;
		auto scan_vector = destination - origin.xy();
		
		/* move toward the given destination accumulating polygon indexes */
		do
		{
			polygon_data *polygon= get_polygon_data(polygon_index);
			short state= _looking_for_first_nonzero_vertex; /* really: testing first vertex state (we don’t have zero vertices) */
			short vertex_index= 0, vertex_delta= 1; /* start searching clockwise from vertex zero */
			world_point2d vertex_a, vertex_b; // the vertices (in clockwise order) of the line we cross, if any
			
			world_point2d prev_vertex;
			do
			{
				const auto vertex = get_endpoint_data(polygon->endpoint_indexes[vertex_index])->vertex;
				
				if (cross_product_k(vertex - origin.xy(), scan_vector) >= 0)
				{
					/* endpoint is on the left side of our vector */
					switch (state)
					{
						case _looking_for_first_nonzero_vertex:
							/* search clockwise for transition (left to right) */
							state= _looking_clockwise_for_right_vertex;
							break;
						
						case _looking_counterclockwise_for_left_vertex: /* found the transition we were looking for */
							vertex_a = vertex;
							vertex_b = prev_vertex;
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
							vertex_a = prev_vertex;
							vertex_b = vertex;
							polygon_index= polygon->adjacent_polygon_indexes[WRAP_LOW(vertex_index, polygon->vertex_count-1)];
							state= NONE;
							break;
					}
				}
				
				/* adjust vertex_index (clockwise or counterclockwise, depending on vertex_delta) */
				vertex_index= (vertex_delta<0) ? WRAP_LOW(vertex_index, polygon->vertex_count-1) :
					WRAP_HIGH(vertex_index, polygon->vertex_count-1);
				if (state!=NONE&&!vertex_index) polygon_index= state= NONE; /* we can’t find a way out; give up */
				
				prev_vertex = vertex;
			}
			while (state!=NONE);
			
			if (polygon_index!=NONE)
			{
				polygon= get_polygon_data(polygon_index);
				
				/* can’t do above clipping (see note in change history) */
				if ((view->origin.z < origin.z && polygon->floor_height < origin_polygon_floor_height) ||
					(view->origin.z > origin.z && origin.z + WORLD_ONE_HALF < polygon->floor_height && polygon->floor_height > origin_polygon_floor_height))
				{
					/* if we’re above the viewer and going into a lower polygon or below the viewer and going
						into a higher polygon, don’t */
					polygon_index= NONE;
				}
				else
				{
					if (!TEST_RENDER_FLAG(polygon_index, _polygon_is_visible)) polygon_index= NONE; /* don’t have transformed data, don’t even try! */
					
					const auto line_vec = vertex_b - vertex_a;
					
					if (cross_product_k(destination - vertex_a, line_vec) <= 0)
						polygon_index = NONE; // reached destination without entering polygon_index
					
					if (polygon_index != NONE)
					{
						result.base_nodes.push_back({polygon_index_to_sorted_node[polygon_index], {}});
						
						// Update the relevant .right_pt (the previous node if scanning rightward, else the new node)
						const auto cpk_vl = cross_product_k(scan_vector, line_vec); // > 0
						const float u = 1.f*cross_product_k(vertex_a - origin.xy(), scan_vector) / cpk_vl; // [0, 1]
						const auto right_pt = vertex_a + u*line_vec;
						const int new_index = result.base_nodes.size() - 1;
						result.base_nodes[new_index - (scanning_rightward ? 1 : 0)].right_pt = right_pt;
					}
				}
			}
		}
		while (polygon_index!=NONE);
	};
	
	world_distance left_distance = INT16_MAX;
	world_distance right_distance = INT16_MIN;
	for (const auto* ro = render_object; ro; ro = ro->next_object)
	{
		left_distance = std::min(left_distance, ro->rectangle.WorldLeft);
		right_distance = std::max(right_distance, ro->rectangle.WorldRight);
	}
	
	auto pt_along_object_rect = [&](world_distance offset_from_origin) -> long_point2d // can be off-map
	{
		const angle right = normalize_angle(view->yaw + QUARTER_CIRCLE);
		const auto v = (1.f*offset_from_origin/TRIG_MAGNITUDE) * long_vector2d{cosine_table[right], sine_table[right]};
		return origin.xy() + v;
	};
	
	result.left_pt = pt_along_object_rect(left_distance);
	result.right_pt = pt_along_object_rect(right_distance);
	
	const auto eye_pt = view->origin.xy();
	const bool left_pt_is_left_of_origin = cross_product_k(result.left_pt - eye_pt, origin.xy() - eye_pt) > 0;
	const bool right_pt_is_right_of_origin = cross_product_k(result.right_pt - eye_pt, origin.xy() - eye_pt) < 0;
	
	if (left_pt_is_left_of_origin)
		scan_toward(result.left_pt, /*scanning_rightward:*/ false);
	
	std::reverse(result.base_nodes.begin(), result.base_nodes.end()); // re-order left-to-right
	result.base_nodes.push_back({polygon_index_to_sorted_node[origin_polygon_index], {}});
	
	if (right_pt_is_right_of_origin)
		scan_toward(result.right_pt, /*scanning_rightward:*/ true);
	
	result.base_nodes.back().right_pt = result.right_pt; // even if the point is outside the node
	
	return result;
}

/* ---------- initializing and calculating clip data */

// Assign to a render object (or list of such) clipping windows aggregated from its base nodes
//  - any newly created windows don't overlap and are linked left-to-right like node windows
void RenderPlaceObjsClass::build_aggregate_render_object_clipping_window(
	render_object_data *render_object,
	const span_data& span)
{
	clipping_window_data *first_window= NULL;
	// LP: references to simplify the code
	vector<clipping_window_data>& ClippingWindows = RVPtr->ClippingWindows;
	vector<sorted_node_data>& SortedNodes = RSPtr->SortedNodes;
	
	if (span.base_nodes.size() == 1)
	{
		/* trivial case of one base node */
		first_window = span.base_nodes[0].node->clipping_windows;
	}
	else
	{
		// Add new windows with x-extents that are the union of every overlap between a node window and the span of the
		// object in that node, but without clipping the outermost extents of the outermost contributing windows
	
		auto eye_vec_toward = [&](long_point2d pt) -> long_vector2d // == 1024*(eye vec _to_ the pt)
		{ 
			const auto v = pt - view->origin.xy();
			const int16 c = cosine_table[view->yaw];
			const int16 s = sine_table[view->yaw];
			return {c*v.i + s*v.j, c*v.j - s*v.i};
		};
	
		int32 head = NONE; // index of first window in result list, if non-empty, else NONE
		int32 top; // index of contributing node window with highest top-clip
		int32 bottom; // index of contributing node window with lowest bottom-clip
		auto base_left = eye_vec_toward(span.left_pt);
	
		for (int i = 0, n = span.base_nodes.size(); i < n; ++i)
		{ 
			// base_left and base_right delimit the span of the object within node i
			// (we pretend the outermost nodes contain the object ends even if they don't)
			const auto base_right = eye_vec_toward(span.base_nodes[i].right_pt);
	
			for (auto* win = span.base_nodes[i].node->clipping_windows; win; win = win->next_window)
			{ 
				if (cross_product_k(base_right, win->left) > 0)
					break; // this and remaining windows of this node are fully right of the node span
				
				if (cross_product_k(-win->right, base_left) > 0)
					continue; // window is fully left of the node span
				
				const int32 win_index = win - ClippingWindows.data();
				
				if (head == NONE || cross_product_k(ClippingWindows[top].top, win->top) > 0)
					top = win_index; // found higher top-clip
				
				if (head == NONE || cross_product_k(ClippingWindows[bottom].bottom, win->bottom) < 0)
					bottom = win_index; // found lower bottom-clip
				
				if (head != NONE && cross_product_k(win->left, -ClippingWindows.back().right) >= 0)
				{ 
					// Window overlaps or abuts last-added window from a prior node (both windows overlap base_left);
					// tentatively use its right-clip
					ClippingWindows.back().right = win->right;
					ClippingWindows.back().x1 = win->x1;
				} 
				else
				{
					// Copy the window into the result list (right-clip is tentative; y-clips will be set later)
					
					size_t Length = ClippingWindows.size();
					POINTER_DATA OldCWPointer = POINTER_CAST(ClippingWindows.data());
					
					ClippingWindows.push_back(*win);
					ClippingWindows.back().next_window = nullptr;
					
					// Restore any clipping window pointers invalidated by reallocation
					POINTER_DATA NewCWPointer = POINTER_CAST(ClippingWindows.data());
					if (NewCWPointer != OldCWPointer)
					{
						win = &ClippingWindows[win_index];
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
						for (unsigned k=0; k<RenderObjects.size(); k++)
						{
							render_object_data &RenderObject = RenderObjects[k];
							if (RenderObject.clipping_windows != NULL)
								RenderObject.clipping_windows = (clipping_window_data *)(NewCWPointer + (POINTER_CAST(RenderObject.clipping_windows) - OldCWPointer));
						}
					}

					// Link into the result list
					const int32 new_index = ClippingWindows.size() - 1;
					if (head == NONE)
						head = new_index;
					else
						ClippingWindows[new_index - 1].next_window = &ClippingWindows.back();
				}
			} // window loop
			
			base_left = base_right;
		} // node loop
		
		if (head != NONE)
			first_window = &ClippingWindows[head];
		
		// Assign bounding y-clips
		// (using bounding y avoids overclipping in some cases when the object intersects a floor/ceiling)
		for (auto* win = first_window; win; win = win->next_window)
		{
			win->top = ClippingWindows[top].top;
			win->bottom = ClippingWindows[bottom].bottom;
			win->y0 = ClippingWindows[top].y0;
			win->y1 = ClippingWindows[bottom].y1;
		}
	}
	
	/* stuff our windows in all objects hanging off our first object (i.e., all parasites) */	
	for (; render_object; render_object= render_object->next_object) render_object->clipping_windows= first_window;
}

bool RenderPlaceObjsClass::add_object_to_sorted_nodes(
	object_data* object,
	_fixed floor_intensity,
	_fixed ceiling_intensity,
	float Opacity)
{
	const auto render_object = build_render_object(object, floor_intensity, ceiling_intensity, Opacity, nullptr, nullptr);
	if (!render_object)
		return false;
	
	const auto span = build_base_node_list(render_object, object->polygon);
	
	build_aggregate_render_object_clipping_window(render_object, span);
	sort_render_object_into_tree(render_object, span); // consumes the render_object list
	
	return true;
}

#define NUMBER_OF_SCALED_VALUES 6

shape_information_data *RenderPlaceObjsClass::rescale_shape_information(
	shape_information_data *unscaled,
	shape_information_data *scaled,
	uint16 flags)
{
	// Idiot-proofing
	if (!unscaled) return NULL;

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


// Creates a fake sprite rectangle from a model's bounding box;
// models behind the view get a degenerate rectangle (and thus just 1 base node)
//
// also finds:
//
// Distance of farthest part of the bounding box
// Reference distance for calculating bounding-box projection
// 
// For player-illumination "Miner's Light" effect:
// Light-position depth (halfway between closest point and bbox centroid)
// Light-position direction
void FindProjectedBoundingBox(GLfloat BoundingBox[2][3],
	long_point3d& TransformedPosition,
	GLfloat Scale,
	short RelativeAngle,
	shape_information_data& ShapeInfo,
	short DepthType,
	int& Farthest,
	int& ProjDistance,
	int& DistanceRef,
	int& LightDepth,
	GLfloat *Direction
)
{
	// Reduce to circle range then find trig values
	short ReducedRA = normalize_angle(RelativeAngle);
	const double TrigMagReciprocal = 1/double(TRIG_MAGNITUDE);
	double Cosine = TrigMagReciprocal*double(cosine_table[ReducedRA]);
	double Sine = TrigMagReciprocal*double(sine_table[ReducedRA]);
	double ScaledCosine = Scale*Cosine;
	double ScaledSine = Scale*Sine;
	
	// Binary representation: (000) (001) (010) (011) (100) (101) (110) (111)
	// where 0 is the first BB vertex and 1 is the second;
	// these are converted into binary numbers for the index
	GLfloat ExpandedBB[8][3];
	
	GLfloat BB00C = (float)ScaledCosine*BoundingBox[0][0];
	GLfloat BB00S = (float)ScaledSine*BoundingBox[0][0];
	GLfloat BB01C = (float)ScaledCosine*BoundingBox[0][1];
	GLfloat BB01S = (float)ScaledSine*BoundingBox[0][1];
	GLfloat BB02 = (float)Scale*BoundingBox[0][2];
	GLfloat BB10C = (float)ScaledCosine*BoundingBox[1][0];
	GLfloat BB10S = (float)ScaledSine*BoundingBox[1][0];
	GLfloat BB11C = (float)ScaledCosine*BoundingBox[1][1];
	GLfloat BB11S = (float)ScaledSine*BoundingBox[1][1];
	GLfloat BB12 = (float)Scale*BoundingBox[1][2];
	
	// 000, 001
	ExpandedBB[0][0] = ExpandedBB[1][0] = BB00C - BB01S;
	ExpandedBB[0][1] = ExpandedBB[1][1] = BB00S + BB01C;
	ExpandedBB[0][2] = BB02;
	ExpandedBB[1][2] = BB12;
	
	// 010, 011
	ExpandedBB[2][0] = ExpandedBB[3][0] = BB00C - BB11S;
	ExpandedBB[2][1] = ExpandedBB[3][1] = BB00S + BB11C;
	ExpandedBB[2][2] = BB02;
	ExpandedBB[3][2] = BB12;
	
	// 100, 101
	ExpandedBB[4][0] = ExpandedBB[5][0] = BB10C - BB01S;
	ExpandedBB[4][1] = ExpandedBB[5][1] = BB10S + BB01C;
	ExpandedBB[4][2] = BB02;
	ExpandedBB[5][2] = BB12;
	
	// 110, 111
	ExpandedBB[6][0] = ExpandedBB[7][0] = BB10C - BB11S;
	ExpandedBB[6][1] = ExpandedBB[7][1] = BB10S + BB11C;
	ExpandedBB[6][2] = BB02;
	ExpandedBB[7][2] = BB12;
	
	// Shift by the object's position
	GLfloat X0 = (float)TransformedPosition.x;
	GLfloat Y0 = (float)TransformedPosition.y;
	GLfloat Z0 = (float)TransformedPosition.z;
	for (int k=0; k<8; k++)
	{
		ExpandedBB[k][0] += X0;
		ExpandedBB[k][1] += Y0;
		ExpandedBB[k][2] += Z0;
	}
	
	// Find minimum and maximum projected Y, Z;
	// scale to the object's position to be compatible
	// with the rest of the code.
	GLfloat XMin, XMax;
	GLfloat Proj_YMin, Proj_YMax, Proj_ZMin, Proj_ZMax;
	
	for (int k=0; k<8; k++)
	{
		GLfloat X = ExpandedBB[k][0];
		GLfloat Y = ExpandedBB[k][1];
		GLfloat Z = ExpandedBB[k][2];
		
		GLfloat XClip = MAX(X,MINIMUM_OBJECT_DISTANCE);
		
		GLfloat Proj = 1/XClip;
		GLfloat Proj_Y = Proj*Y;
		GLfloat Proj_Z = Proj*Z;
		
		if (k == 0)
		{
			XMin = X;
			XMax = X;
			Proj_YMin = Proj_YMax = Proj_Y;
			Proj_ZMin = Proj_ZMax = Proj_Z;
		}
		else
		{
			XMin = MIN(XMin, X);
			XMax = MAX(XMax, X);
			Proj_YMin = MIN(Proj_YMin,Proj_Y);
			Proj_YMax = MAX(Proj_YMax,Proj_Y);
			Proj_ZMin = MIN(Proj_ZMin,Proj_Z);
			Proj_ZMax = MAX(Proj_ZMax,Proj_Z);
		}
	}
	
	// Projected distance of the center point
	ProjDistance = (X0 >= 0) ? int(X0 + 0.5) : - int(-X0 + 0.5);
	
	// Reference distance for projected bounding box
	GLfloat XRef = (DepthType > 0) ? XMax : ((DepthType < 0) ? XMin : X0);
	DistanceRef = int(MAX(XRef,MINIMUM_OBJECT_DISTANCE) + 0.5);
	
	// Moved multiplication by XRef down here,
	// since one needs to find it afterwards
	Proj_YMin *= XRef;
	Proj_YMax *= XRef;
	Proj_ZMin *= XRef;
	Proj_ZMax *= XRef;
	
	// Unshift by the object's position
	Proj_YMin -= Y0;
	Proj_YMax -= Y0;
	Proj_ZMin -= Z0;
	Proj_ZMax -= Z0;
	
	// Plug back into the sprite
	ShapeInfo.world_left = int16(std::clamp<float>(Proj_YMin, INT16_MIN, 0));
	ShapeInfo.world_right = int16(std::clamp<float>(Proj_YMax, 0, INT16_MAX));
	ShapeInfo.world_bottom = int16(std::clamp<float>(Proj_ZMin, INT16_MIN, 0));
	ShapeInfo.world_top = int16(std::clamp<float>(Proj_ZMax, 0, INT16_MAX));
	
	// Set X0, Y0, Z0 to location of center of bounding box
	X0 += (ExpandedBB[0][0] + ExpandedBB[7][0])/2;
	Y0 += (ExpandedBB[0][1] + ExpandedBB[7][1])/2;
	Z0 += (ExpandedBB[0][2] + ExpandedBB[7][2])/2;
	
	// For checking if any of the bounding box will be visible
	Farthest = int(XMax + 0.5);
	
	// Find the depth to use in the miner's light for the object
	LightDepth = MAX(int((XMin + X0)/2 + 0.5),0);
	
	// Find the direction to the object;
	// use the engine's fast square root
	GLfloat DistSq = X0*X0 + Y0*Y0 + Z0*Z0;
	double DistRecip = 1.0/double(isqrt(uint32(DistSq + 0.5)));
	
	// Rotate it to get to the object's internal coordinates
	Direction[0] = (float)(DistRecip*(  X0*Cosine + Y0*Sine));
	Direction[1] = (float)(DistRecip*(- X0*Sine + Y0*Cosine));
	Direction[2] = (float)(DistRecip*Z0);
}
