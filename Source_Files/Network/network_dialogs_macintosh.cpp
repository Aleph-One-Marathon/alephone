/*
NETWORK_DIALOGS.C  (now network_dialogs_macintosh.cpp)

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

Monday, June 20, 1994 12:36:39 PM
Thursday, June 30, 1994 6:27:43 PM (ajr)
	Made some UPPs for the dialogs
Tuesday, July 19, 1994 7:16:54 PM (ajr)
	fixed up dialogs. added dialog for net game stats
Tuesday, September 6, 1994 3:50:01 PM (ajr)
	recently, the net game stats dialog has been rewritten to be a graph and some other
	stuff has been cleaned up a bit.

Jan 30, 2000 (Loren Petrich):
	Added some typecasts

Feb. 4, 2000 (Loren Petrich):
	Changed halt() to assert(false) for better debugging

Apr 30, 2000 (Loren Petrich):
	Did change for getting default player name from outside

Jul 1, 2000 (Loren Petrich):
	Added Benad's netgame stuff
        
Oct 15, 2001 (Woody Zenfell):
        Renamed from network_dialogs.cpp to network_dialogs_macintosh.cpp, to make
        intended platform more obvious and _also_ to allow for the creation of a new
        network_dialogs.cpp that will have shared code.
        Moved some elements to network_dialogs.cpp for sharing.  Exposed some routines
        (via network_dialogs.h) that the shared routines depend on, but which will have
        separate implementations (a link-time version of "template method"?).
*/

// add a taunt window for postgame.
// add a message window for gathering
// Don't allow gather on map with no entry points (marathon bites it)
// _overhead_map_is_omniscient is now _burn_items_on_death

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "macintosh_cseries.h"
#if HAVE_SDL_NET
#include "sdl_network.h"
#include "network_lookup_sdl.h"
#else
#include "macintosh_network.h" // solely for ADSP.h, AppleTalk.h, and network_lookup functions.
#endif

#include "shell.h"  // for preferences
#include "map.h"    // so i can include player.h
#include "player.h" // for displaying damage statistics after network game.
#include "preferences.h"
#include "interface.h" // for _multiplayer_carnage_entry_point
#include "screen_drawing.h"

#include "network_games.h"
//#include "network_stream.h"

// ZZZ: shared dialog item ID constants
#include "network_dialogs.h"

// LP change: outside handler for the default player name
#include "PlayerName.h"

//#define TEST_NET_STATS_DIALOG  // for testing the dialog when i don't want to play a net game

#ifdef TEST_NET_STATS_DIALOG
//#define TEST_TEAM_DISPLAY_FOR_DIALOG
#endif

#ifdef env68k
#pragma segment network_dialogs
#endif

// #define USE_MODEM

struct network_speeds
{
	short updates_per_packet;
	short update_latency;
};

/* ---------- constants */

// LP change: get player name from outside
#define PLAYER_TYPE GetPlayerName()

#define MONSTER_TEAM                8

#define fontTOP_LEVEL_FONT        130
#define menuZONES                1002

// ZZZ: Dialog and string constants moved to network_dialogs.h for sharing between SDL and Mac versions.

#define NAME_BOX_HEIGHT   28
#define NAME_BOX_WIDTH   114

#define BOX_SPACING  8

#define OPTION_KEYCODE             0x3a

/* ---------- globals */

static int accepted_into_game;
static ListHandle network_list_box= (ListHandle) NULL;

// these speeds correspond to what's in the popup menu
static struct network_speeds net_speeds[] =
{
	{3, 2},  // Appletalk Remote
	{2, 1},  // LocalTalk
	{1, 0},  // TokenTalk
	{1, 0},  // Ethernet
	{2, 1}	 // Modem stats
};

/* from screen_drawing.c */
extern TextSpec *_get_font_spec(short font_index);

/* ---------- private code */
static bool network_game_setup(player_info *player_information, game_info *game_information);
/* static */ short fill_in_game_setup_dialog(DialogPtr dialog, player_info *player_information, bool allow_all_levels);
/* static */ void extract_setup_dialog_information(DialogPtr dialog, player_info *player_information, 
	game_info *game_information, short game_limit_type, bool allow_all_levels);
bool check_setup_information(DialogPtr dialog, short game_limit_type);

// ZZZ: moved to csdialogs
//static short get_dialog_control_value(DialogPtr dialog, short which_control);

static pascal Boolean gather_dialog_filter_proc(DialogPtr dialog, EventRecord *event, short *item_hit);
static pascal Boolean join_dialog_filter_proc(DialogPtr dialog, EventRecord *event, short *item_hit);
static pascal Boolean game_setup_filter_proc(DialogPtr dialog, EventRecord *event, short *item_hit);

static void setup_network_list_box(WindowPtr window, Rect *frame, unsigned char *zone);
static void dispose_network_list_box(void);
static void network_list_box_update_proc(short message, short index);
static void update_player_list_item(DialogPtr dialog, short item_num);

// ZZZ: moved to network.cpp (network.h) so we can share
//static void reassign_player_colors(short player_index, short num_players);

#define NAME_BEVEL_SIZE    4
static void draw_beveled_text_box(bool inset, Rect *box, short bevel_size, RGBColor *brightest_color, char *text,short flags, bool name_box);

// ZZZ: moved a few static functions to network_dialogs.h so we can share

static MenuHandle get_popup_menu_handle(DialogPtr dialog, short item);
/* static */ void setup_dialog_for_game_type(DialogPtr dialog, short game_type);
static void draw_player_box_with_team(Rect *rectangle, short player_index);

static void setup_network_speed_for_join(DialogPtr dialog);
static void setup_network_speed_for_gather(DialogPtr dialog);
/* static */ void setup_for_untimed_game(DialogPtr dialog);
/* static */ void setup_for_timed_game(DialogPtr dialog);
static short get_game_duration_radio(DialogPtr dialog);

static bool key_is_down(short key_code);

/* ---------- code */

extern void NetUpdateTopology(void);

/*************************************************************************************************
 *
 * Function: network_gather
 * Purpose:  do the game setup and gather dialogs
 *
 *************************************************************************************************/
bool network_gather(
	void)
{
	bool successful= false;
	game_info myGameInfo;
	player_info myPlayerInfo;

	if (network_game_setup(&myPlayerInfo, &myGameInfo))
	{
		myPlayerInfo.desired_color= myPlayerInfo.color;
		memcpy(myPlayerInfo.long_serial_number, serial_preferences->long_serial_number, 10);
	
		if(NetEnter())
		{
			DialogPtr dialog;
			MenuHandle zones_menu;
			Rect item_rectangle;
			Handle item_handle;
			bool internet= false;
			ModalFilterUPP gather_dialog_upp;
			short current_zone_index, item_type;
			OSErr error;
			Cell cell;
			short item_hit;

			zones_menu= GetMenuHandle(menuZONES);
			if (!zones_menu)
			{
				zones_menu= GetMenu(menuZONES);
				assert(zones_menu);
				InsertMenu(zones_menu, -1);
			}
			
			// error= NetGetZonePopupMenu(zones_menu, &current_zone_index);
			error = noErr;
			if (error==noErr) internet= true;
			
			dialog= myGetNewDialog(dlogGATHER, NULL, (WindowPtr) -1, refNETWORK_GATHER_DIALOG);
			assert(dialog);
			gather_dialog_upp= NewModalFilterProc(gather_dialog_filter_proc);
			assert(gather_dialog_upp);
		
			GetDialogItem(dialog, iZONES_MENU, &item_type, &item_handle, &item_rectangle);
			SetControlValue((ControlHandle) item_handle, current_zone_index);
			GetDialogItem(dialog, iNETWORK_LIST_BOX, &item_type, &item_handle, &item_rectangle);
			setup_network_list_box(dialog, &item_rectangle, "\p*");
			
			// we'll show either the zone list, or a text item "Players in Network:"
			HideDialogItem(dialog, internet ? iPLAYER_LIST_TEXT : iZONES_MENU);
			
			ShowWindow(dialog);
					
			if(NetGather(&myGameInfo, sizeof(game_info), (void*) &myPlayerInfo, 
				sizeof(myPlayerInfo)))
			{
				do
				{
					short number_of_players= NetGetNumberOfPlayers();
					
					/* set button states */
					SetPt(&cell, 0, 0);
					modify_control(dialog, iADD, (number_of_players<MAXIMUM_NUMBER_OF_NETWORK_PLAYERS && LGetSelect(true, &cell, network_list_box)) ? CONTROL_ACTIVE : CONTROL_INACTIVE, 0);
					modify_control(dialog, iOK, number_of_players>1 ? CONTROL_ACTIVE : CONTROL_INACTIVE, 0);
					
					ModalDialog(gather_dialog_upp, &item_hit);
			
					switch (item_hit)
					{
						case iADD:
							SetPt(&cell, 0, 0);
							if (LGetSelect(true, &cell, network_list_box)) /* if no selection, we goofed */
							{
								/*
								if (NetGatherPlayer(cell.v, reassign_player_colors))
								{
									update_player_list_item(dialog, iPLAYER_DISPLAY_AREA);
								}
								*/
							}
							break;
							
						case iZONES_MENU:
							GetDialogItem(dialog, iZONES_MENU, &item_type, &item_handle, &item_rectangle);
							CheckItem(zones_menu, current_zone_index, false);
							current_zone_index= GetControlValue((ControlHandle) item_handle);
							CheckItem(zones_menu, current_zone_index, true);
							GetMenuItemText(zones_menu, current_zone_index, ptemporary);
							GetDialogItem(dialog, iNETWORK_LIST_BOX, &item_type, &item_handle, &item_rectangle);
							setup_network_list_box(dialog, &item_rectangle, ptemporary);
							break;
					}
				} while(item_hit!=iCANCEL && item_hit!=iOK);
			} else {
				/* Failed on NetGather */
				item_hit=iCANCEL;
			}

			dispose_network_list_box();
		
			DisposeRoutineDescriptor(gather_dialog_upp);
			DisposeDialog(dialog);
		
			if (item_hit==iOK)
			{
				successful= NetStart();
			}
			else
			{
				NetCancelGather();
				NetExit();
			}
		} else {
			/* error correction handled in the network code now.. */
		}
	}

	return successful;
}

