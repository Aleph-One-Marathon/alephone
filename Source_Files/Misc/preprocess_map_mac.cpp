/*
	preprocess_map_mac.c

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

	This license is contained in the file "GNU_GeneralPublicLicense.txt",
	which is included with this source code; it is available online at
	http://www.gnu.org/licenses/gpl.html

	Mac specific map reading routines.

	Sunday, December 5, 1993 1:00:31 PM- rdm created
	Friday, June 16, 1995 9:12:50 AM- I am completely fired.  And a complete fucking moron.  The
		revert game information is stored in stale pointers, which is crack, because they are 
		pointing into random stack memory.  This is completely evil, totally stupid, and I feel
		ridiculously dumb.
	Saturday, August 26, 1995 12:33:51 PM- most of the crap in this file can be cleaned up.  Some
		of these functions need tweaking, and some should be in game_wad.c

Jan 30, 2000 (Loren Petrich):
	Changed "new" to "_new" to make data structures more C++-friendly
	Surrounded choose_saved_game_to_load with "extern "C""

Aug 12, 2000 (Loren Petrich):
	Using object-oriented file handler

Aug 24, 2000 (Loren Petrich):
	Moved save-dialog Mac-specific code out to FileHandler

Jan 31, 2001 (Loren Petrich);
	Delete some old MacOS-specific load-and-save junk.
*/

#include "macintosh_cseries.h"
#include "world.h"
#include "map.h"
// LP change: moved this into main directory:
#include "editor.h"
// #include ":editor code:editor.h"
#include "interface.h"
#include "shell.h"
#include "game_wad.h"
#include "overhead_map.h"
#include "player.h"
#include "game_window.h"
#include "game_errors.h"
#include "FileHandler.h"

#include "tags.h"
#include "wad.h"
#include "game_wad.h"

#include <string.h>

#ifdef env68k
#pragma segment file_io
#endif

#define kFolderBit 0x10
#define THUMBNAIL_ID 128
#define THUMBNAIL_WIDTH 100
#define THUMBNAIL_HEIGHT 100
#define strSAVE_LEVEL_NAME 128

#define dlogMY_REPLACE 131

/* Needed for thumbnail crap.. */
extern GWorldPtr world_pixels;

enum {
	dlogCUSTOM_GET= 130,
	iTHUMBNAIL_RECT= 11,
	iLEVEL_NAME
};

/* ---- local prototypes */
static void add_overhead_thumbnail(void);


/* --------- code begins */

void get_default_map_spec(FileSpecifier& File)
{
	File.SetToApp();
	File.SetName(getcstr(temporary, strFILENAMES, filenameDEFAULT_MAP),_typecode_scenario);
	if (!File.Exists()) alert_user(fatalError, strERRORS, badExtraFileLocations, fnfErr);
}

void get_default_physics_spec(FileSpecifier& File)
{
	File.SetToApp();
	File.SetName(getcstr(temporary, strFILENAMES, filenamePHYSICS_MODEL),_typecode_physics);
	// Don't care if it does not exist
}

void get_default_shapes_spec(FileSpecifier& File)
{
	File.SetToApp();
	File.SetName(getcstr(temporary, strFILENAMES, filenameSHAPES8),_typecode_shapes);
	if (!File.Exists()) alert_user(fatalError, strERRORS, badExtraFileLocations, fnfErr);
}

void get_default_sounds_spec(FileSpecifier& File)
{
	File.SetToApp();
	File.SetName(getcstr(temporary, strFILENAMES, filenameSOUNDS8),_typecode_sounds);
	if (!File.Exists()) alert_user(fatalError, strERRORS, badExtraFileLocations, fnfErr);
}


// extern "C" {
extern bool choose_saved_game_to_load(FileSpecifier& File);
// extern bool choose_saved_game_to_load(FSSpec *saved_game);
// }

