/*
 *  network_star_spoke.cpp

	Copyright (C) 2003 and beyond by Woody Zenfell, III
	and the "Aleph One" developers.

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	This license is contained in the file "COPYING",
	which is included with this source code; it is available online at
	http://www.gnu.org/licenses/gpl.html

 *  The portion of the star game protocol run on every player's machine.
 * 
 *  Created by Woody Zenfell, III on Fri May 02 2003.
 *
 *  May 27, 2003 (Woody Zenfell): lossy byte-stream distribution.
 *
 *  June 30, 2003 (Woody Zenfell): lossy byte-stream distribution more tolerant of scheduling jitter
 *	(i.e. will queue multiple chunks before send, instead of dropping all data but most recent)
 *
 *  September 17, 2004 (jkvw):
 *	NAT-friendly networking - we no longer get spoke addresses form topology -
 *	instead spokes send identification packets to hub with player ID.
 *	Hub can then associate the ID in the identification packet with the paket's source address.
 */

#if !defined(DISABLE_NETWORKING)

#include "network_star.h"
#include "AStream.h"
#include "mytm.h"
#include "network_private.h" // kPROTOCOL_TYPE
#include "WindowedNthElementFinder.h"
#include "vbl.h" // parse_keymap
#include "CircularByteBuffer.h"
#include "Logging.h"
#include "crc.h"
#include "player.h"
#include "InfoTree.h"

#include <map>

extern void make_player_really_net_dead(size_t inPlayerIndex);
extern void call_distribution_response_function_if_available(byte* inBuffer, uint16 inBufferSize, int16 inDistributionType, uint8 inSendingPlayerIndex);


enum {
        kDefaultPregameTicksBeforeNetDeath = 90 * TICKS_PER_SECOND,
        kDefaultInGameTicksBeforeNetDeath = 5 * TICKS_PER_SECOND,
        kDefaultOutgoingFlagsQueueSize = TICKS_PER_SECOND / 2,
        kDefaultRecoverySendPeriod = TICKS_PER_SECOND / 2,
	kDefaultTimingWindowSize = 3 * TICKS_PER_SECOND,
	kDefaultTimingNthElement = kDefaultTimingWindowSize / 2,
	kLossyByteStreamDataBufferSize = 1280,
	kTypicalLossyByteStreamChunkSize = 56,
	kLossyByteStreamDescriptorCount = kLossyByteStreamDataBufferSize / kTypicalLossyByteStreamChunkSize
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
static TickBasedActionQueue sUnconfirmedFlags(kDefaultOutgoingFlagsQueueSize);
static DuplicatingTickBasedCircularQueue<action_flags_t> sLocallyGeneratedFlags;
static int32 sSmallestRealGameTick;

struct IncomingGameDataPacketProcessingContext {
        bool mMessagesDone;
        bool mGotTimingAdjustmentMessage;

