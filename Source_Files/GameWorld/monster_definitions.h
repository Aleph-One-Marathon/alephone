#ifndef __MONSTER_DEFINITIONS_H
#define __MONSTER_DEFINITIONS_H

/*
MONSTER_DEFINITIONS.H

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

Monday, May 30, 1994 9:04:01 PM

Jan 30, 2000 (Loren Petrich):
	Changed "class" to "_class" to make data structures more C++-friendly

Feb 3, 2000 (Loren Petrich):
	Added VacBobs and their physics
	
Oct 26, 2000 (Mark Levin)
	Added some includes that this file depends on
*/

#include "effects.h"
#include "items.h"
#include "map.h"
#include "monsters.h"
#include "projectiles.h"
#include "SoundManagerEnums.h"


/* ---------- macros */

#define TYPE_IS_NEUTRAL(definition,type) (!((definition->friends|definition->enemies)&monster_definitions[type]._class))
#define TYPE_IS_ENEMY(definition,type) (definition->enemies&monster_definitions[type]._class)
#define TYPE_IS_FRIEND(definition,type) (definition->friends&monster_definitions[type]._class)

/* ---------- constants */

enum /* monster classes */
{
	_class_player_bit,
	_class_human_civilian_bit,
	_class_madd_bit,
	_class_possessed_hummer_bit,
		
	_class_defender_bit,

	_class_fighter_bit,
	_class_trooper_bit,
	_class_hunter_bit,
	_class_enforcer_bit,
	_class_juggernaut_bit,
	_class_hummer_bit,
	
	_class_compiler_bit,
	_class_cyborg_bit,
	_class_assimilated_civilian_bit,

	_class_tick_bit,
	_class_yeti_bit,

	_class_player= 1<<_class_player_bit,
	_class_human_civilian= 1<<_class_human_civilian_bit,
	_class_madd= 1<<_class_madd_bit,
	_class_possessed_hummer= 1<<_class_possessed_hummer_bit,
	_class_human= _class_player|_class_human_civilian|_class_madd|_class_possessed_hummer,
	
	_class_defender= 1<<_class_defender_bit,

	_class_fighter= 1<<_class_fighter_bit,
	_class_trooper= 1<<_class_trooper_bit,
	_class_hunter= 1<<_class_hunter_bit,
	_class_enforcer= 1<<_class_enforcer_bit,
	_class_juggernaut= 1<<_class_juggernaut_bit,
	_class_pfhor= _class_fighter|_class_trooper|_class_hunter|_class_enforcer|_class_juggernaut,
	
	_class_compiler= 1<<_class_compiler_bit,
	_class_cyborg= 1<<_class_cyborg_bit,
	_class_assimilated_civilian= 1<<_class_assimilated_civilian_bit,
	_class_hummer= 1<<_class_hummer_bit,
	_class_client= _class_compiler|_class_assimilated_civilian|_class_cyborg|_class_hummer,
	
	_class_tick= 1<<_class_tick_bit,
	_class_yeti= 1<<_class_yeti_bit,
	_class_native= _class_tick|_class_yeti,

	_class_hostile_alien= _class_pfhor|_class_client,
	_class_neutral_alien= _class_native
};

// old Marathon monster classes
enum 
{
	_class_player_m1 = 0x01,
	_class_human_civilian_m1 = 0x02,
	_class_madd_m1 = 0x04,
	_class_fighter_m1 = 0x08,
	_class_trooper_m1 = 0x10,
	_class_hunter_m1 = 0x20,
	_class_enforcer_m1 = 0x40,
	_class_juggernaut_m1 = 0x80,
	// unused 0x100
	_class_compiler_m1 = 0x200,
	_class_hulk = 0x400,
	// unused 0x800
	_class_looker = 0x1000,
	// unused 0x2000,
	_class_wasp = 0x4000,
	_class_assimilated_civilian_m1 = 0x8000,

	_class_client_m1 = _class_compiler_m1|_class_hulk|_class_assimilated_civilian_m1,
	_class_pfhor_m1 = _class_fighter_m1|_class_trooper_m1|_class_hunter_m1|_class_enforcer_m1|_class_juggernaut_m1
	
};

enum /* intelligence: maximum polygon switches before losing lock */
{
	_intelligence_low= 2,
	_intelligence_average= 3,
	_intelligence_high= 8
};

enum /* door retry masks */
{
	_slow_door_retry_mask= 63,
	_normal_door_retry_mask= 31,
	_fast_door_retry_mask= 15,
	_vidmaster_door_retry_mask= 3
};

enum /* flags */
{
	_monster_is_omniscent= 0x1, /* ignores line-of-sight during find_closest_appropriate_target() */
	_monster_flys= 0x2,
	_monster_is_alien= 0x4, /* moves slower on slower levels, etc. */
	_monster_major= 0x8, /* type -1 is minor */
	_monster_minor= 0x10, /* type +1 is major */
	_monster_cannot_be_dropped= 0x20, /* low levels cannot skip this monster */
	_monster_floats= 0x40, /* exclusive from flys; forces the monster to take +¶h gradually */
	_monster_cannot_attack= 0x80, /* monster has no weapons and cannot attack (runs constantly to safety) */
	_monster_uses_sniper_ledges= 0x100, /* sit on ledges and hurl shit at the player (ranged attack monsters only) */
	_monster_is_invisible= 0x200, /* this monster uses _xfer_invisibility */
	_monster_is_subtly_invisible= 0x400, /* this monster uses _xfer_subtle_invisibility */
	_monster_is_kamakazi= 0x800, /* monster does shrapnel damage and will suicide if close enough to target */
	_monster_is_berserker= 0x1000, /* below 1/4 vitality this monster goes berserk */
	_monster_is_enlarged= 0x2000, /* monster is 1.25 times normal height */
	_monster_has_delayed_hard_death= 0x4000, /* always dies soft, then switches to hard */
	_monster_fires_symmetrically= 0x8000, /* fires at ±dy, simultaneously */
	_monster_has_nuclear_hard_death= 0x10000, /* playerÕs screen whites out and slowly recovers */
	_monster_cant_fire_backwards= 0x20000, /* monster canÕt turn more than 135¡ to fire */
	_monster_can_die_in_flames= 0x40000, /* uses humanoid flaming body shape */
	_monster_waits_with_clear_shot= 0x80000, /* will sit and fire (slowly) if we have a clear shot */
	_monster_is_tiny= 0x100000, /* 0.25-size normal height */
	_monster_attacks_immediately= 0x200000, /* monster will try an attack immediately */
	_monster_is_not_afraid_of_water= 0x400000,
	_monster_is_not_afraid_of_sewage= 0x800000,
	_monster_is_not_afraid_of_lava= 0x1000000,
	_monster_is_not_afraid_of_goo= 0x2000000,
	_monster_can_teleport_under_media= 0x4000000,
	_monster_chooses_weapons_randomly= 0x8000000,
	/* monsters unable to open doors have door retry masks of NONE */
	/* monsters unable to switch levels have min,max ledge deltas of 0 */
	/* monsters unstopped by bullets have hit frames of NONE */

	// pseudo flags set when reading Marathon 1 physics
	_monster_weaknesses_cause_soft_death = 0x10000000,
	_monster_screams_when_crushed = 0x20000000,
	_monster_makes_sound_when_activated = 0x40000000, // instead of when locking on a target
	_monster_can_grenade_climb = 0x80000000, // only applies to player
};

enum /* monster speeds (world_distance per tick); also used for projectiles */
{
	_speed_slow= WORLD_ONE/120,
	_speed_medium= WORLD_ONE/80,
	_speed_almost_fast= WORLD_ONE/70,
	_speed_fast= WORLD_ONE/40,
	_speed_superfast1= WORLD_ONE/30,
	_speed_superfast2= WORLD_ONE/28,
	_speed_superfast3= WORLD_ONE/26,
	_speed_superfast4= WORLD_ONE/24,
	_speed_superfast5= WORLD_ONE/22,
	_speed_blinding= WORLD_ONE/20,
	_speed_insane= WORLD_ONE/10
};

#define NORMAL_MONSTER_GRAVITY (WORLD_ONE/120)
#define NORMAL_MONSTER_TERMINAL_VELOCITY (WORLD_ONE/14)

/* ---------- monster definition structures */

struct attack_definition
{
	int16 type;
	int16 repetitions;
	angle error; /* ±error is added to the firing angle */
	world_distance range; /* beyond which we cannot attack */
	int16 attack_shape; /* attack occurs when keyframe is displayed */
	
	world_distance dx, dy, dz; /* +dy is right, +dx is out, +dz is up */
};

struct monster_definition /* <128 bytes */
{
	int16 collection;
	
	int16 vitality;
	uint32 immunities, weaknesses;
	uint32 flags;

	int32 _class; /* our class */
	int32 friends, enemies; /* bit fields of what classes we consider friendly and what types we donÕt like */

	_fixed sound_pitch;
	int16 activation_sound, friendly_activation_sound, clear_sound;
	int16 kill_sound, apology_sound, friendly_fire_sound;
	int16 flaming_sound; /* the scream we play when we go down in flames */
	int16 random_sound, random_sound_mask; /* if moving and locked play this sound if we get time and our mask comes up */

	int16 carrying_item_type; /* an item type we might drop if we donÕt explode */

	world_distance radius, height;
	world_distance preferred_hover_height;
	world_distance minimum_ledge_delta, maximum_ledge_delta;
	_fixed external_velocity_scale;
	int16 impact_effect, melee_impact_effect, contrail_effect;

	int16 half_visual_arc, half_vertical_visual_arc;
	world_distance visual_range, dark_visual_range;
	int16 intelligence;
	int16 speed, gravity, terminal_velocity;
	int16 door_retry_mask;
	int16 shrapnel_radius; /* no shrapnel if NONE */
	struct damage_definition shrapnel_damage;

	shape_descriptor hit_shapes;
	shape_descriptor hard_dying_shape, soft_dying_shape; /* minus dead frame */
	shape_descriptor hard_dead_shapes, soft_dead_shapes; /* NONE for vanishing */
	shape_descriptor stationary_shape, moving_shape;
	shape_descriptor teleport_in_shape, teleport_out_shape;
	
	/* which type of attack the monster actually uses is determined at attack time; typically
		melee attacks will occur twice as often as ranged attacks because the monster will be
		stopped (and stationary monsters attack twice as often as moving ones) */
	int16 attack_frequency;
	struct attack_definition melee_attack;
	struct attack_definition ranged_attack;
};

// So as not to repeat in script_instructions.cpp (Pfhortran)
#ifndef DONT_REPEAT_DEFINITIONS

/* ---------- monster definitions */

