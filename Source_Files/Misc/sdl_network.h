/*
 *  sdl_network.h - Definitions for SDL implementation of networking

	Copyright (C) 1991-2001 and beyond by Bungie Studios, Inc.
	and the "Aleph One" developers.
 
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

 *  Definitions for SDL implementation of networking
 *
 *  (believed to be) Created by Christian Bauer
 *
 *  Sept-Nov 2001 (Woody Zenfell): split some defs out of here to network_lookup_sdl.h
 */

#ifndef __SDL__NETWORK_H
#define __SDL__NETWORK_H

#include <SDL_net.h>

#include	"sdl_cseries.h"

#include "network.h"

/* ---------- constants */

#define asyncUncompleted 1	/* ioResult value */

#define strNETWORK_ERRORS 132
enum /* error string for user */
{
	netErrCantAddPlayer,
	netErrCouldntDistribute,
	netErrCouldntJoin,
	netErrServerCanceled,
	netErrMapDistribFailed,
	netErrWaitedTooLongForMap,
	netErrSyncFailed,
	netErrJoinFailed,
	netErrCantContinue
};

/* missing from AppleTalk.h */
// ZZZ: note that this determines only the amount of storage allocated for packets, not
// the size of actual packets sent.  I believe UDP on Ethernet should be able to carry
// around 1.5K per packet, not sure of the exact figure off the top of my head though.
// Note that the SDL network microphone code is the one sending "big" packets these days.
#define ddpMaxData 1500

/* default IP port number for Marathon */
const uint16 DEFAULT_PORT = 4226;

typedef char EntityName[32];
typedef IPaddress AddrBlock;

/* ---------- DDPFrame and PacketBuffer (DDP) */

struct DDPFrame
{
	uint16 data_size;
	byte data[ddpMaxData];
	UDPsocket socket;
};
typedef struct DDPFrame DDPFrame, *DDPFramePtr;

struct DDPPacketBuffer
{
	byte protocolType;
	AddrBlock sourceAddress;
	
	uint16 datagramSize;
	byte datagramData[ddpMaxData];
};
typedef struct DDPPacketBuffer DDPPacketBuffer, *DDPPacketBufferPtr;

/* ---------- ConnectionEnd (ADSP) */

struct ConnectionEnd
{
	TCPsocket		socket;
        TCPsocket		server_socket;
        SDLNet_SocketSet	server_socket_set;
};
typedef struct ConnectionEnd ConnectionEnd, *ConnectionEndPtr;

/* ---------- types */

typedef EntityName *EntityNamePtr;

typedef void (*lookupUpdateProcPtr)(short message, short index);
typedef bool (*lookupFilterProcPtr)(EntityName *entity, AddrBlock *address);
typedef void (*PacketHandlerProcPtr)(DDPPacketBufferPtr packet);

/* ---------- prototypes/NETWORK.C */

short NetState(void);

void NetSetServerIdentifier(short identifier);

/* for giving to NetLookupOpen() as a filter procedure */
bool NetEntityNotInGame(EntityName *entity, AddrBlock *address);

/* ---------- prototypes/NETWORK_NAMES.C */

// ZZZ: moved to network_lookup_sdl.h to localize changes.

/* ---------- prototypes/NETWORK_LOOKUP.C */

// ZZZ: moved to network_lookup_sdl.h to localize changes.

/* ---------- prototypes/NETWORK_DDP.C */

OSErr NetDDPOpen(void);
OSErr NetDDPClose(void);

OSErr NetDDPOpenSocket(short *socketNumber, PacketHandlerProcPtr packetHandler);
OSErr NetDDPCloseSocket(short socketNumber);

DDPFramePtr NetDDPNewFrame(void);
void NetDDPDisposeFrame(DDPFramePtr frame);

OSErr NetDDPSendFrame(DDPFramePtr frame, AddrBlock *address, short protocolType, short socket);

/* ---------- prototypes/NETWORK_ADSP.C */

OSErr NetADSPOpen(void);
OSErr NetADSPClose(void);

OSErr NetADSPEstablishConnectionEnd(ConnectionEndPtr *connection);
OSErr NetADSPDisposeConnectionEnd(ConnectionEndPtr connectionEnd);

OSErr NetADSPOpenConnection(ConnectionEndPtr connectionEnd, AddrBlock *address);
OSErr NetADSPCloseConnection(ConnectionEndPtr connectionEnd, bool abort);
OSErr NetADSPWaitForConnection(ConnectionEndPtr connectionEnd);
bool NetADSPCheckConnectionStatus(ConnectionEndPtr connectionEnd, AddrBlock *address);

OSErr NetADSPWrite(ConnectionEndPtr connectionEnd, void *buffer, uint16 *count);
OSErr NetADSPRead(ConnectionEndPtr connectionEnd, void *buffer, uint16 *count);

#endif
