/*
PROJECTILES.C

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

Friday, May 27, 1994 10:54:44 AM

Friday, July 15, 1994 12:28:36 PM
	added maximum range.
Monday, February 6, 1995 2:46:08 AM  (Jason')
	persistent/virulent projectiles; media detonation effects.
Tuesday, June 13, 1995 12:07:00 PM  (Jason)
	non-melee projectiles must start above media.
Monday, June 26, 1995 8:52:32 AM  (Jason)
	bouncing projectiles
Tuesday, August 1, 1995 3:31:08 PM  (Jason)
	guided projectiles bite on low levels
Thursday, August 17, 1995 9:35:13 AM  (Jason)
	wandering projectiles
Thursday, October 5, 1995 10:19:48 AM  (Jason)
	until we fix it, calling translate_projectile() is too time consuming on high levels.
Friday, October 6, 1995 8:35:04 AM  (Jason)
	simpler guided projectile model.

Feb 4, 2000 (Loren Petrich):
	Added effects of "penetrates media boundary" flag;
	assuming it to be like "penetrates media" flag until I can figure out
	the difference between the two.
	
	Changed halt() to assert(false) for better debugging
	
	Determined that "penetrates media boundary" means
	making a splash but nevertheless continuing

Feb 6, 2000 (Loren Petrich):
	Added access to size of projectile-definition structure.

Feb 9, 2000 (Loren Petrich):
	Put in handling of "penetrates media boundary" flag

Feb 13, 2000 (Loren Petrich):
	Fixed bug in setting will_go_through when hitting a media boundary;
	this banishes the floating-mine effect.

Feb 14, 2000 (Loren Petrich):
	Added workaround for Pfhorte bug: if there is no polygon on the other side
	of a non-solid line, then treat the line as if it was solid.

Feb 16, 2000 (Loren Petrich):
	Improved the handling of "penetrates media boundary" -- if the rocket has that,
	it will explode on the surface, and then afterward on something it hits.

Feb 17, 2000 (Loren Petrich):
	Fixed stuff near GUESS_HYPOTENUSE() to be long-distance-friendly

Feb 19, 2000 (Loren Petrich):
	Added growable lists of indices of objects to be checked for collisions

Jul 1, 2000 (Loren Petrich):
	Added Benad's changes

Aug 30, 2000 (Loren Petrich):
	Added stuff for unpacking and packing
	
Oct 13, 2000 (Loren Petrich)
	Converted the intersected-objects list into a Standard Template Library vector
*/

#include "cseries.h"
#include "map.h"
#include "interface.h"
#include "effects.h"
#include "monsters.h"
#include "projectiles.h"
#include "player.h"
#include "scenery.h"
#include "media.h"
#include "SoundManager.h"
#include "items.h"

// LP additions
#include "dynamic_limits.h"
#include "Packing.h"

#include "lua_script.h"

/*
//translate_projectile() must set _projectile_hit_landscape bit
*/

/* ---------- constants */

enum
{
	GRAVITATIONAL_ACCELERATION= 1, // per tick
	
	WANDER_MAGNITUDE= WORLD_ONE/TICKS_PER_SECOND,
	
	MINIMUM_REBOUND_VELOCITY= GRAVITATIONAL_ACCELERATION*TICKS_PER_SECOND/3
};

enum /* things the projectile can hit in detonate_projectile() */
{
	_hit_nothing,
	_hit_floor,
	_hit_media,
	_hit_ceiling,
	_hit_wall,
	_hit_monster,
	_hit_scenery
};

#define MAXIMUM_PROJECTILE_ELEVATION (QUARTER_CIRCLE/2)

/* ---------- structures */

/* ---------- private prototypes */

/* ---------- globals */

/* import projectile definition structures, constants and globals */
#include "projectile_definitions.h"

/* if copy-protection fails, these are replaced externally with the rocket and the rifle bullet, respectively */
short alien_projectile_override= NONE;
short human_projectile_override= NONE;

// LP addition: growable list of intersected objects
static vector<short> IntersectedObjects;

/* ---------- private prototypes */

static short adjust_projectile_type(world_point3d *origin, short polygon_index, short type,
	short owner_index, short owner_type, short intended_target_index, _fixed damage_scale);

static void update_guided_projectile(short projectile_index);

/*static*/ projectile_definition *get_projectile_definition(
	short type);

/* ---------- code */

projectile_data *get_projectile_data(
	const short projectile_index)
{
	struct projectile_data *projectile =  GetMemberWithBounds(projectiles,projectile_index,MAXIMUM_PROJECTILES_PER_MAP);
	
	vassert(projectile, csprintf(temporary, "projectile index #%d is out of range", projectile_index));
	vassert(SLOT_IS_USED(projectile), csprintf(temporary, "projectile index #%d (%p) is unused", projectile_index, (void*)projectile));
	
	return projectile;
}

// LP change: moved down here to use the projectile definitions
projectile_definition *get_projectile_definition(
	short type)
{
	projectile_definition *definition = GetMemberWithBounds(projectile_definitions,type,NUMBER_OF_PROJECTILE_TYPES);
	vassert(definition, csprintf(temporary, "projectile type #%d is out of range", type));
	
	return definition;
}

/* false means donÕt fire this (itÕs in a floor or ceiling or outside of the map), otherwise
	the monster that was intersected first (or NONE) is returned in target_index */
