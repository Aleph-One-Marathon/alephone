#ifndef __VBL_DEFINITIONS_H
#define __VBL_DEFINITIONS_H

/*

	vbl_definitions.h
	Friday, September 29, 1995 4:20:17 PM- rdm created.

	This is only used by vbl.c and vbl_macintosh.c

Aug 12, 2000 (Loren Petrich):
	Using object-oriented file handler; removing refnum from here
*/

#define MAXIMUM_QUEUE_SIZE           512

typedef struct action_queue /* 8 bytes */
{
	int16 read_index, write_index;
	int32 *buffer;
} ActionQueue;

struct recording_header
{
	int32 length;
	int16 num_players;
	int16 level_number;
	uint32 map_checksum;
	int16 version;
	struct player_start_data starts[MAXIMUM_NUMBER_OF_PLAYERS];
	struct game_data game_information;
};
const int SIZEOF_recording_header = 352;

struct replay_private_data {
	bool valid;
	struct recording_header header;
	int16 replay_speed;
	bool game_is_being_replayed;
	bool game_is_being_recorded;
	bool have_read_last_chunk;
	ActionQueue *recording_queues;
	
	// fileref recording_file_refnum;
	char *fsread_buffer;
	char *location_in_cache;
	int32 bytes_in_cache;
	
	int32 film_resource_offset;
	char *resource_data;
	int32 resource_data_size;
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

timer_task_proc install_timer_task(short tasks_per_second, bool (*func)(void));
void remove_timer_task(timer_task_proc proc);

void set_keys_to_match_preferences(void);

#endif

