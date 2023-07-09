#include "shell.h"
#include "world.h"
#include "FileHandler.h"
#include "shell_options.h"
#include <catch2/catch_test_macros.hpp>

extern ShellOptions shell_options;

using Replay = std::pair<std::string, uint16_t>; //replay file path and seed

static uint16_t get_seed_from_filename(const std::string& file_name) {
	auto position = file_name.find_last_of('.');
	auto name_without_ext = file_name.substr(0, position);
	auto seed_position = name_without_ext.find_last_of('.') + 1;
	return stoi(name_without_ext.substr(seed_position));
}

#ifndef REPLAY_SET_SEED_FILENAME //enable and run this to set the correct file name with seed on new replay files

static std::vector<Replay> get_replays(std::string& directory_path) {

	FileSpecifier directory = directory_path;

	std::vector<dir_entry> entries;
	directory.ReadDirectory(entries);

	std::vector<Replay> results;
	for (std::vector<dir_entry>::const_iterator it = entries.begin(); it != entries.end(); ++it) {

		FileSpecifier file = directory + it->name;

		if (file.GetType() != _typecode_film) continue;

		auto seed = get_seed_from_filename(it->name);
		results.push_back({ std::string(file.GetPath()), seed });
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

#else

static std::vector<std::string> get_replays(std::string& directory_path) {

	FileSpecifier directory = directory_path;

	std::vector<dir_entry> entries;
	directory.ReadDirectory(entries);

	std::vector<string> results;
	for (std::vector<dir_entry>::const_iterator it = entries.begin(); it != entries.end(); ++it) {

		FileSpecifier file = directory + it->name;

		if (file.GetType() != _typecode_film) continue;

		try {
			get_seed_from_filename(it->name);
		} catch (...) {
			results.push_back(file.GetPath());
		}
	}

	return results;
}

TEST_CASE("Film replay set seed") {

	REQUIRE(shell_options.replay_film_and_exit);
	REQUIRE(!shell_options.directory.empty());
	REQUIRE(!shell_options.replay_directory.empty());

	const auto replays = get_replays(shell_options.replay_directory);

	initialize_application();

	for (const auto& replay : replays) {

		REQUIRE(handle_open_document(replay));
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