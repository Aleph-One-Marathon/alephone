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

Mar 1, 2002 (Woody Zenfell):
    Reworked SDL dialog-box level-choosing code; interface is different now (can't use
    get_selection_control_value(), gives and takes level numbers instead).  SDL prefs now store
    level number instead of menu index.  Using #ifdef mac to decide which interface to use.

Mar 8, 2002 (Woody Zenfell):
    Network microphone UI is now handled for SDL version as well (since SDL has net-audio now)
    
Feb 12, 2003 (Woody Zenfell):
    Support for resuming netgames (optionally get game options from saved-game not prefs, optionally don't save options into prefs)

Apr 10, 2003 (Woody Zenfell):
    Join hinting and autogathering have Preferences entries now

 August 27, 2003 (Woody Zenfell):
	Reworked netscript selection stuff to use Preferences and to be more cross-platform
	and more consistent with other dialog code
*/

#include	"cseries.h"
#include	"map.h"
#include	"shell.h"
#include	"preferences.h"
#include	"network.h"
#include	"network_dialogs.h"
#include	"network_games.h"
#include    "player.h" // ZZZ: for MAXIMUM_NUMBER_OF_PLAYERS, for reassign_player_colors

// For LAN netgame location services
#include	<sstream>
#include	"network_private.h" // actually just need "network_dialogs_private.h"
#include	"SSLP_API.h"
extern void NetRetargetJoinAttempts(const IPaddress* inAddress);


#ifdef HAVE_SDL_NET

// For the recent host addresses
#include <list>

struct HostName_Type
{
	// C string
	char Name[kJoinHintingAddressLength+1];
};

static list<HostName_Type> RecentHostAddresses;
static list<HostName_Type>::iterator RHAddrIter = RecentHostAddresses.end();

#endif


static uint16 get_dialog_game_options(DialogPtr dialog, short game_type);
static void set_dialog_game_options(DialogPtr dialog, uint16 game_options);

#ifdef USES_NIBS

static uint16 GetDialogGameOptions(NetgameSetupData& Data, short game_type);
static void SetDialogGameOptions(NetgameSetupData& Data, uint16 game_options);

#endif



////////////////////////////////////////////////////////////////////////////////
// LAN game-location services support

static const string
get_sslp_service_type()
{
	stringstream ss;
	ss << "A1 Gatherer V" << get_network_version();
	return ss.str();
}


GathererAvailableAnnouncer::GathererAvailableAnnouncer()
{
	strncpy(mServiceInstance.sslps_type, get_sslp_service_type().c_str(), SSLP_MAX_TYPE_LENGTH);
	strncpy(mServiceInstance.sslps_name, "Boomer", SSLP_MAX_NAME_LENGTH);
	memset(&(mServiceInstance.sslps_address), '\0', sizeof(mServiceInstance.sslps_address));
	SSLP_Allow_Service_Discovery(&mServiceInstance);
}

GathererAvailableAnnouncer::~GathererAvailableAnnouncer()
{
	SSLP_Disallow_Service_Discovery(&mServiceInstance);
}

void // static
GathererAvailableAnnouncer::pump()
{
	SSLP_Pump();
}


JoinerSeekingGathererAnnouncer::JoinerSeekingGathererAnnouncer(bool shouldSeek) : mShouldSeek(shouldSeek)
{
	if(mShouldSeek)
		SSLP_Locate_Service_Instances(
				get_sslp_service_type().c_str(),
				found_gatherer_callback,
				lost_gatherer_callback,
				found_gatherer_callback
				);
}

JoinerSeekingGathererAnnouncer::~JoinerSeekingGathererAnnouncer()
{
	if(mShouldSeek)
		SSLP_Stop_Locating_Service_Instances(get_sslp_service_type().c_str());
}

void // static
JoinerSeekingGathererAnnouncer::pump()
{
	SSLP_Pump();
}

void // static
JoinerSeekingGathererAnnouncer::found_gatherer_callback(const SSLP_ServiceInstance* instance)
{
	NetRetargetJoinAttempts(&instance->sslps_address);
}

void // static
JoinerSeekingGathererAnnouncer::lost_gatherer_callback(const SSLP_ServiceInstance* instance)
{
	NetRetargetJoinAttempts(NULL);
}




/*************************************************************************************************
 *
 * Function: extract_setup_dialog_information
 * Purpose:  extract all the information that we can squeeze out of the game setup dialog
 *
 *************************************************************************************************/

#ifdef USES_NIBS

static long GetCtrlNumber(ControlRef Ctrl)
{
	GetEditPascalText(Ctrl, ptemporary);
	long Num;
	StringToNum(ptemporary, &Num);
	return Num;
}

void NetgameSetup_Extract(
	NetgameSetupData& Data,
	player_info *player_information,
	game_info *game_information,
	short game_limit_type,
	bool allow_all_levels,
	bool ResumingGame
	)
{
	short               updates_per_packet, update_latency;
	struct entry_point  entry;
	long entry_flags;
	
	// get player information
	GetEditPascalText(Data.PlayerNameCtrl,ptemporary);
	if (*temporary > MAX_NET_PLAYER_NAME_LENGTH) 
		*temporary = MAX_NET_PLAYER_NAME_LENGTH;
	pstrcpy(player_information->name, ptemporary);
	player_information->color= GetControl32BitValue(Data.PlayerColorCtrl) - 1;
	player_information->team= GetControl32BitValue(Data.PlayerTeamCtrl) - 1;

	pstrcpy(player_preferences->name, player_information->name);
	player_preferences->color= player_information->color;
	player_preferences->team= player_information->team;

	game_information->server_is_playing = true;
	game_information->net_game_type= GetControl32BitValue(Data.GameTypeCtrl)-1;

	// get game information
	game_information->game_options= GetDialogGameOptions(Data, game_information->net_game_type);
	
 	// ZZZ: don't screw with the limits if resuming.
 	if(ResumingGame)
 	{
		game_information->time_limit = dynamic_world->game_information.game_time_remaining;
 		game_information->kill_limit = dynamic_world->game_information.kill_limit;
	}
	else
	{
		if (game_limit_type == duration_no_time_limit)
		{
			game_information->time_limit = LONG_MAX;
		}
		else if (game_limit_type == duration_kill_limit)
		{
			// START Benad
			if (GetControl32BitValue(Data.GameTypeCtrl)-1 == _game_of_defense)
			{
				game_information->game_options |= _game_has_kill_limit;
				game_information->time_limit = GetCtrlNumber(Data.TimeTextCtrl);
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
			if (GetControl32BitValue(Data.GameTypeCtrl)-1 == _game_of_defense)
			{
				game_information->game_options |= _game_has_kill_limit;
				game_information->time_limit = GetCtrlNumber(Data.TimeTextCtrl);
				game_information->time_limit *= TICKS_PER_SECOND * 60;
			}
			else
			{
				game_information->time_limit = GetCtrlNumber(Data.TimeTextCtrl);
				game_information->time_limit *= TICKS_PER_SECOND * 60;
			}
			// END Benad
		}
		game_information->kill_limit = GetCtrlNumber(Data.KillsTextCtrl);
		// START Benad
		if (GetControl32BitValue(Data.GameTypeCtrl)-1 == _game_of_defense)
  			game_information->kill_limit *= 60; // It's "Time On Hill" limit, in seconds.
		// END Benad
        }
        
	/* Determine the entry point flags by the game type. */
    if(allow_all_levels)
	{
		entry_flags= NONE;
	} else {
		entry_flags= get_entry_point_flags_for_game_type(game_information->net_game_type);
	}
#ifdef mac
	menu_index_to_level_entry(GetControl32BitValue(Data.EntryPointCtrl), entry_flags, &entry);
#else
    get_selected_entry_point(dialog, iENTRY_MENU, &entry);
#endif
	game_information->level_number = entry.level_number;
	strcpy(game_information->level_name, entry.level_name);
	game_information->difficulty_level = GetControl32BitValue(Data.DifficultyCtrl)-1;
	
	game_information->allow_mic = GetControl32BitValue(Data.UseMicrophoneCtrl);
	network_preferences->use_speex_encoder =
		GetControl32BitValue(Data.MicrophoneTypeCtrl) == mic_type_speex;
	
#if HAVE_SDL_NET
	updates_per_packet = 1;
	update_latency = 0;
#endif
	vassert(updates_per_packet > 0 && update_latency >= 0 && updates_per_packet < 16,
		csprintf(temporary, "You idiot! updates_per_packet = %d, update_latency = %d", updates_per_packet, update_latency));
	game_information->initial_updates_per_packet = updates_per_packet;
	game_information->initial_update_latency = update_latency;
	NetSetInitialParameters(updates_per_packet, update_latency);
	
	game_information->initial_random_seed = ResumingGame ? dynamic_world->random_seed : (uint16) machine_tick_count();

	// allow all cheats
	game_information->cheat_flags = 
	  _allow_crosshair | 
	  _allow_tunnel_vision |
	  _allow_behindview;

	// now save some of these to the preferences - only if not resuming a game
	if(!ResumingGame)
	{
		network_preferences->game_type= game_information->net_game_type;
		
		network_preferences->allow_microphone = game_information->allow_mic;
		network_preferences->difficulty_level = game_information->difficulty_level;
#ifdef mac
		 network_preferences->entry_point= GetControl32BitValue(Data.EntryPointCtrl)-1;
#else
		network_preferences->entry_point = entry.level_number;
#endif
		network_preferences->game_options = game_information->game_options;
		network_preferences->time_limit = GetCtrlNumber(Data.TimeTextCtrl)*60*TICKS_PER_SECOND;
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
	}

	// We write the preferences here (instead of inside the if) because they may have changed
	// their player name/color/etc. and we do want to save that change.
	write_preferences();

	/* Don't save the preferences of their team... */
	if(TEST_FLAG(game_information->game_options, _force_unique_teams))
	{
		player_information->team= player_information->color;
	}
}

#else

void
extract_setup_dialog_information(
	DialogPtr dialog,
	player_info *player_information,
	game_info *game_information,
	short game_limit_type,
	bool allow_all_levels,
        bool resuming_game)
{
#if !HAVE_SDL_NET
	short               network_speed;
#endif
	short               updates_per_packet, update_latency;
	struct entry_point  entry;
	long entry_flags;
        
	// get player information
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

        // ZZZ: don't screw with the limits if resuming.
        if(resuming_game)
        {
                game_information->time_limit = dynamic_world->game_information.game_time_remaining;
                game_information->kill_limit = dynamic_world->game_information.kill_limit;
        }
        else
        {
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
        }
        
	/* Determine the entry point flags by the game type. */
    if(allow_all_levels)
	{
		entry_flags= NONE;
	} else {
		entry_flags= get_entry_point_flags_for_game_type(game_information->net_game_type);
	}
#ifdef mac
	menu_index_to_level_entry(get_selection_control_value(dialog, iENTRY_MENU), entry_flags, &entry);
#else
    get_selected_entry_point(dialog, iENTRY_MENU, &entry);
#endif
	game_information->level_number = entry.level_number;
	strcpy(game_information->level_name, entry.level_name);
	game_information->difficulty_level = get_selection_control_value(dialog, iDIFFICULTY_MENU)-1;

	game_information->allow_mic = get_boolean_control_value(dialog, iREAL_TIME_SOUND);


#if HAVE_SDL_NET
	updates_per_packet = 1;
	update_latency = 0;
#endif
	vassert(updates_per_packet > 0 && update_latency >= 0 && updates_per_packet < 16,
		csprintf(temporary, "You idiot! updates_per_packet = %d, update_latency = %d", updates_per_packet, update_latency));
	game_information->initial_updates_per_packet = updates_per_packet;
	game_information->initial_update_latency = update_latency;
	NetSetInitialParameters(updates_per_packet, update_latency);
	
	game_information->initial_random_seed = resuming_game ? dynamic_world->random_seed : (uint16) machine_tick_count();

	bool shouldUseNetscript = get_boolean_control_value(dialog, iUSE_SCRIPT);
	FileSpecifier theNetscriptFile = get_dialog_netscript_file(dialog);

	// This will be set true below if appropriate
	SetNetscriptStatus(false);
	
	if(shouldUseNetscript)
	{
		OpenedFile script_file;

		if(theNetscriptFile.Open (script_file))
		{
			long script_length;
			script_file.GetLength (script_length);

			// DeferredScriptSend will delete this storage the *next time* we call it (!)
			byte* script_buffer = new byte [script_length];
			
			if (script_file.Read (script_length, script_buffer))
			{
				DeferredScriptSend (script_buffer, script_length);
				SetNetscriptStatus (true);
			}
			
			script_file.Close ();
		}
		else
			// hmm failing quietly is probably not the best course of action, but ...
			shouldUseNetscript = false;
	}

	// allow all cheats
	game_information->cheat_flags = _allow_crosshair | 
					_allow_tunnel_vision |
					_allow_behindview;

	// now save some of these to the preferences - only if not resuming a game
        if(!resuming_game)
        {
                network_preferences->game_type= game_information->net_game_type;

#if !HAVE_SDL_NET
                network_preferences->type = network_speed; 
#endif
                network_preferences->allow_microphone = game_information->allow_mic;
                network_preferences->difficulty_level = game_information->difficulty_level;
#ifdef mac
                network_preferences->entry_point= get_selection_control_value(dialog, iENTRY_MENU)-1;
#else
                network_preferences->entry_point = entry.level_number;
#endif
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

		network_preferences->use_netscript = shouldUseNetscript;
		if(shouldUseNetscript)
		{
			// Sorry, probably should use a FileSpecifier in the prefs,
			// but that means prefs reading/writing have to be reworked instead
#ifdef SDL
			strncpy(network_preferences->netscript_file, theNetscriptFile.GetPath(), sizeof(network_preferences->netscript_file));
#else
			network_preferences->netscript_file = theNetscriptFile.GetSpec();
#endif
		}
        }

        // We write the preferences here (instead of inside the if) because they may have changed
        // their player name/color/etc. and we do want to save that change.
        write_preferences();

        /* Don't save the preferences of their team... */
	if(game_information->game_options & _force_unique_teams)
	{
		player_information->team= player_information->color;
	}
}

#endif


/*************************************************************************************************
 *
 * Function: fill_in_game_setup_dialog
 * Purpose:  setup the majority of the game setup dialog.
 *
 *************************************************************************************************/

#ifdef USES_NIBS

short NetgameSetup_FillIn(
	NetgameSetupData& Data,
	player_info *player_information,
	bool allow_all_levels,
	bool ResumingGame
	)
{
	// Derived from Woody Zenfell's modifications to the original code:
	
	// We use a temporary structure so that we can change things without messing with the real preferences
	network_preferences_data theAdjustedPreferences = *network_preferences;

	if (ResumingGame)
	{
		// Adjust the apparent preferences to get values from the loaded game (dynamic_world)
		// rather than from the actual network_preferences.
		theAdjustedPreferences.game_type = GET_GAME_TYPE();
		theAdjustedPreferences.difficulty_level = dynamic_world->game_information.difficulty_level;
		theAdjustedPreferences.entry_point = dynamic_world->current_level_number;
		theAdjustedPreferences.kill_limit = dynamic_world->game_information.kill_limit;
		theAdjustedPreferences.time_limit = dynamic_world->game_information.game_time_remaining;
		theAdjustedPreferences.game_options = GET_GAME_OPTIONS();
		// If the time limit is longer than a week, we figure it's untimed (  ;)
		theAdjustedPreferences.game_is_untimed = (dynamic_world->game_information.game_time_remaining > 7 * 24 * 3600 * TICKS_PER_SECOND);
		// If they are resuming a single-player game, assume they want cooperative play now.
		if(dynamic_world->player_count == 1 && GET_GAME_TYPE() == _game_of_kill_monsters)
		{
			theAdjustedPreferences.game_type = _game_of_cooperative_play;
			theAdjustedPreferences.game_options |= _live_network_stats; // single-player game doesn't, and they probably want it
			}
		allow_all_levels = true;
        }
        
	/* Fill in the entry points */
	long entry_flags;
	if(allow_all_levels)
	{
		entry_flags= NONE;
	} else {
		entry_flags= get_entry_point_flags_for_game_type(theAdjustedPreferences.game_type);
	}
	
    // ZZZ: SDL version takes care of this its own way, simpler not to change.
#ifdef mac
	EntryPoints_FillIn(Data.EntryPointCtrl, entry_flags, NONE);
#endif
	
	SetControl32BitValue(Data.GameTypeCtrl, theAdjustedPreferences.game_type+1);
	NetgameSetup_GameType(Data, theAdjustedPreferences.game_type);
	
	/* set up the name of the player. */
	short name_length= player_preferences->name[0];
	if(name_length>MAX_NET_PLAYER_NAME_LENGTH) name_length= MAX_NET_PLAYER_NAME_LENGTH;
        // XXX (ZZZ): it looks to me that this forgets to set player_information->name[0] to name_length?
        // not sure if this is a problem in practice, but the Bungie guys were usually pretty careful about this sort of thing.
	memcpy(player_information->name, player_preferences->name, name_length+1);
	
	SetEditPascalText(Data.PlayerNameCtrl, player_information->name);
 
 #ifdef mac
	// SelectDialogItemText(dialog, iGATHER_NAME, 0, INT16_MAX);
#endif

	/* Set the menu values */
	SetControl32BitValue(Data.PlayerColorCtrl, player_preferences->color+1);
	SetControl32BitValue(Data.PlayerTeamCtrl, player_preferences->team+1);
	SetControl32BitValue(Data.DifficultyCtrl, theAdjustedPreferences.difficulty_level+1);
	SetControl32BitValue(Data.EntryPointCtrl, theAdjustedPreferences.entry_point+1);

	// START Benad
	if (theAdjustedPreferences.game_type == _game_of_defense)
		NumToString(theAdjustedPreferences.kill_limit/60, ptemporary);
	else
		NumToString(theAdjustedPreferences.kill_limit, ptemporary);
	SetEditPascalText(Data.KillsTextCtrl,ptemporary);
	// END Benad
	
	// If resuming an untimed game, show the "time limit" from the prefs in the grayed-out widget
	// rather than some ridiculously large number
	NumToString(
		((ResumingGame && theAdjustedPreferences.game_is_untimed) ?
			network_preferences->time_limit :
				theAdjustedPreferences.time_limit)/TICKS_PER_SECOND/60,
					ptemporary);
	SetEditPascalText(Data.TimeTextCtrl,ptemporary);

	if (theAdjustedPreferences.game_options & _game_has_kill_limit)
	{
		// ZZZ: factored out into new function
        NetgameSetup_ScoreLimit(Data);
	}
	else if (theAdjustedPreferences.game_is_untimed)
	{
        NetgameSetup_Untimed(Data);
	}
	else
	{
        NetgameSetup_Timed(Data);
	}

	if (player_information->name[0]==0) SetControlActivity(Data.OK_Ctrl, false);
	
	// set up the game options
	SetControl32BitValue(Data.UseMicrophoneCtrl, network_preferences->allow_microphone);
	SetControl32BitValue(Data.MicrophoneTypeCtrl, network_preferences->use_speex_encoder ?
		mic_type_speex : mic_type_plain);
	SetDialogGameOptions(Data, theAdjustedPreferences.game_options);
	
	// If they're resuming a single-player game, now we overwrite any existing options with the cooperative-play options.
	// (We presumably twiddled the game_type from _game_of_kill_monsters up at the top there.)
	if(ResumingGame && dynamic_world->player_count == 1 && theAdjustedPreferences.game_type == _game_of_cooperative_play)
	{
		NetgameSetup_GameType(Data, theAdjustedPreferences.game_type);
	}

	/* Setup the team popup.. */
	SetControlActivity(Data.PlayerTeamCtrl, GetControl32BitValue(Data.UniqTeamsCtrl));

//#if !HAVE_SDL_NET
//	// set up network options
//	modify_selection_control(dialog, iNETWORK_SPEED, CONTROL_ACTIVE, network_preferences->type+1);
//#endif

	// Disable certain elements when resuming a game
	if(ResumingGame)
	{
		SetControlActivity(Data.GameTypeCtrl, false);
		SetControlActivity(Data.EntryPointCtrl, false);
  		SetControlActivity(Data.TimeTextCtrl, false);
		SetControlActivity(Data.KillsTextCtrl, false);
 		SetControlActivity(Data.DurationCtrl, false);
  	}

	return theAdjustedPreferences.game_type;
}

#else

short
fill_in_game_setup_dialog(
	DialogPtr dialog, 
	player_info *player_information,
	bool allow_all_levels,
        bool resuming_game)
{
	short name_length;
	long entry_flags;

        // We use a temporary structure so that we can change things without messing with the real preferences
        network_preferences_data theAdjustedPreferences = *network_preferences;
        if(resuming_game)
        {
                // Adjust the apparent preferences to get values from the loaded game (dynamic_world)
                // rather than from the actual network_preferences.
                theAdjustedPreferences.game_type = GET_GAME_TYPE();
                theAdjustedPreferences.difficulty_level = dynamic_world->game_information.difficulty_level;
                theAdjustedPreferences.entry_point = dynamic_world->current_level_number;
                theAdjustedPreferences.kill_limit = dynamic_world->game_information.kill_limit;
                theAdjustedPreferences.time_limit = dynamic_world->game_information.game_time_remaining;
                theAdjustedPreferences.game_options = GET_GAME_OPTIONS();
                // If the time limit is longer than a week, we figure it's untimed (  ;)
                theAdjustedPreferences.game_is_untimed = (dynamic_world->game_information.game_time_remaining > 7 * 24 * 3600 * TICKS_PER_SECOND);
                // If they are resuming a single-player game, assume they want cooperative play now.
                if(dynamic_world->player_count == 1 && GET_GAME_TYPE() == _game_of_kill_monsters)
                {
                        theAdjustedPreferences.game_type = _game_of_cooperative_play;
                        theAdjustedPreferences.game_options |= _live_network_stats; // single-player game doesn't, and they probably want it
                }
                allow_all_levels = true;
        }
        
	/* Fill in the entry points */
	if(allow_all_levels)
	{
		entry_flags= NONE;
	} else {
		entry_flags= get_entry_point_flags_for_game_type(theAdjustedPreferences.game_type);
	}

    // ZZZ: SDL version takes care of this its own way, simpler not to change.
#ifdef mac
    fill_in_entry_points(dialog, iENTRY_MENU, entry_flags, NONE);
#endif

	modify_selection_control(dialog, iGAME_TYPE, CONTROL_ACTIVE, theAdjustedPreferences.game_type+1);
        setup_dialog_for_game_type(dialog, theAdjustedPreferences.game_type);

	/* set up the name of the player. */
	name_length= player_preferences->name[0];
	if(name_length>MAX_NET_PLAYER_NAME_LENGTH) name_length= MAX_NET_PLAYER_NAME_LENGTH;
        // XXX (ZZZ): it looks to me that this forgets to set player_information->name[0] to name_length?
        // not sure if this is a problem in practice, but the Bungie guys were usually pretty careful about this sort of thing.
	memcpy(player_information->name, player_preferences->name, name_length+1);
	
        copy_pstring_to_text_field(dialog, iGATHER_NAME, player_information->name);
#ifdef mac
	SelectDialogItemText(dialog, iGATHER_NAME, 0, INT16_MAX);
#endif

	/* Set the menu values */
	modify_selection_control(dialog, iGATHER_COLOR, CONTROL_ACTIVE, player_preferences->color+1);
	modify_selection_control(dialog, iGATHER_TEAM, CONTROL_ACTIVE, player_preferences->team+1);
        modify_selection_control(dialog, iDIFFICULTY_MENU, CONTROL_ACTIVE, theAdjustedPreferences.difficulty_level+1);
        select_entry_point(dialog, iENTRY_MENU, theAdjustedPreferences.entry_point);

	// START Benad
	if (theAdjustedPreferences.game_type == _game_of_defense)
		insert_number_into_text_item(dialog, iKILL_LIMIT, theAdjustedPreferences.kill_limit/60);
	else
		insert_number_into_text_item(dialog, iKILL_LIMIT, theAdjustedPreferences.kill_limit);
	// END Benad

        // If resuming an untimed game, show the "time limit" from the prefs in the grayed-out widget
        // rather than some ridiculously large number
	insert_number_into_text_item(dialog, iTIME_LIMIT, ((resuming_game && theAdjustedPreferences.game_is_untimed) ? network_preferences->time_limit : theAdjustedPreferences.time_limit)/TICKS_PER_SECOND/60);

#ifdef HAVE_LUA
        modify_boolean_control(dialog, iUSE_SCRIPT, CONTROL_ACTIVE, theAdjustedPreferences.use_netscript);
#else
        modify_boolean_control(dialog, iUSE_SCRIPT, CONTROL_INACTIVE, false);
#endif
	
#ifdef SDL
# ifdef HAVE_LUA
	modify_control_enabled(dialog, iCHOOSE_SCRIPT, theAdjustedPreferences.use_netscript ? CONTROL_ACTIVE : CONTROL_INACTIVE);
# else
	modify_control_enabled(dialog, iCHOOSE_SCRIPT, CONTROL_INACTIVE);
# endif
#endif

	if (theAdjustedPreferences.game_options & _game_has_kill_limit)
	{
                // ZZZ: factored out into new function
                setup_for_score_limited_game(dialog);
	}
	else if (theAdjustedPreferences.game_is_untimed)
	{
		setup_for_untimed_game(dialog);
	}
	else
	{
		setup_for_timed_game(dialog);
	}

	if (player_information->name[0]==0) modify_control_enabled(dialog, iOK, CONTROL_INACTIVE);

	// set up the game options
	modify_boolean_control(dialog, iREAL_TIME_SOUND, CONTROL_ACTIVE, theAdjustedPreferences.allow_microphone);
	set_dialog_game_options(dialog, theAdjustedPreferences.game_options);

        // If they're resuming a single-player game, now we overwrite any existing options with the cooperative-play options.
        // (We presumably twiddled the game_type from _game_of_kill_monsters up at the top there.)
        if(resuming_game && dynamic_world->player_count == 1 && theAdjustedPreferences.game_type == _game_of_cooperative_play)
        {
                setup_dialog_for_game_type(dialog, theAdjustedPreferences.game_type);
        }

	/* Setup the team popup.. */
        modify_control_enabled(dialog, iGATHER_TEAM, get_boolean_control_value(dialog, iFORCE_UNIQUE_TEAMS) ? CONTROL_ACTIVE : CONTROL_INACTIVE);



#if !HAVE_SDL_NET
	// set up network options
	modify_selection_control(dialog, iNETWORK_SPEED, CONTROL_ACTIVE, theAdjustedPreferences.type+1);
#endif

        // Disable certain elements when resuming a game
        if(resuming_game)
        {
                modify_control_enabled(dialog, iGAME_TYPE, CONTROL_INACTIVE);
                modify_control_enabled(dialog, iENTRY_MENU, CONTROL_INACTIVE);
                modify_control_enabled(dialog, iKILL_LIMIT, CONTROL_INACTIVE);
                modify_control_enabled(dialog, iTIME_LIMIT, CONTROL_INACTIVE);
                modify_limit_type_choice_enabled(dialog, CONTROL_INACTIVE);
        }

	FileSpecifier theFile
#ifdef SDL
		(theAdjustedPreferences.netscript_file);
#else
	;
	theFile.SetSpec(theAdjustedPreferences.netscript_file);
#endif
	
	set_dialog_netscript_file(dialog, theFile);

	return theAdjustedPreferences.game_type;
}

#endif


/*************************************************************************************************
 *
 * Function: get_dialog_game_options
 * Purpose:  extract the game option flags from the net game setup's controls
 *
 *************************************************************************************************/

#ifdef USES_NIBS


static uint16 GetDialogGameOptions(NetgameSetupData& Data, short game_type)
{
	// These used to be options in the dialog. now they are always true, i guess.
	uint16 game_options = (_ammo_replenishes | _weapons_replenish | _specials_replenish);
	
	if(game_type==_game_of_cooperative_play) SET_FLAG(game_options,_overhead_map_is_omniscient,true);

	SET_FLAG(game_options, _monsters_replenish, GetControl32BitValue(Data.NoMotionSensorCtrl));
	SET_FLAG(game_options, _motion_sensor_does_not_work, GetControl32BitValue(Data.NoMotionSensorCtrl));
	SET_FLAG(game_options, _dying_is_penalized, GetControl32BitValue(Data.BadDyingCtrl));
	SET_FLAG(game_options, _suicide_is_penalized, GetControl32BitValue(Data.BadSuicideCtrl));
	SET_FLAG(game_options, _force_unique_teams, !GetControl32BitValue(Data.UniqTeamsCtrl));	// Reversed
	SET_FLAG(game_options, _burn_items_on_death, !GetControl32BitValue(Data.BurnItemsCtrl));	// Reversed
	SET_FLAG(game_options, _live_network_stats, GetControl32BitValue(Data.StatsReportingCtrl));
	 
	 return game_options;
}


#endif


static uint16
get_dialog_game_options(
	DialogPtr dialog,
	short game_type)
{
	uint16 game_options = 0;
	
	// These used to be options in the dialog. now they are always true, i guess.
	game_options |= (_ammo_replenishes | _weapons_replenish | _specials_replenish);
//#ifdef DEBUG
//	game_options |= _overhead_map_is_omniscient;
//#endif
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

#ifdef USES_NIBS

static void SetDialogGameOptions(NetgameSetupData& Data, uint16 game_options)
{
	SetControl32BitValue(Data.NoMotionSensorCtrl, TEST_FLAG(game_options, _monsters_replenish));
	SetControl32BitValue(Data.NoMotionSensorCtrl, TEST_FLAG(game_options, _motion_sensor_does_not_work));
	SetControl32BitValue(Data.BadDyingCtrl, TEST_FLAG(game_options, _dying_is_penalized));
	SetControl32BitValue(Data.BadSuicideCtrl, TEST_FLAG(game_options, _suicide_is_penalized));
	SetControl32BitValue(Data.UniqTeamsCtrl, !TEST_FLAG(game_options, _force_unique_teams));	// Reversed
	SetControl32BitValue(Data.BurnItemsCtrl, !TEST_FLAG(game_options, _burn_items_on_death));	// Reversed
	SetControl32BitValue(Data.StatsReportingCtrl, TEST_FLAG(game_options, _live_network_stats));
}

#endif


static void
set_dialog_game_options(
	DialogPtr dialog, 
	uint16 game_options)
{
	modify_boolean_control(dialog, iUNLIMITED_MONSTERS, NONE, (game_options & _monsters_replenish) ? 1 : 0);
	modify_boolean_control(dialog, iMOTION_SENSOR_DISABLED, NONE, (game_options & _motion_sensor_does_not_work) ? 1 : 0);
	modify_boolean_control(dialog, iDYING_PUNISHED, NONE, (game_options & _dying_is_penalized) ? 1 : 0);
	modify_boolean_control(dialog, iSUICIDE_PUNISHED, NONE, (game_options & _suicide_is_penalized) ? 1 : 0);
	modify_boolean_control(dialog, iFORCE_UNIQUE_TEAMS, NONE, (game_options & _force_unique_teams) ? false : true);
	modify_boolean_control(dialog, iBURN_ITEMS_ON_DEATH, NONE, (game_options & _burn_items_on_death) ? false : true);
	modify_boolean_control(dialog, iREALTIME_NET_STATS, NONE, (game_options & _live_network_stats) ? true : false);
}


#ifdef USES_NIBS

static void SetDurationText(NetgameSetupData& Data,
	short radio_item, short radio_stringset_id, short radio_string_index,
	ControlRef UnitsCtrl, short units_stringset_id, short units_string_index)
{
	OSStatus err;
	
	// Extract the individual radio button
	ControlRef RBCtrl;
	err = GetIndexedSubControl(Data.DurationCtrl,radio_item, &RBCtrl);
	
	vassert(err == noErr, csprintf(temporary, "Error in GetIndexedSubControl: %d for item %hd", err, radio_item));
	
	getpstr(ptemporary, radio_stringset_id, radio_string_index);	
	SetControlTitle(RBCtrl, ptemporary);
	
 	getpstr(ptemporary, units_stringset_id, units_string_index);
 	SetStaticPascalText(UnitsCtrl, ptemporary);
 }


void NetgameSetup_GameType(
	NetgameSetupData& Data,
	int game_type
	)
{
	switch(game_type)
	{
	case _game_of_cooperative_play:
		/* set & disable the drop items checkbox */
		// Benad
		
		SetControlActivity(Data.UniqTeamsCtrl, true);
		
		SetControl32BitValue(Data.BurnItemsCtrl, true);
		SetControlActivity(Data.BurnItemsCtrl, false);
		
		SetControl32BitValue(Data.MonstersCtrl, true);
		SetControlActivity(Data.MonstersCtrl, false);
		
		SetDurationText(Data, duration_kill_limit, strSETUP_NET_GAME_MESSAGES, killLimitString,
			Data.KillsLabelCtrl, strSETUP_NET_GAME_MESSAGES, killsString);
 		
 		NetgameSetup_Untimed(Data);
 		
 		break;
 		
	case _game_of_kill_monsters:
	case _game_of_king_of_the_hill:
	case _game_of_kill_man_with_ball:
	case _game_of_tag:
		// Benad
		
		SetControlActivity(Data.UniqTeamsCtrl, true);
		
		SetControl32BitValue(Data.BurnItemsCtrl, false);
		SetControlActivity(Data.BurnItemsCtrl, true);
		
		SetControlActivity(Data.MonstersCtrl, true);

		SetDurationText(Data, duration_kill_limit, strSETUP_NET_GAME_MESSAGES, killLimitString,
			Data.KillsLabelCtrl, strSETUP_NET_GAME_MESSAGES, killsString);
		 		
 		NetgameSetup_Timed(Data);
 		
 		break;
	
	case _game_of_capture_the_flag:
		// START Benad
			
		SetControl32BitValue(Data.UniqTeamsCtrl, true);
		SetControlActivity(Data.UniqTeamsCtrl, false);
			
		SetControlActivity(Data.PlayerTeamCtrl, true);

		// END Benad
		/* Allow them to decide on the burn items on death */
		
		SetControl32BitValue(Data.BurnItemsCtrl, false);
		SetControlActivity(Data.BurnItemsCtrl, true);
		
		SetControlActivity(Data.MonstersCtrl, true);

		SetDurationText(Data, duration_kill_limit, strSETUP_NET_GAME_MESSAGES, flagPullsString,
			Data.KillsLabelCtrl, strSETUP_NET_GAME_MESSAGES, flagsString);
		 		
 		NetgameSetup_Timed(Data);
 		
 		break;
			
	case _game_of_rugby:

		// START Benad
		// Disable "Allow teams", and force it to be checked.
		
		SetControl32BitValue(Data.UniqTeamsCtrl, true);
		SetControlActivity(Data.UniqTeamsCtrl, false);
			
		SetControlActivity(Data.PlayerTeamCtrl, true);
				
		// END Benad
		/* Allow them to decide on the burn items on death */
		
		SetControl32BitValue(Data.BurnItemsCtrl, false);
		SetControlActivity(Data.BurnItemsCtrl, true);
		
		SetControlActivity(Data.MonstersCtrl, true);

		SetDurationText(Data, duration_kill_limit, strSETUP_NET_GAME_MESSAGES, pointLimitString,
			Data.KillsLabelCtrl, strSETUP_NET_GAME_MESSAGES, pointsString);
 		
 		NetgameSetup_Timed(Data);
 		
 		break;

	case _game_of_defense:
		/* Allow them to decide on the burn items on death */
		// START Benad
		
		SetControl32BitValue(Data.UniqTeamsCtrl, true);
		SetControlActivity(Data.UniqTeamsCtrl, false);
			
		SetControlActivity(Data.PlayerTeamCtrl, true);
		
		// END Benad
		
		SetControl32BitValue(Data.BurnItemsCtrl, false);
		SetControlActivity(Data.BurnItemsCtrl, true);
		
		SetControlActivity(Data.MonstersCtrl, true);
		
		SetDurationText(Data, duration_kill_limit, strSETUP_NET_GAME_MESSAGES, timeOnBaseString,
			Data.KillsLabelCtrl, strSETUP_NET_GAME_MESSAGES, minutesString);
 		
 		NetgameSetup_Timed(Data);
 		
 		break;
			
		default:
			assert(false);
			break;
	}

}

// From some of ZZZ's functions
void NetgameSetup_Untimed(NetgameSetupData& Data)
{
	SetControl32BitValue(Data.DurationCtrl, duration_no_time_limit);
	
	HideControl(Data.TimeLabelCtrl);
	HideControl(Data.TimeTextCtrl);
	HideControl(Data.KillsLabelCtrl);
	HideControl(Data.KillsTextCtrl);
}

void NetgameSetup_Timed(NetgameSetupData& Data)
{
	SetControl32BitValue(Data.DurationCtrl, duration_time_limit);
	
	// START Benad
	if (GetControl32BitValue(Data.GameTypeCtrl) - 1 != _game_of_defense)
	{
		ShowControl(Data.TimeLabelCtrl);
		ShowControl(Data.TimeTextCtrl);
		HideControl(Data.KillsLabelCtrl);
		HideControl(Data.KillsTextCtrl);
	}
	else
	{
		ShowControl(Data.TimeLabelCtrl);
		ShowControl(Data.TimeTextCtrl);
		ShowControl(Data.KillsLabelCtrl);
		ShowControl(Data.KillsTextCtrl);
	}
	// END Benad
}

void NetgameSetup_ScoreLimit(NetgameSetupData& Data)
{
	SetControl32BitValue(Data.DurationCtrl, duration_kill_limit);
	
	// START Benad
	if (GetControl32BitValue(Data.GameTypeCtrl) - 1 != _game_of_defense)
	{
		HideControl(Data.TimeLabelCtrl);
		HideControl(Data.TimeTextCtrl);
		ShowControl(Data.KillsLabelCtrl);
		ShowControl(Data.KillsTextCtrl);
	}
	else
	{
		ShowControl(Data.TimeLabelCtrl);
		ShowControl(Data.TimeTextCtrl);
		ShowControl(Data.KillsLabelCtrl);
		ShowControl(Data.KillsTextCtrl);
	}
	// END Benad
}

#else

// ZZZ: moved here from network_dialogs_macintosh.cpp
void setup_dialog_for_game_type(
	DialogPtr dialog, 
	size_t game_type)
{
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
                        
			/* Untimed.. */
			setup_for_untimed_game(dialog);
			break;
			
		case _game_of_kill_monsters:
		case _game_of_king_of_the_hill:
		case _game_of_kill_man_with_ball:
		case _game_of_tag:
			// Benad
			modify_control_enabled(dialog, iFORCE_UNIQUE_TEAMS, CONTROL_ACTIVE);
			
			modify_boolean_control(dialog, iBURN_ITEMS_ON_DEATH, CONTROL_ACTIVE, false);
			modify_control_enabled(dialog, iUNLIMITED_MONSTERS, CONTROL_ACTIVE);

			set_limit_text(dialog, iRADIO_KILL_LIMIT, strSETUP_NET_GAME_MESSAGES, killLimitString,
                                        iTEXT_KILL_LIMIT, strSETUP_NET_GAME_MESSAGES, killsString);

			setup_for_timed_game(dialog);
			break;

		case _game_of_capture_the_flag:
			// START Benad
			modify_boolean_control(dialog, iFORCE_UNIQUE_TEAMS, CONTROL_INACTIVE, true);
			modify_control_enabled(dialog, iGATHER_TEAM, CONTROL_ACTIVE);
			// END Benad
			/* Allow them to decide on the burn items on death */
			modify_boolean_control(dialog, iBURN_ITEMS_ON_DEATH, CONTROL_ACTIVE, false);
			modify_control_enabled(dialog, iUNLIMITED_MONSTERS, CONTROL_ACTIVE);

			set_limit_text(dialog, iRADIO_KILL_LIMIT, strSETUP_NET_GAME_MESSAGES, flagPullsString,
                                        iTEXT_KILL_LIMIT, strSETUP_NET_GAME_MESSAGES, flagsString);
                        
			setup_for_timed_game(dialog);
			break;
			
		case _game_of_rugby:
			/* Allow them to decide on the burn items on death */
			modify_boolean_control(dialog, iBURN_ITEMS_ON_DEATH, CONTROL_ACTIVE, false);
			modify_control_enabled(dialog, iUNLIMITED_MONSTERS, CONTROL_ACTIVE);

			set_limit_text(dialog, iRADIO_KILL_LIMIT, strSETUP_NET_GAME_MESSAGES, pointLimitString,
                                        iTEXT_KILL_LIMIT, strSETUP_NET_GAME_MESSAGES, pointsString);

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
			
			ShowDialogItem(dialog, iTEXT_KILL_LIMIT); ShowDialogItem(dialog, iKILL_LIMIT);
			ShowDialogItem(dialog, iTIME_LIMIT); ShowDialogItem(dialog, iTEXT_TIME_LIMIT);
			
			// END Benad
			modify_boolean_control(dialog, iBURN_ITEMS_ON_DEATH, CONTROL_ACTIVE, false);
			modify_control_enabled(dialog, iUNLIMITED_MONSTERS, CONTROL_ACTIVE);
			setup_for_timed_game(dialog);
			break;
			
		default:
			assert(false);
			break;
	}
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
}

#endif


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
#ifdef USES_NIBS
	NetgameOutcomeData &Data,
#else
	DialogPtr dialog,
#endif
	short *index)
{
	short value;
	short graph_type = NONE;
	bool has_scores;
	
	has_scores= current_net_game_has_scores();
	
	/* Popups are 1 based */
#ifdef USES_NIBS
	value = GetControl32BitValue(Data.SelectCtrl) - 1;
#else
	value = get_selection_control_value(dialog, iGRAPH_POPUP)-1;
#endif
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
				assert(false);
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
				assert(false);
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
				assert(false);
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
#ifdef USES_NIBS
	NetgameOutcomeData &Data,
#else
	DialogPtr dialog,
#endif
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

#ifdef USES_NIBS
	draw_names(Data, ranks, dynamic_world->player_count, index);
	draw_kill_bars(Data, ranks, dynamic_world->player_count, index, false, false);
#else
	draw_names(dialog, ranks, dynamic_world->player_count, index);
	draw_kill_bars(dialog, ranks, dynamic_world->player_count, index, false, false);
#endif
}


// ZZZ: team vs team carnage (analogous to draw_player_graph's player vs player carnage)
// THIS IS UNFINISHED (and thus unused at the moment :) )
void draw_team_graph(
#ifdef USES_NIBS
	NetgameOutcomeData &Data,
#else
	DialogPtr dialog,
#endif
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
}



// (ZZZ annotation:) Total Carnage graph
void draw_totals_graph(
#ifdef USES_NIBS
	NetgameOutcomeData &Data)
#else
	DialogPtr dialog)
