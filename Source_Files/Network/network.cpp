/*
NETWORK.C

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

#if defined(DISABLE_NETWORKING)

#include "network_dummy.cpp"

#else

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

#include "sdl_network.h"
#include "network_lookup_sdl.h"
#include "SDL_thread.h"

#include "game_errors.h"
#include "CommunicationsChannel.h"
#include "Console.h"
#include "MessageDispatcher.h"
#include "MessageInflater.h"
#include "MessageHandler.h"
#include "progress.h"
#include "extensions.h"

#include <stdlib.h>
#include <string.h>

#include <map>
#include <vector>
#include "Logging.h"

// ZZZ: moved many struct definitions, constant #defines, etc. to header for (limited) sharing
#include "network_private.h"

// ZZZ: since network data format is now distinct from in-memory format.
// (quite similar, admittedly, in this first effort... ;) )
#include "network_data_formats.h"

#include "network_messages.h"

#include "NetworkGameProtocol.h"

#include "RingGameProtocol.h"
#include "StarGameProtocol.h"

#include "lua_script.h"

#include "libnat.h"

#include <boost/bind.hpp>

#include "network_metaserver.h"

#include "network_sound.h"

#include "ConnectPool.h"

/* ---------- globals */

static short ddpSocket; /* our ddp socket number */

static short localPlayerIndex;
static short localPlayerIdentifier;
static std::string gameSessionIdentifier;
static NetTopologyPtr topology;
static short sServerPlayerIndex;
static bool sOldSelfSendStatus;
static RingGameProtocol sRingGameProtocol;
static StarGameProtocol sStarGameProtocol;
static NetworkGameProtocol* sCurrentGameProtocol = NULL;

static byte *deferred_script_data = NULL;
static size_t deferred_script_length = 0;
static bool do_netscript;

static CommunicationsChannelFactory *server = NULL;

typedef std::map<int, Client *> client_map_t;
static client_map_t connections_to_clients;
typedef std::map<int, ClientChatInfo *> client_chat_info_map_t;
static client_chat_info_map_t client_chat_info;
static CommunicationsChannel *connection_to_server = NULL;
static NonblockingConnect *server_nbc = 0;
static bool nbc_is_resolving = false;
static int next_stream_id = 1; // 0 is local player
static IPaddress host_address;
static bool host_address_specified = false;
static MessageInflater *inflater = NULL;
static MessageDispatcher *joinDispatcher = NULL;
static uint32 next_join_attempt;
static Capabilities my_capabilities;

static GatherCallbacks *gatherCallbacks = NULL;
static ChatCallbacks *chatCallbacks = NULL;

static UpnpController *controller = NULL;
extern MetaserverClient* gMetaserverClient;

static std::vector<NetworkStats> sNetworkStats;
const static NetworkStats sInvalidStats = {
	NetworkStats::invalid,
	NetworkStats::invalid,
	0
};
uint32 last_network_stats_send = 0;
const static int network_stats_send_period = MACHINE_TICKS_PER_SECOND;

// ignore list
static std::set<int> sIgnoredPlayers;

bool player_is_ignored(int player_index)
{
	return (sIgnoredPlayers.find(player_index) != sIgnoredPlayers.end());
}

struct ignore_player {
	void operator()(const std::string& s) const {
		int player_index = atoi(s.c_str());
		if (player_index == localPlayerIndex)
		{
			screen_printf("you can't ignore yourself");
		} 
		else if (player_index >= 0 && player_index < topology->player_count)
		{
			if (sIgnoredPlayers.find(player_index) != sIgnoredPlayers.end())
			{
				screen_printf("removing player %i from the ignore list", player_index);
				sIgnoredPlayers.erase(player_index);
			} 
			else
			{
				screen_printf("adding player %i to the ignore list", player_index);
				sIgnoredPlayers.insert(player_index);
			}
		} 
		else
		{
			screen_printf("invalid player %i", player_index);
		}
	}
};

struct ignore_lua
{
	void operator()(const std::string&) const {
		ToggleLuaMute();
	}
};

struct ignore_mic
{
	void operator()(const std::string& s) const {
		int player_index = atoi(s.c_str());
		if (player_index == localPlayerIndex)
		{
			screen_printf("you can't ignore your own mic");
		}
		else if (player_index >= 0 && player_index < topology->player_count)
		{
			mute_player_mic(player_index);
		}
		else
		{
			screen_printf("invalid player %i", player_index);
		}
	}
};

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
void NetInitializeSessionIdentifier(void);

// ZZZ: cmon, we're not fooling anyone... game_data is a game_info*; player_data is a player_info*
// Originally I guess the plan was to have a strong separation between Marathon game code and the networking code,
// such that they could be compiled independently and only know about each other at link-time, but I don't see any
// reason to try to keep that... and I suspect Jason abandoned this separation long ago anyway.
// For now, the only effect I see is a reduction in type-safety.  :)
static void NetInitializeTopology(void *game_data, short game_data_size, void *player_data, short player_data_size);
static void NetLocalAddrBlock(NetAddrBlock *address, short socketNumber);

static int net_compare(void const *p1, void const *p2);

static void NetUpdateTopology(void);
static void NetDistributeTopology(short tag);

static bool NetSetSelfSend(bool on);

static void NetDDPPacketHandler(DDPPacketBufferPtr inPacket);

int getStreamIdFromChannel(CommunicationsChannel *channel) {
  client_map_t::iterator it;
  for (it = connections_to_clients.begin(); it != connections_to_clients.end(); it++) {
    if (it->second->channel == channel) {
      return it->first;
    }
  }
  return -1;
}

//-----------------------------------------------------------------------------
// Message handlers
//-----------------------------------------------------------------------------

// Gatherer
Client::~Client() {
  delete channel;
}

CheckPlayerProcPtr Client::check_player = 0;

Client::Client(CommunicationsChannel *inChannel) : channel(inChannel), state(_connecting), network_version(0), mDispatcher(new MessageDispatcher())
{
	std::fill_n(name, MAX_NET_PLAYER_NAME_LENGTH, '\0');
	mJoinerInfoMessageHandler.reset(newMessageHandlerMethod(this, &Client::handleJoinerInfoMessage));
	mCapabilitiesMessageHandler.reset(newMessageHandlerMethod(this, &Client::handleCapabilitiesMessage));
	mAcceptJoinMessageHandler.reset(newMessageHandlerMethod(this, &Client::handleAcceptJoinMessage));
	mChatMessageHandler.reset(newMessageHandlerMethod(this, &Client::handleChatMessage));
	mChangeColorsMessageHandler.reset(newMessageHandlerMethod(this, &Client::handleChangeColorsMessage));
	mUnexpectedMessageHandler.reset(newMessageHandlerMethod(this, &Client::unexpectedMessageHandler));
	mDispatcher->setDefaultHandler(mUnexpectedMessageHandler.get());
	mDispatcher->setHandlerForType(mJoinerInfoMessageHandler.get(), JoinerInfoMessage::kType);
	mDispatcher->setHandlerForType(mCapabilitiesMessageHandler.get(), CapabilitiesMessage::kType);
	mDispatcher->setHandlerForType(mAcceptJoinMessageHandler.get(), AcceptJoinMessage::kType);
	mDispatcher->setHandlerForType(mChatMessageHandler.get(), NetworkChatMessage::kType);
	mDispatcher->setHandlerForType(mChangeColorsMessageHandler.get(), ChangeColorsMessage::kType);
	channel->setMessageHandler(mDispatcher.get());
}

void Client::drop()
{
	int stream_id = getStreamIdFromChannel(channel);
	if (client_chat_info[stream_id]) {
		ClientInfoMessage clientInfoMessage(stream_id, client_chat_info[stream_id], (int16) ClientInfoMessage::kRemove);
		client_map_t::iterator it;
		for (it = connections_to_clients.begin(); it != connections_to_clients.end(); it++) {
			if (it->second->can_pregame_chat()) {
				it->second->channel->enqueueOutgoingMessage(clientInfoMessage);
			}
		}

		delete client_chat_info[stream_id];
		client_chat_info.erase(stream_id);
	}

	if (state == _connected) { // (remove from the list of joinable players)
		if (gatherCallbacks) {
			prospective_joiner_info player;
			player.stream_id = getStreamIdFromChannel(channel);
			gatherCallbacks->JoiningPlayerDropped(&player);
		}
	} else if (state == _awaiting_map) { // need to remove from topo
		uint16 stream_id = getStreamIdFromChannel(channel);
		int i;
		for (i = 1; i < topology->player_count; i++) {
			if (topology->players[i].stream_id == stream_id) {
				break;
			}
		}
		if (i != topology->player_count) {
			for (; i < topology->player_count - 1; i++) {
				topology->players[i] = topology->players[i + 1];
			}
			topology->player_count--;
			NetUpdateTopology();

			if (gatherCallbacks) {
				prospective_joiner_info player;
				player.stream_id = getStreamIdFromChannel(channel);
				gatherCallbacks->JoinedPlayerDropped(&player);
			}

			NetDistributeTopology(tagDROPPED_PLAYER);
		} else {
			logAnomaly("a client in state _awaiting_map dropped, but was not found in the topology");
		}
	} 
}

// This serves as a generic M1 check. It doesn't guarantee
// map or physics are M1, but for now it suffices.
extern bool shapes_file_is_m1();

