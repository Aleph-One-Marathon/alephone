#ifndef __EFFECTS_H
#define __EFFECTS_H

/*
EFFECTS.H

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

Saturday, June 18, 1994 10:44:10 PM

Feb 3, 2000 (Loren Petrich):
	Added Jjaro-texture effects
	Added VacBob effects

Feb 6, 2000 (Loren Petrich):
	Added access to size of effect-definition structure

Feb 10, 2000 (Loren Petrich):
	Added dynamic-limits setting of MAXIMUM_EFECTS_PER_MAP
	
Jul 1, 2000 (Loren Petrich):
	Made effects accessor an inline function

Aug 30, 2000 (Loren Petrich):
	Added stuff for unpacking and packing
*/

// LP addition:
#include "dynamic_limits.h"

#include "world.h"
#include <vector>

/* ---------- effect structure */

enum /* effect types */
{
	_effect_rocket_explosion,
	_effect_rocket_contrail,
	_effect_grenade_explosion,
	_effect_grenade_contrail,
	_effect_bullet_ricochet,
	_effect_alien_weapon_ricochet,
	_effect_flamethrower_burst,
	_effect_fighter_blood_splash,
	_effect_player_blood_splash,
	_effect_civilian_blood_splash,
	_effect_assimilated_civilian_blood_splash,
	_effect_enforcer_blood_splash,
	_effect_compiler_bolt_minor_detonation,
	_effect_compiler_bolt_major_detonation,
	_effect_compiler_bolt_major_contrail,
	_effect_fighter_projectile_detonation,
	_effect_fighter_melee_detonation,
	_effect_hunter_projectile_detonation,
	_effect_hunter_spark,
	_effect_minor_fusion_detonation,
	_effect_major_fusion_detonation,
	_effect_major_fusion_contrail,
	_effect_fist_detonation,
	_effect_minor_defender_detonation,
	_effect_major_defender_detonation,
	_effect_defender_spark,
	_effect_trooper_blood_splash,
	_effect_water_lamp_breaking,
	_effect_lava_lamp_breaking,
	_effect_sewage_lamp_breaking,
	_effect_alien_lamp_breaking,
	_effect_metallic_clang,
	_effect_teleport_object_in,
	_effect_teleport_object_out,
	_effect_small_water_splash,
	_effect_medium_water_splash,
	_effect_large_water_splash,
	_effect_large_water_emergence,
	_effect_small_lava_splash,
	_effect_medium_lava_splash,
	_effect_large_lava_splash,
	_effect_large_lava_emergence,
	_effect_small_sewage_splash,
	_effect_medium_sewage_splash,
	_effect_large_sewage_splash,
	_effect_large_sewage_emergence,
	_effect_small_goo_splash,
	_effect_medium_goo_splash,
	_effect_large_goo_splash,
	_effect_large_goo_emergence,
	_effect_minor_hummer_projectile_detonation,
	_effect_major_hummer_projectile_detonation,
	_effect_durandal_hummer_projectile_detonation,
	_effect_hummer_spark,
	_effect_cyborg_projectile_detonation,
	_effect_cyborg_blood_splash,
	_effect_minor_fusion_dispersal,
	_effect_major_fusion_dispersal,
	_effect_overloaded_fusion_dispersal,
	_effect_sewage_yeti_blood_splash,
	_effect_sewage_yeti_projectile_detonation,
	_effect_water_yeti_blood_splash,
	_effect_lava_yeti_blood_splash,
	_effect_lava_yeti_projectile_detonation,
	_effect_yeti_melee_detonation,
	_effect_juggernaut_spark,
	_effect_juggernaut_missile_contrail,
	// LP addition: Jjaro stuff
	_effect_small_jjaro_splash,
	_effect_medium_jjaro_splash,
	_effect_large_jjaro_splash,
	_effect_large_jjaro_emergence,
	_effect_civilian_fusion_blood_splash,
	_effect_assimilated_civilian_fusion_blood_splash,
	NUMBER_OF_EFFECT_TYPES
};

// LP change: made this settable from the resource fork
#define MAXIMUM_EFFECTS_PER_MAP (get_dynamic_limit(_dynamic_limit_effects))

/* uses SLOT_IS_USED(), SLOT_IS_FREE(), MARK_SLOT_AS_FREE(), MARK_SLOT_AS_USED() macros (0x8000 bit) */

struct effect_data /* 16 bytes LP: really 32 bytes */
{
	short type;
	short object_index;
	
	uint16 flags; /* [slot_used.1] [unused.15] */

	short data; /* used for special effects (effects) */
	short delay; /* the effect is invisible and inactive for this many ticks */
	
	short unused[11];
};
const int SIZEOF_effect_data = 32;

const int SIZEOF_effect_definition = 14;

/* ---------- globals */

// Turned the list of active effects into a variable array

extern std::vector<effect_data> EffectList;
#define effects (EffectList.data())

// extern struct effect_data *effects;

/* ---------- prototypes/EFFECTS.C */

short new_effect(world_point3d *origin, short polygon_index, short type, angle facing);
void update_effects(void); /* assumes ¶t==1 tick */

void remove_all_nonpersistent_effects(void);
void remove_effect(short effect_index);

void mark_effect_collections(short type, bool loading);

void teleport_object_in(short object_index);
void teleport_object_out(short object_index);

effect_data *get_effect_data(
	const short effect_index);

// LP: to pack and unpack this data;
// these do not make the definitions visible to the outside world

uint8 *unpack_effect_data(uint8 *Stream, effect_data *Objects, size_t Count);
uint8 *pack_effect_data(uint8 *Stream, effect_data *Objects, size_t Count);
uint8 *unpack_effect_definition(uint8 *Stream, size_t Count);
uint8 *pack_effect_definition(uint8 *Stream, size_t Count);
uint8* unpack_m1_effect_definition(uint8* Stream, size_t Count);
void init_effect_definitions();

#endif

