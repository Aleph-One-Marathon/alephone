/*
 *  CircularByteBuffer.h
 *  created for Marathon: Aleph One <http://source.bungie.org/>

	Copyright (C) 2003 Woody Zenfell, III

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

 *  A circular queue of bytes, with support for mass enqueueing from/peeking to caller's buffer
 *
 *  Created by Woody Zenfell, III on Sun Jun 29 2003.

 July 19, 2003 (Woody Zenfell):
	Additional "NoCopy" interface may let clients avoid a copy, though it's not as safe.
 
 */

#ifndef CIRCULAR_BYTE_BUFFER_H
#define CIRCULAR_BYTE_BUFFER_H

#include <utility> // std::pair

#include "CircularQueue.h"

typedef CircularQueue<char> CircularByteBufferBase;

class CircularByteBuffer : public CircularByteBufferBase
{
public:
	CircularByteBuffer(unsigned int inUsableBytes) : CircularByteBufferBase(inUsableBytes) {}

	// These peek and enqueue a chunk of data inByteCount long, correctly handling wraparound.
	// Caller must make sure there is enough data/space available first.
	void peekBytes(void* outBytes, unsigned int inByteCount);
	void enqueueBytes(const void* inBytes, unsigned int inByteCount);

	// These routines allow more-direct access to the buffer itself - something I'd like to
	// avoid, strictly speaking, but there are so many places where it would let us skip a copy...

	// Returns pointers into the buffer and the number of bytes to be taken from each.
	// Caller is responsible for checking inByteCount <= getCountOfElements().
	// The First and SecondByteCounts will sum to inByteCount.
	// We expose both pointers and lengths at once to facilitate the use of writev()-type routines.
	// *outSecondBytes == NULL && *outSecondByteCount == 0 if the second chunk is unneeded.
	// Clearly, you may choose to read fewer than inByteCount bytes, as the read index is not actually advanced
	// until you call dequeue().
	// Any of the out-pointers may be passed as NULL if you don't care about the returned value.
	void peekBytesNoCopy(unsigned int inByteCount, const void** outFirstBytes, unsigned int* outFirstByteCount,
				const void** outSecondBytes, unsigned int* outSecondByteCount);

	// The following two should be paired...
	// Call enqueueBytesNoCopyStart(), write your bytes into the pointer, then call enqueueBytesNoCopyFinish().

	// This starts an enqueueing operation: it gives you pointers into the buffer where you should
	// stick data and the number of bytes that you may stick there.  Caller is responsible for checking
	// that inByteCount <= getRemainingSpace().  The First and SecondByteCounts will sum to inByteCount.
	// We expose both pointers and lengths at once to facilitate the use of readv()-style routines.
	// *outSecondBytes == NULL && *outSecondByteCount == 0 if the second chunk is unneeded.
	// You may write fewer than inByteCount bytes; specify how many were actually written when
	// calling enqueueBytesNoCopyFinish().
	// Any of the out-pointers may be passed as NULL if you don't care about the returned value.
	void enqueueBytesNoCopyStart(unsigned int inByteCount, void** outFirstBytes, unsigned int* outFirstByteCount,
					void** outSecondBytes, unsigned int* outSecondByteCount);

	// This finishes an enqueueing operation: you tell it how many bytes you actually enqueued.
	// This cannot be rolled into the above, as proper operation for the data structure requires that
	// data be written before the write index is advanced.  It is legal to write fewer bytes than
	// you said you planned to at the start.
	void enqueueBytesNoCopyFinish(unsigned int inActualByteCount);

	// Circular buffer may have incoming or outgoing data splitting across the "seam"
	// so this routine returns the length of the chunk at the starting index "first" and
	// the length of the chunk needed from the beginning of the buffer to satisfy inByteCount.
	// (If the chunk does not need splitting, the result's first == inByteCount and second == 0.)
	// This is static since it can be, basically; others clients (outside this class) could use it if they wanted to...
	// I'm still not totally happy with it because it seems it'd be too easy to mix up the parameters,
	// but OTOH using a structure for the parameters seems too clunky for such a simple routine...
	static std::pair<unsigned int, unsigned int> splitIntoChunks(unsigned int inByteCount, unsigned int inStartingIndex, unsigned int inQueueSize);

protected:
};

#endif // CIRCULAR_BYTE_BUFFER_H
