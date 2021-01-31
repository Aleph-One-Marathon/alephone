#ifndef __VBL_H
#define __VBL_H

/*
	vbl.h

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

	Friday, September 29, 1995 3:24:01 PM- rdm created.

Aug 12, 2000 (Loren Petrich):
	Using object-oriented file handler; revising definitions accordingly

Jul 5, 2000 (Loren Petrich):
	Added XML support for setting up the keyboard
*/

// LP: CodeWarrior complains unless I give the full definition of these classes
#include "FileHandler.h"

/* ------------ prototypes/VBL.C */
bool setup_for_replay_from_file(FileSpecifier& File, uint32 map_checksum, bool prompt_to_export = false);
bool setup_replay_from_random_resource(uint32 map_checksum);

void start_recording(void);

bool find_replay_to_use(bool ask_user, FileSpecifier& File);

void set_recording_header_data(short number_of_players, short level_number, uint32 map_checksum,
	short version, struct player_start_data *starts, struct game_data *game_information);
void get_recording_header_data(short *number_of_players, short *level_number, uint32 *map_checksum,
	short *version, struct player_start_data *starts, struct game_data *game_information);

bool input_controller(void);
void increment_heartbeat_count(int value = 1);

/* ------------ prototypes/VBL_MACINTOSH.C */
void initialize_keyboard_controller(void);

/* true if it found it, false otherwise. always fills in vrefnum and dirid*/
bool get_recording_filedesc(FileSpecifier& File);
void move_replay(void);
uint32 parse_keymap(void);

bool setup_replay_from_random_resource(uint32 map_checksum);

#ifdef DEBUG_REPLAY
struct recorded_flag {
	uint32 flag;
	int16 player_index;
};

void open_stream_file(void);
void write_flags(struct recorded_flag *buffer, int32 count);
static void debug_stream_of_flags(uint32 action_flag, short player_index);
static void close_stream_file(void);
#endif


class InfoTree;
void parse_mml_keyboard(const InfoTree& root);
void reset_mml_keyboard();

#endif
