/*

NETWORK_MESSAGES.CPP

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
#include "AStream.h"
#include "network_messages.h"

void NumberAndStringMessage::reallyDeflateTo(AOStream& outputStream) const {
  outputStream << mNumber;
  outputStream.write(mString, strlen(mString));
}

bool NumberAndStringMessage::reallyInflateFrom(AIStream& inputStream) {
  inputStream >> mNumber;
  // woody came up with this. how clever
  Uint32 stringLength = inputStream.maxg() - inputStream.tellg();
  if (stringLength > sizeof(mString) - 1) {
    stringLength = sizeof(mString) - 1;
  }
  inputStream.read(mString, stringLength);
  mString[stringLength] = '\0';
  return true;
}

void serialize_player_info(AOStream &outputStream,
			   const player_info *playerInfo) {
  outputStream.write(playerInfo->name, sizeof(playerInfo->name));
  outputStream << playerInfo->desired_color;
  outputStream << playerInfo->team;
  outputStream << playerInfo->color;
  outputStream.write(playerInfo->long_serial_number,
		     sizeof(playerInfo->long_serial_number));
}

void unserialize_player_info(AIStream &inputStream,
			     player_info *playerInfo) {
  inputStream.read(playerInfo->name, sizeof(playerInfo->name));
  inputStream >> playerInfo->desired_color;
  inputStream >> playerInfo->team;
  inputStream >> playerInfo->color;
  inputStream.read(playerInfo->long_serial_number,
		   sizeof(playerInfo->long_serial_number));
}

void PlayerInfoMessage::reallyDeflateTo(AOStream& outputStream) const {
  serialize_player_info(outputStream, &mPlayerInfo);		
}

bool PlayerInfoMessage::reallyInflateFrom(AIStream& inputStream) {
  unserialize_player_info(inputStream, &mPlayerInfo);
  return true;
}

void serialize_game_info(AOStream &outputStream, const game_info *gameInfo) {
  outputStream << gameInfo->initial_random_seed;
  outputStream << gameInfo->net_game_type;
  outputStream << gameInfo->time_limit;
  outputStream << gameInfo->kill_limit;
  outputStream << gameInfo->game_options;
  outputStream << gameInfo->difficulty_level;
  outputStream << (gameInfo->server_is_playing ? 1 : 0);
  outputStream << (gameInfo->allow_mic ? 1 : 0);
  outputStream << gameInfo->level_number;
  outputStream.write(gameInfo->level_name, sizeof(gameInfo->level_name));
  outputStream << gameInfo->initial_updates_per_packet;
  outputStream << gameInfo->initial_update_latency;
}

void unserialize_game_info(AIStream &inputStream, game_info *gameInfo) {
  inputStream >> gameInfo->initial_random_seed;
  inputStream >> gameInfo->net_game_type;
  inputStream >> gameInfo->time_limit;
  inputStream >> gameInfo->kill_limit;
  inputStream >> gameInfo->game_options;
  inputStream >> gameInfo->difficulty_level;
  int server_is_playing;
  inputStream >> server_is_playing;
  gameInfo->server_is_playing = (server_is_playing != 1);
  int allow_mic;
  inputStream >> allow_mic;
  gameInfo->allow_mic = (allow_mic != 1);
  inputStream >> gameInfo->level_number;
  inputStream.read(gameInfo->level_name, sizeof(gameInfo->level_name));
  inputStream >> gameInfo->initial_updates_per_packet;
  inputStream >> gameInfo->initial_update_latency;
}
  

void TopologyMessage::reallyDeflateTo(AOStream& outputStream) const {

  outputStream << mNetTopology.tag;
  outputStream << mNetTopology.player_count;
  outputStream << mNetTopology.nextIdentifier;

  serialize_game_info(outputStream, &mNetTopology.gameInfo);

  for (int i = 0; i < MAXIMUM_NUMBER_OF_NETWORK_PLAYERS; i++) {
    /*    outputStream.write((char *)mNetTopology.players[i].dspAddress.host, 4);
    outputStream.write((char *)mNetTopology.players[i].dspAddress.port, 2);
    outputStream.write((char *)mNetTopology.players[i].ddpAddress.host, 4);
    outputStream.write((char *)mNetTopology.players[i].ddpAddress.port, 2); */
    outputStream << mNetTopology.players[i].identifier;
    outputStream << (mNetTopology.players[i].net_dead ? 1 : 0);
    serialize_player_info(outputStream, &mNetTopology.players[i].playerInfo);
  }
}

bool TopologyMessage::reallyInflateFrom(AIStream& inputStream) {
  inputStream >> mNetTopology.tag;
  inputStream >> mNetTopology.player_count;
  inputStream >> mNetTopology.nextIdentifier;

  unserialize_game_info(inputStream, &mNetTopology.gameInfo);
  
  for (int i = 0; i < MAXIMUM_NUMBER_OF_NETWORK_PLAYERS; i++) {
    /*    inputStream.read(mNetTopology.players[i].dspAddress.host, 4);
    inputStream.read(mNetTopology.players[i].dspAddress.port, 2);
    inputStream.read(mNetTopology.players[i].ddpAddress.host, 4);
    inputStream.read(mNetTopology.players[i].ddpAddress.port, 2); */
    inputStream >> mNetTopology.players[i].identifier;
    int net_dead;
    inputStream >> net_dead;
    mNetTopology.players[i].net_dead = (net_dead != 0);
    unserialize_player_info(inputStream, &mNetTopology.players[i].playerInfo);
  }
}

