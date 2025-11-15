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
#include "Logging.h"

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
		auto receive_result = mSocket->receive(inBuffer + ioBufferPosition, theBytesLeft);

		if (!receive_result.has_value())
		{
			const auto& error = receive_result.error();
			logError("couldn't receive udp packet: error code [%d] message [%s]", error.code, error.message.c_str());
			disconnect();
			return kError;
		}

		auto receive_value = receive_result.value();
		
		if (receive_value > 0)
		{
			mTicksAtLastReceive = machine_tick_count();
		}
	
		ioBufferPosition += receive_value;
	} // if we actually expect to receive something
	
	return (ioBufferPosition == inBufferLength) ? kComplete : kIncomplete;
}



CommunicationsChannel::CommunicationResult
CommunicationsChannel::send_some(byte* inBuffer, size_t& ioBufferPosition, size_t inBufferLength)
{	
	size_t theBytesLeft = inBufferLength - ioBufferPosition;

	auto send_result = mSocket->send(inBuffer + ioBufferPosition, theBytesLeft);

	if (!send_result.has_value())
	{
		const auto& error = send_result.error();
		logError("couldn't send udp packet: error code [%d] message [%s]", error.code, error.message.c_str());
		disconnect();
		return kError;
	}
	else
	{
		auto send_value = send_result.value();

		if (send_value > 0)
			mTicksAtLastSend = machine_tick_count();
		
		ioBufferPosition += send_value;
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

	auto connect_socket_result = NetGetNetworkInterface()->tcp_connect_socket(inAddress);
	if (!connect_socket_result.has_value())
	{
		const auto& error = connect_socket_result.error();
		logError("couldn't connect socket: error code [%d] message [%s]", error.code, error.message.c_str());
		return;
	}

	mSocket = std::move(connect_socket_result.value());
	mConnected = true;
	mTicksAtLastReceive = machine_tick_count();
	mTicksAtLastSend = machine_tick_count();

	auto non_blocking_result = mSocket->set_non_blocking(true);

	if (!non_blocking_result.has_value())
	{
		const auto& error = non_blocking_result.error();
		logError("couldn't set socket to non blocking mode: error code [%d] message [%s]", error.code, error.message.c_str());
	}
}



void
CommunicationsChannel::connect(const std::string& inAddressString, uint16 inPort)
{
	auto resolve_address_result = NetGetNetworkInterface()->resolve_address(inAddressString, inPort);

	if (!resolve_address_result.has_value()) 
	{
		const auto& error = resolve_address_result.error();
		logError("couldn't resolve ip address: error code [%d] message [%s]", error.code, error.message.c_str());
		return;
	}

	auto resolved_address = resolve_address_result.value();

	if (!resolved_address.has_value())
	{
		logError("no valid/supported address could be resolved");
		return;
	}

	connect(resolved_address.value());
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
	uint64_t theTicksAtStart = machine_tick_count();
	
	uint64_t theDeadline = machine_tick_count() + inOverallTimeout;

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
	uint64_t theDeadline = machine_tick_count() + inOverallTimeout;

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
	uint64_t	theDeadline = machine_tick_count() + inOverallTimeout;
	uint64_t	theTicksAtStart = machine_tick_count();

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
	uint64_t theDeadline = machine_tick_count() + inOverallTimeout;
	uint64_t theTicksAtStart = machine_tick_count();

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
	auto open_listener_result = NetGetNetworkInterface()->tcp_open_listener(inPort);

	if (!open_listener_result.has_value())
	{
		const auto& error = open_listener_result.error();
		logError("couldn't open tcp listener: error code [%d] message [%s]", error.code, error.message.c_str());
		return;
	}

	mSocketListener = std::move(open_listener_result.value());

	auto non_blocking_result = mSocketListener->set_non_blocking(true);

	if (!non_blocking_result.has_value())
	{
		const auto& error = non_blocking_result.error();
		logError("couldn't set tcp listener socket to non blocking mode: error code [%d] message [%s]", error.code, error.message.c_str());
	}
}


CommunicationsChannel*
CommunicationsChannelFactory::newIncomingConnection()
{
	auto accept_connection_result = mSocketListener->accept_connection();
	if (!accept_connection_result.has_value())
	{
		const auto& error = accept_connection_result.error();
		logError("couldn't check for new incoming connection: error code [%d] message [%s]", error.code, error.message.c_str());
		return nullptr;
	}

	auto& connection = accept_connection_result.value();
	if (!connection) return nullptr; //no incoming connection

	auto non_blocking_result = connection->set_non_blocking(true);

	if (!non_blocking_result.has_value())
	{
		const auto& error = non_blocking_result.error();
		logError("couldn't set new incoming connection socket to non blocking mode: error code [%d] message [%s]", error.code, error.message.c_str());
	}

	return new CommunicationsChannel(std::move(connection));
}

#endif // !defined(DISABLE_NETWORKING)

