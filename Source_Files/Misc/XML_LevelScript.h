#ifndef _XML_LEVEL_SCRIPT_
#define _XML_LEVEL_SCRIPT_
/*
	Support for XML scripts in map files
	by Loren Petrich,
	April 16, 2000

	The reason for a separate object is that it will be necessary to execute certain commands
	only on certain levels.

Nov 25, 2000 (Loren Petrich)
	Added support for specifying movies for levels, as Jesse Simko had requested
*/


#include "FileHandler.h"

// Loads all those in resource 128 in a map file (or some appropriate equivalent)
void LoadLevelScripts(FileSpecifier& MapFile);

// Runs a script for some level; loads Pfhortran,
// runs level-specific MML...
void RunLevelScript(int LevelIndex);

// Intended to be run at the end of a game
void RunEndScript();

// Intended for restoring old parameter values, because MML sets values at a variety
// of different places, and it may be easier to simply set stuff back to defaults
// by including those defaults in the script.
void RunRestorationScript();

// Indicates whether a level is active; this is for telling music.cpp not to loop the music
bool IsLevelMusicActive();

// When leaving the game for the main menu; indicates that there will be no script
// to use for music files
void StopLevelMusic();

// Gets the next song file to play music from, as a pointer to the file specifier.
// A NULL pointer means no music to play
FileSpecifier *GetLevelMusic();

// Finds the level movie and the end movie, to be used in show_movie()
// The first is for some level,
// while the second is for the end of a game
void FindLevelMovie(short index);
void FindEndMovie();

// Gets the pointer of a movie to play at a level, as a pointer to the file specifier.
// A NULL pointer means no movie to play
FileSpecifier *GetLevelMovie();

#endif
