/*
 *  preprocess_map_sdl.cpp - Save game routines, SDL implementation
 *
 *  Written in 2000 by Christian Bauer
 */

#include "cseries.h"
#include "FileHandler.h"

#include "world.h"
#include "map.h"
#include "shell.h"
#include "interface.h"
#include "game_wad.h"
#include "game_errors.h"


/*
 *  Get FileSpecifiers for default data files
 */

void get_default_map_spec(FileSpecifier &file)
{
	file.SetToGlobalDataDir();
	file.AddPart(getcstr(temporary, strFILENAMES, filenameDEFAULT_MAP));
	if (!file.Exists())
		alert_user(fatalError, strERRORS, badExtraFileLocations, -1);
}

void get_default_physics_spec(FileSpecifier &file)
{
	file.SetToGlobalDataDir();
	file.AddPart(getcstr(temporary, strFILENAMES, filenamePHYSICS_MODEL));
	// Don't care if it does not exist
}

void get_default_shapes_spec(FileSpecifier &file)
{
	file.SetToGlobalDataDir();
	file.AddPart(getcstr(temporary, strFILENAMES, filenameSHAPES8));
	if (!file.Exists())
		alert_user(fatalError, strERRORS, badExtraFileLocations, -1);
}

void get_default_sounds_spec(FileSpecifier &file)
{
	file.SetToGlobalDataDir();
	file.AddPart(getcstr(temporary, strFILENAMES, filenameSOUNDS8));
	// Don't care if it does not exist
}

bool get_default_music_spec(FileSpecifier &file)
{
	file.SetToGlobalDataDir();
	file.AddPart(getcstr(temporary, strFILENAMES, filenameMUSIC));
	return file.Exists();
}


/*
 *  Choose saved game for loading
 */

bool choose_saved_game_to_load(FileSpecifier &saved_game)
{
	return saved_game.ReadDialog(_typecode_savegame);
}


/*
 *  Save game
 */

bool save_game(void)
{
	pause_game();
	show_cursor();

	// Translate the name
	FileSpecifier file;
	get_current_saved_game_name(file);
	char game_name[256];
	file.GetName(game_name);

	// Display the dialog
	char prompt[256];
	bool success = file.WriteDialogAsync(_typecode_savegame, getcstr(prompt, strPROMPTS, _save_game_prompt), game_name);

	// Save game
	if (success)
		success = save_game_file(file);

	hide_cursor();
	resume_game();

	return success;
}


/*
 *  Store additional data in saved game file
 */

void add_finishing_touches_to_save_file(FileSpecifier &file)
{
printf("*** add_finishing_touches_to_save_file(%s)\n", file.GetPath());
	//!! overhead thumbnail, level name
}
