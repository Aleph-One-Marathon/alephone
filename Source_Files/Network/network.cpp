/*
NETWORK.C

	Copyright (C) 1991-2001 and beyond by Bungie Studios, Inc.
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

Monday, June 20, 1994 12:22:03 PM
Wednesday, June 29, 1994 9:14:21 PM
	made ddp ring work with more than 2 players (upring and downring were confused)
Saturday, July 2, 1994 3:54:12 PM
	simple distribution of map
Friday, July 15, 1994 10:51:38 AM
	gracefully handling players dropping from the game. don't allow quiting from the game while
	we have the ring packet. changed distribution of the map now that we transfer a level at a time.
Sunday, July 17, 1994 4:01:18 PM
	multiple updates per packet
Monday, July 18, 1994 11:51:51 AM
	transfering map in chunks now, since ADSP can only write 64K at a time.
Tuesday, July 19, 1994 7:14:30 PM
	fixed one player ring bug yesterday.
Wednesday, July 20, 1994 12:34:06 AM
	variable number of updates per packet. (can only be adjusted upward, not downward).
Monday, July 25, 1994 9:04:24 PM
	Jason's new algorithm. dropping players and slowing down the ring doesn't work now.
	but performance is much smoother, and better understood, to boot. 
Sunday, August 21, 1994 3:58:23 PM
	about a week ago, added stuff to use the ring to distribute other information, like
	sound or text for the game.

Jan 30, 2000 (Loren Petrich):
	Added some typecasts

Feb. 4, 2000 (Loren Petrich):
	Changed halt() to assert(false) for better debugging
        
Sept-Oct 2001 (Woody Zenfell): (roughly in order)
        Plugged in netcpy/_NET stuff and a couple #ifdef SDL byte-swappers for portable data formats.
        Changed a couple memcpy() calls to memmove() calls as they have overlapping source and dest.
        Allowed the use of the MyTM* functions, which now have SDL_thread-based implementations.
        Added optional NETWORK_FAUX_QUEUE mechanism, should work on either platform.
        It was a good idea, I think, but ultimately fairly pointless.  NETWORK_ADAPTIVE_LATENCY should be better.
        Added optional NETWORK_ADAPTIVE_LATENCY mechanism, should work on either platform.
        Changed some #ifdef mac conditionals to #ifndef NETWORK_IP to better convey what we're worried about.
        Added NETWORK_USE_RECENT_FLAGS option to discard excess flags from the head, rather than tail, of the queue.
        Added... how to say... "copious" comments (ZZZ) at various times as I browsed the source and made changes.
        Found that a basic assumption I was using in my optimizations (i.e. that the game processed action_flags
        at a constant rate) was wrong, which made the Bungie way make a lot more sense.  I now recommend using *none*
        of the three NETWORK_* options (do use NETWORK_IP though of course if appropriate).
        
Nov 13, 2001 (Woody Zenfell):
        Although things were basically OK under favorable conditions, they were IMO too "fragile" - sensitive
        to latency and jitter.  I couldn't help but try again... so NETWORK_ADAPTIVE_LATENCY_2 has been added.
        Also put in NETWORK_SMARTER_FLAG_DITCHING mechanism.

Feb 27, 2002 (Br'fin (Jeremy Parsons)):
	Rewired things to more generally key off of HAVE_SDL_NET than SDL (The Carbon build has SDL_NET, but
		understandably lacks SDL)
	Uses #if HAVE_SDL_NET in place of calls to #ifndef mac to allow SDL networking under Carbon

Mar 3-8, 2002 (Woody Zenfell):
    Changed net distribution stuff to use an STL map to associate distribution types with
    {lossy, handling procedure}.  Now different endstations can have different distribution
    types installed for handling (previously they had to all install the same handlers in the
    same order to get the same distribution type ID's).

Feb 5, 2003 (Woody Zenfell):
        Preliminary support for resuming saved-games networked.
        
Feb 13, 2003 (Woody Zenfell):
        Resuming saved-games as network games works.

May 24, 2003 (Woody Zenfell):
	Split out ring-protocol-specific stuff from here to RingGameProtocol.cpp.
	This is multiple-game-protocol-savvy now.
	Support for graceful handling of unknown streaming-data packet types.
        
July 03, 2003 (jkvw):
        Added network lua scripts.

September 17, 2004 (jkvw):
	NAT-friendly networking.  That is, joiners behind firewalls should be able to play.
	Also moved to TCPMess for TCP communications.
*/

/*
I would really like to be able to let the Unysnc packet go around the loop, but it is difficult
	because all the code is currently setup to handle only one packet at a time.
Currently 1 player games (when others are dropped) always have lots of late packets (never get
	acknowledged properly, since they are the only player.)
Note that the unregister isn't fast enough, and that the registration code is stupid.  Also should
	setup the dialog such that it doesn't allow for network play when a player is added.
*/

/*
NetADSPRead() should time out by calling PBControl(dspStatus, ...) to see if there are enough bytes
clearly this is all broken until we have packet types
*/


#include "cseries.h"
#include "map.h"       // for TICKS_PER_SECOND and "struct entry_point"
#include "interface.h" // for transfering map
#include "mytm.h"	// ZZZ: both versions use mytm now
#include "preferences.h" // for network_preferences and environment_preferences

#if defined(SDL) || HAVE_SDL_NET
#include "sdl_network.h"
#include	"network_lookup_sdl.h"
#include	"SDL_thread.h"
#elif defined(mac)
#include "macintosh_network.h"
#endif

#include "game_errors.h"
#include "CommunicationsChannel.h"
#include "MessageInflater.h"
#include "progress.h"
#include "extensions.h"

// #define TEST_MODEM

#ifdef TEST_MODEM
#include "network_modem.h"
#include "network_modem_protocol.h"
#endif

#include <stdlib.h>
#include <string.h>

#include <map>
#include "Logging.h"

#ifdef env68k
#pragma segment network
#endif

#define NO_PHYSICS

#ifdef DEBUG
//	#define DEBUG_NET
//#define DEBUG_NET_RECORD_PROFILE
#endif
// #define DEBUG_NET

// ZZZ: moved many struct definitions, constant #defines, etc. to header for (limited) sharing
#include "network_private.h"

// ZZZ: since network data format is now distinct from in-memory format.
// (quite similar, admittedly, in this first effort... ;) )
#include "network_data_formats.h"

#if HAVE_SDL_NET
#define	NETWORK_IP			// needed if using IPaddress { host, port }; (as in SDL_net) rather than NetAddrBlock for addressing.
#endif

#include "NetworkGameProtocol.h"

#include "RingGameProtocol.h"
#include "StarGameProtocol.h"

// LP: kludge so I can get the code to compile
#if defined(mac) && !HAVE_SDL_NET
//#define NETWORK_IP // JTP: No no no, this defeats the whole purpose of NETWORK_IP
#undef NETWORK_IP
#endif

#include "lua_script.h"

/* ---------- globals */

static short ddpSocket; /* our ddp socket number */

static short localPlayerIndex;
static short localPlayerIdentifier;
static NetTopologyPtr topology;
static char *network_adsp_packet;
static short sServerPlayerIndex;
static bool sOldSelfSendStatus;
static RingGameProtocol sRingGameProtocol;
static StarGameProtocol sStarGameProtocol;
static NetworkGameProtocol* sCurrentGameProtocol = NULL;

static byte *deferred_script_data = NULL;
static size_t deferred_script_length = 0;
static bool do_netscript;

static CommunicationsChannelFactory *server = NULL;
typedef std::map<int, CommunicationsChannel*> connection_map_t;
static connection_map_t connections_to_clients;
static CommunicationsChannel *connection_to_server = NULL;
static int next_stream_id = 0;
static IPaddress host_address;
static bool host_address_specified = false;
static MessageInflater *inflater = NULL;
static uint32 next_join_attempt;

// ZZZ note: very few folks touch the streaming data, so the data-format issues outlined above with
// datagrams (the data from which are passed around, interpreted, and touched by many functions)
// don't matter as much.  Do observe, though, that users of the "distribution" mechanism will have
// to pack and unpack their own distribution data - we can't be expected to know what they're doing.

// ZZZ note: read this externally with the NetState() function.
static short netState= netUninitialized;

