/*
 *  network_star_spoke.cpp

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

 *  Created by Woody Zenfell, III on Fri May 02 2003.
 */

#include "network_star.h"
#include "AStream.h"
#include "mytm.h"
#include "network_private.h" // kPROTOCOL_TYPE
#include "WindowedNthElementFinder.h"
#include "vbl.h" // parse_keymap
#include "Logging.h"

#include <map>

extern void make_player_really_net_dead(size_t inPlayerIndex);


enum {
        kDefaultPregameTicksBeforeNetDeath = 30 * TICKS_PER_SECOND,
        kDefaultInGameTicksBeforeNetDeath = 3 * TICKS_PER_SECOND,
        kDefaultOutgoingFlagsQueueSize = TICKS_PER_SECOND / 2,
        kDefaultRecoverySendPeriod = TICKS_PER_SECOND / 2,
	kDefaultTimingWindowSize = 3 * TICKS_PER_SECOND,
	kDefaultTimingNthElement = kDefaultTimingWindowSize / 2
};

struct SpokePreferences
{
	int32	mPregameTicksBeforeNetDeath;
	int32	mInGameTicksBeforeNetDeath;
//	int32	mOutgoingFlagsQueueSize;
	int32	mRecoverySendPeriod;
	int32	mTimingWindowSize;
	int32	mTimingNthElement;
	bool	mAdjustTiming;
};

static SpokePreferences sSpokePreferences;

static TickBasedActionQueue sOutgoingFlags(kDefaultOutgoingFlagsQueueSize);
static DuplicatingTickBasedCircularQueue<action_flags_t> sLocallyGeneratedFlags;
static int32 sSmallestRealGameTick;

struct IncomingGameDataPacketProcessingContext {
        bool mMessagesDone;
        bool mGotTimingAdjustmentMessage;

        IncomingGameDataPacketProcessingContext() : mMessagesDone(false), mGotTimingAdjustmentMessage(false) {}
};

typedef void (*MessageHandler)(AIStream& s, IncomingGameDataPacketProcessingContext& c);
typedef std::map<uint16, MessageHandler> MessageTypeToMessageHandler;

static MessageTypeToMessageHandler sMessageTypeToMessageHandler;

static int8 sRequestedTimingAdjustment;
static int8 sOutstandingTimingAdjustment;

struct NetworkPlayer_spoke {
        bool				mZombie;
        bool				mConnected;
        int32				mNetDeadTick;
        WritableTickBasedActionQueue* 	mQueue;
};

static vector<NetworkPlayer_spoke> sNetworkPlayers;
static int32 sNetworkTicker;
static int32 sLastNetworkTickHeard;
static int32 sLastNetworkTickSent;
static bool sConnected = false;
static bool sSpokeActive = false;
static myTMTaskPtr sSpokeTickTask = NULL;
static DDPFramePtr sOutgoingFrame = NULL;
static DDPPacketBuffer sLocalOutgoingBuffer;
static bool sNeedToSendLocalOutgoingBuffer = false;
static bool sHubIsLocal = false;
static NetAddrBlock sHubAddress;
static size_t sLocalPlayerIndex;
static int32 sSmallestUnreceivedTick;
static WindowedNthElementFinder<int32> sNthElementFinder(kDefaultTimingWindowSize);
static bool sTimingMeasurementValid;
static int32 sTimingMeasurement;



static void spoke_became_disconnected();
static void spoke_received_game_data_packet_v1(AIStream& ps);
static void process_messages(AIStream& ps, IncomingGameDataPacketProcessingContext& context);
static void handle_end_of_messages_message(AIStream& ps, IncomingGameDataPacketProcessingContext& context);
static void handle_player_net_dead_message(AIStream& ps, IncomingGameDataPacketProcessingContext& context);
static void handle_timing_adjustment_message(AIStream& ps, IncomingGameDataPacketProcessingContext& context);
static void process_optional_message(AIStream& ps, IncomingGameDataPacketProcessingContext& context, uint16 inMessageType);
static bool spoke_tick();
static void send_packet();



