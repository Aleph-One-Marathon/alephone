#ifndef __NETWORK_MESSAGES_H
#define __NETWORK_MESSAGES_H

/*
NETWORK_MESSAGES.H

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

#include "cseries.h"
#include "cstypes.h"

#include "network.h"
#include "network_private.h"
#include "AStream.h"
#include "Message.h"

// this file is for messages used by the setup piece of the code

enum { // message types
  kNetworkVersionMessage = 1001,
  kCapabilitiesMessage,
  kPlayerNameMessage,
  kServerToClientMessage,
  kDisconnectNowMessage,
  kChatNamesMessage,
  kBeginJoiningMessage,
  kJoinInfoMessage,
  kJoinAcceptedMessage,
  kStopJoiningMessage,
  kTopologyMessage,
  kMapMessage,
  kPhysicsMessage,
  kLuaMessage,
  kNetgameStartMessage,
  kChatMessage
};
  

typedef SimpleMessage<Uint32> NumberMessage;

class NumberAndStringMessage : public SmallMessageHelper
{
  NumberAndStringMessage(MessageTypeID inType, const char *inString = NULL,
			 Uint32 inNumber = 0)
    : mType(inType), mNumber(inNumber)
  {
    strncpy(mString, (inString == NULL) ? "" : inString, sizeof(mString));
    mString[sizeof(mString)] = '\0';
  }

  COVARIANT_RETURN(Message*, NumberAndStringMessage*) clone() const { 
    return new NumberAndStringMessage(*this);
  }

  MessageTypeID type() const { return mType; }
  const char * string() const { return mString; }
  Uint32 number() const { return mNumber; }

protected:
  void reallyDeflateTo(AOStream& outputStream) const;
  bool reallyInflateFrom(AIStream &inputStream);

private:
  MessageTypeID mType;
  Uint32 mNumber;
  char mString[1024];
};

class PlayerInfoMessage : public SmallMessageHelper
{
 public:
  enum { kType = kJoinInfoMessage };
  PlayerInfoMessage(player_info *playerInfo) {
    if (playerInfo) {
      mPlayerInfo = *playerInfo;
    }
  }
  
  COVARIANT_RETURN(Message*, PlayerInfoMessage*) clone() const {
    return new PlayerInfoMessage(*this);
  }
  
  MessageTypeID type() const { return kType; }
  const struct player_info* playerInfo() const { return &mPlayerInfo; }
  
 protected:
  void reallyDeflateTo(AOStream& outputStream) const;
  bool reallyInflateFrom(AIStream& inputStream);

 private:

  struct player_info mPlayerInfo;
};

class TopologyMessage : public SmallMessageHelper
{
 public:
  enum { kType = kTopologyMessage };
  TopologyMessage(struct NetTopology *netTopology) {
    if (netTopology) {
      mNetTopology = *netTopology;
    }
  }

  COVARIANT_RETURN(Message*, TopologyMessage*) clone() const {
    return new TopologyMessage(*this);
  }

  MessageTypeID type() const { return kType; }
  const struct NetTopology* NetTopology() const { return &mNetTopology; }

 protected:
  void reallyDeflateTo(AOStream& outputStream) const;
  bool reallyInflateFrom(AIStream& inputStream);

 private:
  struct NetTopology mNetTopology;
};



#endif
