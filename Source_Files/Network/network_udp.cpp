/*

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

	This license is contained in the file "GNU_GeneralPublicLicense.txt",
	which is included with this source code; it is available online at
	http://www.gnu.org/licenses/gpl.html

*/
/*
 *  network_udp.cpp - UDP network functions (corresponds to AppleTalk DDP)
 *
 *  Written in 2000 by Christian Bauer
 */

#include "cseries.h"
#include "sdl_network.h"


// Global variables
static UDPpacket *ddpPacketBuffer = NULL;
static UDPsocket socket = NULL;


/*
 *  Initialize/shutdown module
 */

OSErr NetDDPOpen(void)
{
printf("NetDDPOpen\n");
	// nothing to do
	return 0;
}

OSErr NetDDPClose(void)
{
printf("NetDDPClose\n");
	// nothing to do
	return 0;
}


/*
 *  Open socket
 */

OSErr NetDDPOpenSocket(short *portNumber, PacketHandlerProcPtr packetHandler)
{
printf("NetDDPOpenSocket\n");
	assert(packetHandler);

	// Allocate packet buffer
	assert(!ddpPacketBuffer);
	ddpPacketBuffer = SDLNet_AllocPacket(ddpMaxData);
	if (ddpPacketBuffer == NULL)
		return -1;

	// Open socket
	socket = SDLNet_UDP_Open(DEFAULT_PORT);
	if (socket == NULL) {
		SDLNet_FreePacket(ddpPacketBuffer);
		ddpPacketBuffer = NULL;
		return -1;
	}
	*portNumber = DEFAULT_PORT;
	return 0;
}


/*
 *  Close socket
 */

OSErr NetDDPCloseSocket(short portNumber)
{
printf("NetDDPCloseSocket\n");
	if (ddpPacketBuffer) {
		SDLNet_FreePacket(ddpPacketBuffer);
		ddpPacketBuffer = NULL;

		SDLNet_UDP_Close(socket);
		socket = NULL;
	}
	return 0;
}


/*
 *  Allocate frame
 */

DDPFramePtr NetDDPNewFrame(void)
{
printf("NetDDPNewFrame\n");
	DDPFramePtr frame = (DDPFramePtr)malloc(sizeof(DDPFrame));
	if (frame) {
		memset(frame, 0, sizeof(DDPFrame));
		frame->socket = socket;
	}
	return frame;
}


/*
 *  Dispose of frame
 */

void NetDDPDisposeFrame(DDPFramePtr frame)
{
printf("NetDDPDisposeFrame\n");
	if (frame)
		free(frame);
}


/*
 *  Send frame to remote machine
 */

OSErr NetDDPSendFrame(DDPFramePtr frame, AddrBlock *address, short protocolType, short port)
{
printf("NetDDPSendFrame\n");
	assert(frame->data_size <= ddpMaxData);

	ddpPacketBuffer->channel = -1;
	memcpy(ddpPacketBuffer->data, frame->data, frame->data_size);
	ddpPacketBuffer->len = frame->data_size;
	ddpPacketBuffer->address = *address;
	return SDLNet_UDP_Send(socket, -1, ddpPacketBuffer) ? 0 : -1;
}
