/*
	music.c

	Copyright (C) 1991-2001 and beyond by Bungie Studios, Inc.
	and the "Aleph One" developers.
 
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	This license is contained in the file "COPYING",
	which is included with this source code; it is available online at
	http://www.gnu.org/licenses/gpl.html

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

Oct 12, 2000 (Loren Petrich):
	Added Quicktime support; will use Quicktime for playback
	whenever it is available

Oct 14, 2000 (Loren Petrich):
	Added support for per-level music that is specified in a level script

Mar 14, 2001 (Loren Petrich):
	Added use of old music player in case the music file's typecode was the "official" Marathon one

Jan 25, 2002 (Br'fin (Jeremy Parsons)):
	Added TARGET_API_MAC_CARBON for Quicktime.h
	Had to disable all of the all music player under Carbon
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


#if defined(EXPLICIT_CARBON_HEADER)
    #include <quicktime/Quicktime.h>
#else
#include <Movies.h>
#endif
#include <stdlib.h>
#include <string.h>

#include "XML_LevelScript.h"
#include "macintosh_cseries.h"

// #include "portable_files.h"
#include "shell.h"
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
	OpenedFile OFile;
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

//static struct song_definition *get_song_definition(	
//	short index);

/* ----------------- globals */
static music_data music_state;

// Stuff for playing a soundtrack with Quicktime;
// a NULL movie means don't play anything.
// Doing fadeouts by having a current volume and a change rate
// Need to save the introductory music file here
static FileSpecifier IntroMusicFile;
static FileSpecifier QTMusicFile;
static Movie QTMusicMovie = NULL;
static bool QTMMPlaying = false;
static bool QTMMPreloaded = false;
static double MusicVolume = 0;				// 0 to 1
static double MusicVolumeChange = 0;		// Change per tick [TickCount()]
static long MostRecentUpdateTicks = 0;
static bool IsLooped = false;				// Whether or not it repeats endlessly
static bool UsingOldPlayer = false;			// So as to be able to play 'mus2' files.

// This returns a value from between 0 and 1
static double GetOverallMusicVolume();
// Turned into a form more useful for QT
inline short GetQTMusicVolume() {return short(double(0x100)*GetOverallMusicVolume()*MusicVolume + 0.5);}

// Whether one should use the "new" music player to play anything.
inline bool UseNewPlayer() {return machine_has_quicktime() && !UsingOldPlayer;}

/* ----------------- local prototypes */
#if 0
static void shutdown_music_handler(void);
static pascal void file_play_completion_routine(SndChannelPtr channel);
static void allocate_music_channel(void);
#endif

static void PlayMusic(FileSpecifier& SongFile);
static void StartMusic();
static void PreloadMusic();

/* ----------------- code */

#if 0
song_definition *get_song_definition(	
	short index)
{
	assert(index>=0 && index<NUMBER_OF_SONGS);
	return songs+index;
}
#endif

// LP: Quicktime music player...

void PreloadLevelMusic()
{
	if (!UseNewPlayer()) return;
	
	stop_music();
	
	FileSpecifier *LevelSongFilePtr = GetLevelMusic();
	if (LevelSongFilePtr)
	{
		QTMMPreloaded = true;	// PlayMusic() depends on this being set to do preloading
		PlayMusic(*LevelSongFilePtr);
		IsLooped = false;
	}
}

void PlayMusic(FileSpecifier& SongFile)
{
	if (!UseNewPlayer()) return;

	if (QTMusicMovie)
	{
		if (SongFile == QTMusicFile)
		{
			// Don't need to rebuild the movie object; simply rewind and play again
			GoToBeginningOfMovie(QTMusicMovie);
			StartMusic();
			return;
		}
		else
		{
			QTMusicFile = SongFile;
			
			// Get rid of the old one and have a fallback in case
			// the new one is invalid
			DisposeMovie(QTMusicMovie);
			QTMusicMovie = NULL;
		}
	}
	QTMMPlaying = false;
	
	// Create a new movie object
	OSErr Err;
	short RefNum;
	short ResID = 0;	// Only use the first resource
	Boolean WasChanged;
	Err = OpenMovieFile(&SongFile.GetSpec(), &RefNum, fsRdPerm);
	if (Err != noErr) return;
	Err = NewMovieFromFile(&QTMusicMovie, RefNum, &ResID, NULL, newMovieActive, &WasChanged);
	CloseMovieFile(RefNum);
	if (Err != noErr) return;
	
	// Start or preload, as appropriate
	if (QTMMPreloaded)
		PreloadMusic();
	else
		StartMusic();
}