        IncomingGameDataPacketProcessingContext() : mMessagesDone(false), mGotTimingAdjustmentMessage(false) {}
};

typedef void (*StarMessageHandler)(AIStream& s, IncomingGameDataPacketProcessingContext& c);
typedef std::map<uint16, StarMessageHandler> MessageTypeToMessageHandler;

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
static bool sHeardFromHub = false;
static bool sWorldUpdate = false;

static vector<int32> sDisplayLatencyBuffer; // stores the last 30 latency calculations, in ticks
static uint32 sDisplayLatencyCount = 0;
static int32 sDisplayLatencyTicks = 0; // sum of the latency ticks from the last 30 seconds, using above two

static int32 sSmallestUnconfirmedTick;

struct SpokeLossyByteStreamChunkDescriptor
{
	uint16	mLength;
	int16	mType;
	uint32	mDestinations;
};

// This holds outgoing lossy byte stream data
static CircularByteBuffer sOutgoingLossyByteStreamData(kLossyByteStreamDataBufferSize);

// This holds a descriptor for each chunk of lossy byte stream data held in the above buffer
static CircularQueue<SpokeLossyByteStreamChunkDescriptor> sOutgoingLossyByteStreamDescriptors(kLossyByteStreamDescriptorCount);

// This is currently used only to hold incoming streaming data until it's passed to the upper-level code
static byte sScratchBuffer[kLossyByteStreamDataBufferSize];


static void spoke_became_disconnected();
static void spoke_received_game_data_packet_v1(AIStream& ps, bool reflected_flags);
static void spoke_received_ping_request(AIStream& ps, NetAddrBlock address);
static void spoke_received_ping_response(AIStream& ps, NetAddrBlock address);
static void process_messages(AIStream& ps, IncomingGameDataPacketProcessingContext& context);
static void handle_end_of_messages_message(AIStream& ps, IncomingGameDataPacketProcessingContext& context);
static void handle_player_net_dead_message(AIStream& ps, IncomingGameDataPacketProcessingContext& context);
static void handle_timing_adjustment_message(AIStream& ps, IncomingGameDataPacketProcessingContext& context);
static void handle_lossy_byte_stream_message(AIStream& ps, IncomingGameDataPacketProcessingContext& context);
static void process_optional_message(AIStream& ps, IncomingGameDataPacketProcessingContext& context, uint16 inMessageType);
static bool spoke_tick();
static void send_packet();
static void send_identification_packet();


static inline NetworkPlayer_spoke&
getNetworkPlayer(size_t inIndex)
{
        assert(inIndex < sNetworkPlayers.size());
        return sNetworkPlayers[inIndex];
}



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
		logContextNMT("delivering stored packet to local hub");
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
	sUnconfirmedFlags.reset(sSmallestRealGameTick);
	sSmallestUnconfirmedTick = sSmallestRealGameTick;
        sSmallestUnreceivedTick = theFirstPregameTick;
        
        sNetworkPlayers.clear();
        sNetworkPlayers.resize(inNumberOfPlayers);

        sLocallyGeneratedFlags.children().clear();
        sLocallyGeneratedFlags.children().insert(&sOutgoingFlags);
	sLocallyGeneratedFlags.children().insert(&sUnconfirmedFlags);

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
		sWorldUpdate = false;
        sLastNetworkTickHeard = 0;
        sLastNetworkTickSent = 0;
        sConnected = true;
	sNthElementFinder.reset(sSpokePreferences.mTimingWindowSize);
	sTimingMeasurementValid = false;

	sOutgoingLossyByteStreamDescriptors.reset();
	sOutgoingLossyByteStreamData.reset();

        sMessageTypeToMessageHandler.clear();
        sMessageTypeToMessageHandler[kEndOfMessagesMessageType] = handle_end_of_messages_message;
        sMessageTypeToMessageHandler[kTimingAdjustmentMessageType] = handle_timing_adjustment_message;
        sMessageTypeToMessageHandler[kPlayerNetDeadMessageType] = handle_player_net_dead_message;
	sMessageTypeToMessageHandler[kHubToSpokeLossyByteStreamMessageType] = handle_lossy_byte_stream_message;

        sNeedToSendLocalOutgoingBuffer = false;

        sSpokeActive = true;
        sSpokeTickTask = myXTMSetup(1000/TICKS_PER_SECOND, spoke_tick);

	sDisplayLatencyBuffer.resize(TICKS_PER_SECOND, 0);
	sDisplayLatencyCount = 0;
	sDisplayLatencyTicks = 0;
	
	sHeardFromHub = false;
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

		// We send one last packet here to try to not leave the hub hanging on our ACK.
		send_packet();
		check_send_packet_to_hub();

		release_mytm_mutex();
        }

        // This waits for the tick task to actually finish
        myTMCleanup(true);
        
        sMessageTypeToMessageHandler.clear();
        sNetworkPlayers.clear();
        sLocallyGeneratedFlags.children().clear();
	sDisplayLatencyBuffer.clear();
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
		logDump("local delay is now %d", theDelay);
		sPreviousDelay = theDelay;
	}

	return (sConnected ? sOutgoingFlags.getWriteTick() - theDelay : getNetworkPlayer(sLocalPlayerIndex).mQueue->getWriteTick());
}