/*************************************************************************************************
 *
 * Function: network_join
 * Purpose:  do the dialog to join a network game.
 *
 *************************************************************************************************/
bool network_join(
	void)
{
	bool successful= false;

	/* If we can enter the network... */
	if(NetEnter())
	{
		short item_hit, item_type;
#ifdef USE_MODEM
		short transport_type;
#endif
		GrafPtr old_port;
		bool did_join = false;
		DialogPtr dialog;
		player_info myPlayerInfo;
		game_info *myGameInfo;
		ModalFilterUPP join_dialog_upp;
		Rect item_rect;
		Handle item_handle;
		short name_length;
		
		dialog= myGetNewDialog(dlogJOIN, NULL, (WindowPtr) -1, refNETWORK_JOIN_DIALOG);
		assert(dialog);
		join_dialog_upp = NewModalFilterProc(join_dialog_filter_proc);
		assert(join_dialog_upp);
	
		name_length= player_preferences->name[0];
		if(name_length>MAX_NET_PLAYER_NAME_LENGTH) name_length= MAX_NET_PLAYER_NAME_LENGTH;
		memcpy(myPlayerInfo.name, player_preferences->name, name_length+1);

		GetDialogItem(dialog, iJOIN_NAME, &item_type, &item_handle, &item_rect);
		SetDialogItemText(item_handle, myPlayerInfo.name);
		SelectDialogItemText(dialog, iJOIN_NAME, 0, INT16_MAX);
		modify_control(dialog, iJOIN_TEAM, CONTROL_ACTIVE, player_preferences->team+1);
		modify_control(dialog, iJOIN_COLOR, CONTROL_ACTIVE, player_preferences->color+1);
		if (myPlayerInfo.name[0] == 0) modify_control(dialog, iOK, CONTROL_INACTIVE, NONE);
	
		GetDialogItem(dialog, iJOIN_MESSAGES, &item_type, &item_handle, &item_rect);
		SetDialogItemText(item_handle, getpstr(ptemporary, strJOIN_DIALOG_MESSAGES, _join_dialog_welcome_string));

#ifdef USE_MODEM	
		/* Adjust the transport layer using what's available */
		setup_network_speed_for_join(dialog);
#endif
	
		accepted_into_game = false;

		GetPort(&old_port);
		SetPort(dialog);
		ShowWindow(dialog);
	
		do
		{
			ModalDialog(join_dialog_upp, &item_hit);
			switch(item_hit)
			{
				case iJOIN:
					SetPort(dialog);
					GetDialogItem(dialog, iJOIN_NAME, &item_type, &item_handle, &item_rect);
					GetDialogItemText(item_handle, ptemporary);
					if (*temporary > MAX_NET_PLAYER_NAME_LENGTH) *temporary = MAX_NET_PLAYER_NAME_LENGTH;
					pstrcpy(myPlayerInfo.name, ptemporary);
					GetDialogItem(dialog, iJOIN_TEAM, &item_type, &item_handle, &item_rect);
					myPlayerInfo.team= GetControlValue((ControlHandle) item_handle) - 1;
					GetDialogItem(dialog, iJOIN_COLOR, &item_type, &item_handle, &item_rect);
					myPlayerInfo.color= GetControlValue((ControlHandle) item_handle) - 1;
					myPlayerInfo.desired_color= myPlayerInfo.color;
					memcpy(myPlayerInfo.long_serial_number, serial_preferences->long_serial_number, 10);
				
					SetCursor(*GetCursor(watchCursor));
					// did_join= NetGameJoin(myPlayerInfo.name, PLAYER_TYPE, (void *) &myPlayerInfo, sizeof(myPlayerInfo), 
					//	get_network_version());
					did_join = false;
					
					SetCursor(&qd.arrow);
					if(did_join)
					{
						SelectDialogItemText(dialog, iJOIN_NAME, 0, 0);
						GetDialogItem(dialog, iJOIN_NAME, &item_type, &item_handle, &item_rect);
						SetDialogItem(dialog, iJOIN_NAME, statText, item_handle, &item_rect);
						((DialogPeek)(dialog))->editField = -1;
						InsetRect(&item_rect, -4, -4); EraseRect(&item_rect); InvalRect(&item_rect); // force it to be updated
						modify_control(dialog, iJOIN_TEAM, CONTROL_INACTIVE, NONE);
						modify_control(dialog, iJOIN_COLOR, CONTROL_INACTIVE, NONE);
						modify_control(dialog, iJOIN, CONTROL_INACTIVE, NONE);
#ifdef USE_MODEM
						modify_control(dialog, iJOIN_NETWORK_TYPE, CONTROL_INACTIVE, NONE);
#endif
	
						// update preferences for user (Eat Gaseous Worms!)
						pstrcpy(player_preferences->name, myPlayerInfo.name);
						player_preferences->team = myPlayerInfo.team;
						player_preferences->color = myPlayerInfo.color;
						write_preferences();
	
						GetDialogItem(dialog, iJOIN_MESSAGES, &item_type, &item_handle, &item_rect);
						SetDialogItemText(item_handle, getpstr(ptemporary, strJOIN_DIALOG_MESSAGES, _join_dialog_waiting_string));
					} else {
						/* If you fail in joining the game, print the error and return */
						/*  to the main menu (this is primarily for modem) */
						item_hit= iCANCEL;
					}
					break;
					
				case iCANCEL:
					break;
					
				case iJOIN_TEAM:
					break;
	
				case iJOIN_NETWORK_TYPE:
#ifdef USE_MODEM
					GetDialogItem(dialog, iJOIN_NETWORK_TYPE, &item_type, &item_handle, &item_rect);
					transport_type= GetControlValue((ControlHandle) item_handle);
					NetSetTransportType(transport_type-1);
#endif
					break;
					
				default:
					break;
			}
		}
		while (!accepted_into_game && item_hit != iCANCEL);
	
		SetPort(old_port);
	
		DisposeRoutineDescriptor(join_dialog_upp);
		DisposeDialog(dialog);
	
		if (accepted_into_game)
		{
			successful= true;
			myGameInfo= (game_info *)NetGetGameData();
			NetSetInitialParameters(myGameInfo->initial_updates_per_packet, myGameInfo->initial_update_latency);
		}
		else
		{
			if (did_join)
			{
				NetCancelJoin();
			}
			
			NetExit();
		}
	}
	
	return successful;
}

/* ---------- private code */

/*************************************************************************************************
 *
 * Function: network_game_setup
 * Purpose:  handle the dialog to setup a network game.
 *
 *************************************************************************************************/
