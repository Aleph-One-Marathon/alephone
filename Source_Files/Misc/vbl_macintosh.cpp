/*

	vbl_macintosh.c
	Friday, September 29, 1995 3:12:46 PM- rdm created.

Jan 30, 2000 (Loren Petrich):
	Did some typecasts

Feb 4, 2000 (Loren Petrich):
	Changed halt() to assert(false) for better debugging

Feb 10, 2000 (Loren Petrich):
	Added effects of run/walk and swim/sink interchanges
	to parse_keymap() -- this works with single-player;
	not sure if it will work with multiplayer

Jul 7, 2000 (Loren Petrich)
	Added Ben Thompson's ISp-support changes
*/

#include "macintosh_cseries.h"
#include <Folders.h>
#include <string.h>
#include <stdlib.h>

#include "map.h"
#include "shell.h"
#include "preferences.h"
#include "interface.h"
#include "mytm.h"
#include "player.h"
#include "mouse.h"
#include "key_definitions.h"
#include "tags.h" // for filetypes.
#include "computer_interface.h"
#include "ISp_Support.h" /* BT: Added April 16, 2000 for Input Sprocket Support */

// #include "portable_files.h"
#include "vbl.h"

#include "vbl_definitions.h"

#define MAXIMUM_FLAG_PERSISTENCE    15
#define DOUBLE_CLICK_PERSISTENCE    10
#define FILM_RESOURCE_TYPE          'film'

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

/* ------------ globals */
/* ------------ prototypes */
static OSErr copy_file(FSSpec *source, FSSpec *destination);
static void remove_input_controller(void);

/* ------------ code */
boolean find_replay_to_use(
	boolean ask_user,
	FileSpecifier& File)
//	FileDesc *file)
{
	boolean successful = FALSE;
	
	if(ask_user)
	{
		successful = File.ReadDialog(_typecode_film);
		/*
		StandardFileReply reply;
		SFTypeList types;
		short type_count= 0;

		types[type_count++]= FILM_FILE_TYPE;
		
		StandardGetFile(NULL, type_count, types, &reply);
		if (reply.sfGood)
		{
			memcpy(file, &reply.sfFile, sizeof(FSSpec));
			successful= TRUE;
		}
		*/
	}
	else
	{
		successful= get_recording_filedesc(File);
		// successful= get_recording_filedesc(file);
	}

	return successful;
}

// Now a member of FileSpecifier
/*
boolean get_freespace_on_disk(
	FileDesc *file,
	unsigned long *free_space)
{
	OSErr           error;
	HParamBlockRec  parms;

	memset(&parms, 0, sizeof(HParamBlockRec));	
	parms.volumeParam.ioCompletion = NULL;
	parms.volumeParam.ioVolIndex = 0;
	parms.volumeParam.ioNamePtr = ptemporary;
	parms.volumeParam.ioVRefNum = file->vRefNum;
	
	error = PBHGetVInfo(&parms, FALSE);
	if (error == noErr)
		*free_space = (unsigned long) parms.volumeParam.ioVAlBlkSiz * (unsigned long) parms.volumeParam.ioVFrBlk;
	return (error==noErr);
}
*/

/* true if it found it, false otherwise. always fills in vrefnum and dirid*/
boolean get_recording_filedesc(FileSpecifier& File)
//	FileDesc *file)
{
	File.SetParentToPreferences();
	File.SetName(getcstr(temporary, strFILENAMES, filenameMARATHON_RECORDING),_typecode_film);
	return File.Exists();
/*
	short vRef;
	long parID;
	OSErr error;

	error= FindFolder(kOnSystemDisk, kPreferencesFolderType, kCreateFolder, &vRef, &parID);
	if(!error)
	{
		getpstr(ptemporary, strFILENAMES, filenameMARATHON_RECORDING);
		error= FSMakeFSSpec(vRef, parID, ptemporary, (FSSpec *) file);
	}
	
	return (error==noErr);
*/
}

