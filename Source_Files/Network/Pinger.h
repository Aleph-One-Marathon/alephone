/*
	Copyright (C) 2024 Benoit Hauquier and the "Aleph One" developers.

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

#ifndef __PINGER_H
#define __PINGER_H

#if !defined(DISABLE_NETWORKING)

#include "NetworkInterface.h"
#include <unordered_map>
#include <atomic>

class Pinger
{
public:
	uint16_t Register(const IPaddress& ipv4);
	void Ping(uint8_t number_of_tries = 1, bool unpinged_addresses_only = false);
	std::unordered_map<uint16_t, uint16_t> GetResponseTime(uint16_t timeout_ms = 0);
	void StoreResponse(uint16_t identifier, const IPaddress& address);
private:

	struct PingAddress
	{
		IPaddress ipv4;
		uint32_t ping_sent_tick = 0;
		std::atomic_uint32_t pong_received_tick = 0;
		PingAddress(const IPaddress& address) : ipv4(address) {}
	};

	static uint16_t _ping_identifier_counter;
	std::unordered_map<uint16_t, PingAddress> _registered_ipv4s;
};

#endif
#endif