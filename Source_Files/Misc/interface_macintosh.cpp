/*
	interface_macintosh.c

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

	Thursday, September 28, 1995 7:32:09 PM- rdm created.

Feb. 4, 2000 (Loren Petrich):
	Changed halt() to assert(false) for better debugging

Mar 19, 2000 (Loren Petrich):
	Won't do screen reinit if OpenGL has been selected

Oct 12, 2000 (Loren Petrich):
	Moved Quicktime init to shell_macintosh.cpp

Nov 25, 2000 (Loren Petrich):
	Added support for movies played at the beginning of any level,
	at the request of Jesse Simko

Dec 23, 2000 (Loren Petrich):
	Added some code for reiniting the screen if in full-screen mode or going into or out of it;
	won't be active until the DrawSprocket works properly

Jan 12, 2001 (Loren Petrich):
	Added popup-menu level selector

Jan 25, 2002 (Br'fin (Jeremy Parsons)):
	Added TARGET_API_MAC_CARBON for Quicktime.h
	Added accessors for datafields now opaque in Carbon
	Disabled networking under Carbon
	Game menus added to the menu bar normally instead of as popups. (otherwise Carbon's app-menu
		was stealing our command-Q menu item)
	Included Steve Bytnar's Cursor Hiding

Feb 27, 2002 (Br'fin (Jeremy Parsons)):
	Uses generalized networking under Carbon instead of macintosh_network.h

Feb 13, 2003 (Woody Zenfell):
	Trivial implementation of should_restore_game_networked(), lets folks resume netgames until
	someone builds a proper dialog box or something.
*/

#include "macintosh_cseries.h"
#if defined(EXPLICIT_CARBON_HEADER)
    #include <quicktime/Quicktime.h>
#else
#include <Movies.h>
#include <FixMath.h>
#endif

#include <string.h>

#include "map.h"
#include "shell.h"
#include "interface.h"
#include "player.h"

#if defined(TARGET_API_MAC_CARBON) || HAVE_SDL_NET
#include "network.h"
#else
#include "macintosh_network.h"
#endif

#include "screen_drawing.h"
#include "mysound.h"
#include "preferences.h"
#include "fades.h"
#include "game_window.h"
#include "game_errors.h"
#include "screen.h"

// #include "portable_files.h"
#include "images.h"

#include "interface_menus.h"

#include "XML_LevelScript.h"
#include "music.h"

#ifdef env68k
	#pragma segment macintosh_
#endif

#ifdef USES_NIBS
const CFStringRef Window_Game_Goto_Level = CFSTR("Game_Goto_Level");
const short iLEVEL_SELECTOR = 3;
#else
enum { /* Cheat level dialog */
	dlogLEVEL_NUMBER= 200,
	iLEVEL_SELECTOR= 3
/*
	dlogLEVEL_NUMBER= 137,
	iLEVEL_NUMBER= 3
*/
};
#endif

#define TICKS_BETWEEN_XNEXTEVENT_CALLS 10
#define MAXIMUM_CONSECUTIVE_GETOSEVENT_CALLS 12 // 2 seconds

/* -------- local prototypes */
// static bool machine_has_quicktime(void);

/* -------- code */
void do_preferences(
	void)
{
	struct screen_mode_data mode= graphics_preferences->screen_mode;
	GDSpec old_spec= graphics_preferences->device_spec;
	
	handle_preferences();
	if (!EqualGDSpec(&graphics_preferences->device_spec, &old_spec) ||
		mode.bit_depth != graphics_preferences->screen_mode.bit_depth ||
		mode.fullscreen || graphics_preferences->screen_mode.fullscreen)
		// mode.acceleration != graphics_preferences->screen_mode.acceleration)
	{
		change_screen_mode(&graphics_preferences->screen_mode, false);
		paint_window_black();
		initialize_screen(&graphics_preferences->screen_mode,true);

		/* Re fade in, so that we get the proper colortable loaded.. */
		display_main_menu();
	}
	else if (memcmp(&mode, &graphics_preferences->screen_mode, sizeof(struct screen_mode_data)))
	{
		change_screen_mode(&graphics_preferences->screen_mode, false);
	}
}


