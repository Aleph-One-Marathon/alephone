#ifndef _FILM_PROFILE_
#define _FILM_PROFILE_
/*

	Copyright (C) 2011 and beyond by Bungie Studios, Inc.
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
	
	Film profiles tell Aleph One exactly how to behave when
	playing back films

	Written by Gregory Smith, 2011
*/

struct FilmProfile
{
	// some LP bug fix
	bool keyframe_fix;

	// ghs changed this so that suiciding in tag made you it
	bool damage_aggressor_last_in_tag;

	// some LP bug fix
	bool swipe_nearby_items_fix;
	
	// ghs added this to stop monsters from spawning in net games
	bool initial_monster_fix;

	// LP changed arctangent and distance2d to handle long distances
	bool long_distance_physics;

	// LP added to support animated items
	bool animate_items;

	// astrange changed M2's fast PIN to a new version which
	// doesn't shortcircuit and returns a different result when
	// floor/ceiling are reversed
	bool inexplicable_pin_change;

	// LP increased the dynamic limits, which persisted through 1.0
	bool increased_dynamic_limits_1_0;

	// 1.1 reverted number of paths to preserve original AI behavior
	bool increased_dynamic_limits_1_1;
    
	// Infinity has an improved line_is_obstructed
	bool line_is_obstructed_fix;

	// Aleph One implements pass_media_boundary one way
	bool a1_smg;

	// Marathon Infinity implements pass_media_boundary a different way
	bool infinity_smg;

	// Marathon 2 and Infinity give fusion damage a vertical
	// component when delta_vitality is greater than 100
	bool use_vertical_kick_threshold;

	// Marathon Infinity fixes tag suicides in a different way
	bool infinity_tag_fix;

	// Marathon Infinity always adds adjacent polygons to the
	// intersecting indexes (unmerged maps only)
	bool adjacent_polygons_always_intersect;

	// Aleph One moved object initialization to improve Lua access
	bool early_object_initialization;

	// Aleph One 1.1 fixes
	bool fix_sliding_on_platforms;
	bool prevent_dead_projectile_owners;
	bool validate_random_ranged_attack;
	bool allow_short_kamikaze;
	bool ketchup_fix;
	bool lua_increments_rng;
	bool destroy_players_ball_fix;
	bool calculate_terminal_lines_correctly;
	bool key_frame_zero_shrapnel_fix; // for M1 lookers and simulacra
	bool count_dead_dropped_items_correctly;

	// Aleph One 1.2 fixes
	bool m1_low_gravity_projectiles;
	bool m1_buggy_repair_goal;
	bool find_action_key_target_has_side_effects;

	// Aleph One 1.3 fixes
	bool m1_object_unused; // location.z and flags are unused in Marathon
	bool m1_platform_flood; // checks more than just adjacent polygons
	bool m1_teleport_without_delay; // Marathon terminals teleport immediately

	// Aleph One 1.4 fixes
	bool better_terminal_word_wrap; // fixes rare infinity films
	bool lua_monster_killed_trigger_fix;
};

extern FilmProfile film_profile;

enum FilmProfileType {
	FILM_PROFILE_ALEPH_ONE_1_0,
	FILM_PROFILE_MARATHON_2,
	FILM_PROFILE_MARATHON_INFINITY,
	FILM_PROFILE_ALEPH_ONE_1_1,
	FILM_PROFILE_ALEPH_ONE_1_2,
	FILM_PROFILE_ALEPH_ONE_1_3,
	FILM_PROFILE_DEFAULT,
};

void load_film_profile(FilmProfileType type, bool reload_mml = true);

#endif
