#ifndef __NETWORK_H
#define __NETWORK_H

/*
NETWORK.H

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

Tuesday, June 21, 1994 3:26:46 PM

 May 24, 2003 (Woody Zenfell):
	compile-time constant MARATHON_NETWORK_VERSION replaced with runtime get_network_version()
*/

#include        "cseries.h"
#include	"cstypes.h"

// This file should be used only for stuff that folks outside the network subsystem care about
// (i.e. it's the interface to the subsystem)

// I'm tempted to slice the routines the network dialogs deal with away from those that the
// rest of the game deals with, but will leave that for another day.

// unfortunately, this requires map.h because it needs "struct entry_point"

#ifdef DEMO
#define MAXIMUM_NUMBER_OF_NETWORK_PLAYERS 2
#else
#define MAXIMUM_NUMBER_OF_NETWORK_PLAYERS 8
#endif

#define MAX_LEVEL_NAME_LENGTH 64

#define DEFAULT_GAME_PORT 4226

// change this if you make a major change to the way the setup messages work
#define kNetworkSetupProtocolID "Aleph One WonderNAT V1"

// ZZZ: there probably should be a published max size somewhere, but this isn't used anywhere; better
// not to pretend it's real.
//#define MAX_NET_DISTRIBUTION_BUFFER_SIZE 512

enum // base network speeds
{
	_appletalk_remote, // ARA
	_localtalk,
	_tokentalk,
	_ethernet,
#ifdef USE_MODEM
	_modem,
#endif
	NUMBER_OF_NETWORK_TYPES
};

typedef struct game_info
{
	uint16 initial_random_seed;
	int16  net_game_type;
	int32  time_limit;
	int16  kill_limit;
	int16  game_options;
	int16  difficulty_level;
	bool   server_is_playing; // if false, then observing
	bool   allow_mic;

        int16 cheat_flags;
	
	// where the game takes place
	int16  level_number;
	char   level_name[MAX_LEVEL_NAME_LENGTH+1];
	uint32 parent_checksum;
	
	// network parameters
	int16  initial_updates_per_packet;
	int16  initial_update_latency;
} game_info;

#define MAX_NET_PLAYER_NAME_LENGTH  32
#define LONG_SERIAL_NUMBER_LENGTH 10

typedef struct player_info
{
	char name[MAX_NET_PLAYER_NAME_LENGTH+1];
	int16 desired_color;
	int16 team;   // from player.h
	int16 color;
	byte long_serial_number[LONG_SERIAL_NUMBER_LENGTH];
} player_info;


struct prospective_joiner_info {
	uint16 stream_id;
	char name[MAX_NET_PLAYER_NAME_LENGTH];
	int16 color;
	int16 team;
	bool gathering;
};



/* ---------------- functions from network.c */
enum /* message types passed to the user’s names lookup update procedure */
{
	removeEntity,
	insertEntity
};

class GatherCallbacks
{
 public:
  virtual ~GatherCallbacks() { };
  
  virtual void JoinSucceeded(const prospective_joiner_info *player) = 0;
  virtual void JoiningPlayerDropped(const prospective_joiner_info *player) = 0;
  virtual void JoinedPlayerDropped(const prospective_joiner_info *player) = 0;
  virtual void JoinedPlayerChanged(const prospective_joiner_info *player) { };
};

class ChatCallbacks
{
 public:
  virtual ~ChatCallbacks() { };
  static void SendChatMessage(const std::string& message);
  virtual void ReceivedMessageFromPlayer(const char *player_name,
					 const char *message) = 0;
};

class InGameChatCallbacks : public ChatCallbacks
{
 public:
  ~InGameChatCallbacks() { }
  static InGameChatCallbacks *instance();
  void ReceivedMessageFromPlayer(const char *player_name, const char *message);

  static std::string prompt();

 private:
  InGameChatCallbacks() { }
};



// ZZZ note: netPlayerAdded, netChatMessageReceived, and netStartingResumeGame are 'pseudo-states';
// they are returned from NetUpdateJoinState() but will never be assigned to the actual "NetState()".
// ghs: netAwaitingHello isn't ever returned or assigned to netState
enum /* states */
{
	netUninitialized, /* NetEnter() has not been called */
	netGathering, /* looking for players */
	netConnecting, /* trying to establish connection to gatherer */
	netJoining, /* waiting to be gathered */
	netWaiting, /* have been gathered, waiting for start message */
	netStartingUp, /* waiting for everyone to report (via NetSync) and begin queueing commands */
	netActive, /* in game */
	netComingDown, /* Coming down... */
	netDown, /* game over, waiting for new gather or join call */
	netCancelled, /* the game was just cancelled */
	netPlayerAdded, /* a new player was just added to the topology (will return to netWaiting) */
	netPlayerDropped, /* like netPlayerAdded */
	netPlayerChanged, /* like netPlayerAdded */
	netJoinErrorOccurred,
        netChatMessageReceived, // ZZZ addition
        netStartingResumeGame, // ZZZ addition: like netStartingUp, but starting a resume-game instead of a new game
	netAwaitingHello // only used for handler state
};

