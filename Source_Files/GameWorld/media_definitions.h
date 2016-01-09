#ifndef __MEDIA_DEFINITIONS_H
#define __MEDIA_DEFINITIONS_H

/*
MEDIA_DEFINITIONS.H

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

Sunday, March 26, 1995 11:09:26 PM  (Jason')

Feb 3, 2000 (Loren Petrich):
	Added Jjaro media type

May 17, 2000 (Loren Petrich):
	Jjaro media type has its own fade effect
*/

#include "effects.h"
#include "fades.h"
#include "media.h"
#include "SoundManagerEnums.h"

/* ---------- structures */

struct media_definition
{
	int16 collection, shape, shape_count, shape_frequency;
	int16 transfer_mode;
	
	int16 damage_frequency; // mask&ticks
	struct damage_definition damage;

	int16 detonation_effects[NUMBER_OF_MEDIA_DETONATION_TYPES];	
	int16 sounds[NUMBER_OF_MEDIA_SOUNDS];
	
	int16 submerged_fade_effect;
};

/* ---------- globals */

// LP addition: added JjaroGoo support (copy of sewage)
static struct media_definition media_definitions[NUMBER_OF_MEDIA_TYPES]=
{
	/* _media_water */
	{
		_collection_walls1, 19, 1, 0, /* collection, shape, shape_count, frequency */
		_xfer_normal, /* transfer mode */
		
		0, {NONE, 0, 0, 0, FIXED_ONE}, /* damage frequency and definition */
		
		{_effect_small_water_splash, _effect_medium_water_splash, _effect_large_water_splash, _effect_large_water_emergence}, /* small, medium, large detonation effects */
		{NONE, NONE, _snd_enter_water, _snd_exit_water,
			_snd_walking_in_water, _ambient_snd_water, _ambient_snd_under_media,
			_snd_enter_water, _snd_exit_water},
		
		_effect_under_water, /* submerged fade effect */
	},
	
	/* _media_lava */
	{
		_collection_walls2, 12, 1, 0, /* collection, shape, shape_count, frequency */
		_xfer_normal, /* transfer mode */
		
		0xf, {_damage_lava, _alien_damage, 16, 0, FIXED_ONE}, /* damage frequency and definition */
		
		{_effect_small_lava_splash, _effect_medium_lava_splash, _effect_large_lava_splash, _effect_large_lava_emergence}, /* small, medium, large detonation effects */
		{NONE, NONE, _snd_enter_lava, _snd_exit_lava,
			_snd_walking_in_lava, _ambient_snd_lava, _ambient_snd_under_media,
			_snd_enter_lava, _snd_exit_lava},

		_effect_under_lava, /* submerged fade effect */
	},
	
	/* _media_goo */
	{
		_collection_walls5, 5, 1, 0, /* collection, shape, shape_count, frequency */
		_xfer_normal, /* transfer mode */
		
		0x7, {_damage_goo, _alien_damage, 8, 0, FIXED_ONE}, /* damage frequency and definition */
		
		{_effect_small_goo_splash, _effect_medium_goo_splash, _effect_large_goo_splash, _effect_large_goo_emergence}, /* small, medium, large detonation effects */
		{NONE, NONE, _snd_enter_lava, _snd_exit_lava,
			_snd_walking_in_lava, _ambient_snd_goo, _ambient_snd_under_media,
			_snd_enter_lava, _snd_exit_lava},

		_effect_under_goo, /* submerged fade effect */
	},
	
	/* _media_sewage */
	{
		_collection_walls3, 13, 1, 0, /* collection, shape, shape_count, frequency */
		_xfer_normal, /* transfer mode */
		
		0, {NONE, 0, 0, 0, FIXED_ONE}, /* damage frequency and definition */
		
		{_effect_small_sewage_splash, _effect_medium_sewage_splash, _effect_large_sewage_splash, _effect_large_sewage_emergence}, /* small, medium, large detonation effects */
		{NONE, NONE, _snd_enter_sewage, _snd_exit_sewage,
			NONE, _ambient_snd_sewage, _ambient_snd_under_media,
			_snd_enter_sewage, _snd_exit_sewage},

		_effect_under_sewage, /* submerged fade effect */
	},
	
	/* _media_jjaro */
	{
		_collection_walls4, 13, 1, 0, /* collection, shape, shape_count, frequency */
		_xfer_normal, /* transfer mode */
		
		0, {NONE, 0, 0, 0, FIXED_ONE}, /* damage frequency and definition */
		
		{_effect_small_jjaro_splash, _effect_medium_jjaro_splash, _effect_large_jjaro_splash, _effect_large_jjaro_emergence}, /* small, medium, large detonation effects */
		{NONE, NONE, _snd_enter_sewage, _snd_exit_sewage,
			NONE, _ambient_snd_sewage, _ambient_snd_under_media,
			_snd_enter_sewage, _snd_exit_sewage},

		_effect_under_jjaro, /* submerged fade effect */
	},
};

#endif
