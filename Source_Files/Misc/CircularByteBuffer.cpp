/*
 *  CircularByteBuffer.cpp
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

#include "cseries.h" // assert()

#include "CircularByteBuffer.h"

#include <algorithm> // std::min()

// static
std::pair<unsigned int, unsigned int>
CircularByteBuffer::splitIntoChunks(unsigned int inByteCount, unsigned int inStartingIndex, unsigned int inQueueSize)
{
	// Copy, potentially, two separate chunks (one at end of buffer; one at beginning)
	unsigned int theSpaceAtEndOfBuffer = inQueueSize - inStartingIndex;
	unsigned int theFirstChunkSize = std::min(inByteCount, theSpaceAtEndOfBuffer);
	unsigned int theSecondChunkSize = inByteCount - theFirstChunkSize;

	return std::pair<unsigned int, unsigned int>(theFirstChunkSize, theSecondChunkSize);
}


void
CircularByteBuffer::enqueueBytes(const void* inBytes, unsigned int inByteCount)
{
	// I believe everything works right without this check, but it makes me feel safer anyway.
	if(inByteCount > 0)
	{
		assert(inByteCount <= getRemainingSpace());
	
		const char* theBytes = static_cast<const char*>(inBytes);
	
		std::pair<unsigned int, unsigned int> theChunkSizes = splitIntoChunks(inByteCount, mWriteIndex, mQueueSize);
	
		memcpy(&(mData[mWriteIndex]), theBytes, theChunkSizes.first);
	
		if(theChunkSizes.second > 0)
			memcpy(mData, &(theBytes[theChunkSizes.first]), theChunkSizes.second);
	
		advanceWriteIndex(inByteCount);
	}
}


void
CircularByteBuffer::enqueueBytesNoCopyStart(unsigned int inByteCount, void** outFirstBytes, unsigned int* outFirstByteCount,
			      void** outSecondBytes, unsigned int* outSecondByteCount)
{
	void* theFirstBytes = NULL;
	void* theSecondBytes = NULL;
	unsigned int theFirstByteCount = 0;
	unsigned int theSecondByteCount = 0;
	
	if(inByteCount > 0)
	{
		assert(inByteCount <= getRemainingSpace());

		std::pair<unsigned int, unsigned int> theChunkSizes = splitIntoChunks(inByteCount, mWriteIndex, mQueueSize);

		theFirstBytes = &(mData[getWriteIndex()]);
		theFirstByteCount = theChunkSizes.first;
		theSecondByteCount = theChunkSizes.second;
		theSecondBytes = (theSecondByteCount > 0) ? mData : NULL;
	}

	if(outFirstBytes != NULL)
		*outFirstBytes = theFirstBytes;

	if(outFirstByteCount != NULL)
		*outFirstByteCount = theFirstByteCount;

	if(outSecondBytes != NULL)
		*outSecondBytes = theSecondBytes;

	if(outSecondByteCount != NULL)
		*outSecondByteCount = theSecondByteCount;
}


void
CircularByteBuffer::enqueueBytesNoCopyFinish(unsigned int inActualByteCount)
{
	if(inActualByteCount > 0)
		advanceWriteIndex(inActualByteCount);
}


void
CircularByteBuffer::peekBytes(void* outBytes, unsigned int inByteCount)
{
	// I believe everything works right without this check, but it makes me feel safer anyway.
	if(inByteCount > 0)
	{
		assert(inByteCount <= getCountOfElements());

		char* theBytes = static_cast<char*>(outBytes);
	
		std::pair<unsigned int, unsigned int> theChunkSizes = splitIntoChunks(inByteCount, mReadIndex, mQueueSize);
	
		memcpy(theBytes, &(mData[mReadIndex]), theChunkSizes.first);
	
		if(theChunkSizes.second > 0)
			memcpy(&(theBytes[theChunkSizes.first]), mData, theChunkSizes.second);
	}
}


void
CircularByteBuffer::peekBytesNoCopy(unsigned int inByteCount, const void** outFirstBytes, unsigned int* outFirstByteCount,
				    const void** outSecondBytes, unsigned int* outSecondByteCount)
{
	void* theFirstBytes = NULL;
	void* theSecondBytes = NULL;
	unsigned int theFirstByteCount = 0;
	unsigned int theSecondByteCount = 0;
	
	if(inByteCount > 0)
	{
		assert(inByteCount <= getCountOfElements());

		std::pair<unsigned int, unsigned int> theChunkSizes = splitIntoChunks(inByteCount, mReadIndex, mQueueSize);

		theFirstBytes = &(mData[getReadIndex()]);
		theFirstByteCount = theChunkSizes.first;
		theSecondByteCount = theChunkSizes.second;
		theSecondBytes = (theSecondByteCount > 0) ? mData : NULL;
	}

	if(outFirstBytes != NULL)
		*outFirstBytes = theFirstBytes;

	if(outFirstByteCount != NULL)
		*outFirstByteCount = theFirstByteCount;

	if(outSecondBytes != NULL)
		*outSecondBytes = theSecondBytes;

	if(outSecondByteCount != NULL)
		*outSecondByteCount = theSecondByteCount;	
}
