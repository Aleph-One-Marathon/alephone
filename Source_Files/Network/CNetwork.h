#ifndef CNETWORK_H
#define CNETWORK_H

/*
CNETWORK.H

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

class Network
{
 public:
  bool Enter();
  bool Exit();

  bool Sync();
  bool UnSync();

  // distribution
  void AddDistributionFunction(int16 dataTypeID, NetDistributionProc proc,
			       bool lossy);
  void RemoveDistributionFunction(int16 dataTypeID);
  const NetDistributionInfo* GetDistributionInfoForType(int16 type);
  void DistributeInformation(short type, void *buffer, short buffer_size,
				bool send_to_self);

  short NetState(void) { return netState_; };

  void BecomeGatherer();
  bool BeginGathering(game_info gameInfo, player_info playerInfo, 
		      bool resuming_game);
  void CancelGathering(void);
  bool StartGame();
  int Join(unsigned char *player_name,
	   unsigned char *player_type,
	   player_info *playerInfo,
	   const char *address,
	   Uint16 port);
  void Cancel(void);

  void ServerIdentifier(short identifier) { 
    serverPlayerIndex_ = identifier; 
  }

  short LocalPlayerIndex(void) { return localPlayerIndex_; }
  bool NumberOfPlayerIsValid(void) { return true; }
  short NumPlayers(void) { return topology_->player_count; }
  player_info *PlayerInfo(short player_index) { 
    return topology_->players[player_index].player_info; }
  game_info *GameInfo(void) { return topology_->game_info; }
  void SetupTopologyFromStarts(const player_start_data* inStartArray,
			       short inStartCount);
  bool IsServer(void) { 
    return localPlayerIndex_ != 0 && localPlayerIndex_ == serverPlayerIndex_;}


  // game data
  // CNetwork makes a copy, and doesn't free the original buffer
  void SetMap(byte* data, size_t length);
  void SetPhysics(byte* data, size_t length);
  void SetNetscript(byte* data, size_t length);

  void DistributeGameData(void);
  
  
 protected:

  void InitializeTopology(game_info gameInfo, player_info playerInfo);
  NetTopology topology_;

  // message handlers
  void handleNetworkVersionMessage(Message *message, CommunicationsChannel *channel);
  void handleCapabilitiesMessage(Message *message, CommunicationsChannel *channel);
  void handleServerToClientMessage(Message *message, CommunicationsChannel *channel);
  void handleDisconnectNowMessage(Message *message, CommunicationsChannel *channel);
  void handleChatNamesMessage(Message *message, CommunicationsChannel *channel);
  void handlePlayerInfoMessage(Message *message, CommunicationsChannel *channel);
  void handleJoinAcceptedMessage(Message *message, CommunicationsChannel *channel);
  void handleTopologyMessage(Message *message, CommunicationsChannel *channel);
  void handleChangeMapMessage(Message *message, CommunicationsCHannel *channel);
  void handleMapMessage(Message *message, CommunicationsChannel *channel);
  void handlePhysicsMessage(Message *message, CommunicationsChannel *channel);
  void handleLuaMessage(Message *message, CommunicationsChannel *channel);
  void handleNetgameStartMessage(Message *message, CommunicationsChannel *channel);
  void handleChatMessage(Message *message, CommunicationsChannel *channel);
  void handleUnknownMessage(Message *message, CommunicationsChannel *channel);

  // message handler objects
  MessageHandler *NetworkVersionMessageHandler_;
  MessageHandler *CapabilitiesMessageHandler_;
  MessageHandler *ServerToClientMessageHandler_;
  MessageHandler *DisconnectNowMessageHandler_;
  MessageHandler *ChatNamesMessageHandler_;
  MessageHandler *PlayerInfoMessageHandler_;
  MessageHandler *JoinAcceptedMessageHandler_;
  MessageHandler *TopologyMessageHandler_;
  MessageHandler *ChangeMapMessageHandler_;
  MessageHandler *MapMessageHandler_;
  MessageHandler *PhysicsMessageHandler_;
  MessageHandler *LuaMessageHandler_;
  MessageHandler *NetgameStartMessageHandler_;
  MessageHandler *ChatMessageHandler_;
  MessageHandler *UnknownMessageHandler_;
  
  
  short udpSocket_; // port number
  short state_ = stateUninitialized;
  short netState_ = netUninitialized; // the game protocols use this
  bool oldSelfSendStatus_;
  short localPlayerIndex_; // into topology
  short localPlayerIdentifier_;
  short serverPlayerIndex_; // into topology

  byte *mapData_;
  size_t mapLength_;
  byte *physicsData_;
  size_t physicsLength_;
  byte *netscriptData_;
  size_t netScriptLength_;

  CommunicationsChannelFactory *communicationsChannelFactory_;
  // this is your connection to the server, if you're a joiner or wannabe
  // if you're the server, this is unused for the moment
  CommunicationsChannel *serverChannel_;

  // these are unused unless you're the server
  CommunicationsChannel *listeningChannel_;
  CommunicationsChannel *clientChannels_[MAX_CONNECTIONS];
  int channelToPlayerMap[MAX_CONNECTIONS];
  int playerToChannelMap[MAXIMUM_NUMBER_OF_NETWORK_PLAYERS];

  // this is valid for servers (join info) and (potential) joiners (for
  //      looking up chat name / color)
  player_info *clients[MAX_CONNECTIONS];
  int clientStates[MAX_CONNECTIONS];

  bool resuming_saved_game_;

  typedef std::map<int16, NetDistributionInfo> distribution_info_map_t;
  distribution_info_map_t distributionInfoMap_;

  RingGameProtocol ringGameProtocol_;
  StarGameProtocol starGameProtocol_;
  NetworkGameProtocol currentGameProtocol_;

  // states
  enum {
    stateUninitialized,
    stateInitialized,

    // gather states
    stateGListening,
    stateGGathering,
    stateGSendingGameData,
    stateGInGame,

    // join states
    stateJConnecting,
    stateJAwaitingVersion,
    stateJAwaitingCapabilities,
    stateJAwaitingChatInfo,
    stateJAwaitingAcceptJoin,
    stateJAwaitingLevelData,
    stateJAwaitingNetGameStart,
    stateJInGame,
    
    // gather states (for individual joiners)
    stateGJAwaitingVersion,
    stateGJAwaitingCapabilities,
    stateGJAwaitingJoinInfo,
    stateGJAwaitingGather,
    stateGJAwaitingReadyToPlay,
    stateGJReadyToPlay,
    stateGJInGame
  };
  
  enum {
    errorConnectionRefused
  };

};

#endif
