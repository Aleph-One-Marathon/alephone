#ifndef __COMPUTER_INTERFACE_H
#define __COMPUTER_INTERFACE_H

/*
	computer_interface.h

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

	Tuesday, August 23, 1994 11:25:40 PM (ajr)
	Thursday, May 25, 1995 5:18:03 PM- rewriting.

	New paradigm:
	Groups each start with one of the following groups:
	 #UNFINISHED, #SUCCESS, #FAILURE

	First is shown the
	#LOGON XXXXX
	
	Then there are any number of groups with:
	#INFORMATION, #CHECKPOINT, #SOUND, #MOVIE, #TRACK
	
	And a final:
	#INTERLEVEL TELEPORT, #INTRALEVEL TELEPORT		
	
	Each group ends with:
	#END

	Groupings:
	#logon XXXX- login message (XXXX is shape for login screen)
	#unfinished- unfinished message
	#success- success message
	#failure- failure message
	#information- information
	#briefing XX- briefing, then load XX
	#checkpoint XX- Checkpoint xx (associated with goal) 
	#sound XXXX- play sound XXXX
	#movie XXXX- play movie XXXX (from Movie file)
	#track XXXX- play soundtrack XXXX (from Music file)
	#interlevel teleport XXX- go to level XXX
	#intralevel teleport XXX- go to polygon XXX
	#pict XXXX- diplay the pict resource XXXX

	Special embedded keys:   
	$B- Bold on
	$b- bold off
	$I- Italic on
	$i- italic off
	$U- underline on
	$u- underline off
	$- anything else is passed through unchanged

	Preprocessed format:
	static:
		int32 total_length;
		short grouping_count;
		short font_changes_count;
		short total_text_length;
	dynamic:
		struct terminal_groupings groups[grouping_count];
		struct text_face_data[font_changes_count];
		char text;
*/

#include "cstypes.h"

/* ------------ structures */
struct static_preprocessed_terminal_data {
	int16 total_length;
	int16 flags;
	int16 lines_per_page; /* Added for internationalization/sync problems */
	int16 grouping_count;
	int16 font_changes_count;
};
const int SIZEOF_static_preprocessed_terminal_data = 10;

struct view_terminal_data {
	short top, left, bottom, right;
	short vertical_offset;
};

// External-data size of current terminal state
const int SIZEOF_player_terminal_data = 20;

/* ------------ prototypes */
void initialize_terminal_manager(void);
void initialize_player_terminal_info(short player_index);
void enter_computer_interface(short player_index, short text_number, short completion_flag);
void _render_computer_interface(void);
void update_player_for_terminal_mode(short player_index);
void update_player_keys_for_terminal(short player_index, uint32 action_flags);
uint32 build_terminal_action_flags(char *keymap);
void dirty_terminal_view(short player_index);
void abort_terminal_mode(short player_index);

bool player_in_terminal_mode(short player_index);

// LP: to pack and unpack this data;
// these hide the unpacked data from the outside world.
// "Map terminal" means the terminal data read in from the map;
// "player terminal" means the terminal state for each player.
// For the map terminal data, the "count" is number of packed bytes.

extern void unpack_map_terminal_data(uint8 *Stream, size_t Count);
extern void pack_map_terminal_data(uint8 *Stream, size_t Count);
uint8 *unpack_player_terminal_data(uint8 *Stream, size_t Count);
uint8 *pack_player_terminal_data(uint8 *Stream, size_t Count);

extern size_t calculate_packed_terminal_data_length(void);

void clear_compiled_terminal_cache();

#endif