// ZZZ change: now using an STL 'map' to, well, _map_ distribution types to info records.
typedef std::map<int16, NetDistributionInfo> distribution_info_map_t;
static  distribution_info_map_t distribution_info_map;


#ifdef NETWORK_CHAT
static	NetChatMessage	incoming_chat_message_buffer;
static bool		new_incoming_chat_message = false;
#endif


// ZZZ: are we trying to start a new game or resume a saved-game?
// This is only valid on the gatherer after NetGather() is called;
// only valid on a joiner once he receives the final topology (tagRESUME_GAME)
// Used, at least, on the gatherer to determine whether or not to resort players by address
static bool resuming_saved_game = false;


/* ---------- private prototypes */
void NetPrintInfo(void);

// ZZZ: cmon, we're not fooling anyone... game_data is a game_info*; player_data is a player_info*
// Originally I guess the plan was to have a strong separation between Marathon game code and the networking code,
// such that they could be compiled independently and only know about each other at link-time, but I don't see any
// reason to try to keep that... and I suspect Jason abandoned this separation long ago anyway.
// For now, the only effect I see is a reduction in type-safety.  :)
static void NetInitializeTopology(void *game_data, short game_data_size, void *player_data, short player_data_size);
static void NetLocalAddrBlock(NetAddrBlock *address, short socketNumber);

static int net_compare(void const *p1, void const *p2);

static void NetUpdateTopology(void);
static OSErr NetDistributeTopology(short tag);

static bool NetSetSelfSend(bool on);

static void NetDDPPacketHandler(DDPPacketBufferPtr inPacket);

static void *receive_stream_data(CommunicationsChannel *channel, size_t *length, OSErr *receive_error);
static OSErr send_stream_data(CommunicationsChannel *channel, void *data, size_t length);

// jkvw: moved from (now unused) network_streams.h

uint16 MaxStreamPacketLength(void);
uint16 NetStreamPacketLength(short packet_type);

enum {
	kNetworkTransportType= 0,
	kModemTransportType,
	NUMBER_OF_TRANSPORT_TYPES
};

static short transport_type= kNetworkTransportType;

short NetGetTransportType(
	void)
{
	return transport_type;
}
void NetSetTransportType(
	short type)
{
	assert(type>=0 && type<NUMBER_OF_TRANSPORT_TYPES);
#ifndef USE_MODEM
	assert(type==kNetworkTransportType);
#endif
	transport_type= type;
}

/* ---------- code */


/*
--------
NetEnter
--------

	(no parameters)

make sure the MPP and DSP drivers are open and remembers the DSP’s reference number in dspRefNum.
opens two connection end (upring and downring ring connections).

-------
NetExit
-------

	(no parameters)

frees memory, disposes our three connection ends, etc.

--------
NetState
--------

	<--- state of the network
*/

bool NetEnter(
	void)
{
	OSErr error;
	bool success= true; /* optimism */

#ifdef TEST_MODEM
	success= ModemEnter();
#else
	
	assert(netState==netUninitialized);

	/* if this is the first time we’ve been called, add NetExit to the list of cleanup procedures */
	{
		static bool added_exit_procedure= false;
		
		if (!added_exit_procedure) atexit(NetExit);
		added_exit_procedure= true;
	}

	// ZZZ: choose a game protocol
	sCurrentGameProtocol = (network_preferences->game_protocol == _network_game_protocol_star) ?
		static_cast<NetworkGameProtocol*>(&sStarGameProtocol) :
		static_cast<NetworkGameProtocol*>(&sRingGameProtocol);

	error= NetDDPOpen();
	if (!error)
	{
		if (!error)
		{
			topology = (NetTopologyPtr)malloc(sizeof(NetTopology));
			memset(topology, 0, sizeof(NetTopology));
			network_adsp_packet = (char *)malloc(MaxStreamPacketLength());
			memset(network_adsp_packet, 0, MaxStreamPacketLength());
			if (topology && network_adsp_packet)
			{
				/* Set the server player identifier */
				NetSetServerIdentifier(0);

				if (error==noErr)
				{
					// ZZZ: Sorry, if this swapping is not supported on all current A1
					// platforms, feel free to rewrite it in a way that is.
					ddpSocket= SDL_SwapBE16(GAME_PORT);
					error= NetDDPOpenSocket(&ddpSocket, NetDDPPacketHandler);
					if (error==noErr)
					{
						sOldSelfSendStatus= NetSetSelfSend(true);
						sServerPlayerIndex= 0;

						sCurrentGameProtocol->Enter(&netState);

						netState= netDown;
					}
				}
			}
		}
	}

	if (!inflater) {
		inflater = new MessageInflater ();
		for (int i = 0; i < NUMBER_OF_STREAM_PACKET_TYPES; i++) {
			BigChunkOfDataMessage *prototype = new BigChunkOfDataMessage (i);
			inflater->learnPrototypeForType (i, *prototype);
			delete prototype;
		}
	}
	
	next_join_attempt = machine_tick_count();

	/* Handle our own errors.. */
	if(error)
	{
		alert_user(infoError, strNETWORK_ERRORS, netErrCantContinue, error);
		NetExit();
		success= false;
	}
#endif
	
	return success;
}

void NetDoneGathering(void)
{
	if (server) {
		delete server;
		server = NULL;
	}
}

void NetExit(
	void)
{
	OSErr error = noErr;

#ifdef TEST_MODEM
	ModemExit();
#else

	sCurrentGameProtocol->Exit1();

        // ZZZ: clean up SDL Time Manager emulation.  true says wait for any late finishers to finish
        // (but does NOT say to kill anyone not already removed.)
        myTMCleanup(true);

	if (netState!=netUninitialized)
	{
		
		if (!error)
		{
			error= NetDDPCloseSocket(ddpSocket);
			vwarn(!error, csprintf(temporary, "NetDDPCloseSocket returned %d", error));
			if (!error)
			{
				NetSetSelfSend(sOldSelfSendStatus);

				free(topology);
				topology= NULL;

				sCurrentGameProtocol->Exit2();
				
				netState= netUninitialized;
			}
		}
	}

	

	if (connection_to_server) {
		delete connection_to_server;
		connection_to_server = NULL;
	}
	
	connection_map_t::iterator it;
	for (it = connections_to_clients.begin(); it != connections_to_clients.end(); it++)
		delete(it->second);
	connections_to_clients.clear();
	
	if (server) {
		delete server;
		server = NULL;
	}
	
	NetDDPClose();
#endif
}

bool
NetSync()
{
	return sCurrentGameProtocol->Sync(topology, dynamic_world->tick_count, localPlayerIndex, sServerPlayerIndex);
}



bool
NetUnSync()
{
	return sCurrentGameProtocol->UnSync(true, dynamic_world->tick_count);
}



/* Add a function for a distribution type. returns the type, or NONE if it can't be
 * installed. It's safe to call this function multiple times for the same proc. */
// ZZZ: changed to take in the desired type, so a given machine can handle perhaps only some
// of the distribution types.
void
NetAddDistributionFunction(int16 inDataTypeID, NetDistributionProc inProc, bool inLossy) {
    // We don't support lossless distribution yet.
    assert(inLossy);

    // Prepare a NetDistributionInfo with the desired data.
    NetDistributionInfo theInfo;
    theInfo.lossy               = inLossy;
    theInfo.distribution_proc   = inProc;

    // Insert or update a map entry
    distribution_info_map[inDataTypeID] = theInfo;
}


/* Remove a distribution proc that has been installed. */
void
NetRemoveDistributionFunction(int16 inDataTypeID) {
    distribution_info_map.erase(inDataTypeID);
}


const NetDistributionInfo*
NetGetDistributionInfoForType(int16 inType)
{
	distribution_info_map_t::const_iterator    theEntry = distribution_info_map.find(inType);
	if(theEntry != distribution_info_map.end())
		return &(theEntry->second);
	else
		return NULL;
}

	


void NetDistributeInformation(
                              short type,
                              void *buffer,
                              short buffer_size,
                              bool send_to_self)
{
	sCurrentGameProtocol->DistributeInformation(type, buffer, buffer_size, send_to_self);
}


short NetState(
	       void)
{
	return netState;
}


/*
 // ZZZ addition: 
void NetSetNetState(
	       short inNewState)
{
	assert(inNewState >= 0 && inNewState < NUMBER_OF_NET_STATES);
	netState= inNewState;
}
*/

