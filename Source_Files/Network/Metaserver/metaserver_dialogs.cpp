/*
 *  metaserver_dialogs.cpp - UI for metaserver client

	Copyright (C) 2004 and beyond by Woody Zenfell, III
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

 April 29, 2004 (Woody Zenfell):
	Created.
 */

#if !defined(DISABLE_NETWORKING)

#include "cseries.h"

#include "alephversion.h"
#include "metaserver_dialogs.h"
#include "network_private.h" // GAME_PORT
#include "preferences.h"
#include "network_metaserver.h"
#include "map.h" // for _force_unique_teams!?!
#include "SoundManager.h"
#include "game_wad.h" // embedded physics/lua detection!!?!?!!

#include "Update.h"
#include "progress.h"

extern ChatHistory gMetaserverChatHistory;
extern MetaserverClient* gMetaserverClient;

const IPaddress
run_network_metaserver_ui()
{
	return MetaserverClientUi::Create()->GetJoinAddressByRunning();
}


void
setupAndConnectClient(MetaserverClient& client)
{
	{
		client.setPlayerName(player_preferences->name);
	}

	// Check the updates URL for updates
	
	static bool user_informed = false;

	if (network_preferences->check_for_updates && !user_informed)
	{
		static bool first_check = true;
		
		if (first_check)
		{
			uint32 ticks = machine_tick_count();

			// if we get an update in a short amount of time, don't display progress
			while (Update::instance()->GetStatus() == Update::CheckingForUpdate && machine_tick_count() - ticks < 500);

			// check another couple seconds, but with a progress dialog
			open_progress_dialog(_checking_for_updates);
			while (Update::instance()->GetStatus() == Update::CheckingForUpdate && machine_tick_count() - ticks < 2500);
			close_progress_dialog();
			first_check = false;
		}

		Update::Status status = Update::instance()->GetStatus();
		if (status == Update::UpdateAvailable)
		{
			dialog d;
			vertical_placer *placer = new vertical_placer;

			placer->dual_add(new w_title("UPDATE AVAILABLE"), d);
			placer->add(new w_spacer(), true);

			placer->dual_add(new w_static_text(expand_app_variables("An update for $appName$ is available.").c_str()), d);
#ifdef MAC_APP_STORE
			placer->dual_add(new w_static_text("Please download it from the App Store"), d);
#else
			placer->dual_add(new w_static_text("Please download it from"), d);
			placer->dual_add(new w_hyperlink(A1_HOMEPAGE_URL), d);
#endif
			placer->dual_add(new w_static_text("before playing games online."), d);
			
			placer->add(new w_spacer(), true);
			placer->dual_add(new w_button("OK", dialog_ok, &d), d);
			d.set_widget_placer(placer);
			d.run();
		}
		
		if (status != Update::CheckingForUpdate) user_informed = true;
	}

	client.setPlayerTeamName("");
	client.connect(A1_METASERVER_HOST, 6321, network_preferences->metaserver_login, network_preferences->metaserver_password);
}



GameAvailableMetaserverAnnouncer::GameAvailableMetaserverAnnouncer(const game_info& info)
{
	setupAndConnectClient(*gMetaserverClient);

	GameDescription description;
	description.m_type = info.net_game_type;
	// If the time limit is longer than a week, we figure it's untimed (  ;)
	description.m_timeLimit = (info.time_limit > 7 * 24 * 3600 * TICKS_PER_SECOND) ? -1 : info.time_limit;
	description.m_difficulty = info.difficulty_level;
	description.m_mapName = string(info.level_name);
	description.m_name = gMetaserverClient->playerName() + "'s Game";
	description.m_teamsAllowed = !(info.game_options & _force_unique_teams);
	
	// description's constructor gets scenario info, aleph one's protocol ID for us
	
	description.m_alephoneBuildString = string(A1_DISPLAY_VERSION) + " (" + A1_DISPLAY_PLATFORM + ")";

	bool HasPhysics, HasLua;
	level_has_embedded_physics_lua(info.level_number, HasPhysics, HasLua);

	if (network_preferences->use_netscript)
	{
		FileSpecifier netScript(network_preferences->netscript_file);
		char netScriptName[256];
		netScript.GetName(netScriptName);
		netScriptName[32] = '\0';
		description.m_netScript = netScriptName;
	}
	else if (HasLua)
	{
		description.m_netScript = "Embedded";
	} // else constructor's blank string is desirable

	description.m_hasGameOptions = true;
	description.m_gameOptions = info.game_options;
	description.m_cheatFlags = info.cheat_flags;
	description.m_killLimit = info.kill_limit;

	if (HasPhysics)
	{
		description.m_physicsName = "Embedded";
	}
	else
	{
		char name[256];
		FileSpecifier fs = environment_preferences->map_file;
		fs.GetName(name);
		description.m_mapFileName = name;
		
		fs = environment_preferences->physics_file;
		if (fs.Exists() && fs.GetType() == _typecode_physics)
		{
			fs.GetName(name);
			description.m_physicsName = name;
		}
	}
	
	gMetaserverClient->announceGame(GAME_PORT, description);
}