bool preflight_projectile(
	world_point3d *origin,
	short origin_polygon_index,
	world_point3d *destination,
	angle delta_theta,
	short type,
	short owner,
	short owner_type,
	short *obstruction_index)
{
	bool legal_projectile= false;
	struct projectile_definition *definition= get_projectile_definition(type);
	
	(void) (delta_theta);
	
	/* will be used when we truly preflight projectiles */
	(void) (owner_type);
	
	if (origin_polygon_index!=NONE)
	{
		world_distance dx= destination->x-origin->x, dy= destination->y-origin->y;
		angle elevation= arctangent(isqrt(dx*dx + dy*dy), destination->z-origin->z);
		
		if (elevation<MAXIMUM_PROJECTILE_ELEVATION || elevation>FULL_CIRCLE-MAXIMUM_PROJECTILE_ELEVATION)
		{
			struct polygon_data *origin_polygon= get_polygon_data(origin_polygon_index);
			
			// LP note: "penetrates media boundary" means "hit media surface and continue";
			// it will act like "penetrates_media" here
			// Added idiot-proofing to media data
			media_data *media = get_media_data(origin_polygon->media_index);
			if (origin->z>origin_polygon->floor_height && origin->z<origin_polygon->ceiling_height &&
				(origin_polygon->media_index==NONE || definition->flags&(_penetrates_media) || (media ? origin->z>media->height : true)))
			{
				/* make sure it hits something */
				uint16 flags= translate_projectile(type, origin, origin_polygon_index, destination, (short *) NULL, owner, obstruction_index, 0, true, NONE);
				
				*obstruction_index= (flags&_projectile_hit_monster) ? get_object_data(*obstruction_index)->permutation : NONE;
				legal_projectile= true;
			}
		}
	}
	
	return legal_projectile;
}

// pointless if not an area-of-effect weapon
void detonate_projectile(
	world_point3d *origin,
	short polygon_index,
	short type,
	short owner_index,
	short owner_type,
	_fixed damage_scale)
{
	struct projectile_definition *definition= get_projectile_definition(type);
	struct damage_definition *damage= &definition->damage;
	
	damage->scale= damage_scale;
	damage_monsters_in_radius(NONE, owner_index, owner_type, origin, polygon_index,
		definition->area_of_effect, damage, NONE);
	if (definition->detonation_effect!=NONE) new_effect(origin, polygon_index, definition->detonation_effect, 0);
	L_Call_Projectile_Detonated(type, owner_index, polygon_index, *origin, 0, NONE, NONE);
}

short new_projectile(
	world_point3d *origin,
	short polygon_index,
	world_point3d *_vector,
	angle delta_theta, /* ±¶theta is added (in a circle) to the angle before firing */
	short type,
	short owner_index,
	short owner_type,
	short intended_target_index, /* can be NONE */
	_fixed damage_scale)
{
	struct projectile_definition *definition;
	struct projectile_data *projectile;
	short projectile_index;

	type= adjust_projectile_type(origin, polygon_index, type, owner_index, owner_type, intended_target_index, damage_scale);
	definition= get_projectile_definition(type);

	for (projectile_index= 0, projectile= projectiles; projectile_index<MAXIMUM_PROJECTILES_PER_MAP;
		++projectile_index, ++projectile)
	{
		if (SLOT_IS_FREE(projectile))
		{
			angle facing, elevation;
			short object_index;
			struct object_data *object;

			facing= arctangent(_vector->x, _vector->y);
			elevation= arctangent(isqrt(_vector->x*_vector->x+_vector->y*_vector->y), _vector->z);
			if (delta_theta)
			{
				if (!(definition->flags&_no_horizontal_error)) facing= normalize_angle(facing+global_random()%(2*delta_theta)-delta_theta);
				if (!(definition->flags&_no_vertical_error)) elevation= (definition->flags&_positive_vertical_error) ? normalize_angle(elevation+global_random()%delta_theta) :
					normalize_angle(elevation+global_random()%(2*delta_theta)-delta_theta);
			}
			
			object_index= new_map_object3d(origin, polygon_index, definition->collection==NONE ? NONE : BUILD_DESCRIPTOR(definition->collection, definition->shape), facing);
			if (object_index!=NONE)
			{
				object= get_object_data(object_index);
				
				projectile->type= (definition->flags&_alien_projectile) ?
					(alien_projectile_override==NONE ? type : alien_projectile_override) :
					(human_projectile_override==NONE ? type : human_projectile_override);
				projectile->object_index= object_index;
				projectile->owner_index= owner_index;
				projectile->target_index= intended_target_index;
				projectile->owner_type= owner_type;
				projectile->flags= 0;
				projectile->gravity= 0;
				projectile->ticks_since_last_contrail= projectile->contrail_count= 0;
				projectile->elevation= elevation;
				projectile->distance_travelled= 0;
				projectile->damage_scale= damage_scale;
				MARK_SLOT_AS_USED(projectile);

				SET_OBJECT_OWNER(object, _object_is_projectile);
				object->sound_pitch= definition->sound_pitch;
				L_Call_Projectile_Created(projectile_index);
			}
			else
			{
				projectile_index= NONE;
			}
			
			break;
		}
	}
	if (projectile_index==MAXIMUM_PROJECTILES_PER_MAP) projectile_index= NONE;
	
	return projectile_index;
}

extern void track_contrail_interpolation(int16_t, int16_t);

