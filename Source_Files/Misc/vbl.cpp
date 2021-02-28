/*
VBL.C

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

Friday, August 21, 1992 7:06:54 PM

Tuesday, November 17, 1992 3:53:29 PM
	the new task of the vbl controller is only to move the player.  this is necessary for
	good control of the game.  everything else (doors, monsters, projectiles, etc) will
	be moved immediately before the next frame is drawn, based on delta-time values.
	collisions (including the player with walls) will also be handled at this time.
Thursday, November 19, 1992 1:27:23 AM
	the enumeration 'turning_head' had to be changed to '_turn_not_rotate' to make this
	file compile.  go figure.
Wednesday, December 2, 1992 2:31:05 PM
	the world doesnÕt change while the mouse button is pressed.
Friday, January 15, 1993 11:19:11 AM
	the world doesnÕt change after 14 ticks have passed without a screen refresh.
Friday, January 22, 1993 3:06:32 PM
	world_ticks was never being initialized to zero.  hmmm.
Saturday, March 6, 1993 12:23:48 PM
	at exit, we remove our vbl task.
Sunday, May 16, 1993 4:07:47 PM
	finally recoding everything
Monday, August 16, 1993 10:22:17 AM
	#ifdef CHARLES added.
Saturday, August 21, 1993 12:35:29 PM
	from pathways VBL_CONTROLLER.C.
Sunday, May 22, 1994 8:51:15 PM
	all the world physics has been moved into PHYSICS.C; all we do now is maintain and
	distribute a circular queue of keyboard flags (we're the keyboard_controller, not the
	movement_controller).
Thursday, June 2, 1994 12:55:52 PM
	gee, now we donÕt even maintain the queue we just send our actions to PLAYER.C.
Tuesday, July 5, 1994 9:27:49 PM
	nuked most of the shit in here. changed the vbl task to a time
	manager task. the only functions from the old vbl.c that remain are precalculate_key_information()
	and parse_keymap().
Thursday, July 7, 1994 11:59:32 AM
	Added recording/replaying
Wednesday, August 10, 1994 2:44:57 PM
	added caching system for FSRead.
Friday, January 13, 1995 11:38:51 AM  (Jason')
	fixed the 'a' key getting blacklisted.

Jan 30, 2000 (Loren Petrich)
	Did some typecasts

Jul 7, 2000 (Loren Petrich)
	Added Ben Thompson's ISp-support changes

Aug 12, 2000 (Loren Petrich):
	Using object-oriented file handler

Aug 26, 2000 (Loren Petrich):
	Created alternative to SetLength(): delete a file, then re-create it.
	This should be more stdio-friendly.

Feb 20, 2002 (Woody Zenfell):
    Uses GetRealActionQueues()->enqueueActionFlags() rather than queue_action_flags().
*/

#include "cseries.h"
#include <string.h>
#include <stdlib.h>

#include "map.h"
#include "interface.h"
#include "shell.h"
#include "preferences.h"
#include "Logging.h"
#include "mouse.h"
#include "player.h"
#include "key_definitions.h"
#include "tags.h"
#include "vbl.h"
#include "FileHandler.h"
#include "Packing.h"
#include "ActionQueues.h"
#include "computer_interface.h"
#include "Console.h"
#include "joystick.h"
#include "Movie.h"
#include "InfoTree.h"

/* ---------- constants */

#define RECORD_CHUNK_SIZE            (MAXIMUM_QUEUE_SIZE/2)
#define END_OF_RECORDING_INDICATOR  (RECORD_CHUNK_SIZE+1)
#define MAXIMUM_TIME_DIFFERENCE     15 // allowed between heartbeat_count and dynamic_world->tick_count
#define MAXIMUM_NET_QUEUE_SIZE       8
#define DISK_CACHE_SIZE             ((sizeof(int16)+sizeof(uint32))*100)
#define MAXIMUM_REPLAY_SPEED         5
#define MINIMUM_REPLAY_SPEED        -5

/* ---------- macros */

#define INCREMENT_QUEUE_COUNTER(c) { (c)++; if ((c)>=MAXIMUM_QUEUE_SIZE) (c) = 0; }

// LP: fake portable-files stuff
inline short memory_error() {return 0;}

/* ---------- structures */
#include "vbl_definitions.h"

/* ---------- globals */

static int32 heartbeat_count;
static bool input_task_active;
static timer_task_proc input_task;

// LP: defined this here so it will work properly
static FileSpecifier FilmFileSpec;
static OpenedFile FilmFile;

struct replay_private_data replay;

#ifdef DEBUG
ActionQueue *get_player_recording_queue(
	short player_index)
{
	assert(replay.recording_queues);
	assert(player_index>=0 && player_index<MAXIMUM_NUMBER_OF_PLAYERS);
	
	return (replay.recording_queues+player_index);
}
#endif

/* ---------- private prototypes */
static void remove_input_controller(void);
static void save_recording_queue_chunk(short player_index);
static void read_recording_queue_chunks(void);
static bool pull_flags_from_recording(short count);
// LP modifications for object-oriented file handling; returns a test for end-of-file
static bool vblFSRead(OpenedFile& File, int32 *count, void *dest, bool& HitEOF);
static void record_action_flags(short player_identifier, const uint32 *action_flags, short count);
static short get_recording_queue_size(short which_queue);