bool Client::capabilities_indicate_player_is_gatherable(bool warn_joiner)
{
	// ghs: perhaps someday there will be an elegant, extensible way to do this
	//      but for now, this is what you get

	char s[256];
  
	// As this is the first version, I do not check version numbers for
	// compatibility! Because I don't know if higher numbers necessarily mean
	// incompatibility. In the future, gatherer and joiner should advertise what
	// they have, and the higher one must contain the backward compatibility
	// logic if any exists

	// joiners can disable themselves from being gathered by replying with
	// capabilities where the "Gatherable" capability is set to 0...this will
	// not trigger a warning message from the gatherer

	// gatherer disables gathering by returning false from this function, and
	// sending a descriptive error message to the joiner saying he won't show up

	if (capabilities[Capabilities::kGatherable] == 0) {
		// no warning, the joiner already knows he is incompatible
		return false;
	}

	if (capabilities[Capabilities::kGameworld] < Capabilities::kGameworldVersion || (shapes_file_is_m1() && capabilities[Capabilities::kGameworldM1] < Capabilities::kGameworldM1Version))
	{
		if (warn_joiner)
		{
			ServerWarningMessage serverWarningMessage(expand_app_variables("The gatherer is using a new version of $appName$. You will not appear in the list of available players."), ServerWarningMessage::kJoinerUngatherable);
			channel->enqueueOutgoingMessage(serverWarningMessage);
		}
		return false;
	}

	if (network_preferences->game_protocol == _network_game_protocol_star) {
		if (capabilities[Capabilities::kStar] == 0) {
			if (warn_joiner) {
				ServerWarningMessage serverWarningMessage(getcstr(s, strNETWORK_ERRORS, netWarnJoinerHasNoStar), ServerWarningMessage::kJoinerUngatherable);
				channel->enqueueOutgoingMessage(serverWarningMessage);
			}
			return false;
		} else if (capabilities[Capabilities::kStar] < Capabilities::kStarVersion) {
			if (warn_joiner) {
				ServerWarningMessage serverWarningMessage(expand_app_variables("The gatherer is using a newer version of $appName$. You will not appear in the list of available players."), ServerWarningMessage::kJoinerUngatherable);
				channel->enqueueOutgoingMessage(serverWarningMessage);
			}
			return false;
		}
	} else {
		if (capabilities[Capabilities::kRing] == 0) {
			if (warn_joiner) {
				ServerWarningMessage serverWarningMessage(getcstr(s, strNETWORK_ERRORS, netWarnJoinerHasNoRing), ServerWarningMessage::kJoinerUngatherable);
				channel->enqueueOutgoingMessage(serverWarningMessage);
			}
			return false;
		}
	}

	if (do_netscript)
	{
		if (capabilities[Capabilities::kLua] == 0) {
			if (warn_joiner) {
				char s[256];
				ServerWarningMessage serverWarningMessage(getcstr(s, strNETWORK_ERRORS, netWarnJoinerNoLua), ServerWarningMessage::kJoinerUngatherable);
				channel->enqueueOutgoingMessage(serverWarningMessage);
			}
			return false;
		} else if (capabilities[Capabilities::kLua] < Capabilities::kLuaVersion)
		{
			if (warn_joiner)
			{
				ServerWarningMessage serverWarningMessage("The gatherer is using a newer version of Lua (net script) that you do not have. You will not appear in the list of available players.", ServerWarningMessage::kJoinerUngatherable);
				channel->enqueueOutgoingMessage(serverWarningMessage);
			}
		return false;
		}
	}

	if (topology->game_data.net_game_type == _game_of_rugby)
	{
		if (capabilities[Capabilities::kRugby] == 0)
		{
			if (warn_joiner)
			{
				ServerWarningMessage serverWarningMessage(expand_app_variables("The gatherer is using a newer version of $appName$ with different rugby scoring. You will not appear in the list of available players."), ServerWarningMessage::kJoinerUngatherable);
				channel->enqueueOutgoingMessage(serverWarningMessage);
			}
			return false;
		}
	}
	
	return true;
}
  

void Client::handleJoinerInfoMessage(JoinerInfoMessage* joinerInfoMessage, CommunicationsChannel *) 
{
  if (netState == netGathering) {
    if (joinerInfoMessage->version() == kNetworkSetupProtocolID) {
      strncpy(name, joinerInfoMessage->info()->name, MAX_NET_PLAYER_NAME_LENGTH);

      int16 stream_id = getStreamIdFromChannel(channel);
      client_chat_info[stream_id] = new ClientChatInfo;
      client_chat_info[stream_id]->name = joinerInfoMessage->info()->name;
      client_chat_info[stream_id]->color = joinerInfoMessage->info()->color;
      client_chat_info[stream_id]->team = joinerInfoMessage->info()->team;

      ClientInfoMessage clientInfoMessage(stream_id, client_chat_info[stream_id], (int16) ClientInfoMessage::kAdd);
      client_map_t::iterator it;
      for (it = connections_to_clients.begin(); it != connections_to_clients.end(); it++) {
	      if (it->second->can_pregame_chat()) {
		      it->second->channel->enqueueOutgoingMessage(clientInfoMessage);
	      }
      }
      
      // send gatherer capabilities
      CapabilitiesMessage capabilitiesMessage(my_capabilities);
      channel->enqueueOutgoingMessage(capabilitiesMessage);
      state = Client::_awaiting_capabilities;
    } else {
      // strange, joiner should have realized he couldn't join
      // ok, disconnect him
      state = Client::_disconnect;
    }
  } else {
    logAnomaly("unexpected joiner info message received (netState is %i)", netState);
  }
}

void Client::handleCapabilitiesMessage(CapabilitiesMessage* capabilitiesMessage, CommunicationsChannel *)
{
	if (state == _awaiting_capabilities) {
		capabilities = *capabilitiesMessage->capabilities();
		
		if (capabilities_indicate_player_is_gatherable(_warn_joiner)) {
			state = Client::_connected_but_not_yet_shown;
		} else {
			state = Client::_ungatherable;
		}
		
		client_chat_info_map_t::iterator it;
		for (it = client_chat_info.begin(); it != client_chat_info.end(); it++) {
			if (it->second)
			{
				ClientInfoMessage clientInfoMessage(it->first, it->second, (int16) ClientInfoMessage::kAdd);
				channel->enqueueOutgoingMessage(clientInfoMessage);
			}
		}
	} else {
		logAnomaly("unexpected capabilities message received (state is %i)", state);
	}
}

/*
void Client::handleScriptMessage(ScriptMessage* scriptMessage, CommunicationsChannel *) 
{
  if (state == _awaiting_script_message) {
    if (do_netscript &&
	scriptMessage->value() != _netscript_yes_script_message) {
      alert_user(infoError, strNETWORK_ERRORS, netErrUngatheredPlayerUnacceptable, 0);
      state = _ungatherable;
    } else {
      JoinPlayerMessage joinPlayerMessage(topology->nextIdentifier);
      channel->enqueueOutgoingMessage(joinPlayerMessage);
      state = _awaiting_accept_join;
    }
  } else {
    logAnomaly1("unexpected script message received (state is %i)", state);
  }
}
*/

void Client::handleAcceptJoinMessage(AcceptJoinMessage* acceptJoinMessage,
				     CommunicationsChannel *)
{
  if (state == _awaiting_accept_join) {
    if (acceptJoinMessage->accepted()) {
      topology->players[topology->player_count] = *acceptJoinMessage->player();
      topology->players[topology->player_count].stream_id = getStreamIdFromChannel(channel);
	  topology->players[topology->player_count].net_dead = false;
      prospective_joiner_info player;
      player.stream_id = topology->players[topology->player_count].stream_id;
      topology->players[topology->player_count].dspAddress = channel->peerAddress();
      topology->players[topology->player_count].ddpAddress.host = channel->peerAddress().host;
      
      topology->player_count += 1;
      check_player(topology->player_count - 1, topology->player_count);
      NetUpdateTopology();
 
      GameSessionMessage gameSessionMessage(reinterpret_cast<const uint8*>(gameSessionIdentifier.c_str()), gameSessionIdentifier.size());
      CommunicationsChannel *channel = connections_to_clients[player.stream_id]->channel;
      channel->enqueueOutgoingMessage(gameSessionMessage);

      NetDistributeTopology(tagNEW_PLAYER);
      state = _awaiting_map;
      if (gatherCallbacks) gatherCallbacks->JoinSucceeded(&player);
    } else {
      // joiner didn't accept!?
      alert_user(infoError, strNETWORK_ERRORS, netErrCantAddPlayer, 0);
      state = _ungatherable;
    }
  } else {
    logAnomaly("unexpected accept join message received (state is %i)", state);
  }
}

void Client::handleChangeColorsMessage(ChangeColorsMessage *changeColorsMessage,
				       CommunicationsChannel *channel)
{
	if (can_pregame_chat()) {
		int stream_id = getStreamIdFromChannel(channel);
		if (client_chat_info[stream_id]) {
			client_chat_info[stream_id]->color = changeColorsMessage->color();
			client_chat_info[stream_id]->team = changeColorsMessage->team();
			
			ClientInfoMessage clientInfoMessage(stream_id, client_chat_info[stream_id], (int16) ClientInfoMessage::kUpdate);
			client_map_t::iterator it;
			for (it = connections_to_clients.begin(); it != connections_to_clients.end(); it++) {
				if (it->second->can_pregame_chat()) {
					it->second->channel->enqueueOutgoingMessage(clientInfoMessage);
				}
			}
		} else {
			logAnomaly("change colors message received, but client chat info does not exist for %i", stream_id);
		}
	}
	if (state == _awaiting_map) {
		uint16 stream_id = getStreamIdFromChannel(channel);
		int i;
		for (i = 1; i < topology->player_count; i++) {
			if (topology->players[i].stream_id == stream_id) {
				break;
			}
		}

		if (i != topology->player_count) {
			player_info *player = &topology->players[i].player_data;
			if (player->desired_color != changeColorsMessage->color() ||
			    player->team != changeColorsMessage->team()) {

				player->desired_color = changeColorsMessage->color();
				player->team = changeColorsMessage->team();

				check_player(i, topology->player_count);
				NetUpdateTopology();
	
				NetDistributeTopology(tagCHANGED_PLAYER);
				if (gatherCallbacks) {
					prospective_joiner_info player_to_change;
					player_to_change.stream_id = stream_id;
					gatherCallbacks->JoinedPlayerChanged(&player_to_change);
				}
			}
		} else {
			logAnomaly("a client in state _awaiting_map requested a color change, but was not found in the topology");
		}
	} else if (!can_pregame_chat()) {
		logAnomaly("unexpected change colors message received (state is %i)", state);
	}
}

