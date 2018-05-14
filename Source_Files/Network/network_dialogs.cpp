/*
NETWORK_DIALOGS.C  (network_dialogs.cpp)

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
#include "game_wad.h" // get_map_file

#include <map>

// For LAN netgame location services
#include	<sstream>
#include	"network_private.h" // actually just need "network_dialogs_private.h"
#include	"SSLP_API.h"

// for game types...
#include "network_dialogs.h"
#include "TextStrings.h"

#include "network_dialog_widgets_sdl.h"
#include "screen.h"
#include "SoundManager.h"
#include "progress.h"


extern void NetRetargetJoinAttempts(const IPaddress* inAddress);


// Metaserver Globals
// We can't construct a MetaserverClient until MetaserverClient::s_instances is initialised.
MetaserverClient* gMetaserverClient = NULL;

// Chat History Globals
ChatHistory gMetaserverChatHistory;
ChatHistory gPregameChatHistory;




////////////////////////////////////////////////////////////////////////////////
// LAN game-location services support

static const string
get_sslp_service_type()
{
	return kNetworkSetupProtocolID;
}


GathererAvailableAnnouncer::GathererAvailableAnnouncer()
{
	strncpy(mServiceInstance.sslps_type, get_sslp_service_type().c_str(), SSLP_MAX_TYPE_LENGTH);
	strncpy(mServiceInstance.sslps_name, "Boomer", SSLP_MAX_NAME_LENGTH);
	memset(&(mServiceInstance.sslps_address), '\0', sizeof(mServiceInstance.sslps_address));
	mServiceInstance.sslps_address.port = SDL_SwapBE16(GAME_PORT);
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
		memset(myPlayerInfo.long_serial_number, 0, LONG_SERIAL_NUMBER_LENGTH);
		
		std::unique_ptr<GameAvailableMetaserverAnnouncer> metaserverAnnouncer;
		if(NetEnter())
		{
			bool gather_dialog_result;
		
			if(NetGather(&myGameInfo, sizeof(game_info), (void*) &myPlayerInfo, 
				sizeof(myPlayerInfo), inResumingGame))
			{
				GathererAvailableAnnouncer announcer;
				
				if (!gMetaserverClient) gMetaserverClient = new MetaserverClient ();

				if(advertiseOnMetaserver)
				{
					try
					{
						metaserverAnnouncer.reset(new GameAvailableMetaserverAnnouncer(myGameInfo));
					}
					catch (const MetaserverClient::LoginDeniedException& e)
					{
						char message[1024];
						if (e.code() == MetaserverClient::LoginDeniedException::BadUserOrPassword)
						{
							strncpy(message, "Login denied: bad username or password. Your game could not be advertised on the Internet.", 1024);
						}
						else if (e.code() == MetaserverClient::LoginDeniedException::UserAlreadyLoggedIn)
						{
							strncpy(message, "Login denied: that user is already logged in. Your game could not be advertised on the Internet.", 1024);
						}
						else if (e.code() == MetaserverClient::LoginDeniedException::AccountAlreadyLoggedIn)
						{
							strncpy(message, "Login denied: that account is already logged in. Your game could not be advertised on the Internet.", 1024);
						}
						else if (e.code() == MetaserverClient::LoginDeniedException::RoomFull)
						{
							strncpy(message, "Login denied: room full! Your game could not be advertised on the Internet.", 1024);
						}
						else if (e.code() == MetaserverClient::LoginDeniedException::AccountLocked)
						{
							strncpy(message, "Login denied: your account is locked. Your game could not be advertised on the Internet.", 1024);
						}
						else
						{
							sprintf(message, "There was a problem connecting to the server that tracks Internet games (%s). Please try again later.", e.what());
						}

						alert_user(message, 0);
					}
					catch (const MetaserverClient::ServerConnectException&)
					{
						alert_user(infoError, strNETWORK_ERRORS, netWarnCouldNotAdvertiseOnMetaserver, 0);
					}
				}

				gather_dialog_result = GatherDialog::Create()->GatherNetworkGameByRunning();
				
			} else {
				gather_dialog_result = false;
			}
			
			if (gather_dialog_result) {
				NetDoneGathering();
				if (advertiseOnMetaserver) 
				{
					metaserverAnnouncer->Start(myGameInfo.time_limit);
					gMetaserverClient->setMode(1, NetSessionIdentifier());
					gMetaserverClient->pump();
				}
				successful= true;
			}
			else
			{
				delete gMetaserverClient;
				gMetaserverClient = new MetaserverClient();
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

	gMetaserverClient->associateNotificationAdapter(0);

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

	m_startWidget->deactivate ();
	
	NetSetGatherCallbacks(this);
	
	m_chatChoiceWidget->set_callback(boost::bind(&GatherDialog::chatChoiceHit, this));
	m_chatEntryWidget->set_callback(boost::bind(&GatherDialog::chatTextEntered, this, _1));
	
	gPregameChatHistory.clear ();
	NetSetChatCallbacks(this);

	BoolPref autoGatherPref (network_preferences->autogather);
	Binder<bool> binder (m_autogatherWidget, &autoGatherPref);
	binder.migrate_second_to_first ();
	
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
	
	bool result = Run ();
	
	binder.migrate_first_to_second ();
	
	// Save autogather setting, even if we cancel the dialog
	write_preferences ();
	
	return result;
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
		std::map<int, prospective_joiner_info>::iterator it;
		it = m_ungathered_players.begin ();
		while (it != m_ungathered_players.end () && NetGetNumberOfPlayers() < MAXIMUM_NUMBER_OF_PLAYERS) {
			gathered_player ((it++)->second);
		}
	}
}

void GatherDialog::update_ungathered_widget ()
{
	vector<prospective_joiner_info> temp;

	for (std::map<int, prospective_joiner_info>::iterator it = m_ungathered_players.begin (); it != m_ungathered_players.end (); ++it)
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
	if (NetGetNumberOfPlayers() >= MAXIMUM_NUMBER_OF_PLAYERS) return false;
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
	for (std::map<int, prospective_joiner_info>::iterator it = m_ungathered_players.begin (); it != m_ungathered_players.end (); ++it)
		NetHandleUngatheredPlayer ((*it).second);
	
	Stop (true);
}

void GatherDialog::JoinSucceeded(const prospective_joiner_info* player)
{
	if (NetGetNumberOfPlayers () > 1)
		m_startWidget->activate ();
	
	m_pigWidget->redraw ();

	if (gMetaserverClient->isConnected())
	{
		gMetaserverClient->announcePlayersInGame(NetGetNumberOfPlayers());
	}
}

void GatherDialog::JoiningPlayerDropped(const prospective_joiner_info* player)
{
	std::map<int, prospective_joiner_info>::iterator it = m_ungathered_players.find (player->stream_id);
	if (it != m_ungathered_players.end ())
		m_ungathered_players.erase (it);
	
	update_ungathered_widget ();
}

void GatherDialog::JoinedPlayerDropped(const prospective_joiner_info* player)
{
	if (NetGetNumberOfPlayers () < 2)
		m_startWidget->deactivate ();

	m_pigWidget->redraw ();

	if (gMetaserverClient->isConnected())
	{
		gMetaserverClient->announcePlayersInGame(NetGetNumberOfPlayers());
	}
}

void GatherDialog::JoinedPlayerChanged(const prospective_joiner_info* player)
{	
	m_pigWidget->redraw ();
}

void GatherDialog::sendChat ()
{
	string message = m_chatEntryWidget->get_text();
		
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
	ColoredChatEntry e;
	e.type = ColoredChatEntry::ChatMessage;
	e.sender = player_name;
	e.message = message;

	gPregameChatHistory.append(e);
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
			if (gMetaserverClient && gMetaserverClient->isConnected())
			{
				gMetaserverClient->setMode(1, NetSessionIdentifier());
				gMetaserverClient->pump();
			}
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

JoinDialog::JoinDialog() : got_gathered(false), skipToMetaserver(network_preferences->join_metaserver_by_default)
	{ if (!gMetaserverClient) gMetaserverClient = new MetaserverClient (); }

JoinDialog::~JoinDialog ()
{
	gMetaserverClient->associateNotificationAdapter(0);
	
	delete m_cancelWidget;
	delete m_joinWidget;
	delete m_joinMetaserverWidget;
	delete m_joinAddressWidget;
	delete m_joinByAddressWidget;
	delete m_nameWidget;
	delete m_colourWidget;
	delete m_teamWidget;
	delete m_messagesWidget;
	delete m_pigWidget;
	delete m_chatEntryWidget;
	delete m_chatChoiceWidget;
	delete m_chatWidget;
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
	
	getcstr(temporary, strJOIN_DIALOG_MESSAGES, _join_dialog_welcome_string);
	m_messagesWidget->set_text(temporary);
	
	CStringPref joinAddressPref (network_preferences->join_address, 255);
	binders.insert<std::string> (m_joinAddressWidget, &joinAddressPref);
	BoolPref joinByAddressPref (network_preferences->join_by_address);
	binders.insert<bool> (m_joinByAddressWidget, &joinByAddressPref);
	
	CStringPref namePref (player_preferences->name, MAX_NET_PLAYER_NAME_LENGTH);
	binders.insert<std::string> (m_nameWidget, &namePref);
	Int16Pref colourPref (player_preferences->color);
	binders.insert<int> (m_colourWidget, &colourPref);
	Int16Pref teamPref (player_preferences->team);
	binders.insert<int> (m_teamWidget, &teamPref);
	
	binders.migrate_all_second_to_first ();
	
	Run ();
	
	binders.migrate_all_first_to_second ();
	
	return join_result;
	
	// We'll choose to use old prefs or new changes when we return into network_join
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
	copy_string_to_cstring (m_nameWidget->get_text (), myPlayerInfo.name, MAX_NET_PLAYER_NAME_LENGTH);
	myPlayerInfo.team = m_teamWidget->get_value ();
	myPlayerInfo.desired_color = m_colourWidget->get_value ();
	
	// jkvw: It may look like we're passing our player name into NetGameJoin,
	//       but network code will later draw the name directly from prefs.
	binders.migrate_all_first_to_second ();	
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
		
		getcstr(temporary, strJOIN_DIALOG_MESSAGES, _join_dialog_waiting_string);
		m_messagesWidget->set_text(temporary);

		if (!m_joinByAddressWidget->get_value()) {
			join_announcer.reset(new JoinerSeekingGathererAnnouncer(true));
		}
		
		respondToJoinHit ();
	}
}

void JoinDialog::gathererSearch ()
{
	if (skipToMetaserver)
	{
		skipToMetaserver = false;
		getJoinAddressFromMetaserver();
	}

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
				if (!(info->game_options & _force_unique_teams)) {
					m_teamWidget->activate ();
				}
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
	binders.migrate_all_first_to_second ();

	try
	{
		IPaddress result = run_network_metaserver_ui();
		if(result.host != 0)
		{
			uint8* hostBytes = reinterpret_cast<uint8*>(&(result.host));
			std::ostringstream s;
			s << (uint16)hostBytes[0] << '.'
			  << (uint16)hostBytes[1] << '.'
			  << (uint16)hostBytes[2] << '.'
			  << (uint16)hostBytes[3];
			if (result.port != DEFAULT_GAME_PORT)
			{
				s << ':' << result.port;
			}
			m_joinByAddressWidget->set_value (true);
			m_joinAddressWidget->set_text (s.str());
			m_joinWidget->push ();
		}
	}
	catch (const MetaserverClient::LoginDeniedException& e)
	{
		char message[1024];
		if (e.code() == MetaserverClient::LoginDeniedException::BadUserOrPassword)
		{
			strncpy(message, "Login denied: bad username or password.", 1024);
		}
		else if (e.code() == MetaserverClient::LoginDeniedException::UserAlreadyLoggedIn)
		{
			strncpy(message, "Login denied: that user is already logged in.", 1024);
		}
		else if (e.code() == MetaserverClient::LoginDeniedException::AccountAlreadyLoggedIn)
		{
			strncpy(message, "Login denied: that account is already logged in.", 1024);
		}
		else if (e.code() == MetaserverClient::LoginDeniedException::RoomFull)
		{
			strncpy(message, "Login denied: room is full!?", 1024);
		}
		else if (e.code() == MetaserverClient::LoginDeniedException::AccountLocked)
		{
			strncpy(message, "Login denied: your account is locked.", 1024);
		}
		else
		{
			sprintf(message, "There was a problem connecting to the server that tracks Internet games (%s). Please try again later.", e.what());
		}

		alert_user(message, 0);
	}
	catch (const MetaserverClient::ServerConnectException&)
	{
		alert_user(infoError, strNETWORK_ERRORS, netErrMetaserverConnectionFailure, 0);
	}
}

void JoinDialog::sendChat ()
{
	string message = m_chatEntryWidget->get_text();
	
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

void JoinDialog::chatChoiceHit ()
{
	if (m_chatChoiceWidget->get_value () == kPregameChat)
		m_chatWidget->attachHistory (&gPregameChatHistory);
	else
		m_chatWidget->attachHistory (&gMetaserverChatHistory);
}

void JoinDialog::ReceivedMessageFromPlayer(const char *player_name, const char *message)
{
	ColoredChatEntry e;
	e.type = ColoredChatEntry::ChatMessage;
	e.sender = player_name;
	e.message = message;
	
	gPregameChatHistory.append(e);
}

/****************************************************
 *
 *	Shared Network Game Setup Code
 *
 ****************************************************/

