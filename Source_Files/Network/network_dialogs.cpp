/*
NETWORK_DIALOGS.C  (network_dialogs.cpp)

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

Sept-Nov 2001 (Woody Zenfell):
        This file was split into Mac-specific code (in network_dialogs_macintosh.cpp) and
        shared code (this file, network_dialogs.cpp).

Feb 27, 2002 (Br'fin (Jeremy Parsons)):
	Moved shared SDL hint address info here from network_dialogs_sdl.cpp
	Reworked #ifdef mac to #if !defined(HAVE_SDL_NET)
*/

#include	"cseries.h"
#include	"map.h"
#include	"shell.h"
#include	"preferences.h"
#include	"network.h"
#include	"network_dialogs.h"
#include	"network_games.h"
#include    "player.h" // ZZZ: for MAXIMUM_NUMBER_OF_PLAYERS, for reassign_player_colors

#if HAVE_SDL_NET
// JTP: Shared join information. Here because it can be shared with Carbon/SDL networking
bool	sUserWantsJoinHinting				   = false;
char	sJoinHintingAddress[kJoinHintingAddressLength + 1] = "";
#endif

static long get_dialog_game_options(DialogPtr dialog, short game_type);
static void set_dialog_game_options(DialogPtr dialog, long game_options);


/*************************************************************************************************
 *
 * Function: extract_setup_dialog_information
 * Purpose:  extract all the information that we can squeeze out of the game setup dialog
 *
 *************************************************************************************************/
void
extract_setup_dialog_information(
	DialogPtr dialog,
	player_info *player_information,
	game_info *game_information,
	short game_limit_type,
	bool allow_all_levels)
{
#if !HAVE_SDL_NET
	short               network_speed;
#endif
	short               updates_per_packet, update_latency;
	struct entry_point  entry;
	long entry_flags;
	
	// get player information
//	GetDialogItem(dialog, iGATHER_NAME, &item_type, &item_handle, &item_rect);
//	GetDialogItemText(item_handle, ptemporary);
        copy_pstring_from_text_field(dialog, iGATHER_NAME, ptemporary);
	if (*temporary > MAX_NET_PLAYER_NAME_LENGTH) 
		*temporary = MAX_NET_PLAYER_NAME_LENGTH;
	pstrcpy(player_information->name, ptemporary);
	player_information->color= get_selection_control_value(dialog, iGATHER_COLOR) -1;
	player_information->team= get_selection_control_value(dialog, iGATHER_TEAM)-1;

	pstrcpy(player_preferences->name, player_information->name);
	player_preferences->color= player_information->color;
	player_preferences->team= player_information->team;

	game_information->server_is_playing = true;
	game_information->net_game_type= get_selection_control_value(dialog, iGAME_TYPE)-1;

	// get game information
	game_information->game_options= get_dialog_game_options(dialog, game_information->net_game_type);
	if (game_limit_type == iRADIO_NO_TIME_LIMIT)
	{
		game_information->time_limit = LONG_MAX;
	}
	else if (game_limit_type == iRADIO_KILL_LIMIT)
	{
		// START Benad
		if (get_selection_control_value(dialog, iGAME_TYPE)-1 == _game_of_defense)
		{
			game_information->game_options |= _game_has_kill_limit;
			game_information->time_limit = extract_number_from_text_item(dialog, iTIME_LIMIT);
			game_information->time_limit *= TICKS_PER_SECOND * 60;
		}
		else
		{
			game_information->game_options |= _game_has_kill_limit;
			game_information->time_limit = LONG_MAX;
		}
		// END Benad
	}
	else
	{
		// START Benad
		if (get_selection_control_value(dialog, iGAME_TYPE)-1 == _game_of_defense)
		{
			game_information->game_options |= _game_has_kill_limit;
			game_information->time_limit = extract_number_from_text_item(dialog, iTIME_LIMIT);
			game_information->time_limit *= TICKS_PER_SECOND * 60;
		}
		else
		{
			game_information->time_limit = extract_number_from_text_item(dialog, iTIME_LIMIT);
			game_information->time_limit *= TICKS_PER_SECOND * 60;
		}
		// END Benad
	}
	game_information->kill_limit = extract_number_from_text_item(dialog, iKILL_LIMIT);
	// START Benad
	if (get_selection_control_value(dialog, iGAME_TYPE)-1 == _game_of_defense)
		game_information->kill_limit *= 60; // It's "Time On Hill" limit, in seconds.
	// END Benad

	/* Determine the entry point flags by the game type. */
    if(allow_all_levels)
	{
		entry_flags= NONE;
	} else {
		entry_flags= get_entry_point_flags_for_game_type(game_information->net_game_type);
	}
	menu_index_to_level_entry(get_selection_control_value(dialog, iENTRY_MENU), entry_flags, &entry);    
    network_preferences->game_type= game_information->net_game_type;

	game_information->level_number = entry.level_number;
	strcpy(game_information->level_name, entry.level_name);
	game_information->difficulty_level = get_selection_control_value(dialog, iDIFFICULTY_MENU)-1;

#if !HAVE_SDL_NET
	game_information->allow_mic = (bool) get_boolean_control_value(dialog, iREAL_TIME_SOUND);
#else
        game_information->allow_mic = false;
#endif


#if !HAVE_SDL_NET
#ifdef OBSOLETE
	// get network information
	network_speed = get_selection_control_value(dialog, iNETWORK_SPEED)-1;
	updates_per_packet = net_speeds[network_speed].updates_per_packet;
	update_latency = net_speeds[network_speed].update_latency;
#endif
#else
        updates_per_packet = 1;
        update_latency = 0;
#endif
	vassert(updates_per_packet > 0 && update_latency >= 0 && updates_per_packet < 16,
		csprintf(temporary, "You idiot! updates_per_packet = %d, update_latency = %d", updates_per_packet, update_latency));
	game_information->initial_updates_per_packet = updates_per_packet;
	game_information->initial_update_latency = update_latency;
	NetSetInitialParameters(updates_per_packet, update_latency);
	
	game_information->initial_random_seed = (uint16) machine_tick_count(); // was (uint16) TickCount(); // bo-ring

	// now save some of these to the preferences
#if !HAVE_SDL_NET
	network_preferences->type = network_speed; 
#endif
	network_preferences->allow_microphone = game_information->allow_mic;
	network_preferences->difficulty_level = game_information->difficulty_level;
	network_preferences->entry_point= get_selection_control_value(dialog, iENTRY_MENU)-1;
	network_preferences->game_options = game_information->game_options;
	network_preferences->time_limit = extract_number_from_text_item(dialog, iTIME_LIMIT)*60*TICKS_PER_SECOND;
	if (network_preferences->time_limit <= 0) // if it wasn't chosen, this could be so
	{
		network_preferences->time_limit = 10*60*TICKS_PER_SECOND;
	}
	if (game_information->time_limit == LONG_MAX)
	{
		network_preferences->game_is_untimed = true;
	}
	else
	{
		network_preferences->game_is_untimed = false;	
	}
	network_preferences->kill_limit = game_information->kill_limit;

	write_preferences();

	/* Don't save the preferences of their team... */
	if(game_information->game_options & _force_unique_teams)
	{
		player_information->team= player_information->color;
	}

	return;
}



