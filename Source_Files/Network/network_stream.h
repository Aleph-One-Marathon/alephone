#ifndef __NETWORK_STREAM_H
#define __NETWORK_STREAM_H

/*
	network_stream.h

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

	Saturday, September 30, 1995 8:01:13 PM- rdm created.

*/

enum {
	kNetworkTransportType= 0,
	kModemTransportType,
	NUMBER_OF_TRANSPORT_TYPES
};

bool NetTransportAvailable(short type);

OSErr NetStreamEstablishConnectionEnd(void);
OSErr NetStreamDisposeConnectionEnd(void);

OSErr NetOpenStreamToPlayer(short player_index);
OSErr NetCloseStreamConnection(bool abort);

OSErr NetSendStreamPacket(short packet_type, void *packet_data);
OSErr NetReceiveStreamPacket(short *packet_type, void *buffer);

short NetGetTransportType(void);
void NetSetTransportType(short type);

bool NetStreamCheckConnectionStatus(void);
OSErr NetStreamWaitForConnection(void);

void NetGetStreamAddress(NetAddrBlock *address);
short NetGetStreamSocketNumber(void);

/* ------ application must supply these */
uint16 MaxStreamPacketLength(void);
uint16 NetStreamPacketLength(short packet_type);
NetAddrBlock *NetGetPlayerADSPAddress(short player_index);

#endif
