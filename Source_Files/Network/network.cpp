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

#if !defined(DISABLE_NETWORKING)

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
#include "MessageDispatcher.h"
#include "MessageInflater.h"
#include "MessageHandler.h"
#include "progress.h"
#include "extensions.h"

#include <stdlib.h>
#include <string.h>

#include <map>
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

/* ---------- globals */

static short ddpSocket; /* our ddp socket number */

static short localPlayerIndex;
static short localPlayerIdentifier;
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
static CommunicationsChannel *connection_to_server = NULL;
static int next_stream_id = 1; // 0 is local player
static IPaddress host_address;
static bool host_address_specified = false;
static MessageInflater *inflater = NULL;
static MessageDispatcher *joinDispatcher = NULL;
static MessageDispatcher *gatherDispatcher = NULL;
static uint32 next_join_attempt;

static GatherCallbacks *gatherCallbacks = NULL;
static ChatCallbacks *chatCallbacks = NULL;

int InGameChatCallbacks::bufferPtr = 0;
char InGameChatCallbacks::buffer[bufferSize] = "";
int InGameChatCallbacks::displayBufferPtr = 0;
char InGameChatCallbacks::displayBuffer[bufferSize] = "";


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

CheckPlayerProcPtr Client::check_player;

Client::Client(CommunicationsChannel *inChannel) : mDispatcher(new MessageDispatcher()),
						   channel(inChannel)
{
  mJoinerInfoMessageHandler.reset(newMessageHandlerMethod(this, &Client::handleJoinerInfoMessage));
  mScriptMessageHandler.reset(newMessageHandlerMethod(this, &Client::handleScriptMessage));
  mAcceptJoinMessageHandler.reset(newMessageHandlerMethod(this, &Client::handleAcceptJoinMessage));
  mChatMessageHandler.reset(newMessageHandlerMethod(this, &Client::handleChatMessage));
  mUnexpectedMessageHandler.reset(newMessageHandlerMethod(this, &Client::unexpectedMessageHandler));
  mDispatcher->setDefaultHandler(mUnexpectedMessageHandler.get());
  mDispatcher->setHandlerForType(mJoinerInfoMessageHandler.get(), JoinerInfoMessage::kType);
  mDispatcher->setHandlerForType(mScriptMessageHandler.get(), ScriptMessage::kType);
  mDispatcher->setHandlerForType(mAcceptJoinMessageHandler.get(), AcceptJoinMessage::kType);
  mDispatcher->setHandlerForType(mChatMessageHandler.get(), NetworkChatMessage::kType);
  channel->setMessageHandler(mDispatcher.get());
}

void Client::handleJoinerInfoMessage(JoinerInfoMessage* joinerInfoMessage, CommunicationsChannel *) 
{
  if (netState == netGathering) {
    fprintf(stderr, "received joiner info message\n");
    pstrcpy(name, joinerInfoMessage->info()->name);
    network_version = joinerInfoMessage->info()->network_version;
    state = Client::_connected_but_not_yet_shown;
  } else {
    fprintf(stderr, "unexpected joiner info message received\n");
  }
}

void Client::handleScriptMessage(ScriptMessage* scriptMessage, CommunicationsChannel *) 
{
  if (state == _awaiting_script_message) {
    fprintf(stderr, "received script message\n");
    if (do_netscript &&
	scriptMessage->value() != _netscript_yes_script_message) {
      alert_user(infoError, strNETWORK_ERRORS, netErrUngatheredPlayerUnacceptable, 0);
      state = _ungatherable;
    } else {
      fprintf(stderr, "sending join player message\n");
      JoinPlayerMessage joinPlayerMessage(topology->nextIdentifier);
      channel->enqueueOutgoingMessage(joinPlayerMessage);
      state = _awaiting_accept_join;
    }
  } else {
    fprintf(stderr, "unexpected script message received!\n");
  }
}