static inline bool
operator !=(const NetAddrBlock& a, const NetAddrBlock& b)
{
        return memcmp(&a, &b, sizeof(a)) != 0;
}



static OSErr
send_frame_to_local_hub(DDPFramePtr frame, NetAddrBlock *address, short protocolType, short port)
{
        sLocalOutgoingBuffer.datagramSize = frame->data_size;
        memcpy(sLocalOutgoingBuffer.datagramData, frame->data, frame->data_size);
        sLocalOutgoingBuffer.protocolType = protocolType;
        // An all-0 sourceAddress is the cue for "local spoke" currently.
        obj_clear(sLocalOutgoingBuffer.sourceAddress);
        sNeedToSendLocalOutgoingBuffer = true;
        return noErr;
}



static inline void
check_send_packet_to_hub()
{
        if(sNeedToSendLocalOutgoingBuffer)
	{
		logContext("delivering stored packet to local hub");
                hub_received_network_packet(&sLocalOutgoingBuffer);
	}

        sNeedToSendLocalOutgoingBuffer = false;
}



void
spoke_initialize(const NetAddrBlock& inHubAddress, int32 inFirstTick, size_t inNumberOfPlayers, WritableTickBasedActionQueue* const inPlayerQueues[], bool inPlayerConnected[], size_t inLocalPlayerIndex, bool inHubIsLocal)
{
        assert(inNumberOfPlayers >= 1);
        assert(inLocalPlayerIndex < inNumberOfPlayers);
        assert(inPlayerQueues[inLocalPlayerIndex] != NULL);
        assert(inPlayerConnected[inLocalPlayerIndex]);

        sHubIsLocal = inHubIsLocal;
        sHubAddress = inHubAddress;

        sLocalPlayerIndex = inLocalPlayerIndex;

        sOutgoingFrame = NetDDPNewFrame();

        sSmallestRealGameTick = inFirstTick;
        int32 theFirstPregameTick = inFirstTick - kPregameTicks;
        sOutgoingFlags.reset(theFirstPregameTick);
        sSmallestUnreceivedTick = theFirstPregameTick;
        
        sNetworkPlayers.clear();
        sNetworkPlayers.resize(inNumberOfPlayers);

        sLocallyGeneratedFlags.children().clear();
        sLocallyGeneratedFlags.children().insert(&sOutgoingFlags);
        sLocallyGeneratedFlags.children().insert(inPlayerQueues[inLocalPlayerIndex]);

        for(size_t i = 0; i < inNumberOfPlayers; i++)
        {
                sNetworkPlayers[i].mZombie = (inPlayerQueues[i] == NULL);
                sNetworkPlayers[i].mConnected = inPlayerConnected[i];
                sNetworkPlayers[i].mNetDeadTick = theFirstPregameTick - 1;
                sNetworkPlayers[i].mQueue = inPlayerQueues[i];
                if(sNetworkPlayers[i].mConnected)
                {
                        sNetworkPlayers[i].mQueue->reset(sSmallestRealGameTick);
                }
        }

        sRequestedTimingAdjustment = 0;
        sOutstandingTimingAdjustment = 0;

        sNetworkTicker = 0;
        sLastNetworkTickHeard = 0;
        sLastNetworkTickSent = 0;
        sConnected = true;
	sNthElementFinder.reset(sSpokePreferences.mTimingWindowSize);
	sTimingMeasurementValid = false;

        sMessageTypeToMessageHandler.clear();
        sMessageTypeToMessageHandler[kEndOfMessagesMessageType] = handle_end_of_messages_message;
        sMessageTypeToMessageHandler[kTimingAdjustmentMessageType] = handle_timing_adjustment_message;
        sMessageTypeToMessageHandler[kPlayerNetDeadMessageType] = handle_player_net_dead_message;

        sNeedToSendLocalOutgoingBuffer = false;

        sSpokeActive = true;
        sSpokeTickTask = myXTMSetup(1000/TICKS_PER_SECOND, spoke_tick);
}



