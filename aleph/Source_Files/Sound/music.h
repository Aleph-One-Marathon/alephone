#ifndef __MUSIC_H
#define __MUSIC_H

/*
	music.h

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

	Wednesday, July 12, 1995 11:43:21 PM
	This only allows for one song to be played at a given time.

Aug 24, 2000 (Loren Petrich):
	Adding object-oriented file handling
*/


// LP: CodeWarrior complains unless I give the full definition of these classes
#include "FileHandler.h"

enum { // All of our songs.
	_introduction_song= 0,
	NUMBER_OF_SONGS
};

#ifdef WIN32
#ifndef WIN32_DISABLE_MUSIC
// We need to process window events to find out about media events. 
#   define WM_DSHOW_GRAPH_NOTIFY (WM_APP+1)

void process_music_event_win32(const SDL_Event& event); // in sound_sdl.cpp
#endif
#endif

// All this is for the introduction/ending song file

bool initialize_music_handler(FileSpecifier& SongFile);

void queue_song(short song_index);

void music_idle_proc(void);

void stop_music(void);
void pause_music(bool pause);

bool music_playing(void);

void free_music_channel(void);
void fade_out_music(short duration);

// LP: this is so that a level can have some music loaded when it starts running
void PreloadLevelMusic();
void StopLevelMusic();

#endif