void move_replay(
	void)
{
	// FSSpec source_spec;
	FileSpecifier OrigFilmFile, MovedFilmFile;
	if(!get_recording_filedesc(OrigFilmFile)) return;
	// if(!get_recording_filedesc((FileDesc *) &source_spec)) return;
	
	// Need this temporary space for getting the strings
	char Prompt[256], DefaultName[256];
	if (!MovedFilmFile.WriteDialog(
			_typecode_film,
			getcstr(Prompt, strPROMPTS, _save_replay_prompt),
			getcstr(DefaultName, strFILENAMES, filenameMARATHON_RECORDING)
		))
		return;
	
	// OSErr err= copy_file(&OrigFilmFile.Spec, &MovedFilmFile.Spec);
	MovedFilmFile.CopyContents(OrigFilmFile);
	
	/* Alert them on problems */
	OSErr Err = MovedFilmFile.GetError();
	if (Err != noErr) alert_user(infoError, strERRORS, fileError, Err);

// Begin no-compile
#if 0
	Str255 suggested_name;
	StandardFileReply reply;
	
	getpstr(ptemporary, strPROMPTS, _save_replay_prompt);
	getpstr(suggested_name, strFILENAMES, filenameMARATHON_RECORDING);
	StandardPutFile(ptemporary, suggested_name, &reply);
	if(reply.sfGood)
	{
		FSSpec source_spec;
				
		if(get_recording_filedesc((FileDesc *) &source_spec))
		{
			OSErr err;
		
			/* If we are replacing.. */
			if(reply.sfReplacing)
			{
				FSpDelete(&reply.sfFile);
			}

			err= copy_file(&source_spec, &reply.sfFile);

			/* Alert them on problems */			
			if (err) alert_user(infoError, strERRORS, fileError, err);
		}
	}
// End no-compile
#endif

	return;
}

long parse_keymap(
	void)
{
	short i;
	long flags= 0;
	KeyMap key_map;
	struct key_definition *key= current_key_definitions;
	struct special_flag_data *special= special_flags;
	
	GetKeys(key_map);
	
	/******************************************************************************************/
	/* BT: Added April 16, 2000 ISp: This is where we get the input sprocket events */
	if(input_preferences->input_device==_input_sprocket_only || input_preferences->input_device==_keyboard_or_game_pad)
		flags = InputSprocketTestElements();
	/******************************************************************************************/

	/* parse the keymap */	
	for (i=0;i<NUMBER_OF_STANDARD_KEY_DEFINITIONS;++i,++key)
	{
		if (*((byte*)key_map + key->offset) & key->mask) flags|= key->action_flag;
	}

	/* post-process the keymap */
	for (i=0;i<NUMBER_OF_SPECIAL_FLAGS;++i,++special)
	{
		if (flags&special->flag)
		{
			switch (special->type)
			{
				case _double_flag:
					/* if this flag has a double-click flag and has been hit within
						DOUBLE_CLICK_PERSISTENCE (but not at MAXIMUM_FLAG_PERSISTENCE),
						mask on the double-click flag */
					if (special->persistence<MAXIMUM_FLAG_PERSISTENCE &&
						special->persistence>MAXIMUM_FLAG_PERSISTENCE-DOUBLE_CLICK_PERSISTENCE)
					{
						flags|= special->alternate_flag;
					}
					break;
				
				case _latched_flag:
					/* if this flag is latched and still being held down, mask it out */
					if (special->persistence==MAXIMUM_FLAG_PERSISTENCE) flags&= ~special->flag;
					break;
				
				default:
					// LP change:
					assert(false);
					// halt();
			}
			
			special->persistence= MAXIMUM_FLAG_PERSISTENCE;
		}
		else
		{
			special->persistence= FLOOR(special->persistence-1, 0);
		}
	}

	/* handle the selected input controller */
	if (input_preferences->input_device!=_keyboard_or_game_pad)
	{
		fixed delta_yaw, delta_pitch, delta_velocity;
		
		mouse_idle(input_preferences->input_device);
		test_mouse(input_preferences->input_device, &flags, &delta_yaw, &delta_pitch, &delta_velocity);
		flags= mask_in_absolute_positioning_information(flags, delta_yaw, delta_pitch, delta_velocity);
	}
	
	// LP addition: modify flags with run/walk and swim/sink
	bool do_interchange =
		(local_player->variables.flags&_HEAD_BELOW_MEDIA_BIT) ?
			(input_preferences->modifiers&_inputmod_interchange_swim_sink) != 0:
			(input_preferences->modifiers&_inputmod_interchange_run_walk) != 0;
	if (do_interchange) flags ^= _run_dont_walk;

	if(player_in_terminal_mode(local_player_index))
	{
		flags= build_terminal_action_flags((char *) key_map);
	}

	return flags;
}

