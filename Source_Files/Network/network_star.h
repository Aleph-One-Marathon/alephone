/*
 *  network_star.h

	Copyright (C) 2003 and beyond by Woody Zenfell, III
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

 *  Created by Woody Zenfell, III on Mon Apr 28 2003.
 */

#ifndef NETWORK_STAR_H
#define NETWORK_STAR_H

#include "TickBasedCircularQueue.h"
#include "ActionQueues.h"

#include "sdl_network.h"

#include "map.h" // TICKS_PER_SECOND
#include "player.h" // GetRealActionQueues
#include "interface.h" // process_action_flags (despite paf() being defined in vbl.*)

#include <stdio.h>

enum {
        kEndOfMessagesMessageType = 0x454d,	// 'EM'
        kTimingAdjustmentMessageType = 0x5441,	// 'TA'
        kPlayerNetDeadMessageType = 0x4e44,	// 'ND'

        kSpokeToHubGameDataPacketV1Magic = 0x53484731,	// 'SHG1'
        kHubToSpokeGameDataPacketV1Magic = 0x48534731,	// 'HSG1'

        kPregameTicks = TICKS_PER_SECOND * 3,	// Synchronization/timing adjustment before real data
        kActionFlagsSerializedLength = 4,	// bytes for each serialized action_flags_t (should be elsewhere)
};

typedef uint32 action_flags_t;	// (should be elsewhere)
typedef ConcreteTickBasedCircularQueue<action_flags_t> TickBasedActionQueue;
typedef WritableTickBasedCircularQueue<action_flags_t> WritableTickBasedActionQueue;


// This is a bit hacky yeah, we really ought to check both RealActionQueues and the recording queues, etc.
template <typename tValueType>
class LegacyActionQueueToTickBasedQueueAdapter : public WritableTickBasedCircularQueue<tValueType> {
public:
        explicit LegacyActionQueueToTickBasedQueueAdapter(int inTargetPlayerIndex) : mPlayerIndex(inTargetPlayerIndex) { reset(0); }
        void reset(int32 inTick) { mWriteTick = inTick; GetRealActionQueues()->resetQueue(mPlayerIndex); }
        int32 availableCapacity() const { return GetRealActionQueues()->availableCapacity(mPlayerIndex); }
        void enqueue(const tValueType& inFlags) { process_action_flags(mPlayerIndex, static_cast<const uint32 *>(&inFlags), 1);  ++mWriteTick; }
        int32 getWriteTick() const { return mWriteTick; }

protected:
        int 	mPlayerIndex;
        int32	mWriteTick;
};


class XML_ElementParser;

extern void hub_initialize(int32 inStartingTick, size_t inNumPlayers, const NetAddrBlock* const* inPlayerAddresses, size_t inLocalPlayerIndex);
extern void hub_cleanup(bool inGraceful, int32 inSmallestPostGameTick);
extern void hub_received_network_packet(DDPPacketBufferPtr inPacket);
extern XML_ElementParser* Hub_GetParser();
extern void DefaultHubPreferences();
extern void WriteHubPreferences(FILE* F);

extern void spoke_initialize(const NetAddrBlock& inHubAddress, int32 inFirstTick, size_t inNumberOfPlayers, WritableTickBasedActionQueue* const inPlayerQueues[], bool inPlayerConnectedStatus[], size_t inLocalPlayerIndex, bool inHubIsLocal);
extern void spoke_cleanup(bool inGraceful);
extern void spoke_received_network_packet(DDPPacketBufferPtr inPacket);
extern int32 spoke_get_net_time();
extern XML_ElementParser* Spoke_GetParser();
extern void DefaultSpokePreferences();
extern void WriteSpokePreferences(FILE* F);

#endif // NETWORK_STAR_H