static uint8 *unpack_recording_header(uint8 *Stream, recording_header *Objects, size_t Count);
static uint8 *pack_recording_header(uint8 *Stream, recording_header *Objects, size_t Count);

// #define DEBUG_REPLAY

#ifdef DEBUG_REPLAY
static void open_stream_file(void);
static void debug_stream_of_flags(uint32 action_flag, short player_index);
static void close_stream_file(void);
#endif

/* ---------- code */
void initialize_keyboard_controller(
	void)
{
	ActionQueue *queue;
	short player_index;
	
//	vassert(NUMBER_OF_KEYS == NUMBER_OF_STANDARD_KEY_DEFINITIONS,
//		csprintf(temporary, "NUMBER_OF_KEYS == %d, NUMBER_OF_KEY_DEFS = %d. Not Equal!", NUMBER_OF_KEYS, NUMBER_OF_STANDARD_KEY_DEFINITIONS));
	assert(NUMBER_OF_STANDARD_KEY_DEFINITIONS==NUMBER_OF_LEFT_HANDED_KEY_DEFINITIONS);
	assert(NUMBER_OF_LEFT_HANDED_KEY_DEFINITIONS==NUMBER_OF_POWERBOOK_KEY_DEFINITIONS);
	
	// get globals initialized
	heartbeat_count= 0;
	input_task_active= false;
	obj_clear(replay);

	input_task= install_timer_task(TICKS_PER_SECOND, input_controller);
	assert(input_task);
	
	atexit(remove_input_controller);
	
	/* Allocate the recording queues */	
	replay.recording_queues = new ActionQueue[MAXIMUM_NUMBER_OF_PLAYERS];
	assert(replay.recording_queues);
	if(!replay.recording_queues) alert_out_of_memory();
	
	/* Allocate the individual ones */
	for (player_index= 0; player_index<MAXIMUM_NUMBER_OF_PLAYERS; player_index++)
	{
		queue= get_player_recording_queue(player_index);
		queue->read_index= queue->write_index = 0;
		queue->buffer= new uint32[MAXIMUM_QUEUE_SIZE];
		if(!queue->buffer) alert_out_of_memory();
	}
	enter_mouse(0);
}

void set_keyboard_controller_status(
	bool active)
{
	input_task_active= active;

	// flush events when changing game state
	SDL_PumpEvents();
	SDL_FlushEvents(SDL_KEYDOWN, SDL_KEYUP);
	SDL_FlushEvents(SDL_MOUSEMOTION, SDL_MOUSEWHEEL);
	SDL_FlushEvents(SDL_CONTROLLERAXISMOTION, SDL_CONTROLLERBUTTONUP);

	// We enable/disable mouse control here
	if (active) {
		enter_mouse(input_preferences->input_device);
                enter_joystick();
        } else {
		exit_mouse(input_preferences->input_device);
                exit_joystick();
        }
	
	/******************************************************************************************/
}

bool get_keyboard_controller_status(
	void)
{
	return input_task_active;
}

int32 get_heartbeat_count(
	void)
{
	return heartbeat_count;
}

void sync_heartbeat_count(
	void)
{
	heartbeat_count= dynamic_world->tick_count;
}

void increment_replay_speed(
	void)
{
	if (replay.replay_speed < MAXIMUM_REPLAY_SPEED) replay.replay_speed++;
}

void decrement_replay_speed(
	void)
{
	if (replay.replay_speed > MINIMUM_REPLAY_SPEED) replay.replay_speed--;
}

int get_replay_speed()
{
	return replay.replay_speed;
}

bool game_is_being_replayed()
{
	return replay.game_is_being_replayed;
}

void increment_heartbeat_count(int value)
{
	heartbeat_count+=value;
}

bool has_recording_file(void)
{
	FileSpecifier File;
	return get_recording_filedesc(File);
}

/* Called by the time manager task in vbl_macintosh.c */
bool input_controller(
	void)
{
	if (input_task_active || Movie::instance()->IsRecording())
	{
		if((heartbeat_count-dynamic_world->tick_count) < MAXIMUM_TIME_DIFFERENCE)
		{
			if (game_is_networked) // input from network
			{
				; // all handled elsewhere now. (in network.c)
			}
			else if (replay.game_is_being_replayed) // input from recorded game file
			{
				static short phase= 0; /* When this gets to 0, update the world */

				/* Minimum replay speed is a pause. */
				if(replay.replay_speed != MINIMUM_REPLAY_SPEED)
				{
					if (replay.replay_speed > 0 || (--phase<=0))
					{
						short flag_count= MAX(replay.replay_speed, 1);
					
						if (!pull_flags_from_recording(flag_count)) // oops. silly me.
						{
							if (replay.have_read_last_chunk)
							{
								assert(get_game_state()==_game_in_progress || get_game_state()==_switch_demo);
								set_game_state(_switch_demo);
							}
						}
						else
						{	
							/* Increment the heartbeat.. */
							heartbeat_count+= flag_count;
						}
	
						/* Reset the phase-> doesn't matter if the replay speed is positive */					
						/* +1 so that replay_speed 0 is different from replay_speed 1 */
						phase= -(replay.replay_speed) + 1;
					}
				}
			}
			else // then getting input from the keyboard/mouse
			{
				uint32 action_flags= parse_keymap();
				
				process_action_flags(local_player_index, &action_flags, 1);
				heartbeat_count++; // ba-doom
			}
		} else {
// dprintf("Out of phase.. (%d);g", heartbeat_count - dynamic_world->tick_count);
		}
	}
	
	return true; // tells the time manager library to reschedule this task
}

