#ifndef __VBL_DEFINITIONS_H
#define __VBL_DEFINITIONS_H

/*

	vbl_definitions.h
	Friday, September 29, 1995 4:20:17 PM- rdm created.

	This is only used by vbl.c and vbl_macintosh.c

Aug 12, 2000 (Loren Petrich):
	Using object-oriented file handler; removing refnum from here
*/

#include "FileHandler.h"

#define MAXIMUM_QUEUE_SIZE           512

typedef struct action_queue /* 8 bytes */
{
	short read_index, write_index;
	long *buffer;
} ActionQueue;

struct recording_header
{
	long length;
	short num_players;
	short level_number;
	unsigned long map_checksum;
	short version;
	struct player_start_data starts[MAXIMUM_NUMBER_OF_PLAYERS];
	struct game_data game_information;
};

struct replay_private_data {
	boolean valid;
	struct recording_header header;
	short replay_speed;
	boolean game_is_being_replayed;
	boolean game_is_being_recorded;
	boolean have_read_last_chunk;
	ActionQueue *recording_queues;
	
	// fileref recording_file_refnum;
	char *fsread_buffer;
	char *location_in_cache;
	long bytes_in_cache;
	
	long film_resource_offset;
	char *resource_data;
	long resource_data_size;
};

/* ----- globals */
extern struct replay_private_data replay;

/* ------ prototypes */
#ifndef DEBUG
	#define get_player_recording_queue(x) (replay.recording_queues+(x))
#else
ActionQueue *get_player_recording_queue(short player_index);
#endif

typedef void *timer_task_proc;

timer_task_proc install_timer_task(short tasks_per_second, boolean (*func)(void));
void remove_timer_task(timer_task_proc proc);

void set_keys_to_match_preferences(void);

#endif