/* assumes ¶t==1 tick */
void move_projectiles(
	void)
{
	struct projectile_data *projectile;
	short projectile_index;
	
	for (projectile_index=0,projectile=projectiles;projectile_index<MAXIMUM_PROJECTILES_PER_MAP;++projectile_index,++projectile)
	{
		if (SLOT_IS_USED(projectile))
		{
			struct object_data *object= get_object_data(projectile->object_index);

			
//			if (!OBJECT_IS_INVISIBLE(object))
			{
				struct projectile_definition *definition= get_projectile_definition(projectile->type);
				short old_polygon_index= object->polygon;
				world_point3d new_location, old_location;
				short obstruction_index, new_polygon_index;
				short line_index;
				
				new_location= old_location= object->location;
	
				/* update our objectÕs animation */
				animate_object(projectile->object_index);
				
				/* if weÕre supposed to end when our animation loops, check this condition */
				if ((definition->flags&_stop_when_animation_loops) && (GET_OBJECT_ANIMATION_FLAGS(object)&_obj_last_frame_animated))
				{
					remove_projectile(projectile_index);
				}
				else
				{
					world_distance speed= definition->speed;
					uint32 adjusted_definition_flags = 0;
					uint16 flags;
					
					/* base alien projectile speed on difficulty level */
					if (definition->flags&_alien_projectile)
					{
						switch (dynamic_world->game_information.difficulty_level)
						{
							case _wuss_level: speed-= speed>>3; break;
							case _easy_level: speed-= speed>>4; break;
							case _major_damage_level: speed+= speed>>3; break;
							case _total_carnage_level: speed+= speed>>2; break;
						}
					}
	
					/* if this is a guided projectile with a valid target, update guidance system */				
					if ((definition->flags&_guided) && projectile->target_index!=NONE && (dynamic_world->tick_count&1)) update_guided_projectile(projectile_index);

					if (PROJECTILE_HAS_CROSSED_MEDIA_BOUNDARY(projectile)) adjusted_definition_flags= _penetrates_media;
					
					/* move the projectile and check for collisions; if we didnÕt detonate move the
						projectile and check to see if we need to leave a contrail */
					if ((definition->flags&_affected_by_half_gravity) && (dynamic_world->tick_count&1)) projectile->gravity-= GRAVITATIONAL_ACCELERATION;
					if (definition->flags&_affected_by_gravity) projectile->gravity-= GRAVITATIONAL_ACCELERATION;
					if (definition->flags&_doubly_affected_by_gravity) projectile->gravity-= 2*GRAVITATIONAL_ACCELERATION;
					if (film_profile.m1_low_gravity_projectiles && static_world->environment_flags&_environment_low_gravity && static_world->environment_flags&_environment_m1_weapons)
					{
						projectile->gravity /= 2;
					}
					new_location.z+= projectile->gravity;
					translate_point3d(&new_location, speed, object->facing, projectile->elevation);
					if (definition->flags&_vertical_wander) new_location.z+= (global_random()&1) ? WANDER_MAGNITUDE : -WANDER_MAGNITUDE;
					if (definition->flags&_horizontal_wander) translate_point3d(&new_location, (global_random()&1) ? WANDER_MAGNITUDE : -WANDER_MAGNITUDE, NORMALIZE_ANGLE(object->facing+QUARTER_CIRCLE), 0);
					if (film_profile.infinity_smg)
					{
						definition->flags ^= adjusted_definition_flags;
					}
					flags= translate_projectile(projectile->type, &old_location, object->polygon, &new_location, &new_polygon_index, projectile->owner_index, &obstruction_index, &line_index, false, projectile_index);
					if (film_profile.infinity_smg)
					{
						definition->flags ^= adjusted_definition_flags;
					}
					
					// LP change: set up for penetrating media boundary
					bool will_go_through = false;
					
					if (flags&_projectile_hit)
					{
						if ((flags&_projectile_hit_floor) && (definition->flags&_rebounds_from_floor) &&
							projectile->gravity<-MINIMUM_REBOUND_VELOCITY)
						{
							play_object_sound(projectile->object_index, definition->rebound_sound);
							projectile->gravity= - projectile->gravity + (projectile->gravity>>2); /* 0.75 */
						}
						else
						{
 							short monster_obstruction_index= (flags&_projectile_hit_monster) ? get_object_data(obstruction_index)->permutation : NONE;
							bool destroy_persistent_projectile= false;
							
							if (flags&_projectile_hit_scenery) damage_scenery(obstruction_index);
							
							/* cause damage, if we can */
							if (!PROJECTILE_HAS_CAUSED_DAMAGE(projectile))
							{
								struct damage_definition *damage= &definition->damage;
								
								damage->scale= projectile->damage_scale;
								if (definition->flags&_becomes_item_on_detonation)
								{
									if (monster_obstruction_index==NONE)
									{
										struct object_location location;
										
										location.p= object->location, location.p.z= 0;
										location.polygon_index= object->polygon;
										location.yaw= location.pitch= 0;
										location.flags= 0;
										// START Benad
										// Found it!
										// With new_item(), current_item_count[item] increases, but not
										// with try_and_add_player_item(). So reverse the effect of new_item in advance.
										dynamic_world->current_item_count[projectile->permutation]--;
										// END Benad
										new_item(&location, projectile->permutation);
										
										destroy_persistent_projectile= true;
									}
									else
									{
										if(MONSTER_IS_PLAYER(get_monster_data(monster_obstruction_index)))
										{
											short player_obstruction_index= monster_index_to_player_index(monster_obstruction_index);
											destroy_persistent_projectile= try_and_add_player_item(player_obstruction_index, projectile->permutation);
										}
									}
								}
								else
								{
									if (definition->area_of_effect)
									{
										damage_monsters_in_radius(monster_obstruction_index, projectile->owner_index, projectile->owner_type, &old_location, object->polygon, definition->area_of_effect, damage, projectile_index);
									}
									else
									{
										if (monster_obstruction_index!=NONE) damage_monster(monster_obstruction_index, projectile->owner_index, projectile->owner_type, &old_location, damage, projectile_index);
									}
								}
							}
              
							if ((definition->flags&_persistent) && !destroy_persistent_projectile)
							{
								SET_PROJECTILE_DAMAGE_STATUS(projectile, true);
							}
							else
							{
								short detonation_effect= definition->detonation_effect;
								
								if (monster_obstruction_index!=NONE)
								{
									if (definition->flags&_bleeding_projectile)
									{
										detonation_effect= get_monster_impact_effect(monster_obstruction_index);
									}
									if (definition->flags&_melee_projectile)
									{
										short new_detonation_effect= get_monster_melee_impact_effect(monster_obstruction_index);
										if (new_detonation_effect!=NONE) detonation_effect= new_detonation_effect;
									}
								}
								if (flags&_projectile_hit_media)
								{
									get_media_detonation_effect(get_polygon_data(obstruction_index)->media_index, definition->media_detonation_effect, &detonation_effect);
									// LP addition: check if projectile will hit media and continue (PMB flag)
									// set will_go_through for later processing
									if (film_profile.a1_smg)
									{
										// Be careful about parentheses here!
										will_go_through = (definition->flags&_penetrates_media_boundary) != 0;
										// Push the projectile upward or downward, if necessary
										if (will_go_through) {
											if (projectile->elevation == 0) {}
											else if (projectile->elevation < HALF_CIRCLE) new_location.z++;
											else if (projectile->elevation > HALF_CIRCLE) new_location.z--;
										}
									}
								}
								if (film_profile.a1_smg)
								{
									// LP addition: don't detonate if going through media
									// if PMB is set; otherwise, detonate if doing so.
									// Some of the later routines may set both "hit landscape" and "hit media",
									// so be careful.
									if (flags&_projectile_hit_landscape && !(flags&_projectile_hit_media)) detonation_effect= NONE;
								}
								else
								{
									if (flags&_projectile_hit_landscape) detonation_effect = NONE;
								}
								
								if (detonation_effect!=NONE) new_effect(&new_location, new_polygon_index, detonation_effect, object->facing);
								L_Call_Projectile_Detonated(projectile->type, projectile->owner_index, new_polygon_index, new_location, flags, obstruction_index, line_index);
								
								if (!film_profile.infinity_smg || (!(definition->flags&_penetrates_media_boundary) || !(flags&_projectile_hit_media)))
								{
									if ((definition->flags&_persistent_and_virulent) && !destroy_persistent_projectile && monster_obstruction_index!=NONE)
									{
										bool reassign_projectile = true;
										if (film_profile.prevent_dead_projectile_owners)
										{
											monster_data *monster = get_monster_data(monster_obstruction_index);
											reassign_projectile = MONSTER_IS_PLAYER(monster) || !MONSTER_IS_DYING(monster);
										}
										if (reassign_projectile)
											projectile->owner_index= monster_obstruction_index; /* keep going, but donÕt hit this target again */
									}
									// LP addition: don't remove a projectile that will hit media and continue (PMB flag)
									else if (!will_go_through)
									{
										remove_projectile(projectile_index);
									}
								}
								else if (film_profile.infinity_smg)
								{
									SET_PROJECTILE_CROSSED_MEDIA_BOUNDARY_STATUS(projectile, true);
								}
							}
						}
					}
					// Move the projectile if it hit nothing or it will go through media surface
					if (!(flags&_projectile_hit) || will_go_through)
					// else
					{
						/* move to the new_polygon_index */
						translate_map_object(projectile->object_index, &new_location, new_polygon_index);
						
						/* should we leave a contrail at our old location? */
						if ((projectile->ticks_since_last_contrail+=1)>=definition->ticks_between_contrails)
						{
							if (definition->maximum_contrails==NONE || projectile->contrail_count<definition->maximum_contrails)
							{
								projectile->contrail_count+= 1;
								projectile->ticks_since_last_contrail= 0;
								if (definition->contrail_effect!=NONE)
								{
									auto effect_index = new_effect(&old_location, old_polygon_index, definition->contrail_effect, object->facing);
									if (effect_index != NONE && definition->ticks_between_contrails <= 1)
									{
										track_contrail_interpolation(projectile->object_index, get_effect_data(effect_index)->object_index);
									}
								}
							}
						}
		
						if ((flags&_flyby_of_current_player) && !PROJECTILE_HAS_MADE_A_FLYBY(projectile))
						{
							SET_PROJECTILE_FLYBY_STATUS(projectile, true);
							play_object_sound(projectile->object_index, definition->flyby_sound);
						}
		
						/* if we have a maximum range and we have exceeded it then remove the projectile */
						if (definition->maximum_range!=NONE)
						{
							if ((projectile->distance_travelled+= speed)>=definition->maximum_range)
							{
								remove_projectile(projectile_index);
							}
						}
					}
				}
			}
		}
	}
}

