/*
 *  network_star_hub.cpp

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

 *  The portion of the star game network protocol run on a single machine, perhaps (eventually)
 *	without an actual player on the machine - or even without most of the game, for that matter.
 *
 *  Created by Woody Zenfell, III on Mon Apr 28 2003.
 *
 *  May 27, 2003 (Woody Zenfell): lossy byte-stream distribution.
 */

#include "network_star.h"

//#include "sdl_network.h"
#include "TickBasedCircularQueue.h"
#include "network_private.h"
#include "mytm.h"
#include "AStream.h"
#include "Logging.h"
#include "WindowedNthElementFinder.h"
#include "SDL_timer.h" // SDL_Delay()

#include <vector>
#include <map>

// Synchronization:
// hub_received_network_packet() is not reentrant
// hub_tick() is not reentrant
// hub_tick() and hub_received_network_packet() are mutex
// hmm had probably better take the lock in hub_cleanup() too as part of thread-cancellation strategy,
// unless thread-cancellation API guarantees not only that routine won't be called again, but that it's
// now finished executing as well.
// hub_initialize() should be safe as long as it's not called while the other threads are running.
// (this last bit assumes that hub_cleanup() doesn't exit until threads are completely finished.)

// specifically,
// In Mac OS 9, hrm hope that starting and stopping the packet listener and ticker
//   is safe with regard to hub_initialize() and hub_cleanup()
// In Mac OS X "Carbon", well maybe it should use full SDL-style stuff, sounds like Mac OS X
//   emulates Time Manager basically the same way I do anyway.  Well, if not moving to full
//   SDL-style, need to take mytm_mutex inside hub_tick, hub_initialize,
//   and hub_cleanup.  (mytm_mutex is already taken by packet-listener emulation.)
// In full SDL-style, mytm_mutex is already taken by both packet-listener and mytm emulation,
//   so we just need to take it at hub_initialize and hub_cleanup.
// All of the above applies to spokes as well.

// Hub<->local spoke communication: (THIS APPLIES ONLY TO THE LOCAL SPOKE ON THE HUB'S MACHINE)
// Since current NetDDP* system only supports the notion of a single datagram socket,
//   we need to have another mechanism to allow the local spoke and the hub to communicate.
// I propose having one packet-buffer for each direction of communication, into which the
//   "outgoing" packet is stored.  When the task that generated the packet is finished, instead
//   of returning and allowing the mutex to be released, it calls the other task directly.
//   Naturally this other task must not take the mutex, because we're already holding it.
//   Note that this other task could potentially generate a packet also.
// So, here are the "longest" possible situations:
//   TAKE MUTEX - hub_tick stores packet - spoke_received_packet - RELEASE MUTEX
//   TAKE MUTEX - hub_received_packet stores packet - spoke_received_packet - RELEASE MUTEX
//   TAKE MUTEX - spoke_tick stores packet - hub_received_packet stores packet - spoke_received_packet - RELEASE MUTEX
// Hmm, spoke_received_packet won't ever be called outside of the above circumstances, since there's
//   no real socket it can receive packets on.
// All of the above "chains" are easily accomplished, and fall out naturally from whatever mechanisms
//   are in place for synchronization as listed further above.

// What do we need to know, initially, from the outside world?
// number of players
// address for each player
// whether each player is actually connected
// starting game-tick
// local player index (only for data received reference tick and for hacky communicate-with-local-spoke stuff)

/*
enum NetworkState {
        eNetworkDown,
        eNetworkStartingUp,
        eNetworkUp,
        eNetworkComingDown
};

static NetworkState sNetworkState;
*/


enum {
        kFlagsQueueSize = TICKS_PER_SECOND / 2,
        kDefaultPregameWindowSize = TICKS_PER_SECOND / 2,
        kDefaultInGameWindowSize = TICKS_PER_SECOND * 5,
	kDefaultPregameNthElement = 2,
	kDefaultInGameNthElement = kDefaultInGameWindowSize / 2,
//        kAverageTickArrivalOffsetMaximumWindowSize = kInGameWindowSize,
        kDefaultPregameTicksBeforeNetDeath = 20 * TICKS_PER_SECOND,
        kDefaultInGameTicksBeforeNetDeath = 3 * TICKS_PER_SECOND,
        kDefaultRecoverySendPeriod = TICKS_PER_SECOND / 2,
	kLossyStreamingDataBufferSize = 1280,
};


struct HubPreferences {
//	int32	mFlagsQueueSize;
	int32	mPregameWindowSize;
	int32	mInGameWindowSize;
	int32	mPregameNthElement;
	int32	mInGameNthElement;
	int32	mPregameTicksBeforeNetDeath;
	int32	mInGameTicksBeforeNetDeath;
	int32	mRecoverySendPeriod;
};

static HubPreferences sHubPreferences;


// sNetworkTicker advances even if the game clock doesn't.
// sLastNetworkTickSent is used to force us to resend packets (at a lower rate) even if we're no longer
// getting new data.
static int32 sNetworkTicker;
static int32 sLastNetworkTickSent;

// We have a pregame startup period to help establish (via standard adjustment mechanism) everyone's
// timing.  Ticks smaller than sSmallestRealGameTick are part of this startup period.  They smell
// just like real in-game ticks, except that spokes won't enqueue them on player_queues, and we
// may have different adjustment window sizes and timeout periods for pre-game and in-game ticks.
static int32 sSmallestRealGameTick;

// Once everyone ACKs this tick, we're satisfied the game is ended.  (They should all agree on which
// tick is last due to the symmetric execution model.)
static int32 sSmallestPostGameTick;

