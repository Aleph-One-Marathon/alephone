#if !defined(DISABLE_NETWORKING)

#include "Pinger.h"
#include "network_private.h"
#include "network_star.h"
#include "crc.h"
#include "mytm.h"
#include <AStream.h>

uint16_t Pinger::Register(const IPaddress& ipv4)
{
	_registered_ipv4s.emplace( ++_ping_identifier_counter, ipv4 );
	return _ping_identifier_counter;
}

void Pinger::Ping(uint8_t number_of_tries)
{
	number_of_tries = std::max(number_of_tries, (uint8_t)1);

	for (auto& [identifier, address] : _registered_ipv4s)
	{
		address.ping_sent_tick = 0;
		address.pong_received_tick = 0;

		auto ping_packet = NetDDPNewFrame();

		try
		{
			AOStreamBE hdr(ping_packet->data, kStarPacketHeaderSize);
			AOStreamBE ops(ping_packet->data, ddpMaxData, kStarPacketHeaderSize);

			hdr << (uint16_t)kPingRequestPacket;
			ops << identifier;

			ping_packet->data[2] = 0;
			ping_packet->data[3] = 0;

			auto crc = calculate_data_crc_ccitt(ping_packet->data, ops.tellp());
			hdr << crc;

			ping_packet->data_size = ops.tellp();

			if (take_mytm_mutex())
			{
				for (int i = 0; i < number_of_tries; i++)
				{
					if (NetDDPSendFrame(ping_packet, &address.ipv4, kPROTOCOL_TYPE, 0) == 0 && !address.ping_sent_tick)
					{
						address.ping_sent_tick = machine_tick_count();
					}
				}

				release_mytm_mutex();
			}

		}
		catch (...) {}

		NetDDPDisposeFrame(ping_packet);
	}
}

std::unordered_map<uint16_t, uint16_t> Pinger::GetResponseTime(uint16_t timeout_ms)
{
	std::unordered_map<uint16_t, uint16_t> results;
	if (_registered_ipv4s.empty()) return results;

	auto start_time = machine_tick_count();

	while (machine_tick_count() - start_time < timeout_ms)
	{
		for (auto& [identifier, address] : _registered_ipv4s)
		{
			if (results.find(identifier) != results.end()) continue;

			if (!address.ping_sent_tick)
			{
				results.insert({ identifier, UINT16_MAX });
				continue;
			}

			if (auto response_tick = address.pong_received_tick.load())
			{
				results.insert({ identifier, response_tick - address.ping_sent_tick});
			}
		}

		if (results.size() == _registered_ipv4s.size()) return results; //got response for each registered address

		sleep_for_machine_ticks(1);
	}

	for (auto& [identifier, address] : _registered_ipv4s)
	{
		if (results.find(identifier) == results.end())
		{
			results.insert({ identifier, UINT16_MAX });
		}
	}

	return results;
}

void Pinger::StoreResponse(uint16_t identifier)
{
	auto ping_request = _registered_ipv4s.find(identifier);

	if (ping_request != _registered_ipv4s.end() && !ping_request->second.pong_received_tick)
	{
		ping_request->second.pong_received_tick = machine_tick_count();
	}
}

#endif