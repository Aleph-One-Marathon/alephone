/*
 *  CircularByteBuffer.h
 *  created for Marathon: Aleph One <http://source.bungie.org/>

	Copyright (C) 2003 Woody Zenfell, III

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

 *  A circular queue of bytes, with support for mass enqueueing from/peeking to caller's buffer
 *
 *  Created by Woody Zenfell, III on Sun Jun 29 2003.
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

protected:
	// Circular buffer may have incoming or outgoing data splitting across the "seam"
	// so this routine returns the length of the chunk at the starting index "first" and
	// the length of the chunk needed from the beginning of the buffer to satisfy inByteCount.
	// (If the chunk does not need splitting, the result's first == inByteCount and second == 0.)
	std::pair<unsigned int, unsigned int> splitIntoChunks(unsigned int inByteCount, unsigned int inStartingIndex);
};

#endif // CIRCULAR_BYTE_BUFFER_H