void Client::handleAcceptJoinMessage(AcceptJoinMessage* acceptJoinMessage,
				     CommunicationsChannel *)
{
  if (state == _awaiting_accept_join) {
    fprintf(stderr, "received accept join message\n");
    if (acceptJoinMessage->accepted()) {
      fprintf(stderr, "join accepted\n");
      topology->nextIdentifier++;
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
      
      NetDistributeTopology(tagNEW_PLAYER);
      state = _awaiting_map;
	  if (gatherCallbacks) gatherCallbacks->JoinSucceeded(&player);
    } else {
      // joiner didn't accept!?
      alert_user(infoError, strNETWORK_ERRORS, netErrCantAddPlayer, 0);
      state = _ungatherable;
    }
  } else {
    fprintf(stderr, "unexpected accept join message received!\n");
  }
}

void Client::handleChatMessage(NetworkChatMessage* netChatMessage, 
			       CommunicationsChannel *)
{
  fprintf(stderr, "chat message received\n");
  // relay this to all clients
  if (state == _ingame) {
    assert(netState == netActive);
    NetworkChatMessage chatMessage(netChatMessage->chatText(), getStreamIdFromChannel(channel));
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
	  static unsigned char name[MAX_NET_PLAYER_NAME_LENGTH + 1];
	  pstrcpy(name, topology->players[playerIndex].player_data.name);
	  a1_p2cstr(name);
	  chatCallbacks->ReceivedMessageFromPlayer((char *)name, netChatMessage->chatText());
	  return;
	}
      }
    }
  } else {
    fprintf(stderr, "non in-game chat messages are not yet implemented");
  }
}
  

void Client::unexpectedMessageHandler(Message *, CommunicationsChannel *) {
  fprintf(stderr, "unexpected message received\n");
}
    


static short handlerState;

static void handleHelloMessage(HelloMessage* helloMessage, CommunicationsChannel*)
{
  if (netState == netJoining) {
    fprintf(stderr, "received hello message\n");
    // reply with my join info
    prospective_joiner_info my_info;
    
    pstrcpy(my_info.name, player_preferences->name);
    my_info.network_version = get_network_version();
    JoinerInfoMessage joinerInfoMessage(&my_info);
    fprintf(stderr, "sending joinerInfoMessage\n");
    connection_to_server->enqueueOutgoingMessage(joinerInfoMessage);
  } else {
    // log an error
    fprintf(stderr, "unexpected hello message received!\n");
  }
}

static void handleJoinPlayerMessage(JoinPlayerMessage* joinPlayerMessage, CommunicationsChannel*) {
  if (netState == netJoining) {
    fprintf(stderr, "received join player message\n");

    /* Note that we could set accepted to false if we wanted to for some */
    /*  reason- such as bad serial numbers.... */
    
    SetNetscriptStatus (false); // Unless told otherwise, we don't expect a netscript
    
    /* Note that we share the buffers.. */
    localPlayerIdentifier= joinPlayerMessage->value();
    topology->players[localPlayerIndex].identifier= localPlayerIdentifier;
    topology->players[localPlayerIndex].net_dead= false;
    
    /* Confirm. */
    fprintf(stderr, "sending an acceptJoinMessage\n");
    AcceptJoinMessage acceptJoinMessage(true, &topology->players[localPlayerIndex]);
    connection_to_server->enqueueOutgoingMessage(acceptJoinMessage);

    if (acceptJoinMessage.accepted()) {
      handlerState = netWaiting;
    } else {
      handlerState = netJoinErrorOccurred;
    }
  } else {
    fprintf(stderr, "unexpected join player message received!\n");
  }
}

static byte *handlerLuaBuffer = NULL;
static size_t handlerLuaLength = 0;

static void handleLuaMessage(LuaMessage *luaMessage, CommunicationsChannel *) {
  if (netState == netStartingUp) {
    fprintf(stderr, "received lua\n");
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
    fprintf(stderr, "unexpected lua message received!\n");
  }
}

static byte *handlerMapBuffer = NULL;
static size_t handlerMapLength = 0;

static void handleMapMessage(MapMessage *mapMessage, CommunicationsChannel *) {
  if (netState == netStartingUp) {
    fprintf(stderr, "received map\n");
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
    fprintf(stderr, "unexpected map message received!\n");
  }
}
    