#endif
{
#ifdef USES_NIBS
	draw_names(Data, rankings, dynamic_world->player_count, NONE);
	draw_kill_bars(Data, rankings, dynamic_world->player_count, NONE, true, false);
#else
	draw_names(dialog, rankings, dynamic_world->player_count, NONE);
	draw_kill_bars(dialog, rankings, dynamic_world->player_count, NONE, true, false);
#endif
}



// (ZZZ annotation:) Total Team Carnage graph
void draw_team_totals_graph(
#ifdef USES_NIBS
	NetgameOutcomeData &Data)
#else
	DialogPtr dialog)
#endif
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
	
#ifdef USES_NIBS
	draw_names(Data, ranks, num_teams, NONE);
	draw_kill_bars(Data, ranks, num_teams, NONE, true, true);
#else
	draw_names(dialog, ranks, num_teams, NONE);
	draw_kill_bars(dialog, ranks, num_teams, NONE, true, true);
#endif
}



// (ZZZ annotation:) Time on Hill, etc. graph
void draw_total_scores_graph(
#ifdef USES_NIBS
	NetgameOutcomeData &Data)
#else
	DialogPtr dialog)
#endif
{
	struct net_rank ranks[MAXIMUM_NUMBER_OF_PLAYERS];
	
	/* Use a private copy to avoid boning things */
	objlist_copy(ranks, rankings, dynamic_world->player_count);

	/* First qsort the rankings arrray by game_ranking.. */
	qsort(ranks, dynamic_world->player_count, sizeof(struct net_rank), score_rank_compare);

	/* Draw the names. */
#ifdef USES_NIBS
	draw_names(Data, ranks, dynamic_world->player_count, NONE);
#else
	draw_names(dialog, ranks, dynamic_world->player_count, NONE);
#endif
	
	/* And draw the bars... */
#ifdef USES_NIBS
	draw_score_bars(Data, ranks, dynamic_world->player_count);
#else
	draw_score_bars(dialog, ranks, dynamic_world->player_count);
#endif
}