/*
---------
NetGather
---------

	---> game data (pointer to typeless data no bigger than MAXIMUM_GAME_DATA_SIZE)
	---> size of game data
	---> player data of gathering player (no bigger than MAXIMUM_PLAYER_DATA_SIZE)
	---> size of player data
	---> lookupUpdateProc (we call NetLookupOpen() and NetLookupClose())
	
	<--- error

start gathering players.

---------------
NetGatherPlayer
---------------

	---> player index (into the array of looked up names)
	
	<--- error

bring the given player into our game.

---------------
NetCancelGather
---------------

	<--- error

tells all players in the game that the game has been cancelled.

--------
NetStart
--------

	<--- error

start the game with the existing topology (which all players should have)
*/

bool NetGather(
	void *game_data,
	short game_data_size,
	void *player_data,
	short player_data_size,
        bool resuming_game)
{
        resuming_saved_game = resuming_game;
        
#ifdef TEST_MODEM
	return ModemGather(game_data, game_data_size, player_data, player_data_size);
#else

	NetInitializeTopology(game_data, game_data_size, player_data, player_data_size);
	
	// Start listening for joiners
	server = new CommunicationsChannelFactory(GAME_PORT);
	
	netState= netGathering;
#endif
	
	return true;
}

void NetCancelGather(
	void)
{
#ifdef TEST_MODEM
	ModemCancelGather();
#else
	assert(netState==netGathering);

	NetDistributeTopology(tagCANCEL_GAME);
#endif
}

bool NetStart(
	void)
{
	OSErr error;
	bool success;

#ifdef TEST_MODEM
	success= ModemStart();
#else
	assert(netState==netGathering);

	// how about we sort the players before we pass them out to everyone?
	// This is an attempt to have a slightly more efficent ring in a multi-zone network.
	// we should really do some sort of pinging to determine an optimal order (or perhaps
	// sort on hop counts) but we'll just order them by their network numbers.
	// however, we need to leave the server player index 0 because we assume that the person
	// that starts out at index 0 is the server.
        
        // ZZZ: do that sorting in a new game only - in a resume game, the ordering is significant
        // (it's how netplayers will line up with existing saved-game players).
	
        if(!resuming_saved_game)
        {
                if (topology->player_count > 2)
                {
                        qsort(topology->players+1, topology->player_count-1, sizeof(struct NetPlayer), net_compare);
                }
        
                NetUpdateTopology();
        }

	error= NetDistributeTopology(resuming_saved_game ? tagRESUME_GAME : tagSTART_GAME);

	if(error)
	{
		alert_user(infoError, strNETWORK_ERRORS, netErrCouldntDistribute, error);
		success= false;
	} else {
		success= true;
	}
#endif

	return success;
}

static int net_compare(
	void const *p1, 
	void const *p2)
{
#ifndef NETWORK_IP
	return 0;
#else
	uint32 p1_host = SDL_SwapBE32(((const NetPlayer *)p1)->ddpAddress.host);
	uint32 p2_host = SDL_SwapBE32(((const NetPlayer *)p2)->ddpAddress.host);
	return p2_host - p1_host;
#endif
}

/*
-----------
NetGameJoin
-----------

	---> player name (to register)
	---> player type (to register)
	---> player data (no larger than MAXIMUM_PLAYER_DATA_SIZE)
	---> size of player data
	---> version number of network protocol (used with player type to construct entity name)
	---> host address
	
	<--- error

------------------
NetUpdateJoinState
------------------

	<--- new state (==netJoined,netWaiting,netStartingUp)

-------------
NetCancelJoin
-------------

	<--- error

can’t be called after the player has been gathered
*/

bool NetGameJoin(
	unsigned char *player_name,
	unsigned char *player_type,
	void *player_data,
	short player_data_size,
	short version_number,
	const char* host_addr_string
	)
{
	OSErr error = noErr;
	bool success= false;

#ifdef TEST_MODEM
	success= ModemGameJoin(player_name, player_type, player_data, player_data_size, version_number);
#else
	
	/* Attempt a connection to host */

	host_address_specified = (host_addr_string != NULL);
	if (host_address_specified)
	{
		// SDL_net declares ResolveAddress without "const" on the char; we can't guarantee
		// our caller that it will remain const unless we protect it like this.
		char*		theStringCopy = strdup(host_addr_string);
		error = SDLNet_ResolveHost(&host_address, theStringCopy, GAME_PORT);
		free(theStringCopy);
	}
	
	if (!error) {
		connection_to_server = new CommunicationsChannel();
		connection_to_server->setMessageInflater(inflater);
		netState = netConnecting;
		success = true;
	}
	
	if(error)
	{
		alert_user(infoError, strNETWORK_ERRORS, netErrCouldntJoin, error);
	} else {
		/* initialize default topology (no game data) */
		NetInitializeTopology((void *) NULL, 0, player_data, player_data_size);
	}
#endif
	
	return success;
}

void NetRetargetJoinAttempts(const IPaddress* inAddress)
{
	host_address_specified = (inAddress != NULL);
	if(host_address_specified)
	{
		host_address = *inAddress;
		host_address.port = SDL_SwapBE16(GAME_PORT);
	}
}

void NetCancelJoin(
	void)
{
#ifdef TEST_MODEM
	ModemCancelJoin();
#else
	assert(netState==netConnecting||netState==netJoining||netState==netWaiting||netState==netCancelled||netState==netJoinErrorOccurred);
#endif	
}

/* 
	Externally, this is only called before a new game.  It removes the reliance that 
	localPlayerIndex of zero is the server, which is not necessarily true if we are
	resyncing for another cooperative level.
*/
void NetSetServerIdentifier(
	short identifier)
{
	sServerPlayerIndex= identifier;
}


/*
net accessor functions
*/

short NetGetLocalPlayerIndex(
	void)
{
#ifdef TEST_MODEM
	return ModemGetLocalPlayerIndex();
#else
	assert(netState!=netUninitialized&&netState!=netDown&&netState!=netJoining);

	return localPlayerIndex;
#endif
}

short NetGetPlayerIdentifier(
	short player_index)
{
#ifdef TEST_MODEM
	return ModemGetPlayerIdentifier(player_index);
#else
	assert(netState!=netUninitialized&&netState!=netDown&&netState!=netJoining);
	assert(player_index>=0&&player_index<topology->player_count);
	
	return topology->players[player_index].identifier;
#endif
}

bool NetNumberOfPlayerIsValid(
	void)
{
	bool valid;

#ifdef TEST_MODEM
	valid= ModemNumberOfPlayerIsValid();
#else
	switch(netState)
	{
		case netUninitialized:
		case netJoining:
			valid= false;
			break;
		default:
			valid= true;
			break;
	}
#endif
	
	return valid;
}

short NetGetNumberOfPlayers(
	void)
{
#ifdef TEST_MODEM
	return ModemGetNumberOfPlayers();
#else
	assert(netState!=netUninitialized /* &&netState!=netDown*/ &&netState!=netJoining);
	
	return topology->player_count;
#endif
}

void *NetGetPlayerData(
	short player_index)
{
#ifdef TEST_MODEM
	return ModemGetPlayerData(player_index);
#else
	assert(netState!=netUninitialized/* && netState!=netDown */ &&netState!=netJoining);
	assert(player_index>=0&&player_index<topology->player_count);
	
	return topology->players[player_index].player_data;
#endif
}

void *NetGetGameData(
	void)
{
#ifdef TEST_MODEM
	return ModemGetGameData();
#else
	assert(netState!=netUninitialized&&netState!=netDown&&netState!=netJoining);
	
	return topology->game_data;
#endif
}

/* ZZZ addition:
--------------------------
NetSetupTopologyFromStarts
--------------------------

        ---> array of starts
        ---> count of starts
        
        This should be called once (at most) per game, after deciding who's going to take over which player,
        before calling NetStart().
*/