void GameAvailableMetaserverAnnouncer::Start(int32 time_limit)
{
	gMetaserverClient->announceGameStarted(time_limit);

}

extern void PlayInterfaceButtonSound(short);

void GlobalMetaserverChatNotificationAdapter::playersInRoomChanged(const std::vector<MetaserverPlayerInfo>& playerChanges)
{
	// print some notifications to the chat window
	for (size_t i = 0; i < playerChanges.size(); i++) 
	{
		if (playerChanges[i].verb() == MetaserverClient::PlayersInRoom::kAdd)
		{
			receivedLocalMessage(playerChanges[i].name() + "|p has joined the room");
		}
		else if (playerChanges[i].verb() == MetaserverClient::PlayersInRoom::kDelete)
		{
			receivedLocalMessage(playerChanges[i].name() + "|p has left the room");
		}
	}
}

void GlobalMetaserverChatNotificationAdapter::gamesInRoomChanged(const std::vector<GameListMessage::GameListEntry>& gameChanges)
{
	for (size_t i = 0; i < gameChanges.size(); i++) {
		if (gameChanges[i].verb() == MetaserverClient::GamesInRoom::kAdd) {
			string name;
			// find the player's name
			for (size_t playerIndex = 0; playerIndex < gMetaserverClient->playersInRoom().size(); playerIndex++)
			{
				if (gMetaserverClient->playersInRoom()[playerIndex].id() == gameChanges[i].m_hostPlayerID) {
					name = gMetaserverClient->playersInRoom()[playerIndex].name();
					break;
				}
				}
			
			if (name.size() > 0) {
				receivedLocalMessage(gameChanges[i].format_for_chat(name));
			}
		}
	}
}

static void color_entry(ColoredChatEntry& e, const MetaserverPlayerInfo *player)
{
	if (player)
	{
		e.color.red = player->color()[0];
		e.color.green = player->color()[1];
		e.color.blue = player->color()[2];
	}
}
void GlobalMetaserverChatNotificationAdapter::receivedChatMessage(const std::string& senderName, uint32 senderID, const std::string& message)
{
	ColoredChatEntry e;
	e.type = ColoredChatEntry::ChatMessage;
	e.sender = senderName;
	e.message = message;

	color_entry(e, gMetaserverClient->find_player(senderID));

	gMetaserverChatHistory.append(e);
	PlayInterfaceButtonSound(_snd_computer_interface_logon);
}

void GlobalMetaserverChatNotificationAdapter::receivedPrivateMessage(const std::string& senderName, uint32 senderID, const std::string& message)
{
	ColoredChatEntry e;
	e.type = ColoredChatEntry::PrivateMessage;
	e.sender = senderName;
	e.message = message;

	color_entry(e, gMetaserverClient->find_player(senderID));
	
	gMetaserverChatHistory.append(e);
	PlayInterfaceButtonSound(_snd_compiler_projectile_flyby);
}

void GlobalMetaserverChatNotificationAdapter::receivedBroadcastMessage(const std::string& message)
{
	ColoredChatEntry e;
	e.type = ColoredChatEntry::ServerMessage;
	e.message = message;
	
	gMetaserverChatHistory.append(e);
}

