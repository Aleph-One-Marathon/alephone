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

#include "StandaloneHub.h"

std::unique_ptr<StandaloneHub> StandaloneHub::_instance;

bool StandaloneHub::Init(uint16 port)
{
	if (_instance) return true;
	if (!port) return false;
	return NetEnter(false) && (_instance = std::unique_ptr<StandaloneHub>(new StandaloneHub(port)));
}

StandaloneHub::StandaloneHub(uint16 port)
{
	_port = port;
	_server = std::make_unique<CommunicationsChannelFactory>(port);
}

bool StandaloneHub::SetupGathererGame(bool& gathering_done)
{
	gathering_done = false;

	_gatherer->enqueueOutgoingMessage(RemoteHubReadyMessage());

	if (!NetProcessNewJoiner(_gatherer)) return Reset();

	_gatherer_client = std::weak_ptr<CommunicationsChannel>(_gatherer);
	_gatherer.reset();

	_start_check_timeout_ms = machine_tick_count();

	if (!NetGather(&_topology_message->topology()->game_data, sizeof(game_info), &_topology_message->topology()->players->player_data, sizeof(player_info), _saved_game, false))
	{
		return Reset();
	}

	if (!GatherJoiners())
	{
		NetCancelGather();
		return Reset();
	}

	gathering_done = true;
	return true;
}

bool StandaloneHub::WaitForGatherer()
{
	if (_gatherer || !_gatherer_client.expired()) return true;

	_gatherer = std::shared_ptr<CommunicationsChannel>(_server->newIncomingConnection());

	if (!_gatherer) return false;

	NetSetDefaultInflater(_gatherer.get());

	auto gatherer_request = std::unique_ptr<RemoteHubHostConnectMessage>(_gatherer->receiveSpecificMessage<RemoteHubHostConnectMessage>(3000u, 3000u));

	if (!gatherer_request)
	{
		_gatherer.reset();
		return false;
	}

	bool can_use_hub = gatherer_request->version() == kNetworkSetupProtocolID;

	if (can_use_hub)
	{
		auto gatherer_capabilities = std::unique_ptr<CapabilitiesMessage>(_gatherer->receiveSpecificMessage<CapabilitiesMessage>(5000u, 5000u));

		if (can_use_hub = gatherer_capabilities && CheckGathererCapabilities(gatherer_capabilities->capabilities()))
		{
			NetSetCapabilities(gatherer_capabilities->capabilities());
		}
	}

	_gatherer->enqueueOutgoingMessage(RemoteHubHostResponseMessage(can_use_hub));
	_gatherer->flushOutgoingMessages(false);

	if (!can_use_hub) _gatherer.reset();

	return can_use_hub;
}

bool StandaloneHub::CheckGathererCapabilities(const Capabilities* capabilities)
{
	auto gatherer_capabilities = *capabilities;

	return
		gatherer_capabilities[Capabilities::kStar] == Capabilities::kStarVersion &&
		gatherer_capabilities[Capabilities::kGatherable] == Capabilities::kGatherableVersion &&
		gatherer_capabilities[Capabilities::kLua] == Capabilities::kLuaVersion &&
		gatherer_capabilities[Capabilities::kZippedData] == Capabilities::kZippedDataVersion;
}

bool StandaloneHub::Reset()
{
	auto port = _instance->_port;
	_instance.reset();
	NetExit(); //must be called after instance was cleaned
	return Init(port);
}

bool StandaloneHub::GatherJoiners()
{
	while (!_gatherer_client.expired() && !_start_game_signal && machine_tick_count() - _start_check_timeout_ms < _gathering_timeout_ms)
	{
		prospective_joiner_info player;
		NetCheckForNewJoiner(player, _server.get(), _gatherer_joined_as_client);
		sleep_for_machine_ticks(1);
	}

	return _start_game_signal;
}

void StandaloneHub::SendMessageToGatherer(const Message& message)
{
	if (auto gatherer = _gatherer_client.lock()) 
	{
		gatherer->enqueueOutgoingMessage(message);
		gatherer->flushOutgoingMessages(false);
	}
}

bool StandaloneHub::GetGameDataFromGatherer()
{
	_map_message.reset();

	if (auto client = _gatherer ? _gatherer : _gatherer_client.lock())
	{
		while (auto message = client->receiveMessage())
		{
			switch (message->type())
			{
				case kTOPOLOGY_MESSAGE:
					if (_topology_message) delete message; //we don't expect to get a topology message while transiting to another level
					else _topology_message = std::unique_ptr<TopologyMessage>(static_cast<TopologyMessage*>(message));
					break;
				case kLUA_MESSAGE:
				case kZIPPED_LUA_MESSAGE:
				{
					const auto lua_message = static_cast<LuaMessage*>(message);
					std::vector<byte> buffer(lua_message->buffer(), lua_message->buffer() + lua_message->length());
					DeferredScriptSend(buffer);
					delete lua_message;
					break;
				}
				case kPHYSICS_MESSAGE:
				case kZIPPED_PHYSICS_MESSAGE:
					_physics_message = std::unique_ptr<PhysicsMessage>(static_cast<PhysicsMessage*>(message));
					break;
				case kMAP_MESSAGE:
				case kZIPPED_MAP_MESSAGE:
					_map_message = std::unique_ptr<MapMessage>(static_cast<MapMessage*>(message));
					break;
				case kEND_GAME_DATA_MESSAGE:
					delete message;
					return _map_message && _topology_message;
				default:
					delete message;
					break;
			}
		}
	}

	return false;
}

int StandaloneHub::GetMapData(uint8** data)
{
	if (!_map_message) return 0;

	*data = _map_message->buffer();
	return _map_message->length();
}

int StandaloneHub::GetPhysicsData(uint8** data)
{
	if (!_physics_message) return 0;

	*data = _physics_message->buffer();
	return _physics_message->length();
}