#ifndef __NETWORK_MODEM_PROTOCOL_H
#define __NETWORK_MODEM_PROTOCOL_H

/*

	network_modem_protocol.h
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