void NetSetupTopologyFromStarts(const player_start_data* inStartArray, short inStartCount)
{
	NetPlayer thePlayers[MAXIMUM_NUMBER_OF_NETWORK_PLAYERS];
        memcpy(thePlayers, topology->players, sizeof(thePlayers));
        for(int s = 0; s < inStartCount; s++)
        {
                if(inStartArray[s].identifier == NONE)
                {
                        // Is this really all I have to do here?
                        // NO, I need to set up the player name, color, team, etc. for transmission to others.
                        // That requires knowledge of the player_info or player_data (whichever one the net system
                        // uses for such transmission.)
                        topology->players[s].identifier = NONE;
                        // XXX ZZZ violation of separation of church and state - oops I mean net code and game code
                        player_info* thePlayerInfo = (player_info*)topology->players[s].player_data;
                        // XXX ZZZ faux security - we strncpy but the following line assumes the length is within range (i.e. that there's a null)
                        strncpy((char*)&(thePlayerInfo->name[1]), inStartArray[s].name, MAX_NET_PLAYER_NAME_LENGTH);
                        thePlayerInfo->name[0] = strlen(inStartArray[s].name);
                        thePlayerInfo->desired_color = 0; // currently unused
                        thePlayerInfo->team = inStartArray[s].team;
                        thePlayerInfo->color = inStartArray[s].color;
                        memset(thePlayerInfo->long_serial_number, 0, LONG_SERIAL_NUMBER_LENGTH);
                        //topology->nextIdentifier++;
                }
                else
                {
                        int p;
                        for(p = 0; p < topology->player_count; p++)
                        {
                                if(thePlayers[p].identifier == inStartArray[s].identifier)
                                        break;
                        }
                        
                        assert(p != topology->player_count);
                        
                        topology->players[s] = thePlayers[p];
                }
        }
        
        topology->player_count = inStartCount;
        
        NetUpdateTopology();
}

/*
------------------
NetEntityNotInGame
------------------

	---> entity
	---> address
	
	<--- true if the entity is not in the game, false otherwise

used to filter entities which have been added to a game out of the lookup list
*/

/* if the given address is already added to our game, filter it out of the gather dialog */
bool NetEntityNotInGame(
	NetEntityName *entity,
	NetAddrBlock *address)
{
	short player_index;
	bool valid= true;
	
	(void) (entity);
	
	for (player_index=0;player_index<topology->player_count;++player_index)
	{
		NetAddrBlock *player_address= &topology->players[player_index].dspAddress;
		
#ifndef NETWORK_IP
#ifdef CLASSIC_MAC_NETWORKING
		if (address->aNode==player_address->aNode && address->aSocket==player_address->aSocket &&
			address->aNet==player_address->aNet)
#endif
#else
		if (address->host == player_address->host && address->port == player_address->port)
#endif
		{
			valid= false;
			break;
		}
	}
	
	return valid;
}




/* ---------- private code */

void
NetDDPPacketHandler(DDPPacketBufferPtr packet)
{
	sCurrentGameProtocol->PacketHandler(packet);
}



/*
local player initializers
*/

static void NetInitializeTopology(
	void *game_data,
	short game_data_size,
	void *player_data,
	short player_data_size)
{
	NetPlayerPtr local_player;
	
	assert(player_data_size>=0&&player_data_size<MAXIMUM_PLAYER_DATA_SIZE);
	assert(game_data_size>=0&&game_data_size<MAXIMUM_GAME_DATA_SIZE);

	/* initialize the local player (assume we’re index zero, identifier zero) */
	localPlayerIndex= localPlayerIdentifier= 0;
	local_player= topology->players + localPlayerIndex;
	local_player->identifier= localPlayerIdentifier;
	local_player->net_dead= false;

	if(NetGetTransportType()!=kModemTransportType)
	{
		NetLocalAddrBlock(&local_player->dspAddress, GAME_PORT);
		NetLocalAddrBlock(&local_player->ddpAddress, ddpSocket);
	}
	memcpy(local_player->player_data, player_data, player_data_size);
	
	/* initialize the network topology (assume we’re the only player) */
	topology->player_count= 1;
	topology->nextIdentifier= 1;
	memcpy(topology->game_data, game_data, game_data_size);
}

static void NetLocalAddrBlock(
	NetAddrBlock *address,
	short socketNumber)
{
	
#ifndef NETWORK_IP
#ifdef CLASSIC_MAC_NETWORKING
	short node, network;

	GetNodeAddress(&node, &network);
	
	address->aSocket= socketNumber;
	address->aNode= node;
	address->aNet= network;
#endif
#else
	address->host = 0x7f000001;	//!! XXX (ZZZ) yeah, that's really bad.
	address->port = socketNumber;	// OTOH, I guess others are set up to "stuff" the address they actually saw for us instead of
#endif					// this, anyway... right??  So maybe it's not that big a deal.......
}



static void NetUpdateTopology(
	void)
{
	/* recalculate localPlayerIndex */					
	for (localPlayerIndex=0;localPlayerIndex<topology->player_count;++localPlayerIndex)
	{
		if (topology->players[localPlayerIndex].identifier==localPlayerIdentifier) break;
	}
#ifdef DEBUG
	if (localPlayerIndex==topology->player_count) fdprintf("couldn’t find my identifier: %p", topology);
#endif
}



static bool NetSetSelfSend(
                           bool on)
{
#ifndef NETWORK_IP
        OSErr          err;
        MPPParamBlock  pb;

        pb.SETSELF.newSelfFlag = on;
        err = PSetSelfSend(&pb, false);
        assert(err == noErr);
        return pb.SETSELF.oldSelfFlag;
#else
        return false;
#endif
}



/* ------ this needs to let the gatherer keep going if there was an error.. */
/* ••• Marathon Specific Code ••• */
/* Returns error code.. */
// ZZZ annotation: this function doesn't seem to belong here - maybe more like interface.cpp?
bool NetChangeMap(
	struct entry_point *entry)
{
	byte   *wad= NULL;
	long   length;
	OSErr  error= noErr;
	bool success= true;

#ifdef TEST_MODEM
	success= ModemChangeMap(entry);
#else

	/* If the guy that was the server died, and we are trying to change levels, we lose */
        // ZZZ: if we used the parent_wad_checksum stuff to locate the containing Map file,
        // this would be the case somewhat less frequently, probably...
	if(localPlayerIndex==sServerPlayerIndex && localPlayerIndex != 0)
	{
#ifdef DEBUG_NET
		fdprintf("Server died, and trying to get another level. You lose;g");
#endif
		success= false;
		set_game_error(gameError, errServerDied);
	}
	else
	{
		// being the server, we must send out the map to everyone.	
		if(localPlayerIndex==sServerPlayerIndex) 
		{
			wad= (unsigned char *)get_map_for_net_transfer(entry);
			if(wad)
			{
				length= get_net_map_data_length(wad);
				error= NetDistributeGameDataToAllPlayers(wad, length, true);
				if(error) success= false;
				set_game_error(systemError, error);
			} else {
//				if (!wad) alert_user(fatalError, strERRORS, badReadMap, -1);
				assert(error_pending());
			}
		} 
		else // wait for de damn map.
		{
			wad= NetReceiveGameData(true);
			if(!wad) success= false;
			// Note that NetReceiveMap handles display of its own errors, therefore we don't
			//  assert that an error is pending.....
		}
	
		/* Now load the level.. */
		if (!error && wad)
		{
			/* Note that this frees the wad as well!! */
			process_net_map_data(wad);
		}
	}
#endif
	
	return success;
}

void DeferredScriptSend (byte* data, size_t length)
{
        if (deferred_script_data != NULL) {
            delete [] deferred_script_data;
            deferred_script_data = NULL;
        }
        deferred_script_data = data;
        deferred_script_length = length;
}

void SetNetscriptStatus (bool status)
{
        do_netscript = status;
}