/*************************************************************************************************
 *
 * Function: fill_in_game_setup_dialog
 * Purpose:  setup the majority of the game setup dialog.
 *
 *************************************************************************************************/
short
fill_in_game_setup_dialog(
	DialogPtr dialog, 
	player_info *player_information,
	bool allow_all_levels)
{
	short name_length;
	long entry_flags;
	short net_game_type;

	/* Fill in the entry points */
	if(allow_all_levels)
	{
		entry_flags= NONE;
	} else {
		entry_flags= get_entry_point_flags_for_game_type(network_preferences->game_type);
	}

    // ZZZ: SDL version takes care of this its own way, simpler not to change.
#ifdef mac
    fill_in_entry_points(dialog, iENTRY_MENU, entry_flags, NONE);
#endif

	modify_selection_control(dialog, iGAME_TYPE, CONTROL_ACTIVE, network_preferences->game_type+1);
    setup_dialog_for_game_type(dialog, network_preferences->game_type);
    net_game_type= network_preferences->game_type;

	/* set up the name of the player. */
	name_length= player_preferences->name[0];
	if(name_length>MAX_NET_PLAYER_NAME_LENGTH) name_length= MAX_NET_PLAYER_NAME_LENGTH;
        // XXX (ZZZ): it looks to me that this forgets to set player_information->name[0] to name_length?
        // not sure if this is a problem in practice, but the Bungie guys were usually pretty careful about this sort of thing.
	memcpy(player_information->name, player_preferences->name, name_length+1);
	
//	GetDialogItem(dialog, iGATHER_NAME, &item_type, &item_handle, &item_rect);
//	SetDialogItemText(item_handle, player_information->name);
        copy_pstring_to_text_field(dialog, iGATHER_NAME, player_information->name);
#ifdef mac
	SelectDialogItemText(dialog, iGATHER_NAME, 0, INT16_MAX);
#endif

	/* Set the menu values */
	modify_selection_control(dialog, iGATHER_COLOR, CONTROL_ACTIVE, player_preferences->color+1);
	modify_selection_control(dialog, iGATHER_TEAM, CONTROL_ACTIVE, player_preferences->team+1);
	modify_selection_control(dialog, iDIFFICULTY_MENU, CONTROL_ACTIVE, network_preferences->difficulty_level+1);
	modify_selection_control(dialog, iENTRY_MENU, CONTROL_ACTIVE, network_preferences->entry_point+1);

	// START Benad
	if (network_preferences->game_type == _game_of_defense)
		insert_number_into_text_item(dialog, iKILL_LIMIT, network_preferences->kill_limit/60);
	else
		insert_number_into_text_item(dialog, iKILL_LIMIT, network_preferences->kill_limit);
	// END Benad
	insert_number_into_text_item(dialog, iTIME_LIMIT, network_preferences->time_limit/TICKS_PER_SECOND/60);

	if (network_preferences->game_options & _game_has_kill_limit)
	{
                // ZZZ: factored out into new function
                setup_for_score_limited_game(dialog);
	}
	else if (network_preferences->game_is_untimed)
	{
		setup_for_untimed_game(dialog);
	}
	else
	{
		setup_for_timed_game(dialog);
	}

	if (player_information->name[0]==0) modify_control_enabled(dialog, iOK, CONTROL_INACTIVE);

	// set up the game options
#if !HAVE_SDL_NET
	modify_boolean_control(dialog, iREAL_TIME_SOUND, CONTROL_ACTIVE, network_preferences->allow_microphone);
#endif
	set_dialog_game_options(dialog, network_preferences->game_options);

	/* Setup the team popup.. */
	if(!get_boolean_control_value(dialog, iFORCE_UNIQUE_TEAMS))
	{
		modify_control_enabled(dialog, iGATHER_TEAM, CONTROL_INACTIVE);
	} else {
		modify_control_enabled(dialog, iGATHER_TEAM, CONTROL_ACTIVE);
	}

#if !HAVE_SDL_NET
	// set up network options
#ifdef OBSOLETE
	setup_network_speed_for_gather(dialog);
#endif
	modify_selection_control(dialog, iNETWORK_SPEED, CONTROL_ACTIVE, network_preferences->type+1);
#endif

	return net_game_type;
}



/*************************************************************************************************
 *
 * Function: get_dialog_game_options
 * Purpose:  extract the game option flags from the net game setup's controls
 *
 *************************************************************************************************/
