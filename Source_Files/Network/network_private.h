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

#include	"sdl_network.h"

#include	"network.h"

// "network_dialogs_private.h"
#include	"SSLP_API.h"

#include <memory>

#define	GAME_PORT (network_preferences->game_port)

// (ZZZ:) Moved here from sdl_network.h and macintosh_network.h

/* ---------- constants */

#define asyncUncompleted 1	/* ioResult value */

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
	netWarnUPnPConfigureFailed
};

// (ZZZ:) Moved here from network.cpp

/* ---------- constants */

#define NET_DEAD_ACTION_FLAG_COUNT (-1)
#define NET_DEAD_ACTION_FLAG (NONE)

#define MAXIMUM_GAME_DATA_SIZE       256
#define MAXIMUM_PLAYER_DATA_SIZE     128
#define MAXIMUM_UPDATES_PER_PACKET    16 // how many action flags per player can be sent in each ring packet
#define UPDATES_PER_PACKET             1  // defines action flags per packet and period of the ring
#define UPDATE_LATENCY                 1

#define NET_QUEUE_SIZE (MAXIMUM_UPDATES_PER_PACKET+1)

#define UNSYNC_TIMEOUT (3*MACHINE_TICKS_PER_SECOND) // 3 seconds

#define STREAM_TRANSFER_CHUNK_SIZE (10000)
#define MAP_TRANSFER_TIME_OUT   (MACHINE_TICKS_PER_SECOND*70) // 70 seconds to wait for map.
#define NET_SYNC_TIME_OUT       (MACHINE_TICKS_PER_SECOND*50) // 50 seconds to time out of syncing. 

#define kACK_TIMEOUT 40
#define kRETRIES     50  // how many timeouts allowed before dropping the next player
                         // kRETRIES * kACK_TIMEOUT / 1000 = timeout in seconds

#define NUM_DISTRIBUTION_TYPES    3

// Altering constants below should make you alter get_network_version().  - Woody
#define kPROTOCOL_TYPE           69

enum /* tag */
{	// ZZZ annotation: these (in NetPacketHeader) indicate the rest of the datagram is a NetPacket (i.e. a ring packet).
	tagRING_PACKET,
	tagACKNOWLEDGEMENT,
	tagCHANGE_RING_PACKET,  // to tell a player to change his downring address. also has action flags.
	
        // ZZZ annotation: these should only be found in streaming data (in a NetTopology).
	tagNEW_PLAYER,
	tagCANCEL_GAME,
	tagSTART_GAME,
	tagDROPPED_PLAYER,
	tagCHANGED_PLAYER,
	
        // ZZZ annotation: these (in NetPacketHeader) indicate the rest of the datagram is a NetDistributionPacket.
	tagLOSSY_DISTRIBUTION,     // for transfer data other than action flags
	tagLOSSLESS_DISTRIBUTION,   // ditto, but currently unimplemented
        
        // ZZZ: more streaming data (topology) packet types
        tagRESUME_GAME	// ZZZ addition: trying to resume a saved-game rather than start a new netgame.
};

enum
{
	typeSYNC_RING_PACKET,    // first packet of the game, gets everyone in the game
	typeTIME_RING_PACKET,    // second packet of the game, sets everyone's clock
	typeNORMAL_RING_PACKET,   // all the other packets of the game
	
	typeUNSYNC_RING_PACKET,	// last packet of the game, get everyone unsynced properly.
	typeDEAD_PACKET	// This is simply a convenience for a switch. This packet never leaves the server.
};

/* ---------- structures */


// (ZZZ:) Note ye well!!: if you alter these network-related structures, you are probably going to need to modify
// the corresponding _NET structures in network_data_formats.h AND *both* corresponding netcpy() functions in
// network_data_formats.cpp.  AND, you'll probably need to alter get_network_version() while you're at it,
// since you've made an incompatible change to the network communication protocol.

