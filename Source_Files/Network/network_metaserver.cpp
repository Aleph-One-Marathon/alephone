/*
 *  network_metaserver.cpp
 *  AlephOne-OSX
 *
 *  Created by Woody Zenfell, III on Wed Jul 16 2003.
 *  Copyright (c) 2003 __MyCompanyName__. All rights reserved.
 *
 */

#include "network_metaserver.h"

#include "AStream.h"


static const char* sRoomNames[] = {
	"Crows Bridge",
	"Otter Ferry",
	"White Falls",
	"Silvermines",
	"Shoal",
	"Madrigal",
	"Tyr",
	"Ash",
	"Scales",
	"Covenant",
	"Muirthemne",
	"Seven Gates",
	"Bagrada",
	"The Barrier",
	"Forest Heart",
	"The Ermine",
	"The Dire Marsh",
	"The Highlands",
	"The Drowned Kingdom",
	"The Great Devoid",
	"Shiver",
	"The Caterthuns",
	"Soulblighter",
	"Balor",
	"Sons of Myrgard",
	"Heart of the Stone",
	"The Last Battle",
	"Legends",
	"Vimy Ridge",
	"Stilwell Road"
};

static const char* sServiceName = "MARATHON";
static const char* sLoginServerName = "myth.mariusnet.com";
static const char* sUserName = "guest";
static const char* sPassword = "0000000000000000";
static std::string sPlayerName("No Name Set");
static std::string sTeamName("No Team Set");

/*
public static final int SENDCHAT = 200;
public static final int CREATEGAME = 104;
public static final int REMOVEGAME = 105;
public static final int SYNCPLAYER =  97;
public static final int SYNCGAMES = 110;
public static final int PRIVATE_MESSAGE = 201;
public static final int SETPLAYERMODE = 107;
public static final int SETPLAYERDATA = 103;
public static final int GETBUDDIES = 118;

public static int DENY_LOGIN = 3;

public static final int RANK_DAGGER = 0;
public static final int RANK_COMET = 16;
public static final int RANK_UNRANKED = 255;
*/

enum
{
	kLoginServerPort	= 6321,		// TCP port # for login server
	
	kMaximumIncomingMessageLength = 4095,	// we will choke and die if any incoming message is longer than this

	kHeaderBytes		= 8,		// number of bytes of header per message
	kMagic			= 0xDEAD,	// message header magic

	// Message types sent from servers to clients
	kSERVER_BROADCAST	= 10,
	kSERVER_PLAYERLIST	= 1,
	kSERVER_ROOMLIST	= 0,
	kSERVER_FIND		= 14,
	kSERVER_LIMIT		= 9,
	kSERVER_ACCEPT		= 3,
	kSERVER_STATS		= 17,
	
	// Messages sent from clients to servers
	kCLIENT_KEY		= 109,
	kCLIENT_LOCALIZE	= 115,
	kCLIENT_LOGIN		= 100,
	kCLIENT_NAME_TEAM	= 103,
	kCLIENT_FIND		= 117,
	kCLIENT_ROOM_LOGIN	= 101,
	kCLIENT_STATS		= 121,

	// Message types sent in both directions
	kBOTH_CHAT		= 200,
	kBOTH_PRIVATE_MESSAGE	= 201,
	kBOTH_KEEP_ALIVE	= 202,
};

enum
{
	kCRYPT_PLAINTEXT = 0,
	kCRYPT_SIMPLE = 1,

	kSTATE_AWAKE = 0,
	kSTATE_AWAY = 1,

	kPlatformType = 1,
};


enum ClientState
{
	kUninitialized,
	kUnconnected,

	// Interactions with login server
	kWaitingForKey,
	kWaitingForAcceptance,
	kWaitingForLoginData,
	kWaitingForMythData,
	kWaitingForRoomList,

	// Interactions with room server
	kWaitingForIDAndLimit,
	kWaitingForRoomAcceptance,

	kConnectedToRoom,
};


typedef void (*ReceiveMessageFunction)(uint16 inMessageType, const void* inMessageBody, uint32 inMessageLength);

typedef std::map<uint16, ReceiveMessageFunction>	TypeToReceiveMessageFunction;
static TypeToReceiveMessageFunction sTypeToReceiveMessageFunction;
typedef typename TypeToReceiveMessageFunction::iterator TypeToReceiveMessageFunctionIterator;
typedef typename TypeToReceiveMessageFunction::const_iterator TypeToReceiveMessageFunctionConstIterator;


static ClientState		sClientState;
static bool			sIncomingMessage;
static uint16			sIncomingMessageType;
static uint32			sIncomingMessageLength;
static CircularByteBuffer	sIncomingData(kMaximumIncomingMessageLength);
static TCPSocket		sSocket;


void
metaserver_initialize()
{
}


void
metaserver_connect()
{
	assert(sClientState == kUnconnected);

	IPaddress theMetaserverAddress;
	int theResult = SDLNet_ResolveHost(&theMetaserverAddress, sLoginServerName, kLoginServerPort);

	if(theResult < 0)
		throw something;

	theResult = SDLNet_TCP_Open(theMetaserverAddress);

	if(theResult < 0)
		throw something;

	sSocket = theResult;

	send_login_and_player_info();

	sClientState = kWaitingForKey;
}