static long
get_dialog_game_options(
	DialogPtr dialog,
	short game_type)
{
	long game_options = 0;
	
	// These used to be options in the dialog. now they are always true, i guess.
	game_options |= (_ammo_replenishes | _weapons_replenish | _specials_replenish);
#ifdef DEBUG
	game_options |= _overhead_map_is_omniscient;
#endif
	if(game_type==_game_of_cooperative_play) game_options |= _overhead_map_is_omniscient;
	if (get_boolean_control_value(dialog, iUNLIMITED_MONSTERS)) game_options |= _monsters_replenish;
	if (get_boolean_control_value(dialog, iMOTION_SENSOR_DISABLED)) game_options |= _motion_sensor_does_not_work;
	if (get_boolean_control_value(dialog, iDYING_PUNISHED)) game_options |= _dying_is_penalized;
	if (get_boolean_control_value(dialog, iSUICIDE_PUNISHED)) game_options |= _suicide_is_penalized;
	if (!get_boolean_control_value(dialog, iFORCE_UNIQUE_TEAMS)) game_options |= _force_unique_teams;
	if (!get_boolean_control_value(dialog, iBURN_ITEMS_ON_DEATH)) game_options |= _burn_items_on_death;
	if (get_boolean_control_value(dialog, iREALTIME_NET_STATS)) game_options |= _live_network_stats;
	
	return game_options;
}



/*************************************************************************************************
 *
 * Function: set_dialog_game_options
 * Purpose:  setup the game dialog's radio buttons given the game option flags.
 *
 *************************************************************************************************/
static void
set_dialog_game_options(
	DialogPtr dialog, 
	long game_options)
{
	modify_boolean_control(dialog, iUNLIMITED_MONSTERS, NONE, (game_options & _monsters_replenish) ? 1 : 0);
	modify_boolean_control(dialog, iMOTION_SENSOR_DISABLED, NONE, (game_options & _motion_sensor_does_not_work) ? 1 : 0);
	modify_boolean_control(dialog, iDYING_PUNISHED, NONE, (game_options & _dying_is_penalized) ? 1 : 0);
	modify_boolean_control(dialog, iSUICIDE_PUNISHED, NONE, (game_options & _suicide_is_penalized) ? 1 : 0);
	modify_boolean_control(dialog, iFORCE_UNIQUE_TEAMS, NONE, (game_options & _force_unique_teams) ? false : true);
	modify_boolean_control(dialog, iBURN_ITEMS_ON_DEATH, NONE, (game_options & _burn_items_on_death) ? false : true);
	modify_boolean_control(dialog, iREALTIME_NET_STATS, NONE, (game_options & _live_network_stats) ? true : false);

	return;
}