void remove_projectile(
	short projectile_index)
{
	struct projectile_data *projectile= get_projectile_data(projectile_index);
	L_Invalidate_Projectile(projectile_index);
	remove_map_object(projectile->object_index);
	MARK_SLOT_AS_FREE(projectile);
}

void remove_all_projectiles(
	void)
{
	struct projectile_data *projectile;
	short projectile_index;
	
	for (projectile_index=0,projectile=projectiles;projectile_index<MAXIMUM_PROJECTILES_PER_MAP;++projectile_index,++projectile)
	{
		if (SLOT_IS_USED(projectile)) remove_projectile(projectile_index);
	}
}

/* when a given monster is deactivated (or killed), all his active projectiles should become
	ownerless (or all sorts of neat little problems can occur) */
void orphan_projectiles(
	short monster_index)
{
	struct projectile_data *projectile;
	short projectile_index;

	/* first, adjust all current projectile's .owner fields */
	for (projectile_index=0,projectile=projectiles;projectile_index<MAXIMUM_PROJECTILES_PER_MAP;++projectile_index,++projectile)
	{
		if (projectile->owner_index==monster_index) projectile->owner_index= NONE;
		if (projectile->target_index==monster_index) projectile->target_index= NONE;
	}
}

void load_projectile_sounds(
	short projectile_type)
{
	if (projectile_type!=NONE)
	{
		struct projectile_definition *definition= get_projectile_definition(projectile_type);
		
		SoundManager::instance()->LoadSound(definition->flyby_sound);
		SoundManager::instance()->LoadSound(definition->rebound_sound);
	}
}

void mark_projectile_collections(
	short projectile_type,
	bool loading)
{
	if (projectile_type!=NONE)
	{
		struct projectile_definition *definition= get_projectile_definition(projectile_type);

		/* If the projectile is not invisible */
		if (definition->collection!=NONE)
		{
			/* mark the projectile collection */
			loading ? mark_collection_for_loading(definition->collection) : mark_collection_for_unloading(definition->collection);
		}
		
		/* mark the projectileÕs effectÕs collection */
		mark_effect_collections(definition->detonation_effect, loading);
		mark_effect_collections(definition->contrail_effect, loading);
	}
}


