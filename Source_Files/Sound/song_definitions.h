#ifndef __SONG_DEFINITIONS_H
#define __SONG_DEFINITIONS_H

/*
	song_definitions.h

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

	Wednesday, July 12, 1995 11:45:34 PM- rdm created.

*/

#include "csmisc.h"

#define RANDOM_COUNT(x) (-(x))

enum {
	_no_song_flags= 0x0000,
	_song_automatically_loops= 0x0001
};

struct sound_snippet {
	int32 start_offset;
	int32 end_offset;
};

struct song_definition {
	int16 flags;
	int32 sound_start;
	struct sound_snippet introduction;
	struct sound_snippet chorus;
	int16 chorus_count; /* If it is negative then it is a random count */
	struct sound_snippet trailer;
	int32 restart_delay;
};

struct song_definition songs[]= {
	{ _song_automatically_loops, 
		0l, 
		{ 0, 0}, // intro
		{ 0, 0},	// chorus
		RANDOM_COUNT(3), // chorus count
		{ 0, 0}, // trailer
		30*MACHINE_TICKS_PER_SECOND
	}
};

#endif