static void handleNetworkChatMessage(NetworkChatMessage *chatMessage, CommunicationsChannel *) {
  fprintf(stderr, "chat message received\n");
  if (netState == netActive && chatCallbacks) {
    for (int playerIndex = 0; playerIndex < topology->player_count; playerIndex++) {
      if (topology->players[playerIndex].stream_id == chatMessage->senderID()) {
	static unsigned char name[MAX_NET_PLAYER_NAME_LENGTH + 1];
	pstrcpy(name, topology->players[playerIndex].player_data.name);
	a1_p2cstr(name);
	chatCallbacks->ReceivedMessageFromPlayer((char *)name, chatMessage->chatText());
	return;
      }
    }
    fprintf(stderr, "chat message from %i, player not found\n", chatMessage->senderID());
  } else {
    fprintf(stderr, "non in-game chat not yet implemented\n");
  }
}

static byte *handlerPhysicsBuffer = NULL;
static size_t handlerPhysicsLength = 0;

static void handlePhysicsMessage(PhysicsMessage *physicsMessage, CommunicationsChannel *) {
  if (netState == netStartingUp) {
    fprintf(stderr, "received physics\n");
    if (handlerPhysicsBuffer) {
      delete[] handlerPhysicsBuffer;
      handlerPhysicsBuffer = NULL;
    }
    handlerPhysicsLength = physicsMessage->length();
    if (handlerPhysicsLength > 0) {
      handlerPhysicsBuffer = new byte[handlerPhysicsLength];
      memcpy(handlerPhysicsBuffer, physicsMessage->buffer(), handlerPhysicsLength);
    }
  } else {
    fprintf(stderr, "unexpected physics message received! (state is %i)\n", netState);
  }
}


static void handleScriptMessage(ScriptMessage* scriptMessage, CommunicationsChannel*) {
  if (netState == netJoining) {
    fprintf(stderr, "received script message\n");
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
    fprintf(stderr, "sending script message\n");
    connection_to_server->enqueueOutgoingMessage(replyToScriptMessage);
  } else {
    fprintf(stderr, "unexpected script message received!\n");
  }
}

static void handleTopologyMessage(TopologyMessage* topologyMessage, CommunicationsChannel *) {
  if (netState == netWaiting) {
    fprintf(stderr, "received topology message\n");
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
	break;
      }
  } else {
    fprintf(stderr, "unexpected topology message received!\n");
  }
}

static void handleUnexpectedMessage(Message *inMessage, CommunicationsChannel *) {
  fprintf(stderr, "unexpected message ID %i received!\n", inMessage->type());
}
    
static TypedMessageHandlerFunction<HelloMessage> helloMessageHandler(&handleHelloMessage);
static TypedMessageHandlerFunction<JoinPlayerMessage> joinPlayerMessageHandler(&handleJoinPlayerMessage);
static TypedMessageHandlerFunction<LuaMessage> luaMessageHandler(&handleLuaMessage);
static TypedMessageHandlerFunction<MapMessage> mapMessageHandler(&handleMapMessage);
static TypedMessageHandlerFunction<NetworkChatMessage> networkChatMessageHandler(&handleNetworkChatMessage);
static TypedMessageHandlerFunction<PhysicsMessage> physicsMessageHandler(&handlePhysicsMessage);
static TypedMessageHandlerFunction<ScriptMessage> scriptMessageHandler(&handleScriptMessage);
static TypedMessageHandlerFunction<TopologyMessage> topologyMessageHandler(&handleTopologyMessage);
static TypedMessageHandlerFunction<Message> unexpectedMessageHandler(&handleUnexpectedMessage);

void NetSetGatherCallbacks(GatherCallbacks *gc) {
  gatherCallbacks = gc;
}

void NetSetChatCallbacks(ChatCallbacks *cc) {
  chatCallbacks = cc;
}

