#ifndef __PORT_FORWARD_H
#define __PORT_FORWARD_H

/*
UPNPC.H

	Copyright (C) 2021 and beyond by Gregory Smith
 
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

#include "cseries.h"

#ifdef HAVE_MINIUPNPC

#include <stdexcept>
#include <memory>

#include <miniupnpc/miniupnpc.h>

class PortForwardException : public std::runtime_error {
public:
	PortForwardException(const char* what) : std::runtime_error(what) { }
};

class PortForward
{
public:
	PortForward(uint16_t port); // both TCP+UDP
	~PortForward();

private:
	// wrap UPNP functions for automatic resource management
	using url_freer_t = std::unique_ptr<UPNPUrls, decltype(&FreeUPNPUrls)>;
	using devlist_freer_t = std::unique_ptr<UPNPDev, decltype(&freeUPNPDevlist)>;
	
	std::string port_;
	UPNPUrls urls_;
	IGDdatas data_;
	url_freer_t url_freer_;
};

#endif

#endif
