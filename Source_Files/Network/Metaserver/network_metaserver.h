/*
 *  network_metaserver.h - Metaserver client

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

 April 15, 2004 (Woody Zenfell):
	Created.
 */

#ifndef NETWORK_METASERVER_H
#define NETWORK_METASERVER_H

#include "metaserver_messages.h" // RoomDescription

#include <vector>
#include <map>
#include <memory> // auto_ptr
#include <set>

#include "Logging.h"

template <typename tElement>
class MetaserverMaintainedList
{
public:
	typedef tElement		 	Element;
	typedef typename Element::IDType	IDType;

	void processUpdates(const std::vector<Element>& updates)
	{
		for(size_t i = 0; i < updates.size(); i++)
			processUpdate(updates[i].verb(), updates[i].id(), updates[i]);
	}

	const std::vector<Element> entries() const
	{
		std::vector<Element>	result;

		for(typename Map::const_iterator i = m_entries.begin(); i != m_entries.end(); ++i)
		{
			result.push_back(i->second);
		}

		return result;
	}

private:
	typedef std::map<IDType, Element>	Map;

	enum
	{
		kAdd		= 0,
		kDelete		= 1,
		kRefresh	= 2
	};

	Map	m_entries;

	void processUpdate(uint8 verb, IDType id, const Element& update)
	{
		switch(verb)
		{
			case kAdd:
				if(m_entries.find(id) != m_entries.end())
				{
					logAnomaly1("received instruction to add item with same ID (%d) as known item; using the new one only", id);
					m_entries.erase(id);
				}
				m_entries.insert(typename Map::value_type(id, update));
				break;

			case kDelete:
				if(m_entries.erase(id) == 0)
				{
					logAnomaly1("received instruction to delete unknown item (ID %d)", id);
				}
				break;

			case kRefresh:
				if(m_entries.erase(id) == 0)
				{
					logAnomaly1("received instruction to refresh unknown item (ID %d); treating it as an add", id);
				}
				m_entries.insert(typename Map::value_type(id, update));
				break;

			default:
				logAnomaly1("unknown list item verb %d - ignored", verb);
				break;
		}
	}
};



class CommunicationsChannel;
class MessageInflater;
class MessageHandler;
class MessageDispatcher;

class Message;
class ChatMessage;
class BroadcastMessage;

class MetaserverClient
{
public:
        class NotificationAdapter
        {
        public:
                virtual void playersInRoomChanged() = 0;
                virtual void gamesInRoomChanged() = 0;
                virtual void receivedChatMessage(const std::string& senderName, uint32 senderID, const std::string& message) = 0;
                virtual void receivedBroadcastMessage(const std::string& message) = 0;
                virtual ~NotificationAdapter() {}
        };

		class NotificationAdapterInstaller
		{
		public:
			NotificationAdapterInstaller(NotificationAdapter* adapter, MetaserverClient& metaserverClient)
				: m_adapter(adapter), m_metaserverClient(metaserverClient)
			{
				m_previousAdapter = m_metaserverClient.notificationAdapter();
				m_metaserverClient.associateNotificationAdapter(m_adapter);
			}

			~NotificationAdapterInstaller()
			{
				assert(m_metaserverClient.notificationAdapter() == m_adapter);
				m_metaserverClient.associateNotificationAdapter(m_previousAdapter);
			}

		private:
			NotificationAdapter*	m_previousAdapter;
			NotificationAdapter*	m_adapter;
			MetaserverClient&		m_metaserverClient;

			NotificationAdapterInstaller(const NotificationAdapterInstaller&);
			NotificationAdapterInstaller& operator =(const NotificationAdapterInstaller&);
		};

        typedef std::vector<RoomDescription>					Rooms;
        typedef MetaserverMaintainedList<MetaserverPlayerInfo>			PlayersInRoom;
        typedef MetaserverMaintainedList<GameListMessage::GameListEntry>	GamesInRoom;
        
	MetaserverClient();

        void associateNotificationAdapter(NotificationAdapter* adapter)
                { m_notificationAdapter = adapter; }
        NotificationAdapter* notificationAdapter() const { return m_notificationAdapter; }

	void connect(const std::string& serverName, uint16 port, const std::string& userName, const std::string& userPassword);
	void disconnect();
	bool isConnected() const;

	void setPlayerName(const std::string& name);
	const std::string& playerName() const { return m_playerName; }

	void setPlayerTeamName(const std::string& team);

	const Rooms& rooms() const;
	void setRoom(const RoomDescription& room);

        const std::vector<MetaserverPlayerInfo> playersInRoom() const { return m_playersInRoom.entries(); }
        const std::vector<GameListMessage::GameListEntry> gamesInRoom() const { return m_gamesInRoom.entries(); }

	void pump();
	static void pumpAll();

	void sendChatMessage(const std::string& message);
	void announceGame(uint16 gamePort, const GameDescription& description);
	void announceGameStarted(int32 gameTimeInSeconds);
	void announceGameReset();
	void announceGameDeleted();
	void syncGames();

	~MetaserverClient();

private:
	void handleUnexpectedMessage(Message* inMessage, CommunicationsChannel* inChannel);
	void handleChatMessage(ChatMessage* inMessage, CommunicationsChannel* inChannel);
	void handleKeepAliveMessage(Message* inMessage, CommunicationsChannel* inChannel);
	void handleBroadcastMessage(BroadcastMessage* inMessage, CommunicationsChannel* inChannel);
        void handlePlayerListMessage(PlayerListMessage* inMessage, CommunicationsChannel* inChannel);
        void handleRoomListMessage(RoomListMessage* inMessage, CommunicationsChannel* inChannel);
        void handleGameListMessage(GameListMessage* inMessage, CommunicationsChannel* inChannel);

	std::auto_ptr<CommunicationsChannel>	m_channel;
	std::auto_ptr<MessageInflater>		m_inflater;
	std::auto_ptr<MessageDispatcher>	m_dispatcher;
	std::auto_ptr<MessageHandler>		m_unexpectedMessageHandler;
	std::auto_ptr<MessageHandler>		m_chatMessageHandler;
	std::auto_ptr<MessageHandler>		m_keepAliveMessageHandler;
	std::auto_ptr<MessageHandler>		m_broadcastMessageHandler;
        std::auto_ptr<MessageHandler>		m_playerListMessageHandler;
        std::auto_ptr<MessageHandler>		m_roomListMessageHandler;
	std::auto_ptr<MessageHandler>		m_gameListMessageHandler;
	Rooms					m_rooms;
	RoomDescription				m_room;
        PlayersInRoom				m_playersInRoom;
        GamesInRoom				m_gamesInRoom;
	std::string				m_playerName;
	std::string				m_teamName;
        NotificationAdapter*			m_notificationAdapter;
	uint32					m_playerID;

	static std::set<MetaserverClient*>	s_instances;
};

#endif // NETWORK_METASERVER_H