// ZZZ this "ought" to distribute to all players simultaneously (by interleaving send calls)
// in case the server bandwidth is much greater than the others' bandwidths.  But that would
// take a fair amount of reworking of the streaming system, which only groks talking with one
// machine at a time.
OSErr NetDistributeGameDataToAllPlayers(
	byte *wad_buffer, 
	long wad_length,
        bool do_physics)
{
	short playerIndex, message_id;
	OSErr error= noErr;
	long total_length, length_written;
	uint32 initial_ticks= machine_tick_count();
	short physics_message_id;
	byte *physics_buffer;
	long physics_length;
        
	message_id= (topology->player_count==2) ? (_distribute_map_single) : (_distribute_map_multiple);
	physics_message_id= (topology->player_count==2) ? (_distribute_physics_single) : (_distribute_physics_multiple);
	open_progress_dialog(physics_message_id);

	/* For updating our progress bar.. */
	total_length= (topology->player_count-1)*wad_length;
	length_written= 0l;

	/* Get the physics */
        if(do_physics)
                physics_buffer= (unsigned char *)get_network_physics_buffer(&physics_length);

	// go ahead and transfer the map to each player
	for (playerIndex= 0; !error && playerIndex<topology->player_count; playerIndex++)
	{
		/* If the player is not net dead. */ // ZZZ: and is not going to be a zombie and is not us
		if(!topology->players[playerIndex].net_dead && topology->players[playerIndex].identifier != NONE && playerIndex != localPlayerIndex)
		{

			/* Send the physics.. */
			set_progress_dialog_message(physics_message_id);

			if (!error)
			{
#ifdef NO_PHYSICS
				if(do_physics)
                                        error= send_stream_data(connections_to_clients[topology->players[playerIndex].stream_id], physics_buffer, physics_length);
#endif

				if(!error)
				{
					set_progress_dialog_message(message_id);
					reset_progress_bar(); /* Reset the progress bar */
					error= send_stream_data(connections_to_clients[topology->players[playerIndex].stream_id], wad_buffer, wad_length);
				}

                                if(!error && do_netscript)
                                {
					reset_progress_bar ();
					error = send_stream_data (connections_to_clients[topology->players[playerIndex].stream_id], deferred_script_data, deferred_script_length);
				}

			}
                        
		}
		
		if (error)
		{
			alert_user(infoError, strNETWORK_ERRORS, netErrCouldntDistribute, error);
		} 
		else if  (machine_tick_count()-initial_ticks>uint32(topology->player_count*MAP_TRANSFER_TIME_OUT))
		{
			alert_user(infoError, strNETWORK_ERRORS, netErrWaitedTooLongForMap, error);
			error= 1;
		}
	}
	
	/* Fill the progress bar.. */
	if (!error) 
	{
		/* Process the physics file & frees it!.. */
                if(do_physics)
                        process_network_physics_model(physics_buffer);
                
		draw_progress_bar(total_length, total_length);

#ifdef HAVE_LUA
                if (do_netscript)
                        LoadLuaScript ((char*)deferred_script_data, deferred_script_length);
#endif
	}

	close_progress_dialog();
	
	return error;
}

byte *NetReceiveGameData(bool do_physics)
{
	byte *map_buffer= NULL;
	size_t map_length;
	uint32 ticks;
	OSErr error= noErr;
	bool timed_out= false;

        byte *script_buffer = NULL;
        size_t script_length;
        
	open_progress_dialog(_awaiting_map);

	// wait for our connection to start up. server will contact us.
	ticks= machine_tick_count();
	
	while (!connection_to_server->isMessageAvailable() && !timed_out)
	{
		if((machine_tick_count()-ticks)>MAP_TRANSFER_TIME_OUT)  timed_out= true;
	}
	
	if (timed_out)
	{
                close_progress_dialog();
			alert_user(infoError, strNETWORK_ERRORS, netErrWaitedTooLongForMap, 0);
	} 
	else
	{
		byte *physics_buffer= NULL;
		size_t physics_length;

		/* Receiving map.. */
		set_progress_dialog_message(_receiving_physics);

#ifdef NO_PHYSICS
                if(do_physics)
                        physics_buffer= (unsigned char *)receive_stream_data(connection_to_server, &physics_length, &error);
#endif

		if(!error)
		{
			/* Process the physics file & frees it!.. */
                        if(do_physics)
                                process_network_physics_model(physics_buffer);

			/* receiving the map.. */
			set_progress_dialog_message(_receiving_map);
			reset_progress_bar(); /* Reset the progress bar */
			map_buffer= (unsigned char *)receive_stream_data(connection_to_server, &map_length, &error);
		}
                
#ifdef HAVE_LUA
                if (!error && do_netscript)
                {
                        reset_progress_bar(); /* Reset the progress bar */
			script_buffer = (byte*)receive_stream_data(connection_to_server, &script_length, &error);
                        LoadLuaScript ((char*)script_buffer, script_length);
                }
#endif

		/* And close our dialog.. */
		draw_progress_bar(10, 10);
		close_progress_dialog();
	
		if (error != noErr || map_buffer == NULL)
		{
			if(error)
			{
				alert_user(infoError, strNETWORK_ERRORS, netErrMapDistribFailed, error);
			} else {
#ifdef mac
				alert_user(infoError, strERRORS, outOfMemory, MemError());
#else
				alert_user(infoError, strERRORS, outOfMemory, -1);
#endif
			}
	
			if(map_buffer)
			{
				delete []map_buffer;
				map_buffer= NULL;
			}
		}
	}
	
	return map_buffer;
}

void NetSetInitialParameters(
	short updates_per_packet, 
	short update_latency)
{
//	initial_updates_per_packet= updates_per_packet;
//	initial_update_latency= update_latency;
}

long
NetGetNetTime(void)
{
        return sCurrentGameProtocol->GetNetTime();
}



short
get_network_version()
{
	// This number needs to be changed whenever a change occurs in the networking code
	// that would make 2 versions incompatible, or a change in the game occurs that
	// would make 2 versions out of sync.
	// ZZZ: I have made efforts to preserve existing "classic" Mac OS protocol and data formats,
	// but IPring introduces new _NET formats and so needs an increment.
	// (OK OK this is a bit pedantic - I mean, I don't think IPring is going to accidentally start
	// sending or receiving AppleTalk traffic ;) - but, you know, it's the principle of the thing.)
#if HAVE_SDL_NET
	// Ring is 12; star is 13.
	return (network_preferences->game_protocol == _network_game_protocol_ring) ? _ip_ring_network_version : _ip_star_network_version;
#else
	return _appletalk_ring_network_version;
#endif
}


static void
NetSendPacket (CommunicationsChannel *channel, short packet_type, const void *packet_data)
{
	BigChunkOfDataMessage *message;
	
	message = new BigChunkOfDataMessage(packet_type, (const Uint8*)packet_data, NetStreamPacketLength(packet_type));
	channel->enqueueOutgoingMessage(*message);
	channel->flushOutgoingMessages(false, 30000, 30000);
	delete message;
}

static bool
NetReceivePacket (CommunicationsChannel *channel, short &packet_type, void *packet_data, bool wait)
{
	Message *premessage;
	
	int timeout = wait ? 30000 : 0;
	
	premessage = channel->receiveMessage(timeout, timeout);
	if (!premessage)
		return false;
	
	if (premessage->type() == 0xffff) {
		UninflatedMessage *message = dynamic_cast<UninflatedMessage*>(premessage);
		uint16 bad_packet_type = message->inflatedType();
		NetSendPacket (channel, _unknown_packet_type_response_packet, &bad_packet_type);
		delete message;
		return false;
	} else {
		BigChunkOfDataMessage *message = dynamic_cast<BigChunkOfDataMessage*>(premessage);
		packet_type = message->type();
		memcpy(packet_data, message->buffer(), NetStreamPacketLength(packet_type));
		delete message;
		return true;
	}
}


	
static void *receive_stream_data(
	CommunicationsChannel *channel,
	size_t *length,
	OSErr *receive_error)
{
	OSErr error = noErr;
	short packet_type;
	void *buffer= NULL;

	// first we'll get the map length
	if (NetReceivePacket(channel, packet_type, network_adsp_packet, true))
	{
		if(packet_type==_stream_size_packet)
		{
			// ZZZ: byte-swap if necessary
			long length_NET = *((long*) network_adsp_packet);

#if HAVE_SDL_NET
			*length = SDL_SwapBE32(length_NET);
#else
			*length = length_NET;
#endif

			if(*length)
			{
				buffer = new byte[*length]; // because my caller expects it to be portable.

				if (buffer)
				{
					// we transfer the map in chunks, since ADSP can only transfer 64K at a time.
					for (size_t offset = 0; !error && offset < *length; offset += STREAM_TRANSFER_CHUNK_SIZE)
					{
						size_t expected_count;

						expected_count = MIN(STREAM_TRANSFER_CHUNK_SIZE, *length - offset);

						if (NetReceivePacket(channel, packet_type, network_adsp_packet, true))
						{
							if(packet_type != _stream_data_packet)
							{
								error= errInvalidMapPacket;
							} else {
								/* Copy the data in.  This is done in two steps because the final */
								/*  packet would overrite the map buffer, unless it was perfectly */
								/*  a multiple of the STREAM_TRANSFER_CHUNK_SIZE */
								memcpy(((byte *)buffer)+offset, network_adsp_packet, expected_count);
							}
							draw_progress_bar(offset, *length);
						} else {
							// probably should error here :)
						}
					}
				}
			}
		}
	}
	
	*receive_error= error;
		
	return buffer;
} // receive_stream_data

