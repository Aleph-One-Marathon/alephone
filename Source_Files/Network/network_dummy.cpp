/*
 *  network_dummy.cpp - Dummy network functions
 */

#include "cseries.h"
#include "map.h"
#include "network.h"
#include "network_games.h"


void NetExit(void)
{
}

bool NetSync(void)
{
	return true;
}

bool NetUnSync(void)
{
	return true;
}

short NetGetLocalPlayerIndex(void)
{
	return 0;
}

short NetGetPlayerIdentifier(short player_index)
{
	return 0;
}

short NetGetNumberOfPlayers(void)
{
	return 1;
}

void *NetGetPlayerData(short player_index)
{
	return NULL;
}

void *NetGetGameData(void)
{
	return NULL;
}

bool NetChangeMap(struct entry_point *entry)
{
	return false;
}

long NetGetNetTime(void)
{
	return 0;
}

void display_net_game_stats(void)
{
}