// ZZZ: moved here from network_dialogs_macintosh.cpp
void setup_dialog_for_game_type(
	DialogPtr dialog, 
	short game_type)
{
/*	Handle item;
	short item_type;
	Rect bounds;
*/	
	switch(game_type)
	{
		case _game_of_cooperative_play:
			/* set & disable the drop items checkbox */
			// Benad
			modify_boolean_control(dialog, iFORCE_UNIQUE_TEAMS, CONTROL_ACTIVE, NONE);
			
			modify_boolean_control(dialog, iBURN_ITEMS_ON_DEATH, CONTROL_INACTIVE, true);
			modify_boolean_control(dialog, iUNLIMITED_MONSTERS, CONTROL_INACTIVE, true);
		
			set_limit_text(dialog, iRADIO_KILL_LIMIT, strSETUP_NET_GAME_MESSAGES, killLimitString,
                                        iTEXT_KILL_LIMIT, strSETUP_NET_GAME_MESSAGES, killsString);
                        /*
                        GetDialogItem(dialog, iRADIO_KILL_LIMIT, &item_type, &item, &bounds);
			getpstr(ptemporary, strSETUP_NET_GAME_MESSAGES, killLimitString);
			SetControlTitle((ControlHandle) item, ptemporary);
			
			GetDialogItem(dialog, iTEXT_KILL_LIMIT, &item_type, &item, &bounds);
			getpstr(ptemporary, strSETUP_NET_GAME_MESSAGES, killsString);
			SetDialogItemText(item, ptemporary);
                        */
                        
			/* Untimed.. */
			setup_for_untimed_game(dialog);
			break;
			
		case _game_of_kill_monsters:
		case _game_of_king_of_the_hill:
		case _game_of_kill_man_with_ball:
		case _game_of_tag:
			// Benad
			modify_boolean_control(dialog, iFORCE_UNIQUE_TEAMS, CONTROL_ACTIVE, NONE);
			
			modify_boolean_control(dialog, iBURN_ITEMS_ON_DEATH, CONTROL_ACTIVE, false);
			modify_boolean_control(dialog, iUNLIMITED_MONSTERS, CONTROL_ACTIVE, NONE);

			set_limit_text(dialog, iRADIO_KILL_LIMIT, strSETUP_NET_GAME_MESSAGES, killLimitString,
                                        iTEXT_KILL_LIMIT, strSETUP_NET_GAME_MESSAGES, killsString);
                        /*
			GetDialogItem(dialog, iRADIO_KILL_LIMIT, &item_type, &item, &bounds);
			getpstr(ptemporary, strSETUP_NET_GAME_MESSAGES, killLimitString);
			SetControlTitle((ControlHandle) item, ptemporary);
			
			GetDialogItem(dialog, iTEXT_KILL_LIMIT, &item_type, &item, &bounds);
			getpstr(ptemporary, strSETUP_NET_GAME_MESSAGES, killsString);
			SetDialogItemText(item, ptemporary);
                        */

			setup_for_timed_game(dialog);
			break;

		case _game_of_capture_the_flag:
			// START Benad
			modify_boolean_control(dialog, iFORCE_UNIQUE_TEAMS, CONTROL_INACTIVE, true);
			modify_control_enabled(dialog, iGATHER_TEAM, CONTROL_ACTIVE);
			// END Benad
			/* Allow them to decide on the burn items on death */
			modify_boolean_control(dialog, iBURN_ITEMS_ON_DEATH, CONTROL_ACTIVE, false);
			modify_boolean_control(dialog, iUNLIMITED_MONSTERS, CONTROL_ACTIVE, NONE);

			set_limit_text(dialog, iRADIO_KILL_LIMIT, strSETUP_NET_GAME_MESSAGES, flagPullsString,
                                        iTEXT_KILL_LIMIT, strSETUP_NET_GAME_MESSAGES, flagsString);
                        /*
			GetDialogItem(dialog, iRADIO_KILL_LIMIT, &item_type, &item, &bounds);
			getpstr(ptemporary, strSETUP_NET_GAME_MESSAGES, flagPullsString);
			SetControlTitle((ControlHandle) item, ptemporary);
			
			GetDialogItem(dialog, iTEXT_KILL_LIMIT, &item_type, &item, &bounds);
			getpstr(ptemporary, strSETUP_NET_GAME_MESSAGES, flagsString);
			SetDialogItemText(item, ptemporary);
                        */
                        
			setup_for_timed_game(dialog);
			break;
			
		case _game_of_rugby:
			/* Allow them to decide on the burn items on death */
			modify_boolean_control(dialog, iBURN_ITEMS_ON_DEATH, CONTROL_ACTIVE, false);
			modify_boolean_control(dialog, iUNLIMITED_MONSTERS, CONTROL_ACTIVE, NONE);

			set_limit_text(dialog, iRADIO_KILL_LIMIT, strSETUP_NET_GAME_MESSAGES, pointLimitString,
                                        iTEXT_KILL_LIMIT, strSETUP_NET_GAME_MESSAGES, pointsString);
                        /*
			GetDialogItem(dialog, iRADIO_KILL_LIMIT, &item_type, &item, &bounds);
			getpstr(ptemporary, strSETUP_NET_GAME_MESSAGES, pointLimitString);
			SetControlTitle((ControlHandle) item, ptemporary);
			
			GetDialogItem(dialog, iTEXT_KILL_LIMIT, &item_type, &item, &bounds);
			getpstr(ptemporary, strSETUP_NET_GAME_MESSAGES, pointsString);
			SetDialogItemText(item, ptemporary);
                        */

			// START Benad
			// Disable "Allow teams", and force it to be checked.
			modify_boolean_control(dialog, iFORCE_UNIQUE_TEAMS, CONTROL_INACTIVE, true);
			modify_control_enabled(dialog, iGATHER_TEAM, CONTROL_ACTIVE);
			// END Benad
			setup_for_timed_game(dialog);
			break;

		case _game_of_defense:
			/* Allow them to decide on the burn items on death */
			// START Benad
			modify_boolean_control(dialog, iFORCE_UNIQUE_TEAMS, CONTROL_INACTIVE, true);
			modify_control_enabled(dialog, iGATHER_TEAM, CONTROL_ACTIVE);
			
			set_limit_text(dialog, iRADIO_KILL_LIMIT, strSETUP_NET_GAME_MESSAGES, timeOnBaseString,
                                        iTEXT_KILL_LIMIT, strSETUP_NET_GAME_MESSAGES, minutesString);
                        /*
			GetDialogItem(dialog, iRADIO_KILL_LIMIT, &item_type, &item, &bounds);
			getpstr(ptemporary, strSETUP_NET_GAME_MESSAGES, timeOnBaseString);
			SetControlTitle((ControlHandle) item, ptemporary);
			
			GetDialogItem(dialog, iTEXT_KILL_LIMIT, &item_type, &item, &bounds);
			getpstr(ptemporary, strSETUP_NET_GAME_MESSAGES, minutesString);
			SetDialogItemText(item, ptemporary);
                        */
			
			ShowDialogItem(dialog, iTEXT_KILL_LIMIT); ShowDialogItem(dialog, iKILL_LIMIT);
			ShowDialogItem(dialog, iTIME_LIMIT); ShowDialogItem(dialog, iTEXT_TIME_LIMIT);
			
			// END Benad
			modify_boolean_control(dialog, iBURN_ITEMS_ON_DEATH, CONTROL_ACTIVE, false);
			modify_boolean_control(dialog, iUNLIMITED_MONSTERS, CONTROL_ACTIVE, NONE);
			setup_for_timed_game(dialog);
			break;
			
		default:
			// LP change:
			assert(false);
			// halt();
			break;
	}

	return;
}



// ZZZ: new function for easier sharing etc.
void setup_for_score_limited_game(
	DialogPtr dialog)
{
//        modify_radio_button_family(dialog, iRADIO_NO_TIME_LIMIT, iRADIO_KILL_LIMIT, iRADIO_KILL_LIMIT);
        set_limit_type(dialog, iRADIO_KILL_LIMIT);
        
        // START Benad
        // ZZZ changed if condition:
        //if (network_preferences->game_type == _game_of_defense)
        if(get_selection_control_value(dialog, iGAME_TYPE) - 1 == _game_of_defense)
        {
                ShowDialogItem(dialog, iTIME_LIMIT); 
                ShowDialogItem(dialog, iTEXT_TIME_LIMIT);
                
                ShowDialogItem(dialog, iKILL_LIMIT);
                ShowDialogItem(dialog, iTEXT_KILL_LIMIT);
        }
        else
        {
                HideDialogItem(dialog, iTIME_LIMIT); 
                HideDialogItem(dialog, iTEXT_TIME_LIMIT);
                
                ShowDialogItem(dialog, iKILL_LIMIT);
                ShowDialogItem(dialog, iTEXT_KILL_LIMIT);
        }
        // END Benad
}

// ZZZ: moved here from network_dialogs_macintosh.cpp
void setup_for_untimed_game(
	DialogPtr dialog)
{
//	modify_radio_button_family(dialog, iRADIO_NO_TIME_LIMIT, iRADIO_KILL_LIMIT, iRADIO_NO_TIME_LIMIT);
        set_limit_type(dialog, iRADIO_NO_TIME_LIMIT);
	HideDialogItem(dialog, iKILL_LIMIT); HideDialogItem(dialog, iTEXT_KILL_LIMIT);
	HideDialogItem(dialog, iTIME_LIMIT); HideDialogItem(dialog, iTEXT_TIME_LIMIT);
}

