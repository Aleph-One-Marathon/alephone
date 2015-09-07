/*
 *  SSLP_Protocol.h - implementations of SSLP services use this to describe the network protocol.
 *
 *  Copyright (c) 2001 Woody Zenfell, III.
 *  woodyzenfell@hotmail.com

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

 *  Simple Service Location Protocol (hope that name's ok with everyone, did not research it at all)
 *	(Near-)conflict with well-known Secure Socket Layer is unfortunate, but I doubt anyone will mistake them :) .
 *	Someone smart has surely done this better somewhere and has probably proven various characteristics,
 *	but I decided to hack out my own anyway.  Tough.
 *
 *	The current implementation (and a small amount of the interface) relies on the SDL_net cross-platform
 *	IP networking library.  The current implementation (and no part of the interface) relies on my own
 *	"SDL_netx" UDP broadcast companion to SDL_net, as well as some other features of SDL.
 *
 *	for the Aleph One project, to do NBP-ish player location on non-AppleTalk networks.  The current
 *	implementation is a (significant) simplification of what this could or perhaps ought to be, but
 *	it's sufficient for Aleph One's needs, which is all that matters right now.  Besides, like I said,
 *	I'm sure there's something better already out there that's designed for heavyweight application.
 *
 *  Created by Woody Zenfell, III on Tue Sep 11 2001.
 
 June 15, 2002 (Loren Petrich):
 	Added a packed size, so as to avoid compiler dependency
 */

#ifndef SSLP_PROTOCOL_H
#define	SSLP_PROTOCOL_H

#include	<SDL_net.h>


#ifndef SSLP_PORT
#define	SSLP_PORT		15367		// I made this up, change if it sucks
#endif

#define	SSLPP_MAGIC		0x73736c70	// 'sslp' (when in network byte order)
#define	SSLPP_VERSION		1		// change this number if you change the protocol or packet format
#define	SSLPP_MESSAGE_FIND	0x66696e64	// 'find'
#define	SSLPP_MESSAGE_HAVE	0x68617665	// 'have'
#define	SSLPP_MESSAGE_LOST	0x6c6f7374	// you guessed it, 'lost'

// note: Marathon game protocol has its own version independent of SSLP version - don't adjust SSLPP_VERSION
// if changing only the Marathon protocol.

// SSLP is implemented on datagram-type protocols (e.g. UDP).  Datagrams may get lost.  That's ok.
// It's not overly clever or anything - in fact, it could hardly be more straightforward.  Thus "Simple" :)
// ("stupid"?  "silly"?)

// It's assumed that the host and port a FIND message originates from is the same port HAVE and LOST messages should
// be sent to.  It's assumed that a host advertising a service with HAVE has the same host-address as (but maybe a
// different port from) the service itself.
// Life will be easier for everyone if all participants send and receive all SSLP packets on port number SSLP_PORT.

// SSLP does not "guarantee" anything about its findings - it's intended merely as an aid.  This means (in particular)
// a host that found a service via SSLP cannot assume that the service definitely exists at the host and port provided...
// but it's a probably a good place to look ;)


// Here's how it works:

// Host wanting to find instances of a service on the network (in this case, gathering computer finding
// network Aleph One players) broadcasts datagrams carrying FIND messages.  The service_type field
// indicates what kind of service the host is looking for.  Typically, the message would be rebroadcast
// once every 1-5 seconds or so for whatever duration the application wants (in Aleph One, probably duration
// of Gather Network Game "add players" dialog) - this is our only mechanism for coping with dropped packets.
// Host may also unicast a FIND message if it suspects a service resides on a particular host, e.g. if it was
// there a while ago.
// As noted above, FIND messages should be sent from the port that HAVE messages should be received on.

// Hosts allowing discovery of their services must respond (to the sender's host and port) to every incoming
// FIND message carrying the correct service_type (as defined by strncmp() == 0) with a HAVE message.  service_type
// of the HAVE message ought to match that of the FIND message.  service_name of the HAVE message gives the name of
// a service instance on the host.
// In Aleph One, the service_name would be the name of the player.
// A host is allowed to broadcast a HAVE message (once) when a service becomes available, to speed things up a little.
// A host may unicast an unsolicited HAVE message if it thinks a host is looking for its service (e.g. if the host
// was looking for the service a while ago, or a user "hints" the system that a named host probably wants to find
// our service).  Heck, if the user bothered to hint us an address, it probably means the broadcast FIND messages
// don't get to us (and our single broadcast HAVE probably won't get to it).  May as well unicast it periodically
// until we don't need to be discovered anymore.
// Typically, an Aleph One player service would be allowed to be discovered while the "Join Network Game" dialog is
// "committed" but before the player has been gathered.

// Hosts should, as a courtesy to FINDers, send a LOST message when discovery is no longer desired.  Ideally you'd
// send a LOST to everyone you sent a HAVE to, but just broadcasting one on SSLP_PORT is probably good enough.
// LOST is not required for "correct" operation (whatever that is for a system that guarantees nothing), since FINDers
// ought to age and discard found services for which no HAVE has been received for some time, but LOST in most cases
// should help the FINDer keep his lists better up-to-date.
// Typically, Aleph One would send this when a player cancels "Join Network Game" after "committing", or when
// the player has been gathered (and thus no longer needs to be "discovered"). 

// Ugly laziness here, using same constants for network protocol format as for API.  Oh well.
#ifndef SSLP_MAX_TYPE_LENGTH
#define	SSLP_MAX_TYPE_LENGTH	32
#endif
#ifndef SSLP_MAX_NAME_LENGTH
#define	SSLP_MAX_NAME_LENGTH	32
#endif

struct SSLP_Packet {
    Uint32	sslpp_magic;		// should always be SSLPP_MAGIC
    Uint32	sslpp_version;		// set to SSLPP_VERSION, for catching version mismatch
    Uint32	sslpp_message;		// set to SSLPP_MESSAGE_*
    Uint16	sslpp_service_port;	// only valid in HAVE messages, states on which port the service can be contacted.
    Uint16	sslpp_reserved;		// should always be 0
    char	sslpp_service_type[SSLP_MAX_TYPE_LENGTH];	// type desired, for FIND; type provided or no longer provided on HAVE or LOST
    char	sslpp_service_name[SSLP_MAX_NAME_LENGTH];	// name of the service instance.  meaningless in FIND.
};

// The packed size and packed-packet type
const int SIZEOF_SSLP_Packet = 3*4 + 2*2 + SSLP_MAX_TYPE_LENGTH + SSLP_MAX_NAME_LENGTH;

// service_type and service_name are treated as C-strings in comparisons - i.e. it's a match if all the bytes up to the first '\0'
// are equal.  also, whenever a name or type is copied, only the bytes up to and including the first '\0' are copied.
// Note that it's not an error for all SSLP_MAX_*_LENGTH bytes of name or type storage to be filled with data - a terminating
// NULL is _not_ required if all SSLP_MAX_*_LENGTH bytes are used.

// sizeof(struct SSLP_Packet) == 80

#endif//SSLP_PROTOCOL_H
