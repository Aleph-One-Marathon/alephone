/*
 *  CommunicationsChannel.cpp
 *  Created by Woody Zenfell, III on Mon Sep 01 2003.
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

#include "CommunicationsChannel.h"
#include "AStream.h"
#include "MessageInflater.h"
#include "MessageHandler.h"
#include "network.h"
#include <stdlib.h>
#include <iostream> // debugging
#include <cerrno>
#include "cseries.h"
#include <algorithm>

enum
{
	// If any incoming message claims to be longer than this, we bail
	kMaximumMessageLength = 4 * 1024 * 1024,

	// Milliseconds we wait between pump() calls during receive[Specific]Message()
	kSSRPumpInterval = 50,

	// Milliseconds we wait between pump() calls during flushOutgoingMessages()
	kFlushPumpInterval = kSSRPumpInterval,
};

CommunicationsChannel::CommunicationsChannel()
	: mConnected(false),
	mSocket(nullptr),
	mMessageInflater(NULL),
	mMessageHandler(NULL),
	mMemento(NULL),
	mIncomingHeaderPosition(0),
	mIncomingMessage(NULL),
	mIncomingMessagePosition(0),
	mOutgoingHeaderPosition(0),
	mOutgoingMessagePosition(0)
{
	mTicksAtLastReceive = machine_tick_count();
	mTicksAtLastSend = machine_tick_count();
}



CommunicationsChannel::CommunicationsChannel(std::unique_ptr<TCPsocket> inSocket)
	: mConnected(inSocket != NULL),
	mSocket(std::move(inSocket)),
	mMessageInflater(NULL),
	mMessageHandler(NULL),
	mMemento(NULL),
	mIncomingHeaderPosition(0),
	mIncomingMessage(NULL),
	mIncomingMessagePosition(0),
	mOutgoingHeaderPosition(0),
	mOutgoingMessagePosition(0)
{
	mTicksAtLastReceive = machine_tick_count();
	mTicksAtLastSend = machine_tick_count();
}



CommunicationsChannel::~CommunicationsChannel()
{
    disconnect();
}



CommunicationsChannel::CommunicationResult
CommunicationsChannel::receive_some(byte* inBuffer, size_t& ioBufferPosition, size_t inBufferLength)
{
	if (inBufferLength == 0) return kComplete;

	size_t theBytesLeft = inBufferLength - ioBufferPosition;

	if(theBytesLeft > 0)
	{
		int theResult = mSocket->receive(inBuffer + ioBufferPosition, theBytesLeft);

		if(theResult < 0)
		{
			disconnect();
			return kError;
		}
		
		if(theResult > 0)
		{
			mTicksAtLastReceive = machine_tick_count();
		}
	
		ioBufferPosition += theResult;
	} // if we actually expect to receive something
	
	return (ioBufferPosition == inBufferLength) ? kComplete : kIncomplete;
}



CommunicationsChannel::CommunicationResult
CommunicationsChannel::send_some(byte* inBuffer, size_t& ioBufferPosition, size_t inBufferLength)
{	
	size_t theBytesLeft = inBufferLength - ioBufferPosition;

	int theResult = mSocket->send(inBuffer + ioBufferPosition, theBytesLeft);

	if(theResult < 0)
	{
		disconnect();
		return kError;
	}
	else
	{
		if(theResult > 0)
			mTicksAtLastSend = machine_tick_count();
		
		ioBufferPosition += theResult;
		return (ioBufferPosition == inBufferLength) ? kComplete : kIncomplete;
	}
}



bool
CommunicationsChannel::receiveHeader()
{
	CommunicationResult theResult =
		receive_some(mIncomingHeader, mIncomingHeaderPosition, kHeaderPackedSize);

	if(theResult == kComplete)
	{
		// Finished receiving a header
		AIStreamBE theHeaderStream(mIncomingHeader, kHeaderPackedSize);

		uint16 theMagic;
		uint16 theMessageType;
		uint32 theMessageLength;

		theHeaderStream >> theMagic
			>> theMessageType
			>> theMessageLength;

		// Incoming length includes header length
		theMessageLength -= kHeaderPackedSize;

		if(theMagic != kHeaderMagic || theMessageLength > kMaximumMessageLength)
		{
			disconnect();
		}
		else
		{
			// Successfully received a valid header; switch to receive-message mode
			mIncomingMessage = new UninflatedMessage(theMessageType, theMessageLength);
			mIncomingMessagePosition = 0;
		}

		// We should try to receive more stuff, since we got all we asked for.
		return true;
	}
	else
	{
		// We got less than we wanted - no sense in trying for more.
		return false;
	}
}



bool
CommunicationsChannel::_receiveMessage()
{
	CommunicationResult theResult =
		receive_some(mIncomingMessage->buffer(), mIncomingMessagePosition, mIncomingMessage->length());

	if(theResult == kComplete)
	{
		// Received a complete message; inflate (if possible) then enqueue it
		Message* theMessageToEnqueue = mIncomingMessage;

		if(mMessageInflater != NULL)
		{
			theMessageToEnqueue = mMessageInflater->inflate(*mIncomingMessage);
			delete mIncomingMessage;
		}

		mIncomingMessages.push_back(theMessageToEnqueue);

		// No longer receiving message body - prepare to receive next header
		mIncomingMessage = NULL;
		mIncomingHeaderPosition = 0;

		// We got all we wanted - so we should go again to see if there's more.
		return true;
	}
	else
	{
		// Ran out of data to receive, or error - no sense looking for more data
		return false;
	}
}



bool
CommunicationsChannel::sendHeader()
{
	CommunicationResult theResult =
		send_some(mOutgoingHeader, mOutgoingHeaderPosition, kHeaderPackedSize);

	if(theResult == kComplete)
	{
		// Finished sending a header; switch to sending message now
		mOutgoingMessagePosition = 0;
		
		// We should try to send more stuff, since we sent all we asked to.
		return true;
	}
	else
	{
		// We sent less than we wanted - no sense in trying for more.
		return false;
	}	
}



bool
CommunicationsChannel::sendMessage()
{
	UninflatedMessage* theOutgoingMessage = mOutgoingMessages.front();
	
	CommunicationResult theResult =
		send_some(theOutgoingMessage->buffer(), mOutgoingMessagePosition, theOutgoingMessage->length());
	
	if(theResult == kComplete)
	{
		// Sent a complete message; delete and dequeue it
		delete theOutgoingMessage;
		mOutgoingMessages.pop_front();
	
		// No longer sending message body - prepare to send next header
		mOutgoingHeaderPosition = 0;
	
		// We sent all we wanted - so we should go again to see if we can do more.
		return true;
	}
	else
	{
		// Could not send it all, or error - no sense trying for more data
		return false;
	}
}



void
CommunicationsChannel::pumpReceivingSide()
{
	bool keepGoing = true;
	while(keepGoing && mConnected)
	{
		if(mIncomingMessage != NULL)
		{
			// Already working on receiving message body
			keepGoing = _receiveMessage();
		}
		else
		{
			// Not receiving message body - must be receiving message header then
			keepGoing = receiveHeader();
		}
	}
}



void
CommunicationsChannel::pumpSendingSide()
{
	bool keepGoing = true;
	while(keepGoing && mConnected && !mOutgoingMessages.empty())
	{
		if(mOutgoingHeaderPosition == 0)
		{
			// Need to fill packed header buffer with packed header
			// We may end up doing this more than once if for some reason we can't
			// send any data bytes to TCP ... but that's OK.
			UninflatedMessage* theMessage = mOutgoingMessages.front();
			AOStreamBE theHeaderStream(mOutgoingHeader, kHeaderPackedSize);
			theHeaderStream << (Uint16)kHeaderMagic
				<< theMessage->inflatedType()
				<< (uint32)(theMessage->length() + kHeaderPackedSize);
		}

		if(mOutgoingHeaderPosition < kHeaderPackedSize)
		{
			keepGoing = sendHeader();
		}
		else
		{
			keepGoing = sendMessage();
		}
	}
}



void
CommunicationsChannel::pump()
{
	pumpSendingSide();
	pumpReceivingSide();
}

bool CommunicationsChannel::dispatchOneIncomingMessage() 
{
  if (mIncomingMessages.empty()) return false;
  Message* theMessage = mIncomingMessages.front();
  if (messageHandler() != NULL) {
    messageHandler()->handle(theMessage, this);
  }
  delete theMessage;
  mIncomingMessages.pop_front();
  return true;
}

void
CommunicationsChannel::dispatchIncomingMessages()
{
  while (dispatchOneIncomingMessage());
}



void
CommunicationsChannel::enqueueOutgoingMessage(const Message& inMessage)
{
	if(isConnected())
	{
		UninflatedMessage* theUninflatedMessage = inMessage.deflate();
		mOutgoingMessages.push_back(theUninflatedMessage);
	}
}

IPaddress
CommunicationsChannel::peerAddress() const
{
	return mSocket->remote_address();
}

void
CommunicationsChannel::connect(const IPaddress& inAddress)
{
	assert(!isConnected());

	mIncomingHeaderPosition = 0;
	mIncomingMessagePosition = 0;
	delete mIncomingMessage;
	mIncomingMessage = 0;

	for(MessageQueue::iterator i = mIncomingMessages.begin(); i != mIncomingMessages.end(); ++i)
		delete *i;
	
	mIncomingMessages.clear();

	mSocket = NetGetNetworkInterface()->tcp_connect_socket(inAddress);

	if(mSocket)
	{
		mSocket->set_non_blocking(true);
		mConnected = true;
		mTicksAtLastReceive = machine_tick_count();
		mTicksAtLastSend = machine_tick_count();
	}
}



void
CommunicationsChannel::connect(const std::string& inAddressString, uint16 inPort)
{
	auto address = NetGetNetworkInterface()->resolve_address(inAddressString, inPort);
	if (address.has_value()) connect(address.value());
}



void
CommunicationsChannel::disconnect()
{
	mSocket.reset();
	mConnected = false;

    // Discard all data so next connect()ion starts with a clean slate
    mOutgoingHeaderPosition = 0;
    mOutgoingMessagePosition = 0;

    for(UninflatedMessageQueue::iterator i = mOutgoingMessages.begin(); i != mOutgoingMessages.end(); ++i)
        delete *i;

    mOutgoingMessages.clear();
}



bool
CommunicationsChannel::isMessageAvailable()
{
	pump();

	return !mIncomingMessages.empty();
}



// Call does not return unless (1) times out (NULL); (2) disconnected (NULL); or
// (3) some message received (pointer to inflated message object).
Message*
CommunicationsChannel::receiveMessage(Uint32 inOverallTimeout, Uint32 inInactivityTimeout)
{
	// Here we give a backstop for our inactivity timeout
	Uint32 theTicksAtStart = machine_tick_count();
	
	Uint32 theDeadline = machine_tick_count() + inOverallTimeout;

	pump();

	while(machine_tick_count() - std::max(mTicksAtLastReceive, theTicksAtStart) < inInactivityTimeout
		&& machine_tick_count() < theDeadline
		&& isConnected()
		&& mIncomingMessages.empty())
	{
		sleep_for_machine_ticks(kSSRPumpInterval);
		pump();
	}

	Message* theMessage = NULL;

	if(!mIncomingMessages.empty())
	{
		theMessage = mIncomingMessages.front();
		mIncomingMessages.pop_front();
	}

	return theMessage;
}



// As above, but if messages of type other than inType are received, they're handled
// normally (so might want to install conservative Handler first)
Message*
CommunicationsChannel::receiveSpecificMessage(
	MessageTypeID inType,
	Uint32 inOverallTimeout,
	Uint32 inInactivityTimeout)
{
	Message* theMessage = NULL;
	Uint32 theDeadline = machine_tick_count() + inOverallTimeout;

	while(machine_tick_count() < theDeadline)
	{
		theMessage = receiveMessage(theDeadline - machine_tick_count(), inInactivityTimeout);
		
		if(theMessage)
		{
			if(theMessage->type() == inType)
				// Got our message
				break;
			else
			{
				// Got some other message - handle it and destroy it
				if(messageHandler() != NULL)
				{
					messageHandler()->handle(theMessage, this);
				}
				delete theMessage;
				theMessage = NULL;
			}
		}
		else
			// Other routine timed out or got disconnected
			break;
	}
	
	return theMessage;
}



void
CommunicationsChannel::flushOutgoingMessages(bool shouldDispatchIncomingMessages,
			    Uint32 inOverallTimeout,
			    Uint32 inInactivityTimeout)
{
	Uint32	theDeadline = machine_tick_count() + inOverallTimeout;
	Uint32	theTicksAtStart = machine_tick_count();

	while(isConnected()
		&& !mOutgoingMessages.empty()
		&& machine_tick_count() < theDeadline
		&& machine_tick_count() - std::max(mTicksAtLastSend, theTicksAtStart) < inInactivityTimeout)
	{
		sleep_for_machine_ticks(kFlushPumpInterval);
		pump();
		if(shouldDispatchIncomingMessages)
			dispatchIncomingMessages();
	}
}


void CommunicationsChannel::multipleFlushOutgoingMessages(
	std::vector<CommunicationsChannel *>& channels,
	bool shouldDispatchIncomingMessages,
	Uint32 inOverallTimeout,
	Uint32 inInactivityTimeout)
{
	Uint32 theDeadline = machine_tick_count() + inOverallTimeout;
	Uint32 theTicksAtStart = machine_tick_count();

	bool someoneIsStillActive = true;

	while (machine_tick_count() < theDeadline && someoneIsStillActive)
	{
		someoneIsStillActive = false;

		sleep_for_machine_ticks(kFlushPumpInterval);

		for (std::vector<CommunicationsChannel*>::iterator it = channels.begin(); it != channels.end(); it++)
		{
			if (!(*it)->mOutgoingMessages.empty() && machine_tick_count() - std::max((*it)->mTicksAtLastSend, theTicksAtStart) < inInactivityTimeout)
			{
				someoneIsStillActive = true;
			}

			(*it)->pump();
			if (shouldDispatchIncomingMessages)
				(*it)->dispatchIncomingMessages();
		}

	}

}


CommunicationsChannelFactory::CommunicationsChannelFactory(uint16 inPort)
{
	mSocketListener = NetGetNetworkInterface()->tcp_open_listener(inPort);
	mSocketListener->set_non_blocking(true);
}


CommunicationsChannel*
CommunicationsChannelFactory::newIncomingConnection()
{
	if (auto connection = mSocketListener->accept_connection())
	{
		connection->set_non_blocking(true);
		return new CommunicationsChannel(std::move(connection));
	}

	return nullptr;
}

#endif // !defined(DISABLE_NETWORKING)