bool network_game_setup(
	player_info *player_information,
	game_info *game_information)
{
	short item_hit;
	GrafPtr old_port;
	bool successful = false, information_is_acceptable;
	DialogPtr dialog;
	ModalFilterUPP game_setup_filter_upp;
	bool allow_all_levels= key_is_down(OPTION_KEYCODE);

	dialog= myGetNewDialog(dlogGAME_SETUP, NULL, (WindowPtr) -1, refNETWORK_SETUP_DIALOG);
	assert(dialog);
	game_setup_filter_upp= NewModalFilterProc(game_setup_filter_proc);
	assert(game_setup_filter_upp);
	GetPort(&old_port);
	SetPort(dialog);

	game_information->net_game_type= fill_in_game_setup_dialog(dialog, player_information, allow_all_levels);

	ShowWindow(dialog);

	do
	{
		do
		{
			ModalDialog(game_setup_filter_upp, &item_hit);
			
			SetPort(dialog);
			
			switch (item_hit)
			{
				case iRADIO_NO_TIME_LIMIT:
					setup_for_untimed_game(dialog);
					break;
					
				case iRADIO_TIME_LIMIT:
					setup_for_timed_game(dialog);
					break;
					
				case iRADIO_KILL_LIMIT:
                                /* // ZZZ: changed to use function like the others
					// START Benad
					if (get_dialog_control_value(dialog, iGAME_TYPE)-1 != _game_of_defense)
					{
						HideDialogItem(dialog, iTIME_LIMIT); HideDialogItem(dialog, iTEXT_TIME_LIMIT);
						ShowDialogItem(dialog, iTEXT_KILL_LIMIT); ShowDialogItem(dialog, iKILL_LIMIT);
					}
					else
					{
						ShowDialogItem(dialog, iTIME_LIMIT); ShowDialogItem(dialog, iTEXT_TIME_LIMIT);
						ShowDialogItem(dialog, iTEXT_KILL_LIMIT); ShowDialogItem(dialog, iKILL_LIMIT);
					}
					// END Benad
					
					modify_radio_button_family(dialog, iRADIO_NO_TIME_LIMIT, iRADIO_KILL_LIMIT, iRADIO_KILL_LIMIT);
                                */
                                        setup_for_score_limited_game(dialog);
                                	break;

				case iFORCE_UNIQUE_TEAMS:
					modify_control(dialog, item_hit, NONE, !get_dialog_control_value(dialog, item_hit));
					if(!get_dialog_control_value(dialog, item_hit))
					{
						modify_control(dialog, iGATHER_TEAM, CONTROL_INACTIVE, NONE);
					} else {
						modify_control(dialog, iGATHER_TEAM, CONTROL_ACTIVE, NONE);
					}
					break;

				case iUNLIMITED_MONSTERS:
				case iMOTION_SENSOR_DISABLED:
				case iDYING_PUNISHED:
				case iSUICIDE_PUNISHED:
				case iBURN_ITEMS_ON_DEATH:
				case iREALTIME_NET_STATS:
					modify_control(dialog, item_hit, NONE, !get_dialog_control_value(dialog, item_hit));
					break;

				case iGAME_TYPE:
					{
						short new_game_type;
						
						new_game_type= get_dialog_control_value(dialog, iGAME_TYPE)-1;
						
						if(new_game_type != game_information->net_game_type)
						{
							long entry_flags, old_entry_flags;
							struct entry_point entry;

							if(allow_all_levels)
							{
								entry_flags= old_entry_flags= NONE;
							} else {
								old_entry_flags= get_entry_point_flags_for_game_type(game_information->net_game_type);
								entry_flags= get_entry_point_flags_for_game_type(new_game_type);
							}

							menu_index_to_level_entry(get_dialog_control_value(dialog, iENTRY_MENU), old_entry_flags, &entry);
							
							/* Get the old one and reset.. */
							fill_in_entry_points(dialog, iENTRY_MENU, entry_flags, entry.level_number);
							game_information->net_game_type= new_game_type;

							setup_dialog_for_game_type(dialog, new_game_type);
						}
					}
					break;

				case iREAL_TIME_SOUND:
					modify_control(dialog, iREAL_TIME_SOUND, NONE, !get_dialog_control_value(dialog, iREAL_TIME_SOUND));
					break;
			}
		} while (item_hit != iOK && item_hit != iCANCEL);
		
		if (item_hit==iCANCEL)
		{
			information_is_acceptable= true;
		}
		else
		{
			// START Benad
			if (get_dialog_control_value(dialog, iGAME_TYPE)-1 == _game_of_defense)
			{
				information_is_acceptable = (check_setup_information(dialog, iRADIO_TIME_LIMIT) &&
					check_setup_information(dialog, iRADIO_KILL_LIMIT));
			}
			else
			{
				short game_limit_type= get_game_duration_radio(dialog);
			
				information_is_acceptable= check_setup_information(dialog, game_limit_type);
			}
			// END Benad
		}
	} while (!information_is_acceptable);	

	if (item_hit == iOK)
	{
		short game_limit_type= get_game_duration_radio(dialog);
			
		extract_setup_dialog_information(dialog, player_information, game_information, 
			game_limit_type, allow_all_levels);
	}
	
	SetPort(old_port);
	DisposeRoutineDescriptor(game_setup_filter_upp);
	DisposeDialog(dialog);	

	return (item_hit==iOK);
}

/*************************************************************************************************
 *
 * Function: fill_in_game_setup_dialog
 * Purpose:  setup the majority of the game setup dialog.
 *
 *************************************************************************************************/
// ZZZ: moved this function to shared network_dialogs.cpp


// ZZZ: new function (has different implementation on SDL)
void set_limit_type(DialogPtr dialog, short limit_type) {
    modify_radio_button_family(dialog, iRADIO_NO_TIME_LIMIT, iRADIO_KILL_LIMIT, limit_type);
}


static short get_game_duration_radio(
	DialogPtr dialog)
{
	short items[]= {iRADIO_NO_TIME_LIMIT, iRADIO_TIME_LIMIT, iRADIO_KILL_LIMIT};
	short index, item_hit;
	
	for(index= 0; index<sizeof(items)/sizeof(items[0]); ++index)
	{
		short item_type;
		ControlHandle control;
		Rect bounds;
	
		GetDialogItem(dialog, items[index], &item_type, (Handle *) &control, &bounds);
		if(GetControlValue(control)) 
		{
			item_hit= items[index];
			break;
		}
	}
	
	assert(index!=sizeof(items)/sizeof(items[0]));
	
	return item_hit;
}


/*************************************************************************************************
 *
 * Function: extract_setup_dialog_information
 * Purpose:  extract all the information that we can squeeze out of the game setup dialog
 *
 *************************************************************************************************/
// ZZZ: moved this function to shared network_dialogs.cpp


/*************************************************************************************************
 *
 * Function: set_dialog_game_options
 * Purpose:  setup the game dialog's radio buttons given the game option flags.
 *
 *************************************************************************************************/
// ZZZ: moved this function to shared network_dialogs.cpp


// ZZZ: exposed this function
void fill_in_entry_points(
	DialogPtr dialog,
	short item,
	long entry_flags,
	short default_level)
{
	short item_type;
	Rect bounds;
	ControlHandle control;
	MenuHandle menu;
	short map_index, menu_index;
	struct entry_point entry;
	short default_item= NONE;

	/* Add the maps.. */
	GetDialogItem(dialog, item, &item_type, (Handle *) &control, &bounds);
	menu= get_popup_menu_handle(dialog, item);

	/* Nix the maps */
	while(CountMItems(menu))
	{
		DeleteMenuItem(menu, 1);
	}
	
	map_index= menu_index= 0;
	while (get_indexed_entry_point(&entry, &map_index, entry_flags))
	{
		AppendMenu(menu, "\p ");
		c2pstr(entry.level_name);
		menu_index++;
		if(entry.level_name[0])
		{
			SetMenuItemText(menu, menu_index, (StringPtr)entry.level_name);
		}

		if(entry.level_number==default_level) 
		{
			default_item= menu_index;
		}
	}
	SetControlMaximum(control, menu_index);
	
	if(default_item != NONE)
	{
		SetControlValue(control, default_item);	
	} 
	else if(GetControlValue(control)>=GetControlMaximum(control)) 
	{	
		SetControlValue(control, 1);
	}

	if (!CountMItems(menu)) modify_control(dialog, iOK, CONTROL_INACTIVE, 0);
}


/*************************************************************************************************
 *
 * Function: check_setup_information
 * Purpose:  check to make sure that the user entered usable information in the dialog.
 *
 *************************************************************************************************/
bool check_setup_information(
	DialogPtr dialog, 
	short game_limit_type)
{
	Rect     item_rect;
	short    item_type;
	short    limit;
	short    bad_item;
	Handle   item_handle;
	bool  information_is_acceptable;
	
	limit = extract_number_from_text_item(dialog, iTIME_LIMIT);
	if (game_limit_type == iRADIO_TIME_LIMIT && limit <= 0)
	{
		bad_item = iTIME_LIMIT;
		information_is_acceptable = false;
	}
	else
	{
		limit = extract_number_from_text_item(dialog, iKILL_LIMIT);
		if (game_limit_type == iRADIO_KILL_LIMIT && limit <= 0)
		{
			bad_item = iKILL_LIMIT;
			information_is_acceptable = false;
		}
		else
		{
			GetDialogItem(dialog, iGATHER_NAME, &item_type, &item_handle, &item_rect);
			GetDialogItemText(item_handle, ptemporary);
			if (*temporary == 0)
			{
				bad_item = iGATHER_NAME;
				information_is_acceptable = false;
			}
			else
				information_is_acceptable = true;
		}
	}
	
	if (!information_is_acceptable)
	{
		SysBeep(3);
		SelectDialogItemText(dialog, bad_item, 0, INT16_MAX);
	}

	return information_is_acceptable;
}

/*************************************************************************************************
 *
 * Function: get_dialog_control_value
 * Purpose:  given a dialog and an item number, extract the value of the control
 *
 *************************************************************************************************/
// ZZZ: moved this function to csdialogs_macintosh.cpp, and made one like it for csdialogs_sdl.cpp


/*************************************************************************************************
 *
 * Function: gather_dialog_filter_proc
 * Purpose:  the dialog filter procedure passed to ModalDialog() for the gathering dialog
 *
 *************************************************************************************************/
