/*
 *  metaserver_dialogs.h - UI for metaserver client

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

#ifndef METASERVER_DIALOGS_H
#define METASERVER_DIALOGS_H

#include "network_metaserver.h"
#include "metaserver_messages.h"
#include "shared_widgets.h"


const IPaddress run_network_metaserver_ui();

// This doesn't go here
void setupAndConnectClient(MetaserverClient& client);



struct game_info;

class GameAvailableMetaserverAnnouncer
{
public:
	GameAvailableMetaserverAnnouncer(const game_info& info);
	void Start(int32 time_limit);

private:
	// using gMetaserverClient instead
	// MetaserverClient	m_client;
};

class GlobalMetaserverChatNotificationAdapter : public MetaserverClient::NotificationAdapter
{
public:
	virtual void playersInRoomChanged(const std::vector<MetaserverPlayerInfo>&);
	virtual void gamesInRoomChanged(const std::vector<GameListMessage::GameListEntry>&);
	virtual void receivedChatMessage(const std::string& senderName, uint32 senderID, const std::string& message);
	virtual void receivedLocalMessage(const std::string& message);
	virtual void receivedBroadcastMessage(const std::string& message);
	virtual void receivedPrivateMessage(const std::string& senderName, uint32 senderID, const std::string& message);
	virtual void roomDisconnected();
};

// Eventually this may disappear behind the facade of run_network_metaserver_ui()
// Or maybe it will disappear instead, leaving this.  Unsure.
class MetaserverClientUi : public GlobalMetaserverChatNotificationAdapter
{
public:
	// Abstract factory; concrete type determined at link-time
	static std::unique_ptr<MetaserverClientUi> Create();

	const IPaddress GetJoinAddressByRunning();

	virtual ~MetaserverClientUi () {};

protected:
	MetaserverClientUi() : m_used (false), m_lastGameSelected(0) {}

	void delete_widgets ();

	virtual int Run() = 0;
	virtual void Stop() = 0;

	void GameSelected(GameListMessage::GameListEntry game);
	void JoinGame(const GameListMessage::GameListEntry&);
	void PlayerSelected(MetaserverPlayerInfo info);
	void MuteClicked();
	void JoinClicked();
	virtual void InfoClicked() { };
	void playersInRoomChanged(const std::vector<MetaserverPlayerInfo> &playerChanges);
	void gamesInRoomChanged(const std::vector<GameListMessage::GameListEntry> &gamesChanges);
	void sendChat();
	void ChatTextEntered (char character);
	void handleCancel();
	void UpdatePlayerButtons();
	void UpdateGameButtons();
	
	PlayerListWidget*				m_playersInRoomWidget;
	GameListWidget*					m_gamesInRoomWidget;
	EditTextWidget*					m_chatEntryWidget;
//	HistoricTextboxWidget*				m_textboxWidget;
	ColorfulChatWidget*                             m_chatWidget;
	ButtonWidget*					m_cancelWidget;
	IPaddress					m_joinAddress;
	bool						m_used;
	ButtonWidget*                                   m_muteWidget;
	ButtonWidget*                                   m_joinWidget;
	ButtonWidget*                                   m_gameInfoWidget;

	Uint32 m_lastGameSelected;
	bool m_stay_selected; // doesn't deselect after PM
};

#endif // METASERVER_DIALOGS_H
