/*
 *  network_metaserver.cpp - Metaserver client

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

#if !defined(DISABLE_NETWORKING)

#include "cseries.h"

#include "network_metaserver.h"

#include "metaserver_messages.h"

#include "Message.h"
#include "MessageHandler.h"
#include "MessageDispatcher.h"
#include "MessageInflater.h"
#include "CommunicationsChannel.h"

#include <string>
#include <iostream>
#include <iterator> // ostream_iterator
#include <algorithm>
#include "Logging.h"



using namespace std;


set<MetaserverClient*> MetaserverClient::s_instances;
set<string> MetaserverClient::s_ignoreNames;


static const int kKeyLength = 16;


void
MetaserverClient::handleUnexpectedMessage(Message* inMessage, CommunicationsChannel* inChannel)
{
	logAnomaly1("Metaserver received message ID %i", inMessage->type());
	if(inMessage->type() == UninflatedMessage::kTypeID)
	{
		UninflatedMessage* theMessage = dynamic_cast<UninflatedMessage*>(inMessage);
		if(theMessage != NULL)
			logAnomaly2("-- internal ID %i, length %i", theMessage->inflatedType(), theMessage->length());
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
	if(m_notificationAdapter)
	{
		std::string realSenderName = message->senderName();
		if (realSenderName[0] == '\260') realSenderName.erase(realSenderName.begin());
		if (s_ignoreNames.find(realSenderName) != s_ignoreNames.end()) 
			return;

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
	m_playersInRoom.processUpdates(inMessage->players());

	if(m_notificationAdapter) {
		m_notificationAdapter->playersInRoomChanged(inMessage->players());
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
	m_gamesInRoom.processUpdates(inMessage->entries());

	if(m_notificationAdapter) {
		m_notificationAdapter->gamesInRoomChanged(inMessage->entries());
	}
};


MetaserverClient::MetaserverClient()
	: m_channel(new CommunicationsChannel())
	, m_inflater(new MessageInflater())
	, m_dispatcher(new MessageDispatcher())
	, m_notificationAdapter(NULL)
{
	m_unexpectedMessageHandler.reset(newMessageHandlerMethod(this, &MetaserverClient::handleUnexpectedMessage));
	m_broadcastMessageHandler.reset(newMessageHandlerMethod(this, &MetaserverClient::handleBroadcastMessage));
	m_chatMessageHandler.reset(newMessageHandlerMethod(this, &MetaserverClient::handleChatMessage));
	m_keepAliveMessageHandler.reset(newMessageHandlerMethod(this, &MetaserverClient::handleKeepAliveMessage));
	m_playerListMessageHandler.reset(newMessageHandlerMethod(this, &MetaserverClient::handlePlayerListMessage));
	m_roomListMessageHandler.reset(newMessageHandlerMethod(this, &MetaserverClient::handleRoomListMessage));
	m_gameListMessageHandler.reset(newMessageHandlerMethod(this, &MetaserverClient::handleGameListMessage));

	m_inflater->learnPrototype(SaltMessage());
	m_inflater->learnPrototype(AcceptMessage());
	m_inflater->learnPrototype(RoomListMessage());
	m_inflater->learnPrototype(IDAndLimitMessage());
	m_inflater->learnPrototype(DenialMessage());
	m_inflater->learnPrototype(BroadcastMessage());
	m_inflater->learnPrototype(ChatMessage());
	m_inflater->learnPrototype(KeepAliveMessage());
	m_inflater->learnPrototype(PlayerListMessage());
	m_inflater->learnPrototype(LoginSuccessfulMessage());
	m_inflater->learnPrototype(SetPlayerDataMessage());
	m_inflater->learnPrototype(GameListMessage());

	m_channel->setMessageInflater(m_inflater.get());

	m_dispatcher->setDefaultHandler(m_unexpectedMessageHandler.get());
	m_dispatcher->setHandlerForType(m_broadcastMessageHandler.get(), BroadcastMessage::kType);
	m_dispatcher->setHandlerForType(m_chatMessageHandler.get(), ChatMessage::kType);
	m_dispatcher->setHandlerForType(m_keepAliveMessageHandler.get(), KeepAliveMessage::kType);
	m_dispatcher->setHandlerForType(m_playerListMessageHandler.get(), PlayerListMessage::kType);
	m_dispatcher->setHandlerForType(m_roomListMessageHandler.get(), RoomListMessage::kType);
	m_dispatcher->setHandlerForType(m_gameListMessageHandler.get(), GameListMessage::kType);

	s_instances.insert(this);
	s_ignoreNames.insert("Bacon");
}


void
MetaserverClient::connect(const std::string& serverName, uint16 port, const std::string& userName, const std::string& userPassword)
{
	m_channel->setMessageHandler(m_unexpectedMessageHandler.get());

	m_channel->connect(serverName.c_str(), port);

	LoginAndPlayerInfoMessage theLoginMessage(userName, m_playerName, m_teamName);
	m_channel->enqueueOutgoingMessage(theLoginMessage);
	
	auto_ptr<Message> theSaltOrAcceptMessage(m_channel->receiveMessage());
	if (theSaltOrAcceptMessage.get() == 0)
		throw 0;
	
	if (dynamic_cast<SaltMessage*>(theSaltOrAcceptMessage.get()) != 0)
	{
		SaltMessage* theSaltMessage = dynamic_cast<SaltMessage*>(theSaltOrAcceptMessage.get());

		uint8 theKey[kKeyLength];
		uint8 thePasswordCopy[kKeyLength];
		strncpy(reinterpret_cast<char*>(thePasswordCopy), userPassword.c_str(), kKeyLength);
		memcpy(theKey, theSaltMessage->salt(), sizeof(theKey));
		for(size_t i = 0; i < sizeof(theKey); i++)
			theKey[i] ^= thePasswordCopy[i];

		BigChunkOfDataMessage theKeyMessage(kCLIENT_KEY, theKey, sizeof(theKey));
		m_channel->enqueueOutgoingMessage(theKeyMessage);

		auto_ptr<AcceptMessage> theAcceptMessage(m_channel->receiveSpecificMessageOrThrow<AcceptMessage>());
	}
	else if (dynamic_cast<AcceptMessage*>(theSaltOrAcceptMessage.get()) == 0)
		throw 0;

	m_channel->enqueueOutgoingMessage(LocalizationMessage());

	auto_ptr<LoginSuccessfulMessage> theLoginSuccessfulMessage(m_channel->receiveSpecificMessageOrThrow<LoginSuccessfulMessage>());

//	auto_ptr<SetPlayerDataMessage> theSetPlayerDataMessage(m_channel->receiveSpecificMessageOrThrow<SetPlayerDataMessage>());

	auto_ptr<RoomListMessage> theRoomListMessage(m_channel->receiveSpecificMessageOrThrow<RoomListMessage>());
	m_dispatcher.get()->handle(theRoomListMessage.get(), m_channel.get());

	m_channel->disconnect();

	///// ROOM CONNECTION
	
	IPaddress roomServerAddress = m_rooms[0].roomServerAddress();
	
	for (int i = 0; i < m_rooms.size(); i++) 
	{
		if (m_rooms[i].roomName() == "Arrival")
		{
			roomServerAddress = m_rooms[i].roomServerAddress();
			break;
		}
	}
	
	m_channel->connect(roomServerAddress);

	m_channel->enqueueOutgoingMessage(RoomLoginMessage(userName, theLoginSuccessfulMessage->token()));

	m_channel->enqueueOutgoingMessage(NameAndTeamMessage(m_playerName, m_teamName));

	auto_ptr<IDAndLimitMessage> theIDAndLimitMessage(m_channel->receiveSpecificMessageOrThrow<IDAndLimitMessage>());
	m_playerID = theIDAndLimitMessage->playerID();

	auto_ptr<DenialMessage> theRoomAcceptMessage(m_channel->receiveSpecificMessageOrThrow<DenialMessage>());

	m_channel->setMessageHandler(m_dispatcher.get());
}



bool
MetaserverClient::isConnected() const
{
	return m_channel->isConnected();
}



void
MetaserverClient::disconnect()
{
	m_channel->disconnect();
}



void
MetaserverClient::pump()
{
	m_channel->pump();
	m_channel->dispatchIncomingMessages();	
}




void
MetaserverClient::pumpAll()
{
	for_each(s_instances.begin(), s_instances.end(), mem_fun(&MetaserverClient::pump));
}




void
MetaserverClient::sendChatMessage(const std::string& message)
{
	if (message == ".who") {
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
	} else {
		m_channel->enqueueOutgoingMessage(ChatMessage(m_playerID, m_playerName, message));
	}
}

void
MetaserverClient::announceGame(uint16 gamePort, const GameDescription& description)
{
	m_channel->enqueueOutgoingMessage(CreateGameMessage(gamePort, description));
}



void
MetaserverClient::announceGameStarted(int32 gameTimeInSeconds)
{
	m_channel->enqueueOutgoingMessage(StartGameMessage(gameTimeInSeconds));
}



void
MetaserverClient::announceGameReset()
{
	m_channel->enqueueOutgoingMessage(ResetGameMessage());
}



void
MetaserverClient::announceGameDeleted()
{
	m_channel->enqueueOutgoingMessage(RemoveGameMessage());
}

void MetaserverClient::ignore(const std::string& name)
{
	if (s_ignoreNames.find(name) != s_ignoreNames.end())
	{
		s_ignoreNames.erase(name);
		if (m_notificationAdapter) {
			m_notificationAdapter->receivedLocalMessage("Removing \"" + name + "\" from the ignore list");
		}
	} else {
		s_ignoreNames.insert(name);
		if (m_notificationAdapter) {
			m_notificationAdapter->receivedLocalMessage("Adding \"" + name + "\" to the ignore list");
		}
	}
}


void
MetaserverClient::syncGames()
{
	m_channel->enqueueOutgoingMessage(SyncGamesMessage());
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