static pascal Boolean gather_dialog_filter_proc(
	DialogPtr dialog,
	EventRecord *event,
	short *item_hit)
{
	short item_type, charcode;
	Handle item_handle;
	Rect item_rectangle;
	GrafPtr old_port;
	bool handled;
	Point where;
	bool cell_is_selected;
	Cell selected_cell;
	short cell_count;

	/* preprocess events */	
	handled= false;
	GetPort(&old_port);
	SetPort(dialog);

	/* update the names list box; if we donÕt have a selection afterwords, dim the ADD button */
	// NetLookupUpdate();
	SetPt(&selected_cell, 0, 0);
	if (!LGetSelect(true, &selected_cell, network_list_box)) modify_control(dialog, iADD, CONTROL_INACTIVE, 0);

	switch(event->what)
	{
		case mouseDown:
			/* get the mouse in local coordinates */
			where= event->where;
			GlobalToLocal(&where);
			
			/* check for clicks in the list box */
			GetDialogItem(dialog, iNETWORK_LIST_BOX, &item_type, &item_handle, &item_rectangle);
			if (PtInRect(where, &item_rectangle))
			{
				if (LClick(where, event->modifiers, network_list_box))
				{
					GetDialogItem(dialog, iADD, &item_type, &item_handle, &item_rectangle);
					if (hit_dialog_button(dialog, iADD)) *item_hit= iADD;
				}
				
				handled= true;
			}

			/* check for clicks in the zone popup menu */
			break;
			
		case updateEvt:
			if ((DialogPtr)event->message==dialog)
			{
				/* update the zone popup menu */
				
				/* update the network list box and itÕs frame */
				LUpdate(dialog->visRgn, network_list_box);
				GetDialogItem(dialog, iNETWORK_LIST_BOX, &item_type, &item_handle, &item_rectangle);
				InsetRect(&item_rectangle, -1, -1);
				FrameRect(&item_rectangle);
				
				/* update the player area */
				update_player_list_item(dialog, iPLAYER_DISPLAY_AREA);
			}
			break;
		case keyDown:
			charcode = event->message & charCodeMask;

			cell_count = (**network_list_box).dataBounds.bottom - (**network_list_box).dataBounds.top;		
			SetPt(&selected_cell, 0, 0);
			cell_is_selected = LGetSelect(true, &selected_cell, network_list_box);

			if (cell_count)
			{
				*item_hit = iNETWORK_LIST_BOX; // for ADD button to be updated.
				switch(charcode)
				{
					case kUP_ARROW:
						if (cell_is_selected)
						{
							if (selected_cell.v > 0)
							{
								LSetSelect(false, selected_cell, network_list_box);
								selected_cell.v--;
								LSetSelect(true, selected_cell, network_list_box);
								LAutoScroll(network_list_box);
							}
						}
						else
						{
							SetPt(&selected_cell, 0, cell_count-1);
							LSetSelect(true, selected_cell, network_list_box);
							LAutoScroll(network_list_box);
						}
						*item_hit = iNETWORK_LIST_BOX; // for ADD button to be updated.
						handled = true;
						break;

					case kDOWN_ARROW:
						if (cell_is_selected)
						{
							if (selected_cell.v < cell_count-1)
							{
								LSetSelect(false, selected_cell, network_list_box);
								selected_cell.v++;
								LSetSelect(true, selected_cell, network_list_box);
								LAutoScroll(network_list_box);
							}
						}
						else
						{
							SetPt(&selected_cell, 0, 0);
							LSetSelect(true, selected_cell, network_list_box);
							LAutoScroll(network_list_box);
						}
						*item_hit = iNETWORK_LIST_BOX; // for ADD button to be updated.
						handled = true;
						break;
				}
			}
			break;
	}

	/* give the player area time (for animation, etc.) */

	/* check and see if weÕve gotten any connection requests */	
	
	SetPort(old_port);

	return handled ? true : general_filter_proc(dialog, event, item_hit);
}

/*************************************************************************************************
 *
 * Function: join_dialog_filter_proc
 * Purpose:  the dialog filter procedure passed to ModalDialog() for the joining dialog
 *
 *************************************************************************************************/
static pascal Boolean join_dialog_filter_proc(
	DialogPtr dialog,
	EventRecord *event,
	short *item_hit)
{
	Rect     item_rect;
	short    join_state;
	short    item_type;
	Handle   item_handle;
	bool  handled= false;
	GrafPtr  old_port;
	static short last_join_state;

	/* preprocess events */	
	GetPort(&old_port);
	SetPort(dialog);
	switch(event->what)
	{
		case updateEvt:
			if ((DialogPtr)event->message==dialog)
			{
				update_player_list_item(dialog, iPLAYER_DISPLAY_AREA);
			}
			break;
	}

	/* give the player area time (for animation, etc.) */

	/* check and see if weÕve gotten any connection requests */
	join_state= NetUpdateJoinState();
	switch (join_state)
	{
		case NONE: // haven't Joined yet.
			break;

		case netJoining:
			break;

		case netCancelled: /* the server cancelled the game; force bail */
			*item_hit= iCANCEL;
			handled= true;
			break;

		case netWaiting: /* if we just changed netJoining to netWaiting change the dialog text */
			modify_control(dialog, iCANCEL, CONTROL_INACTIVE, 0);
			break;

		case netStartingUp: /* the game is starting up (we have the network topography) */
			accepted_into_game = true;
			handled= true;
			break;

		case netPlayerAdded:
			if(last_join_state==netWaiting)
			{
				game_info *info= (game_info *)NetGetGameData();

				GetDialogItem(dialog, iJOIN_MESSAGES, &item_type, &item_handle, &item_rect);
				get_network_joined_message(temporary, info->net_game_type);
				c2pstr(temporary);
				SetDialogItemText(item_handle, ptemporary);
			}
			update_player_list_item(dialog, iPLAYER_DISPLAY_AREA);
			break;

		case netJoinErrorOccurred:
			*item_hit= iCANCEL;
			handled= true;
			break;
		
                case netChatMessageReceived:
                        player_info*	sending_player;
                        char*		chat_message;
                        
                        if(NetGetMostRecentChatMessage(&sending_player, &chat_message)) {
                            // should actually do something with the message here
                            // be sure to copy any info you need as the next call to
                            // NetUpdateJoinState could overwrite the storage we're pointing at.
                        }
                break;

		default:
			assert(false);
			break;
	}
	last_join_state= join_state;

	GetDialogItem(dialog, iJOIN_NAME, &item_type, &item_handle, &item_rect);
	GetDialogItemText(item_handle, ptemporary);
	if (join_state == NONE && *temporary)
		modify_control(dialog, iOK, CONTROL_ACTIVE, NONE);
	else
		modify_control(dialog, iOK, CONTROL_INACTIVE, NONE);

	SetPort(old_port);
	
	return handled ? true : general_filter_proc(dialog, event, item_hit);
}

/*************************************************************************************************
 *
 * Function: game_setup_filter_proc
 * Purpose:  the dialog filter procedure passed to ModalDialog() for the net game setup dialog
 *
 *************************************************************************************************/
static pascal Boolean game_setup_filter_proc(
	DialogPtr dialog,
	EventRecord *event,
	short *item_hit)
{
	Rect     item_rect;
	short    item_type;
	Handle   item_handle;
	GrafPtr  old_port;

	(void)(event, item_hit);
	GetPort(&old_port);
	SetPort(dialog);
	GetDialogItem(dialog, iGATHER_NAME, &item_type, &item_handle, &item_rect);
	GetDialogItemText(item_handle, ptemporary);
	if (*temporary)
		modify_control(dialog, iOK, CONTROL_ACTIVE, NONE);
	else
		modify_control(dialog, iOK, CONTROL_INACTIVE, NONE);
	SetPort(old_port);

	return general_filter_proc(dialog, event, item_hit);
}

/*************************************************************************************************
 *
 * Function: setup_network_list_box
 * Purpose:  allocates or clears the list that will list all the players in a zone.
 *
 *************************************************************************************************/
static void setup_network_list_box(
	WindowPtr window,
	Rect *frame,
	unsigned char *zone)
{
	Cell cell;
	OSErr error;
	
	if (!network_list_box)
	{
		Rect bounds;
		Rect adjusted_frame;

		/* allocate the list */

		SetPt(&cell, 0, 0);
		SetRect(&bounds, 0, 0, 1, 0);
	
		adjusted_frame= *frame;
		adjusted_frame.right-= SCROLLBAR_WIDTH-1;
		network_list_box= LNew(&adjusted_frame, &bounds, cell, 0, window, false, false, false, true);
		assert(network_list_box);
		LSetDrawingMode(true, network_list_box);	
		(*network_list_box)->selFlags= lOnlyOne;
	}
	else
	{
		/* the list is already allocated; delete all rows and close the existing network name lookup */
		
		LDelRow(0, 0, network_list_box);
		NetLookupClose();
	}

	/* spawn an asynchronous network name lookup */
	// LP: kludge to get the code to compile
	#ifndef mac
	error= NetLookupOpen("\p=", PLAYER_TYPE, zone, get_network_version(),
		network_list_box_update_proc, NetEntityNotInGame);
	if (error!=noErr) dprintf("NetLookupOpen() returned %d", error);
	#endif
}

