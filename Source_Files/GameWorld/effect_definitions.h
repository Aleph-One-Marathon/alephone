#ifndef __EFFECT_DEFINITIONS_H
#define __EFFECT_DEFINITIONS_H

/*
EFFECT_DEFINITIONS.H

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

Tuesday, May 31, 1994 5:21:28 PM

Feb 3, 2000 (Loren Petrich):
	Added Jjaro-texture effects
	Added VacBob effects
*/

#include "effects.h"
#include "map.h"
#include "SoundManagerEnums.h"

/* ---------- constants */

enum /* flags */
{
	_end_when_animation_loops= 0x0001,
	_end_when_transfer_animation_loops= 0x0002,
	_sound_only= 0x0004, /* play the animation’s initial sound and nothing else */
	_make_twin_visible= 0x0008,
	_media_effect= 0x0010
};

/* ---------- structures */

struct effect_definition
{
	int16 collection, shape;

	_fixed sound_pitch;
	
	uint16 flags;
	int16 delay, delay_sound;
};

/* ---------- effect definitions */

static struct effect_definition effect_definitions[NUMBER_OF_EFFECT_TYPES];
const struct effect_definition original_effect_definitions[NUMBER_OF_EFFECT_TYPES]=
{
	/* rocket explosion, contrail */
	{_collection_rocket, 1, _normal_frequency, _end_when_animation_loops, 0, NONE},
	{_collection_rocket, 2, _normal_frequency, _end_when_animation_loops, 0, NONE},

	/* grenade explosion, contrail */
	{_collection_rocket, 9, _normal_frequency, _end_when_animation_loops, 0, NONE},
	{_collection_rocket, 4, _normal_frequency, _end_when_animation_loops, 0, NONE},

	/* bullet ricochet */
	{_collection_rocket, 13, _normal_frequency, _end_when_animation_loops, 0, NONE},

	/* _effect_alien_weapon_ricochet */
	{_collection_rocket, 5, _normal_frequency, _end_when_animation_loops, 0, NONE},

	/* flame thrower burst */
	{_collection_rocket, 6, _normal_frequency, _end_when_animation_loops, 0, NONE},
	
	/* fighter blood splash */
	{_collection_fighter, 8, _normal_frequency, _end_when_animation_loops, 0, NONE},
	
	/* player blood splash */
	{_collection_rocket, 10, _normal_frequency, _end_when_animation_loops, 0, NONE},
	
	/* civilian blood splash, assimilated civilian blood splash */
	{_collection_civilian, 7, _normal_frequency, _end_when_animation_loops, 0, NONE},
	{BUILD_COLLECTION(_collection_civilian, 3), 12, _normal_frequency, _end_when_animation_loops, 0, NONE},
	
	/* enforcer blood splash */
	{_collection_enforcer, 5, _normal_frequency, _end_when_animation_loops, 0, NONE},
	
	/* _effect_compiler_bolt_minor_detonation, _effect_compiler_bolt_major_detonation,
		_effect_compiler_bolt_major_contrail */
	{_collection_compiler, 6, _normal_frequency, _end_when_animation_loops, 0, NONE},
	{BUILD_COLLECTION(_collection_compiler, 1), 6, _normal_frequency, _end_when_animation_loops, 0, NONE},
	{BUILD_COLLECTION(_collection_compiler, 1), 5, _normal_frequency, _end_when_animation_loops, 0, NONE},
	
	/* _effect_fighter_projectile_detonation, _effect_fighter_melee_detonation */
	{BUILD_COLLECTION(_collection_fighter, 0), 10, _normal_frequency, _end_when_animation_loops, 0, NONE},
	{BUILD_COLLECTION(_collection_fighter, 0), 11, _normal_frequency, _sound_only, 0, NONE},
	
	/* _effect_hunter_projectile_detonation, _effect_hunter_spark */
	{BUILD_COLLECTION(_collection_hunter, 0), 4, _normal_frequency, _end_when_animation_loops, 0, NONE},
	{BUILD_COLLECTION(_collection_hunter, 0), 8, _normal_frequency, _end_when_animation_loops, 0, NONE},
	
	/* _effect_minor_fusion_detonation, _effect_major_fusion_detonation, _effect_major_fusion_contrail */
	{BUILD_COLLECTION(_collection_rocket, 0), 14, _normal_frequency, _end_when_animation_loops, 0, NONE},
	{BUILD_COLLECTION(_collection_rocket, 0), 15, _higher_frequency, _end_when_animation_loops, 0, NONE},
	{BUILD_COLLECTION(_collection_rocket, 0), 16, _higher_frequency, _end_when_animation_loops, 0, NONE},
	
	/* _effect_fist_detonation */
	{_collection_rocket, 17, _normal_frequency, _sound_only, 0, NONE},

	/* _effect_minor_defender_detonation, _effect_major_defender_detonation, _effect_defender_spark */
	{BUILD_COLLECTION(_collection_defender, 0), 5, _normal_frequency, _end_when_animation_loops, 0, NONE},
	{BUILD_COLLECTION(_collection_defender, 1), 5, _normal_frequency, _end_when_animation_loops, 0, NONE},
	{BUILD_COLLECTION(_collection_defender, 0), 7, _normal_frequency, _end_when_animation_loops, 0, NONE},
	
	/* _effect_trooper_blood_splash */
	{_collection_trooper, 8, _normal_frequency, _end_when_animation_loops, 0, NONE},
	
	/* _effect_lamp_breaking */
	{_collection_scenery1, 22, _normal_frequency, _end_when_animation_loops, 0, NONE},
	{_collection_scenery2, 18, _normal_frequency, _end_when_animation_loops, 0, NONE},
	{_collection_scenery3, 16, _normal_frequency, _end_when_animation_loops, 0, NONE},
	{_collection_scenery5, 8, _normal_frequency, _end_when_animation_loops, 0, NONE},
	
	/* _effect_metallic_clang */
	{_collection_rocket, 23, _normal_frequency, _sound_only, 0, NONE},
	
	/* _effect_teleport_in, _effect_teleport_out */
	{_collection_items, 0, _normal_frequency, _end_when_transfer_animation_loops|_make_twin_visible, TICKS_PER_SECOND, _snd_teleport_in},
	{_collection_items, 0, _normal_frequency, _end_when_transfer_animation_loops, 0, NONE},
	
	/* _effect_small_water_splash, _effect_medium_water_splash, _effect_large_water_splash, _effect_large_water_emergence */
	{_collection_scenery1, 0, _normal_frequency, _end_when_animation_loops|_media_effect, 0, NONE},
	{_collection_scenery1, 1, _normal_frequency, _end_when_animation_loops|_media_effect, 0, NONE},
	{_collection_scenery1, 2, _normal_frequency, _end_when_animation_loops|_sound_only, NONE},
	{_collection_scenery1, 3, _normal_frequency, _end_when_animation_loops|_sound_only, NONE},
	
	/* _effect_small_lava_splash, _effect_medium_lava_splash, _effect_large_lava_splash, _effect_large_lava_emergence */
	{_collection_scenery2, 0, _normal_frequency, _end_when_animation_loops|_media_effect, 0, NONE},
	{_collection_scenery2, 1, _normal_frequency, _end_when_animation_loops|_media_effect, 0, NONE},
	{_collection_scenery2, 2, _normal_frequency, _end_when_animation_loops|_sound_only, 0, NONE},
	{_collection_scenery2, 17, _normal_frequency, _end_when_animation_loops|_sound_only, 0, NONE},

	/* _effect_small_sewage_splash, _effect_medium_sewage_splash, _effect_large_sewage_splash, _effect_large_sewage_emergence */
	{_collection_scenery3, 0, _normal_frequency, _end_when_animation_loops|_media_effect, 0, NONE},
	{_collection_scenery3, 1, _normal_frequency, _end_when_animation_loops|_media_effect, 0, NONE},
	{_collection_scenery3, 2, _normal_frequency, _end_when_animation_loops|_sound_only, 0, NONE},
	{_collection_scenery3, 3, _normal_frequency, _end_when_animation_loops|_sound_only, 0, NONE},

	/* _effect_small_goo_splash, _effect_medium_goo_splash, _effect_large_goo_splash, _effect_large_goo_emergence */
	{_collection_scenery5, 0, _normal_frequency, _end_when_animation_loops|_media_effect, 0, NONE},
	{_collection_scenery5, 1, _normal_frequency, _end_when_animation_loops|_media_effect, 0, NONE},
	{_collection_scenery5, 2, _normal_frequency, _end_when_animation_loops|_sound_only, 0, NONE},
	{_collection_scenery5, 3, _normal_frequency, _end_when_animation_loops|_sound_only, 0, NONE},
	
	/* _effect_minor_hummer_projectile_detonation, _effect_major_hummer_projectile_detonation,
		_effect_durandal_hummer_projectile_detonation, _effect_hummer_spark */
	{BUILD_COLLECTION(_collection_hummer, 0), 6, _normal_frequency, _end_when_animation_loops, 0, NONE},
	{BUILD_COLLECTION(_collection_hummer, 1), 6, _higher_frequency, _end_when_animation_loops, 0, NONE},
	{BUILD_COLLECTION(_collection_hummer, 4), 6, _lower_frequency, _end_when_animation_loops, 0, NONE},
	{BUILD_COLLECTION(_collection_hummer, 0), 7, _normal_frequency, _end_when_animation_loops, 0, NONE},
	
	/* _effect_cyborg_projectile_detonation, _effect_cyborg_blood_splash */
	{_collection_cyborg, 7, _normal_frequency, _end_when_animation_loops, 0, NONE},
	{_collection_cyborg, 8, _normal_frequency, _end_when_animation_loops, 0, NONE},

	/* 	_effect_minor_fusion_dispersal, _effect_major_fusion_dispersal, _effect_overloaded_fusion_dispersal */
	{BUILD_COLLECTION(_collection_rocket, 0), 19, _normal_frequency, _end_when_animation_loops|_sound_only, 0, NONE},
	{BUILD_COLLECTION(_collection_rocket, 0), 20, _higher_frequency, _end_when_animation_loops|_sound_only, 0, NONE},
	{BUILD_COLLECTION(_collection_rocket, 0), 21, _lower_frequency, _end_when_animation_loops|_sound_only, 0, NONE},

	/* _effect_sewage_yeti_blood_splash, _effect_sewage_yeti_projectile_detonation */
	{BUILD_COLLECTION(_collection_yeti, 0), 5, _normal_frequency, _end_when_animation_loops, 0, NONE},
	{BUILD_COLLECTION(_collection_yeti, 0), 11, _normal_frequency, _end_when_animation_loops, 0, NONE},
	
	/* _effect_water_yeti_blood_splash */
	{BUILD_COLLECTION(_collection_yeti, 1), 5, _normal_frequency, _end_when_animation_loops, 0, NONE},
	
	/* _effect_lava_yeti_blood_splash, _effect_lava_yeti_projectile_detonation */
	{BUILD_COLLECTION(_collection_yeti, 2), 5, _normal_frequency, _end_when_animation_loops, 0, NONE},
	{BUILD_COLLECTION(_collection_yeti, 2), 7, _normal_frequency, _end_when_animation_loops, 0, NONE},

	/* _effect_yeti_melee_detonation */
	{_collection_yeti, 8, _normal_frequency, _sound_only, 0, NONE},
	
	/* _effect_juggernaut_spark, _effect_juggernaut_missile_contrail */
	{_collection_juggernaut, 3, _normal_frequency, _end_when_animation_loops, 0, NONE},
	{_collection_rocket, 24, _normal_frequency, _end_when_animation_loops, 0, NONE},

	// LP addition: Jjaro stuff and VacBobs
	/* _effect_small_jjaro_splash, _effect_medium_jjaro_splash, _effect_large_jjaro_splash, _effect_large_jjaro_emergence */
	{_collection_scenery4, 0, _normal_frequency, _end_when_animation_loops|_media_effect, 0, NONE},
	{_collection_scenery4, 1, _normal_frequency, _end_when_animation_loops|_media_effect, 0, NONE},
	{_collection_scenery4, 2, _normal_frequency, _end_when_animation_loops|_sound_only, 0, NONE},
	{_collection_scenery4, 3, _normal_frequency, _end_when_animation_loops|_sound_only, 0, NONE},
	
	/* _effect_civilian_fusion_blood_splash, _effect_assimilated_civilian_fusion_blood_splash */
	{_collection_civilian_fusion, 7, _normal_frequency, _end_when_animation_loops, 0, NONE},
	{BUILD_COLLECTION(_collection_civilian_fusion, 3), 12, _normal_frequency, _end_when_animation_loops, 0, NONE},
};

// Added for the convenience of the 1-2-3 Converter
uint8 *unpack_effect_definition(uint8 *Stream, effect_definition *Objects, size_t Count);
uint8 *pack_effect_definition(uint8 *Stream, effect_definition *Objects, size_t Count);

#endif