void StartMusic()
{
	MusicVolume = 1;				// Full blast
	MusicVolumeChange = 0;			// Staying full blast
	SetMovieVolume(QTMusicMovie,GetQTMusicVolume());
	StartMovie(QTMusicMovie);
	QTMMPlaying = true;
	MostRecentUpdateTicks = TickCount();
}

void PreloadMusic()
{
	MusicVolume = 1;				// Full blast
	MusicVolumeChange = 0;			// Staying full blast
	SetMovieVolume(QTMusicMovie,GetQTMusicVolume());
	PrerollMovie(QTMusicMovie,0,FIXED_ONE);	// The actual preloading function...
	MostRecentUpdateTicks = TickCount();
}


/* If channel is null, we don't initialize */
bool initialize_music_handler(FileSpecifier& SongFile)
{
/*
#if !defined(SUPPRESS_MACOS_CLASSIC)
	// In case the old player doesn't get initialized...
	music_state.initialized= false;
	
	// Check on whether we'll be using it
	UsingOldPlayer = (SongFile.GetType() == _typecode_music);
#endif
*/
	
	// LP: using Quicktime to play the movie if available
	if (UseNewPlayer())
	{
		// Will need to remember what introductory music file
		IntroMusicFile = SongFile;
		return true;
	}

/*	
#if !defined(SUPPRESS_MACOS_CLASSIC)
	// Just in case the initial music was skipped and we want to go straight to a level...
	UsingOldPlayer = false;

	short song_file_refnum;
	OSErr error;

	// assert(music_state==NULL);
	assert(NUMBER_OF_SONGS==sizeof(songs)/sizeof(struct song_definition));
		
	// LP addition: resolving music file if it was an alias
	// Boolean is_folder, was_aliased;
	// ResolveAliasFile((FSSpec *)song_file, true, &is_folder, &was_aliased);
	
	*//* Does the file exist? *//*
	// LP change: using a file object
	if(SongFile.Open(music_state.OFile))
	{
		// LP change: check to see if the file is an AIFF one;
		OSType MusicHeader;
		const OSType AIFF_Header = 'FORM';
		
		if (!music_state.OFile.Read(sizeof(MusicHeader),&MusicHeader)) return false;
		if (MusicHeader != AIFF_Header) return false;
		
		// Reposition the file
		if (!music_state.OFile.SetPosition(0)) return false;
		
		// LP: removed allocation of music-state object
		music_state.initialized= true;
		music_state.flags= 0;
		music_state.state= _no_song_playing;
		music_state.phase= 0;
		music_state.song_index= 0;
		music_state.next_song_index= NONE;
		music_state.completion_proc= NewFilePlayCompletionProc(file_play_completion_routine);
		music_state.ticks_at_last_update= TickCount();
		*//* Allocate our buffer *//*
		music_state.sound_buffer_size= kDefaultSoundBufferSize;
		music_state.sound_buffer= NULL;
			
		allocate_music_channel();
		
		assert(music_state.completion_proc);
		atexit(shutdown_music_handler);
	}
#endif
*/

	return false;
}

void free_music_channel(
	void)
{
	// Who cares about this if QT is present?
	if (UseNewPlayer()) return;
	
#if !defined(SUPRESS_MACOS_CLASSIC)	
	if (music_state.initialized && music_state.channel)
	{
		OSErr error;
		
		error= SndDisposeChannel(music_state.channel, true);
		vwarn(error==noErr, csprintf(temporary, "SndDisposeChannel returned %d;g", error));
		music_state.channel= NULL;
	}
#endif
}