void drop_the_ball(
	world_point3d *origin,
	short polygon_index,
	short owner_index,
	short owner_type,
	short item_type)
{
	struct world_point3d _vector;
	short projectile_index;

	_vector.x= _vector.y= _vector.z= 0;
	projectile_index= new_projectile(origin, polygon_index, &_vector, 0, _projectile_ball,
		owner_index, owner_type, NONE, FIXED_ONE);
	if (projectile_index!=NONE)
	{
		struct projectile_data *projectile= get_projectile_data(projectile_index);
		struct object_data *object= get_object_data(projectile->object_index);
		
		projectile->permutation= item_type;
		
		object->shape= get_item_shape(item_type);
	}
}

/* ---------- private code */


static short adjust_projectile_type(
	world_point3d *origin,
	short polygon_index,
	short type,
	short owner_index,
	short owner_type,
	short intended_target_index,
	_fixed damage_scale)
{
	struct projectile_definition *definition= get_projectile_definition(type);
	short media_index= get_polygon_data(polygon_index)->media_index;
	
	(void) (owner_index);
	(void) (owner_type);
	(void) (intended_target_index);
	(void) (damage_scale);
	
	if (media_index!=NONE)
	{
		// LP change: idiot-proofing
		media_data *media = get_media_data(media_index);
		if (media)
		{
			if (media->height>origin->z)
			{
				if (definition->media_projectile_promotion!=NONE) type= definition->media_projectile_promotion;
			}
		}
	}
	
	return type;
}
	
#define MAXIMUM_GUIDED_DELTA_YAW 8
#define MAXIMUM_GUIDED_DELTA_PITCH 6

/* changes are at a rate of ±1 angular unit per tick */
static void update_guided_projectile(
	short projectile_index)
{
	struct projectile_data *projectile= get_projectile_data(projectile_index);
	struct monster_data *target= get_monster_data(projectile->target_index);
	struct object_data *projectile_object= get_object_data(projectile->object_index);
	struct object_data *target_object= get_object_data(target->object_index);
	world_distance target_radius, target_height;
	world_point3d target_location;
	
	get_monster_dimensions(projectile->target_index, &target_radius, &target_height);
	target_location= target_object->location;
	target_location.z+= target_height>>1;
	
	switch (target_object->transfer_mode)
	{
		case _xfer_invisibility:
		case _xfer_subtle_invisibility:
			/* canÕt hold lock on invisible targets unless on _total_carnage_level */
			if (dynamic_world->game_information.difficulty_level!=_total_carnage_level) break;
		default:
		{
			// LP change: made this long-distance-friendly
			int32 dx= int32(target_location.x) - int32(projectile_object->location.x);
			int32 dy= int32(target_location.y) - int32(projectile_object->location.y);
			world_distance dz= target_location.z - projectile_object->location.z;
			short delta_yaw= MAXIMUM_GUIDED_DELTA_YAW+_normal_level-dynamic_world->game_information.difficulty_level;
			short delta_pitch= MAXIMUM_GUIDED_DELTA_PITCH+_normal_level-dynamic_world->game_information.difficulty_level;

			if (dx*sine_table[projectile_object->facing] - dy*cosine_table[projectile_object->facing] > 0)
			{
				// turn left
				delta_yaw= -delta_yaw;
			}
			
			dx= ABS(dx), dy= ABS(dy);
			if (GUESS_HYPOTENUSE(dx, dy)*sine_table[projectile->elevation] - dz*cosine_table[projectile->elevation] > 0)
			{
				// turn down
				delta_pitch= -delta_pitch;
			}

			projectile_object->facing= NORMALIZE_ANGLE(projectile_object->facing+delta_yaw);
			projectile->elevation= NORMALIZE_ANGLE(projectile->elevation+delta_pitch);
		}
	}
}

