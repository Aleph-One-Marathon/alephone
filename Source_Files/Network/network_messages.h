/*
 *  network_messages.h - TCPMess message types for network game setup

	Copyright (C) 2005 and beyond by Gregory Smith
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

#ifndef NETWORK_MESSAGES_H
#define NETWORK_MESSAGES_H

#include "cseries.h"
#include "AStream.h"
#include "Message.h"

#include "SDL_net.h"

#include "network_private.h"

enum {
  kHELLO_MESSAGE = 700,
  kJOINER_INFO_MESSAGE,
  kJOIN_PLAYER_MESSAGE,
  kACCEPT_JOIN_MESSAGE,
  kTOPOLOGY_MESSAGE,
  kMAP_MESSAGE,
  kPHYSICS_MESSAGE,
  kLUA_MESSAGE,
  kCHAT_MESSAGE,
  kUNKNOWN_MESSAGE_MESSAGE,
  kSCRIPT_MESSAGE
};

template <MessageTypeID tMessageType, typename tValueType>
class TemplatizedSimpleMessage : public SimpleMessage<tValueType>
{
 public:
  enum { kType = tMessageType };
  
  TemplatizedSimpleMessage() : SimpleMessage<tValueType>(tMessageType) { }

  TemplatizedSimpleMessage(tValueType inValue) : SimpleMessage<tValueType>(tMessageType, inValue) { }

  TemplatizedSimpleMessage<tMessageType, tValueType>* clone () const {
    return new TemplatizedSimpleMessage<tMessageType, tValueType>(*this);
  }
  
  MessageTypeID type() const { return kType; }
};

class AcceptJoinMessage : public SmallMessageHelper
{
 public:
  enum { kType = kACCEPT_JOIN_MESSAGE };
  
  AcceptJoinMessage() : SmallMessageHelper() { }
  
  AcceptJoinMessage(bool accepted, NetPlayer *player) : SmallMessageHelper() {
    mAccepted = accepted;
    mPlayer = *player;
  }

  AcceptJoinMessage *clone() const {
    return new AcceptJoinMessage(*this);
  }

  bool accepted() { return mAccepted; }
  void accepted(bool isAccepted) { mAccepted = isAccepted; }
  NetPlayer *player() { return &mPlayer; }
  void player(const NetPlayer *thePlayer) { mPlayer = *thePlayer; }
 
  MessageTypeID type() const { return kType; }

 protected:
  void reallyDeflateTo(AOStream& outputStream) const;
  bool reallyInflateFrom(AIStream& inputStream);
  
 private:
  
  bool mAccepted;
  NetPlayer mPlayer;
};

typedef DatalessMessage<kHELLO_MESSAGE> HelloMessage;

typedef TemplatizedSimpleMessage<kJOIN_PLAYER_MESSAGE, int16> JoinPlayerMessage;
class JoinerInfoMessage : public SmallMessageHelper
{
 public: 
  enum { kType = kJOINER_INFO_MESSAGE };

  JoinerInfoMessage() : SmallMessageHelper() { }

  JoinerInfoMessage(prospective_joiner_info *info) : SmallMessageHelper() {
    mInfo = *info;
  }

  JoinerInfoMessage *clone() const {
    return new JoinerInfoMessage(*this);
  }

  prospective_joiner_info *info() { return &mInfo; }
  void info(const prospective_joiner_info *theInfo) { 
    mInfo = *theInfo;
  }

  MessageTypeID type() const { return kType; }

 protected:
  void reallyDeflateTo(AOStream& outputStream) const;
  bool reallyInflateFrom(AIStream& inputStream);

 private:
  prospective_joiner_info mInfo;
  
};
      
class LuaMessage : public BigChunkOfDataMessage
{
 public:
  enum { kType = kLUA_MESSAGE };

  LuaMessage(const Uint8* inBuffer = NULL, size_t inLength = 0) :
    BigChunkOfDataMessage(kType, inBuffer, inLength) { };

  LuaMessage(const LuaMessage& other) : BigChunkOfDataMessage(other) { }

  COVARIANT_RETURN(Message *, LuaMessage *) clone () const {
    return new LuaMessage(*this);
  }
};

class MapMessage : public BigChunkOfDataMessage
{
 public:
  enum { kType = kMAP_MESSAGE };
  
  MapMessage(const Uint8* inBuffer = NULL, size_t inLength = 0) : 
    BigChunkOfDataMessage(kType, inBuffer, inLength) { };

  MapMessage(const MapMessage& other) : BigChunkOfDataMessage(other) { }

  COVARIANT_RETURN(Message *, MapMessage *) clone () const {
    return new MapMessage(*this);
  }

};

class NetworkChatMessage : public SmallMessageHelper
{
 public:
  enum { kType = kCHAT_MESSAGE };
  enum { CHAT_MESSAGE_SIZE = 1024 };

  NetworkChatMessage(const char *chatText = NULL, Uint16 senderID = 0)
    : mSenderID(senderID)
    {
      strncpy(mChatText, (chatText == NULL) ? "" : chatText, CHAT_MESSAGE_SIZE);
      mChatText[CHAT_MESSAGE_SIZE - 1] = '\0';
    }

  NetworkChatMessage * clone() const {
    return new NetworkChatMessage(*this);
  }

  MessageTypeID type() const { return kType; }
  const char *chatText() const { return mChatText; }
  Uint16 senderID() const { return mSenderID; }

 protected:
  void reallyDeflateTo(AOStream& outputStream) const;
  bool reallyInflateFrom(AIStream& includeStream);

 private:
  char mChatText[CHAT_MESSAGE_SIZE];
  Uint16 mSenderID;
};

class PhysicsMessage : public BigChunkOfDataMessage
{
 public:
  enum { kType = kPHYSICS_MESSAGE };
  
  PhysicsMessage(const Uint8* inBuffer = NULL, size_t inLength = 0) :
    BigChunkOfDataMessage(kType, inBuffer, inLength) { };

  PhysicsMessage(const PhysicsMessage& other) : BigChunkOfDataMessage(other) { }

  COVARIANT_RETURN(Message *, PhysicsMessage *) clone() const {
    return new PhysicsMessage(*this);
  }
  
};

typedef TemplatizedSimpleMessage<kSCRIPT_MESSAGE, int16> ScriptMessage;

class TopologyMessage : public SmallMessageHelper
{
 public: 
  enum { kType = kTOPOLOGY_MESSAGE };
  
  TopologyMessage() : SmallMessageHelper() { }

  TopologyMessage(NetTopology *topology) : SmallMessageHelper() {
    mTopology = *topology;
  }

  TopologyMessage *clone() const {
    return new TopologyMessage(*this);
  }

  NetTopology *topology() { return &mTopology; }
  void topology(const NetTopology *theTopology) { mTopology = *theTopology; }
  
  MessageTypeID type() const { return kType; }

 protected:
  void reallyDeflateTo(AOStream& outputStream) const;
  bool reallyInflateFrom(AIStream& inputStream);
  
 private:
  NetTopology mTopology;
};




#endif // NETWORK_MESSAGES_H