void
spoke_distribute_lossy_streaming_bytes_to_everyone(int16 inDistributionType, byte* inBytes, uint16 inLength, bool inExcludeLocalPlayer, bool onlySendToTeam)
{

	int16 local_team;
	if (onlySendToTeam)
	{
		player_info* player = (player_info *)NetGetPlayerData(sLocalPlayerIndex);
		local_team = player->team;
	}

	uint32 theDestinations = 0;
	for(size_t i = 0; i < sNetworkPlayers.size(); i++)
	{
		if((i != sLocalPlayerIndex || !inExcludeLocalPlayer) && !sNetworkPlayers[i].mZombie && sNetworkPlayers[i].mConnected)
		{
			if (onlySendToTeam)
			{
				player_info* player = (player_info *)NetGetPlayerData(i);
				if (player->team == local_team) 
					theDestinations |= (((uint32)1) << i);
				
			}
			else
			{
				theDestinations |= (((uint32)1) << i);
			}
		}
	}

	spoke_distribute_lossy_streaming_bytes(inDistributionType, theDestinations, inBytes, inLength);
}



void
spoke_distribute_lossy_streaming_bytes(int16 inDistributionType, uint32 inDestinationsBitmask, byte* inBytes, uint16 inLength)
{
	if(inLength > sOutgoingLossyByteStreamData.getRemainingSpace())
	{
		logNoteNMT("spoke has insufficient buffer space for %hu bytes of outgoing lossy streaming type %hd; discarded", inLength, inDistributionType);
		return;
	}

	if(sOutgoingLossyByteStreamDescriptors.getRemainingSpace() < 1)
	{
		logNoteNMT("spoke has exhausted descriptor buffer space; discarding %hu bytes of outgoing lossy streaming type %hd", inLength, inDistributionType);
		return;
	}
	
	struct SpokeLossyByteStreamChunkDescriptor theDescriptor;
	theDescriptor.mLength = inLength;
	theDescriptor.mDestinations = inDestinationsBitmask;
	theDescriptor.mType = inDistributionType;

	logDumpNMT("spoke application decided to send %d bytes of lossy streaming type %d destined for players 0x%x", inLength, inDistributionType, inDestinationsBitmask);
	
	sOutgoingLossyByteStreamData.enqueueBytes(inBytes, inLength);
	sOutgoingLossyByteStreamDescriptors.enqueue(theDescriptor);
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
	logContextNMT("spoke processing a received packet");
	
        // Ignore packets not from our hub
//        if(inPacket->sourceAddress != sHubAddress)
//                return;

        try {
                AIStreamBE ps(inPacket->datagramData, inPacket->datagramSize);

		uint16 thePacketMagic;
		ps >> thePacketMagic;
			
		// If we've already given up on the connection, ignore non-ping packets.
		if((!sConnected || !sSpokeActive) &&
		   thePacketMagic != kPingRequestPacket &&
		   thePacketMagic != kPingResponsePacket)
			return;

		uint16 thePacketCRC;
		ps >> thePacketCRC;

		// blank out the CRC field before calculating
		inPacket->datagramData[2] = 0;
		inPacket->datagramData[3] = 0;

		if (thePacketCRC != calculate_data_crc_ccitt(inPacket->datagramData, inPacket->datagramSize))
		{
			logWarningNMT("CRC failure; discarding packet type %i", thePacketMagic);
			return;
		}
		
                switch(thePacketMagic)
                {
		case kHubToSpokeGameDataPacketV1Magic:
			spoke_received_game_data_packet_v1(ps, false);
			break;

		case kHubToSpokeGameDataPacketWithSpokeFlagsV1Magic:
			spoke_received_game_data_packet_v1(ps, true);
			break;
		
		case kPingRequestPacket:
			spoke_received_ping_request(ps, inPacket->sourceAddress);
			break;
		
		case kPingResponsePacket:
			spoke_received_ping_response(ps, inPacket->sourceAddress);
			break;
		
		default:
			// Ignore unknown packet types
			logTraceNMT("unknown packet type %i", thePacketMagic);
			break;
                }
        }
        catch(...)
        {
                // do nothing special, we just ignore the remainder of the packet.
        }
}