uint16 translate_projectile(
	short type,
	world_point3d *old_location,
	short old_polygon_index,
	world_point3d *new_location,
	short *new_polygon_index,
	short owner_index,
	short *obstruction_index,
	short *last_line_index,
	bool preflight,
	short projectile_index)
{
	struct projectile_definition *definition= get_projectile_definition(type);
	struct polygon_data *old_polygon;
	world_point3d intersection;
	world_distance media_height;
	short line_index;
	size_t intersected_object_count;
	short contact;
	uint16 flags= 0;

	*obstruction_index= NONE;

	contact= _hit_nothing;
	IntersectedObjects.clear();
	old_polygon= get_polygon_data(old_polygon_index);
	if (new_polygon_index) *new_polygon_index= old_polygon_index;
	do
	{
		// LP note: "penetrates media boundary" means "hit media surface and continue";
		// create a hack for enabling a projectile with this flag to continue
		// Idiot-proofing of media handling
		media_data *media = get_media_data(old_polygon->media_index);
		media_height= (old_polygon->media_index==NONE || definition->flags&_penetrates_media || !media) ? INT16_MIN : media->height;
		
		// This flag says it can
		bool traveled_underneath = false;
		if (film_profile.a1_smg)
		{
			traveled_underneath = (definition->flags&_penetrates_media_boundary) && (old_location->z <= media_height);
		}
		
		/* add this polygonÕs monsters to our non-redundant list of possible intersections */
		if (!(definition->flags & _passes_through_objects))
		{
			possible_intersecting_monsters(&IntersectedObjects, GLOBAL_INTERSECTING_MONSTER_BUFFER_SIZE, old_polygon_index, true);
			intersected_object_count = IntersectedObjects.size();
		}
		
 		line_index= find_line_crossed_leaving_polygon(old_polygon_index, (world_point2d *)old_location, (world_point2d *)new_location);
		if (line_index!=NONE)
		{
			/* we crossed a line: if the line is solid, we detonate immediately on the wall,
				otherwise we calculate our Z at the line and compare it to the ceiling and
				floor heights of the old and new polygon to see if we hit a wall between the
				polygons, or the floor or ceiling somewhere in the old polygon */

			struct line_data *line= get_line_data(line_index);

			find_line_intersection(&get_endpoint_data(line->endpoint_indexes[0])->vertex,
				&get_endpoint_data(line->endpoint_indexes[1])->vertex, old_location, new_location,
				&intersection);

			// LP change: workaround for Pfhorte map bug: check to see if there is a polygon
			// on the other side
			short adjacent_polygon_index= find_adjacent_polygon(old_polygon_index, line_index);
			if ((!LINE_IS_SOLID(line) || LINE_HAS_TRANSPARENT_SIDE(line)) && (adjacent_polygon_index != NONE))
			{
				// short adjacent_polygon_index= find_adjacent_polygon(old_polygon_index, line_index);
				
				struct polygon_data *adjacent_polygon= get_polygon_data(adjacent_polygon_index);
				
				// LP change: if PMB is set, then a projectile can travel as if the liquid did not exist,
				// but it will be able to run into a media surface.
				// Here, test for whether the projectile is above the floor or the media surface;
				// ignore the latter test if PMB is set.
				if ((traveled_underneath || intersection.z>media_height) && intersection.z>old_polygon->floor_height)
				{
					// If PMB was set, check to see if the projectile hit the media surface.
					if ((!traveled_underneath || intersection.z<media_height) && intersection.z<old_polygon->ceiling_height)
					{
						if (intersection.z>adjacent_polygon->floor_height&&intersection.z<adjacent_polygon->ceiling_height)
						{
							if (!LINE_HAS_TRANSPARENT_SIDE(line) || line->is_decorative() || (preflight && (definition->flags&(_usually_pass_transparent_side|_sometimes_pass_transparent_side))) ||
								((definition->flags&_usually_pass_transparent_side) && (global_random()&3)) ||
								((definition->flags&_sometimes_pass_transparent_side) && !(global_random()&3)))
							{
								/* no intersections, successfully entered new polygon */
								if (new_polygon_index) *new_polygon_index= adjacent_polygon_index;
								old_polygon_index= adjacent_polygon_index;
								old_polygon= adjacent_polygon;
							}
							else
							{
								/* hit and could not pass transparent texture */
								contact= _hit_wall;
							}
						}
						else
						{
							/* hit wall created by ceiling or floor of new polygon; test to see
								if we toggle a control panel */
							if (!preflight && (definition->flags&_can_toggle_control_panels)) try_and_toggle_control_panel(old_polygon_index, line_index, projectile_index);
							contact= _hit_wall;
						}
					}
					else
					{
						/* hit ceiling of old polygon */
						*obstruction_index= old_polygon_index;
						if (adjacent_polygon->ceiling_transfer_mode==_xfer_landscape) flags|= _projectile_hit_landscape;
						// LP change: if PMB was set, check to see if hit media
						contact= (!traveled_underneath || old_polygon->ceiling_height<media_height) ? _hit_ceiling : _hit_media;
					}
				}
				else
				{
					/* hit floor or media of old polygon */
					*obstruction_index= old_polygon_index;
					if (adjacent_polygon->floor_transfer_mode==_xfer_landscape) flags|= _projectile_hit_landscape;
					// LP change: suppress media-hit test only if PMB was set and the projectile was underneath the media
					contact= (traveled_underneath || old_polygon->floor_height>media_height) ? _hit_floor : _hit_media;
				}
			}
			else
			{
				/* hit wall created by solid line; test to see if we toggle a control panel */
				if (!preflight && (definition->flags&_can_toggle_control_panels)) try_and_toggle_control_panel(old_polygon_index, line_index, projectile_index);
				if (line_is_landscaped(old_polygon_index, line_index, intersection.z)) flags|= _projectile_hit_landscape;
				contact= _hit_wall;
			}
		}
		else
		{
			/* make sure we didnÕt hit the ceiling or floor in this polygon */
			// LP change: if PMB is set, then a projectile can travel as if the liquid did not exist,
			// but it will be able to run into a media surface.
			// Here, test for whether the projectile is above the floor or the media surface;
			// ignore the latter test if PMB is set.
			if ((traveled_underneath || new_location->z>media_height) && new_location->z>old_polygon->floor_height)
			{
				// If PMB was set, check to see if the projectile hit the media surface.
				if ((!traveled_underneath || new_location->z<media_height) && new_location->z<old_polygon->ceiling_height)
				{
					/* weÕre staying in this polygon and weÕre finally done screwing around;
						the caller can look in *new_polygon_index to find out where we ended up */
				}
				else
				{
					/* hit ceiling of current polygon */
					*obstruction_index= old_polygon_index;
					if (old_polygon->ceiling_transfer_mode==_xfer_landscape) flags|= _projectile_hit_landscape;
					// LP change: if PMB was set, check to see if hit media
					contact= (!traveled_underneath || old_polygon->ceiling_height<media_height) ? _hit_ceiling : _hit_media;
				}
			}
			else
			{
				/* hit floor of current polygon */
				*obstruction_index= old_polygon_index;
				if (old_polygon->floor_transfer_mode==_xfer_landscape) flags|= _projectile_hit_landscape;
				// LP change: suppress media-hit test only if PMB was set and the projectile was underneath the media
				contact= (traveled_underneath || old_polygon->floor_height>media_height) ? _hit_floor : _hit_media;
			}
		}
	}
	while (line_index!=NONE&&contact==_hit_nothing);
	
	/* ceilings and floor intersections still donÕt have accurate intersection points, so calculate
		them */
	if (contact!=_hit_nothing)
	{
		switch (contact)
		{
			case _hit_media:
				find_floor_or_ceiling_intersection(media_height, old_location, new_location, &intersection);
				break;
			case _hit_floor:
				find_floor_or_ceiling_intersection(old_polygon->floor_height, old_location, new_location, &intersection);
				break;
			case _hit_ceiling:
				find_floor_or_ceiling_intersection(old_polygon->ceiling_height, old_location, new_location, &intersection);
				break;
		}
		
		/* change new_location to the point of intersection with the ceiling, floor, or wall */
		*new_location= intersection;

	}
	
	/* check our object list and find the best intersection ... if we find an intersection at all,
		then we hit this before we hit the wall, because the object list is checked against the
		clipped new_location. */
	if (!(definition->flags & _passes_through_objects))
	{
		world_distance best_intersection_distance = 0;
		world_distance distance_traveled;
		world_distance best_radius = 0;
		short best_intersection_object = NONE;
		
		distance_traveled= distance2d((world_point2d *)old_location, (world_point2d *)new_location);
		for (size_t i=0;i<intersected_object_count;++i)
		{
			// LP change:
			struct object_data *object= get_object_data(IntersectedObjects[i]);
			int32 separation= point_to_line_segment_distance_squared((world_point2d *)&object->location,
				(world_point2d *)old_location, (world_point2d *)new_location);
			world_distance radius, height;
				
			if (object->permutation!=owner_index) /* donÕt hit ourselves */
			{
				int32 radius_squared;
				
				switch (GET_OBJECT_OWNER(object))
				{
					case _object_is_monster: get_monster_dimensions(object->permutation, &radius, &height); break;
					case _object_is_scenery: get_scenery_dimensions(object->permutation, &radius, &height); break;
					default:
						assert(false);
						break;
				}
				radius_squared= (radius+definition->radius)*(radius+definition->radius);
				
				if (separation<radius_squared) /* if weÕre within radius^2 we passed through this monster */
				{
					world_distance distance= distance2d((world_point2d *)old_location, (world_point2d *)&object->location);
					world_distance projectile_z= distance_traveled ? 
						old_location->z + (distance*(new_location->z-old_location->z))/distance_traveled :
						old_location->z;
					
					if ((height>0 && projectile_z>=object->location.z && projectile_z<=object->location.z+height) ||
						(height<0 && projectile_z>=object->location.z+height && projectile_z<=object->location.z))
					{
						if (best_intersection_object==NONE || distance<best_intersection_distance)
						{
							// LP change:
							best_intersection_object= IntersectedObjects[i];
							best_intersection_distance= distance;
							best_radius= radius;

							switch (GET_OBJECT_OWNER(object))
							{
								case _object_is_monster: contact= _hit_monster; break;
								case _object_is_scenery: contact= _hit_scenery; break;
								default:
									assert(false);
									break;
							}
						}
					}
				}
				else
				{
					if (GET_OBJECT_OWNER(object)==_object_is_monster && separation<12*radius_squared) /* if weÕre within (x*radius)^2 we passed near this monster */
					{
						if (MONSTER_IS_PLAYER(get_monster_data(object->permutation)) && 
							monster_index_to_player_index(object->permutation)==current_player_index)
						{
							flags|= _flyby_of_current_player;
						}
					}
				}
			}
		}
		
		if (best_intersection_object!=NONE) /* if we hit something, take it */
		{
			struct object_data *object= get_object_data(best_intersection_object);
			
			*obstruction_index= best_intersection_object;
			
			if (distance_traveled)
			{
				world_distance actual_distance_to_hit;
				
				actual_distance_to_hit= distance2d((world_point2d *)old_location, (world_point2d *) &object->location);
				actual_distance_to_hit-= best_radius;
				
				new_location->x= old_location->x + (actual_distance_to_hit*(new_location->x-old_location->x))/distance_traveled;
				new_location->y= old_location->y + (actual_distance_to_hit*(new_location->y-old_location->y))/distance_traveled;
				new_location->z= old_location->z + (actual_distance_to_hit*(new_location->z-old_location->z))/distance_traveled;
				
				if (new_polygon_index) *new_polygon_index= find_new_object_polygon((world_point2d *) &object->location,
					(world_point2d *) new_location, object->polygon);
			}
			else
			{
				*new_location= *old_location;
			}
		}
	}

	switch (contact)
	{
		case _hit_monster: flags|= _projectile_hit|_projectile_hit_monster; break;
		case _hit_floor: flags|= _projectile_hit|_projectile_hit_floor; break;
		case _hit_media: flags|= _projectile_hit|_projectile_hit_media; break;
		case _hit_scenery: flags|= _projectile_hit|_projectile_hit_scenery; break;
		case _hit_nothing: break;
		default: flags|= _projectile_hit; break;
	}

	if (last_line_index) *last_line_index = line_index;

	/* returns true if we hit something, false otherwise */
	return flags;
}


