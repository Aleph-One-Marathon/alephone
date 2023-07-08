#include "shell.h"
#include "world.h"
#include "FileHandler.h"
#include "shell_options.h"
#include <catch2/catch_test_macros.hpp>

extern ShellOptions shell_options;

using Replay = std::pair<std::string, uint16_t>; //replay file path and seed

static std::vector<Replay> get_replays(std::string& directory_path) {

	FileSpecifier directory = directory_path;

	std::vector<dir_entry> entries;
	directory.ReadDirectory(entries);

	std::vector<Replay> results;
	for (std::vector<dir_entry>::const_iterator it = entries.begin(); it != entries.end(); ++it) {

		FileSpecifier file = directory + it->name;

		if (file.GetType() == _typecode_film) {

			auto position = it->name.find_last_of('.');
			auto name_without_ext = it->name.substr(0, position);
			auto seed_position = name_without_ext.find_last_of('.') + 1;
			uint16_t seed = stoi(name_without_ext.substr(seed_position));
			results.push_back({ std::string(file.GetPath()), seed });
		}
	}

	return results;
}

TEST_CASE("Film replay") {

	REQUIRE(shell_options.replay_film_and_exit);
	REQUIRE(!shell_options.directory.empty());
	REQUIRE(!shell_options.replay_directory.empty());

	const auto replays = get_replays(shell_options.replay_directory);

	initialize_application();

	for (const auto& replay : replays) {

		REQUIRE(handle_open_document(replay.first));
		main_event_loop();
		auto seed = get_random_seed();
		REQUIRE(seed == replay.second);
	}

	shutdown_application();
}