static OSErr send_stream_data(
	CommunicationsChannel* channel,
	void *data,
	size_t length)
{
	OSErr error = noErr;

	// transfer the length of the level.
        // ZZZ: byte-swap if necessary
        long	length_NET;

#if HAVE_SDL_NET
        length_NET = SDL_SwapBE32((long)length);
#else
        length_NET = (long)length;
#endif

	NetSendPacket(channel, _stream_size_packet, &length_NET);

	if(!error)
	{
		size_t offset, length_written= 0;
	
		// ready or not, here it comes, in smaller chunks
		for (offset = 0; !error && offset < length; offset += STREAM_TRANSFER_CHUNK_SIZE)
		{
			size_t adsp_count;
			
			adsp_count = MIN(STREAM_TRANSFER_CHUNK_SIZE, length - offset);

			/* Note that the last time through this we will have garbage at the */
			/*  end of the packet, but it doesn't matter since we know the length */
			/*  of the map. */
			memcpy(network_adsp_packet, ((byte *) data)+offset, adsp_count);
			NetSendPacket(channel, _stream_data_packet, network_adsp_packet);
			
			length_written+= adsp_count;
			draw_progress_bar(length_written, length);
		}
	}

	return error;
} // send_stream_data


        
#ifdef NETWORK_CHAT
// Core logic brazenly ripped off NetDistributeTopology :)
OSErr NetDistributeChatMessage(
	short sender_identifier, const char* message)
{
	OSErr error = 0;
	short playerIndex;
	
	assert(netState==netGathering);
	
        NetChatMessage		theChatMessage;
        NetChatMessage_NET	theChatMessage_NET;
        
        theChatMessage.sender_identifier = sender_identifier;
        strncpy(theChatMessage.text, message, CHAT_MESSAGE_TEXT_BUFFER_SIZE);
        
        netcpy(&theChatMessage_NET, &theChatMessage);

	for (playerIndex=0; playerIndex<topology->player_count; ++playerIndex)
	{
                // ZZZ: skip players with identifier NONE - they don't really exist... also skip ourselves
                if(topology->players[playerIndex].identifier != NONE || playerIndex == localPlayerIndex)
			NetSendPacket(connections_to_clients[topology->players[playerIndex].stream_id], _chat_packet, &theChatMessage_NET);
	}
	
	return error;
}

bool
NetGetMostRecentChatMessage(player_info** outPlayerData, char** outMessage) {
    if(new_incoming_chat_message) {
        player_info* thePlayer = NULL;
        new_incoming_chat_message = false;
        
        for(int i = 0; i < NetGetNumberOfPlayers(); i++)
            if(NetGetPlayerIdentifier(i) == incoming_chat_message_buffer.sender_identifier)
                thePlayer = (player_info*) NetGetPlayerData(i);

        if(thePlayer != NULL) {
            *outPlayerData = thePlayer;
            *outMessage = incoming_chat_message_buffer.text;
            return true;
        }
    }

    return false;
}
#endif // NETWORK_CHAT

// If a potential joiner has connected to us, handle em
bool NetCheckForNewJoiner (prospective_joiner_info &info)
{
	OSErr error = noErr;
	short packet_type;
	
	CommunicationsChannel *new_joiner = server->newIncomingConnection();
	
	if (new_joiner) {
		
		new_joiner->setMessageInflater(inflater);
		
		int no_data_to_send_silly;
		NetSendPacket (new_joiner, _hello_packet, &no_data_to_send_silly);
		
		if (NetReceivePacket (new_joiner, packet_type, network_adsp_packet, true)) {
			if (packet_type != _joiner_info_packet)
				error = 1; // wrong response
		} else {
			error = 1; // timed out
		}
		
		if (!error) { 
			netcpy(&info,(prospective_joiner_info_NET*)network_adsp_packet);
			if (info.network_version == get_network_version()) {
				info.stream_id = next_stream_id;
				connections_to_clients[next_stream_id]=new_joiner;
				next_stream_id++;
			} else {
				error = 1;
			}
		}
		
		if (error)
			delete new_joiner;
		else
			return true;
	}
	return false;
}

