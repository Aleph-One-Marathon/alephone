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
 *  network_tcp.cpp - TCP network functions (corresponds to AppleTalk ADSP)
 *
 *  Written in 2000 by Christian Bauer
 */

#include "cseries.h"
#include "sdl_network.h"


/*
 *  Initialize/shutdown module
 */

OSErr NetADSPOpen(void)
{
printf("NetADSPOpen\n");
	// nothing to do
	return 0;
}

OSErr NetADSPClose(void)
{
printf("NetADSPClose\n");
	// nothing to do
	return 0;
}


/*
 *  Create connection endpoint
 */

OSErr NetADSPEstablishConnectionEnd(ConnectionEndPtr *connection)
{
printf("NetADSPEstablishConnectionEnd\n");
	// Allocate endpoint data structure
	ConnectionEndPtr connectionEnd = (ConnectionEndPtr)malloc(sizeof(ConnectionEnd));
	if (connectionEnd == NULL)
		return -1;
	connectionEnd->socket = connectionEnd->server_socket = NULL;
	*connection = connectionEnd;
	return 0;
}


/*
 *  Dispose of connection endpoint
 */

OSErr NetADSPDisposeConnectionEnd(ConnectionEndPtr connectionEnd)
{
printf("NetADSPDisposeConnectionEnd\n");
	if (connectionEnd == NULL)
		return 0;

	// Close sockets
	if (connectionEnd->socket)
		SDLNet_TCP_Close(connectionEnd->socket);
	if (connectionEnd->server_socket)
		SDLNet_TCP_Close(connectionEnd->server_socket);

	// Free endpoint data structure
	free(connectionEnd);
	return 0;
}


/*
 *  Open connection to remote machine
 */

OSErr NetADSPOpenConnection(ConnectionEndPtr connectionEnd, AddrBlock *address)
{
printf("NetADSPOpenConnection\n");
	// Open socket
	connectionEnd->socket = SDLNet_TCP_Open(address);
	return connectionEnd->socket ? 0 : -1;
}


/*
 *  Close connection to remote machine
 */

OSErr NetADSPCloseConnection(ConnectionEndPtr connectionEnd, bool abort)
{
printf("NetADSPCloseConnection\n");
	// Close socket
	if (connectionEnd->socket) {
		SDLNet_TCP_Close(connectionEnd->socket);
		connectionEnd->socket = NULL;
	}
	return 0;
}


/*
 *  Open server socket, wait (asynchronously) for connection from remote machine
 */

OSErr NetADSPWaitForConnection(ConnectionEndPtr connectionEnd)
{
printf("NetADSPWaitForConnection\n");
	// Close old sockets
	if (connectionEnd->socket) {
		SDLNet_TCP_Close(connectionEnd->socket);
		connectionEnd->socket = NULL;
	}
	if (connectionEnd->server_socket) {
		SDLNet_TCP_Close(connectionEnd->server_socket);
		connectionEnd->server_socket = NULL;
	}

	// Open server socket
	IPaddress addr = {INADDR_NONE, DEFAULT_PORT};
	connectionEnd->server_socket = SDLNet_TCP_Open(&addr);
	return connectionEnd->server_socket ? 0 : -1;
}


/*
 *  Check status of connection, return peer address
 */

bool NetADSPCheckConnectionStatus(ConnectionEndPtr connectionEnd, AddrBlock *address)
{
printf("NetADSPCheckConnectionStatus\n");
	if (connectionEnd->socket) {
		if (address) {
			AddrBlock *remote = SDLNet_TCP_GetPeerAddress(connectionEnd->socket);
			if (remote)
				*address = *remote;
		}
		return true;
	} else
		return false;
}


/*
 *  Write data to stream
 */

OSErr NetADSPWrite(ConnectionEndPtr connectionEnd, void *buffer, uint16 *count)
{
printf("NetADSPWrite\n");
	int actual = SDLNet_TCP_Send(connectionEnd->socket, buffer, *count);
	return actual == *count ? 0 : -1;
}


/*
 *  Read data from stream
 */

OSErr NetADSPRead(ConnectionEndPtr connectionEnd, void *buffer, uint16 *count)
{
printf("NetADSPRead\n");
	int actual = SDLNet_TCP_Recv(connectionEnd->socket, buffer, *count);
	*count = actual;
	return actual > 0 ? 0 : -1;
}
