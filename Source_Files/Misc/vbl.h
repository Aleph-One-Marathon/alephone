#ifndef __VBL_H
#define __VBL_H

/*

	vbl.h
	Friday, September 29, 1995 3:24:01 PM- rdm created.

Aug 12, 2000 (Loren Petrich):
	Using object-oriented file handler; revising definitions accordingly

*/

#include "FileHandler.h"

/* ------------ prototypes/VBL.C */
boolean setup_for_replay_from_file(FileObject& File, unsigned long map_checksum);
// boolean setup_for_replay_from_file(FileDesc *file, unsigned long map_checksum);
boolean setup_replay_from_random_resource(unsigned long map_checksum);

void start_recording(void);

boolean find_replay_to_use(boolean ask_user, FileObject& File);
// boolean find_replay_to_use(boolean ask_user, FileDesc *file);

void set_recording_header_data(short number_of_players, short level_number, unsigned long map_checksum,
	short version, struct player_start_data *starts, struct game_data *game_information);
void get_recording_header_data(short *number_of_players, short *level_number, unsigned long *map_checksum,
	short *version, struct player_start_data *starts, struct game_data *game_information);

boolean input_controller(void);

/* ------------ prototypes/VBL_MACINTOSH.C */
void initialize_keyboard_controller(void);

// boolean find_replay_to_use(boolean ask_user, FileDesc *file);
// Made a member of class FileObject.
// boolean get_freespace_on_disk(FileDesc *file, unsigned long *free_space);
void initialize_keyboard_controller(void);

/* true if it found it, false otherwise. always fills in vrefnum and dirid*/
boolean get_recording_filedesc(FileObject& File);
// boolean get_recording_filedesc(FileDesc *file);
void move_replay(void);
long parse_keymap(void);

boolean setup_replay_from_random_resource(unsigned long map_checksum);

#ifdef DEBUG_REPLAY
struct recorded_flag {
	long flag;
	short player_index;
};

void open_stream_file(void);
void write_flags(struct recorded_flag *buffer, long count);
static void debug_stream_of_flags(long action_flag, short player_index);
static void close_stream_file(void);
#endif

#endif