const int32 AllPlayableLevels = _single_player_entry_point | _multiplayer_carnage_entry_point | _multiplayer_cooperative_entry_point;

#ifdef USES_NIBS

static bool LevelNumberMenuBuilder(
	int Indx, Str255 ItemName, bool &ThisIsInitial, void *BuildMenuData)
{
	vector<entry_point> *LvPtr = (vector<entry_point> *)(BuildMenuData);
	vector<entry_point> &Levels = *LvPtr;
	
	bool UseThis = (Indx <= Levels.size()); // One-based test
	if (UseThis)
	{
		entry_point &Level = Levels[Indx-1];	// One-based to zero-based
		psprintf(ItemName, "%d: %s",Level.level_number+1,Level.level_name);
	}
	
	return UseThis;
}

short get_level_number_from_user(
	void)
{	
	// Get the window
	AutoNibWindow Window(GUI_Nib,Window_Game_Goto_Level);
	
	// Set up the popup menu
	ControlRef MenuCtrl = GetCtrlFromWindow(Window(), 0, iLEVEL_SELECTOR);
	
	vector<entry_point> Levels;
	if (!get_entry_points(Levels, AllPlayableLevels)) {
		entry_point dummy;
		dummy.level_number = 0;
		strcpy(dummy.level_name, "Untitled Level");
		Levels.push_back(dummy);
	}
		
	BuildMenu(MenuCtrl, LevelNumberMenuBuilder, &Levels);
	
	// Should do noncontiguous map files OK
	short LevelNumber = RunModalDialog(Window(), false) ?
		Levels[(GetControl32BitValue(MenuCtrl) - 1)].level_number :
		NONE;
	
	return LevelNumber;
}
#else
short get_level_number_from_user(
	void)
{
	short index, item_hit, level_number, maximum_level_number;
	DialogPtr dialog;
	struct entry_point entry;
	bool done= false;
	
	// For popup-menu level selector;
	// cribbed from wad_prefs_macintosh.cpp and preferences_macintosh.cpp
	short item_type;
	Rect bounds;
	ControlHandle SelectorHdl;
/*
#if !defined(USE_CARBON_ACCESSORS)
	struct PopupPrivateData **privateHndl;
#endif
*/
	MenuHandle mHandle;
	
	index = 0; maximum_level_number= 0;
		
	dialog = myGetNewDialog(dlogLEVEL_NUMBER, NULL, (WindowPtr) -1, 0);
	assert(dialog);
	
	/* Setup the popup list.. */
	GetDialogItem(dialog, iLEVEL_SELECTOR, &item_type, 
		(Handle *) &SelectorHdl, &bounds);
	assert(SelectorHdl);
//#if defined(USE_CARBON_ACCESSORS)
	mHandle= GetControlPopupMenuHandle(SelectorHdl);
/*
#else
	privateHndl= (PopupPrivateData **) ((*SelectorHdl)->contrlData);
	mHandle= (*privateHndl)->mHandle;
#endif
*/	
	// Get rid of old contents
	while(CountMenuItems(mHandle)) DeleteMenuItem(mHandle, 1);
	
	// Add level names
	while (get_indexed_entry_point(&entry, &index, AllPlayableLevels))
	{
		psprintf(ptemporary, "%d: %s",entry.level_number+1,entry.level_name);
		AppendMenu(mHandle, "\pSome Level");
		maximum_level_number++;
		SetMenuItemText(mHandle, maximum_level_number, ptemporary);
	}
	
	/* Set our max value.. */
	SetControlMaximum(SelectorHdl, maximum_level_number);
	SetControlValue(SelectorHdl, 1);
	/*
	psprintf(ptemporary, "%d", maximum_level_number); 
	ParamText(ptemporary, "\p", "\p", "\p");
	SelectDialogItemText(dialog, iLEVEL_NUMBER, 0, SHRT_MAX);
	*/
	
	while(!done)
	{
		do
		{
			ModalDialog(get_general_filter_upp(), &item_hit);
		} while (item_hit>iCANCEL);

		// level_number= extract_number_from_text_item(dialog, iLEVEL_NUMBER);

		switch(item_hit)
		{
			case iOK:
				level_number = GetControlValue(SelectorHdl) - 1;
				done = true;
				/*
				if(level_number<=0 || level_number>maximum_level_number)
				{
					SelectDialogItemText(dialog, iLEVEL_NUMBER, 0, SHRT_MAX);
					SysBeep(-1);
				} else {
					level_number-= 1; *//* Make it zero based *//*
					done= true;
				}
				*/
				break;
				
			case iCANCEL:
				done= true;
				level_number= NONE;
				break;
				
			default:
				assert(false);
				break;
		}
	}
	DisposeDialog(dialog);

	return level_number;
}
#endif

