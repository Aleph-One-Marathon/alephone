/*
EFFECTS.C
Friday, May 27, 1994 10:40:13 AM

Saturday, May 28, 1994 2:13:08 AM
	hopefully effects can be used for sparks.  most effects disappear when their animations
	terminate.
Friday, September 30, 1994 5:48:25 PM (Jason)
	hopefully.  ha.  added sound-only effects.
Wednesday, February 1, 1995 12:58:17 AM  (Jason')
	teleporting item effects.

Feb 6, 2000 (Loren Petrich):
	Added access to size of effect-definition structure
*/

#include "cseries.h"
#include "map.h"
#include "interface.h"
#include "effects.h"
#include "mysound.h"

#ifdef env68k
#pragma segment objects
#endif

/*
ryan reports get_object_data() failing on effect->data after a teleport effect terminates
*/

/* ---------- macros */

/* ---------- structures */

/* ---------- private prototypes */

/*
#ifdef DEBUG
struct effect_definition *get_effect_definition(short type);
#else
#define get_effect_definition(i) (effect_definitions+(i))
#endif
*/

/* ---------- globals */

/* import effect definition constants, structures and globals */
#include "effect_definitions.h"

struct effect_data *effects;

/* ---------- code */

// LP change: moved down here because it refers to effect definitions
inline struct effect_definition *get_effect_definition(const short type)
{
	return GetMemberWithBounds(effect_definitions,type,NUMBER_OF_EFFECT_TYPES);
}


short new_effect(
	world_point3d *origin,
	short polygon_index,
	short type,
	angle facing)
{
	short effect_index= NONE;

	if (polygon_index!=NONE)
	{
		struct effect_data *effect;
		struct effect_definition *definition;
	
		definition= get_effect_definition(type);
		// LP change: idiot-proofing
		if (!definition) return NONE;
		
		if (definition->flags&_sound_only)
		{
			struct shape_animation_data *animation= get_shape_animation_data(BUILD_DESCRIPTOR(definition->collection, definition->shape));
			
			play_world_sound(polygon_index, origin, animation->first_frame_sound);
		}
		else
		{
			for (effect_index= 0,effect= effects; effect_index<MAXIMUM_EFFECTS_PER_MAP; ++effect_index, ++effect)
			{
				if (SLOT_IS_FREE(effect))
				{
					short object_index= new_map_object3d(origin, polygon_index, BUILD_DESCRIPTOR(definition->collection, definition->shape), facing);
					
					if (object_index!=NONE)
					{
						struct object_data *object= get_object_data(object_index);
						
						effect->type= type;
						effect->flags= 0;
						effect->object_index= object_index;
						effect->data= 0;
						effect->delay= definition->delay ? global_random()%definition->delay : 0;
						MARK_SLOT_AS_USED(effect);
						
						SET_OBJECT_OWNER(object, _object_is_effect);
						object->sound_pitch= definition->sound_pitch;
						if (effect->delay) SET_OBJECT_INVISIBILITY(object, true);
						if (definition->flags&_media_effect) SET_OBJECT_IS_MEDIA_EFFECT(object);
					}
					else
					{
						effect_index= NONE;
					}
					
					break;
				}
			}
			if (effect_index==MAXIMUM_EFFECTS_PER_MAP) effect_index= NONE;
		}
	}
	
	return effect_index;
}

/* assumes ∂t==1 tick */
void update_effects(
	void)
{
	struct effect_data *effect;
	short effect_index;
	
	for (effect_index= 0, effect= effects; effect_index<MAXIMUM_EFFECTS_PER_MAP; ++effect_index, ++effect)
	{
		if (SLOT_IS_USED(effect))
		{
			struct object_data *object= get_object_data(effect->object_index);
			struct effect_definition *definition= get_effect_definition(effect->type);
			// LP change: idiot-proofing
			if (!definition) continue;
			
			if (effect->delay)
			{
				/* handle invisible, delayed effects */
				if (!(effect->delay-= 1))
				{
					SET_OBJECT_INVISIBILITY(object, false);
					play_object_sound(effect->object_index, definition->delay_sound);
				}
			}
			else
			{
				/* update our object’s animation */
				animate_object(effect->object_index);
				
				/* if the effect’s animation has terminated and we’re supposed to deactive it, do so */
				if (((GET_OBJECT_ANIMATION_FLAGS(object)&_obj_last_frame_animated)&&(definition->flags&_end_when_animation_loops)) ||
					((GET_OBJECT_ANIMATION_FLAGS(object)&_obj_transfer_mode_finished)&&(definition->flags&_end_when_transfer_animation_loops)))
				{
					remove_effect(effect_index);
					
					/* if we’re supposed to make another item visible, do so */
					if (definition->flags&_make_twin_visible)
					{
						struct object_data *object= get_object_data(effect->data);
						
						SET_OBJECT_INVISIBILITY(object, false);
					}
				}
			}
		}
	}
	
	return;
}