static void
write_string(AOStream& inStream, const char* inString)
{
	inStream.write(inString.c_str(), strlen(inString.c_str()) + 1);
}


static void
write_padded_string(AOStream& inStream, const char* inString, size_t inTargetLength)
{
	write_padded_bytes(inString, inString, strlen(inString), inTargetLength);
}


static void
write_padded_bytes(AOStream& inStream, const char* inBytes, size_t inByteCount, size_t inTargetLength)
{
	size_t theByteCountToWrite = std::min(inByteCount, inTargetLength);
	
	inStream.write(inBytes, theByteCountToWrite);

	size_t theZeroCount = inTargetLength - theByteCountToWrite;

	for(size_t i = 0; i < theZeroCount; i++)
	{
		inStream << '\0';
	}
}


static void
send_login_and_player_info()
{
	uint8 thePacketData[256];
	
	AOStreamBE thePacket(thePacketData, sizeof(thePacketData));

	// AFAICT this must be the size of the "aux player data" below
	// I'd much rather write the stuff and THEN figure out how much there is, but
	// given the way AStreams work currently, this is much easier.  Trust me.  :)
	uint16 thePlayerDataSize = 40 + strlen(sPlayerName) + strlen(sTeamName);

	thePacket << (uint32)kPlatformType
		<< (uint32)0	// no explanation given in gooey
		<< (uint32)0	// this one is a seemingly-uninitialized 'userID' in gooey
		<< (uint16)1	// this is called "maxA" in gooey
		<< thePlayerDataSize;

	write_padded_string(thePacket, sServiceName, 32);
	write_padded_string(thePacket, __DATE__, 32);
	write_padded_string(thePacket, __TIME__, 32);
	write_padded_string(thePacket, sUserName, 32);

	// "aux player data"
	/* @bnetpacket Byte - Their icon
	* @bnetpacket Short - Awake status (0 is awake, 1 is afk)
	* @bnetpacket byte - skipped
	* @bnetpacket byte[6] - Primary Colors
	* @bnetpacket 2 Bytes - Skipped
	* @bnetpacket byte[6] - Seconary Colors
	* @bnetpacket 20 bytes - Skipped
	* @bnetpacket String - Name
	* @bnetpacket String - Team
	*/

	thePacket << kPlayerIcon
		<< sPlayerStatus
		<< (char)0
		<< (uint16)256
		<< (uint16)256
		<< (uint16)256
		<< (uint16)0
		<< (uint16)256
		<< (uint16)256
		<< (uint16)256;

	write_padded_bytes(thePacket, NULL, 0, 20);

	write_string(thePacket, sPlayerName);
	write_string(thePacket, sPlayerTeam);

	send_metaserver_message(kCLIENT_LOGIN, thePacketData, thePacket.tellg());
}


static void
receive_login_key(uint16 inMessageType, const void* inMessageBytes, uint32 inMessageLength)
{
	assert(inMessageType == );
	if(sClientState == kWaitingForKey)
	{
		AIStreamBE theMessage(inMessageBytes, inMessageLength);

		uint16	theEncryptionScheme;
		theMessage >> theEncryptionScheme;

		if(theEncryptionScheme == something)
		{
			uint8 theKey[16];
			theMessage.read(theKey, sizeof(theKey));

			for(size_t i = 0; i < sizeof(theKey); i++)
			{
				theKey[i] ^= sUserPassword[i];
			}

			send_encrypted_password(theKey, sizeof(theKey));
		
			sClientState = kWaitingForAcceptance;
		}
	}
}


static void
receive_login_acceptance(uint16 inMessageType, const void* inMessageBytes, uint32 inMessageLength)
{
	assert(inMessageType == kSERVER_ACCEPT);
	
	if(sClientState == kWaitingForAcceptance)
	{
		AIStreamBE theMessage(inMessageBytes, inMessageLength);
		uint32 the
		sClientState = kWaitingForLoginData;
	}
}


void
metaserver_pump()
{
	if(sClientState == kConnectingToLoginServer || sClientState == kConnectingToRoomServer)
	{
	}
	else
	{
		bool didReceiveMessage;

		do
		{
			accept_incoming_data();
			detect_incoming_message();
			didReceiveMessage = receive_incoming_message();
		} while(didReceiveMessage);
	}
}


static bool
accept_incoming_data()
{
	unsigned int	theAmountTransferred = 0;
	
	if(socket seems to have data)
	{
		unsigned int 	theMaxBytesToTransfer = sIncomingData.getRemainingSpace();

		void*		theFirstChunk;
		void*		theSecondChunk;
		unsigned int	theFirstChunkSize;
		unsigned int	theSecondChunkSize;

		sIncomingData.enqueueBytesNoCopyStart(theMaxBytesToTransfer, &theFirstChunk,
					&theFirstChunkSize, &theSecondChunk, &theSecondChunkSize);
		
		int	theResult = SDLNet_TCP_Recv(sSocket, theFirstChunk, theFirstChunkSize);

		if(theResult < 0)
			throw something;

		if(theResult > 0)
		{
			theAmountTransferred += theResult;

			if(theAmountTransferred == theFirstChunkSize && theSecondChunkSize > 0)
			{
				theResult = SDLNet_TCP_Recv(sSocket, theSecondChunk, theSecondChunkSize);

				if(theResult < 0)
					throw something;

				theAmountTransferred += theResult;
			}
		}
		
		sIncomingData.enqueueBytesNoCopyFinish(theAmountTransferred);
	}

	return theAmountTransferred > 0;
}


