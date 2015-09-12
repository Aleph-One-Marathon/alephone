/*
 *  ActionQueues.cpp
 *  created for Marathon: Aleph One <http://source.bungie.org/>

    Copyright (C) 1991-2002 and beyond by Bungie Studios, Inc.
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
	
	
May 9, 2002 (Loren Petrich):
	Changed enqueueActionFlags() so that it can make zombie players controllable by Pfhortran;
	did this by adding the argument "ZombiesControllable" (default: false)
	
Jun 9, 2002 (tiennou):
	Following the above example, I modified dequeueActionFlags() & countActionFlags().

Feb 3, 2003 (Woody Zenfell):
        Made 'ZombiesControllable' a property of a queue-set rather than an argument to the methods.

May 14, 2003 (Woody Zenfell):
	Can reset a single action queue within a set now, principally for use with
	LegacyActionQueueToTickBasedQueueAdapter.

 June 14, 2003 (Woody Zenfell):
	Added "peekActionFlags()" method to examine action_flags without removing them
 
 *  An ActionQueues object encapsulates a set of players' action_queues.
 *
 *  Created by woody on Wed Feb 20 2002.
 */

#include "ActionQueues.h"

#include "player.h"  // for get_player_data()
#include "Logging.h"

// basically ripped from player.cpp::allocate_player_memory().
ActionQueues::ActionQueues(unsigned int inNumPlayers, unsigned int inQueueSize, bool inZombiesControllable) : mNumPlayers(inNumPlayers), mQueueSize(inQueueSize), mZombiesControllable(inZombiesControllable) {

    /* allocate space for our action queue headers and the queues themselves */
    mQueueHeaders	= new action_queue[mNumPlayers];
    mFlagsBuffer	= new uint32[mNumPlayers * mQueueSize];
    
    assert(mQueueHeaders && mFlagsBuffer);
    
    /* tell the queues where their buffers are */
    for (unsigned i = 0; i < mNumPlayers; ++i)
    {
            mQueueHeaders[i].buffer= mFlagsBuffer + i*mQueueSize;
            // From reset()
            mQueueHeaders[i].read_index = mQueueHeaders[i].write_index = 0;
    }
}



ActionQueues::~ActionQueues() {
    if(mFlagsBuffer)
        delete [] mFlagsBuffer;

    if(mQueueHeaders)
        delete [] mQueueHeaders;
}



// Lifted from player.cpp::reset_player_queues()
void
ActionQueues::reset()
{
	for (unsigned i=0; i < mNumPlayers; ++i) {
		mQueueHeaders[i].read_index = mQueueHeaders[i].write_index = 0;
	}
}



void
ActionQueues::resetQueue(int inPlayerIndex)
{
	assert(inPlayerIndex >= 0 && inPlayerIndex < static_cast<int>(mNumPlayers));
	mQueueHeaders[inPlayerIndex].read_index = mQueueHeaders[inPlayerIndex].write_index = 0;
}



// Lifted from player.cpp::queue_action_flags()
/* queue an action flag on the given playerÕs queue (no zombies allowed) */
void
ActionQueues::enqueueActionFlags(
	int player_index,
	const uint32 *action_flags,
	int count)
{
	struct player_data *player= get_player_data(player_index);
	struct action_queue *queue= mQueueHeaders+player_index;

        // Cannot enqueue onto a Zombie queue unless explicitly allowed
	if (!mZombiesControllable && PLAYER_IS_ZOMBIE(player))
		return;
                
	while ((count-= 1)>=0)
	{
		queue->buffer[queue->write_index]= *action_flags++;
		queue->write_index= (queue->write_index+1) % mQueueSize;
		if (queue->write_index==queue->read_index)
			logError("blew player %d's queue", player_index);
	}
	
	return;
}


// Lifted from player.cpp::dequeue_action_flags()
/* dequeueÕs a single action flag from the given queue (zombies always return zero) */
uint32
ActionQueues::dequeueActionFlags(
	int player_index)
{
	struct player_data *player= get_player_data(player_index);
	struct action_queue *queue= mQueueHeaders+player_index;

	uint32 action_flags;

        // Non-controllable zombies always just return 0 for their action_flags.
	if (!mZombiesControllable && PLAYER_IS_ZOMBIE(player))
	{
		action_flags= 0;
	}
	else if (queue->read_index==queue->write_index)
	{
		// None to be read
		action_flags= 0;
		logError("dequeueing empty ActionQueue for player %d", player_index);
	}
	else
	{
		// assert(queue->read_index!=queue->write_index);
		action_flags= queue->buffer[queue->read_index];
		queue->read_index= (queue->read_index+1) % mQueueSize;
	}

	return action_flags;
}



uint32
ActionQueues::peekActionFlags(int inPlayerIndex, size_t inElementsFromHead)
{
	// ZZZ: much of this body copied from dequeueActionFlags.  Sorry about that...
	struct player_data *player= get_player_data(inPlayerIndex);
	struct action_queue *queue= mQueueHeaders+inPlayerIndex;

	uint32 action_flags;

        // Non-controllable zombies always just return 0 for their action_flags.
	if (!mZombiesControllable && PLAYER_IS_ZOMBIE(player))
	{
		action_flags= 0;
	}
	else if (inElementsFromHead >= countActionFlags(inPlayerIndex))
	{
		// None to be read
		action_flags= 0;
		logError("peeking too far ahead (%d/%d) in ActionQueue for player %d", inElementsFromHead, countActionFlags(inPlayerIndex), inPlayerIndex);
	}
	else
	{
		size_t theQueueIndex = (queue->read_index + inElementsFromHead) % mQueueSize;
		action_flags= queue->buffer[theQueueIndex];
	}

	return action_flags;
}


// Lifted from player.cpp::get_action_queue_size()
/* returns the number of elements sitting in the given queue (zombies always return queue diameter) */
unsigned int
ActionQueues::countActionFlags(
	int player_index)
{
	struct player_data *player= get_player_data(player_index);
	struct action_queue *queue= mQueueHeaders+player_index;
	unsigned int size;

        // Non-controllable zombies have lots and lots of do-nothing flags available.
	if (!mZombiesControllable && PLAYER_IS_ZOMBIE(player))
	{
		size= mQueueSize;
	} 
	else
	{
                // ZZZ: better? phrasing of this operation (no branching; only one store; also, works with unsigned's)
                size = (mQueueSize + queue->write_index - queue->read_index) % mQueueSize;
	}
	
	return size;
}



bool
ActionQueues::zombiesControllable() {
        return mZombiesControllable;
}



void
ActionQueues::setZombiesControllable(bool inZombiesControllable) {
        mZombiesControllable = inZombiesControllable;
}

void ModifiableActionQueues::modifyActionFlags(int inPlayerIndex, uint32 inFlags, uint32 inFlagsMask)
{
	if (!countActionFlags(inPlayerIndex))
	{
		logError("no flags when modifying ActionQueue for player %d", inPlayerIndex);
		return;
	}

	action_queue *queue = mQueueHeaders + inPlayerIndex;
	if (queue->buffer[queue->read_index] != 0xffffffff)
		queue->buffer[queue->read_index] = (queue->buffer[queue->read_index] & ~inFlagsMask) | (inFlags & inFlagsMask);

}
