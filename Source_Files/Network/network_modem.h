#ifndef __NETWORK_MODEM_H
#define __NETWORK_MODEM_H

/*
	network_modem.h

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

	Friday, September 29, 1995 11:05:14 PM- rdm created.

*/

enum {
	kNoTimeout= -1,
	kBestTry= 0
	/* anything else is the ticks for timeout.. */
};

OSErr initialize_modem_endpoint(void);
OSErr teardown_modem_endpoint(void);

bool call_modem_player(void);
bool answer_modem_player(void);

/* Ideally, packet data has only a checksum, and stream data knows how to ask */
/*  that the data be resent. */
OSErr write_modem_endpoint(void *data, uint16 length, long timeout);
OSErr read_modem_endpoint(void *data, uint16 length, long timeout);

void write_modem_endpoint_asynchronous(void *data, uint16 length, long timeout);
bool asynchronous_write_completed(void);

void idle_modem_endpoint(void);

bool modem_available(void);
long modem_read_bytes_available(void);

#endif