void toggle_menus(
	bool game_started)
{
	MenuHandle menu_interface, menu_game;
	short item;
	static bool first_time= true;

	/* First time, we insert the menus... */	
	if(first_time)
	{
		MenuHandle menu;

#ifdef USES_NIBS

		for (int n=0; n<NumMenus; n++)
		{
			OSStatus err;
			err = CreateMenuFromNib(GUI_Nib,MenuNames[n],&menu);
			
			const int BufferSize = 192;	// Smaller than 256, the size of "temporary"
			char Buffer[BufferSize];
			if (err != noErr)
			{
				CFStringGetCString(MenuNames[n], Buffer, BufferSize, kCFStringEncodingMacRoman);
				Buffer[BufferSize-1] = 0; // Null terminator byte
			}
			vassert(err == noErr,
				csprintf(temporary,"CreateMenuFromNib error: %d for menu %s",err,Buffer));
			
			// Now that we have it, insert it
			InsertMenu(menu, 0);
		}
#else
		/* Insert our fake menu */
		menu= GetMenu(mFakeEmptyMenu);
		assert(menu);
		InsertMenu(menu, 0);

		menu= GetMenu(mInterface);
		assert(menu);
#if defined(TARGET_API_MAC_CARBON)
		// JTP: Carbon doesn't intercept CMD-Q if we don't use popups
		InsertMenu(menu, 0);
#else
		InsertMenu(menu, -1);
#endif

		menu= GetMenu(mGame);
		assert(menu);
#if defined(TARGET_API_MAC_CARBON)
		InsertMenu(menu, 0);
#else
		InsertMenu(menu, -1);
#endif
#endif

		first_time= false;
	}
	
	menu_interface= GetMenuHandle(mInterface);
	menu_game= GetMenuHandle(mGame);
	assert(menu_game && menu_interface);

	if(game_started)
	{
#if defined(TARGET_API_MAC_CARBON)
		DisableMenuItem(menu_interface, 0);
		EnableMenuItem(menu_game, 0);
#else
		DisableItem(menu_interface, 0);
		EnableItem(menu_game, 0);
#endif
	} else {
#if defined(TARGET_API_MAC_CARBON)
		DisableMenuItem(menu_game, 0);
		EnableMenuItem(menu_interface, 0);
#else
		DisableItem(menu_game, 0);
		EnableItem(menu_interface, 0);
#endif
	}

	/* Handle enabling/disabling everything.. */
	for(item= iNewGame; item<=iCenterButton; ++item)
	{
		if(enabled_item(item))
		{
#if defined(TARGET_API_MAC_CARBON)
			EnableMenuItem(menu_interface, item);
#else
			EnableItem(menu_interface, item);
#endif
		} else {
#if defined(TARGET_API_MAC_CARBON)
			DisableMenuItem(menu_interface, item);
#else
			DisableItem(menu_interface, item);
#endif
		}
	}
}