void process_action_flags(
	short player_identifier, 
	const uint32 *action_flags, 
	short count)
{
	if (replay.game_is_being_recorded)
	{
		record_action_flags(player_identifier, action_flags, count);
	}
	
	GetRealActionQueues()->enqueueActionFlags(player_identifier, action_flags, count);
}

static void record_action_flags(
	short player_identifier, 
	const uint32 *action_flags, 
	short count)
{
	short index;
	ActionQueue  *queue;
	
	queue= get_player_recording_queue(player_identifier);
	assert(queue && queue->write_index >= 0 && queue->write_index < MAXIMUM_QUEUE_SIZE);
	for (index= 0; index<count; index++)
	{
		*(queue->buffer + queue->write_index) = *action_flags++;
		INCREMENT_QUEUE_COUNTER(queue->write_index);
		if (queue->write_index == queue->read_index)
		{
			dprintf("blew recording queue for player %d", player_identifier);
		}
	}
}

/*********************************************************************************************
 *
 * Function: save_recording_queue_chunk
 * Purpose:  saves one chunk of the queue to the recording file, using run-length encoding.
 *
 *********************************************************************************************/
void save_recording_queue_chunk(
	short player_index)
{
	uint8 *location;
	uint32 last_flag, count, flag = 0;
	int16 i, run_count, num_flags_saved, max_flags;
	static uint8 *buffer= NULL;
	ActionQueue *queue;
	
	// The data format is (run length (int16)) + (action flag (uint32))
	int DataSize = sizeof(int16) + sizeof(uint32);
	
	if (buffer == NULL)
		buffer = new byte[RECORD_CHUNK_SIZE * DataSize];
	
	location= buffer;
	count= 0; // keeps track of how many bytes we'll save.
	last_flag= (uint32)NONE;

	queue= get_player_recording_queue(player_index);
	
	// don't want to save too much stuff
	max_flags= MIN(RECORD_CHUNK_SIZE, get_recording_queue_size(player_index)); 

	// save what's in the queue
	run_count= num_flags_saved= 0;
	for (i = 0; i<max_flags; i++)
	{
		flag = queue->buffer[queue->read_index];
		INCREMENT_QUEUE_COUNTER(queue->read_index);
		
		if (i && flag != last_flag)
		{
			ValueToStream(location,run_count);
			ValueToStream(location,last_flag);
			count += DataSize;
			num_flags_saved += run_count;
			run_count = 1;
		}
		else
		{
			run_count++;
		}
		last_flag = flag;
	}
	
	// now save the final run
	ValueToStream(location,run_count);
	ValueToStream(location,last_flag);
	count += DataSize;
	num_flags_saved += run_count;
	
	if (max_flags<RECORD_CHUNK_SIZE)
	{
		short end_indicator = END_OF_RECORDING_INDICATOR;
		ValueToStream(location,end_indicator);
		int32 end_flag = 0;
		ValueToStream(location,end_flag);
		count += DataSize;
		num_flags_saved += RECORD_CHUNK_SIZE-max_flags;
	}
	
	FilmFile.Write(count,buffer);
	replay.header.length+= count;
		
	vwarn(num_flags_saved == RECORD_CHUNK_SIZE,
		csprintf(temporary, "bad recording: %d flags, max=%d, count = %u;dm #%p #%u", num_flags_saved, max_flags,
			count, buffer, count));
}

/*********************************************************************************************
 *
 * Function: pull_flags_from_recording
 * Purpose:  remove one flag from each queue from the recording buffer.
 * Returns:  true if it pulled the flags, false if it didn't
 *
 *********************************************************************************************/
static bool pull_flags_from_recording(
	short count)
{
	short player_index;
	bool success= true;
	
	// first check that we can pull something from each playerÕs queue
	// (at the beginning of the game, we wonÕt be able to)
	// i'm not sure that i really need to do this check. oh well.
	for (player_index = 0; success && player_index<dynamic_world->player_count; player_index++)
	{
		if(get_recording_queue_size(player_index)==0) success= false;
	}

	if(success)
	{
		for (player_index = 0; player_index < dynamic_world->player_count; player_index++)
		{
			short index;
			ActionQueue  *queue;
		
			queue= get_player_recording_queue(player_index);
			for (index= 0; index<count; index++)
			{
				if (queue->read_index != queue->write_index)
				{
#ifdef DEBUG_REPLAY
					debug_stream_of_flags(*(queue->buffer+queue->read_index), player_index);
#endif
                    GetRealActionQueues()->enqueueActionFlags(player_index, queue->buffer + queue->read_index, 1);
					INCREMENT_QUEUE_COUNTER(queue->read_index);
				} else {
					dprintf("Dropping flag?");
				}
			}
		}
	}
	
	return success;
}

