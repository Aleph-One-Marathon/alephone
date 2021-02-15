/*
 *  metaserver_messages.h - TCPMess message types for metaserver client

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

#ifndef METASERVER_MESSAGES_H
#define METASERVER_MESSAGES_H

#include "Message.h"

#include "AStream.h"
#include "SDL_net.h"
#include "Scenario.h" // for scenario name and ID
#include "network.h" // for network protocol ID

#include <string>
#include <vector>
#include <boost/algorithm/string/case_conv.hpp>

using boost::algorithm::to_lower_copy;

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
	kCLIENT_LOGOUT          = 102,
	kCLIENT_NAME_TEAM	= 103,
	kCLIENT_CREATEGAME	= 104,
	kCLIENT_REMOVEGAME	= 105,
	kCLIENT_PLAYERMODE      = 107,
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
	
	// future versions will take action on these:
	std::string m_scenarioID;
	std::string m_networkSetupProtocolID;
	
	// while these are purely for display purposes
	std::string m_scenarioName;
	std::string m_scenarioVersion;
	std::string m_alephoneBuildString;
	std::string m_netScript;

	// more stuff
	bool m_hasGameOptions;
	int16 m_gameOptions;
	int16 m_cheatFlags;
	int16 m_killLimit;
	std::string m_mapFileName;
	std::string m_physicsName;
	
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
		, m_scenarioID(Scenario::instance()->GetID())
		, m_networkSetupProtocolID(kNetworkSetupProtocolID)
		, m_scenarioName(Scenario::instance()->GetName())
		, m_scenarioVersion(Scenario::instance()->GetVersion())
		, m_hasGameOptions(false)
		, m_gameOptions(0)
		, m_cheatFlags(0)
		, m_killLimit(0)
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

	LoginAndPlayerInfoMessage* clone() const
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

typedef DatalessMessage<kCLIENT_LOGOUT> LogoutMessage;

class SetPlayerModeMessage : public SmallMessageHelper
{
public:
	enum { kType = kCLIENT_PLAYERMODE };

	MessageTypeID type() const { return kType; }

	SetPlayerModeMessage* clone() const
		{ return new SetPlayerModeMessage(*this); }

	SetPlayerModeMessage(uint16 mode, const std::string& session_id) : m_mode(mode), m_session_id(session_id) { }

protected:
	void reallyDeflateTo(AOStream& thePacket) const
		{
			thePacket << m_mode;
			thePacket.write(const_cast<char*>(m_session_id.c_str()), m_session_id.size());
		}

	bool reallyInflateFrom(AIStream& inStream)
	{ 
		assert(false);
		return false;
	}

private:
	uint16 m_mode;
	std::string m_session_id;
};

class SaltMessage : public SmallMessageHelper
{
public:
	enum { kType = kSERVER_SALT, kKeyLength = 16 };

	enum { kPlaintextEncryption, kBraindeadSimpleEncryption, kHTTPSEncryption = 4 };

	uint16 encryptionType() const { return m_encryptionType; }
	const uint8* salt() const { return &(m_salt[0]); }

	MessageTypeID type() const { return kType; }

	SaltMessage* clone() const
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

	LocalizationMessage* clone() const
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

	RoomListMessage* clone() const
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

	RoomLoginMessage* clone() const
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

	NameAndTeamMessage* clone() const
	{ return new NameAndTeamMessage(*this); }

	NameAndTeamMessage(const std::string& name, const std::string& team) : m_name(name), m_team(team), m_away(false), m_away_message("") {}

	NameAndTeamMessage(const std::string& name, const std::string& team, bool away, const std::string& away_message) : m_name(name), m_team(team), m_away(away), m_away_message(away_message) { }

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

	bool            m_away;
	std::string     m_away_message;
};

class IDAndLimitMessage : public SmallMessageHelper
{
public:
	enum { kType = kSERVER_LIMIT };

	uint32 playerID() const { return m_playerID; }
	uint16 playerLimit() const { return m_playerLimit; }

	MessageTypeID type() const { return kType; }

	IDAndLimitMessage* clone() const
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

	DenialMessage* clone() const
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

	BroadcastMessage* clone() const
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

class PrivateMessage : public SmallMessageHelper
{
public:
	enum { kType = kBOTH_PRIVATE_MESSAGE };
	static const int kDirectedBit = 0x1;
	
	MessageTypeID type() const { return kType; }

	PrivateMessage() {}

	PrivateMessage(uint32 inSenderID, const std::string& inSenderName, uint32 inSelectedID, const std::string& inMessage);

	const uint32 senderID() const { return m_senderID; }
	const uint32 selectedID() const { return m_selectedID; }
	const uint16 internalType() const { return m_internalType; }
	const std::string senderName() const { return m_senderName; }
	const std::string message() const { return m_message; }
	const bool directed() const { return m_flags & kDirectedBit; }
	
	PrivateMessage* clone() const
		{ return new PrivateMessage(*this); }

protected:
	void reallyDeflateTo(AOStream& thePacket) const;
	bool reallyInflateFrom(AIStream& inStream);

private:
	uint16 m_color[3] = {};
	uint32 m_senderID = 0;
	uint32 m_selectedID = 0;
	uint16 m_internalType = 0;
	uint16 m_flags = 0;
	std::string m_senderName;
	std::string m_message;
};
	

class ChatMessage : public SmallMessageHelper
{
public:
	enum { kType = kBOTH_CHAT };

	static const int kDirectedBit = 0x1;

	MessageTypeID type() const { return kType; }

	ChatMessage() {}

	ChatMessage(uint32 inSenderID, const std::string& inSenderName, const std::string& inMessage);

	const uint32 senderID() const { return m_senderID; }
	const uint16 internalType() const { return m_internalType; }
	const std::string senderName() const { return m_senderName; }
	const std::string message() const { return m_message; }
	const bool directed() const { return m_flags & kDirectedBit; }

	ChatMessage* clone() const
	{ return new ChatMessage(*this); }

protected:
	void reallyDeflateTo(AOStream& thePacket) const;
	bool reallyInflateFrom(AIStream& inStream);

private:
	uint16 m_color[3] = {};
	uint32 m_senderID = 0;
	uint16 m_internalType = 0;
	uint16 m_flags = 0;
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

	static bool sort(const MetaserverPlayerInfo& a, const MetaserverPlayerInfo& b) { 
		return (a.m_adminFlags == b.m_adminFlags) ? ( (a.m_status == b.m_status) ? a.playerID() < b.playerID() : a.m_status < b.m_status) : a.m_adminFlags > b.m_adminFlags;
	}

	friend std::ostream& operator <<(std::ostream& out, const MetaserverPlayerInfo& info);

	bool away() const { return m_status & 0x1; }

	// Conformance to metaserver-maintained-list interface
	typedef uint32 IDType;
	static const IDType IdNone = 0xffffffff;

	IDType id() const { return playerID(); }
	
	bool target() const { return m_target; }
	void target(bool _target) { m_target = _target; } 

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

	bool m_target;
};

class PlayerListMessage : public SmallMessageHelper
{
public:
	enum { kType = kSERVER_PLAYERLIST };
	
	MessageTypeID type() const { return kType; }
	
	PlayerListMessage() {}
	
	PlayerListMessage* clone() const
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

	CreateGameMessage* clone() const
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

	LoginSuccessfulMessage* clone() const
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

	SetPlayerDataMessage* clone() const
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
		static const IDType IdNone = 0xffffffff;
		IDType id() const { return m_gameID; }
		uint8 verb() const { return m_verb; }

		bool target() const { return m_target; }
		void target(bool _target) { m_target = _target; }

		bool running() const { return m_description.m_running; }
		bool compatible() const { return Scenario::instance()->IsCompatible(m_description.m_scenarioID); }

		static bool sort(const GameListEntry& a, const GameListEntry& b) {
			// sort compatible/not running, then compatible/running
			// then not compatible
			// in those groups, sort by ID
			if (a.compatible() && b.compatible())
			{
				if (a.running() == b.running())
				{
					return a.id() < b.id();
				}
				else 
				{
					return !a.running();
				}
			}
			else if (a.compatible() == b.compatible())
			{
				return a.id() < b.id();
			}
			else
				return a.compatible();
		}

		int minutes_remaining() const {
			if (m_timeRemaining == -1) return -1;
			int remaining = m_timeRemaining / 60 - (machine_tick_count() - m_ticks) / 1000 / 60;
			if (remaining < 0) remaining = 0;
			return remaining;
		}

		std::string format_for_chat(const std::string& player_name) const;
		std::string game_string() const;

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

		uint32          m_ticks; // SDL ticks at last update
		std::string     m_hostPlayerName;

	GameListEntry() : m_target(false) { }
		bool            m_target;
	};
	
	enum { kType = kSERVER_GAMELIST };

	MessageTypeID type() const { return kType; }

	GameListMessage* clone() const
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

	StartGameMessage* clone() const
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
