/*
	network_stream.c

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

	Saturday, September 30, 1995 1:24:52 AM- rdm created.

	The goal of this file is to have a high-level stream interface that can be 
	integrated into future products.  
	
	Definition:
	Stream- A method of communications that guarantees accurate, inorder delivery.
	This implies ADSP for network transports, and some form of xmodem/something for
	modem connections.

Feb. 4, 2000 (Loren Petrich):
	Changed halt() to assert(false) for better debugging

Sept-Nov 2001 (Woody Zenfell): strategic byte-swapping for cross-platform compatibility

Feb 27, 2002 (Br'fin (Jeremy Parsons)):
	Enabled SDL networking for Carbon without fully using SDL
	Replaced many mac checks with checks for HAVE_SDL_NET

 May 24, 2003 (Woody Zenfell):
	Support for more tolerant streaming communications (safe to send streaming packet
	receiver doesn't understand now; he'll tell you, and ignore it).
*/

#include "cseries.h"



#if defined(SDL) || HAVE_SDL_NET
#include "sdl_network.h"
#elif defined(mac)
#include "macintosh_network.h"
#endif

#ifdef TEST_MODEM
#include "network_modem.h"
#endif

#include "preferences.h"
#include "network_stream.h"

#include "shell.h" //--> only for system_information!

#ifdef env68k
	#pragma segment network_modem
#endif

// #define USE_MODEM

static short transport_type= kNetworkTransportType;
static ConnectionEndPtr dspConnection; /* our adsp connection */
static NetAddrBlock adsp_end_address;

/* ----- local prototypes */
static OSErr stream_write(void *data, uint16 length);
static OSErr stream_read(void *data, uint16 *length);

// ZZZ change: in newer get_network_versions(), we send the packet length as well as the
// type, to ease future cases where some folks might not understand some messages: they'll
// be able to skip unrecognized packets now.
/* ------ code */
OSErr NetSendStreamPacket(
	short packet_type,
	void *packet_data)
{
	OSErr error;
	uint16 packet_length;

	packet_length= NetStreamPacketLength(packet_type);

        // ZZZ: byte-swap packet type code and packet length if necessary
	short	packet_type_NET;
	uint16	packet_length_NET;
    
#if HAVE_SDL_NET
        packet_type_NET = SDL_SwapBE16(packet_type);
	packet_length_NET = SDL_SwapBE16(packet_length);
#else
        packet_type_NET = packet_type;
	packet_length_NET = packet_length;
#endif
        
	error= stream_write(&packet_type_NET, sizeof(short));
	if(!error)
	{
		if(get_network_version() >= kMinimumNetworkVersionForGracefulUnknownStreamPackets)
		{
			error = stream_write(&packet_length_NET, sizeof(packet_length_NET));
		}
		if(!error)
			error= stream_write(packet_data, packet_length);
	}

	return error;
}

// ZZZ change: to pair with above, we now get lengths for received packets from the packets,
// rather than from our hard-coded values.  This means we can now receive packets of types we
// don't recognize.  Higher-level code will have to figure out what to do with them though.
OSErr NetReceiveStreamPacket(
	short *packet_type,
	void *buffer) /* Must be large enough for the biggest packet type.. */
{
	OSErr error = noErr;
	uint16 length;
	uint16	packet_length;

	length= sizeof(short);

	// ZZZ: byte-swap if necessary
	short	packet_type_NET;
	uint16	packet_length_NET;

	error= stream_read(&packet_type_NET, &length);
	if(!error && get_network_version() >= kMinimumNetworkVersionForGracefulUnknownStreamPackets)
	{
		length= sizeof(packet_length_NET);
		error= stream_read(&packet_length_NET, &length);
	}

	if(!error)
	{
#if HAVE_SDL_NET
		*packet_type= SDL_SwapBE16(packet_type_NET);
		packet_length= SDL_SwapBE16(packet_length_NET);
#else
		*packet_type= packet_type_NET;
		packet_length= packet_length_NET;
#endif
		length= ((get_network_version() >= kMinimumNetworkVersionForGracefulUnknownStreamPackets) ? packet_length : NetStreamPacketLength(*packet_type));

		error= stream_read(buffer, &length);
	}

	return error;
}

OSErr NetOpenStreamToPlayer(
	short player_index)
{
	OSErr error;

	switch(transport_type)
	{
		case kNetworkTransportType:
			// LP: kludge to get the code to compile
			#if HAVE_SDL_NET
			error= NetADSPOpenConnection(dspConnection, 
				NetGetPlayerADSPAddress(player_index));
			#else
			error = noErr;
			#endif
			break;
			
		case kModemTransportType:
#ifdef USE_MODEM
			assert(player_index<2); /* Only two players with a modem connection! */
			error= noErr;
			break;
#endif			
		default:
			assert(false);
			break;
	}

	return error;
}

