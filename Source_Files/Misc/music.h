#ifndef __MUSIC_H
#define __MUSIC_H

/*
	music.h
	Wednesday, July 12, 1995 11:43:21 PM
	This only allows for one song to be played at a given time.

Aug 24, 2000 (Loren Petrich):
	Adding object-oriented file handling
*/


// LP: CodeWarrior complains unless I give the full definition of these classes
#include "FileHandler.h"
// class FileSpecifier;

enum { // All of our songs.
	_introduction_song= 0,
	NUMBER_OF_SONGS
};

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

#endif