// Indicates this feature of some type of projectile
bool ProjectileIsGuided(short Type)
{
	projectile_definition *definition = get_projectile_definition(Type);
	return ((definition->flags&_guided) != 0);
}


uint8 *unpack_projectile_data(uint8 *Stream, projectile_data* Objects, size_t Count)
{
	uint8* S = Stream;
	projectile_data* ObjPtr = Objects;
	
	for (size_t k = 0; k < Count; k++, ObjPtr++)
	{
		StreamToValue(S,ObjPtr->type);
		
		StreamToValue(S,ObjPtr->object_index);
		
		StreamToValue(S,ObjPtr->target_index);
		
		StreamToValue(S,ObjPtr->elevation);
		
		StreamToValue(S,ObjPtr->owner_index);
		StreamToValue(S,ObjPtr->owner_type);
		StreamToValue(S,ObjPtr->flags);
		
		StreamToValue(S,ObjPtr->ticks_since_last_contrail);
		StreamToValue(S,ObjPtr->contrail_count);
		
		StreamToValue(S,ObjPtr->distance_travelled);
		
		StreamToValue(S,ObjPtr->gravity);
		
		StreamToValue(S,ObjPtr->damage_scale);
		
		StreamToValue(S,ObjPtr->permutation);
		
		S += 2*2;
	}
	
	assert((S - Stream) == static_cast<ptrdiff_t>(Count*SIZEOF_projectile_data));
	return S;
}