static void
spoke_received_game_data_packet_v1(AIStream& ps, bool reflected_flags)
{
	sHeardFromHub = true;

        IncomingGameDataPacketProcessingContext context;
        
        // Piggybacked ACK
        int32 theSmallestUnacknowledgedTick;
        ps >> theSmallestUnacknowledgedTick;

	// we can get an early ACK only if the server made up flags for us...
	if (theSmallestUnacknowledgedTick > sOutgoingFlags.getWriteTick())
	{
		if (reflected_flags) 
		{
			theSmallestUnacknowledgedTick = sOutgoingFlags.getWriteTick();
		}
		else
		{
			logTraceNMT("early ack (%d > %d)", theSmallestUnacknowledgedTick, sOutgoingFlags.getWriteTick());
			return;
		}
	}


        // Heard from hub
        sLastNetworkTickHeard = sNetworkTicker;

        // Remove acknowledged elements from outgoing queue
        for(int tick = sOutgoingFlags.getReadTick(); tick < theSmallestUnacknowledgedTick; tick++)
	{
		logTraceNMT("dequeueing tick %d from sOutgoingFlags", tick);
                sOutgoingFlags.dequeue();
	}

        // Process messages
        process_messages(ps, context);

        if(!context.mGotTimingAdjustmentMessage)
	{
		if(sRequestedTimingAdjustment != 0)
			logTraceNMT("timing adjustment no longer requested");
		
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
			logContextNMT("handling special \"we are alone\" case");
			
			for(size_t i = 0; i < sNetworkPlayers.size(); i++)
			{
				NetworkPlayer_spoke& thePlayer = sNetworkPlayers[i];
				if (i == sLocalPlayerIndex)
				{
					while (sSmallestUnconfirmedTick < sUnconfirmedFlags.getWriteTick())
					{
						sNetworkPlayers[i].mQueue->enqueue(sUnconfirmedFlags.peek(sSmallestUnconfirmedTick++));
					}
				} 
				else if (!thePlayer.mZombie)
				{
					while(thePlayer.mQueue->getWriteTick() < theSmallestUnacknowledgedTick)
					{
						logDumpNMT("enqueued NET_DEAD_ACTION_FLAG for player %d tick %d", i, thePlayer.mQueue->getWriteTick());
						thePlayer.mQueue->enqueue(static_cast<action_flags_t>(NET_DEAD_ACTION_FLAG));
					}
				}
			}

			sSmallestUnreceivedTick = theSmallestUnacknowledgedTick;
			logDumpNMT("sSmallestUnreceivedTick is now %d", sSmallestUnreceivedTick);
		}
		
                return;
	} // no data left in packet

        int32 theSmallestUnreadTick;
        ps >> theSmallestUnreadTick;

        // Can't accept packets that skip ticks
        if(theSmallestUnreadTick > sSmallestUnreceivedTick)
	{
		logTraceNMT("early flags (%d > %d)", theSmallestUnreadTick, sSmallestUnreceivedTick);
                return;
	}

        // Figure out how many ticks of flags we can actually enqueue
        // We want to stock all queues evenly, since we ACK everyone's flags for a tick together.
        int theSmallestQueueSpace = INT_MAX;
        for(size_t i = 0; i < sNetworkPlayers.size(); i++)
        {
		// we'll never get flags for zombies, and we're not expected 
		// to enqueue flags for zombies
                if(sNetworkPlayers[i].mZombie)
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

	logDumpNMT("%d queue space available", theSmallestQueueSpace);

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

			// We'll never get flags for zombies
			if (sNetworkPlayers[i].mZombie)
				continue;

			// if our own flags are not sent back to us,
			// confirm the ones we have in our unconfirmed queue,
			// and do not read any from the packet
			if (i == sLocalPlayerIndex && !reflected_flags)
			{
				if (theSmallestUnreadTick == sSmallestUnreceivedTick && theSmallestUnreadTick >= sSmallestRealGameTick)
				{
					assert(sNetworkPlayers[i].mQueue->getWriteTick() == sSmallestUnconfirmedTick);
					assert(sSmallestUnconfirmedTick >= sUnconfirmedFlags.getReadTick());
					assert(sSmallestUnconfirmedTick < sUnconfirmedFlags.getWriteTick());
					// confirm this flag
					sNetworkPlayers[i].mQueue->enqueue(sUnconfirmedFlags.peek(sSmallestUnconfirmedTick));
					sSmallestUnconfirmedTick++;
				}
				
				continue;
			}

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
                                theFlags = static_cast<action_flags_t>(NET_DEAD_ACTION_FLAG);
                        else
			{
                                // We should have a flag for this player for this tick!
				try 
				{
					ps >> theFlags;
				}
				catch (const AStream::failure& f)
				{
					logWarningNMT("AStream exception (%s) for player %i at theSmallestUnreadTick %i! OOS is likely!\n", f.what(), i, theSmallestUnreadTick);
					return;
				}
			}


                        // Now, we've gotten flags, probably from the packet... should we enqueue them?
                        if(theSmallestUnreadTick == sSmallestUnreceivedTick)
                        {
				if(theSmallestUnreadTick >= sSmallestRealGameTick)
				{
					WritableTickBasedActionQueue& theQueue = *(sNetworkPlayers[i].mQueue);
					assert(theQueue.getWriteTick() == sSmallestUnreceivedTick);
					assert(theQueue.availableCapacity() > 0);
					logTraceNMT("enqueueing flags %x for player %d tick %d", theFlags, i, theQueue.getWriteTick());
					theQueue.enqueue(theFlags);
					if (i == sLocalPlayerIndex) sSmallestUnconfirmedTick++;
				}
                        }

                } // iterate over players

		theSmallestUnreadTick++;
		if(sSmallestUnreceivedTick < theSmallestUnreadTick)
		{
			theSmallestQueueSpace--;
			sSmallestUnreceivedTick = theSmallestUnreadTick;

			int32 theLatencyMeasurement = sOutgoingFlags.getWriteTick() - sSmallestUnreceivedTick;
			logDumpNMT("latency measurement: %d", theLatencyMeasurement);

			sNthElementFinder.insert(theLatencyMeasurement);
			// We capture these values here so we don't have to take a lock in GetNetTime.
			sTimingMeasurementValid = sNthElementFinder.window_full();
			if(sTimingMeasurementValid)
				sTimingMeasurement = sNthElementFinder.nth_largest_element(sSpokePreferences.mTimingNthElement);

			// update the latency display
			sDisplayLatencyTicks -= sDisplayLatencyBuffer[sDisplayLatencyCount % sDisplayLatencyBuffer.size()];
			sDisplayLatencyBuffer[sDisplayLatencyCount++ % sDisplayLatencyBuffer.size()] = theLatencyMeasurement;
			sDisplayLatencyTicks += theLatencyMeasurement;
		}

	} // loop while there's packet data left

}


