/*
 *  metaserver_messages.cpp - TCPMess message types for metaserver client

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

#include "metaserver_messages.h"

#include "CommunicationsChannel.h"
#include "Message.h"
#include "MessageDispatcher.h"
#include "MessageHandler.h"
#include "MessageInflater.h"
#include <iostream>
#include "Logging.h"
#include "AStream.h"
#include <string>
#include <vector>
#include <iterator>
#include <sstream>

#include "preferences.h"
#include "shell.h" // get_player_color :(

#include "map.h" // TICKS_PER_SECOND

// game types
#include "network_dialogs.h"
#include "TextStrings.h"

#include <boost/algorithm/string/predicate.hpp>

using namespace std;

static const char* sRoomNames[] = {
	"Crows Bridge",
	"Otter Ferry",
	"White Falls",
	"Silvermines",
	"Shoal",
	"Madrigal",
	"Tyr",
	"Ash",
	"Scales",
	"Covenant",
	"Muirthemne",
	"Seven Gates",
	"Bagrada",
	"The Barrier",
	"Forest Heart",
	"The Ermine",
	"The Dire Marsh",
	"The Highlands",
	"The Drowned Kingdom",
	"The Great Devoid",
	"Shiver",
	"The Caterthuns",
	"Soulblighter",
	"Balor",
	"Sons of Myrgard",
	"Heart of the Stone",
	"Arrival",
	"Ingue Ferroque",
	"Vimy Ridge",
	"Stilwell Road"
};

const int kRoomNameCount = sizeof(sRoomNames) / sizeof(sRoomNames[0]);


static const char* kServiceName = "MARATHON";

enum
{
	kCRYPT_PLAINTEXT = 0,
	kCRYPT_SIMPLE = 1,
	kCRYPT_MD5 = 2,	// implemented by Mariusnet
	kCRYPT_ROOMSERVER = 3,	// another mystery
	kCRYPT_HTTPS = 4,

	kSTATE_AWAKE = 0,
	kSTATE_AWAY = 1,

	kPlatformIsMacintosh = 0,
	kPlatformIsWindows = 1,
	kPlatformIsOther = 4,

	kResetPlayerData = 1,

};

static const uint8 kPlayerIcon = 0;
static const uint16 kAlephOneClientVersion = 5000;

static void
write_padded_bytes(AOStream& inStream, const char* inBytes, size_t inByteCount, size_t inTargetLength)
{
	size_t theByteCountToWrite = min(inByteCount, inTargetLength);

	inStream.write(const_cast<char*>(inBytes), theByteCountToWrite);

	size_t theZeroCount = inTargetLength - theByteCountToWrite;

	for(size_t i = 0; i < theZeroCount; i++)
	{
		inStream << static_cast<uint8>('\0');
	}
}

static void
write_string(AOStream& inStream, const char* inString)
{
	inStream.write(const_cast<char*>(inString), strlen(inString) + 1);
}

static void
write_string(AOStream& inStream, const string& inString)
{
	write_string(inStream, inString.c_str());
}


static void
write_padded_string(AOStream& inStream, const char* inString, size_t inTargetLength)
{
	write_padded_bytes(inStream, inString, strlen(inString), inTargetLength);
}

static const string
read_string(AIStream& in)
{
	string result;
	int8 c;
	in >> c;
	while(c != '\0')
	{
		result += c;
		in >> c;
	}
	return result;
}

static const string
read_padded_string(AIStream& in, size_t length)
{
	vector<char> temp;
	temp.resize(length + 1);
	in.read(&temp[0],length);
	temp[length] = '\0';
	string result = (&temp[0]);
	return result;
}

void get_metaserver_player_color(size_t colorIndex, uint16* color) {
  RGBColor c;
  _get_player_color(colorIndex, &c);
  color[0] = c.red;
  color[1] = c.green;
  color[2] = c.blue;
}

void get_metaserver_player_color(rgb_color color, uint16* metaserver_color) {
	metaserver_color[0] = color.red;
	metaserver_color[1] = color.green;
	metaserver_color[2] = color.blue;
}

void
write_player_aux_data(AOStream& out, string name, const string& team, bool away, const string& away_message)
{
	uint8	unused8 = 0;
	uint16 primaryColor[3];
	if (network_preferences->use_custom_metaserver_colors)
		get_metaserver_player_color(network_preferences->metaserver_colors[0], primaryColor);
	else
		get_metaserver_player_color(player_preferences->color, primaryColor);
	uint16	unused16 = 0;
	uint16	secondaryColor[3];
	if (network_preferences->use_custom_metaserver_colors)
		get_metaserver_player_color(network_preferences->metaserver_colors[1], secondaryColor);
	else
		get_metaserver_player_color(player_preferences->team, secondaryColor);
	uint16	orderIndex = 0;

	if (away)
	{
		// alter the player's name
		name = away_message.substr(0, 8) + "-" + name;
	}

	out
		<< kPlayerIcon
		<< unused8
		<< (uint16) (away ? kSTATE_AWAY : kSTATE_AWAKE)
		<< primaryColor[0]
		<< primaryColor[1]
		<< primaryColor[2]
		<< unused16
		<< secondaryColor[0]
		<< secondaryColor[1]
		<< secondaryColor[2]
		<< unused16
		<< orderIndex
		<< kAlephOneClientVersion;

	write_padded_bytes(out, NULL, 0, 14);

	write_string(out, name);
	write_string(out, team);
}



void
LoginAndPlayerInfoMessage::reallyDeflateTo(AOStream& thePacket) const
{
	// AFAICT this must be the size of the "aux player data" below
	// I'd much rather write the stuff and THEN figure out how much there is, but
	// given the way AStreams work currently, this is much easier.  Trust me.  :)
	uint16 thePlayerDataSize = 40 + strlen(m_playerName.c_str()) + strlen(m_teamName.c_str());

	uint16 platform_type;
#ifdef WIN32
	platform_type = kPlatformIsWindows;
#elif defined(__APPLE__) && defined(__MACH__)
	platform_type = kPlatformIsMacintosh;
#else
	platform_type = kPlatformIsOther;
#endif

	uint16 metaserver_version = 0;
	uint32 flags = kResetPlayerData;
	uint32 user_id = 0;
	uint16 max_authentication = kCRYPT_SIMPLE;
#ifdef HAVE_CURL
	max_authentication = kCRYPT_HTTPS;
#endif

	thePacket << platform_type
		  << metaserver_version
		  << flags
		  << user_id
		  << max_authentication
		  << thePlayerDataSize;

	write_padded_string(thePacket, kServiceName, 32);
	write_padded_string(thePacket, __DATE__, 32);
	write_padded_string(thePacket, __TIME__, 32);
	write_padded_string(thePacket, m_userName.c_str(), 32);

	write_player_aux_data(thePacket, m_playerName, m_teamName, false, "");
}



bool
SaltMessage::reallyInflateFrom(AIStream& inStream)
{
	inStream >> m_encryptionType;
	inStream.read(m_salt, sizeof(m_salt));
	return true;
}



void
LocalizationMessage::reallyDeflateTo(AOStream& thePacket) const
{
	uint32 mystery1 = 1;
	uint32 mystery2 = 2;
	uint32 mystery3 = 3;
	uint32 unknown32 = 0;

	thePacket
		<< mystery1
		<< mystery2
		<< mystery3
		<< unknown32;
}



void
RoomDescription::read(AIStream& inStream)
{
	inStream >> m_id
		>> m_playerCount;

	inStream.read((char*) &(m_address.host), sizeof(m_address.host));
	inStream.read((char*) &(m_address.port), sizeof(m_address.port));

	inStream >> m_gameCount
		>> m_type;

	inStream.ignore(10);
}



const std::string
RoomDescription::roomName() const
{
	return (m_id < kRoomNameCount) ? std::string(sRoomNames[m_id]) : std::string("Unknown");
}



ostream&
operator <<(ostream& out, const RoomDescription& roomDesc)
{
	out << roomDesc.roomName() << " : " << roomDesc.roomType() << " : " << roomDesc.playerCount() << " : " << roomDesc.gameCount();
	return out;
}



bool
RoomListMessage::reallyInflateFrom(AIStream& inStream)
{
	while(inStream.maxg() > inStream.tellg())
	{
		m_rooms.push_back(RoomDescription());
		m_rooms.back().read(inStream);
	}
	return true;
}



void
RoomLoginMessage::reallyDeflateTo(AOStream& out) const
{
	// Can't persuade linker this should work
	//out << m_token;
	out.write(m_token, sizeof(m_token));
	write_string(out, m_loginName);
}



ostream&
operator <<(ostream& out, const RoomListMessage& message)
{
	out << "RoomListMessage: " << message.rooms().size() << " elements\n";
	copy(message.rooms().begin(), message.rooms().end(), ostream_iterator<RoomDescription>(out, "\n"));
	return out;
}



void
NameAndTeamMessage::reallyDeflateTo(AOStream& out) const
{
	write_player_aux_data(out, m_name, m_team, m_away, m_away_message);
}

bool
IDAndLimitMessage::reallyInflateFrom(AIStream& inStream)
{
	uint16 ignore16;

	inStream
		>> m_playerID
		>> m_playerLimit
		>> ignore16;

	return true;
}



bool
DenialMessage::reallyInflateFrom(AIStream& inStream)
{
	inStream >> m_code;
	m_message = read_string(inStream);

	return true;
}



bool
BroadcastMessage::reallyInflateFrom(AIStream& inStream)
{
	m_message = read_string(inStream);
	return true;
}

PrivateMessage::PrivateMessage(uint32 inSenderID, const string& inSenderName, uint32 inSelectedID, const string& inMessage) : m_senderID(inSenderID), m_selectedID(inSelectedID), m_internalType(0), m_flags(kDirectedBit), m_senderName(inSenderName), m_message(inMessage)
{
	get_metaserver_player_color(player_preferences->color, m_color);
}

void
PrivateMessage::reallyDeflateTo(AOStream& thePacket) const
{
	uint32 echo = 1;
	uint16 size = 0;
	uint16 colorFlags = 0;
	uint16 unused16 = 0;

	thePacket
		<< m_selectedID
		<< echo
		<< m_internalType
		<< size;
	
	thePacket.write(m_color, 3);

	thePacket
		<< colorFlags
		<< m_flags
		<< unused16
		<< m_senderID
		<< m_selectedID;

	write_string(thePacket, m_senderName);
	write_string(thePacket, m_message);
}

bool
PrivateMessage::reallyInflateFrom(AIStream& inStream)
{
	uint32 player_id;
	uint32 echo;
	uint16 size;
	uint16 colorFlags;
	uint16 unused16;

	inStream
		>> player_id
		>> echo
		>> m_internalType
		>> size;

	inStream.read(m_color, 3);

	inStream
		>> colorFlags
		>> m_flags
		>> unused16
		>> m_senderID
		>> m_selectedID;

	m_senderName = read_string(inStream);
	m_message = read_string(inStream);

	return true;
}

ChatMessage::ChatMessage(uint32 inSenderID, const string& inSenderName, const string& inMessage)
	: m_senderID(inSenderID), m_internalType(0), m_flags(0), m_senderName(inSenderName), m_message(inMessage)
{
  get_metaserver_player_color(player_preferences->color, m_color);
}

void
ChatMessage::reallyDeflateTo(AOStream& thePacket) const
{
	uint16 size = 0;
	uint16 colorFlags = 0;
	uint16 unused16 = 0;
	uint32 destinationPlayerID = 0;

	thePacket
		<< m_internalType
		<< size;
	
	thePacket.write(m_color, 3);

	thePacket
		<< colorFlags
		<< m_flags
		<< unused16
		<< m_senderID
		<< destinationPlayerID;

	write_string(thePacket, m_senderName);
	write_string(thePacket, m_message);
}

bool
ChatMessage::reallyInflateFrom(AIStream& inStream)
{
	uint16 size;
	uint16 colorFlags;
	uint16 unused16;
	uint32 destinationPlayerID;

	inStream
		>> m_internalType
		>> size;

	inStream.read(m_color, 3);

	inStream
		>> colorFlags
		>> m_flags
		>> unused16
		>> m_senderID
		>> destinationPlayerID;

	m_senderName = read_string(inStream);
	m_message = read_string(inStream);

	return true;
}



MetaserverPlayerInfo::MetaserverPlayerInfo(AIStream& inStream) : m_target(false)
{
	uint32 ignore32;
	uint8 ignore8;

	inStream
		>> m_verb
		>> m_adminFlags
		>> ignore32
		>> m_playerID
		>> m_roomID
		>> ignore8
		>> m_rank
		>> m_playerDataSize
		>> ignore32
		>> m_icon
		>> ignore8
		>> m_status;

	inStream.read(m_primaryColor, 3);
	inStream.ignore(2);
	inStream.read(m_secondaryColor, 3);
	inStream.ignore(20);
	m_name = read_string(inStream);
	m_team = read_string(inStream);
}

ostream&
operator <<(ostream& out, const MetaserverPlayerInfo& info)
{
	return (out
		<< info.m_verb		<< "; "
		<< info.m_adminFlags	<< "; "
		<< info.m_playerID	<< "; "
		<< info.m_roomID	<< "; "
		<< info.m_rank		<< "; "
		<< info.m_playerDataSize << "; "
		<< (uint16)info.m_icon	<< "; "
		<< (uint16)info.m_status << "; "
		<< info.m_name		<< "; "
		<< info.m_team << endl << endl
	 );
}

bool
PlayerListMessage::reallyInflateFrom(AIStream& inStream)
{
	while(inStream.maxg() > inStream.tellg())
		m_players.push_back(MetaserverPlayerInfo(inStream));

	return true;
}

ostream&
operator <<(ostream& out, const PlayerListMessage& message)
{
	out << "PlayerListMessage: " << message.players().size() << " elements\n";
	copy(message.players().begin(), message.players().end(), ostream_iterator<MetaserverPlayerInfo>(out, "\n"));
	return out;
}



void
CreateGameMessage::reallyDeflateTo(AOStream& thePacket) const
{
	uint16 orderGame = 0;
	
	thePacket
		<< m_gamePort
		<< orderGame
		<< m_description;
}



AIStream&
operator >>(AIStream& stream, HandoffToken& token)
{
	stream.read(token, sizeof(token));
	return stream;
}

AOStream&
operator <<(AOStream& stream, const HandoffToken& token)
{
	stream.write(token, sizeof(token));
	return stream;
}



bool
LoginSuccessfulMessage::reallyInflateFrom(AIStream& inStream)
{
	uint16 unused;

	inStream
		>> m_userID
		>> m_order
		>> unused
		>> m_token;

	return true;
}



bool
SetPlayerDataMessage::reallyInflateFrom(AIStream& inStream)
{
	return true;
}



AIStream&
operator >>(AIStream& stream, GameDescription& desc)
{
	uint16 unknown16;
	uint32 options;
	uint16 teamRandomSeed;
	uint16 maxTeams;
	uint32 randomSeed;
	uint32 planningTime;
	uint32 unused32;
	uint16 pluginFlag;
	const int kPluginListSize = 512;
	uint32 clientVersion;
	uint32 unknown32;
	uint16 status;
	uint8 action;

	stream
		>> unknown16
		>> desc.m_type
		>> options
		>> desc.m_timeLimit
		>> desc.m_mapChecksum
		>> desc.m_difficulty
		>> desc.m_maxPlayers
		>> teamRandomSeed
		>> maxTeams
		>> randomSeed
		>> planningTime
		>> unused32
		>> unused32
		>> unknown16
		>> pluginFlag;
	
	if (pluginFlag & 0x1) {
		desc.m_alephoneBuildString = read_padded_string(stream, 32);
		desc.m_networkSetupProtocolID = read_padded_string(stream, 32);
		desc.m_scenarioName = read_padded_string(stream, 32);
		desc.m_scenarioID = read_padded_string(stream, 24);
		desc.m_scenarioVersion = read_padded_string(stream, 8);
		desc.m_netScript = read_padded_string(stream, 32);
		
		if (pluginFlag & 0x2)
		{
			desc.m_hasGameOptions = true;
			stream >> desc.m_gameOptions
			       >> desc.m_cheatFlags
			       >> desc.m_killLimit;

			desc.m_mapFileName = read_padded_string(stream, 32);
			desc.m_physicsName = read_padded_string(stream, 32);
			stream.ignore(kPluginListSize - (32 + 32 + 32 + 24 + 8 + 32) - (2 + 2 + 2 + 32 + 32));
		}
		else
		{
			desc.m_hasGameOptions = false;
			stream.ignore(kPluginListSize - (32 + 32 + 32 + 24 + 8 + 32));
		}
	} else {
		desc.m_hasGameOptions = false;
		stream.ignore(kPluginListSize);
	}

	stream
		>> clientVersion
		>> unknown32
		>> status
		>> desc.m_numPlayers
		>> action;

	desc.m_name = read_string(stream);
	desc.m_mapName = read_string(stream);

	desc.m_closed = ((status & 0x0010) == 0x0010);
	desc.m_running = ((status & 0x0001) == 0x0001);
	desc.m_teamsAllowed = (maxTeams != (uint16)-1);

	return stream;
}



AOStream&
operator <<(AOStream& stream, const GameDescription& desc)
{
	uint16 unknown16 = 0;
	uint32 options = 0;
	uint16 teamRandomSeed = 0;
	uint16 maxTeams = desc.m_teamsAllowed ? desc.m_maxPlayers : (uint16)-1;
	uint32 randomSeed = 0;
	uint32 planningTime = 0;
	uint32 unused32 = 0;
	uint16 pluginFlag = 0x3;
	const int kPluginListSize = 512;
	uint32 clientVersion = 0xc136e436;
	uint32 unknown32 = 0;
	uint16 status = 0
		| (desc.m_closed ? 0x0010 : 0x0000)
		| (desc.m_running ? 0x0001 : 0x0000)
	;
	uint8 action = 0;

	stream
		<< unknown16
		<< desc.m_type
		<< options
		<< desc.m_timeLimit
		<< desc.m_mapChecksum
		<< desc.m_difficulty
		<< desc.m_maxPlayers
		<< teamRandomSeed
		<< maxTeams
		<< randomSeed
		<< planningTime
		<< unused32
		<< unused32
		<< unknown16
		<< pluginFlag;
	
	// plugin flag 0x1 stuff
	const int pluginFlag1Size = 32 + 32 + 32 + 24 + 8 + 32;
	{
		write_padded_string(stream, desc.m_alephoneBuildString.c_str(), 32);
		write_padded_string(stream, desc.m_networkSetupProtocolID.c_str(), 32);
		write_padded_string(stream, desc.m_scenarioName.c_str(), 32);
		write_padded_string(stream, desc.m_scenarioID.c_str(), 24);
		write_padded_string(stream, desc.m_scenarioVersion.c_str(), 8);
		write_padded_string(stream, desc.m_netScript.c_str(), 32);
	}

	const int pluginFlag2Size = 2 + 2 + 2 + 32 + 32;
	// plugin flag 0x2 stuff
	{
		stream << desc.m_gameOptions
		       << desc.m_cheatFlags
		       << desc.m_killLimit;

		write_padded_string(stream, desc.m_mapFileName.c_str(), 32);
		write_padded_string(stream, desc.m_physicsName.c_str(), 32);
	}
	
	write_padded_bytes(stream, NULL, 0, kPluginListSize - pluginFlag1Size - pluginFlag2Size);

	stream
		<< clientVersion
		<< unknown32
		<< status
		<< desc.m_numPlayers
		<< action;

	write_string(stream, desc.m_name);
	write_string(stream, desc.m_mapName);

	return stream;
}

/* shouldn't these be using the STR# that contains the list instead? */
static const char* gameTypeString[] =
{
	"Every Man for Himself",
	"Cooperative Play",
	"Capture the Flag",
	"King of the Hill",
	"Kill the Man With the Ball",
	"Defense",
	"Rugby",
	"Tag",
	"Netscript",
};

