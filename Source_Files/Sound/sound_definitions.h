#ifndef __SOUND_DEFINTIONS_H
#define __SOUND_DEFINTIONS_H

/*
SOUND_DEFINITIONS.H

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

Saturday, August 20, 1994 2:42:56 PM

Monday, September 26, 1994 2:06:09 PM  (Jason)
	changed the depth-fading curves significantly.
Wednesday, February 1, 1995 12:52:03 PM  (Jason')
	marathon 2.

Feb 2, 2000 (Loren Petrich):
	Added Marathon Infinity sound definitions;
	nomenclature is being kept consistent with Anvil's sound-selection popup menu;
	this menu will provide the order of the sounds.
	
	Discovered that SOUND_FILE_TAG should not be changed from 'snd2' to be Infinity-compatible

Aug 17, 2000 (Loren Petrich):
	Turned handle for loaded sound into a pointer; added length of that sound object.
*/

#include "SoundManagerEnums.h"
#include "world.h"

/* ---------- constants */

enum
{
	MAXIMUM_PERMUTATIONS_PER_SOUND= 5
};

enum /* sound behaviors */
{
	_sound_is_quiet,
	_sound_is_normal,
	_sound_is_loud,
	NUMBER_OF_SOUND_BEHAVIOR_DEFINITIONS
};

enum /* flags */
{
	_sound_cannot_be_restarted= 0x0001,
	_sound_does_not_self_abort= 0x0002,
	_sound_resists_pitch_changes= 0x0004, // 0.5 external pitch changes
	_sound_cannot_change_pitch= 0x0008, // no external pitch changes
	_sound_cannot_be_obstructed= 0x0010, // ignore obstructions
	_sound_cannot_be_media_obstructed= 0x0020, // ignore media obstructions
	_sound_is_ambient= 0x0040 // will not be loaded unless _ambient_sound_flag is asserted
};

enum /* sound chances */
{
	_ten_percent= 32768*9/10,
	_twenty_percent= 32768*8/10,
	_thirty_percent= 32768*7/10,
	_fourty_percent= 32768*6/10,
	_fifty_percent= 32768*5/10,
	_sixty_percent= 32768*4/10,
	_seventy_percent= 32768*3/10,
	_eighty_percent= 32768*2/10,
	_ninty_percent= 32768*1/10,
	_always= 0
};

/* ---------- structures */

struct ambient_sound_definition
{
	int16 sound_index;
};

struct random_sound_definition
{
	int16 sound_index;
};

enum
{
	SOUND_FILE_VERSION= 1,
	SOUND_FILE_TAG= FOUR_CHARS_TO_INT('s', 'n', 'd', '2')
};

struct sound_file_header
{
	int32 version;
	int32 tag;
	
	int16 source_count; // usually 2 (8-bit, 16-bit)
	int16 sound_count;
	
	int16 unused[124];
	
	// immediately followed by source_count*sound_count sound_definition structures
};
const int SIZEOF_sound_file_header = 260;

struct sound_definition
{
	int16 sound_code;
	
	int16 behavior_index;
	uint16 flags;

	uint16 chance; // play sound if AbsRandom()>=chance
	
	/* if low_pitch==0, use FIXED_ONE; if high_pitch==0 use low pitch; else choose in [low_pitch,high_pitch] */
	_fixed low_pitch, high_pitch;
	
	/* filled in later */
	int16 permutations;
	uint16 permutations_played;
	int32 group_offset, single_length, total_length; // magic numbers necessary to load sounds
	int32 sound_offsets[MAXIMUM_PERMUTATIONS_PER_SOUND]; // zero-based from group offset
	
	uint32 last_played; // machine ticks
	
	// Pointer to loaded sound and size of sound object pointed to
	uint8 *ptr;
	int32 size;
};
const int SIZEOF_sound_definition = 64;

struct depth_curve_definition
{
	int16 maximum_volume, maximum_volume_distance;
	int16 minimum_volume, minimum_volume_distance;
};

struct sound_behavior_definition
{
	struct depth_curve_definition obstructed_curve, unobstructed_curve;
};