#define COPY_BUFFER_SIZE (3*1024)

static OSErr copy_file(
	FSSpec *source,
	FSSpec *destination)
{
	OSErr err;
	FInfo info;
	
	err= FSpGetFInfo(source, &info);
	if(!err)
	{
		err= FSpCreate(destination, info.fdCreator, info.fdType, smSystemScript);
		if(!err)
		{
			short dest_refnum, source_refnum;
		
			err= FSpOpenDF(destination, fsWrPerm, &dest_refnum);
			if(!err)
			{
				err= FSpOpenDF(source, fsRdPerm, &source_refnum);
				if(!err)
				{
					/* Everything is opened. Do the deed.. */
					Ptr data;
					long total_length;
					
					SetFPos(source_refnum, fsFromLEOF, 0l);
					GetFPos(source_refnum, &total_length);
					SetFPos(source_refnum, fsFromStart, 0l);
					
					data= new char[COPY_BUFFER_SIZE];
					if(data)
					{
						long running_length= total_length;
						
						while(running_length && !err)
						{
							long count= MIN(COPY_BUFFER_SIZE, running_length);
						
							err= FSRead(source_refnum, &count, data);
							if(!err)
							{
								err= FSWrite(dest_refnum, &count, data);
							}
							running_length -= count;
						}
					
						free(data);
					} else {
						err= MemError();
					}
					
					FSClose(source_refnum);
				}

				FSClose(dest_refnum);
			}

			/* Delete it on an error */
			if(err) FSpDelete(destination);
		}
	}
	
	return err;
}

boolean setup_replay_from_random_resource(
	unsigned long map_checksum)
{
	short number_of_films;
	static short index_of_last_film_played= 0;
	boolean success= FALSE;
	
	number_of_films= CountResources(FILM_RESOURCE_TYPE);
	if(number_of_films > 0)
	{
		short which_film_to_play;
		Handle resource;
		long size;

		if (number_of_films == 1)
		{
			which_film_to_play= 0;
		}
		else
		{
			for (which_film_to_play = index_of_last_film_played; 
					which_film_to_play == index_of_last_film_played;
					which_film_to_play = (abs(Random()) % number_of_films))
				;
			index_of_last_film_played= which_film_to_play;
		}
	
		resource= GetIndResource(FILM_RESOURCE_TYPE, which_film_to_play+1);
		vassert(resource, csprintf(temporary, "film_to_play = %d", which_film_to_play+1));
		
		size= GetHandleSize(resource);
		replay.resource_data= new char[size];
		if(!replay.resource_data) alert_user(fatalError, strERRORS, outOfMemory, MemError());
		
		HLock(resource);
		BlockMove(*resource, &replay.header, sizeof(struct recording_header));
		BlockMove(*resource, replay.resource_data, size);
		HUnlock(resource);

		ReleaseResource(resource);

		if(replay.header.map_checksum==map_checksum)
		{
			replay.have_read_last_chunk= FALSE;
			replay.game_is_being_replayed= TRUE;

			replay.film_resource_offset= sizeof(struct recording_header);
			replay.resource_data_size= size;

			replay.valid= TRUE;
			
			success= TRUE;
		} else {
			replay.game_is_being_replayed= FALSE;

			replay.film_resource_offset= NONE;
			replay.resource_data_size= 0;
			delete []replay.resource_data;
			replay.resource_data= NULL;

			replay.valid= FALSE;
			
			success= FALSE;
		}
		
		replay.replay_speed= 1;
	}
	
	return success;
}

