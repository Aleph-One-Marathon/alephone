/*
 *  network_metaserver.h - Metaserver client

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

 April 15, 2004 (Woody Zenfell):
	Created.
 */

#ifndef NETWORK_METASERVER_H
#define NETWORK_METASERVER_H

#include "metaserver_messages.h" // RoomDescription

#include <exception>
#include <vector>
#include <map>
#include <memory> // unique_ptr
#include <set>
#include <stdexcept>

#include "Logging.h"

template <typename tElement>
class MetaserverMaintainedList
{
public:
	MetaserverMaintainedList() : m_target(Element::IdNone) { }
	typedef tElement		 	Element;
	typedef typename Element::IDType	IDType;

	void clear() { m_entries.clear(); }

	void processUpdates(const std::vector<Element>& updates)
	{
		for(size_t i = 0; i < updates.size(); i++)
			processUpdate(updates[i].verb(), updates[i].id(), updates[i]);

		if (m_target != Element::IdNone)
		{
			typename Map::iterator target = m_entries.find(m_target);
			if (target != m_entries.end())
			{
				target->second.target(true);
			}
			else
			{
				m_target = Element::IdNone;
			}
		}
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

	const Element* find(IDType id) const
	{
		typename Map::const_iterator it = m_entries.find(id);
		if (it != m_entries.end())
		{
			return &it->second;
		}
		else
		{
			return 0;
		}
	}

	enum
	{
		kAdd		= 0,
		kDelete		= 1,
		kRefresh	= 2
	};

	IDType target() { return m_target; }
	void target(IDType id) 
	{
		// clear the old target
		typename Map::iterator old_target = (m_target == Element::IdNone) ? m_entries.end() : m_entries.find(m_target);
		if (old_target != m_entries.end())
		{
			old_target->second.target(false);
		}
		
		typename Map::iterator e = (id == Element::IdNone) ? m_entries.end() : m_entries.find(id);
		if (e == m_entries.end())
		{
			m_target = Element::IdNone;
		}
		else
		{
			m_target = id;
			e->second.target(true);
		}
	}
		
	
private:
	typedef std::map<IDType, Element>	Map;

	Map	m_entries;

	void processUpdate(uint8 verb, IDType id, const Element& update)
	{
		switch(verb)
		{
			case kAdd:
				if(m_entries.find(id) != m_entries.end())
				{
					logAnomaly("received instruction to add item with same ID (%d) as known item; using the new one only", id);
					m_entries.erase(id);
				}
				m_entries.insert(typename Map::value_type(id, update));
				break;

			case kDelete:
				if(m_entries.erase(id) == 0)
				{
					logAnomaly("received instruction to delete unknown item (ID %d)", id);
				}
				break;

			case kRefresh:
				if(m_entries.erase(id) == 0)
				{
					logAnomaly("received instruction to refresh unknown item (ID %d); treating it as an add", id);
				}
				m_entries.insert(typename Map::value_type(id, update));
				break;

			default:
				logAnomaly("unknown list item verb %d - ignored", verb);
				break;
		}
	}

	IDType m_target;
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
                virtual void playersInRoomChanged(const std::vector<MetaserverPlayerInfo>&) = 0;
                virtual void gamesInRoomChanged(const std::vector<GameListMessage::GameListEntry>&) = 0;
                virtual void receivedChatMessage(const std::string& senderName, uint32 senderID, const std::string& message) = 0;
		virtual void receivedLocalMessage(const std::string& message) = 0;
                virtual void receivedBroadcastMessage(const std::string& message) = 0;
                virtual void receivedPrivateMessage(const std::string& senderName, uint32 senderID, const std::string& message) = 0;
		virtual void roomDisconnected() = 0;
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

	class LoginDeniedException : public std::runtime_error 
	{
	public:
		enum {
			SyntaxError,
			GamesNotAllowed,
			InvalidVersion,
			BadUserOrPassword,
			UserNotLoggedIn,
			BadMetaserverVersion,
			UserAlreadyLoggedIn,
			UnknownGameType,
			LoginSuccessful,
			LogoutSuccessful,
			PlayerNotInRoom,
			GameAlreadyExists,
			AccountAlreadyLoggedIn,
			RoomFull,
			AccountLocked,
			NotSupported
		};

		LoginDeniedException(int code, const std::string& arg) : std::runtime_error(arg), m_code(code) { }
		int code() const { return m_code; }
	private:
		int m_code;
	};
	class ServerConnectException : public std::runtime_error 
	{ 
	public:
		ServerConnectException(const std::string& arg) : std::runtime_error(arg) { }
	};

        void connect(const std::string& serverName, uint16 port, const std::string& userName, const std::string& userPassword);
	void disconnect();
	bool isConnected() const;

	void setPlayerName(const std::string& name);
	const std::string& playerName() const { return m_playerName; }

	void setAway(bool away, const std::string& away_message);
	void setMode(uint16 mode, const std::string& session_id);

	void setPlayerTeamName(const std::string& team);

	const Rooms& rooms() const;
	void setRoom(const RoomDescription& room);

        const std::vector<MetaserverPlayerInfo> playersInRoom() const { return m_playersInRoom.entries(); }
        const std::vector<GameListMessage::GameListEntry> gamesInRoom() const { return m_gamesInRoom.entries(); }

	void pump();
	static void pumpAll();

	void sendChatMessage(const std::string& message);
	void sendPrivateMessage(MetaserverPlayerInfo::IDType destination, const std::string& message);
	void announceGame(uint16 gamePort, const GameDescription& description);
	void announcePlayersInGame(uint8 players);
	void announceGameStarted(int32 gameTimeInSeconds);
	void announceGameReset();
	void announceGameDeleted();
	void ignore(const std::string& name);
	void ignore(MetaserverPlayerInfo::IDType id);
	bool is_ignored(MetaserverPlayerInfo::IDType id);
	void syncGames();

	void player_target(MetaserverPlayerInfo::IDType id) { m_playersInRoom.target(id); };
	MetaserverPlayerInfo::IDType player_target() { return m_playersInRoom.target(); };
	const MetaserverPlayerInfo* find_player(MetaserverPlayerInfo::IDType id) { return m_playersInRoom.find(id); }
	void game_target(GameListMessage::GameListEntry::IDType id) { m_gamesInRoom.target(id); }
	GameListMessage::GameListEntry::IDType game_target() { return m_gamesInRoom.target(); };
	const GameListMessage::GameListEntry* find_game(GameListMessage::GameListEntry::IDType id) { return m_gamesInRoom.find(id); }

	~MetaserverClient();

private:
	void handleUnexpectedMessage(Message* inMessage, CommunicationsChannel* inChannel);
	void handleChatMessage(ChatMessage* inMessage, CommunicationsChannel* inChannel);
	void handlePrivateMessage(PrivateMessage* inMessage, CommunicationsChannel* inChannel);
	void handleKeepAliveMessage(Message* inMessage, CommunicationsChannel* inChannel);
	void handleBroadcastMessage(BroadcastMessage* inMessage, CommunicationsChannel* inChannel);
        void handlePlayerListMessage(PlayerListMessage* inMessage, CommunicationsChannel* inChannel);
        void handleRoomListMessage(RoomListMessage* inMessage, CommunicationsChannel* inChannel);
        void handleGameListMessage(GameListMessage* inMessage, CommunicationsChannel* inChannel);
	void handleSetPlayerDataMessage(SetPlayerDataMessage*, CommunicationsChannel *) { }

	std::unique_ptr<CommunicationsChannel>    m_channel;
	std::unique_ptr<MessageInflater>          m_inflater;
	std::unique_ptr<MessageDispatcher>        m_dispatcher;
	std::unique_ptr<MessageDispatcher>        m_loginDispatcher;
	std::unique_ptr<MessageHandler>           m_unexpectedMessageHandler;
	std::unique_ptr<MessageHandler>           m_chatMessageHandler;
	std::unique_ptr<MessageHandler>           m_keepAliveMessageHandler;
	std::unique_ptr<MessageHandler>           m_broadcastMessageHandler;
	std::unique_ptr<MessageHandler>           m_playerListMessageHandler;
	std::unique_ptr<MessageHandler>           m_roomListMessageHandler;
	std::unique_ptr<MessageHandler>           m_gameListMessageHandler;
	std::unique_ptr<MessageHandler>           m_privateMessageHandler;
	std::unique_ptr<MessageHandler>           m_setPlayerDataMessageHandler;
	Rooms					m_rooms;
	RoomDescription				m_room;
        PlayersInRoom				m_playersInRoom;
        GamesInRoom				m_gamesInRoom;
	std::string				m_playerName;
	std::string				m_teamName;
        NotificationAdapter*			m_notificationAdapter;
	uint32					m_playerID;

	static std::set<MetaserverClient*>	s_instances;
	static std::set<std::string>            s_ignoreNames;

	GameDescription                         m_gameDescription;
	uint16                                  m_gamePort;

	MetaserverPlayerInfo::IDType            m_player_target;
	bool                                    m_player_target_exists;

	bool                                    m_notifiedOfDisconnected;
	bool                                    m_gameAnnounced;
};

#endif // NETWORK_METASERVER_H