static short get_recording_queue_size(
	short which_queue)
{
	short size;
	ActionQueue *queue= get_player_recording_queue(which_queue);

	/* Note that this is a circular queue */
	size= queue->write_index-queue->read_index;
	if(size<0) size+= MAXIMUM_QUEUE_SIZE;
	
	return size;
}

void set_recording_header_data(
	short number_of_players, 
	short level_number, 
	uint32 map_checksum,
	short version, 
	struct player_start_data *starts, 
	struct game_data *game_information)
{
	assert(!replay.valid);
	obj_clear(replay.header);
	replay.header.num_players= number_of_players;
	replay.header.level_number= level_number;
	replay.header.map_checksum= map_checksum;
	replay.header.version= version;
	objlist_copy(replay.header.starts, starts, MAXIMUM_NUMBER_OF_PLAYERS);
	obj_copy(replay.header.game_information, *game_information);
	// Use the packed size here!!!
	replay.header.length= SIZEOF_recording_header;
}

void get_recording_header_data(
	short *number_of_players, 
	short *level_number, 
	uint32 *map_checksum,
	short *version, 
	struct player_start_data *starts, 
	struct game_data *game_information)
{
	assert(replay.valid);
	*number_of_players= replay.header.num_players;
	*level_number= replay.header.level_number;
	*map_checksum= replay.header.map_checksum;
	*version= replay.header.version;
	objlist_copy(starts, replay.header.starts, MAXIMUM_NUMBER_OF_PLAYERS);
	obj_copy(*game_information, replay.header.game_information);
}

extern int movie_export_phase;

bool setup_for_replay_from_file(
	FileSpecifier& File,
	uint32 map_checksum,
	bool prompt_to_export)
{
	bool successful= false;

	(void)(map_checksum);
	
	FilmFileSpec = File;
	if (FilmFileSpec.Open(FilmFile))
	{
		replay.valid= true;
		replay.have_read_last_chunk = false;
		replay.game_is_being_replayed = true;
		assert(!replay.resource_data);
		replay.resource_data= NULL;
		replay.resource_data_size= 0l;
		replay.film_resource_offset= NONE;
		movie_export_phase = 0;
		
		byte Header[SIZEOF_recording_header];
		FilmFile.Read(SIZEOF_recording_header,Header);
		unpack_recording_header(Header,&replay.header,1);
		replay.header.game_information.cheat_flags = _allow_crosshair | _allow_tunnel_vision | _allow_behindview | _allow_overlay_map;
	
		/* Set to the mapfile this replay came from.. */
		if(use_map_file(replay.header.map_checksum))
		{
			replay.fsread_buffer= new char[DISK_CACHE_SIZE]; 
			assert(replay.fsread_buffer);
			
			replay.location_in_cache= NULL;
			replay.bytes_in_cache= 0;
			replay.replay_speed= 1;
			
#ifdef DEBUG_REPLAY
			open_stream_file();
#endif
			if (prompt_to_export)
				Movie::instance()->PromptForRecording();
			successful= true;
		} else {
			/* Tell them that this map wasn't found.  They lose. */
			alert_user(infoError, strERRORS, cantFindReplayMap, 0);
			replay.valid= false;
			replay.game_is_being_replayed= false;
			FilmFile.Close();
		}
	}
	
	return successful;
}

/* Note that we _must_ set the header information before we start recording!! */
void start_recording(
	void)
{
	assert(!replay.valid);
	replay.valid= true;
	
	if(get_recording_filedesc(FilmFileSpec))
		FilmFileSpec.Delete();

	if (FilmFileSpec.Create(_typecode_film))
	{
		/* I debate the validity of fsCurPerm here, but Alain had it, and it has been working */
		if (FilmFileSpec.Open(FilmFile,true))
		{
			replay.game_is_being_recorded= true;
	
			// save a header containing information about the game.
			byte Header[SIZEOF_recording_header];
			pack_recording_header(Header,&replay.header,1);
			FilmFile.Write(SIZEOF_recording_header,Header);
		}
	}
}

void stop_recording(
	void)
{
	if (replay.game_is_being_recorded)
	{
		replay.game_is_being_recorded = false;
		
		short player_index;
		int32 total_length;

		assert(replay.valid);
		for (player_index= 0; player_index<dynamic_world->player_count; player_index++)
		{
			save_recording_queue_chunk(player_index);
		}

		/* Rewrite the header, since it has the new length */
		FilmFile.SetPosition(0);
		byte Header[SIZEOF_recording_header];
		pack_recording_header(Header,&replay.header,1);

		// ZZZ: removing code that does stuff from assert() argument.  BUT...
		// should we really be asserting on this anyway?  I mean, the write could fail
		// in 'normal operation' too, not just when we screwed something up in writing the program?
		bool successfulWrite = FilmFile.Write(SIZEOF_recording_header,Header);
		assert(successfulWrite);
		
		FilmFile.GetLength(total_length);
		assert(total_length==replay.header.length);
		
		FilmFile.Close();
	}

	replay.valid= false;
}

