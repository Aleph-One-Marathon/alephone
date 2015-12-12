#ifndef __SOUNDMANAGERENUMS_H
#define __SOUNDMANAGERENUMS_H

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

*/

// ghs: moved these here because SoundManager's header file is enormous already

#include "cstypes.h"

/* ---------- sound codes */

enum /* ambient sound codes */
{
	_ambient_snd_water,
	_ambient_snd_sewage,
	_ambient_snd_lava,
	_ambient_snd_goo,
	_ambient_snd_under_media,
	_ambient_snd_wind,
	_ambient_snd_waterfall,
	_ambient_snd_siren,
	_ambient_snd_fan,
	_ambient_snd_spht_door,
	_ambient_snd_spht_platform,
	_ambient_snd_heavy_spht_door,
	_ambient_snd_heavy_spht_platform,
	_ambient_snd_light_machinery,
	_ambient_snd_heavy_machinery,
	_ambient_snd_transformer,
	_ambient_snd_sparking_transformer,
	_ambient_snd_machine_binder,
	_ambient_snd_machine_bookpress,
	_ambient_snd_machine_puncher,
	_ambient_snd_electric,
	_ambient_snd_alarm,
	_ambient_snd_night_wind,
	_ambient_snd_pfhor_door,
	_ambient_snd_pfhor_platform,
	_ambient_snd_alien_noise1,
	_ambient_snd_alien_noise2,
	// LP addition:
	_ambient_snd_alien_harmonics,
	
	NUMBER_OF_AMBIENT_SOUND_DEFINITIONS
};

enum /* random sound codes */
{
	_random_snd_water_drip,
	_random_snd_surface_explosion,
	_random_snd_underground_explosion,
	_random_snd_owl,
	// LP addition:
	_random_snd_creak,

	NUMBER_OF_RANDOM_SOUND_DEFINITIONS
};

enum /* sound codes */
{
	_snd_startup,
	_snd_teleport_in,
	_snd_teleport_out,
	_snd_body_being_crunched,
	// LP change:
	_snd_creak,
	// _snd_nuclear_hard_death,
	_snd_absorbed,
	
	_snd_breathing,
	_snd_oxygen_warning,
	_snd_suffocation,

	_snd_energy_refuel,
	_snd_oxygen_refuel,
	_snd_cant_toggle_switch,
	_snd_switch_on,
	_snd_switch_off,
	_snd_puzzle_switch,
	_snd_chip_insertion,
	_snd_pattern_buffer,
	_snd_destroy_control_panel,
	
	_snd_adjust_volume,
	_snd_got_powerup,
	_snd_got_item,

	_snd_bullet_ricochet,
	_snd_metallic_ricochet,
	_snd_empty_gun,

	_snd_spht_door_opening,
	_snd_spht_door_closing,
	_snd_spht_door_obstructed,
	
	_snd_spht_platform_starting,
	_snd_spht_platform_stopping,

	_snd_owl,
	// LP change:
	_snd_smg_firing,
	_snd_smg_reloading,
	// _snd_unused2,
	// _snd_unused3,
	
	_snd_heavy_spht_platform_starting,
	_snd_heavy_spht_platform_stopping,

	_snd_fist_hitting,
	
	_snd_magnum_firing,
	_snd_magnum_reloading,

	_snd_assault_rifle_firing,
	_snd_grenade_launcher_firing,
	_snd_grenade_exploding,
	_snd_grenade_flyby,
	
	_snd_fusion_firing,
	_snd_fusion_exploding,
	_snd_fusion_flyby,
	_snd_fusion_charging,

	_snd_rocket_exploding,
	_snd_rocket_flyby,
	_snd_rocket_firing,
	
	_snd_flamethrower,

	_snd_body_falling,
	_snd_body_exploding,
	_snd_bullet_hitting_flesh,

	_snd_fighter_activate,
	_snd_fighter_wail,
	_snd_fighter_scream,
	_snd_fighter_chatter,
	_snd_fighter_attack,
	_snd_fighter_projectile_hit,
	_snd_fighter_projectile_flyby,

