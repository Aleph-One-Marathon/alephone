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

void Network::Network()
{
  NetworkVersionMessageHandler = newMessageHandlerMethod(this, &Network::handleNetworkVersionMessage);
  CapabilitiesMessageHandler = newMessageHandlerMethod(this, &Network::handleCapabilitiesMessage);
  ServerToClientMessageHandler = newMessageHandlerMethod(this, &Network::handleServerToClientMessage);
  DisconnectNowMessageHandler = newMessageHandlerMethod(this, &Network::handleDisconnectNowMessage);
  ChatNamesMessageHandler = newMessageHandlerMethod(this, &Network::handleChatNamesMessage);
  PlayerInfoMessageHandler = newMessageHandlerMethod(this, &Network::handlePlayerInfoMessage);
  JoinAcceptedMessageHandler = newMessageHandlerMethod(this, &Network::handleJoinAcceptedMessage);
  TopologyMessageHandler = newMessageHandlerMethod(this, &Network::handleTopologyMessage);
  ChangeMapMessageHandler = newMessageHandlerMethod(this, &Network::handleChangeMapMessage);
  MapMessageHandler = newMessageHandlerMethod(this, &Network::handleMapMessage);
  PhysicsMessageHandler = newMessageHandlerMethod(this, &Network::handlePhysicsMessage);
  LuaMessageHandler = newMessageHandlerMethod(this, &Network::handleLuaMessage);
  NetgameStartMessageHandler = newMessageHandlerMethod(this, &Network::handleNetgameStartMessage);
  ChatMessageHandler = newMessageHandlerMethod(this, &Network::handleChatMessage);
  UnknownMessageHandler = newMessageHandlerMethod(this, &Network::handleUnknownMessage);
}



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
  communicationsChannelFactory_ = new CommunicationsChannelFactory(15367);
  if (communicationsChannelFactory_->isFunctional()) {
    state_ = stateGGathering;
    listeningChannel = communicationsChannelFactory_->newIncomingConnection();
  } else {
    // can't gather for some reason, error out
  }
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

bool Network::GathererAndActive() {
  return (state_ == stateGGathering ||
	  state_ == stateGSendingGameData ||
	  state_ == stateGInGame);
}

void Network::handleNetworkVersionMessage(Message *message, CommunicationsChannel *channel) {
  int clientIndex = channel->Memento();
  if (GathererAndActive() &&
      clientStates[clientIndex] == stateGJAwaitingVersion) { 
    // reply with our network version
    NumberMessage message(kVersionMessage, GetNetworkVersion());
    clientChannels[clientIndex]->enqueueOutgoingMessage(message);
    clientStates[clientIndex] == stateGJAwaitingCapabilities;
  } else if (state_ == stateJAwaitingVersion) { 
    // reply with capabilities
    BigChunkOfDataMessage capabilitiesMessage(kCapabilitiesMessage, NULL, 0); // hehe
    serverChannel_.enqueueOutgoingMessage(capabilitiesMessage);
    state_ == stateJAwaitingCapabilities;

  } else {
    // ignore the message and log the error
  }
}

void Network::handleCapabilitiesMessage(Message *message, CommunicationsChannel *channel) {
  int clientIndex = channel->Memento();
  if (GathererAndActive() &&
      clientStates[clientIndex] == stateGJAwaitingCapabilities) {
    // act on capabilities
    
    // including replying with our own capabilities
    BigChunkOfDataMessage capabilitiesMessage(kCapabilitiesMessage, NULL, 0);
    clientChannels[clientIndex]->enqueueOutgoingMessage(message);
    // wait for player info
    clientStates[clientIndex] = stateGJAwaitingJoinInfo;
  } else if (state_ == stateJAwaitingCapabilities) {
    // reply with join info
    PlayerInfoMessage playerInfoMessage(topology_->players[local_player_index_]);
    serverChannel_.enqueueOutgoingMessage(playerInfoMessage);
    // wait for chat names
    state_ == stateJAwaitingChatInfo;
  } else {
    // ignore the message and log the error
  }
}
					
void Network::handleServerToClientMessage(Message *message, CommunicationsChannel *channel) {
  int clientIndex = channel->Memento();
  // for now, ignore server to client messages
}