// The sFlagsQueues hold all flags for ticks for which we've received data from at least one
// station, but for which we haven't received an ACK from all stations.
// sFlagsQueues[all].getReadIndex() == sPlayerDataDisposition.getReadIndex();
// max(sFlagsQueues[all].getWriteIndex()) == sPlayerDataDisposition.getWriteIndex();
// min(sFlagsQueues[all].getWriteIndex()) == sSmallestIncompleteTick;
typedef std::vector<TickBasedActionQueue> TickBasedActionQueueCollection;
static TickBasedActionQueueCollection	sFlagsQueues;
typedef ConcreteTickBasedCircularQueue<int> WindowedAverageQueue;

struct NetworkPlayer_hub {
//        NetworkPlayer_hub() {} // : mAverageTickArrivalOffsetQueue(kAverageTickArrivalOffsetMaximumWindowSize) {}
        
        NetAddrBlock	mAddress;		// network address of player
        bool		mConnected;		// is player still connected?
        int32		mLastNetworkTickHeard;	// our sNetworkTicker last time we got a packet from them
        int32		mSmallestUnacknowledgedTick;

	WindowedNthElementFinder<int32>	mNthElementFinder;
	// We can "hear" ticks that we can't enqueue - this is useful for timing data.
	int32		mSmallestUnheardTick;

        // These three are used to implement a windowed average.  Although they're
        // TickBasedActionQueues, the ticks are not synchronized with game-ticks.
        // There's an element for each packet we received from this player (up to
        // mAverageTickArrivalOffsetWindowSize, at which point older ones are discarded)
        // The value of each element is the difference (positive or negative) between the
        // packet's highest-numbered tick and our reference spoke's highest-numbered tick.
        // (Currently there's expected to be a player on the hub, so the reference spoke
        // is the one corresponding to the local player.)  So, the average can be used to
        // tell us if this player is delivering data consistently early or consistently late.
        // If he is, we can send him a message telling him to adjust appropriately.
//        WindowedAverageQueue	mAverageTickArrivalOffsetQueue;
//        int32		mAverageTickArrivalOffsetRunningSum;
//        int		mAverageTickArrivalOffsetWindowSize;

        // When we decide a timing adjustment is needed, we include the timing adjustment
        // request in every packet outbound to the player until we're sure he's seen it.
        // In particular, mTimingAdjustmentTick is set to the sSmallestIncompleteTick, so we
        // know nobody's received data for that tick yet.  We continue to send the message
        // until the station ACKs past that tick; at that point we know he must have seen
        // our message.
        // When we get that ACK, we clear the window and let it fill up again to make sure
        // we have clean, fresh, post-adjustment data to work from.  We won't make any new
        // timing adjustment requests to a station with an outstanding timing adjustment
        // or an incomplete averaging window.
        int		mOutstandingTimingAdjustment;
        int32		mTimingAdjustmentTick;

        // If the player is dropped during a game, we need to tell the other players about it.
        // This is accomplished in much the same way as the timing adjustment - we include
        // a "player netdead" message in every outbound packet until we're sure they've gotten
        // the message.  mNetDeadTick, then, works pretty much just like mTimingAdjustmentTick.
        // mNetDeadTick is the first tick for which the netdead player isn't providing data.
        int32		mNetDeadTick;
};

// Housekeeping queues:
// sPlayerDataDisposition holds an element for every tick for which data has been received from
// someone, but which at least one player has not yet acknowledged.
// sPlayerDataDisposition.getReadIndex() <= sSmallestIncompleteTick <= sPlayerDataDisposition.getWriteIndex()
// sSmallestIncompleteTick indexes into sPlayerDataDisposition also; it divides the queue into ticks
// for which data has been received from someone but not yet everyone (>= sSmallestIncompleteTick) and
// ticks for which data has been sent out (to everyone) but for which someone hasn't yet acknowledged
// (< sSmallestIncompleteTick).

// The value of a queue element is a bit-set (indexed by player index) with a 1 bit for each player
// that we're waiting on.  So, we can mask out successive players' bits as their traffic reaches us;
// when the value hits 0, all players have checked in and we can advance an index.
// sConnectedPlayersBitmask has '1' set for every connected player.
static MutableElementsTickBasedCircularQueue<uint32>	sPlayerDataDisposition(kFlagsQueueSize);
static int32 sSmallestIncompleteTick;
static uint32 sConnectedPlayersBitmask;

typedef std::map<NetAddrBlock, int>	AddressToPlayerIndexType;
static AddressToPlayerIndexType	sAddressToPlayerIndex;

typedef std::vector<NetworkPlayer_hub>	NetworkPlayerCollection;
static NetworkPlayerCollection	sNetworkPlayers;
static size_t			sLocalPlayerIndex;

static DDPFramePtr	sOutgoingFrame = NULL;
static DDPPacketBuffer	sLocalOutgoingBuffer;
static bool		sNeedToSendLocalOutgoingBuffer = false;

static byte		sLossyStreamingData[kLossyStreamingDataBufferSize];
static uint16		sOutstandingLossyByteStreamLength;
static uint32		sOutstandingLossyByteStreamDestinations;
static uint8		sOutstandingLossyByteStreamSender;
static int16		sOutstandingLossyByteStreamType;

static myTMTaskPtr	sHubTickTask = NULL;
static bool		sHubActive = false;	// used to enable the packet handler
static bool		sHubInitialized = false;



static void hub_check_for_completion();
static void player_acknowledged_up_to_tick(size_t inPlayerIndex, int32 inSmallestUnacknowledgedTick);
static bool player_provided_flags_from_tick_to_tick(size_t inPlayerIndex, int32 inFirstNewTick, int32 inSmallestUnreceivedTick);
static void hub_received_game_data_packet_v1(AIStream& ps, int inSenderIndex);
static void process_messages(AIStream& ps, int inSenderIndex);
static void process_optional_message(AIStream& ps, int inSenderIndex, uint16 inMessageType);
static void make_player_netdead(int inPlayerIndex);
static bool hub_tick();
static void send_packets();