// ZZZ: moved here from network_dialogs_macintosh.cpp
void setup_for_timed_game(
	DialogPtr dialog)
{
	// START Benad
	if (get_selection_control_value(dialog, iGAME_TYPE)-1 != _game_of_defense)
	{
		HideDialogItem(dialog, iTEXT_KILL_LIMIT); HideDialogItem(dialog, iKILL_LIMIT);
		ShowDialogItem(dialog, iTIME_LIMIT); ShowDialogItem(dialog, iTEXT_TIME_LIMIT);
	}
	else
	{
		ShowDialogItem(dialog, iTEXT_KILL_LIMIT); ShowDialogItem(dialog, iKILL_LIMIT);
		ShowDialogItem(dialog, iTIME_LIMIT); ShowDialogItem(dialog, iTEXT_TIME_LIMIT);
	}
	// END Benad
	//modify_radio_button_family(dialog, iRADIO_NO_TIME_LIMIT, iRADIO_KILL_LIMIT, iRADIO_TIME_LIMIT);
        set_limit_type(dialog, iRADIO_TIME_LIMIT);

	return;
}



/*************************************************************************************************
 *
 * Function: reassign_player_colors
 * Purpose:  This function used to reassign a player's color if it conflicted with another
 *           player's color. Now it reassigns everyone's colors. for the old function, see the
 *           obsoleted version (called check_player_info) at the end of this file.
 *           (Woody note: check_player_info can be found in network_dialogs_macintosh.cpp.)
 *
 *************************************************************************************************/
/* Note that we now only force unique colors across teams. */

// ZZZ: moved here (from network_dialogs_macintosh.cpp) so it can be shared with SDL version

void reassign_player_colors(
	short player_index,
	short num_players)
{
	short actual_colors[MAXIMUM_NUMBER_OF_PLAYERS];  // indexed by player
	bool colors_taken[NUMBER_OF_TEAM_COLORS];   // as opposed to desired team. indexed by team
	game_info *game;
	
	(void)(player_index);

	assert(num_players<=MAXIMUM_NUMBER_OF_PLAYERS);
	game= (game_info *)NetGetGameData();

	objlist_set(colors_taken, false, NUMBER_OF_TEAM_COLORS);
	objlist_set(actual_colors, NONE, MAXIMUM_NUMBER_OF_PLAYERS);

	if(game->game_options & _force_unique_teams)
	{
		short index;
		
		for(index= 0; index<num_players; ++index)
		{
			player_info *player= (player_info *)NetGetPlayerData(index);
			if(!colors_taken[player->desired_color])
			{
				player->color= player->desired_color;
				player->team= player->color;
				colors_taken[player->color]= true;
				actual_colors[index]= player->color;
			}
		}
		
		/* Now give them a random color.. */
		for (index= 0; index<num_players; index++)
		{
			player_info *player= (player_info *)NetGetPlayerData(index);

			if (actual_colors[index]==NONE) // This player needs a team
			{
				short remap_index;
				
				for (remap_index= 0; remap_index<num_players; remap_index++)
				{
					if (!colors_taken[remap_index])
					{
						player->color= remap_index;
						player->team= remap_index;
						colors_taken[remap_index] = true;
						break;
					}
				}
				assert(remap_index<num_players);
			}
		}	
	} else {
		short index;
		short team_color;
		
		/* Allow teams.. */
		for(team_color= 0; team_color<NUMBER_OF_TEAM_COLORS; ++team_color)
		{
			// let's mark everybody down for the teams that they can get without conflicts.
			for (index = 0; index < num_players; index++)
			{
				player_info *player= (player_info *)NetGetPlayerData(index);
		
				if (player->team==team_color && !colors_taken[player->desired_color])
				{
					player->color= player->desired_color;
					colors_taken[player->color] = true;
					actual_colors[index]= player->color;
				}
			}
			
			// ok, everyone remaining gets a team that we pick for them.
			for (index = 0; index < num_players; index++)
			{
				player_info *player= (player_info *)NetGetPlayerData(index);
	
				if (player->team==team_color && actual_colors[index]==NONE) // This player needs a team
				{
					short j;
					
					for (j = 0; j < num_players; j++)
					{
						if (!colors_taken[j])
						{
							player->color= j;
							colors_taken[j] = true;
							break;
						}
					}
					assert(j < num_players);
				}
			}
		}
	}
		
	return;
}



////////////////////////////////////////////////////////////////////////////////
// Postgame Carnage Report stuff
struct net_rank rankings[MAXIMUM_NUMBER_OF_PLAYERS];

#if 0
// These were used for an array-lookup-based find_graph_mode, which worked but was later
// abandoned in favor of the original (very slightly modified to "add back in" separator indices).
// Left here for the curious.

// This should not conflict with the other _*_graph identifiers
enum { _total_team_carnage_or_total_scores_graph = 4242 };

// We lookup into a menu contents array now
static int	sMenuContents[] =
#ifdef mac
{
    NONE,	// separator
    _total_carnage_graph,
    _total_team_carnage_or_total_scores_graph,
    NONE,	// separator
    _total_team_carnage_graph,
    _total_team_scores_graph
};
#else // !mac
{
    _total_carnage_graph,
    _total_team_carnage_or_total_scores_graph,
    _total_team_carnage_graph,
    _total_team_scores_graph
};
#endif // !mac
#endif // 0