void update_game_window(
	WindowPtr window,
	EventRecord *event)
{
	switch(get_game_state())
	{
		case _game_in_progress:
			assert(event);
			update_screen_window(window, event);
			break;
			
		case _display_quit_screens:
		case _display_intro_screens_for_demo:
		case _display_intro_screens:
		case _display_chapter_heading:
		case _display_prologue:
		case _display_epilogue:
		case _display_credits:
		case _display_main_menu:
			update_interface_display();
			break;
			
		case _quit_game:
		case _close_game:
		case _switch_demo:
		case _change_level:
		case _revert_game:
		case _begin_display_of_epilogue:
		case _displaying_network_game_dialogs:
			break;
	
		default:
			assert(false);
			break;
	}
}
	
void process_game_key(
	EventRecord *event,
	short key)
{
	switch(get_game_state())
	{
		case _game_in_progress:
			if(event->modifiers&cmdKey && event->what==keyDown)
			{
				long menu_key= MenuKey(key);
				short menuID, menuItem;

				menuID= menu_key>>16;
				menuItem= menu_key & 0x0000ffff;
				if(menuID)
				{
					assert(menuID==mGame);

					do_menu_item_command(menuID, menuItem, 
						has_cheat_modifiers(event));
				} else {
					handle_game_key(event, key);
				}
			} else {
				handle_game_key(event, key);
			}
			break;
			
		case _display_intro_screens:
		case _display_chapter_heading:
		case _display_prologue:
		case _display_epilogue:
		case _display_credits:
		case _display_quit_screens:
			/* Force the state change next time through.. */
			if(interface_fade_finished())
			{
				force_game_state_change();
			} else {
				stop_interface_fade();
			}
			break;

		case _display_intro_screens_for_demo:
			/* Get out of user mode. */
			stop_interface_fade();
			display_main_menu();
			break;
			
		case _quit_game:
		case _close_game:
		case _revert_game:
		case _switch_demo:	
		case _change_level:
		case _begin_display_of_epilogue:
		case _displaying_network_game_dialogs:
			break;
	
		case _display_main_menu:
			if(event->modifiers&cmdKey && event->what==keyDown)
			{
				long menu_key= MenuKey(key);
				short menuID, menuItem;

				menuID= menu_key>>16;
				menuItem= menu_key & 0x0000ffff;

				/* If it was a menu key. */
				if(menuID) 
				{
					GrafPtr old_port;
				
					assert(menuID==mInterface);

					stop_interface_fade();

					/* Change to this port... */
					GetPort(&old_port);
					SetPort((GrafPtr)GetScreenGrafPort());
					draw_menu_button_for_command(menuItem);
					SetPort(old_port);
					
					do_menu_item_command(mInterface, menuItem, 
						has_cheat_modifiers(event));
				}
			} else {
				/* If they are fading in, stop the interface fade */
				if(!interface_fade_finished())
				{
					stop_interface_fade();
				}
			}
			break;
		
		default:
			assert(false);
			break;
	}
}	