uint8 *pack_projectile_data(uint8 *Stream, projectile_data* Objects, size_t Count)
{
	uint8* S = Stream;
	projectile_data* ObjPtr = Objects;
	
	for (size_t k = 0; k < Count; k++, ObjPtr++)
	{
		ValueToStream(S,ObjPtr->type);
		
		ValueToStream(S,ObjPtr->object_index);
		
		ValueToStream(S,ObjPtr->target_index);
		
		ValueToStream(S,ObjPtr->elevation);
		
		ValueToStream(S,ObjPtr->owner_index);
		ValueToStream(S,ObjPtr->owner_type);
		ValueToStream(S,ObjPtr->flags);
		
		ValueToStream(S,ObjPtr->ticks_since_last_contrail);
		ValueToStream(S,ObjPtr->contrail_count);
		
		ValueToStream(S,ObjPtr->distance_travelled);
		
		ValueToStream(S,ObjPtr->gravity);
		
		ValueToStream(S,ObjPtr->damage_scale);
		
		ValueToStream(S,ObjPtr->permutation);
		
		S += 2*2;
	}
	
	assert((S - Stream) == static_cast<ptrdiff_t>(Count*SIZEOF_projectile_data));
	return S;
}


uint8 *unpack_projectile_definition(uint8 *Stream, projectile_definition *Objects, size_t Count)
{
	uint8* S = Stream;
	projectile_definition* ObjPtr = Objects;
	
	for (size_t k = 0; k < Count; k++, ObjPtr++)
	{
		StreamToValue(S,ObjPtr->collection);
		StreamToValue(S,ObjPtr->shape);
		StreamToValue(S,ObjPtr->detonation_effect);
		StreamToValue(S,ObjPtr->media_detonation_effect);
		StreamToValue(S,ObjPtr->contrail_effect);
		StreamToValue(S,ObjPtr->ticks_between_contrails);
		StreamToValue(S,ObjPtr->maximum_contrails);
		StreamToValue(S,ObjPtr->media_projectile_promotion);
		
		StreamToValue(S,ObjPtr->radius);
		StreamToValue(S,ObjPtr->area_of_effect);
		S = unpack_damage_definition(S,&ObjPtr->damage,1);
		
		StreamToValue(S,ObjPtr->flags);
		
		StreamToValue(S,ObjPtr->speed);
		StreamToValue(S,ObjPtr->maximum_range);
		
		StreamToValue(S,ObjPtr->sound_pitch);
		StreamToValue(S,ObjPtr->flyby_sound);
		StreamToValue(S,ObjPtr->rebound_sound);
	}
	
	assert((S - Stream) == static_cast<ptrdiff_t>(Count*SIZEOF_projectile_definition));
	return S;
}

uint8 *unpack_projectile_definition(uint8 *Stream, size_t Count)
{
	return unpack_projectile_definition(Stream,projectile_definitions,Count);
}

uint8* unpack_m1_projectile_definition(uint8* Stream, size_t Count)
{
	uint8* S = Stream;
	projectile_definition* ObjPtr = projectile_definitions;

	for (size_t k = 0; k < Count; k++, ObjPtr++)
	{
		StreamToValue(S,ObjPtr->collection);
		StreamToValue(S,ObjPtr->shape);
		StreamToValue(S,ObjPtr->detonation_effect);
		ObjPtr->media_detonation_effect = NONE;
		StreamToValue(S,ObjPtr->contrail_effect);
		StreamToValue(S,ObjPtr->ticks_between_contrails);
		StreamToValue(S,ObjPtr->maximum_contrails);
		ObjPtr->media_projectile_promotion = 0;

		StreamToValue(S,ObjPtr->radius);
		StreamToValue(S,ObjPtr->area_of_effect);
		S = unpack_damage_definition(S, &ObjPtr->damage, 1);

		uint16 flags;
		StreamToValue(S, flags);
		ObjPtr->flags = flags;
		
		StreamToValue(S,ObjPtr->speed);
		StreamToValue(S,ObjPtr->maximum_range);

		ObjPtr->sound_pitch = FIXED_ONE;
		StreamToValue(S,ObjPtr->flyby_sound);
		ObjPtr->rebound_sound = NONE;

		if (ObjPtr->damage.type == _damage_projectile)
		{
			ObjPtr->flags |= _bleeding_projectile;
		}
	}

	return S;
}


uint8 *pack_projectile_definition(uint8 *Stream, projectile_definition *Objects, size_t Count)
{
	uint8* S = Stream;
	projectile_definition* ObjPtr = Objects;
	
	for (size_t k = 0; k < Count; k++, ObjPtr++)
	{
		ValueToStream(S,ObjPtr->collection);
		ValueToStream(S,ObjPtr->shape);
		ValueToStream(S,ObjPtr->detonation_effect);
		ValueToStream(S,ObjPtr->media_detonation_effect);
		ValueToStream(S,ObjPtr->contrail_effect);
		ValueToStream(S,ObjPtr->ticks_between_contrails);
		ValueToStream(S,ObjPtr->maximum_contrails);
		ValueToStream(S,ObjPtr->media_projectile_promotion);
		
		ValueToStream(S,ObjPtr->radius);
		ValueToStream(S,ObjPtr->area_of_effect);
		S = pack_damage_definition(S,&ObjPtr->damage,1);
		
		ValueToStream(S,ObjPtr->flags);
		
		ValueToStream(S,ObjPtr->speed);
		ValueToStream(S,ObjPtr->maximum_range);
		
		ValueToStream(S,ObjPtr->sound_pitch);
		ValueToStream(S,ObjPtr->flyby_sound);
		ValueToStream(S,ObjPtr->rebound_sound);
	}
	
	assert((S - Stream) == static_cast<ptrdiff_t>(Count*SIZEOF_projectile_definition));
	return S;
}

uint8 *pack_projectile_definition(uint8 *Stream, size_t Count)
{
	return pack_projectile_definition(Stream,projectile_definitions,Count);
}

void init_projectile_definitions()
{
	memcpy(projectile_definitions, original_projectile_definitions, sizeof(projectile_definitions));
}