static void
detect_incoming_message()
{
	if(!sIncomingMessage)
	{
		if(sIncomingData.getCountOfElements() >= kHeaderBytes)
		{
			uint8 theHeaderBytes[kHeaderBytes];
			sIncomingData.peekBytes(theHeaderBytes, sizeof(theHeaderBytes));
			sIncomingData.dequeue(sizeof(theHeaderBytes));

			AIStreamBE	theHeaderStream(theHeaderBytes, sizeof(theHeaderBytes));

			uint16	theMagic;
			uint32	theTotalLength;

			theHeaderStream >> theMagic >> sIncomingMessageType >> theTotalLength;

			if(theMagic != kMagic)
				throw something;

			if(theTotalLength < kHeaderBytes)
				throw something;

			sIncomingMessageLength = theTotalLength - kHeaderBytes;

			sIncomingMessage = true;
		}
	}
}


static bool
receive_incoming_message()
{
	bool didReceiveMessage = false;
	
	if(sIncomingMessage)
	{
		if(sIncomingData.getCountOfElements() >= sIncomingMessageLength)
		{
			const void* theMessageBytes = NULL;
			uint8*	theAllocatedBytes = NULL;
		
			if(sIncomingMessageLength > 0)
			{
				const void* theFirstChunkBytes;
				unsigned int theFirstChunkByteCount;

				sIncomingData.peekBytesNoCopy(sIncomingMessageLength, &theFirstChunkBytes, &theFirstChunkByteCount, NULL, NULL);

				// Here we avoid a copy if it's easy (if the whole message is contiguous in the buffer)
				if(theFirstChunkByteCount == sIncomingMessageLength)
				{
					theMessageBytes = theFirstChunkBytes;
				}
				else
				{
					theAllocatedBytes = new uint8[theMessageLength]; // XXX potential leak if message handler throws
					sIncomingData.peekBytes(theMessageBytes, sIncomingMessageLength);
					theMessageBytes = theAllocatedBytes;
				}
			}
		
			TypeToReceiveMessageFunctionConstIterator i = sTypeToReceiveMessageFunction.find(theType);
			if(i != sTypeToReceiveMessageFunction.end())
				theReceiveMessageFunction(sIncomingMessageType, theMessageBytes, sIncomingMessageLength);

			if(sIncomingMessageLength > 0)
				sIncomingData.dequeue(sIncomingMessageLength);

			// The C++-FAQ-Lite promises that this is safe against theAllocatedBytes == NULL
			delete [] theAllocatedBytes;

			didReceiveMessage = true;
			sIncomingMessage = false;
			
		} // if the entire message body has been received
		
	} // if a message header has been identified

	return didReceiveMessage;
} // receive_incoming_message()


void
metaserver_set_player_data()
{
}


void
metaserver_set_game_data()
{
}


void
metaserver_send_chat()
{
}


void
metaserver_send_private_message()
{
}


void
metaserver_set_player_state()
{
}


void
metaserver_disconnect()
{
}


void
metaserver_cleanup()
{
}


// We could adopt a non-blocking reading scheme:
// We could have a CircularByteBuffer.  We'd call nonblocking_read() to get min(cbb.getRemainingSpace(), sizeof(tempbuffer))
// bytes into tempbuffer.  Then we'd stick 'em into the cbb.  We would only claim to have a received message once we had
// gotten all the data for the message - thus not tying us up for the time between when data becomes available and when
// we've received an entire message's worth (as the blocking scheme would do).
static void
receive_bytes(uint8* outBytes, uint32 inLength)
{
	int theResult = blocking_receive(sSocket, outBytes, inLength);
	if(theResult != inLength)
		throw something;
}


static void
send_bytes(const uint8* inBytes, uint32 inLength)
{
	int theResult = blocking_send(sSocket, inBytes, inLength);
	if(theResult != inLength)
		throw something;
}


static void
send_metaserver_message(uint16 inType, const uint8* inMessageBytes, uint32 inMessageLength)
{
	uint32 theTotalLength = inMessageLength + kHeaderBytes;

	// guard against wraparound
	assert(theTotalLength > inMessageLength);

	uint8 theHeaderBytes[kHeaderBytes];
	AOStreamBE theHeaderStream(theHeaderBytes, sizeof(theHeaderBytes));

	theHeaderStream << ((uint16)kMagic) << inType << theTotalLength;

	send_bytes(theHeaderBytes, sizeof(theHeaderBytes));

	if(inMessageLength > 0)
		send_bytes(inMessageBytes, inMessageLength);
}
