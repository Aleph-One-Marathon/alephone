#include "shell.h"
#include "world.h"
#include "FileHandler.h"
#include "shell_options.h"
#include "interface.h"
#include "preferences.h"
#include <catch2/catch_test_macros.hpp>

extern ShellOptions shell_options;

using Replay = std::pair<std::string, uint16_t>; //replay file path and seed

static uint16_t get_seed_from_filename(const std::string& file_name) {
	auto position = file_name.find_last_of('.');
	auto name_without_ext = file_name.substr(0, position);
	auto seed_position = name_without_ext.find_last_of('.');
	if (seed_position == string::npos) throw std::exception();
	return stoi(name_without_ext.substr(seed_position + 1));
}

#ifndef REPLAY_SET_SEED_FILENAME //enable and run this to set the correct file name with seed on new replay files

static std::vector<Replay> get_replays(std::string& directory_path) {

	FileSpecifier directory = directory_path;

	std::vector<dir_entry> entries;
	directory.ReadDirectory(entries);

	std::vector<Replay> results;
	for (std::vector<dir_entry>::const_iterator it = entries.begin(); it != entries.end(); ++it) {

		FileSpecifier entry = directory + it->name;
		std::string entry_path = entry.GetPath();

		if (entry.IsDir()) {
			auto sub_replays = get_replays(entry_path);
			results.insert(results.end(), sub_replays.begin(), sub_replays.end());
		}
		else
		{
			if (entry.GetType() != _typecode_film) continue;

			auto seed = get_seed_from_filename(it->name);
			results.push_back({ entry_path, seed });
		}
	}

	return results;
}

static void set_replay_preferences() {
	graphics_preferences->fps_target = 60;
}

TEST_CASE("Film replay", "[Replay]") {

	REQUIRE(!shell_options.directory.empty());
	REQUIRE(!shell_options.replay_directory.empty());

	const auto replays = get_replays(shell_options.replay_directory);

	initialize_application();
	set_replay_preferences();

	for (const auto& replay : replays) {
		INFO(replay.first);
		REQUIRE(handle_open_document(replay.first));
		set_replay_speed(INT16_MAX);
		main_event_loop();
		auto seed = get_random_seed();
		CHECK(seed == replay.second);
	}

	shutdown_application();
}

#else

static std::vector<std::string> get_replays(std::string& directory_path) {

	FileSpecifier directory = directory_path;

	std::vector<dir_entry> entries;
	directory.ReadDirectory(entries);

	std::vector<string> results;
	for (std::vector<dir_entry>::const_iterator it = entries.begin(); it != entries.end(); ++it) {

		FileSpecifier entry = directory + it->name;
		std::string entry_path = entry.GetPath();

		if (entry.IsDir()) {
			auto sub_replays = get_replays(entry_path);
			results.insert(results.end(), sub_replays.begin(), sub_replays.end());
		}
		else
		{
			if (entry.GetType() != _typecode_film) continue;

			try {
				get_seed_from_filename(it->name);
			}
			catch (...) {
				results.push_back(entry_path);
			}
		}
	}

	return results;
}

TEST_CASE("Film replay set seed", "[Replay]") {

	REQUIRE(!shell_options.directory.empty());
	REQUIRE(!shell_options.replay_directory.empty());

	const auto replays = get_replays(shell_options.replay_directory);

	initialize_application();

	for (const auto& replay : replays) {
		INFO(replay);
		REQUIRE(handle_open_document(replay));
		set_replay_speed(INT16_MAX);
		main_event_loop();
		auto seed = get_random_seed();
		FileSpecifier file = replay;
		std::string directory, file_name;
		file.SplitPath(directory, file_name);
		auto position = file_name.find_last_of('.');
		auto name_without_ext = file_name.substr(0, position);
		auto name_with_seed = name_without_ext + "." + std::to_string(seed) + ".filA";
		FileSpecifier new_file = directory;
		new_file.AddPart(name_with_seed);
		REQUIRE(file.Rename(new_file));
	}

	shutdown_application();
}

#endif