static void
spoke_received_ping_request(AIStream& ps, NetAddrBlock address)
{
	uint16 pingIdentifier;
	ps >> pingIdentifier;
	
	// respond back to requestor
	bool initedFrame = false;
	if (!sOutgoingFrame)
	{
		sOutgoingFrame = NetDDPNewFrame();
		initedFrame = true;
	}
	
	AOStreamBE hdr(sOutgoingFrame->data, kStarPacketHeaderSize);
	AOStreamBE ops(sOutgoingFrame->data, ddpMaxData, kStarPacketHeaderSize);
	
	try {
		hdr << (uint16)kPingResponsePacket;
		ops << pingIdentifier;
		
		// blank out the CRC field before calculating
		sOutgoingFrame->data[2] = 0;
		sOutgoingFrame->data[3] = 0;
		
		uint16 crc = calculate_data_crc_ccitt(sOutgoingFrame->data, ops.tellp());
		hdr << crc;
		
		// Send the packet
		sOutgoingFrame->data_size = ops.tellp();
		NetDDPSendFrame(sOutgoingFrame, &address, kPROTOCOL_TYPE, 0 /* ignored */);
	} catch (...) {
		logWarningNMT("Caught exception while constructing/sending ping response packet");
	}
	
	if (initedFrame)
	{
		NetDDPDisposeFrame(sOutgoingFrame);
		sOutgoingFrame = NULL;
	}
} // spoke_received_ping_request()