// (ZZZ annotation:) Team Time on Hill, etc. graph
void draw_team_total_scores_graph(
#ifdef USES_NIBS
	NetgameOutcomeData &Data)
#else
	DialogPtr dialog)
#endif
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
#ifdef USES_NIBS
	draw_names(Data, ranks, team_count, NONE);
	draw_score_bars(Data, ranks, team_count);
#else
	draw_names(dialog, ranks, team_count, NONE);
	draw_score_bars(dialog, ranks, team_count);
#endif
}



// (ZZZ) ripped this out of draw_kill_bars since we can share this bit but not the rest of draw_kill_bars().
// (ZZZ annotation:) Update the "N deaths total (D dpm) including S suicides"-type text at the bottom.
void
update_carnage_summary(                 	
#ifdef USES_NIBS
	NetgameOutcomeData &Data,
#else
	DialogPtr dialog,
#endif
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
    long    total_kills = 0;
    long    total_deaths = 0;
	char    kill_string_format[65];
    char    death_string_format[65];
    char    suicide_string_format[65];
    
    for (i = 0; i < num_players; i++)
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

	minutes = ((float)dynamic_world->tick_count / TICKS_PER_SECOND) / 60.0F;
	if (minutes > 0) kpm = total_kills / minutes;
	else kpm = 0;
	getcstr(kill_string_format, strNET_STATS_STRINGS, strTOTAL_KILLS_STRING);
	psprintf(ptemporary, kill_string_format, total_kills, kpm);
//	GetDialogItem(dialog, iTOTAL_KILLS, &item_type, &item_handle, &item_rect);
//	SetDialogItemText(item_handle, ptemporary);
    
#ifdef USES_NIBS
    SetStaticPascalText(Data.KillsTextCtrl, ptemporary);
#else
    copy_pstring_to_static_text(dialog, iTOTAL_KILLS, ptemporary);
#endif

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

#ifdef USES_NIBS
    SetStaticPascalText(Data.DeathsTextCtrl, ptemporary);
#else
    copy_pstring_to_static_text(dialog, iTOTAL_DEATHS, ptemporary);
#endif
}



