#ifndef __CNETWORK_H
#define __CNETWORK_H

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

  void SetServerIdentifier(short identifier) { 
    serverPlayerIndex_ = identifier; 
  };

  short GetLocalPlayerIndex(void) { return localPlayerIndex_; };
  
  
  
 protected:

  void InitializeTopology(game_info gameInfo, player_info playerInfo);
  NetTopology topology_;

  short udpSocket_; // port number
  short state_ = stateUninitialized;
  short netState_ = netUninitialized; // the game protocols use this
  bool oldSelfSendStatus_;
  short localPlayerIndex_; // into topology
  short localPlayerIdentifier_;
  short serverPlayerIndex_; // into topology

  CommunicationsChannel serverChannel_;

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
    stateGGathering,
    stateGSendingGameData,
    stateGInGame,

    // join states
    stateJConnecting,
    stateJAwaitingVersion,
    stateJAwaitingChatInfo,
    stateJAwaitingStartJoin,
    stateJAwaitingAcceptJoin,
    stateJAwaitingNetGameState,
    stateJInGame,
    
    // gather states (for individual joiners)
    stateGJAwaitingVersion,
    stateGJAwaitingCapabilities,
    stateGJAwaitingName,
    stateGJReadyToJoin,
    stateGJAwaitingJoinInfo,
    stateGJReadyToPlay,
    stateGJInGame
  }
  
  enum {
    errorConnectionRefused
  }

  short netState_ = netUninitialized;
    

}

#endif
