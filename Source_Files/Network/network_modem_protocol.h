#ifndef __NETWORK_MODEM_PROTOCOL_H
#define __NETWORK_MODEM_PROTOCOL_H

/*
	network_modem_protocol.h

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

	Tuesday, October 17, 1995 5:47:20 PM- rdm created.

*/

bool ModemEnter(void);
void ModemExit(void);

bool ModemSync(void);
bool ModemUnsync(void);

short ModemAddDistributionFunction(NetDistributionProc proc, bool lossy);
void ModemDistributeInformation(short type, void *buffer, short buffer_size, 
	bool send_to_self);
void ModemRemoveDistributionFunction(short type);

bool ModemGather(void *game_data, short game_data_size, void *player_data, 
	short player_data_size);

bool ModemGatherPlayer(short player_index, CheckPlayerProcPtr check_player);
bool ModemGameJoin(unsigned char *player_name, unsigned char *player_type, void *player_data, 
	short player_data_size, short version_number);

void ModemCancelJoin(void);
void ModemCancelGather(void);

short ModemUpdateJoinState(void);

short ModemGetLocalPlayerIndex(void);
short ModemGetPlayerIdentifier(short player_index);
short ModemGetNumberOfPlayers(void);

void *ModemGetPlayerData(short player_index);
void *ModemGetGameData(void);

bool ModemNumberOfPlayerIsValid(void);

bool ModemStart(void);
bool ModemChangeMap(struct entry_point *entry);

long ModemGetNetTime(void);

#endif