void rewind_recording(
	void)
{
	if(replay.game_is_being_recorded)
	{
		/* This is unnecessary, because it is called from reset_player_queues, */
		/* which is always called from revert_game */
		/*
		FilmFile.SetLength(sizeof(recording_header));
		FilmFile.SetPosition(sizeof(recording_header));
		*/
		// Alternative that does not use "SetLength", but instead creates and re-creates the file.
		FilmFile.SetPosition(0);
		byte Header[SIZEOF_recording_header];
		FilmFile.Read(SIZEOF_recording_header,Header);
		FilmFile.Close();
		FilmFileSpec.Delete();
		FilmFileSpec.Create(_typecode_film);
		FilmFileSpec.Open(FilmFile,true);
		FilmFile.Write(SIZEOF_recording_header,Header);
		
		// Use the packed length here!!!
		replay.header.length= SIZEOF_recording_header;
	}
}

void check_recording_replaying(
	void)
{
	short player_index, queue_size;

	if (replay.game_is_being_recorded)
	{
		bool enough_data_to_save= true;
	
		// it's time to save the queues if all of them have >= RECORD_CHUNK_SIZE flags in them.
		for (player_index= 0; enough_data_to_save && player_index<dynamic_world->player_count; player_index++)
		{
			queue_size= get_recording_queue_size(player_index);
			if (queue_size < RECORD_CHUNK_SIZE)	enough_data_to_save= false;
		}
		
		if(enough_data_to_save)
		{
			bool success;
			uint32 freespace = 0;
			FileSpecifier FilmFile_Check;
			
			get_recording_filedesc(FilmFile_Check);

			success= FilmFile_Check.GetFreeSpace(freespace);
			if (success && freespace>(RECORD_CHUNK_SIZE*sizeof(int16)*sizeof(uint32)*dynamic_world->player_count))
			{
				for (player_index= 0; player_index<dynamic_world->player_count; player_index++)
				{
					save_recording_queue_chunk(player_index);
				}
			}
		}
	}
	else if (replay.game_is_being_replayed)
	{
		bool load_new_data= true;
	
		// it's time to refill the requeues if they all have < RECORD_CHUNK_SIZE flags in them.
		for (player_index= 0; load_new_data && player_index<dynamic_world->player_count; player_index++)
		{
			queue_size= get_recording_queue_size(player_index);
			if(queue_size>= RECORD_CHUNK_SIZE) load_new_data= false;
		}
		
		if(load_new_data)
		{
			// at this point, we've determined that the queues are sufficently empty, so
			// we'll fill 'em up.
			read_recording_queue_chunks();
		}
	}
}

void reset_recording_and_playback_queues(
	void)
{
	short index;
	
	for(index= 0; index<MAXIMUM_NUMBER_OF_PLAYERS; ++index)
	{
		replay.recording_queues[index].read_index= replay.recording_queues[index].write_index= 0;
	}
}

void stop_replay(
	void)
{
	if (replay.game_is_being_replayed)
	{
		assert(replay.valid);

		replay.game_is_being_replayed= false;
		if (replay.resource_data)
		{
			delete []replay.resource_data;
			replay.resource_data= NULL;
		}
		else
		{
			FilmFile.Close();
			assert(replay.fsread_buffer);
			delete []replay.fsread_buffer;
		}
#ifdef DEBUG_REPLAY
		close_stream_file();
#endif
	}

	/* Unecessary, because reset_player_queues calls this. */
	replay.valid= false;
}

static void read_recording_queue_chunks(
	void)
{
	logContext("reading recording queue chunks");

	int32 i, sizeof_read;
	uint32 action_flags; 
	int16 count, player_index, num_flags;
	ActionQueue *queue;
	
	for (player_index = 0; player_index < dynamic_world->player_count; player_index++)
	{
		queue= get_player_recording_queue(player_index);
		for (count = 0; count < RECORD_CHUNK_SIZE; )
		{
			if (replay.resource_data)
			{
				bool hit_end= false;
				
				if (replay.film_resource_offset >= replay.resource_data_size)
				{
					hit_end = true;
				}
				else
				{
					uint8* S;
					S = (uint8 *)(replay.resource_data + replay.film_resource_offset);
					StreamToValue(S,num_flags);
					replay.film_resource_offset += sizeof(num_flags);
					S = (uint8 *)(replay.resource_data + replay.film_resource_offset);
					StreamToValue(S,action_flags);
					replay.film_resource_offset+= sizeof(action_flags);
				}
				
				if (hit_end || num_flags == END_OF_RECORDING_INDICATOR)
				{
					replay.have_read_last_chunk= true;
					break;
				}
			}
			else
			{
				sizeof_read = sizeof(num_flags);
				uint8 NumFlagsBuffer[sizeof(num_flags)];
				bool HitEOF = false;
				if (vblFSRead(FilmFile, &sizeof_read, NumFlagsBuffer, HitEOF))
				{
					uint8 *S = NumFlagsBuffer;
					StreamToValue(S,num_flags);
					sizeof_read = sizeof(action_flags);
					uint8 ActionFlagsBuffer[sizeof(action_flags)];
					bool status = vblFSRead(FilmFile, &sizeof_read, ActionFlagsBuffer, HitEOF);
					S = ActionFlagsBuffer;
					StreamToValue(S,action_flags);
					assert(status || (HitEOF && sizeof_read == sizeof(action_flags)));
				}
				else
				{
					logError("film file read error");
					replay.have_read_last_chunk = true;
					break;
				}
				
				if ((HitEOF && sizeof_read != sizeof(action_flags)) || num_flags == END_OF_RECORDING_INDICATOR)
				{
					replay.have_read_last_chunk = true;
					break;
				}
			}

			if (!(replay.have_read_last_chunk || num_flags))
			{
				logAnomaly("chunk contains no flags");
			}

			count += num_flags;

			for (i = 0; i < num_flags; i++)
			{
				*(queue->buffer + queue->write_index) = action_flags;
				INCREMENT_QUEUE_COUNTER(queue->write_index);
				assert(queue->read_index != queue->write_index);
			}
		}
		assert(replay.have_read_last_chunk || count == RECORD_CHUNK_SIZE);
	}
}