bool network_game_setup(
	player_info *player_information,
	game_info *game_information,
	bool ResumingGame,
	bool& outAdvertiseGameOnMetaserver)
{
	if (SetupNetgameDialog::Create ()->SetupNetworkGameByRunning (player_information, game_information, ResumingGame, outAdvertiseGameOnMetaserver)) {
		write_preferences ();
		return true;
	} else {
		read_preferences ();
		load_environment_from_preferences(); // In case user changed map
		return false;
	}
}

// converts menu index <---> level index
class LevelInt16Pref : public Bindable<int>
{
public:
	LevelInt16Pref (int16& pref, int& gametype) : m_pref (pref), m_gametype (gametype) {}
	
	virtual int bind_export ()
	{
		int32 entry_flags = get_entry_point_flags_for_game_type (m_gametype);
		return level_index_to_menu_index (m_pref, entry_flags);
	}
	
	virtual void bind_import (int value)
	{
		int32 entry_flags = get_entry_point_flags_for_game_type (m_gametype);
		m_pref = menu_index_to_level_index (value, entry_flags);
	}
	
protected:
	int16& m_pref;
	int& m_gametype;
};

class TimerInt32Pref : public Bindable<int>
{
public:
	TimerInt32Pref (int32& pref) : m_pref (pref) {}
	
	virtual int bind_export ()
	{
		return m_pref / (60 * TICKS_PER_SECOND);
	}
	
	virtual void bind_import (int value)
	{
		m_pref = static_cast<int32>(value) * (60 * TICKS_PER_SECOND);
	}
	
protected:
	int32& m_pref;
};

// limit type is represented by a bool and a bit in game options
class LimitTypePref : public Bindable<int>
{
public:
	LimitTypePref (bool& untimed_pref, uint16& options_pref, uint16 kill_limit_mask)
		: m_untimed (untimed_pref)
		, m_kill_limited (options_pref, kill_limit_mask)
		{}
	
	virtual int bind_export ()
	{
		if (!m_untimed.bind_export ())
			return duration_time_limit;
		else if (m_kill_limited.bind_export ())
			return duration_kill_limit;
		else
			return duration_no_time_limit;
	}
	
	virtual void bind_import (int value)
	{
		if (value == duration_no_time_limit) {
			m_untimed.bind_import (true);
			m_kill_limited.bind_import (false);
		} else if (value == duration_time_limit) {
			m_untimed.bind_import (false);
			m_kill_limited.bind_import (false);
		} else {
			m_untimed.bind_import (true);
			m_kill_limited.bind_import (true);
		}
	}
	
protected:
	BoolPref m_untimed;
	BitPref m_kill_limited;
};

class GametypePref : public Bindable<int>
{
public:
	GametypePref (int16& pref) : m_pref (pref) {}
	
	virtual int bind_export ()
	{
		return ((m_pref < 5) ? m_pref : m_pref - 1);
	}
	
	virtual void bind_import (int value)
	{
		m_pref = ((value < 5) ? value : value + 1);
	}
	
protected:
	int16& m_pref;
};

class LatencyTolerancePref : public Bindable<int>
{
public:
	LatencyTolerancePref (int32& pref) : m_pref(pref) { }
	
	virtual int bind_export () {
		return (m_pref == 0) ? 5 : (m_pref - 1);
	}

	virtual void bind_import(int value) {
		m_pref = (value == 5) ? 0 : (value + 1);
	}
protected:
	int32& m_pref;
};

static const vector<string> make_entry_vector (int32 entry_flags)
{	
	vector<string> result;
	
	entry_point ep;
	short index = 0;
	
	while (get_indexed_entry_point (&ep, &index, entry_flags))
		result.push_back (string (ep.level_name));
	
	return result;
}

SetupNetgameDialog::SetupNetgameDialog()
{  }

SetupNetgameDialog::~SetupNetgameDialog ()
{
	delete m_cancelWidget;
	delete m_okWidget;
	
	delete m_nameWidget;
	delete m_colourWidget;
	delete m_teamWidget;
	
	delete m_mapWidget;
	delete m_levelWidget;
	delete m_gameTypeWidget;
	delete m_difficultyWidget;
	
	delete m_timeLimitWidget;
	delete m_scoreLimitWidget;
	
	delete m_aliensWidget;
	delete m_allowTeamsWidget;
	delete m_deadPlayersDropItemsWidget;
	delete m_penalizeDeathWidget;
	delete m_penalizeSuicideWidget;
	
	delete m_useMetaserverWidget;
	
	delete m_useScriptWidget;
	delete m_scriptWidget;
	
	delete m_allowMicWidget;
	
	delete m_liveCarnageWidget;
	delete m_motionSensorWidget;
	
	delete m_zoomWidget;
	delete m_crosshairWidget;
	delete m_overlayWidget;
	delete m_laraCroftWidget;
	delete m_carnageMessagesWidget;
	
	delete m_useUpnpWidget;
}

extern int32& hub_get_minimum_send_period();

