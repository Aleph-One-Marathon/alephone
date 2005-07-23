/*
 *  metaserver_messages.h - TCPMess message types for metaserver client

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

#ifndef METASERVER_MESSAGES_H
#define METASERVER_MESSAGES_H

#include "Message.h"

#include "AStream.h"
#include "SDL_net.h"

#include <string>
#include <vector>

enum
{
	// Message types sent from servers to clients
	kSERVER_ROOMLIST	= 0,
	kSERVER_PLAYERLIST	= 1,
	kSERVER_GAMELIST	= 2,
	kSERVER_DENY		= 3,
	kSERVER_SALT		= 6,
	kSERVER_LOGINSUCCESS	= 7,
	kSERVER_SETPLAYERDATA	= 8,
	kSERVER_LIMIT		= 9,
	kSERVER_BROADCAST	= 10,
	kSERVER_ACCEPT		= 12,
	kSERVER_FIND		= 14,
	kSERVER_STATS		= 17,

	// Messages sent from clients to servers
	kCLIENT_LOGIN		= 100,
	kCLIENT_ROOM_LOGIN	= 101,
	kCLIENT_NAME_TEAM	= 103,
	kCLIENT_CREATEGAME	= 104,
	kCLIENT_REMOVEGAME	= 105,
	kCLIENT_KEY		= 109,
	kCLIENT_SYNCGAMES	= 110,
        kCLIENT_GAMEPLAYERLIST	= 111,
        kCLIENT_GAMESCORE	= 112,
	kCLIENT_RESETGAME	= 113,
	kCLIENT_STARTGAME	= 114,
	kCLIENT_LOCALIZE	= 115,
	kCLIENT_FIND		= 117,
	kCLIENT_STATS		= 121,
	
	// Message types sent in both directions
	kBOTH_CHAT		= 200,
	kBOTH_PRIVATE_MESSAGE	= 201,
	kBOTH_KEEP_ALIVE	= 202,
};



typedef uint8 HandoffToken[32];

AIStream& operator >>(AIStream& stream, HandoffToken& token);
// Damned if I can convince the linker this operator exists . . .
//AOStream& operator <<(AOStream& stream, const HandoffToken& token);



struct GameDescription
{
	uint16		m_type;
	int32		m_timeLimit;
	uint32		m_mapChecksum;
	uint16		m_difficulty;
	uint16		m_maxPlayers;
	bool		m_teamsAllowed;
	bool		m_closed;
	bool		m_running;
	uint8		m_numPlayers;
	std::string	m_name;
	std::string	m_mapName;

	GameDescription()
		: m_type(0)
		, m_timeLimit(0)
		, m_mapChecksum(0)
		, m_difficulty(0)
		, m_maxPlayers(8)
		, m_teamsAllowed(false)
		, m_closed(false)
		, m_running(false)
		, m_numPlayers(1)
		, m_name("Untitled Game")
		, m_mapName("Unspecified Map")
	{}
};

AIStream& operator >>(AIStream& stream, GameDescription& desc);
AOStream& operator <<(AOStream& stream, const GameDescription& desc);
std::ostream& operator <<(std::ostream& stream, const GameDescription& desc);



class LoginAndPlayerInfoMessage : public SmallMessageHelper
{
public:
	LoginAndPlayerInfoMessage(const std::string& userName, const std::string& playerName, const std::string& teamName)
		: m_userName(userName), m_playerName(playerName), m_teamName(teamName) {}
	
	enum { kType = kCLIENT_LOGIN };

	MessageTypeID type() const { return kType; }

	COVARIANT_RETURN(Message*, LoginAndPlayerInfoMessage*) clone() const
	{ return new LoginAndPlayerInfoMessage(*this); }

protected:
	void reallyDeflateTo(AOStream& thePacket) const;

	bool reallyInflateFrom(AIStream& inStream)
	{
		// no support for receiving this type of message
		return false;
	}

private:
	std::string	m_userName;
	std::string	m_playerName;
	std::string	m_teamName;
};



class SaltMessage : public SmallMessageHelper
{
public:
	enum { kType = kSERVER_SALT, kKeyLength = 16 };

	uint16 encryptionType() const { return m_encryptionType; }
	const uint8* salt() const { return &(m_salt[0]); }

	MessageTypeID type() const { return kType; }

	COVARIANT_RETURN(Message*, SaltMessage*) clone() const
	{ return new SaltMessage(*this); }

protected:
	void reallyDeflateTo(AOStream& thePacket) const
	{
		// no need for deflation
		assert(false);
	}

	bool reallyInflateFrom(AIStream& inStream);

private:
	uint16	m_encryptionType;
	uint8	m_salt[kKeyLength];
};



typedef DatalessMessage<kSERVER_ACCEPT> AcceptMessage;



class LocalizationMessage : public SmallMessageHelper
{
public:
	enum { kType = kCLIENT_LOCALIZE };

	MessageTypeID type() const { return kType; }

	COVARIANT_RETURN(Message*, LocalizationMessage*) clone() const
	{ return new LocalizationMessage(*this); }

protected:
	void reallyDeflateTo(AOStream& thePacket) const;

	bool reallyInflateFrom(AIStream& inStream)
	{
		// no need for inflation
		return false;
	}
};



class RoomDescription
{
public:
	enum
	{
		kNormalRoomType = 0,
		kRankedRoomType,
		kTournamentRoomType
	};

	void read(AIStream& inStream);

	const std::string roomName() const;
	int playerCount() const { return m_playerCount; }
	int gameCount() const { return m_gameCount; }
	int roomType() const { return m_type; }
	const IPaddress roomServerAddress() const { return m_address; }

private:
	uint16 m_id;
	uint16 m_playerCount;
	uint16 m_gameCount;
	uint16 m_type;
	IPaddress m_address;
};

std::ostream& operator <<(std::ostream& out, const RoomDescription& roomDesc);

class RoomListMessage : public SmallMessageHelper
{
public:
	enum { kType = kSERVER_ROOMLIST };

	MessageTypeID type() const { return kType; }

	COVARIANT_RETURN(Message*, RoomListMessage*) clone() const
	{ return new RoomListMessage(*this); }

	const std::vector<RoomDescription>& rooms() const { return m_rooms; }

protected:
	void reallyDeflateTo(AOStream& thePacket) const
	{
		// no need for deflation
		assert(false);
	}

	bool reallyInflateFrom(AIStream& inStream);

private:
	std::vector<RoomDescription>	m_rooms;
};

std::ostream& operator <<(std::ostream& out, const RoomListMessage& message);



class RoomLoginMessage : public SmallMessageHelper
{
public:
	enum { kType = kCLIENT_ROOM_LOGIN, kKeyLength = 16 };

	MessageTypeID type() const { return kType; }

	COVARIANT_RETURN(Message*, RoomLoginMessage*) clone() const
	{ return new RoomLoginMessage(*this); }

	RoomLoginMessage(const std::string& login, const HandoffToken& token) : m_loginName(login)
	{
		memcpy(m_token, token, sizeof(m_token));
	}

protected:
	void reallyDeflateTo(AOStream& out) const;

	bool reallyInflateFrom(AIStream&)
	{
		// don't need to be able to receive these
		return false;
	}

private:
	HandoffToken	m_token;
	std::string	m_loginName;
};



class NameAndTeamMessage : public SmallMessageHelper
{
public:
	enum { kType = kCLIENT_NAME_TEAM };

	MessageTypeID type() const { return kType; }

	COVARIANT_RETURN(Message*, NameAndTeamMessage*) clone() const
	{ return new NameAndTeamMessage(*this); }

	NameAndTeamMessage(const std::string& name, const std::string& team) : m_name(name), m_team(team)
	{}

protected:
	void reallyDeflateTo(AOStream& out) const;

	bool reallyInflateFrom(AIStream&)
	{
		// don't need to be able to receive these
		return false;
	}

private:
	std::string	m_name;
	std::string	m_team;
};



class IDAndLimitMessage : public SmallMessageHelper
{
public:
	enum { kType = kSERVER_LIMIT };

	uint32 playerID() const { return m_playerID; }
	uint16 playerLimit() const { return m_playerLimit; }

	MessageTypeID type() const { return kType; }

	COVARIANT_RETURN(Message*, IDAndLimitMessage*) clone() const
	{ return new IDAndLimitMessage(*this); }

protected:
	void reallyDeflateTo(AOStream& thePacket) const
	{
		// no need for deflation
		assert(false);
	}

	bool reallyInflateFrom(AIStream& inStream);

private:
	uint32	m_playerID;
	uint16	m_playerLimit;
};



class DenialMessage : public SmallMessageHelper
{
public:
	enum { kType = kSERVER_DENY };

	MessageTypeID type() const { return kType; }

	uint32 code() const { return m_code; }
	const std::string message() const { return m_message; }

	COVARIANT_RETURN(Message*, DenialMessage*) clone() const
	{ return new DenialMessage(*this); }

protected:
	void reallyDeflateTo(AOStream& thePacket) const
	{
		// no need for deflation
		assert(false);
	}

	bool reallyInflateFrom(AIStream& inStream);

private:
	uint32		m_code;
	std::string	m_message;
};



class BroadcastMessage : public SmallMessageHelper
{
public:
	enum { kType = kSERVER_BROADCAST };

	MessageTypeID type() const { return kType; }

	const std::string message() const { return m_message; }

	COVARIANT_RETURN(Message*, BroadcastMessage*) clone() const
	{ return new BroadcastMessage(*this); }

protected:
	void reallyDeflateTo(AOStream& thePacket) const
	{
		// no need for deflation
		assert(false);
	}

	bool reallyInflateFrom(AIStream& inStream);

private:
	std::string m_message;
};



class ChatMessage : public SmallMessageHelper
{
public:
	enum { kType = kBOTH_CHAT };

	MessageTypeID type() const { return kType; }

	ChatMessage() {}

	ChatMessage(uint32 inSenderID, const std::string& inSenderName, const std::string& inMessage);

	const uint32 senderID() const { return m_senderID; }
	const std::string senderName() const { return m_senderName; }
	const std::string message() const { return m_message; }

	COVARIANT_RETURN(Message*, ChatMessage*) clone() const
	{ return new ChatMessage(*this); }

protected:
	void reallyDeflateTo(AOStream& thePacket) const;
	bool reallyInflateFrom(AIStream& inStream);

private:
	uint16		m_color[3];
	uint32		m_senderID;
	std::string	m_senderName;
	std::string	m_message;
};



typedef DatalessMessage<kBOTH_KEEP_ALIVE> KeepAliveMessage;



class MetaserverPlayerInfo
{
public:
	// Flags for adminFlags
	static const uint16	kNotAdmin	= 0x0;
	static const uint16	kBungie		= 0x1;
	static const uint16	kAdmin		= 0x4;
	
	MetaserverPlayerInfo(AIStream& fromStream);

	uint16 verb() const { return m_verb; }
	uint32 playerID() const { return m_playerID; }
	const std::string& name() const { return m_name; }

	const uint16 *color() const { return m_primaryColor; }
	const uint16 *team_color() const { return m_secondaryColor; }

	friend std::ostream& operator <<(std::ostream& out, const MetaserverPlayerInfo& info);

	// Conformance to metaserver-maintained-list interface
	typedef uint32 IDType;
	IDType id() const { return playerID(); }
	
private:
	MetaserverPlayerInfo();

	uint16		m_verb;
	uint16		m_adminFlags;
	uint32		m_ranking;
	uint32		m_playerID;
	uint32		m_roomID;
	uint16		m_rank;
	uint16		m_playerDataSize;
	uint8		m_icon;
	uint8		m_status;
	uint16		m_primaryColor[3];
	uint16		m_secondaryColor[3];
	std::string	m_name;
	std::string	m_team;
};

class PlayerListMessage : public SmallMessageHelper
{
public:
	enum { kType = kSERVER_PLAYERLIST };
	
	MessageTypeID type() const { return kType; }
	
	PlayerListMessage() {}
	
	COVARIANT_RETURN(Message*, PlayerListMessage*) clone() const
	{ return new PlayerListMessage(*this); }
	
	const std::vector<MetaserverPlayerInfo>& players() const { return m_players; }

	
protected:
	void reallyDeflateTo(AOStream& thePacket) const
	{
		// no need for deflation
		assert(false);
	}
	
	bool reallyInflateFrom(AIStream& inStream);
	
private:
	std::vector<MetaserverPlayerInfo>	m_players;
};

std::ostream& operator <<(std::ostream& out, const PlayerListMessage& info);



class CreateGameMessage : public SmallMessageHelper
{
public:
	enum { kType = kCLIENT_CREATEGAME };

	MessageTypeID type() const { return kType; }

	CreateGameMessage(uint16 gamePort, const GameDescription& description)
		: m_gamePort(gamePort)
		, m_description(description)
	{}

	COVARIANT_RETURN(Message*, CreateGameMessage*) clone() const
	{ return new CreateGameMessage(*this); }


protected:
	void reallyDeflateTo(AOStream& thePacket) const;

	bool reallyInflateFrom(AIStream& inStream)
	{
		// no need for inflation
		assert(false);
		return false;
	}

	
private:
	uint16		m_gamePort;
	GameDescription	m_description;
};



typedef DatalessMessage<kCLIENT_SYNCGAMES> SyncGamesMessage;



class LoginSuccessfulMessage : public SmallMessageHelper
{
public:
	enum { kType = kSERVER_LOGINSUCCESS };

	MessageTypeID type() const { return kType; }

	COVARIANT_RETURN(Message*, LoginSuccessfulMessage*) clone() const
	{ return new LoginSuccessfulMessage(*this); }

	uint32 userID() const { return m_userID; }
	const HandoffToken& token() const { return m_token; }

protected:
	void reallyDeflateTo(AOStream& thePacket) const
	{
		// no need for deflation
		assert(false);
	}

	bool reallyInflateFrom(AIStream& inStream);

private:
	uint32		m_userID;
	uint16		m_order;
	HandoffToken	m_token;
};



class SetPlayerDataMessage : public SmallMessageHelper
{
public:
	enum { kType = kSERVER_SETPLAYERDATA };

	MessageTypeID type() const { return kType; }

	COVARIANT_RETURN(Message*, SetPlayerDataMessage*) clone() const
	{ return new SetPlayerDataMessage(*this); }


protected:
	void reallyDeflateTo(AOStream& thePacket) const
	{
		// no need for deflation
		assert(false);
	}

	bool reallyInflateFrom(AIStream& inStream);

private:
};



class GameListMessage : public SmallMessageHelper
{
public:
	struct GameListEntry
	{
		// Conformance to MetaserverMaintainedList's Element interface
		typedef uint32 IDType;
		IDType id() const { return m_gameID; }
		uint8 verb() const { return m_verb; }

		// Conformance to w_items_in_game<>'s Element interface
		const std::string& name() const { return m_description.m_name; }

		uint32		m_gameID;
		uint8		m_ipAddress[4];
		uint16		m_port;
		uint8		m_verb;
		uint8		m_gameEnable;
		uint32		m_timeRemaining;
		uint32		m_hostPlayerID;
		uint16		m_len;
		GameDescription	m_description;
	};
	
	enum { kType = kSERVER_GAMELIST };

	MessageTypeID type() const { return kType; }

	COVARIANT_RETURN(Message*, GameListMessage*) clone() const
	{ return new GameListMessage(*this); }

	const std::vector<GameListEntry>& entries() const { return m_entries; }

protected:
	void reallyDeflateTo(AOStream& thePacket) const
	{
		// no need for deflation
		assert(false);
	}

	bool reallyInflateFrom(AIStream& inStream);

private:
	std::vector<GameListEntry>	m_entries;
};

AIStream& operator >>(AIStream& stream, GameListMessage::GameListEntry& entry);
std::ostream& operator <<(std::ostream& stream, const GameListMessage::GameListEntry& entry);



typedef DatalessMessage<kCLIENT_RESETGAME>	ResetGameMessage;
typedef DatalessMessage<kCLIENT_REMOVEGAME>	RemoveGameMessage;



class StartGameMessage : public SmallMessageHelper
{
public:
	enum { kType = kCLIENT_STARTGAME };

	MessageTypeID type() const { return kType; }

	COVARIANT_RETURN(Message*, StartGameMessage*) clone() const
	{ return new StartGameMessage(*this); }

	StartGameMessage(int32 gameTimeInSeconds) : m_gameTimeInSeconds(gameTimeInSeconds) {}

protected:
	void reallyDeflateTo(AOStream& thePacket) const;

	bool reallyInflateFrom(AIStream& inStream)
	{
		// no need for inflation
		assert(false);
		return false;
	}
	
private:
	int32	m_gameTimeInSeconds;
};

#endif // METASERVER_MESSAGES_H
