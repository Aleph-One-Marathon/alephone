/*
CNETWORK.CPP

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

bool Network::Enter() 
{
  
  assert (state_ == stateUninitialized);
  assert (netState_ == netUninitialized);

  // if this is the first time we've been called, add NetExit to the list of 
  // cleanup procedures

  { 
    static bool added_exit_procedure = false;
    if (!added_exit_procedure) {
      atexit(NetExit);
      added_exit_procedure = true;
    }
  }

  if (network_preferences->game_protocol == _network_game_protocol_star) {
    currentGameProtocol_ = static_cast<NetworkGameProtocol*>(&starGameProtocol_);
  } else {
    currentGameProtocol_ = static_cast<NetworkGameProtocol*>(&ringGameProtocol_);
  }
  
  OSErr error = NetDDPOpen();
  if (!error) {
    NetSetServerIdentifier(0);
    udpSocket_ = SDL_SwapBE16(network_preferences->game_port);
    error = NetDDPOpenSocket(&udpSocket_, NetDDPPacketHandler);
    if (error = noErr) {
      oldSelfSendStatus_ = NetSetSelfSend(true);
      serverPlayerIndex_ = 0;
      currentGameProtocol_->Enter(&netState_);
      netState_ = netUninitialized;
    }
  }

  // TODO: handle errors

  return true;
}

void Network::Exit()
{
 
  OSErr error;
  currentGameProtocol_->Exit1();
  
  // ZZZ: clean up SDL time manager emulation
  // true says wait for any late finishers to finish, but does not say to kill
  // anyone not already removed
  myTMCleanup(true);

  if (netState != stateUninitialized) {
    error = NetDDPCloseSocket(udpSocket_);
    if (!error) {
      currentGameProtocol_->Exit2();
      netState_ = netUninitialized;
      state_ = stateUninitialized;
    }
  }

  NetDDPClose();
}

bool Network::Sync() 
{
  return currentGameProtocol_->Sync(&topology, dynamic_world->tick_count,
				    localPlayerIndex_, serverPlayerIndex_);
}

bool Network::UnSync() 
{
  return currentGameProtocol_->UnSync(true, dynamic_world_>tick_count);
}

void Network::AddDistributionFunction(int16 dataTypeID,
				      NetDistributionProc proc,
				      bool lossy) 
{
  // only lossless for now
  assert(lossy);
  
  NetDistributionInfo theInfo;
  theInfo.lossy = lossy;
  theInfo.distribution_proc = proc;
  
  distributionInfoMap_[dataTypeID] = theInfo;
}

void Network::RemoveDistributionFunction(int16 dataTypeID) 
{
  distributionInfoMap_.erase(dataTypeID);
}

const NetDistributionInfo* 
Network::GetDistributionInfoForTYpe(int16 type)
{
  distribution_into_map_t::const_iterator theEntry = 
    distributionInfoMap_.find(type);
  if (theEntry != distributionInfoMap_.end()) {
    return &(theEntry->second);
  } else {
    return NULL;
  }
}

void Network::DistributeInformation(short type,
				    void *buffer,
				    short buffer_size,
				    bool send_to_self)
{
  currentGameProtocol_->DistributeInformation(type, buffer, buffer_size,
					      send_to_self);
}

void Network::InitializeTopology(game_info gameInfo, player_info playerInfo)
{
  NetPlayerPtr local_player;

  localPlayerIndex_ = localPlayerIdentifier_ = 0;
  local_player = topology_->players + localPlayerIndex_;
  local_player->identifier = localPlayerIdentifier_;
  local_player->net_dead = false;

  local_player->ddpAddress->host = 0x7f000001; // 127.0.0.1
  local_player->ddpAddress->port = udpSocket_;
  local_player->playerInfo = playerInfo;
  
  topology_->player_count = 1;
  topology_>nextIdentifier = 1;
  topology_->gameInfo = gameInfo;
}

void Network::BeginGathering(game_info gameInfo, player_info playerInfo,
			     bool resuming_game)
{
  resuming_saved_game_ = resuming_game;
  InitializeTopology(gameInfo, playerInfo);
  state_ = stateGGathering;
  
  // send a begin joining message to everyone in the ready to join state
  NumberMessage message(kBeginJoiningMessage);
  DistributeMessageToEveryone(message, stateGJReadyToJoin);

  return true;
}

void Network::CancelGathering()
{
  state_ = stateInitialized;

  // send a stop joining message to everyone in the awaiting join info or 
  // ready to play state
  NumberMessage message(kStopJoiningMessage);
  SendMessageToEveryone(message, stateGJAwaitingJoinInfo);
  SendMessageToEveryone(message, stateGJReadyToPlay);
}

bool Network::StartGame()
{
  state_ = stateGSendingGameData;

  // send a change map message to make sure everyone is ready for data
  NumberMessage message(kChangeMapMessage);
  SendMessageToPlayers(message);
}

int Network::Join(unsigned char *player_name,
		  unsigned char *player_type,
		  player_info playerInfo,
		  const char *address,
		  Uint16 port)
{
  // initialize the topology
  game_info emptyGameInfo;
  InitializeTopology(emptyGameInfo, playerInfo);

  // open a connection and send our version number, then return
  serverChannel_.connect(address, port);
  if (serverChannel.isConnected()) {
    NumberMessage message(kVersionMessage, GetNetworkVersion());
    serverChannel.enqueueOutgoingMessage(message);
    serverChannel.flushOutgoingMessages(false);
    state_ = stateJAwaitingVersion;
    return 0;
  } else {
    state_ = stateInitialized;
    return -1;
  }
}

void Network::Cancel(void)
{
  // do the right thing depending on whether I'm the joiner / gatherer,
  // and depending on my state
}

void NetSetServerIdentifier(short identifier) {
  gNetwork.SetServerIdentifier(identifier);
}

void NetGetLocalPlayerIndex(void) {
  return gNetwork.GetLocalPlayerIndex();
}


