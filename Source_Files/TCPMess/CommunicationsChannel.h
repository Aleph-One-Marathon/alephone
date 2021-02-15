/*
 *  CommunicationsChannel.h
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

#ifndef COMMUNICATIONSCHANNEL_H
#define COMMUNICATIONSCHANNEL_H

// Failures: on almost any nontrivial failure, the channel simply becomes disconnected.
// In most cases future communication would be meaningless anyway.
// Channels can be created by the caller (for outgoing connections) or by some sort of
// listener/acceptor/factory thingy (for incoming connections).

#include <list>
#include <string>
#include <memory>
#include <stdexcept>
#include <vector>
#include "SDL_net.h"

#include "Message.h"
#include "csmisc.h"

// Client code may subclass this and have the CommunicationsChannel remember
// an instance of a subclass for later retrieval (with safe downcasting via
// dynamic_cast<>())
class Memento
{
public:
	virtual ~Memento() {}
};


class MessageInflater;
class MessageHandler;


class CommunicationsChannel
{
public:
	enum
	{
		// Constants for synchronous-style receiving (receive[Specific]Message())
		kSSRAnyDataTimeout = 10 * 1000,		// if no data received in this many ms
		kSSRAnyMessageTimeout = 30 * 1000,	// if no complete message received in this many ms
		kSSRSpecificMessageTimeout = 30 * 1000,	// if desired message not received in this many ms

		// Constants for flushOutgoingMessages()
		kOutgoingOverallTimeout = 10*60*1000,	// if haven't finished flushing in this many ms
		kOutgoingInactivityTimeout = 10*1000,	// if TCP hasn't accepted any data from us in this many ms
	};
	
	CommunicationsChannel();
	CommunicationsChannel(TCPsocket inSocket);
	virtual ~CommunicationsChannel();  // allow subclassing (for extension purposes only - no overriding)

	// Each of these is an association - no ownership (for disposal purposes) is implied
	void		setMessageInflater(MessageInflater* inInflater) { mMessageInflater = inInflater; }
	MessageInflater* messageInflater() const { return mMessageInflater; }

	void		setMessageHandler(MessageHandler* inHandler) { mMessageHandler = inHandler; }
	MessageHandler*	messageHandler() const { return mMessageHandler; }

	void		setMemento(Memento* inMemento) { mMemento = inMemento; }
	Memento*	memento() const { return mMemento; }

	// Moves data around but does not callback handlers
	void		pump();

	// Calls back message handler (if appropriate)
	// returns false if there are no messages to dispatch
	bool            dispatchOneIncomingMessage();

	// Calls back message handlers (if appropriate)
	void		dispatchIncomingMessages();

	bool		isMessageAvailable();

	// Call does not return unless (1) times out (NULL); (2) disconnected (NULL); or
	// (3) some message received (pointer to inflated message object).
	// Caller is responsible for deleting the returned object!
	// Timeouts are in milliseconds.  'Overall' timeouts limit the amount of time
	// spent waiting for a message (or a specific message, below) to be completely
	// received.  'Inactivity' timeouts limit the amount of time that may pass without
	// any incoming data at all being found on the underlying connection.
	Message*	receiveMessage(Uint32 inOverallTimeout = kSSRAnyMessageTimeout,
				Uint32 inInactivityTimeout = kSSRAnyDataTimeout);

	// As above, but if messages of type other than inType are received, they're handled
	// normally (so might want to install conservative Handler first)
	Message*	receiveSpecificMessage(MessageTypeID inType,
				 Uint32 inOverallTimeout = kSSRSpecificMessageTimeout,
				 Uint32 inInactivityTimeout = kSSRAnyDataTimeout);

	template <typename tMessage>
	tMessage*	receiveSpecificMessage(MessageTypeID inType,
				  Uint32 inOverallTimeout = kSSRSpecificMessageTimeout,
				  Uint32 inInactivityTimeout = kSSRAnyDataTimeout)
	{
		std::unique_ptr<Message> receivedMessage(receiveSpecificMessage(inType, inOverallTimeout, inInactivityTimeout));
		tMessage* result = dynamic_cast<tMessage*>(receivedMessage.get());
		if(result != NULL)
			receivedMessage.release();
		return result;
	}

	template <typename tMessage>
	tMessage*	receiveSpecificMessage(Uint32 inOverallTimeout = kSSRSpecificMessageTimeout,
				  Uint32 inInactivityTimeout = kSSRAnyDataTimeout)
	{
		return receiveSpecificMessage<tMessage>(tMessage::kType, inOverallTimeout, inInactivityTimeout);
	}

	class FailedToReceiveSpecificMessageException : public std::runtime_error
	{
	public:
		FailedToReceiveSpecificMessageException()
			: std::runtime_error("Did not receive message of expected specific type")
		{}
	};
	
	template <typename tMessage>
	tMessage*	receiveSpecificMessageOrThrow(Uint32 inOverallTimeout = kSSRSpecificMessageTimeout,
				Uint32 inInactivityTimeout = kSSRAnyDataTimeout)
	{
		tMessage* result = receiveSpecificMessage<tMessage>(inOverallTimeout, inInactivityTimeout);
		if (result == NULL)
			throw FailedToReceiveSpecificMessageException();
		return result;
	}

	// This doesn't return until timeout, disconnection, or all queued outgoing messages have
	// been delivered to TCP.  Incoming messages are dispatched iff dispatchIncomingMessages == true.
	void		flushOutgoingMessages(bool dispatchIncomingMessages,
			     Uint32 inOverallTimeout = kOutgoingOverallTimeout,
			     Uint32 inInactivityTimeout = kOutgoingInactivityTimeout);

	// similar to above, but more efficient when there are multiple
	// channels with outgoing messages (usually the case)
	static void     multipleFlushOutgoingMessages(
		std::vector<CommunicationsChannel*>&, 
		bool dispatchIncomingMessages,
		Uint32 inOverallTimeout = kOutgoingInactivityTimeout,
		Uint32 inInactivityTimeout = kOutgoingInactivityTimeout);
	
	// Copies the given message (or at least its bytes) to make use less error-prone
	void		enqueueOutgoingMessage(const Message& inMessage);

	bool		isConnected() const { return mConnected; }

	// inPort should be in host byte order
	void		connect(const std::string& inAddressString, Uint16 inPort);

	// inAddress.port should be in network (big-endian) byte order
	void		connect(const IPaddress& inAddress);
	
	void		disconnect();

	IPaddress	peerAddress() const;

	// Callers can use these (compared with machine_tick_count()) to gauge activity on the Channel:
	// each time pump() receives/sends new data, value is set to machine_tick_count() at that time.
	Uint32		ticksAtLastReceive() const { return mTicksAtLastReceive; }
	Uint32		ticksAtLastSend() const { return mTicksAtLastSend; }

	// Or callers can just use these.
	Uint32		millisecondsSinceLastReceive() const { return machine_tick_count() - mTicksAtLastReceive; }
	Uint32		millisecondsSinceLastSend() const { return machine_tick_count() - mTicksAtLastSend; }

private:
	enum CommunicationResult
	{
		kIncomplete,
		kComplete,
		kError
	};

	CommunicationResult receive_some(TCPsocket inSocket, Uint8* inBuffer, size_t& ioBufferPosition, size_t inBufferLength);
	CommunicationResult send_some(TCPsocket inSocket, Uint8* inBuffer, size_t& ioBufferPosition, size_t inBufferLength);

	void		pumpReceivingSide();
	bool		receiveHeader();
	bool		_receiveMessage();
	
	void		pumpSendingSide();
	bool		sendHeader();
	bool		sendMessage();


	bool		mConnected;
	TCPsocket	mSocket;
	MessageInflater* mMessageInflater;
	MessageHandler*	mMessageHandler;
	Memento*	mMemento;
	

	enum
	{
		kHeaderPackedSize = 8,
		kHeaderMagic = 0xDEAD,
	};

	Uint8		mIncomingHeader[kHeaderPackedSize];
	size_t		mIncomingHeaderPosition;

	UninflatedMessage* mIncomingMessage;
	size_t		mIncomingMessagePosition;

	Uint32		mTicksAtLastReceive;

	typedef std::list<Message*>	MessageQueue;
	MessageQueue	mIncomingMessages;


	Uint8		mOutgoingHeader[kHeaderPackedSize];
	size_t		mOutgoingHeaderPosition;

	Uint32		mTicksAtLastSend;

	typedef std::list<UninflatedMessage*>	UninflatedMessageQueue;
	UninflatedMessageQueue	mOutgoingMessages;
	size_t		mOutgoingMessagePosition;
};



class CommunicationsChannelFactory
{
public:
	CommunicationsChannelFactory(Uint16 inPort);
	bool	isFunctional() const { return mSocket != NULL; }
	CommunicationsChannel* newIncomingConnection();
	~CommunicationsChannelFactory();
	
private:
	TCPsocket	mSocket;
};

#endif // COMMUNICATIONSCHANNEL_H