void ChatCallbacks::SendChatMessage(const char *message)
{
  if (netState == netActive) {
    if (connection_to_server) {
      NetworkChatMessage chatMessage(message, 0); // gatherer will replace
                                                  // with my ID
      connection_to_server->enqueueOutgoingMessage(chatMessage);
    } else { 
      NetworkChatMessage chatMessage(message, 0);
      client_map_t::iterator it;
      for (it = connections_to_clients.begin(); it != connections_to_clients.end(); it++) {
	if (it->second->state == Client::_ingame) {
	  it->second->channel->enqueueOutgoingMessage(chatMessage);
	}
      }
      if (chatCallbacks) {
	for (int playerIndex = 0; playerIndex < topology->player_count; playerIndex++) {
	  if (playerIndex == localPlayerIndex) {
	    static unsigned char name[MAX_NET_PLAYER_NAME_LENGTH + 1];
	    pstrcpy(name, topology->players[playerIndex].player_data.name);
	    a1_p2cstr(name);
	    chatCallbacks->ReceivedMessageFromPlayer((char *)name, message);
	  }
	}
      }
    }
  } else {
    fprintf(stderr, "non in-game chat message are not yet implemented\n");
  }
}

InGameChatCallbacks *InGameChatCallbacks::m_instance = NULL;

InGameChatCallbacks *InGameChatCallbacks::instance() {
  if (!m_instance) {
    m_instance = new InGameChatCallbacks();
  }
  return m_instance;
};

void InGameChatCallbacks::ReceivedMessageFromPlayer(const char *player_name, const char *message) {
  screen_printf("%s: %s", player_name, message);
}

void InGameChatCallbacks::add(const char c)
{
  buffer[bufferPtr++] = c;
  buffer[bufferPtr] = '\0';
  displayBuffer[displayBufferPtr++] = c;
  displayBuffer[displayBufferPtr] = '_';
  displayBuffer[displayBufferPtr + 1] = '\0';
}

void InGameChatCallbacks::remove()
{
  if (bufferPtr > 0) {
    buffer[bufferPtr--] = '\0';
    displayBuffer[--displayBufferPtr] = '_';
    displayBuffer[displayBufferPtr + 1] = '\0';
  }
}

void InGameChatCallbacks::send()
{
  if (bufferPtr > 0) {
    ChatCallbacks::SendChatMessage(buffer);
  }
  displayBuffer[0] = '\0';
  displayBufferPtr = 0;
}

void InGameChatCallbacks::abort()
{
  displayBuffer[0] = '\0';
  displayBufferPtr = 0;
}

void InGameChatCallbacks::clear()
{
  bufferPtr = 0;
  buffer[0] = '\0';
  pstrcpy((unsigned char *) displayBuffer, player_preferences->name);
  displayBufferPtr = (int) displayBuffer[0];
  a1_p2cstr((unsigned char *) displayBuffer);
  displayBuffer[displayBufferPtr++] = ':';
  displayBuffer[displayBufferPtr++] = ' ';
  displayBuffer[displayBufferPtr] = '_';
  displayBuffer[displayBufferPtr + 1] = '\0';
}


