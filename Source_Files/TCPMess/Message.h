/*
 *  Message.h
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

#ifndef MESSAGE_H
#define MESSAGE_H

#include <string.h>	// memcpy
#include "SDL.h"

typedef Uint16 MessageTypeID;

class UninflatedMessage;

class Message
{
public:
	virtual	MessageTypeID		type() const = 0;

	// May return false or raise an exception on failed inflation
	virtual	bool			inflateFrom(const UninflatedMessage& inUninflated) = 0;

	// Caller must dispose of returned message via 'delete'
	virtual	UninflatedMessage*	deflate() const = 0;

	virtual Message*		clone() const = 0;

	virtual ~Message() {}

protected:
};



class UninflatedMessage : public Message
{
public:
	enum { kTypeID = 0xffff };

	// If bytes are provided, this object takes ownership of them (does not copy).
	// If no bytes are provided, this object creates a buffer of size inLength.
	//    Clients should write into the pointer returned by buffer().
	UninflatedMessage(MessageTypeID inType, size_t inLength, Uint8* inBytes = NULL)
		: mType(inType), mLength(inLength), mBuffer(inBytes)
	{
		if(mBuffer == NULL)
			mBuffer = new Uint8[mLength];
	}

	UninflatedMessage(const UninflatedMessage& inSource) { copyToThis(inSource); }

	UninflatedMessage& operator =(const UninflatedMessage& inSource)
	{
		if(&inSource != this)
			copyToThis(inSource);

		return *this;
	}
	
	MessageTypeID		type() const	{ return kTypeID; }
	bool			inflateFrom(const UninflatedMessage& inUninflated) { *this = inUninflated; return true; }
	UninflatedMessage*	deflate() const { return new UninflatedMessage(*this); }
	
	UninflatedMessage* clone() const { return new UninflatedMessage(*this); }

	~UninflatedMessage()	{ delete [] mBuffer; }

	MessageTypeID	inflatedType() const	{ return mType; }
	size_t		length() const		{ return mLength; }
	Uint8*		buffer()		{ return mBuffer; }
	const Uint8*	buffer() const		{ return mBuffer; }

private:
	void copyToThis(const UninflatedMessage& inSource)
	{
		mType	= inSource.mType;
		mLength	= inSource.mLength;
		mBuffer	= new Uint8[mLength];
		memcpy(mBuffer, inSource.mBuffer, mLength);
	}
		
	MessageTypeID	mType;
	size_t		mLength;
	Uint8*		mBuffer;
};



class AIStream;
class AOStream;

class SmallMessageHelper : public Message
{
public:
	bool			inflateFrom(const UninflatedMessage& inUninflated);
	UninflatedMessage*	deflate() const;

protected:
	virtual bool	reallyInflateFrom(AIStream& inStream) = 0;
	virtual void	reallyDeflateTo(AOStream& inStream) const = 0;

private:
};



class BigChunkOfDataMessage : public Message
{
public:
	BigChunkOfDataMessage(MessageTypeID inType, const Uint8* inBuffer = NULL, size_t inLength = 0);
	BigChunkOfDataMessage(const BigChunkOfDataMessage& other) : mLength(0), mBuffer(NULL)
	{
		mType = other.type();
		copyBufferFrom(other.buffer(), other.length());
	}

	BigChunkOfDataMessage& operator =(const BigChunkOfDataMessage& other)
	{
		if(&other != this)
		{
			mType = other.type();
			copyBufferFrom(other.buffer(), other.length());
		}

		return *this;
	}
	
	bool			inflateFrom(const UninflatedMessage& inUninflated);
	UninflatedMessage*	deflate() const;
	MessageTypeID		type() const	{ return mType; }

	void			copyBufferFrom(const Uint8* inBuffer, size_t inLength);
	
	size_t			length() const	{ return mLength; }
	Uint8*			buffer()	{ return mBuffer; }
	const Uint8*		buffer() const	{ return mBuffer; }
	
	BigChunkOfDataMessage*	clone() const;

	~BigChunkOfDataMessage();
	
private:
	MessageTypeID	mType;
	size_t		mLength;
	Uint8*		mBuffer;
};



template <typename tValueType>
class SimpleMessage : public SmallMessageHelper
{
public:
	typedef tValueType ValueType;

	SimpleMessage(MessageTypeID inType)
		: mType(inType)
	{
		// Use default initializer for value
		new (static_cast<void*>(&mValue)) tValueType();
	}
	
	SimpleMessage(MessageTypeID inType, const tValueType& inValue)
		: mType(inType), mValue(inValue)
	{
	}

	SimpleMessage<tValueType>* clone() const
		{ return new SimpleMessage<tValueType>(*this); }

	MessageTypeID type() const { return mType; }

	void setValue(const tValueType& inValue) { mValue = inValue; }
	const tValueType& value() const { return mValue; }

protected:
	void	reallyDeflateTo(AOStream& inStream) const
	{
		inStream << mValue;
	}

	bool	reallyInflateFrom(AIStream& inStream)
	{
		inStream >> mValue;
		return true;
	}

private:
	MessageTypeID	mType;
	tValueType	mValue;
};



template <MessageTypeID tMessageType>
class DatalessMessage : public Message
{
public:
	enum { kType = tMessageType };

	MessageTypeID type() const { return kType; }

	bool inflateFrom(const UninflatedMessage& inUninflated)
	{
		return inUninflated.inflatedType() == kType && inUninflated.length() == 0;
	}

	UninflatedMessage* deflate() const { return new UninflatedMessage(kType, 0); }
	
	DatalessMessage<tMessageType>* clone() const
	{ return new DatalessMessage<tMessageType>; }
};

#endif // MESSAGE_H
