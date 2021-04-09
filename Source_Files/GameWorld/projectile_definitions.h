#ifndef __PROJECTILE_DEFINITIONS_H
#define __PROJECTILE_DEFINITIONS_H

/*
PROJECTILE_DEFINITIONS.H

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

Tuesday, May 31, 1994 5:19:56 PM

Feb 4, 2000 (Loren Petrich):
	Added SMG bullet and its ability to enter/exit liquids
*/

#include "effects.h"
#include "map.h"
#include "media.h"
#include "projectiles.h"
#include "SoundManagerEnums.h"

/* ---------- constants */

enum /* projectile flags */
{
	_guided= 0x0001,
	_stop_when_animation_loops= 0x0002,
	_persistent= 0x0004, /* does stops doing damage and stops moving against a target, but doesn't vanish */
	_alien_projectile= 0x0008, /* does less damage and moves slower on lower levels */
	_affected_by_gravity= 0x0010,
	_no_horizontal_error= 0x0020,
	_no_vertical_error= 0x0040,
	_can_toggle_control_panels= 0x0080,
	_positive_vertical_error= 0x0100,
	_melee_projectile= 0x0200, /* can use a monster’s custom melee detonation */
	_persistent_and_virulent= 0x0400, /* keeps moving and doing damage after a successful hit */
	_usually_pass_transparent_side= 0x0800,
	_sometimes_pass_transparent_side= 0x1000,
	_doubly_affected_by_gravity= 0x2000,
	_rebounds_from_floor= 0x4000, /* unless v.z<kvzMIN */
	_penetrates_media= 0x8000, /* huh uh huh ... i said penetrate */
	_becomes_item_on_detonation= 0x10000, /* item type in .permutation field of projectile */
	_bleeding_projectile= 0x20000, /* can use a monster’s custom bleeding detonation */
	_horizontal_wander= 0x40000, /* random horizontal error perpendicular to direction of movement */
	_vertical_wander= 0x80000, /* random vertical movement perpendicular to direction of movement */
	_affected_by_half_gravity= 0x100000,
	_penetrates_media_boundary=0x200000, // Can enter/exit liquids
	_passes_through_objects = 0x400000	 // and does no damage as it passes
};

/* ---------- structures */

struct projectile_definition
{
	int16 collection, shape; /* collection can be NONE (invisible) */
	int16 detonation_effect, media_detonation_effect;
	int16 contrail_effect, ticks_between_contrails, maximum_contrails; /* maximum of NONE is infinite */
	int16 media_projectile_promotion;

	world_distance radius; /* can be zero and will still hit */
	world_distance area_of_effect; /* one target if ==0 */
	struct damage_definition damage;

	uint32 flags;

	world_distance speed;
	world_distance maximum_range;

	_fixed sound_pitch;	
	int16 flyby_sound, rebound_sound;
};

#ifndef DONT_REPEAT_DEFINITIONS

/* ---------- projectile definitions */