OSErr NetCloseStreamConnection(
	bool abort)
{
	OSErr error;

	switch(transport_type)
	{
		case kNetworkTransportType:
			error= NetADSPCloseConnection(dspConnection, abort);
			break;
			
		case kModemTransportType:
#ifdef USE_MODEM
			error= noErr;
			break;
#endif			
		default:
			assert(false);
			break;
	}

	return error;
}

OSErr NetStreamEstablishConnectionEnd(
	void)
{
	OSErr error;

	switch(transport_type)
	{
		case kNetworkTransportType:
			error= NetADSPEstablishConnectionEnd(&dspConnection);
			break;
			
		case kModemTransportType:
#ifdef USE_MODEM
			initialize_modem_endpoint();
			break;
#endif			
		default:
			assert(false);
			break;
	}

	return error;
}

OSErr NetStreamDisposeConnectionEnd(
	void)
{
	OSErr error;

	switch(transport_type)
	{
		case kNetworkTransportType:
			error= NetADSPDisposeConnectionEnd(dspConnection);
			break;
			
		case kModemTransportType:
#ifdef USE_MODEM
			teardown_modem_endpoint();
			error= noErr;
			break;
#endif			
		default:
			assert(false);
			break;
	}

	return error;
}

short NetGetTransportType(
	void)
{
	return transport_type;
}

void NetSetTransportType(
	short type)
{
	assert(type>=0 && type<NUMBER_OF_TRANSPORT_TYPES);
#ifndef USE_MODEM
	assert(type==kNetworkTransportType);
#endif
	transport_type= type;
}

bool NetStreamCheckConnectionStatus(
	void)
{
	bool connection_made;

	switch(transport_type)
	{
		case kNetworkTransportType:
			connection_made= NetADSPCheckConnectionStatus(dspConnection, 
				&adsp_end_address);
			break;
			
		case kModemTransportType:
#ifdef USE_MODEM
			connection_made= false;
			break;
#endif			
		default:
			assert(false);
			break;
	}

	return connection_made;
}

OSErr NetStreamWaitForConnection(
	void)
{
	OSErr error;

	switch(transport_type)
	{
		case kNetworkTransportType:
			// ZZZ: If SDL_SwapBE16() is not actually available on all current A1 platforms, feel free to
			// rework this to provide network_preferences->game_port in big-endian order.
			error= NetADSPWaitForConnection(dspConnection, SDL_SwapBE16(network_preferences->game_port));
			break;
			
		case kModemTransportType:
#ifdef USE_MODEM
			error= noErr;
			break;

#endif			
		default:
			assert(false);
			break;
	}

	return error;
}

void NetGetStreamAddress(
	NetAddrBlock *address)
{
	assert(transport_type==kNetworkTransportType);
	
	*address= adsp_end_address;	
}

short NetGetStreamSocketNumber(
	void)
{
	assert(transport_type==kNetworkTransportType);

#if defined(mac) && !HAVE_SDL_NET
	return dspConnection->socketNum;
#else
        //PORTGUESS we should usually keep port in network byte order?
	return SDL_SwapBE16(network_preferences->game_port);
#endif
}

bool NetTransportAvailable(
	short type)
{
	bool available;
	
	switch(type)
	{
		case kNetworkTransportType:
#if defined(mac) && !HAVE_SDL_NET
			available= system_information->appletalk_is_available;
#elif HAVE_SDL_NET
			available= true;
#else
			available= false;
#endif
			break;
		case kModemTransportType:
#ifdef USE_MODEM
			available= modem_available();
			break;
#else
			available= false;
#endif
			break;

		default:
			assert(false);
			break;
	}

	return available;
}

/* ------------ code for stream implementations */
static OSErr stream_write(
	void *data,
	uint16 length)
{
	OSErr error;

	switch(transport_type)
	{
		case kNetworkTransportType:
			error= NetADSPWrite(dspConnection, data, &length);
			break;
			
		case kModemTransportType:
#ifdef USE_MODEM
			error= write_modem_endpoint(data, length, kNoTimeout);
			break;
#endif			
		default:
			assert(false);
			break;
	}

	return error;
}

static OSErr stream_read(
	void *data,
	uint16 *length)
{
	OSErr error;

	switch(transport_type)
	{
		case kNetworkTransportType:
			error= NetADSPRead(dspConnection, data, length);
			break;
			
		case kModemTransportType:
#ifdef USE_MODEM
			error= read_modem_endpoint(data, *length, kNoTimeout);
			break;
#endif			
		default:
			assert(false);
			break;
	}

	return error;
}