void Client::handleChatMessage(NetworkChatMessage* netChatMessage, 
			       CommunicationsChannel *)
{
	// relay this to all clients
	if (state == _ingame) {
		assert(netState == netActive);
		if (netChatMessage->target() == NetworkChatMessage::kTargetPlayers) {
			NetworkChatMessage chatMessage(netChatMessage->chatText(), getStreamIdFromChannel(channel), NetworkChatMessage::kTargetPlayers);
			client_map_t::iterator it;
			for (it = connections_to_clients.begin(); it != connections_to_clients.end(); it++) {
				if (it->second->state == _ingame) {
					it->second->channel->enqueueOutgoingMessage(chatMessage);
				}
			}
      
			// display it locally
			if (chatCallbacks) {
				for (int playerIndex = 0; playerIndex < topology->player_count; playerIndex++) {
					if (topology->players[playerIndex].stream_id == getStreamIdFromChannel(channel)) {
						if (player_is_ignored(playerIndex)) return;
						chatCallbacks->ReceivedMessageFromPlayer(topology->players[playerIndex].player_data.name, netChatMessage->chatText());
						return;
					}
				}
			}
		} else {
			logNote("in-game chat message currently only supports sending messages to all players; not relaying");
		}
	} else if (can_pregame_chat()) {
		if (netChatMessage->target() == NetworkChatMessage::kTargetClients) {
			NetworkChatMessage chatMessage(netChatMessage->chatText(), getStreamIdFromChannel(channel), NetworkChatMessage::kTargetClients);
			client_map_t::iterator it;
			for (it = connections_to_clients.begin(); it != connections_to_clients.end(); it++) {
				if (it->second->can_pregame_chat()) {
					it->second->channel->enqueueOutgoingMessage(chatMessage);
				}
			}
			if (chatCallbacks) {
				int stream_id = getStreamIdFromChannel(channel);
				if (client_chat_info[stream_id]) {
					chatCallbacks->ReceivedMessageFromPlayer(client_chat_info[stream_id]->name.c_str(), netChatMessage->chatText());
				} else {
					logAnomaly("chat message from %i, player not found", stream_id);
				}
			}
		} else {
			logNote("pre-game chat currently only supports sending messages to all players; not relaying");
		}
	} else {
		logNote("non in-game/pre-game chat message received; ignoring");
	}
}

void Client::unexpectedMessageHandler(Message *message, CommunicationsChannel *) {
	logAnomaly("unexpected message type %i received (net state)", message->type(), netState);
}
    


static short handlerState;

static void handleHelloMessage(HelloMessage* helloMessage, CommunicationsChannel*)
{
	if (handlerState == netAwaitingHello) {
		// if the network versions match, reply with my join info
		if (helloMessage->version() == kNetworkSetupProtocolID) {
			prospective_joiner_info my_info = {};
      
			strncpy(my_info.name, player_preferences->name, sizeof(my_info.name) - 1);
			my_info.color = player_preferences->color;
			my_info.team = player_preferences->team;

			JoinerInfoMessage joinerInfoMessage(&my_info, kNetworkSetupProtocolID);
			connection_to_server->enqueueOutgoingMessage(joinerInfoMessage);
			handlerState = netJoining;
		} else {
			alert_user(infoError, strNETWORK_ERRORS, netErrIncompatibleVersion, 0);
			handlerState = netJoinErrorOccurred;
		}
	} else {
		logAnomaly("unexpected hello message received (netState is %i)", netState);
	}
}

static void handleCapabilitiesMessage(CapabilitiesMessage* capabilitiesMessage, 
				      CommunicationsChannel *)
{
	if (handlerState == netJoining) {
		Capabilities capabilities = *capabilitiesMessage->capabilities();
		if (capabilities[Capabilities::kGameworld] < Capabilities::kGameworldVersion || (shapes_file_is_m1() && capabilities[Capabilities::kGameworldM1] < Capabilities::kGameworldM1Version) || (network_preferences->game_protocol == _network_game_protocol_star && capabilities[Capabilities::kStar] < Capabilities::kStarVersion))
		{
			// I'm not gatherable
			my_capabilities[Capabilities::kGatherable] = 0;
			CapabilitiesMessage capabilitiesMessageReply(my_capabilities);
			connection_to_server->enqueueOutgoingMessage(capabilitiesMessageReply);
			my_capabilities[Capabilities::kGatherable] = Capabilities::kGatherableVersion;
			
			alert_user(expand_app_variables("The gatherer is using an old version of $appName$. You will not appear in the list of available players.").c_str());
		} else {
			// everything else is version 1
			CapabilitiesMessage capabilitiesMessageReply(my_capabilities);
			connection_to_server->enqueueOutgoingMessage(capabilitiesMessageReply);
		}
		
	} else {
		logAnomaly("unexpected capabilities message received (netState is %i)", netState);
	}
}   

static void handleClientInfoMessage(ClientInfoMessage* clientInfoMessage, CommunicationsChannel *) {
	if (netState == netJoining || netState == netWaiting || netState == netStartingUp || netState == netActive) {
		int16 id = clientInfoMessage->stream_id();
		if (clientInfoMessage->action() == ClientInfoMessage::kAdd) {
			if (client_chat_info[id]) {
				logAnomaly("add message for client that already exists (%i)", id);
				delete client_chat_info[id];
				client_chat_info.erase(id);
			}
			client_chat_info[id] = new ClientChatInfo;
			*client_chat_info[id] = *clientInfoMessage->info();
		} else if (clientInfoMessage->action() == ClientInfoMessage::kRemove) {
			delete client_chat_info[id];
			client_chat_info.erase(id);
		} else if (clientInfoMessage->action() == ClientInfoMessage::kUpdate) {
			*client_chat_info[id] = *clientInfoMessage->info();
		} else {
			logAnomaly("unknown client info message action %i", clientInfoMessage->action());
		}
	} else {
		logAnomaly("unexpected client info message received (netState is %i)", netState);
	}
}

static void handleJoinPlayerMessage(JoinPlayerMessage* joinPlayerMessage, CommunicationsChannel*) {
  if (handlerState == netJoining) {
    /* Note that we could set accepted to false if we wanted to for some */
    /*  reason- such as bad serial numbers.... */
    
    SetNetscriptStatus (false); // Unless told otherwise, we don't expect a netscript
    
    /* Note that we share the buffers.. */
    localPlayerIdentifier= joinPlayerMessage->value();
    topology->players[localPlayerIndex].identifier= localPlayerIdentifier;
    topology->players[localPlayerIndex].net_dead= false;
    
    /* Confirm. */
    AcceptJoinMessage acceptJoinMessage(true, &topology->players[localPlayerIndex]);
    connection_to_server->enqueueOutgoingMessage(acceptJoinMessage);

    if (acceptJoinMessage.accepted()) {
      handlerState = netWaiting;
    } else {
      handlerState = netJoinErrorOccurred;
    }
  } else {
    logAnomaly("unexpected join player message received (netState is %i)", netState);
  }
}

static byte *handlerLuaBuffer = NULL;
static size_t handlerLuaLength = 0;

static void handleLuaMessage(BigChunkOfDataMessage *luaMessage, CommunicationsChannel *) {
  if (netState == netStartingUp || netState == netDown) {
    if (handlerLuaBuffer) {
      delete[] handlerLuaBuffer;
      handlerLuaBuffer = NULL;
    }
    handlerLuaLength = luaMessage->length();
    if (handlerLuaLength > 0) {
      handlerLuaBuffer = new byte[handlerLuaLength];
      memcpy(handlerLuaBuffer, luaMessage->buffer(), handlerLuaLength);
    }
  } else {
    logAnomaly("unexpected lua message received (netState is %i)", netState);
  }
}

static byte *handlerMapBuffer = NULL;
static size_t handlerMapLength = 0;

static void handleMapMessage(BigChunkOfDataMessage *mapMessage, CommunicationsChannel *) {
	if (netState == netStartingUp || netState == netDown) {
		if (handlerMapBuffer) { // assume the last map the server sent is right
			free(handlerMapBuffer);
			handlerMapBuffer = NULL;
		}
		handlerMapLength = mapMessage->length();
		if (handlerMapLength > 0) {
			handlerMapBuffer = reinterpret_cast<byte*>(malloc(handlerMapLength));
			memcpy(handlerMapBuffer, mapMessage->buffer(), handlerMapLength);
		}
	} else {
		logAnomaly("unexpected map message received (netState is %i)", netState);
	}
}