// (ZZZ annotation:) Figure out which graph type the user wants to display based
// on his selection from the popup/selection control.  (See also draw_new_graph().)
short
find_graph_mode(
	DialogPtr dialog,
	short *index)
{
	short value;
	short graph_type = NONE;
	bool has_scores;
	
	has_scores= current_net_game_has_scores();
	
	/* Popups are 1 based */
	value = get_selection_control_value(dialog, iGRAPH_POPUP)-1;
	if(value<dynamic_world->player_count)
	{
		if(index) *index= value;
		graph_type= _player_graph;
	} 
	else 
	{
#if 0
            // alternate method based on array lookup, works but abandoned.
            // left here for the curious.

                // ZZZ change: lookup graph type from static array
                int theIndexAfterPlayers = value - dynamic_world->player_count;
                int theNumberOfMenuItemsAfterPlayers = (sizeof(sMenuContents) / sizeof(sMenuContents[0]));
                
                // Make sure the index is sane
                assert(theIndexAfterPlayers >= 0 && theIndexAfterPlayers < theNumberOfMenuItemsAfterPlayers);
                
                // Do the lookup
                graph_type = sMenuContents[theIndexAfterPlayers];
                
                // Make sure graph type is sane
                assert(graph_type != NONE);
                
                bool	isTeamGame = ((GET_GAME_OPTIONS() & _force_unique_teams) ? false : true);

                // Disambiguate
                if(graph_type == _total_team_carnage_or_total_scores_graph)
                    graph_type = has_scores ? _total_scores_graph : _total_team_carnage_graph;

                // Sanity check the graph type
                if(!isTeamGame) {
                    assert(graph_type != _total_team_carnage_graph);
                    assert(graph_type != _total_team_scores_graph);
                }
                
                if(!has_scores) {
                    assert(graph_type != _total_scores_graph);
                    assert(graph_type != _total_team_scores_graph);
                }
                
#else // !0

                int theValueAfterPlayers = value-dynamic_world->player_count;
#ifndef mac
                // ZZZ: Account for (lack of) separators
                if(theValueAfterPlayers >= 0)	theValueAfterPlayers++;
                if(theValueAfterPlayers >= 3)	theValueAfterPlayers++;
#endif
                
		/* Different numbers of items based on game type. */
		switch(theValueAfterPlayers)
		{
			case 0:
				/* Separator line */
				// LP change:
				assert(false);
				// halt();
				break;
		
			case 1: /* FIrst item after the players. */
				graph_type= _total_carnage_graph; /* Always.. */
				break;
			
			case 2: /* May be either: _total_scores or _total_team_carnage */
				if(has_scores)
				{
					graph_type= _total_scores_graph;
				} else {
					assert(!(GET_GAME_OPTIONS() & _force_unique_teams));
					graph_type= _total_team_carnage_graph;
				}
				break;
				
			case 3:
				/* Separator line */
				// LP change:
				assert(false);
				// halt();
				break;
				
			case 4:	
				assert(!(GET_GAME_OPTIONS() & _force_unique_teams));
				graph_type= _total_team_carnage_graph;
				break;
				
			case 5:
				assert(has_scores);
				graph_type= _total_team_scores_graph;
				break;
				
			default:
				// LP change:
				assert(false);
				// halt();
				break;
		}
#endif // !0
	}

	return graph_type;
}



// (ZZZ annotation:) Fill in array of net_rank with total carnage values, individual scores, 
// colors, etc.  Note that team-by-team rankings (draw_team_*_graph()) and player vs. 
// player rankings (draw_player_graph()) use their own local ranks[] arrays instead.
// The team-by-team rankings are computed from these; the player vs. player are not.
void calculate_rankings(
	struct net_rank *ranks, 
	short num_players)
{
	short player_index;
	
	for(player_index= 0; player_index<num_players; ++player_index)
	{
		ranks[player_index].player_index= player_index;
		ranks[player_index].color= get_player_data(player_index)->color;
		ranks[player_index].game_ranking= get_player_net_ranking(player_index, 
			&ranks[player_index].kills,
			&ranks[player_index].deaths, true);
		ranks[player_index].ranking= ranks[player_index].kills-ranks[player_index].deaths;
	}

	return;
}


// (ZZZ annotation:) Individual carnage totals comparison for sorting.
int rank_compare(
	void const *r1, 
	void const *r2)
{
	struct net_rank const *rank1=(struct net_rank const *)r1;
	struct net_rank const *rank2=(struct net_rank const *)r2;
	int diff;
	struct player_data *p1, *p2;
	
	diff = rank2->ranking - rank1->ranking;
	
    // (ZZZ annotation:) Tiebreaker: which player did more killing (and thus dying)?
    if (diff == 0)
	{
		// i have to resort to looking here because the information may not be contained
		// in the rank structure if we're not displaying the totals graph. 
		p1 = get_player_data(rank1->player_index);
		p2 = get_player_data(rank2->player_index);
		diff = p2->total_damage_given.kills - p1->total_damage_given.kills;
	}
	
	return diff;
}

// (ZZZ annotation:) Team carnage totals comparison for sorting.
// Same as rank_compare(), but without tiebreaking.
int team_rank_compare(
	void const *rank1, 
	void const *rank2)
{
	return ((struct net_rank const *)rank2)->ranking
		  -((struct net_rank const *)rank1)->ranking;
}

// (ZZZ annotation:) Game-specific score comparison for sorting.
int score_rank_compare(
	void const *rank1, 
	void const *rank2)
{
	return ((struct net_rank const *)rank2)->game_ranking
		  -((struct net_rank const *)rank1)->game_ranking;
}



// (ZZZ annotation:) Graph of player's killing performance vs each other player
void draw_player_graph(
	DialogPtr dialog, 
	short index)
{
	short key_player_index= rankings[index].player_index;
	struct player_data *key_player= get_player_data(key_player_index);
	struct net_rank ranks[MAXIMUM_NUMBER_OF_PLAYERS];
	short loop;

	/* Copy in the total ranks. */	
	for(loop= 0; loop<dynamic_world->player_count; ++loop)
	{
		short test_player_index= rankings[loop].player_index;
		struct player_data *player= get_player_data(test_player_index);
	
		/* Copy most of the data */
		ranks[loop]= rankings[loop];
		
		/* How many times did I kill this guy? */
		ranks[loop].kills= player->damage_taken[key_player_index].kills;

		/* How many times did this guy kill me? */
		ranks[loop].deaths= key_player->damage_taken[test_player_index].kills;
	}

	draw_names(dialog, ranks, dynamic_world->player_count, index);
	draw_kill_bars(dialog, ranks, dynamic_world->player_count, index, false, false);
	
	return;
}