static const char* difficultyLevelString[] =
{
	"Kindergarten",
	"Easy",
	"Normal",
	"Major Damage",
	"Total Carnage"
};

ostream&
operator <<(ostream& stream, const GameDescription& desc)
{
	stream
		<< desc.m_name << " : "
		<< desc.m_mapName << " (" << desc.m_mapChecksum << ") : "
		<< gameTypeString[desc.m_type] << " : "
		<< difficultyLevelString[desc.m_difficulty] << " : "
		<< desc.m_timeLimit << " : "
		<< static_cast<uint16>(desc.m_numPlayers) << "/" << desc.m_maxPlayers << " : "
		<< (desc.m_closed ? "Closed" : "Open") << " : "
		<< (desc.m_running ? "Running" : "Not Running") << " : "
		<< (desc.m_teamsAllowed ? "Teams" : "No Teams");

	return stream;
}

static string lua_to_game_string(const std::string& lua)
{
	string game_string = lua;
	if (boost::algorithm::ends_with(game_string, ".lua") || boost::algorithm::ends_with(game_string, ".txt"))
	{
		game_string.resize(game_string.length() - 4);
	}

	for (string::iterator it = game_string.begin(); it != game_string.end(); ++it)
	{
		if (*it == '_')
			*it = ' ';
	}

	return game_string;
}

