/*
 *  ActionQueues.cpp
 *  created for Marathon: Aleph One <http://source.bungie.org/>

    Copyright (C) 1991-2002 and beyond by Bungie Studios, Inc.
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
	
	
May 9, 2002 (Loren Petrich):
	Changed enqueueActionFlags() so that it can make zombie players controllable by Pfhortran;
	did this by adding the argument "ZombiesControllable" (default: false)
	
Jun 9, 2002 (tiennou):
	Following the above example, I modified dequeueActionFlags() & countActionFlags().
	
 *  An ActionQueues object encapsulates a set of players' action_queues.
 *
 *  Created by woody on Wed Feb 20 2002.
 */

#include	"ActionQueues.h"

#include    "player.h"  // for get_player_data()

// Lifted from player.cpp; changed short to int
struct ActionQueues::action_queue {
	unsigned int read_index, write_index;
	
	uint32 *buffer;
};



// basically ripped from player.cpp::allocate_player_memory().
ActionQueues::ActionQueues(unsigned int inNumPlayers, unsigned int inQueueSize) : mNumPlayers(inNumPlayers), mQueueSize(inQueueSize) {

    /* allocate space for our action queue headers and the queues themselves */
    mQueueHeaders	= new action_queue[mNumPlayers];
    mFlagsBuffer	= new uint32[mNumPlayers * mQueueSize];
    
    assert(mQueueHeaders && mFlagsBuffer);
    
    /* tell the queues where their buffers are */
    for (unsigned i = 0; i < mNumPlayers; ++i)
    {
            mQueueHeaders[i].buffer= mFlagsBuffer + i*mQueueSize;
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



// Lifted from player.cpp::queue_action_flags()
/* queue an action flag on the given playerÕs queue (no zombies allowed) */
void
ActionQueues::enqueueActionFlags(
	int player_index,
	uint32 *action_flags,
	int count,
	bool ZombiesControllable)
{
	struct player_data *player= get_player_data(player_index);
	struct action_queue *queue= mQueueHeaders+player_index;

	//assert(!PLAYER_IS_ZOMBIE(player)); // CP: Changed for scripting
	if (!ZombiesControllable && PLAYER_IS_ZOMBIE(player))
		return;
                
	while ((count-= 1)>=0)
	{
		queue->buffer[queue->write_index]= *action_flags++;
		queue->write_index= (queue->write_index+1) % mQueueSize;
		if (queue->write_index==queue->read_index) dprintf("blew player %dÕs queue at %p;g;", player_index, queue);
	}

	return;
}


// Lifted from player.cpp::dequeue_action_flags()
/* dequeueÕs a single action flag from the given queue (zombies always return zero) */
uint32
ActionQueues::dequeueActionFlags(
	int player_index,
	bool ZombiesControllable)
{
	struct player_data *player= get_player_data(player_index);
	struct action_queue *queue= mQueueHeaders+player_index;

	uint32 action_flags;

	if (!ZombiesControllable && PLAYER_IS_ZOMBIE(player))
	{
		//dprintf("Player is zombie!", player_index);	// CP: Disabled for scripting
		action_flags= 0;
	}
	else
	{
		assert(queue->read_index!=queue->write_index);
		action_flags= queue->buffer[queue->read_index];
		queue->read_index= (queue->read_index+1) % mQueueSize;
	}

	return action_flags;
}



// Lifted from player.cpp::get_action_queue_size()
/* returns the number of elements sitting in the given queue (zombies always return queue diameter) */
unsigned int
ActionQueues::countActionFlags(
	int player_index,
	bool ZombiesControllable)
{
	struct player_data *player= get_player_data(player_index);
	struct action_queue *queue= mQueueHeaders+player_index;
	unsigned int size;

	if (!ZombiesControllable && PLAYER_IS_ZOMBIE(player))
	{
		//dprintf("PLayer %d is a zombie", player_index);  // CP: Disabled for scripting
		size= mQueueSize;
	} 
	else
	{
                // ZZZ: better? phrasing of this operation (no branching; only one store; also, works with unsigned's)
                size = (mQueueSize + queue->write_index - queue->read_index) % mQueueSize;
                
		//if ((size= queue->write_index-queue->read_index)<0) size+= mQueueSize;
	}
	
	return size;
}
