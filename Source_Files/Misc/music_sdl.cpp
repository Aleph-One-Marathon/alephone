/*
 *  music_sdl.cpp - Music playing, SDL implementation
 *
 *  Written in 2000 by Christian Bauer
 */

#include "cseries.h"
#include "FileHandler.h"
#include "music.h"
#include "song_definitions.h"


/*
 *  Initialize music handling
 */

boolean initialize_music_handler(FileSpecifier &song_file)
{
printf("*** initialize_music_handler()\n");
	//!!
	return false;
}


/*
 *  Free music channel
 */

void free_music_channel(void)
{
printf("*** free_music_channel()\n");
	//!!
}


/*
 *  Queue song for playing
 */

void queue_song(short song_index)
{
printf("*** queue_song(%d)\n", song_index);
	//!!
}


/*
 *  Fade out music
 */

void fade_out_music(short duration)
{
printf("*** fade_out_music(%d)\n", duration);
	//!!
}


/*
 *  Called regularily when music plays
 */

void music_idle_proc(void)
{
	//!!
}


/*
 *  Is music playing?
 */

boolean music_playing(void)
{
	//!!
	return false;
}
