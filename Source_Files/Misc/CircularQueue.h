/*
 *  CircularQueue.h
 *  created for Marathon: Aleph One <http://source.bungie.org/>

	Copyright (C) 2002 and beyond by Woody Zenfell, III
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
 */

#ifndef CIRCULAR_QUEUE_H
#define CIRCULAR_QUEUE_H

template<typename T>
class CircularQueue {
public:
    explicit    CircularQueue(unsigned int inSize) : mData(NULL) { reset(inSize); }
    
    void		reset() { reset(mQueueSize); }
    void		reset(unsigned int inSize) {
        mReadIndex = mWriteIndex = 0;
        if(inSize != mQueueSize || mData == NULL) {
            mQueueSize = inSize;
            if(mData != NULL)
                delete [] mData;
            mData = new T[mQueueSize];
        }
    }

    unsigned int	getCountOfElements()
                            { return (mQueueSize + mWriteIndex - mReadIndex) % mQueueSize; }
    unsigned int	getSpaceRemaining()
                            { return mQueueSize - 1 - getCountOfElements(); }
    
    const T&        peek() { return mData[getReadIndex()]; }
    void            dequeue() { advanceReadIndex(); }
    void            enqueue(const T& inData) { mData[getWriteIndex()] = inData;  advanceWriteIndex(); }

    ~CircularQueue() { if(mData != NULL) delete [] mData; }

protected:
    unsigned int	getReadIndex(unsigned int inOffset = 0)
                            { assert(getCountOfElements() > inOffset);  return (mReadIndex + inOffset) % mQueueSize; }
    unsigned int	getWriteIndex(unsigned int inOffset = 0)
                            { assert(getSpaceRemaining() > inOffset);  return (mWriteIndex + inOffset) % mQueueSize; }
                            
    unsigned int	advanceReadIndex(unsigned int inAmount = 1) {   
        if(inAmount > 0) {
            assert(getCountOfElements() > inAmount - 1);
            mReadIndex = (mReadIndex + inAmount) % mQueueSize;
        }
        return mReadIndex;
    }
    unsigned int	advanceWriteIndex(unsigned int inAmount = 1) {
        if(inAmount > 0) {
            assert(getSpaceRemaining() > inAmount - 1);
            mWriteIndex = (mWriteIndex + inAmount) % mQueueSize;
        }
        return mWriteIndex;
    }

    unsigned int	mReadIndex;
    unsigned int	mWriteIndex;
    unsigned int	mQueueSize;

    T*              mData;

private:
    // disallow copying for now, just to be sure nobody tries it while it does the wrong thing.
    CircularQueue(CircularQueue<T>&);
    CircularQueue<T>& operator =(CircularQueue<T>&);
};

#endif // CIRCULAR_QUEUE_H