/*************************************************************************************************
 *
 * Function: dispose_network_list_box
 * Purpose:  destroys the list that contains the list of players for a zone.
 *
 *************************************************************************************************/
static void dispose_network_list_box(
	void)
{
	assert(network_list_box);
	
	NetLookupClose();

	LDispose(network_list_box);
	network_list_box= (ListHandle) NULL;
}

/*************************************************************************************************
 *
 * Function: network_list_box_update_proc
 * Purpose:  adds or removes a player from the list of players in a zone.
 *
 *************************************************************************************************/
static void network_list_box_update_proc(
	short message,
	short index)
{
	NetEntityName entity;
	Cell cell;

	switch (message)
	{
		case removeEntity:
			LDelRow(1, index, network_list_box);
			break;
		
		case insertEntity:
			SetPt(&cell, 0, index);
			LAddRow(1, index, network_list_box);
			// NetLookupInformation(index, (NetAddrBlock *) NULL, &entity);
			// LSetCell(entity.objStr+1, entity.objStr[0], cell, network_list_box);
			LSetCell(entity+1, entity[0], cell, network_list_box);
			break;
		
		default:
			assert(false);
			break;
	}
}

/*************************************************************************************************
 *
 * Function: update_player_list_item
 * Purpose:
 *
 *************************************************************************************************/
static void update_player_list_item(
	WindowPtr dialog, 
	short item_num)
{
	Rect         item_rect, name_rect;
	short        i, num_players;
	short        item_type;
	short        height;
	Handle       item_handle;
	GrafPtr      old_port;
	FontInfo     finfo;
	
	GetPort(&old_port);
	SetPort(dialog);
	
	GetDialogItem(dialog, item_num, &item_type, &item_handle, &item_rect);
	GetFontInfo(&finfo);
	height = finfo.ascent + finfo.descent + finfo.leading;
	MoveTo(item_rect.left + 3, item_rect.top+height);
	num_players = NetNumberOfPlayerIsValid() ? NetGetNumberOfPlayers() : 0;
	SetRect(&name_rect, item_rect.left, item_rect.top, item_rect.left+NAME_BOX_WIDTH, item_rect.top+NAME_BOX_HEIGHT);
	for (i = 0; i < num_players; i++)
	{
		draw_player_box_with_team(&name_rect, i);
		if (!(i % 2))
		{
			OffsetRect(&name_rect, NAME_BOX_WIDTH+BOX_SPACING, 0);
		}
		else
		{
			OffsetRect(&name_rect, -(NAME_BOX_WIDTH+BOX_SPACING), NAME_BOX_HEIGHT + BOX_SPACING);
		}
	}
	
	SetPort(old_port);
}

static void calculate_box_colors(
	short color_index,
	RGBColor *highlight_color,
	RGBColor *bar_color,
	RGBColor *shadow_color)
{
	_get_player_color(color_index, highlight_color);

	bar_color->red = (highlight_color->red * 7) / 10;
	bar_color->blue = (highlight_color->blue * 7) / 10;
	bar_color->green = (highlight_color->green * 7) / 10;
	
	shadow_color->red = (highlight_color->red * 2) / 10;
	shadow_color->blue = (highlight_color->blue * 2) / 10;
	shadow_color->green = (highlight_color->green * 2) / 10;
}

/*************************************************************************************************
 *
 * Function: reassign_player_colors
 * Purpose:  This function used to reassign a player's color if it conflicted with another
 *           player's color. Now it reassigns everyone's colors. for the old function, see the
 *           obsoleted version (called check_player_info) at the end of this file.
 *
 *************************************************************************************************/
/* Note that we now only force unique colors across teams. */

// ZZZ: moved this function to network.cpp so it can be shared between Mac and SDL versions.



/*************************************************************************************************
 *
 * Function: menu_index_to_level_entry
 * Purpose:
 *
 *************************************************************************************************/
// ZZZ: exposed this function
void menu_index_to_level_entry(
	short menu_index, 
	long entry_flags,
	struct entry_point *entry)
{
	short  i, map_index;
	
	SetCursor(*GetCursor(watchCursor));

	map_index= 0;
	for (i= 0; i<menu_index; i++)
	{
		get_indexed_entry_point(entry, &map_index, entry_flags);
	}
	
	SetCursor(&qd.arrow);
}

static MenuHandle get_popup_menu_handle(
	DialogPtr dialog,
	short item)
{
	struct PopupPrivateData **privateHndl;
	MenuHandle menu;
	short item_type;
	ControlHandle control;
	Rect bounds;

	/* Add the maps.. */
	GetDialogItem(dialog, item, &item_type, (Handle *) &control, &bounds);

	/* I don't know how to assert that it is a popup control... <sigh> */
	privateHndl= (PopupPrivateData **) ((*control)->contrlData);
	assert(privateHndl);

	menu= (*privateHndl)->mHandle;
	assert(menu);

	return menu;
}

#ifdef TEST_NET_STATS_DIALOG
static void fake_initialize_stat_data(void)
{
	short i, j;
	
	for (i = 0; i < MAXIMUM_NUMBER_OF_PLAYERS; i++)
	{
		(players+i)->monster_damage_taken.damage = abs(Random()%200);
		(players+i)->monster_damage_taken.kills = abs(Random()%30);
		(players+i)->monster_damage_given.damage = abs(Random()%200);
		(players+i)->monster_damage_given.kills = abs(Random()%30);
		
		for (j = 0; j < MAXIMUM_NUMBER_OF_PLAYERS; j++)
		{
			(players+i)->damage_taken[j].damage = abs(Random()%200);
			(players+i)->damage_taken[j].kills = abs(Random()%6);
		}
	}
}
#endif

// ZZZ: new function used by setup_dialog_for_game_type
void set_limit_text(DialogPtr dialog, short radio_item, short radio_stringset_id, short radio_string_index,
                                short units_item, short units_stringset_id, short units_string_index)
{
	Handle item;
	short item_type;
	Rect bounds;

        GetDialogItem(dialog, radio_item, &item_type, &item, &bounds);
        getpstr(ptemporary, radio_stringset_id, radio_string_index);
        SetControlTitle((ControlHandle) item, ptemporary);
        
        GetDialogItem(dialog, units_item, &item_type, &item, &bounds);
        getpstr(ptemporary, units_stringset_id, units_string_index);
        SetDialogItemText(item, ptemporary);
}


/* For join & gather dialogs. */
static void draw_player_box_with_team(
	Rect *rectangle, 
	short player_index)
{
	player_info *player= (player_info *) NetGetPlayerData(player_index);
	RGBColor highlight_color, bar_color, shadow_color;
	Rect team_badge, color_badge, text_box;
	RGBColor old_color;
	short index;

	/* Save the color */
	GetForeColor(&old_color);

#define TEAM_BADGE_WIDTH 16	
	/* Setup the rectangles.. */
	team_badge= color_badge= *rectangle;
	team_badge.right= team_badge.left+TEAM_BADGE_WIDTH;
	color_badge.left= team_badge.right;

	/* Determine the colors */
	calculate_box_colors(player->team, &highlight_color,
		&bar_color, &shadow_color);

	/* Erase the team badge area. */
	RGBForeColor(&bar_color);
	PaintRect(&team_badge);
	
	/* Draw the highlight for this one. */
	RGBForeColor(&highlight_color);
	for (index = 0; index < NAME_BEVEL_SIZE; index++)
	{
		MoveTo(team_badge.left+index, team_badge.bottom-index);
		LineTo(team_badge.left+index, team_badge.top+index);
		LineTo(team_badge.right, team_badge.top+index);
	}
	
	/* Draw the drop shadow.. */
	RGBForeColor(&shadow_color);
	for (index = 0; index < NAME_BEVEL_SIZE; index++)
	{
		MoveTo(team_badge.left+index, team_badge.bottom-index);
		LineTo(team_badge.right, team_badge.bottom-index);
	}

	/* Now draw the player color. */
	calculate_box_colors(player->color, &highlight_color,
		&bar_color, &shadow_color);

	/* Erase the team badge area. */
	RGBForeColor(&bar_color);
	PaintRect(&color_badge);
	
	/* Draw the highlight for this one. */
	RGBForeColor(&highlight_color);
	for (index = 0; index < NAME_BEVEL_SIZE; index++)
	{
		MoveTo(color_badge.left, color_badge.top+index);
		LineTo(color_badge.right-index, color_badge.top+index);
	}
	
	/* Draw the drop shadow.. */
	RGBForeColor(&shadow_color);
	for (index = 0; index < NAME_BEVEL_SIZE; index++)
	{
		MoveTo(color_badge.left, color_badge.bottom-index);
		LineTo(color_badge.right-index, color_badge.bottom-index);
		LineTo(color_badge.right-index, color_badge.top+index);
	}

	/* Finally, draw the name. */
	text_box= *rectangle;
	InsetRect(&text_box, NAME_BEVEL_SIZE, NAME_BEVEL_SIZE);
	pstrcpy(ptemporary, player->name); 
	p2cstr(ptemporary);
	_draw_screen_text(temporary, (screen_rectangle *) &text_box, 
		_center_horizontal|_center_vertical, _net_stats_font, _white_color);		

	/* Restore the color */
	RGBForeColor(&old_color);
}