void
spoke_cleanup(bool inGraceful)
{
        // Stop processing incoming packets (packet processor won't start processing another packet
        // due to sSpokeActive = false, and we know it's not in the middle of processing one because
        // we take the mutex).
        if(take_mytm_mutex())
        {
		// Mark the tick task for cancellation (it won't start running again after this returns).
		myTMRemove(sSpokeTickTask);
		sSpokeTickTask = NULL;

                sSpokeActive = false;
		send_packet();
		check_send_packet_to_hub();
                release_mytm_mutex();
        }

        // This waits for the tick task to actually finish
        myTMCleanup(true);
        
        sMessageTypeToMessageHandler.clear();
        sNetworkPlayers.clear();
        sLocallyGeneratedFlags.children().clear();
        NetDDPDisposeFrame(sOutgoingFrame);
        sOutgoingFrame = NULL;
}



int32
spoke_get_net_time()
{
	static int32 sPreviousDelay = -1;

	int32 theDelay = (sSpokePreferences.mAdjustTiming && sTimingMeasurementValid) ? sTimingMeasurement : 0;

	if(theDelay != sPreviousDelay)
	{
		logDump1("local delay is now %d", theDelay);
		sPreviousDelay = theDelay;
	}

	return (sConnected ? sOutgoingFlags.getWriteTick() - theDelay : sNetworkPlayers.at(sLocalPlayerIndex).mQueue->getWriteTick());
}



static void
spoke_became_disconnected()
{
        sConnected = false;
        for(size_t i = 0; i < sNetworkPlayers.size(); i++)
        {
                if(sNetworkPlayers[i].mConnected)
                        make_player_really_net_dead(i);
        }
}



void
spoke_received_network_packet(DDPPacketBufferPtr inPacket)
{
	logContext("spoke processing a received packet");
	
        // If we've already given up on the connection, ignore packets.
        if(!sConnected || !sSpokeActive)
                return;
        
        // Ignore packets not from our hub
//        if(inPacket->sourceAddress != sHubAddress)
//                return;

        try {
                AIStreamBE ps(inPacket->datagramData, inPacket->datagramSize);

                uint32 thePacketMagic;
                ps >> thePacketMagic;

                switch(thePacketMagic)
                {
                        case kHubToSpokeGameDataPacketV1Magic:
                                spoke_received_game_data_packet_v1(ps);
                                break;

                        default:
                                // Ignore unknown packet types
				logTrace1("unknown packet magic %x", thePacketMagic);
                                break;
                }
        }
        catch(...)
        {
                // do nothing special, we just ignore the remainder of the packet.
        }
}