/* This is gross, (Alain wrote it, not me!) but I don't have time to clean it up */
static bool vblFSRead(
	OpenedFile& File,
	int32 *count, 
	void *dest,
	bool& HitEOF)
{
	int32 fsread_count;
	bool status = true;
	
	assert(replay.fsread_buffer);
	
	// LP: way for testing whether hitting end-of-file;
	// doing that by testing for whether a read was complete.
	HitEOF = false;

	if (replay.bytes_in_cache < *count)
	{
		assert(replay.bytes_in_cache + *count < int(DISK_CACHE_SIZE));
		if (replay.bytes_in_cache)
		{
			memcpy(replay.fsread_buffer, replay.location_in_cache, replay.bytes_in_cache);
		}
		replay.location_in_cache = replay.fsread_buffer;
		fsread_count= DISK_CACHE_SIZE - replay.bytes_in_cache;
		int32 PrevPos;
		File.GetPosition(PrevPos);
		int32 replay_left= replay.header.length - PrevPos;
		if(replay_left < fsread_count)
			fsread_count= replay_left;
		if(fsread_count > 0)
		{
			assert(fsread_count > 0);
			// LP: wrapped the routines with some for finding out the file positions;
			// this finds out how much is read indirectly
			status = File.Read(fsread_count,replay.fsread_buffer+replay.bytes_in_cache);
			int32 CurrPos;
			File.GetPosition(CurrPos);
			int32 new_fsread_count = CurrPos - PrevPos;
			int32 FileLen;
			File.GetLength(FileLen);
			HitEOF = (new_fsread_count < fsread_count) && (CurrPos == FileLen);
			fsread_count = new_fsread_count;
			if(status) replay.bytes_in_cache += fsread_count;
		}
	}

	// If we're still low, then we've consumed the disk cache
	if(replay.bytes_in_cache < *count)
	{
		HitEOF = true;
	}

	// Ignore EOF if we still have cache
	if (HitEOF && replay.bytes_in_cache < *count)
	{
		*count= replay.bytes_in_cache;
	}
	else
	{
		status = true;
		HitEOF = false;
	}
	
	memcpy(dest, replay.location_in_cache, *count);
	replay.bytes_in_cache -= *count;
	replay.location_in_cache += *count;
	
	return status;
}

static void remove_input_controller(
	void)
{
	remove_timer_task(input_task);
	if (replay.game_is_being_recorded)
	{
		stop_recording();
	}
	else if (replay.game_is_being_replayed)
	{
		if (replay.resource_data)
		{
			delete []replay.resource_data;
			replay.resource_data= NULL;
			replay.resource_data_size= 0l;
			replay.film_resource_offset= NONE;
		}
		else
		{
			FilmFile.Close();
		}
	}

	replay.valid= false;
}


void reset_mml_keyboard()
{
	// no reset
}

void parse_mml_keyboard(const InfoTree& root)
{
	int16 which_set;
	if (!root.read_indexed("set", which_set, NUMBER_OF_KEY_SETUPS))
		return;
	
	BOOST_FOREACH(InfoTree ktree, root.children_named("key"))
	{
		int16 index;
		if (!ktree.read_indexed("index", index, NUMBER_OF_STANDARD_KEY_DEFINITIONS))
			continue;
		
		int16 keycode;
		if (!ktree.read_attr("sdl", keycode))
			continue;
		
		all_key_definitions[which_set][index].offset = static_cast<SDL_Scancode>(keycode);
	}
}


static void StreamToPlayerStart(uint8* &S, player_start_data& Object)
{
	StreamToValue(S,Object.team);
	StreamToValue(S,Object.identifier);
	StreamToValue(S,Object.color);
	StreamToBytes(S,Object.name,MAXIMUM_PLAYER_START_NAME_LENGTH+2);
}

static void PlayerStartToStream(uint8* &S, player_start_data& Object)
{
	ValueToStream(S,Object.team);
	ValueToStream(S,Object.identifier);
	ValueToStream(S,Object.color);
	BytesToStream(S,Object.name,MAXIMUM_PLAYER_START_NAME_LENGTH+2);
}