/* -------------------------- Statics for PostGame Carnage Report (redone) (sorta) */

// #include "network_games.h"

/* This function is used elsewhere */
//static void draw_beveled_text_box(bool inset, Rect *box, short bevel_size, 
//	RGBColor *brightest_color, char *text, short flags, bool name_box);

/* ------------ constants */
#define KILL_BAR_HEIGHT   21
#define DEATH_BAR_HEIGHT  14
#define GRAPH_LEFT_INSET  10
#define GRAPH_RIGHT_INSET 40
#define GRAPH_TOP_INSET   20
#define GRAPH_BAR_SPACING  5
#define RANKING_INSET      7
#define DEATH_BEVEL_SIZE   3
#define KILL_BEVEL_SIZE    4

/* ---------------------- globals */


/* ---------------------- prototypes */
static pascal Boolean display_net_stats_proc(DialogPtr dialog, EventRecord *event, short *item_hit);
static void update_damage_item(WindowPtr dialog);
static short create_graph_popup_menu(DialogPtr dialog, short item);
/*
static void draw_names(DialogPtr dialog, struct net_rank *ranks, short number_of_bars,
	short which_player);
static void draw_player_graph(DialogPtr dialog, short index);
static void get_net_color(short index, RGBColor *color);
static void draw_kill_bars(DialogPtr dialog, struct net_rank *ranks, short num_players, 
	short suicide_index, bool do_totals, bool friendly_fire);
static short calculate_max_kills(short num_players);
*/
static void draw_beveled_box(bool inset, Rect *box, short bevel_size, RGBColor *brightest_color);
/*
static void draw_totals_graph(DialogPtr dialog);
static void draw_team_totals_graph(DialogPtr dialog);
static void draw_total_scores_graph(DialogPtr dialog);
static void draw_team_total_scores_graph(DialogPtr dialog);
*/
static void calculate_maximum_bar(DialogPtr dialog, Rect *kill_bar_rect);
/*
static void draw_score_bars(DialogPtr dialog, struct net_rank *ranks, short bar_count);
*/
static bool will_new_mode_reorder_dialog(short new_mode, short previous_mode);

/* ---------------- code */
void display_net_game_stats(
	void)
{
	Rect item_rect;
	short item_hit, item_type, value, current_graph_selection;
	Handle item_handle;
	GrafPtr old_port;
	DialogPtr dialog;
	ModalFilterUPP stats_dialog_upp;
	short previous_mode;
	
	// eat all stray keypresses
	{
		EventRecord event;
		
		while (WaitNextEvent(keyDownMask|keyUpMask|autoKeyMask, &event, 0, (RgnHandle) NULL))
			;
	}
	
	dialog = myGetNewDialog(dlogNET_GAME_STATS, NULL, (WindowPtr) -1, refNETWORK_CARNAGE_DIALOG);
	assert(dialog);
	stats_dialog_upp= NewModalFilterProc(display_net_stats_proc);
	assert(stats_dialog_upp);

	/* Calculate the rankings (once) for the entire graph */
	calculate_rankings(rankings, dynamic_world->player_count);
	qsort(rankings, dynamic_world->player_count, sizeof(struct net_rank), rank_compare);

	/* Create the graph popup menu */
	current_graph_selection= create_graph_popup_menu(dialog, iGRAPH_POPUP);
	previous_mode= find_graph_mode(dialog, NULL);
	
	ShowWindow(dialog);
	
	GetPort(&old_port);
	SetPort(dialog);

	do
	{
		ModalDialog(stats_dialog_upp, &item_hit);
		switch (item_hit)
		{
			case iGRAPH_POPUP:
				value= get_dialog_control_value(dialog, iGRAPH_POPUP);
				if (current_graph_selection != value)
				{
					short new_mode;
				
					/* Determine what we need to delete. */
					GetDialogItem(dialog, iDAMAGE_STATS, &item_type, &item_handle, &item_rect);
					InsetRect(&item_rect, 1, 1); /* Avoid the grey border */

					new_mode= find_graph_mode(dialog, NULL);
					if (!will_new_mode_reorder_dialog(new_mode, previous_mode))
					{
						item_rect.left += GRAPH_LEFT_INSET + NAME_BOX_WIDTH + GRAPH_BAR_SPACING - 1;
					}
					item_rect.top += GRAPH_TOP_INSET-1;
					previous_mode= new_mode;

					/* Erase! */
					EraseRect(&item_rect);
					InvalRect(&item_rect);

					current_graph_selection= value;
				}
				break;
		}
	} while(item_hit != iOK);

	SetPort(old_port);

	DisposeRoutineDescriptor(stats_dialog_upp);
	DisposeDialog(dialog);
}

/* ------------------------- private code */
static short create_graph_popup_menu(
	DialogPtr dialog, 
	short item)
{
	MenuHandle graph_popup;
	short index, item_type;
	Handle item_handle;
	Rect item_rect;
	short current_graph_selection;
	bool has_scores;

	/* Clear the graph popup */
	graph_popup= get_popup_menu_handle(dialog, item);
	while(CountMenuItems(graph_popup))
		DeleteMenuItem(graph_popup, 1);

	/* Setup the player names */
	for (index= 0; index<dynamic_world->player_count; index++)
	{
		struct player_data *player= get_player_data(rankings[index].player_index);
		
		strcpy(temporary, player->name); 
		c2pstr(temporary);
		AppendMenu(graph_popup, "\p ");
		SetMenuItemText(graph_popup, index+1, ptemporary); // +1 since it is 1's based
	}
	
	/* Add in the separator line */
	AppendMenu(graph_popup, "\p-");

	/* Add in the total carnage.. */
	AppendMenu(graph_popup, getpstr(ptemporary, strNET_STATS_STRINGS, strTOTALS_STRING));
	current_graph_selection= CountMItems(graph_popup);
	
	/* Add in the scores */
	has_scores= get_network_score_text_for_postgame(temporary, false);
	if(has_scores)
	{
		c2pstr(temporary);
		AppendMenu(graph_popup, ptemporary);
		current_graph_selection= CountMItems(graph_popup);
	}
	
	/* If the game has teams, show the team stats. */
	if (!(dynamic_world->game_information.game_options & _force_unique_teams)) 
	{
		/* Separator line */
		if(has_scores) AppendMenu(graph_popup, "\p-");

		AppendMenu(graph_popup, getpstr(ptemporary, strNET_STATS_STRINGS, strTEAM_TOTALS_STRING));

		if(has_scores)
		{
			get_network_score_text_for_postgame(temporary, true);
			c2pstr(temporary);
			AppendMenu(graph_popup, ptemporary);
		}
	} 

	GetDialogItem(dialog, iGRAPH_POPUP, &item_type, &item_handle, &item_rect);
	SetControlMaximum((ControlHandle) item_handle, CountMItems(graph_popup));
	SetControlValue((ControlHandle) item_handle, current_graph_selection); 
	
	return current_graph_selection;
}


