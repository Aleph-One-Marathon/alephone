/*
 *  preprocess_map_sdl.cpp - Save game routines, SDL implementation
 *
 *  Written in 2000 by Christian Bauer
 */

#include "cseries.h"
#include "FileHandler.h"

#include "world.h"
#include "map.h"
#include "interface.h"
#include "game_wad.h"


/*
 *  Choose saved game for loading
 */

boolean choose_saved_game_to_load(FileSpecifier &saved_game)
{
printf("*** choose_saved_game_to_load()\n");
	//!!
	return false;
}


/*
 *  Save game
 */

boolean save_game(void)
{
	pause_game();
	show_cursor();

	// Translate the name, and display the dialog
	FileSpecifier file;
	get_current_saved_game_name(file);

printf("*** save_game()\n");
	//!!

	hide_cursor();
	resume_game();

	return false;
}


/*
 *  Store additional data in saved game file
 */

void add_finishing_touches_to_save_file(FileSpecifier &file)
{
printf("*** add_finishing_touches_to_save_file(%s)\n", file.GetName());
	//!! overhead thumbnail, level name
}