string GameListMessage::GameListEntry::game_string() const
{
	if (m_description.m_type == _game_of_custom)
	{
		// convert the Lua script to a name
		return lua_to_game_string(m_description.m_netScript);
	}
	else
	{
		int type = m_description.m_type - (m_description.m_type > 5 ? 1 : 0);
		if (TS_GetCString(kNetworkGameTypesStringSetID, type))
		{
			return string(TS_GetCString(kNetworkGameTypesStringSetID, type));
		}
		else
		{
			return string("Unknown Game Type");
		}
	}
}

string GameListMessage::GameListEntry::format_for_chat(const string& player_name) const
{
	ostringstream message;
	message << player_name << "|p";
	if (running())
		message << " is hosting ";
	else
		message << " is gathering ";

	if (m_description.m_timeLimit && !(m_description.m_timeLimit == INT32_MAX || m_description.m_timeLimit == -1))
	{
		message << m_description.m_timeLimit / 60 / TICKS_PER_SECOND
			<< " minutes of |i";
	}
	message << m_description.m_mapName
		<< ",|p "
		<< game_string();
	
	return message.str();
}

bool
GameListMessage::reallyInflateFrom(AIStream& inStream)
{
	while(inStream.tellg() != inStream.maxg())
	{
		GameListEntry entry;
		inStream >> entry;
		entry.m_ticks = machine_tick_count();

		m_entries.push_back(entry);
	}

	return true;
}



