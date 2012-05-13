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

	// LP increased the dynamic limits
	bool increased_dynamic_limits;

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
	
	// multi-level films need to preserve action queues between levels
	bool reset_action_queues;
};

extern FilmProfile film_profile;

enum FilmProfileType {
	FILM_PROFILE_DEFAULT,
	FILM_PROFILE_MARATHON_2,
	FILM_PROFILE_MARATHON_INFINITY
};

void load_film_profile(FilmProfileType type, bool reload_mml = true);

#endif
