/*
 *  network_metaserver.cpp - Metaserver client

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

#if !defined(DISABLE_NETWORKING)

#include "cseries.h"

#include "network_metaserver.h"

#include "metaserver_messages.h"

#include "Message.h"
#include "MessageHandler.h"
#include "MessageDispatcher.h"
#include "MessageInflater.h"
#include "CommunicationsChannel.h"
#include "preferences.h"
#include "alephversion.h"
#include "HTTP.h"

#include <string>
#include <iostream>
#include <iterator> // ostream_iterator
#include <algorithm>
#include "Logging.h"

#include <boost/algorithm/string/predicate.hpp>


using namespace std;


set<MetaserverClient*> MetaserverClient::s_instances;
set<string> MetaserverClient::s_ignoreNames;


static const int kKeyLength = 16;

static std::string remove_formatting(const std::string &s);

void
MetaserverClient::handleUnexpectedMessage(Message* inMessage, CommunicationsChannel* inChannel)
{
	logAnomaly("Metaserver received message ID %i", inMessage->type());
	if(inMessage->type() == UninflatedMessage::kTypeID)
	{
		UninflatedMessage* theMessage = dynamic_cast<UninflatedMessage*>(inMessage);
		if(theMessage != NULL)
			logAnomaly("-- internal ID %i, length %i", theMessage->inflatedType(), theMessage->length());
	}
}


void
MetaserverClient::handleBroadcastMessage(BroadcastMessage* inMessage, CommunicationsChannel* inChannel)
{
	if(m_notificationAdapter)
		m_notificationAdapter->receivedBroadcastMessage(inMessage->message());
}


void
MetaserverClient::handleChatMessage(ChatMessage* message, CommunicationsChannel* inChannel)
{
	if(message->internalType() == 0 && m_notificationAdapter)
	{
		std::string realSenderName = remove_formatting(message->senderName());
		if (realSenderName[0] == '\260') realSenderName.erase(realSenderName.begin());
		if (network_preferences->mute_metaserver_guests && boost::algorithm::starts_with(realSenderName, "Guest "))
			return;
		if (s_ignoreNames.find(realSenderName) != s_ignoreNames.end()) 
			return;

		if (message->directed())
			m_notificationAdapter->receivedPrivateMessage(message->senderName(), message->senderID(), message->message());
		else
			m_notificationAdapter->receivedChatMessage(message->senderName(), message->senderID(), message->message());
	}
}

void MetaserverClient::handlePrivateMessage(PrivateMessage* message, CommunicationsChannel* inChannel)
{
	if (message->internalType() == 0 && m_notificationAdapter)
	{		
		std::string realSenderName = remove_formatting(message->senderName());
		if (realSenderName[0] == '\260') realSenderName.erase(realSenderName.begin());
		if (network_preferences->mute_metaserver_guests && boost::algorithm::starts_with(realSenderName, "Guest "))
			return;

		if (s_ignoreNames.find(realSenderName) != s_ignoreNames.end()) 
			return;

		if (message->directed())
			m_notificationAdapter->receivedPrivateMessage(message->senderName(), message->senderID(), message->message());
		else
			m_notificationAdapter->receivedChatMessage(message->senderName(), message->senderID(), message->message());
	}
}


void
MetaserverClient::handleKeepAliveMessage(Message* inMessage, CommunicationsChannel* inChannel)
{
	inChannel->enqueueOutgoingMessage(KeepAliveMessage());
}


void
MetaserverClient::handlePlayerListMessage(PlayerListMessage* inMessage, CommunicationsChannel* inChannel)
{
	std::vector<MetaserverPlayerInfo> players = inMessage->players();
	if (m_notificationAdapter)
	{
		// metaserver says invisible people leave twice
		std::vector<MetaserverPlayerInfo>::iterator it = players.begin();
		while (it != players.end())
		{
			if (it->verb() == PlayersInRoom::kDelete && !find_player(it->playerID()))
			{
				it = players.erase(it);
			}
			else
			{
				++it;
			}
		}
	}

	m_playersInRoom.processUpdates(inMessage->players());

	if(m_notificationAdapter) {
		m_notificationAdapter->playersInRoomChanged(players);
	}
}


void
MetaserverClient::handleRoomListMessage(RoomListMessage* inMessage, CommunicationsChannel* inChannel)
{
	m_rooms = inMessage->rooms();
}

void
MetaserverClient::handleGameListMessage(GameListMessage* inMessage, CommunicationsChannel* inChannel)
{
	vector<GameListMessage::GameListEntry> entries = inMessage->entries();
	for (vector<GameListMessage::GameListEntry>::iterator it = entries.begin(); it != entries.end(); ++it)
	{
		const MetaserverPlayerInfo *player = m_playersInRoom.find(it->m_hostPlayerID);
		if (player)
		{
			it->m_hostPlayerName = player->name();
		}
	}

	m_gamesInRoom.processUpdates(entries);

	if(m_notificationAdapter) {
		m_notificationAdapter->gamesInRoomChanged(inMessage->entries());
	}
}


MetaserverClient::MetaserverClient()
	: m_channel(new CommunicationsChannel())
	, m_inflater(new MessageInflater())
	, m_dispatcher(new MessageDispatcher())
	, m_loginDispatcher(new MessageDispatcher())
	, m_notificationAdapter(NULL)
	, m_notifiedOfDisconnected(false)
	, m_gameAnnounced(false)
{
	m_unexpectedMessageHandler.reset(newMessageHandlerMethod(this, &MetaserverClient::handleUnexpectedMessage));
	m_broadcastMessageHandler.reset(newMessageHandlerMethod(this, &MetaserverClient::handleBroadcastMessage));
	m_chatMessageHandler.reset(newMessageHandlerMethod(this, &MetaserverClient::handleChatMessage));
	m_privateMessageHandler.reset(newMessageHandlerMethod(this, &MetaserverClient::handlePrivateMessage));
	m_keepAliveMessageHandler.reset(newMessageHandlerMethod(this, &MetaserverClient::handleKeepAliveMessage));
	m_playerListMessageHandler.reset(newMessageHandlerMethod(this, &MetaserverClient::handlePlayerListMessage));
	m_roomListMessageHandler.reset(newMessageHandlerMethod(this, &MetaserverClient::handleRoomListMessage));
	m_gameListMessageHandler.reset(newMessageHandlerMethod(this, &MetaserverClient::handleGameListMessage));
	m_setPlayerDataMessageHandler.reset(newMessageHandlerMethod(this, &MetaserverClient::handleSetPlayerDataMessage));

	m_inflater->learnPrototype(SaltMessage());
	m_inflater->learnPrototype(AcceptMessage());
	m_inflater->learnPrototype(RoomListMessage());
	m_inflater->learnPrototype(IDAndLimitMessage());
	m_inflater->learnPrototype(DenialMessage());
	m_inflater->learnPrototype(BroadcastMessage());
	m_inflater->learnPrototype(ChatMessage());
	m_inflater->learnPrototype(PrivateMessage());
	m_inflater->learnPrototype(KeepAliveMessage());
	m_inflater->learnPrototype(PlayerListMessage());
	m_inflater->learnPrototype(LoginSuccessfulMessage());
	m_inflater->learnPrototype(SetPlayerDataMessage());
	m_inflater->learnPrototype(GameListMessage());

	m_channel->setMessageInflater(m_inflater.get());

	m_dispatcher->setDefaultHandler(m_unexpectedMessageHandler.get());
	m_dispatcher->setHandlerForType(m_broadcastMessageHandler.get(), BroadcastMessage::kType);
	m_dispatcher->setHandlerForType(m_chatMessageHandler.get(), ChatMessage::kType);
	m_dispatcher->setHandlerForType(m_privateMessageHandler.get(), PrivateMessage::kType);
	m_dispatcher->setHandlerForType(m_keepAliveMessageHandler.get(), KeepAliveMessage::kType);
	m_dispatcher->setHandlerForType(m_playerListMessageHandler.get(), PlayerListMessage::kType);
	m_dispatcher->setHandlerForType(m_roomListMessageHandler.get(), RoomListMessage::kType);
	m_dispatcher->setHandlerForType(m_gameListMessageHandler.get(), GameListMessage::kType);
	m_dispatcher->setHandlerForType(m_setPlayerDataMessageHandler.get(), SetPlayerDataMessage::kType);

	m_loginDispatcher->setDefaultHandler(m_unexpectedMessageHandler.get());
	m_loginDispatcher->setHandlerForType(m_setPlayerDataMessageHandler.get(), SetPlayerDataMessage::kType);

	s_instances.insert(this);
	s_ignoreNames.insert("Bacon");

}

void
MetaserverClient::connect(const std::string& serverName, uint16 port, const std::string& userName, const std::string& userPassword)
{
	try 
	{
		m_channel->setMessageHandler(m_loginDispatcher.get());

		if (m_channel->isConnected()) m_channel->disconnect();

		m_playersInRoom.clear();
		m_gamesInRoom.clear();

		m_channel->connect(serverName.c_str(), port);

		LoginAndPlayerInfoMessage theLoginMessage(userName, m_playerName, m_teamName);
		m_channel->enqueueOutgoingMessage(theLoginMessage);
	
		std::unique_ptr<Message> theSaltOrAcceptMessage(m_channel->receiveMessage());
		if (theSaltOrAcceptMessage.get() == 0)
			throw ServerConnectException("Server Disconnected");
	
		if (dynamic_cast<SaltMessage*>(theSaltOrAcceptMessage.get()) != 0)
		{
			SaltMessage* theSaltMessage = dynamic_cast<SaltMessage*>(theSaltOrAcceptMessage.get());

			char theKey[kKeyLength];
			char thePasswordCopy[kKeyLength];
			memset(thePasswordCopy, 0x23, sizeof(thePasswordCopy));
			strncpy(reinterpret_cast<char*>(thePasswordCopy), userPassword.c_str(), kKeyLength);

			if (theSaltMessage->encryptionType() == SaltMessage::kPlaintextEncryption) 
			{
				strncpy(theKey, thePasswordCopy, kKeyLength);
			} 
			else if (theSaltMessage->encryptionType() == SaltMessage::kBraindeadSimpleEncryption)
			{
				for(size_t i = 0; i < sizeof(theKey); i++)
				{
					theKey[i] = thePasswordCopy[i] ^ (theSaltMessage->salt()[i]);
				}
			
				for(size_t i = 1; i < sizeof(theKey); i++)
				{
					theKey[i] = theKey[i]^theKey[i - 1];
				}
			
				for(size_t i = 1; i < sizeof(theKey); i++) {
					short value;
				
					value = ~( theKey[i]*theKey[i-1]);
					theKey[i] = (unsigned char) value;
				}
			}
			else if (theSaltMessage->encryptionType() == SaltMessage::kHTTPSEncryption)
			{
				HTTPClient conn;
				HTTPClient::parameter_map params;
				params["username"] = userName;
				params["password"] = userPassword;
				params["salt"] = std::string(reinterpret_cast<const char *>(theSaltMessage->salt()));
				
				if (conn.Post(A1_METASERVER_LOGIN_URL, params))
				{
					strncpy(theKey, conn.Response().c_str(), sizeof(theKey));
				}
				else
				{
					throw ServerConnectException("HTTPS Connection Failed");
				}
			}
			else
			{
				throw ServerConnectException("Unsupported Encryption");
			}

			BigChunkOfDataMessage theKeyMessage(kCLIENT_KEY, (uint8 *) theKey, sizeof(theKey));
			m_channel->enqueueOutgoingMessage(theKeyMessage);

			std::unique_ptr<Message> theResponseMessage(m_channel->receiveMessage());
			if (!theResponseMessage.get())
			{
				throw ServerConnectException("Server Disconnected");
			}
			else if (dynamic_cast<DenialMessage*>(theResponseMessage.get()))
			{
				DenialMessage *dm = dynamic_cast<DenialMessage*>(theResponseMessage.get());
				throw LoginDeniedException(dm->code(), dm->message());
			}
			else if (!dynamic_cast<AcceptMessage*>(theResponseMessage.get()))
			{
				throw CommunicationsChannel::FailedToReceiveSpecificMessageException();
			}
		}
		else if (dynamic_cast<AcceptMessage*>(theSaltOrAcceptMessage.get()) == 0)
		{
			if (dynamic_cast<DenialMessage*>(theSaltOrAcceptMessage.get()))
			{
				DenialMessage *dm = dynamic_cast<DenialMessage*>(theSaltOrAcceptMessage.get());
				throw LoginDeniedException(dm->code(), dm->message());
			}
			else
			{
				throw CommunicationsChannel::FailedToReceiveSpecificMessageException();
			}
		}

		m_channel->enqueueOutgoingMessage(LocalizationMessage());

		std::unique_ptr<LoginSuccessfulMessage> theLoginSuccessfulMessage(m_channel->receiveSpecificMessageOrThrow<LoginSuccessfulMessage>());

//	std::unique_ptr<SetPlayerDataMessage> theSetPlayerDataMessage(m_channel->receiveSpecificMessageOrThrow<SetPlayerDataMessage>());

		std::unique_ptr<RoomListMessage> theRoomListMessage(m_channel->receiveSpecificMessageOrThrow<RoomListMessage>());
		m_dispatcher.get()->handle(theRoomListMessage.get(), m_channel.get());

		m_channel->disconnect();

		///// ROOM CONNECTION

		if (!m_rooms.size())
		{
			throw ServerConnectException("No Rooms Available");
		}

		IPaddress roomServerAddress = m_rooms[0].roomServerAddress();
	
		for (int i = 0; i < m_rooms.size(); i++) 
		{
			if (m_rooms[i].roomName() == "Arrival")
			{
				roomServerAddress = m_rooms[i].roomServerAddress();
				break;
			}
		}

//	roomServerAddress.host = 0xC0A80108;
//	roomServerAddress.host = 0x0801A8C0;
	
		m_channel->connect(roomServerAddress);

		m_channel->enqueueOutgoingMessage(RoomLoginMessage(userName, theLoginSuccessfulMessage->token()));

		m_channel->enqueueOutgoingMessage(NameAndTeamMessage(m_playerName, m_teamName));

		std::unique_ptr<IDAndLimitMessage> theIDAndLimitMessage(m_channel->receiveSpecificMessageOrThrow<IDAndLimitMessage>());
		m_playerID = theIDAndLimitMessage->playerID();

		std::unique_ptr<DenialMessage> theRoomAcceptMessage(m_channel->receiveSpecificMessageOrThrow<DenialMessage>());

		m_channel->setMessageHandler(m_dispatcher.get());
	} 
	catch (const CommunicationsChannel::FailedToReceiveSpecificMessageException&)
	{
		// translate for caller
		throw ServerConnectException("Unexpected Response");
	}
}



bool
MetaserverClient::isConnected() const
{
	return m_channel->isConnected();
}



void
MetaserverClient::disconnect()
{
	// don't throw, we're called in ~MetaserverClient
	try {
		m_channel->enqueueOutgoingMessage(LogoutMessage());
		m_channel->pump();
	}
	catch(...) { }
	m_channel->disconnect();
}



void
MetaserverClient::pump()
{
	m_channel->pump();
	m_channel->dispatchIncomingMessages();	
	if (!m_channel->isConnected() && !m_notifiedOfDisconnected && m_notificationAdapter)
	{
		m_notifiedOfDisconnected = true;
		m_notificationAdapter->roomDisconnected();
	}
}




void
MetaserverClient::pumpAll()
{
	for_each(s_instances.begin(), s_instances.end(), mem_fun(&MetaserverClient::pump));
}




void
MetaserverClient::sendChatMessage(const std::string& message)
{
	if (message == ".available" || message == ".avail") {
		if(m_notificationAdapter) {
			string players = "Available Players: ";
			bool found_players = false;
			for (size_t i = 0; i < playersInRoom().size(); i++) {
				if (!playersInRoom()[i].away()) {
					if (found_players)
						players += ", ";
					found_players = true;
					players += "\"" + playersInRoom()[i].name() + "\"";
				}
			}
			if (!found_players)
				players += "none";
			m_notificationAdapter->receivedLocalMessage(players);
		}
	} else if (message == ".who") {
		if(m_notificationAdapter) {
			string players = "Players: ";
			if (playersInRoom().size()) {
				players += "\"" + playersInRoom()[0].name() + "\"";
				for (size_t i = 1; i < playersInRoom().size(); i++) {
					players += ", \"" + playersInRoom()[i].name() + "\"";
				}
			} else {
				players += "none";
			}
			m_notificationAdapter->receivedLocalMessage(players); 	
		}
	} else if (message.compare(0, strlen(".pork"), ".pork") == 0) {
		if (m_notificationAdapter) {
			m_notificationAdapter->receivedLocalMessage("NO BACON FOR YOU");
		}
	} else if (message == ".ignore") {
		if (m_notificationAdapter) {
			// list the players on the ignore list
			string players = "Ignoring: ";
			for (set<string>::iterator it = s_ignoreNames.begin(); it != s_ignoreNames.end(); it++)
			{
				if (it == s_ignoreNames.begin())
				{
					players += "\"" + *it + "\"";
				} 
				else
				{
					players += ", \"" + *it +"\"";
				}
			}
			
			m_notificationAdapter->receivedLocalMessage(players);
		}
	} else if (message.compare(0, strlen(".ignore "), ".ignore ") == 0) {
		// everything after the space is the name to ignore
		string name = message.substr(strlen(".ignore "));
		ignore(name);
	} else if (message == ".games") {
		if (m_notificationAdapter) {
			m_notificationAdapter->receivedLocalMessage("Games:");
			vector<GameListMessage::GameListEntry> games = gamesInRoom();
			vector<MetaserverPlayerInfo> players = playersInRoom();
			for (vector<GameListMessage::GameListEntry>::iterator it = games.begin(); it != games.end(); ++it)
			{
				// look up the player name
 				string player_name;
				for (vector<MetaserverPlayerInfo>::iterator player_it = players.begin(); player_it != players.end(); ++player_it)
 				{
 					if (player_it->id() == it->m_hostPlayerID) 
 					{
						player_name = player_it->name();
						break;
 					}
 				}

 				if (player_name.size() > 0) {
 					m_notificationAdapter->receivedLocalMessage(it->format_for_chat(player_name));
				}
			}
		}
	} else {
		m_channel->enqueueOutgoingMessage(ChatMessage(m_playerID, m_playerName, message));
	}
}

void
MetaserverClient::sendPrivateMessage(MetaserverPlayerInfo::IDType id, const std::string& message)
{
	m_channel->enqueueOutgoingMessage(PrivateMessage(m_playerID, m_playerName, id, message));
}

void
MetaserverClient::announceGame(uint16 gamePort, const GameDescription& description)
{
	m_channel->enqueueOutgoingMessage(CreateGameMessage(gamePort, description));
	m_gameDescription = description;
	m_gamePort = gamePort;
	m_gameAnnounced = true;
}

void
MetaserverClient::announcePlayersInGame(uint8 players)
{
	m_gameDescription.m_numPlayers = players;
	m_channel->enqueueOutgoingMessage(CreateGameMessage(m_gamePort, m_gameDescription));
}

void
MetaserverClient::announceGameStarted(int32 gameTimeInSeconds)
{
	m_gameDescription.m_closed = true;
	m_gameDescription.m_running = true;
	m_channel->enqueueOutgoingMessage(CreateGameMessage(m_gamePort, m_gameDescription));
	m_channel->enqueueOutgoingMessage(StartGameMessage(gameTimeInSeconds == INT32_MAX ? INT32_MIN : gameTimeInSeconds));
}



void
MetaserverClient::announceGameReset()
{
	m_channel->enqueueOutgoingMessage(ResetGameMessage());
}



void
MetaserverClient::announceGameDeleted()
{
	if (m_gameAnnounced)
	{
		m_channel->enqueueOutgoingMessage(RemoveGameMessage());
		m_gameAnnounced = false;
	}
}

static 	inline bool style_code(char c)
{
	switch(tolower(c)) {
	case 'p':
	case 'b':
	case 'i':
	case 'l':
	case 'r':
	case 'c':
	case 's':
		return true;
	default:
		return false;
	}
}

static std::string remove_formatting(const std::string &s)
{
	string temp;
	int i = 0;
	while (i < s.size()) {
		if (s[i] == '|' && style_code(s[i + 1]))
			i += 2;
		else
			temp += s[i++];
	}

	return temp;
}

void MetaserverClient::ignore(const std::string& name)
{
	std::string cleaned_name = remove_formatting(name);
	if (s_ignoreNames.find(cleaned_name) != s_ignoreNames.end())
	{
		s_ignoreNames.erase(cleaned_name);
		if (m_notificationAdapter) {
			m_notificationAdapter->receivedLocalMessage("Removing \"" + cleaned_name + "\" from the ignore list");
		}
	} else {
		s_ignoreNames.insert(cleaned_name);
		if (m_notificationAdapter) {
			m_notificationAdapter->receivedLocalMessage("Adding \"" + cleaned_name + "\" to the ignore list");
		}
	}
}

void MetaserverClient::ignore(MetaserverPlayerInfo::IDType id)
{
	// find the guy's name, remove the \266 if it's there
	const MetaserverPlayerInfo *player = m_playersInRoom.find(id);
	if (player)
	{
		std::string name = player->name();
		if (name[0] == '\260') name.erase(name.begin());

		ignore(name);
	}
}

bool MetaserverClient::is_ignored(MetaserverPlayerInfo::IDType id)
{
	std::string name;
	const MetaserverPlayerInfo *player = m_playersInRoom.find(id);
	if (player)
	{
		std::string name = player->name();
		if (name[0] == '\260') name.erase(name.begin());

		return (s_ignoreNames.find(remove_formatting(name)) != s_ignoreNames.end());
	}
	else
	{
		// uh, what do we do?
		return false;
	}
}


void
MetaserverClient::syncGames()
{
	m_channel->enqueueOutgoingMessage(SyncGamesMessage());
}


void MetaserverClient::setAway(bool away, const std::string& away_message)
{
	m_channel->enqueueOutgoingMessage(NameAndTeamMessage(m_playerName, m_teamName, away, away_message));
}

void MetaserverClient::setMode(uint16 mode, const std::string& session_id)
{
	m_channel->enqueueOutgoingMessage(SetPlayerModeMessage(mode, session_id));
}

void
MetaserverClient::setPlayerName(const std::string& name)
{
	m_playerName = name;
}



void
MetaserverClient::setPlayerTeamName(const std::string& name)
{
	m_teamName = name;
}



MetaserverClient::~MetaserverClient()
{
	disconnect();
	s_instances.erase(this);
}

#endif // !defined(DISABLE_NETWORKING)