void GlobalMetaserverChatNotificationAdapter::receivedLocalMessage(const std::string& message)
{
	ColoredChatEntry e;
	e.type = ColoredChatEntry::LocalMessage;
	e.message = message;

	gMetaserverChatHistory.append(e);
}

void GlobalMetaserverChatNotificationAdapter::roomDisconnected()
{
	ColoredChatEntry e;
	e.type = ColoredChatEntry::LocalMessage;
	e.message = "|iConnection to room lost.";
	
	gMetaserverChatHistory.append(e);
}

void MetaserverClientUi::delete_widgets ()
{
	delete m_playersInRoomWidget;
	delete m_gamesInRoomWidget;
	delete m_chatEntryWidget;
	delete m_chatWidget;
	delete m_cancelWidget;
	delete m_muteWidget;
	delete m_joinWidget;
	delete m_gameInfoWidget;
}

const IPaddress MetaserverClientUi::GetJoinAddressByRunning()
{
	// This was designed with one-shot-ness in mind
	assert(!m_used);
	m_used = true;
	
	obj_clear(m_joinAddress);

	setupAndConnectClient(*gMetaserverClient);
	gMetaserverClient->associateNotificationAdapter(this);

	m_gamesInRoomWidget->SetItemSelectedCallback(bind(&MetaserverClientUi::GameSelected, this, _1));
	m_playersInRoomWidget->SetItemSelectedCallback(bind(&MetaserverClientUi::PlayerSelected, this, _1));
	m_muteWidget->set_callback(boost::bind(&MetaserverClientUi::MuteClicked, this));
	m_chatEntryWidget->set_callback(bind(&MetaserverClientUi::ChatTextEntered, this, _1));
	m_cancelWidget->set_callback(boost::bind(&MetaserverClientUi::handleCancel, this));
	m_joinWidget->set_callback(boost::bind(&MetaserverClientUi::JoinClicked, this));
	m_gameInfoWidget->set_callback(boost::bind(&MetaserverClientUi::InfoClicked, this));
	
	gMetaserverChatHistory.clear ();
	m_chatWidget->attachHistory (&gMetaserverChatHistory);

	if (Run() < 0) {
		handleCancel();
	}
	
	return m_joinAddress;
}

void MetaserverClientUi::GameSelected(GameListMessage::GameListEntry game)
{
	if (gMetaserverClient->game_target() == game.id())
	{
		if (machine_tick_count() - m_lastGameSelected < 333 && (!game.running() && Scenario::instance()->IsCompatible(game.m_description.m_scenarioID)))
		{
			JoinClicked();
		}
		else
		{
			gMetaserverClient->game_target(GameListMessage::GameListEntry::IdNone);
		}
	}
	else
	{
		m_lastGameSelected = machine_tick_count();
		gMetaserverClient->game_target(game.id());
	}

	std::vector<GameListMessage::GameListEntry> sortedGames = gMetaserverClient->gamesInRoom();
	std::sort(sortedGames.begin(), sortedGames.end(), GameListMessage::GameListEntry::sort);
	m_gamesInRoomWidget->SetItems(sortedGames);
	UpdateGameButtons();
}

void MetaserverClientUi::UpdatePlayerButtons()
{
	if (gMetaserverClient->player_target() == MetaserverPlayerInfo::IdNone)
	{
		m_muteWidget->deactivate();
	}
	else
	{
		m_muteWidget->activate();
	}	
}

void MetaserverClientUi::UpdateGameButtons()
{
	if (gMetaserverClient->game_target() == GameListMessage::GameListEntry::IdNone)
	{
		m_joinWidget->deactivate();
		m_gameInfoWidget->deactivate();
	}
	else
	{
		const GameListMessage::GameListEntry *game = gMetaserverClient->find_game(gMetaserverClient->game_target());
		if (game)
		{
			m_gameInfoWidget->activate();
			if (!game->running() && Scenario::instance()->IsCompatible(game->m_description.m_scenarioID))
			{
				m_joinWidget->activate();
			}
			else
			{
				m_joinWidget->deactivate();
			}
		}
		else
		{
			// huh?
			m_gameInfoWidget->deactivate();
			m_joinWidget->deactivate();
		}
	}
}

