#ifndef __SONG_DEFINITIONS_H
#define __SONG_DEFINITIONS_H

/*

	song_definitions.h
	Wednesday, July 12, 1995 11:45:34 PM- rdm created.

*/

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