bool try_for_event(
	bool *use_waitnext)
{
	bool try_for_event= false;
	static long last_xnextevent_call= 0;
	static short consecutive_getosevent_calls= 0;
	
	switch(get_game_state())
	{
		case _game_in_progress:
		case _change_level:
			if (TickCount()-last_xnextevent_call>=TICKS_BETWEEN_XNEXTEVENT_CALLS)
			{
				try_for_event= true;

//#if defined(USE_CARBON_ACCESSORS)
				RgnHandle updateRgn = NewRgn();
				GetWindowRegion(GetWindowFromPort(GetScreenGrafPort()), kWindowUpdateRgn, updateRgn);
				if (suppress_background_events() && 
					EmptyRgn(updateRgn))
/*
#else
				if (suppress_background_events() && 
					EmptyRgn(((WindowPeek)GetScreenGrafPort())->updateRgn)) 
#endif
*/
					// && consecutive_getosevent_calls<MAXIMUM_CONSECUTIVE_GETOSEVENT_CALLS)
				{
					*use_waitnext= false;
					consecutive_getosevent_calls+= 1;
				} else {
					*use_waitnext= true;
					consecutive_getosevent_calls= 0;
				}
//#if defined(USE_CARBON_ACCESSORS)
				DisposeRgn(updateRgn);
//#endif
		
				last_xnextevent_call= TickCount();
			}
			break;

		case _display_intro_screens:
		case _display_main_menu:
		case _display_chapter_heading:
		case _display_prologue:
		case _display_epilogue:
		case _begin_display_of_epilogue:
		case _display_credits:
		case _display_intro_screens_for_demo:
		case _display_quit_screens:
		case _displaying_network_game_dialogs:
			*use_waitnext= interface_fade_finished();
			try_for_event= true;
			break;
			
		case _quit_game:
		case _close_game:
		case _switch_demo:
		case _revert_game:
			*use_waitnext= true;
			try_for_event= true;
			break;
			
		default:
			vhalt(csprintf(temporary, "What the hell is state: %d?", get_game_state()));
			break;
	}

	if(try_for_event)
	{
		global_idle_proc();
	}
	
	return try_for_event;
}

void exit_networking(
	void)
{
	NetExit();
}

bool has_cheat_modifiers(
	EventRecord *event)
{
	bool cheat= false;

	if ((event->modifiers&cmdKey) && (event->modifiers&optionKey)
		&& !(event->modifiers&shiftKey) && !(event->modifiers&controlKey))
	{
		cheat= true;
	}
	
	return cheat;
}

#if defined(TARGET_API_MAC_CARBON)
static int cursor_showing = -1;
#endif

// Bytnar: Under Carbon, we can't Hide twice and Show once and expect the cursor to show.
/* ------------ these are all functions that are simple to write for various */
/* --- machines */
void hide_cursor(
	void)
{
#if defined(TARGET_API_MAC_CARBON)
	if (cursor_showing != false)
	{
		// JTP: This covers a glitch whereby the cursor would remain visible if you
		// suspended AlephOne, then resumed it by clicking on the icon in the dock.
		Rect screen_rect;
		GetPortBounds(GetScreenGrafPort(), &screen_rect);
		ShieldCursor(&screen_rect, (Point){screen_rect.top,screen_rect.left});
		cursor_showing = false;
	}
#else
	HideCursor();
#endif
}

void show_cursor(
	void)
{
#if defined(TARGET_API_MAC_CARBON)
	if (cursor_showing != true)
	{
		ShowCursor();
		cursor_showing = true;
	}
#else
	InitCursor(); // why worry about balancing?
#endif
}

bool mouse_still_down(
	void)
{
	return StillDown();
}

void get_mouse_position(
	short *x,
	short *y)
{
	Point p;
	
	GetMouse(&p);
	*x= p.h; *y= p.v;
}

void set_drawing_clip_rectangle(
	short top,
	short left,
	short bottom,
	short right)
{
	Rect rectangle;
	
	SetRect(&rectangle, left, top, right, bottom);
	ClipRect(&rectangle);
}