void MetaserverClientUi::PlayerSelected(MetaserverPlayerInfo info)
{
	if (gMetaserverClient->player_target() == info.id())
	{
		gMetaserverClient->player_target(MetaserverPlayerInfo::IdNone);
	}
	else
	{
		gMetaserverClient->player_target(info.id());
		if (SDL_GetModState() & KMOD_CTRL)
		{
			m_stay_selected = true;
		}
		else
		{
			m_stay_selected = false;
		}
	}

	std::vector<MetaserverPlayerInfo> sortedPlayers = gMetaserverClient->playersInRoom();
	std::sort(sortedPlayers.begin(), sortedPlayers.end(), MetaserverPlayerInfo::sort);

	m_playersInRoomWidget->SetItems(sortedPlayers);
	UpdatePlayerButtons();
}

void MetaserverClientUi::MuteClicked()
{
	gMetaserverClient->ignore(gMetaserverClient->player_target());
}

void MetaserverClientUi::JoinClicked()
{
	const GameListMessage::GameListEntry *game = gMetaserverClient->find_game(gMetaserverClient->game_target());
	if (game)
	{
		JoinGame(*game);
	}
}

void MetaserverClientUi::JoinGame(const GameListMessage::GameListEntry& game)
{
	memcpy(&m_joinAddress.host, &game.m_ipAddress, sizeof(m_joinAddress.host));
	m_joinAddress.port = game.m_port;
	Stop();
}

void MetaserverClientUi::playersInRoomChanged(const std::vector<MetaserverPlayerInfo> &playerChanges)
{
	std::vector<MetaserverPlayerInfo> sortedPlayers = gMetaserverClient->playersInRoom();
	std::sort(sortedPlayers.begin(), sortedPlayers.end(), MetaserverPlayerInfo::sort);

	m_playersInRoomWidget->SetItems(sortedPlayers);
	UpdatePlayerButtons();
	GlobalMetaserverChatNotificationAdapter::playersInRoomChanged(playerChanges);

}

void MetaserverClientUi::gamesInRoomChanged(const std::vector<GameListMessage::GameListEntry> &gameChanges)
{
	std::vector<GameListMessage::GameListEntry> sortedGames = gMetaserverClient->gamesInRoom();
	std::sort(sortedGames.begin(), sortedGames.end(), GameListMessage::GameListEntry::sort);
	m_gamesInRoomWidget->SetItems(sortedGames);
	UpdateGameButtons();
	GlobalMetaserverChatNotificationAdapter::gamesInRoomChanged(gameChanges);
	for (size_t i = 0; i < gameChanges.size(); i++) 
	{
		if (gameChanges[i].verb() == MetaserverClient::GamesInRoom::kAdd && !gameChanges[i].running())
		{
			PlayInterfaceButtonSound(_snd_got_ball);
			break;
		}
	}
}

void MetaserverClientUi::sendChat()
{
	string message = m_chatEntryWidget->get_text();
	if (gMetaserverClient->player_target() != MetaserverPlayerInfo::IdNone)
	{
		gMetaserverClient->sendPrivateMessage(gMetaserverClient->player_target(), message);
		if (!m_stay_selected)
			gMetaserverClient->player_target(MetaserverPlayerInfo::IdNone);
		std::vector<MetaserverPlayerInfo> sortedPlayers = gMetaserverClient->playersInRoom();
		std::sort(sortedPlayers.begin(), sortedPlayers.end(), MetaserverPlayerInfo::sort);
		
		m_playersInRoomWidget->SetItems(sortedPlayers);
		UpdatePlayerButtons();
	}
	else
		gMetaserverClient->sendChatMessage(message);
	m_chatEntryWidget->set_text(string());
}
	
void MetaserverClientUi::ChatTextEntered (char character)
{
	if (character == '\r')
		sendChat();
}

void MetaserverClientUi::handleCancel ()
{
	// gMetaserverClient->disconnect ();
	delete gMetaserverClient;
	gMetaserverClient = new MetaserverClient ();
	Stop ();
}

#endif // !defined(DISABLE_NETWORKING)