/* check for messages from gather nodes; returns new state */
short NetUpdateJoinState(
	void)
{
        logContext("updating network join status");

	OSErr error = noErr;
	short newState= netState;
	short packet_type;

#ifdef TEST_MODEM
	newState= ModemUpdateJoinState();
#else
	
	switch (netState)
	{
		case netConnecting:	// trying to connect to gatherer
			// jkvw: Here's what's up - We want to be able to click join before gatherer
			//	 has started gathering.  So after a join click we attempt a connection
			//	 every five seconds.  This is fine so long as remote computer quickly
			//	 refuses connection each time we poll; the dialog will remain responsive.
			//	 It's possible that a connection attempt will take a long time, however,
			//	 so we look for that and abort dialog when it happens.
			if (machine_tick_count() >= next_join_attempt) {
				uint32 ticks_before_connection_attempt = machine_tick_count();
				if(host_address_specified)
					connection_to_server->connect(host_address);
				if (connection_to_server->isConnected())
					newState = netJoining;
				else if (ticks_before_connection_attempt + 3*MACHINE_TICKS_PER_SECOND < machine_tick_count ()) {
					newState= netJoinErrorOccurred;
					alert_user(infoError, strNETWORK_ERRORS, netErrCouldntJoin, 3);
				}
				next_join_attempt = machine_tick_count() + 5*MACHINE_TICKS_PER_SECOND;
			}
			break;

		case netJoining:	// waiting to be gathered
			if (!connection_to_server->isConnected ()) {
				newState = netJoinErrorOccurred;
				alert_user(infoError, strNETWORK_ERRORS, netErrLostConnection, 0);
			} else if (NetReceivePacket(connection_to_server, packet_type, network_adsp_packet, false))
			{
				if(!error && packet_type==_hello_packet)
				{
					prospective_joiner_info my_info;
					prospective_joiner_info_NET my_info_NET;
					pstrcpy((unsigned char*)my_info.name,player_preferences->name);
					my_info.network_version = get_network_version();
					netcpy(&my_info_NET, &my_info);
					NetSendPacket(connection_to_server, _joiner_info_packet, &my_info_NET);
				}
				
				if(!error && packet_type==_script_packet)
                                {	// Gatherer wants to know if we can handle net scripts before he commits to gathering us
                                	int16 message_in, message_out;
                                        message_in = *(int16*)network_adsp_packet;
                                        message_in = SDL_SwapBE16(message_in);
                                        if (message_in == _netscript_query_message)
                                        {
#ifdef HAVE_LUA
                                        	message_out = _netscript_yes_script_message;
#else
                                                message_out = _netscript_no_script_message;
#endif
                                                message_out = SDL_SwapBE16(message_out);
                                                NetSendPacket(connection_to_server, _script_packet, &message_out);
                                        } else {
                                                // Must be newer build asking for net script functionality we can't handle
                                                message_out = _netscript_no_script_message;
                                                message_out = SDL_SwapBE16(message_out);
                                                NetSendPacket(connection_to_server, _script_packet, &message_out);
                                        }
                                }

				if(!error && packet_type==_join_player_packet)
				{
					/* NOTE THESE ARE SHARED! */

                                        gather_player_data_NET* gathering_data_NET = (gather_player_data_NET*) network_adsp_packet;
                                        gather_player_data	gathering_data_storage;
                                        gather_player_data*	gathering_data = &gathering_data_storage;
                                        netcpy(gathering_data, gathering_data_NET);
                                        
                                        accept_gather_data	new_player_data;
                                        accept_gather_data_NET	new_player_data_NET;

					/* Note that we could set accepted to false if we wanted to for some */
					/*  reason- such as bad serial numbers.... */
                                
                                        SetNetscriptStatus (false); // Unless told otherwise, we don't expect a netscript
                                        
                                        /* Note that we share the buffers.. */
                                        localPlayerIdentifier= gathering_data->new_local_player_identifier;
                                        topology->players[localPlayerIndex].identifier= localPlayerIdentifier;
                                        topology->players[localPlayerIndex].net_dead= false;
                                        
                                        /* Confirm. */
                                        new_player_data.accepted= kStateOfTheArtJoinerAccepted;
                                        obj_copy(new_player_data.player, topology->players[localPlayerIndex]);

                                        netcpy(&new_player_data_NET, &new_player_data);
                                        NetSendPacket(connection_to_server, _accept_join_packet, &new_player_data_NET);

					if(new_player_data.accepted)
						newState= netWaiting;
				} 

				if (error != noErr)
				{
                                        logAnomaly1("error != noErr; error == %d", error);
                                
					newState= netJoinErrorOccurred;
					// no way to get here
					alert_user(infoError, strNETWORK_ERRORS, netErrJoinFailed, error);
				}
			}
			break;
                    // netJoining
		
		case netWaiting:	// have been gathered, waiting for other players / game start
			if (!connection_to_server->isConnected ()) {
				newState = netJoinErrorOccurred;
				alert_user(infoError, strNETWORK_ERRORS, netErrLostConnection, 0);
			} else if (NetReceivePacket(connection_to_server, packet_type, network_adsp_packet, false))
			{
				/* and now, the packet you’ve all been waiting for ... (the server is trying to
					hook us up with the network topology) */
				if(!error)
				{	// ZZZ change to accept more kinds of packets here
                                    switch(packet_type) {
                                    
                                    case _script_packet:
                                    {	// Gatherer wants to tell us to expect script goodness
                                	int16 message_in;
                                        message_in = *(int16*)network_adsp_packet;
                                        message_in = SDL_SwapBE16(message_in);
                                        if (message_in == _netscript_script_intent_message)
                                            SetNetscriptStatus (true);
                                    }
                                    break;
                                    
                                    case _topology_packet:

					/* Copy it in */
                                        netcpy(topology, (NetTopology_NET*)network_adsp_packet);

					if(NetGetTransportType()==kNetworkTransportType)
					{
						NetAddrBlock address;
						
						// LP: NetAddrBlock is the trouble here
						#ifdef NETWORK_IP
						address = connection_to_server->peerAddress();
						#endif
						
                                                // ZZZ: the code below used to assume the server was _index_ 0; now, we merely
                                                // assume the server has _identifier_ 0.
                                                int theServerIndex;
                                                for(theServerIndex = 0; theServerIndex < topology->player_count; theServerIndex++)
                                                {
                                                        if(topology->players[theServerIndex].identifier == 0) break;
                                                }
                                                
                                                assert(theServerIndex != topology->player_count);
                                                
						/* for ARA, make stuff in an address we know is correct (don’t believe the server) */
						topology->players[theServerIndex].dspAddress= address;
#ifndef NETWORK_IP
#ifdef CLASSIC_MAC_NETWORKING
						topology->players[theServerIndex].ddpAddress.aNet= address.aNet;
						topology->players[theServerIndex].ddpAddress.aNode= address.aNode;
#endif
#else
						topology->players[theServerIndex].ddpAddress.host = address.host;
#endif
//						fdprintf("ddp %8x, dsp %8x;g;", *((long*)&topology->players[0].ddpAddress),
//							*((long*)&topology->players[0].dspAddress));
					}

					NetUpdateTopology();
				
					switch (topology->tag)
					{
						case tagNEW_PLAYER:
							newState= netPlayerAdded;
							break;
							
						case tagCANCEL_GAME:
							newState= netCancelled;
							alert_user(infoError, strNETWORK_ERRORS, netErrServerCanceled, 0);
							break;
							
						case tagSTART_GAME:
							newState= netStartingUp;
                                                        resuming_saved_game = false;
							break;
                                                
                                                // ZZZ addition
                                                case tagRESUME_GAME:
                                                        newState = netStartingResumeGame;
                                                        resuming_saved_game = true;
                                                        break;
							
						default:
							break;
					}
					
					if (error)
					{
						newState= netJoinErrorOccurred;
						alert_user(infoError, strNETWORK_ERRORS, netErrJoinFailed, error);
					}
                                    break;
                                    // _topology_packet

                                    // ZZZ addition
                                    case _chat_packet:
                                        netcpy(&incoming_chat_message_buffer, (NetChatMessage_NET*) network_adsp_packet);
                                        
							/* successfully got a chat message from the server */
                                                        newState = netChatMessageReceived;
                                                        new_incoming_chat_message = true;
					
                                    break;
                                    // _chat_packet

                                    default:	// unrecognized stream packet type

                                    break;
                                    
                                    } // switch(packet_type)
				}
				else	// error
				{
					newState= netJoinErrorOccurred;
					// jkvw: It's not possible to get here right now
					alert_user(infoError, strNETWORK_ERRORS, netErrJoinFailed, error);
				}
			}
			break;
                    // netWaiting
                    
		default:
			newState= NONE;
			break;
	}
	
	/* return netPlayerAdded to tell the caller to refresh his topology, but don’t change netState to that */
        // ZZZ: similar behavior for netChatMessageReceived and netStartingResumeGame
	if (newState!=netPlayerAdded && newState != netChatMessageReceived && newState != netStartingResumeGame && newState != NONE)
                netState= newState;
        
        // ZZZ: netStartingResumeGame is used as a return value only; the corresponding netState is netStartingUp.
        if(newState == netStartingResumeGame)
                netState = netStartingUp;
#endif // !TEST_MODEM

	return newState;
}



