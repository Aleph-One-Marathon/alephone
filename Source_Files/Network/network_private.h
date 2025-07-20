/*
 *  network_private.h

	Copyright (C) 1991-2001 and beyond by Bungie Studios, Inc.
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

 * This file holds stuff that various parts of the network subsystem want to see, but that portions
 * of Aleph One outside of the networking code should not care about.
 *
 * Oct 11, 2001 (Woody Zenfell): split code away from network.cpp to create this file.
 *	Made any simple modifications needed to make things compile/work.
 *
 * Oct-Nov 2001 (Woody Zenfell): added code in support of text messages in stream packets.
 *	Added many comments.

Feb 27, 2002 (Br'fin (Jeremy Parsons)):
	Enabled SDL networking for Carbon without fully using SDL

 May 24, 2003 (Woody Zenfell):
	Split ring-protocol-specific stuff out into RingGameProtocol.cpp;
	"Unknown packet type response" streaming packet type; NetGetDistributionInfoForType()
 */

#ifndef	NETWORK_PRIVATE_H
#define	NETWORK_PRIVATE_H

#include	"cstypes.h"
#include	"network.h"

// "network_dialogs_private.h"
#include	"SSLP_API.h"

#include <memory>

#define	GAME_PORT (network_preferences->game_port)

// (ZZZ:) Moved here from sdl_network.h and macintosh_network.h

/* ---------- constants */

#define strNETWORK_ERRORS 132

enum /* error string for user */
{
	netErrCantAddPlayer,
	netErrCouldntDistribute,
	netErrCouldntJoin,
	netErrServerCanceled,
	netErrMapDistribFailed,
	netErrWaitedTooLongForMap,
	netErrSyncFailed,
	netErrJoinFailed,
	netErrCantContinue,
        netErrIncompatibleVersion,
        netErrGatheredPlayerUnacceptable,
        netErrUngatheredPlayerUnacceptable,
        netErrJoinerCantFindScenario,
	netErrLostConnection,
	netErrCouldntResolve,
	netErrCouldntReceiveMap,
	netWarnJoinerHasNoStar,
	netWarnJoinerHasNoRing,
	netWarnJoinerNoLua,
	netErrMetaserverConnectionFailure,
	netWarnCouldNotAdvertiseOnMetaserver,
	netWarnUPnPConfigureFailed,
	netWarnRemoteHubServerNotAvailable
};

// (ZZZ:) Moved here from network.cpp

/* ---------- constants */

#define NET_DEAD_ACTION_FLAG (NONE)

#define MAXIMUM_GAME_DATA_SIZE       256
#define MAXIMUM_PLAYER_DATA_SIZE     128

#define MAP_TRANSFER_TIME_OUT   (MACHINE_TICKS_PER_SECOND*70) // 70 seconds to wait for map.

enum /* tag */
{
	tagRING_PACKET, //obsolete
	tagACKNOWLEDGEMENT, //obsolete
	tagCHANGE_RING_PACKET, //obsolete
	
        // ZZZ annotation: these should only be found in streaming data (in a NetTopology).
	tagNEW_PLAYER,
	tagCANCEL_GAME,
	tagSTART_GAME,
	tagDROPPED_PLAYER,
	tagCHANGED_PLAYER,
	
        // ZZZ annotation: these (in NetPacketHeader) indicate the rest of the datagram is a NetDistributionPacket.
	tagLOSSY_DISTRIBUTION, //obsolete
	tagLOSSLESS_DISTRIBUTION, //obsolete
        
        // ZZZ: more streaming data (topology) packet types
        tagRESUME_GAME	// ZZZ addition: trying to resume a saved-game rather than start a new netgame.
};

/* ---------- structures */


// (ZZZ:) Note ye well!!: if you alter these network-related structures, 
// you'll probably need to alter get_network_version() while you're at it,
// since you've made an incompatible change to the network communication protocol.

// Information passed in streams
struct NetPlayer
{
	IPaddress dspAddress, ddpAddress;
	
	int16 identifier;

	int16 stream_id; // to remind gatherer how to contact joiners

	bool net_dead; // only valid if you are the server.

  //uint8 player_data[MAXIMUM_PLAYER_DATA_SIZE];
  player_info player_data;
};
typedef struct NetPlayer NetPlayer, *NetPlayerPtr;

struct NetServer
{
	IPaddress dspAddress, ddpAddress;
};

struct NetTopology
{
	int16 tag;
	int16 player_count;
	
	int16 nextIdentifier;
	
  //uint8 game_data[MAXIMUM_GAME_DATA_SIZE];
	game_info game_data;
	
	struct NetPlayer players[MAXIMUM_NUMBER_OF_NETWORK_PLAYERS];
	struct NetServer server;
};
typedef struct NetTopology NetTopology, *NetTopologyPtr;

/* ===== application specific data structures/enums */

// Altering these constants requires changes to get_network_version().  - Woody
enum {
	_hello_packet,
	_joiner_info_packet,
	_join_player_packet,
	_accept_join_packet,
	_topology_packet,
	_stream_size_packet,
	_stream_data_packet,
	// ZZZ additions below
        _chat_packet,
	// The following should only be sent when get_network_version() >= kMinimumNetworkVersionForGracefulUnknownStreamPackets
	_unknown_packet_type_response_packet,
        _script_packet,
	NUMBER_OF_BUFFERED_STREAM_PACKET_TYPES,
	NUMBER_OF_STREAM_PACKET_TYPES= 	NUMBER_OF_BUFFERED_STREAM_PACKET_TYPES
};

/* ===== end of application specific data structures/enums */

class CommunicationsChannel;
class MessageDispatcher;
class MessageHandler;
class Message;

struct ClientChatInfo
{
	std::string name;
	int16 color;
	int16 team;
};

// "network_dialogs_private.h" follows

class GathererAvailableAnnouncer
{
public:
	GathererAvailableAnnouncer();
	~GathererAvailableAnnouncer();
	static void pump();

private:
	SSLP_ServiceInstance	mServiceInstance;
};


class JoinerSeekingGathererAnnouncer
{
public:
	JoinerSeekingGathererAnnouncer(bool shouldSeek);
	~JoinerSeekingGathererAnnouncer();
	static void pump();

private:
	static void found_gatherer_callback(const SSLP_ServiceInstance* instance);
	static void lost_gatherer_callback(const SSLP_ServiceInstance* instance);
	
	bool mShouldSeek;
};


#endif//NETWORK_PRIVATE_H
