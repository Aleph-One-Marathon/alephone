#ifndef __VBL_H
#define __VBL_H

/*

	vbl.h
	Friday, September 29, 1995 3:24:01 PM- rdm created.

Aug 12, 2000 (Loren Petrich):
	Using object-oriented file handler; revising definitions accordingly

*/

// LP: CodeWarrior complains unless I give the full definition of these classes
#include "FileHandler.h"
// class FileSpecifier;

/* ------------ prototypes/VBL.C */
bool setup_for_replay_from_file(FileSpecifier& File, unsigned long map_checksum);
bool setup_replay_from_random_resource(unsigned long map_checksum);

void start_recording(void);

bool find_replay_to_use(bool ask_user, FileSpecifier& File);

void set_recording_header_data(short number_of_players, short level_number, unsigned long map_checksum,
	short version, struct player_start_data *starts, struct game_data *game_information);
void get_recording_header_data(short *number_of_players, short *level_number, unsigned long *map_checksum,
	short *version, struct player_start_data *starts, struct game_data *game_information);

bool input_controller(void);

/* ------------ prototypes/VBL_MACINTOSH.C */
void initialize_keyboard_controller(void);

/* true if it found it, false otherwise. always fills in vrefnum and dirid*/
bool get_recording_filedesc(FileSpecifier& File);
void move_replay(void);
long parse_keymap(void);

bool setup_replay_from_random_resource(unsigned long map_checksum);

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

