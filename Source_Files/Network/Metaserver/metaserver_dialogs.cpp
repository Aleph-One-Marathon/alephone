/*
 *  metaserver_dialogs.cpp - UI for metaserver client

	Copyright (C) 2004 and beyond by Woody Zenfell, III
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

 April 29, 2004 (Woody Zenfell):
	Created.
 */

#if !defined(DISABLE_NETWORKING)

#include "cseries.h"

#include "metaserver_dialogs.h"
#include "network_private.h" // GAME_PORT
#include "preferences.h"
#include "network_metaserver.h"
#include "map.h" // for _force_unique_teams!?!

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
		unsigned char* playerNameCStringStorage = pstrdup(player_preferences->name);
		char* playerNameCString = a1_p2cstr(playerNameCStringStorage);
		client.setPlayerName(playerNameCString);
		free(playerNameCStringStorage);
	}
	
	client.setPlayerTeamName("");
	client.connect("myth.mariusnet.com", 6321, "guest", "0000000000000000");
}



GameAvailableMetaserverAnnouncer::GameAvailableMetaserverAnnouncer(const game_info& info)
{
	setupAndConnectClient(*gMetaserverClient);

	GameDescription description;
	description.m_type = info.net_game_type;
	description.m_timeLimit = info.time_limit;
	description.m_difficulty = info.difficulty_level;
	description.m_mapName = string(info.level_name);
	description.m_name = gMetaserverClient->playerName() + "'s Game";
	description.m_teamsAllowed = !(info.game_options & _force_unique_teams);

	gMetaserverClient->announceGame(GAME_PORT, description);
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
	m_chatEntryWidget->set_callback(bind(&MetaserverClientUi::ChatTextEntered, this, _1));
	m_cancelWidget->set_callback(boost::bind(&MetaserverClientUi::handleCancel, this));

	gMetaserverChatHistory.clear ();
	m_textboxWidget->attachHistory (&gMetaserverChatHistory);

	Run();
	
	return m_joinAddress;
}

void MetaserverClientUi::GameSelected(GameListMessage::GameListEntry game)
{
	memcpy(&m_joinAddress.host, &game.m_ipAddress, sizeof(m_joinAddress.host));
	m_joinAddress.port = game.m_port;
	Stop();
}

void MetaserverClientUi::playersInRoomChanged()
{
	m_playersInRoomWidget->SetItems(gMetaserverClient->playersInRoom());
}

void MetaserverClientUi::gamesInRoomChanged()
{
	m_gamesInRoomWidget->SetItems(gMetaserverClient->gamesInRoom());
}

void MetaserverClientUi::receivedChatMessage(const std::string& senderName, uint32 senderID, const std::string& message)
{
	gMetaserverChatHistory.appendString (senderName + ": " + message);
}

void MetaserverClientUi::receivedBroadcastMessage(const std::string& message)
{
	gMetaserverChatHistory.appendString (message);
}

void MetaserverClientUi::sendChat()
{
	string message = m_chatEntryWidget->get_text();
#ifndef SDL
	// It's just a little semantic difference, really.  :)
	message = string(message.data (), message.length () - 1); // lose the last character, i.e. '\r'.
#endif
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
