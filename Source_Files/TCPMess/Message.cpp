/*
 *  Message.cpp
 *  Created by Woody Zenfell, III on Sun Aug 31 2003.
 */

/*
  Copyright (c) 2003, Woody Zenfell, III

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

#if !defined(DISABLE_NETWORKING)

#include "Message.h"

#include <string.h>	// memcpy
#include <vector>

#include "AStream.h"



enum
{
	kSmallMessageBufferSize = 4 * 1024
};

bool
SmallMessageHelper::inflateFrom(const UninflatedMessage& inUninflated)
{
	AIStreamBE	theStream(inUninflated.buffer(), inUninflated.length());
	return reallyInflateFrom(theStream);
}



UninflatedMessage*
SmallMessageHelper::deflate() const
{
	std::vector<byte> theBuffer(kSmallMessageBufferSize);
	AOStreamBE	theStream(&(theBuffer[0]), theBuffer.size());
	reallyDeflateTo(theStream);
	UninflatedMessage* theDeflatedMessage = new UninflatedMessage(type(), theStream.tellp());
	memcpy(theDeflatedMessage->buffer(), &(theBuffer[0]), theDeflatedMessage->length());
	return theDeflatedMessage;
}



BigChunkOfDataMessage::BigChunkOfDataMessage(MessageTypeID inType, const byte* inBuffer, size_t inLength)
	: mType(inType), mLength(0), mBuffer(NULL)
{
	copyBufferFrom(inBuffer, inLength);
}



bool
BigChunkOfDataMessage::inflateFrom(const UninflatedMessage& inUninflated)
{
	copyBufferFrom(inUninflated.buffer(), inUninflated.length());
	return true;
}



UninflatedMessage*
BigChunkOfDataMessage::deflate() const
{
	UninflatedMessage* theMessage = new UninflatedMessage(type(), length());
	memcpy(theMessage->buffer(), buffer(), length());
	return theMessage;
}



void
BigChunkOfDataMessage::copyBufferFrom(const byte* inBuffer, size_t inLength)
{
	delete [] mBuffer;
	mLength = inLength;
	if(mLength > 0)
	{
		mBuffer = new byte[mLength];
		memcpy(mBuffer, inBuffer, mLength);
	}
	else
	{
		mBuffer = NULL;
	}
}



BigChunkOfDataMessage*
BigChunkOfDataMessage::clone() const
{
	return new BigChunkOfDataMessage(type(), buffer(), length());
}



BigChunkOfDataMessage::~BigChunkOfDataMessage()
{
	delete [] mBuffer;
}

#endif // !defined(DISABLE_NETWORKING)