static pascal Boolean display_net_stats_proc(
	DialogPtr dialog,
	EventRecord *event,
	short *item_hit)
{
	GrafPtr old_port;
	bool handled= false;
	short item_type;
	Rect item_rect;
	Handle item_handle;
	short key, value, max;

	/* preprocess events */	
	GetPort(&old_port);
	SetPort(dialog);
	
	switch(event->what)
	{
		case updateEvt:
			if ((DialogPtr)event->message==dialog)
			{
				/* update the damage stats */
				update_damage_item(dialog);
			}
			break;
			
		case keyDown: 
		case autoKey:
			key = event->message & charCodeMask;
			GetDialogItem(dialog, iGRAPH_POPUP, &item_type, &item_handle, &item_rect);
			value= GetControlValue((ControlHandle) item_handle);
			max= GetControlMaximum((ControlHandle) item_handle);
			
			switch(key)
			{
				case kUP_ARROW:
				case kLEFT_ARROW:
				case kPAGE_UP:
					if(value>1)
					{
						short new_value= value-1;
	
						switch(new_value-dynamic_world->player_count-1)
						{
							case 0:
							case 3:
								/* This is a separator line-> skip it */
								new_value--;
								break;
						}
						
						SetControlValue((ControlHandle) item_handle, new_value);
						*item_hit= iGRAPH_POPUP; 
						handled= true;
					}
					break;
					
				case kDOWN_ARROW:
				case kRIGHT_ARROW:
				case kPAGE_DOWN:
					if(value<max)
					{
						short new_value= value+1;
	
						switch(new_value-dynamic_world->player_count-1)
						{
							case 0:
							case 3:
								/* This is a separator line-> skip it */
								new_value++;
								break;
						}

						SetControlValue((ControlHandle) item_handle, new_value);
						*item_hit= iGRAPH_POPUP; 
						handled= true;
					}
					break;
			
				default:
					break;
			}
			break;
			
		case mouseDown:
			{
				Rect   box;
				short  index, max;
				Point  where;
				bool in_box = false;

				where = event->where;
				GlobalToLocal(&where);
				GetDialogItem(dialog, iDAMAGE_STATS, &item_type, &item_handle, &item_rect);
				SetRect(&box, 0, 0, NAME_BOX_WIDTH, NAME_BOX_HEIGHT);
				OffsetRect(&box, item_rect.left+GRAPH_LEFT_INSET, item_rect.top+GRAPH_TOP_INSET);
				
				/* Find if they clicked in an area.. */
				switch(find_graph_mode(dialog, NULL))
				{
					case _player_graph:
					case _total_carnage_graph:
					case _total_scores_graph:
						max= dynamic_world->player_count;
						break;

					case _total_team_carnage_graph:
					case _total_team_scores_graph:
						max= 0; /* Don't let them click in any of these. (what would you do?) */
						break;
						
					default:
						assert(false);
						break;
				}
				
				/* Find the one clicked in.. */
				for (index= 0; index<max; index++)
				{
					if (PtInRect(where, &box))
					{
						in_box = true;
						break;
					}
					OffsetRect(&box, 0, RECTANGLE_HEIGHT(&box)+GRAPH_BAR_SPACING);
				}
				
				/* IF the one that they clicked in isn't the current one.. */
				GetDialogItem(dialog, iGRAPH_POPUP, &item_type, &item_handle, &item_rect);
				if (in_box && (index+1) != GetControlValue((ControlHandle) item_handle))
				{
					bool last_in_box= false;
					RGBColor color;

					_get_player_color(rankings[index].color, &color);
								
					while (StillDown())
					{
						GetMouse(&where);
						in_box = PtInRect(where, &box);

						if (last_in_box != in_box)
						{
							if(rankings[index].player_index==NONE)
							{
								draw_beveled_text_box(in_box, &box, NAME_BEVEL_SIZE, &color, 
									"", _center_horizontal|_center_vertical, true);
							} else {
								struct player_data *player= get_player_data(rankings[index].player_index);
							
								draw_beveled_text_box(in_box, &box, NAME_BEVEL_SIZE, &color, 
									player->name, _center_horizontal|_center_vertical, true);
							}
							last_in_box= in_box;
						}
					}
					
					/* IF we were still in at the end.. */
					if (in_box)
					{
						GetDialogItem(dialog, iGRAPH_POPUP, &item_type, &item_handle, &item_rect);
						SetControlValue((ControlHandle) item_handle, index+1); // 1 based
						*item_hit = iGRAPH_POPUP; 
						handled = true;
					}
				}
			}
			break;
	}

	SetPort(old_port);

	return handled ? true : general_filter_proc(dialog, event, item_hit);
}

static void update_damage_item(
	WindowPtr dialog)
{
	GrafPtr old_port;
  	TextSpec font_info;
	TextSpec old_font;

	GetPort(&old_port);
	SetPort(dialog);

	GetNewTextSpec(&font_info, fontTOP_LEVEL_FONT, 0);
	GetFont(&old_font);
	SetFont(&font_info);

    // ZZZ: ripped this out into separate function for better sharing.
    draw_new_graph(dialog);

	SetFont(&old_font);	
	SetPort(old_port);
}

/* This function takes a rank structure because the rank structure contains the team & is */
/*  sorted.  */
void draw_names(
	DialogPtr dialog, 
	struct net_rank *ranks, 
	short number_of_bars,
	short which_player)
{
	Rect item_rect, name_rect;
	short item_type, i;
	Handle item_handle;
	RGBColor color;

	SetRect(&name_rect, 0, 0, NAME_BOX_WIDTH, NAME_BOX_HEIGHT);
	GetDialogItem(dialog, iDAMAGE_STATS, &item_type, &item_handle, &item_rect);
	OffsetRect(&name_rect, item_rect.left+GRAPH_LEFT_INSET, item_rect.top+GRAPH_TOP_INSET);
	for (i = 0; i <number_of_bars; i++)
	{
		if (ranks[i].player_index != NONE)
		{
			struct player_data *player= get_player_data(ranks[i].player_index);

			_get_player_color(ranks[i].color, &color);
			draw_beveled_text_box(which_player==i, &name_rect, NAME_BEVEL_SIZE, &color, player->name, 
				_center_horizontal|_center_vertical, true);
		}
		else
		{
			_get_player_color(ranks[i].color, &color);
			draw_beveled_box(false, &name_rect, NAME_BEVEL_SIZE, &color);
		}
		OffsetRect(&name_rect, 0, RECTANGLE_HEIGHT(&name_rect)+GRAPH_BAR_SPACING);
	}
}


void draw_kill_bars(
	DialogPtr dialog, 
	struct net_rank *ranks, 
	short num_players, 
	short suicide_index, 
	bool do_totals, 
	bool friendly_fire)
{
	char kill_string_format[65], death_string_format[65], suicide_string_format[65];
	Rect item_rect, kill_bar_rect, death_bar_rect, suicide_bar_rect;
	short i;
	short item_type, max_kills, max_width;
	Handle item_handle;
	RGBColor kill_color, suicide_color, death_color;

	get_net_color(_kill_color, &kill_color);
	get_net_color(_suicide_color, &suicide_color);
	get_net_color(_death_color, &death_color);

	getcstr(kill_string_format, strNET_STATS_STRINGS, strKILLS_STRING);
	getcstr(death_string_format, strNET_STATS_STRINGS, strDEATHS_STRING);
	getcstr(suicide_string_format, strNET_STATS_STRINGS, strSUICIDES_STRING);

	GetDialogItem(dialog, iDAMAGE_STATS, &item_type, &item_handle, &item_rect);
	kill_bar_rect.left = item_rect.left + GRAPH_LEFT_INSET + NAME_BOX_WIDTH + GRAPH_BAR_SPACING;
	kill_bar_rect.top = item_rect.top + GRAPH_TOP_INSET;
	kill_bar_rect.bottom = kill_bar_rect.top + KILL_BAR_HEIGHT;
	
	death_bar_rect.left = item_rect.left + GRAPH_LEFT_INSET + NAME_BOX_WIDTH + GRAPH_BAR_SPACING;
	death_bar_rect.top = item_rect.top + GRAPH_TOP_INSET + NAME_BOX_HEIGHT - DEATH_BAR_HEIGHT;
	death_bar_rect.bottom = death_bar_rect.top + DEATH_BAR_HEIGHT;
	
	if (do_totals)
	{
		for (i = 0, max_kills = 0; i < num_players; i++)
		{
			if (ranks[i].kills > max_kills) max_kills = ranks[i].kills;
			if (ranks[i].deaths > max_kills) max_kills = ranks[i].deaths;
		}
	}
	else
	{
		max_kills= calculate_max_kills(num_players);
	}
	max_width = item_rect.right - GRAPH_RIGHT_INSET - kill_bar_rect.left;
	
	for (i = 0; i < num_players; i++)
	{
		if (max_kills)
			kill_bar_rect.right = kill_bar_rect.left + ((ranks[i].kills * max_width) / max_kills);
		else
			kill_bar_rect.right = kill_bar_rect.left;
		if (max_kills)
			death_bar_rect.right = death_bar_rect.left + ((ranks[i].deaths * max_width) / max_kills);
		else
			death_bar_rect.right = death_bar_rect.left;

		if (suicide_index != i)
		{
			Rect              ranking_rect;
			short             diff = ranks[i].kills - ranks[i].deaths;
			screen_rectangle  rr;
			
			if (ranks[i].kills)
			{
				sprintf(temporary, kill_string_format, ranks[i].kills);
				draw_beveled_text_box(false, &kill_bar_rect, KILL_BEVEL_SIZE, &kill_color, temporary, _right_justified|_top_justified, false);
				if (diff > 0)
				{
					ranking_rect = kill_bar_rect;
					ranking_rect.top += KILL_BEVEL_SIZE;
					ranking_rect.bottom -= KILL_BEVEL_SIZE;
				}
				
			}
			if (ranks[i].deaths)
			{
				sprintf(temporary, death_string_format, ranks[i].deaths);
				draw_beveled_text_box(false, &death_bar_rect, DEATH_BEVEL_SIZE, &death_color, temporary, _right_justified|_center_vertical, false);
				if (diff < 0)
				{
					ranking_rect = death_bar_rect;
					ranking_rect.top += DEATH_BEVEL_SIZE;
					ranking_rect.bottom -= DEATH_BEVEL_SIZE;
				}
			}
			if (diff == 0)
			{
				ranking_rect.top = kill_bar_rect.top;
				ranking_rect.bottom = death_bar_rect.bottom;
				ranking_rect.left = kill_bar_rect.left;
				ranking_rect.right = MAX(kill_bar_rect.right, death_bar_rect.right);
			}
			rr.top = ranking_rect.top;
			rr.bottom = ranking_rect.bottom;
			rr.left = ranking_rect.right + RANKING_INSET;
			rr.right = rr.left + 1000; // just make it big enough.
			
			sprintf(temporary, "%+d", diff);
			_draw_screen_text(temporary, &rr, _center_vertical, _net_stats_font, _black_color);
		}
		else
		{
			if (ranks[i].kills)
			{
				SetRect(&suicide_bar_rect, kill_bar_rect.left, kill_bar_rect.top, kill_bar_rect.right , death_bar_rect.bottom);
				sprintf(temporary, suicide_string_format, ranks[i].kills);
				draw_beveled_text_box(false, &suicide_bar_rect, KILL_BEVEL_SIZE, &suicide_color, temporary, _right_justified|_center_vertical, false);
			}
		}
		OffsetRect(&kill_bar_rect, 0, NAME_BOX_HEIGHT + GRAPH_BAR_SPACING);
		OffsetRect(&death_bar_rect, 0, NAME_BOX_HEIGHT + GRAPH_BAR_SPACING);
	}

    // ZZZ: ripped this out into a new function for sharing with SDL version
    update_carnage_summary(dialog, ranks, num_players, suicide_index, do_totals, friendly_fire);
}