// These are excellent candidates for templatization, but MSVC++6.0 has broken function templates.
// (Actually, they might not be broken if the template parameter is a typename, but... not taking chances.)
static inline NetworkPlayer_hub&
getNetworkPlayer(size_t inIndex)
{
        assert(inIndex < sNetworkPlayers.size());
        return sNetworkPlayers[inIndex];
}

static inline TickBasedActionQueue&
getFlagsQueue(size_t inIndex)
{
        assert(inIndex < sFlagsQueues.size());
        return sFlagsQueues[inIndex];
}



static inline bool
operator <(const NetAddrBlock& a, const NetAddrBlock& b)
{
        return memcmp(&a, &b, sizeof(a)) < 0;
}



static OSErr
send_frame_to_local_spoke(DDPFramePtr frame, NetAddrBlock *address, short protocolType, short port)
{
        sLocalOutgoingBuffer.datagramSize = frame->data_size;
        memcpy(sLocalOutgoingBuffer.datagramData, frame->data, frame->data_size);
        sLocalOutgoingBuffer.protocolType = protocolType;
        // We ignore the 'source address' because the spoke does too.
        sNeedToSendLocalOutgoingBuffer = true;
        return noErr;
}



static inline void
check_send_packet_to_spoke()
{
        if(sNeedToSendLocalOutgoingBuffer)
                spoke_received_network_packet(&sLocalOutgoingBuffer);

        sNeedToSendLocalOutgoingBuffer = false;
}



#ifndef INT32_MAX
#define INT32_MAX 0x7fffffff
#endif

void
hub_initialize(int32 inStartingTick, size_t inNumPlayers, const NetAddrBlock* const* inPlayerAddresses, size_t inLocalPlayerIndex)
{
//        assert(sNetworkState == eNetworkDown);

//        sNetworkState = eNetworkJustStartingUp;

        assert(inLocalPlayerIndex < inNumPlayers);
        sLocalPlayerIndex = inLocalPlayerIndex;

	sSmallestPostGameTick = INT32_MAX;
        sSmallestRealGameTick = inStartingTick;
        int32 theFirstTick = inStartingTick - kPregameTicks;

        if(sOutgoingFrame == NULL)
                sOutgoingFrame = NetDDPNewFrame();

        sNeedToSendLocalOutgoingBuffer = false;

        sNetworkPlayers.clear();
        sFlagsQueues.clear();
        sNetworkPlayers.resize(inNumPlayers);
        sFlagsQueues.resize(inNumPlayers, TickBasedActionQueue(kFlagsQueueSize));
        sAddressToPlayerIndex.clear();
        sConnectedPlayersBitmask = 0;

	sOutstandingLossyByteStreamLength = 0;

        for(size_t i = 0; i < inNumPlayers; i++)
        {
                NetworkPlayer_hub& thePlayer = sNetworkPlayers[i];

                if(inPlayerAddresses[i] != NULL)
                {
                        thePlayer.mConnected = true;
                        sConnectedPlayersBitmask |= (((uint32)1) << i);
                        thePlayer.mAddress = *(inPlayerAddresses[i]);
                        // Currently, all-0 address is cue for local spoke.
                        if(i == sLocalPlayerIndex)
                                obj_clear(thePlayer.mAddress);
                        sAddressToPlayerIndex[thePlayer.mAddress] = i;
                }
                else
                {
                        thePlayer.mConnected = false;
                }

                thePlayer.mLastNetworkTickHeard = 0;
                thePlayer.mSmallestUnacknowledgedTick = theFirstTick;
		thePlayer.mSmallestUnheardTick = theFirstTick;
//                thePlayer.mAverageTickArrivalOffsetQueue.reset(theFirstTick);
//                thePlayer.mAverageTickArrivalOffsetRunningSum = 0;
//                thePlayer.mAverageTickArrivalOffsetWindowSize = kPregameWindowSize;
		thePlayer.mNthElementFinder.reset(sHubPreferences.mPregameWindowSize);
                thePlayer.mOutstandingTimingAdjustment = 0;
                thePlayer.mTimingAdjustmentTick = 0;
                thePlayer.mNetDeadTick = theFirstTick - 1;

                sFlagsQueues[i].reset(theFirstTick);
        }
        
        sPlayerDataDisposition.reset(theFirstTick);
        sSmallestIncompleteTick = theFirstTick;
        sNetworkTicker = 0;
        sLastNetworkTickSent = 0;

        sHubActive = true;

        sHubTickTask = myXTMSetup(1000/TICKS_PER_SECOND, hub_tick);

	sHubInitialized = true;
}



void
hub_cleanup(bool inGraceful, int32 inSmallestPostGameTick)
{
	if(sHubInitialized)
	{
		if(inGraceful)
		{
			// Signal our demise
			sSmallestPostGameTick = inSmallestPostGameTick;

			// We have to do a check now in case the conditions are already met
			if(take_mytm_mutex())
			{
				hub_check_for_completion();
				release_mytm_mutex();
			}
			
			// Now we should wait/sleep for the rest of the machinery to wind down
			// Packet handler will set sHubActive = false once it has acks from all connected players;
			while(sHubActive)
			{
// Here we try to isolate the "Classic" Mac OS (we can only sleep on the others)
#if !defined(mac) || defined(__MACH__)
				SDL_Delay(10);
#endif
			}
		}
		else
		{		
			// Stop processing incoming packets (packet processor won't start processing another packet
			// due to sHubActive = false, and we know it's not in the middle of processing one because
			// we take the mutex).
			if(take_mytm_mutex())
			{
				sHubActive = false;
				release_mytm_mutex();
			}
		}
	
		// Mark the tick task for cancellation (it won't start running again after this returns).
		myTMRemove(sHubTickTask);
		sHubTickTask = NULL;
	
		// This waits for the tick task to actually finish - so we know the tick task isn't in
		// the middle of processing when we do the rest of the cleanup below.
		myTMCleanup(true);
		
		sNetworkPlayers.clear();
		sFlagsQueues.clear();
		sAddressToPlayerIndex.clear();
		NetDDPDisposeFrame(sOutgoingFrame);
		sOutgoingFrame = NULL;
	}
}



