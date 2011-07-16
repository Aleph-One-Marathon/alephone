/*
	preprocess_map_mac.c

	Copyright (C) 1991-2001 and beyond by Bungie Studios, Inc.
	and the "Aleph One" developers.
 
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	This license is contained in the file "COPYING",
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

Jan 25, 2002 (Br'fin (Jeremy Parsons)):
	Replaced a c2pstr with CopyCString2Pascal for Carbon
	Added accessors for datafields now opaque in Carbon

Feb 16, 2002 (Br'fin (Jeremy Parsons)):
	Removed enums for long removed standard file preview junk
	Replaced creation of old preview junk with NavigationServices preview junk

Apr 22, 2003 (Woody Zenfell):
        Reworked default-support-files getting; searches A1's tree for the first match rather
        than giving Map, Shapes, Sounds, Physics Model special status.  Means no more need
        to have those files alongside A1 application!
*/

#include "macintosh_cseries.h"
#include "world.h"
#include "map.h"
#include "editor.h"
#include "interface.h"
#include "shell.h"
#include "game_wad.h"
#include "overhead_map.h"
#include "player.h"
#include "game_window.h"
#include "game_errors.h"
#include "FileHandler.h"
#include "find_files.h"

#include "tags.h"
#include "wad.h"
#include "game_wad.h"

#include <string.h>

#ifdef env68k
#pragma segment file_io
#endif

#define kNavServicesVersion_2_0 0x02000000

#define kFolderBit 0x10
#define THUMBNAIL_ID 128
#define THUMBNAIL_WIDTH 100
#define THUMBNAIL_HEIGHT 100
#define PREVIEW_WIDTH 128
#define PREVIEW_HEIGHT 124
#define PREVIEW_IMAGE_X ((PREVIEW_WIDTH - THUMBNAIL_WIDTH)/2)
#define PREVIEW_IMAGE_Y 0
#define PREVIEW_LABEL_X 0
#define PREVIEW_LABEL_Y 105

#define strSAVE_LEVEL_NAME 128

#define dlogMY_REPLACE 131

/* Needed for thumbnail crap.. */
extern GWorldPtr world_pixels;

/* ---- local prototypes */
static void add_overhead_thumbnail(FileSpecifier &File);


/* --------- code begins */

void get_default_map_spec(FileSpecifier& File)
{
        get_default_file_specs(&File, NULL, NULL, NULL);
}

void get_default_physics_spec(FileSpecifier& File)
{
        get_default_file_specs(NULL, NULL, NULL, &File);
}

void get_default_shapes_spec(FileSpecifier& File)
{
        get_default_file_specs(NULL, &File, NULL, NULL);
}

void get_default_sounds_spec(FileSpecifier& File)
{
        get_default_file_specs(NULL, NULL, &File, NULL);
}

struct file_type_to_find_rec
{
        int		type;
        bool 		found;
        FileSpecifier	file;
        FileSpecifier*	output_file;
};

static int file_typecodes_to_find[] =
{
        _typecode_scenario,
        _typecode_shapes,
        _typecode_sounds,
        _typecode_physics
};

static const int NUMBER_OF_FILE_TYPECODES_TO_FIND = sizeof(file_typecodes_to_find) / sizeof(file_typecodes_to_find[0]);

static file_type_to_find_rec file_types_to_find[NUMBER_OF_FILE_TYPECODES_TO_FIND];

// ZZZ: code lifted and slightly altered from preferences_macintosh.h
static bool found_some_file_callback(
                                               FileSpecifier& File,
                                               void *data)
{
        bool FoundAll = true;
        int Filetype = File.GetType();
        for(int i = 0; i < NUMBER_OF_FILE_TYPECODES_TO_FIND; i++)
        {
                if(!file_types_to_find[i].found && Filetype == file_types_to_find[i].type)
                {
                        file_types_to_find[i].found = true;
                        file_types_to_find[i].file = File;
                }
                FoundAll &= file_types_to_find[i].found;
        }

        // Return value indicates whether we want to continue the search
        return !FoundAll;
}

static void search_from_directory(DirectorySpecifier& BaseDir)
//	FSSpec *file)
{
        FileFinder pb;

        pb.Clear();
        pb.version= 0;
        // LP change: always recurse
        pb.flags= _ff_recurse | _ff_callback_with_catinfo;
#if 0
#ifdef FINAL
        pb.flags= _ff_recurse | _ff_callback_with_catinfo;
#else
        pb.flags= _ff_callback_with_catinfo;
#endif
#endif
        pb.search_type= _callback_only;
        pb.BaseDir = BaseDir;
        pb.Type= WILDCARD_TYPE;
        pb.buffer= NULL;
        pb.max= 32767; // maximum
        pb.callback= found_some_file_callback;
        pb.user_data= NULL;
        pb.count= 0;

        bool seek_ok= pb.Find();
        if(!seek_ok)
          vassert(seek_ok, csprintf(temporary, "Error: %d", pb.GetError()));
}

