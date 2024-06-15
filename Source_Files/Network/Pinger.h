#ifndef __PINGER_H
#define __PINGER_H

#if !defined(DISABLE_NETWORKING)

#include <SDL2/SDL_net.h>
#include <unordered_map>
#include <atomic>

class Pinger
{
public:
	uint16_t Register(const IPaddress& ipv4);
	void Ping(uint8_t number_of_tries = 1);
	std::unordered_map<uint16_t, uint16_t> GetResponseTime(uint16_t timeout_ms);
	void StoreResponse(uint16_t identifier);
private:

	struct PingAddress
	{
		IPaddress ipv4;
		uint32_t ping_sent_tick = 0;
		std::atomic_uint32_t pong_received_tick = 0;
		PingAddress(const IPaddress& address) : ipv4(address) {}
	};

    uint16_t _ping_identifier_counter = 0;
	std::unordered_map<uint16_t, PingAddress> _registered_ipv4s;
};

#endif
#endif