bool SetupNetgameDialog::SetupNetworkGameByRunning (
	player_info *player_information,
	game_info *game_information,
	bool resuming_game,
	bool& outAdvertiseGameOnMetaserver)
{
	int32 entry_flags;

	m_allow_all_levels = allLevelsAllowed ();
	
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
		m_allow_all_levels = true;
		
		// If resuming an untimed game, show the "time limit" from the prefs in the grayed-out widget
		// rather than some ridiculously large number
		if (theAdjustedPreferences.game_is_untimed)
			theAdjustedPreferences.time_limit = theAdjustedPreferences.time_limit/TICKS_PER_SECOND/60;
		
		// Disable certain elements when resuming a game
		m_gameTypeWidget->deactivate ();
		m_levelWidget->deactivate ();
		m_scoreLimitWidget->deactivate ();
		m_timeLimitWidget->deactivate ();
		m_limitTypeWidget->deactivate ();
		m_mapWidget->deactivate ();
	}

	// if we're resuming, use the temporary prefs structure, otherwise use the prefs as usual
	network_preferences_data* active_network_preferences = resuming_game ? &theAdjustedPreferences : network_preferences;

	m_old_game_type = active_network_preferences->game_type;

	/* Fill in the entry points */
	if(m_allow_all_levels)
	{
		entry_flags= NONE;
	} else {
		entry_flags= get_entry_point_flags_for_game_type(active_network_preferences->game_type);
	}
	m_levelWidget->set_labels (make_entry_vector (entry_flags));
	m_gameTypeWidget->set_labels (kNetworkGameTypesStringSetID);
	m_colourWidget->set_labels (kTeamColorsStringSetID);
	m_teamWidget->set_labels (kTeamColorsStringSetID);
	m_difficultyWidget->set_labels (kDifficultyLevelsStringSetID);
	m_limitTypeWidget->set_labels (kEndConditionTypeStringSetID);

	vector<string> toleranceLabels;
	toleranceLabels.push_back("33 ms");
	toleranceLabels.push_back("66 ms");
	toleranceLabels.push_back("100 ms");
	toleranceLabels.push_back("133 ms");
	toleranceLabels.push_back("166 ms");
	toleranceLabels.push_back("2 sec");
	m_latencyToleranceWidget->set_labels (toleranceLabels);
	
	BinderSet binders;
	
	CStringPref namePref (player_preferences->name, MAX_NET_PLAYER_NAME_LENGTH);
	binders.insert<std::string> (m_nameWidget, &namePref);
	Int16Pref colourPref (player_preferences->color);
	binders.insert<int> (m_colourWidget, &colourPref);
	Int16Pref teamPref (player_preferences->team);
	binders.insert<int> (m_teamWidget, &teamPref);

	FilePref mapPref (environment_preferences->map_file);
	binders.insert<FileSpecifier> (m_mapWidget, &mapPref);

	LevelInt16Pref levelPref (active_network_preferences->entry_point, m_old_game_type);
	binders.insert<int> (m_levelWidget, &levelPref);
	GametypePref gameTypePref (active_network_preferences->game_type);
	binders.insert<int> (m_gameTypeWidget, &gameTypePref);
	Int16Pref difficultyPref (active_network_preferences->difficulty_level);
	binders.insert<int> (m_difficultyWidget, &difficultyPref);

	LimitTypePref limitTypePref (active_network_preferences->game_is_untimed, active_network_preferences->game_options, _game_has_kill_limit);
	binders.insert<int> (m_limitTypeWidget, &limitTypePref);
	TimerInt32Pref timeLimitPref (active_network_preferences->time_limit);
	binders.insert<int> (m_timeLimitWidget, &timeLimitPref);
	Int16Pref scoreLimitPref (active_network_preferences->kill_limit);
	binders.insert<int> (m_scoreLimitWidget, &scoreLimitPref);
	
	BitPref aliensPref (active_network_preferences->game_options, _monsters_replenish);
	binders.insert<bool> (m_aliensWidget, &aliensPref);
	BitPref allowTeamsPref (active_network_preferences->game_options, _force_unique_teams, true);
	binders.insert<bool> (m_allowTeamsWidget, &allowTeamsPref);
	BitPref deadPlayersDropItemsPref (active_network_preferences->game_options, _burn_items_on_death, true);
	binders.insert<bool> (m_deadPlayersDropItemsWidget, &deadPlayersDropItemsPref);
	BitPref penalizeDeathPref (active_network_preferences->game_options, _dying_is_penalized);
	binders.insert<bool> (m_penalizeDeathWidget, &penalizeDeathPref);
	BitPref penalizeSuicidePref (active_network_preferences->game_options, _suicide_is_penalized);
	binders.insert<bool> (m_penalizeSuicideWidget, &penalizeSuicidePref);
				
	BoolPref useMetaserverPref (active_network_preferences->advertise_on_metaserver);
	binders.insert<bool> (m_useMetaserverWidget, &useMetaserverPref);
	
	BoolPref allowMicPref (active_network_preferences->allow_microphone);
	binders.insert<bool> (m_allowMicWidget, &allowMicPref);
	
	BitPref liveCarnagePref (active_network_preferences->game_options, _live_network_stats);
	binders.insert<bool> (m_liveCarnageWidget, &liveCarnagePref);
	BitPref motionSensorPref (active_network_preferences->game_options, _motion_sensor_does_not_work);
	binders.insert<bool> (m_motionSensorWidget, &motionSensorPref);

	BitPref zoomPref (active_network_preferences->cheat_flags, _allow_tunnel_vision);
	binders.insert<bool> (m_zoomWidget, &zoomPref);
	BitPref crosshairPref (active_network_preferences->cheat_flags, _allow_crosshair);
	binders.insert<bool> (m_crosshairWidget, &crosshairPref);
	BitPref overlayPref (active_network_preferences->cheat_flags, _allow_overlay_map);
	binders.insert<bool> (m_overlayWidget, &overlayPref);
	BitPref laraCroftPref (active_network_preferences->cheat_flags, _allow_behindview);
	binders.insert<bool> (m_laraCroftWidget, &laraCroftPref);
	BitPref carnageMessagesPref (active_network_preferences->cheat_flags, _disable_carnage_messages, true);
	binders.insert<bool> (m_carnageMessagesWidget, &carnageMessagesPref);
	BitPref savingLevelPref (active_network_preferences->cheat_flags, _disable_saving_level, true);
	binders.insert<bool> (m_savingLevelWidget, &savingLevelPref);

	BoolPref useScriptPref (active_network_preferences->use_netscript);
	binders.insert<bool> (m_useScriptWidget, &useScriptPref);
	FilePref scriptPref (active_network_preferences->netscript_file);
	binders.insert<FileSpecifier> (m_scriptWidget, &scriptPref);
	
	BoolPref useUpnpPref (active_network_preferences->attempt_upnp);
	binders.insert<bool> (m_useUpnpWidget, &useUpnpPref);

	LatencyTolerancePref latencyTolerancePref (hub_get_minimum_send_period());
	binders.insert<int> (m_latencyToleranceWidget, &latencyTolerancePref);

	binders.migrate_all_second_to_first ();
	
	m_cancelWidget->set_callback (boost::bind (&SetupNetgameDialog::Stop, this, false));
	m_okWidget->set_callback (boost::bind (&SetupNetgameDialog::okHit, this));
	m_limitTypeWidget->set_callback (boost::bind (&SetupNetgameDialog::limitTypeHit, this));
	m_allowTeamsWidget->set_callback (boost::bind (&SetupNetgameDialog::teamsHit, this));
	m_gameTypeWidget->set_callback (boost::bind (&SetupNetgameDialog::gameTypeHit, this));
	m_mapWidget->set_callback (boost::bind (&SetupNetgameDialog::chooseMapHit, this));

	setupForGameType ();

	if (m_limitTypeWidget->get_value () == duration_kill_limit)
		setupForScoreGame ();
	else if (m_limitTypeWidget->get_value () == duration_no_time_limit)
		setupForUntimedGame ();
	else
		setupForTimedGame ();

	/* Setup the team popup.. */
	if (!m_allowTeamsWidget->get_value ())
		m_teamWidget->deactivate ();
	
	if (Run ()) {
	
		// migrate widget settings to preferences structure
		binders.migrate_all_first_to_second ();
	
		strncpy (player_information->name, player_preferences->name, MAX_NET_PLAYER_NAME_LENGTH+1);
		player_information->color = player_preferences->color;
		player_information->team = player_preferences->team;

		game_information->server_is_playing = true;
		game_information->net_game_type = active_network_preferences->game_type;
		
		game_information->game_options = active_network_preferences->game_options;
		game_information->game_options |= (_ammo_replenishes | _weapons_replenish | _specials_replenish);
		if (active_network_preferences->game_type == _game_of_cooperative_play)
			game_information->game_options |= _overhead_map_is_omniscient;

		// ZZZ: don't screw with the limits if resuming.
		if (resuming_game)
		{
			game_information->time_limit = dynamic_world->game_information.game_time_remaining;
			game_information->kill_limit = dynamic_world->game_information.kill_limit;
		} else {
			if (!active_network_preferences->game_is_untimed)
				game_information->time_limit = m_timeLimitWidget->get_value () * TICKS_PER_SECOND * 60;
			else
				game_information->time_limit = INT32_MAX;
			
			game_information->kill_limit = active_network_preferences->kill_limit;
		}
		
		entry_point entry;
		menu_index_to_level_entry (active_network_preferences->entry_point, NONE, &entry);
		game_information->level_number = entry.level_number;
		strncpy (game_information->level_name, entry.level_name, MAX_LEVEL_NAME_LENGTH+1);
		game_information->parent_checksum = read_wad_file_checksum(get_map_file());
		game_information->difficulty_level = active_network_preferences->difficulty_level;
		game_information->allow_mic = active_network_preferences->allow_microphone;

		int updates_per_packet = 1;
		int update_latency = 0;
		vassert(updates_per_packet > 0 && update_latency >= 0 && updates_per_packet < 16,
			csprintf(temporary, "You idiot! updates_per_packet = %d, update_latency = %d", updates_per_packet, update_latency));
		game_information->initial_updates_per_packet = updates_per_packet;
		game_information->initial_update_latency = update_latency;
		NetSetInitialParameters(updates_per_packet, update_latency);
	
		game_information->initial_random_seed = resuming_game ? dynamic_world->random_seed : (uint16) machine_tick_count();

#if mac
		FileSpecifier theNetscriptFile;
		theNetscriptFile.SetSpec (active_network_preferences->netscript_file);
#else
		FileSpecifier theNetscriptFile (active_network_preferences->netscript_file);
#endif

		// This will be set true below if appropriate
		SetNetscriptStatus(false);
	
		if (active_network_preferences->use_netscript)
		{
			OpenedFile script_file;

			if (theNetscriptFile.Open (script_file))
			{
				int32 script_length;
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
				;
		}
	
		game_information->cheat_flags = active_network_preferences->cheat_flags;

		outAdvertiseGameOnMetaserver = active_network_preferences->advertise_on_metaserver;

		//if(shouldUseNetscript)
		//{
			// Sorry, probably should use a FileSpecifier in the prefs,
			// but that means prefs reading/writing have to be reworked instead
		//	strncpy(network_preferences->netscript_file, theNetscriptFile.GetPath(), sizeof(network_preferences->netscript_file));
		//}
		
		return true;

	} else // dialog was cancelled
		return false;
}

