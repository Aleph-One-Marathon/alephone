/*
	music.c
	Wednesday, July 12, 1995 11:03:53 PM
	This only allows for one song to be played at a given time.

Monday, November 6, 1995 10:20:01 AM  (Jason)
	allocate buffer when necessary.

Jan 30, 2000 (Loren Petrich):
	Added some typecasts

Mar 2, 2000 (Loren Petrich):
	Added alias resolution to opening of file

Mar 5, 2000 (Loren Petrich):
	Added crude checking for AIFF files (first 4 bytes must be 'FORM')

Aug 12, 2000 (Loren Petrich):
	Using object-oriented file handler
*/

/*
	Note I use the userInfo of the sound for my a5.
	(which means jason loses)

	Future->
		Have an AIFF file, that I use sndplaydoublebuffer to play from, reading in my
		buffers as needed.
*/
// LP note: A5 worlds are for interrupt-time stuff in the 68K MacOS;
// no need for them in the PowerPC MacOS or anywhere else


#include <stdlib.h>
#include <string.h>

#include "macintosh_cseries.h"

// #include "portable_files.h"
#include "music.h"

#include "song_definitions.h"

enum {
	_no_song_playing,
	_playing_introduction,
	_playing_chorus,
	_playing_trailer,
	_delaying_for_loop,
	_music_fading,
	NUMBER_OF_MUSIC_STATES
};

enum {
	_no_flags= 0x0000,
	_song_completed= 0x0001,
	_song_paused= 0x0002
};

/* ----------------- structures */
struct music_data {
	bool initialized;
	short flags;
	short state;
	short phase;
	short fade_duration;
	short play_count;
	short song_index;
	short next_song_index;
	// LP: using opened-file object
	OpenedFile OFile;
	// short song_file_refnum;
	short fade_interval_duration;
	short fade_interval_ticks;
	long ticks_at_last_update;
	char *sound_buffer;
	long sound_buffer_size;
	SndChannelPtr channel;
	FilePlayCompletionUPP completion_proc;
};

#define kDefaultSoundBufferSize (500*KILO)
#define BUILD_STEREO_VOLUME(l, r) ((((long)(r))<<16)|(l))

#ifdef DEBUG
static struct song_definition *get_song_definition(	
	short index)
{
	assert(index>=0 && index<NUMBER_OF_SONGS);
	return songs+index;
}
#else
#define get_song_definition(exp) (songs+(exp))
#endif

/* ----------------- globals */
// LP: no need for it to be a pointer
// static struct music_data *music_state= NULL;
music_data music_state;

/* ----------------- local prototypes */
static void shutdown_music_handler(void);
static pascal void file_play_completion_routine(SndChannelPtr channel);
static void allocate_music_channel(void);
static short get_sound_volume(void);

/* ----------------- code */

/* If channel is null, we don't initialize */
bool initialize_music_handler(FileSpecifier& SongFile)
//	FileDesc *song_file)
{
	short song_file_refnum;
	OSErr error;

	// assert(music_state==NULL);
	assert(NUMBER_OF_SONGS==sizeof(songs)/sizeof(struct song_definition));
		
	// LP addition: resolving music file if it was an alias
	// Boolean is_folder, was_aliased;
	// ResolveAliasFile((FSSpec *)song_file, true, &is_folder, &was_aliased);
		
	/* Does the file exist? */
	// LP change: using a file object
	// error= FSpOpenDF((FSSpec *) song_file, fsRdPerm, &song_file_refnum);
	if(SongFile.Open(music_state.OFile))
	{
		
		// LP change: check to see if the file is an AIFF one;
		OSType MusicHeader;
		const OSType AIFF_Header = 'FORM';
		
		if (!music_state.OFile.ReadObject(MusicHeader)) return false;
		// long NumBytes = 4;
		// error = FSRead(song_file_refnum, &NumBytes, &MusicHeader);
		// if (error != noErr) return false;
		if (MusicHeader != AIFF_Header) return false;
		
		// Reposition the file
		if (!music_state.OFile.SetPosition(0)) return false;
		// error = SetFPos(song_file_refnum, fsFromStart, 0);
		// if (error != noErr) return false;
		
		// LP: removed allocation of music-state object
		music_state.initialized= true;
		music_state.flags= 0;
		music_state.state= _no_song_playing;
		music_state.phase= 0;
		music_state.song_index= 0;
		music_state.next_song_index= NONE;
		// music_state.song_file_refnum= song_file_refnum;
		music_state.completion_proc= NewFilePlayCompletionProc(file_play_completion_routine);
		music_state.ticks_at_last_update= TickCount();
		/* Allocate our buffer */
		music_state.sound_buffer_size= kDefaultSoundBufferSize;
		music_state.sound_buffer= NULL;
			
//			music_state->sound_buffer= malloc(music_state->sound_buffer_size);
//			assert(music_state->sound_buffer);
		
		allocate_music_channel();
		
		assert(music_state.completion_proc);
		atexit(shutdown_music_handler);
	}
	// LP addition:
		return false;
	
	return true;
}

