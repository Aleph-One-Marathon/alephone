/*
 *  MessageDispatcher.h
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


#ifndef MESSAGEDISPATCHER_H
#define MESSAGEDISPATCHER_H

#include <map>

#include "Message.h"
#include "MessageHandler.h"


class MessageDispatcher : public MessageHandler
{
public:
	MessageDispatcher() : mDefaultHandler(NULL) {}
	
	void setHandlerForType(MessageHandler* inHandler, MessageTypeID inType)
	{
		if(inHandler == NULL)
			clearHandlerForType(inType);
		else
			mMap[inType] = inHandler;
	}

	MessageHandler* handlerForType(MessageTypeID inType)
	{
		MessageHandler* theHandler = mDefaultHandler;

		MessageDispatcherMap::iterator i = mMap.find(inType);
		if(i != mMap.end())
			theHandler = i->second;

		return theHandler;
	}

	MessageHandler* handlerForTypeNoDefault(MessageTypeID inType)
	{
		MessageDispatcherMap::iterator i = mMap.find(inType);
		return (i != mMap.end()) ? i->second : NULL;
	}

	void clearHandlerForType(MessageTypeID inType)
	{
		mMap.erase(inType);
	}

	void setDefaultHandler(MessageHandler* inHandler)
	{
		mDefaultHandler = inHandler;
	}

	MessageHandler* defaultHandler() const
	{
		return mDefaultHandler;
	}

	void handle(Message* inMessage, CommunicationsChannel* inChannel)
	{
		MessageHandler* theHandler = handlerForType(inMessage->type());

		if(theHandler != NULL)
			theHandler->handle(inMessage, inChannel);
	}

private:
	typedef std::map<MessageTypeID, MessageHandler*> MessageDispatcherMap;
	
	MessageDispatcherMap	mMap;
	MessageHandler*		mDefaultHandler;
};

#endif // MESSAGEDISPATCHER_H