static void
spoke_received_game_data_packet_v1(AIStream& ps)
{
        IncomingGameDataPacketProcessingContext context;
        
        // Piggybacked ACK
        int32 theSmallestUnacknowledgedTick;
        ps >> theSmallestUnacknowledgedTick;

        // If ACK is early, ignore the rest of the packet to be safer
        if(theSmallestUnacknowledgedTick > sOutgoingFlags.getWriteTick())
	{
		logTrace2("early ack (%d > %d)", theSmallestUnacknowledgedTick, sOutgoingFlags.getWriteTick());
                return;
	}

        // Heard from hub
        sLastNetworkTickHeard = sNetworkTicker;

        // Remove acknowledged elements from outgoing queue
        for(int tick = sOutgoingFlags.getReadTick(); tick < theSmallestUnacknowledgedTick; tick++)
	{
		logTrace1("dequeueing tick %d from sOutgoingFlags", tick);
                sOutgoingFlags.dequeue();
	}

        // Process messages
        process_messages(ps, context);

        if(!context.mGotTimingAdjustmentMessage)
	{
		if(sRequestedTimingAdjustment != 0)
			logTrace("timing adjustment no longer requested");
		
                sRequestedTimingAdjustment = 0;
	}

        // Action_flags!!!
        // If there aren't any, we're done
        if(ps.tellg() >= ps.maxg())
	{
		// If we are the only connected player, well because of the way the loop below works,
		// we have to enqueue net_dead flags for the other players now, up to match the most
		// recent tick the hub has acknowledged.
		// We can't assume we are the only connected player merely by virtue of there being no
		// flags in the packet.  The hub could be waiting on somebody else to send and is
		// essentially sending us "keep-alive" packets (which could contain no new flags) in the meantime.
		// In other words, (we are alone) implies (no flags in packet), but not the converse.
		// Here "alone" means there are no other players who are connected or who will be netdead
		// sometime in the future.  (If their NetDeadTick is greater than the ACKed tick, we expect
		// that the hub will be sending actual flags in the future to make up the difference.)
		bool weAreAlone = true;
		int32 theSmallestUnacknowledgedTick = sOutgoingFlags.getReadTick();
		for(size_t i = 0; i < sNetworkPlayers.size(); i++)
		{
			if(i != sLocalPlayerIndex && (sNetworkPlayers[i].mConnected || sNetworkPlayers[i].mNetDeadTick > theSmallestUnacknowledgedTick))
			{
				weAreAlone = false;
				break;
			}
		}
		
		if(weAreAlone)
		{
			logContext("handling special \"we are alone\" case");
			
			for(size_t i = 0; i < sNetworkPlayers.size(); i++)
			{
				NetworkPlayer_spoke& thePlayer = sNetworkPlayers[i];
				if(i != sLocalPlayerIndex && !thePlayer.mZombie)
				{
					while(thePlayer.mQueue->getWriteTick() < theSmallestUnacknowledgedTick)
					{
						logDump2("enqueued NET_DEAD_ACTION_FLAG for player %d tick %d", i, thePlayer.mQueue->getWriteTick());
						thePlayer.mQueue->enqueue(NET_DEAD_ACTION_FLAG);
					}
				}
			}

			sSmallestUnreceivedTick = theSmallestUnacknowledgedTick;
			logDump1("sSmallestUnreceivedTick is now %d", sSmallestUnreceivedTick);
		}
		
                return;
	} // no data left in packet

        int32 theSmallestUnreadTick;
        ps >> theSmallestUnreadTick;

        // Can't accept packets that skip ticks
        if(theSmallestUnreadTick > sSmallestUnreceivedTick)
	{
		logTrace2("early flags (%d > %d)", theSmallestUnreadTick, sSmallestUnreceivedTick);
                return;
	}

        // Figure out how many ticks of flags we can actually enqueue
        // We want to stock all queues evenly, since we ACK everyone's flags for a tick together.
        int theSmallestQueueSpace = INT_MAX;
        for(size_t i = 0; i < sNetworkPlayers.size(); i++)
        {
                if(sNetworkPlayers[i].mZombie || i == sLocalPlayerIndex)
                        continue;

                int theQueueSpace = sNetworkPlayers[i].mQueue->availableCapacity();

                /*
                        hmm, taking this exemption out, because we will start enqueueing PLAYER_NET_DEAD_FLAG onto the queue.
                // If player is netdead or will become netdead before queue fills,
                // player's queue space will not limit us
                if(!sNetworkPlayers[i].mConnected)
                {
                        int theRemainingLiveTicks = sNetworkPlayers[i].mNetDeadTick - sSmallestUnreceivedTick;
                        if(theRemainingLiveTicks < theQueueSpace)
                                continue;
                }
                 */

                if(theQueueSpace < theSmallestQueueSpace)
                        theSmallestQueueSpace = theQueueSpace;
        }

	logDump1("%d queue space available", theSmallestQueueSpace);

        // Read and enqueue the actual action_flags from the packet
        // The body of this loop is a bit more convoluted than you might
        // expect, because the same loop is used to skip already-seen action_flags
        // and to enqueue new ones.
	while(ps.tellg() < ps.maxg())
        {
                // If we've no room to enqueue stuff, no point in finishing reading the packet.
                if(theSmallestQueueSpace <= 0)
                        break;
                
                for(size_t i = 0; i < sNetworkPlayers.size(); i++)
                {
                        // Our own flags are not sent back to us; we'll never get flags for zombies.
                        // We are not expected to enqueue flags in either case.
                        if(sNetworkPlayers[i].mZombie || i == sLocalPlayerIndex)
                                continue;

                        bool shouldEnqueueNetDeadFlags = false;

                        // We won't get flags for netdead players
                        NetworkPlayer_spoke& thePlayer = sNetworkPlayers[i];
                        if(!thePlayer.mConnected)
                        {
                                if(thePlayer.mNetDeadTick < theSmallestUnreadTick)
                                        shouldEnqueueNetDeadFlags = true;

                                if(thePlayer.mNetDeadTick == theSmallestUnreadTick)
                                {
                                        // Only actually act if this tick is new to us
                                        if(theSmallestUnreadTick == sSmallestUnreceivedTick)
                                                make_player_really_net_dead(i);
                                        shouldEnqueueNetDeadFlags = true;
                                }
                        }

                        // Decide what flags are appropriate for this player this tick
                        action_flags_t theFlags;
                        if(shouldEnqueueNetDeadFlags)
                                // We effectively generate a tick's worth of flags in lieu of reading it from the packet.
                                theFlags = NET_DEAD_ACTION_FLAG;
                        else
                                // We should have a flag for this player for this tick!
                                ps >> theFlags;

                        // Now, we've gotten flags, probably from the packet... should we enqueue them?
                        if(theSmallestUnreadTick == sSmallestUnreceivedTick)
                        {
				if(theSmallestUnreadTick >= sSmallestRealGameTick)
				{
					WritableTickBasedActionQueue& theQueue = *(sNetworkPlayers[i].mQueue);
					assert(theQueue.getWriteTick() == sSmallestUnreceivedTick);
					assert(theQueue.availableCapacity() > 0);
					logTrace3("enqueueing flags %x for player %d tick %d", theFlags, i, theQueue.getWriteTick());
					theQueue.enqueue(theFlags);
				}
                        }

                } // iterate over players

		theSmallestUnreadTick++;
		if(sSmallestUnreceivedTick < theSmallestUnreadTick)
		{
			theSmallestQueueSpace--;
			sSmallestUnreceivedTick = theSmallestUnreadTick;

			int32 theLatencyMeasurement = sOutgoingFlags.getWriteTick() - sSmallestUnreceivedTick;
			logDump1("latency measurement: %d", theLatencyMeasurement);
			sNthElementFinder.insert(theLatencyMeasurement);
			// We capture these values here so we don't have to take a lock in GetNetTime.
			sTimingMeasurementValid = sNthElementFinder.window_full();
			if(sTimingMeasurementValid)
				sTimingMeasurement = sNthElementFinder.nth_largest_element(sSpokePreferences.mTimingNthElement);
		}

	} // loop while there's packet data left

}