void queue_song(
	short song_index)
{
	// Test for using the old player: whether it had been inited
	UsingOldPlayer = music_state.initialized;

	// This routine plays only the introductory song;
	// it is looped, and will repeat unless stopped by something
	if (UseNewPlayer())
	{
		PlayMusic(IntroMusicFile);
		IsLooped = true;
		return;
	}

/*
#if !defined(SUPRESS_MACOS_CLASSIC)
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
				*//* By setting the song_index after we tell it to fade, we will *//*
				*//*  cause the new song to start at the end of the fade. *//*
				fade_out_music(10*MACINTOSH_TICKS_PER_SECOND);
				music_state.song_index= song_index;
			}
			else
			{
				assert(music_state.state==_no_song_playing);
		
				*//* Must be done everytime in case Jason killed it in sound.c *//*
				music_state.channel->userInfo= (long)(&music_state);
				music_state.song_index= song_index;
				music_state.state= _delaying_for_loop;
				music_state.phase= 1;
				music_state.ticks_at_last_update= TickCount();
				music_state.flags &= ~_song_completed;
				*//* next time through we will start.. *//*
			}
		}
	}
#endif
*/
}

void fade_out_music(
	short duration)
{
	if (UseNewPlayer())
	{
		MusicVolumeChange = - 1.0/duration;
		return;
	}

/*
#if !defined(SUPRESS_MACOS_CLASSIC)
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
#endif
*/
}

void music_idle_proc(
	void)
{
	// Quicktime: what could be easier?
	if (UseNewPlayer())
	{
		// Start preloaded music
		if (QTMusicMovie && QTMMPreloaded)
		{
			StartMovie(QTMusicMovie);
			QTMMPlaying = true;
			QTMMPreloaded = false;
		}
		
		// Keep the music going if it is already going
		if (QTMusicMovie && QTMMPlaying)
		{
			if (IsMovieDone(QTMusicMovie))
			{
				if (IsLooped)
				{
					GoToBeginningOfMovie(QTMusicMovie);
					StartMusic();
				}
				else
					QTMMPlaying = false;
			}
			else
			{
				// Adjust the volume
				long CurrentTicks = TickCount();
				MusicVolume += MusicVolumeChange*(CurrentTicks - MostRecentUpdateTicks);
				MostRecentUpdateTicks = CurrentTicks;
				
				if (MusicVolume > 1) MusicVolume = 1;
				else if (MusicVolume <= 0)
				{
					stop_music();
					return;
				}
				SetMovieVolume(QTMusicMovie,GetQTMusicVolume());
			
				// Keep it going
				MoviesTask(QTMusicMovie,0);
			}
		}
		
		// Get some music to play if in a level;
		// it will play only once, and has to be explicitly restarted to repeat
		if (!QTMMPlaying)
		{
			FileSpecifier *LevelSongFilePtr = GetLevelMusic();
			if (LevelSongFilePtr)
			{
				PlayMusic(*LevelSongFilePtr);
				IsLooped = false;
			}
		}
		
		return;
	}

/*
#if !defined(SUPPRESS_MACOS_CLASSIC)
	if(music_state.initialized && music_state.state != _no_song_playing)
	{
		short ticks_elapsed= TickCount()-music_state.ticks_at_last_update;

		switch(music_state.state)
		{
			case _delaying_for_loop:
				if((music_state.phase-=ticks_elapsed)<=0)
				{
					*//* Start playing.. *//*
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
						*//* oops. we are done.. *//*
						stop_music();
						music_state.state= _no_song_playing;
						if(music_state.song_index != NONE)
						{
							*//* Start the new song playing.. *//*
							queue_song(music_state.song_index);
						}
					} else {
						if(--music_state.fade_interval_ticks<=0)
						{
							short new_volume;
							SndCommand command;
							OSErr error;
							
							*//* Only do this a few times.. *//*
							music_state.fade_interval_ticks= music_state.fade_interval_duration;
	
							*//* Set the sound volume *//*
							new_volume= (0x100*music_state.phase)/music_state.fade_duration;

							*//* set the sound volume *//*
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
				*//* Don't change states until song_completed flag is set. *//*
				if(music_state.flags & _song_completed)
				{
					struct song_definition *song= get_song_definition(music_state.song_index);
					
					if(song->flags & _song_automatically_loops)
					{
						music_state.state= _delaying_for_loop;
						music_state.phase= song->restart_delay;
					} else {
						music_state.state= _no_song_playing;
						// Set up for doing level music, which needs the new player
						UsingOldPlayer = false;
					}
					music_state.flags &= ~_song_completed;
				}
				break;
		}
		music_state.ticks_at_last_update= TickCount();
	}
#endif
*/
}

