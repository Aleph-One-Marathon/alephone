/*
 *  CircularQueue.h
 *  created for Marathon: Aleph One <http://source.bungie.org/>

	Copyright (C) 2002 and beyond by Woody Zenfell, III
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

 *  The code in this file is licensed to you under the GNU GPL.  As the copyright holder,
 *  however, I reserve the right to use this code as I see fit, without being bound by the
 *  GPL's terms.  This exemption is not intended to apply to modified versions of this file -
 *  if I were to use a modified version, I would be a licensee of whomever modified it, and
 *  thus must observe the GPL terms.
 *
 *  Done a thousand times a thousand ways, this is another circular queue.
 *  Hope it's useful.  (And correct.)
 *
 *  Created by woody in February 2002.
 *
 *  May 8, 2003 (Woody Zenfell): minor modifications; can now copy/assign queues.
 */

#ifndef CIRCULAR_QUEUE_H
#define CIRCULAR_QUEUE_H

#include "csalerts.h"  // need assert

template<typename T>
class CircularQueue {
public:
        CircularQueue() : mData(NULL) { reset(0); }

        explicit CircularQueue(unsigned int inSize) : mData(NULL) { reset(inSize); }

	// The queue being copied had best not change while we're copying...
        CircularQueue(const CircularQueue<T>& o) : mData(NULL) {
		*this = o;
	}
		
        CircularQueue<T>& operator =(const CircularQueue<T>& o) {
		if(&o != this)
		{
			reset(o.getTotalSpace());
			for(unsigned int i = 0; i < o.getCountOfElements(); i++)
				enqueue(o.mData[o.getReadIndex(i)]);
		}
		return *this;
	}		

        void reset() { reset(getTotalSpace()); }
        
        void reset(unsigned int inSize) {
                // We need size+1 elements of storage to allow size elements in queue
                unsigned int theStorageCount = inSize + 1;

                // Guard against wrap-around
                assert(theStorageCount > inSize);

                mReadIndex = mWriteIndex = 0;

                if(theStorageCount != mQueueSize || mData == NULL) {
                        mQueueSize = theStorageCount;
                        if(mData != NULL)
                                delete [] mData;
                        mData = new T[mQueueSize];
                }
        }

        unsigned int	getCountOfElements() const
				{ return (mQueueSize > 0) ? (mQueueSize + mWriteIndex - mReadIndex) % mQueueSize : 0; }

        unsigned int	getRemainingSpace() const
                            { return getTotalSpace() - getCountOfElements(); }

        unsigned int	getTotalSpace() const { return (mQueueSize > 0) ? mQueueSize - 1 : 0; }
    
        const T&        peek() const { return mData[getReadIndex()]; }

        void            dequeue(unsigned int inAmount = 1) { advanceReadIndex(inAmount); }

        void            enqueue(const T& inData) { mData[getWriteIndex()] = inData;  advanceWriteIndex(); }

        virtual ~CircularQueue() { if(mData != NULL) delete [] mData; }

protected:
        unsigned int	getReadIndex(unsigned int inOffset = 0) const
                            { assert(getCountOfElements() > inOffset);  return (mReadIndex + inOffset) % mQueueSize; }
        unsigned int	getWriteIndex(unsigned int inOffset = 0) const
                            { assert(getRemainingSpace() > inOffset);  return (mWriteIndex + inOffset) % mQueueSize; }
                            
        unsigned int advanceReadIndex(unsigned int inAmount = 1) {   
                if(inAmount > 0) {
                        assert(inAmount <= getCountOfElements());
                        mReadIndex = (mReadIndex + inAmount) % mQueueSize;
                }
                return mReadIndex;
        }
        
        unsigned int advanceWriteIndex(unsigned int inAmount = 1) {
                if(inAmount > 0) {
                        assert(inAmount <= getRemainingSpace());
                        mWriteIndex = (mWriteIndex + inAmount) % mQueueSize;
                }
                return mWriteIndex;
        }

        unsigned int	mReadIndex;
        unsigned int	mWriteIndex;
        unsigned int	mQueueSize;

        T*              mData;
};

#endif // CIRCULAR_QUEUE_H