/* -------- typedefs */
// player index is the index of the player that is sending the information
typedef void (*NetDistributionProc)(void *buffer, short buffer_size, short player_index);
typedef void (*CheckPlayerProcPtr)(short player_index, short num_players);

/* --------- prototypes/NETWORK.C */
void NetSetGatherCallbacks(GatherCallbacks *gc);
void NetSetChatCallbacks(ChatCallbacks *cc);
bool NetEnter();
void NetDoneGathering (void);
void NetExit(void);

bool NetGather(void *game_data, short game_data_size, void *player_data, 
	short player_data_size, bool resuming_game);

struct SSLP_ServiceInstance;

enum { // NetGatherPlayer results
        kGatherPlayerFailed, // generic
        kGatherPlayerSuccessful, // generic
        kGatheredUnacceptablePlayer // we had already committed to gathering this jimmy,
        // but we can't start a game with him - upper-level code needs to make sure gathering is cancelled.
};

int NetGatherPlayer(
// ZZZ: in my formulation, player info is all passed along in one structure from the dialog here.
const prospective_joiner_info &player,
CheckPlayerProcPtr check_player);

void NetHandleUngatheredPlayer(prospective_joiner_info ungathered_player);

// jkvw: replaced SSLP hinting address with host address
bool NetGameJoin(void *player_data, short player_data_size, const char* host_address_string);

bool NetCheckForNewJoiner (prospective_joiner_info &info);
short NetUpdateJoinState(void);
void NetCancelJoin(void);

// ask to change color and team; it's up to the gatherer to update the topo
// usually he'll change your team, if the color's free you'll get that too
void NetChangeColors(int16 color, int16 team);

// ghs: these are obsolete, I'll get rid of them when I'm sure I won't want
//      to refer back to them

// ZZZ addition - pre-game/(eventually) postgame chat
// Returns true if there was a pending message.
// Returns pointer to chat text.
// Returns pointer to sending player's data (does not copy player data).
// Data returned in pointers is only good until the next call to NetUpdateJoinState or NetCheckForIncomingMessages.
bool NetGetMostRecentChatMessage(player_info** outSendingPlayerData, char** outMessage);

// Gatherer should use this to send out his messages or to broadcast a message received from a joiner
OSErr NetDistributeChatMessage(short sender_identifier, const char* message);

void NetProcessMessagesInGame();

short NetGetLocalPlayerIndex(void);
short NetGetPlayerIdentifier(short player_index);

bool NetNumberOfPlayerIsValid(void);
short NetGetNumberOfPlayers(void);

void *NetGetPlayerData(short player_index);
void *NetGetGameData(void);

struct player_start_data;
// Gatherer may call this once after all players are gathered but before NetStart()
void NetSetupTopologyFromStarts(const player_start_data* inStartArray, short inStartCount);

void NetSetInitialParameters(short updates_per_packet, short update_latency);

bool NetSync(void);
bool NetUnSync(void);

bool NetStart(void);
void NetCancelGather(void);

int32 NetGetNetTime(void);

bool NetChangeMap(struct entry_point *entry);
OSErr NetDistributeGameDataToAllPlayers(byte* wad_buffer, int32 wad_length, bool do_physics);
byte* NetReceiveGameData(bool do_physics);

void DeferredScriptSend (byte* data, size_t length);
void SetNetscriptStatus (bool status);

void display_net_game_stats(void);

// ZZZ change: caller specifies int16 ID for distribution type.  Unknown types (when received) are
// passed along but ignored.  Uses an STL 'map' so ID's need not be consecutive or in any particular
// sub-range.
void NetAddDistributionFunction(int16 type, NetDistributionProc proc, bool lossy);
void NetDistributeInformation(short type, void *buffer, short buffer_size, bool send_to_self, bool send_only_to_team = false);
void NetRemoveDistributionFunction(short type);

// disable "cheats"
bool NetAllowCrosshair();
bool NetAllowOverlayMap();
bool NetAllowTunnelVision();
bool NetAllowBehindview();
bool NetAllowCarnageMessages();
bool NetAllowSavingLevel();

// spoke stuf

int32 NetGetUnconfirmedActionFlagsCount(); // how many flags can we use for prediction?
uint32 NetGetUnconfirmedActionFlag(int32 offset); // offset < GetUnconfirmedActionFlagsCount
void NetUpdateUnconfirmedActionFlags();

struct NetworkStats
{
	enum {
		invalid = -1,
		disconnected = -2,
	};

	int16 latency;
	int16 jitter;
	uint16 errors;
};

// returns latency in ms, or kNetLatencyInvalid or kNetLatencyDisconnected
int32 NetGetLatency();

const NetworkStats& NetGetStats(int player_index);
bool NetCheckWorldUpdate();

#endif
