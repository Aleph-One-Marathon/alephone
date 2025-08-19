/*
 *  network_udp.cpp - UDP network functions (corresponds to AppleTalk DDP)

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

*/
/*
 *  network_udp.cpp - UDP network functions (corresponds to AppleTalk DDP)
 *
 *  Written in 2000 by Christian Bauer
 *
 *  Sept-Nov 2001 (Woody Zenfell): a few additions to implement socket-listening thread.
 *
 *  May 18, 2003 (Woody Zenfell): now uses passed-in port number for local socket.
 */

#if !defined(DISABLE_NETWORKING)
#include <SDL2/SDL_thread.h>
#include "thread_priority_sdl.h"
#include "cseries.h"
#include "network_private.h"
#include "mytm.h" // mytm_mutex stuff

// Keep track of our one sending/receiving socket
static std::unique_ptr<UDPsocket> sSocket;
static PacketHandlerProcPtr	sPacketHandler = NULL;
// See if the receiving thread should exit
static std::atomic_bool sKeepListening = false;
// Keep track of the receiving thread
static SDL_Thread* sReceivingThread = NULL;

// ZZZ: the socket listening thread loops in this function.  It calls the registered
// packet handler when it gets something.
static int
receive_thread_function(void*) {

	UDPpacket packet;
	sSocket->register_receive_async(packet);

	bool got_packet = false;
	int receive_result = 0;

	while (sKeepListening) {

		if (!got_packet)
		{
			receive_result = sSocket->receive_async(1000);
			got_packet = receive_result > 0;
		}

		if (got_packet && take_mytm_mutex()) {
			sPacketHandler(packet);
			release_mytm_mutex();
			got_packet = false;
		}

		if (!got_packet && receive_result != 0)
		{
			sSocket->register_receive_async(packet);
		}
	}

	return 0;
}

/*
 *  Open socket
 */
// Most of this function by ZZZ.
// Incoming port number should be in network byte order.
bool NetDDPOpenSocket(uint16_t ioPortNumber, PacketHandlerProcPtr packetHandler)
{
	sPacketHandler = packetHandler;
	sSocket = NetGetNetworkInterface()->udp_open_socket(ioPortNumber);
	if (!sSocket) return false;

	// Set up receiver
	sKeepListening = true;
	sPacketHandler = packetHandler;
	sReceivingThread = SDL_CreateThread(receive_thread_function, "NetDDPOpenSocket_ReceivingThread", NULL);

	// Set receiving thread priority very high
	bool theResult = BoostThreadPriority(sReceivingThread);
	if (theResult == false)
		fdprintf("warning: BoostThreadPriority() failed; network performance may suffer\n");
	return true;
}


/*
 *  Close socket
 */

bool NetDDPCloseSocket()
{
	// ZZZ: shut down receiving thread
	if (sReceivingThread) {
		sKeepListening = false;
		SDL_WaitThread(sReceivingThread, NULL);
		sReceivingThread = NULL;
	}

	sSocket.reset();
	return true;
}

/*
 *  Send frame to remote machine
 */

bool NetDDPSendFrame(UDPpacket& frame, const IPaddress& address)
{
	assert(frame.data_size <= ddpMaxData);
	frame.address = address;
	return sSocket->send(frame) > 0;
}

#endif // !defined(DISABLE_NETWORKING)
