/*
 *  ActionQueues.h
 *  created for Marathon: Aleph One <http://source.bungie.org/>

    Copyright (C) 1991-2002 and beyond by Woody Zenfell, III
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
 	
 *  Encapsulates a set of action_queues, so we can have multiple sets and explicitly choose one.
 *
 *  Created by woody on Wed Feb 20 2002.
 */

#ifndef	ACTIONQUEUES_H
#define	ACTIONQUEUES_H

#include "cseries.h"

class ActionQueues {
public:
    ActionQueues(unsigned int inNumPlayers, unsigned int inQueueSize);
    
    void		reset();

    void		enqueueActionFlags(int inPlayerIndex, uint32* inFlags, int inFlagsCount,
    				bool ZombiesControllable = false);
    uint32		dequeueActionFlags(int inPlayerIndex);
    
    unsigned int	countActionFlags(int inPlayerIndex);
    
    ~ActionQueues();
    
protected:
    struct action_queue;

    unsigned int	mNumPlayers;
    unsigned int	mQueueSize;
    action_queue*	mQueueHeaders;
    uint32*		mFlagsBuffer;

// Hide these until they have valid implementation
private:
    ActionQueues(ActionQueues&);
    ActionQueues& operator =(ActionQueues&);
};

#endif // ACTIONQUEUES_H