void set_keys_to_match_preferences(
	void)
{
	set_keys(input_preferences->keycodes);

	return;
}

timer_task_proc install_timer_task(
	short tasks_per_second, 
	boolean (*func)(void))
{
	myTMTaskPtr task;
	
	task= myTMSetup(1000/tasks_per_second, func);
	
	return task;
}

void remove_timer_task(
	timer_task_proc proc)
{
	myTMRemove((myTMTaskPtr) proc);
}

#ifdef DEBUG_REPLAY
#define MAXIMUM_STREAM_FLAGS (8192) // 48k

static short stream_refnum= NONE;
static struct recorded_flag *action_flag_buffer= NULL;
static long action_flag_index= 0;

static void open_stream_file(
	void)
{
	FSSpec file;
	char name[]= "\pReplay Stream";
	OSErr error;

	get_my_fsspec(&file);
	memcpy(file.name, name, name[0]+1);
	
	FSpDelete(&file);
	error= FSpCreate(&file, 'ttxt', 'TEXT', smSystemScript);
	if(error) dprintf("Err:%d", error);
	error= FSpOpenDF(&file, fsWrPerm, &stream_refnum);
	if(error || stream_refnum==NONE) dprintf("Open Err:%d", error);
	
	action_flag_buffer= new recorded_flag[MAXIMUM_STREAM_FLAGS];
	assert(action_flag_buffer);
	action_flag_index= 0;
}

static void write_flags(
	struct recorded_flag *buffer, 
	long count)
{
	long index, size;
	boolean sorted= TRUE;
	OSErr error;

	sprintf(temporary, "%d Total Flags\n", count);
	size= strlen(temporary);
	error= FSWrite(stream_refnum, &size, temporary);
	if(error) dprintf("Error: %d", error);

	if(sorted)
	{
		short player_index;
		
		for(player_index= 0; player_index<dynamic_world->player_count; ++player_index)
		{
			short player_action_flag_count= 0;
		
			for(index= 0; index<count; ++index)
			{
				if(buffer[index].player_index==player_index)
				{
					if(!(player_action_flag_count%TICKS_PER_SECOND))
					{
						sprintf(temporary, "%d 0x%08x (%d secs)\n", buffer[index].player_index, 
							buffer[index].flag, player_action_flag_count/TICKS_PER_SECOND);
					} else {
						sprintf(temporary, "%d 0x%08x\n", buffer[index].player_index, buffer[index].flag);
					}
					size= strlen(temporary);
					error= FSWrite(stream_refnum, &size, temporary);
					if(error) dprintf("Error: %d", error);
					player_action_flag_count++;
				}
			}
		}
	} else {
		for(index= 0; index<count; ++index)
		{
			sprintf(temporary, "%d 0x%08x\n", buffer[index].player_index, buffer[index].flag);
			size= strlen(temporary);
			error= FSWrite(stream_refnum, &size, temporary);
			if(error) dprintf("Error: %d", error);
		}
	}
}

static void debug_stream_of_flags(
	long action_flag,
	short player_index)
{
	if(stream_refnum != NONE)
	{
		assert(action_flag_buffer);
		if(action_flag_index<MAXIMUM_STREAM_FLAGS)
		{
			action_flag_buffer[action_flag_index].player_index= player_index;
			action_flag_buffer[action_flag_index++].flag= action_flag;
		}
	}
}

static void close_stream_file(
	void)
{
	if(stream_refnum != NONE)
	{
		assert(action_flag_buffer);

		write_flags(action_flag_buffer, action_flag_index-1);
		FSClose(stream_refnum);
		
		delete []action_flag_buffer;
		action_flag_buffer= NULL;
	}
}
#endif