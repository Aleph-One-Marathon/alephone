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

#include <SDL2/SDL_thread.h>

#include "game_errors.h"
#include "Console.h"
#include "MessageDispatcher.h"
#include "MessageInflater.h"
#include "MessageHandler.h"
#include "PortForward.h"
#include "progress.h"
#include "extensions.h"
#include "player.h"
#include <memory>
#include <stdlib.h>
#include <string.h>
#include <map>
#include <vector>
#include "Logging.h"

// ZZZ: moved many struct definitions, constant #defines, etc. to header for (limited) sharing
#include "network_private.h"

#include "network_messages.h"

#include "NetworkGameProtocol.h"

#include "StarGameProtocol.h"

#include "lua_script.h"

#include "network_metaserver.h"

#include "ConnectPool.h"

#ifdef A1_NETWORK_STANDALONE_HUB
#include "StandaloneHub.h"
#endif


/* ---------- globals */

static short localPlayerIndex;
static short localPlayerIdentifier;
static std::string gameSessionIdentifier;
static NetTopology* topology;
static StarGameProtocol sCurrentGameProtocol;
static std::unique_ptr<NetworkInterface> network_interface;
static std::vector<byte> deferred_script;
static CommunicationsChannelFactory *server = NULL;
static bool use_remote_hub = false;
static byte* resumed_wad_data_for_remote_hub = NULL;
static int resumed_wad_size_for_remote_hub = 0;
typedef std::map<int, Client *> client_map_t;
static client_map_t connections_to_clients;
typedef std::map<int, ClientChatInfo *> client_chat_info_map_t;
static client_chat_info_map_t client_chat_info;
static std::unique_ptr<CommunicationsChannel> connection_to_server = NULL;
static NonblockingConnect *server_nbc = 0;
static bool nbc_is_resolving = false;
static int next_stream_id = 1; // 0 is local player
static IPaddress host_address;
static bool host_address_specified = false;
static MessageInflater *inflater = NULL;
static MessageDispatcher *joinDispatcher = NULL;
static uint64_t next_join_attempt;
static Capabilities my_capabilities;
static std::shared_ptr<Pinger> pinger = nullptr; //multithread safety
static GatherCallbacks *gatherCallbacks = NULL;
static ChatCallbacks *chatCallbacks = NULL;

#ifdef HAVE_MINIUPNPC
static std::unique_ptr<PortForward> port_forward;
#endif

extern MetaserverClient* gMetaserverClient;

static std::vector<NetworkStats> sNetworkStats;
const static NetworkStats sInvalidStats = {
	NetworkStats::invalid,
	NetworkStats::invalid,
	NetworkStats::invalid,
	0
};
uint64_t last_network_stats_send = 0;
const static int network_stats_send_period = MACHINE_TICKS_PER_SECOND;

// ignore list
static std::set<int> sIgnoredPlayers;

static bool player_is_ignored(int player_index)
{
	return (sIgnoredPlayers.find(player_index) != sIgnoredPlayers.end());
}