static void
process_messages(AIStream& ps, IncomingGameDataPacketProcessingContext& context)
{
        while(!context.mMessagesDone)
        {
                uint16 theMessageType;
                ps >> theMessageType;

                MessageTypeToMessageHandler::iterator i = sMessageTypeToMessageHandler.find(theMessageType);

                if(i == sMessageTypeToMessageHandler.end())
                        process_optional_message(ps, context, theMessageType);
                else
                        i->second(ps, context);
        }
}



static void
handle_end_of_messages_message(AIStream& ps, IncomingGameDataPacketProcessingContext& context)
{
        context.mMessagesDone = true;
}



static void
handle_player_net_dead_message(AIStream& ps, IncomingGameDataPacketProcessingContext& context)
{
        uint8 thePlayerIndex;
        int32 theTick;

        ps >> thePlayerIndex >> theTick;

        if(thePlayerIndex > sNetworkPlayers.size())
                return;

        sNetworkPlayers[thePlayerIndex].mConnected = false;
        sNetworkPlayers[thePlayerIndex].mNetDeadTick = theTick;

	logDump2("netDead message: player %d in tick %d", thePlayerIndex, theTick);
}



static void
handle_timing_adjustment_message(AIStream& ps, IncomingGameDataPacketProcessingContext& context)
{
        int8 theAdjustment;

        ps >> theAdjustment;

        if(theAdjustment != sRequestedTimingAdjustment)
        {
                sOutstandingTimingAdjustment = theAdjustment;
                sRequestedTimingAdjustment = theAdjustment;
		logTrace2("new timing adjustment message; requested: %d outstanding: %d", sRequestedTimingAdjustment, sOutstandingTimingAdjustment);
        }

        context.mGotTimingAdjustmentMessage = true;
}