// ZZZ new function finds any or all default files at once.
// default file is now the first matching file of correct type (no more "Map", "Shapes", "Sounds", etc.)
void get_default_file_specs(FileSpecifier* outMapSpec, FileSpecifier* outShapesSpec, FileSpecifier* outSoundsSpec, FileSpecifier* outPhysicsSpec)
{
        for(int i = 0; i < NUMBER_OF_FILE_TYPECODES_TO_FIND; i++)
        {
                file_types_to_find[i].type = file_typecodes_to_find[i];
                
                // Set up to find what the caller is looking for (anything not NULL)
                switch(file_types_to_find[i].type)
                {
                        case _typecode_scenario:
                                file_types_to_find[i].output_file = outMapSpec;
                                break;
                        case _typecode_shapes:
                                file_types_to_find[i].output_file = outShapesSpec;
                                break;
                        case _typecode_sounds:
                                file_types_to_find[i].output_file = outSoundsSpec;
                                break;
                        case _typecode_physics:
                                file_types_to_find[i].output_file = outPhysicsSpec;
                                break;
                        default:
                                assert(false);
                                break;
                }

                file_types_to_find[i].found = (file_types_to_find[i].output_file == NULL);
        }

        // Do the search
        DirectorySpecifier BaseDir;
        Files_GetRootDirectory(BaseDir);
        search_from_directory(BaseDir);

        // Write out the results and determine whether we found the required files.
        bool FoundAllRequired = true;
        for(int i = 0; i < NUMBER_OF_FILE_TYPECODES_TO_FIND; i++)
        {
                if(file_types_to_find[i].found)
                {
                        if(file_types_to_find[i].output_file != NULL)
                                *(file_types_to_find[i].output_file) = file_types_to_find[i].file;
                }
                else
                {
                        // We don't care if we don't find physics file
                        if(file_types_to_find[i].type != _typecode_physics)
                                FoundAllRequired = false;
                }
        }

        if(!FoundAllRequired)
        {
                // This will also exit the application.
                alert_user(fatalError, strERRORS, badExtraFileLocations, fnfErr);
        }
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
	
	/* Add in the overhead thumbnail. */
	add_overhead_thumbnail(File);
	
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
	FileSpecifier &File)
{
	PicHandle picture;
	PicHandle preview;
	RgnHandle clip_region;
	FontInfo info;
	short text_x, text_y;
	short text_length;
	Str255 temporary;
	GWorldPtr old_gworld;
	GDHandle old_device;
	struct overhead_map_data overhead_data;
	Rect bounds;
	AEDesc aeFileSpec;
	FSSpec *SpecPtr;
	
	// Skip all this if there's no nav services to install the preview
	if(!machine_has_nav_services() || NavLibraryVersion() < kNavServicesVersion_2_0)
		return;
	
	GetGWorld(&old_gworld, &old_device);
	SetGWorld(world_pixels, (GDHandle) NULL);

	// Note well. We're using world_pixels to create our thumbnail pict within.
	// If world_pixels is runing as a postage stamp (low-res + small display space)
	// Then it is actually smaller than the size we're looking to build a thumbnail
	// within. But seeing as we're generating pict images and using drawing commands
	// instead of bit-wise operations. It all works out.

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
	
	// JTP: Add Nav Services style preview
	SetRect(&bounds, 0, 0, PREVIEW_WIDTH, PREVIEW_HEIGHT);
	preview= OpenPicture(&bounds);
	
	SetRect(&bounds, PREVIEW_IMAGE_X, PREVIEW_IMAGE_Y,
		THUMBNAIL_WIDTH + PREVIEW_IMAGE_X, THUMBNAIL_HEIGHT + PREVIEW_IMAGE_Y);
	clip_region= NewRgn();
	GetClip(clip_region);
	ClipRect(&bounds);
	DrawPicture(picture, &bounds);
	SetClip(clip_region);

	/* Center the text in the rectangle */
	// LP: Classic doesn't have this function
#ifdef TARGET_API_MAC_CARBON
	CopyCStringToPascal(static_world->level_name, temporary);
#else
	strncpy((char *)temporary,static_world->level_name,LEVEL_NAME_LENGTH);
	c2pstr((char *)temporary);
#endif
	// LP: fix to allow lengths more than 127 bytes (not really necessary, but...)
	text_length = *ptemporary;
	TruncText(PREVIEW_WIDTH, (char *)temporary+1, &text_length, smTruncEnd);
	*ptemporary = text_length;

	GetFontInfo(&info);
	text_y= PREVIEW_HEIGHT - info.descent;
	text_x= PREVIEW_LABEL_X + (PREVIEW_WIDTH-StringWidth(temporary))/2;
	MoveTo(text_x, text_y);
	DrawString(temporary);

	ClosePicture();
	
	// This requires NavServices 2.0, what's the inline check?
	// From FSS get a AEDesc
	OSStatus err;
	SpecPtr = &File.GetSpec();
	err = AECreateDesc(typeFSS, SpecPtr, sizeof(FSSpec), &aeFileSpec);

	HLock((Handle)preview);
	err = NavCreatePreview(&aeFileSpec, 'PICT', *preview, GetHandleSize((Handle)preview));
	HUnlock((Handle)preview);

	AEDisposeDesc(&aeFileSpec);
	KillPicture(preview);	
	KillPicture(picture);
	DisposeRgn(clip_region);

	SetGWorld(old_gworld, old_device);
}