/* ---------- sound behavior structures */

static struct sound_behavior_definition sound_behavior_definitions[NUMBER_OF_SOUND_BEHAVIOR_DEFINITIONS]=
{
	/* _sound_is_quiet */
	{
		{0, 0, 0, 0}, /* obstructed quiet sounds make no sound */
		{MAXIMUM_SOUND_VOLUME, 0, 0, 5*WORLD_ONE},
	},

	/* _sound_is_normal */
	{
		{MAXIMUM_SOUND_VOLUME/2, 0, 0, 7*WORLD_ONE},
		{MAXIMUM_SOUND_VOLUME, WORLD_ONE, 0, 10*WORLD_ONE},
	},
	
	/* _sound_is_loud */
	{
		{(3*MAXIMUM_SOUND_VOLUME)/4, 0, 0, 10*WORLD_ONE},
		{MAXIMUM_SOUND_VOLUME, 2*WORLD_ONE, MAXIMUM_SOUND_VOLUME/8, 15*WORLD_ONE},
	}
};

/* ---------- ambient sound definition structures */

static struct ambient_sound_definition ambient_sound_definitions[NUMBER_OF_AMBIENT_SOUND_DEFINITIONS]=
{
	{_snd_water},
	{_snd_sewage},
	{_snd_lava},
	{_snd_goo},
	{_snd_under_media},
	{_snd_wind},
	{_snd_waterfall},
	{_snd_siren},
	{_snd_fan},
	{_snd_spht_door},
	{_snd_spht_platform},
	{_snd_heavy_spht_door},
	{_snd_heavy_spht_platform},
	{_snd_light_machinery},
	{_snd_heavy_machinery},
	{_snd_transformer},
	{_snd_sparking_transformer},
	{_snd_machine_binder},
	{_snd_machine_bookpress},
	{_snd_machine_puncher},
	{_snd_electric},
	{_snd_alarm},
	{_snd_night_wind},
	{_snd_pfhor_door},
	{_snd_pfhor_platform},
	{_snd_alien_noise1},
	{_snd_alien_noise2},
	// LP addition: Marathon Infinity ambient sound
	{_snd_alien_harmonics},
};

/* ---------- random sound definition structures */

static struct random_sound_definition random_sound_definitions[NUMBER_OF_RANDOM_SOUND_DEFINITIONS]=
{
	{_snd_water_drip},
	{_snd_surface_explosion},
	{_snd_underground_explosion},
	{_snd_owl},
	// LP addition: Marathon Infinity random sound
	{_snd_creak},
};

/* ---------- sound definition structures */

// #ifndef STATIC_DEFINITIONS