	_snd_compiler_attack,
	_snd_compiler_death,
	_snd_compiler_hit,
	_snd_compiler_projectile_flyby,
	_snd_compiler_projectile_hit,
	
	_snd_cyborg_moving,
	_snd_cyborg_attack,
	_snd_cyborg_hit,
	_snd_cyborg_death,
	_snd_cyborg_projectile_bounce,
	_snd_cyborg_projectile_hit,
	_snd_cyborg_projectile_flyby,

	_snd_hummer_activate,
	_snd_hummer_start_attack,
	_snd_hummer_attack,
	_snd_hummer_dying,
	_snd_hummer_death,
	_snd_hummer_projectile_hit,
	_snd_hummer_projectile_flyby,

	_snd_human_wail,
	_snd_human_scream,
	_snd_human_hit,
	_snd_human_chatter,
	_snd_assimilated_human_chatter,
	_snd_human_trash_talk,
	_snd_human_apology,
	_snd_human_activation,
	_snd_human_clear,
	_snd_human_stop_shooting_me_you_bastard,
	_snd_human_area_secure,
	_snd_kill_the_player,
	
	_snd_water,
	_snd_sewage,
	_snd_lava,
	_snd_goo,
	_snd_under_media,
	_snd_wind,
	_snd_waterfall,
	_snd_siren,
	_snd_fan,
	_snd_spht_door,
	_snd_spht_platform,
	// LP change:
	_snd_alien_harmonics,
	// _snd_unused4,
	_snd_heavy_spht_platform,
	_snd_light_machinery,
	_snd_heavy_machinery,
	_snd_transformer,
	_snd_sparking_transformer,

	_snd_water_drip,
	
	_snd_walking_in_water,
	_snd_exit_water,
	_snd_enter_water,
	_snd_small_water_splash,
	_snd_medium_water_splash,
	_snd_large_water_splash,

	_snd_walking_in_lava,
	_snd_enter_lava,
	_snd_exit_lava,
	_snd_small_lava_splash,
	_snd_medium_lava_splash,
	_snd_large_lava_splash,

	_snd_walking_in_sewage,
	_snd_exit_sewage,
	_snd_enter_sewage,
	_snd_small_sewage_splash,
	_snd_medium_sewage_splash,
	_snd_large_sewage_splash,

	_snd_walking_in_goo,
	_snd_exit_goo,
	_snd_enter_goo,
	_snd_small_goo_splash,
	_snd_medium_goo_splash,
	_snd_large_goo_splash,

	_snd_major_fusion_firing,
	_snd_major_fusion_charged,

	_snd_assault_rifle_reloading,
	_snd_assault_rifle_shell_casings,
	
	_snd_shotgun_firing,
	_snd_shotgun_reloading,
	
	_snd_ball_bounce,
	_snd_you_are_it,
	_snd_got_ball,
	
	_snd_computer_interface_logon,
	_snd_computer_interface_logout,
	_snd_computer_interface_page,

	_snd_heavy_spht_door,
	_snd_heavy_spht_door_opening,
	_snd_heavy_spht_door_closing,
	_snd_heavy_spht_door_open,
	_snd_heavy_spht_door_closed,
	_snd_heavy_spht_door_obstructed,

	_snd_hunter_activate,
	_snd_hunter_attack,
	_snd_hunter_dying,
	_snd_hunter_landing,
	_snd_hunter_exploding,
	_snd_hunter_projectile_hit,
	_snd_hunter_projectile_flyby,

	_snd_enforcer_activate,
	_snd_enforcer_attack,
	_snd_enforcer_projectile_hit,
	_snd_enforcer_projectile_flyby,

	_snd_yeti_melee_attack,
	_snd_yeti_melee_attack_hit,
	_snd_yeti_projectile_attack,
	_snd_yeti_projectile_sewage_attack_hit,
	_snd_yeti_projectile_sewage_flyby,
	_snd_yeti_projectile_lava_attack_hit,
	_snd_yeti_projectile_lava_flyby,
	_snd_yeti_dying,