static void
process_optional_message(AIStream& ps, IncomingGameDataPacketProcessingContext& context, uint16 inMessageType)
{
        // We don't know of any optional messages, so we just skip any we encounter.
        // (All optional messages are required to encode their length (not including the
        // space required for the message type or length) in the two bytes immediately
        // following the message type.)
        uint16 theLength;
        ps >> theLength;

        ps.ignore(theLength);
}



static bool
spoke_tick()
{
	logContext1("processing spoke_tick %d", sNetworkTicker);
	
        sNetworkTicker++;

        if(sConnected)
        {
                int32 theSilentTicksBeforeNetDeath = (sOutgoingFlags.getReadTick() >= sSmallestRealGameTick) ? sSpokePreferences.mInGameTicksBeforeNetDeath : sSpokePreferences.mPregameTicksBeforeNetDeath;
        
                if(sNetworkTicker - sLastNetworkTickHeard > theSilentTicksBeforeNetDeath)
                {
			logTrace("giving up on hub; disconnecting");
                        spoke_became_disconnected();
                        return true;
                }
        }

        bool shouldSend = false;

        // Negative timing adjustment means we need to provide extra ticks because we're late.
        // We let this cover the normal timing adjustment = 0 case too.
        if(sOutstandingTimingAdjustment <= 0)
        {
                int theNumberOfFlagsToProvide = -sOutstandingTimingAdjustment + 1;

		logDump1("want to provide %d flags", theNumberOfFlagsToProvide);

                // If we're not connected, write only to our local game's queue.
                // If we are connected,
                //	if we're actually in the game, write to both local game and outbound flags queues
                //	else (if pregame), write only to the outbound flags queue.
                WritableTickBasedActionQueue& theTargetQueue =
                        sConnected ? ((sOutgoingFlags.getWriteTick() >= sSmallestRealGameTick) ? static_cast<WritableTickBasedActionQueue&>(sLocallyGeneratedFlags) : static_cast<WritableTickBasedActionQueue&>(sOutgoingFlags))
                                   : *(sNetworkPlayers[sLocalPlayerIndex].mQueue);

                while(theNumberOfFlagsToProvide > 0)
                {
                        if(theTargetQueue.availableCapacity() <= 0)
                                break;

			logDump1("enqueueing flags for tick %d", theTargetQueue.getWriteTick());

                        theTargetQueue.enqueue(parse_keymap());
                        shouldSend = true;
                        theNumberOfFlagsToProvide--;
                }
		
		// Prevent creeping timing adjustment during "lulls"; OTOH remember to
		// finish next time if we made progress but couldn't complete our obligation.
		if(theNumberOfFlagsToProvide != -sOutstandingTimingAdjustment + 1)
			sOutstandingTimingAdjustment = -theNumberOfFlagsToProvide;
	}
        // Positive timing adjustment means we should delay sending for a while,
        // so we just throw away this local tick.
        else
	{
		logDump("ignoring this tick for timing adjustment"); 
                sOutstandingTimingAdjustment--;
	}

	logDump1("sOutstandingTimingAdjustment is now %d", sOutstandingTimingAdjustment);

        // If we're connected and (we generated new data or if it's been long enough since we last sent), send.
        if(sConnected)
	{
		if(shouldSend || (sNetworkTicker - sLastNetworkTickSent) >= sSpokePreferences.mRecoverySendPeriod)
			send_packet();
	}
	else
	{
		int32 theLocalPlayerWriteTick = sNetworkPlayers.at(sLocalPlayerIndex).mQueue->getWriteTick();

		// Since we're not connected, we won't be enqueueing flags for the other players in the packet handler.
		// So, we do it here to keep the game moving.
		for(size_t i = 0; i < sNetworkPlayers.size(); i++)
		{
			if(i == sLocalPlayerIndex)
				continue;
			
			NetworkPlayer_spoke& thePlayer = sNetworkPlayers.at(i);
			
			if(!thePlayer.mZombie)
			{
				while(thePlayer.mQueue->getWriteTick() < theLocalPlayerWriteTick)
				{
					logDump2("enqueueing NET_DEAD_ACTION_FLAG for player %d tick %d", i, thePlayer.mQueue->getWriteTick());
					thePlayer.mQueue->enqueue(NET_DEAD_ACTION_FLAG);
				}
			}
		}
	}

        check_send_packet_to_hub();

        // We want to run again.
        return true;
}



