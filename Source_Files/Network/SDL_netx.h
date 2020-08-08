/*
 *  SDL_netx.h
 *
 *  Copyright (c) 2001 Woody Zenfell, III.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 *  SDL_netx is intended to provide a few capabilities (like broadcast) that
 *  SDL_net seems like it ought to support (but doesn't).
 *
 *  Created by Woody Zenfell, III on Mon Sep 24 2001.
 */

#ifndef	SDL_NETX_H
#define	SDL_NETX_H

// Officially, to be more SDL-like, we ought to arrange our structures with particular alignment,
// specify C-style linkage, etc.  I don't think that's important at this point, but should anyone be
// deranged enough to detach SDL_netx from Aleph One and position it as a real extension to SDL_net,
// these things should be investigated.

#include "SDL_net.h"

// SDLNetx_EnableBroadcast - allow SDLNetx_UDP_Broadcast to succeed on inSocket
//   inputs: UDPsocket inSocket - socket for which broadcasting should be enabled
//   outputs: return value - positive if broadcasting was enabled; 0 if not
int	SDLNetx_EnableBroadcast(UDPsocket inSocket);

// SDLNetx_DisableBroadcast - disallow SDLNetx_UDP_Broadcast to succeed on inSocket
//   inputs: UDPsocket inSocket - socket for which broadcasting should be disabled
//   outputs: return value - positive if broadcasting was disabled; 0 if not
int	SDLNetx_DisableBroadcast(UDPsocket inSocket);


// SDLNetx_UDP_Broadcast - attempt to send inPacket to the broadcast address of each available
//     interface in the system capable of IP (UDP) broadcast.
//   inputs: UDPsocket inSocket - the socket that should send the broadcast packet.  You should have
//              already called EnableBroadcast() on it.  Note that any "channels" are ignored here.
//           UDPpacket* inPacket - packet which should be broadcast.  The host-part of the address is ignored;
//              the port-part is used.
//   outputs: return value - number of interfaces packet was broadcast on; 0 if not broadcast
//            inPacket->status - status result of the broadcast Send call on the last interface
int	SDLNetx_UDP_Broadcast(UDPsocket inSocket, UDPpacket* inPacket);


#endif//SDL_NETX_H