// (ZZZ:) Information passed in datagrams (note: the _NET version is ALWAYS the one sent/received on the wire.
// If not, it's a BUG.  These are used to setup/extract data.)
struct NetPacketHeader
{
	int16 tag;
	int32 sequence;
	
	/* data */
};
typedef struct NetPacketHeader NetPacketHeader, *NetPacketHeaderPtr;

struct NetPacket
{
	uint8 ring_packet_type;         // typeSYNC_RING_PACKET, etc...
	uint8 server_player_index;
	int32 server_net_time;
	int16 required_action_flags;                         // handed down from on high (the server)
	int16 action_flag_count[MAXIMUM_NUMBER_OF_NETWORK_PLAYERS];  // how many each player actually has.
	uint32 action_flags[1];
};
typedef struct NetPacket NetPacket, *NetPacketPtr;

struct NetDistributionPacket
{
	int16 distribution_type;  // type of information
	int16 first_player_index; // who sent the information
	int16 data_size;          // how much they're sending.
	uint8  data[2];            // the chunk Õo shit to send
};
typedef struct NetDistributionPacket NetDistributionPacket, *NetDistributionPacketPtr;

// Information passed in streams
struct NetPlayer
{
	NetAddrBlock dspAddress, ddpAddress;
	
	int16 identifier;

	int16 stream_id; // to remind gatherer how to contact joiners

	bool net_dead; // only valid if you are the server.

  //uint8 player_data[MAXIMUM_PLAYER_DATA_SIZE];
  player_info player_data;
};
typedef struct NetPlayer NetPlayer, *NetPlayerPtr;

struct NetTopology
{
	int16 tag;
	int16 player_count;
	
	int16 nextIdentifier;
	
  //uint8 game_data[MAXIMUM_GAME_DATA_SIZE];
  game_info game_data;
	
	struct NetPlayer players[MAXIMUM_NUMBER_OF_NETWORK_PLAYERS];
};
typedef struct NetTopology NetTopology, *NetTopologyPtr;

#ifdef NETWORK_CHAT
// (ZZZ addition)
enum { CHAT_MESSAGE_TEXT_BUFFER_SIZE = 250 };

struct NetChatMessage {
    int16   sender_identifier;
    char    text[CHAT_MESSAGE_TEXT_BUFFER_SIZE];
};
#endif


// ZZZ: same here (should be safe to alter)
struct NetDistributionInfo
{
	bool              lossy;
	NetDistributionProc  distribution_proc;
};

typedef struct NetDistributionInfo NetDistributionInfo, *NetDistributionInfoPtr;

#define errInvalidMapPacket (42)
// ZZZ: taking a cue... used when trying to gather a player whose A1 doesn't support all the features we need.
#define errPlayerTooNaive (43)

/* ===== application specific data structures/enums */

// Information sent via streaming protocol - warning above still applies!
struct gather_player_data {
	int16 new_local_player_identifier;
};

// used in accept_gather_data::accepted - this is a sneaky way of detecting whether
// we're playing with a resume netgame-capable player or not.  (Old code always sent
// 1 on accept, never 2; old code interprets any nonzero 'accepted' as an accept.)
enum {
        kNaiveJoinerAccepted = 1,
        kResumeNetgameSavvyJoinerAccepted = 2,	// build knows how to resume saved games as netgames
        kFixedTagAndBallJoinerAccepted = 3,	// build lacks multiple-ball-drop bug and tag-suicide bug

        // this should always be updated to match the current best (unless our build isn't up to spec)
        kStateOfTheArtJoinerAccepted = kFixedTagAndBallJoinerAccepted
};

struct accept_gather_data {
	uint8 accepted;
	NetPlayer player;
};

enum {
	_netscript_query_message,
        _netscript_no_script_message,
        _netscript_yes_script_message,
        _netscript_script_intent_message
};

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


const NetDistributionInfo* NetGetDistributionInfoForType(int16 inType);

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