// ZZZ: hindsight is 20/20 - probably we should just return the joiner's acceptance number and let higher-level
// code figure out whether that number is acceptable, etc.
int NetGatherPlayer(
#if !HAVE_SDL_NET
        /* player_index in our lookup list */
	short player_index,
#else
        prospective_joiner_info &player,
#endif
	CheckPlayerProcPtr check_player)
{
	OSErr error = noErr;
	NetAddrBlock address;
	short packet_type;
	short stream_transport_type= NetGetTransportType();
	int theResult = kGatherPlayerSuccessful;
	bool preGatherRejection = false;

#ifdef TEST_MODEM
	success= ModemGatherPlayer(player_index, check_player);
#else
	assert(netState==netGathering);
	assert(topology->player_count<MAXIMUM_NUMBER_OF_NETWORK_PLAYERS);
	
	/* Get the address from the dialog */
#if !HAVE_SDL_NET
	// LP: NetAddrBlock is the trouble here
	#ifndef mac
	NetLookupInformation(player_index, &address, NULL);
	#endif
#else
	address = connections_to_clients[player.stream_id]->peerAddress();
#endif

	/* Note that the address will be garbage for modem, but that's okay.. */
	/* Force the address to be correct, so we can use our stream system.. */
	topology->players[topology->player_count].dspAddress= address;
#ifndef NETWORK_IP
#ifdef CLASSIC_MAC_NETWORKING
	topology->players[topology->player_count].ddpAddress.aNet= address.aNet;
	topology->players[topology->player_count].ddpAddress.aNode= address.aNode;
#endif
#else
	topology->players[topology->player_count].ddpAddress.host = address.host;
#endif

        int thePlayerAcceptNumber = 0;
	
        // reject a player if e can't handle our script demands
        if (!error && do_netscript)
        {
                int16 message_out = _netscript_query_message;
                message_out = SDL_SwapBE16(message_out);

                NetSendPacket(connections_to_clients[player.stream_id], _script_packet, &message_out);
                if (!error)
                {
                        short packet_type;
                        if (NetReceivePacket(connections_to_clients[player.stream_id], packet_type, network_adsp_packet, true))
                        {
                                int16 message_in;
                        	switch(packet_type)
                                {
                                        case _script_packet:
                                                message_in = *((int16*) network_adsp_packet);
                                                message_in = SDL_SwapBE16(message_in);
                                                if (message_in == _netscript_yes_script_message)
                                                        ; // accept
                                                else
                                                        preGatherRejection = true; // reject - e probably built without HAVE_LUA
                                                break;
                                        case _unknown_packet_type_response_packet:
                                        default:
                                                        preGatherRejection = true; // reject - old build
                                                break;
                                }
                                
                        } else {
				error = 1; // Didn't receive response to stream query
			}
                }
        }
        
        if (preGatherRejection)
        {
                alert_user(infoError, strNETWORK_ERRORS, netErrUngatheredPlayerUnacceptable, 0);
                theResult = kGatherPlayerFailed;
        }
        
	if (!error && !preGatherRejection)
	{
		struct gather_player_data gather_data;
                gather_player_data_NET	gather_data_NET;

		/* Setup the gather data. */
		gather_data.new_local_player_identifier= topology->nextIdentifier;

                netcpy(&gather_data_NET, &gather_data);
                NetSendPacket(connections_to_clients[player.stream_id], _join_player_packet, &gather_data_NET);

		if(!error)
		{
			if (NetReceivePacket(connections_to_clients[player.stream_id], packet_type, network_adsp_packet, true))
			{
				if(packet_type==_accept_join_packet)
				{
                                        accept_gather_data_NET* new_player_data_NET = (accept_gather_data_NET*) network_adsp_packet;
                                        accept_gather_data	new_player_data_storage;
                                        accept_gather_data*	new_player_data = &new_player_data_storage;
                                        netcpy(new_player_data, new_player_data_NET);
                                        
                                        thePlayerAcceptNumber = new_player_data->accepted;

					if(new_player_data->accepted)
					{
						// jkvw: Here we used to check if we had gathered an unacceptable player.
						//	 There's no need to perform the check any more, however,
						//	 since the old builds aren't going to interoperate with
						//	 the new firewall-friendly builds anyway.
						//
						//	 If we have need for a new interoperability check, I strongly
						//	 reccomend doing it before commiting to the gather, similar to the check
						//	 for lua capability.
                                                
						/* make sure everybody gets a unique identifier */
						topology->nextIdentifier+= 1;

						/* Copy in the player data */
						obj_copy(topology->players[topology->player_count], new_player_data->player);
						
						/* this is how gatherer remembers how to contact joiners */
						topology->players[topology->player_count].stream_id = player.stream_id;
						
						/* force in some addresses we know are correct */
						topology->players[topology->player_count].dspAddress= address;
#ifndef NETWORK_IP
#ifdef CLASSIC_MAC_NETWORKING
						topology->players[topology->player_count].ddpAddress.aNet= address.aNet;
						topology->players[topology->player_count].ddpAddress.aNode= address.aNode;
#endif
#else
						topology->players[topology->player_count].ddpAddress.host = address.host;
#endif
//						fdprintf("ddp %8x, dsp %8x;g;", *((long*)&topology->players[topology->player_count].ddpAddress),
//							*((long*)&topology->players[topology->player_count].dspAddress));
                                                    
                                                if (!error)
                                                {
                                                        /* remove this player from the list of players so we can’t even try to add him again */
                                                        if(stream_transport_type!=kModemTransportType)
                                                        {
// ZZZ: in my formulation, entry is removed from list instantly by widget when clicked
#if !HAVE_SDL_NET
                                                                NetLookupRemove(player_index);
#endif
                                                        }

                                                        /* successfully added a player */
                                                        topology->player_count+= 1;
						
                                                        check_player(topology->player_count-1, topology->player_count);
	
                                                        /* recalculate our local information */
                                                        NetUpdateTopology();
						
                                                        /* distribute this new topology with the new player tag */
                                                        /* There is no reason to check the error here, because we can't do */
                                                        /* anything about it.. */
                                                        NetDistributeTopology(tagNEW_PLAYER);
                                                }
					}
					else 
					{
						// joiner did not accept
						error = 1;
					}
				} 
				else
				{
					// got wrong packet type
					error = 1;
				}
			}
			else
			{
				// got no packet
				error = 1;
			}
		}  
		else 
		{
			// shouldn't get here
		}
	}

        if (!error && do_netscript && !preGatherRejection)
        {  // Let joiner know to expect a script
		int16 message_out = _netscript_script_intent_message;
#ifdef HAVE_SDL_NET
        	message_out = SDL_SwapBE16(message_out);
#endif
        	NetSendPacket(connections_to_clients[player.stream_id], _script_packet, &message_out);
        } 
                                                        
	if(error)
	{
		alert_user(infoError, strNETWORK_ERRORS, netErrCantAddPlayer, error);
// ZZZ: in my formulation, entry is removed as soon as it's clicked, by the clicked widget.
#if !HAVE_SDL_NET
		NetLookupRemove(player_index); /* get this guy out of here, he didn’t respond */
#endif
		theResult = kGatherPlayerFailed;
	}
#endif

        if (theResult == kGatheredUnacceptablePlayer) {
                alert_user(infoError, strNETWORK_ERRORS, netErrGatheredPlayerUnacceptable, thePlayerAcceptNumber);
        } else if (theResult != kGatherPlayerSuccessful) {
		// Drop connection of failed joiner
		delete connections_to_clients[player.stream_id];
		connections_to_clients.erase(player.stream_id);
	}
	
	return theResult;
}

void NetHandleUngatheredPlayer (prospective_joiner_info ungathered_player)
{
	// Drop connection of ungathered player
	delete connections_to_clients[ungathered_player.stream_id];
	connections_to_clients.erase(ungathered_player.stream_id);
}

/*
---------------------
NetDistributeTopology
---------------------

	<--- error

connect to everyone’s dspAddress and give them the latest copy of the network topology.  this
used to be NetStart() and it used to connect all upring and downring ADSP connections.
*/
static OSErr NetDistributeTopology(
	short tag)
{
	OSErr error = 0; //JTP: initialize to no error
	short playerIndex;
	
	assert(netState==netGathering);
	
	topology->tag= tag;
        
        NetTopology_NET	topology_NET_storage;
        NetTopology_NET* topology_NET = &topology_NET_storage;
        netcpy(topology_NET, topology);

	for (playerIndex=0; playerIndex<topology->player_count; ++playerIndex)
	{
                // ZZZ: skip players with identifier NONE - they don't really exist... also skip ourselves.
                if(topology->players[playerIndex].identifier != NONE && playerIndex != localPlayerIndex)
                {
			NetSendPacket(connections_to_clients[topology->players[playerIndex].stream_id], _topology_packet, topology_NET);
                }
	}
	
	return error; // must be noErr (not like we check return value anyway :))
}

/* -------------------- application specific code */
uint16 MaxStreamPacketLength(
	void)
{
	short packet_type;
	uint16 max_length= 0;

	for(packet_type= 0; packet_type<NUMBER_OF_BUFFERED_STREAM_PACKET_TYPES; ++packet_type)
	{	
		uint16 length= NetStreamPacketLength(packet_type);
		if(length>max_length) max_length= length;
	}
	
	return max_length;
}

uint16 NetStreamPacketLength(
	short packet_type)
{
	uint16 length;

	switch(packet_type)
	{
		case _hello_packet:
			length= 0;
			break;
			
		case _joiner_info_packet:
			length= sizeof(prospective_joiner_info_NET);
			break;
	
		case _join_player_packet:
			length= sizeof(gather_player_data_NET);
			break;
			
		case _accept_join_packet:
			length= sizeof(accept_gather_data_NET);
			break;
			
		case _topology_packet:
			length= sizeof(NetTopology_NET);
			break;

		case _stream_size_packet:
			length= sizeof(long);
			break;
			
		case _stream_data_packet:
			length= STREAM_TRANSFER_CHUNK_SIZE;
			break;
                        
#ifdef NETWORK_CHAT
                case _chat_packet:
                        length = sizeof(NetChatMessage_NET);
                        break;
#endif
		case _unknown_packet_type_response_packet:
			// we just send the unknown packet type back to them
			length = 2;
			break;
	
                case _script_packet:
			length = sizeof(int16);
			break;
        
		default:
			length= 0;
			assert(false);
			break;
	}

	return length;
}

// LP: NetAddrBlock is the trouble here
#ifdef NETWORK_IP
NetAddrBlock *NetGetPlayerADSPAddress(
	short player_index)
{
	return &topology->players[player_index].dspAddress;
}
#endif

bool NetAllowCrosshair() {
  return (dynamic_world->player_count == 1 ||
	  (dynamic_world->game_information.cheat_flags & _allow_crosshair));
}

bool NetAllowTunnelVision() {
  return (dynamic_world->player_count == 1 ||
	  dynamic_world->game_information.cheat_flags & _allow_tunnel_vision);
}

bool NetAllowBehindview() {
  return (dynamic_world->player_count == 1 ||
	  dynamic_world->game_information.cheat_flags & _allow_behindview);
}


