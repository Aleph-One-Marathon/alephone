/*
 *  network_udp.cpp - UDP network functions (corresponds to AppleTalk DDP)

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

*/
/*
 *  network_udp.cpp - UDP network functions (corresponds to AppleTalk DDP)
 *
 *  Written in 2000 by Christian Bauer
 *
 *  Sept-Nov 2001 (Woody Zenfell): a few additions to implement socket-listening thread.
 */

#ifdef __MWERKS__
#include 	<stdlib.h>
#include 	<extras.h>
#endif

#include "cseries.h"
#include "sdl_network.h"
#include	"network_private.h"

#include	<SDL_thread.h>

#include	"thread_priority_sdl.h"
#include	"mytm.h" // mytm_mutex stuff

// Global variables (most comments and "sSomething" variables are ZZZ)
// Storage for incoming packet data
static UDPpacket*		sUDPPacketBuffer	= NULL;

// Storage for the DDP packet we pass back to the handler proc
static DDPPacketBuffer		ddpPacketBuffer;

// Keep track of our one sending/receiving socket
static UDPsocket 		socket			= NULL;

// Keep track of the socket-set the receiving thread uses (so we don't have to allocate/free it in that thread)
static	SDLNet_SocketSet	sSocketSet		= NULL;

// Keep track of the function to call when we receive data
static PacketHandlerProcPtr	sPacketHandler		= NULL;

// Keep track of the receiving thread
static SDL_Thread*		sReceivingThread	= NULL;

// See if the receiving thread should exit
static volatile bool		sKeepListening		= false;


// ZZZ: the socket listening thread loops in this function.  It calls the registered
// packet handler when it gets something.
static int
receive_thread_function(void*) {
    while(true) {
        // We listen with a timeout so we can shut ourselves down when needed.
        int theResult = SDLNet_CheckSockets(sSocketSet, 1000);
        
        if(!sKeepListening)
            break;
        
        if(theResult > 0) {
            theResult = SDLNet_UDP_Recv(socket, sUDPPacketBuffer);
            if(theResult > 0) {
                if(take_mytm_mutex()) {
                    ddpPacketBuffer.protocolType	= kPROTOCOL_TYPE;
                    ddpPacketBuffer.sourceAddress	= sUDPPacketBuffer->address;
                    ddpPacketBuffer.datagramSize	= sUDPPacketBuffer->len;
                    
                    // Hope the other guy is done using whatever's in there!
                    // (As I recall, all uses happen in sPacketHandler and its progeny, so we should be fine.)
                    memcpy(ddpPacketBuffer.datagramData, sUDPPacketBuffer->data, sUDPPacketBuffer->len);
                    
                    sPacketHandler(&ddpPacketBuffer);
                    
                    release_mytm_mutex();
                }
                else
                    fdprintf("could not take mytm mutex - incoming packet dropped");
            }
        }
    }
    
    return 0;
}


/*
 *  Initialize/shutdown module
 */

OSErr NetDDPOpen(void)
{
//fdprintf("NetDDPOpen\n");
	// nothing to do
	return 0;
}

OSErr NetDDPClose(void)
{
//fdprintf("NetDDPClose\n");
	// nothing to do
	return 0;
}


/*
 *  Open socket
 */
// Most of this function by ZZZ.
OSErr NetDDPOpenSocket(short *portNumber, PacketHandlerProcPtr packetHandler)
{
//fdprintf("NetDDPOpenSocket\n");
	assert(packetHandler);

	// Allocate packet buffer (this is Christian's part)
	assert(!sUDPPacketBuffer);
	sUDPPacketBuffer = SDLNet_AllocPacket(ddpMaxData);
	if (sUDPPacketBuffer == NULL)
		return -1;

        //PORTGUESS
	// Open socket (SDLNet_Open seems to like port in host byte order)
        // NOTE: only SDLNet_UDP_Open wants port in host byte order.  All other uses of port in SDL_net
        // are in network byte order.
        // Note further that if, in the future, this behavior of SDL_net is deemed buggy, and it's altered
        // to take a network byte order port, this port number will need to be changed to SDL_SwapBE16(port).
        // Final note: At my suggestion, Sam Lantinga to update the SDL_net documentation to indicate that this is
        // the only place in SDL_net that the port is used in host byte order.  So it will probably stay this way!
	socket = SDLNet_UDP_Open(network_preferences->game_port);
	if (socket == NULL) {
		SDLNet_FreePacket(sUDPPacketBuffer);
		sUDPPacketBuffer = NULL;
		return -1;
	}

        // Set up socket set
        sSocketSet = SDLNet_AllocSocketSet(1);
        SDLNet_UDP_AddSocket(sSocketSet, socket);
        
        // Set up receiver
        sKeepListening		= true;
        sPacketHandler		= packetHandler;
        sReceivingThread	= SDL_CreateThread(receive_thread_function, NULL);

        // Set receiving thread priority very high
        bool	theResult = BoostThreadPriority(sReceivingThread);
        if(theResult == false)
            fdprintf("warning: BoostThreadPriority() failed; network performance may suffer\n");
        
        //PORTGUESS but we should generally keep port in network order, I think?
	*portNumber = SDL_SwapBE16(network_preferences->game_port);
	return 0;
}


/*
 *  Close socket
 */

OSErr NetDDPCloseSocket(short portNumber)
{
//fdprintf("NetDDPCloseSocket\n");
        // ZZZ: shut down receiving thread
        if(sReceivingThread) {
            sKeepListening	= false;
            SDL_WaitThread(sReceivingThread, NULL);
            sReceivingThread	= NULL;
        }

        if(sSocketSet) {
            SDLNet_FreeSocketSet(sSocketSet);
            sSocketSet = NULL;
        }
    
        // (CB's code follows)
	if (sUDPPacketBuffer) {
		SDLNet_FreePacket(sUDPPacketBuffer);
		sUDPPacketBuffer = NULL;

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
//fdprintf("NetDDPNewFrame\n");
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
//fdprintf("NetDDPDisposeFrame\n");
	if (frame)
		free(frame);
}


/*
 *  Send frame to remote machine
 */

OSErr NetDDPSendFrame(DDPFramePtr frame, NetAddrBlock *address, short protocolType, short port)
{
//fdprintf("NetDDPSendFrame\n");
	assert(frame->data_size <= ddpMaxData);

	sUDPPacketBuffer->channel = -1;
	memcpy(sUDPPacketBuffer->data, frame->data, frame->data_size);
	sUDPPacketBuffer->len = frame->data_size;
	sUDPPacketBuffer->address = *address;
	return SDLNet_UDP_Send(socket, -1, sUDPPacketBuffer) ? 0 : -1;
}
