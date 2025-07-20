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

#ifndef __STANDALONE_HUB_H
#define __STANDALONE_HUB_H

#include "MessageInflater.h"
#include "network_messages.h"

#define STANDALONE_HUB_VERSION "01.01"

class StandaloneHub {
private:
	static std::unique_ptr<StandaloneHub> _instance;
	std::unique_ptr<CommunicationsChannelFactory> _server;
	std::shared_ptr<CommunicationsChannel> _gatherer;
	std::weak_ptr<CommunicationsChannel> _gatherer_client;
	std::unique_ptr<TopologyMessage> _topology_message;
	std::unique_ptr<LuaMessage> _lua_message;
	std::unique_ptr<MapMessage> _map_message;
	std::unique_ptr<PhysicsMessage> _physics_message;
	int _port;
	bool _start_game_signal = false;
	bool _end_game_signal = false;
	bool _gatherer_joined_as_client = false;
	bool _saved_game = false;
	int _start_check_timeout_ms = 0;
	static constexpr int _gathering_timeout_ms = 5 * 60 * 1000;
	StandaloneHub(uint16 port);
	bool GatherJoiners();
	bool CheckGathererCapabilities(const Capabilities* capabilities);
public:
	bool GetGameDataFromGatherer();
	bool SetupGathererGame(bool& gathering_done);
	bool WaitForGatherer();
	static bool Init(uint16 port);
	static StandaloneHub* Instance() { return _instance.get(); }
	static bool Reset();
	CommunicationsChannel* GetGathererChannel() const { return _gatherer ? _gatherer.get() : _gatherer_client.lock().get(); }
	void SendMessageToGatherer(const Message& message);
	void StartGame() { _start_game_signal = true; }
	void SetGameEnded(bool game_ended) { _end_game_signal = game_ended; }
	bool HasGameEnded() const { return _end_game_signal; }
	void SetSavedGame(bool saved_game) { _saved_game = saved_game; }
	void GathererJoinedAsClient() { _gatherer_joined_as_client = true; }
	int GetMapData(uint8** data);
	int GetPhysicsData(uint8** data);
	int GetLuaData(uint8** data);
};

#endif