void Network::handleDisconnectNowMessage(Message *message, CommunicationsChannel *channel) {
  int clientIndex = channel->Memento();
  // nyi
}

void Network::handleChatNamesMessage(Message *message, CommunicationsChannel *channel) {
  int clientIndex = channel->Memento();
  if (GathererAndActive()) {
    // ignore the message and log the error
  } else if (state_ == stateJAwaitingChatInfo) {
    // copy my chat names stuff into my client array
    
    state_ = stateJAwaitingAcceptJoin;
  } else {
    // ignore the message and log the error
  }
}

void Network::handlePlayerInfoMessage(Message *message, CommunicationsChannel *channel) {
  int clientIndex = channel->Memento();
  if (GathererActive()) {
    if (clientStates[clientIndex] == stateGJAwaitingJoinInfo) {
      // add this person's info to the list of clients
      clients[clientIndex] = new player_info;
      memcpy(clients[clientIndex], message->playerInfo(), sizeof(player_info));
      
      // he's waiting for us to gather him, now
      clientStates[clientIndex] == stateGJAwaitingGather;
      
      // send him the chat names
      ChatNamesMessage chatNamesMessage();
      clientsChannels_[clientIndex]->enqueueOutgoingMessage(chatNamesMessage);
      
      // callback the interface and say this client is available for gathering
      
      // update the clients who already have the chat name list with this client's info
      PlayerInfoMessage playerInfoMessage(message->playerInfo(), clientIndex);
      for (int i = 0; i < MAX_CONNECTIONS; i++) {
	if (clientStates[i] == stateGJAwaitingGather ||
	    clientStates[i] == stateGJAwaitingReadyToPlay ||
	    clientStates[i] == stateGJInGame) {
	  clientChannels_[i]->enqueueOutgoingMessage(playerInfoMessage);
	}
      }
    } else if (clientStates[clientIndex] == stateGJAwaitingAcceptJoin ||
	       clientStates[clientIndex] == stateGJAwaitingLevelInfo) {
      // implement changing your player info before the game has started
    }
  } else if (state_ == stateJAwaitingAcceptJoin ||
	     state_ == stateJAwaitingLevelInfo || 
	     state_ == stateJAwaitingNetGameStart ||
	     state_ == stateJInGame) {
    // copy this person's new info in
    if (clients[message->index()]) {
      memcpy(clients[message->index()], message->playerInfo(), sizeof(player_info));
    } else {
      // add this guy
      clients[message->index()] = new player_info;
      memcpy(clients[message->index()], message->playerInfo(), sizeof(player_info));
    }
  } else {
    // ignore the message and log the error
  }
}

void Network::handleJoinAcceptedMessage(Message *message, CommunicationsChannel *channel) {
  int clientIndex = channel->Memento();

  if (state_ == stateJAwaitingAcceptJoin) {
    // we've been gathered; update the interface, and change our state
    state_ = stateJAwaitingLevelInfo;
  } else {
    // ignore the message and log the error
  }
}

void Network::handleTopologyMessage(Message *message, CommunicationsChannel *channel) {
  int clientIndex = channel->Memento();
  
  if (state_ == stateJAwaitingLevelInfo) {
    // update my topology
    topology_ = *(message->topology());
    // update the ui
  } else {
    // ignore the message and log the error
  }
}


void Network::handleChangeMapMessage(Message *message, CommunicationsChannel *channel) {
  int clientIndex = channel->Memento();

  // no changing maps just yet
}

void Network::handleMapMessage(Message *message, CommunicationsChannel *channel) {
  int clientIndex = channel->Memento();

}

void Network::handlePhysicsMessage(Message *message, CommunicationsChannel *channel) {
  int clientIndex = channel->Memento();
}

void Network::handleLuaMessage(Message *message, CommunicationsChannel *channel) {
  int clientIndex = channel->Memento();
}

void Network::handleNetgameStartMessage(Message *message, CommunicationsChannel *channel) {
  int clientIndex = channel->Memento();

  // start the game
}

void Network::handleChatMessage(Message *message, CommunicationsChannel *channel) {
  int clientIndex = channel->Memento();

  // meh
}

void Network::handleUnknownMessage(Message *message, CommunicationsChannel *channel) {
  int clientIndex = channel->Memento();

  // meh
}



    