static void handleNetworkChatMessage(NetworkChatMessage *chatMessage, CommunicationsChannel *) {
	if (chatCallbacks) {
		if (netState == netActive) {
			for (int playerIndex = 0; playerIndex < topology->player_count; playerIndex++) {
				if (topology->players[playerIndex].stream_id == chatMessage->senderID()) {
					if (player_is_ignored(playerIndex)) return;
					chatCallbacks->ReceivedMessageFromPlayer(topology->players[playerIndex].player_data.name, chatMessage->chatText());
					return;
				}
			}
			logAnomaly("chat message from %i, player not found", chatMessage->senderID());
		} else if (netState == netJoining || netState == netWaiting) {
			if (client_chat_info[chatMessage->senderID()]) {
				chatCallbacks->ReceivedMessageFromPlayer(client_chat_info[chatMessage->senderID()]->name.c_str(), chatMessage->chatText());
			} else {
				logAnomaly("chat message from %i, player not found", chatMessage->senderID());
			}
			return;
		} else {
			// not enough smarts to correctly redistrbute these, so just ignore them
			logNote("non in-game chat message received; ignoring");
		}
	}
}

static void handleNetworkStatsMessage(NetworkStatsMessage *statsMessage, CommunicationsChannel *)
{
	if (netState == netActive)
	{
		if (sNetworkStats.empty() || statsMessage->mStats.size() == sNetworkStats.size())
		{
			sNetworkStats = statsMessage->mStats;
		}
		else
		{
			logWarning("network stats message is wrong size; lost stats");
			std::fill(sNetworkStats.begin(), sNetworkStats.end(), sInvalidStats);
		}
	} else {
		logAnomaly("unexpected network stats message received (netState is %i", netState);
	}
}

static byte *handlerPhysicsBuffer = NULL;
static size_t handlerPhysicsLength = 0;

static void handlePhysicsMessage(BigChunkOfDataMessage *physicsMessage, CommunicationsChannel *) {
	if (netState == netStartingUp || netState == netDown) {
		if (handlerPhysicsBuffer) {
			free(handlerPhysicsBuffer);
			handlerPhysicsBuffer = NULL;
		}
		handlerPhysicsLength = physicsMessage->length();
		if (handlerPhysicsLength > 0) {
			handlerPhysicsBuffer = reinterpret_cast<byte*>(malloc(handlerPhysicsLength));
			memcpy(handlerPhysicsBuffer, physicsMessage->buffer(), handlerPhysicsLength);
		}
	} else {
		logAnomaly("unexpected physics message received (netState is %i)", netState);
	}
}

/*
static void handleScriptMessage(ScriptMessage* scriptMessage, CommunicationsChannel*) {
  if (netState == netJoining) {
    ScriptMessage replyToScriptMessage;
    if (scriptMessage->value() == _netscript_query_message) {
#ifdef HAVE_LUA
      replyToScriptMessage.setValue(_netscript_yes_script_message);
#else
      replyToScriptMessage.setValue(_netscript_no_script_message);
#endif
    } else {
      replyToScriptMessage.setValue(_netscript_no_script_message);
    }
    connection_to_server->enqueueOutgoingMessage(replyToScriptMessage);
  } else {
    logAnomaly1("unexpected script message received (netState is %i)", netState);
  }
}
*/

static void handleServerWarningMessage(ServerWarningMessage *serverWarningMessage, CommunicationsChannel *) {
  char *s = strdup(serverWarningMessage->string()->c_str());
  alert_user(s);
  free(s);
}


static void handleTopologyMessage(TopologyMessage* topologyMessage, CommunicationsChannel *) {
  if (netState == netWaiting) {
    *topology = *(topologyMessage->topology());
    
    NetAddrBlock address;
      
      address = connection_to_server->peerAddress();
      
      // ZZZ: the code below used to assume the server was _index_ 0; now, we merely
      // assume the server has _identifier_ 0.
      int theServerIndex;
      for(theServerIndex = 0; theServerIndex < topology->player_count; theServerIndex++)
	{
	  if(topology->players[theServerIndex].identifier == 0) break;
	}
      
      assert(theServerIndex != topology->player_count);
      
      topology->players[theServerIndex].dspAddress= address;
      topology->players[theServerIndex].ddpAddress.host = address.host;
      
      NetUpdateTopology();
    
    switch (topology->tag)
      {
      case tagNEW_PLAYER:
	handlerState = netPlayerAdded;
	break;
      case tagDROPPED_PLAYER:
	handlerState = netPlayerDropped;
	break;
      case tagCHANGED_PLAYER:
	handlerState = netPlayerChanged;
	break;
	
      case tagCANCEL_GAME:
	handlerState= netCancelled;
	alert_user(infoError, strNETWORK_ERRORS, netErrServerCanceled, 0);
	break;
	
      case tagSTART_GAME:
	handlerState = netStartingUp;
	resuming_saved_game = false;
	break;
	
	// ZZZ addition
      case tagRESUME_GAME:
	handlerState = netStartingResumeGame;
	resuming_saved_game = true;
	break;
	
      default:
	logAnomaly("topology message received with unknown tag %i; ignoring", topology->tag);
	break;
      }
  } else {
    logWarning("unexpected topology message received -- gatherer and joiner could disagree on topology! (netState is %i)", netState);
  }
}

static void handleGameSessionMessage(GameSessionMessage* gameSessionMessage, CommunicationsChannel*) {
	if (handlerState == netWaiting) {
		gameSessionIdentifier.assign(gameSessionMessage->buffer(), gameSessionMessage->buffer() + gameSessionMessage->length());
	} else {
		logAnomaly("unexpected game session message received (netState is %i)", netState);
	}
}

static void handleUnexpectedMessage(Message *inMessage, CommunicationsChannel *) {
  if (handlerState == netAwaitingHello) {
    // an unexpected message before hello usually means we couldn't parse
    // hello; which means it's likely we're not compatible
    alert_user(infoError, strNETWORK_ERRORS, netErrIncompatibleVersion, 0);
    handlerState = netJoinErrorOccurred;
  }
  logAnomaly("unexpected message ID %i received", inMessage->type());
}
    
static TypedMessageHandlerFunction<HelloMessage> helloMessageHandler(&handleHelloMessage);
static TypedMessageHandlerFunction<JoinPlayerMessage> joinPlayerMessageHandler(&handleJoinPlayerMessage);
static TypedMessageHandlerFunction<BigChunkOfDataMessage> luaMessageHandler(&handleLuaMessage);
static TypedMessageHandlerFunction<BigChunkOfDataMessage> mapMessageHandler(&handleMapMessage);
static TypedMessageHandlerFunction<NetworkChatMessage> networkChatMessageHandler(&handleNetworkChatMessage);
static TypedMessageHandlerFunction<BigChunkOfDataMessage> physicsMessageHandler(&handlePhysicsMessage);
 static TypedMessageHandlerFunction<CapabilitiesMessage> capabilitiesMessageHandler(&handleCapabilitiesMessage);
static TypedMessageHandlerFunction<TopologyMessage> topologyMessageHandler(&handleTopologyMessage);
static TypedMessageHandlerFunction<ServerWarningMessage> serverWarningMessageHandler(&handleServerWarningMessage);
static TypedMessageHandlerFunction<ClientInfoMessage> clientInfoMessageHandler(&handleClientInfoMessage);
static TypedMessageHandlerFunction<NetworkStatsMessage> networkStatsMessageHandler(&handleNetworkStatsMessage);
static TypedMessageHandlerFunction<GameSessionMessage> gameSessionMessageHandler(&handleGameSessionMessage);
static TypedMessageHandlerFunction<Message> unexpectedMessageHandler(&handleUnexpectedMessage);

void NetSetGatherCallbacks(GatherCallbacks *gc) {
  gatherCallbacks = gc;
}

void NetSetChatCallbacks(ChatCallbacks *cc) {
  chatCallbacks = cc;
}

void ChatCallbacks::SendChatMessage(const std::string& message)
{
	if (message == "") return;
	if (netState == netActive) {
		if (connection_to_server) {
			NetworkChatMessage chatMessage(message.c_str(), 0, NetworkChatMessage::kTargetPlayers); // gatherer will replace senderID with my ID
			connection_to_server->enqueueOutgoingMessage(chatMessage);
		} else { 
			NetworkChatMessage chatMessage(message.c_str(), 0, NetworkChatMessage::kTargetPlayers); // gatherer stream ID is always 0
			client_map_t::iterator it;
			for (it = connections_to_clients.begin(); it != connections_to_clients.end(); it++) {
				if (it->second->state == Client::_ingame) {
					it->second->channel->enqueueOutgoingMessage(chatMessage);
				}
			}
			if (chatCallbacks) {
				for (int playerIndex = 0; playerIndex < topology->player_count; playerIndex++) {
					if (playerIndex == localPlayerIndex) {
						chatCallbacks->ReceivedMessageFromPlayer(topology->players[playerIndex].player_data.name, message.c_str());
					}
				}
			}
		}
	} else if (netState == netGathering) {
		NetworkChatMessage chatMessage(message.c_str(), 0, NetworkChatMessage::kTargetClients); // gatherer stream ID is always 0
		client_map_t::iterator it;
		for (it = connections_to_clients.begin(); it != connections_to_clients.end(); it++) {
			if (it->second->can_pregame_chat()) {
				it->second->channel->enqueueOutgoingMessage(chatMessage);
			}
		}
		if (chatCallbacks) {
			chatCallbacks->ReceivedMessageFromPlayer(client_chat_info[0]->name.c_str(), message.c_str());
		}
	} else if (netState == netJoining || netState == netWaiting) {
		assert(connection_to_server);
		NetworkChatMessage chatMessage(message.c_str(), 0, NetworkChatMessage::kTargetClients); // gatherer will replace senderID with my ID
		connection_to_server->enqueueOutgoingMessage(chatMessage);
	} else {
		logNote("SendChatMessage called but non in-game/pre-game chat messages are not yet implemented");
	}
}