static void
send_packet()
{
        try {
                AOStreamBE ps(sOutgoingFrame->data, ddpMaxData);
        
                // Packet magic
                ps << (uint32)kSpokeToHubGameDataPacketV1Magic;
        
                // Acknowledgement
                ps << sSmallestUnreceivedTick;
        
                // Messages
                // No more messages
                ps << (uint16)kEndOfMessagesMessageType;
        
                // Action_flags!!!
                if(sOutgoingFlags.size() > 0)
                {
                        ps << sOutgoingFlags.getReadTick();
                        for(int32 tick = sOutgoingFlags.getReadTick(); tick < sOutgoingFlags.getWriteTick(); tick++)
                                ps << sOutgoingFlags.peek(tick);
                }

		logDump3("preparing to send packet: ACK %d, flags [%d,%d)", sSmallestUnreceivedTick, sOutgoingFlags.getReadTick(), sOutgoingFlags.getWriteTick());

                // Send the packet
                sOutgoingFrame->data_size = ps.tellp();
                if(sHubIsLocal)
                        send_frame_to_local_hub(sOutgoingFrame, &sHubAddress, kPROTOCOL_TYPE, 0 /* ignored */);
                else
                        NetDDPSendFrame(sOutgoingFrame, &sHubAddress, kPROTOCOL_TYPE, 0 /* ignored */);

                sLastNetworkTickSent = sNetworkTicker;
        }
        catch (...) {
        }
}



static inline const char *BoolString(bool B) {return (B ? "true" : "false");}

enum {
	kPregameTicksBeforeNetDeathAttribute,
	kInGameTicksBeforeNetDeathAttribute,
	// kOutgoingFlagsQueueSizeAttribute,
	kRecoverySendPeriodAttribute,
	kTimingWindowSizeAttribute,
	kTimingNthElementAttribute,
	kNumInt32Attributes,
	kAdjustTimingAttribute = kNumInt32Attributes,
	kNumAttributes
};

static const char* sAttributeStrings[kNumInt32Attributes] =
{
	"pregame_ticks_before_net_death",
	"ingame_ticks_before_net_death",
//	"outgoing_flags_queue_size",
	"recovery_send_period",
	"timing_window_size",
	"timing_nth_element"
};

static int32* sAttributeDestinations[kNumInt32Attributes] =
{
	&sSpokePreferences.mPregameTicksBeforeNetDeath,
	&sSpokePreferences.mInGameTicksBeforeNetDeath,
//	&sSpokePreferences.mOutgoingFlagsQueueSize,
	&sSpokePreferences.mRecoverySendPeriod,
	&sSpokePreferences.mTimingWindowSize,
	&sSpokePreferences.mTimingNthElement
};

class XML_SpokeConfigurationParser: public XML_ElementParser
{
public:
	bool Start();
	bool HandleAttribute(const char *Tag, const char *Value);
	bool AttributesDone();

	XML_SpokeConfigurationParser(): XML_ElementParser("spoke") {}

protected:
	bool	mAttributePresent[kNumAttributes];
	int32	mAttribute[kNumInt32Attributes];
	bool	mAdjustTiming;
};

bool XML_SpokeConfigurationParser::Start()
{
        for(int i = 0; i < kNumAttributes; i++)
                mAttributePresent[i] = false;

	return true;
}

static const char* sAttributeMultiplySpecifiedString = "attribute multiply specified";