void free_music_channel(
	void)
{
	if (music_state.initialized && music_state.channel)
	{
		OSErr error;
		
		error= SndDisposeChannel(music_state.channel, true);
		vwarn(error==noErr, csprintf(temporary, "SndDisposeChannel returned %d;g", error));
		music_state.channel= NULL;
	}
}

void queue_song(
	short song_index)
{
	if (music_state.initialized && get_sound_volume())
	{
		if (!music_state.channel)
		{	
			allocate_music_channel();
		}
	
		if (music_state.channel)
		{
			if (music_playing())
			{
				/* By setting the song_index after we tell it to fade, we will */
				/*  cause the new song to start at the end of the fade. */
				fade_out_music(10*MACINTOSH_TICKS_PER_SECOND);
				music_state.song_index= song_index;
			}
			else
			{
				assert(music_state.state==_no_song_playing);
		
				/* Must be done everytime in case Jason killed it in sound.c */
				music_state.channel->userInfo= (long)(&music_state);
				music_state.song_index= song_index;
				music_state.state= _delaying_for_loop;
				music_state.phase= 1;
				music_state.ticks_at_last_update= TickCount();
				music_state.flags &= ~_song_completed;
				/* next time through we will start.. */
			}
		}
	}
}

void fade_out_music(
	short duration)
{
	if(music_playing())
	{
		music_state.fade_duration= duration;
		music_state.phase= duration;
		music_state.state= _music_fading;
		music_state.ticks_at_last_update= TickCount();
		music_state.fade_interval_duration= 5;
		music_state.fade_interval_ticks= 5;
		music_state.song_index= NONE;
	}
}