void SetupNetgameDialog::setupForUntimedGame ()
{
	m_timeLimitWidget->hide ();
	m_scoreLimitWidget->hide ();
}

void SetupNetgameDialog::setupForTimedGame ()
{
	m_timeLimitWidget->show ();
	m_scoreLimitWidget->hide ();
}

void SetupNetgameDialog::setupForScoreGame ()
{
	m_timeLimitWidget->hide ();
	m_scoreLimitWidget->show ();
}

void SetupNetgameDialog::limitTypeHit ()
{
	switch(m_limitTypeWidget->get_value ())
	{
		case 0:
			setupForUntimedGame ();
			break;
			
		case 1:
			setupForTimedGame ();
			break;
			
		case 2:
			setupForScoreGame ();
			break;
	}
}

void SetupNetgameDialog::teamsHit ()
{
	if (m_allowTeamsWidget->get_value ())
		m_teamWidget->activate ();
	else
		m_teamWidget->deactivate ();
}

void SetupNetgameDialog::setupForGameType ()
{
	int raw_value = m_gameTypeWidget->get_value ();
	switch (raw_value < 5 ? raw_value : raw_value + 1)
	{
		case _game_of_cooperative_play:
			m_allowTeamsWidget->activate ();
			m_deadPlayersDropItemsWidget->deactivate ();
			m_aliensWidget->deactivate ();
			
			m_deadPlayersDropItemsWidget->set_value (true);
			m_aliensWidget->set_value (true);
			break;
			
		case _game_of_kill_monsters:
		case _game_of_king_of_the_hill:
		case _game_of_kill_man_with_ball:
		case _game_of_tag:
	case _game_of_custom:
			m_allowTeamsWidget->activate ();
			m_deadPlayersDropItemsWidget->activate ();
			m_aliensWidget->activate ();
			break;

		case _game_of_capture_the_flag:
			m_allowTeamsWidget->deactivate ();
			m_deadPlayersDropItemsWidget->activate ();
			m_aliensWidget->activate ();
			
			m_allowTeamsWidget->set_value (true);
			m_teamWidget->activate ();
			break;
			
		case _game_of_rugby:
			m_allowTeamsWidget->deactivate ();
			m_deadPlayersDropItemsWidget->activate ();
			m_aliensWidget->activate ();
			
			m_allowTeamsWidget->set_value (true);
			m_teamWidget->activate ();
			break;
			
		default:
			assert(false);
			break;
	}
}

void SetupNetgameDialog::gameTypeHit ()
{
	int new_game_type= m_gameTypeWidget->get_value ();
	if (new_game_type >= 5)
		++new_game_type;
	
	if (new_game_type != m_old_game_type) {
		int32 new_entry_flags, old_entry_flags;
		struct entry_point entry;
			
		if(m_allow_all_levels) {
			new_entry_flags= old_entry_flags= NONE;
		} else {
			new_entry_flags= get_entry_point_flags_for_game_type(new_game_type);
			old_entry_flags= get_entry_point_flags_for_game_type(m_old_game_type);
		}
		menu_index_to_level_entry (m_levelWidget->get_value (), old_entry_flags, &entry);
			
		/* Now reset entry points */
		m_levelWidget->set_labels (make_entry_vector (new_entry_flags));
		m_levelWidget->set_value (level_index_to_menu_index (entry.level_number, new_entry_flags));
		m_old_game_type= new_game_type;
				
		setupForGameType ();
	}
}

void SetupNetgameDialog::chooseMapHit ()
{
	FileSpecifier mapFile = m_mapWidget->get_file ();

	environment_preferences->map_checksum = read_wad_file_checksum (mapFile);
	strncpy(environment_preferences->map_file, mapFile.GetPath(), 256);
	load_environment_from_preferences();
		
	m_levelWidget->set_labels (make_entry_vector (get_entry_point_flags_for_game_type (m_old_game_type)));
	m_levelWidget->set_value (0);
}

bool SetupNetgameDialog::informationIsAcceptable ()
{
	bool information_is_acceptable = true;
	short game_limit_type = m_limitTypeWidget->get_value ();
	
	if (information_is_acceptable)
		if (game_limit_type == duration_time_limit)
		{
			information_is_acceptable = m_timeLimitWidget->get_value () >= 1;
		}
		
	if (information_is_acceptable)
		if (game_limit_type == duration_kill_limit)
		{
			information_is_acceptable = m_scoreLimitWidget->get_value () >= 1;
		}
	
	if (information_is_acceptable)
		information_is_acceptable = !(m_nameWidget->get_text ().empty ());
	
	if (information_is_acceptable)
		information_is_acceptable = m_mapWidget->get_file ().Exists ();

	if (information_is_acceptable)
	{
		entry_point ep;
		short index = 0;
		information_is_acceptable = get_indexed_entry_point(&ep, &index, get_entry_point_flags_for_game_type(m_old_game_type));
	}
		
	return (information_is_acceptable);
}

void SetupNetgameDialog::okHit ()
{
	if (informationIsAcceptable ())
		Stop (true);
	else
		unacceptableInfo ();
		
}