// ZZZ: team vs team carnage (analogous to draw_player_graph's player vs player carnage)
// THIS IS UNFINISHED (and thus unused at the moment :) )
void draw_team_graph(
	DialogPtr dialog, 
	short team_index)
{
    // ZZZZZZ this is where I add my team vs team ranking computation.  Yay.
    // We'll just fill in the rank structures straight, then count the teams later.
    struct net_rank team_ranks[NUMBER_OF_TEAM_COLORS];

    objlist_clear(team_ranks, NUMBER_OF_TEAM_COLORS);
    
	/* Loop across players on the reference team */
	for(int ref_player_index = 0; ref_player_index < dynamic_world->player_count; ref_player_index++)
	{
//		short test_player_index= rankings[loop].player_index;
		struct player_data *ref_player= get_player_data(ref_player_index);

        if(ref_player->team != team_index)
            continue;

	    /* Loop across all players */
	    for(int player_index = 0; player_index < dynamic_world->player_count; player_index++)
	    {
    //		short test_player_index= rankings[loop].player_index;
		    struct player_data *player= get_player_data(player_index);

            team_ranks[player->team].player_index   = NONE;
            team_ranks[player->team].color          = player->team;
            team_ranks[player->team].kills  += player->damage_taken[ref_player_index].kills;
            team_ranks[player->team].deaths += ref_player->damage_taken[player_index].kills;
        } // all players
    } // players on reference team

    // Condense into the first group of slots in the rankings
    // NOTE ideally these will be ordered the same way the team_total_carnage rankings are.

    // Draw the bars
//	draw_names(dialog, team_ranks, dynamic_world->player_count, index);
//	draw_kill_bars(dialog, team_ranks, dynamic_world->player_count, index, false, false);
	
	return;
}



// (ZZZ annotation:) Total Carnage graph
void draw_totals_graph(
	DialogPtr dialog)
{
	draw_names(dialog, rankings, dynamic_world->player_count, NONE);
	draw_kill_bars(dialog, rankings, dynamic_world->player_count, NONE, true, false);

	return;
}



// (ZZZ annotation:) Total Team Carnage graph
void draw_team_totals_graph(
	DialogPtr dialog)
{
	short team_index, player_index, opponent_player_index, num_teams;
	bool found_team_of_current_color;
	struct net_rank ranks[MAXIMUM_NUMBER_OF_PLAYERS];
	
	// first of all, set up the rankings array.
	objlist_clear(ranks, MAXIMUM_NUMBER_OF_PLAYERS);
	for (team_index= 0, num_teams = 0; team_index<NUMBER_OF_TEAM_COLORS; team_index++)
	{
		found_team_of_current_color = false;

		for (player_index= 0; player_index<dynamic_world->player_count; player_index++)
		{
			struct player_data *player = get_player_data(player_index);

			if (player->team==team_index)
			{
				found_team_of_current_color= true;
				ranks[num_teams].player_index= NONE;
				ranks[num_teams].color= team_index;
				
				// calculate how many kills this player scored for his team.
				for (opponent_player_index= 0; opponent_player_index<dynamic_world->player_count; opponent_player_index++)
				{
					struct player_data *opponent = get_player_data(opponent_player_index);
					if (player->team != opponent->team)
					{
						ranks[num_teams].kills += opponent->damage_taken[player_index].kills;
					}
					else
					{
						ranks[num_teams].friendly_fire_kills += opponent->damage_taken[player_index].kills;
					}
				}
				
				// then calculate how many deaths this player had
				for (opponent_player_index= 0; opponent_player_index<dynamic_world->player_count; opponent_player_index++)
				{
					ranks[num_teams].deaths += player->damage_taken[opponent_player_index].kills;
				}
			}
		}
		
		if (found_team_of_current_color) num_teams++;
	}

	/* Setup the team rankings.. */
	for (team_index= 0; team_index<num_teams; team_index++)
	{
		ranks[team_index].ranking= ranks[team_index].kills - ranks[team_index].deaths;
	}
	qsort(ranks, num_teams, sizeof(struct net_rank), team_rank_compare);
	
	draw_names(dialog, ranks, num_teams, NONE);
	draw_kill_bars(dialog, ranks, num_teams, NONE, true, true);
	
	return;
}



// (ZZZ annotation:) Time on Hill, etc. graph
void draw_total_scores_graph(
	DialogPtr dialog)
{
	struct net_rank ranks[MAXIMUM_NUMBER_OF_PLAYERS];
	
	/* Use a private copy to avoid boning things */
	objlist_copy(ranks, rankings, dynamic_world->player_count);

	/* First qsort the rankings arrray by game_ranking.. */
	qsort(ranks, dynamic_world->player_count, sizeof(struct net_rank), score_rank_compare);

	/* Draw the names. */
	draw_names(dialog, ranks, dynamic_world->player_count, NONE);
	
	/* And draw the bars... */
	draw_score_bars(dialog, ranks, dynamic_world->player_count);
}