InGameChatCallbacks *InGameChatCallbacks::instance() {
	static InGameChatCallbacks *m_instance = nullptr;
  if (!m_instance) {
    m_instance = new InGameChatCallbacks();
  }
  return m_instance;
}

std::string InGameChatCallbacks::prompt() {
  return (std::string(player_preferences->name) + ":");
}

void InGameChatCallbacks::ReceivedMessageFromPlayer(const char *player_name, const char *message) {
  screen_printf("%s: %s", player_name, message);
}

bool NetEnter(void)
{
	OSErr error;
  
	assert(netState==netUninitialized);
  
	{
		static bool added_exit_procedure= false;
    
		if (!added_exit_procedure) atexit(NetExit);
		added_exit_procedure= true;
	}
  
	sCurrentGameProtocol = (network_preferences->game_protocol == _network_game_protocol_star) ?
		static_cast<NetworkGameProtocol*>(&sStarGameProtocol) :
		static_cast<NetworkGameProtocol*>(&sRingGameProtocol);
  
	error= NetDDPOpen();
	if (!error) {
		topology = (NetTopologyPtr)malloc(sizeof(NetTopology));
		assert(topology);
		memset(topology, 0, sizeof(NetTopology));
    
		NetSetServerIdentifier(0);
    
		// ZZZ: Sorry, if this swapping is not supported on all current A1
		// platforms, feel free to rewrite it in a way that is.
		ddpSocket= SDL_SwapBE16(GAME_PORT);
		error= NetDDPOpenSocket(&ddpSocket, NetDDPPacketHandler);
		if (!error) {
			sOldSelfSendStatus= NetSetSelfSend(true);
			sServerPlayerIndex= 0;
      
			sCurrentGameProtocol->Enter(&netState);
      
			netState= netDown;
			handlerState = netDown;
		} else {
			logError("unable to open socket");
		}
	}
  
	if (!inflater) {
		inflater = new MessageInflater ();
		for (int i = 0; i < NUMBER_OF_STREAM_PACKET_TYPES; i++) {
			BigChunkOfDataMessage *prototype = new BigChunkOfDataMessage (i);
			inflater->learnPrototypeForType (i, *prototype);
			delete prototype;
		}
    
		inflater->learnPrototype(AcceptJoinMessage());
		inflater->learnPrototype(EndGameDataMessage());
		inflater->learnPrototype(HelloMessage());
		inflater->learnPrototype(JoinerInfoMessage());
		inflater->learnPrototype(JoinPlayerMessage());
		inflater->learnPrototype(LuaMessage());
		inflater->learnPrototype(ZippedLuaMessage());
		inflater->learnPrototype(MapMessage());
		inflater->learnPrototype(ZippedMapMessage());
		inflater->learnPrototype(NetworkChatMessage());
		inflater->learnPrototype(PhysicsMessage());
		inflater->learnPrototype(ZippedPhysicsMessage());
		inflater->learnPrototype(CapabilitiesMessage());
		inflater->learnPrototype(TopologyMessage());
		inflater->learnPrototype(ChangeColorsMessage());
		inflater->learnPrototype(ServerWarningMessage());
		inflater->learnPrototype(ClientInfoMessage());
		inflater->learnPrototype(NetworkStatsMessage());
		inflater->learnPrototype(GameSessionMessage());
	}
  
	if (!joinDispatcher) {
		joinDispatcher = new MessageDispatcher();
    
		joinDispatcher->setDefaultHandler(&unexpectedMessageHandler);
		joinDispatcher->setHandlerForType(&helloMessageHandler, HelloMessage::kType);
		joinDispatcher->setHandlerForType(&joinPlayerMessageHandler, JoinPlayerMessage::kType);
		joinDispatcher->setHandlerForType(&luaMessageHandler, LuaMessage::kType);
		joinDispatcher->setHandlerForType(&luaMessageHandler, ZippedLuaMessage::kType);
		joinDispatcher->setHandlerForType(&mapMessageHandler, MapMessage::kType);
		joinDispatcher->setHandlerForType(&mapMessageHandler, ZippedMapMessage::kType);
		joinDispatcher->setHandlerForType(&networkChatMessageHandler, NetworkChatMessage::kType);
		joinDispatcher->setHandlerForType(&physicsMessageHandler, PhysicsMessage::kType);
		joinDispatcher->setHandlerForType(&physicsMessageHandler, ZippedPhysicsMessage::kType);
		joinDispatcher->setHandlerForType(&capabilitiesMessageHandler, CapabilitiesMessage::kType);
		joinDispatcher->setHandlerForType(&serverWarningMessageHandler, ServerWarningMessage::kType);
		joinDispatcher->setHandlerForType(&clientInfoMessageHandler, ClientInfoMessage::kType);
		joinDispatcher->setHandlerForType(&topologyMessageHandler, TopologyMessage::kType);
		joinDispatcher->setHandlerForType(&networkStatsMessageHandler, NetworkStatsMessage::kType);
		joinDispatcher->setHandlerForType(&gameSessionMessageHandler, GameSessionMessage::kType);
	}

	my_capabilities.clear();
	my_capabilities[Capabilities::kGameworld] = Capabilities::kGameworldVersion;
	my_capabilities[Capabilities::kGameworldM1] = Capabilities::kGameworldM1Version;
	my_capabilities[Capabilities::kSpeex] = Capabilities::kSpeexVersion;
	if (network_preferences->game_protocol == _network_game_protocol_star) {
		my_capabilities[Capabilities::kStar] = Capabilities::kStarVersion;
	} else {
		my_capabilities[Capabilities::kRing] = Capabilities::kRingVersion;
	}
#ifdef HAVE_LUA
	my_capabilities[Capabilities::kLua] = Capabilities::kLuaVersion;
#endif
	my_capabilities[Capabilities::kGatherable] = Capabilities::kGatherableVersion;
	my_capabilities[Capabilities::kZippedData] = Capabilities::kZippedDataVersion;
	my_capabilities[Capabilities::kNetworkStats] = Capabilities::kNetworkStatsVersion;
	my_capabilities[Capabilities::kRugby] = Capabilities::kRugbyVersion;

	// net commands!
	sIgnoredPlayers.clear();
	CommandParser IgnoreParser;
	IgnoreParser.register_command("player", ignore_player());

	clear_player_mic_mutes();
	IgnoreParser.register_command("mic", ignore_mic());

	ResetLuaMute();
	IgnoreParser.register_command("lua", ignore_lua());

	Console::instance()->register_command("ignore", IgnoreParser);

	next_join_attempt = last_network_stats_send = machine_tick_count();
  
	if (error) {
		alert_user(infoError, strNETWORK_ERRORS, netErrCantContinue, error);
		NetExit();
		return false;
	} else {
		return true;
	}
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
  
	sCurrentGameProtocol->Exit1();
  
	// ZZZ: clean up SDL Time Manager emulation.  
	// true says wait for any late finishers to finish
	// (but does NOT say to kill anyone not already removed.)
	myTMCleanup(true);
  
	if (netState!=netUninitialized) {
		error= NetDDPCloseSocket(ddpSocket);
		if (!error) {
			NetSetSelfSend(sOldSelfSendStatus);
      
			free(topology);
			topology= NULL;
      
			sCurrentGameProtocol->Exit2();
      
			netState= netUninitialized;
		} else {
			logAnomaly("NetDDPCloseSocket returned %i", error);
		}
	}
  
	if (connection_to_server) {
		delete connection_to_server;
		connection_to_server = NULL;
	}

	if (server_nbc) {
		ConnectPool::instance()->abandon(server_nbc);
		server_nbc = 0;
	}
  
	{  
		client_map_t::iterator it;
		for (it = connections_to_clients.begin(); it != connections_to_clients.end(); it++)
			delete(it->second);
		connections_to_clients.clear();
	}
	
	{  
		client_chat_info_map_t::iterator it;
		for (it = client_chat_info.begin(); it != client_chat_info.end(); it++)
			delete(it->second);
		client_chat_info.clear();
	}

	sNetworkStats.clear();
  
	if (server) {
		delete server;
		server = NULL;
	}

	delete gMetaserverClient;
	gMetaserverClient = new MetaserverClient();
	
	if (controller)
	{
		open_progress_dialog(_closing_router_ports);
		LNat_Upnp_Remove_Port_Mapping(controller, GAME_PORT, "TCP");
		LNat_Upnp_Remove_Port_Mapping(controller, GAME_PORT, "UDP");
		LNat_Upnp_Controller_Free(&controller);
		controller = NULL;
		close_progress_dialog();
	}

	Console::instance()->unregister_command("ignore");
  
	NetDDPClose();

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
                              bool send_to_self,
	bool only_send_to_team)
{
	sCurrentGameProtocol->DistributeInformation(type, buffer, buffer_size, send_to_self, only_send_to_team);
}


short NetState(
	       void)
{
	return netState;
}

// Game session identifiers allow the metaserver to
// identify which players join a particular game.

std::string NetSessionIdentifier(void)
{
	return gameSessionIdentifier;
}