static void
hub_check_for_completion()
{
	// When all players (including the local spoke) have either ACKed up to sSmallestPostGameTick
	// or become disconnected, we're clear to cleanup.  (In other words, we should avoid cleaning
	// up if there are connected players that haven't ACKed up to the game's end tick.)
	bool someoneStillActive = false;
	for(size_t i = 0; i < sNetworkPlayers.size(); i++)
	{
		NetworkPlayer_hub& thePlayer = sNetworkPlayers[i];
		if(thePlayer.mConnected && thePlayer.mSmallestUnacknowledgedTick < sSmallestPostGameTick)
		{
			someoneStillActive = true;
			break;
		}
	}

	if(!someoneStillActive)
		sHubActive = false;
}
		


void
hub_received_network_packet(DDPPacketBufferPtr inPacket)
{
        // Processing packets?
        if(!sHubActive)
                return;

	logContext("hub processing a received packet");
	
        AIStreamBE ps(inPacket->datagramData, inPacket->datagramSize);

        // Find sender
        AddressToPlayerIndexType::iterator theEntry = sAddressToPlayerIndex.find(inPacket->sourceAddress);
        if(theEntry == sAddressToPlayerIndex.end())
                return;

        int theSenderIndex = theEntry->second;

        // Unconnected players should not have entries in sAddressToPlayerIndex
        assert(getNetworkPlayer(theSenderIndex).mConnected);

        try {
                uint32	thePacketMagic;
                ps >> thePacketMagic;
        
                switch(thePacketMagic)
                {
                        case kSpokeToHubGameDataPacketV1Magic:
                                hub_received_game_data_packet_v1(ps, theSenderIndex);
                                break;
        
                        default:
                                break;
                }
        }
        catch (...)
        {
                // ignore errors - we just discard the packet, effectively.
        }

        check_send_packet_to_spoke();
}



// I suppose to be safer, this should check the entire packet before acting on any of it.
// As it stands, a malformed packet could have have a well-formed prefix of it interpreted
// before the remainder is discarded.
static void
hub_received_game_data_packet_v1(AIStream& ps, int inSenderIndex)
{
        // Process the piggybacked acknowledgement
        int32	theSmallestUnacknowledgedTick;
        ps >> theSmallestUnacknowledgedTick;

        // If ack is too soon we throw out the entire packet to be safer
        if(theSmallestUnacknowledgedTick > sSmallestIncompleteTick)
        {
                logAnomaly3("received ack from player %d for tick %d; have only sent up to %d", inSenderIndex, theSmallestUnacknowledgedTick, sSmallestIncompleteTick);
                return;
        }                

        player_acknowledged_up_to_tick(inSenderIndex, theSmallestUnacknowledgedTick);

        // If that's all the data, we're done
        if(ps.tellg() == ps.maxg())
                return;

        // Process messages, if present
        process_messages(ps, inSenderIndex);

        // If that's all the data, we're done
        if(ps.tellg() == ps.maxg())
                return;

        // If present, process the action_flags
        int32	theStartTick;
        ps >> theStartTick;

        // Make sure there's an integral number of action_flags
        int	theRemainingDataLength = ps.maxg() - ps.tellg();
        if(theRemainingDataLength % kActionFlagsSerializedLength != 0)
                return;

        int	theActionFlagsCount = theRemainingDataLength / kActionFlagsSerializedLength;

        TickBasedActionQueue& theQueue = getFlagsQueue(inSenderIndex);

        // Packet is malformed if they're skipping ahead
        if(theStartTick > theQueue.getWriteTick())
                return;

        // Skip redundant flags without processing/checking them
        int	theRedundantActionFlagsCount = theQueue.getWriteTick() - theStartTick;
        int	theRedundantDataLength = theRedundantActionFlagsCount * kActionFlagsSerializedLength;
        ps.ignore(theRedundantDataLength);

        // Enqueue flags that are new to us
        int	theUsefulActionFlagsCount = theActionFlagsCount - theRedundantActionFlagsCount;
        int	theRemainingQueueSpace = theQueue.availableCapacity();
        int	theEnqueueableFlagsCount = min(theUsefulActionFlagsCount, theRemainingQueueSpace);
        
        for(int i = 0; i < theEnqueueableFlagsCount; i++)
        {
                action_flags_t theActionFlags;
                ps >> theActionFlags;
                theQueue.enqueue(theActionFlags);
        }

	// Update timing data
	NetworkPlayer_hub& thePlayer = getNetworkPlayer(inSenderIndex);
	NetworkPlayer_hub& theLocalPlayer = getNetworkPlayer(sLocalPlayerIndex);
	while(thePlayer.mSmallestUnheardTick < theStartTick + theActionFlagsCount)
	{
		int32 theReferenceTick = theLocalPlayer.mSmallestUnheardTick;
		int32 theArrivalOffset = thePlayer.mSmallestUnheardTick - theReferenceTick;
		logDump2("player %d's arrivalOffset is %d", inSenderIndex, theArrivalOffset);
		thePlayer.mNthElementFinder.insert(theArrivalOffset);
		thePlayer.mSmallestUnheardTick++;
	}

        // Make the pregame -> ingame transition
        if(thePlayer.mSmallestUnheardTick >= sSmallestRealGameTick && thePlayer.mNthElementFinder.window_size() != sHubPreferences.mInGameWindowSize)
		//                thePlayer.mAverageTickArrivalOffsetWindowSize = kInGameWindowSize;
		thePlayer.mNthElementFinder.reset(sHubPreferences.mInGameWindowSize);

	if(thePlayer.mOutstandingTimingAdjustment == 0 && thePlayer.mNthElementFinder.window_full())
	{
		/*                        float theAverageOffset = ((float)thePlayer.mAverageTickArrivalOffsetRunningSum) / ((float)thePlayer.mAverageTickArrivalOffsetWindowSize);
		theAverageOffset = floor(theAverageOffset);
		thePlayer.mOutstandingTimingAdjustment = static_cast<int>(theAverageOffset);
		*/
		thePlayer.mOutstandingTimingAdjustment = thePlayer.mNthElementFinder.nth_smallest_element((thePlayer.mSmallestUnheardTick >= sSmallestRealGameTick) ? sHubPreferences.mInGameNthElement : sHubPreferences.mPregameNthElement);
		if(thePlayer.mOutstandingTimingAdjustment != 0)
		{
			thePlayer.mTimingAdjustmentTick = sSmallestIncompleteTick;
			logTrace3("tick %d: asking player %d to adjust timing by %d", sSmallestIncompleteTick, inSenderIndex, thePlayer.mOutstandingTimingAdjustment);
		}
	}

        // Do any needed post-processing
        if(theEnqueueableFlagsCount > 0)
        {
                bool shouldSend = player_provided_flags_from_tick_to_tick(inSenderIndex, theStartTick + theRedundantActionFlagsCount, theStartTick + theRedundantActionFlagsCount + theEnqueueableFlagsCount);
                if(shouldSend)
                        send_packets();
        }
} // hub_received_game_data_packet_v1()