// (ZZZ annotation:) Team Time on Hill, etc. graph
void draw_team_total_scores_graph(
	DialogPtr dialog)
{
	short team_index, team_count;
	struct net_rank ranks[MAXIMUM_NUMBER_OF_PLAYERS];
	
	// first of all, set up the rankings array.
	objlist_clear(ranks, MAXIMUM_NUMBER_OF_PLAYERS);
	team_count= 0;
	
	for(team_index= 0; team_index<NUMBER_OF_TEAM_COLORS; ++team_index)
	{
		short ranking_index;
		bool team_is_valid= false;
		
		for(ranking_index= 0; ranking_index<dynamic_world->player_count; ++ranking_index)
		{
			struct player_data *player= get_player_data(rankings[ranking_index].player_index);
			
			if(player->team==team_index)
			{
				ranks[team_count].player_index= NONE;
				ranks[team_count].color= team_index;
				ranks[team_count].game_ranking+= rankings[ranking_index].game_ranking;
				team_is_valid= true;
			}
		}
		
		if(team_is_valid) team_count++;
	}

	/* Now qsort our team stuff. */
	qsort(ranks, team_count, sizeof(struct net_rank), score_rank_compare);
	
	/* And draw the bars.. */
	draw_names(dialog, ranks, team_count, NONE);
	draw_score_bars(dialog, ranks, team_count);
}



// (ZZZ) ripped this out of draw_kill_bars since we can share this bit but not the rest of draw_kill_bars().
// (ZZZ annotation:) Update the "N deaths total (D dpm) including S suicides"-type text at the bottom.
void
update_carnage_summary(                 	
    DialogPtr dialog, 
	struct net_rank *ranks, 
	short num_players, 
	short suicide_index, 
	bool do_totals, 
	bool friendly_fire)
{
    short   i;
    short   num_suicides;
    float   minutes;
    float   kpm;
    float   dpm;
    long    total_kills;
    long    total_deaths;
	char    kill_string_format[65];
    char    death_string_format[65];
    char    suicide_string_format[65];
    
    for (i = total_kills = total_deaths = 0; i < num_players; i++)
	{
		if (do_totals || i != suicide_index)
			total_kills += ranks[i].kills;
		total_deaths += ranks[i].deaths;
	}
	if (do_totals)
	{
		for (i = num_suicides = 0; i < num_players; i++)
		{
			if (friendly_fire)
				num_suicides += ranks[i].friendly_fire_kills;
			else
				num_suicides += (players+i)->damage_taken[i].kills;
		}
	}
	else
		num_suicides = ranks[suicide_index].kills;

#ifdef mac
	TextFace(0); TextFont(0); TextSize(0);
#endif

	minutes = ((float)dynamic_world->tick_count / TICKS_PER_SECOND) / 60.0;
	if (minutes > 0) kpm = total_kills / minutes;
	else kpm = 0;
	getcstr(kill_string_format, strNET_STATS_STRINGS, strTOTAL_KILLS_STRING);
	psprintf(ptemporary, kill_string_format, total_kills, kpm);
//	GetDialogItem(dialog, iTOTAL_KILLS, &item_type, &item_handle, &item_rect);
//	SetDialogItemText(item_handle, ptemporary);
    
    copy_pstring_to_static_text(dialog, iTOTAL_KILLS, ptemporary);

	if (minutes > 0) dpm = total_deaths / minutes;
	else dpm = 0;
	getcstr(death_string_format, strNET_STATS_STRINGS, strTOTAL_DEATHS_STRING);
	
	if (num_suicides)
	{
		if (friendly_fire)
			getcstr(suicide_string_format, strNET_STATS_STRINGS, strFRIENDLY_FIRE_STRING);
		else
			getcstr(suicide_string_format, strNET_STATS_STRINGS, strINCLUDING_SUICIDES_STRING);
		strcat(death_string_format, suicide_string_format);
		psprintf(ptemporary, death_string_format, total_deaths, dpm, num_suicides);
	}
	else
		psprintf(ptemporary, death_string_format, total_deaths, dpm);
//	GetDialogItem(dialog, iTOTAL_DEATHS, &item_type, &item_handle, &item_rect);
//	SetDialogItemText(item_handle, ptemporary);

    copy_pstring_to_static_text(dialog, iTOTAL_DEATHS, ptemporary);
}



// ZZZ: ripped out of update_damage_item
// (ZZZ annotation:) Demultiplex to draw_X_graph() function based on find_graph_mode().
void
draw_new_graph(DialogPtr dialog) {
    short   graph_type;
    short   index;

    graph_type= find_graph_mode(dialog, &index);
	
	switch(graph_type)
	{
		case _player_graph:
			draw_player_graph(dialog, index);
			break;
			
		case _total_carnage_graph:
			draw_totals_graph(dialog);
			break;
			
		case _total_scores_graph:
			draw_total_scores_graph(dialog);
			break;

		/* These two functions need to have the team colors. */
		case _total_team_carnage_graph:
			draw_team_totals_graph(dialog);
			break;
			
		case _total_team_scores_graph:
			draw_team_total_scores_graph(dialog);
			break;
			
		default:
			// LP change:
			assert(false);
			// halt();
			break;
	}
}

// (ZZZ annotation:) Find the highest player vs. player kills
// (so all player vs. player graphs can be at the same scale)
short calculate_max_kills(
	short num_players)
{
	short  max_kills = 0;
	short  i, j;
	
	for (i = 0; i < num_players; i++)
	{
		for (j = 0; j < num_players; j++)
		{
			struct player_data *player= get_player_data(i);
			
			if (player->damage_taken[j].kills > max_kills)
			{
				max_kills= player->damage_taken[j].kills;
			}
		}
	}
	
	return max_kills;
}



// (ZZZ annotation:) Get postgame bar color for _suicide_color, etc.
/* If alain wasn't a tool, this would be in a resource.. */
void get_net_color(
	short index,
	RGBColor *color)
{
	switch(index)
	{
		case _suicide_color:
			color->red= color->green= LONG_MAX;
			color->blue= 0;
			break;
		case _kill_color:
			color->red= LONG_MAX;
			color->green= color->blue= 0;
			break;
		case _death_color:
		case _score_color:
			color->red= color->green= color->blue= 60000;
			break;
		default:
			// LP change:
			assert(false);
			// halt();
			break;
	}
}
