#ifndef __GAME_WAD_H
#define __GAME_WAD_H

/*
	GAME_WAD.H
	Sunday, July 3, 1994 10:45:56 PM

Jan 30, 2000 (Loren Petrich)
	Changed "new" to "_new" to make data structures more C++-friendly

June 15, 2000 (Loren Petrich):
	Added supprt for Chris Pruett's Pfhortran

Aug 12, 2000 (Loren Petrich):
	Using object-oriented file handler
*/

class FileSpecifier;

boolean save_game_file(FileSpecifier& File);

/* -------------- New functions */
void pause_game(void);
void resume_game(void);
void get_current_saved_game_name(FileSpecifier& File);

boolean match_checksum_with_map(short vRefNum, long dirID, unsigned long checksum, 
	FileSpecifier& File);
void set_map_file(FileSpecifier& File);

//CP Addition: get_map_file returns the FileDesc pointer to the current map
FileSpecifier& get_map_file(void);

/* --------- from PREPROCESS_MAP_MAC.C */
// Most of the get_default_filespecs moved to interface.h
void get_savegame_filedesc(FileSpecifier& File);

void add_finishing_touches_to_save_file(FileSpecifier& File);

#endif
