/*
	preprocess_map_mac.c
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

/* Local globals */
static Str255 save_level_name;
static SFReply load_file_reply;
static PicHandle overhead_pict;
static boolean overhead_pict_valid= FALSE;
static FSSpec previous_selection; /* FSSpec for convenience-> never used as such */

/* ---- local prototypes */
static void enumerate_catalog(long dir_id);
static void add_overhead_thumbnail(void);
pascal short custom_get_hook(short item, DialogPtr theDialog);
static pascal Boolean custom_get_filter_proc(DialogPtr theDialog, 
	EventRecord *theEvent, short *itemHit);
pascal short custom_put_hook(short item, DialogPtr theDialog, void *user_data);
static boolean confirm_save_choice(FSSpec *file);


/* --------- code begins */
void get_default_map_spec(
	FileDesc *_new)
{
	static boolean first_try= TRUE;
	static FSSpec default_map_spec;

	if (first_try)
	{
		OSErr error;

		/* Get the Marathon FSSpec */
		error= get_file_spec(&default_map_spec, strFILENAMES, filenameDEFAULT_MAP, strPATHS);
		if (error) alert_user(fatalError, strERRORS, badExtraFileLocations, error);
		
		first_try= FALSE;
	}
	
	/* Copy it in. */
	memcpy(_new, &default_map_spec, sizeof(FileDesc));
	
	return;
}

void get_default_physics_spec(
	FileDesc *_new)
{
	static boolean first_try= TRUE;
	static FSSpec default_physics_spec;

	if (first_try)
	{
		OSErr error;

		/* Get the Marathon FSSpec */
		error= get_file_spec(&default_physics_spec, strFILENAMES, filenamePHYSICS_MODEL, strPATHS);
		if(error)
		{
			get_my_fsspec(&default_physics_spec);
			getpstr(default_physics_spec.name, strFILENAMES, filenamePHYSICS_MODEL);
		}
		
		first_try= FALSE;
	}
	
	/* Copy it in. */
	memcpy(_new, &default_physics_spec, sizeof(FileDesc));
	
	return;
}

extern "C" {
extern boolean choose_saved_game_to_load(FSSpec *saved_game);
}

/* Note this should show the map of where you are... */
boolean choose_saved_game_to_load(
	FSSpec *saved_game)
{
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
}

boolean save_game(
	void)
{
	Str255 prompt;
	Str63 file_name;	
	DlgHookYDUPP dlgHook;
	StandardFileReply reply;
	Point top_left= {-1, -1}; /* auto center */
	boolean success= FALSE;

	pause_game();
	ShowCursor();
	
	/* Translate the name, and display the dialog */
	get_current_saved_game_name(file_name);
	
	/* Create the UPP's */
	dlgHook= NewDlgHookYDProc(custom_put_hook);
	assert(dlgHook);

	/* The drawback of this method-> I don't get a New Folder button. */
	/* If this is terribly annoying, I will add the Sys7 only code. */
	CustomPutFile(getpstr(prompt, strPROMPTS, _save_game_prompt), 
		file_name, &reply, 0, top_left, dlgHook, NULL, NULL, NULL, &reply.sfFile);

	/* Free them... */
	DisposeRoutineDescriptor((UniversalProcPtr) dlgHook);

	if(reply.sfGood)
	{
		/* And save it. */
		if (save_game_file((FileDesc *) &reply.sfFile))
		{
			success= TRUE;
		}
	}

	HideCursor();
	resume_game();
	
	return success;
}