void NetInitializeSessionIdentifier(void)
{
	// A robust GUID would be even better here, but
	// all we really need is an ID unlikely to be
	// chosen by two gatherers at about the same time.
	gameSessionIdentifier.clear();
	for (int i = 0; i < 16; i++)
	{
		gameSessionIdentifier += static_cast<char>(rand() % 256);
	}
	
}


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
        
	NetInitializeTopology(game_data, game_data_size, player_data, player_data_size);
	NetInitializeSessionIdentifier();
	
	if (network_preferences->attempt_upnp)
	{
		// open the port!
		open_progress_dialog(_opening_router_ports);
		char public_ip[32];
		int ret = 0;
		if ((ret = LNat_Upnp_Discover(&controller)) != 0)
			logWarning("LibNAT: Failed to discover UPnP controller");
		if (ret == 0) 
			if ((ret = LNat_Upnp_Get_Public_Ip(controller, public_ip, 32)) != 0)
				logWarning("LibNAT: Failed to acquire public IP");
		if (ret == 0)
			if ((ret = LNat_Upnp_Set_Port_Mapping(controller, NULL, GAME_PORT, "TCP")) != 0)
				logWarning("LibNAT: Failed to map port %d (TCP)", GAME_PORT);
		if (ret == 0)
			if ((ret = LNat_Upnp_Set_Port_Mapping(controller, NULL, GAME_PORT, "UDP")) != 0)
				logWarning("LibNAT: Failed to map port %d (UDP)", GAME_PORT);
		close_progress_dialog();
		
		if (ret != 0)
		{
			controller = 0;
			alert_user(infoError, strNETWORK_ERRORS, netWarnUPnPConfigureFailed, ret);
		}
	}
	
	// Start listening for joiners
	server = new CommunicationsChannelFactory(GAME_PORT);
	
	netState= netGathering;

	client_chat_info[0] = new ClientChatInfo;
	client_chat_info[0]->name = player_preferences->name;
	client_chat_info[0]->color = player_preferences->color;
	client_chat_info[0]->team = player_preferences->team;
	
	return true;
}

void NetCancelGather(
	void)
{
	assert(netState==netGathering);

	NetDistributeTopology(tagCANCEL_GAME);
}

bool NetStart(
	void)
{
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

	NetDistributeTopology(resuming_saved_game ? tagRESUME_GAME : tagSTART_GAME);

	return true;
}

