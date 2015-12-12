/*
 *  network_lookup_sdl.h - SDL network lookup stuff

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

 *  Sept-Nov 2001 (Woody Zenfell): split some prototypes out of sdl_network.h to localize
 *	changes and reduce recompilation
 *
 *  Sept-Nov 2001 (Woody Zenfell): retooled (weakened) this interface to expose more of the
 *	underlying SSLP implementation, to make things a little simpler
 */

#ifndef NETWORK_LOOKUP_SDL_H
#define NETWORK_LOOKUP_SDL_H

#include "cseries.h"  // need OSErr
#include	"SSLP_API.h"

/* ---------- prototypes/NETWORK_NAMES.C */

// ZZZ: added support for SSLP hinting
OSErr NetRegisterName(const unsigned char *name, const unsigned char *type,
					  short version, short socketNumber, const char* hint_addr_string);
OSErr NetUnRegisterName(void);

/* ---------- prototypes/NETWORK_LOOKUP.C */

// Now unused - as long as SSLP_Pump() is called, e.g. by the dialog, it's unnecessary.
//void NetLookupUpdate(void);

void NetLookupClose(void);

// ZZZ: changed this interface to be more SSLP- and SDL-dialog-friendly
// I have renamed it since its interface has changed quite a bit - if this is a bad idea,
// feel free to name it back.
// Note it still takes a Pstring for maximum compatibility with MacOS version.
OSErr NetLookupOpen_SSLP(const unsigned char *type, short version,
        SSLP_Service_Instance_Status_Changed_Callback foundInstance,
        SSLP_Service_Instance_Status_Changed_Callback lostInstance,
        SSLP_Service_Instance_Status_Changed_Callback nameChanged
);

// Now unused - functionality handled in w_found_players widget type
//void NetLookupRemove(short index);
//void NetLookupInformation(short index, NetAddrBlock *address, NetEntityName *entity);

#endif//NETWORK_LOOKUP_SDL_H