	_snd_machine_binder,
	_snd_machine_bookpress,
	_snd_machine_puncher,
	_snd_electric,
	_snd_alarm,
	_snd_night_wind,
	
	_snd_surface_explosion,
	_snd_underground_explosion,

	_snd_defender_attack,
	_snd_defender_hit,
	_snd_defender_flyby,
	_snd_defender_being_hit,
	_snd_defender_exploding,

	_snd_tick_chatter,
	_snd_tick_falling,
	_snd_tick_flapping,
	_snd_tick_exploding,

	_snd_ceiling_lamp_exploding,

	_snd_pfhor_platform_starting,
	_snd_pfhor_platform_stopping,
	_snd_pfhor_platform,

	_snd_pfhor_door_opening,
	_snd_pfhor_door_closing,
	_snd_pfhor_door_obstructed,
	_snd_pfhor_door,

	_snd_pfhor_switch_off,
	_snd_pfhor_switch_on,

	_snd_juggernaut_firing,
	_snd_juggernaut_warning,
	_snd_juggernaut_exploding,
	_snd_juggernaut_preparing_to_fire,

	_snd_enforcer_exploding,

	_snd_alien_noise1,
	_snd_alien_noise2,
	
	// LP addition: this means that there are more Moo sound types
	// than M2 ones.
	_snd_civilian_fusion_wail,
	_snd_civilian_fusion_scream,
	_snd_civilian_fusion_hit,
	_snd_civilian_fusion_chatter,
	_snd_assimilated_civilian_fusion_chatter,
	_snd_civilian_fusion_trash_talk,
	_snd_civilian_fusion_apology,
	_snd_civilian_fusion_activation,
	_snd_civilian_fusion_clear,
	_snd_civilian_fusion_stop_shooting_me_you_bastard,
	_snd_civilian_fusion_area_secure,
	_snd_civilian_fusion_kill_the_player,

	NUMBER_OF_SOUND_DEFINITIONS
};

enum
{
	NUMBER_OF_SOUND_VOLUME_LEVELS= 8,
	
	MAXIMUM_SOUND_VOLUME_BITS= 8,
	MAXIMUM_SOUND_VOLUME= 1<<MAXIMUM_SOUND_VOLUME_BITS,
};

enum // sound sources
{
	_8bit_22k_source,
	_16bit_22k_source,
	
	NUMBER_OF_SOUND_SOURCES
};

enum // initialization flags (some of these are used by the prefs, which fixes them)
{
	_stereo_flag= 0x0001, /* play sounds in stereo [prefs] */
	_dynamic_tracking_flag= 0x0002, /* tracks sound sources during idle_proc [prefs] */
	_doppler_shift_flag= 0x0004, /* adjusts sound pitch during idle_proc */
	_ambient_sound_flag= 0x0008, /* plays and tracks ambient sounds (valid iff _dynamic_tracking_flag) [prefs] */
	_16bit_sound_flag= 0x0010, /* loads 16bit audio instead of 8bit [prefs] */
	_more_sounds_flag= 0x0020, /* loads all permutations; only loads #0 if false [prefs] */
	_relative_volume_flag = 0x0040, /* LP: Ian Rickard's relative-volume flag [prefs] */
	_extra_memory_flag= 0x0100, /* double usual memory */
	_extra_extra_memory_flag= 0x0200, /* LP: quadruple usual memory, because RAM is more available */
	_zero_restart_delay = 0x0400 /* ghs: restart sounds immediately */
};

enum // _sound_obstructed_proc() flags
{
	_sound_was_obstructed= 0x0001, // no clear path between source and listener
	_sound_was_media_obstructed= 0x0002, // source and listener are on different sides of the media
	_sound_was_media_muffled= 0x0004 // source and listener both under the same media
};

enum // frequencies
{
	_lower_frequency= FIXED_ONE-FIXED_ONE/8,
	_normal_frequency= FIXED_ONE,
	_higher_frequency= FIXED_ONE+FIXED_ONE/8,
	_m1_high_frequency= FIXED_ONE+FIXED_ONE/4
};

#endif