static void
spoke_received_ping_response(AIStream& ps, NetAddrBlock address)
{
	uint16 pingIdentifier;
	ps >> pingIdentifier;
	
	// we don't send ping requests, so we don't expect to get one
	logWarningNMT("Received unexpected ping response packet");
	
} // spoke_received_ping_response()


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

	logDumpNMT("netDead message: player %d in tick %d", thePlayerIndex, theTick);
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
		logTraceNMT("new timing adjustment message; requested: %d outstanding: %d", sRequestedTimingAdjustment, sOutstandingTimingAdjustment);
        }

        context.mGotTimingAdjustmentMessage = true;
}



static void
handle_lossy_byte_stream_message(AIStream& ps, IncomingGameDataPacketProcessingContext& context)
{
	uint16 theMessageLength;
	ps >> theMessageLength;

	size_t theStartOfMessage = ps.tellg();

	int16 theDistributionType;
	uint8 theSendingPlayer;
	ps >> theDistributionType >> theSendingPlayer;

	uint16 theDataLength = theMessageLength - (ps.tellg() - theStartOfMessage);
	uint16 theSpilloverDataLength = 0;
	if(theDataLength > sizeof(sScratchBuffer))
	{
		logNoteNMT("received too many bytes (%d) of lossy streaming data type %d from player %d; truncating", theDataLength, theDistributionType, theSendingPlayer);
		theSpilloverDataLength = theDataLength - sizeof(sScratchBuffer);
		theDataLength = sizeof(sScratchBuffer);
	}
	ps.read(sScratchBuffer, theDataLength);
	ps.ignore(theSpilloverDataLength);

	logDumpNMT("received %d bytes of lossy streaming type %d data from player %d", theDataLength, theDistributionType, theSendingPlayer);

	call_distribution_response_function_if_available(sScratchBuffer, theDataLength, theDistributionType, theSendingPlayer);
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
	logContextNMT("processing spoke_tick %d", sNetworkTicker);
	
        sNetworkTicker++;

        if(sConnected)
        {
                int32 theSilentTicksBeforeNetDeath = (sOutgoingFlags.getReadTick() >= sSmallestRealGameTick) ? sSpokePreferences.mInGameTicksBeforeNetDeath : sSpokePreferences.mPregameTicksBeforeNetDeath;
        
                if(sNetworkTicker - sLastNetworkTickHeard > theSilentTicksBeforeNetDeath)
                {
			logTraceNMT("giving up on hub; disconnecting");
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

		logDumpNMT("want to provide %d flags", theNumberOfFlagsToProvide);

                while(theNumberOfFlagsToProvide > 0)
		{
		
			// If we're not connected, write only to our local game's queue.
			// If we are connected,
			//	if we're actually in the game, write to both local game and outbound flags queues
			//	else (if pregame), write only to the outbound flags queue.

			WritableTickBasedActionQueue& theTargetQueue =
				sConnected ?
					((sOutgoingFlags.getWriteTick() >= sSmallestRealGameTick) ?
						static_cast<WritableTickBasedActionQueue&>(sLocallyGeneratedFlags)
						: static_cast<WritableTickBasedActionQueue&>(sOutgoingFlags))
					: *(sNetworkPlayers[sLocalPlayerIndex].mQueue);

			if(theTargetQueue.availableCapacity() <= 0)
				break;

			logDumpNMT("enqueueing flags for tick %d", theTargetQueue.getWriteTick());

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
		logDumpNMT("ignoring this tick for timing adjustment"); 
                sOutstandingTimingAdjustment--;
	}

	logDumpNMT("sOutstandingTimingAdjustment is now %d", sOutstandingTimingAdjustment);

	if(sOutgoingLossyByteStreamDescriptors.getCountOfElements() > 0)
		shouldSend = true;

        // If we're connected and (we generated new data or if it's been long enough since we last sent), send.
        if(sConnected)
	{
		if (sHeardFromHub) {
			if(shouldSend || (sNetworkTicker - sLastNetworkTickSent) >= sSpokePreferences.mRecoverySendPeriod)
				send_packet();
		} else {
			if (!(sNetworkTicker % 30))
				send_identification_packet();
		}
	}
	else
	{
		int32 theLocalPlayerWriteTick = getNetworkPlayer(sLocalPlayerIndex).mQueue->getWriteTick();

		// Since we're not connected, we won't be enqueueing flags for the other players in the packet handler.
		// So, we do it here to keep the game moving.
		for(size_t i = 0; i < sNetworkPlayers.size(); i++)
		{
			if(i == sLocalPlayerIndex)
			{
				// move our flags from sent queue to player queue
				while (sSmallestUnconfirmedTick < sUnconfirmedFlags.getWriteTick())
				{
					sNetworkPlayers[i].mQueue->enqueue(sUnconfirmedFlags.peek(sSmallestUnconfirmedTick++));
				}
				continue;
			}
			
			NetworkPlayer_spoke& thePlayer = sNetworkPlayers[i];
			
			if(!thePlayer.mZombie)
			{
				while(thePlayer.mQueue->getWriteTick() < theLocalPlayerWriteTick)
				{
					logDumpNMT("enqueueing NET_DEAD_ACTION_FLAG for player %d tick %d", i, thePlayer.mQueue->getWriteTick());
					thePlayer.mQueue->enqueue(static_cast<action_flags_t>(NET_DEAD_ACTION_FLAG));
				}
			}
		}
	}

        check_send_packet_to_hub();

		sWorldUpdate = true;

        // We want to run again.
        return true;
}

bool spoke_check_world_update()
{
	if (sWorldUpdate)
	{
		sWorldUpdate = false;
		return true;
	}

	return false;
}

static void
send_packet()
{
        try {
		AOStreamBE hdr(sOutgoingFrame->data, kStarPacketHeaderSize);
                AOStreamBE ps(sOutgoingFrame->data, ddpMaxData, kStarPacketHeaderSize);
        
                // Packet type
                hdr << (uint16)kSpokeToHubGameDataPacketV1Magic;

                // Acknowledgement
                ps << sSmallestUnreceivedTick;
        
                // Messages
		// Outstanding lossy streaming bytes?
		if(sOutgoingLossyByteStreamDescriptors.getCountOfElements() > 0)
		{
			// Note: we make a conscious decision here to dequeue these things before
			// writing to ps, so that if the latter operation exhausts ps's buffer and
			// throws, we have less data to mess with next time, and shouldn't end up
			// throwing every time we try to send here.
			// If we eventually got smarter about managing packet space, we could try
			// harder to preserve and pace data - e.g. change the 'if' immediately before this
			// comment to a 'while', only put in as much data as we think we can fit, etc.
			SpokeLossyByteStreamChunkDescriptor theDescriptor = sOutgoingLossyByteStreamDescriptors.peek();
			sOutgoingLossyByteStreamDescriptors.dequeue();
			
			uint16 theMessageLength = theDescriptor.mLength + sizeof(theDescriptor.mType) + sizeof(theDescriptor.mDestinations);

			ps << (uint16)kSpokeToHubLossyByteStreamMessageType
				<< theMessageLength
				<< theDescriptor.mType
				<< theDescriptor.mDestinations;

			// XXX unnecessary copy due to overly restrictive interfaces (retaining for clarity)
			assert(theDescriptor.mLength <= sizeof(sScratchBuffer));
			sOutgoingLossyByteStreamData.peekBytes(sScratchBuffer, theDescriptor.mLength);
			sOutgoingLossyByteStreamData.dequeue(theDescriptor.mLength);

			ps.write(sScratchBuffer, theDescriptor.mLength);
		}
		
                // No more messages
                ps << (uint16)kEndOfMessagesMessageType;
        
                // Action_flags!!!
                if(sOutgoingFlags.size() > 0)
                {
                        ps << sOutgoingFlags.getReadTick();
                        for(int32 tick = sOutgoingFlags.getReadTick(); tick < sOutgoingFlags.getWriteTick(); tick++)
                                ps << sOutgoingFlags.peek(tick);
                }

		logDumpNMT("preparing to send packet: ACK %d, flags [%d,%d)", sSmallestUnreceivedTick, sOutgoingFlags.getReadTick(), sOutgoingFlags.getWriteTick());

		// blank out the CRC before calculating it
		sOutgoingFrame->data[2] = 0;
		sOutgoingFrame->data[3] = 0;

		uint16 crc = calculate_data_crc_ccitt(sOutgoingFrame->data, ps.tellp());
		hdr << crc;

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



static void
send_identification_packet()
{
        try {
		AOStreamBE hdr(sOutgoingFrame->data, kStarPacketHeaderSize);
                AOStreamBE ps(sOutgoingFrame->data, ddpMaxData, kStarPacketHeaderSize);
        
		// Message type
		hdr << (uint16) kSpokeToHubIdentification;
        
                // ID
                ps << (uint16)sLocalPlayerIndex;

		// blank out the CRC field before calculating
		sOutgoingFrame->data[2] = 0;
		sOutgoingFrame->data[3] = 0;

		uint16 crc = calculate_data_crc_ccitt(sOutgoingFrame->data, ps.tellp());
		hdr << crc;

                // Send the packet
                sOutgoingFrame->data_size = ps.tellp();
                if(sHubIsLocal)
                        send_frame_to_local_hub(sOutgoingFrame, &sHubAddress, kPROTOCOL_TYPE, 0 /* ignored */);
                else
                        NetDDPSendFrame(sOutgoingFrame, &sHubAddress, kPROTOCOL_TYPE, 0 /* ignored */);
        }
        catch (...) {
        }
}

int32 spoke_latency()
{
	return (sDisplayLatencyCount >= TICKS_PER_SECOND) ? sDisplayLatencyTicks * 1000 / TICKS_PER_SECOND / sDisplayLatencyBuffer.size() : NetworkStats::invalid;
}

TickBasedActionQueue* spoke_get_unconfirmed_flags_queue()
{
	return &sUnconfirmedFlags;
}

int32 spoke_get_smallest_unconfirmed_tick()
{
	return sSmallestUnconfirmedTick;
}
		

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


void SpokeParsePreferencesTree(InfoTree prefs, std::string version)
{
	for (size_t i = 0; i < kNumInt32Attributes; ++i)
	{
		int32 value = *(sAttributeDestinations[i]);
		if (prefs.read_attr(sAttributeStrings[i], value))
		{
			int32 min = INT32_MIN;
			switch (i) {
				case kPregameTicksBeforeNetDeathAttribute:
				case kInGameTicksBeforeNetDeathAttribute:
				case kRecoverySendPeriodAttribute:
				case kTimingWindowSizeAttribute:
					min = 1;
					break;
				case kTimingNthElementAttribute:
					min = 0;
					break;
			}
			if (value < min)
				logWarning("improper value %d for attribute %s of <spoke>; must be at least %d. using default of %d", value, sAttributeStrings[i], min, *(sAttributeDestinations[i]));
			else
				*(sAttributeDestinations[i]) = value;
		}
	}

	prefs.read_attr("adjust_timing", sSpokePreferences.mAdjustTiming);
	
	
	// The checks above are not sufficient to catch all bad cases; if user specified a window size
	// smaller than default, this is our only chance to deal with it.
	if(sSpokePreferences.mTimingNthElement >= sSpokePreferences.mTimingWindowSize)
	{
		logWarning("value for <spoke> attribute %s (%d) must be less than value for %s (%d).  using %d", sAttributeStrings[kTimingNthElementAttribute], sSpokePreferences.mTimingNthElement, sAttributeStrings[kTimingWindowSizeAttribute], sSpokePreferences.mTimingWindowSize, sSpokePreferences.mTimingWindowSize - 1);
		
		sSpokePreferences.mTimingNthElement = sSpokePreferences.mTimingWindowSize - 1;
	}
}


InfoTree SpokePreferencesTree()
{
	InfoTree root;
	
	for (size_t i = 0; i < kNumInt32Attributes; ++i)
		root.put_attr(sAttributeStrings[i], *(sAttributeDestinations[i]));
	root.put_attr("adjust_timing", sSpokePreferences.mAdjustTiming);
	
	return root;
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

#endif // !defined(DISABLE_NETWORKING)
