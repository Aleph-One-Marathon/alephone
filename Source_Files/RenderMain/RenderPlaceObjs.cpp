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


// LP: "recommended" sizes of stuff in growable lists
#define MAXIMUM_RENDER_OBJECTS 72
#define MAXIMUM_OBJECT_BASE_NODES 6

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
			short base_node_count;
			sorted_node_data *base_nodes[MAXIMUM_OBJECT_BASE_NODES];
			
			float Opacity = (object_index == self_index) ? GetChaseCamData().Opacity : 1;
			render_object_data *render_object=
				build_render_object(NULL, floor_intensity, ceiling_intensity,
									base_nodes, &base_node_count,
									get_object_data(object_index),
									Opacity, NULL);
			
			if (render_object)
			{
				build_aggregate_render_object_clipping_window(render_object, base_nodes, base_node_count);
				sort_render_object_into_tree(render_object, base_nodes, base_node_count);
			}
			
			object_index= get_object_data(object_index)->next_object;
		}

		if (graphics_preferences->ephemera_quality != _ephemera_off)
		{
			auto ephemera_index = get_polygon_ephemera(sorted_node->polygon_index);
			while (ephemera_index != NONE)
			{
				short base_node_count;
				sorted_node_data* base_nodes[MAXIMUM_OBJECT_BASE_NODES];
				
				render_object_data* render_object =
					build_render_object(nullptr, floor_intensity, ceiling_intensity, base_nodes, &base_node_count, get_ephemera_data(ephemera_index), 1, nullptr);
				
				if (render_object)
				{
					build_aggregate_render_object_clipping_window(render_object, base_nodes, base_node_count);
					sort_render_object_into_tree(render_object, base_nodes, base_node_count);
				}
				
				ephemera_index = get_ephemera_data(ephemera_index)->next_object;
			}
		}
	}
}


// LP change: make it better able to do long-distance views
render_object_data *RenderPlaceObjsClass::build_render_object(
	long_point3d *origin, // world_point3d *origin,
	_fixed floor_intensity,
	_fixed ceiling_intensity,
	sorted_node_data **base_nodes,
	short *base_node_count,
	object_data* object, float Opacity,
	long_point3d *rel_origin)
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
			