// ZZZ: ripped out of update_damage_item
// (ZZZ annotation:) Demultiplex to draw_X_graph() function based on find_graph_mode().
void
#ifdef USES_NIBS
draw_new_graph(NetgameOutcomeData &Data) {
#else
draw_new_graph(DialogPtr dialog) {
#endif
    short   graph_type;
    short   index;

#ifdef USES_NIBS
    graph_type= find_graph_mode(Data, &index);
#else
    graph_type= find_graph_mode(dialog, &index);
#endif
	
	switch(graph_type)
	{
		case _player_graph:
#ifdef USES_NIBS
			draw_player_graph(Data, index);
#else
			draw_player_graph(dialog, index);
#endif
			break;
			
		case _total_carnage_graph:
#ifdef USES_NIBS
			draw_totals_graph(Data);
#else
			draw_totals_graph(dialog);
#endif
			break;
			
		case _total_scores_graph:
#ifdef USES_NIBS
			draw_total_scores_graph(Data);
#else
			draw_total_scores_graph(dialog);
#endif
			break;

		/* These two functions need to have the team colors. */
		case _total_team_carnage_graph:
#ifdef USES_NIBS
			draw_team_totals_graph(Data);
#else
			draw_team_totals_graph(dialog);
#endif
			break;
			
		case _total_team_scores_graph:
#ifdef USES_NIBS
			draw_team_total_scores_graph(Data);
#else
			draw_team_total_scores_graph(dialog);
#endif
			break;
			
		default:
			assert(false);
			break;
	}
}

// (ZZZ annotation:) Find the highest player vs. player kills
// (so all player vs. player graphs can be at the same scale)
short calculate_max_kills(
	size_t num_players)
{
	short  max_kills = 0;
	size_t  i, j;
	
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
			color->red= color->green= USHRT_MAX;
			color->blue= 0;
			break;
		case _kill_color:
			color->red= USHRT_MAX;
			color->green= color->blue= 0;
			break;
		case _death_color:
		case _score_color:
			color->red= color->green= color->blue= 60000;
			break;
		default:
			assert(false);
			break;
	}
}


