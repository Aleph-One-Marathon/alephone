/*
 *  TickBasedCircularQueue.h

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

 *  Created by Woody Zenfell, III on Fri Apr 25 2003.
 *
 *  A circular queue of elements (probably action_flags) keyed by game_tick.  Note Bad Things could happen
 *  if game_tick wraps around the int32 max; but that's the case in general in A1 I think.
 */

#ifndef TICKBASEDCIRCULARQUEUE_H
#define TICKBASEDCIRCULARQUEUE_H

#include "cseries.h"

#include <set>

// (1) if(q.size() > n) { q.peek(q.getReadTick() + n); } and
// (2) if(q.size() > 0) { q.dequeue(); } should always be safe (in reader) as writer can only add elements to q

// (3) if(q.availableCapacity() > 0) { q.enqueue(blah); } should always be safe (in writer) since
//	reader may only remove elements from q

// more succinctly,
// behind writer's back, q.availableCapacity() can only increase (and q.size() only decrease);
// behind reader's back, q.size() can only increase (and q.availableCapacity() only decrease).

// WritableTickBasedCircularQueue is a restricted interface to a general TickBasedCircularQueue.
template <typename tValueType>
class WritableTickBasedCircularQueue {
public:
        typedef tValueType ValueType;
        virtual ~WritableTickBasedCircularQueue() {}
        virtual void reset(int32 inTick) = 0;
        virtual int32 availableCapacity() const = 0;
        virtual void enqueue(const tValueType& inFlags) = 0;
        virtual int32 getWriteTick() const = 0;
};



// A DuplicatingTickBasedCircularQueue isn't really a Queue; instead it's a mechanism
// for writing to multiple WritableTickBasedCircularQueues simultaneously, as if they're one.
// A DuplicatingTBCQ serves as the "one writer" for each of its child-queues.
template <typename tValueType>
class DuplicatingTickBasedCircularQueue : public WritableTickBasedCircularQueue<tValueType> {
public:
        typedef std::set<WritableTickBasedCircularQueue<tValueType>*> ChildrenCollection;

        // collection of children MUST NOT be modified while writer is active
        ChildrenCollection& children() { return mChildren; }

        // resets all child queues
        void reset(int32 inTick)
        {
                for(typename ChildrenCollection::iterator i = mChildren.begin(); i != mChildren.end(); i++)
                        (*i)->reset(inTick);
        }
        
        // returns the smallest capacity of any child, so (3) above is true for us as well
        // (note of course although all queues should have same write tick, they need not have
        //  same read tick, and thus may have differing availableCapacities.)
        int32 availableCapacity() const
        {
                assert(!mChildren.empty());
                int32 theCapacity = INT_MAX;
                for(typename ChildrenCollection::const_iterator i = mChildren.begin(); i != mChildren.end(); i++)
                {
                        int32 theSingleCapacity = (*i)->availableCapacity();
                        if(theSingleCapacity < theCapacity)
                                theCapacity = theSingleCapacity;
                }
                
                return theCapacity;
        }

        // Since we're the only writer, all children should have the same write-tick
        int32 getWriteTick() const { assert(!mChildren.empty());  return (*(mChildren.begin()))->getWriteTick(); }

        void enqueue(const tValueType& inFlags)
        {
                for(typename ChildrenCollection::iterator i = mChildren.begin(); i != mChildren.end(); i++)
                        (*i)->enqueue(inFlags);
        }
        
private:
        ChildrenCollection	mChildren;
};



template <typename tValueType>
class ConcreteTickBasedCircularQueue : public WritableTickBasedCircularQueue<tValueType> {
public:
        // Methods for use only when neither reader nor writer active
        ConcreteTickBasedCircularQueue(int inBufferCapacity)
                : mBufferSize(inBufferCapacity + 1)
        {
                mFlagsBuffer = new tValueType[mBufferSize];
                reset(0);
        }
        
        ConcreteTickBasedCircularQueue(const ConcreteTickBasedCircularQueue<tValueType>& o)
                : mReadTick(o.mReadTick), mWriteTick(o.mWriteTick), mBufferSize(o.mBufferSize)
        {
                // XXX this could be made more efficient when given actual objects by allocating
                // raw storage and then using new (void*) tValueType to initialize them etc. (like vector)
                // (of course, I suppose we could always use a vector rather than an array, too...)
                mFlagsBuffer = new tValueType[mBufferSize];
                for(int32 tick = getReadTick(); tick < getWriteTick(); tick++)
                        elementForTick(tick) = o.peek(tick);
        }
        
        ~ConcreteTickBasedCircularQueue() { delete [] mFlagsBuffer; }

        void reset(int32 inTick) { mReadTick = mWriteTick = inTick; }

        // Methods for use by anyone, anytime - but be careful how you interpret the results :)
        int32 getReadTick() const { return mReadTick; }
        int32 getWriteTick() const { return mWriteTick; }
        int32 size() const { return mWriteTick - mReadTick; }
        int32 totalCapacity() const { return mBufferSize - 1; }
        int32 availableCapacity() const { return totalCapacity() - size(); }

        // Methods for use only by reader
        const tValueType& peek(int32 inTick) const { return elementForTick(inTick); }
        void dequeue() { assert(size() > 0);  mReadTick++; }

        // Methods for use only by writer
        void enqueue(const tValueType& inFlags) { 
		assert(availableCapacity() > 0);
		int32 positiveWriteTick = mWriteTick;
		while (positiveWriteTick < mBufferSize) positiveWriteTick += mBufferSize;
		mFlagsBuffer[positiveWriteTick % mBufferSize] = inFlags;  
		mWriteTick++; 
	}

protected:
        tValueType& elementForTick(int32 inTick) const { 
		assert(inTick >= mReadTick); 
		assert(inTick < mWriteTick); 
		int32 positiveInTick = inTick;
		while (positiveInTick < 0) positiveInTick += mBufferSize;
		return mFlagsBuffer[positiveInTick % mBufferSize]; 
	}
//        const uint32& elementForTick(int32 inTick) const { assert(inTick >= mReadTick); assert(inTick < mWriteTick); return mFlagsBuffer[inTick % mBufferSize]; }

private:
        int32			mReadTick;
        int32			mWriteTick;
        tValueType*	mFlagsBuffer;
        int			mBufferSize;	// buffer capacity + 1 due to circular queue overhead
};



// A MutableElements circular queue can have its elements changed after enqueueing (but before
// dequeueing).  Clearly, careful synchronization is required when using this feature.
template <typename tValueType>
class MutableElementsTickBasedCircularQueue : public ConcreteTickBasedCircularQueue<tValueType> {
public:
        MutableElementsTickBasedCircularQueue(int inBufferCapacity) : ConcreteTickBasedCircularQueue<tValueType>(inBufferCapacity) {}
        tValueType& at(int32 inTick) { return ConcreteTickBasedCircularQueue<tValueType>::elementForTick(inTick); }
        tValueType& operator [](int32 inTick) { return at(inTick); }
};

#endif // TICKBASEDCIRCULARQUEUE_H
