#ifndef __STANDALONE_HUB_H
#define __STANDALONE_HUB_H

#include "MessageInflater.h"
#include "network_messages.h"

#define STANDALONE_HUB_VERSION "01.00"

class StandaloneHub {
private:
	static StandaloneHub* _instance;
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
	int _start_check_timeout_ms = 0;
	static constexpr int _gathering_timeout_ms = 5 * 60 * 1000;
	StandaloneHub(uint16 port);
	~StandaloneHub();
	bool GatherJoiners();
	bool CheckGathererCapabilities(const Capabilities* capabilities);
public:
	bool GetGameDataFromGatherer();
	bool SetupGathererGame(bool& gathering_done);
	bool WaitForGatherer();
	static bool Init(uint16 port);
	static StandaloneHub* Instance() { return _instance; }
	static bool Reset();
	CommunicationsChannel* GetGathererChannel() const { return _gatherer ? _gatherer.get() : _gatherer_client.lock().get(); }
	void SendMessageToGatherer(const Message& message);
	void StartGame() { _start_game_signal = true; }
	void SetGameEnded(bool game_ended) { _end_game_signal = game_ended; }
	bool HasGameEnded() const { return _end_game_signal; }
	void GathererJoinedAsClient() { _gatherer_joined_as_client = true; }
	int GetMapData(uint8** data);
	int GetPhysicsData(uint8** data);
	int GetLuaData(uint8** data);
};

#endif