static void
player_acknowledged_up_to_tick(size_t inPlayerIndex, int32 inSmallestUnacknowledgedTick)
{
	logTrace2("player_acknowledged_up_to_tick(%d, %d)", inPlayerIndex, inSmallestUnacknowledgedTick);
	
        NetworkPlayer_hub& thePlayer = getNetworkPlayer(inPlayerIndex);

        // Ignore late ACK's
        if(inSmallestUnacknowledgedTick < thePlayer.mSmallestUnacknowledgedTick)
                return;

        // We've heard from this player
        thePlayer.mLastNetworkTickHeard = sNetworkTicker;

        // Mark us ACKed for each intermediate tick
        for(int theTick = thePlayer.mSmallestUnacknowledgedTick; theTick < inSmallestUnacknowledgedTick; theTick++)
        {
		logDump2("tick %d: sPlayerDataDisposition=%d", theTick, sPlayerDataDisposition[theTick]);
		
                assert(sPlayerDataDisposition[theTick] & (((uint32)1) << inPlayerIndex));
                sPlayerDataDisposition[theTick] &= ~(((uint32)1) << inPlayerIndex);
                if(sPlayerDataDisposition[theTick] == 0)
                {
                        assert(theTick == sPlayerDataDisposition.getReadTick());
                        
                        sPlayerDataDisposition.dequeue();
                        for(size_t i = 0; i < sFlagsQueues.size(); i++)
                        {
                                if(sFlagsQueues[i].size() > 0)
                                {
                                        assert(sFlagsQueues[i].getReadTick() == theTick);
                                        sFlagsQueues[i].dequeue();
                                }
                        }
                }
        }

        thePlayer.mSmallestUnacknowledgedTick = inSmallestUnacknowledgedTick;

        // If player has acknowledged timing adjustment tick, adjustment is no longer
        // outstanding, and we need to restart the data collection to get a new average.
        if(thePlayer.mOutstandingTimingAdjustment != 0 && thePlayer.mTimingAdjustmentTick < inSmallestUnacknowledgedTick)
        {
                thePlayer.mOutstandingTimingAdjustment = 0;
		thePlayer.mNthElementFinder.reset();
//                thePlayer.mAverageTickArrivalOffsetQueue.reset(thePlayer.mAverageTickArrivalOffsetQueue.getWriteTick());
//                thePlayer.mAverageTickArrivalOffsetRunningSum = 0;
        }

	hub_check_for_completion();
	
} // player_acknowledged_up_to_tick()



// Returns true if we now have enough data to send at least one new tick
static bool
player_provided_flags_from_tick_to_tick(size_t inPlayerIndex, int32 inFirstNewTick, int32 inSmallestUnreceivedTick)
{
	logTrace3("player_provided_flags_from_tick_to_tick(%d, %d, %d)", inPlayerIndex, inFirstNewTick, inSmallestUnreceivedTick);
	
        bool shouldSend = false;

        for(int i = sPlayerDataDisposition.getWriteTick(); i < inSmallestUnreceivedTick; i++)
        {
		logDump2("tick %d: enqueueing sPlayerDataDisposition %d", i, sConnectedPlayersBitmask);
                sPlayerDataDisposition.enqueue(sConnectedPlayersBitmask);
        }

        for(int i = inFirstNewTick; i < inSmallestUnreceivedTick; i++)
        {
		logDump2("tick %d: sPlayerDataDisposition=%d", i, sPlayerDataDisposition[i]);
		
                assert(sPlayerDataDisposition[i] & (((uint32)1) << inPlayerIndex));
                sPlayerDataDisposition[i] &= ~(((uint32)1) << inPlayerIndex);
                if(sPlayerDataDisposition[i] == 0)
                {
                        assert(sSmallestIncompleteTick == i);
                        sSmallestIncompleteTick++;
                        shouldSend = true;

                        // Now people need to ACK
                        sPlayerDataDisposition[i] = sConnectedPlayersBitmask;
                }

/*                while(thePlayer.mAverageTickArrivalOffsetQueue.size() >= thePlayer.mAverageTickArrivalOffsetWindowSize)
                {
                        thePlayer.mAverageTickArrivalOffsetRunningSum -= thePlayer.mAverageTickArrivalOffsetQueue.peek(thePlayer.mAverageTickArrivalOffsetQueue.getReadTick());
                        thePlayer.mAverageTickArrivalOffsetQueue.dequeue();
                }
                thePlayer.mAverageTickArrivalOffsetRunningSum += theArrivalOffset;
                thePlayer.mAverageTickArrivalOffsetQueue.enqueue(theArrivalOffset);
*/
                // If player's data is not arriving along with our reference player's data,
                // ask player to adjust his timing
//                if(thePlayer.mOutstandingTimingAdjustment == 0 && thePlayer.mAverageTickArrivalOffsetQueue.size() == thePlayer.mAverageTickArrivalOffsetWindowSize)
        } // loop over ticks with new data

        return shouldSend;
} // player_provided_flags_from_tick_to_tick()



