/*
 *  MessageHandler.h
 *  Created by Woody Zenfell, III on Thu Sep 11 2003.
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

#ifndef MESSAGEHANDLER_H
#define MESSAGEHANDLER_H

#include <unistd.h>

class Message;
class CommunicationsChannel;

class MessageHandler
{
public:
	virtual void handle(Message* inMessage, CommunicationsChannel* inChannel) = 0;
	virtual ~MessageHandler() {}
};



class MessageHandlerFunction : public MessageHandler
{
public:
	typedef void (*FunctionType)(Message* inMessage, CommunicationsChannel* inChannel);

	MessageHandlerFunction(FunctionType inFunction)
		: mFunction(inFunction)
	{}
	
	void handle(Message* inMessage, CommunicationsChannel* inChannel)
	{
		if(mFunction != NULL)
			mFunction(inMessage, inChannel);
	}

	FunctionType	function() const { return mFunction; }
	void	setFunction(FunctionType inFunction) { mFunction = inFunction; }

private:
	FunctionType	mFunction;
};

#endif // MESSAGEHANDLER_H