bool XML_SpokeConfigurationParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (StringsEqual(Tag,"adjust_timing"))
	{
                if(!mAttributePresent[kAdjustTimingAttribute]) {
                        if(ReadBooleanValueAsBool(Value,mAdjustTiming)) {
                                mAttributePresent[kAdjustTimingAttribute] = true;
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

	else
	{
		for(size_t i = 0; i < kNumInt32Attributes; i++)
		{
			if(strcasecmp(Tag,sAttributeStrings[i]) == 0)
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
	}

	UnrecognizedTag();
	return false;
}

bool XML_SpokeConfigurationParser::AttributesDone() {
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
				case kTimingWindowSizeAttribute:
					if(mAttribute[i] < 1)
					{
						// I don't know whether this actually does anything if I don't return false,
						// but I'd like to honor the user's wishes as far as I can without just throwing
						// up my hands.
						BadNumericalValue();
						logWarning3("improper value %d for attribute %s of <spoke>; must be at least 1.  using default of %d", mAttribute[i], sAttributeStrings[i], *(sAttributeDestinations[i]));
						mAttributePresent[i] = false;
					}
					else
					{
						*(sAttributeDestinations[i]) = mAttribute[i];
					}
					break;

				case kTimingNthElementAttribute:
					if(mAttribute[i] < 0 || mAttribute[i] >= *(sAttributeDestinations[kTimingWindowSizeAttribute]))
					{
						BadNumericalValue();
						logWarning4("improper value %d for attribute %s of <spoke>; must be at least 0 but less than %s.  using default of %d", mAttribute[i], sAttributeStrings[i], sAttributeStrings[kTimingWindowSizeAttribute], *(sAttributeDestinations[i]));
						mAttributePresent[i] = false;
					}
					else
					{
						*(sAttributeDestinations[i]) = mAttribute[i];
					}
					break;

				case kAdjustTimingAttribute:
					sSpokePreferences.mAdjustTiming = mAdjustTiming;
					break;

				default:
					assert(false);
					break;
			} // switch(attribute)
			
		} // if attribute present
		
	} // loop over attributes

	// The checks above are not sufficient to catch all bad cases; if user specified a window size
	// smaller than default, this is our only chance to deal with it.
	if(sSpokePreferences.mTimingNthElement >= sSpokePreferences.mTimingWindowSize)
	{
		logWarning5("value for <spoke> attribute %s (%d) must be less than value for %s (%d).  using %d", sAttributeStrings[kTimingNthElementAttribute], mAttribute[kTimingNthElementAttribute], sAttributeStrings[kTimingWindowSizeAttribute], mAttribute[kTimingWindowSizeAttribute], sSpokePreferences.mTimingWindowSize - 1);
			
		sSpokePreferences.mTimingNthElement = sSpokePreferences.mTimingWindowSize - 1;
	}

        return true;
}


static XML_SpokeConfigurationParser SpokeConfigurationParser;


XML_ElementParser*
Spoke_GetParser() {
	return &SpokeConfigurationParser;
}



void
WriteSpokePreferences(FILE* F)
{
	fprintf(F, "    <spoke\n");
	for(size_t i = 0; i < kNumInt32Attributes; i++)
		fprintf(F, "      %s=\"%d\"\n", sAttributeStrings[i], *(sAttributeDestinations[i]));
	fprintf(F, "      adjust_timing=\"%s\"\n", BoolString(sSpokePreferences.mAdjustTiming));
	fprintf(F, "    />\n");
}



void
DefaultSpokePreferences()
{
	sSpokePreferences.mPregameTicksBeforeNetDeath = kDefaultPregameTicksBeforeNetDeath;
	sSpokePreferences.mInGameTicksBeforeNetDeath = kDefaultInGameTicksBeforeNetDeath;
	//	sSpokePreferences.mOutgoingFlagsQueueSize = kDefaultOutgoingFlagsQueueSize;
	sSpokePreferences.mRecoverySendPeriod = kDefaultRecoverySendPeriod;
	sSpokePreferences.mTimingWindowSize = kDefaultTimingWindowSize;
	sSpokePreferences.mTimingNthElement = kDefaultTimingNthElement;
	sSpokePreferences.mAdjustTiming = true;
}