static void StreamToGameData(uint8* &S, game_data& Object)
{
	StreamToValue(S,Object.game_time_remaining);
	StreamToValue(S,Object.game_type);
	StreamToValue(S,Object.game_options);
	StreamToValue(S,Object.kill_limit);
	StreamToValue(S,Object.initial_random_seed);
	StreamToValue(S,Object.difficulty_level);
	StreamToList(S,Object.parameters,2);
}

static void GameDataToStream(uint8* &S, game_data& Object)
{
	ValueToStream(S,Object.game_time_remaining);
	ValueToStream(S,Object.game_type);
	ValueToStream(S,Object.game_options);
	ValueToStream(S,Object.kill_limit);
	ValueToStream(S,Object.initial_random_seed);
	ValueToStream(S,Object.difficulty_level);
	ListToStream(S,Object.parameters,2);
}


uint8 *unpack_recording_header(uint8 *Stream, recording_header *Objects, size_t Count)
{
	uint8* S = Stream;
	recording_header* ObjPtr = Objects;
	
	for (size_t k = 0; k < Count; k++, ObjPtr++)
	{
		StreamToValue(S,ObjPtr->length);
		StreamToValue(S,ObjPtr->num_players);
		StreamToValue(S,ObjPtr->level_number);
		StreamToValue(S,ObjPtr->map_checksum);
		StreamToValue(S,ObjPtr->version);
		for (int m = 0; m < MAXIMUM_NUMBER_OF_PLAYERS; m++)
			StreamToPlayerStart(S,ObjPtr->starts[m]);
		StreamToGameData(S,ObjPtr->game_information);
	}
	
	assert(static_cast<size_t>(S - Stream) == (Count*SIZEOF_recording_header));
	return S;
}

uint8 *pack_recording_header(uint8 *Stream, recording_header *Objects, size_t Count)
{
	uint8* S = Stream;
	recording_header* ObjPtr = Objects;
	
	for (size_t k = 0; k < Count; k++, ObjPtr++)
	{
		ValueToStream(S,ObjPtr->length);
		ValueToStream(S,ObjPtr->num_players);
		ValueToStream(S,ObjPtr->level_number);
		ValueToStream(S,ObjPtr->map_checksum);
		ValueToStream(S,ObjPtr->version);
		for (size_t m = 0; m < MAXIMUM_NUMBER_OF_PLAYERS; m++)
			PlayerStartToStream(S,ObjPtr->starts[m]);
		GameDataToStream(S,ObjPtr->game_information);
	}
	
	assert(static_cast<size_t>(S - Stream) == (Count*SIZEOF_recording_header));
	return S;
}

// Constants
#define MAXIMUM_FLAG_PERSISTENCE    15
#define DOUBLE_CLICK_PERSISTENCE    10
#define FILM_RESOURCE_TYPE          FOUR_CHARS_TO_INT('f', 'i', 'l', 'm')

#define NUMBER_OF_SPECIAL_FLAGS (sizeof(special_flags)/sizeof(struct special_flag_data))
static struct special_flag_data special_flags[]=
{
	{_double_flag, _look_dont_turn, _looking_center},
	{_double_flag, _run_dont_walk, _action_trigger_state},
	{_latched_flag, _action_trigger_state},
	{_latched_flag, _cycle_weapons_forward},
	{_latched_flag, _cycle_weapons_backward},
	{_latched_flag, _toggle_map}
};


/*
 *  Get FileDesc for replay, ask user if desired
 */

bool find_replay_to_use(bool ask_user, FileSpecifier &file)
{
	if (ask_user) {
		return file.ReadDialog(_typecode_film);
	} else
		return get_recording_filedesc(file);
}


/*
 *  Get FileDesc for default recording file
 */

bool get_recording_filedesc(FileSpecifier &File)
{
	File.SetToLocalDataDir();
	File += getcstr(temporary, strFILENAMES, filenameMARATHON_RECORDING);
	return File.Exists();
}


/*
 *  Save film buffer to user-selected file
 */

void move_replay(void)
{
	// Get source file specification
	FileSpecifier src_file, dst_file;
	if (!get_recording_filedesc(src_file))
		return;

	// Ask user for destination file
	char prompt[256], default_name[256];
	if (!dst_file.WriteDialog(_typecode_film, getcstr(prompt, strPROMPTS, _save_replay_prompt), getcstr(default_name, strFILENAMES, filenameMARATHON_RECORDING)))
		return;

	// Copy file
	dst_file.CopyContents(src_file);
	int error = dst_file.GetError();
	if (error)
		alert_user(infoError, strERRORS, fileError, error);
}

static uint32_t hotkey_sequence[3] {0};
static constexpr uint32_t hotkey_used = 0x80000000;

void encode_hotkey_sequence(int hotkey)
{
	hotkey_sequence[0] =
		(3 << _cycle_weapons_forward_bit) |
		hotkey_used;
	
	hotkey_sequence[1] =
		((hotkey / 4 + 1) << _cycle_weapons_forward_bit) |
		hotkey_used;
	
	hotkey_sequence[2] =
		((hotkey % 4) << _cycle_weapons_forward_bit) |
		hotkey_used;
}

/*
 *  Poll keyboard and return action flags
 */

uint32_t last_input_update;