static struct projectile_definition projectile_definitions[NUMBER_OF_PROJECTILE_TYPES];
const struct projectile_definition original_projectile_definitions[NUMBER_OF_PROJECTILE_TYPES]=
{
	{	/* player’s rocket */
		_collection_rocket, 0, /* collection number, shape number */
		_effect_rocket_explosion, NONE, /* detonation effect, media_detonation_effect */
		_effect_rocket_contrail, 1, NONE, /* contrail effect, ticks between contrails, maximum contrails */
		NONE, /* media projectile promotion */
			
		WORLD_ONE/8, /* radius */
		WORLD_ONE+WORLD_ONE_HALF, /* area-of-effect */
		{_damage_explosion, 0, 250, 50}, /* damage */
		
		_can_toggle_control_panels|_guided, /* flags */
		
		WORLD_ONE/4, /* speed */
		NONE, /* maximum range */
		
		_normal_frequency, /* sound pitch */
		_snd_rocket_flyby, NONE, /* flyby sound, rebound sound */
	},
	
	{	/* player’s grenade */
		_collection_rocket, 3, /* collection number, shape number */
		_effect_grenade_explosion, _medium_media_detonation_effect, /* detonation effect, media_detonation_effect */
		_effect_grenade_contrail, 1, 8, /* contrail effect, ticks between contrails, maximum contrails */
		NONE, /* media projectile promotion */
		
		0, /* radius */
		WORLD_THREE_FOURTHS, /* area-of-effect */
		{_damage_explosion, 0, 80, 20}, /* damage */
		
		_affected_by_gravity|_can_toggle_control_panels, /* flags */
		
		WORLD_ONE/4, /* speed */
		NONE, /* maximum range */
		
		_normal_frequency, /* sound pitch */
		_snd_grenade_flyby, NONE, /* flyby sound, rebound sound */
	},
	
	{	/* player’s pistol bullet */
		NONE, 0, /* collection number, shape number */
		_effect_bullet_ricochet, _small_media_detonation_effect, /* detonation effect, media_detonation_effect */
		NONE, 0, 0, /* contrail effect, ticks between contrails, maximum contrails */
		NONE, /* media projectile promotion */

		0, /* radius */
		0, /* area-of-effect */
		{_damage_projectile, 0, 20, 8}, /* damage */
		
		_bleeding_projectile|_usually_pass_transparent_side, /* flags */
		
		WORLD_ONE, /* speed */
		NONE, /* maximum range */

		_normal_frequency, /* sound pitch */
		NONE, NONE, /* flyby sound, rebound sound */
	},
	
	{	/* player’s rifle bullet */
		NONE, 0, /* collection number, shape number */
		_effect_bullet_ricochet, _small_media_detonation_effect, /* detonation effect, media_detonation_effect */
		NONE, 0, 0, /* contrail effect, ticks between contrails, maximum contrails */
		NONE, /* media projectile promotion */

		0, /* radius */
		0, /* area-of-effect */
		{_damage_projectile, 0, 9, 6}, /* damage */
		
		_bleeding_projectile|_usually_pass_transparent_side, /* flags */
		
		WORLD_ONE, /* speed */
		NONE, /* maximum range */

		_normal_frequency, /* sound pitch */
		NONE, NONE, /* flyby sound, rebound sound */
	},

	{	/* player’s shotgun bullet */
		NONE, 0, /* collection number, shape number */
		_effect_bullet_ricochet, _small_media_detonation_effect, /* detonation effect, media_detonation_effect */
		NONE, 0, 0, /* contrail effect, ticks between contrails, maximum contrails */
		NONE, /* media projectile promotion */

		0, /* radius */
		0, /* area-of-effect */
		{_damage_shotgun_projectile, 0, 20, 4}, /* damage */
		
		_bleeding_projectile|_can_toggle_control_panels|_usually_pass_transparent_side, /* flags */
		
		WORLD_ONE, /* speed */
		NONE, /* maximum range */

		_normal_frequency, /* sound pitch */
		NONE, NONE, /* flyby sound, rebound sound */
	},

	{	/* electrical melee staff */
		NONE, 0, /* collection number, shape number */
		_effect_fighter_melee_detonation, NONE, /* detonation effect, media_detonation_effect */
		NONE, 0, 0, /* contrail effect, ticks between contrails, maximum contrails */
		NONE, /* media projectile promotion */

		0, /* radius */
		0, /* area-of-effect */
		{_damage_electrical_staff, _alien_damage, 20, 5}, /* damage */
		
		_sometimes_pass_transparent_side|_alien_projectile|_melee_projectile|_penetrates_media, /* flags */
		
		WORLD_ONE_HALF, /* speed */
		WORLD_ONE, /* maximum range */

		_normal_frequency, /* sound pitch */
		NONE, NONE, /* flyby sound, rebound sound */
	},
	
	{	/* electrical melee staff projectile */
		BUILD_COLLECTION(_collection_fighter, 2), 9, /* collection number, shape number */
		_effect_fighter_projectile_detonation, _small_media_detonation_effect, /* detonation effect, media_detonation_effect */
		NONE, 0, 0, /* contrail effect, ticks between contrails, maximum contrails */
		NONE, /* media projectile promotion */

		0, /* radius */
		0, /* area-of-effect */
		{_damage_electrical_staff, _alien_damage, 30, 5}, /* damage */
		
		_sometimes_pass_transparent_side|_alien_projectile, /* flags */
		
		WORLD_ONE/8, /* speed */
		NONE, /* maximum range */

		_normal_frequency, /* sound pitch */
		_snd_fighter_projectile_flyby, NONE, /* flyby sound, rebound sound */
	},
	
	{	/* player’s flame thrower burst */
		_collection_rocket, 6, /* collection number, shape number */
		NONE, NONE, /* detonation effect, media_detonation_effect */
		NONE, 0, 0, /* contrail effect, ticks between contrails, maximum contrails */
		NONE, /* media projectile promotion */
		
		WORLD_ONE/3, /* radius */
		0, /* area-of-effect */
		{_damage_flame, 0, 8, 4}, /* damage */
		
		_sometimes_pass_transparent_side|_stop_when_animation_loops|_persistent, /* flags */
		
		WORLD_ONE/3, /* speed */
		NONE, /* maximum range */
		
		_normal_frequency, /* sound pitch */
		NONE, NONE, /* flyby sound, rebound sound */
	},

	{	/* _projectile_compiler_bolt_minor */
		BUILD_COLLECTION(_collection_compiler, 0), 4, /* collection number, shape number */
		_effect_compiler_bolt_minor_detonation, _small_media_detonation_effect, /* detonation effect, media_detonation_effect */
		NONE, 0, 0, /* contrail effect, ticks between contrails, maximum contrails */
		NONE, /* media projectile promotion */

		0, /* radius */
		0, /* area-of-effect */
		{_damage_compiler_bolt, _alien_damage, 40, 10}, /* damage */
		
		_sometimes_pass_transparent_side|_alien_projectile, /* flags */
		
		WORLD_ONE/8, /* speed */
		NONE, /* maximum range */

		_normal_frequency, /* sound pitch */
		_snd_compiler_projectile_flyby, NONE, /* flyby sound, rebound sound */
	},

	{	/* _projectile_compiler_bolt_major */
		BUILD_COLLECTION(_collection_compiler, 1), 4, /* collection number, shape number */
		_effect_compiler_bolt_major_detonation, _small_media_detonation_effect, /* detonation effect, media_detonation_effect */
		_effect_compiler_bolt_major_contrail, 0, NONE, /* contrail effect, ticks between contrails, maximum contrails */
		NONE, /* media projectile promotion */

		0, /* radius */
		0, /* area-of-effect */
		{_damage_compiler_bolt, _alien_damage, 40, 10}, /* damage */
		
		_sometimes_pass_transparent_side|_alien_projectile|_guided, /* flags */
		
		WORLD_ONE/12, /* speed */
		NONE, /* maximum range */

		_higher_frequency, /* sound pitch */
		_snd_compiler_projectile_flyby, NONE, /* flyby sound, rebound sound */
	},

	{	/* alien weapon */
		_collection_rocket, 22, /* collection number, shape number */
		_effect_alien_weapon_ricochet, _small_media_detonation_effect, /* detonation effect, media_detonation_effect */
		NONE, 0, 0, /* contrail effect, ticks between contrails, maximum contrails */
		NONE, /* media projectile promotion */

		WORLD_ONE/10, /* radius */
		0, /* area-of-effect */
		{_damage_alien_projectile, _alien_damage, 20, 8}, /* damage */
		
		_usually_pass_transparent_side|_alien_projectile|_can_toggle_control_panels, /* flags */
		
		WORLD_ONE/4, /* speed */
		NONE, /* maximum range */

		_snd_enforcer_projectile_flyby, NONE, /* flyby sound, rebound sound */
	},
	
	{	/* _projectile_fusion_minor */
		_collection_rocket, 11, /* collection number, shape number */
		_effect_minor_fusion_detonation, _small_media_detonation_effect, /* detonation effect, media_detonation_effect */
		NONE, 0, NONE, /* contrail effect, ticks between contrails, maximum contrails */
		_projectile_minor_fusion_dispersal, /* media projectile promotion */

		WORLD_ONE/20, /* radius */
		0, /* area-of-effect */
		{_damage_fusion_bolt, 0, 30, 10}, /* damage */
		
		_usually_pass_transparent_side, /* flags */
		
		WORLD_ONE/4, /* speed */
		NONE, /* maximum range */

		_normal_frequency, /* sound pitch */
		_snd_fusion_flyby, NONE, /* flyby sound, rebound sound */
	},

	{	/* _projectile_fusion_major */
		_collection_rocket, 12, /* collection number, shape number */
		_effect_major_fusion_detonation, _medium_media_detonation_effect, /* detonation effect, media_detonation_effect */
		_effect_major_fusion_contrail, 0, NONE, /* contrail effect, ticks between contrails, maximum contrails */
		_projectile_major_fusion_dispersal, /* media projectile promotion */

		WORLD_ONE/10, /* radius */
		0, /* area-of-effect */
		{_damage_fusion_bolt, 0, 80, 20}, /* damage */
		
		_sometimes_pass_transparent_side|_can_toggle_control_panels, /* flags */
		
		WORLD_ONE/3, /* speed */
		NONE, /* maximum range */

		_higher_frequency, /* sound pitch */
		_snd_fusion_flyby, NONE, /* flyby sound, rebound sound */
	},

	{	/* _projectile_hunter */
		BUILD_COLLECTION(_collection_hunter, 0), 5, /* collection number, shape number */
		_effect_hunter_projectile_detonation, _small_media_detonation_effect, /* detonation effect, media_detonation_effect */
		NONE, 0, NONE, /* contrail effect, ticks between contrails, maximum contrails */
		NONE, /* media projectile promotion */

		0, /* radius */
		0, /* area-of-effect */
		{_damage_hunter_bolt, 0, 15, 5}, /* damage */
		
		_usually_pass_transparent_side|_alien_projectile, /* flags */
		
		WORLD_ONE/4, /* speed */
		NONE, /* maximum range */

		_normal_frequency, /* sound pitch */
		_snd_hunter_projectile_flyby, NONE, /* flyby sound, rebound sound */
	},

	{	/* _projectile_fist */
		NONE, 0, /* collection number, shape number */
		_effect_fist_detonation, NONE, /* detonation effect, media_detonation_effect */
		NONE, 0, 0, /* contrail effect, ticks between contrails, maximum contrails */
		NONE, /* media projectile promotion */

		WORLD_ONE/4, /* radius */
		0, /* area-of-effect */
		{_damage_fist, 0, 50, 10}, /* damage (will be scaled by player’s velocity) */
		
		_usually_pass_transparent_side|_can_toggle_control_panels|_melee_projectile|_penetrates_media, /* flags */
		
		(3*WORLD_ONE)/4, /* speed */
		(3*WORLD_ONE)/4, /* maximum range */

		_normal_frequency, /* sound pitch */
		NONE, NONE, /* flyby sound, rebound sound */
	},

	{	/* _projectile_armageddon_sphere */
		0
	},

	{	/* _projectile_armageddon_electricity */
		0
	},

	{ /* _projectile_juggernaut_rocket */
		_collection_rocket, 0, /* collection number, shape number */
		_effect_rocket_explosion, _medium_media_detonation_effect, /* detonation effect, media_detonation_effect */
		_effect_rocket_contrail, 1, NONE, /* contrail effect, ticks between contrails, maximum contrails */
		NONE, /* media projectile promotion */
		
		WORLD_ONE/8, /* radius */
		WORLD_ONE+WORLD_ONE_HALF, /* area-of-effect */
		{_damage_explosion, _alien_damage, 250, 50}, /* damage */
		
		_guided|_can_toggle_control_panels, /* flags */
		
		WORLD_ONE/4, /* speed */
		NONE, /* maximum range */
		
		_normal_frequency, /* sound pitch */
		NONE, NONE, /* flyby sound, rebound sound */
	},

	{ /* _projectile_trooper_bullet */
		NONE, 0, /* collection number, shape number */
		_effect_bullet_ricochet, _small_media_detonation_effect, /* detonation effect, media_detonation_effect */
		NONE, 0, 0, /* contrail effect, ticks between contrails, maximum contrails */
		NONE, /* media projectile promotion */

		0, /* radius */
		0, /* area-of-effect */
		{_damage_projectile, _alien_damage, 15, 4}, /* damage */
		
		_bleeding_projectile|_usually_pass_transparent_side, /* flags */
		
		WORLD_ONE, /* speed */
		NONE, /* maximum range */

		_normal_frequency, /* sound pitch */
		NONE, NONE, /* flyby sound, rebound sound */
	},
	
	{ /* _projectile_trooper_grenade */
		_collection_trooper, 5, /* collection number, shape number */
		_effect_grenade_explosion, _medium_media_detonation_effect, /* detonation effect, media_detonation_effect */
		_effect_grenade_contrail, 1, 8, /* contrail effect, ticks between contrails, maximum contrails */
		NONE, /* media projectile promotion */
		
		0, /* radius */
		WORLD_THREE_FOURTHS, /* area-of-effect */
		{_damage_explosion, _alien_damage, 40, 20}, /* damage */
		
		_affected_by_gravity|_can_toggle_control_panels, /* flags */
		
		WORLD_ONE/5, /* speed */
		NONE, /* maximum range */
		
		_normal_frequency, /* sound pitch */
		NONE, NONE, /* flyby sound, rebound sound */
	},
	
	{ /* _projectile_minor_defender */
		BUILD_COLLECTION(_collection_defender, 0), 4, /* collection number, shape number */
		_effect_minor_defender_detonation, _small_media_detonation_effect, /* detonation effect, media_detonation_effect */
		NONE, 0, 0, /* contrail effect, ticks between contrails, maximum contrails */
		NONE, /* media projectile promotion */

		WORLD_ONE/4, /* radius */
		0, /* area-of-effect */
		{_damage_defender, 0, 30, 8}, /* damage */
		
		_usually_pass_transparent_side, /* flags */
		
		WORLD_ONE/8, /* speed */
		NONE, /* maximum range */

		_normal_frequency, /* sound pitch */
		_snd_defender_flyby, NONE, /* flyby sound, rebound sound */
	},
	
	{ /* _projectile_major_defender */
		BUILD_COLLECTION(_collection_defender, 1), 4, /* collection number, shape number */
		_effect_major_defender_detonation, _small_media_detonation_effect, /* detonation effect, media_detonation_effect */
		NONE, 0, 0, /* contrail effect, ticks between contrails, maximum contrails */
		NONE, /* media projectile promotion */

		WORLD_ONE/4, /* radius */
		0, /* area-of-effect */
		{_damage_defender, 0, 30, 8}, /* damage */
		
		_usually_pass_transparent_side|_guided, /* flags */
		
		WORLD_ONE/6, /* speed */
		NONE, /* maximum range */

		_higher_frequency, /* sound pitch */
		_snd_defender_flyby, NONE, /* flyby sound, rebound sound */
	},

	{ /* _projectile_juggernaut_missile */
		_collection_juggernaut, 4, /* collection number, shape number */
		_effect_grenade_explosion, _medium_media_detonation_effect, /* detonation effect, media_detonation_effect */
		_effect_juggernaut_missile_contrail, 2, NONE, /* contrail effect, ticks between contrails, maximum contrails */
		NONE, /* media projectile promotion */
		
		0, /* radius */
		WORLD_THREE_FOURTHS, /* area-of-effect */
		{_damage_explosion, _alien_damage, 40, 20}, /* damage */
		
		_affected_by_half_gravity|_can_toggle_control_panels|_guided|_positive_vertical_error, /* flags */
		
		WORLD_ONE/5, /* speed */
		NONE, /* maximum range */
		
		_normal_frequency, /* sound pitch */
		NONE, NONE, /* flyby sound, rebound sound */
	},

	{	/* _projectile_minor_energy_drain */
		NONE, 0, /* collection number, shape number */
		NONE, NONE, /* detonation effect, media_detonation_effect */
		NONE, 0, 0, /* contrail effect, ticks between contrails, maximum contrails */
		NONE, /* media projectile promotion */

		WORLD_ONE/8, /* radius */
		0, /* area-of-effect */
		{_damage_energy_drain, 0, 4, 0}, /* damage (will be scaled by player’s velocity) */
		
		_melee_projectile|_penetrates_media, /* flags */
		
		(3*WORLD_ONE)/4, /* speed */
		(3*WORLD_ONE)/4, /* maximum range */

		_normal_frequency, /* sound pitch */
		NONE, NONE, /* flyby sound, rebound sound */
	},

	{	/* _projectile_major_energy_drain */
		NONE, 0, /* collection number, shape number */
		NONE, NONE, /* detonation effect, media_detonation_effect */
		NONE, 0, 0, /* contrail effect, ticks between contrails, maximum contrails */
		NONE, /* media projectile promotion */

		WORLD_ONE/8, /* radius */
		0, /* area-of-effect */
		{_damage_energy_drain, 0, 8, 0}, /* damage (will be scaled by player’s velocity) */
		
		_melee_projectile|_penetrates_media, /* flags */
		
		(3*WORLD_ONE)/4, /* speed */
		(3*WORLD_ONE)/4, /* maximum range */

		_normal_frequency, /* sound pitch */
		NONE, NONE, /* flyby sound, rebound sound */
	},

	{	/* _projectile_oxygen_drain */
		NONE, 0, /* collection number, shape number */
		NONE, NONE, /* detonation effect, media_detonation_effect */
		NONE, 0, 0, /* contrail effect, ticks between contrails, maximum contrails */
		NONE, /* media projectile promotion */

		WORLD_ONE/8, /* radius */
		0, /* area-of-effect */
		{_damage_oxygen_drain, 0, 4, 0}, /* damage (will be scaled by player’s velocity) */
		
		_melee_projectile|_penetrates_media, /* flags */
		
		(3*WORLD_ONE)/4, /* speed */
		(3*WORLD_ONE)/4, /* maximum range */

		_normal_frequency, /* sound pitch */
		NONE, NONE, /* flyby sound, rebound sound */
	},

	{	/* _projectile_hummer_slow */
		BUILD_COLLECTION(_collection_hummer, 0), 5, /* collection number, shape number */
		_effect_minor_hummer_projectile_detonation, _small_media_detonation_effect, /* detonation effect, media_detonation_effect */
		NONE, 0, NONE, /* contrail effect, ticks between contrails, maximum contrails */
		NONE, /* media projectile promotion */

		0, /* radius */
		0, /* area-of-effect */
		{_damage_hummer_bolt, 0, 15, 5}, /* damage */
		
		_usually_pass_transparent_side|_alien_projectile, /* flags */
		
		WORLD_ONE/8, /* speed */
		NONE, /* maximum range */

		_normal_frequency, /* sound pitch */
		_snd_hummer_projectile_flyby, NONE, /* flyby sound, rebound sound */
	},

	{	/* _projectile_hummer_fast */
		BUILD_COLLECTION(_collection_hummer, 1), 5, /* collection number, shape number */
		_effect_major_hummer_projectile_detonation, _small_media_detonation_effect, /* detonation effect, media_detonation_effect */
		NONE, 0, NONE, /* contrail effect, ticks between contrails, maximum contrails */
		NONE, /* media projectile promotion */

		0, /* radius */
		0, /* area-of-effect */
		{_damage_hummer_bolt, 0, 15, 5}, /* damage */
		
		_usually_pass_transparent_side|_alien_projectile, /* flags */
		
		WORLD_ONE/6, /* speed */
		NONE, /* maximum range */

		_higher_frequency, /* sound pitch */
		_snd_hummer_projectile_flyby, NONE, /* flyby sound, rebound sound */
	},

	{	/* _projectile_hummer_durandal */
		BUILD_COLLECTION(_collection_hummer, 1), 5, /* collection number, shape number */
		_effect_durandal_hummer_projectile_detonation, _small_media_detonation_effect, /* detonation effect, media_detonation_effect */
		NONE, 0, NONE, /* contrail effect, ticks between contrails, maximum contrails */
		NONE, /* media projectile promotion */

		0, /* radius */
		0, /* area-of-effect */
		{_damage_hummer_bolt, 0, 15, 5}, /* damage */
		
		_guided|_usually_pass_transparent_side|_alien_projectile, /* flags */
		
		WORLD_ONE/8, /* speed */
		NONE, /* maximum range */

		_lower_frequency, /* sound pitch */
		_snd_hummer_projectile_flyby, NONE, /* flyby sound, rebound sound */
	},

	{	/* _projectile_minor_cyborg_ball */
		BUILD_COLLECTION(_collection_cyborg, 0), 6, /* collection number, shape number */
		_effect_grenade_explosion, _medium_media_detonation_effect, /* detonation effect, media_detonation_effect */
		_effect_grenade_contrail, 1, 8, /* contrail effect, ticks between contrails, maximum contrails */
		NONE, /* media projectile promotion */

		WORLD_ONE/8, /* radius */
		WORLD_ONE, /* area-of-effect */
		{_damage_explosion, 0, 20, 10}, /* damage */
		
		_can_toggle_control_panels|_sometimes_pass_transparent_side|_alien_projectile|_rebounds_from_floor|_doubly_affected_by_gravity, /* flags */
		
		WORLD_ONE/10, /* speed */
		NONE, /* maximum range */

		_normal_frequency, /* sound pitch */
		_snd_cyborg_projectile_flyby, _snd_cyborg_projectile_bounce, /* flyby sound, rebound sound */
	},

	{	/* _projectile_major_cyborg_ball */
		BUILD_COLLECTION(_collection_cyborg, 1), 6, /* collection number, shape number */
		_effect_grenade_explosion, _medium_media_detonation_effect, /* detonation effect, media_detonation_effect */
		_effect_grenade_contrail, 1, 8, /* contrail effect, ticks between contrails, maximum contrails */
		NONE, /* media projectile promotion */

		WORLD_ONE/8, /* radius */
		WORLD_ONE, /* area-of-effect */
		{_damage_explosion, 0, 40, 10}, /* damage */
		
		_guided|_can_toggle_control_panels|_sometimes_pass_transparent_side|_alien_projectile|_rebounds_from_floor|_doubly_affected_by_gravity, /* flags */
		
		WORLD_ONE/8, /* speed */
		NONE, /* maximum range */

		_lower_frequency, /* sound pitch */
		_snd_cyborg_projectile_flyby, _snd_cyborg_projectile_bounce, /* flyby sound, rebound sound */
	},

	{	/* _projectile_ball */
		BUILD_COLLECTION(_collection_player, 0), 0, /* collection number, shape number */
		NONE, NONE, /* detonation effect, media_detonation_effect */
		NONE, 1, 8, /* contrail effect, ticks between contrails, maximum contrails */
		NONE, /* media projectile promotion */

		WORLD_ONE/4, /* radius */
		NONE, /* area-of-effect */
		{NONE, 0, 40, 10}, /* damage */
		
		_persistent_and_virulent|_penetrates_media|_becomes_item_on_detonation|_can_toggle_control_panels|_rebounds_from_floor|_doubly_affected_by_gravity|_penetrates_media, /* flags */
		
		0, /* speed */
		NONE, /* maximum range */

		_normal_frequency, /* sound pitch */
		NONE, _snd_ball_bounce, /* flyby sound, rebound sound */
	},

	{	/* _projectile_minor_fusion_dispersal */
		_collection_rocket, 11, /* collection number, shape number */
		_effect_minor_fusion_dispersal, NONE, /* detonation effect, media_detonation_effect */
		NONE, 0, NONE, /* contrail effect, ticks between contrails, maximum contrails */
		NONE, /* media projectile promotion */

		WORLD_ONE/20, /* radius */
		WORLD_ONE, /* area-of-effect */
		{_damage_fusion_bolt, 0, 30, 10}, /* damage */
		
		0, /* flags */
		
		WORLD_ONE/4, /* speed */
		NONE, /* maximum range */

		_normal_frequency, /* sound pitch */
		_snd_fusion_flyby, NONE, /* flyby sound, rebound sound */
	},

	{	/* _projectile_major_fusion_dispersal */
		_collection_rocket, 12, /* collection number, shape number */
		_effect_major_fusion_dispersal, NONE, /* detonation effect, media_detonation_effect */
		NONE, 0, NONE, /* contrail effect, ticks between contrails, maximum contrails */
		NONE, /* media projectile promotion */

		WORLD_ONE/10, /* radius */
		2*WORLD_ONE, /* area-of-effect */
		{_damage_fusion_bolt, 0, 80, 20}, /* damage */
		
		0, /* flags */
		
		WORLD_ONE/3, /* speed */
		NONE, /* maximum range */

		_higher_frequency, /* sound pitch */
		_snd_fusion_flyby, NONE, /* flyby sound, rebound sound */
	},

	{	/* _projectile_overloaded_fusion_dispersal */
		_collection_rocket, 12, /* collection number, shape number */
		_effect_overloaded_fusion_dispersal, NONE, /* detonation effect, media_detonation_effect */
		NONE, 0, NONE, /* contrail effect, ticks between contrails, maximum contrails */
		NONE, /* media projectile promotion */

		WORLD_ONE/10, /* radius */
		4*WORLD_ONE, /* area-of-effect */
		{_damage_fusion_bolt, 0, 500, 0}, /* damage */
		
		0, /* flags */
		
		WORLD_ONE/3, /* speed */
		NONE, /* maximum range */

		_lower_frequency, /* sound pitch */
		_snd_fusion_flyby, NONE, /* flyby sound, rebound sound */
	},

	{	/* _projectile_yeti */
		NONE, 0, /* collection number, shape number */
		_effect_yeti_melee_detonation, NONE, /* detonation effect, media_detonation_effect */
		NONE, 0, 0, /* contrail effect, ticks between contrails, maximum contrails */
		NONE, /* media projectile promotion */

		0, /* radius */
		0, /* area-of-effect */
		{_damage_yeti_claws, _alien_damage, 20, 5}, /* damage */
		
		_sometimes_pass_transparent_side|_alien_projectile|_melee_projectile|_penetrates_media, /* flags */
		
		WORLD_ONE_HALF, /* speed */
		WORLD_ONE, /* maximum range */

		_normal_frequency, /* sound pitch */
		NONE, NONE, /* flyby sound, rebound sound */
	},
	
	{	/* _projectile_sewage_yeti */
		BUILD_COLLECTION(_collection_yeti, 0), 10, /* collection number, shape number */
		_effect_sewage_yeti_projectile_detonation, _small_media_detonation_effect, /* detonation effect, media_detonation_effect */
		NONE, 0, NONE, /* contrail effect, ticks between contrails, maximum contrails */
		NONE, /* media projectile promotion */

		0, /* radius */
		0, /* area-of-effect */
		{_damage_yeti_projectile, 0, 15, 5}, /* damage */
		
		_usually_pass_transparent_side|_alien_projectile|_affected_by_half_gravity, /* flags */
		
		WORLD_ONE/8, /* speed */
		NONE, /* maximum range */

		_normal_frequency, /* sound pitch */
		_snd_yeti_projectile_sewage_flyby, NONE, /* flyby sound, rebound sound */
	},

	{	/* _projectile_lava_yeti */
		BUILD_COLLECTION(_collection_yeti, 2), 6, /* collection number, shape number */
		_effect_lava_yeti_projectile_detonation, _small_media_detonation_effect, /* detonation effect, media_detonation_effect */
		NONE, 0, NONE, /* contrail effect, ticks between contrails, maximum contrails */
		NONE, /* media projectile promotion */

		0, /* radius */
		0, /* area-of-effect */
		{_damage_flame, 0, 30, 10}, /* damage */
		
		_usually_pass_transparent_side|_alien_projectile, /* flags */
		
		WORLD_ONE/8, /* speed */
		NONE, /* maximum range */

		_normal_frequency, /* sound pitch */
		_snd_yeti_projectile_lava_flyby, NONE, /* flyby sound, rebound sound */
	},
	
	// LP addition: SMG bullet is a clone of the rifle one, except for entering/exiting liquids
	{	/* player’s smg bullet */
		NONE, 0, /* collection number, shape number */
		_effect_bullet_ricochet, _small_media_detonation_effect, /* detonation effect, media_detonation_effect */
		NONE, 0, 0, /* contrail effect, ticks between contrails, maximum contrails */
		NONE, /* media projectile promotion */

		0, /* radius */
		0, /* area-of-effect */
		{_damage_projectile, 0, 9, 6}, /* damage */
		
		_bleeding_projectile|_usually_pass_transparent_side|_penetrates_media_boundary, /* flags */
		
		WORLD_ONE, /* speed */
		NONE, /* maximum range */

		_normal_frequency, /* sound pitch */
		NONE, NONE, /* flyby sound, rebound sound */
	},
};

// Added for the convenience of the 1-2-3 Converter
uint8 *unpack_projectile_definition(uint8 *Stream, projectile_definition *Objects, size_t Count);
uint8 *pack_projectile_definition(uint8 *Stream, projectile_definition *Objects, size_t Count);

#endif

#endif