void remove_effect(
	short effect_index)
{
	struct effect_data *effect;
	
	effect= get_effect_data(effect_index);
	remove_map_object(effect->object_index);
	MARK_SLOT_AS_FREE(effect);
	
	return;
}

void remove_all_nonpersistent_effects(
	void)
{
	struct effect_data *effect;
	short effect_index;
	
	for (effect_index= 0, effect= effects; effect_index<MAXIMUM_EFFECTS_PER_MAP; ++effect_index, ++effect)
	{
		if (SLOT_IS_USED(effect))
		{
			struct effect_definition *definition= get_effect_definition(effect->type);
			// LP change: idiot-proofing
			if (!definition) continue;

			if (definition->flags&(_end_when_animation_loops|_end_when_transfer_animation_loops))
			{
				remove_effect(effect_index);
			}
		}
	}
	
	return;
}

void mark_effect_collections(
	short effect_type,
	bool loading)
{
	if (effect_type!=NONE)
	{
		struct effect_definition *definition= get_effect_definition(effect_type);
		// LP change: idiot-proofing
		if (!definition) return;

		/* mark the effect collection */
		loading ? mark_collection_for_loading(definition->collection) : mark_collection_for_unloading(definition->collection);
	}
	
	return;
}

void teleport_object_out(
	short object_index)
{
	struct object_data *object= get_object_data(object_index);
	
	if (!OBJECT_IS_INVISIBLE(object))
	{
		short effect_index= new_effect(&object->location, object->polygon, _effect_teleport_object_out, object->facing);
		
		if (effect_index!=NONE)
		{
			struct effect_data *effect= get_effect_data(effect_index);
			struct object_data *effect_object= get_object_data(effect->object_index);
			
			// make the effect look like the object
			effect_object->shape= object->shape;
			effect_object->sequence= object->sequence;
			effect_object->transfer_mode= _xfer_fold_out;
			effect_object->transfer_period= TELEPORTING_MIDPOINT;
			effect_object->transfer_phase= 0;
			effect_object->flags|= object->flags&(_object_is_enlarged|_object_is_tiny);
			
			// make the object invisible
			SET_OBJECT_INVISIBILITY(object, true);

			play_object_sound(effect->object_index, _snd_teleport_out); /* teleport in sound, at destination */
		}
	}
	
	return;
}

// if the given object isn’t already teleporting in, do so
void teleport_object_in(
	short object_index)
{
	struct effect_data *effect;
	short effect_index;

	for (effect_index= 0, effect= effects; effect_index<MAXIMUM_EFFECTS_PER_MAP; ++effect_index, ++effect)
	{
		if (SLOT_IS_USED(effect))
		{
			if (effect->type==_effect_teleport_object_in && effect->data==object_index)
			{
				object_index= NONE;
				break;
			}
		}
	}
	
	if (object_index!=NONE)
	{
		struct object_data *object= get_object_data(object_index);

		effect_index= new_effect(&object->location, object->polygon, _effect_teleport_object_in, object->facing);
		if (effect_index!=NONE)
		{
			struct object_data *effect_object;
			
			effect= get_effect_data(effect_index);
			effect->data= object_index;
			
			effect_object= get_object_data(effect->object_index);
			effect_object->shape= object->shape;
			effect_object->transfer_mode= _xfer_fold_in;
			effect_object->transfer_period= TELEPORTING_MIDPOINT;
			effect_object->transfer_phase= 0;
			effect_object->flags|= object->flags&(_object_is_enlarged|_object_is_tiny);
		}
	}
	
	return;
}

/*
#ifdef DEBUG
struct effect_data *get_effect_data(
	short effect_index)
{
	struct effect_data *effect;
	
	vassert(effect_index>=0&&effect_index<MAXIMUM_EFFECTS_PER_MAP, csprintf(temporary, "effect index #%d is out of range", effect_index));
	
	effect= effects+effect_index;
	vassert(SLOT_IS_USED(effect), csprintf(temporary, "effect index #%d (%p) is unused", effect_index, effect));
	
	return effect;
}
#endif
*/

/* ---------- private code */

/*
#ifdef DEBUG
struct effect_definition *get_effect_definition(
	short type)
{
	assert(type>=0&&type<NUMBER_OF_EFFECT_TYPES);
	
	return effect_definitions+type;
}
#endif
*/

// LP addition: Get effect-definition size
int get_effect_defintion_size() {return sizeof(struct effect_definition);}