bool NetEnter(void)
{
	OSErr error;
	bool success= true; /* optimism */

	assert(netState==netUninitialized);

	/* if this is the first time weÕve been called, add NetExit to the list of cleanup procedures */
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
			if (topology)
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

		inflater->learnPrototype(AcceptJoinMessage());
		inflater->learnPrototype(EndGameDataMessage());
		inflater->learnPrototype(HelloMessage());
		inflater->learnPrototype(JoinerInfoMessage());
		inflater->learnPrototype(JoinPlayerMessage());
		inflater->learnPrototype(LuaMessage());
		inflater->learnPrototype(MapMessage());
		inflater->learnPrototype(NetworkChatMessage());
		inflater->learnPrototype(PhysicsMessage());
		inflater->learnPrototype(ScriptMessage());
		inflater->learnPrototype(TopologyMessage());
	}
	
	if (!joinDispatcher) {
	  joinDispatcher = new MessageDispatcher();

	  joinDispatcher->setDefaultHandler(&unexpectedMessageHandler);
	  joinDispatcher->setHandlerForType(&helloMessageHandler, HelloMessage::kType);
	  joinDispatcher->setHandlerForType(&joinPlayerMessageHandler, JoinPlayerMessage::kType);
	  joinDispatcher->setHandlerForType(&luaMessageHandler, LuaMessage::kType);
	  joinDispatcher->setHandlerForType(&mapMessageHandler, MapMessage::kType);
	  joinDispatcher->setHandlerForType(&networkChatMessageHandler, NetworkChatMessage::kType);
	  joinDispatcher->setHandlerForType(&physicsMessageHandler, PhysicsMessage::kType);
	  joinDispatcher->setHandlerForType(&scriptMessageHandler, ScriptMessage::kType);
	  joinDispatcher->setHandlerForType(&topologyMessageHandler, TopologyMessage::kType);
	}

	next_join_attempt = machine_tick_count();

	/* Handle our own errors.. */
	if(error)
	{
		alert_user(infoError, strNETWORK_ERRORS, netErrCantContinue, error);
		NetExit();
		success= false;
	}
	
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
	
	client_map_t::iterator it;
	for (it = connections_to_clients.begin(); it != connections_to_clients.end(); it++)
		delete(it->second);
	connections_to_clients.clear();
	
	if (server) {
		delete server;
		server = NULL;
	}
	
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
	
	// Start listening for joiners
	server = new CommunicationsChannelFactory(GAME_PORT);
	
	netState= netGathering;
	
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
	OSErr error;
	bool success;

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

	return success;
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
	OSErr error = noErr;
	bool success= false;
	
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
		connection_to_server->setMessageHandler(joinDispatcher);
		
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
	assert(netState==netConnecting||netState==netJoining||netState==netWaiting||netState==netCancelled||netState==netJoinErrorOccurred);
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
		case netJoining:
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
	assert(netState!=netUninitialized&&netState!=netDown&&netState!=netJoining);
	
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
	memcpy(&local_player->player_data, player_data, player_data_size);
	
	/* initialize the network topology (assume weÕre the only player) */
	topology->player_count= 1;
	topology->nextIdentifier= 1;
	memcpy(&topology->game_data, game_data, game_data_size);
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
	if (localPlayerIndex==topology->player_count) fdprintf("couldnÕt find my identifier: %p", topology);
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
	long   length;
	OSErr  error= noErr;
	bool success= true;


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
  for (playerIndex= 0; !error && playerIndex<topology->player_count; playerIndex++) {
    
    NetPlayer player = topology->players[playerIndex];
    
    /* If the player is not net dead. */ 
    // ZZZ: and is not going to be a zombie and is not us
    if(!player.net_dead
       && player.identifier != NONE
       && playerIndex != localPlayerIndex) {

      CommunicationsChannel *channel = 
	connections_to_clients[player.stream_id]->channel;
    
      
      set_progress_dialog_message(physics_message_id);
      if(do_physics) {
	fprintf(stderr, "Transfering physics (%i bytes)\n", physics_length);
	PhysicsMessage physicsMessage(physics_buffer, physics_length);
	channel->enqueueOutgoingMessage(physicsMessage);
      }
      
      set_progress_dialog_message(message_id);
      reset_progress_bar();
      {
	fprintf(stderr, "Transfering map (%i bytes)\n", wad_length);
	MapMessage mapMessage(wad_buffer, wad_length);
	channel->enqueueOutgoingMessage(mapMessage);
      }
      
      if (do_netscript) {
	fprintf(stderr, "Transfering lua\n");
	LuaMessage luaMessage(deferred_script_data, deferred_script_length);
	channel->enqueueOutgoingMessage(luaMessage);
      } 

      EndGameDataMessage endGameDataMessage;
      channel->enqueueOutgoingMessage(endGameDataMessage);
    }
  }

  for (playerIndex = 0; playerIndex < topology->player_count; playerIndex++) {
    if (playerIndex != localPlayerIndex) {
      CommunicationsChannel *channel = connections_to_clients[topology->players[playerIndex].stream_id]->channel;
      
      channel->flushOutgoingMessages(false, 30000, 30000);
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
  
  open_progress_dialog(_awaiting_map);
  
  // handlers will take care of all messages, and when they're done
  // the server will send us this:
  auto_ptr<EndGameDataMessage> endGameDataMessage(connection_to_server->receiveSpecificMessage<EndGameDataMessage>((Uint32) 60000, (Uint32) 30000));
  if (endGameDataMessage.get()) {
    // game data was received OK
    if (do_physics && handlerPhysicsLength > 0) {
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
      LoadLuaScript((char *) handlerLuaBuffer, handlerLuaLength);
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
    
    bool distributionFailed = false;
    if (handlerPhysicsLength > 0) {
      delete[] handlerPhysicsBuffer;
      handlerPhysicsBuffer = NULL;
      handlerPhysicsLength = 0;
      distributionFailed = true;
    }
    if (handlerMapLength > 0) {
      delete[] handlerMapBuffer;
      handlerMapBuffer = NULL;
      handlerMapLength = 0;
      distributionFailed = true;
    }
    if (handlerLuaLength > 0) {
      delete[] handlerLuaBuffer;
      handlerLuaBuffer = NULL;
      handlerLuaLength = 0;
      distributionFailed = true;
    }
    
    if (distributionFailed) {
      alert_user(infoError, strNETWORK_ERRORS, netErrMapDistribFailed, 1);
    } else {
      alert_user(infoError, strNETWORK_ERRORS, netErrWaitedTooLongForMap, 1);
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
	// Ring is 12; star is 13.
	return (network_preferences->game_protocol == _network_game_protocol_ring) ? _ip_ring_network_version : _ip_star_network_version;
}

void NetProcessMessagesInGame() {
  if (connection_to_server) {
    connection_to_server->pump();
    connection_to_server->dispatchIncomingMessages();
  } else {
    client_map_t::iterator it;
    for (it = connections_to_clients.begin(); it != connections_to_clients.end(); it++) {
      it->second->channel->pump();
      it->second->channel->dispatchIncomingMessages();
    }
  }
}

// If a potential joiner has connected to us, handle em
bool NetCheckForNewJoiner (prospective_joiner_info &info)
{
  OSErr error = noErr;
  short packet_type;
  
  CommunicationsChannel *new_joiner = server->newIncomingConnection();
  
  if (new_joiner) {
    
    new_joiner->setMessageInflater(inflater);
    Client *client = new Client(new_joiner);
    connections_to_clients[next_stream_id] = client;
    next_stream_id++;
    
    HelloMessage helloMessage;
    fprintf(stderr, "sending hello message\n");
    new_joiner->enqueueOutgoingMessage(helloMessage);
  }

  {
    client_map_t::iterator it;
    for (it = connections_to_clients.begin(); it != connections_to_clients.end(); it++) {
      it->second->channel->pump();
      it->second->channel->dispatchIncomingMessages();

    }
  }

  // now check to see if any one has actually connected
  client_map_t::iterator it;
  for (it = connections_to_clients.begin(); it != connections_to_clients.end(); it++) {
    if (it->second->state == Client::_connected_but_not_yet_shown) {
      info.network_version = it->second->network_version;
      info.stream_id = it->first;
      pstrcpy(info.name, it->second->name);
      it->second->state = Client::_connected;
      info.gathering = false;
      return true;
    }
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
      } else {
	handlerState = netJoining;
	connection_to_server->pump();
	connection_to_server->dispatchIncomingMessages();
	if (handlerState == netWaiting) {	  
		newState= netWaiting;
	} else if (handlerState == netJoinErrorOccurred) {
	  error = 1;
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
      } else {
	handlerState = netWaiting;
	connection_to_server->pump();
	connection_to_server->dispatchOneIncomingMessage();
	if (handlerState != netWaiting) {
	  newState = handlerState;
	}
      }
      //	alert_user(infoError, strNETWORK_ERRORS, netErrJoinFailed, error);
      break;
      // netWaiting
      
    default:
      newState= NONE;
      break;
    }
  
  /* return netPlayerAdded to tell the caller to refresh his topology, but donÕt change netState to that */
  // ZZZ: similar behavior for netChatMessageReceived and netStartingResumeGame
  if (newState!=netPlayerAdded && newState != netChatMessageReceived && newState != netStartingResumeGame && newState != NONE)
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

  // reject a player if he can't handle our script demands
  ScriptMessage scriptMessage(_netscript_query_message);
  connections_to_clients[player.stream_id]->channel->enqueueOutgoingMessage(scriptMessage);
  connections_to_clients[player.stream_id]->state = Client::_awaiting_script_message;

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
static OSErr NetDistributeTopology(
	short tag)
{
	OSErr error = 0; //JTP: initialize to no error
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
	
	return error; // must be noErr (not like we check return value anyway :))
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

#endif // !defined(DISABLE_NETWORKING)

