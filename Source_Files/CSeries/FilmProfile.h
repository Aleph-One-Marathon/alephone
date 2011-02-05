#ifndef _FILM_PROFILE_
#define _FILM_PROFILE_
/*

	Copyright (C) 2011 and beyond by Bungie Studios, Inc.
	and the "Aleph One" developers.
 
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
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
};

extern FilmProfile film_profile;

enum FilmProfileType {
	FILM_PROFILE_DEFAULT,
	FILM_PROFILE_MARATHON_2,
};

void load_film_profile(FilmProfileType type, bool reload_mml = true);

#endif
