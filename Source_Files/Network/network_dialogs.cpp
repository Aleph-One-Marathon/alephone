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

#if !defined(DISABLE_NETWORKING)

#include	"cseries.h"
#include	"map.h"
#include	"shell.h"
#include	"preferences.h"
#include	"network.h"
#include	"network_dialogs.h"
#include	"network_games.h"
#include	"player.h" // ZZZ: for MAXIMUM_NUMBER_OF_PLAYERS, for reassign_player_colors
#include	"metaserver_dialogs.h" // GameAvailableMetaserverAnnouncer
#include	"wad.h" // jkvw: for read_wad_file_checksum 

#include <map>

// For LAN netgame location services
#include	<sstream>
#include	"network_private.h" // actually just need "network_dialogs_private.h"
#include	"SSLP_API.h"

#ifdef USES_NIBS
	#include "NibsUiHelpers.h"
#endif


extern void NetRetargetJoinAttempts(const IPaddress* inAddress);


// Metaserver Globals
// We can't construct a MetaserverClient until MetaserverClient::s_instances is initialised.
MetaserverClient* gMetaserverClient = NULL;

// Chat History Globals
ChatHistory gMetaserverChatHistory;
ChatHistory gPregameChatHistory;


// For the recent host addresses
#include <list>

struct HostName_Type
{
	// C string
	char Name[kJoinHintingAddressLength+1];
};

static list<HostName_Type> RecentHostAddresses;
static list<HostName_Type>::iterator RHAddrIter = RecentHostAddresses.end();


static uint16 get_dialog_game_options(DialogPTR dialog, short game_type);
static void set_dialog_game_options(DialogPTR dialog, uint16 game_options);


////////////////////////////////////////////////////////////////////////////////
// LAN game-location services support