AIStream&
operator >>(AIStream& stream, GameListMessage::GameListEntry& entry)
{
	uint32	unused32;
	uint16	unused16;

	stream
		>> entry.m_gameID
		>> entry.m_ipAddress[0]
		>> entry.m_ipAddress[1]
		>> entry.m_ipAddress[2]
		>> entry.m_ipAddress[3]
		>> entry.m_port
		>> entry.m_verb
		>> entry.m_gameEnable
		>> entry.m_timeRemaining
		>> entry.m_hostPlayerID
		>> entry.m_len
		>> unused32
		>> unused32
		>> unused16
		>> entry.m_description;

	return stream;
}



static ostream&
printDottedDecimal(ostream& stream, const uint8* ip)
{
	stream
		<< static_cast<unsigned int>(ip[0]) << "."
		<< static_cast<unsigned int>(ip[1]) << "."
		<< static_cast<unsigned int>(ip[2]) << "."
		<< static_cast<unsigned int>(ip[3]);

	return stream;
}



std::ostream&
operator <<(std::ostream& stream, const GameListMessage::GameListEntry& entry)
{
	stream
		<< static_cast<uint16>(entry.m_verb) << " : "
		<< entry.m_gameID << " : ";

	printDottedDecimal(stream, entry.m_ipAddress);

	stream
		<< ":" << entry.m_port << " : "
		<< static_cast<uint16>(entry.m_gameEnable) << " : "
		<< entry.m_timeRemaining << " : "
		<< entry.m_hostPlayerID << " : "
		<< entry.m_len << " : "
		<< entry.m_description;

	return stream;
}



void
StartGameMessage::reallyDeflateTo(AOStream& thePacket) const
{
	uint32 unused32 = 0;
	thePacket
		<< m_gameTimeInSeconds
		<< unused32
		<< unused32;
}

#endif // !defined(DISABLE_NETWORKING)
