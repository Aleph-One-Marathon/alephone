/*
NETWORK_GLUE.CPP

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

*/

// this file is an interface between the legacy network game setup interface 
// code and the new network layer
// hopefully once the interface is updated this can go away

#include "CNetwork.h"

Network gNetwork;

bool NetEnter(void)
{
  gNetwork.Enter();
}

bool NetExit(void)
{
  gNetwork.Exit();
}
bool NetSync(void)
{
  gNetwork.Sync();
}

bool NetUnSync(void)
{
  gNetwork.UnSync();
}

void NetAddDistributionFunction(int16 inDataTypeID, NetDistributionProc inProc,
				bool inLossy)
{
  gNetwork.AddDistributionFunction(inDataTypeID, inProc, inLossy);
}

void NetRemoveDistributionFunction(int16 inDataTypeID)
{
  gNetwork.RemoveDistributionFunction(inDataTypeID);
}

const NetDistributionInfo* NetGetDistributionInfoForTYpe(int16 inType)
{
  return gNetwork.GetDistributionInfoForType(inType);
}

void NetDistributeInformation(short type,
			      void *buffer,
			      short buffer_size,
			      bool send_to_self)
{
  gNetwork.DistributeInformation(type, buffer, buffer_size, send_to_self);
}

bool NetGather(void *game_data, short game_data_size,
	       void *player_data, short player_data_size,
	       bool resuming_game)
{
  
  game_info gameInfo;
  player_info playerInfo;
  memcpy(&gameInfo, game_data, game_data_size);
  memcpy(&playerInfo, player_data, player_data_size);

  gNetwork.BeginGathering(gameInfo, playerInfo, resuming_game);
}

void NetCancelGather(void)
{
  gNetwork.CancelGathering();
}

bool NetStart(void)
{
  gNetwork.StartGame();
}

bool NetGameJoin(unsigned char *player_name,
		 unsigned char *player_type,
		 void *player_data,
		 short player_data_size,
		 short version_number,
		 const char * hint_addr_string)
{

  player_info playerInfo;
  memcpy(&playerInfo, player_data, player_data_size);
  int retCode;
  while (retCode = gNetwork.Join(player_name,
				 player_type,
				 playerInfo,
				 hint_addr_string,
				 15367)) {
    // if retcode is couldn't join display an error and return -1
  }
  return true;
}

void NetCancelJoin(void)
{
  gNetwork.Cancel();
}

void NetSetServerIdentifier(short identifier) {
  gNetwork.ServerIdentifier(identifier);
}

short NetGetLocalPlayerIndex(void) {
  return gNetwork.LocalPlayerIndex();
}

bool NetNumberOfPlayerValid(void) {
  return gNetwork.NumberOfPLayerIsValid();
}

void *NetGetPlayerData(short player_index) {
  return (void *) gNetwork.PlayerInfo(player_index);
}

void *NetGetGameData(void) {
  return (void *) gNetwork.GameInfo();
}

void NetSetupTopologyFromStarts(const player_start_data* inStartArray,
				short inStartCount) {
  gNetwork.SetupTopologyFromStarts(inStartArray, inStartCount);
}

bool NetEntityNotInGame(NetEntityName *entity, NetAddrBlock *address) {
  return true;
}