static void
process_messages(AIStream& ps, int inSenderIndex)
{
        bool done = false;

        while(!done)
        {
                uint16	theMessageType;
                ps >> theMessageType;

                switch(theMessageType)
                {
                        case kEndOfMessagesMessageType:
                                done = true;
                                break;

                        default:
                                process_optional_message(ps, inSenderIndex, theMessageType);
                                break;
                }
        }
}



static void
process_lossy_byte_stream_message(AIStream& ps, int inSenderIndex, uint16 inLength)
{
	assert(inSenderIndex >= 0 && inSenderIndex < sNetworkPlayers.size());
	
	if(sOutstandingLossyByteStreamLength > 0)
		logNote4("discarding %uh bytes of lossy streaming data of distribution type %hd from player %hu destined for 0x%x", sOutstandingLossyByteStreamLength, sOutstandingLossyByteStreamType, sOutstandingLossyByteStreamSender, sOutstandingLossyByteStreamDestinations);

	size_t theHeaderStreamPosition = ps.tellg();
	ps >> sOutstandingLossyByteStreamType >> sOutstandingLossyByteStreamDestinations;
	sOutstandingLossyByteStreamLength = inLength - (ps.tellg() - theHeaderStreamPosition);
	sOutstandingLossyByteStreamSender = inSenderIndex;

	logDump4("got %d bytes of lossy stream type %d from player %d for destinations 0x%x", sOutstandingLossyByteStreamLength, sOutstandingLossyByteStreamType, sOutstandingLossyByteStreamSender, sOutstandingLossyByteStreamDestinations);

	uint16 theSpilloverDataLength = 0;
	
	if(sOutstandingLossyByteStreamLength > kLossyStreamingDataBufferSize)
	{
		logNote4("too many (%uh) bytes of lossy streaming data of distribution type %hd from player %hu destined for 0x%lx; truncating", sOutstandingLossyByteStreamLength, sOutstandingLossyByteStreamType, sOutstandingLossyByteStreamSender, sOutstandingLossyByteStreamDestinations);
		theSpilloverDataLength = sOutstandingLossyByteStreamLength - kLossyStreamingDataBufferSize;
		sOutstandingLossyByteStreamLength = kLossyStreamingDataBufferSize;
	}

	ps.read(sLossyStreamingData, sOutstandingLossyByteStreamLength);
	ps.ignore(theSpilloverDataLength);
}



static void
process_optional_message(AIStream& ps, int inSenderIndex, uint16 inMessageType)
{
        // All optional messages are required to give their length in the two bytes
        // immediately following their type.  (The message length value does not include
        // the space required for the message type ID or the encoded message length.)
        uint16 theMessageLength;
        ps >> theMessageLength;

	if(inMessageType == kSpokeToHubLossyByteStreamMessageType)
		process_lossy_byte_stream_message(ps, inSenderIndex, theMessageLength);
	else
	{
		// Currently we ignore (skip) all optional messages
		ps.ignore(theMessageLength);
	}
}



static void
make_player_netdead(int inPlayerIndex)
{
	logContext1("making player %d netdead", inPlayerIndex);
	
        NetworkPlayer_hub& thePlayer = getNetworkPlayer(inPlayerIndex);

        thePlayer.mNetDeadTick = sSmallestIncompleteTick;
        thePlayer.mConnected = false;
        sConnectedPlayersBitmask &= ~(((uint32)1) << inPlayerIndex);
        sAddressToPlayerIndex.erase(thePlayer.mAddress);

	// We save this off because player_provided... call below may change it.
	int32 theSavedIncompleteTick = sSmallestIncompleteTick;
	
        // Pretend for housekeeping that he's provided data for all currently known ticks
        // We go from the first tick for which we don't actually have his data through the last
        // tick we actually know about.
        player_provided_flags_from_tick_to_tick(inPlayerIndex, getFlagsQueue(inPlayerIndex).getWriteTick(), sPlayerDataDisposition.getWriteTick());

        // Pretend for housekeeping that he's already acknowledged all sent ticks
        player_acknowledged_up_to_tick(inPlayerIndex, theSavedIncompleteTick);
}



static bool
hub_tick()
{
        sNetworkTicker++;

	logContext1("performing hub_tick %d", sNetworkTicker);

        // Check for newly netdead players
        bool shouldSend = false;
        for(size_t i = 0; i < sNetworkPlayers.size(); i++)
        {
                int theSilentTicksBeforeNetDeath = (sNetworkPlayers[i].mSmallestUnacknowledgedTick < sSmallestRealGameTick) ? sHubPreferences.mPregameTicksBeforeNetDeath : sHubPreferences.mInGameTicksBeforeNetDeath;
                if(sNetworkPlayers[i].mConnected && sNetworkTicker - sNetworkPlayers[i].mLastNetworkTickHeard > theSilentTicksBeforeNetDeath)
                {
                        make_player_netdead(i);
                        shouldSend = true;
                }
        }

        if(shouldSend)
                send_packets();
        else
        {
                // Make sure we send at least every once in a while to keep things going
                if(sNetworkTicker > sLastNetworkTickSent && (sNetworkTicker - sLastNetworkTickSent) >= sHubPreferences.mRecoverySendPeriod)
                        send_packets();
        }

        check_send_packet_to_spoke();

        // We want to run again.
        return true;
}