void add_finishing_touches_to_save_file(
	FileDesc *file)
{
	short resource_file_ref;
	unsigned char name[64+1];
	
	/* Save the STR resource that tells us what our application name is. */
	resource_file_ref= FSpOpenResFile((FSSpec *) file, fsWrPerm);
	if(resource_file_ref>= 0)
	{
		Handle resource;
		OSErr err;

		/* Add in the save level name */
		strcpy((char *)name, static_world->level_name);
		c2pstr((char *)name);
		err= PtrToHand(name, &resource, name[0]+1);
		assert(!err);
		
		AddResource(resource, 'STR ', strSAVE_LEVEL_NAME, "\p");
		ReleaseResource(resource);

		/* Add in the overhead thumbnail. */
		add_overhead_thumbnail();

		CloseResFile(resource_file_ref);
	}

	/* Add the application name resource.. */
	getpstr(name, strFILENAMES, filenameMARATHON_NAME);
	add_application_name_to_fsspec((FileDesc *) file, name);
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

/* load_file_reply is valid.. */
pascal short custom_put_hook(
	short item, 
	DialogPtr theDialog,
	void *user_data)
{
	FSSpec *selected_file= (FSSpec *) user_data;

	global_idle_proc();

	if(GetWRefCon((WindowPtr) theDialog)==sfMainDialogRefCon)
	{
		if(item==sfItemOpenButton)
		{
			if(!confirm_save_choice(selected_file))
			{
				item= sfHookNullEvent;
			}
		}
	}
	
	return item;
}

#define REPLACE_H_OFFSET 52
#define REPLACE_V_OFFSET 52

/* Doesn't actually use it as an FSSpec, just easier */
/* Return TRUE if we want to pass it through, otherwise return FALSE. */
static boolean confirm_save_choice(
	FSSpec *file)
{
	OSErr err;
	HFileParam pb;
	boolean pass_through= TRUE;
	DialogPtr dialog;
	short item_hit;
	Rect frame;

	/* Clear! */
	memset(&pb, 0, sizeof(HFileParam));
	pb.ioNamePtr= file->name;
	pb.ioVRefNum= file->vRefNum;
	pb.ioDirID= file->parID;
	err= PBHGetFInfo((HParmBlkPtr) &pb, FALSE); 

	if(!err)
	{
		/* IF we aren't a folder.. */
		if(!(pb.ioFlAttrib & kFolderBit))
		{
			if(pb.ioFlFndrInfo.fdType==SAVE_GAME_TYPE)
			{
				/* Get the default dialog's frame.. */
				get_window_frame(FrontWindow(), &frame);

				/* Slam the ParamText */
				ParamText(file->name, "\p", "\p", "\p");

				/* Load in the dialog.. */
				dialog= myGetNewDialog(dlogMY_REPLACE, NULL, (WindowPtr) -1, 0);
				assert(dialog);
				
				/* Move the window to the proper location.. */
				MoveWindow((WindowPtr) dialog, frame.left+REPLACE_H_OFFSET, 
					frame.top+REPLACE_V_OFFSET, FALSE);

				/* Show the window. */
				ShowWindow((WindowPtr) dialog);			
				do {
					ModalDialog(get_general_filter_upp(), &item_hit);
				} while(item_hit > iCANCEL);

				/* Restore and cleanup.. */				
				ParamText("\p", "\p", "\p", "\p");
				DisposeDialog(dialog);
				
				if(item_hit==iOK) /* replace.. */
				{
					/* Want to delete it... */
					err= FSpDelete(file);
					/* Pass it on through.. they won't bring up the replace now. */
				} else {
					/* They cancelled.. */
					pass_through= FALSE;
				}
			}
		}
	}
	
	return pass_through;
}

#ifdef OBSOLETE
static pascal short custom_get_hook(
	short item, 
	DialogPtr theDialog)
{
	short file_ref;
	short itemType;
	Handle item_handle, resource;
	Rect bounds;
	GrafPtr old_port;
	long parID;
	OSErr err;

	global_idle_proc();

	parID= LMGetCurDirStore();

	/* IF the selection has changed... */	
	if(previous_selection.vRefNum != load_file_reply.vRefNum 
		|| previous_selection.parID != parID 
		|| previous_selection.name[0]!=load_file_reply.fName[0]
		|| memcmp(previous_selection.name, load_file_reply.fName, load_file_reply.fName[0]+1))
	{
		/* .. save the new current one. .. */
		previous_selection.vRefNum= load_file_reply.vRefNum;
		previous_selection.parID= parID;
		BlockMove(load_file_reply.fName, previous_selection.name, load_file_reply.fName[0]+1);

		/* Pessimism.. */
		overhead_pict_valid= FALSE;
		save_level_name[0]= 0;
		
		/* ... check the file type.. */	
		if(load_file_reply.fType==SAVE_GAME_TYPE)
		{
			/* .. free memory if necessary... */
			if(overhead_pict)
			{
				/* Free it.. */
				DisposeHandle((Handle) overhead_pict);
				overhead_pict= (PicHandle) NULL;
			}

			/* ... read in the thumbnail... */
			file_ref= HOpenResFile(load_file_reply.vRefNum, parID, load_file_reply.fName, fsRdPerm);
			if(file_ref!= -1)
			{
				resource= Get1Resource('PICT', THUMBNAIL_ID);
				if(resource)
				{
					/* Copy it.. */
					overhead_pict= (PicHandle) resource;
					err= HandToHand((Handle *) &overhead_pict);
					assert(!err);
					
					/* Release it. */
					ReleaseResource(resource);

					overhead_pict_valid= TRUE;
				} 
				
				resource= Get1Resource('STR ', strSAVE_LEVEL_NAME);
				if(resource)
				{
					BlockMove(*resource, save_level_name, (**resource)+1);
					ReleaseResource(resource);
				}

				CloseResFile(file_ref);
			}
		} 
		
		/* Inval the rectangle.. */
		GetDialogItem(theDialog, iTHUMBNAIL_RECT, &itemType, &item_handle, &bounds);
		
		/* Inval, and let the modal proc draw the picture.. */
		GetPort(&old_port);
		SetPort((GrafPtr) theDialog);
		InvalRect(&bounds);
		SetPort(old_port);
	}
	
	return item;
}

static pascal Boolean custom_get_filter_proc(
	DialogPtr theDialog, 
	EventRecord *theEvent, 
	short *itemHit)
{
	short itemType;
	Handle item;
	Rect bounds, frame, name_bounds;
	GrafPtr old_port;
	RgnHandle clip_region;
	FontInfo info;

	(void)(itemHit);

	if(theEvent->what==updateEvt)
	{
		if ((DialogPtr)theEvent->message==theDialog)
		{
			GetPort(&old_port);
			SetPort((GrafPtr) theDialog);
		
			GetDialogItem(theDialog, iTHUMBNAIL_RECT, &itemType, &item, &bounds);
			GetDialogItem(theDialog, iLEVEL_NAME, &itemType, &item, &name_bounds);

			frame= bounds;

			/* Draw a border around it. */
			InsetRect(&frame, -2, -2);
			FrameRect(&frame);
		
			if(overhead_pict_valid)
			{
				short text_x, text_y;
				short text_length;
			
				assert(overhead_pict);
				clip_region= NewRgn();
				GetClip(clip_region);
				ClipRect(&bounds);
				DrawPicture(overhead_pict, &bounds);
				SetClip(clip_region);

				/* Center the text in the rectangle */
				pstrcpy(temporary, save_level_name);
				text_length = *temporary;
				TruncText(RECTANGLE_WIDTH(&name_bounds), temporary+1, &text_length, smTruncEnd);
				*temporary = text_length;

				GetFontInfo(&info);
				text_y= name_bounds.bottom - info.descent;
				text_x= name_bounds.left + (RECTANGLE_WIDTH(&name_bounds)-StringWidth(temporary))/2;
				MoveTo(text_x, text_y);
				EraseRect(&name_bounds);
				DrawString(temporary);
			} else {
				EraseRect(&bounds);
				EraseRect(&name_bounds);
			}
	
			/* Return to the old port */
			SetPort(old_port);
		}
	}

	return FALSE;
}
#endif