/* Note this should show the map of where you are... */
bool choose_saved_game_to_load(FileSpecifier& File)
	// FSSpec *saved_game)
{
	return File.ReadDialog(_typecode_savegame);
	/*
	SFTypeList type_list;
	short type_count= 0;
	StandardFileReply reply;

	type_list[type_count++]= SAVE_GAME_TYPE;

	StandardGetFile(NULL, type_count, type_list, &reply);

	if(reply.sfGood)
	{
		BlockMove(&reply.sfFile, saved_game, sizeof(FSSpec));
	}

	return reply.sfGood;
	*/
}
extern bool Tracer;
bool save_game(
	void)
{
	pause_game();
	show_cursor();

	/* Translate the name, and display the dialog */
	FileSpecifier SaveFile;
	get_current_saved_game_name(SaveFile);
	char GameName[256];
	SaveFile.GetName(GameName);
	
	char Prompt[256];
	// Must allow the sound to play in the background
	bool success = SaveFile.WriteDialogAsync(
			_typecode_savegame,
			getcstr(Prompt, strPROMPTS, _save_game_prompt),
			GameName);

	if (success)
		success = save_game_file(SaveFile);
	
	hide_cursor();
	resume_game();
	
	return success;
}

void add_finishing_touches_to_save_file(FileSpecifier &File)
// 	FileDesc *file)
{
	short refnum;
	unsigned char name[64+1];
	OSErr err;
	
	FSSpec *SpecPtr = &File.GetSpec();
	
	FInfo finder_info;
	err = FSpGetFInfo((FSSpec *)SpecPtr, &finder_info);
	if (err != noErr) return;
	
	FSpCreateResFile(SpecPtr,  finder_info.fdCreator, finder_info.fdType, smSystemScript);
	if (ResError() != noErr) return;
		
	/* Save the STR resource that tells us what our application name is. */
	refnum= FSpOpenResFile(&File.GetSpec(), fsWrPerm);
	// resource_file_ref= FSpOpenResFile((FSSpec *) file, fsWrPerm);
	if (refnum < 0) return;
	
	Handle resource;
	
	/* Add in the save level name */
	strcpy((char *)name, static_world->level_name);
	c2pstr((char *)name);
	err= PtrToHand(name, &resource, name[0]+1);
	assert(!err && resource);
	
	AddResource(resource, 'STR ', strSAVE_LEVEL_NAME, "\p");
	ReleaseResource(resource);

	/* Add in the overhead thumbnail. */
	add_overhead_thumbnail();
	
	/* Add the application name resource.. */
	getpstr(name, strFILENAMES, filenameMARATHON_NAME);
	// add_application_name_to_fsspec((FileDesc *) file, name);
	
	// LP: copied out of files_macintosh.c -- add_application_name_to_fsspec();
	// this is the only place that uses this code
									
	/* Add in the application name */
	err= PtrToHand(name, &resource, name[0]+1);
	assert(!err && resource);
							
	AddResource(resource, 'STR ', -16396, "\p");
	ReleaseResource(resource);
				
	CloseResFile(refnum);
	// End copying
}

/* ------------ Local code */
static void add_overhead_thumbnail(
	void)
{
	PicHandle picture;
	GWorldPtr old_gworld;
	GDHandle old_device;
	struct overhead_map_data overhead_data;
	Rect bounds;

	GetGWorld(&old_gworld, &old_device);
	SetGWorld(world_pixels, (GDHandle) NULL);

	assert(THUMBNAIL_WIDTH<RECTANGLE_WIDTH(&world_pixels->portRect));
	assert(THUMBNAIL_HEIGHT<RECTANGLE_HEIGHT(&world_pixels->portRect));

	/* Create the bounding rectangle */
	SetRect(&bounds, 0, 0, THUMBNAIL_WIDTH, THUMBNAIL_HEIGHT);

	/* Start recording.. */
	picture= OpenPicture(&bounds);
	
	PaintRect(&bounds);

	overhead_data.scale= OVERHEAD_MAP_MINIMUM_SCALE;
	overhead_data.origin.x= local_player->location.x;
	overhead_data.origin.y= local_player->location.y;
	overhead_data.half_width= RECTANGLE_WIDTH(&bounds)/2;
	overhead_data.half_height= RECTANGLE_HEIGHT(&bounds)/2;
	overhead_data.width= RECTANGLE_WIDTH(&bounds);
	overhead_data.height= RECTANGLE_HEIGHT(&bounds);
	overhead_data.mode= _rendering_saved_game_preview;
	
	_render_overhead_map(&overhead_data);

	RGBForeColor(&rgb_black);
	PenSize(1, 1);
	TextFont(0);
	TextFace(normal);
	TextSize(0);

	ClosePicture();
	
	AddResource((Handle) picture, 'PICT', THUMBNAIL_ID, "\p");
	ReleaseResource((Handle) picture);

	SetGWorld(old_gworld, old_device);
	
	return;
}
