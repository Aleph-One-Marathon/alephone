/*
 *  StarGameProtocol.cpp

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

 *  Created by Woody Zenfell, III on Sat May 17 2003.
 *
 *  Glue to make the star network protocol code interface with the rest of A1.
 *
 *  May 27, 2003 (Woody Zenfell):
 *	Support for lossy streaming data distribution.
 */

#if !defined(DISABLE_NETWORKING)

#include "cseries.h"

#include "StarGameProtocol.h"

#include "network_star.h"
#include "TickBasedCircularQueue.h"
#include "player.h" // GetRealActionQueues
#include "interface.h" // process_action_flags (despite paf() being defined in vbl.*)
#include "InfoTree.h"

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


static WritableTickBasedActionQueue* sStarQueues[MAXIMUM_NUMBER_OF_NETWORK_PLAYERS];
static NetTopology*	sTopology = NULL;
static short*		sNetStatePtr = NULL;

#ifdef A1_NETWORK_STANDALONE_HUB
static constexpr bool sHubIsLocal = true;
#else
static bool sHubIsLocal;
#endif



bool
StarGameProtocol::Enter(short* inNetStatePtr)
{
#ifndef A1_NETWORK_STANDALONE_HUB
	sHubIsLocal = false;
#endif
	sNetStatePtr = inNetStatePtr;
	return true;
}

void
StarGameProtocol::PacketHandler(UDPpacket& packet)
{
        if(sHubIsLocal)
                hub_received_network_packet(packet);
        else
                spoke_received_network_packet(packet);
}



bool
StarGameProtocol::Sync(NetTopology* inTopology, int32 inSmallestGameTick, int inLocalPlayerIndex, bool isServer)
{
	assert(inTopology != NULL);

#ifdef A1_NETWORK_STANDALONE_HUB
	assert(isServer && inLocalPlayerIndex == NONE);
#endif
	
	sTopology = inTopology;
	
        bool theConnectedPlayerStatus[MAXIMUM_NUMBER_OF_NETWORK_PLAYERS];

        for(int i = 0; i < sTopology->player_count; i++)
        {
                if(sTopology->players[i].identifier == NONE)
                        sStarQueues[i] = NULL;
                else
                        sStarQueues[i] = new LegacyActionQueueToTickBasedQueueAdapter<action_flags_t>(i);

                theConnectedPlayerStatus[i] = ((sTopology->players[i].identifier != NONE) && !sTopology->players[i].net_dead);
        }

        if(isServer)
        {
#ifndef A1_NETWORK_STANDALONE_HUB
		sHubIsLocal = true;
#endif
		
                IPaddress* theAddresses[MAXIMUM_NUMBER_OF_NETWORK_PLAYERS];

                for(int i = 0; i < sTopology->player_count; i++)
                        theAddresses[i] = (theConnectedPlayerStatus[i] ? &(sTopology->players[i].ddpAddress) : NULL);

                hub_initialize(inSmallestGameTick, sTopology->player_count, theAddresses, inLocalPlayerIndex);
        }
#ifndef A1_NETWORK_STANDALONE_HUB
	else
		sHubIsLocal = false;


        spoke_initialize(sTopology->server.ddpAddress, inSmallestGameTick, sTopology->player_count,
                         sStarQueues, theConnectedPlayerStatus, inLocalPlayerIndex, sHubIsLocal);
#endif

        *sNetStatePtr = netActive;

        return true;
}



bool
StarGameProtocol::UnSync(bool inGraceful, int32 inSmallestPostgameTick)
{
        if(*sNetStatePtr == netStartingUp || *sNetStatePtr == netActive)
        {
#ifndef A1_NETWORK_STANDALONE_HUB
                spoke_cleanup(inGraceful);
#endif
                if(sHubIsLocal)
                        hub_cleanup(inGraceful, inSmallestPostgameTick);

                for(int i = 0; i < sTopology->player_count; i++)
                {
                        if(sStarQueues[i] != NULL)
                        {
                                delete sStarQueues[i];
                                sStarQueues[i] = NULL;
                        }
                }
        }

        *sNetStatePtr = netDown;

        return true;
}



int32
StarGameProtocol::GetNetTime(void)
{
        return spoke_get_net_time();
}

int32
StarGameProtocol::GetUnconfirmedActionFlagsCount()
{
	TickBasedActionQueue *q = spoke_get_unconfirmed_flags_queue();
	return q->getWriteTick() - q->getReadTick();
}

uint32 
StarGameProtocol::PeekUnconfirmedActionFlag(int32 offset)
{
	TickBasedActionQueue *q = spoke_get_unconfirmed_flags_queue();
	return q->peek(q->getReadTick() + offset);
}

void 
StarGameProtocol::UpdateUnconfirmedActionFlags()
{
	TickBasedActionQueue *q = spoke_get_unconfirmed_flags_queue();
	while (q->getReadTick() < spoke_get_smallest_unconfirmed_tick() && q->getReadTick() < q->getWriteTick())
	{
		q->dequeue();
	}
}

bool
StarGameProtocol::CheckWorldUpdate()
{
	return spoke_check_world_update();
}

/* ZZZ addition:
---------------------------
	make_player_really_net_dead
---------------------------

---> player index of newly net-dead player

	sets up our housekeeping here to mark a player as net-dead.
	*/

void
make_player_really_net_dead(size_t inPlayerIndex)
{
        assert(inPlayerIndex < static_cast<size_t>(sTopology->player_count));
        sTopology->players[inPlayerIndex].net_dead = true;
}

void
StarGameProtocol::ParsePreferencesTree(InfoTree prefs, std::string version)
{
	for (const InfoTree &child : prefs.children_named("hub"))
		HubParsePreferencesTree(child, version);
	for (const InfoTree &child : prefs.children_named("spoke"))
		SpokeParsePreferencesTree(child, version);
}



void
DefaultStarPreferences()
{
	DefaultHubPreferences();
	DefaultSpokePreferences();
}



InfoTree StarPreferencesTree()
{
	InfoTree root;
	root.put_child("hub", HubPreferencesTree());
	root.put_child("spoke", SpokePreferencesTree());
	return root;
}

#endif // !defined(DISABLE_NETWORKING)