static bool local_is_server()
{
	return !connection_to_server;
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

// ZZZ note: very few folks touch the streaming data, so the data-format issues outlined above with
// datagrams (the data from which are passed around, interpreted, and touched by many functions)
// don't matter as much.  Do observe, though, that users of the "distribution" mechanism will have
// to pack and unpack their own distribution data - we can't be expected to know what they're doing.

// ZZZ note: read this externally with the NetState() function.
static short netState= netUninitialized;

// ZZZ: are we trying to start a new game or resume a saved-game?
// This is only valid on the gatherer after NetGather() is called;
// only valid on a joiner once he receives the final topology (tagRESUME_GAME)
// Used, at least, on the gatherer to determine whether or not to resort players by address
static bool resuming_saved_game = false;


/* ---------- private prototypes */
void NetInitializeSessionIdentifier(void);

// ZZZ: cmon, we're not fooling anyone... game_data is a game_info*; player_data is a player_info*
// Originally I guess the plan was to have a strong separation between Marathon game code and the networking code,
// such that they could be compiled independently and only know about each other at link-time, but I don't see any
// reason to try to keep that... and I suspect Jason abandoned this separation long ago anyway.
// For now, the only effect I see is a reduction in type-safety.  :)
static void NetInitializeTopology(void *game_data, short game_data_size, void *player_data, short player_data_size);

static void NetUpdateTopology(void);
static void NetDistributeTopology(short tag);

static void NetDDPPacketHandler(UDPpacket& inPacket);

int getStreamIdFromChannel(CommunicationsChannel *channel) {
  client_map_t::iterator it;
  for (it = connections_to_clients.begin(); it != connections_to_clients.end(); it++) {
    if (it->second->channel.get() == channel) {
      return it->first;
    }
  }
  return -1;
}

//-----------------------------------------------------------------------------
// Message handlers
//-----------------------------------------------------------------------------

CheckPlayerProcPtr Client::check_player = 0;

Client::Client(std::shared_ptr<CommunicationsChannel> inChannel) : channel(inChannel), state(_connecting), network_version(0), mDispatcher(new MessageDispatcher())
{
	std::fill_n(name, MAX_NET_PLAYER_NAME_LENGTH, '\0');
	mJoinerInfoMessageHandler.reset(newMessageHandlerMethod(this, &Client::handleJoinerInfoMessage));
	mCapabilitiesMessageHandler.reset(newMessageHandlerMethod(this, &Client::handleCapabilitiesMessage));
	mAcceptJoinMessageHandler.reset(newMessageHandlerMethod(this, &Client::handleAcceptJoinMessage));
	mChatMessageHandler.reset(newMessageHandlerMethod(this, &Client::handleChatMessage));
	mChangeColorsMessageHandler.reset(newMessageHandlerMethod(this, &Client::handleChangeColorsMessage));
	mRemoteHubCommandMessageHandler.reset(newMessageHandlerMethod(this, &Client::handleRemoteHubCommandMessage));
	mRemoteHubHostRequestMessageHandler.reset(newMessageHandlerMethod(this, &Client::handleRemoteHubHostConnectMessage));
	mUnexpectedMessageHandler.reset(newMessageHandlerMethod(this, &Client::unexpectedMessageHandler));
	mDispatcher->setDefaultHandler(mUnexpectedMessageHandler.get());
	mDispatcher->setHandlerForType(mJoinerInfoMessageHandler.get(), JoinerInfoMessage::kType);
	mDispatcher->setHandlerForType(mCapabilitiesMessageHandler.get(), CapabilitiesMessage::kType);
	mDispatcher->setHandlerForType(mAcceptJoinMessageHandler.get(), AcceptJoinMessage::kType);
	mDispatcher->setHandlerForType(mChatMessageHandler.get(), NetworkChatMessage::kType);
	mDispatcher->setHandlerForType(mChangeColorsMessageHandler.get(), ChangeColorsMessage::kType);
	mDispatcher->setHandlerForType(mRemoteHubCommandMessageHandler.get(), RemoteHubCommandMessage::kType);
	mDispatcher->setHandlerForType(mRemoteHubHostRequestMessageHandler.get(), RemoteHubHostConnectMessage::kType);
	channel->setMessageHandler(mDispatcher.get());
}

void Client::drop()
{
	int stream_id = getStreamIdFromChannel(channel.get());

	if (state == _connected) { // (remove from the list of joinable players)
		if (gatherCallbacks) {
			prospective_joiner_info player;
			player.stream_id = getStreamIdFromChannel(channel.get());
			gatherCallbacks->JoiningPlayerDropped(&player);
		}
	} else if (state == _awaiting_map) { // need to remove from topo

#ifdef A1_NETWORK_STANDALONE_HUB
		int i = 0;
#else
		int i = 1;
#endif

		for (; i < topology->player_count; i++) {
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
				player.stream_id = stream_id;
				gatherCallbacks->JoinedPlayerDropped(&player);
			}

			if (gMetaserverClient && gMetaserverClient->isConnected())
			{
				gMetaserverClient->announcePlayersInGame(NetGetNumberOfPlayers());
			}

			NetDistributeTopology(tagDROPPED_PLAYER);
		} else {
			logAnomaly("a client in state _awaiting_map dropped, but was not found in the topology");
		}
	}

	if (client_chat_info[stream_id]) { //we want this to be sent after the topology update for remote hubs
		ClientInfoMessage clientInfoMessage(stream_id, client_chat_info[stream_id], (int16)ClientInfoMessage::kRemove);
		client_map_t::iterator it;
		for (it = connections_to_clients.begin(); it != connections_to_clients.end(); it++) {
			if (it->second->can_pregame_chat()) {
				it->second->channel->enqueueOutgoingMessage(clientInfoMessage);
			}
		}

		delete client_chat_info[stream_id];
		client_chat_info.erase(stream_id);
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

	if (capabilities[Capabilities::kGameworld] < my_capabilities[Capabilities::kGameworld] || (shapes_file_is_m1() && capabilities[Capabilities::kGameworldM1] < my_capabilities[Capabilities::kGameworldM1]))
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
		} else if (capabilities[Capabilities::kStar] < my_capabilities[Capabilities::kStar]) {
			if (warn_joiner) {
				ServerWarningMessage serverWarningMessage(expand_app_variables("The gatherer is using a newer version of $appName$. You will not appear in the list of available players."), ServerWarningMessage::kJoinerUngatherable);
				channel->enqueueOutgoingMessage(serverWarningMessage);
			}
			return false;
		}
	}

	if (deferred_script.size())
	{
		if (capabilities[Capabilities::kLua] == 0) {
			if (warn_joiner) {
				char s[256];
				ServerWarningMessage serverWarningMessage(getcstr(s, strNETWORK_ERRORS, netWarnJoinerNoLua), ServerWarningMessage::kJoinerUngatherable);
				channel->enqueueOutgoingMessage(serverWarningMessage);
			}
			return false;
		} else if (capabilities[Capabilities::kLua] < my_capabilities[Capabilities::kLua])
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

      int16 stream_id = getStreamIdFromChannel(channel.get());
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

void Client::handleCapabilitiesMessage(CapabilitiesMessage* capabilitiesMessage, CommunicationsChannel * channel)
{
#ifdef A1_NETWORK_STANDALONE_HUB
	if (state == _disconnect) return;
#endif

	if (state == _awaiting_capabilities) {
		capabilities = *capabilitiesMessage->capabilities();
		
		if (capabilities_indicate_player_is_gatherable(_warn_joiner)) {
			state = Client::_connected_but_not_yet_shown;

#ifdef A1_NETWORK_STANDALONE_HUB
			int stream_id = getStreamIdFromChannel(channel);

			if (channel == StandaloneHub::Instance()->GetGathererChannel())
			{
				prospective_joiner_info gatherer = {};
				gatherer.stream_id = stream_id;
				gatherer.gathering = true;

				if (!NetGatherPlayer(gatherer, reassign_player_colors))
				{
					state = Client::_ungatherable;
				}
			}
			else
			{
				auto client = client_chat_info[stream_id];
				prospective_joiner_info joiner = {};
				joiner.stream_id = stream_id;
				std::strncpy(joiner.name, client->name.c_str(), client->name.length());
				joiner.color = client->color;
				joiner.team = client->team;
				JoinerInfoMessage joinerInfoMessage(&joiner, kNetworkSetupProtocolID);
				StandaloneHub::Instance()->SendMessageToGatherer(joinerInfoMessage);
			}
#endif // A1_NETWORK_STANDALONE_HUB
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

NetworkInterface* NetGetNetworkInterface()
{
	return network_interface.get();
}

void Client::handleAcceptJoinMessage(AcceptJoinMessage* acceptJoinMessage,
				     CommunicationsChannel *)
{
  if (state == _awaiting_accept_join) {
    if (acceptJoinMessage->accepted()) {
      topology->players[topology->player_count] = *acceptJoinMessage->player();
      topology->players[topology->player_count].stream_id = getStreamIdFromChannel(channel.get());
	  topology->players[topology->player_count].net_dead = false;
      prospective_joiner_info player;
      player.stream_id = topology->players[topology->player_count].stream_id;
      topology->players[topology->player_count].dspAddress = channel->peerAddress();
	  topology->players[topology->player_count].ddpAddress.set_address(channel->peerAddress().address());
      
      topology->player_count += 1;
      if (check_player) check_player(topology->player_count - 1, topology->player_count);
      NetUpdateTopology();
 
      GameSessionMessage gameSessionMessage(reinterpret_cast<const uint8*>(gameSessionIdentifier.c_str()), gameSessionIdentifier.size());
      auto& channel = connections_to_clients[player.stream_id]->channel;
      channel->enqueueOutgoingMessage(gameSessionMessage);

      NetDistributeTopology(tagNEW_PLAYER);
      state = _awaiting_map;

	  if (gatherCallbacks) 
	  {
		  gatherCallbacks->JoinSucceeded(&player);

		  if (gMetaserverClient && gMetaserverClient->isConnected())
		  {
			  gMetaserverClient->announcePlayersInGame(NetGetNumberOfPlayers());
		  }
	  }

#ifdef A1_NETWORK_STANDALONE_HUB
	  if (channel.get() == StandaloneHub::Instance()->GetGathererChannel())
		  StandaloneHub::Instance()->GathererJoinedAsClient();
	  else
		  StandaloneHub::Instance()->SendMessageToGatherer(*acceptJoinMessage);
#endif

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

#ifdef A1_NETWORK_STANDALONE_HUB
		int i = 0;
#else
		int i = 1;
#endif

		for (; i < topology->player_count; i++) {
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

				if (check_player) check_player(i, topology->player_count);
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

void Client::handleRemoteHubHostConnectMessage(RemoteHubHostConnectMessage* message, CommunicationsChannel* channel)
{
	channel->enqueueOutgoingMessage(RemoteHubHostResponseMessage(false)); // we already have a gatherer using us (the remote hub)
	state = Client::_disconnect;
}

void Client::handleRemoteHubCommandMessage(RemoteHubCommandMessage* message, CommunicationsChannel*)
{
#ifdef A1_NETWORK_STANDALONE_HUB
	if (channel.get() == StandaloneHub::Instance()->GetGathererChannel())
	{
		switch (message->command())
		{
			case RemoteHubCommand::kAcceptJoiner_Command:
			{
				if (NetGetNumberOfPlayers() < MAXIMUM_NUMBER_OF_PLAYERS)
				{
					prospective_joiner_info info = {};
					info.stream_id = message->data();
					NetGatherPlayer(info, reassign_player_colors);
				}
				break;
			}
			case RemoteHubCommand::kStartGame_Command:
				StandaloneHub::Instance()->StartGame();
				break;
			case RemoteHubCommand::kEndGame_Command:
				dynamic_world->tick_count = message->data();
				StandaloneHub::Instance()->SetGameEnded(true);
				break;
			default:
				logAnomaly("unexpected command received from remote hub message (command is %d)", message->command());
				break;
		}

		return;
	}
#endif

	logAnomaly("unexpected remote hub command message received; ignoring");
}

void Client::handleChatMessage(NetworkChatMessage* netChatMessage, 
			       CommunicationsChannel *)
{
	// relay this to all clients
	if (state == _ingame) {
		assert(netState == netActive);
		if (netChatMessage->target() == NetworkChatMessage::kTargetPlayers) {
			NetworkChatMessage chatMessage(netChatMessage->chatText(), getStreamIdFromChannel(channel.get()), NetworkChatMessage::kTargetPlayers);
			client_map_t::iterator it;
			for (it = connections_to_clients.begin(); it != connections_to_clients.end(); it++) {
				if (it->second->state == _ingame) {
					it->second->channel->enqueueOutgoingMessage(chatMessage);
				}
			}
      
			// display it locally
			if (chatCallbacks) {
				for (int playerIndex = 0; playerIndex < topology->player_count; playerIndex++) {
					if (topology->players[playerIndex].stream_id == getStreamIdFromChannel(channel.get())) {
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
			NetworkChatMessage chatMessage(netChatMessage->chatText(), getStreamIdFromChannel(channel.get()), NetworkChatMessage::kTargetClients);
			client_map_t::iterator it;
			for (it = connections_to_clients.begin(); it != connections_to_clients.end(); it++) {
				if (it->second->can_pregame_chat()) {
					it->second->channel->enqueueOutgoingMessage(chatMessage);
				}
			}
			if (chatCallbacks) {
				int stream_id = getStreamIdFromChannel(channel.get());
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
		if (capabilities[Capabilities::kGameworld] < my_capabilities[Capabilities::kGameworld] || (shapes_file_is_m1() && capabilities[Capabilities::kGameworldM1] < my_capabilities[Capabilities::kGameworldM1]) || (network_preferences->game_protocol == _network_game_protocol_star && capabilities[Capabilities::kStar] < my_capabilities[Capabilities::kStar]))
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

static void handleClientInfoMessage(ClientInfoMessage* clientInfoMessage, CommunicationsChannel * channel) {
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

			if (use_remote_hub)
			{
				if (gatherCallbacks)
				{
					prospective_joiner_info joiner_info = {};
					joiner_info.stream_id = clientInfoMessage->stream_id();
					joiner_info.color = clientInfoMessage->info()->color;
					joiner_info.team = clientInfoMessage->info()->team;
					std::strncpy(joiner_info.name, clientInfoMessage->info()->name.c_str(), clientInfoMessage->info()->name.length());

					if (!gatherCallbacks->JoiningPlayerDropped(&joiner_info)) //we don't really know the joiner state here so we deduce it like this
					{ 
						gatherCallbacks->JoinedPlayerDropped(&joiner_info);

						if (gMetaserverClient && gMetaserverClient->isConnected())
						{
							gMetaserverClient->announcePlayersInGame(NetGetNumberOfPlayers());
						}
					}
				}
			}

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

	assert(localPlayerIndex != NONE);
    
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
  }
  else {
    logAnomaly("unexpected join player message received (netState is %i)", netState);
  }
}

static std::vector<byte> handlerLuaBuffer;

static void handleLuaMessage(BigChunkOfDataMessage *luaMessage, CommunicationsChannel *) {
  if (netState == netStartingUp || netState == netDown) {
	handlerLuaBuffer = std::vector<byte>(luaMessage->buffer(), luaMessage->buffer() + luaMessage->length());
  } else {
    logAnomaly("unexpected lua message received (netState is %i)", netState);
  }
}

static byte *handlerMapBuffer = NULL;
static size_t handlerMapLength = 0;

static void handleMapMessage(BigChunkOfDataMessage *mapMessage, CommunicationsChannel *) {
	if (netState == netStartingUp || netState == netDown) {
		if (handlerMapBuffer) { // assume the last map the server sent is right
			delete[] handlerMapBuffer;
			handlerMapBuffer = NULL;
		}
		handlerMapLength = mapMessage->length();
		if (handlerMapLength > 0) {
			handlerMapBuffer = new byte[handlerMapLength];
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

static std::vector<byte> handlerPhysicsBuffer;

static void handlePhysicsMessage(BigChunkOfDataMessage *physicsMessage, CommunicationsChannel *) {
	if (netState == netStartingUp || netState == netDown) {
		handlerPhysicsBuffer = std::vector<byte>(physicsMessage->buffer(), physicsMessage->buffer() + physicsMessage->length());
	} else {
		logAnomaly("unexpected physics message received (netState is %i)", netState);
	}
}

static void handleServerWarningMessage(ServerWarningMessage *serverWarningMessage, CommunicationsChannel *) {
  char *s = strdup(serverWarningMessage->string()->c_str());
  alert_user(s);
  free(s);
}


static void handleTopologyMessage(TopologyMessage* topologyMessage, CommunicationsChannel *) {
  if (netState == netWaiting) {
    *topology = *(topologyMessage->topology());

     auto peer_address = connection_to_server->peerAddress();

      topology->server.dspAddress = peer_address;
	  topology->server.ddpAddress.set_address(peer_address.address());
      
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

static void handleJoinerInfoMessage(JoinerInfoMessage* joinerInfoMessage, CommunicationsChannel*) {
	if (use_remote_hub)
	{
		if (gatherCallbacks)
		{
			gatherCallbacks->JoiningPlayerArrived(joinerInfoMessage->info());
		}
	}
	else
	{
		logAnomaly("unexpected joiner info message received");
	}
}

static void handleAcceptJoinMessage(AcceptJoinMessage* acceptJoinMessage, CommunicationsChannel*) {
	if (use_remote_hub)
	{
		if (acceptJoinMessage->accepted() && gatherCallbacks)
		{
			prospective_joiner_info info = {};
			info.stream_id = acceptJoinMessage->player()->stream_id;
			gatherCallbacks->JoinSucceeded(&info);

			if (gMetaserverClient && gMetaserverClient->isConnected())
			{
				gMetaserverClient->announcePlayersInGame(NetGetNumberOfPlayers());
			}
		}
	}
	else
	{
		logAnomaly("unexpected accept join message received");
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
static TypedMessageHandlerFunction<AcceptJoinMessage> acceptJoinMessageHandler(&handleAcceptJoinMessage);
static TypedMessageHandlerFunction<JoinerInfoMessage> joinerInfoMessageHandler(&handleJoinerInfoMessage);
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

bool NetEnter(bool use_remote_hub)
{
	::use_remote_hub = use_remote_hub;
	network_interface = std::make_unique<NetworkInterface>();
  
	assert(netState==netUninitialized);
  
	{
		static bool added_exit_procedure= false;
    
		if (!added_exit_procedure) atexit(NetExit);
		added_exit_procedure= true;
	}

	topology = new NetTopology();

	bool success = NetDDPOpenSocket(GAME_PORT, NetDDPPacketHandler);
	if (success) {
		sCurrentGameProtocol.Enter(&netState);
		netState = netDown;
		handlerState = netDown;
	}
	else {
		logError("unable to open socket");
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
		inflater->learnPrototype(RemoteHubCommandMessage());
		inflater->learnPrototype(RemoteHubReadyMessage());
		inflater->learnPrototype(RemoteHubHostResponseMessage());
		inflater->learnPrototype(RemoteHubHostConnectMessage());
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
		joinDispatcher->setHandlerForType(&acceptJoinMessageHandler, AcceptJoinMessage::kType);
		joinDispatcher->setHandlerForType(&joinerInfoMessageHandler, JoinerInfoMessage::kType);
	}

	my_capabilities.clear();
	my_capabilities[Capabilities::kGameworld] = Capabilities::kGameworldVersion;
	my_capabilities[Capabilities::kGameworldM1] = Capabilities::kGameworldM1Version;
	if (network_preferences->game_protocol == _network_game_protocol_star) {
		my_capabilities[Capabilities::kStar] = Capabilities::kStarVersion;
	}
	my_capabilities[Capabilities::kLua] = Capabilities::kLuaVersion;
	my_capabilities[Capabilities::kGatherable] = Capabilities::kGatherableVersion;
	my_capabilities[Capabilities::kZippedData] = Capabilities::kZippedDataVersion;
	my_capabilities[Capabilities::kNetworkStats] = Capabilities::kNetworkStatsVersion;
	my_capabilities[Capabilities::kRugby] = Capabilities::kRugbyVersion;

	// net commands!
	sIgnoredPlayers.clear();
	CommandParser IgnoreParser;
	IgnoreParser.register_command("player", ignore_player());

	ResetLuaMute();
	IgnoreParser.register_command("lua", ignore_lua());

	Console::instance()->register_command("ignore", IgnoreParser);

	next_join_attempt = last_network_stats_send = machine_tick_count();
  
	if (!success) {
#ifndef A1_NETWORK_STANDALONE_HUB
		alert_user(infoError, strNETWORK_ERRORS, netErrCantContinue, -1);
#endif
		NetExit();
		return false;
	} else {
		return true;
	}
}

void NetSetDefaultInflater(CommunicationsChannel* channel)
{
	channel->setMessageInflater(inflater);
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
	// ZZZ: clean up SDL Time Manager emulation.  
	// true says wait for any late finishers to finish
	// (but does NOT say to kill anyone not already removed.)
	myTMCleanup();
  
	if (netState!=netUninitialized) {
		if (NetDDPCloseSocket()) {
			delete topology;
			topology = nullptr;
			netState= netUninitialized;
		} else {
			logAnomaly("NetDDPCloseSocket failed");
		}
	}
  
	connection_to_server.reset();

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
	deferred_script.clear();
	handlerLuaBuffer.clear();
	handlerPhysicsBuffer.clear();
  
	if (server) {
		delete server;
		server = NULL;
	}

	if (gMetaserverClient)
	{
		delete gMetaserverClient;
		gMetaserverClient = nullptr;
	}

	network_interface.reset();
	
	Console::instance()->unregister_command("ignore");
}

bool
NetSync()
{
	return sCurrentGameProtocol.Sync(topology, dynamic_world->tick_count, localPlayerIndex, local_is_server());
}

bool
NetUnSync()
{
	if (use_remote_hub)
	{
		NetRemoteHubSendCommand(RemoteHubCommand::kEndGame_Command, dynamic_world->tick_count);
	}

	return sCurrentGameProtocol.UnSync(true, dynamic_world->tick_count);
}

std::weak_ptr<Pinger>
NetGetPinger()
{
	return std::weak_ptr<Pinger>(pinger);
}

void
NetCreatePinger()
{
	if (!pinger)
	{
		pinger = std::make_shared<Pinger>();
	}
}

void
NetRemovePinger()
{
	pinger.reset();
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

bool NetGather(
	void *game_data,
	short game_data_size,
	void *player_data,
	short player_data_size,
	bool resuming_game,
	bool attempt_upnp)
{
        resuming_saved_game = resuming_game;
        
	NetInitializeTopology(game_data, game_data_size, player_data, player_data_size);
	NetInitializeSessionIdentifier();

#ifdef HAVE_MINIUPNPC
	if (!port_forward && attempt_upnp && !use_remote_hub)
	{
		open_progress_dialog(_opening_router_ports);
		try
		{
			port_forward.reset(new PortForward(4226));
			close_progress_dialog();
		}
		catch (const PortForwardException& e)
		{
			logWarning("miniupnpc: %s", e.what());
			close_progress_dialog();
			alert_user(infoError, strNETWORK_ERRORS, netWarnUPnPConfigureFailed, -1);
		}
	}
	else if (port_forward && !attempt_upnp)
	{
		port_forward.reset();
	}
#endif

	netState = netGathering;

#ifndef A1_NETWORK_STANDALONE_HUB
	// Start listening for joiners
	if (!use_remote_hub)
	{
		server = new CommunicationsChannelFactory(GAME_PORT);

		client_chat_info[0] = new ClientChatInfo;
		client_chat_info[0]->name = player_preferences->name;
		client_chat_info[0]->color = player_preferences->color;
		client_chat_info[0]->team = player_preferences->team;
	}
#endif
	
	return true;
}

bool NetConnectRemoteHub(const IPaddress& remote_hub_address)
{
	connection_to_server = std::make_unique<CommunicationsChannel>();

	connection_to_server->connect(remote_hub_address);
	if (!connection_to_server->isConnected()) return false;

	NetSetDefaultInflater(connection_to_server.get());

	connection_to_server->enqueueOutgoingMessage(RemoteHubHostConnectMessage(kNetworkSetupProtocolID));
	connection_to_server->enqueueOutgoingMessage(CapabilitiesMessage(my_capabilities));

	auto response_message = std::unique_ptr<RemoteHubHostResponseMessage>(connection_to_server->receiveSpecificMessage<RemoteHubHostResponseMessage>(3000u, 3000u));
	if (!response_message || !response_message->accepted())
	{
		connection_to_server->disconnect();
		return false;
	}

	connection_to_server->enqueueOutgoingMessage(TopologyMessage(topology));

	byte* wad = nullptr;
	int wad_length;

	if (resuming_saved_game)
	{
		assert(resumed_wad_data_for_remote_hub && resumed_wad_size_for_remote_hub);
		wad = resumed_wad_data_for_remote_hub;
		wad_length = resumed_wad_size_for_remote_hub;
	}
	else
	{
		entry_point entry = { topology->game_data.level_number };
		wad = (byte*)get_map_for_net_transfer(&entry);
		assert(wad);
		wad_length = get_net_map_data_length(wad);
	}

	NetDistributeGameDataToAllPlayers(wad, wad_length, !resuming_saved_game, connection_to_server.get());

	if (resuming_saved_game)
		NetSetResumedGameWadForRemoteHub(nullptr, 0);
	else
		free(wad);

	if (!std::unique_ptr<RemoteHubReadyMessage>(connection_to_server->receiveSpecificMessage<RemoteHubReadyMessage>()))
	{
		connection_to_server->disconnect();
		return false;
	}

	return true;
}

void NetSetResumedGameWadForRemoteHub(byte* resumed_wad_data, int length)
{
	resumed_wad_data_for_remote_hub = resumed_wad_data;
	resumed_wad_size_for_remote_hub = length;
}

void NetCancelGather(
	void)
{
	assert(netState==netGathering);

	NetDistributeTopology(tagCANCEL_GAME);
}

void NetSetCapabilities(const Capabilities* capabilities)
{
	my_capabilities = *capabilities;
}

bool NetStart(
	void)
{
	assert(netState==netGathering);

#ifdef A1_NETWORK_STANDALONE_HUB
		if (resuming_saved_game)
		{
			player_start_data theStarts[MAXIMUM_NUMBER_OF_PLAYERS];
			short theNumberOfStarts;
			construct_multiplayer_starts(theStarts, &theNumberOfStarts);
			match_starts_with_existing_players(theStarts, &theNumberOfStarts);
			NetSetupTopologyFromStarts(theStarts, theNumberOfStarts);
		}
#endif

	NetDistributeTopology(resuming_saved_game ? tagRESUME_GAME : tagSTART_GAME);

	return true;
}

bool NetGameJoin(
	void *player_data,
	short player_data_size,
	const char* host_addr_string
	)
{
  /* Attempt a connection to host */
	netState = netConnecting;

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
  else if (use_remote_hub)
  {
	  netState = netJoining;
	  handlerState = netAwaitingHello;
	  connection_to_server->setMessageHandler(joinDispatcher);
  }
      
    NetInitializeTopology((void *) NULL, 0, player_data, player_data_size);
    return true;
}

void NetRetargetJoinAttempts(const IPaddress* inAddress)
{
	host_address_specified = (inAddress != NULL);
	if(host_address_specified)
	{
		host_address = *inAddress;
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
	assert(netState!=netUninitialized /* &&netState!=netDown*/ && (netState!=netJoining || use_remote_hub));
	
	return topology->player_count;
}

void *NetGetPlayerData(
	short player_index)
{
	assert(netState!=netUninitialized/* && netState!=netDown */ && (netState!=netJoining || use_remote_hub));
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


/* ---------- private code */

void
NetDDPPacketHandler(UDPpacket& packet)
{
	sCurrentGameProtocol.PacketHandler(packet);
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
	
	assert(player_data_size>=0&&player_data_size<MAXIMUM_PLAYER_DATA_SIZE);
	assert(game_data_size>=0&&game_data_size<MAXIMUM_GAME_DATA_SIZE);


#ifdef A1_NETWORK_STANDALONE_HUB
	localPlayerIndex = localPlayerIdentifier = NONE;
	topology->player_count = 0;
	topology->nextIdentifier = 0;
	topology->server.dspAddress.set_port(GAME_PORT);
	topology->server.ddpAddress.set_port(GAME_PORT);
#else
	topology->player_count = 1;
	localPlayerIndex = localPlayerIdentifier = 0;

	NetPlayerPtr local_player;

	/* initialize the local player (assume we’re index zero, identifier zero) */
	local_player = topology->players + localPlayerIndex;
	local_player->identifier = localPlayerIdentifier;
	local_player->net_dead = false;
	local_player->stream_id = 0;

	local_player->dspAddress.set_port(GAME_PORT);
	local_player->ddpAddress.set_port(GAME_PORT);

	topology->server.dspAddress = local_player->dspAddress;
	topology->server.ddpAddress = local_player->ddpAddress;

	if (player_data_size > 0)
		memcpy(&local_player->player_data, player_data, player_data_size);

	/* initialize the network topology (assume we’re the only player) */
	topology->nextIdentifier = 1;
#endif
	
	if (game_data_size > 0)
		memcpy(&topology->game_data, game_data, game_data_size);
	gameSessionIdentifier.clear();
}

static void NetUpdateTopology(
	void)
{
#ifndef A1_NETWORK_STANDALONE_HUB
	/* recalculate localPlayerIndex */					
	for (localPlayerIndex=0;localPlayerIndex<topology->player_count;++localPlayerIndex)
	{
		if (topology->players[localPlayerIndex].identifier==localPlayerIdentifier) break;
	}
#ifdef DEBUG
	if (localPlayerIndex==topology->player_count) fdprintf("couldn't find my identifier: %p", (void*)topology);
#endif
#endif
}


void construct_multiplayer_starts(player_start_data* outStartArray, short* outStartCount)
{
	int number_of_players = NetGetNumberOfPlayers();

	if (outStartCount != NULL)
	{
		*outStartCount = number_of_players;
	}

	for (int player_index = 0; player_index < number_of_players; ++player_index)
	{
		player_info* player_information = (player_info*)NetGetPlayerData(player_index);
		outStartArray[player_index].team = player_information->team;
		outStartArray[player_index].color = player_information->color;
		outStartArray[player_index].identifier = NetGetPlayerIdentifier(player_index);
		strncpy(outStartArray[player_index].name, player_information->name, MAXIMUM_PLAYER_START_NAME_LENGTH + 1);
	}
}

// This should be safe to use whether starting or resuming and whether single-player or multiplayer.
void match_starts_with_existing_players(player_start_data* ioStartArray, short* ioStartCount)
{
	// This code could be smarter, but it doesn't run very often, doesn't get big data sets, etc.
	// so I'm not going to worry about it.

	bool startAssigned[MAXIMUM_NUMBER_OF_PLAYERS];
	int8 startAssignedToPlayer[MAXIMUM_NUMBER_OF_PLAYERS];
	for (int i = 0; i < MAXIMUM_NUMBER_OF_PLAYERS; i++)
	{
		startAssigned[i] = false;
		startAssignedToPlayer[i] = NONE;
	}

	// First, match starts to players by name.
	for (int s = 0; s < *ioStartCount; s++)
	{
		for (int p = 0; p < dynamic_world->player_count; p++)
		{
			if (startAssignedToPlayer[p] == NONE)
			{
				if (strcmp(ioStartArray[s].name, get_player_data(p)->name) == 0)
				{
					startAssignedToPlayer[p] = s;
					startAssigned[s] = true;
					break;
				}
			}
		}
	}

	// Match remaining starts to remaining players arbitrarily.
	for (int s = 0; s < *ioStartCount; s++)
	{
		if (!startAssigned[s])
		{
			for (int p = 0; p < dynamic_world->player_count; p++)
			{
				if (startAssignedToPlayer[p] == NONE)
				{
					startAssignedToPlayer[p] = s;
					startAssigned[s] = true;
					break;
				}
			}
		}
	}

	// Create new starts for any players not covered.
	int p = 0;
	while (*ioStartCount < dynamic_world->player_count)
	{
		if (startAssignedToPlayer[p] == NONE)
		{
			player_data* thePlayer = get_player_data(p);
			ioStartArray[*ioStartCount].team = thePlayer->team;
			ioStartArray[*ioStartCount].color = thePlayer->color;
			ioStartArray[*ioStartCount].identifier = NONE;
			strncpy(ioStartArray[*ioStartCount].name, thePlayer->name, MAXIMUM_PLAYER_START_NAME_LENGTH + 1);
			startAssignedToPlayer[p] = *ioStartCount;
			startAssigned[*ioStartCount] = true;
			(*ioStartCount)++;
		}

		p++;
	}

	// Assign remaining starts to players that don't exist yet
	p = dynamic_world->player_count;
	for (int s = 0; s < *ioStartCount; s++)
	{
		if (!startAssigned[s])
		{
			startAssignedToPlayer[p] = s;
			startAssigned[s] = true;
			p++;
		}
	}

	// Reorder starts to match players - this is particularly unclever
	player_start_data theOriginalStarts[MAXIMUM_NUMBER_OF_PLAYERS];
	memcpy(theOriginalStarts, ioStartArray, sizeof(theOriginalStarts));
	for (p = 0; p < *ioStartCount; p++)
	{
		ioStartArray[p] = theOriginalStarts[startAssignedToPlayer[p]];
	}
}


/* ------ this needs to let the gatherer keep going if there was an error.. */
/* ••• Marathon Specific Code ••• */
/* Returns error code.. */
// ZZZ annotation: this function doesn't seem to belong here - maybe more like interface.cpp?
bool NetChangeMap(
	struct entry_point *entry)
{
	byte *wad = NULL;
	int32 length;
	bool do_physics = true;

	/* If the guy that was the server died, and we are trying to change levels, we lose */
        // ZZZ: if we used the parent_wad_checksum stuff to locate the containing Map file,
        // this would be the case somewhat less frequently, probably...
	  // being the server, we must send out the map to everyone.	
	  if (local_is_server()) {

#ifdef A1_NETWORK_STANDALONE_HUB
		length = StandaloneHub::Instance()->GetMapData(&wad);
		byte* physics = nullptr;
		do_physics = StandaloneHub::Instance()->GetPhysicsData(&physics);
#else
		wad = (unsigned char*)get_map_for_net_transfer(entry);
		length = wad ? get_net_map_data_length(wad) : 0;
		do_physics = true;
#endif
		if (wad) NetDistributeGameDataToAllPlayers(wad, length, do_physics);

	  } else { // wait for de damn map.

		  if (use_remote_hub && get_game_state() == _change_level) //if the gatherer is using a remote hub, it has to send it to the hub first
		  {
			  wad = (unsigned char*)get_map_for_net_transfer(entry);
			  length = wad ? get_net_map_data_length(wad) : 0;

			  if (wad)
			  {
				  NetDistributeGameDataToAllPlayers(wad, length, true, connection_to_server.get());
				  free(wad);
			  }
		  }

	      wad = NetReceiveGameData(true);
	      if (!wad) alert_user(infoError, strNETWORK_ERRORS, netErrCouldntReceiveMap, 0);
	  }
	  
#ifndef A1_NETWORK_STANDALONE_HUB
	  sNetworkStats.clear(); //reset the pregame state

	  /* Now load the level.. */
	  if (wad) process_net_map_data(wad); //Note that this frees the wad as well!!
#endif
	
	return wad;
}

void DeferredScriptSend(const std::vector<byte>& script_data)
{
    deferred_script = script_data;
}

// ZZZ this "ought" to distribute to all players simultaneously (by interleaving send calls)
// in case the server bandwidth is much greater than the others' bandwidths.  But that would
// take a fair amount of reworking of the streaming system, which only groks talking with one
// machine at a time.
OSErr NetDistributeGameDataToAllPlayers(byte *wad_buffer, 
					int32 wad_length,
					bool do_physics,
					CommunicationsChannel* remote_hub)
{
	short playerIndex, message_id;
	OSErr error= noErr;
	int32 total_length;
	uint64_t initial_ticks= machine_tick_count();
	short physics_message_id;
	byte *physics_buffer = NULL;
	int32 physics_length;
	
	message_id = remote_hub ? _connecting_to_remote_hub : (topology->player_count==2) ? (_distribute_map_single) : (_distribute_map_multiple);
	physics_message_id = remote_hub ? _connecting_to_remote_hub : (topology->player_count==2) ? (_distribute_physics_single) : (_distribute_physics_multiple);

#ifndef A1_NETWORK_STANDALONE_HUB
	if (!remote_hub) open_progress_dialog(physics_message_id);
#endif
	
	/* For updating our progress bar.. */
	total_length= (topology->player_count-1)*wad_length;
	
	/* Get the physics */
	if (do_physics)
	{
#ifdef A1_NETWORK_STANDALONE_HUB
		physics_length = StandaloneHub::Instance()->GetPhysicsData(&physics_buffer);
#else
		physics_buffer = (unsigned char*)get_network_physics_buffer(&physics_length);
#endif 
	}
	
	// build a list of players to send to
	std::vector<CommunicationsChannel *> channels;

	// also a list of who and who can not take compressed data
	std::vector<CommunicationsChannel *> zipCapableChannels;
	std::vector<CommunicationsChannel *> zipIncapableChannels;

	if (remote_hub)
	{
		channels.push_back(remote_hub);
		zipCapableChannels.push_back(remote_hub);
	}
	else
	{
		for (playerIndex = 0; playerIndex < topology->player_count; playerIndex++)
		{
			NetPlayer player = topology->players[playerIndex];

			/* If the player is not net dead. */
			// ZZZ: and is not going to be a zombie and is not us

			if (!player.net_dead && player.identifier != NONE && playerIndex != localPlayerIndex)
			{
				Client* client = connections_to_clients[player.stream_id];
				channels.push_back(client->channel.get());
				if (client->capabilities[Capabilities::kZippedData] >= my_capabilities[Capabilities::kZippedData])
				{
					zipCapableChannels.push_back(client->channel.get());
				}
				else
				{
					zipIncapableChannels.push_back(client->channel.get());
				}

			}
		}
	}

#ifndef A1_NETWORK_STANDALONE_HUB
	if (!remote_hub)
	{
		set_progress_dialog_message(message_id);
		reset_progress_bar();
	}
#endif
	
	if (physics_buffer)
	{
		if (zipCapableChannels.size())
		{
			ZippedPhysicsMessage zippedPhysicsMessage(physics_buffer, physics_length);
			std::unique_ptr<UninflatedMessage> uninflatedMessage(zippedPhysicsMessage.deflate());
			std::for_each(zipCapableChannels.begin(), zipCapableChannels.end(), std::bind(&CommunicationsChannel::enqueueOutgoingMessage, std::placeholders::_1, *uninflatedMessage));
		}

		if (zipIncapableChannels.size())
		{
			PhysicsMessage physicsMessage(physics_buffer, physics_length);
			std::for_each(zipIncapableChannels.begin(), zipIncapableChannels.end(), std::bind(&CommunicationsChannel::enqueueOutgoingMessage, std::placeholders::_1, physicsMessage));
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
			std::for_each(zipCapableChannels.begin(), zipCapableChannels.end(), std::bind(&CommunicationsChannel::enqueueOutgoingMessage, std::placeholders::_1, *uninflatedMessage));
		}

		if (zipIncapableChannels.size())
		{
			MapMessage mapMessage(wad_buffer, wad_length);
			std::for_each(zipIncapableChannels.begin(), zipIncapableChannels.end(), std::bind(&CommunicationsChannel::enqueueOutgoingMessage, std::placeholders::_1, mapMessage));
		}
	}

	if (deferred_script.size())
	{
		if (zipCapableChannels.size())
		{
			ZippedLuaMessage zippedLuaMessage(deferred_script.data(), deferred_script.size());
			std::unique_ptr<UninflatedMessage> uninflatedMessage(zippedLuaMessage.deflate());
			std::for_each(zipCapableChannels.begin(), zipCapableChannels.end(), std::bind(&CommunicationsChannel::enqueueOutgoingMessage, std::placeholders::_1, *uninflatedMessage));
		}

		if (zipIncapableChannels.size())
		{
			LuaMessage luaMessage(deferred_script.data(), deferred_script.size());
			std::for_each(zipIncapableChannels.begin(), zipIncapableChannels.end(), std::bind(&CommunicationsChannel::enqueueOutgoingMessage, std::placeholders::_1, luaMessage));
		}
	}

	{
		EndGameDataMessage endGameDataMessage;
		std::for_each(channels.begin(), channels.end(), std::bind(&CommunicationsChannel::enqueueOutgoingMessage, std::placeholders::_1, endGameDataMessage));
	}

	CommunicationsChannel::multipleFlushOutgoingMessages(channels, false, 30000, 30000);

	if (!remote_hub)
	{
		for (playerIndex = 0; playerIndex < topology->player_count; playerIndex++) {

			NetPlayer player = topology->players[playerIndex];

			if (playerIndex != localPlayerIndex && !player.net_dead && player.identifier != NONE) {
				connections_to_clients[topology->players[playerIndex].stream_id]->state = Client::_ingame;
			}
		}
	}
    
	if (error) { // ghs: nothing above returns an error at the moment,
		// but I'll leave so you know what error could be displayed
		alert_user(infoError, strNETWORK_ERRORS, netErrCouldntDistribute, error);
	} else if  (machine_tick_count()-initial_ticks>static_cast<uint64_t>(topology->player_count*MAP_TRANSFER_TIME_OUT)) {
		alert_user(infoError, strNETWORK_ERRORS, netErrWaitedTooLongForMap, error);
		error= 1;
	}

#ifdef A1_NETWORK_STANDALONE_HUB
	return error;
#endif

	if (remote_hub || error)
	{
		free(physics_buffer);
		if (!remote_hub) close_progress_dialog();
		return error;
	}

	/* Process the physics file & frees it!.. */
	if (physics_buffer)
		process_network_physics_model(physics_buffer);

	draw_progress_bar(total_length, total_length);

	if (deferred_script.size()) {
		LoadLuaScript((char*)deferred_script.data(), deferred_script.size(), _lua_netscript);
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
	  auto physics_buffer = handlerPhysicsBuffer.size() > 0 ? std::malloc(handlerPhysicsBuffer.size()) : nullptr;
	  if (physics_buffer) std::memcpy(physics_buffer, handlerPhysicsBuffer.data(), handlerPhysicsBuffer.size());
      process_network_physics_model(physics_buffer); //will free the buffer, that's why we need to allocate for a buffer copy here
    }
    
    if (handlerMapLength > 0) {
      map_buffer = handlerMapBuffer;
      handlerMapBuffer = NULL;
      handlerMapLength = 0;
    }
    
    if (handlerLuaBuffer.size() > 0) {
      LoadLuaScript((char *)handlerLuaBuffer.data(), handlerLuaBuffer.size(), _lua_netscript);
    }
    
    draw_progress_bar(10, 10);
    close_progress_dialog();
  } else {
    draw_progress_bar(10, 10);
    close_progress_dialog();
    
    if (handlerMapLength > 0) {
      delete[] handlerMapBuffer;
      handlerMapBuffer = NULL;
      handlerMapLength = 0;
    }
    
    alert_user(infoError, strNETWORK_ERRORS, netErrMapDistribFailed, 1);
  }
  
  return map_buffer;
}

int32
NetGetNetTime(void)
{
        return sCurrentGameProtocol.GetNetTime();
}

bool
NetCheckWorldUpdate()
{
	return sCurrentGameProtocol.CheckWorldUpdate();
}

extern const NetworkStats& hub_stats(int player_index);

void NetProcessMessagesInGame() {
	if (connection_to_server) {
		connection_to_server->pump();
		connection_to_server->dispatchIncomingMessages();
	} else {
		// update stats
		if (last_network_stats_send + network_stats_send_period < machine_tick_count())
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

bool NetProcessNewJoiner(std::shared_ptr<CommunicationsChannel> new_joiner)
{
	if (new_joiner) {

		new_joiner->setMessageInflater(inflater);
		Client* client = new Client(new_joiner);
		connections_to_clients[next_stream_id] = client;
		next_stream_id++;

		HelloMessage helloMessage(kNetworkSetupProtocolID);
		new_joiner->enqueueOutgoingMessage(helloMessage);
		return true;
	}

	return false;
}

// If a potential joiner has connected to us, handle em
bool NetCheckForNewJoiner (prospective_joiner_info &info, CommunicationsChannelFactory* server_override, bool process_new_joiners)
{  
	auto actual_server = server_override ? server_override : server;
	CommunicationsChannel* new_joiner = process_new_joiners && actual_server ? actual_server->newIncomingConnection() : nullptr;
	
	NetProcessNewJoiner(std::shared_ptr<CommunicationsChannel>(new_joiner));
	
	if (actual_server)
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
				    connection_to_server = std::unique_ptr<CommunicationsChannel>(server_nbc->release());
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
				    if (nbc_is_resolving)
					    host_address = server_nbc->address();
				    nbc_is_resolving = false;
				    ConnectPool::instance()->abandon(server_nbc);
				    server_nbc = host_address_specified ? ConnectPool::instance()->connect(host_address) : nullptr;
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
  
  /* return netPlayerAdded to tell the caller to refresh his topology, but don’t change netState to that */
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
  assert(netState == netGathering || player.gathering);
  assert(topology->player_count < MAXIMUM_NUMBER_OF_NETWORK_PLAYERS);

  Client::check_player = check_player;

  JoinPlayerMessage joinPlayerMessage(topology->nextIdentifier++);
  connections_to_clients[player.stream_id]->channel->enqueueOutgoingMessage(joinPlayerMessage);
  connections_to_clients[player.stream_id]->state = Client::_awaiting_accept_join;

  // lie to the network code and say we gathered successfully
  return kGatherPlayerSuccessful;
}

void NetHandleUngatheredPlayer (prospective_joiner_info ungathered_player)
{
  // Drop connection of ungathered player
  delete connections_to_clients[ungathered_player.stream_id];
  connections_to_clients.erase(ungathered_player.stream_id);
}

void NetRemoteHubSendCommand(RemoteHubCommand command, int data)
{
	if (connection_to_server)
	{
		auto message = RemoteHubCommandMessage(command, data);
		connection_to_server->enqueueOutgoingMessage(message);
		connection_to_server->flushOutgoingMessages(false);
	}
}

/*************************************************************************************************
 *
 * Function: reassign_player_colors
 * Purpose:  This function used to reassign a player's color if it conflicted with another
 *           player's color. Now it reassigns everyone's colors. for the old function, see the
 *           obsoleted version (called check_player_info) at the end of this file.
 *           (Woody note: check_player_info can be found in network_dialogs_macintosh.cpp.)
 *
 *************************************************************************************************/
 /* Note that we now only force unique colors across teams. */

 // ZZZ: moved here (from network_dialogs_macintosh.cpp) so it can be shared with SDL version

void reassign_player_colors(
	short player_index,
	short num_players)
{
	short actual_colors[MAXIMUM_NUMBER_OF_PLAYERS];  // indexed by player
	bool colors_taken[NUMBER_OF_TEAM_COLORS];   // as opposed to desired team. indexed by team
	game_info* game;

	(void)(player_index);

	assert(num_players <= MAXIMUM_NUMBER_OF_PLAYERS);
	game = (game_info*)NetGetGameData();

	objlist_set(colors_taken, false, NUMBER_OF_TEAM_COLORS);
	objlist_set(actual_colors, NONE, MAXIMUM_NUMBER_OF_PLAYERS);

	if (game->game_options & _force_unique_teams)
	{
		short index;

		for (index = 0; index < num_players; ++index)
		{
			player_info* player = (player_info*)NetGetPlayerData(index);
			if (!colors_taken[player->desired_color])
			{
				player->color = player->desired_color;
				player->team = player->color;
				colors_taken[player->color] = true;
				actual_colors[index] = player->color;
			}
		}

		/* Now give them a random color.. */
		for (index = 0; index < num_players; index++)
		{
			player_info* player = (player_info*)NetGetPlayerData(index);
			if (actual_colors[index] == NONE) // This player needs a team
			{
				short remap_index;

				for (remap_index = 0; remap_index < num_players; remap_index++)
				{
					if (!colors_taken[remap_index])
					{
						player->color = remap_index;
						player->team = remap_index;
						colors_taken[remap_index] = true;
						break;
					}
				}
				assert(remap_index < num_players);
			}
		}
	}
	else {
		short index;
		short team_color;

		/* Allow teams.. */
		for (team_color = 0; team_color < NUMBER_OF_TEAM_COLORS; ++team_color)
		{
			// let's mark everybody down for the teams that they can get without conflicts.
			for (index = 0; index < num_players; index++)
			{
				player_info* player = (player_info*)NetGetPlayerData(index);
				if (player->team == team_color && !colors_taken[player->desired_color])
				{
					player->color = player->desired_color;
					colors_taken[player->color] = true;
					actual_colors[index] = player->color;
				}
			}

			// ok, everyone remaining gets a team that we pick for them.
			for (index = 0; index < num_players; index++)
			{
				player_info* player = (player_info*)NetGetPlayerData(index);
				if (player->team == team_color && actual_colors[index] == NONE) // This player needs a team
				{
					short j;

					for (j = 0; j < num_players; j++)
					{
						if (!colors_taken[j])
						{
							player->color = j;
							colors_taken[j] = true;
							break;
						}
					}
					assert(j < num_players);
				}
			}
		}
	}
}

/*
---------------------
NetDistributeTopology
---------------------

	<--- error

connect to everyone’s dspAddress and give them the latest copy of the network topology.  this
used to be NetStart() and it used to connect all upring and downring ADSP connections.
*/
static void NetDistributeTopology(
	short tag)
{
	short playerIndex;
	
	assert(netState==netGathering || use_remote_hub);
	
	topology->tag= tag;

	TopologyMessage topologyMessage(topology);
        
	for (playerIndex=0; playerIndex<topology->player_count; ++playerIndex)
	  {
	    // ZZZ: skip players with identifier NONE - they don't really exist... also skip ourselves.
	    if(topology->players[playerIndex].identifier != NONE && playerIndex != localPlayerIndex)
	      {
			auto& client = connections_to_clients[topology->players[playerIndex].stream_id];
			if (client) client->channel->enqueueOutgoingMessage(topologyMessage);
	      }
	  }
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
		local_is_server() || use_remote_hub ||
		!(dynamic_world->game_information.cheat_flags & _disable_saving_level));

}

bool NetAllowOverlayMap() {
	return (dynamic_world->player_count == 1 ||
			(dynamic_world->game_information.cheat_flags & _allow_overlay_map));
}


extern int32 spoke_latency();

int32 NetGetLatency()
{
	return local_is_server() ? NetworkStats::invalid : spoke_latency();
}

const NetworkStats& NetGetStats(int player_index)
{
	if (local_is_server())
	{
		return hub_stats(player_index);
	}

	return player_index < sNetworkStats.size() ? sNetworkStats[player_index] : sInvalidStats;
}

int32 NetGetUnconfirmedActionFlagsCount()
{
	return sCurrentGameProtocol.GetUnconfirmedActionFlagsCount();
}

uint32 NetGetUnconfirmedActionFlag(int32 offset)
{
	return sCurrentGameProtocol.PeekUnconfirmedActionFlag(offset);
}

void NetUpdateUnconfirmedActionFlags()
{
	return sCurrentGameProtocol.UpdateUnconfirmedActionFlags();
}

#endif // !defined(DISABLE_NETWORKING)

