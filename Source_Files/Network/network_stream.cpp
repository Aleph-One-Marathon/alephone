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
*/

#include "cseries.h"



#if defined(mac)
#include "macintosh_network.h"
#elif defined(SDL)
#include "sdl_network.h"
#endif

#include "network_modem.h"
#include "network_stream.h"

#include "shell.h" //--> only for system_information!

#ifdef env68k
	#pragma segment network_modem
#endif

// #define USE_MODEM

static short transport_type= kNetworkTransportType;
static ConnectionEndPtr dspConnection; /* our adsp connection */
static AddrBlock adsp_end_address;

/* ----- local prototypes */
static OSErr stream_write(void *data, uint16 length);
static OSErr stream_read(void *data, uint16 *length);

/* ------ code */
OSErr NetSendStreamPacket(
	short packet_type,
	void *packet_data)
{
	OSErr error;
	uint16 packet_length;

	packet_length= NetStreamPacketLength(packet_type);

        // ZZZ: byte-swap packet type code if necessary
	short	packet_type_NET;
    
#ifndef mac
        packet_type_NET = SDL_SwapBE16(packet_type);
#else
        packet_type_NET = packet_type;
#endif
        
	error= stream_write(&packet_type_NET, sizeof(short));
	if(!error)
	{
		error= stream_write(packet_data, packet_length);
	}

	return error;
}

OSErr NetReceiveStreamPacket(
	short *packet_type,
	void *buffer) /* Must be large enough for the biggest packet type.. */
{
	OSErr error;
	uint16 length;
	
	length= sizeof(short);

        // ZZZ: byte-swap if necessary
        short	packet_type_NET;

	error= stream_read(&packet_type_NET, &length);
        
#ifndef mac
        *packet_type = SDL_SwapBE16(packet_type_NET);
#else
        *packet_type = packet_type_NET;
#endif

	if(!error)
	{
		length= NetStreamPacketLength(*packet_type);
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
			#ifndef mac
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
			// LP change:
			assert(false);
			// halt();
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
			// LP change:
			assert(false);
			// halt();
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
			// LP change:
			assert(false);
			// halt();
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
			// LP change:
			assert(false);
			// halt();
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
			// LP change:
			assert(false);
			// halt();
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
			error= NetADSPWaitForConnection(dspConnection);
			break;
			
		case kModemTransportType:
#ifdef USE_MODEM
			error= noErr;
			break;

#endif			
		default:
			// LP change:
			assert(false);
			// halt();
			break;
	}

	return error;
}

void NetGetStreamAddress(
	AddrBlock *address)
{
	assert(transport_type==kNetworkTransportType);
	
	*address= adsp_end_address;	
	
	return;
}

short NetGetStreamSocketNumber(
	void)
{
	assert(transport_type==kNetworkTransportType);

#ifdef mac
	return dspConnection->socketNum;
#else
        //PORTGUESS we should usually keep port in network byte order?
	return SDL_SwapBE16(DEFAULT_PORT);
#endif
}

bool NetTransportAvailable(
	short type)
{
	bool available;
	
	switch(type)
	{
		case kNetworkTransportType:
#if defined(mac)
			available= system_information->appletalk_is_available;
#elif defined(HAVE_SDL_NET)
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
			// LP change:
			assert(false);
			// halt();
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
			// LP change:
			assert(false);
			// halt();
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
			// LP change:
			assert(false);
			// halt();
			break;
	}

	return error;
}