struct monster_definition monster_definitions[NUMBER_OF_MONSTER_TYPES];
const struct monster_definition original_monster_definitions[NUMBER_OF_MONSTER_TYPES]=
{
	{ /* _monster_marine (canÕt be used as a regular monster) */
		_collection_player, /* shape collection */
		20, 0, 0, /* vitality, immunities, weaknesses */
		_monster_cannot_be_dropped|_monster_can_die_in_flames, /* flags */

		_class_player,	
		_class_human, /* friends */
		-1, /* enemies */

		_normal_frequency, /* sound pitch */	
		NONE, NONE, NONE, NONE, NONE, NONE, /* sounds: activation, friendly activation, clear, kill, apology, friendly-fire */
		NONE, /* flaming death sound */
		NONE, 0, /* random sound, random sound mask */
		
		NONE, /* carrying item type */

		WORLD_ONE/5, (4*WORLD_ONE)/5, /* radius, height */
		0, /* preferred hover height */
		0, 0,
		(3*FIXED_ONE)/4, /* external velocity scale */
		_effect_player_blood_splash, NONE, NONE, /* impact effect, melee impact effect, contrail effect */

		0, 0, /* half visual arc, half vertical visual arc */
		0, 0, /* visual range, dark visual range */
		_intelligence_low, /* intelligence */
		_speed_almost_fast, /* speed */
		NORMAL_MONSTER_GRAVITY, NORMAL_MONSTER_TERMINAL_VELOCITY, /* gravity, terminal velocity */
		_normal_door_retry_mask, /* door retry mask */
		NONE, {NONE, 0, 0, 0}, /* shrapnel radius, shrapnel damage */
		
		0, /* being hit */
		0, 0, /* dying hard (popping), dying soft (falling) */
		0, 0, /* hard dead frames, soft dead frames */
		0, 0, /* stationary shape, moving shape (no permutations) */
		0, 0, /* teleport in shape, teleport out shape */
	},
	
	{ /* _monster_tick_minor */
		BUILD_COLLECTION(_collection_tick, 0), /* shape collection */
		0, 0, 0, /* vitality, immunities, weaknesses */
		_monster_is_alien|_monster_minor|_monster_flys|_monster_has_delayed_hard_death|_monster_cannot_attack, /* flags */
		
		_class_tick, /* class */
		-1, /* friends */
		0, /* enemies */
		
		_normal_frequency, /* sound pitch */
		NONE, NONE, NONE, NONE, NONE, NONE, /* sounds: activation, friendly activation, clear, kill, apology, friendly-fire */
		NONE, /* flaming death sound */
		_snd_tick_chatter, 0, /* random sound, random sound mask */

		NONE, /* carrying item type */
	
		WORLD_ONE_HALF, WORLD_ONE_HALF, /* radius, height */
		0, /* preferred hover height */
		INT16_MIN, INT16_MAX, /* minimum ledge delta, maximum ledge delta */
		FIXED_ONE, /* external velocity scale */
		NONE, NONE, NONE, /* impact effect, melee impact effect, contrail effect */
	
		QUARTER_CIRCLE, QUARTER_CIRCLE/3, /* half visual arc, half vertical visual arc */
		30*WORLD_ONE, 15*WORLD_ONE, /* visual range, dark visual range */
		_intelligence_low, /* intelligence */
		_speed_blinding, /* speed */
		NORMAL_MONSTER_GRAVITY, NORMAL_MONSTER_TERMINAL_VELOCITY, /* gravity, terminal velocity */
		NONE, /* door retry mask */
		NONE, {NONE, _alien_damage, 30, 10, FIXED_ONE}, /* shrapnel radius, shrapnel damage */
		
		0, /* being hit */
		6, 3, /* dying hard (popping), dying soft (falling) */
		5, 5, /* hard dead frames, soft dead frames */
		1, 1, /* stationary shape, moving shape (no permutations) */
		UNONE, UNONE, /* teleport in shape, teleport out shape */
		
		2*TICKS_PER_SECOND, /* attack frequency (for both melee and ranged attacks) */
		
		/* melee attack */
		{
			NONE, /* melee attack type */
		},
		
		/* ranged attack */
		{
			NONE, /* ranged attack type */
		}
	},

	{ /* _monster_tick_major */
		BUILD_COLLECTION(_collection_tick, 0), /* shape collection */
		0, 0, 0, /* vitality, immunities, weaknesses */
		_monster_is_alien|_monster_major|_monster_flys|_monster_has_delayed_hard_death|_monster_cannot_attack, /* flags */
		
		_class_tick, /* class */
		-1, /* friends */
		0, /* enemies */
		
		_higher_frequency, /* sound pitch */
		NONE, NONE, NONE, NONE, NONE, NONE, /* sounds: activation, friendly activation, clear, kill, apology, friendly-fire */
		NONE, /* flaming death sound */
		_snd_tick_chatter, 0, /* random sound, random sound mask */

		NONE, /* carrying item type */
	
		WORLD_ONE/5, WORLD_ONE_FOURTH, /* radius, height */
		0, /* preferred hover height */
		INT16_MIN, INT16_MAX, /* minimum ledge delta, maximum ledge delta */
		FIXED_ONE, /* external velocity scale */
		NONE, NONE, NONE, /* impact effect, melee impact effect, contrail effect */
	
		QUARTER_CIRCLE, QUARTER_CIRCLE/3, /* half visual arc, half vertical visual arc */
		30*WORLD_ONE, 15*WORLD_ONE, /* visual range, dark visual range */
		_intelligence_low, /* intelligence */
		_speed_blinding, /* speed */
		NORMAL_MONSTER_GRAVITY, NORMAL_MONSTER_TERMINAL_VELOCITY, /* gravity, terminal velocity */
		NONE, /* door retry mask */
		2*WORLD_ONE, {_damage_explosion, _alien_damage, 40, 20, FIXED_ONE}, /* shrapnel radius, shrapnel damage */
		
		0, /* being hit */
		4, 3, /* dying hard (popping), dying soft (falling) */
		5, 5, /* hard dead frames, soft dead frames */
		1, 1, /* stationary shape, moving shape (no permutations) */
		UNONE, UNONE, /* teleport in shape, teleport out shape */
		
		2*TICKS_PER_SECOND, /* attack frequency (for both melee and ranged attacks) */
		
		/* melee attack */
		{
			NONE, /* melee attack type */
		},
		
		/* ranged attack */
		{
			NONE, /* ranged attack type */
		}
	},

	{ /* _monster_tick_kamakazi */
		BUILD_COLLECTION(_collection_tick, 0), /* shape collection */
		0, 0, 0, /* vitality, immunities, weaknesses */
		_monster_is_alien|_monster_flys|_monster_is_kamakazi|_monster_has_delayed_hard_death, /* flags */
		
		_class_tick, /* class */
		0, /* friends */
		-1, /* enemies */
		
		_normal_frequency, /* sound pitch */
		NONE, NONE, NONE, NONE, NONE, NONE, /* sounds: activation, friendly activation, clear, kill, apology, friendly-fire */
		NONE, /* flaming death sound */
		NONE, 0, /* random sound, random sound mask */

		NONE, /* carrying item type */
	
		WORLD_ONE/5, WORLD_ONE_FOURTH, /* radius, height */
		0, /* preferred hover height */
		-5*WORLD_ONE, 5*WORLD_ONE, /* minimum ledge delta, maximum ledge delta */
		FIXED_ONE, /* external velocity scale */
		NONE, NONE, NONE, /* impact effect, melee impact effect, contrail effect */
	
		QUARTER_CIRCLE, QUARTER_CIRCLE/3, /* half visual arc, half vertical visual arc */
		30*WORLD_ONE, 15*WORLD_ONE, /* visual range, dark visual range */
		_intelligence_low, /* intelligence */
		_speed_blinding, /* speed */
		NORMAL_MONSTER_GRAVITY, NORMAL_MONSTER_TERMINAL_VELOCITY, /* gravity, terminal velocity */
		NONE, /* door retry mask */
		WORLD_ONE, {_damage_explosion, _alien_damage, 30, 10, FIXED_ONE}, /* shrapnel radius, shrapnel damage */
		
		0, /* being hit */
		6, 4, /* dying hard (popping), dying soft (falling) */
		5, 5, /* hard dead frames, soft dead frames */
		0, 1, /* stationary shape, moving shape (no permutations) */
		UNONE, UNONE, /* teleport in shape, teleport out shape */
		
		2*TICKS_PER_SECOND, /* attack frequency (for both melee and ranged attacks) */
		
		/* melee attack */
		{
			_projectile_minor_energy_drain, /* melee attack type */
			5000, /* repetitions */
			0, /* error */
			WORLD_ONE, /* range */
			
			2, /* melee attack shape */

			0, 0, WORLD_ONE_HALF, /* dx, dy, dz */
		},
		
		/* ranged attack */
		{
			NONE, /* ranged attack type */
		}
	},

	{ /* _monster_compiler_minor */
		BUILD_COLLECTION(_collection_compiler, 0), /* shape collection */
		160, FLAG(_damage_flame)|FLAG(_damage_lava), FLAG(_damage_fusion_bolt), /* vitality, immunities, weaknesses */
		_monster_is_alien|_monster_minor|_monster_floats|_monster_can_teleport_under_media, /* flags */

		_class_compiler, /* class */
		_class_compiler, /* friends */
		_class_human|_class_native|_class_defender, /* enemies */
	
		_normal_frequency, /* sound pitch */	
		NONE, NONE, NONE, NONE, NONE, NONE, /* sounds: activation, friendly activation, clear, kill, apology, friendly-fire */
		NONE, /* flaming dying sound */
		NONE, 0, /* random sound, random sound mask */

		NONE, /* carrying item type */
	
		WORLD_ONE/5, WORLD_ONE, /* radius, height */
		0, /* preferred hover height */
		INT16_MIN, INT16_MAX, /* minimum ledge delta, maximum ledge delta */
		0, /* external velocity scale */
		NONE, NONE, NONE, /* impact effect, melee impact effect, contrail effect */
	
		QUARTER_CIRCLE, QUARTER_CIRCLE/3, /* half visual arc, half vertical visual arc */
		30*WORLD_ONE, 3*WORLD_ONE, /* visual range, dark visual range */
		_intelligence_high, /* intelligence */
		_speed_superfast2, /* speed */
		NORMAL_MONSTER_GRAVITY, NORMAL_MONSTER_TERMINAL_VELOCITY, /* gravity, terminal velocity */
		_vidmaster_door_retry_mask, /* door retry mask */
		NONE, {NONE, 0, 0, 0}, /* shrapnel radius, shrapnel damage */
		
		7, /* being hit */
		UNONE, 2, /* dying hard (popping), dying soft (falling) */
		UNONE, UNONE, /* hard dead frames, soft dead frames */
		0, 3, /* stationary shape, moving shape */
		0, 0, /* teleport in shape, teleport out shape */
		
		3*TICKS_PER_SECOND, /* attack frequency */
		
		/* melee attack */
		{
			NONE, /* melee attack type */
		},
		
		/* ranged attack */
		{
			_projectile_compiler_bolt_minor, /* ranged attack type */
			0, /* repetitions */
			NUMBER_OF_ANGLES/200, /* error angle */
			20*WORLD_ONE, /* range */
			1, /* ranged attack shape */
			
			WORLD_ONE/16, 0, WORLD_ONE_HALF+WORLD_ONE_FOURTH/2, /* dx, dy, dz */
		}
	},

	{ /* _monster_compiler_major */
		BUILD_COLLECTION(_collection_compiler, 1), /* shape collection */
		200, FLAG(_damage_flame)|FLAG(_damage_lava), FLAG(_damage_fusion_bolt), /* vitality, immunities, weaknesses */
		_monster_is_alien|_monster_major|_monster_floats|_monster_can_teleport_under_media, /* flags */
	
		_class_compiler, /* class */
		_class_compiler, /* friends */
		_class_human|_class_native|_class_defender, /* enemies */
	
		_higher_frequency, /* sound pitch */	
		NONE, NONE, NONE, NONE, NONE, NONE, /* sounds: activation, friendly activation, clear, kill, apology, friendly-fire */
		NONE, /* flaming dying sound */
		NONE, 0, /* random sound, random sound mask */

		NONE, /* carrying item type */
	
		WORLD_ONE/5, WORLD_ONE, /* radius, height */
		0, /* preferred hover height */
		INT16_MIN, INT16_MAX, /* minimum ledge delta, maximum ledge delta */
		0, /* external velocity scale */
		NONE, NONE, NONE, /* impact effect, melee impact effect, contrail effect */
	
		QUARTER_CIRCLE, QUARTER_CIRCLE/3, /* half visual arc, half vertical visual arc */
		30*WORLD_ONE, 3*WORLD_ONE, /* visual range, dark visual range */
		_intelligence_high, /* intelligence */
		_speed_superfast3, /* speed */
		NORMAL_MONSTER_GRAVITY, NORMAL_MONSTER_TERMINAL_VELOCITY, /* gravity, terminal velocity */
		_vidmaster_door_retry_mask, /* door retry mask */
		NONE, {NONE, 0, 0, 0}, /* shrapnel radius, shrapnel damage */
		
		7, /* being hit */
		UNONE, 2, /* dying hard (popping), dying soft (falling) */
		UNONE, UNONE, /* hard dead frames, soft dead frames */
		0, 3, /* stationary shape, moving shape */
		0, 0, /* teleport in shape, teleport out shape */
		
		4*TICKS_PER_SECOND, /* attack frequency */
		
		/* melee attack */
		{
			NONE, /* melee attack type */
		},
		
		/* ranged attack */
		{
			_projectile_compiler_bolt_major, /* ranged attack type */
			0, /* repetitions */
			0, /* error angle */
			20*WORLD_ONE, /* range */
			1, /* ranged attack shape */
			
			WORLD_ONE/16, 0, WORLD_ONE_HALF+WORLD_ONE_FOURTH/2, /* dx, dy, dz */
		}
	},
	
	{ /* _monster_compiler_minor_invisible */
		BUILD_COLLECTION(_collection_compiler, 0), /* shape collection */
		160, FLAG(_damage_flame)|FLAG(_damage_lava), FLAG(_damage_fusion_bolt), /* vitality, immunities, weaknesses */
		_monster_is_alien|_monster_minor|_monster_floats|_monster_is_invisible|_monster_can_teleport_under_media, /* flags */
	
		_class_compiler, /* class */
		_class_compiler, /* friends */
		_class_human|_class_native|_class_defender, /* enemies */
	
		_normal_frequency, /* sound pitch */	
		NONE, NONE, NONE, NONE, NONE, NONE, /* sounds: activation, friendly activation, clear, kill, apology, friendly-fire */
		NONE, /* flaming dying sound */
		NONE, 0, /* random sound, random sound mask */

		NONE, /* carrying item type */
	
		WORLD_ONE/5, WORLD_ONE, /* radius, height */
		0, /* preferred hover height */
		INT16_MIN, INT16_MAX, /* minimum ledge delta, maximum ledge delta */
		0, /* external velocity scale */
		NONE, NONE, NONE, /* impact effect, melee impact effect, contrail effect */
	
		QUARTER_CIRCLE, QUARTER_CIRCLE/3, /* half visual arc, half vertical visual arc */
		30*WORLD_ONE, 3*WORLD_ONE, /* visual range, dark visual range */
		_intelligence_high, /* intelligence */
		_speed_superfast4, /* speed */
		NORMAL_MONSTER_GRAVITY, NORMAL_MONSTER_TERMINAL_VELOCITY, /* gravity, terminal velocity */
		_vidmaster_door_retry_mask, /* door retry mask */
		NONE, {NONE, 0, 0, 0}, /* shrapnel radius, shrapnel damage */
		
		7, /* being hit */
		UNONE, 2, /* dying hard (popping), dying soft (falling) */
		UNONE, UNONE, /* hard dead frames, soft dead frames */
		0, 3, /* stationary shape, moving shape */
		0, 0, /* teleport in shape, teleport out shape */
		
		3*TICKS_PER_SECOND, /* attack frequency */
		
		/* melee attack */
		{
			NONE, /* melee attack type */
		},
		
		/* ranged attack */
		{
			_projectile_compiler_bolt_minor, /* ranged attack type */
			0, /* repetitions */
			NUMBER_OF_ANGLES/200, /* error angle */
			20*WORLD_ONE, /* range */
			1, /* ranged attack shape */
			
			WORLD_ONE/16, 0, WORLD_ONE_HALF+WORLD_ONE_FOURTH/2, /* dx, dy, dz */
		}
	},
	
	{ /* _monster_compiler_major_invisible */
		BUILD_COLLECTION(_collection_compiler, 1), /* shape collection */
		200, FLAG(_damage_flame)|FLAG(_damage_lava), FLAG(_damage_fusion_bolt), /* vitality, immunities, weaknesses */
		_monster_is_alien|_monster_major|_monster_floats|_monster_is_subtly_invisible|_monster_can_teleport_under_media, /* flags */
	
		_class_compiler, /* class */
		_class_compiler, /* friends */
		_class_human|_class_native|_class_defender, /* enemies */
	
		_higher_frequency, /* sound pitch */	
		NONE, NONE, NONE, NONE, NONE, NONE, /* sounds: activation, friendly activation, clear, kill, apology, friendly-fire */
		NONE, /* flaming dying sound */
		NONE, 0, /* random sound, random sound mask */

		NONE, /* carrying item type */
	
		WORLD_ONE/5, WORLD_ONE, /* radius, height */
		0, /* preferred hover height */
		INT16_MIN, INT16_MAX, /* minimum ledge delta, maximum ledge delta */
		0, /* external velocity scale */
		NONE, NONE, NONE, /* impact effect, melee impact effect, contrail effect */
	
		QUARTER_CIRCLE, QUARTER_CIRCLE/3, /* half visual arc, half vertical visual arc */
		30*WORLD_ONE, 3*WORLD_ONE, /* visual range, dark visual range */
		_intelligence_high, /* intelligence */
		_speed_superfast5, /* speed */
		NORMAL_MONSTER_GRAVITY, NORMAL_MONSTER_TERMINAL_VELOCITY, /* gravity, terminal velocity */
		_vidmaster_door_retry_mask, /* door retry mask */
		NONE, {NONE, 0, 0, 0}, /* shrapnel radius, shrapnel damage */
		
		7, /* being hit */
		UNONE, 2, /* dying hard (popping), dying soft (falling) */
		UNONE, UNONE, /* hard dead frames, soft dead frames */
		0, 3, /* stationary shape, moving shape */
		0, 0, /* teleport in shape, teleport out shape */
		
		4*TICKS_PER_SECOND, /* attack frequency */
		
		/* melee attack */
		{
			NONE, /* melee attack type */
		},
		
		/* ranged attack */
		{
			_projectile_compiler_bolt_major, /* ranged attack type */
			0, /* repetitions */
			0, /* error angle */
			20*WORLD_ONE, /* range */
			1, /* ranged attack shape */
			
			WORLD_ONE/16, 0, WORLD_ONE_HALF+WORLD_ONE_FOURTH/2, /* dx, dy, dz */
		}
	},
	
	{ /* _monster_fighter (minor) */
		BUILD_COLLECTION(_collection_fighter, 0), /* shape collection */
		40, 0, 0, /* vitality, immunities, weaknesses */
		_monster_is_alien|_monster_minor|_monster_can_die_in_flames, /* flags */
		
		_class_fighter, /* class */
		_class_pfhor, /* friends */
		_class_human|_class_native|_class_defender, /* enemies */
	
		_normal_frequency, /* sound pitch */	
		_snd_fighter_activate, NONE, NONE, NONE, NONE, NONE, /* sounds: activation, friendly activation, clear, kill, apology, friendly-fire */
		_snd_fighter_wail, /* dying flaming */
		_snd_fighter_chatter, 15, /* random sound, random sound mask */

		NONE, /* carrying item type */
	
		WORLD_ONE/5, (4*WORLD_ONE)/5, /* radius, height */
		0, /* preferred hover height */
		-4*WORLD_ONE, WORLD_ONE/3, /* minimum ledge delta, maximum ledge delta */
		(3*FIXED_ONE)/4, /* external velocity scale */
		_effect_fighter_blood_splash, NONE, NONE, /* impact effect, melee impact effect, contrail effect */
	
		QUARTER_CIRCLE, QUARTER_CIRCLE/3, /* half visual arc, half vertical visual arc */
		30*WORLD_ONE, 2*WORLD_ONE, /* visual range, dark visual range */
		_intelligence_high, /* intelligence */
		_speed_superfast1, /* speed */
		NORMAL_MONSTER_GRAVITY, NORMAL_MONSTER_TERMINAL_VELOCITY, /* gravity, terminal velocity */
		_normal_door_retry_mask, /* door retry mask */
		NONE, {NONE, 0, 0, 0}, /* shrapnel radius, shrapnel damage */
		
		4, /* being hit */
		1, 3, /* dying hard (popping), dying soft (falling) */
		6, 5, /* hard dead frames, soft dead frames */
		7, 0, /* stationary shape, moving shape */
		12, 12, /* teleport in shape, teleport out shape */
		
		4*TICKS_PER_SECOND, /* attack frequency (for both melee and ranged attacks) */
		
		/* melee attack */
		{
			_projectile_staff, /* melee attack type */
			0, /* repetitions */
			0, /* error */
			WORLD_ONE, /* range */
			
			2, /* melee attack shape */

			0, 0, WORLD_ONE_HALF+WORLD_ONE_FOURTH/2, /* dx, dy, dz */
		},
		
		/* ranged attack */
		{
			NONE, /* ranged attack type */
			0, /* repetitions */
			0, /* error angle */
			12*WORLD_ONE, /* range */
			3, /* ranged attack shape */
			
			0, 0, WORLD_ONE_HALF+WORLD_ONE_FOURTH/2, /* dx, dy, dz */
		}
	},

	{ /* _monster_fighter (major) */
		BUILD_COLLECTION(_collection_fighter, 1), /* shape collection */
		80, 0, 0, /* vitality, immunities, weaknesses */
		_monster_is_alien|_monster_major|_monster_is_berserker|_monster_can_die_in_flames, /* flags */
	
		_class_fighter, /* class */
		_class_pfhor, /* friends */
		_class_human|_class_native|_class_defender, /* enemies */
	
		_lower_frequency, /* sound pitch */	
		_snd_fighter_activate, NONE, NONE, NONE, NONE, NONE, /* sounds: activation, friendly activation, clear, kill, apology, friendly-fire */
		_snd_fighter_wail, /* dying flaming */
		_snd_fighter_chatter, 15, /* random sound, random sound mask */

		NONE, /* carrying item type */
	
		WORLD_ONE/5, (4*WORLD_ONE)/5, /* radius, height */
		0, /* preferred hover height */
		-4*WORLD_ONE, WORLD_ONE/3, /* minimum ledge delta, maximum ledge delta */
		(3*FIXED_ONE)/4, /* external velocity scale */
		_effect_fighter_blood_splash, NONE, NONE, /* impact effect, melee impact effect, contrail effect */
	
		QUARTER_CIRCLE, QUARTER_CIRCLE/3, /* half visual arc, half vertical visual arc */
		30*WORLD_ONE, 3*WORLD_ONE, /* visual range, dark visual range */
		_intelligence_high, /* intelligence */
		_speed_superfast2, /* speed */
		NORMAL_MONSTER_GRAVITY, NORMAL_MONSTER_TERMINAL_VELOCITY, /* gravity, terminal velocity */
		_normal_door_retry_mask, /* door retry mask */
		NONE, {NONE, 0, 0, 0}, /* shrapnel radius, shrapnel damage */
		
		4, /* being hit */
		1, 3, /* dying hard (popping), dying soft (falling) */
		6, 5, /* hard dead frames, soft dead frames */
		7, 0, /* stationary shape, moving shape */
		12, 12, /* teleport in shape, teleport out shape */
		
		2*TICKS_PER_SECOND, /* attack frequency (for both melee and ranged attacks) */
		
		/* melee attack */
		{
			_projectile_staff, /* melee attack type */
			0, /* repetitions */
			0, /* error */
			WORLD_ONE, /* range */
			
			2, /* melee attack shape */

			0, 0, WORLD_ONE_HALF+WORLD_ONE_FOURTH/2, /* dx, dy, dz */
		},
		
		/* ranged attack */
		{
			NONE, /* ranged attack type */
			0, /* repetitions */
			0, /* error angle */
			12*WORLD_ONE, /* range */
			3, /* ranged attack shape */
			
			0, 0, WORLD_ONE_HALF+WORLD_ONE_FOURTH/2, /* dx, dy, dz */
		}
	},

	{ /* _monster_fighter (minor projectile) */
		BUILD_COLLECTION(_collection_fighter, 2), /* shape collection */
		80, 0, 0, /* vitality, immunities, weaknesses */
		_monster_is_alien|_monster_minor|_monster_uses_sniper_ledges|_monster_can_die_in_flames, /* flags */
	
		_class_fighter, /* class */
		_class_pfhor, /* friends */
		_class_human|_class_native|_class_defender, /* enemies */
	
		_normal_frequency, /* sound pitch */	
		_snd_fighter_activate, NONE, NONE, NONE, NONE, NONE, /* sounds: activation, friendly activation, clear, kill, apology, friendly-fire */
		_snd_fighter_wail, /* dying flaming */
		_snd_fighter_chatter, 15, /* random sound, random sound mask */

		NONE, /* carrying item type */
	
		WORLD_ONE/5, (4*WORLD_ONE)/5, /* radius, height */
		0, /* preferred hover height */
		-4*WORLD_ONE, WORLD_ONE/3, /* minimum ledge delta, maximum ledge delta */
		(3*FIXED_ONE)/4, /* external velocity scale */
		_effect_fighter_blood_splash, NONE, NONE, /* impact effect, melee impact effect, contrail effect */
	
		QUARTER_CIRCLE, QUARTER_CIRCLE/3, /* half visual arc, half vertical visual arc */
		30*WORLD_ONE, 3*WORLD_ONE, /* visual range, dark visual range */
		_intelligence_high, /* intelligence */
		_speed_superfast3, /* speed */
		NORMAL_MONSTER_GRAVITY, NORMAL_MONSTER_TERMINAL_VELOCITY, /* gravity, terminal velocity */
		_normal_door_retry_mask, /* door retry mask */
		NONE, {NONE, 0, 0, 0}, /* shrapnel radius, shrapnel damage */
		
		4, /* being hit */
		1, 3, /* dying hard (popping), dying soft (falling) */
		6, 5, /* hard dead frames, soft dead frames */
		7, 0, /* stationary shape, moving shape */
		12, 12, /* teleport in shape, teleport out shape */
		
		4*TICKS_PER_SECOND, /* attack frequency (for both melee and ranged attacks) */
		
		/* melee attack */
		{
			_projectile_staff, /* melee attack type */
			0, /* repetitions */
			0, /* error */
			WORLD_ONE, /* range */
			
			2, /* melee attack shape */

			WORLD_ONE/16, 0, WORLD_ONE_FOURTH, /* dx, dy, dz */
		},
		
		/* ranged attack */
		{
			_projectile_staff_bolt, /* ranged attack type */
			0, /* repetitions */
			NUMBER_OF_ANGLES/150, /* error angle */
			12*WORLD_ONE, /* range */
			2, /* ranged attack shape */
			
			WORLD_ONE/16, 0, WORLD_ONE_FOURTH, /* dx, dy, dz */
		}
	},

	{ /* _monster_fighter (major projectile) */
		BUILD_COLLECTION(_collection_fighter, 3), /* shape collection */
		80, 0, 0, /* vitality, immunities, weaknesses */
		_monster_is_alien|_monster_major|_monster_uses_sniper_ledges|_monster_is_berserker|_monster_can_die_in_flames, /* flags */
	
		_class_fighter, /* class */
		_class_pfhor, /* friends */
		_class_human|_class_native|_class_defender, /* enemies */
	
		_higher_frequency, /* sound pitch */	
		_snd_fighter_activate, NONE, NONE, NONE, NONE, NONE, /* sounds: activation, friendly activation, clear, kill, apology, friendly-fire */
		_snd_fighter_wail, /* dying flaming */
		_snd_fighter_chatter, 15, /* random sound, random sound mask */

		NONE, /* carrying item type */
	
		WORLD_ONE/5, (4*WORLD_ONE)/5, /* radius, height */
		0, /* preferred hover height */
		-4*WORLD_ONE, WORLD_ONE/3, /* minimum ledge delta, maximum ledge delta */
		(3*FIXED_ONE)/4, /* external velocity scale */
		_effect_fighter_blood_splash, NONE, NONE, /* impact effect, melee impact effect, contrail effect */
	
		QUARTER_CIRCLE, QUARTER_CIRCLE/3, /* half visual arc, half vertical visual arc */
		30*WORLD_ONE, 5*WORLD_ONE, /* visual range, dark visual range */
		_intelligence_high, /* intelligence */
		_speed_superfast4, /* speed */
		NORMAL_MONSTER_GRAVITY, NORMAL_MONSTER_TERMINAL_VELOCITY, /* gravity, terminal velocity */
		_normal_door_retry_mask, /* door retry mask */
		NONE, {NONE, 0, 0, 0}, /* shrapnel radius, shrapnel damage */
		
		4, /* being hit */
		1, 3, /* dying hard (popping), dying soft (falling) */
		6, 5, /* hard dead frames, soft dead frames */
		7, 0, /* stationary shape, moving shape */
		12, 12, /* teleport in shape, teleport out shape */
		
		3*TICKS_PER_SECOND, /* attack frequency (for both melee and ranged attacks) */
		
		/* melee attack */
		{
			_projectile_staff, /* melee attack type */
			1, /* repetitions */
			0, /* error */
			WORLD_ONE, /* range */
			
			2, /* melee attack shape */

			WORLD_ONE/16, 0, WORLD_ONE_FOURTH, /* dx, dy, dz */
		},
		
		/* ranged attack */
		{
			_projectile_staff_bolt, /* ranged attack type */
			1, /* repetitions */
			NUMBER_OF_ANGLES/150, /* error angle */
			12*WORLD_ONE, /* range */
			2, /* ranged attack shape */
			
			WORLD_ONE/16, 0, WORLD_ONE_FOURTH, /* dx, dy, dz */
		}
	},

	{ /* _civilian_crew "bob" */
		BUILD_COLLECTION(_collection_civilian, 0), /* shape collection */
		20, 0, 0, /* vitality, immunities, weaknesses */
		_monster_attacks_immediately|_monster_is_omniscent|_monster_cannot_be_dropped|_monster_waits_with_clear_shot|_monster_can_die_in_flames|_monster_uses_sniper_ledges, /* flags */

		_class_human_civilian, /* class */	
		_class_human, /* friends */
		(_class_hostile_alien^_class_assimilated_civilian)|_class_native, /* enemies */
	
		_normal_frequency, /* sound pitch */	
		_snd_human_activation, _snd_kill_the_player, _snd_human_clear, _snd_human_trash_talk, _snd_human_apology, _snd_human_stop_shooting_me_you_bastard, /* sounds: activation, friendly activation, clear, kill, apology, friendly-fire */
		_snd_human_wail, /* dying flaming */
		_snd_human_chatter, 0x1f, /* random sound, random sound mask */

		_i_magnum_magazine, /* carrying item type */
	
		WORLD_ONE/5, (4*WORLD_ONE)/5, /* radius, height */
		0, /* preferred hover height */
		-2*WORLD_ONE, WORLD_ONE/3, /* minimum ledge delta, maximum ledge delta */
		FIXED_ONE, /* external velocity scale */
		_effect_civilian_blood_splash, NONE, NONE, /* impact effect, melee impact effect, contrail effect */
	
		QUARTER_CIRCLE, QUARTER_CIRCLE/3, /* half visual arc, half vertical visual arc */
		30*WORLD_ONE, WORLD_ONE, /* visual range, dark visual range */
		_intelligence_high, /* intelligence */
		_speed_blinding, /* speed */
		NORMAL_MONSTER_GRAVITY, NORMAL_MONSTER_TERMINAL_VELOCITY, /* gravity, terminal velocity */
		_vidmaster_door_retry_mask, /* door retry mask */
		NONE, {NONE, 0, 0, 0}, /* shrapnel radius, shrapnel damage */
		
		10, /* being hit */
		2, 1, /* dying hard (popping), dying soft (falling) */
		4, 3, /* hard dead frames, soft dead frames */
		6, 0, /* stationary shape, moving shape */
		9, 8, /* teleport in shape, teleport out shape */
		
		3*TICKS_PER_SECOND, /* attack frequency (for both melee and ranged attacks) */
		
		/* melee attack */
		{
			NONE, /* melee attack type */
		},
		
		/* ranged attack */
		{
			_projectile_pistol_bullet, /* ranged attack type */
			1, /* repetitions */
			NUMBER_OF_ANGLES/150, /* error angle */
			10*WORLD_ONE, /* range */
			5, /* ranged attack shape */
			
			0, 0, WORLD_ONE*3/4, /* dx, dy, dz */
		}
	},

	{ /* _civilian_science "fred" */
		BUILD_COLLECTION(_collection_civilian, 1), /* shape collection */
		25, 0, 0, /* vitality, immunities, weaknesses */
		_monster_attacks_immediately|_monster_is_omniscent|_monster_cannot_be_dropped|_monster_waits_with_clear_shot|_monster_can_die_in_flames|_monster_uses_sniper_ledges, /* flags */

		_class_human_civilian, /* class */	
		_class_human|_class_assimilated_civilian, /* friends */
		(_class_hostile_alien^_class_assimilated_civilian)|_class_native, /* enemies */
	
		_normal_frequency, /* sound pitch */	
		_snd_human_activation, _snd_kill_the_player, _snd_human_clear, _snd_human_trash_talk, _snd_human_apology, _snd_human_stop_shooting_me_you_bastard, /* sounds: activation, friendly activation, clear, kill, apology, friendly-fire */
		_snd_human_wail, /* dying flaming */
		_snd_human_chatter, 0x1f, /* random sound, random sound mask */

		_i_magnum_magazine, /* carrying item type */
	
		WORLD_ONE/5, (4*WORLD_ONE)/5, /* radius, height */
		0, /* preferred hover height */
		-2*WORLD_ONE, WORLD_ONE/3, /* minimum ledge delta, maximum ledge delta */
		FIXED_ONE, /* external velocity scale */
		_effect_civilian_blood_splash, NONE, NONE, /* impact effect, melee impact effect, contrail effect */
	
		QUARTER_CIRCLE, QUARTER_CIRCLE/3, /* half visual arc, half vertical visual arc */
		30*WORLD_ONE, WORLD_ONE, /* visual range, dark visual range */
		_intelligence_high, /* intelligence */
		_speed_blinding, /* speed */
		NORMAL_MONSTER_GRAVITY, NORMAL_MONSTER_TERMINAL_VELOCITY, /* gravity, terminal velocity */
		_vidmaster_door_retry_mask, /* door retry mask */
		NONE, {NONE, 0, 0, 0}, /* shrapnel radius, shrapnel damage */
		
		10, /* being hit */
		2, 1, /* dying hard (popping), dying soft (falling) */
		4, 3, /* hard dead frames, soft dead frames */
		6, 0, /* stationary shape, moving shape */
		9, 8, /* teleport in shape, teleport out shape */
		
		3*TICKS_PER_SECOND, /* attack frequency (for both melee and ranged attacks) */
		
		/* melee attack */
		{
			NONE, /* melee attack type */
		},
		
		/* ranged attack */
		{
			_projectile_pistol_bullet, /* ranged attack type */
			2, /* repetitions */
			NUMBER_OF_ANGLES/150, /* error angle */
			13*WORLD_ONE, /* range */
			5, /* ranged attack shape */
			
			0, 0, WORLD_ONE*3/4, /* dx, dy, dz */
		}
	},

	{ /* _civilian_security "steve" */
		BUILD_COLLECTION(_collection_civilian, 2), /* shape collection */
		30, 0, 0, /* vitality, immunities, weaknesses */
		_monster_attacks_immediately|_monster_is_omniscent|_monster_cannot_be_dropped|_monster_waits_with_clear_shot|_monster_can_die_in_flames|_monster_uses_sniper_ledges, /* flags */

		_class_human_civilian, /* class */	
		_class_human|_class_assimilated_civilian, /* friends */
		(_class_hostile_alien^_class_assimilated_civilian)|_class_native, /* enemies */
	
		_normal_frequency, /* sound pitch */	
		_snd_human_activation, _snd_kill_the_player, _snd_human_clear, _snd_human_trash_talk, _snd_human_apology, _snd_human_stop_shooting_me_you_bastard, /* sounds: activation, friendly activation, clear, kill, apology, friendly-fire */
		_snd_human_wail, /* dying flaming */
		_snd_human_chatter, 0x1f, /* random sound, random sound mask */

		_i_magnum, /* carrying item type */
	
		WORLD_ONE/5, (4*WORLD_ONE)/5, /* radius, height */
		0, /* preferred hover height */
		-2*WORLD_ONE, WORLD_ONE/3, /* minimum ledge delta, maximum ledge delta */
		FIXED_ONE, /* external velocity scale */
		_effect_civilian_blood_splash, NONE, NONE, /* impact effect, melee impact effect, contrail effect */
	
		QUARTER_CIRCLE, QUARTER_CIRCLE/3, /* half visual arc, half vertical visual arc */
		30*WORLD_ONE, WORLD_ONE, /* visual range, dark visual range */
		_intelligence_high, /* intelligence */
		_speed_blinding, /* speed */
		NORMAL_MONSTER_GRAVITY, NORMAL_MONSTER_TERMINAL_VELOCITY, /* gravity, terminal velocity */
		_vidmaster_door_retry_mask, /* door retry mask */
		NONE, {NONE, 0, 0, 0}, /* shrapnel radius, shrapnel damage */
		
		10, /* being hit */
		2, 1, /* dying hard (popping), dying soft (falling) */
		4, 3, /* hard dead frames, soft dead frames */
		6, 0, /* stationary shape, moving shape */
		9, 8, /* teleport in shape, teleport out shape */
		
		TICKS_PER_SECOND, /* attack frequency (for both melee and ranged attacks) */
		
		/* melee attack */
		{
			NONE, /* melee attack type */
		},
		
		/* ranged attack */
		{
			_projectile_pistol_bullet, /* ranged attack type */
			5, /* repetitions */
			NUMBER_OF_ANGLES/150, /* error angle */
			17*WORLD_ONE, /* range */
			5, /* ranged attack shape */
			
			0, 0, WORLD_ONE*3/4, /* dx, dy, dz */
		}
	},

	{ /* _civilian_assimilated "evil bob" */
		BUILD_COLLECTION(_collection_civilian, 3), /* shape collection */
		30, 0, 0, /* vitality, immunities, weaknesses */
		_monster_is_alien|_monster_is_kamakazi|_monster_can_die_in_flames, /* flags */
		
		_class_assimilated_civilian,
		_class_pfhor, /* friends */
		_class_player|_class_defender, /* enemies */
	
		_normal_frequency, /* sound pitch */	
		NONE, NONE, NONE, NONE, NONE, _snd_human_stop_shooting_me_you_bastard, /* sounds: activation, friendly activation, clear, kill, apology, friendly-fire */
		_snd_human_wail, /* dying flaming */
		_snd_assimilated_human_chatter, 0xf, /* random sound, random sound mask */

		NONE, /* carrying item type */
	
		WORLD_ONE/5, (4*WORLD_ONE)/5, /* radius, height */
		0, /* preferred hover height */
		-2*WORLD_ONE, WORLD_ONE/3, /* minimum ledge delta, maximum ledge delta */
		FIXED_ONE, /* external velocity scale */
		_effect_assimilated_civilian_blood_splash, NONE, NONE, /* impact effect, melee impact effect, contrail effect */
	
		QUARTER_CIRCLE, QUARTER_CIRCLE/3, /* half visual arc, half vertical visual arc */
		15*WORLD_ONE, WORLD_ONE, /* visual range, dark visual range */
		_intelligence_high, /* intelligence */
		_speed_blinding, /* speed */
		NORMAL_MONSTER_GRAVITY, NORMAL_MONSTER_TERMINAL_VELOCITY, /* gravity, terminal velocity */
		_vidmaster_door_retry_mask, /* door retry mask */
		WORLD_ONE, {_damage_explosion, _alien_damage, 80, 40, FIXED_ONE}, /* shrapnel radius, shrapnel damage  */
		
		10, /* being hit */
		11, UNONE, /* dying hard (popping), dying soft (falling) */
		4, 0, /* hard dead frames, soft dead frames */
		6, 0, /* stationary shape, moving shape */
		8, UNONE, /* teleport in shape, teleport out shape */
		
		2*TICKS_PER_SECOND, /* attack frequency (for both melee and ranged attacks) */
		
		/* melee attack */
		{
			NONE, /* melee attack type */
		},
		
		/* ranged attack */
		{
			NONE, /* ranged attack type */
		}
	},

	{ /* _monster_hummer_minor (small hummer) */
		BUILD_COLLECTION(_collection_hummer, 0), /* shape collection */
		40, 0, FLAG(_damage_fusion_bolt)|FLAG(_damage_compiler_bolt)|FLAG(_damage_electrical_staff), /* vitality, immunities, weaknesses */
		_monster_is_alien|_monster_flys|_monster_minor|_monster_has_delayed_hard_death, /* flags */
		
		_class_hummer, /* class */
		_class_pfhor|_class_client, /* friends */
		_class_human|_class_native|_class_defender, /* enemies */
		
		_normal_frequency, /* sound pitch */
		_snd_hummer_activate, NONE, NONE, NONE, NONE, NONE, /* sounds: activation, friendly activation, clear, kill, apology, friendly-fire */
		NONE, /* flaming death sound */
		NONE, 0, /* random sound, random sound mask */

		NONE, /* carrying item type */
	
		WORLD_ONE/4, WORLD_ONE_HALF, /* radius, height */
		WORLD_ONE_FOURTH, /* preferred hover height */
		-5*WORLD_ONE, 5*WORLD_ONE, /* minimum ledge delta, maximum ledge delta */
		FIXED_ONE, /* external velocity scale */
		_effect_hummer_spark, _effect_metallic_clang, _effect_rocket_contrail, /* impact effect, melee impact effect, contrail effect */
	
		HALF_CIRCLE, QUARTER_CIRCLE, /* half visual arc, half vertical visual arc */
		30*WORLD_ONE, 15*WORLD_ONE, /* visual range, dark visual range */
		_intelligence_low, /* intelligence */
		_speed_blinding, /* speed */
		NORMAL_MONSTER_GRAVITY, NORMAL_MONSTER_TERMINAL_VELOCITY, /* gravity, terminal velocity */
		_normal_door_retry_mask, /* door retry mask */
		NONE, {NONE, _alien_damage, 30, 10, FIXED_ONE}, /* shrapnel radius, shrapnel damage */
		
		0, /* being hit */
		3, 2, /* dying hard (popping), dying soft (falling) */
		4, UNONE, /* hard dead frames, soft dead frames */
		0, 0, /* stationary shape, moving shape (no permutations) */
		0, 0, /* teleport in shape, teleport out shape */
		
		3*TICKS_PER_SECOND, /* attack frequency (for both melee and ranged attacks) */
		
		/* melee attack */
		{
			NONE, /* melee attack type */
		},
		
		/* ranged attack */
		{
			_projectile_minor_hummer, /* ranged attack type */
			0, /* repetitions */
			3, /* error angle */
			12*WORLD_ONE, /* range */
			1, /* ranged attack shape */
			
			0, 0, 0, /* dx, dy, dz */
		}
	},

	{ /* _monster_hummer_major (big hummer) */
		BUILD_COLLECTION(_collection_hummer, 1), /* shape collection */
		60, 0, FLAG(_damage_fusion_bolt)|FLAG(_damage_compiler_bolt)|FLAG(_damage_electrical_staff), /* vitality, immunities, weaknesses */
		_monster_is_alien|_monster_flys|_monster_major|_monster_has_delayed_hard_death, /* flags */
		
		_class_hummer, /* class */
		_class_pfhor|_class_client, /* friends */
		_class_human|_class_native|_class_defender, /* enemies */
		
		_normal_frequency, /* sound pitch */
		_snd_hummer_activate, NONE, NONE, NONE, NONE, NONE, /* sounds: activation, friendly activation, clear, kill, apology, friendly-fire */
		NONE, /* flaming death sound */
		NONE, 0, /* random sound, random sound mask */

		NONE, /* carrying item type */
	
		WORLD_ONE/4, WORLD_ONE_HALF, /* radius, height */
		WORLD_ONE_FOURTH, /* preferred hover height */
		-5*WORLD_ONE, 5*WORLD_ONE, /* minimum ledge delta, maximum ledge delta */
		FIXED_ONE, /* external velocity scale */
		_effect_hummer_spark, _effect_metallic_clang, _effect_rocket_contrail, /* impact effect, melee impact effect, contrail effect */
	
		HALF_CIRCLE, QUARTER_CIRCLE, /* half visual arc, half vertical visual arc */
		30*WORLD_ONE, 15*WORLD_ONE, /* visual range, dark visual range */
		_intelligence_low, /* intelligence */
		_speed_blinding, /* speed */
		NORMAL_MONSTER_GRAVITY, NORMAL_MONSTER_TERMINAL_VELOCITY, /* gravity, terminal velocity */
		_normal_door_retry_mask, /* door retry mask */
		NONE, {NONE, _alien_damage, 30, 10, FIXED_ONE}, /* shrapnel radius, shrapnel damage */
		
		0, /* being hit */
		3, 2, /* dying hard (popping), dying soft (falling) */
		4, UNONE, /* hard dead frames, soft dead frames */
		0, 0, /* stationary shape, moving shape (no permutations) */
		0, 0, /* teleport in shape, teleport out shape */
		
		2*TICKS_PER_SECOND, /* attack frequency (for both melee and ranged attacks) */
		
		/* melee attack */
		{
			NONE, /* melee attack type */
		},
		
		/* ranged attack */
		{
			_projectile_minor_hummer, /* ranged attack type */
			2, /* repetitions */
			5, /* error angle */
			12*WORLD_ONE, /* range */
			1, /* ranged attack shape */
			
			0, 0, 0, /* dx, dy, dz */
		}
	},

	{ /* _monster_hummer_big_minor (big hummer) */
		BUILD_COLLECTION(_collection_hummer, 2), /* shape collection */
		40, 0, FLAG(_damage_fusion_bolt)|FLAG(_damage_compiler_bolt)|FLAG(_damage_electrical_staff), /* vitality, immunities, weaknesses */
		_monster_is_alien|_monster_flys|_monster_minor|_monster_has_delayed_hard_death, /* flags */
		
		_class_hummer, /* class */
		_class_pfhor|_class_client, /* friends */
		_class_human|_class_native|_class_defender, /* enemies */
		
		_higher_frequency, /* sound pitch */
		_snd_hummer_activate, NONE, NONE, NONE, NONE, NONE, /* sounds: activation, friendly activation, clear, kill, apology, friendly-fire */
		NONE, /* flaming death sound */
		NONE, 0, /* random sound, random sound mask */

		NONE, /* carrying item type */
	
		WORLD_ONE/4, WORLD_ONE_HALF, /* radius, height */
		WORLD_ONE_FOURTH, /* preferred hover height */
		-5*WORLD_ONE, 5*WORLD_ONE, /* minimum ledge delta, maximum ledge delta */
		FIXED_ONE, /* external velocity scale */
		_effect_hummer_spark, _effect_metallic_clang, _effect_rocket_contrail, /* impact effect, melee impact effect, contrail effect */
	
		HALF_CIRCLE, QUARTER_CIRCLE, /* half visual arc, half vertical visual arc */
		30*WORLD_ONE, 15*WORLD_ONE, /* visual range, dark visual range */
		_intelligence_low, /* intelligence */
		_speed_blinding, /* speed */
		NORMAL_MONSTER_GRAVITY, NORMAL_MONSTER_TERMINAL_VELOCITY, /* gravity, terminal velocity */
		_normal_door_retry_mask, /* door retry mask */
		NONE, {NONE, _alien_damage, 30, 10, FIXED_ONE}, /* shrapnel radius, shrapnel damage */
		
		0, /* being hit */
		3, 2, /* dying hard (popping), dying soft (falling) */
		4, UNONE, /* hard dead frames, soft dead frames */
		0, 0, /* stationary shape, moving shape (no permutations) */
		0, 0, /* teleport in shape, teleport out shape */
		
		3*TICKS_PER_SECOND, /* attack frequency (for both melee and ranged attacks) */
		
		/* melee attack */
		{
			NONE, /* melee attack type */
		},
		
		/* ranged attack */
		{
			_projectile_minor_hummer, /* ranged attack type */
			0, /* repetitions */
			3, /* error angle */
			12*WORLD_ONE, /* range */
			1, /* ranged attack shape */
			
			0, 0, 0, /* dx, dy, dz */
		}
	},

	{ /* _monster_hummer_big_major (angry hummer) */
		BUILD_COLLECTION(_collection_hummer, 3), /* shape collection */
		60, 0, FLAG(_damage_fusion_bolt)|FLAG(_damage_compiler_bolt)|FLAG(_damage_electrical_staff), /* vitality, immunities, weaknesses */
		_monster_is_alien|_monster_flys|_monster_major|_monster_has_delayed_hard_death, /* flags */
		
		_class_hummer, /* class */
		_class_pfhor|_class_client, /* friends */
		_class_human|_class_native|_class_defender, /* enemies */
		
		_higher_frequency, /* sound pitch */
		_snd_hummer_activate, NONE, NONE, NONE, NONE, NONE, /* sounds: activation, friendly activation, clear, kill, apology, friendly-fire */
		NONE, /* flaming death sound */
		NONE, 0, /* random sound, random sound mask */

		NONE, /* carrying item type */
	
		WORLD_ONE/4, WORLD_ONE_HALF, /* radius, height */
		WORLD_ONE_FOURTH, /* preferred hover height */
		-5*WORLD_ONE, 5*WORLD_ONE, /* minimum ledge delta, maximum ledge delta */
		FIXED_ONE, /* external velocity scale */
		_effect_hummer_spark, _effect_metallic_clang, _effect_rocket_contrail, /* impact effect, melee impact effect, contrail effect */
	
		HALF_CIRCLE, QUARTER_CIRCLE, /* half visual arc, half vertical visual arc */
		30*WORLD_ONE, 15*WORLD_ONE, /* visual range, dark visual range */
		_intelligence_low, /* intelligence */
		_speed_blinding, /* speed */
		NORMAL_MONSTER_GRAVITY, NORMAL_MONSTER_TERMINAL_VELOCITY, /* gravity, terminal velocity */
		_normal_door_retry_mask, /* door retry mask */
		NONE, {NONE, _alien_damage, 30, 10, FIXED_ONE}, /* shrapnel radius, shrapnel damage */
		
		0, /* being hit */
		3, 2, /* dying hard (popping), dying soft (falling) */
		4, UNONE, /* hard dead frames, soft dead frames */
		0, 0, /* stationary shape, moving shape (no permutations) */
		0, 0, /* teleport in shape, teleport out shape */
		
		2*TICKS_PER_SECOND, /* attack frequency (for both melee and ranged attacks) */
		
		/* melee attack */
		{
			NONE, /* melee attack type */
		},
		
		/* ranged attack */
		{
			_projectile_minor_hummer, /* ranged attack type */
			2, /* repetitions */
			5, /* error angle */
			12*WORLD_ONE, /* range */
			1, /* ranged attack shape */
			
			0, 0, 0, /* dx, dy, dz */
		}
	},

	{ /* _monster_hummer_possessed (hummer from durandal) */
		BUILD_COLLECTION(_collection_hummer, 4), /* shape collection */
		60, 0, FLAG(_damage_fusion_bolt)|FLAG(_damage_compiler_bolt)|FLAG(_damage_electrical_staff), /* vitality, immunities, weaknesses */
		_monster_flys|_monster_has_delayed_hard_death|_monster_attacks_immediately, /* flags */
		
		_class_possessed_hummer, /* class */
		_class_human, /* friends */
		_class_pfhor|_class_client|_class_native, /* enemies */
		
		_lower_frequency, /* sound pitch */
		_snd_hummer_activate, NONE, NONE, NONE, NONE, NONE, /* sounds: activation, friendly activation, clear, kill, apology, friendly-fire */
		NONE, /* flaming death sound */
		NONE, 0, /* random sound, random sound mask */

		NONE, /* carrying item type */
	
		WORLD_ONE/4, WORLD_ONE_HALF, /* radius, height */
		WORLD_ONE_FOURTH, /* preferred hover height */
		-5*WORLD_ONE, 5*WORLD_ONE, /* minimum ledge delta, maximum ledge delta */
		FIXED_ONE, /* external velocity scale */
		_effect_hummer_spark, _effect_metallic_clang, _effect_rocket_contrail, /* impact effect, melee impact effect, contrail effect */
	
		HALF_CIRCLE, QUARTER_CIRCLE, /* half visual arc, half vertical visual arc */
		30*WORLD_ONE, 15*WORLD_ONE, /* visual range, dark visual range */
		_intelligence_low, /* intelligence */
		_speed_blinding, /* speed */
		NORMAL_MONSTER_GRAVITY, NORMAL_MONSTER_TERMINAL_VELOCITY, /* gravity, terminal velocity */
		_normal_door_retry_mask, /* door retry mask */
		NONE, {NONE, _alien_damage, 30, 10, FIXED_ONE}, /* shrapnel radius, shrapnel damage */
		
		0, /* being hit */
		3, 2, /* dying hard (popping), dying soft (falling) */
		4, UNONE, /* hard dead frames, soft dead frames */
		0, 0, /* stationary shape, moving shape (no permutations) */
		0, 0, /* teleport in shape, teleport out shape */
		
		2*TICKS_PER_SECOND, /* attack frequency (for both melee and ranged attacks) */
		
		/* melee attack */
		{
			NONE, /* melee attack type */
		},
		
		/* ranged attack */
		{
			_projectile_durandal_hummer, /* ranged attack type */
			1, /* repetitions */
			5, /* error angle */
			12*WORLD_ONE, /* range */
			1, /* ranged attack shape */
			
			0, 0, 0, /* dx, dy, dz */
		}
	},

	{ /* _monster_cyborg_minor */
		BUILD_COLLECTION(_collection_cyborg, 0), /* shape collection */
		300, 0, FLAG(_damage_fusion_bolt), /* vitality, immunities, weaknesses */
		_monster_is_alien|_monster_minor|_monster_uses_sniper_ledges, /* flags */
		
		_class_cyborg, /* class */
		_class_cyborg, /* friends */
		_class_human|_class_native|_class_defender, /* enemies */
	
		_normal_frequency, /* sound pitch */	
		NONE, NONE, NONE, NONE, NONE, NONE, /* sounds: activation, friendly activation, clear, kill, apology, friendly-fire */
		NONE, /* flaming dying sound */
		NONE, 0, /* random sound, random sound mask */

		NONE, /* carrying item type */
	
		WORLD_ONE/4, (4*WORLD_ONE)/5, /* radius, height */
		0, /* preferred hover height */
		-WORLD_ONE, WORLD_ONE/4, /* minimum ledge delta, maximum ledge delta */
		FIXED_ONE/4, /* external velocity scale */
		_effect_cyborg_blood_splash, NONE, NONE, /* impact effect, melee impact effect, contrail effect */
	
		QUARTER_CIRCLE, QUARTER_CIRCLE/3, /* half visual arc, half vertical visual arc */
		30*WORLD_ONE, 3*WORLD_ONE, /* visual range, dark visual range */
		_intelligence_high, /* intelligence */
		_speed_superfast5, /* speed */
		NORMAL_MONSTER_GRAVITY, NORMAL_MONSTER_TERMINAL_VELOCITY, /* gravity, terminal velocity */
		_vidmaster_door_retry_mask, /* door retry mask */
		WORLD_ONE, {_damage_explosion, _alien_damage, 60, 0, FIXED_ONE}, /* shrapnel radius, shrapnel damage */
		
		0, /* being hit */
		3, UNONE, /* dying hard (popping), dying soft (falling) */
		5, UNONE, /* hard dead frames, soft dead frames */
		0, 1, /* stationary shape, moving shape */
		0, 0, /* teleport in shape, teleport out shape */
		
		4*TICKS_PER_SECOND, /* attack frequency */
		
		/* melee attack */
		{
			NONE,
		},
		
		/* ranged attack */
		{
			_projectile_minor_cyborg_ball, /* ranged attack type */
			0, /* repetitions */
			0, /* error angle */
			10*WORLD_ONE, /* range */
			2, /* ranged attack shape */
			
			WORLD_ONE/16, 0, WORLD_ONE_HALF+WORLD_ONE_FOURTH/2, /* dx, dy, dz */
		}
	},
	
	{ /* _monster_cyborg_major */
		BUILD_COLLECTION(_collection_cyborg, 1), /* shape collection */
		450, 0, FLAG(_damage_fusion_bolt), /* vitality, immunities, weaknesses */
		_monster_is_alien|_monster_major|_monster_uses_sniper_ledges, /* flags */
		
		_class_cyborg, /* class */
		_class_cyborg, /* friends */
		_class_human|_class_native|_class_defender, /* enemies */
	
		_normal_frequency, /* sound pitch */	
		NONE, NONE, NONE, NONE, NONE, NONE, /* sounds: activation, friendly activation, clear, kill, apology, friendly-fire */
		NONE, /* flaming dying sound */
		NONE, 0, /* random sound, random sound mask */

		NONE, /* carrying item type */
	
		WORLD_ONE/4, (4*WORLD_ONE)/5, /* radius, height */
		0, /* preferred hover height */
		-WORLD_ONE, WORLD_ONE/4, /* minimum ledge delta, maximum ledge delta */
		FIXED_ONE/4, /* external velocity scale */
		_effect_cyborg_blood_splash, NONE, NONE, /* impact effect, melee impact effect, contrail effect */
	
		QUARTER_CIRCLE, QUARTER_CIRCLE/3, /* half visual arc, half vertical visual arc */
		30*WORLD_ONE, 3*WORLD_ONE, /* visual range, dark visual range */
		_intelligence_high, /* intelligence */
		_speed_superfast4, /* speed */
		NORMAL_MONSTER_GRAVITY, NORMAL_MONSTER_TERMINAL_VELOCITY, /* gravity, terminal velocity */
		_vidmaster_door_retry_mask, /* door retry mask */
		WORLD_ONE, {_damage_explosion, _alien_damage, 60, 0, FIXED_ONE}, /* shrapnel radius, shrapnel damage */
		
		0, /* being hit */
		3, UNONE, /* dying hard (popping), dying soft (falling) */
		5, UNONE, /* hard dead frames, soft dead frames */
		0, 1, /* stationary shape, moving shape */
		0, 0, /* teleport in shape, teleport out shape */
		
		3*TICKS_PER_SECOND, /* attack frequency */
		
		/* melee attack */
		{
			NONE,
		},
		
		/* ranged attack */
		{
			_projectile_minor_cyborg_ball, /* ranged attack type */
			1, /* repetitions */
			0, /* error angle */
			10*WORLD_ONE, /* range */
			2, /* ranged attack shape */
			
			WORLD_ONE/16, 0, WORLD_ONE_HALF+WORLD_ONE_FOURTH/2, /* dx, dy, dz */
		}
	},
	
	{ /* _monster_cyborg_flame_minor */
		BUILD_COLLECTION(_collection_cyborg, 0), /* shape collection */
		300, 0, FLAG(_damage_fusion_bolt), /* vitality, immunities, weaknesses */
		_monster_is_alien|_monster_minor|_monster_uses_sniper_ledges, /* flags */
		
		_class_cyborg, /* class */
		_class_cyborg, /* friends */
		_class_human|_class_native|_class_defender, /* enemies */
	
		_lower_frequency, /* sound pitch */	
		NONE, NONE, NONE, NONE, NONE, NONE, /* sounds: activation, friendly activation, clear, kill, apology, friendly-fire */
		NONE, /* flaming dying sound */
		NONE, 0, /* random sound, random sound mask */

		NONE, /* carrying item type */
	
		WORLD_ONE/4, (4*WORLD_ONE)/5, /* radius, height */
		0, /* preferred hover height */
		-WORLD_ONE, WORLD_ONE/4, /* minimum ledge delta, maximum ledge delta */
		FIXED_ONE/4, /* external velocity scale */
		_effect_cyborg_blood_splash, NONE, NONE, /* impact effect, melee impact effect, contrail effect */
	
		QUARTER_CIRCLE, QUARTER_CIRCLE/3, /* half visual arc, half vertical visual arc */
		30*WORLD_ONE, 3*WORLD_ONE, /* visual range, dark visual range */
		_intelligence_high, /* intelligence */
		_speed_superfast4, /* speed */
		NORMAL_MONSTER_GRAVITY, NORMAL_MONSTER_TERMINAL_VELOCITY, /* gravity, terminal velocity */
		_vidmaster_door_retry_mask, /* door retry mask */
		WORLD_ONE, {_damage_explosion, _alien_damage, 60, 0, FIXED_ONE}, /* shrapnel radius, shrapnel damage */
		
		0, /* being hit */
		3, UNONE, /* dying hard (popping), dying soft (falling) */
		5, UNONE, /* hard dead frames, soft dead frames */
		0, 1, /* stationary shape, moving shape */
		0, 0, /* teleport in shape, teleport out shape */
		
		4*TICKS_PER_SECOND, /* attack frequency */
		
		/* melee attack */
		{
			_projectile_flamethrower_burst, /* ranged attack type */
			15, /* repetitions */
			0, /* error angle */
			2*WORLD_ONE, /* range */
			4, /* ranged attack shape */
			
			WORLD_ONE/16, 0, WORLD_ONE_HALF+WORLD_ONE_FOURTH/2, /* dx, dy, dz */
		},
		
		/* ranged attack */
		{
			_projectile_major_cyborg_ball, /* ranged attack type */
			0, /* repetitions */
			0, /* error angle */
			10*WORLD_ONE, /* range */
			2, /* ranged attack shape */
			
			WORLD_ONE/16, 0, WORLD_ONE_HALF+WORLD_ONE_FOURTH/2, /* dx, dy, dz */
		}
	},
	
	{ /* _monster_cyborg_flame_major */
		BUILD_COLLECTION(_collection_cyborg, 0), /* shape collection */
		450, 0, FLAG(_damage_fusion_bolt), /* vitality, immunities, weaknesses */
		_monster_is_alien|_monster_major|_monster_uses_sniper_ledges, /* flags */
		
		_class_cyborg, /* class */
		_class_cyborg, /* friends */
		_class_human|_class_native|_class_defender, /* enemies */
	
		_lower_frequency, /* sound pitch */	
		NONE, NONE, NONE, NONE, NONE, NONE, /* sounds: activation, friendly activation, clear, kill, apology, friendly-fire */
		NONE, /* flaming dying sound */
		NONE, 0, /* random sound, random sound mask */

		NONE, /* carrying item type */
	
		WORLD_ONE/4, (4*WORLD_ONE)/5, /* radius, height */
		0, /* preferred hover height */
		-WORLD_ONE, WORLD_ONE/4, /* minimum ledge delta, maximum ledge delta */
		FIXED_ONE/4, /* external velocity scale */
		_effect_cyborg_blood_splash, NONE, NONE, /* impact effect, melee impact effect, contrail effect */
	
		QUARTER_CIRCLE, QUARTER_CIRCLE/3, /* half visual arc, half vertical visual arc */
		30*WORLD_ONE, 3*WORLD_ONE, /* visual range, dark visual range */
		_intelligence_high, /* intelligence */
		_speed_superfast4, /* speed */
		NORMAL_MONSTER_GRAVITY, NORMAL_MONSTER_TERMINAL_VELOCITY, /* gravity, terminal velocity */
		_vidmaster_door_retry_mask, /* door retry mask */
		WORLD_ONE, {_damage_explosion, _alien_damage, 60, 0, FIXED_ONE}, /* shrapnel radius, shrapnel damage */
		
		0, /* being hit */
		3, UNONE, /* dying hard (popping), dying soft (falling) */
		5, UNONE, /* hard dead frames, soft dead frames */
		0, 1, /* stationary shape, moving shape */
		0, 0, /* teleport in shape, teleport out shape */
		
		3*TICKS_PER_SECOND, /* attack frequency */
		
		/* melee attack */
		{
			_projectile_flamethrower_burst, /* ranged attack type */
			15, /* repetitions */
			0, /* error angle */
			2*WORLD_ONE, /* range */
			4, /* ranged attack shape */
			
			WORLD_ONE/16, 0, WORLD_ONE_HALF+WORLD_ONE_FOURTH/2, /* dx, dy, dz */
		},
		
		/* ranged attack */
		{
			_projectile_major_cyborg_ball, /* ranged attack type */
			0, /* repetitions */
			0, /* error angle */
			10*WORLD_ONE, /* range */
			2, /* ranged attack shape */
			
			WORLD_ONE/16, 0, WORLD_ONE_HALF+WORLD_ONE_FOURTH/2, /* dx, dy, dz */
		}
	},
	
	{ /* _monster_enforcer_minor */
		BUILD_COLLECTION(_collection_enforcer, 0), /* shape collection */
		120, 0, 0, /* vitality, immunities, weaknesses */
		_monster_is_alien|_monster_minor|_monster_uses_sniper_ledges|_monster_can_die_in_flames|_monster_waits_with_clear_shot, /* flags */
		
		_class_enforcer, /* class */
		_class_pfhor, /* friends */
		_class_human|_class_native|_class_defender, /* enemies */
	
		_normal_frequency, /* sound pitch */	
		_snd_enforcer_activate, NONE, NONE, NONE, NONE, NONE, /* sounds: activation, friendly activation, clear, kill, apology, friendly-fire */
		_snd_fighter_wail, /* flaming dying sound */
		NONE, 0, /* random sound, random sound mask */

		_i_alien_shotgun, /* carrying item type */
	
		WORLD_ONE/5, (4*WORLD_ONE)/5, /* radius, height */
		0, /* preferred hover height */
		-2*WORLD_ONE, WORLD_ONE/3, /* minimum ledge delta, maximum ledge delta */
		(3*FIXED_ONE)/4, /* external velocity scale */
		_effect_enforcer_blood_splash, NONE, NONE, /* impact effect, melee impact effect, contrail effect */
	
		QUARTER_CIRCLE, QUARTER_CIRCLE/3, /* half visual arc, half vertical visual arc */
		30*WORLD_ONE, 3*WORLD_ONE, /* visual range, dark visual range */
		_intelligence_high, /* intelligence */
		_speed_superfast4, /* speed */
		NORMAL_MONSTER_GRAVITY, NORMAL_MONSTER_TERMINAL_VELOCITY, /* gravity, terminal velocity */
		_vidmaster_door_retry_mask, /* door retry mask */
		NONE, {NONE, 0, 0, 0}, /* shrapnel radius, shrapnel damage */
		
		0, /* being hit */
		6, 3, /* dying hard (popping), dying soft (falling) */
		7, 4, /* hard dead frames, soft dead frames */
		0, 1, /* stationary shape, moving shape */
		0, 0, /* teleport in shape, teleport out shape */
		
		4*TICKS_PER_SECOND, /* attack frequency */
		
		/* melee attack */
		{
			NONE, /* melee attack type */
		},
		
		/* ranged attack */
		{
			_projectile_alien_weapon, /* ranged attack type */
			8, /* repetitions */
			2, /* error angle */
			15*WORLD_ONE, /* range */
			2, /* ranged attack shape */
			
			WORLD_ONE/16, 0, WORLD_ONE_HALF+WORLD_ONE_FOURTH/2, /* dx, dy, dz */
		}
	},
	
	{ /* _monster_enforcer_major */
		BUILD_COLLECTION(_collection_enforcer, 1), /* shape collection */
		160, 0, 0, /* vitality, immunities, weaknesses */
		_monster_is_alien|_monster_major|_monster_uses_sniper_ledges|_monster_can_die_in_flames|_monster_waits_with_clear_shot, /* flags */
	
		_class_enforcer, /* class */
		_class_pfhor, /* friends */
		_class_human|_class_native|_class_defender, /* enemies */
	
		_higher_frequency, /* sound pitch */	
		_snd_enforcer_activate, NONE, NONE, NONE, NONE, NONE, /* sounds: activation, friendly activation, clear, kill, apology, friendly-fire */
		_snd_fighter_wail, /* flaming dying sound */
		NONE, 0, /* random sound, random sound mask */

		_i_alien_shotgun, /* carrying item type */
	
		WORLD_ONE/5, (4*WORLD_ONE)/5, /* radius, height */
		0, /* preferred hover height */
		-2*WORLD_ONE, WORLD_ONE/3, /* minimum ledge delta, maximum ledge delta */
		(3*FIXED_ONE)/4, /* external velocity scale */
		_effect_enforcer_blood_splash, NONE, NONE, /* impact effect, melee impact effect, contrail effect */
	
		QUARTER_CIRCLE, QUARTER_CIRCLE/3, /* half visual arc, half vertical visual arc */
		30*WORLD_ONE, 3*WORLD_ONE, /* visual range, dark visual range */
		_intelligence_high, /* intelligence */
		_speed_blinding, /* speed */
		NORMAL_MONSTER_GRAVITY, NORMAL_MONSTER_TERMINAL_VELOCITY, /* gravity, terminal velocity */
		_vidmaster_door_retry_mask, /* door retry mask */
		NONE, {NONE, 0, 0, 0}, /* shrapnel radius, shrapnel damage */
		
		0, /* being hit */
		6, 3, /* dying hard (popping), dying soft (falling) */
		7, 4, /* hard dead frames, soft dead frames */
		0, 1, /* stationary shape, moving shape */
		0, 0, /* teleport in shape, teleport out shape */
		
		2*TICKS_PER_SECOND, /* attack frequency */
		
		/* melee attack */
		{
			NONE, /* melee attack type */
		},
		
		/* ranged attack */
		{
			_projectile_alien_weapon, /* ranged attack type */
			12, /* repetitions */
			5, /* error angle */
			20*WORLD_ONE, /* range */
			2, /* ranged attack shape */
			
			WORLD_ONE/16, 0, WORLD_ONE_HALF+WORLD_ONE_FOURTH/2, /* dx, dy, dz */
		}
	},
	
	{ /* _monster_hunter_minor */
		BUILD_COLLECTION(_collection_hunter, 0), /* shape collection */
		200, FLAG(_damage_flame), FLAG(_damage_fusion_bolt), /* vitality, immunities, weaknesses */
		_monster_is_alien|_monster_uses_sniper_ledges|_monster_minor, /* flags */

		_class_hunter, /* class */
		_class_pfhor, /* friends */
		_class_human|_class_native|_class_defender, /* enemies */
	
		_normal_frequency, /* sound pitch */	
		NONE, NONE, NONE, NONE, NONE, NONE, /* sounds: activation, friendly activation, clear, kill, apology, friendly-fire */
		NONE, /* flaming death sound */
		NONE, 0, /* random sound, random sound mask */

		NONE, /* carrying item type */
	
		WORLD_ONE/5, (4*WORLD_ONE)/5, /* radius, height */
		0, /* preferred hover height */
		-WORLD_ONE, WORLD_ONE/3, /* minimum ledge delta, maximum ledge delta */
		FIXED_ONE_HALF, /* external velocity scale */
		_effect_hunter_spark, _effect_metallic_clang, NONE, /* impact effect, melee impact effect, contrail effect */
	
		QUARTER_CIRCLE, QUARTER_CIRCLE/3, /* half visual arc, half vertical visual arc */
		30*WORLD_ONE, 4*WORLD_ONE, /* visual range, dark visual range */
		_intelligence_high, /* intelligence */
		_speed_superfast3, /* speed */
		NORMAL_MONSTER_GRAVITY, NORMAL_MONSTER_TERMINAL_VELOCITY, /* gravity, terminal velocity */
		_normal_door_retry_mask, /* door retry mask */
		2*WORLD_ONE, {_damage_explosion, _alien_damage, 60, 30, FIXED_ONE}, /* shrapnel radius, shrapnel damage */
		
		7, /* being hit */
		3, 9, /* dying hard (popping), dying soft (falling) */
		6, 10, /* hard dead frames, soft dead frames */
		1, 0, /* stationary shape, moving shape (no permutations) */
		1, 1, /* teleport in shape, teleport out shape */
		
		2*TICKS_PER_SECOND, /* attack frequency (for both melee and ranged attacks) */
		
		/* melee attack */
		{
			NONE, /* melee attack type */
		},
		
		/* ranged attack */
		{
			_projectile_hunter, /* ranged attack type */
			2, /* repetitions */
			3, /* error angle */
			12*WORLD_ONE, /* range */
			2, /* ranged attack shape */
			
			0, WORLD_ONE/8, WORLD_ONE_HALF+WORLD_ONE_FOURTH, /* dx, dy, dz */
		}
	},
	
	{ /* _monster_hunter_major */
		BUILD_COLLECTION(_collection_hunter, 1), /* shape collection */
		300, FLAG(_damage_flame), FLAG(_damage_fusion_bolt), /* vitality, immunities, weaknesses */
		_monster_is_alien|_monster_uses_sniper_ledges|_monster_major, /* flags */

		_class_hunter, /* class */
		_class_pfhor, /* friends */
		_class_human|_class_native|_class_defender, /* enemies */
	
		_higher_frequency, /* sound pitch */	
		NONE, NONE, NONE, NONE, NONE, NONE, /* sounds: activation, friendly activation, clear, kill, apology, friendly-fire */
		NONE, /* flaming death sound */
		NONE, 0, /* random sound, random sound mask */

		NONE, /* carrying item type */
	
		WORLD_ONE/5, (4*WORLD_ONE)/5, /* radius, height */
		0, /* preferred hover height */
		-WORLD_ONE, WORLD_ONE/3, /* minimum ledge delta, maximum ledge delta */
		FIXED_ONE_HALF, /* external velocity scale */
		_effect_hunter_spark, _effect_metallic_clang, NONE, /* impact effect, melee impact effect, contrail effect */
	
		QUARTER_CIRCLE, QUARTER_CIRCLE/3, /* half visual arc, half vertical visual arc */
		30*WORLD_ONE, 4*WORLD_ONE, /* visual range, dark visual range */
		_intelligence_high, /* intelligence */
		_speed_superfast5, /* speed */
		NORMAL_MONSTER_GRAVITY, NORMAL_MONSTER_TERMINAL_VELOCITY, /* gravity, terminal velocity */
		_normal_door_retry_mask, /* door retry mask */
		2*WORLD_ONE, {_damage_explosion, _alien_damage, 60, 30, FIXED_ONE}, /* shrapnel radius, shrapnel damage */
		
		7, /* being hit */
		3, 9, /* dying hard (popping), dying soft (falling) */
		6, 10, /* hard dead frames, soft dead frames */
		1, 0, /* stationary shape, moving shape (no permutations) */
		1, 1, /* teleport in shape, teleport out shape */
		
		2*TICKS_PER_SECOND, /* attack frequency (for both melee and ranged attacks) */
		
		/* melee attack */
		{
			NONE, /* melee attack type */
		},
		
		/* ranged attack */
		{
			_projectile_hunter, /* ranged attack type */
			5, /* repetitions */
			3, /* error angle */
			12*WORLD_ONE, /* range */
			2, /* ranged attack shape */
			
			0, WORLD_ONE/8, WORLD_ONE_HALF+WORLD_ONE_FOURTH, /* dx, dy, dz */
		}
	},

	{ /* _monster_trooper_minor */
		BUILD_COLLECTION(_collection_trooper, 0), /* shape collection */
		110, 0, 0, /* vitality, immunities, weaknesses */
		_monster_is_alien|_monster_minor|_monster_uses_sniper_ledges|_monster_is_berserker|_monster_can_die_in_flames, /* flags */
		
		_class_trooper, /* class */
		_class_pfhor, /* friends */
		_class_human|_class_native|_class_defender, /* enemies */
	
		_normal_frequency, /* sound pitch */	
		_snd_fighter_activate, NONE, NONE, NONE, NONE, NONE, /* sounds: activation, friendly activation, clear, kill, apology, friendly-fire */
		_snd_fighter_wail, /* dying flaming */
		_snd_fighter_chatter, 15, /* random sound, random sound mask */

		NONE, /* carrying item type */
	
		WORLD_ONE/5, (4*WORLD_ONE)/5, /* radius, height */
		0, /* preferred hover height */
		-4*WORLD_ONE, WORLD_ONE/3, /* minimum ledge delta, maximum ledge delta */
		(3*FIXED_ONE)/4, /* external velocity scale */
		_effect_trooper_blood_splash, NONE, NONE, /* impact effect, melee impact effect, contrail effect */
	
		QUARTER_CIRCLE, QUARTER_CIRCLE/3, /* half visual arc, half vertical visual arc */
		30*WORLD_ONE, WORLD_ONE, /* visual range, dark visual range */
		_intelligence_high, /* intelligence */
		_speed_superfast3, /* speed */
		NORMAL_MONSTER_GRAVITY, NORMAL_MONSTER_TERMINAL_VELOCITY, /* gravity, terminal velocity */
		_normal_door_retry_mask, /* door retry mask */
		NONE, {NONE, 0, 0, 0}, /* shrapnel radius, shrapnel damage */
		
		4, /* being hit */
		UNONE, 3, /* dying hard (popping), dying soft (falling) */
		0, 7, /* hard dead frames, soft dead frames */
		1, 0, /* stationary shape, moving shape */
		1, 1, /* teleport in shape, teleport out shape */
		
		4*TICKS_PER_SECOND, /* attack frequency (for both melee and ranged attacks) */
		
		/* melee attack */
		{
			_projectile_trooper_bullet, /* melee attack type */
			3, /* repetitions */
			30, /* error */
			3*WORLD_ONE, /* range */
			
			2, /* melee attack shape */

			0, -WORLD_ONE/10, WORLD_ONE_HALF-WORLD_ONE_FOURTH/4, /* dx, dy, dz */
		},
		
		/* ranged attack */
		{
			_projectile_trooper_grenade, /* ranged attack type */
			0, /* repetitions */
			10, /* error angle */
			10*WORLD_ONE, /* range */
			9, /* ranged attack shape */
			
			-WORLD_ONE/10, WORLD_ONE/8, WORLD_ONE_HALF-WORLD_ONE_FOURTH/8, /* dx, dy, dz */
		}
	},

	{ /* _monster_trooper_major */
		BUILD_COLLECTION(_collection_trooper, 1), /* shape collection */
		200, 0, 0, /* vitality, immunities, weaknesses */
		_monster_is_alien|_monster_major|_monster_uses_sniper_ledges|_monster_is_berserker|_monster_can_die_in_flames, /* flags */
		
		_class_trooper, /* class */
		_class_pfhor, /* friends */
		_class_human|_class_native|_class_defender, /* enemies */
	
		_lower_frequency, /* sound pitch */	
		_snd_fighter_activate, NONE, NONE, NONE, NONE, NONE, /* sounds: activation, friendly activation, clear, kill, apology, friendly-fire */
		_snd_fighter_wail, /* dying flaming */
		_snd_fighter_chatter, 15, /* random sound, random sound mask */

		NONE, /* carrying item type */
	
		WORLD_ONE/5, (4*WORLD_ONE)/5, /* radius, height */
		0, /* preferred hover height */
		-4*WORLD_ONE, WORLD_ONE/3, /* minimum ledge delta, maximum ledge delta */
		(3*FIXED_ONE)/4, /* external velocity scale */
		_effect_trooper_blood_splash, NONE, NONE, /* impact effect, melee impact effect, contrail effect */
	
		QUARTER_CIRCLE, QUARTER_CIRCLE/3, /* half visual arc, half vertical visual arc */
		30*WORLD_ONE, WORLD_ONE, /* visual range, dark visual range */
		_intelligence_high, /* intelligence */
		_speed_superfast3, /* speed */
		NORMAL_MONSTER_GRAVITY, NORMAL_MONSTER_TERMINAL_VELOCITY, /* gravity, terminal velocity */
		_fast_door_retry_mask, /* door retry mask */
		NONE, {NONE, 0, 0, 0}, /* shrapnel radius, shrapnel damage */
		
		4, /* being hit */
		UNONE, 3, /* dying hard (popping), dying soft (falling) */
		0, 7, /* hard dead frames, soft dead frames */
		1, 0, /* stationary shape, moving shape */
		1, 1, /* teleport in shape, teleport out shape */
		
		2*TICKS_PER_SECOND, /* attack frequency (for both melee and ranged attacks) */
		
		/* melee attack */
		{
			_projectile_trooper_bullet, /* melee attack type */
			8, /* repetitions */
			10, /* error */
			3*WORLD_ONE, /* range */
			
			2, /* melee attack shape */

			-WORLD_ONE/10, WORLD_ONE/8, WORLD_ONE_HALF-WORLD_ONE_FOURTH/8, /* dx, dy, dz */
		},
		
		/* ranged attack */
		{
			_projectile_trooper_grenade, /* ranged attack type */
			1, /* repetitions */
			5, /* error angle */
			12*WORLD_ONE, /* range */
			9, /* ranged attack shape */
			
			0, -WORLD_ONE/10, WORLD_ONE_HALF-WORLD_ONE_FOURTH/4, /* dx, dy, dz */
		}
	},

	{ /* _monster_mother_of_all_cyborgs */
		BUILD_COLLECTION(_collection_cyborg, 0), /* shape collection */
		1500, 0, FLAG(_damage_fusion_bolt), /* vitality, immunities, weaknesses */
		_monster_is_enlarged|_monster_is_alien|_monster_cannot_be_dropped|_monster_uses_sniper_ledges, /* flags */
		
		_class_cyborg, /* class */
		0, /* friends */
		_class_human|_class_native|_class_defender, /* enemies */
	
		_lower_frequency, /* sound pitch */	
		NONE, NONE, NONE, NONE, NONE, NONE, /* sounds: activation, friendly activation, clear, kill, apology, friendly-fire */
		NONE, /* flaming dying sound */
		NONE, 0, /* random sound, random sound mask */

		NONE, /* carrying item type */
	
		WORLD_ONE/3, WORLD_ONE + WORLD_ONE/5, /* radius, height */
		0, /* preferred hover height */
		-WORLD_ONE, WORLD_ONE/4, /* minimum ledge delta, maximum ledge delta */
		FIXED_ONE/4, /* external velocity scale */
		_effect_cyborg_blood_splash, NONE, NONE, /* impact effect, melee impact effect, contrail effect */
	
		QUARTER_CIRCLE, QUARTER_CIRCLE/3, /* half visual arc, half vertical visual arc */
		30*WORLD_ONE, 3*WORLD_ONE, /* visual range, dark visual range */
		_intelligence_high, /* intelligence */
		_speed_superfast4, /* speed */
		NORMAL_MONSTER_GRAVITY, NORMAL_MONSTER_TERMINAL_VELOCITY, /* gravity, terminal velocity */
		_vidmaster_door_retry_mask, /* door retry mask */
		3*WORLD_ONE, {_damage_explosion, _alien_damage, 140, 40, FIXED_ONE}, /* shrapnel radius, shrapnel damage */
		
		0, /* being hit */
		3, UNONE, /* dying hard (popping), dying soft (falling) */
		5, UNONE, /* hard dead frames, soft dead frames */
		0, 1, /* stationary shape, moving shape */
		0, 0, /* teleport in shape, teleport out shape */
		
		2*TICKS_PER_SECOND, /* attack frequency */
		
		/* melee attack */
		{
			_projectile_flamethrower_burst, /* ranged attack type */
			15, /* repetitions */
			0, /* error angle */
			2*WORLD_ONE, /* range */
			4, /* ranged attack shape */
			
			WORLD_ONE/16, 0, WORLD_ONE_HALF+WORLD_ONE_FOURTH, /* dx, dy, dz */
		},
		
		/* ranged attack */
		{
			_projectile_major_cyborg_ball, /* ranged attack type */
			0, /* repetitions */
			0, /* error angle */
			10*WORLD_ONE, /* range */
			2, /* ranged attack shape */
			
			WORLD_ONE/16, 0, WORLD_ONE_HALF+WORLD_ONE_FOURTH, /* dx, dy, dz */
		}
	},
	
	{ /* _monster_mother_of_all_hunters */
		BUILD_COLLECTION(_collection_hunter, 2), /* shape collection */
		1500, FLAG(_damage_flame), FLAG(_damage_fusion_bolt), /* vitality, immunities, weaknesses */
		_monster_is_alien|_monster_is_enlarged|_monster_uses_sniper_ledges|_monster_cannot_be_dropped, /* flags */

		_class_hunter, /* class */
		_class_pfhor, /* friends */
		_class_human|_class_native|_class_defender, /* enemies */
	
		_lower_frequency, /* sound pitch */	
		NONE, NONE, NONE, NONE, NONE, NONE, /* sounds: activation, friendly activation, clear, kill, apology, friendly-fire */
		NONE, /* flaming death sound */
		NONE, 0, /* random sound, random sound mask */

		NONE, /* carrying item type */
	
		WORLD_ONE/4, WORLD_ONE+WORLD_ONE/6, /* radius, height */
		0, /* preferred hover height */
		-WORLD_ONE, WORLD_ONE/3, /* minimum ledge delta, maximum ledge delta */
		FIXED_ONE_HALF, /* external velocity scale */
		_effect_hunter_spark, _effect_metallic_clang, NONE, /* impact effect, melee impact effect, contrail effect */
	
		QUARTER_CIRCLE, QUARTER_CIRCLE/3, /* half visual arc, half vertical visual arc */
		30*WORLD_ONE, 4*WORLD_ONE, /* visual range, dark visual range */
		_intelligence_high, /* intelligence */
		_speed_superfast1, /* speed */
		NORMAL_MONSTER_GRAVITY, NORMAL_MONSTER_TERMINAL_VELOCITY, /* gravity, terminal velocity */
		_normal_door_retry_mask, /* door retry mask */
		4*WORLD_ONE, {_damage_explosion, _alien_damage, 140, 50, FIXED_ONE}, /* shrapnel radius, shrapnel damage */
		
		7, /* being hit */
		3, UNONE, /* dying hard (popping), dying soft (falling) */
		6, 8, /* hard dead frames, soft dead frames */
		1, 0, /* stationary shape, moving shape (no permutations) */
		1, 1, /* teleport in shape, teleport out shape */
		
		2*TICKS_PER_SECOND, /* attack frequency (for both melee and ranged attacks) */
		
		/* melee attack */
		{
			NONE, /* melee attack type */
		},
		
		/* ranged attack */
		{
			_projectile_hunter, /* ranged attack type */
			5, /* repetitions */
			3, /* error angle */
			12*WORLD_ONE, /* range */
			2, /* ranged attack shape */
			
			0, WORLD_ONE/8, WORLD_ONE_HALF+WORLD_ONE_FOURTH, /* dx, dy, dz */
		}
	},

	{ /* _monster_sewage_yeti */
		BUILD_COLLECTION(_collection_yeti, 0), /* shape collection */
		100, 0, 0, /* vitality, immunities, weaknesses */
		_monster_is_not_afraid_of_sewage|_monster_is_alien|_monster_is_berserker, /* flags */
		
		_class_yeti, /* class */
		_class_yeti, /* friends */
		_class_pfhor|_class_human|_class_defender, /* enemies */
	
		_normal_frequency, /* sound pitch */	
		NONE, NONE, NONE, NONE, NONE, NONE, /* sounds: activation, friendly activation, clear, kill, apology, friendly-fire */
		NONE, /* dying flaming */
		NONE, 15, /* random sound, random sound mask */

		NONE, /* carrying item type */
	
		WORLD_ONE/4, WORLD_ONE-1, /* radius, height */
		0, /* preferred hover height */
		-2*WORLD_ONE, WORLD_ONE/3, /* minimum ledge delta, maximum ledge delta */
		3*FIXED_ONE/4, /* external velocity scale */
		_effect_sewage_yeti_blood_splash, NONE, NONE, /* impact effect, melee impact effect, contrail effect */
	
		QUARTER_CIRCLE, QUARTER_CIRCLE/3, /* half visual arc, half vertical visual arc */
		30*WORLD_ONE, WORLD_ONE, /* visual range, dark visual range */
		_intelligence_low, /* intelligence */
		_speed_superfast5, /* speed */
		NORMAL_MONSTER_GRAVITY, NORMAL_MONSTER_TERMINAL_VELOCITY, /* gravity, terminal velocity */
		_slow_door_retry_mask, /* door retry mask */
		NONE, {NONE, 0, 0, 0}, /* shrapnel radius, shrapnel damage */
		
		12, /* being hit */
		UNONE, 3, /* dying hard (popping), dying soft (falling) */
		UNONE, 4, /* hard dead frames, soft dead frames */
		0, 1, /* stationary shape, moving shape */
		UNONE, UNONE, /* teleport in shape, teleport out shape */
		
		2*TICKS_PER_SECOND, /* attack frequency (for both melee and ranged attacks) */
		
		/* melee attack */
		{
			_projectile_yeti, /* melee attack type */
			0, /* repetitions */
			0, /* error */
			WORLD_ONE, /* range */
			13, /* melee attack shape */

			0, 0, 4*WORLD_ONE/5, /* dx, dy, dz */
		},
		
		/* ranged attack */
		{
			_projectile_sewage_yeti, /* ranged attack type */
			0, /* repetitions */
			NUMBER_OF_ANGLES/150, /* error angle */
			12*WORLD_ONE, /* range */
			2, /* ranged attack shape */
			
			WORLD_ONE/3, WORLD_ONE/6, 4*WORLD_ONE/5, /* dx, dy, dz */
		}
	},

	{ /* _monster_water_yeti */
		BUILD_COLLECTION(_collection_yeti, 1), /* shape collection */
		250, 0, 0, /* vitality, immunities, weaknesses */
		_monster_is_not_afraid_of_water|_monster_is_alien|_monster_is_berserker, /* flags */
		
		_class_yeti, /* class */
		_class_yeti, /* friends */
		_class_pfhor|_class_human|_class_defender, /* enemies */
	
		_normal_frequency, /* sound pitch */
		NONE, NONE, NONE, NONE, NONE, NONE, /* sounds: activation, friendly activation, clear, kill, apology, friendly-fire */
		NONE, /* dying flaming */
		NONE, 15, /* random sound, random sound mask */

		NONE, /* carrying item type */
	
		WORLD_ONE/4, WORLD_ONE-1, /* radius, height */
		0, /* preferred hover height */
		-2*WORLD_ONE, WORLD_ONE/3, /* minimum ledge delta, maximum ledge delta */
		3*FIXED_ONE/4, /* external velocity scale */
		_effect_water_yeti_blood_splash, NONE, NONE, /* impact effect, melee impact effect, contrail effect */
	
		QUARTER_CIRCLE, QUARTER_CIRCLE/3, /* half visual arc, half vertical visual arc */
		30*WORLD_ONE, WORLD_ONE, /* visual range, dark visual range */
		_intelligence_low, /* intelligence */
		_speed_superfast5, /* speed */
		NORMAL_MONSTER_GRAVITY, NORMAL_MONSTER_TERMINAL_VELOCITY, /* gravity, terminal velocity */
		_slow_door_retry_mask, /* door retry mask */
		NONE, {NONE, 0, 0, 0}, /* shrapnel radius, shrapnel damage */
		
		12, /* being hit */
		UNONE, 3, /* dying hard (popping), dying soft (falling) */
		UNONE, 4, /* hard dead frames, soft dead frames */
		0, 1, /* stationary shape, moving shape */
		UNONE, UNONE, /* teleport in shape, teleport out shape */
		
		TICKS_PER_SECOND/2, /* attack frequency (for both melee and ranged attacks) */
		
		/* melee attack */
		{
			_projectile_yeti, /* melee attack type */
			1, /* repetitions */
			0, /* error */
			WORLD_ONE, /* range */
			13, /* melee attack shape */

			0, WORLD_ONE/6, 4*WORLD_ONE/5, /* dx, dy, dz */
		},
		
		/* ranged attack */
		{
			NONE
		}
	},

	{ /* _monster_lava_yeti */
		BUILD_COLLECTION(_collection_yeti, 2), /* shape collection */
		200, FLAG(_damage_flame)|FLAG(_damage_alien_projectile)|FLAG(_damage_fusion_bolt)|FLAG(_damage_lava), 0, /* vitality, immunities, weaknesses */
		_monster_is_not_afraid_of_lava|_monster_is_alien|_monster_is_berserker, /* flags */
		
		_class_yeti, /* class */
		_class_yeti, /* friends */
		_class_pfhor|_class_human|_class_defender, /* enemies */
	
		_normal_frequency, /* sound pitch */	
		NONE, NONE, NONE, NONE, NONE, NONE, /* sounds: activation, friendly activation, clear, kill, apology, friendly-fire */
		NONE, /* dying flaming */
		NONE, 15, /* random sound, random sound mask */

		NONE, /* carrying item type */
	
		WORLD_ONE/4, WORLD_ONE-1, /* radius, height */
		0, /* preferred hover height */
		-2*WORLD_ONE, WORLD_ONE/3, /* minimum ledge delta, maximum ledge delta */
		3*FIXED_ONE/4, /* external velocity scale */
		_effect_lava_yeti_blood_splash, NONE, NONE, /* impact effect, melee impact effect, contrail effect */
	
		QUARTER_CIRCLE, QUARTER_CIRCLE/3, /* half visual arc, half vertical visual arc */
		30*WORLD_ONE, WORLD_ONE, /* visual range, dark visual range */
		_intelligence_low, /* intelligence */
		_speed_blinding, /* speed */
		NORMAL_MONSTER_GRAVITY, NORMAL_MONSTER_TERMINAL_VELOCITY, /* gravity, terminal velocity */
		_slow_door_retry_mask, /* door retry mask */
		NONE, {NONE, 0, 0, 0}, /* shrapnel radius, shrapnel damage */
		
		12, /* being hit */
		UNONE, 3, /* dying hard (popping), dying soft (falling) */
		UNONE, 4, /* hard dead frames, soft dead frames */
		0, 1, /* stationary shape, moving shape */
		UNONE, UNONE, /* teleport in shape, teleport out shape */
		
		TICKS_PER_SECOND, /* attack frequency (for both melee and ranged attacks) */
		
		/* melee attack */
		{
			_projectile_yeti, /* melee attack type */
			0, /* repetitions */
			0, /* error */
			WORLD_ONE, /* range */
			13, /* melee attack shape */

			0, 0, 4*WORLD_ONE/5, /* dx, dy, dz */
		},
		
		/* ranged attack */
		{
			_projectile_lava_yeti, /* ranged attack type */
			1, /* repetitions */
			NUMBER_OF_ANGLES/150, /* error angle */
			12*WORLD_ONE, /* range */
			2, /* ranged attack shape */
			
			WORLD_ONE/3, WORLD_ONE/6, 4*WORLD_ONE/5, /* dx, dy, dz */
		}
	},

	{ /* _monster_defender_minor */
		BUILD_COLLECTION(_collection_defender, 0), /* shape collection */
		160, 0, FLAG(_damage_fusion_bolt), /* vitality, immunities, weaknesses */
		_monster_flys|_monster_waits_with_clear_shot, /* flags */

		_class_defender, /* class */
		_class_defender, /* friends */
		_class_pfhor|_class_client|_class_native, /* enemies */
	
		_normal_frequency, /* sound pitch */	
		NONE, NONE, NONE, NONE, NONE, NONE, /* sounds: activation, friendly activation, clear, kill, apology, friendly-fire */
		NONE, /* flaming dying sound */
		NONE, 0, /* random sound, random sound mask */

		NONE, /* carrying item type */
	
		WORLD_ONE/5, WORLD_ONE, /* radius, height */
		WORLD_ONE/4, /* preferred hover height */
		INT16_MIN, INT16_MAX, /* minimum ledge delta, maximum ledge delta */
		FIXED_ONE, /* external velocity scale */
		NONE, NONE, NONE, /* impact effect, melee impact effect, contrail effect */
	
		QUARTER_CIRCLE, QUARTER_CIRCLE/3, /* half visual arc, half vertical visual arc */
		30*WORLD_ONE, 3*WORLD_ONE, /* visual range, dark visual range */
		_intelligence_high, /* intelligence */
		_speed_blinding, /* speed */
		NORMAL_MONSTER_GRAVITY, NORMAL_MONSTER_TERMINAL_VELOCITY, /* gravity, terminal velocity */
		_vidmaster_door_retry_mask, /* door retry mask */
		NONE, {NONE, 0, 0, 0}, /* shrapnel radius, shrapnel damage */
		
		3, /* being hit */
		UNONE, 6, /* dying hard (popping), dying soft (falling) */
		UNONE, UNONE, /* hard dead frames, soft dead frames */
		0, 0, /* stationary shape, moving shape */
		8, 8, /* teleport in shape, teleport out shape */
		
		3*TICKS_PER_SECOND, /* attack frequency */
		
		/* melee attack */
		{
			NONE, /* melee attack type */
		},
		
		/* ranged attack */
		{
			_projectile_minor_defender, /* ranged attack type */
			2, /* repetitions */
			NUMBER_OF_ANGLES/200, /* error angle */
			20*WORLD_ONE, /* range */
			2, /* ranged attack shape */
			
			WORLD_ONE/8, -WORLD_ONE/4+WORLD_ONE/10, WORLD_ONE_HALF, /* dx, dy, dz */
		}
	},

	{ /* _monster_defender_major */
		BUILD_COLLECTION(_collection_defender, 1), /* shape collection */
		240, 0, FLAG(_damage_fusion_bolt), /* vitality, immunities, weaknesses */
		_monster_flys|_monster_waits_with_clear_shot, /* flags */

		_class_defender, /* class */
		_class_defender, /* friends */
		_class_pfhor|_class_client|_class_native, /* enemies */
	
		_higher_frequency, /* sound pitch */	
		NONE, NONE, NONE, NONE, NONE, NONE, /* sounds: activation, friendly activation, clear, kill, apology, friendly-fire */
		NONE, /* flaming dying sound */
		NONE, 0, /* random sound, random sound mask */

		NONE, /* carrying item type */
	
		WORLD_ONE/5, WORLD_ONE, /* radius, height */
		WORLD_ONE/4, /* preferred hover height */
		INT16_MIN, INT16_MAX, /* minimum ledge delta, maximum ledge delta */
		FIXED_ONE, /* external velocity scale */
		NONE, NONE, NONE, /* impact effect, melee impact effect, contrail effect */
	
		QUARTER_CIRCLE, QUARTER_CIRCLE/3, /* half visual arc, half vertical visual arc */
		30*WORLD_ONE, 3*WORLD_ONE, /* visual range, dark visual range */
		_intelligence_high, /* intelligence */
		_speed_blinding, /* speed */
		NORMAL_MONSTER_GRAVITY, NORMAL_MONSTER_TERMINAL_VELOCITY, /* gravity, terminal velocity */
		_vidmaster_door_retry_mask, /* door retry mask */
		NONE, {NONE, 0, 0, 0}, /* shrapnel radius, shrapnel damage */
		
		3, /* being hit */
		UNONE, 6, /* dying hard (popping), dying soft (falling) */
		UNONE, UNONE, /* hard dead frames, soft dead frames */
		0, 0, /* stationary shape, moving shape */
		8, 8, /* teleport in shape, teleport out shape */
		
		2*TICKS_PER_SECOND, /* attack frequency */
		
		/* melee attack */
		{
			NONE, /* melee attack type */
		},
		
		/* ranged attack */
		{
			_projectile_major_defender, /* ranged attack type */
			4, /* repetitions */
			NUMBER_OF_ANGLES/100, /* error angle */
			20*WORLD_ONE, /* range */
			2, /* ranged attack shape */
			
			WORLD_ONE/8, -WORLD_ONE/4+WORLD_ONE/10, WORLD_ONE_HALF, /* dx, dy, dz */
		}
	},

	{ /* _monster_juggernaut_minor */
		BUILD_COLLECTION(_collection_juggernaut, 0), /* shape collection */
		2500, 0, FLAG(_damage_fusion_bolt), /* vitality, immunities, weaknesses */
		_monster_minor|_monster_is_alien|_monster_cant_fire_backwards|_monster_has_nuclear_hard_death|
			_monster_has_delayed_hard_death|_monster_cannot_be_dropped|_monster_fires_symmetrically|
			_monster_chooses_weapons_randomly|_monster_flys, /* flags */

		_class_juggernaut, /* class */
		_class_juggernaut, /* friends */
		_class_human|_class_client|_class_native, /* enemies */
	
		_normal_frequency, /* sound pitch */	
		NONE, NONE, NONE, NONE, NONE, NONE, /* sounds: activation, friendly activation, clear, kill, apology, friendly-fire */
		NONE, /* flaming dying sound */
		NONE, 0, /* random sound, random sound mask */

		NONE, /* carrying item type */
	
		WORLD_ONE, 2*WORLD_ONE, /* radius, height */
		WORLD_ONE, /* preferred hover height */
		INT16_MIN, INT16_MAX, /* minimum ledge delta, maximum ledge delta */
		0, /* external velocity scale */
		_effect_juggernaut_spark, _effect_metallic_clang, NONE, /* impact effect, melee impact effect, contrail effect */
	
		QUARTER_CIRCLE, QUARTER_CIRCLE/3, /* half visual arc, half vertical visual arc */
		30*WORLD_ONE, 3*WORLD_ONE, /* visual range, dark visual range */
		_intelligence_high, /* intelligence */
		_speed_blinding, /* speed */
		NORMAL_MONSTER_GRAVITY/4, NORMAL_MONSTER_TERMINAL_VELOCITY/4, /* gravity, terminal velocity */
		NONE, /* door retry mask */
		5*WORLD_ONE, {_damage_explosion, _alien_damage, 350, 50, FIXED_ONE}, /* shrapnel radius, shrapnel damage */
		
		UNONE, /* being hit */
		6, 5, /* dying hard (popping), dying soft (falling) */
		8, 8, /* hard dead frames, soft dead frames */
		0, 0, /* stationary shape, moving shape */
		7, 7, /* teleport in shape, teleport out shape */
		
		2*TICKS_PER_SECOND, /* attack frequency */
		
		/* melee attack */
		{
			_projectile_alien_weapon, /* melee attack type */
			10, /* repetitions */
			5, /* error */
			15*WORLD_ONE, /* range */
			1, /* melee attack shape */

			WORLD_ONE/4, WORLD_ONE_HALF+WORLD_ONE/8, WORLD_ONE-WORLD_ONE/4-WORLD_ONE/16, /* dx, dy, dz */
		},
		
		/* ranged attack */
		{
			_projectile_juggernaut_missile, /* ranged attack type */
			0, /* repetitions */
			40, /* error angle */
			25*WORLD_ONE, /* range */
			2, /* ranged attack shape */
			
			0, WORLD_ONE_HALF, WORLD_ONE+WORLD_ONE_HALF, /* dx, dy, dz */
		}
	},

	{ /* _monster_juggernaut_major */
		BUILD_COLLECTION(_collection_juggernaut, 1), /* shape collection */
		5000, 0, FLAG(_damage_fusion_bolt), /* vitality, immunities, weaknesses */
		_monster_major|_monster_is_alien|_monster_cant_fire_backwards|_monster_has_nuclear_hard_death|
			_monster_has_delayed_hard_death|_monster_cannot_be_dropped|_monster_fires_symmetrically|
			_monster_chooses_weapons_randomly|_monster_flys, /* flags */

		_class_juggernaut, /* class */
		_class_juggernaut, /* friends */
		_class_human|_class_client|_class_native, /* enemies */
	
		_normal_frequency, /* sound pitch */	
		NONE, NONE, NONE, NONE, NONE, NONE, /* sounds: activation, friendly activation, clear, kill, apology, friendly-fire */
		NONE, /* flaming dying sound */
		NONE, 0, /* random sound, random sound mask */

		NONE, /* carrying item type */
	
		WORLD_ONE, 2*WORLD_ONE, /* radius, height */
		WORLD_ONE, /* preferred hover height */
		INT16_MIN, INT16_MAX, /* minimum ledge delta, maximum ledge delta */
		0, /* external velocity scale */
		_effect_juggernaut_spark, _effect_metallic_clang, NONE, /* impact effect, melee impact effect, contrail effect */
	
		QUARTER_CIRCLE, QUARTER_CIRCLE/3, /* half visual arc, half vertical visual arc */
		30*WORLD_ONE, 3*WORLD_ONE, /* visual range, dark visual range */
		_intelligence_high, /* intelligence */
		_speed_blinding, /* speed */
		NORMAL_MONSTER_GRAVITY/4, NORMAL_MONSTER_TERMINAL_VELOCITY/4, /* gravity, terminal velocity */
		NONE, /* door retry mask */
		5*WORLD_ONE, {_damage_explosion, _alien_damage, 350, 50, FIXED_ONE}, /* shrapnel radius, shrapnel damage */
		
		UNONE, /* being hit */
		6, 5, /* dying hard (popping), dying soft (falling) */
		8, 8, /* hard dead frames, soft dead frames */
		0, 0, /* stationary shape, moving shape */
		7, 7, /* teleport in shape, teleport out shape */
		
		2*TICKS_PER_SECOND, /* attack frequency */
		
		/* melee attack */
		{
			_projectile_alien_weapon, /* melee attack type */
			20, /* repetitions */
			5, /* error */
			15*WORLD_ONE, /* range */
			1, /* melee attack shape */

			WORLD_ONE/4, WORLD_ONE_HALF+WORLD_ONE/8, WORLD_ONE-WORLD_ONE/4-WORLD_ONE/16, /* dx, dy, dz */
		},
		
		/* ranged attack */
		{
			_projectile_juggernaut_missile, /* ranged attack type */
			1, /* repetitions */
			40, /* error angle */
			25*WORLD_ONE, /* range */
			2, /* ranged attack shape */
			
			0, WORLD_ONE_HALF, WORLD_ONE+WORLD_ONE_HALF, /* dx, dy, dz */
		}
	},

	{ /* _monster_tiny_fighter */
		BUILD_COLLECTION(_collection_fighter, 1), /* shape collection */
		40, 0, 0, /* vitality, immunities, weaknesses */
		_monster_is_tiny|_monster_is_berserker|_monster_can_die_in_flames, /* flags */
	
		_class_fighter, /* class */
		_class_pfhor, /* friends */
		(_class_human&~_class_player)|_class_native|_class_defender, /* enemies */
	
		FIXED_ONE+FIXED_ONE_HALF, /* sound pitch */	
		_snd_fighter_activate, NONE, NONE, NONE, NONE, NONE, /* sounds: activation, friendly activation, clear, kill, apology, friendly-fire */
		_snd_fighter_wail, /* dying flaming */
		_snd_fighter_chatter, 15, /* random sound, random sound mask */

		NONE, /* carrying item type */
	
		WORLD_ONE/12, (4*WORLD_ONE)/12, /* radius, height */
		0, /* preferred hover height */
		-8*WORLD_ONE, WORLD_ONE/6, /* minimum ledge delta, maximum ledge delta */
		FIXED_ONE/2, /* external velocity scale */
		_effect_fighter_blood_splash, NONE, NONE, /* impact effect, melee impact effect, contrail effect */
	
		QUARTER_CIRCLE, QUARTER_CIRCLE/3, /* half visual arc, half vertical visual arc */
		30*WORLD_ONE, 3*WORLD_ONE, /* visual range, dark visual range */
		_intelligence_high, /* intelligence */
		_speed_fast, /* speed */
		NORMAL_MONSTER_GRAVITY, NORMAL_MONSTER_TERMINAL_VELOCITY, /* gravity, terminal velocity */
		_normal_door_retry_mask, /* door retry mask */
		NONE, {NONE, 0, 0, 0}, /* shrapnel radius, shrapnel damage */
		
		4, /* being hit */
		1, 3, /* dying hard (popping), dying soft (falling) */
		6, 5, /* hard dead frames, soft dead frames */
		7, 0, /* stationary shape, moving shape */
		12, 12, /* teleport in shape, teleport out shape */
		
		2*TICKS_PER_SECOND, /* attack frequency (for both melee and ranged attacks) */
		
		/* melee attack */
		{
			_projectile_staff, /* melee attack type */
			0, /* repetitions */
			0, /* error */
			WORLD_ONE, /* range */
			
			2, /* melee attack shape */

			0, 0, WORLD_ONE/5, /* dx, dy, dz */
		},
		
		/* ranged attack */
		{
			NONE, /* ranged attack type */
		}
	},

	{ /* _monster_tiny_bob */
		BUILD_COLLECTION(_collection_civilian, 0), /* shape collection */
		10, 0, 0, /* vitality, immunities, weaknesses */
		_monster_attacks_immediately|_monster_is_omniscent|_monster_cannot_be_dropped|_monster_waits_with_clear_shot|_monster_can_die_in_flames|_monster_uses_sniper_ledges|_monster_is_tiny, /* flags */

		_class_human_civilian, /* class */	
		_class_human, /* friends */
		(_class_hostile_alien^_class_assimilated_civilian)|_class_native, /* enemies */
	
		FIXED_ONE+FIXED_ONE_HALF, /* sound pitch */	
		_snd_human_activation, _snd_kill_the_player, _snd_human_clear, _snd_human_trash_talk, _snd_human_apology, _snd_human_stop_shooting_me_you_bastard, /* sounds: activation, friendly activation, clear, kill, apology, friendly-fire */
		_snd_human_wail, /* dying flaming */
		_snd_human_chatter, 0x1f, /* random sound, random sound mask */

		NONE, /* carrying item type */
	
		WORLD_ONE/12, (4*WORLD_ONE)/12, /* radius, height */
		0, /* preferred hover height */
		-WORLD_ONE, WORLD_ONE/6, /* minimum ledge delta, maximum ledge delta */
		FIXED_ONE/2, /* external velocity scale */
		_effect_civilian_blood_splash, NONE, NONE, /* impact effect, melee impact effect, contrail effect */
	
		QUARTER_CIRCLE, QUARTER_CIRCLE/3, /* half visual arc, half vertical visual arc */
		30*WORLD_ONE, WORLD_ONE, /* visual range, dark visual range */
		_intelligence_high, /* intelligence */
		_speed_superfast2, /* speed */
		NORMAL_MONSTER_GRAVITY, NORMAL_MONSTER_TERMINAL_VELOCITY, /* gravity, terminal velocity */
		_vidmaster_door_retry_mask, /* door retry mask */
		NONE, {NONE, 0, 0, 0}, /* shrapnel radius, shrapnel damage */
		
		10, /* being hit */
		2, 1, /* dying hard (popping), dying soft (falling) */
		4, 3, /* hard dead frames, soft dead frames */
		6, 0, /* stationary shape, moving shape */
		9, 8, /* teleport in shape, teleport out shape */
		
		3*TICKS_PER_SECOND, /* attack frequency (for both melee and ranged attacks) */
		
		/* melee attack */
		{
			NONE, /* melee attack type */
		},
		
		/* ranged attack */
		{
			_projectile_pistol_bullet, /* ranged attack type */
			1, /* repetitions */
			NUMBER_OF_ANGLES/150, /* error angle */
			10*WORLD_ONE, /* range */
			5, /* ranged attack shape */
			
			0, 0, WORLD_ONE/5, /* dx, dy, dz */
		}
	},

	{ /* _monster_tiny_yeti */
		BUILD_COLLECTION(_collection_yeti, 2), /* shape collection */
		100, FLAG(_damage_flame)|FLAG(_damage_alien_projectile)|FLAG(_damage_fusion_bolt)|FLAG(_damage_lava), 0, /* vitality, immunities, weaknesses */
		_monster_is_not_afraid_of_lava|_monster_is_berserker|_monster_is_tiny, /* flags */
		
		_class_yeti, /* class */
		_class_yeti, /* friends */
		(_class_human&~_class_player)|_class_pfhor, /* enemies */
	
		FIXED_ONE+FIXED_ONE_HALF, /* sound pitch */	
		NONE, NONE, NONE, NONE, NONE, NONE, /* sounds: activation, friendly activation, clear, kill, apology, friendly-fire */
		NONE, /* dying flaming */
		NONE, 15, /* random sound, random sound mask */

		NONE, /* carrying item type */
	
		WORLD_ONE/12, (4*WORLD_ONE)/12, /* radius, height */
		0, /* preferred hover height */
		-WORLD_ONE, WORLD_ONE/6, /* minimum ledge delta, maximum ledge delta */
		FIXED_ONE/2, /* external velocity scale */
		_effect_lava_yeti_blood_splash, NONE, NONE, /* impact effect, melee impact effect, contrail effect */
	
		QUARTER_CIRCLE, QUARTER_CIRCLE/3, /* half visual arc, half vertical visual arc */
		30*WORLD_ONE, WORLD_ONE, /* visual range, dark visual range */
		_intelligence_low, /* intelligence */
		_speed_superfast2, /* speed */
		NORMAL_MONSTER_GRAVITY, NORMAL_MONSTER_TERMINAL_VELOCITY, /* gravity, terminal velocity */
		_slow_door_retry_mask, /* door retry mask */
		NONE, {NONE, 0, 0, 0}, /* shrapnel radius, shrapnel damage */
		
		12, /* being hit */
		UNONE, 3, /* dying hard (popping), dying soft (falling) */
		UNONE, 4, /* hard dead frames, soft dead frames */
		0, 1, /* stationary shape, moving shape */
		UNONE, UNONE, /* teleport in shape, teleport out shape */
		
		TICKS_PER_SECOND, /* attack frequency (for both melee and ranged attacks) */
		
		/* melee attack */
		{
			_projectile_yeti, /* melee attack type */
			0, /* repetitions */
			0, /* error */
			WORLD_ONE, /* range */
			13, /* melee attack shape */

			0, 0, WORLD_ONE/5, /* dx, dy, dz */
		},
		
		/* ranged attack */
		{
			_projectile_lava_yeti, /* ranged attack type */
			1, /* repetitions */
			NUMBER_OF_ANGLES/150, /* error angle */
			12*WORLD_ONE, /* range */
			2, /* ranged attack shape */
			
			0, 0, WORLD_ONE/5, /* dx, dy, dz */
		}
	},
	
	// LP addition: the VacBobs:
	// they drop either fusion batteries or fusion guns as appropriate,
	// they shoot "minor" fusion bolts (those that don't flip switches)

	{ /* _civilian_fusion_crew "bob" */
		BUILD_COLLECTION(_collection_civilian_fusion, 0), /* shape collection */
		20, 0, 0, /* vitality, immunities, weaknesses */
		_monster_attacks_immediately|_monster_is_omniscent|_monster_cannot_be_dropped|_monster_waits_with_clear_shot|_monster_can_die_in_flames|_monster_uses_sniper_ledges, /* flags */

		_class_human_civilian, /* class */	
		_class_human, /* friends */
		(_class_hostile_alien^_class_assimilated_civilian)|_class_native, /* enemies */
	
		_normal_frequency, /* sound pitch */	
		_snd_civilian_fusion_activation, _snd_civilian_fusion_kill_the_player, _snd_civilian_fusion_clear, _snd_civilian_fusion_trash_talk, _snd_civilian_fusion_apology, _snd_civilian_fusion_stop_shooting_me_you_bastard, /* sounds: activation, friendly activation, clear, kill, apology, friendly-fire */
		_snd_civilian_fusion_wail, /* dying flaming */
		_snd_civilian_fusion_chatter, 0x1f, /* random sound, random sound mask */

		_i_plasma_magazine, /* carrying item type */
	
		WORLD_ONE/5, (4*WORLD_ONE)/5, /* radius, height */
		0, /* preferred hover height */
		-2*WORLD_ONE, WORLD_ONE/3, /* minimum ledge delta, maximum ledge delta */
		FIXED_ONE, /* external velocity scale */
		_effect_civilian_fusion_blood_splash, NONE, NONE, /* impact effect, melee impact effect, contrail effect */
	
		QUARTER_CIRCLE, QUARTER_CIRCLE/3, /* half visual arc, half vertical visual arc */
		30*WORLD_ONE, WORLD_ONE, /* visual range, dark visual range */
		_intelligence_high, /* intelligence */
		_speed_blinding, /* speed */
		NORMAL_MONSTER_GRAVITY, NORMAL_MONSTER_TERMINAL_VELOCITY, /* gravity, terminal velocity */
		_vidmaster_door_retry_mask, /* door retry mask */
		NONE, {NONE, 0, 0, 0}, /* shrapnel radius, shrapnel damage */
		
		10, /* being hit */
		2, 1, /* dying hard (popping), dying soft (falling) */
		4, 3, /* hard dead frames, soft dead frames */
		6, 0, /* stationary shape, moving shape */
		9, 8, /* teleport in shape, teleport out shape */
		
		3*TICKS_PER_SECOND, /* attack frequency (for both melee and ranged attacks) */
		
		/* melee attack */
		{
			NONE, /* melee attack type */
		},
		
		/* ranged attack */
		{
			_projectile_fusion_bolt_minor, /* ranged attack type */
			1, /* repetitions */
			NUMBER_OF_ANGLES/150, /* error angle */
			10*WORLD_ONE, /* range */
			5, /* ranged attack shape */
			
			0, 0, WORLD_ONE*3/4, /* dx, dy, dz */
		}
	},

	{ /* _civilian_fusion_science "fred" */
		BUILD_COLLECTION(_collection_civilian_fusion, 1), /* shape collection */
		25, 0, 0, /* vitality, immunities, weaknesses */
		_monster_attacks_immediately|_monster_is_omniscent|_monster_cannot_be_dropped|_monster_waits_with_clear_shot|_monster_can_die_in_flames|_monster_uses_sniper_ledges, /* flags */

		_class_human_civilian, /* class */	
		_class_human|_class_assimilated_civilian, /* friends */
		(_class_hostile_alien^_class_assimilated_civilian)|_class_native, /* enemies */
	
		_normal_frequency, /* sound pitch */	
		_snd_civilian_fusion_activation, _snd_civilian_fusion_kill_the_player, _snd_civilian_fusion_clear, _snd_civilian_fusion_trash_talk, _snd_civilian_fusion_apology, _snd_civilian_fusion_stop_shooting_me_you_bastard, /* sounds: activation, friendly activation, clear, kill, apology, friendly-fire */
		_snd_civilian_fusion_wail, /* dying flaming */
		_snd_civilian_fusion_chatter, 0x1f, /* random sound, random sound mask */

		_i_plasma_magazine, /* carrying item type */
	
		WORLD_ONE/5, (4*WORLD_ONE)/5, /* radius, height */
		0, /* preferred hover height */
		-2*WORLD_ONE, WORLD_ONE/3, /* minimum ledge delta, maximum ledge delta */
		FIXED_ONE, /* external velocity scale */
		_effect_civilian_fusion_blood_splash, NONE, NONE, /* impact effect, melee impact effect, contrail effect */
	
		QUARTER_CIRCLE, QUARTER_CIRCLE/3, /* half visual arc, half vertical visual arc */
		30*WORLD_ONE, WORLD_ONE, /* visual range, dark visual range */
		_intelligence_high, /* intelligence */
		_speed_blinding, /* speed */
		NORMAL_MONSTER_GRAVITY, NORMAL_MONSTER_TERMINAL_VELOCITY, /* gravity, terminal velocity */
		_vidmaster_door_retry_mask, /* door retry mask */
		NONE, {NONE, 0, 0, 0}, /* shrapnel radius, shrapnel damage */
		
		10, /* being hit */
		2, 1, /* dying hard (popping), dying soft (falling) */
		4, 3, /* hard dead frames, soft dead frames */
		6, 0, /* stationary shape, moving shape */
		9, 8, /* teleport in shape, teleport out shape */
		
		3*TICKS_PER_SECOND, /* attack frequency (for both melee and ranged attacks) */
		
		/* melee attack */
		{
			NONE, /* melee attack type */
		},
		
		/* ranged attack */
		{
			_projectile_fusion_bolt_minor, /* ranged attack type */
			2, /* repetitions */
			NUMBER_OF_ANGLES/150, /* error angle */
			13*WORLD_ONE, /* range */
			5, /* ranged attack shape */
			
			0, 0, WORLD_ONE*3/4, /* dx, dy, dz */
		}
	},

	{ /* _civilian_fusion_security "steve" */
		BUILD_COLLECTION(_collection_civilian_fusion, 2), /* shape collection */
		30, 0, 0, /* vitality, immunities, weaknesses */
		_monster_attacks_immediately|_monster_is_omniscent|_monster_cannot_be_dropped|_monster_waits_with_clear_shot|_monster_can_die_in_flames|_monster_uses_sniper_ledges, /* flags */

		_class_human_civilian, /* class */	
		_class_human|_class_assimilated_civilian, /* friends */
		(_class_hostile_alien^_class_assimilated_civilian)|_class_native, /* enemies */
	
		_normal_frequency, /* sound pitch */	
		_snd_civilian_fusion_activation, _snd_civilian_fusion_kill_the_player, _snd_civilian_fusion_clear, _snd_civilian_fusion_trash_talk, _snd_civilian_fusion_apology, _snd_civilian_fusion_stop_shooting_me_you_bastard, /* sounds: activation, friendly activation, clear, kill, apology, friendly-fire */
		_snd_civilian_fusion_wail, /* dying flaming */
		_snd_civilian_fusion_chatter, 0x1f, /* random sound, random sound mask */

		_i_plasma_pistol, /* carrying item type */
	
		WORLD_ONE/5, (4*WORLD_ONE)/5, /* radius, height */
		0, /* preferred hover height */
		-2*WORLD_ONE, WORLD_ONE/3, /* minimum ledge delta, maximum ledge delta */
		FIXED_ONE, /* external velocity scale */
		_effect_civilian_fusion_blood_splash, NONE, NONE, /* impact effect, melee impact effect, contrail effect */
	
		QUARTER_CIRCLE, QUARTER_CIRCLE/3, /* half visual arc, half vertical visual arc */
		30*WORLD_ONE, WORLD_ONE, /* visual range, dark visual range */
		_intelligence_high, /* intelligence */
		_speed_blinding, /* speed */
		NORMAL_MONSTER_GRAVITY, NORMAL_MONSTER_TERMINAL_VELOCITY, /* gravity, terminal velocity */
		_vidmaster_door_retry_mask, /* door retry mask */
		NONE, {NONE, 0, 0, 0}, /* shrapnel radius, shrapnel damage */
		
		10, /* being hit */
		2, 1, /* dying hard (popping), dying soft (falling) */
		4, 3, /* hard dead frames, soft dead frames */
		6, 0, /* stationary shape, moving shape */
		9, 8, /* teleport in shape, teleport out shape */
		
		TICKS_PER_SECOND, /* attack frequency (for both melee and ranged attacks) */
		
		/* melee attack */
		{
			NONE, /* melee attack type */
		},
		
		/* ranged attack */
		{
			_projectile_fusion_bolt_minor, /* ranged attack type */
			5, /* repetitions */
			NUMBER_OF_ANGLES/150, /* error angle */
			17*WORLD_ONE, /* range */
			5, /* ranged attack shape */
			
			0, 0, WORLD_ONE*3/4, /* dx, dy, dz */
		}
	},

	{ /* _civilian_fusion_assimilated "evil bob" */
		BUILD_COLLECTION(_collection_civilian_fusion, 3), /* shape collection */
		30, 0, 0, /* vitality, immunities, weaknesses */
		_monster_is_alien|_monster_is_kamakazi|_monster_can_die_in_flames, /* flags */
		
		_class_assimilated_civilian,
		_class_pfhor, /* friends */
		_class_player|_class_defender, /* enemies */
	
		_normal_frequency, /* sound pitch */	
		NONE, NONE, NONE, NONE, NONE, _snd_civilian_fusion_stop_shooting_me_you_bastard, /* sounds: activation, friendly activation, clear, kill, apology, friendly-fire */
		_snd_civilian_fusion_wail, /* dying flaming */
		_snd_assimilated_civilian_fusion_chatter, 0xf, /* random sound, random sound mask */

		NONE, /* carrying item type */
	
		WORLD_ONE/5, (4*WORLD_ONE)/5, /* radius, height */
		0, /* preferred hover height */
		-2*WORLD_ONE, WORLD_ONE/3, /* minimum ledge delta, maximum ledge delta */
		FIXED_ONE, /* external velocity scale */
		_effect_assimilated_civilian_fusion_blood_splash, NONE, NONE, /* impact effect, melee impact effect, contrail effect */
	
		QUARTER_CIRCLE, QUARTER_CIRCLE/3, /* half visual arc, half vertical visual arc */
		15*WORLD_ONE, WORLD_ONE, /* visual range, dark visual range */
		_intelligence_high, /* intelligence */
		_speed_blinding, /* speed */
		NORMAL_MONSTER_GRAVITY, NORMAL_MONSTER_TERMINAL_VELOCITY, /* gravity, terminal velocity */
		_vidmaster_door_retry_mask, /* door retry mask */
		WORLD_ONE, {_damage_explosion, _alien_damage, 80, 40, FIXED_ONE}, /* shrapnel radius, shrapnel damage  */
		
		10, /* being hit */
		11, UNONE, /* dying hard (popping), dying soft (falling) */
		4, 0, /* hard dead frames, soft dead frames */
		6, 0, /* stationary shape, moving shape */
		8, UNONE, /* teleport in shape, teleport out shape */
		
		2*TICKS_PER_SECOND, /* attack frequency (for both melee and ranged attacks) */
		
		/* melee attack */
		{
			NONE, /* melee attack type */
		},
		
		/* ranged attack */
		{
			NONE, /* ranged attack type */
		}
	},
};

// Added for the convenience of the 1-2-3 Converter
uint8 *unpack_monster_definition(uint8 *Stream, monster_definition *Objects, size_t Count);
uint8 *pack_monster_definition(uint8 *Stream, monster_definition *Objects, size_t Count);

#endif

#endif