void stop_music(
	void)
{
	if (UseNewPlayer())
	{
		if (QTMusicMovie && QTMMPlaying)
		{
			StopMovie(QTMusicMovie);
			QTMMPlaying = false;
			
			DisposeMovie(QTMusicMovie);
			QTMusicMovie = NULL;
		}
		return;
	}

/*
#if !defined(SUPPRESS_MACOS_CLASSIC)
	if (music_state.initialized && music_state.state != _no_song_playing)
	{
		OSErr error;
		
		error= SndStopFilePlay(music_state.channel, true);
		vwarn(error==noErr, csprintf(temporary, "StopFilePlay returned %d;g", error));
		music_state.state= _no_song_playing;
		
		delete []music_state.sound_buffer;
		music_state.sound_buffer= NULL;
	}
#endif
*/
	// Set up for doing level music, which needs the new player
	UsingOldPlayer = false;
}

void pause_music(bool pause)
{
	if (UseNewPlayer())
	{
		if (QTMusicMovie && QTMMPlaying)
		{
			if (pause)
				StopMovie(QTMusicMovie);
			else
				StartMovie(QTMusicMovie);
		}
		return;
	}

/*
#if !defined(SUPPRESS_MACOS_CLASSIC)
	if(music_playing())
	{
		bool pause_it= false;

		*//* If they want us to pause, and it is not already paused. *//*
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

		*//* SndPauseFilePlay is a toggle *//*
		if(pause_it)
		{
			OSErr error;
			
			error= SndPauseFilePlay(music_state.channel);
			vwarn(error==noErr, csprintf(temporary, "Pause error: %d;g", error));
		}
	}
#endif
*/
}

bool music_playing(void)
{
	if (UseNewPlayer()) return QTMMPlaying;

/*
#if !defined(SUPRESS_MACOS_CLASSIC)
	bool playing= false;
	
	if(music_state.initialized && music_state.state != _no_song_playing)
	{
		assert(music_state.channel);
		playing= true;
	}

	return playing;
#else
*/
	return false;
//#endif
}

/* --------------- private code */
#if 0
static void shutdown_music_handler(
	void)
{
	if (UseNewPlayer()) return;

/*
#if !defined(SUPRESS_MACOS_CLASSIC)
	if(music_state.initialized)
	{
		free_music_channel();
		music_state.OFile.Close();
		// FSClose(music_state.song_file_refnum);
	}
#endif
*/
}

static pascal void file_play_completion_routine(
	SndChannelPtr channel)
{
	struct music_data *private_music_data= (struct music_data *) channel->userInfo;

	private_music_data->flags |= _song_completed;
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
#endif

#include "world.h"
#include "map.h"
#include "mysound.h"
// Because something in preferences.h
//    typedef float GLfloat
// in GL/gl.h not to work properly.
extern struct sound_manager_parameters *sound_preferences;
// #include "preferences.h"

/* Non reusable stuff */
#if 0
static short get_sound_volume(
	void)
{
	return sound_preferences->volume;
}
#endif

// LP: added music volume
static double GetOverallMusicVolume()
{
	return double(sound_preferences->music)/double(NUMBER_OF_SOUND_VOLUME_LEVELS-1);
}
