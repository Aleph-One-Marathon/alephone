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
    if (error == noErr) {
      //      oldSelfSendStatus_ = NetSetSelfSend(true);
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

void Network::BecomeGatherer() {
  assert(state_ == stateInitialized);
  // start listening for connections
  communicationsChannelFactory_ = new CommunicationsChannelFactory(15267);
  if (communicationsChannelFactory_->isFunctional()) {
    state_ = stateGListening;
    listeningChannel = communicationsChannelFactory_->newIncomingConnection();
  } else {
    // can't gather for some reason, error out
  }
}
    

void Network::BeginGathering(game_info gameInfo, player_info playerInfo,
			     bool resuming_game)
{
  assert(state_ == stateGListening);
  resuming_saved_game_ = resuming_game;
  InitializeTopology(gameInfo, playerInfo);
  state_ = stateGGathering;
  
  // send a begin joining message to everyone in the ready to join state
  NumberMessage message(kBeginJoiningMessage);
  SendMessageToEveryone(message, stateGJReadyToJoin);

  return true;
}

void Network::CancelGathering()
{
  assert(state_ == stateGGathering);
  state_ = stateGListening;

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
  if (serverChannel_.isConnected()) {
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

void Network::SetupTopologyFromStarts(const player_start_data *inStartArray, short inStartCount) {
  // no resuming saved games yet
  return;
}

static void replace_data(byte* copy, size_t &copy_length,
			 byte* data, size_t length) {
  if (copy) {
    free(copy);
    copy = NULL;
  }
  copy = (byte *) malloc (length);
  copy_length = length;
}

void Network::SetMap(byte* data, size_t length) {
  replace_data(mapData_, mapLength_, data, length);
}

void Network::SetPhysics(byte* data, size_t length) {
  replace_data(physicsData_, physicsLength_, data, length);
}

void Network::SetLua(byte* data, size_t length) {
  replace_data(luaData_, luaLength_, data, length);
}


void Network::DistributeGameData(void) {
  
  assert(mapLength_ > 0);
  if (physicsLength_ > 0 && physicsData_) {
    BigChunkOfDataMessage physicsMessage(kPhysicsMessage, physicsData_, physicsLength_);
    SendMessageToPlayers(physicsMessage);
    physicsLength_ = 0;
    free(physicsData_);
    physicsData_ = NULL;
  }
  if (luaLength_ > 0 && luaData_) {
    BigChunkOfDataMessage luaMessage(kLuaMessage, luaData_, luaLength_);
    SendMessageToPlayers(luaMessage);
    luaLength_ = 0;
    free(luaData_);
    luaData_ = NULL;
  }
  
  BigChunkOfDataMessage mapMessage(kMapMessage, mapData_, mapLength_);
  SendMessageToPlayers(mapMessage);
  mapLength_ = 0;
  free(mapData_);
  mapData_ = NULL;
}


    




