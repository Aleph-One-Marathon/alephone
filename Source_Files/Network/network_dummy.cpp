/*

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

*/
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

int32 NetGetNetTime(void)
{
	return 0;
}

void display_net_game_stats(void)
{
}

bool network_gather(void)
{
	return false;
}

int network_join(void)
{
	return false;
}

void network_speaker_idle_proc(void)
{
}

void network_microphone_idle_proc(void)
{
}

bool current_game_has_balls(void)
{
	return false;
}

bool NetAllowBehindview(void)
{
	return false;
}

bool NetAllowCrosshair(void)
{
	return false;
}

bool NetAllowTunnelVision(void)
{
	return false;
}