static void
send_packets()
{
        for(size_t i = 0; i < sNetworkPlayers.size(); i++)
        {
                NetworkPlayer_hub& thePlayer = sNetworkPlayers[i];
                if(thePlayer.mConnected)
                {
                        AOStreamBE ps(sOutgoingFrame->data, ddpMaxData);

                        try {
                                // Packet magic, acknowledgement
                                ps << (uint32)kHubToSpokeGameDataPacketV1Magic
                                        << getFlagsQueue(i).getWriteTick();
        
                                // Messages
                                // Timing adjustment?
                                if(thePlayer.mOutstandingTimingAdjustment != 0)
                                {
                                        ps << (uint16)kTimingAdjustmentMessageType
                                                << (int8)(thePlayer.mOutstandingTimingAdjustment);
                                }
        
                                // Netdead players?
                                for(size_t j = 0; j < sNetworkPlayers.size(); j++)
                                {
                                        if(thePlayer.mSmallestUnacknowledgedTick <= sNetworkPlayers[j].mNetDeadTick)
                                        {
                                                ps << (uint16)kPlayerNetDeadMessageType
                                                        << (uint8)j	// dead player index
                                                        << sNetworkPlayers[j].mNetDeadTick;
                                        }
                                }

				// Lossy streaming data?
				if(sOutstandingLossyByteStreamLength > 0 && ((sOutstandingLossyByteStreamDestinations & (((uint32)1) << i)) != 0))
				{
					logDump4("packet to player %d will contain %d bytes of lossy byte stream type %d from player %d", i, sOutstandingLossyByteStreamLength, sOutstandingLossyByteStreamType, sOutstandingLossyByteStreamSender);
					// In AStreams, sizeof(packed scalar) == sizeof(unpacked scalar)
					uint16 theMessageLength = sizeof(sOutstandingLossyByteStreamType) + sizeof(sOutstandingLossyByteStreamSender) + sOutstandingLossyByteStreamLength;
					
					ps << (uint16)kHubToSpokeLossyByteStreamMessageType
						<< theMessageLength
						<< sOutstandingLossyByteStreamType
						<< sOutstandingLossyByteStreamSender;
					ps.write(sLossyStreamingData, sOutstandingLossyByteStreamLength);
				}
        
                                // End of messages
                                ps << (uint16)kEndOfMessagesMessageType;
        
                                // We use this flag to make sure we encode the start tick at most once, and
                                // only if we're actually sending action_flags.
                                bool haveSentStartTick = false;
        
                                // Action_flags!!
                                // First, preprocess the players to figure out at what tick they'll each stop
                                // contributing
                                vector<int32> theSmallestTickWeWontSend;
                                theSmallestTickWeWontSend.resize(sNetworkPlayers.size());
                                for(size_t j = 0; j < sNetworkPlayers.size(); j++)
                                {
                                        // Don't encode our own flags
                                        if(j == i)
                                        {
                                                theSmallestTickWeWontSend[j] = thePlayer.mSmallestUnacknowledgedTick - 1;
                                                continue;
                                        }
        
                                        theSmallestTickWeWontSend[j] = sSmallestIncompleteTick;
                                        NetworkPlayer_hub& theOtherPlayer = sNetworkPlayers[j];
        
                                        // Don't send flags for netdead people
                                        if(!theOtherPlayer.mConnected && theSmallestTickWeWontSend[j] > theOtherPlayer.mNetDeadTick)
                                                theSmallestTickWeWontSend[j] = theOtherPlayer.mNetDeadTick;
                                }
        
                                // Now, encode the flags in tick-major order (this is much easier to decode
                                // at the other end)
                                for(int32 tick = thePlayer.mSmallestUnacknowledgedTick; tick < sSmallestIncompleteTick; tick++)
                                {
                                        for(size_t j = 0; j < sNetworkPlayers.size(); j++)
                                        {
                                                if(tick < theSmallestTickWeWontSend[j])
                                                {
                                                        if(!haveSentStartTick)
                                                        {
                                                                ps << tick;
                                                                haveSentStartTick = true;
                                                        }
                                                        ps << getFlagsQueue(j).peek(tick);
                                                }
                                        }
                                }
        
                                // Send the packet
                                sOutgoingFrame->data_size = ps.tellp();
                                if(i == sLocalPlayerIndex)
                                        send_frame_to_local_spoke(sOutgoingFrame, &thePlayer.mAddress, kPROTOCOL_TYPE, 0 /* ignored */);
                                else
                                        NetDDPSendFrame(sOutgoingFrame, &thePlayer.mAddress, kPROTOCOL_TYPE, 0 /* ignored */);
                        } // try
                        catch (...)
                        {
				logWarning("Caught exception while constructing/sending outgoing packet");
                        }
                        
                } // if(connected)

        } // iterate over players

        sLastNetworkTickSent = sNetworkTicker;

	sOutstandingLossyByteStreamLength = 0;
	
} // send_packets()



static inline const char *BoolString(bool B) {return (B ? "true" : "false");}

enum {
	// kOutgoingFlagsQueueSizeAttribute,
	kPregameTicksBeforeNetDeathAttribute,
	kInGameTicksBeforeNetDeathAttribute,
	kPregameWindowSizeAttribute,
	kInGameWindowSizeAttribute,
	kPregameNthElementAttribute,
	kInGameNthElementAttribute,
	kRecoverySendPeriodAttribute,
	kNumAttributes
};

static const char* sAttributeStrings[kNumAttributes] =
{
	//	"outgoing_flags_queue_size",
	"pregame_ticks_before_net_death",
	"ingame_ticks_before_net_death",
	"pregame_window_size",
	"ingame_window_size",
	"pregame_nth_element",
	"ingame_nth_element",
	"recovery_send_period",
};

