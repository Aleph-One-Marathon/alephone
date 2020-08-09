/*
 *  SDL_netx.cpp
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

#include "SDL_netx.h"

#include <sys/types.h>

#if defined(WIN32)
# define WIN32_LEAN_AND_MEAN
# include <winsock.h>
#else
# include <unistd.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <net/if.h>
# if defined(__svr4__)
#  define BSD_COMP 1 // This is required to get SIOC* under Solaris
# endif
# include <sys/ioctl.h>
#endif

// PREPROCESSOR MACROS
// VC++6 chokes if I don't explicitly cast int* to const char* in setsockopt().
#ifdef WIN32
# define MY_TYPE_CAST (const char*)
#else
# define MY_TYPE_CAST
#endif


// Win32 allows 255.255.255.255 as broadcast, much easier
#if !defined(WIN32)
// FILE-LOCAL (static) CONSTANTS
static const int	kMaxNumBroadcastAddresses	= 8;
static const int	kIFConfigBufferSize		= 1024;	// in bytes

// FILE-LOCAL (static) STORAGE
static	bool	sCollectedBroadcastAddresses = false;			// set to true if we've tried
static	int	sNumberOfBroadcastAddresses = 0;			// set to number of valid addresses
static	long	sBroadcastAddresses[kMaxNumBroadcastAddresses];		// collection of addresses to try
// note that we only store the host-part since the packet the user wants to broadcast will specify the port.

// FILE-LOCAL (static) FUNCTIONS
static	int	SDLNetxint_CollectBroadcastAddresses(UDPsocket inSocket);
#endif

//////// START OF CODE ////////

// EXTERNALLY-VISIBLE FUNCTIONS
int
SDLNetx_EnableBroadcast(UDPsocket inSocket) {
#if !defined(WIN32)
    if(!sCollectedBroadcastAddresses)
        SDLNetxint_CollectBroadcastAddresses(inSocket);
#endif

    // This works in Mac OS X and probably other BSD's... and now on Win32 (Win 98, anyway).
    // May need modification for Linux, etc.
    // Absolutely needs modification for "classic" Mac OS (i.e. Open Transport)
    // XXX: this depends on intimate carnal knowledge of the SDL_net struct _UDPsocket
    // if it changes that structure, we are hosed.  (this works with 1.2.2)
    int	theSocketFD = ((int*)inSocket)[1];
    
    // Sanity check
    if(theSocketFD < 0)
        return 0;

    // Try to enable broadcast option on underlying socket
    int theOnValue = 1;

    int theResult = setsockopt(theSocketFD, SOL_SOCKET, SO_BROADCAST, MY_TYPE_CAST &theOnValue, sizeof(theOnValue));
    
    if(theResult < 0)
        return 0;
    else
        return theOnValue;
}


int
SDLNetx_DisableBroadcast(UDPsocket inSocket) {
    // This works in Mac OS X and probably other BSD's... and now in Win32 (Win 98, at least).
    // Might need modification for Linux, etc.
    // Absolutely needs modification for "classic" Mac OS (i.e. Open Transport)
    // XXX: this depends on intimate carnal knowledge of the SDL_net struct _UDPsocket
    // if it changes that structure, we are hosed.
    int	theSocketFD = ((int*)inSocket)[1];
    
    // Sanity check
    if(theSocketFD < 0)
        return 0;
    
    // Try to disable broadcast option on underlying socket
    int theOffValue = 0;
    
    int theResult = setsockopt(theSocketFD, SOL_SOCKET, SO_BROADCAST, MY_TYPE_CAST &theOffValue, sizeof(theOffValue));
    
    if(theResult < 0)
        return 0;
    else
        return (theOffValue == 0) ? 1 : 0;
}


// see simpler function below for Win32
#if !defined(WIN32)
int
SDLNetx_UDP_Broadcast(UDPsocket inSocket, UDPpacket* inPacket) {
    int	theCountOfSuccessfulSends = 0;
    
    // We re-use the packet's destination-address "host" part, but don't want to trample it... so we save it
    Uint32	theSavedHostAddress = inPacket->address.host;
    
    // Write each broadcast address into the packet, in turn, and send it off.
    for(int i = 0; i < sNumberOfBroadcastAddresses; i++) {
        inPacket->address.host = sBroadcastAddresses[i];
        if(SDLNet_UDP_Send(inSocket, -1, inPacket) == 1)
            theCountOfSuccessfulSends++;
    } // for each broadcast address
    
    // restore the saved destination host-part
    inPacket->address.host = theSavedHostAddress;
    
    return theCountOfSuccessfulSends;
}
#else
// Win32 (at least, Win 98) seems to accept 255.255.255.255 as a valid broadcast address.
// I'll live with that for now.
int
SDLNetx_UDP_Broadcast(UDPsocket inSocket, UDPpacket* inPacket) {
	Uint32 theSavedHostAddress = inPacket->address.host;
	inPacket->address.host = 0xffffffff;
	int theResult = SDLNet_UDP_Send(inSocket, -1, inPacket);
	inPacket->address.host = theSavedHostAddress;
	return theResult;
}
#endif



// INTERNAL (static) FUNCTIONS
#if !defined(WIN32)
int
SDLNetxint_CollectBroadcastAddresses(UDPsocket inSocket) {
    // Win or lose, we played the game.
    sCollectedBroadcastAddresses = true;
    
    // This works in Mac OS X and probably other BSD's.
    // I have no idea whether this is the right way or the best way to do this.
    // Probably needs modification for Win32, Linux, etc.
    // Absolutely needs modification for "classic" Mac OS (i.e. Open Transport)
    // XXX: this depends on intimate carnal knowledge of the SDL_net struct _UDPsocket
    // if it changes that structure, we are hosed.
    int	theSocketFD = ((int*)inSocket)[1];
    
    // Sanity check
    if(theSocketFD < 0)
        return 0;

    // Ask the system for interfaces and their addresses
    char		theRequestBuffer[kIFConfigBufferSize];
    struct ifconf	theConfigRequest;
    int			theOffset = 0;
    
    theConfigRequest.ifc_len	= sizeof(theRequestBuffer);
    theConfigRequest.ifc_buf	= theRequestBuffer;
    
    int theResult = ioctl(theSocketFD, SIOCGIFCONF, &theConfigRequest);

    if(theResult < 0)
	return 0;

    // theOffset marches through the return buffer to help us find subsequent entries
    // (entries can have various sizes, but always >= sizeof(struct ifreq)... IP entries have minimum size.)
    // The while() condition avoids misbehaving in case there's only a partial entry at the end
    // of the buffer.
    while(theConfigRequest.ifc_len - theOffset >= int(sizeof(struct ifreq))) {
        // Point at the next result
        struct ifreq*	theReq	= (struct ifreq*) &(theRequestBuffer[theOffset]);
        
        // Make sure it's an entry for IP (not AppleTalk, ARP, etc.)
        if(theReq->ifr_addr.sa_family == AF_INET) {
            // Now we may interpret the address as an IP address.
            struct sockaddr_in*	theInetAddr = (struct sockaddr_in*) &theReq->ifr_addr;

            // Who cares about the actual address?  Turn this around into a request for the interface's
            // broadcast address.
            theResult = ioctl(theSocketFD, SIOCGIFBRDADDR, theReq);

            if(theResult >= 0) {
                // we retrieved an actual broadcast address for IP networking... assume it's good.

                // make sure there's room to store another broadcast address...
                if(sNumberOfBroadcastAddresses < kMaxNumBroadcastAddresses) {
                    // We can still use theInetAddr, since it's just a reference to actual storage
                    sBroadcastAddresses[sNumberOfBroadcastAddresses] = theInetAddr->sin_addr.s_addr;
                    sNumberOfBroadcastAddresses++;
                } // room for address in array
            } // broadcast-address lookup succeeded
        } // entry is IP
        
        // Increment theOffset to point at the next entry.
        // _SIZEOF_ADDR_IFREQ() is provided in Mac OS X.  It accounts for entries whose address families
        // use long addresses.  Use theReq->ifr_addr.sa_len (and add the overhead for the rest of *theReq)
        // if it's not provided for you.
#ifdef _SIZEOF_ADDR_IFREQ
        theOffset += _SIZEOF_ADDR_IFREQ(*theReq);
#else
		theOffset += sizeof(ifreq);
#endif
    } // while more entries
    
    return sNumberOfBroadcastAddresses;
}
#endif