#ifdef HAVE_OPENGL
			// Find which 3D model will take the place of this sprite, if any
			short ModelSequence;
			OGL_ModelData *ModelPtr =
				OGL_GetModelData(GET_COLLECTION(data.collection_code),GET_DESCRIPTOR_SHAPE(object->shape),ModelSequence);
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
			
			/* if the caller wants it, give him the left and right extents of this shape */
			if (base_nodes)
			{
				*base_node_count= build_base_node_list(object->polygon, &object->location,
					shape_information->world_left, shape_information->world_right, base_nodes);
			}
			
			if (ProjDistance == 0)
			{
				x0 = view->half_screen_width;
				x1 = x0 + 1;
				y0 = view->half_screen_height;
				y1 = y0 + 1;
			}
			else
			{
				// Doing this with full-integer arithmetic to avoid mis-clipping;
				x0= view->half_screen_width + (int(transformed_origin.y+shape_information->world_left)*view->world_to_screen_x)/ProjDistance;
				x1= view->half_screen_width + (int(transformed_origin.y+shape_information->world_right)*view->world_to_screen_x)/ProjDistance;
				y0=	view->half_screen_height - (view->world_to_screen_y*int(transformed_origin.z+shape_information->world_top))/ProjDistance + view->dtanpitch;
				y1= view->half_screen_height - (view->world_to_screen_y*int(transformed_origin.z+shape_information->world_bottom))/ProjDistance + view->dtanpitch;
			}
			if (x0<x1 && y0<y1)
			{
				// LP Change:
				size_t Length = RenderObjects.size();
				POINTER_DATA OldROPointer = POINTER_CAST(RenderObjects.data());
				
				// Add a dummy object and check if the pointer got changed
				render_object_data Dummy;
				Dummy.node = NULL;				// Fake initialization to shut up CW
				RenderObjects.push_back(Dummy);
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
				render_object= &RenderObjects[Length];
				
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
#ifdef HAVE_OPENGL
				render_object->rectangle.ModelPtr = ModelPtr;
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
#endif
					
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
					
					parasitic_render_object= build_render_object
						(&parasitic_origin, floor_intensity, ceiling_intensity,
						 NULL, NULL, get_object_data(object->parasitic_object),
						 Opacity, &parasitic_rel_origin);
					
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
	render_object_data *render_object, *last_new_render_object;
	render_object_data *deep_render_object= NULL;
	render_object_data *shallow_render_object= NULL;
	sorted_node_data *desired_node;
	short i;
	// LP: reference to simplify the code
	vector<sorted_node_data>& SortedNodes = RSPtr->SortedNodes;

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
	desired_node= base_nodes[0];
	for (i= 1; i<base_node_count; ++i) if (base_nodes[i]>desired_node) desired_node= base_nodes[i];
	assert((desired_node >= &SortedNodes.front()) && (desired_node <= &SortedNodes.back()));
	
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

enum /* build_base_node_list() states */
{
	_casting_left,
	_casting_right
};

/* we once thought it would be a clever idea to use the transformed endpoints, but, not.  we
	now bail if we can’t find a way out of the polygon we are given; usually this happens
	when we’re moving along gridlines */
short RenderPlaceObjsClass::build_base_node_list(
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
	vector<sorted_node_data *>& polygon_index_to_sorted_node = RSPtr->polygon_index_to_sorted_node;
	
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
				assert(false);
				break;
		}

		vector.i= destination.x - origin->x;
		vector.j= destination.y - origin->y;
		
		/* move toward the given destination accumulating polygon indexes */
		do
		{
			polygon_data *polygon= get_polygon_data(polygon_index);
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

/* ---------- initializing and calculating clip data */

/* find the lowest bottom clip and the highest top clip of all nodes this object crosses.  then
	locate all left and right sides and compile them into one (or several) aggregate windows with
	the same top and bottom */
void RenderPlaceObjsClass::build_aggregate_render_object_clipping_window(
	render_object_data *render_object,
	sorted_node_data **base_nodes,
	short base_node_count)
{
	clipping_window_data *first_window= NULL;
	// LP: references to simplify the code
	vector<clipping_window_data>& ClippingWindows = RVPtr->ClippingWindows;
	vector<sorted_node_data>& SortedNodes = RSPtr->SortedNodes;
	
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
		long_vector2d lvec[MAXIMUM_OBJECT_BASE_NODES], rvec[MAXIMUM_OBJECT_BASE_NODES];
		clipping_window_data *window;
		/* Make sure object depth fits in at least one clipping window */
		int32 win_depth = SHRT_MAX;
		for (i= 0; i<base_node_count; ++i)
		{
			window= base_nodes[i]->clipping_windows;
			if (window)
			{
				win_depth = MIN(win_depth, ABS(window->left.i)+1);
				win_depth = MIN(win_depth, ABS(window->right.i)+1);
			}
		}
		int32 depth= MAX(render_object->rectangle.depth, win_depth);
		
		/* find the upper and lower bounds of the windows; we could do a better job than this by
			doing the same thing we do when the windows are originally built (i.e., calculating a
			new top/bottom for every window.  but screw that.  */
		left_count= right_count= 0;
		y0= SHRT_MAX, y1= SHRT_MIN;
		for (i= 0; i<base_node_count; ++i)
		{
			short j, k;
			
			window= base_nodes[i]->clipping_windows;

			// CB: sometimes, the window pointer seems to be NULL
			if (window == NULL)
				continue;
			
			/* update the top and bottom clipping bounds */
			if (window->y0<y0) y0= window->y0;
			if (window->y1>y1) y1= window->y1;
 			
			if (ABS(window->left.i)<depth || ABS(window->right.i)<depth)
			{
				/* sort in the left side of this window */
				for (j= 0; j<left_count && window->x0>=x0[j]; ++j)
					;
				for (k = left_count - 1; k >= j; --k)
				{
					x0[k+1]= x0[k];
					lvec[k+1]= lvec[k];
				}
				x0[j]= window->x0;
				lvec[j]= window->left;
				left_count+= 1;

				/* sort in the right side of this window */
				for (j= 0; j<right_count && window->x1>=x1[j]; ++j)
					;
				for (k = right_count - 1; k >= j; --k)
				{
					x1[k+1]= x1[k];
					rvec[k+1]= rvec[k];
				}
				x1[j]= window->x1;
				rvec[j]= window->right;
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
					size_t Length = ClippingWindows.size();
					POINTER_DATA OldCWPointer = POINTER_CAST(ClippingWindows.data());
					
					// Add a dummy object and check if the pointer got changed
					clipping_window_data Dummy;
					Dummy.next_window = NULL;			// Fake initialization to shut up CW
					ClippingWindows.push_back(Dummy);
					POINTER_DATA NewCWPointer = POINTER_CAST(ClippingWindows.data());
				
					if (NewCWPointer != OldCWPointer)
					{
						// Get the sorted nodes into sync
						// Also, the render objects and the parent window
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
						if (first_window != NULL)
							first_window = (clipping_window_data *)(NewCWPointer + (POINTER_CAST(first_window) - OldCWPointer));
					}
					window= &ClippingWindows[Length];
					
					/* build it */
					window->x0= x0[left], window->x1= x1[right];
					window->left= lvec[left], window->right= rvec[right];
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
	ShapeInfo.world_left = int(PIN(Proj_YMin,SHRT_MIN,SHRT_MAX));
	ShapeInfo.world_right = int(PIN(Proj_YMax,SHRT_MIN,SHRT_MAX));
	ShapeInfo.world_bottom = int(PIN(Proj_ZMin,SHRT_MIN,SHRT_MAX));
	ShapeInfo.world_top = int(PIN(Proj_ZMax,SHRT_MIN,SHRT_MAX));
	
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