static void draw_beveled_text_box(
	bool inset, 
	Rect *box,
	short bevel_size,
	RGBColor *brightest_color,
	char *text,
	short flags,
	bool name_box)
{
	Rect inset_box;
	short color;
	screen_rectangle text_box;
	
	draw_beveled_box(inset, box, bevel_size, brightest_color);

	color = inset || !name_box ? _black_color : _white_color;
	inset_box = *box; 
	InsetRect(&inset_box, bevel_size, bevel_size);
	text_box.top = inset_box.top; text_box.bottom = inset_box.bottom;
	text_box.left = inset_box.left; text_box.right = inset_box.right;
	_draw_screen_text(text, &text_box, flags, _net_stats_font, color);		
}

static void draw_beveled_box(
	bool inset, 
	Rect *box,
	short bevel_size,
	RGBColor *brightest_color)
{
	short i;
	RGBColor second_color, third_color, old_color;
	
	GetForeColor(&old_color);
	
	second_color.red = (brightest_color->red * 7) / 10;
	second_color.blue = (brightest_color->blue * 7) / 10;
	second_color.green = (brightest_color->green * 7) / 10;
	
	third_color.red = (brightest_color->red * 2) / 10;
	third_color.blue = (brightest_color->blue * 2) / 10;
	third_color.green = (brightest_color->green * 2) / 10;
	
	RGBForeColor(&second_color);
	PaintRect(box);
	
	if (RECTANGLE_WIDTH(box) > 2*bevel_size && RECTANGLE_HEIGHT(box) > 2*bevel_size)
	{
		if (inset)
		{
			RGBForeColor(&third_color);
		}
		else
		{
			RGBForeColor(brightest_color);
		}
		
		for (i = 0; i < bevel_size; i++)
		{
			MoveTo(box->left+i, box->top+i);
			LineTo(box->right-i, box->top+i);
			MoveTo(box->left+i, box->top+i);
			LineTo(box->left+i, box->bottom-i);
		}
		
		if (inset)
		{
			RGBForeColor(brightest_color);
		}
		else
		{
			RGBForeColor(&third_color);
		}
		
		for (i = 0; i < bevel_size; i++)
		{
			MoveTo(box->right-i, box->bottom-i);
			LineTo(box->left+i, box->bottom-i);
			MoveTo(box->right-i, box->bottom-i);
			LineTo(box->right-i, box->top+i);
		}
		
	}

	RGBForeColor(&old_color);
}

static void calculate_maximum_bar(	
	DialogPtr dialog,
	Rect *kill_bar_rect)
{
	short item_type;
	Handle item_handle;
	Rect item_rect;

	GetDialogItem(dialog, iDAMAGE_STATS, &item_type, &item_handle, &item_rect);
	kill_bar_rect->left = item_rect.left + GRAPH_LEFT_INSET + NAME_BOX_WIDTH + GRAPH_BAR_SPACING;
	kill_bar_rect->right = item_rect.right - GRAPH_RIGHT_INSET - kill_bar_rect->left;
	kill_bar_rect->top = item_rect.top + GRAPH_TOP_INSET;
	kill_bar_rect->bottom = kill_bar_rect->top + NAME_BOX_HEIGHT;
}

void draw_score_bars(
	DialogPtr dialog, 
	struct net_rank *ranks, 
	short bar_count)
{
	short index, maximum_width;
	long highest_ranking= LONG_MIN;
	Rect maximum_bar, bar;
	RGBColor color;
	short item_type;
	Handle item_handle;
	
	for(index= 0; index<bar_count; ++index)
	{
		if(ranks[index].game_ranking>highest_ranking) highest_ranking= ranks[index].game_ranking;
	}

	calculate_maximum_bar(dialog, &maximum_bar);
	bar= maximum_bar;
	maximum_width= RECTANGLE_WIDTH(&bar);

	get_net_color(_score_color, &color);

	if(highest_ranking)
	{
		for(index= 0; index<bar_count; ++index)
		{
			/* Get the text. */
			calculate_ranking_text_for_post_game(temporary, ranks[index].game_ranking);
	
			/* Build the bar. */		
			bar.right= bar.left + (ranks[index].game_ranking*maximum_width)/highest_ranking;
	
			/* Draw it! */
			draw_beveled_text_box(false, &bar, NAME_BEVEL_SIZE, &color, temporary, 
				_right_justified|_center_vertical, false);
	
			OffsetRect(&bar, 0, NAME_BOX_HEIGHT + GRAPH_BAR_SPACING);
		}
	}

	/* And clear the text. */
	GetDialogItem(dialog, iTOTAL_DEATHS, &item_type, &item_handle, &bar);
	SetDialogItemText(item_handle, "\p");
	GetDialogItem(dialog, iTOTAL_KILLS, &item_type, &item_handle, &bar);
	SetDialogItemText(item_handle, "\p");
}

static bool will_new_mode_reorder_dialog(
	short new_mode,
	short previous_mode)
{
	bool may_reorder= false;
		
	switch(new_mode)
	{
		case _player_graph:
			switch(previous_mode)
			{
				case _player_graph:
				case _total_carnage_graph:
					may_reorder= false; 
					break;
					
				case _total_scores_graph:
				case _total_team_carnage_graph:
				case _total_team_scores_graph:
					may_reorder= true;
					break;
			}
			break;
		
		case _total_carnage_graph:
			switch(previous_mode)
			{
				case _player_graph:
				case _total_carnage_graph:
					may_reorder= false; 
					break;
					
				case _total_scores_graph:
				case _total_team_carnage_graph:
				case _total_team_scores_graph:
					may_reorder= true;
					break;
			}
			break;
			
		case _total_scores_graph:
			switch(previous_mode)
			{
				case _player_graph:
				case _total_carnage_graph:
				case _total_scores_graph:
					may_reorder= false; 
					break;
					
				case _total_team_carnage_graph:
				case _total_team_scores_graph:
					may_reorder= true;
					break;
			}
			break;
	
		case _total_team_carnage_graph:
			switch(previous_mode)
			{
				case _total_team_carnage_graph:
					may_reorder= false;
					break;
	
				case _player_graph:
				case _total_carnage_graph:
				case _total_scores_graph:
				case _total_team_scores_graph:
					may_reorder= true; 
					break;
			}
			break;
			
		case _total_team_scores_graph:
			switch(previous_mode)
			{
				case _total_team_scores_graph:
					may_reorder= false;
					break;
	
				case _player_graph:
				case _total_carnage_graph:
				case _total_scores_graph:
				case _total_team_carnage_graph:
					may_reorder= true; 
					break;
			}
			break;
			
		default:
			assert(false);
			break;
	}
	
	return may_reorder;
}

static void setup_network_speed_for_join(
	DialogPtr dialog)
{
	MenuHandle menu;
	short index;
	
	menu= get_popup_menu_handle(dialog, iJOIN_NETWORK_TYPE);
	for(index= 0; index<NUMBER_OF_TRANSPORT_TYPES; ++index)
	{
		if(!NetTransportAvailable(index))
		{
			DisableItem(menu, index+1);
		}
	}
}

static void setup_network_speed_for_gather(
	DialogPtr dialog)
{
	short index;
	MenuHandle menu;
	
	menu= get_popup_menu_handle(dialog, iNETWORK_SPEED);
	for(index= 0; index<NUMBER_OF_NETWORK_TYPES; ++index)
	{
		switch(index)
		{
			case _appletalk_remote:
				/* Should actually be able to check for appletalk remote */
//				break;
			case _localtalk:
			case _tokentalk:
			case _ethernet:
				if(!NetTransportAvailable(kNetworkTransportType))
				{
					DisableItem(menu, index+1);
				}
				break;
				
#ifdef USE_MODEM
			case _modem:
				if(!NetTransportAvailable(kModemTransportType))
				{
					DisableItem(menu, index+1);
				}
				break;
#endif				
			default:
				assert(false);
				break;
		}
	}
}

/* Stupid function, here as a hack.. */
static bool key_is_down(
	short key_code)
{
	KeyMap key_map;
	
	GetKeys(key_map);
	return ((((byte*)key_map)[key_code>>3] >> (key_code & 7)) & 1);
}