/*
#else
static struct sound_definition sound_definitions[NUMBER_OF_SOUND_DEFINITIONS]=
{
	{10140, _sound_is_normal, _sound_does_not_self_abort}, // _snd_startup

	{10000, _sound_is_normal}, // _snd_teleport_in
	{10010, _sound_is_normal}, // _snd_teleport_out
	{10020, _sound_is_loud, _sound_cannot_change_pitch}, // _snd_body_being_crunched
	{NONE, _sound_is_normal}, // _snd_nuclear_hard_death
	{10030, _sound_is_normal}, // _snd_absorbed

	{NONE, _sound_is_normal}, // _snd_breathing
	{10040, _sound_is_normal}, // _snd_oxygen_warning
	{10050, _sound_is_normal}, // _snd_suffocation

	// control panels
	{10060, _sound_is_quiet}, // _snd_energy_refuel
	{10070, _sound_is_quiet}, // _snd_oxygen_refuel
	{10080, _sound_is_quiet}, // _snd_cant_toggle_switch
	{10090, _sound_is_quiet}, // _snd_switch_on
	{10100, _sound_is_quiet}, // _snd_switch_off
	{10110, _sound_is_quiet}, // _snd_puzzle_switch
	{10120, _sound_is_quiet}, // _snd_chip_insertion
	{10130, _sound_is_quiet}, // _snd_pattern_buffer
	{11280, _sound_is_loud}, // _snd_destroy_control_panel

	{10150, _sound_is_quiet}, // _snd_adjust_volume
	{10160, _sound_is_quiet}, // _snd_got_powerup
	{10170, _sound_is_quiet}, // _snd_got_item

	{11000, _sound_is_normal}, // _snd_bullet_ricochet
	{11010, _sound_is_normal}, // _snd_metallic_ricochet
	{11020, _sound_is_quiet}, // _snd_empty_gun

	// s’pht doors and platforms
	{12000, _sound_is_normal}, // _snd_spht_door_opening,
	{12010, _sound_is_normal}, // _snd_spht_door_closing,
	{12020, _sound_is_normal}, // _snd_spht_door_obstructed,
	{12030, _sound_is_normal}, // _snd_spht_platform_starting,
	{12040, _sound_is_normal}, // _snd_spht_platform_stopping,

	{14540, _sound_is_normal}, // _snd_owl
	{NONE, _sound_is_normal}, //
	{NONE, _sound_is_normal}, //
	
	{12080, _sound_is_normal}, // _snd_pfhor_platform_starting,
	{12090, _sound_is_normal}, // _snd_pfhor_platform_stopping,

	{11030, _sound_is_quiet}, // _snd_fist_hitting
	
	// magnum
	{11040, _sound_is_normal}, // _snd_magnum_firing
	{11050, _sound_is_quiet}, // _snd_magnum_reloading
	
	// assault rifle
	{11060, _sound_is_normal}, // _snd_assault_rifle_firing
	{11070, _sound_is_normal}, // _snd_grenade_launcher_firing
	{11080, _sound_is_loud}, // _snd_grenade_exploding
	{11090, _sound_is_normal}, // _snd_grenade_flyby

	// fusion pistol
	{11100, _sound_is_normal}, // _snd_fusion_firing
	{11110, _sound_is_normal}, // _snd_fusion_exploding
	{11120, _sound_is_normal}, // _snd_fusion_flyby
	{11130, _sound_is_normal}, // _snd_fusion_charging

	// rocket launcher
	{11140, _sound_is_loud}, // _snd_rocket_exploding
	{11150, _sound_is_normal}, // _snd_rocket_flyby
	{11160, _sound_is_normal}, // _snd_rocket_firing
	
	// flamethrower
	{11170, _sound_is_normal}, // _snd_flamethrower

	{11180, _sound_is_quiet, _sound_cannot_change_pitch}, // _snd_body_falling
	{11190, _sound_is_quiet, _sound_cannot_change_pitch}, // _snd_body_exploding
	{11200, _sound_is_normal, _sound_cannot_change_pitch}, // _snd_bullet_hitting_flesh

	// 21300 fighter
	{15300, _sound_is_normal}, // _snd_fighter_activate
	{15310, _sound_is_normal}, // _snd_fighter_wail
	{15320, _sound_is_normal}, // _snd_fighter_scream
	{15330, _sound_is_normal}, // _snd_fighter_chatter
	{15340, _sound_is_normal}, // _snd_fighter_swing
	{15350, _sound_is_normal}, // _snd_fighter_hit
	{15360, _sound_is_normal}, // _snd_fighter_flyby

	// 21200 compiler	
	{15200, _sound_is_normal}, // _snd_compiler_attack
	{15210, _sound_is_normal}, // _snd_compiler_death
	{15220, _sound_is_normal}, // _snd_compiler_being_hit
	{15230, _sound_is_normal}, // _snd_compiler_projectile_flyby
	{15240, _sound_is_normal}, // _snd_compiler_projectile_hit

	// 21100 cyborg
	{15100, _sound_is_normal, 0, _fifty_percent}, // _snd_cyborg_moving,
	{15110, _sound_is_normal}, // _snd_cyborg_attack,
	{15120, _sound_is_normal}, // _snd_cyborg_hit,
	{15130, _sound_is_loud}, // _snd_cyborg_death,
	{15140, _sound_is_normal}, // _snd_cyborg_projectile_bounce,
	{15150, _sound_is_normal}, // _snd_cyborg_projectile_hit,
	{15160, _sound_is_normal}, // _snd_cyborg_projectile_flyby

	// 21000 hummer
	{15000, _sound_is_quiet}, // _snd_hummer_activate,
	{15010, _sound_is_normal}, // _snd_hummer_start_attack,
	{15020, _sound_is_normal}, // _snd_hummer_attack,
	{15030, _sound_is_normal}, // _snd_hummer_dying,
	{15040, _sound_is_loud}, // _snd_hummer_death,
	{15050, _sound_is_normal}, // _snd_hummer_projectile_hit,
	{15060, _sound_is_normal}, // _snd_hummer_projectile_flyby,

	// bob
	{15400, _sound_is_loud}, // _snd_human_wail
	{15410, _sound_is_normal}, // _snd_human_scream
	{15420, _sound_is_normal}, // _snd_human_hit
	{15430, _sound_is_normal, _sound_cannot_be_restarted}, // _snd_human_chatter "they’re everywhere!"
	{15440, _sound_is_normal, _sound_cannot_be_restarted}, // _snd_assimilated_human_chatter "thank god it’s you!"
	{15450, _sound_is_normal, _sound_cannot_be_restarted, _seventy_percent}, // _snd_human_trash_talk "eat that!"
	{15460, _sound_is_normal, _sound_cannot_be_restarted}, // _snd_human_apology "oops!"
	{15470, _sound_is_normal, _sound_cannot_be_restarted}, // _snd_human_activation "they’re over here!"
	{15480, _sound_is_normal, _sound_cannot_be_restarted, _fifty_percent}, // _snd_human_clear "out of the way!"
	{15490, _sound_is_normal, _sound_cannot_be_restarted, _thirty_percent}, // _snd_human_stop_shooting_me_you_bastard
	{15500, _sound_is_normal, _sound_cannot_be_restarted, _fifty_percent}, // _snd_human_area_secure
	{15510, _sound_is_normal, _sound_cannot_be_restarted}, // _snd_kill_the_player

	// looping ambient
	{14000, _sound_is_normal, _sound_is_ambient}, // _snd_water
	{14010, _sound_is_normal, _sound_is_ambient}, // _snd_sewage
	{14020, _sound_is_normal, _sound_is_ambient}, // _snd_lava
	{14030, _sound_is_normal, _sound_is_ambient}, // _snd_goo
	{14040, _sound_is_normal, _sound_is_ambient}, // _snd_under_media
	{14050, _sound_is_normal, _sound_is_ambient}, // _snd_wind
	{14060, _sound_is_normal, _sound_is_ambient}, // _snd_waterfall
	{14070, _sound_is_normal, _sound_is_ambient}, // _snd_siren,
	{14080, _sound_is_normal, _sound_is_ambient}, // _snd_fan,
	{14090, _sound_is_normal, _sound_is_ambient}, // _snd_spht_door
	{14100, _sound_is_normal, _sound_is_ambient}, // _snd_spht_platform
	{14120, _sound_is_normal, _sound_is_ambient}, // _snd_unused4
	{14130, _sound_is_normal, _sound_is_ambient}, // _snd_heavy_spht_platform
	{14140, _sound_is_normal, _sound_is_ambient}, // _snd_light_machinery
	{14150, _sound_is_normal, _sound_is_ambient}, // _snd_heavy_machinery
	{14160, _sound_is_normal, _sound_is_ambient}, // _snd_transformer
	{14170, _sound_is_normal, _sound_is_ambient}, // _snd_sparking_transformer

	// random ambient
	{14500, _sound_is_normal}, // _snd_water_drip
	
	// water
	{19000, _sound_is_quiet, _sound_cannot_be_media_obstructed}, // _snd_walking_in_water
	{19010, _sound_is_normal, _sound_cannot_be_media_obstructed}, // _snd_exit_water
	{19020, _sound_is_normal, _sound_cannot_be_media_obstructed}, // _snd_enter_water
	{19100, _sound_is_normal, _sound_cannot_be_media_obstructed}, // _snd_small_water_splash
	{19110, _sound_is_normal, _sound_cannot_be_media_obstructed}, // _snd_medium_water_splash
	{19120, _sound_is_normal, _sound_cannot_be_media_obstructed}, // _snd_large_water_splash

	// lava
	{19050, _sound_is_quiet, _sound_cannot_be_media_obstructed}, // _snd_walking_in_lava
	{19060, _sound_is_normal, _sound_cannot_be_media_obstructed}, // _snd_enter_lava
	{19070, _sound_is_normal, _sound_cannot_be_media_obstructed}, // _snd_exit_lava
	{19130, _sound_is_normal, _sound_cannot_be_media_obstructed}, // _snd_small_lava_splash
	{19140, _sound_is_normal, _sound_cannot_be_media_obstructed}, // _snd_medium_lava_splash
	{19150, _sound_is_normal, _sound_cannot_be_media_obstructed}, // _snd_large_lava_splash

	// sewage
	{19160, _sound_is_quiet, _sound_cannot_be_media_obstructed}, // _snd_walking_in_sewage
	{19180, _sound_is_normal, _sound_cannot_be_media_obstructed}, // _snd_exit_sewage
	{19170, _sound_is_normal, _sound_cannot_be_media_obstructed}, // _snd_enter_sewage
	{19190, _sound_is_normal, _sound_cannot_be_media_obstructed}, // _snd_small_sewage_splash
	{19200, _sound_is_normal, _sound_cannot_be_media_obstructed}, // _snd_medium_sewage_splash
	{19210, _sound_is_normal, _sound_cannot_be_media_obstructed}, // _snd_large_sewage_splash

	// goo
	{19220, _sound_is_quiet, _sound_cannot_be_media_obstructed}, // _snd_walking_in_goo
	{19240, _sound_is_normal, _sound_cannot_be_media_obstructed}, // _snd_exit_goo
	{19230, _sound_is_normal, _sound_cannot_be_media_obstructed}, // _snd_enter_goo
	{19250, _sound_is_normal, _sound_cannot_be_media_obstructed}, // _snd_small_goo_splash
	{19260, _sound_is_normal, _sound_cannot_be_media_obstructed}, // _snd_medium_goo_splash
	{19270, _sound_is_normal, _sound_cannot_be_media_obstructed}, // _snd_large_goo_splash

	{11290, _sound_is_normal}, // _snd_major_fusion_firing
	{11330, _sound_is_quiet}, // _snd_major_fusion_charged

	{11210, _sound_is_quiet}, // _snd_assault_rifle_reloading
	{11270, _sound_is_quiet}, // _snd_assault_rifle_shell_casings
	
	{11220, _sound_is_loud}, // _snd_shotgun_firing
	{11230, _sound_is_quiet}, // _snd_shotgun_reloading
	
	{11300, _sound_is_normal}, // _snd_ball_bounce
	{11310, _sound_is_normal}, // _snd_you_are_it
	{11320, _sound_is_normal}, // _snd_got_ball

	{11240, _sound_is_quiet}, // _snd_computer_interface_logon
	{11250, _sound_is_quiet}, // _snd_computer_interface_logout
	{11260, _sound_is_quiet}, // _snd_computer_interface_page

	{13000, _sound_is_normal, _sound_is_ambient}, // _snd_heavy_spht_door
	{13010, _sound_is_normal}, // _snd_heavy_spht_door_opening
	{13020, _sound_is_normal}, // _snd_heavy_spht_door_closing
	{13030, _sound_is_normal}, // _snd_heavy_spht_door_open
	{13040, _sound_is_normal}, // _snd_heavy_spht_door_closed
	{13050, _sound_is_normal}, // _snd_heavy_spht_door_obstructed

	{15600, _sound_is_normal}, // _snd_hunter_activate
	{15610, _sound_is_normal}, // _snd_hunter_attack
	{15620, _sound_is_normal}, // _snd_hunter_dying
	{15630, _sound_is_loud}, // _snd_hunter_landing
	{15640, _sound_is_loud}, // _snd_hunter_exploding
	{15650, _sound_is_normal}, // _snd_hunter_projectile_hit
	{15660, _sound_is_normal}, // _snd_hunter_projectile_flyby

	{15700, _sound_is_normal}, // _snd_enforcer_activate
	{15710, _sound_is_loud}, // _snd_enforcer_attack
	{15720, _sound_is_normal}, // _snd_enforcer_projectile_hit
	{15730, _sound_is_normal}, // _snd_enforcer_projectile_flyby

	{15800, _sound_is_normal}, // _snd_yeti_melee_attack
	{15810, _sound_is_normal}, // _snd_yeti_melee_attack_hit
	{15820, _sound_is_normal}, // _snd_yeti_projectile_attack
	{15860, _sound_is_normal}, // _snd_yeti_projectile_sewage_attack_hit
	{15870, _sound_is_normal}, // _snd_yeti_projectile_sewage_flyby
	{15830, _sound_is_normal}, // _snd_yeti_projectile_lava_attack_hit
	{15840, _sound_is_normal}, // _snd_yeti_projectile_lava_flyby
	{15850, _sound_is_normal}, // _snd_yeti_dying
	
	{14180, _sound_is_normal, _sound_is_ambient}, // _snd_machine_binder
	{14190, _sound_is_normal, _sound_is_ambient}, // _snd_machine_bookpress
	{14200, _sound_is_normal, _sound_is_ambient}, // _snd_machine_puncher
	{14210, _sound_is_normal, _sound_is_ambient}, // _snd_electric
	{14220, _sound_is_normal, _sound_is_ambient}, // _snd_alarm
	{14230, _sound_is_normal, _sound_is_ambient}, // _snd_night_wind
	
	{14510, _sound_is_quiet}, // _snd_surface_explosion
	{14520, _sound_is_quiet}, // _snd_underground_explosion

	{16000, _sound_is_normal}, // _snd_defender_attack
	{16010, _sound_is_normal}, // _snd_defender_hit
	{16020, _sound_is_normal}, // _snd_defender_flyby
	{16030, _sound_is_normal}, // _snd_defender_being_hit
	{16040, _sound_is_loud}, // _snd_defender_exploding

	{17000, _sound_is_normal, 0, _always, 3*FIXED_ONE/4, 3*FIXED_ONE/2}, // _snd_tick_chatter
	{17010, _sound_is_normal}, // _snd_tick_falling
	{17020, _sound_is_quiet}, // _snd_tick_flapping
	{17030, _sound_is_loud}, // _snd_tick_exploding

	{14530, _sound_is_normal}, // _snd_ceiling_lamp_exploding

	{14545, _sound_is_normal}, // _snd_pfhor_platform_starting
	{14550, _sound_is_normal}, // _snd_pfhor_platform_stopping
	{14560, _sound_is_normal, _sound_is_ambient}, // _ambient_snd_pfhor_platform

	{14570, _sound_is_normal}, // _snd_pfhor_door_opening
	{14580, _sound_is_normal}, // _snd_pfhor_door_closing
	{14590, _sound_is_normal}, // _snd_pfhor_door_obstructed
	{14600, _sound_is_normal, _sound_is_ambient}, // _ambient_snd_pfhor_door

	{14610, _sound_is_quiet}, // _snd_pfhor_switch_off
	{14620, _sound_is_quiet}, // _snd_pfhor_switch_on

	{14630, _sound_is_loud}, // _snd_juggernaut_firing
	{14640, _sound_is_loud}, // _snd_juggernaut_warning
	{14650, _sound_is_loud}, // _snd_juggernaut_exploding
	{14660, _sound_is_loud}, // _snd_juggernaut_preparing_to_fire
	
	{14670, _sound_is_loud}, // _snd_enforcer_exploding

	{14240, _sound_is_normal, _sound_is_ambient}, // _snd_alien_noise1
	{14250, _sound_is_normal, _sound_is_ambient}, // _snd_alien_noise2
};
*/

#endif