static int net_compare(
	void const *p1, 
	void const *p2)
{
	uint32 p1_host = SDL_SwapBE32(((const NetPlayer *)p1)->ddpAddress.host);
	uint32 p2_host = SDL_SwapBE32(((const NetPlayer *)p2)->ddpAddress.host);
	return p2_host - p1_host;
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

canÕt be called after the player has been gathered
*/

bool NetGameJoin(
	void *player_data,
	short player_data_size,
	const char* host_addr_string
	)
{
  /* Attempt a connection to host */

  if (server_nbc)
  {
	  ConnectPool::instance()->abandon(server_nbc);
	  server_nbc = 0;
  }

  host_address_specified = (host_addr_string != NULL);
  if (host_address_specified)
    {
	    uint16 port = DEFAULT_GAME_PORT;
	    std::string host_str = host_addr_string;
	    std::string::size_type pos = host_str.rfind(':');
	    if (pos != std::string::npos)
	    {
			port = atoi(host_str.substr(pos + 1).c_str());
			host_str = host_str.substr(0, pos);
	    }

	    nbc_is_resolving = true;
	    server_nbc = ConnectPool::instance()->connect(host_str.c_str(), port);
    }
  
    netState = netConnecting;
    
    NetInitializeTopology((void *) NULL, 0, player_data, player_data_size);
    return true;
}

void NetRetargetJoinAttempts(const IPaddress* inAddress)
{
	host_address_specified = (inAddress != NULL);
	if(host_address_specified)
	{
		host_address = *inAddress;
		// Aleph One 1.1 and earlier didn't send the gather port
		if(host_address.port == 0)
		{
			host_address.port = SDL_SwapBE16(DEFAULT_GAME_PORT);
		}
	}
}

void NetCancelJoin(
	void)
{
	assert(netState==netConnecting||netState==netJoining||netState==netWaiting||netState==netCancelled||netState==netJoinErrorOccurred);
}

void NetChangeColors(int16 color, int16 team) {
  assert(netState == netWaiting || netState == netGathering);
  
  if (netState == netWaiting) {
    ChangeColorsMessage changeColorsMessage(color, team);
    connection_to_server->enqueueOutgoingMessage(changeColorsMessage);
  } else if (netState == netGathering) {
    player_info *player = &topology->players[localPlayerIndex].player_data;
    if (player->desired_color != color ||
	player->team != team) {
      player->desired_color = color;
      player->team = team;

      Client::check_player(localPlayerIndex, topology->player_count);
      NetUpdateTopology();
      
      NetDistributeTopology(tagCHANGED_PLAYER);
    }
  }
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
	assert(netState!=netUninitialized&&netState!=netDown&&netState!=netJoining);

	return localPlayerIndex;
}

short NetGetPlayerIdentifier(
	short player_index)
{
	assert(netState!=netUninitialized&&netState!=netDown&&netState!=netJoining);
	assert(player_index>=0&&player_index<topology->player_count);
	
	return topology->players[player_index].identifier;
}

bool NetNumberOfPlayerIsValid(
	void)
{
	bool valid;

	switch(netState)
	{
		case netUninitialized:
		case netConnecting:
		case netJoining:
		case netJoinErrorOccurred:
			valid= false;
			break;
		default:
			valid= true;
			break;
	}
	
	return valid;
}

short NetGetNumberOfPlayers(
	void)
{
	assert(netState!=netUninitialized /* &&netState!=netDown*/ &&netState!=netJoining);
	
	return topology->player_count;
}

void *NetGetPlayerData(
	short player_index)
{
	assert(netState!=netUninitialized/* && netState!=netDown */ &&netState!=netJoining);
	assert(player_index>=0&&player_index<topology->player_count);
	
	return (void *) &topology->players[player_index].player_data;
}

void *NetGetGameData(
	void)
{
	assert(netState!=netUninitialized && netState!=netJoining);
	
	return &topology->game_data;
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
                        player_info* thePlayerInfo = (player_info*) &topology->players[s].player_data;
                        strncpy(thePlayerInfo->name, inStartArray[s].name, MAX_NET_PLAYER_NAME_LENGTH);
                        thePlayerInfo->name[MAX_NET_PLAYER_NAME_LENGTH] = '\0';
                        thePlayerInfo->desired_color = 0; // currently unused
                        thePlayerInfo->team = inStartArray[s].team;
                        thePlayerInfo->color = inStartArray[s].color;
                        memset(thePlayerInfo->long_serial_number, 0, LONG_SERIAL_NUMBER_LENGTH);
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
		
		if (address->host == player_address->host && address->port == player_address->port)
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

	/* initialize the local player (assume weÕre index zero, identifier zero) */
	localPlayerIndex= localPlayerIdentifier= 0;
	local_player= topology->players + localPlayerIndex;
	local_player->identifier= localPlayerIdentifier;
	local_player->net_dead= false;
	local_player->stream_id = 0;

	NetLocalAddrBlock(&local_player->dspAddress, GAME_PORT);
	NetLocalAddrBlock(&local_player->ddpAddress, ddpSocket);
	if (player_data_size > 0)
		memcpy(&local_player->player_data, player_data, player_data_size);
	
	/* initialize the network topology (assume weÕre the only player) */
	topology->player_count= 1;
	topology->nextIdentifier= 1;
	if (game_data_size > 0)
		memcpy(&topology->game_data, game_data, game_data_size);
	gameSessionIdentifier.clear();
}

static void NetLocalAddrBlock(
	NetAddrBlock *address,
	short socketNumber)
{
	
	address->host = 0x7f000001;	//!! XXX (ZZZ) yeah, that's really bad.
	address->port = socketNumber;	// OTOH, I guess others are set up to "stuff" the address they actually saw for us instead of
					// this, anyway... right??  So maybe it's not that big a deal.......
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
	if (localPlayerIndex==topology->player_count) fdprintf("couldn't find my identifier: %p", (void*)topology);
#endif
}



static bool NetSetSelfSend(
	bool on)
{
	return false;
}



/* ------ this needs to let the gatherer keep going if there was an error.. */
/* ¥¥¥ÊMarathon Specific Code ¥¥¥ */
/* Returns error code.. */
// ZZZ annotation: this function doesn't seem to belong here - maybe more like interface.cpp?
bool NetChangeMap(
	struct entry_point *entry)
{
	byte   *wad= NULL;
	int32   length;
	bool success= true;

	/* If the guy that was the server died, and we are trying to change levels, we lose */
        // ZZZ: if we used the parent_wad_checksum stuff to locate the containing Map file,
        // this would be the case somewhat less frequently, probably...
	if(localPlayerIndex==sServerPlayerIndex && localPlayerIndex != 0) {
	  logError("server died while trying to get another level");
	  success= false;
	} else {
	  // being the server, we must send out the map to everyone.	
	  if(localPlayerIndex==sServerPlayerIndex) {
	    wad = (unsigned char *)get_map_for_net_transfer(entry);
	    assert(wad);	      
	    
	    length= get_net_map_data_length(wad);
	    NetDistributeGameDataToAllPlayers(wad, length, true);
	  } else { // wait for de damn map.
	      wad = NetReceiveGameData(true);
	      if(!wad) {
		alert_user(infoError, strNETWORK_ERRORS, netErrCouldntReceiveMap, 0);
		success= false;
		
	      }
	  }
	  
	  /* Now load the level.. */
	  if (wad)
	    {
	      /* Note that this frees the wad as well!! */
	      process_net_map_data(wad);
	    }
	}
	
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
OSErr NetDistributeGameDataToAllPlayers(byte *wad_buffer, 
					int32 wad_length,
					bool do_physics)
{
	short playerIndex, message_id;
	OSErr error= noErr;
	int32 total_length;
	uint32 initial_ticks= machine_tick_count();
	short physics_message_id;
	byte *physics_buffer = NULL;
	int32 physics_length;
	
	message_id= (topology->player_count==2) ? (_distribute_map_single) : (_distribute_map_multiple);
	physics_message_id= (topology->player_count==2) ? (_distribute_physics_single) : (_distribute_physics_multiple);
	open_progress_dialog(physics_message_id);
	
	/* For updating our progress bar.. */
	total_length= (topology->player_count-1)*wad_length;
	
	/* Get the physics */
	if(do_physics)
		physics_buffer= (unsigned char *)get_network_physics_buffer(&physics_length);
	
	// build a list of players to send to
	std::vector<CommunicationsChannel *> channels;

	// also a list of who and who can not take compressed data
	std::vector<CommunicationsChannel *> zipCapableChannels;
	std::vector<CommunicationsChannel *> zipIncapableChannels;
	for (playerIndex = 0; playerIndex < topology->player_count; playerIndex++)
	{
		NetPlayer player = topology->players[playerIndex];

		/* If the player is not net dead. */ 
		// ZZZ: and is not going to be a zombie and is not us

		if (!player.net_dead && player.identifier != NONE && playerIndex != localPlayerIndex) 
		{
			Client *client = connections_to_clients[player.stream_id];
			channels.push_back(client->channel);
			if (client->capabilities[Capabilities::kZippedData] >= my_capabilities[Capabilities::kZippedData])
			{
				zipCapableChannels.push_back(client->channel);
			}
			else
			{
				zipIncapableChannels.push_back(client->channel);
			}
			
		}
		
	}

	set_progress_dialog_message(message_id);
	reset_progress_bar();
	
	if (physics_buffer)
	{
		if (zipCapableChannels.size())
		{
			ZippedPhysicsMessage zippedPhysicsMessage(physics_buffer, physics_length);
			std::unique_ptr<UninflatedMessage> uninflatedMessage(zippedPhysicsMessage.deflate());
			std::for_each(zipCapableChannels.begin(), zipCapableChannels.end(), boost::bind(&CommunicationsChannel::enqueueOutgoingMessage, _1, *uninflatedMessage));
		}

		if (zipIncapableChannels.size())
		{
			PhysicsMessage physicsMessage(physics_buffer, physics_length);
			std::for_each(zipIncapableChannels.begin(), zipIncapableChannels.end(), boost::bind(&CommunicationsChannel::enqueueOutgoingMessage, _1, physicsMessage));
		}
	}
	
	{
		// send zipped map to anyone who can accept it
		if (zipCapableChannels.size())
		{
			ZippedMapMessage zippedMapMessage(wad_buffer, wad_length);
			// zipped messages are compressed when deflated
			// since we may have to send this to multiple joiners,
			// deflate it now so that compression only happens once
			std::unique_ptr<UninflatedMessage> uninflatedMessage(zippedMapMessage.deflate());
			std::for_each(zipCapableChannels.begin(), zipCapableChannels.end(), boost::bind(&CommunicationsChannel::enqueueOutgoingMessage, _1, *uninflatedMessage));
		}

		if (zipIncapableChannels.size())
		{
			MapMessage mapMessage(wad_buffer, wad_length);
			std::for_each(zipIncapableChannels.begin(), zipIncapableChannels.end(), boost::bind(&CommunicationsChannel::enqueueOutgoingMessage, _1, mapMessage));
		}
	}

	if (do_netscript)
	{
		if (zipCapableChannels.size())
		{
			ZippedLuaMessage zippedLuaMessage(deferred_script_data, deferred_script_length);
			std::unique_ptr<UninflatedMessage> uninflatedMessage(zippedLuaMessage.deflate());
			std::for_each(zipCapableChannels.begin(), zipCapableChannels.end(), boost::bind(&CommunicationsChannel::enqueueOutgoingMessage, _1, *uninflatedMessage));
		}

		if (zipIncapableChannels.size())
		{
			LuaMessage luaMessage(deferred_script_data, deferred_script_length);
			std::for_each(zipIncapableChannels.begin(), zipIncapableChannels.end(), boost::bind(&CommunicationsChannel::enqueueOutgoingMessage, _1, luaMessage));
		}
	}

	{
		EndGameDataMessage endGameDataMessage;
		std::for_each(channels.begin(), channels.end(), boost::bind(&CommunicationsChannel::enqueueOutgoingMessage, _1, endGameDataMessage));
	}

	CommunicationsChannel::multipleFlushOutgoingMessages(channels, false, 30000, 30000);
	
	for (playerIndex = 0; playerIndex < topology->player_count; playerIndex++) {
		if (playerIndex != localPlayerIndex) {
			connections_to_clients[topology->players[playerIndex].stream_id]->state = Client::_ingame;
		}
	}

	
    
	if (error) { // ghs: nothing above returns an error at the moment,
		// but I'll leave so you know what error could be displayed
		alert_user(infoError, strNETWORK_ERRORS, netErrCouldntDistribute, error);
	} else if  (machine_tick_count()-initial_ticks>uint32(topology->player_count*MAP_TRANSFER_TIME_OUT)) {
		alert_user(infoError, strNETWORK_ERRORS, netErrWaitedTooLongForMap, error);
		error= 1;
	}
  
	if (!error) {
		/* Process the physics file & frees it!.. */
		if (physics_buffer)
			process_network_physics_model(physics_buffer);
		
		draw_progress_bar(total_length, total_length);
		
#ifdef HAVE_LUA
		if (do_netscript) {
			LoadLuaScript ((char*)deferred_script_data, deferred_script_length, _lua_netscript);
		}
#endif
	}
	
	close_progress_dialog();
	
	return error;
}

byte *NetReceiveGameData(bool do_physics)
{
  byte *map_buffer= NULL;
  
  open_progress_dialog(_awaiting_map);
  
  // handlers will take care of all messages, and when they're done
  // the server will send us this:
  std::unique_ptr<EndGameDataMessage> endGameDataMessage(connection_to_server->receiveSpecificMessage<EndGameDataMessage>((Uint32) 60000, (Uint32) 30000));
  if (endGameDataMessage.get()) {
    // game data was received OK
	  if (do_physics) {
      process_network_physics_model(handlerPhysicsBuffer);
      handlerPhysicsLength = 0;
      handlerPhysicsBuffer = NULL;
    }
    
    if (handlerMapLength > 0) {
      map_buffer = handlerMapBuffer;
      handlerMapBuffer = NULL;
      handlerMapLength = 0;
    }
    
    if (handlerLuaLength > 0) {
      do_netscript = true;
#ifdef HAVE_LUA
      LoadLuaScript((char *) handlerLuaBuffer, handlerLuaLength, _lua_netscript);
#endif
      handlerLuaBuffer = NULL;
      handlerLuaLength = 0;
    } else {
      do_netscript = false;
    }
    
    draw_progress_bar(10, 10);
    close_progress_dialog();
  } else {
    draw_progress_bar(10, 10);
    close_progress_dialog();
    
    if (handlerPhysicsLength > 0) {
      delete[] handlerPhysicsBuffer;
      handlerPhysicsBuffer = NULL;
      handlerPhysicsLength = 0;
    }
    if (handlerMapLength > 0) {
      delete[] handlerMapBuffer;
      handlerMapBuffer = NULL;
      handlerMapLength = 0;
    }
    if (handlerLuaLength > 0) {
      delete[] handlerLuaBuffer;
      handlerLuaBuffer = NULL;
      handlerLuaLength = 0;
    }
    
    alert_user(infoError, strNETWORK_ERRORS, netErrMapDistribFailed, 1);
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

int32
NetGetNetTime(void)
{
        return sCurrentGameProtocol->GetNetTime();
}

bool
NetCheckWorldUpdate()
{
	return sCurrentGameProtocol->CheckWorldUpdate();
}

extern const NetworkStats& hub_stats(int player_index);

void NetProcessMessagesInGame() {
	if (connection_to_server) {
		connection_to_server->pump();
		connection_to_server->dispatchIncomingMessages();
	} else {
		// update stats
		if (sCurrentGameProtocol == static_cast<NetworkGameProtocol*>(&sStarGameProtocol) && last_network_stats_send + network_stats_send_period < machine_tick_count())
		{
			std::vector<NetworkStats> stats(topology->player_count);
			for (int playerIndex = 0; playerIndex < topology->player_count; ++playerIndex)
			{
				stats[playerIndex] = hub_stats(playerIndex);
			}

			NetworkStatsMessage statsMessage(stats);
			for (int playerIndex = 0; playerIndex < topology->player_count; ++playerIndex)
			{
				NetPlayer player = topology->players[playerIndex];
				if (!player.net_dead && player.identifier != NONE && playerIndex != localPlayerIndex)
				{
					Client *client = connections_to_clients[player.stream_id];
					if (client->capabilities[Capabilities::kNetworkStats] >= Capabilities::kNetworkStatsVersion)
					{
						client->channel->enqueueOutgoingMessage(statsMessage);
					}
				}
			}
			
			last_network_stats_send = machine_tick_count();
		}

		// pump chat messages
		client_map_t::iterator it;
		for (it = connections_to_clients.begin(); it != connections_to_clients.end(); it++) {
			it->second->channel->pump();
			it->second->channel->dispatchIncomingMessages();
		}
	}

	if (gMetaserverClient && gMetaserverClient->isConnected())
		gMetaserverClient->pump();
}

// If a potential joiner has connected to us, handle em
bool NetCheckForNewJoiner (prospective_joiner_info &info)
{  
	CommunicationsChannel *new_joiner = server->newIncomingConnection();
	
	if (new_joiner) {
		
		new_joiner->setMessageInflater(inflater);
		Client *client = new Client(new_joiner);
		connections_to_clients[next_stream_id] = client;
		next_stream_id++;
		
		HelloMessage helloMessage(kNetworkSetupProtocolID);
		new_joiner->enqueueOutgoingMessage(helloMessage);
	}
	
	{
		client_map_t::iterator it = connections_to_clients.begin();
		while (it != connections_to_clients.end()) {
			if (it->second->channel->isConnected()) {
				it->second->channel->pump();
				it->second->channel->dispatchIncomingMessages();
				++it;
			} else {
				it->second->drop();
				delete connections_to_clients[it->first];
				connections_to_clients.erase(it++);
			}
		}
	}
	
	// now check to see if any one has actually connected
	client_map_t::iterator it = connections_to_clients.begin();
	while (it != connections_to_clients.end()) {
		if (it->second->state == Client::_connected_but_not_yet_shown) {
			info.stream_id = it->first;
			strncpy(info.name, it->second->name, MAX_NET_PLAYER_NAME_LENGTH);
			it->second->state = Client::_connected;
			info.gathering = false;
			return true;
		} else if (it->second->state == Client::_disconnect) {
			connections_to_clients[it->first]->drop();
			delete connections_to_clients[it->first];
			connections_to_clients.erase(it++);
		}
		else
		{
			++it;
		}
	}
	
	return false;    
}   
 
/* check for messages from gather nodes; returns new state */
short NetUpdateJoinState(
			 void)
{
  logContext("updating network join status");
  
  short newState= netState;
  
  switch (netState)
    {
    case netConnecting:	
      // trying to connect to gatherer
      // jkvw: Here's what's up - We want to be able to click join before gatherer
      //	 has started gathering.  So after a join click we attempt a connection
      //	 every five seconds.  This is fine so long as remote computer quickly
      //	 refuses connection each time we poll; the dialog will remain responsive.
      //	 It's possible that a connection attempt will take a long time, however,
      //	 so we look for that and abort dialog when it happens.

	    // ghs: it's possible the guy hasn't opened his firewall yet, so always retry
	    if (machine_tick_count() >= next_join_attempt) {
		    if(host_address_specified)
		    {
			    if (!server_nbc)
			    {
				    server_nbc = ConnectPool::instance()->connect(host_address);
			    }
		    }

		    if (server_nbc)
		    {
			    if (server_nbc->status() == NonblockingConnect::Connected)
			    {
				    newState = netJoining;
				    handlerState = netAwaitingHello;
				    connection_to_server = server_nbc->release();
				    ConnectPool::instance()->abandon(server_nbc);
				    server_nbc = 0;
				    if (!connection_to_server->isConnected())
				    {
					    newState= netJoinErrorOccurred;
					    alert_user(infoError, strNETWORK_ERRORS, netErrCouldntJoin, 3);  
				    }
				    else
				    {
					    connection_to_server->setMessageInflater(inflater);
					    connection_to_server->setMessageHandler(joinDispatcher);
				    }
			    }
			    else if (server_nbc->status() == NonblockingConnect::ResolutionFailed)
			    {
				    // name resolution error isn't recoverable
				    ConnectPool::instance()->abandon(server_nbc);
				    server_nbc = 0;

				    newState = netJoinErrorOccurred;
				    alert_user(infoError, strNETWORK_ERRORS, netErrCouldntResolve, 0);
			    }
			    else if (server_nbc->status() == NonblockingConnect::ConnectFailed)
			    {
				    assert(host_address_specified);
				    if (nbc_is_resolving)
					    host_address = server_nbc->address();
				    nbc_is_resolving = false;
				    ConnectPool::instance()->abandon(server_nbc);
				    server_nbc = ConnectPool::instance()->connect(host_address);
			    }
		    }

		    next_join_attempt = machine_tick_count() + 5*MACHINE_TICKS_PER_SECOND;
	    }
	    break;
      
    case netJoining:	// waiting to be gathered
      if (!connection_to_server->isConnected ()) {
	newState = netJoinErrorOccurred;
	alert_user(infoError, strNETWORK_ERRORS, netErrLostConnection, 0);
      } else {
	connection_to_server->pump();
	connection_to_server->dispatchIncomingMessages();
	if (handlerState == netWaiting) {	  
	  newState= netWaiting;
	} else if (handlerState == netJoinErrorOccurred) {
	  newState= netJoinErrorOccurred;
	}
      }
      break;
      // netJoining
      
    case netWaiting:	// have been gathered, waiting for other players / game start
      if (!connection_to_server->isConnected ()) {
	newState = netJoinErrorOccurred;
	alert_user(infoError, strNETWORK_ERRORS, netErrLostConnection, 0);
      } else {
	handlerState = netWaiting;
	connection_to_server->pump();
	connection_to_server->dispatchOneIncomingMessage();
	if (handlerState != netWaiting) {
	  newState = handlerState;
	}
      }
      break;
      // netWaiting
      
    default:
      newState= NONE;
      break;
    }
  
  /* return netPlayerAdded to tell the caller to refresh his topology, but donÕt change netState to that */
  // ZZZ: similar behavior for netChatMessageReceived and netStartingResumeGame
  if (newState!=netPlayerAdded && newState!=netPlayerDropped && newState!=netPlayerChanged && newState != netChatMessageReceived && newState != netStartingResumeGame && newState != NONE)
    netState= newState;
  
  // ZZZ: netStartingResumeGame is used as a return value only; the corresponding netState is netStartingUp.
  if(newState == netStartingResumeGame)
    netState = netStartingUp;
  
  return newState;
}

int NetGatherPlayer(const prospective_joiner_info &player,
  CheckPlayerProcPtr check_player)
{
  assert(netState == netGathering);
  assert(topology->player_count < MAXIMUM_NUMBER_OF_NETWORK_PLAYERS);

  Client::check_player = check_player;

  JoinPlayerMessage joinPlayerMessage(topology->nextIdentifier++);
  connections_to_clients[player.stream_id]->channel->enqueueOutgoingMessage(joinPlayerMessage);
  connections_to_clients[player.stream_id]->state = Client::_awaiting_accept_join;

  /*
  // reject a player if he can't handle our script demands
  ScriptMessage scriptMessage(_netscript_query_message);
  connections_to_clients[player.stream_id]->channel->enqueueOutgoingMessage(scriptMessage);
  connections_to_clients[player.stream_id]->state = Client::_awaiting_script_message;
  */

  // lie to the network code and say we gathered successfully
  return kGatherPlayerSuccessful;
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

connect to everyoneÕs dspAddress and give them the latest copy of the network topology.  this
used to be NetStart() and it used to connect all upring and downring ADSP connections.
*/
static void NetDistributeTopology(
	short tag)
{
	short playerIndex;
	
	assert(netState==netGathering);
	
	topology->tag= tag;

	TopologyMessage topologyMessage(topology);
        
	for (playerIndex=0; playerIndex<topology->player_count; ++playerIndex)
	  {
	    // ZZZ: skip players with identifier NONE - they don't really exist... also skip ourselves.
	    if(topology->players[playerIndex].identifier != NONE && playerIndex != localPlayerIndex)
	      {
		CommunicationsChannel *channel = connections_to_clients[topology->players[playerIndex].stream_id]->channel;
		channel->enqueueOutgoingMessage(topologyMessage);
	      }
	  }
}

NetAddrBlock *NetGetPlayerADSPAddress(
	short player_index)
{
	return &topology->players[player_index].dspAddress;
}

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

bool NetAllowCarnageMessages() {
	return (dynamic_world->player_count == 1 ||
		!(dynamic_world->game_information.cheat_flags & _disable_carnage_messages));
}

bool NetAllowSavingLevel() {
	return (dynamic_world->player_count == 1 ||
		localPlayerIndex == sServerPlayerIndex ||
		!(dynamic_world->game_information.cheat_flags & _disable_saving_level));

}

bool NetAllowOverlayMap() {
	return (dynamic_world->player_count == 1 ||
			(dynamic_world->game_information.cheat_flags & _allow_overlay_map));
}


extern int32 spoke_latency();

int32 NetGetLatency() {
	if (sCurrentGameProtocol == static_cast<NetworkGameProtocol*>(&sStarGameProtocol) && connection_to_server) {
		return spoke_latency();
	} else {
		return NetworkStats::invalid;
	}
}

const NetworkStats& NetGetStats(int player_index)
{
	if (sCurrentGameProtocol == static_cast<NetworkGameProtocol*>(&sStarGameProtocol))
	{
		if (connection_to_server)
		{
			if (player_index < sNetworkStats.size())
			{
				return sNetworkStats[player_index];
			}
			else
			{
				return sInvalidStats;
			}
		}
		else
		{
			return hub_stats(player_index);
		}
	}
	else
	{
		return sInvalidStats;
	}
}

int32 NetGetUnconfirmedActionFlagsCount()
{
	assert (sCurrentGameProtocol);
	return sCurrentGameProtocol->GetUnconfirmedActionFlagsCount();
}

uint32 NetGetUnconfirmedActionFlag(int32 offset)
{
	assert (sCurrentGameProtocol);
	return sCurrentGameProtocol->PeekUnconfirmedActionFlag(offset);
}

void NetUpdateUnconfirmedActionFlags()
{
	assert (sCurrentGameProtocol);
	return sCurrentGameProtocol->UpdateUnconfirmedActionFlags();
}

#endif // !defined(DISABLE_NETWORKING)