#ifdef HAVE_SDL_NET

// For manipulating the list of recent host addresses:

// Sets it to empty, of course
void ResetHostAddresses_Reset()
{
	RecentHostAddresses.clear();
	RHAddrIter = RecentHostAddresses.end();
}

// Returns whether doing so knocked an existing address off the list
bool RecentHostAddresses_Add(const char *Address)
{
	// How many addresses
	int NumAddresses = 0;
	
	// Iterator value for old value to be removed; set to "none"
	list<HostName_Type>::iterator OldAddrIter = RecentHostAddresses.end();
	
	// Check if the new address is present and add up
	for(RHAddrIter = RecentHostAddresses.begin();
		RHAddrIter != RecentHostAddresses.end();
			RHAddrIter++, NumAddresses++)
	{
		if (strncmp(RHAddrIter->Name, Address, kJoinHintingAddressLength) == 0)
			OldAddrIter = RHAddrIter;
	}
	
	// If it is already present, then delete!
	if (OldAddrIter != RecentHostAddresses.end())
	{
		RecentHostAddresses.erase(OldAddrIter);
		NumAddresses--;
	}
	
	// Remove extra ones from the back
	bool NoneRemoved = true;
	while(NumAddresses >= kMaximumRecentAddresses)
	{
		RecentHostAddresses.pop_back();
		NumAddresses--;
		NoneRemoved = false;
	}
	
	// Add to the front
	HostName_Type NewAddress;
	strncpy(NewAddress.Name,Address,kJoinHintingAddressLength);
	NewAddress.Name[kJoinHintingAddressLength] = 0;
	
	RecentHostAddresses.push_front(NewAddress);
	
	// Set to no more iteration to be done
	RHAddrIter = RecentHostAddresses.end();
	
	return NoneRemoved;
}

// Start iterating over it
void RecentHostAddresses_StartIter()
{
	// Start the iterations
	RHAddrIter = RecentHostAddresses.begin();
}

// Returns the next member in sequence;
// if it ran off the end, then it returns NULL
char *RecentHostAddresses_NextIter()
{
	char *Addr = NULL;
	
	if (RHAddrIter != RecentHostAddresses.end())
	{
		Addr = RHAddrIter->Name;
		RHAddrIter++;
	}
	
	return Addr;
}

#endif

