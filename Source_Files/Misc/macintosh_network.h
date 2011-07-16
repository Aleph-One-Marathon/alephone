#ifndef __MACINTOSH__NETWORK_H
#define __MACINTOSH__NETWORK_H

/*
MACINTOSH_NETWORK.H

	Copyright (C) 1991-2001 and beyond by Bungie Studios, Inc.
	and the "Aleph One" developers.
 
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

Monday, June 20, 1994 12:22:25 PM
Wednesday, August 9, 1995 3:34:50 PM- network lookup stuff now takes a version which is 
	concatenated to the lookup type (ryan)

Jan 25, 2002 (Br'fin (Jeremy Parsons)):
	Disabled networking under carbon
*/

#if defined(TARGET_API_MAC_CARBON)
#warning Classic Appletalk networking not supported under carbon
#endif

#include <AppleTalk.h>
#include <ADSP.h>

#include "network.h"

/* ---------- constants */

/* missing from AppleTalk.h */
#define ddpMaxData 586

/* ---------- DDPFrame and PacketBuffer (DDP) */

struct DDPFrame
{
	short data_size;
	byte data[ddpMaxData];
	
	MPPParamBlock pb;

	WDSElement wds[3];
	byte header[17];
};
typedef struct DDPFrame DDPFrame, *DDPFramePtr;

struct DDPPacketBuffer
{
	short inUse;
	
	byte protocolType;
	byte destinationNode;
	AddrBlock sourceAddress;
	short hops;
	
	short datagramSize;
	byte datagramData[ddpMaxData];
};
typedef struct DDPPacketBuffer DDPPacketBuffer, *DDPPacketBufferPtr;

/* ---------- ConnectionEnd (ADSP) */

struct ConnectionEnd
{
	short ccbRefNum, socketNum; /* reference number and socket number of this connection end */

	/* memory for ADSP */
	TPCCB dspCCB;
	Ptr dspSendQPtr;
	Ptr dspRecvQPtr;
	Ptr dspAttnBufPtr;

#ifdef env68k	
	long a5; /* store our current a5 here */
#endif

	DSPParamBlock pb; /* parameter block for this connection end */
};
typedef struct ConnectionEnd ConnectionEnd, *ConnectionEndPtr;

/* ---------- types */

typedef EntityName *EntityNamePtr;

typedef void (*lookupUpdateProcPtr)(short message, short index);
typedef bool (*lookupFilterProcPtr)(EntityName *entity, AddrBlock *address);
typedef void (*PacketHandlerProcPtr)(DDPPacketBufferPtr packet);
#ifdef env68k
typedef PacketHandlerProcPtr PacketHandlerUPP;
#else
typedef UniversalProcPtr PacketHandlerUPP;
#endif
typedef DDPSocketListenerUPP (*InitializeListenerProcPtr)(
	PacketHandlerUPP handler,
	DDPPacketBufferPtr buffer);
#ifdef env68k
typedef InitializeListenerProcPtr InitializeListenerUPP;
#else
typedef UniversalProcPtr InitializeListenerUPP;
#endif

/* ---------- prototypes/NETWORK.C */

short NetState(void);

void NetSetServerIdentifier(short identifier);

/* for giving to NetLookupOpen() as a filter procedure */
bool NetEntityNotInGame(EntityName *entity, AddrBlock *address);

/* ---------- prototypes/NETWORK_NAMES.C */

OSErr NetRegisterName(unsigned char *name, unsigned char *type, short version, short socketNumber);
OSErr NetUnRegisterName(void);

/* ---------- prototypes/NETWORK_LOOKUP.C */

void NetLookupUpdate(void);
void NetLookupClose(void);
OSErr NetLookupOpen(unsigned char *name, unsigned char *type, unsigned char *zone, short version, 
	lookupUpdateProcPtr updateProc, lookupFilterProcPtr filterProc);
void NetLookupRemove(short index);
void NetLookupInformation(short index, AddrBlock *address, EntityName *entity);

OSErr NetGetZonePopupMenu(MenuHandle menu, short *local_zone);
OSErr NetGetZoneList(Str32 *zone_names, short maximum_zone_names, short *zone_count, short *local_zone);
OSErr NetGetLocalZoneName(Str32 local_zone_name);

/* ---------- prototypes/NETWORK_DDP.C */

OSErr NetDDPOpen(void);
OSErr NetDDPClose(void);

OSErr NetDDPOpenSocket(short *socketNumber, PacketHandlerProcPtr packetHandler);
OSErr NetDDPCloseSocket(short socketNumber);

DDPFramePtr NetDDPNewFrame(void);
void NetDDPDisposeFrame(DDPFramePtr frame);

OSErr NetDDPSendFrame(DDPFramePtr frame, AddrBlock *address, short protocolType, short socket);

/* ---------- prototypes/NETWORK_ADSP.C */

// jkvw: removed - we use TCPMess now

#endif
