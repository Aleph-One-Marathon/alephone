#ifndef __NETWORK_H
#define __NETWORK_H

/*
NETWORK.H
Tuesday, June 21, 1994 3:26:46 PM
*/

// unfortunately, this requires map.h because it needs "struct entry_point"

#ifdef DEMO
#define MAXIMUM_NUMBER_OF_NETWORK_PLAYERS 2
#else
#define MAXIMUM_NUMBER_OF_NETWORK_PLAYERS 8
#endif

#define MAX_LEVEL_NAME_LENGTH 64
#define MAX_NET_DISTRIBUTION_BUFFER_SIZE 512

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
	
	// where the game takes place
	int16  level_number;
	char   level_name[MAX_LEVEL_NAME_LENGTH+1];
	
	// network parameters
	int16  initial_updates_per_packet;
	int16  initial_update_latency;
} game_info;

#define MAX_NET_PLAYER_NAME_LENGTH  32

typedef struct player_info
{
	unsigned char name[MAX_NET_PLAYER_NAME_LENGTH+1];
	int16 desired_color;
	int16 team;   // from player.h
	int16 color;
	byte long_serial_number[10];
} player_info;

/* ---------------- functions from network.c */
enum /* message types passed to the user’s names lookup update procedure */
{
	removeEntity,
	insertEntity
};

enum /* states */
{
	netUninitialized, /* NetEnter() has not been called */
	netGathering, /* looking for players */
	netJoining, /* waiting to be gathered */
	netWaiting, /* have been gathered, waiting for start message */
	netStartingUp, /* waiting for everyone to report (via NetSync) and begin queueing commands */
	netActive, /* in game */
	netComingDown, /* Coming down... */
	netDown, /* game over, waiting for new gather or join call */
	netCancelled, /* the game was just cancelled */
	netPlayerAdded, /* a new player was just added to the topology (will return to netWaiting) */
	netJoinErrorOccurred
};

/* -------- typedefs */
// player index is the index of the player that is sending the information
typedef void (*NetDistributionProc)(void *buffer, short buffer_size, short player_index);
typedef void (*CheckPlayerProcPtr)(short player_index, short num_players);

/* --------- prototypes/NETWORK.C */
bool NetEnter(void);
void NetExit(void);

bool NetGather(void *game_data, short game_data_size, void *player_data, 
	short player_data_size);
bool NetGatherPlayer(short player_index, CheckPlayerProcPtr check_player);

bool NetGameJoin(unsigned char *player_name, unsigned char *player_type, void *player_data, short player_data_size, short version_number);
short NetUpdateJoinState(void);
void NetCancelJoin(void);

short NetGetLocalPlayerIndex(void);
short NetGetPlayerIdentifier(short player_index);

bool NetNumberOfPlayerIsValid(void);
short NetGetNumberOfPlayers(void);

void *NetGetPlayerData(short player_index);
void *NetGetGameData(void);

void NetSetInitialParameters(short updates_per_packet, short update_latency);

bool NetSync(void);
bool NetUnSync(void);

bool NetStart(void);
void NetCancelGather(void);

long NetGetNetTime(void);

bool NetChangeMap(struct entry_point *entry);

void display_net_game_stats(void);

short NetAddDistributionFunction(NetDistributionProc proc, bool lossy);
void NetDistributeInformation(short type, void *buffer, short buffer_size, bool send_to_self);
void NetRemoveDistributionFunction(short type);

#endif