void show_movie(
	short index)
{
	// What scale factor for the movie?
	// Note: its default value is the original scaling of movie files
	float PlaybackSize = 2;

	// LP: can do this with any index
	// if(index==0) /* Only one valid.. */
	{
		if(machine_has_quicktime())
		{
			FSSpec movie_spec; // <===== fill in based on index!
			OSErr err= noErr;
			
			FileSpecifier *File = GetLevelMovie(PlaybackSize);
			if (File)
			{
				movie_spec = File->GetSpec();
			}
			else if (index == 0)
			{	
				err= get_file_spec(&movie_spec, strFILENAMES, filenameMOVIE, strPATHS);
			}
			else
				err = fnfErr;
			
			if(!err)
			{
				// Quicktime initialized in shell_macintosh.h
				err = noErr;
				// err= EnterMovies();
				if(!err)
				{
					Movie movie;
					short resRefNum;
		
					if(!OpenMovieFile(&movie_spec, &resRefNum, fsRdPerm)) 
					{
						if(!NewMovieFromFile(&movie, resRefNum, nil, nil, newMovieActive, nil )) 
						{
							Rect dispBounds;
							bool aborted= false;
							
							// Any better way of doing this? This simply causes
							// the upcoming sound to stutter
							/*
							// Don't want previous level's music playing...
							stop_music();
							
							// Busy-wait to clear QT's sound buffers;
							// wait 1/4 second
							int TargetTickCount = machine_tick_count() + 15;
							while(machine_tick_count() < TargetTickCount)
								global_idle_proc();
							*/
							
							/* Find movie bounds and zero base it.... */
							GetMovieBox(movie, &dispBounds);
							OffsetRect(&dispBounds, -dispBounds.left, -dispBounds.top);
							// Original scaling: 2
							dispBounds.right= short(PlaybackSize*RECTANGLE_WIDTH(&dispBounds) + 0.5);
							dispBounds.bottom= short(PlaybackSize*RECTANGLE_HEIGHT(&dispBounds) + 0.5);
							
							/* ‚enter... */
//#if defined(USE_CARBON_ACCESSORS)
							Rect screenBounds;
							GetPortBounds(GetScreenGrafPort(), &screenBounds);
							AdjustRect(&screenBounds, &dispBounds, &dispBounds, centerRect);
/*
#else
							AdjustRect(&GetScreenGrafPort()->portRect, &dispBounds, &dispBounds, centerRect);
#endif
*/
							OffsetRect(&dispBounds, dispBounds.left<0 ? -dispBounds.left : 0, dispBounds.top<0 ? -dispBounds.top : 0);
							SetMovieBox(movie, &dispBounds);
							
							/* Set to the screen_window.. */	
							SetMovieGWorld(movie, (CGrafPtr)  GetScreenGrafPort(), nil);
							
							// After setup, play the movie
							GoToBeginningOfMovie(movie); // Rewind
							
							/* Should the preroll movie use FIXED_ONE as second parameter */
							/*  (or whatever the mac equivalent (Fixed) is) */
							PrerollMovie(movie, (TimeValue) 0, (Fixed) fixed1); // Preload portions
//							SetMovieActive(movie, true); // Set movie active for servicing
							
							// Maybe I should delay after prerolling the movie, since the
							// preroll stuff might be asynchrounous?
							StartMovie(movie);
				
							while (!IsMovieDone(movie) && !aborted) 
							{
								EventRecord event;
							
								if(WaitNextEvent(keyDownMask|autoKeyMask|mDownMask, &event, 2, nil))
								{
									switch(event.what)
									{
										case nullEvent:
											break;
											
										case mouseDown:
										case keyDown:
										case autoKey:
											aborted= true;
											StopMovie(movie);
											break;
									}
								}
								global_idle_proc();
					
								/* Give the movie time */
								MoviesTask(movie, DoTheRightThing);
							}

							/* Paint the window black. */
							paint_window_black();
		
							// Dispose of storage, and return
							DisposeMovie(movie); // Movie
						}
		
						CloseMovieFile(resRefNum); // reference to movie file
					}
				}
			}
		}
	}
}

// "Has Quicktime" test moved to shell_macintosh.cpp


size_t should_restore_game_networked() {
	// We should return 1 for netgame, 0 for single-player game, and -1 (NONE) for "cancel".
	// Currently we just choose based on the number of players, should be right most of the time.
	return dynamic_world->player_count > 1 ? 1 : 0;
}