static const string
get_sslp_service_type()
{
	stringstream ss;
	//	ss << "A1 Gatherer V" << get_network_version();
	ss << kNetworkSetupProtocolID;
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


/****************************************************
 *
 *	Shared Network Gather Dialog Code
 *
 ****************************************************/

bool network_gather(bool inResumingGame)
{
	bool successful= false;
	game_info myGameInfo;
	player_info myPlayerInfo;
	bool advertiseOnMetaserver = false;

	show_cursor(); // JTP: Hidden one way or another
	if (network_game_setup(&myPlayerInfo, &myGameInfo, inResumingGame, advertiseOnMetaserver))
	{
		myPlayerInfo.desired_color= myPlayerInfo.color;
		memcpy(myPlayerInfo.long_serial_number, serial_preferences->long_serial_number, 10);
	
		if(NetEnter())
		{
			bool gather_dialog_result;
		
			if(NetGather(&myGameInfo, sizeof(game_info), (void*) &myPlayerInfo, 
				sizeof(myPlayerInfo), inResumingGame))
			{
				GathererAvailableAnnouncer announcer;
				
				if (!gMetaserverClient) gMetaserverClient = new MetaserverClient ();

				auto_ptr<GameAvailableMetaserverAnnouncer> metaserverAnnouncer;
				if(advertiseOnMetaserver)
				{
					try
					{
						metaserverAnnouncer.reset(new GameAvailableMetaserverAnnouncer(myGameInfo));
					}
					catch (...)
					{
						alert_user(infoError, strNETWORK_ERRORS, netWarnCouldNotAdvertiseOnMetaserver, 0);
					}
				}

				gather_dialog_result = GatherDialog::Create()->GatherNetworkGameByRunning();
				
				// Save autogather pref, even if cancel
				write_preferences ();
			} else {
				gather_dialog_result = false;
			}
			
			if (gather_dialog_result) {
				NetDoneGathering();
				successful= true;
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

	hide_cursor();
	return successful;
}

GatherDialog::GatherDialog() {  }

GatherDialog::~GatherDialog()
{
	delete m_cancelWidget;
	delete m_startWidget;
	delete m_autogatherWidget;
	delete m_ungatheredWidget;
	delete m_pigWidget;
	delete m_chatEntryWidget;
	delete m_chatWidget;
	delete m_chatChoiceWidget;

	// gMetaserverClient->disconnect ();
	delete gMetaserverClient;
	gMetaserverClient = new MetaserverClient ();
}

bool GatherDialog::GatherNetworkGameByRunning ()
{
	vector<string> chat_choice_labels;
	chat_choice_labels.push_back ("with joiners");
	chat_choice_labels.push_back ("with Internet players");
	m_chatChoiceWidget->set_labels (chat_choice_labels);

	m_cancelWidget->set_callback(boost::bind(&GatherDialog::Stop, this, false));
	m_startWidget->set_callback(boost::bind(&GatherDialog::StartGameHit, this));
	m_ungatheredWidget->SetItemSelectedCallback(boost::bind(&GatherDialog::gathered_player, this, _1));
	
	m_startWidget -> deactivate ();
	
	NetSetGatherCallbacks(this);
	
	m_chatChoiceWidget->set_callback(boost::bind(&GatherDialog::chatChoiceHit, this));
	m_chatEntryWidget->set_callback(boost::bind(&GatherDialog::chatTextEntered, this, _1));
	
	gPregameChatHistory.clear ();
	//	gPregameChatHistory.appendString ("Pregame chat does not work currently");
	NetSetChatCallbacks(this);
	
	if (gMetaserverClient->isConnected ()) {
		gMetaserverClient->associateNotificationAdapter(this);
		m_chatChoiceWidget->set_value (kMetaserverChat);
		gMetaserverChatHistory.clear ();
		m_chatWidget->attachHistory (&gMetaserverChatHistory);
	} else {
		m_chatChoiceWidget->deactivate ();
		m_chatChoiceWidget->set_value (kPregameChat);
		gMetaserverChatHistory.clear ();
		m_chatWidget->attachHistory (&gPregameChatHistory);
	}
	
	return Run ();
}

void GatherDialog::idle ()
{
	MetaserverClient::pumpAll();
	
	prospective_joiner_info info;
	if (player_search(info)) {
		m_ungathered_players[info.stream_id] = info;
		update_ungathered_widget ();
	}
	
	if (m_autogatherWidget->get_value ()) {
		map<int, prospective_joiner_info>::iterator it;
		it = m_ungathered_players.begin ();
		while (it != m_ungathered_players.end () && NetGetNumberOfPlayers() < MAXIMUM_NUMBER_OF_PLAYERS) {
			gathered_player ((it++)->second);
		}
	}
}

void GatherDialog::update_ungathered_widget ()
{
	vector<prospective_joiner_info> temp;

	for (map<int, prospective_joiner_info>::iterator it = m_ungathered_players.begin (); it != m_ungathered_players.end (); ++it)
		temp.push_back ((*it).second);
	
	m_ungatheredWidget->SetItems (temp);
}

bool GatherDialog::player_search (prospective_joiner_info& player)
{
	GathererAvailableAnnouncer::pump();

	if (NetCheckForNewJoiner(player)) {
		m_ungathered_players[player.stream_id] = player;
		update_ungathered_widget ();
		return true;
	} else
		return false;
}

bool GatherDialog::gathered_player (const prospective_joiner_info& player)
{
	int theGatherPlayerResult = NetGatherPlayer(player, reassign_player_colors);
	
	if (theGatherPlayerResult != kGatherPlayerFailed) {
		m_ungathered_players.erase (m_ungathered_players.find (player.stream_id));
		update_ungathered_widget ();
		return true;
	} else
		return false;
}

void GatherDialog::StartGameHit ()
{
	for (map<int, prospective_joiner_info>::iterator it = m_ungathered_players.begin (); it != m_ungathered_players.end (); ++it)
		NetHandleUngatheredPlayer ((*it).second);
	
	Stop (true);
}

void GatherDialog::JoinSucceeded(const prospective_joiner_info* player)
{
	if (NetGetNumberOfPlayers () > 1)
		m_startWidget->activate ();
	
	m_pigWidget->redraw ();
}

void GatherDialog::JoiningPlayerDropped(const prospective_joiner_info* player)
{
	map<int, prospective_joiner_info>::iterator it = m_ungathered_players.find (player->stream_id);
	if (it != m_ungathered_players.end ())
		m_ungathered_players.erase (it);
	
	update_ungathered_widget ();
}

void GatherDialog::JoinedPlayerDropped(const prospective_joiner_info* player)
{
	if (NetGetNumberOfPlayers () < 2)
		m_startWidget->deactivate ();

	m_pigWidget->redraw ();
}

void GatherDialog::JoinedPlayerChanged(const prospective_joiner_info* player)
{	
	m_pigWidget->redraw ();
}

void GatherDialog::sendChat ()
{
	string message = m_chatEntryWidget->get_text();
	
#ifndef SDL
	// It's just a little semantic difference, really.  :)
	message = string(message.data (), message.length () - 1); // lose the last character, i.e. '\r'.
#endif
	
	if (m_chatChoiceWidget->get_value () == kMetaserverChat)
		gMetaserverClient->sendChatMessage(message);		
	else
		SendChatMessage(message);
	
	m_chatEntryWidget->set_text(string());
}

void GatherDialog::chatTextEntered (char character)
{
	if (character == '\r')
		sendChat();
}

void GatherDialog::receivedChatMessage(const std::string& senderName, uint32 senderID, const std::string& message)
{
	gMetaserverChatHistory.appendString (senderName + ": " + message);
}

void GatherDialog::receivedBroadcastMessage (const std::string& message)
{
	gMetaserverChatHistory.appendString (message);
}

void GatherDialog::chatChoiceHit ()
{
	if (m_chatChoiceWidget->get_value () == kPregameChat)
		m_chatWidget->attachHistory (&gPregameChatHistory);
	else
		m_chatWidget->attachHistory (&gMetaserverChatHistory);
}

void GatherDialog::ReceivedMessageFromPlayer(
	const char *player_name, 
	const char *message)
{
	gPregameChatHistory.appendString(std::string(player_name) + ": " + std::string(message));
}

/****************************************************
 *
 *	Shared Network Join Dialog Code
 *
 ****************************************************/

int network_join(void)
{
	int join_dialog_result;

	show_cursor(); // Hidden one way or another
	
	/* If we can enter the network... */
	if(NetEnter())
	{

		join_dialog_result = JoinDialog::Create()->JoinNetworkGameByRunning();
		
		if (join_dialog_result == kNetworkJoinedNewGame || join_dialog_result == kNetworkJoinedResumeGame)
		{
			write_preferences ();
		
			game_info* myGameInfo= (game_info *)NetGetGameData();
			NetSetInitialParameters(myGameInfo->initial_updates_per_packet, myGameInfo->initial_update_latency);
		}
		else
		{
			read_preferences ();
		
			if (join_dialog_result == kNetworkJoinFailedJoined)
				NetCancelJoin();
			
			NetExit();
		}
	} else { // Failed NetEnter
		join_dialog_result = kNetworkJoinFailedUnjoined;
	}
	
	hide_cursor();
	return join_dialog_result;
}

JoinDialog::JoinDialog() : join_announcer(true), got_gathered(false)
	{ if (!gMetaserverClient) gMetaserverClient = new MetaserverClient (); }

JoinDialog::~JoinDialog ()
{
	// gMetaserverClient->disconnect ();
	delete gMetaserverClient;
	gMetaserverClient = new MetaserverClient ();
}

const int JoinDialog::JoinNetworkGameByRunning ()
{
	join_result = kNetworkJoinFailedUnjoined;
	
	vector<string> chat_choice_labels;
	chat_choice_labels.push_back ("with joiners/gatherer");
	chat_choice_labels.push_back ("with Internet players");
	m_chatChoiceWidget->set_labels (chat_choice_labels);

	m_colourWidget->set_labels (kTeamColorsStringSetID);
	m_teamWidget->set_labels (kTeamColorsStringSetID);
	
	m_cancelWidget->set_callback(boost::bind(&JoinDialog::Stop, this));
	m_joinWidget->set_callback(boost::bind(&JoinDialog::attemptJoin, this));
	m_joinMetaserverWidget->set_callback(boost::bind(&JoinDialog::getJoinAddressFromMetaserver, this));
	
	m_chatChoiceWidget->set_value (kPregameChat);
	m_chatChoiceWidget->deactivate ();
	m_chatEntryWidget->deactivate ();
	m_chatChoiceWidget->set_callback(boost::bind(&JoinDialog::chatChoiceHit, this));
	m_chatEntryWidget->set_callback(boost::bind(&JoinDialog::chatTextEntered, this, _1));
	
	getpstr(ptemporary, strJOIN_DIALOG_MESSAGES, _join_dialog_welcome_string);
	m_messagesWidget->set_text(pstring_to_string(ptemporary));
	
	Run ();
	
	return join_result;
}

void JoinDialog::respondToJoinHit()
{
	gPregameChatHistory.clear ();
	if (gMetaserverClient->isConnected ()) {
		m_chatChoiceWidget->activate ();
		m_chatChoiceWidget->set_value (kMetaserverChat);
		gMetaserverClient->associateNotificationAdapter(this);
		m_chatWidget->attachHistory (&gMetaserverChatHistory);
	} else {
		m_chatWidget->attachHistory (&gPregameChatHistory);
	}
	m_chatEntryWidget->activate ();	
	NetSetChatCallbacks(this);
}

void JoinDialog::attemptJoin ()
{
	char* hintString = NULL;
	
	if(m_joinByAddressWidget->get_value()) {
		hintString = new char[256];
		copy_string_to_cstring (m_joinAddressWidget->get_text (), hintString);
	}
	
	player_info myPlayerInfo;
	copy_string_to_pstring (m_nameWidget->get_text (), myPlayerInfo.name, MAX_NET_PLAYER_NAME_LENGTH);
	myPlayerInfo.team = m_teamWidget->get_value ();
	myPlayerInfo.desired_color = m_colourWidget->get_value ();
	
	// jkvw: It may look like we're passing our player name into NetGameJoin,
	//       but network code will later draw the name directly from prefs.
	m_nameWidget->update_prefs ();
	
	bool result = NetGameJoin((void *) &myPlayerInfo, sizeof(myPlayerInfo), hintString);
	
	if (hintString)
		delete [] hintString;
	
	if (result) {
		m_nameWidget->deactivate ();
		m_teamWidget->deactivate ();
		m_colourWidget->deactivate ();

		m_joinAddressWidget->deactivate ();
		m_joinByAddressWidget->deactivate ();
		m_joinWidget->deactivate ();
		m_joinMetaserverWidget->deactivate ();
		
		getpstr(ptemporary, strJOIN_DIALOG_MESSAGES, _join_dialog_waiting_string);
		m_messagesWidget->set_text(pstring_to_string(ptemporary));
		
		respondToJoinHit ();
	}
}

void JoinDialog::gathererSearch ()
{
	JoinerSeekingGathererAnnouncer::pump();
	MetaserverClient::pumpAll();
	
	switch (NetUpdateJoinState())
	{
		case NONE: // haven't Joined yet.
			join_result = kNetworkJoinFailedUnjoined;
			break;

		case netConnecting:
		case netJoining:
			join_result = kNetworkJoinFailedJoined;
			break;

		case netCancelled: // the server cancelled the game; force bail
			join_result = kNetworkJoinFailedJoined;
			Stop ();
			break;

		case netWaiting:
			join_result = kNetworkJoinFailedJoined;
			break;

		case netStartingUp: // the game is starting up (we have the network topography)
			join_result = kNetworkJoinedNewGame;
			Stop ();
			break;

		case netStartingResumeGame: // the game is starting up a resume game (we have the network topography)
			join_result = kNetworkJoinedResumeGame;
			Stop ();
			break;

		case netPlayerChanged:
		case netPlayerAdded:
	        case netPlayerDropped:
			if (!got_gathered) {
				// Do this stuff only once - when we become gathered
				got_gathered = true;
				char joinMessage[256];
				game_info *info= (game_info *)NetGetGameData();
				get_network_joined_message(joinMessage, info->net_game_type);
				m_messagesWidget->set_text(std::string(joinMessage));
				m_teamWidget->activate ();
				m_colourWidget->activate ();
				m_colourWidget->set_callback(boost::bind(&JoinDialog::changeColours, this));
				m_teamWidget->set_callback(boost::bind(&JoinDialog::changeColours, this));
			}
			m_pigWidget->redraw ();
			join_result = kNetworkJoinFailedJoined;
			break;

		case netJoinErrorOccurred:
			join_result = kNetworkJoinFailedJoined;
			Stop ();
			break;
                
		case netChatMessageReceived:
			// Show chat message
			join_result = kNetworkJoinFailedJoined;
			break;

		default:
			assert(false);
	}
}

void JoinDialog::changeColours ()
{
	int requested_colour = m_colourWidget->get_value ();
	int requested_team = m_teamWidget->get_value ();
	NetChangeColors(requested_colour, requested_team);
}

void JoinDialog::getJoinAddressFromMetaserver ()
{
	// jkvw: The network metaserver code will draw our name and colour info directly from prefs.
	m_nameWidget->update_prefs ();
	m_colourWidget->update_prefs ();
	m_teamWidget->update_prefs ();

	try
	{
		IPaddress result = run_network_metaserver_ui();
		if(result.host != 0)
		{
			uint8* hostBytes = reinterpret_cast<uint8*>(&(result.host));
			char buffer[16];
			snprintf(buffer, sizeof(buffer), "%u.%u.%u.%u", hostBytes[0], hostBytes[1], hostBytes[2], hostBytes[3]);
			m_joinByAddressWidget->set_value (true);
			m_joinAddressWidget->set_text (string (buffer));
			m_joinWidget->push ();
		}
	}
	catch (...)
	{
		alert_user(infoError, strNETWORK_ERRORS, netErrMetaserverConnectionFailure, 0);
	}
}

void JoinDialog::sendChat ()
{
	string message = m_chatEntryWidget->get_text();
	
	// jkvw: Why don't we need NIBs to strip a trailing \r here, like in
	// GatherDialog::sendChat and MetaserverClientUi::sendChat?
	// Good question, burrito.
	
	if (m_chatChoiceWidget->get_value () == kMetaserverChat)
		gMetaserverClient->sendChatMessage(message);		
	else
		SendChatMessage(message);
	
	m_chatEntryWidget->set_text(string());
}

void JoinDialog::chatTextEntered (char character)
{
	if (character == '\r')
		sendChat();
}

void JoinDialog::receivedChatMessage(const std::string& senderName, uint32 senderID, const std::string& message)
{
	gMetaserverChatHistory.appendString (senderName + ": " + message);
}

void JoinDialog::receivedBroadcastMessage (const std::string& message)
{
	gMetaserverChatHistory.appendString (message);
}

void JoinDialog::chatChoiceHit ()
{
	if (m_chatChoiceWidget->get_value () == kPregameChat)
		m_chatWidget->attachHistory (&gPregameChatHistory);
	else
		m_chatWidget->attachHistory (&gMetaserverChatHistory);
}

void JoinDialog::ReceivedMessageFromPlayer(const char *player_name, const char *message)
{
	gPregameChatHistory.appendString(std::string(player_name) + ": " + std::string(message));
}

/****************************************************
 *
 *	Shared Network Game Setup Code
 *
 ****************************************************/

static bool static_allow_all_levels;
static short old_game_type;
static FileSpecifier sNetscriptFile, sOldMapFile;

bool network_game_setup(
	player_info *player_information,
	game_info *game_information,
	bool ResumingGame,
	bool& outAdvertiseGameOnMetaserver)
{
	if (run_netgame_setup_dialog(player_information, game_information, ResumingGame, outAdvertiseGameOnMetaserver))
		return true;
	else {
		// Restore enviroprefs in case user changed map
		if (sOldMapFile.Exists()) {
			struct environment_preferences_data *prefs= environment_preferences;
#ifdef SDL
			strcpy(prefs->map_file, sOldMapFile.GetPath());
#else
			prefs->map_file = sOldMapFile.GetSpec ();
#endif
			load_environment_from_preferences();
		}
		return false;
	}
}

static const vector<string> make_entry_vector (int32 entry_flags)
{	
	vector<string> result;
	
	entry_point ep;
	short index = 0;
	
	while (get_indexed_entry_point (&ep, &index, entry_flags))
		result.push_back (string (ep.level_name));
	
	return result;
}

short netgame_setup_dialog_initialise (
	DialogPTR dialog, 
	bool allow_all_levels,
	bool resuming_game)
{
	long entry_flags;

	static_allow_all_levels = allow_all_levels;

	// We use a temporary structure so that we can change things without messing with the real preferences
	network_preferences_data theAdjustedPreferences = *network_preferences;
	if (resuming_game)
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
		if (dynamic_world->player_count == 1 && GET_GAME_TYPE() == _game_of_kill_monsters)
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

	struct environment_preferences_data *prefs= environment_preferences;
#ifdef SDL
	sOldMapFile = prefs->map_file;
#else
	sOldMapFile.SetSpec(prefs->map_file);
#endif
	if (sOldMapFile.Exists()) {
		char buffer [256];
		sOldMapFile.GetName (buffer);
		QQ_copy_string_to_text_control (dialog, iTEXT_MAP_NAME, string(buffer));
	} else {
		QQ_copy_string_to_text_control (dialog, iTEXT_MAP_NAME, "Couldn't get map name");
	}

	QQ_set_selector_control_labels (dialog, iENTRY_MENU, make_entry_vector (entry_flags));

	QQ_set_selector_control_labels_from_stringset (dialog, iGAME_TYPE, kNetworkGameTypesStringSetID);
	QQ_set_selector_control_value (dialog, iGAME_TYPE, theAdjustedPreferences.game_type);
	setup_dialog_for_game_type(dialog, theAdjustedPreferences.game_type);

	QQ_copy_string_to_text_control (dialog, iGATHER_NAME, pstring_to_string (player_preferences->name));

	/* Set the menu values */
	QQ_set_selector_control_labels_from_stringset(dialog, iGATHER_COLOR, kTeamColorsStringSetID);
	QQ_set_selector_control_value(dialog, iGATHER_COLOR, player_preferences->color);
	QQ_set_selector_control_labels_from_stringset(dialog, iGATHER_TEAM, kTeamColorsStringSetID);
	QQ_set_selector_control_value(dialog, iGATHER_TEAM, player_preferences->team);
	
	QQ_set_selector_control_labels_from_stringset(dialog, iDIFFICULTY_MENU, kDifficultyLevelsStringSetID);
	QQ_set_selector_control_value(dialog, iDIFFICULTY_MENU, theAdjustedPreferences.difficulty_level);
	
	QQ_set_selector_control_value(dialog, iENTRY_MENU, level_index_to_menu_index(theAdjustedPreferences.entry_point, entry_flags));
	
	// START Benad 
	if (theAdjustedPreferences.game_type == _game_of_defense)
		QQ_insert_number_into_text_control(dialog, iKILL_LIMIT, theAdjustedPreferences.kill_limit/60);
	else
		QQ_insert_number_into_text_control(dialog, iKILL_LIMIT, theAdjustedPreferences.kill_limit); 
	// END Benad

	// If resuming an untimed game, show the "time limit" from the prefs in the grayed-out widget
	// rather than some ridiculously large number
	QQ_insert_number_into_text_control(dialog, iTIME_LIMIT, ((resuming_game && theAdjustedPreferences.game_is_untimed) ? network_preferences->time_limit : theAdjustedPreferences.time_limit)/TICKS_PER_SECOND/60);

#ifdef HAVE_LUA
	QQ_set_boolean_control_value(dialog, iUSE_SCRIPT, theAdjustedPreferences.use_netscript);
#else
	QQ_set_boolean_control_value(dialog, iUSE_SCRIPT, false);
	QQ_set_control_activity(dialog, iUSE_SCRIPT, false);
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

	// set up the game options
	QQ_set_boolean_control_value(dialog, iREAL_TIME_SOUND, theAdjustedPreferences.allow_microphone);

	set_dialog_game_options(dialog, theAdjustedPreferences.game_options);

	// If they're resuming a single-player game, now we overwrite any existing options with the cooperative-play options.
	// (We presumably twiddled the game_type from _game_of_kill_monsters up at the top there.)
	if (resuming_game && dynamic_world->player_count == 1 && theAdjustedPreferences.game_type == _game_of_cooperative_play)
	{
		setup_dialog_for_game_type(dialog, theAdjustedPreferences.game_type);
	}

	/* Setup the team popup.. */
	QQ_set_control_activity(dialog, iGATHER_TEAM, QQ_get_boolean_control_value(dialog, iFORCE_UNIQUE_TEAMS) ? CONTROL_ACTIVE : CONTROL_INACTIVE);

	// Disable certain elements when resuming a game
	if (resuming_game)
	{
		QQ_set_control_activity(dialog, iGAME_TYPE, false);
		QQ_set_control_activity(dialog, iENTRY_MENU, false);
		QQ_set_control_activity(dialog, iKILL_LIMIT, false);
		QQ_set_control_activity(dialog, iTIME_LIMIT, false);
		QQ_set_control_activity(dialog, iRADIO_NO_TIME_LIMIT, false);
		QQ_set_control_activity(dialog, iCHOOSE_MAP, false);
	}
	
	QQ_set_boolean_control_value(dialog, iALLOW_ZOOM, network_preferences->cheat_flags & _allow_tunnel_vision);
	QQ_set_boolean_control_value(dialog, iALLOW_CROSSHAIRS, network_preferences->cheat_flags & _allow_crosshair);
	QQ_set_boolean_control_value(dialog, iALLOW_LARA_CROFT, network_preferences->cheat_flags & _allow_behindview);
	
	QQ_set_boolean_control_value(dialog, iADVERTISE_GAME_ON_METASERVER, network_preferences->advertise_on_metaserver);

#ifdef SDL
	FileSpecifier theFile (theAdjustedPreferences.netscript_file);
#else
	FileSpecifier theFile;
	theFile.SetSpec(theAdjustedPreferences.netscript_file);
#endif

	set_dialog_netscript_file(dialog, theFile);

	return theAdjustedPreferences.game_type;
}


void
netgame_setup_dialog_extract_information(
	DialogPTR dialog,
	player_info *player_information,
	game_info *game_information,
	bool allow_all_levels,
	bool resuming_game,
	bool &outAdvertiseGameOnMetaserver)
{
	short updates_per_packet, update_latency;
	struct entry_point entry;
	long entry_flags;
	short game_limit_type;
	
	// get player information
	copy_string_to_pstring (QQ_copy_string_from_text_control(dialog, iGATHER_NAME), player_information->name, MAX_NET_PLAYER_NAME_LENGTH);
	player_information->color= QQ_get_selector_control_value(dialog, iGATHER_COLOR);
	player_information->team= QQ_get_selector_control_value(dialog, iGATHER_TEAM);

	pstrcpy(player_preferences->name, player_information->name);
	player_preferences->color= player_information->color;
	player_preferences->team= player_information->team;

	game_information->server_is_playing = true;
	game_information->net_game_type= QQ_get_selector_control_value(dialog, iGAME_TYPE);

	// get game information
	game_information->game_options= get_dialog_game_options(dialog, game_information->net_game_type);

	game_limit_type = get_limit_type (dialog);

	// ZZZ: don't screw with the limits if resuming.
	if (resuming_game)
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
			if (QQ_get_selector_control_value(dialog, iGAME_TYPE) == _game_of_defense)
			{
				game_information->game_options |= _game_has_kill_limit;
				game_information->time_limit = QQ_extract_number_from_text_control(dialog, iTIME_LIMIT);
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
			if (QQ_get_selector_control_value(dialog, iGAME_TYPE) == _game_of_defense)
			{
				game_information->game_options |= _game_has_kill_limit;
				game_information->time_limit = QQ_extract_number_from_text_control(dialog, iTIME_LIMIT);
				game_information->time_limit *= TICKS_PER_SECOND * 60;
			}
			else
			{
				game_information->time_limit = QQ_extract_number_from_text_control(dialog, iTIME_LIMIT);
				game_information->time_limit *= TICKS_PER_SECOND * 60;
			}
			// END Benad
		}
		game_information->kill_limit = QQ_extract_number_from_text_control(dialog, iKILL_LIMIT);
		// START Benad
		if (QQ_get_selector_control_value(dialog, iGAME_TYPE) == _game_of_defense)
			game_information->kill_limit *= 60; // It's "Time On Hill" limit, in seconds.
		// END Benad
	}

	/* Determine the entry point flags by the game type. */
	if(allow_all_levels) {
		entry_flags= NONE;
	} else {
		entry_flags= get_entry_point_flags_for_game_type(game_information->net_game_type);
	}

	menu_index_to_level_entry(QQ_get_selector_control_value(dialog, iENTRY_MENU), entry_flags, &entry);

	game_information->level_number = entry.level_number;
	strcpy(game_information->level_name, entry.level_name);
	game_information->difficulty_level = QQ_get_selector_control_value(dialog, iDIFFICULTY_MENU);

	game_information->allow_mic = QQ_get_boolean_control_value(dialog, iREAL_TIME_SOUND);

	updates_per_packet = 1;
	update_latency = 0;

	vassert(updates_per_packet > 0 && update_latency >= 0 && updates_per_packet < 16,
		csprintf(temporary, "You idiot! updates_per_packet = %d, update_latency = %d", updates_per_packet, update_latency));
	game_information->initial_updates_per_packet = updates_per_packet;
	game_information->initial_update_latency = update_latency;
	NetSetInitialParameters(updates_per_packet, update_latency);
	
	game_information->initial_random_seed = resuming_game ? dynamic_world->random_seed : (uint16) machine_tick_count();

	bool shouldUseNetscript = QQ_get_boolean_control_value(dialog, iUSE_SCRIPT);
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
	
	// initialise to disallow all cheats
	game_information->cheat_flags = 0;
	
	// add in allowed cheats
	if (QQ_get_boolean_control_value(dialog, iALLOW_ZOOM))
		game_information->cheat_flags |= _allow_tunnel_vision;
	
	if (QQ_get_boolean_control_value(dialog, iALLOW_CROSSHAIRS))
		game_information->cheat_flags |= _allow_crosshair;
	
	if (QQ_get_boolean_control_value(dialog, iALLOW_LARA_CROFT))
		game_information->cheat_flags |= _allow_behindview;

	outAdvertiseGameOnMetaserver = QQ_get_boolean_control_value (dialog, iADVERTISE_GAME_ON_METASERVER);

	// now save some of these to the preferences - only if not resuming a game
	if(!resuming_game)
	{
		network_preferences->game_type= game_information->net_game_type;

		// jkvw: The previous version had network_preferences->type = network_speed,
		// but network_speed was uninitialised.
		// network_preferences->type = GARBAGE;
		
		network_preferences->allow_microphone = game_information->allow_mic;
		network_preferences->difficulty_level = game_information->difficulty_level;
		network_preferences->entry_point = entry.level_number;
		network_preferences->game_options = game_information->game_options;
		network_preferences->time_limit = QQ_extract_number_from_text_control(dialog, iTIME_LIMIT)*60*TICKS_PER_SECOND;
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
		
		network_preferences->cheat_flags = game_information->cheat_flags;
		
		network_preferences->advertise_on_metaserver = outAdvertiseGameOnMetaserver;
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

/*************************************************************************************************
 * Function: get_dialog_game_options
 * Purpose:  extract the game option flags from the net game setup's controls
 *************************************************************************************************/

static uint16 get_dialog_game_options(
	DialogPTR dialog,
	short game_type)
{
	// These used to be options in the dialog. now they are always true, i guess.
	uint16 game_options = (_ammo_replenishes | _weapons_replenish | _specials_replenish);

	if(game_type==_game_of_cooperative_play) SET_FLAG(game_options,_overhead_map_is_omniscient,true);

	SET_FLAG(game_options, _monsters_replenish, QQ_get_boolean_control_value(dialog, iUNLIMITED_MONSTERS));
	SET_FLAG(game_options, _motion_sensor_does_not_work, QQ_get_boolean_control_value(dialog, iMOTION_SENSOR_DISABLED));
	SET_FLAG(game_options, _dying_is_penalized, QQ_get_boolean_control_value(dialog, iDYING_PUNISHED));
	SET_FLAG(game_options, _suicide_is_penalized, QQ_get_boolean_control_value(dialog, iSUICIDE_PUNISHED));
	SET_FLAG(game_options, _force_unique_teams, !QQ_get_boolean_control_value(dialog, iFORCE_UNIQUE_TEAMS)); // Reversed
	SET_FLAG(game_options, _burn_items_on_death, !QQ_get_boolean_control_value(dialog, iBURN_ITEMS_ON_DEATH)); // Reversed
	SET_FLAG(game_options, _live_network_stats, QQ_get_boolean_control_value(dialog, iREALTIME_NET_STATS));

	return game_options;
}

/*************************************************************************************************
 * Function: set_dialog_game_options
 * Purpose:  setup the game dialog's radio buttons given the game option flags.
 *************************************************************************************************/

static void set_dialog_game_options(
	DialogPTR dialog, 
	uint16 game_options)
{
	QQ_set_boolean_control_value(dialog, iUNLIMITED_MONSTERS, !!TEST_FLAG(game_options, _monsters_replenish));
	QQ_set_boolean_control_value(dialog, iMOTION_SENSOR_DISABLED, !!TEST_FLAG(game_options, _motion_sensor_does_not_work));
	QQ_set_boolean_control_value(dialog, iDYING_PUNISHED, !!TEST_FLAG(game_options, _dying_is_penalized));
	QQ_set_boolean_control_value(dialog, iSUICIDE_PUNISHED, !!TEST_FLAG(game_options, _suicide_is_penalized));
	QQ_set_boolean_control_value(dialog, iFORCE_UNIQUE_TEAMS, !TEST_FLAG(game_options, _force_unique_teams)); // Reversed
	QQ_set_boolean_control_value(dialog, iBURN_ITEMS_ON_DEATH, !TEST_FLAG(game_options, _burn_items_on_death)); // Reversed
	QQ_set_boolean_control_value(dialog, iREALTIME_NET_STATS, !!TEST_FLAG(game_options, _live_network_stats));
}


#ifdef USES_NIBS_IGNORE_FOR_NOW
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
#endif

// ZZZ: moved here from network_dialogs_macintosh.cpp
void setup_dialog_for_game_type(
	DialogPTR dialog, 
	size_t game_type)
{
	switch(game_type)
	{
		case _game_of_cooperative_play:
			QQ_set_control_activity (dialog, iFORCE_UNIQUE_TEAMS, true);
			QQ_set_control_activity (dialog, iBURN_ITEMS_ON_DEATH, false);
			QQ_set_control_activity (dialog, iUNLIMITED_MONSTERS, false);
			
			QQ_set_boolean_control_value (dialog, iBURN_ITEMS_ON_DEATH, true);
			QQ_set_boolean_control_value (dialog, iUNLIMITED_MONSTERS, true);
		
			set_limit_text(dialog, iRADIO_KILL_LIMIT, strSETUP_NET_GAME_MESSAGES, killLimitString,
                                        iTEXT_KILL_LIMIT, strSETUP_NET_GAME_MESSAGES, killsString);
                        
			setup_for_untimed_game(dialog);
			break;
			
		case _game_of_kill_monsters:
		case _game_of_king_of_the_hill:
		case _game_of_kill_man_with_ball:
		case _game_of_tag:
			QQ_set_control_activity (dialog, iFORCE_UNIQUE_TEAMS, true);
			QQ_set_control_activity (dialog, iBURN_ITEMS_ON_DEATH, true);
			QQ_set_control_activity (dialog, iUNLIMITED_MONSTERS, true);

			set_limit_text(dialog, iRADIO_KILL_LIMIT, strSETUP_NET_GAME_MESSAGES, killLimitString,
                                        iTEXT_KILL_LIMIT, strSETUP_NET_GAME_MESSAGES, killsString);

			setup_for_timed_game(dialog);
			break;

		case _game_of_capture_the_flag:
			QQ_set_control_activity (dialog, iFORCE_UNIQUE_TEAMS, false);
			QQ_set_control_activity (dialog, iBURN_ITEMS_ON_DEATH, true);
			QQ_set_control_activity (dialog, iUNLIMITED_MONSTERS, true);
			
			QQ_set_boolean_control_value (dialog, iFORCE_UNIQUE_TEAMS, true);

			set_limit_text(dialog, iRADIO_KILL_LIMIT, strSETUP_NET_GAME_MESSAGES, flagPullsString,
                                        iTEXT_KILL_LIMIT, strSETUP_NET_GAME_MESSAGES, flagsString);
                        
			setup_for_timed_game(dialog);
			break;
			
		case _game_of_rugby:
			QQ_set_control_activity (dialog, iFORCE_UNIQUE_TEAMS, false);
			QQ_set_control_activity (dialog, iBURN_ITEMS_ON_DEATH, true);
			QQ_set_control_activity (dialog, iUNLIMITED_MONSTERS, true);
			
			QQ_set_boolean_control_value (dialog, iFORCE_UNIQUE_TEAMS, true);

			set_limit_text(dialog, iRADIO_KILL_LIMIT, strSETUP_NET_GAME_MESSAGES, pointLimitString,
                                        iTEXT_KILL_LIMIT, strSETUP_NET_GAME_MESSAGES, pointsString);

			setup_for_timed_game(dialog);
			break;

		case _game_of_defense:
			QQ_set_control_activity (dialog, iFORCE_UNIQUE_TEAMS, false);
			QQ_set_control_activity (dialog, iBURN_ITEMS_ON_DEATH, true);
			QQ_set_control_activity (dialog, iUNLIMITED_MONSTERS, true);
			
			QQ_set_boolean_control_value (dialog, iFORCE_UNIQUE_TEAMS, true);
			
			set_limit_text(dialog, iRADIO_KILL_LIMIT, strSETUP_NET_GAME_MESSAGES, timeOnBaseString,
                                        iTEXT_KILL_LIMIT, strSETUP_NET_GAME_MESSAGES, minutesString);
			
			setup_for_timed_game(dialog);
			break;
			
		default:
			assert(false);
			break;
	}
}

void set_limit_type(DialogPTR dialog, short limit_type) {
	if (limit_type == iRADIO_NO_TIME_LIMIT)
		QQ_set_selector_control_value (dialog, iRADIO_NO_TIME_LIMIT, 0);
	else if (limit_type == iRADIO_TIME_LIMIT)
		QQ_set_selector_control_value (dialog, iRADIO_NO_TIME_LIMIT, 1);
	else if (limit_type == iRADIO_KILL_LIMIT)
		QQ_set_selector_control_value (dialog, iRADIO_NO_TIME_LIMIT, 2);
	else
		assert (false);
}

int get_limit_type(DialogPTR dialog) {
	switch (QQ_get_selector_control_value (dialog, iRADIO_NO_TIME_LIMIT)) {
		case 0:
			return iRADIO_NO_TIME_LIMIT;
		case 1:
			return iRADIO_TIME_LIMIT;
		case 2:
			return iRADIO_KILL_LIMIT;
		default:
			assert (false);
	}
}

// ZZZ: new function for easier sharing etc.
void setup_for_score_limited_game(
	DialogPTR dialog)
{
        set_limit_type(dialog, iRADIO_KILL_LIMIT);
        
        if(QQ_get_selector_control_value(dialog, iGAME_TYPE) == _game_of_defense)
        {
                QQ_show_control(dialog, iTIME_LIMIT); 
                QQ_show_control(dialog, iTEXT_TIME_LIMIT);
                
                QQ_show_control(dialog, iKILL_LIMIT);
                QQ_show_control(dialog, iTEXT_KILL_LIMIT);
        }
        else
        {
                QQ_hide_control(dialog, iTIME_LIMIT); 
                QQ_hide_control(dialog, iTEXT_TIME_LIMIT);
                
                QQ_show_control(dialog, iKILL_LIMIT);
                QQ_show_control(dialog, iTEXT_KILL_LIMIT);
        }
}

// ZZZ: moved here from network_dialogs_macintosh.cpp
void setup_for_untimed_game(
	DialogPTR dialog)
{
        set_limit_type(dialog, iRADIO_NO_TIME_LIMIT);
	QQ_hide_control(dialog, iKILL_LIMIT); QQ_hide_control(dialog, iTEXT_KILL_LIMIT);
	QQ_hide_control(dialog, iTIME_LIMIT); QQ_hide_control(dialog, iTEXT_TIME_LIMIT);
}

// ZZZ: moved here from network_dialogs_macintosh.cpp
void setup_for_timed_game(
	DialogPTR dialog)
{
	if (QQ_get_selector_control_value(dialog, iGAME_TYPE) != _game_of_defense)
	{
		QQ_hide_control(dialog, iTEXT_KILL_LIMIT); QQ_hide_control(dialog, iKILL_LIMIT);
		QQ_show_control(dialog, iTIME_LIMIT); QQ_show_control(dialog, iTEXT_TIME_LIMIT);
	}
	else
	{
		QQ_show_control(dialog, iTEXT_KILL_LIMIT); QQ_show_control(dialog, iKILL_LIMIT);
		QQ_show_control(dialog, iTIME_LIMIT); QQ_show_control(dialog, iTEXT_TIME_LIMIT);
	}
        set_limit_type(dialog, iRADIO_TIME_LIMIT);
}

void SNG_limit_type_hit (DialogPTR dialog)
{
	switch(QQ_get_selector_control_value (dialog, iRADIO_NO_TIME_LIMIT))
	{
		case 0:
			setup_for_untimed_game(dialog);
			break;
			
		case 1:
			setup_for_timed_game(dialog);
			break;
			
		case 2:
			setup_for_score_limited_game(dialog);
			break;
	}
}

void SNG_teams_hit (DialogPTR dialog)
{
	QQ_set_control_activity(dialog, iGATHER_TEAM, QQ_get_boolean_control_value(dialog, iFORCE_UNIQUE_TEAMS));
}

void SNG_game_type_hit (DialogPTR dialog)
{
	short new_game_type= QQ_get_selector_control_value (dialog, iGAME_TYPE);
	
	if (new_game_type != old_game_type) {
		long new_entry_flags, old_entry_flags;
		struct entry_point entry;
			
		if(static_allow_all_levels)
		{
			new_entry_flags= old_entry_flags= NONE;
		} else {
			new_entry_flags= get_entry_point_flags_for_game_type(new_game_type);
			old_entry_flags= get_entry_point_flags_for_game_type(old_game_type);
		}
		menu_index_to_level_entry(QQ_get_selector_control_value(dialog, iENTRY_MENU), old_entry_flags, &entry);
			
		/* Now reset entry points */
		QQ_set_selector_control_labels (dialog, iENTRY_MENU, make_entry_vector (new_entry_flags));
		QQ_set_selector_control_value (dialog, iENTRY_MENU, level_index_to_menu_index(entry.level_number, new_entry_flags));
		old_game_type= new_game_type;
				
		setup_dialog_for_game_type(dialog, new_game_type);
	}
}

void SNG_choose_map_hit (DialogPTR dialog)
{
	FileSpecifier mapFile;
	if (mapFile.ReadDialog (_typecode_scenario, "Map Select")) {
		environment_preferences->map_checksum = read_wad_file_checksum (mapFile);
#ifdef SDL
		strcpy(environment_preferences->map_file, mapFile.GetPath());
#else
		environment_preferences->map_file = mapFile.GetSpec();
#endif
		load_environment_from_preferences();
		
		QQ_set_selector_control_labels (dialog, iENTRY_MENU, make_entry_vector (get_entry_point_flags_for_game_type(old_game_type)));
		QQ_set_selector_control_value (dialog, iENTRY_MENU, 0);
		
		char nameBuffer [256];
		mapFile.GetName (nameBuffer);
		QQ_copy_string_to_text_control (dialog, iTEXT_MAP_NAME, string(nameBuffer));
	}
}

void SNG_use_script_hit (DialogPTR dialog)
{
	if (QQ_get_boolean_control_value(dialog, iUSE_SCRIPT)) {
		if (!sNetscriptFile.ReadDialog (_typecode_netscript, "Script Select")) {
			QQ_set_boolean_control_value(dialog, iUSE_SCRIPT, false);
		}
	}
	update_netscript_file_display(dialog);
}

bool SNG_information_is_acceptable (DialogPTR dialog)
{
	bool information_is_acceptable = true;
	short game_limit_type = QQ_get_selector_control_value(dialog, iRADIO_NO_TIME_LIMIT);
	long limit;
	
	if (information_is_acceptable)
		if (game_limit_type == 1)
		{
			limit = QQ_extract_number_from_text_control(dialog, iTIME_LIMIT);
			information_is_acceptable = limit >= 1;
		}
		
	if (information_is_acceptable)
		if (game_limit_type == 2)
		{
			limit = QQ_extract_number_from_text_control(dialog, iKILL_LIMIT);
			information_is_acceptable = limit >= 1;
		}
	
	if (information_is_acceptable)
		information_is_acceptable = !(QQ_copy_string_from_text_control(dialog, iGATHER_NAME).empty ());
		
	return (information_is_acceptable);
}

void
update_netscript_file_display(DialogPTR dialog)
{
	bool shouldUseNetscript = QQ_get_boolean_control_value(dialog, iUSE_SCRIPT);
	//	const unsigned char* theStringToUse = NULL;
	std::string theStringToUse;
	
	if(shouldUseNetscript)
	{
		if (sNetscriptFile.Exists())
		{
			char name [256];

			sNetscriptFile.GetName (name);
			theStringToUse = name;
		}
		else
		{
		  theStringToUse = "(invalid selection)";
		}
	}
	else
	  theStringToUse = "";

	QQ_copy_string_to_text_control (dialog, iTEXT_SCRIPT_NAME, theStringToUse);
}

void
set_dialog_netscript_file(DialogPTR dialog, const FileSpecifier& inFile)
{
	sNetscriptFile = inFile;
	update_netscript_file_display(dialog);
}

const FileSpecifier&
get_dialog_netscript_file(DialogPTR dialog)
{
	return sNetscriptFile;
}

void menu_index_to_level_entry(
	short menu_index, 
	long entry_flags,
	struct entry_point *entry)
{
	short  i, map_index;

	map_index= 0;
	for (i= 0; i<=menu_index; i++)
	{
		get_indexed_entry_point(entry, &map_index, entry_flags);
	}

	return;
}

int level_index_to_menu_index (int level_index, int32 entry_flags)
{	
	entry_point entry;
	short map_index = 0;

	int result = 0;
	while (get_indexed_entry_point(&entry, &map_index, entry_flags)) {
		++result;
		if (map_index == level_index)
			return result;
	}
	
	return 0;
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
	NetgameOutcomeData &outcome,
	short *index)
{
	short value;
	short graph_type = NONE;
	bool has_scores;
	
	has_scores= current_net_game_has_scores();
	
	/* Popups are 1 based */
#ifdef USES_NIBS
	value = GetControl32BitValue(outcome.SelectCtrl) - 1;
#else
	value = get_selection_control_value(outcome, iGRAPH_POPUP)-1;
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
	NetgameOutcomeData &outcome,
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

	draw_names(outcome, ranks, dynamic_world->player_count, index);
	draw_kill_bars(outcome, ranks, dynamic_world->player_count, index, false, false);
}


// ZZZ: team vs team carnage (analogous to draw_player_graph's player vs player carnage)
// THIS IS UNFINISHED (and thus unused at the moment :) )
void draw_team_graph(
	NetgameOutcomeData &outcome,
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
	NetgameOutcomeData &outcome)
{
	draw_names(outcome, rankings, dynamic_world->player_count, NONE);
	draw_kill_bars(outcome, rankings, dynamic_world->player_count, NONE, true, false);
}


// (ZZZ annotation:) Total Team Carnage graph
void draw_team_totals_graph(
	NetgameOutcomeData &outcome)
{
	short team_index, player_index, opponent_player_index, num_teams;
	bool found_team_of_current_color;
	struct net_rank ranks[MAXIMUM_NUMBER_OF_PLAYERS];

	objlist_clear(ranks, MAXIMUM_NUMBER_OF_PLAYERS);
	for (team_index = 0, num_teams = 0; team_index < NUMBER_OF_TEAM_COLORS; team_index++) {
	  found_team_of_current_color = false;
	  if (team_damage_given[team_index].kills ||
	      (team_damage_taken[team_index].kills + team_monster_damage_taken[team_index].kills)) {
	    found_team_of_current_color = true;
	  } else {
	    for (player_index = 0; player_index < dynamic_world->player_count; player_index++) {
	      struct player_data *player = get_player_data(player_index);
	      if (player->team == team_index) {
		found_team_of_current_color = true;
		break;
	      }
	    }
	  }
	  if (found_team_of_current_color) {
	    ranks[num_teams].player_index = NONE;
	    ranks[num_teams].color = team_index;
	    ranks[num_teams].kills = team_damage_given[team_index].kills;
	    ranks[num_teams].deaths = team_damage_taken[team_index].kills + team_monster_damage_taken[team_index].kills;
	    ranks[num_teams].friendly_fire_kills = team_friendly_fire[team_index].kills;
	    num_teams++;
	  }
	}

	/* Setup the team rankings.. */
	for (team_index= 0; team_index<num_teams; team_index++)
	  {
	    ranks[team_index].ranking= ranks[team_index].kills - ranks[team_index].deaths;
	  }
	qsort(ranks, num_teams, sizeof(struct net_rank), team_rank_compare);
	
	draw_names(outcome, ranks, num_teams, NONE);
	draw_kill_bars(outcome, ranks, num_teams, NONE, true, true);
}



// (ZZZ annotation:) Time on Hill, etc. graph
void draw_total_scores_graph(
	NetgameOutcomeData &outcome)
{
	struct net_rank ranks[MAXIMUM_NUMBER_OF_PLAYERS];
	
	/* Use a private copy to avoid boning things */
	objlist_copy(ranks, rankings, dynamic_world->player_count);

	/* First qsort the rankings arrray by game_ranking.. */
	qsort(ranks, dynamic_world->player_count, sizeof(struct net_rank), score_rank_compare);

	draw_names(outcome, ranks, dynamic_world->player_count, NONE);
	draw_score_bars(outcome, ranks, dynamic_world->player_count);
}



// (ZZZ annotation:) Team Time on Hill, etc. graph
void draw_team_total_scores_graph(
	NetgameOutcomeData &outcome)
{
	short team_index, team_count;
	struct net_rank ranks[MAXIMUM_NUMBER_OF_PLAYERS];

	objlist_clear(ranks, MAXIMUM_NUMBER_OF_PLAYERS);
	team_count = 0;

	for (team_index = 0; team_index < NUMBER_OF_TEAM_COLORS; ++team_index) {
		bool team_is_valid = false;
		short kills, deaths;
		long ranking = get_team_net_ranking(team_index, &kills, &deaths, true);

		if (kills || deaths || ranking) {
			team_is_valid = true;
		} else {
			for (short player_index = 0; player_index < dynamic_world->player_count; ++player_index) {
				struct player_data *player = get_player_data(player_index);
				if (player->team == team_index) {
					team_is_valid = true;
					break;
				}
			}
		}
    
		if (team_is_valid) {
			ranks[team_count].kills = kills;
			ranks[team_index].deaths = deaths;
			ranks[team_count].player_index = NONE;
			ranks[team_count].color = team_index;
			ranks[team_count].game_ranking = ranking;
			ranks[team_count].friendly_fire_kills = team_friendly_fire[NUMBER_OF_TEAM_COLORS].kills;
			team_count++;
		}
	}

	/* Now qsort our team stuff. */
	qsort(ranks, team_count, sizeof(struct net_rank), score_rank_compare);

	draw_names(outcome, ranks, team_count, NONE);
	draw_score_bars(outcome, ranks, team_count);
}



// (ZZZ) ripped this out of draw_kill_bars since we can share this bit but not the rest of draw_kill_bars().
// (ZZZ annotation:) Update the "N deaths total (D dpm) including S suicides"-type text at the bottom.
void update_carnage_summary(                 	
	NetgameOutcomeData &outcome,
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
    SetStaticPascalText(outcome.KillsTextCtrl, ptemporary);
#else
    copy_pstring_to_static_text(outcome, iTOTAL_KILLS, ptemporary);
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
    SetStaticPascalText(outcome.DeathsTextCtrl, ptemporary);
#else
    copy_pstring_to_static_text(outcome, iTOTAL_DEATHS, ptemporary);
#endif
}



// ZZZ: ripped out of update_damage_item
// (ZZZ annotation:) Demultiplex to draw_X_graph() function based on find_graph_mode().
void draw_new_graph(
	NetgameOutcomeData &outcome)
{
    short   graph_type;
    short   index;

    graph_type= find_graph_mode(outcome, &index);

	switch(graph_type)
	{
		case _player_graph:
			draw_player_graph(outcome, index);
			break;

		case _total_carnage_graph:
			draw_totals_graph(outcome);
			break;

		case _total_scores_graph:
			draw_total_scores_graph(outcome);
			break;

		/* These two functions need to have the team colors. */
		case _total_team_carnage_graph:
			draw_team_totals_graph(outcome);
			break;

		case _total_team_scores_graph:
			draw_team_total_scores_graph(outcome);
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

#endif // !defined(DISABLE_NETWORKING)