uint32 parse_keymap(void)
{
  uint32 flags = 0;

  if(get_keyboard_controller_status())
    {
		Uint8 key_map[SDL_NUM_SCANCODES];
      if (Console::instance()->input_active()) {
	memset(key_map, 0, sizeof(key_map));
      } else {
		  memcpy(key_map, SDL_GetKeyboardState(NULL), sizeof(key_map));
      }
      
      // ZZZ: let mouse code simulate keypresses
      mouse_buttons_become_keypresses(key_map);
      joystick_buttons_become_keypresses(key_map);
      
      // Parse the keymap
		for (int i = 0; i < NUMBER_OF_STANDARD_KEY_DEFINITIONS; ++i)
		{
			BOOST_FOREACH(const SDL_Scancode& code, input_preferences->key_bindings[i])
			{
				if (key_map[code])
					flags |= standard_key_definitions[i].action_flag;
			}
		}
		
      // Post-process the keymap
		struct special_flag_data *special = special_flags;
      for (unsigned i=0; i<NUMBER_OF_SPECIAL_FLAGS; i++, special++) {
	if (flags & special->flag) {
	  switch (special->type) {
	  case _double_flag:
	    // If this flag has a double-click flag and has been hit within
	    // DOUBLE_CLICK_PERSISTENCE (but not at MAXIMUM_FLAG_PERSISTENCE),
	    // mask on the double-click flag */
	    if (special->persistence < MAXIMUM_FLAG_PERSISTENCE
		&&	special->persistence > MAXIMUM_FLAG_PERSISTENCE - DOUBLE_CLICK_PERSISTENCE)
	      flags |= special->alternate_flag;
	    break;
	    
	  case _latched_flag:
	    // If this flag is latched and still being held down, mask it out
	    if (special->persistence == MAXIMUM_FLAG_PERSISTENCE)
	      flags &= ~special->flag;
	    break;
	    
	  default:
	    assert(false);
	    break;
	  }
	  
	  special->persistence = MAXIMUM_FLAG_PERSISTENCE;
	} else
	  special->persistence = FLOOR(special->persistence-1, 0);
      }

	  if (!hotkey_sequence[0])
	  {
		  for (auto i = 0; i < NUMBER_OF_HOTKEYS; ++i)
		  {
			  auto& hotkey = input_preferences->hotkey_bindings[i];
			  for (auto it : hotkey)
			  {
				  if (key_map[it])
				  {
					  encode_hotkey_sequence(i);
					  break;
				  }
			  }
		  }
	  }

	  if (hotkey_sequence[0])
	  {
		  flags &= ~(_cycle_weapons_forward | _cycle_weapons_backward);
		  flags |= (hotkey_sequence[0] & ~hotkey_used);
		  hotkey_sequence[0] = hotkey_sequence[1];
		  hotkey_sequence[1] = hotkey_sequence[2];
		  hotkey_sequence[2] = 0;
	  }

      bool do_interchange =
	      (local_player->variables.flags & _HEAD_BELOW_MEDIA_BIT) ?
	      (input_preferences->modifiers & _inputmod_interchange_swim_sink) != 0:
	      (input_preferences->modifiers & _inputmod_interchange_run_walk) != 0;
      
      // Handle the selected input controller
      if (input_preferences->input_device == _mouse_yaw_pitch) {
          flags = process_aim_input(flags, pull_mouselook_delta());
      }
        int joyflags = process_joystick_axes(flags, heartbeat_count);
        if (joyflags != flags) {
            flags = joyflags;
        }

          // Modify flags with run/walk and swim/sink
        if (do_interchange)
	    flags ^= _run_dont_walk;
		
      
      if (player_in_terminal_mode(local_player_index))
	flags = build_terminal_action_flags((char *)key_map);
    } // if(get_keyboard_controller_status())
  
  return flags;
}


/*
 *  Get random demo replay from map
 */

bool setup_replay_from_random_resource(uint32 map_checksum)
{
	// not supported in SDL version
	return false;
}


/*
 *  Periodic task management
 */

typedef bool (*timer_func)(void);

static timer_func tm_func = NULL;	// The installed timer task
static uint32 tm_period;			// Ticks between two calls of the timer task
static uint32 tm_last = 0, tm_accum = 0;

timer_task_proc install_timer_task(short tasks_per_second, timer_func func)
{
	// We only handle one task, which is enough
	tm_period = 1000 / tasks_per_second;
	tm_func = func;
	tm_last = machine_tick_count();
	tm_accum = 0;
	return (timer_task_proc)tm_func;
}

void remove_timer_task(timer_task_proc proc)
{
	tm_func = NULL;
}

void execute_timer_tasks(uint32 time)
{
	if (tm_func) {
		if (Movie::instance()->IsRecording()) {
			if (get_fps_target() == 0 ||
				movie_export_phase++ % (get_fps_target() / 30) == 0)
			{
				tm_func();
			}
			return;
		}
		
		uint32 now = time;
		tm_accum += now - tm_last;
		tm_last = now;
		bool first_time = true;
		while (tm_accum >= tm_period) {
			tm_accum -= tm_period;
			if (first_time) {
				if(get_keyboard_controller_status())
					mouse_idle(input_preferences->input_device);

				first_time = false;
			}
			tm_func();
		}
	}
}