static int32* sAttributeDestinations[kNumAttributes] =
{
	//	&sHubPreferences.mOutgoingFlagsQueueSize,
	&sHubPreferences.mPregameTicksBeforeNetDeath,
	&sHubPreferences.mInGameTicksBeforeNetDeath,
	&sHubPreferences.mPregameWindowSize,
	&sHubPreferences.mInGameWindowSize,
	&sHubPreferences.mPregameNthElement,
	&sHubPreferences.mInGameNthElement,
	&sHubPreferences.mRecoverySendPeriod,
};

class XML_HubConfigurationParser: public XML_ElementParser
{
public:
	bool Start();
	bool HandleAttribute(const char *Tag, const char *Value);
	bool AttributesDone();

	XML_HubConfigurationParser(): XML_ElementParser("hub") {}

protected:

        bool	mAttributePresent[kNumAttributes];
	int32	mAttribute[kNumAttributes];
};

bool XML_HubConfigurationParser::Start()
{
        for(int i = 0; i < kNumAttributes; i++)
                mAttributePresent[i] = false;

	return true;
}

static const char* sAttributeMultiplySpecifiedString = "attribute multiply specified";

bool XML_HubConfigurationParser::HandleAttribute(const char *Tag, const char *Value)
{
	for(size_t i = 0; i < kNumAttributes; i++)
	{
		if(StringsEqual(Tag,sAttributeStrings[i]))
		{
			if(!mAttributePresent[i]) {
				if(ReadInt32Value(Value,mAttribute[i])) {
					mAttributePresent[i] = true;
					return true;
				}
				else
					return false;
			}
			else {
				ErrorString = sAttributeMultiplySpecifiedString;
				return false;
			}
		}
	}
	
	UnrecognizedTag();
	return false;
}

bool XML_HubConfigurationParser::AttributesDone() {
	// Ignore out-of-range values
	for(int i = 0; i < kNumAttributes; i++)
	{
		if(mAttributePresent[i])
		{
			switch(i)
			{
				case kPregameTicksBeforeNetDeathAttribute:
				case kInGameTicksBeforeNetDeathAttribute:
				case kRecoverySendPeriodAttribute:
				case kPregameWindowSizeAttribute:
				case kInGameWindowSizeAttribute:
					if(mAttribute[i] < 1)
					{
						// I don't know whether this actually does anything if I don't return false,
						// but I'd like to honor the user's wishes as far as I can without just throwing
						// up my hands.
						BadNumericalValue();
						logWarning3("improper value %d for attribute %s of <hub>; must be at least 1.  using default of %d", mAttribute[i], sAttributeStrings[i], *(sAttributeDestinations[i]));
						mAttributePresent[i] = false;
					}
					else
					{
						*(sAttributeDestinations[i]) = mAttribute[i];
					}
					break;

				case kPregameNthElementAttribute:
				case kInGameNthElementAttribute:
					if(mAttribute[i] < 0)
					{
						BadNumericalValue();
						logWarning3("improper value %d for attribute %s of <hub>; must be at least 1.  using default of %d", mAttribute[i], sAttributeStrings[i], *(sAttributeDestinations[i]));
						mAttributePresent[i] = false;
					}
					else
					{
						*(sAttributeDestinations[i]) = mAttribute[i];
					}
					break;

				default:
					assert(false);
					break;
			} // switch(attribute)

		} // if attribute present

	} // loop over attributes

	// The checks above are not sufficient to catch all bad cases; if user specified a window size
	// smaller than default, this is our only chance to deal with it.
	if(sHubPreferences.mPregameNthElement >= sHubPreferences.mPregameWindowSize) {
		logWarning5("value for <hub> attribute %s (%d) must be less than value for %s (%d).  using %d", sAttributeStrings[kPregameNthElementAttribute], sHubPreferences.mPregameNthElement, sAttributeStrings[kPregameWindowSizeAttribute], sHubPreferences.mPregameWindowSize, sHubPreferences.mPregameWindowSize - 1);
		
		sHubPreferences.mPregameNthElement = sHubPreferences.mPregameWindowSize - 1;
	}

	if(sHubPreferences.mInGameNthElement >= sHubPreferences.mInGameWindowSize) {
		logWarning5("value for <hub> attribute %s (%d) must be less than value for %s (%d).  using %d", sAttributeStrings[kInGameNthElementAttribute], sHubPreferences.mInGameNthElement, sAttributeStrings[kInGameWindowSizeAttribute], sHubPreferences.mInGameWindowSize, sHubPreferences.mInGameWindowSize - 1);
		
		sHubPreferences.mInGameNthElement = sHubPreferences.mInGameWindowSize - 1;
	}

	return true;
}


static XML_HubConfigurationParser HubConfigurationParser;


XML_ElementParser*
Hub_GetParser() {
	return &HubConfigurationParser;
}



void
WriteHubPreferences(FILE* F)
{
	fprintf(F, "    <hub\n");
	for(size_t i = 0; i < kNumAttributes; i++)
		fprintf(F, "      %s=\"%d\"\n", sAttributeStrings[i], *(sAttributeDestinations[i]));
	fprintf(F, "    />\n");
}



void
DefaultHubPreferences()
{
	sHubPreferences.mPregameWindowSize = kDefaultPregameWindowSize;
	sHubPreferences.mInGameWindowSize = kDefaultInGameWindowSize;
	sHubPreferences.mPregameNthElement = kDefaultPregameNthElement;
	sHubPreferences.mInGameNthElement = kDefaultInGameNthElement;
	sHubPreferences.mPregameTicksBeforeNetDeath = kDefaultPregameTicksBeforeNetDeath;
	sHubPreferences.mInGameTicksBeforeNetDeath = kDefaultInGameTicksBeforeNetDeath;
	sHubPreferences.mRecoverySendPeriod = kDefaultRecoverySendPeriod;
}