void music_idle_proc(
	void)
{
	if(music_state.initialized && music_state.state != _no_song_playing)
	{
		short ticks_elapsed= TickCount()-music_state.ticks_at_last_update;

		switch(music_state.state)
		{
			case _delaying_for_loop:
				if((music_state.phase-=ticks_elapsed)<=0)
				{
					/* Start playing.. */
					OSErr error;

					music_state.sound_buffer_size= kDefaultSoundBufferSize;
					music_state.sound_buffer= new char[music_state.sound_buffer_size];
					if (music_state.sound_buffer)
					{
						assert(music_state.channel);					
	
						error= SndStartFilePlay(music_state.channel, // channel
							music_state.OFile.GetRefNum(), // Not from an AIFF file.
							// music_state.song_file_refnum, // Not from an AIFF file.
							0, // our resource id.
							music_state.sound_buffer_size, // Buffer size
							music_state.sound_buffer, // Let it allocate a buffer for us.
							NULL, // Audio selection ptr.
							music_state.completion_proc, // Completion proc
							true); // Async.
						vwarn(error==noErr, csprintf(temporary, "SndStartFilePlay returned %d;g", error));
						if (!error) 
						{
							music_state.state= _playing_introduction;
						}
						else
						{
							music_state.state= _no_song_playing;
						}
					}
				}
				break;

			case _music_fading:
				if (ticks_elapsed>0)
				{
					if((music_state.phase-=ticks_elapsed)<=0 || (music_state.flags & _song_completed))
					{
						/* oops. we are done.. */
						stop_music();
						music_state.state= _no_song_playing;
						if(music_state.song_index != NONE)
						{
							/* Start the new song playing.. */
							queue_song(music_state.song_index);
						}
					} else {
						if(--music_state.fade_interval_ticks<=0)
						{
							short new_volume;
							SndCommand command;
							OSErr error;
							
							/* Only do this a few times.. */
							music_state.fade_interval_ticks= music_state.fade_interval_duration;
	
							/* Set the sound volume */
							new_volume= (0x100*music_state.phase)/music_state.fade_duration;

							/* set the sound volume */
							command.cmd= volumeCmd;
							command.param1= 0;
							command.param2= BUILD_STEREO_VOLUME(new_volume, new_volume);
							error= SndDoImmediate(music_state.channel, &command);
							vwarn(error==noErr, csprintf(temporary, "SndDoImmediate returned %d;g", error));
						}
					}
				}
				break;
			
			default:
				/* Don't change states until song_completed flag is set. */
				if(music_state.flags & _song_completed)
				{
					struct song_definition *song= get_song_definition(music_state.song_index);
					
					if(song->flags & _song_automatically_loops)
					{
						music_state.state= _delaying_for_loop;
						music_state.phase= song->restart_delay;
					} else {
						music_state.state= _no_song_playing;
					}
					music_state.flags &= ~_song_completed;
				}
				break;
		}
		music_state.ticks_at_last_update= TickCount();
	}

	return;
}

void stop_music(
	void)
{
	if (music_state.initialized && music_state.state != _no_song_playing)
	{
		OSErr error;
		
		error= SndStopFilePlay(music_state.channel, true);
		vwarn(error==noErr, csprintf(temporary, "StopFilePlay returned %d;g", error));
		music_state.state= _no_song_playing;
		
		delete []music_state.sound_buffer;
		music_state.sound_buffer= NULL;
	}
}

void pause_music(
	bool pause)
{
	if(music_playing())
	{
		bool pause_it= false;

		/* If they want us to pause, and it is not already paused. */
		if(pause)
		{
			if(!(music_state.flags & _song_paused))
			{
				music_state.flags |= _song_paused;
				pause_it= true;
			}
		} else {
			if(music_state.flags & _song_paused)
			{
				music_state.flags &= ~_song_paused;
				pause_it= true;
			}
		}

		/* SndPauseFilePlay is a toggle */
		if(pause_it)
		{
			OSErr error;
			
			error= SndPauseFilePlay(music_state.channel);
			vwarn(error==noErr, csprintf(temporary, "Pause error: %d;g", error));
		}
	}
}

bool music_playing(
	void)
{
	bool playing= false;
	
	if(music_state.initialized && music_state.state != _no_song_playing)
	{
		assert(music_state.channel);
		playing= true;
	}

	return playing;
}

/* --------------- private code */
static void shutdown_music_handler(
	void)
{
	if(music_state.initialized)
	{
		free_music_channel();
		music_state.OFile.Close();
		// FSClose(music_state.song_file_refnum);
	}
}

static pascal void file_play_completion_routine(
	SndChannelPtr channel)
{
	struct music_data *private_music_data= (struct music_data *) channel->userInfo;

	private_music_data->flags |= _song_completed;

	return;
}

static void allocate_music_channel(
	void)
{
	if(music_state.initialized)
	{
		OSErr error;

		assert(!music_state.channel);
		
		error= SndNewChannel(&music_state.channel, sampledSynth, initStereo, 
			NULL);
//		vwarn(error==noErr, csprintf(temporary, "SndNewChannel returned %d;g", error));
//		warn(music_state->channel);
	}
}

#include "world.h"
#include "map.h"
#include "shell.h"
#include "mysound.h"
#include "preferences.h"

/* Non reusable stuff */
static short get_sound_volume(
	void)
{
	return sound_preferences->volume;
}