void menu_index_to_level_entry(
	short menu_index, 
	int32 entry_flags,
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

int menu_index_to_level_index (int menu_index, int32 entry_flags)
{	
	entry_point entry;
	entry.level_number = 0;

	menu_index_to_level_entry (menu_index, entry_flags, &entry);
	
	return entry.level_number;
}

int level_index_to_menu_index (int level_index, int32 entry_flags)
{	
	entry_point entry;
	short map_index = 0;

	int result = 0;
	while (get_indexed_entry_point(&entry, &map_index, entry_flags)) {
		if (map_index == level_index + 1)
			return result;
		++result;
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
	value = get_selection_control_value(outcome, iGRAPH_POPUP)-1;
	if(value<dynamic_world->player_count)
	{
		if(index) *index= value;
		graph_type= _player_graph;
	} 
	else 
	{
                int theValueAfterPlayers = value-dynamic_world->player_count;
                // ZZZ: Account for (lack of) separators
                if(theValueAfterPlayers >= 0)	theValueAfterPlayers++;
                if(theValueAfterPlayers >= 3)	theValueAfterPlayers++;
                
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
	short team_index, player_index, num_teams;
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
		int32 ranking = get_team_net_ranking(team_index, &kills, &deaths, true);

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
			ranks[team_count].deaths = deaths;
			ranks[team_count].player_index = NONE;
			ranks[team_count].color = team_index;
			ranks[team_count].game_ranking = ranking;
			ranks[team_count].friendly_fire_kills = team_friendly_fire[team_index].kills;
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
    int32    total_kills = 0;
    int32    total_deaths = 0;
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

	minutes = ((float)dynamic_world->tick_count / TICKS_PER_SECOND) / 60.0F;
	if (minutes > 0) kpm = total_kills / minutes;
	else kpm = 0;
	getcstr(kill_string_format, strNET_STATS_STRINGS, strTOTAL_KILLS_STRING);
	csprintf(temporary, kill_string_format, total_kills, kpm);
//	GetDialogItem(dialog, iTOTAL_KILLS, &item_type, &item_handle, &item_rect);
//	SetDialogItemText(item_handle, ptemporary);
    
    copy_cstring_to_static_text(outcome, iTOTAL_KILLS, temporary);

	if (minutes > 0) dpm = total_deaths / minutes;
	else dpm = 0;
	getcstr(death_string_format, strNET_STATS_STRINGS, strTOTAL_DEATHS_STRING);
	
	if (num_suicides)
	{
		if (friendly_fire)
			getcstr(suicide_string_format, strNET_STATS_STRINGS, strFRIENDLY_FIRE_STRING);
		else
			getcstr(suicide_string_format, strNET_STATS_STRINGS, strINCLUDING_SUICIDES_STRING);
		strncat(death_string_format, suicide_string_format, 65-strlen(death_string_format)-1);
		csprintf(temporary, death_string_format, total_deaths, dpm, num_suicides);
	}
	else
		csprintf(temporary, death_string_format, total_deaths, dpm);
//	GetDialogItem(dialog, iTOTAL_DEATHS, &item_type, &item_handle, &item_rect);
//	SetDialogItemText(item_handle, ptemporary);

    copy_cstring_to_static_text(outcome, iTOTAL_DEATHS, temporary);
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


// Get player name from outside
// ZZZ random note: I didn't do this part, and I'm not sure it's right.  At least, the
// documentation seems a bit inconsistent.  The MML docs say that it determines the
// default player name in multiplayer.  This is true, but more importantly, it determines
// the service type advertised/sought when trying to get together a game.  I guess this
// could be advantageous, in case MML is used to change the way a game works, in which case
// you wouldn't want folks with their MML set up in different ways trying to play together
// (instant sync problems - just add water.)
#define PLAYER_TYPE GetPlayerName()

// ZZZ: some features that may or may not be there - these are used to control what UI gets drawn.
// (as of my initial submission, only pregame gatherer-to-joiner messaging works.)
// Eventually, someone will make network microphone for SDL, at which point it should be extended
// to work in pre/postgame also, so the text chat becomes less important.
// NOTE - if you enable the POSTGAME_CHAT UI, you will need to edit kPostgameTopMargin in
// w_players_in_game2's implementation to reduce the dialog height it uses.  (network_dialog_widgets_sdl.cpp)
#define	NETWORK_PREGAME_CHAT	// Gatherer can message joiners at pregame
#undef	NETWORK_POSTGAME_CHAT	// Gatherer can message joiners at postgame
#undef	NETWORK_TWO_WAY_CHAT	// Joiners can message whenever the gatherer can

// Stuff that used to live in network_dialogs_sdl.cpp

#ifdef	DEBUG
//#define	NETWORK_TEST_POSTGAME_DIALOG	// For testing without having to play a netgame

// Note!  If you use NETWORK_TEST_MICROPHONE_LOCALLY you'll also want to set the microphone
// implementation to local loopback, else who knows what will happen...
//#define NETWORK_TEST_MICROPHONE_LOCALLY // can use this OR test postgame dialog, not both at once.
#endif

#ifdef NETWORK_TEST_MICROPHONE_LOCALLY
#include    "network_speaker_sdl.h"
#include	"network_sound.h"
#endif

// ZZZ: graph types are a dynamically-generated StringSet (not loaded from MML)
enum {
    kGraphTypesStringSetID	= 3180
};

// limit types, 0-based, for w_select-compatible use.  (see also set_limit_type())
enum {    
    kNoLimit			= 0,
    kTimeLimit			= 0x01,
    kScoreLimit			= 0x02,
    kScoreAndTimeLimits		= kScoreLimit | kTimeLimit // currently cannot be selected directly
};

// Some identifiers used only locally.  Hope the numeric equivalents don't conflict!
// (they shouldn't.)
enum {
        iDONT_DO_THIS_USE_SHARED_SYMBOLS= 4242,	// Score limit?  Time limit?  No limit?
        iCHAT_HISTORY,				// Where chat text appears
        iCHAT_ENTRY,				// Where chat text is entered
                
};


static bool sAdvertiseGameOnMetaserver = false;

// ZZZ: this is based on the eponymous function on the Mac side
static short create_graph_popup_menu(w_select* theMenu)
{
	short index;
	bool has_scores;

        // Clear the graph types stringset
        TS_DeleteStringSet(kGraphTypesStringSetID);

	/* Setup the player names */
	for (index= 0; index<dynamic_world->player_count; index++)
	{
		struct player_data *player= get_player_data(rankings[index].player_index);

                TS_PutCString(kGraphTypesStringSetID, index, player->name);
	}
	
	/* Add in the total carnage.. */
        getcstr(temporary, strNET_STATS_STRINGS, strTOTALS_STRING);
        TS_PutCString(kGraphTypesStringSetID, index, temporary);
        index++;
	
	/* Add in the scores */
	has_scores= get_network_score_text_for_postgame(temporary, false);
	if(has_scores)
	{
                TS_PutCString(kGraphTypesStringSetID, index, temporary);
                index++;
	}
	
	/* If the game has teams, show the team stats. */
	if (!(dynamic_world->game_information.game_options & _force_unique_teams)) 
	{
                getcstr(temporary, strNET_STATS_STRINGS, strTEAM_TOTALS_STRING);
                TS_PutCString(kGraphTypesStringSetID, index, temporary);
                index++;

		if(has_scores)
		{
			get_network_score_text_for_postgame(temporary, true);
                        TS_PutCString(kGraphTypesStringSetID, index, temporary);
                        index++;
		}
	} 

        // Place the newly-constructed StringSet into the graph selection widget.
        theMenu->set_labels_stringset(kGraphTypesStringSetID);

        // Change of behavior here: instead of choosing individual scores, or failing that, Total Carnage,
        // I select team scores, then failing that either team carnage or individual scores, then failing that,
        // Total Carnage.  I think this better reflects what really happens - in a team game, it's *teams*
        // that win things.  You can view the individual results, sure, but the first thing that pops up
        // (who won??) is a team stat.
        theMenu->set_selection(index - 1);        
        
	return index;
}


void
draw_names(DialogPtr &dialog, struct net_rank *ranks, short number_of_bars, short which_player) {
    // This does nothing here - draw_kill_bars or draw_score_bars is assumed to have enough data to work with,
    // and one of those is always called adjacent to a call to draw_names in practice.
}

void
draw_kill_bars(DialogPtr &dialog, struct net_rank *ranks, short num_players, 
               short suicide_index, bool do_totals, bool friendly_fire)
{
    // We don't actually draw here - we just pass the data along to the widget, and it will take care of the rest. 
    w_players_in_game2* wpig2 = dynamic_cast<w_players_in_game2*>(dialog->get_widget_by_id(iDAMAGE_STATS));
    wpig2->set_graph_data(ranks, num_players, suicide_index, (ranks[0].player_index == NONE) ? true : false, false);

    update_carnage_summary(dialog, ranks, num_players, suicide_index, do_totals, friendly_fire);
}

void
draw_score_bars(DialogPtr &dialog, struct net_rank *ranks, short bar_count) {
    // We don't actually draw here - we just pass the data along to the widget, and it will take care of the rest. 
    w_players_in_game2* wpig2 = dynamic_cast<w_players_in_game2*>(dialog->get_widget_by_id(iDAMAGE_STATS));
    wpig2->set_graph_data(ranks, bar_count, NONE, (ranks[0].player_index == NONE) ? true : false, true);

    // clear the summary text
    copy_cstring_to_static_text(dialog, iTOTAL_KILLS, "");
    copy_cstring_to_static_text(dialog, iTOTAL_DEATHS, "");
}

// User clicked on a postgame carnage report element.  If it was a player and we're showing Total Carnage
// or a player vs player graph, switch to showing a player vs player graph according to the player clicked.
static void
respond_to_element_clicked(w_players_in_game2* inWPIG2, bool inTeam, bool inGraph, bool inScore, size_t inDrawIndex,
                           int inPlayerIndexOrTeamColor) {
    if(inGraph && !inTeam && !inScore) {
        w_select*   theGraphMenu = dynamic_cast<w_select*>(inWPIG2->get_owning_dialog()->get_widget_by_id(iGRAPH_POPUP));

        if(theGraphMenu->get_selection() != inDrawIndex)
            theGraphMenu->set_selection(inDrawIndex, true);
    }
}

// User twiddled the iGRAPH_POPUP; draw a new kind of graph in response.
static void
respond_to_graph_type_change(w_select* inGraphMenu) {
    DialogPtr p = inGraphMenu->get_owning_dialog();
    draw_new_graph(p);
}

#ifdef NETWORK_TWO_WAY_CHAT
// There's currently no underlying support for this, so we just do some fakery.
static void
send_text_fake(w_text_entry* te) {
    assert(te != NULL);
    
    dialog* d = te->get_owning_dialog();
    
    w_chat_history* ch = dynamic_cast<w_chat_history*>(d->get_widget_by_id(iCHAT_HISTORY));
    assert(ch != NULL);
    
    int netState = NetState();
    
    if(netState != netUninitialized && netState != netJoining && netState != netDown
        && !(netState == netGathering && NetGetNumberOfPlayers() <= 1))
    {
        ch->append_chat_entry(NULL, "This is not finished yet.  Your text will not be seen by others.");
        player_info* info = (player_info*)NetGetPlayerData(NetGetLocalPlayerIndex());
        ch->append_chat_entry(info, te->get_text());
    
        te->set_text("");
    }
    else {
        ch->append_chat_entry(NULL, "There is nobody in the game to hear you yet.");
    }
}
#endif // NETWORK_TWO_WAY_CHAT

// Here's the main entry point for thgametypee postgame carnage report.
void display_net_game_stats(void)
{
//printf("display_net_game_stats\n");

	if (gMetaserverClient) 
	{
		gMetaserverClient->announceGameDeleted();
		gMetaserverClient->pump();
	}

    dialog d;
    
    vertical_placer *placer = new vertical_placer;
    placer->dual_add(new w_title("POSTGAME CARNAGE REPORT"), d);
    
    horizontal_placer *graph_type_placer = new horizontal_placer;
    w_select* graph_type_w = new w_select(0, NULL);
    graph_type_w->set_identifier(iGRAPH_POPUP);
    graph_type_w->set_selection_changed_callback(respond_to_graph_type_change);
    graph_type_placer->dual_add(graph_type_w->label("Report on"), d);
    graph_type_placer->dual_add(graph_type_w, d);

    placer->add(graph_type_placer, true);
    
    w_players_in_game2* wpig2 = new w_players_in_game2(true);	// "true": extra space for postgame labels etc.
    wpig2->set_identifier(iDAMAGE_STATS);
    wpig2->set_element_clicked_callback(respond_to_element_clicked);
    wpig2->update_display(true);	// "true": widget gets data from dynamic_world, not topology
    placer->dual_add(wpig2, d);
    
    placer->add(new w_spacer(), true);

    horizontal_placer *carnage_and_ok_placer = new horizontal_placer;
    vertical_placer *carnage_placer = new vertical_placer;

    carnage_placer->add_flags((placeable::placement_flags) ((int) placeable::kAlignLeft | (int) placeable::kFill));
    // (total kills) and (total deaths) will be replaced by update_carnage_summary() or set to "".
    w_static_text*  total_kills_w = new w_static_text("(total kills)");
    total_kills_w->set_identifier(iTOTAL_KILLS);
    carnage_placer->dual_add(total_kills_w, d);

    w_static_text*  total_deaths_w = new w_static_text("(total deaths)");
    total_deaths_w->set_identifier(iTOTAL_DEATHS);
    carnage_placer->dual_add(total_deaths_w, d);
    
    carnage_and_ok_placer->add_flags(placeable::kFill);
    carnage_and_ok_placer->add(carnage_placer, true);
    carnage_and_ok_placer->add_flags();
    carnage_and_ok_placer->dual_add(new w_button("OK", dialog_ok, &d), d);

    placer->add_flags(placeable::kFill);
    placer->add(carnage_and_ok_placer, true);

    /* Calculate the rankings (once) for the entire graph */
    calculate_rankings(rankings, dynamic_world->player_count);
    qsort(rankings, dynamic_world->player_count, sizeof(struct net_rank), rank_compare);
    
    /* Create the graph popup menu */
    create_graph_popup_menu(graph_type_w);
    
    {
	    DialogPtr p = &d;
	    draw_new_graph(p);
    }
    
    d.set_widget_placer(placer);
    d.run();
}

class SdlGatherDialog : public GatherDialog
{
public:
	SdlGatherDialog()
	{
		vertical_placer *placer = new vertical_placer;
		placer->dual_add(new w_title("GATHER NETWORK GAME"), m_dialog);
		placer->add(new w_spacer());
	
		// m_dialog.add(new w_static_text("Players on Network"));

		w_joining_players_in_room* foundplayers_w = new w_joining_players_in_room(NULL, 320, 3);
		placer->dual_add(foundplayers_w, m_dialog);
	
		horizontal_placer *autogather_placer = new horizontal_placer(get_theme_space(ITEM_WIDGET), true);
		w_toggle* autogather_w = new w_toggle(false);
		autogather_placer->dual_add(autogather_w->label("Auto-Gather"), m_dialog);
		autogather_placer->dual_add(autogather_w, m_dialog);

		placer->add(autogather_placer, true);
		placer->add(new w_spacer(), true);
	
		w_players_in_game2* players_w = new w_players_in_game2(false);
		placer->dual_add(players_w, m_dialog);

		horizontal_placer *button_placer = new horizontal_placer;
		w_button* play_button_w = new w_button("PLAY");
		button_placer->dual_add(play_button_w, m_dialog);

		w_button* cancel_w = new w_button("CANCEL");
		button_placer->dual_add(cancel_w, m_dialog);

		placer->add(button_placer, true);

		horizontal_placer *chat_choice_placer = new horizontal_placer;
		w_select_popup* chat_choice_w = new w_select_popup();
		chat_choice_placer->dual_add(chat_choice_w->label("chat:"), m_dialog);
		chat_choice_placer->dual_add(chat_choice_w, m_dialog);
		placer->add(chat_choice_placer, true);

		w_colorful_chat* chat_history_w = new w_colorful_chat(600, 6);
		placer->dual_add(chat_history_w, m_dialog);

		w_chat_entry* chatentry_w = new w_chat_entry(240);
		chatentry_w->enable_mac_roman_input();

		horizontal_placer *say_placer = new horizontal_placer;
		say_placer->dual_add(chatentry_w->label("Say:"), m_dialog);
		say_placer->add_flags(placeable::kFill);
		say_placer->dual_add(chatentry_w, m_dialog);

		placer->add_flags(placeable::kFill);
		placer->add(say_placer, true);

		m_dialog.activate_widget(chatentry_w);

		m_dialog.set_widget_placer(placer);
		
		m_cancelWidget = new ButtonWidget (cancel_w);
		m_startWidget = new ButtonWidget (play_button_w);
		
		m_autogatherWidget = new ToggleWidget (autogather_w);
	
		m_ungatheredWidget = new JoiningPlayerListWidget (foundplayers_w);
		m_pigWidget = new PlayersInGameWidget (players_w);
		
		m_chatEntryWidget = new EditTextWidget (chatentry_w);
		m_chatWidget = new ColorfulChatWidget(new ColorfulChatWidgetImpl(chat_history_w));
		m_chatChoiceWidget = new PopupSelectorWidget (chat_choice_w);
	}
	
	virtual bool Run ()
	{
		m_dialog.set_processing_function (boost::bind(&SdlGatherDialog::idle, this));
		return (m_dialog.run() == 0);
	}
	
	virtual void Stop (bool result)
	{
		if (result)
			m_dialog.quit (0);
		else
			m_dialog.quit (-1);
	}
	
private:
	dialog m_dialog;
};

std::unique_ptr<GatherDialog>
GatherDialog::Create()
{
	return std::unique_ptr<GatherDialog>(new SdlGatherDialog);
}

extern struct color_table *build_8bit_system_color_table(void);

class SdlJoinDialog : public JoinDialog
{
public:
	SdlJoinDialog() : m_tabs(0)
	{
		SDL_Keymod m = SDL_GetModState();
		if ((m & KMOD_ALT) || (m & KMOD_GUI)) skipToMetaserver = !skipToMetaserver;

		vertical_placer *placer = new vertical_placer;
		placer->dual_add(new w_title("JOIN NETWORK GAME"), m_dialog);
		placer->add(new w_spacer(), true);

		table_placer *table = new table_placer(2, get_theme_space(ITEM_WIDGET), true);
		table->col_flags(0, placeable::kAlignRight);

		w_text_entry *name_w = new w_text_entry(PREFERENCES_NAME_LENGTH, "");
		name_w->enable_mac_roman_input();
		table->dual_add(name_w->label("Name"), m_dialog);
		table->dual_add(name_w, m_dialog);
	
		w_player_color *pcolor_w = new w_player_color(0);
		table->dual_add(pcolor_w->label("Color"), m_dialog);
		table->dual_add(pcolor_w, m_dialog);

		w_player_color *tcolor_w = new w_player_color(0);
		table->dual_add(tcolor_w->label("Team Color"), m_dialog);
		table->dual_add(tcolor_w, m_dialog);

		placer->add(table, true);
		placer->add(new w_spacer(), true);

		m_tabs = new tab_placer;

		vertical_placer *prejoin_placer = new vertical_placer;
		table_placer *prejoin_table = new table_placer(2, get_theme_space(ITEM_WIDGET), true);
		prejoin_table->col_flags(0, placeable::kAlignRight);
		
		prejoin_table->add_row(new w_spacer(), true);
		
		w_button* join_by_metaserver_w = new w_button("FIND INTERNET GAME");
		prejoin_table->dual_add_row(join_by_metaserver_w, m_dialog);
		prejoin_table->add_row(new w_spacer(), true);
		
		w_toggle* hint_w = new w_toggle(false);
		prejoin_table->dual_add(hint_w->label("Join by address"), m_dialog);
		prejoin_table->dual_add(hint_w, m_dialog);

		w_text_entry* hint_address_w = new w_text_entry(kJoinHintingAddressLength, "");
		prejoin_table->dual_add(hint_address_w->label("Join address"), m_dialog);
		prejoin_table->dual_add(hint_address_w, m_dialog);

		prejoin_table->add_row(new w_spacer(), true);

		w_button* join_w = new w_button("JOIN LOCAL GAME");
		prejoin_table->dual_add_row(join_w, m_dialog);
		
		prejoin_placer->add(prejoin_table, true);
		prejoin_placer->add(new w_spacer(), true);

		vertical_placer *postjoin_placer = new vertical_placer;

		w_static_text* join_messages_w = new w_static_text("");

		// jkvw: add it to dialog, but never show it.
		//       Two things which we need don't work:
		//       1) w_static_text can't handle text longer than the dialog width
		//       2) widgets in dialog don't update layout position once dialog starts to run
		//       If we get solutions to these issues, then we can show the join messages.

		w_players_in_game2* players_w = new w_players_in_game2(false);
		postjoin_placer->dual_add(players_w, m_dialog);

		horizontal_placer *chat_choice_placer = new horizontal_placer;

		w_select_popup* chat_choice_w = new w_select_popup();
		chat_choice_placer->dual_add(chat_choice_w->label("chat:"), m_dialog);
		chat_choice_placer->dual_add(chat_choice_w, m_dialog);

		postjoin_placer->add(chat_choice_placer, true);

		w_colorful_chat* chat_history_w = new w_colorful_chat(600, 7);
		postjoin_placer->dual_add(chat_history_w, m_dialog);

		chatentry_w = new w_chat_entry(240);
		chatentry_w->enable_mac_roman_input();

		horizontal_placer *say_placer = new horizontal_placer;
		say_placer->dual_add(chatentry_w->label("Say:"), m_dialog);
		say_placer->add_flags(placeable::kFill);
		say_placer->dual_add(chatentry_w, m_dialog);

		postjoin_placer->add_flags(placeable::kFill);
		postjoin_placer->add(say_placer, true);
		postjoin_placer->add_flags();

		m_tabs->add(prejoin_placer, true);
		m_tabs->add(postjoin_placer, true);
		m_tabs->dual_add(join_messages_w, m_dialog);
		placer->add(m_tabs, true);

		horizontal_placer *button_placer = new horizontal_placer;
		
		w_button* cancel_w = new w_button("CANCEL");
		button_placer->dual_add(cancel_w, m_dialog);

		placer->add(button_placer, true);

		m_dialog.set_widget_placer(placer);

		m_cancelWidget = new ButtonWidget (cancel_w);
		m_joinWidget = new ButtonWidget (join_w);
	
		m_joinMetaserverWidget = new ButtonWidget (join_by_metaserver_w);
		m_joinAddressWidget = new EditTextWidget (hint_address_w);
		m_joinByAddressWidget = new ToggleWidget (hint_w);
	
		m_nameWidget = new EditTextWidget (name_w);
		m_colourWidget = new ColourSelectorWidget (pcolor_w);
		m_teamWidget = new ColourSelectorWidget (tcolor_w);
	
		m_messagesWidget = new StaticTextWidget (join_messages_w);
	
		m_pigWidget = new PlayersInGameWidget (players_w);
		
		m_chatEntryWidget = new EditTextWidget (chatentry_w);
		m_chatWidget = new ColorfulChatWidget(new ColorfulChatWidgetImpl(chat_history_w));
		m_chatChoiceWidget = new PopupSelectorWidget (chat_choice_w);
	}

	virtual void Run ()
	{
		m_dialog.set_processing_function (boost::bind(&SdlJoinDialog::gathererSearch, this));
		m_dialog.run();
	}
	
	virtual void Stop()
	{
		if (join_result == kNetworkJoinFailedUnjoined || join_result == kNetworkJoinFailedJoined)
			m_dialog.quit(-1);
		else
			m_dialog.quit(0);
	}
	
	virtual void respondToJoinHit()
	{
		play_dialog_sound(DIALOG_OK_SOUND);
		m_tabs->choose_tab(1);
		m_dialog.draw();
		JoinDialog::respondToJoinHit();
		m_dialog.activate_widget(chatentry_w);		
	}

private:
	enum {
		iJOIN_PREJOIN_TAB,
		iJOIN_POSTJOIN_TAB,
		iJOIN_NEVERSHOW_TAB
	};

	tab_placer *m_tabs;
	w_chat_entry *chatentry_w;
	dialog m_dialog;
};

std::unique_ptr<JoinDialog>
JoinDialog::Create()
{
	return std::unique_ptr<JoinDialog>(new SdlJoinDialog);
}

class SdlSetupNetgameDialog : public SetupNetgameDialog
{
public:
	SdlSetupNetgameDialog ()
	{
		vertical_placer *placer = new vertical_placer;

		placer->dual_add(new w_title("SETUP NETWORK GAME"), m_dialog);
		placer->add(new w_spacer(), true);

		table_placer *table = new table_placer(2, 10);
		vertical_placer *left_placer = new vertical_placer;
		vertical_placer *right_placer = new vertical_placer;
		
		table_placer *player_table = new table_placer(2, get_theme_space(ITEM_WIDGET));
		player_table->col_flags(0, placeable::kAlignRight);
		player_table->dual_add_row(new w_static_text("Appearance"), m_dialog);
		w_text_entry *name_w = new w_text_entry (PREFERENCES_NAME_LENGTH, "");
		name_w->enable_mac_roman_input();

		player_table->dual_add(name_w->label("Name"), m_dialog);
		player_table->dual_add(name_w, m_dialog);

		w_player_color *pcolor_w = new w_player_color (player_preferences->color);
		player_table->dual_add(pcolor_w->label("Color"), m_dialog);
		player_table->dual_add(pcolor_w, m_dialog);

		w_player_color *tcolor_w = new w_player_color (player_preferences->team);
		player_table->dual_add(tcolor_w->label("Team"), m_dialog);
		player_table->dual_add(tcolor_w, m_dialog);

		right_placer->dual_add(new w_static_text("Network"), m_dialog);
		table_placer *network_table = new table_placer(2, get_theme_space(ITEM_WIDGET));
		network_table->col_flags(1, placeable::kAlignLeft);

		w_toggle *advertise_on_metaserver_w = new w_toggle (sAdvertiseGameOnMetaserver);
		network_table->dual_add(advertise_on_metaserver_w, m_dialog);
		network_table->dual_add(advertise_on_metaserver_w->label("Advertise Game on Internet"), m_dialog);

		w_toggle *use_upnp_w = new w_toggle (true);
		network_table->dual_add(use_upnp_w, m_dialog);
		network_table->dual_add(use_upnp_w->label("Configure UPnP Router"), m_dialog);

		w_toggle* realtime_audio_w = new w_toggle(network_preferences->allow_microphone);
		network_table->dual_add(realtime_audio_w, m_dialog);
		network_table->dual_add(realtime_audio_w->label("Allow Microphone"), m_dialog);

		w_select_popup *latency_tolerance_w = new w_select_popup();
		horizontal_placer *latency_placer = new horizontal_placer(get_theme_space(ITEM_WIDGET));
		latency_placer->dual_add(latency_tolerance_w->label("Latency Tolerance"), m_dialog);
		latency_placer->dual_add(latency_tolerance_w, m_dialog);
		
		network_table->add(new w_spacer(), true);
		network_table->add(latency_placer, true);

		right_placer->add(network_table, true);

		player_table->add_row(new w_spacer(), true);
		player_table->dual_add_row(new w_static_text("Game"), m_dialog);

		// Could eventually store this path in network_preferences somewhere, so to have separate map file
		// prefs for single- and multi-player.
		w_file_chooser* map_w = new w_file_chooser ("Choose Map", _typecode_scenario);
#ifndef MAC_APP_STORE
		player_table->dual_add(map_w->label("Map"), m_dialog);
		player_table->dual_add(map_w, m_dialog);
#endif

		w_select_popup* entry_point_w = new w_select_popup ();
		player_table->dual_add(entry_point_w->label("Level"), m_dialog);
		player_table->dual_add(entry_point_w, m_dialog);

		w_select_popup* game_type_w = new w_select_popup ();
		player_table->dual_add(game_type_w->label("Game Type"), m_dialog);
		player_table->dual_add(game_type_w, m_dialog);

		w_select *diff_w = new w_select(network_preferences->difficulty_level, NULL);
		player_table->dual_add(diff_w->label("Difficulty"), m_dialog);
		player_table->dual_add(diff_w, m_dialog);

		left_placer->add(player_table, true);

#ifndef MAC_APP_STORE
		network_table->add_row(new w_spacer(), true);
		network_table->dual_add_row(new w_static_text("Net Script"), m_dialog);
#endif
		w_enabling_toggle* use_netscript_w = new w_enabling_toggle (network_preferences->use_netscript);
#ifndef MAC_APP_STORE
		network_table->dual_add(use_netscript_w, m_dialog);
		network_table->dual_add(use_netscript_w->label("Use Netscript"), m_dialog);
#endif

		w_file_chooser* choose_script_w = new w_file_chooser ("Choose Script", _typecode_netscript);
#ifndef MAC_APP_STORE
		network_table->add(new w_spacer(), true);
		network_table->dual_add(choose_script_w, m_dialog);
#endif
		use_netscript_w->add_dependent_widget(choose_script_w);

		left_placer->add(new w_spacer(), true);
		table_placer *options_table = new table_placer(2, get_theme_space(ITEM_WIDGET));
		options_table->col_flags(1, placeable::kAlignLeft);
		options_table->dual_add_row(new w_static_text("Options"), m_dialog);

		w_toggle *aliens_w = new w_toggle((network_preferences->game_options & _monsters_replenish) != 0);
		options_table->dual_add(aliens_w, m_dialog);
		options_table->dual_add(aliens_w->label("Aliens"), m_dialog);

		w_toggle *live_w = new w_toggle((network_preferences->game_options & _live_network_stats) != 0);
		options_table->dual_add(live_w, m_dialog);
		options_table->dual_add(live_w->label("Live Carnage Reporting"), m_dialog);

		w_toggle *teams_w = new w_toggle(!(network_preferences->game_options & _force_unique_teams));
		options_table->dual_add(teams_w, m_dialog);
		options_table->dual_add(teams_w->label("Teams"), m_dialog);

		w_toggle *drop_w = new w_toggle(!(network_preferences->game_options & _burn_items_on_death));
		options_table->dual_add(drop_w, m_dialog);
		options_table->dual_add(drop_w->label("Dead Players Drop Items"), m_dialog);

		w_toggle *sensor_w = new w_toggle((network_preferences->game_options & _motion_sensor_does_not_work) != 0);
		options_table->dual_add(sensor_w, m_dialog);
		options_table->dual_add(sensor_w->label("Disable Motion Sensor"), m_dialog);

		w_toggle *pen_die_w = new w_toggle((network_preferences->game_options & _dying_is_penalized) != 0);
		options_table->dual_add(pen_die_w, m_dialog);
		options_table->dual_add(pen_die_w->label("Penalize Dying (10 seconds)"), m_dialog);

		w_toggle *pen_sui_w = new w_toggle((network_preferences->game_options & _suicide_is_penalized) != 0);
		options_table->dual_add(pen_sui_w, m_dialog);
		options_table->dual_add(pen_sui_w->label("Penalize Suicide (15 seconds)"), m_dialog);

		left_placer->add(options_table, true);

		network_table->add_row(new w_spacer(), true);
		network_table->dual_add_row(new w_static_text("Cheats / Extras"), m_dialog);
		w_toggle *zoom_w = new w_toggle(true);
		network_table->dual_add(zoom_w, m_dialog);
		network_table->dual_add(zoom_w->label("Allow Zoom"), m_dialog);
	
		w_toggle *crosshairs_w = new w_toggle(true);
		network_table->dual_add(crosshairs_w, m_dialog);
		network_table->dual_add(crosshairs_w->label("Allow Crosshairs"), m_dialog);
	
		w_toggle *overlay_w = new w_toggle(true);
		network_table->dual_add(overlay_w, m_dialog);
		network_table->dual_add(overlay_w->label("Allow Overlay Map"), m_dialog);
		
		w_toggle *lara_croft_w = new w_toggle(true);
		network_table->dual_add(lara_croft_w, m_dialog);
		network_table->dual_add(lara_croft_w->label("Allow Chase Cam"), m_dialog);

		w_toggle *carnage_messages_w = new w_toggle(true);
		network_table->dual_add(carnage_messages_w, m_dialog);
		network_table->dual_add(carnage_messages_w->label("Allow Carnage Messages"), m_dialog);

		w_toggle *saving_level_w = new w_toggle(true);
#ifndef MAC_APP_STORE
		network_table->dual_add(saving_level_w, m_dialog);
		network_table->dual_add(saving_level_w->label("Allow .save level"), m_dialog);
#endif

		right_placer->add(new w_spacer(), true);
		right_placer->dual_add(new w_static_text("Duration"), m_dialog);
		table_placer *limits_table = new table_placer(2, get_theme_space(ITEM_WIDGET));
		limits_table->col_flags(0, placeable::kAlignRight);
		
		w_select* endcondition_w = new w_select(kTimeLimit, NULL);
		limits_table->dual_add(endcondition_w->label("Game Ends At"), m_dialog);
		limits_table->dual_add(endcondition_w, m_dialog);

		w_number_entry*	timelimit_w = new w_number_entry (network_preferences->time_limit);
		limits_table->dual_add(timelimit_w->label("Time Limit (minutes)"), m_dialog);
		limits_table->dual_add(timelimit_w, m_dialog);

		// The name of this widget (score limit) will be replaced by Kill Limit, Flag Capture Limit, etc.
		w_number_entry*	scorelimit_w = new w_number_entry (network_preferences->kill_limit);
		limits_table->dual_add(scorelimit_w->label("Kill / Score Limit"), m_dialog);
		limits_table->dual_add(scorelimit_w, m_dialog);
		right_placer->add(limits_table, true);
		
		table->add(left_placer, true);
		table->add(right_placer, true);

		placer->add(table, true);

		placer->add(new w_spacer(), true);

		horizontal_placer *button_placer = new horizontal_placer;
		w_button* ok_w = new w_button ("OK");
		button_placer->dual_add(ok_w, m_dialog);
		w_button* cancel_w = new w_button ("CANCEL");
		button_placer->dual_add(cancel_w, m_dialog);
		placer->add(button_placer, true);

		m_dialog.set_widget_placer(placer);

		m_cancelWidget = new ButtonWidget (cancel_w);
		m_okWidget = new ButtonWidget (ok_w);
	
		m_nameWidget = new EditTextWidget (name_w);
		m_colourWidget = new ColourSelectorWidget (pcolor_w);
		m_teamWidget = new ColourSelectorWidget (tcolor_w);
	
		m_mapWidget = new FileChooserWidget (map_w);
		
		m_levelWidget = new PopupSelectorWidget (entry_point_w);
		m_gameTypeWidget = new PopupSelectorWidget (game_type_w);
		m_difficultyWidget = new SelectSelectorWidget (diff_w);
	
		m_limitTypeWidget = new SelectSelectorWidget (endcondition_w);
		m_timeLimitWidget = new EditNumberWidget (timelimit_w);
		m_scoreLimitWidget = new EditNumberWidget (scorelimit_w);
	
		m_aliensWidget = new ToggleWidget (aliens_w);
		m_allowTeamsWidget = new ToggleWidget (teams_w);
		m_deadPlayersDropItemsWidget = new ToggleWidget (drop_w);
		m_penalizeDeathWidget = new ToggleWidget (pen_die_w);
		m_penalizeSuicideWidget = new ToggleWidget (pen_sui_w);
	
		m_useMetaserverWidget = new ToggleWidget (advertise_on_metaserver_w);
	
		m_useScriptWidget = new ToggleWidget (use_netscript_w);
		m_scriptWidget = new FileChooserWidget (choose_script_w);
	
		m_allowMicWidget = new ToggleWidget (realtime_audio_w);

		m_liveCarnageWidget = new ToggleWidget (live_w);
		m_motionSensorWidget = new ToggleWidget (sensor_w);
	
		m_zoomWidget = new ToggleWidget (zoom_w);
		m_crosshairWidget = new ToggleWidget (crosshairs_w);
		m_overlayWidget = new ToggleWidget (overlay_w);
		m_laraCroftWidget = new ToggleWidget (lara_croft_w);
		m_carnageMessagesWidget = new ToggleWidget (carnage_messages_w);
		m_savingLevelWidget = new ToggleWidget (saving_level_w);
		
		m_useUpnpWidget = new ToggleWidget (use_upnp_w);
		m_latencyToleranceWidget = new PopupSelectorWidget(latency_tolerance_w);
	}
	
	virtual bool Run ()
	{		
		return (m_dialog.run () == 0);
	}

	virtual void Stop (bool result)
	{
		if (result)
			m_dialog.quit (0);
		else
			m_dialog.quit (-1);
	}

	virtual bool allLevelsAllowed ()
	{
		return false;
	}

	virtual void unacceptableInfo ()
	{
		play_dialog_sound (DIALOG_ERROR_SOUND);
	}

private:
	dialog m_dialog;
};

std::unique_ptr<SetupNetgameDialog>
SetupNetgameDialog::Create ()
{
	return std::unique_ptr<SetupNetgameDialog>(new SdlSetupNetgameDialog);
}

// This should really be done better, I guess, but most people will never see it long enough to read it.
// Currently no actual bar is drawn (just a box with message), and no effort is made to make sure all messages
// will physically fit into the box.  (should probably somehow force the dialog to a width and set_full_width on
// the text widget.  Maybe alter its justification also.)
dialog*		sProgressDialog 	= NULL;
w_static_text*	sProgressMessage	= NULL;
w_progress_bar* sProgressBar		= NULL;

void open_progress_dialog(size_t message_id, bool show_progress_bar)
{
//printf("open_progress_dialog %d\n", message_id);

    assert(sProgressDialog == NULL);
    
    sProgressDialog 	= new dialog;
    sProgressMessage	= new w_static_text(TS_GetCString(strPROGRESS_MESSAGES, message_id));
    if (show_progress_bar) 
	    sProgressBar	= new w_progress_bar(200);
    
    vertical_placer *placer = new vertical_placer;
    placer->dual_add(sProgressMessage, *sProgressDialog);
    if (show_progress_bar) 
	    placer->dual_add(sProgressBar, *sProgressDialog);
    
    sProgressDialog->set_widget_placer(placer);
    
    sProgressDialog->start(false);

//    bool done = sProgressDialog->process_events();
//    assert(!done);
}


void set_progress_dialog_message(size_t message_id)
{
//printf("set_progress_dialog_message %d\n", message_id);
    assert(sProgressMessage != NULL);

    sProgressMessage->set_text(TS_GetCString(strPROGRESS_MESSAGES, message_id));
    
//    bool done = sProgressDialog->process_events();

//    assert(!done);
}

void close_progress_dialog(void)
{
//printf("close_progress_dialog\n");

    assert(sProgressDialog != NULL);
    
    sProgressDialog->quit(0);
    
//    bool done = sProgressDialog->process_events();
    
//    assert(done);
    
    int result = sProgressDialog->finish(false);
    
    assert(result == 0);
    
    delete sProgressDialog;
    
    sProgressDialog	= NULL;
    sProgressMessage	= NULL;
    sProgressBar	= NULL;
}

void draw_progress_bar(size_t sent, size_t total)
{
	if (!sProgressBar) return;
	sProgressBar->set_progress(sent, total);
	sProgressDialog->draw();
}

void reset_progress_bar(void)
{
	if (!sProgressBar) return;
	sProgressBar->set_progress(0, 1);
	sProgressDialog->draw();
}


#ifdef NETWORK_TEST_POSTGAME_DIALOG
static const char*    sTestingNames[] = {
        "Doctor Burrito",
        "Carnage Asada",
        "Bongo Bob",
        "The Napalm Man",
        "Kissy Monster",
        "lala",
        "Prof. Windsurf",
        "-ZED-"
};

// THIS ONE IS FAKE - used to test postgame report dialog without going through a game.
bool network_gather(void) {
    short i, j;
    player_info thePlayerInfo;
    game_info   theGameInfo;
	
    if(network_game_setup(&thePlayerInfo, &theGameInfo)) {

	for (i = 0; i < MAXIMUM_NUMBER_OF_PLAYERS; i++)
	{
        // make up a name
        /*int theNameLength = (local_random() % MAXIMUM_PLAYER_NAME_LENGTH) + 1;
        for(int n = 0; n < theNameLength; n++)
            players[i].name[n] = 'a' + (local_random() % ('z' - 'a'));

        players[i].name[theNameLength] = '\0';
*/
        strcpy(players[i].name, sTestingNames[i]);

        // make up a team and color
        players[i].color = local_random() % 8;
        int theNumberOfTeams = 2 + (local_random() % 3);
        players[i].team  = local_random() % theNumberOfTeams;

		(players+i)->monster_damage_taken.damage = abs(local_random()%200);
		(players+i)->monster_damage_taken.kills = abs(local_random()%30);
		(players+i)->monster_damage_given.damage = abs(local_random()%200);
		(players+i)->monster_damage_given.kills = abs(local_random()%30);
                
                players[i].netgame_parameters[0] = local_random() % 200;
                players[i].netgame_parameters[1] = local_random() % 200;
		
		for (j = 0; j < MAXIMUM_NUMBER_OF_PLAYERS; j++)
		{
			(players+i)->damage_taken[j].damage = abs(local_random()%200);
			(players+i)->damage_taken[j].kills = abs(local_random()%6);
		}
	}

    dynamic_world->player_count = MAXIMUM_NUMBER_OF_PLAYERS;

    game_data& game_information = dynamic_world->game_information;
    game_info* network_game_info = &theGameInfo;

	game_information.game_time_remaining= network_game_info->time_limit;
	game_information.kill_limit= network_game_info->kill_limit;
	game_information.game_type= network_game_info->net_game_type;
	game_information.game_options= network_game_info->game_options;
	game_information.initial_random_seed= network_game_info->initial_random_seed;
	game_information.difficulty_level= network_game_info->difficulty_level;

    display_net_game_stats();
    } // if setup box was OK'd
    return false;
}
#endif // NETWORK_TEST_POSTGAME_DIALOG





#ifdef NETWORK_TEST_MICROPHONE_LOCALLY
static void
respond_to_microphone_toggle(w_select* inWidget) {
    set_network_microphone_state(inWidget->get_selection() != 0);
}

bool
network_gather(bool) {
    open_network_speaker();
    open_network_microphone();

    dialog d;

    d.add(new w_title("TEST MICROPHONE"));

    w_toggle*   onoff_w = new w_toggle("Active", 0);
    onoff_w->set_selection_changed_callback(respond_to_microphone_toggle);
    d.add(onoff_w);

    d.add(new w_button("DONE", dialog_ok, &d));

    d.run();

    close_network_microphone();
    close_network_speaker();

    return false;
}
#endif

#endif // !defined(DISABLE_NETWORKING)

