#ifndef SHELL_OPTIONS_H
#define SHELL_OPTIONS_H

#include <string>
#include <vector>
#include <unordered_map>

struct ShellOptions {
	std::unordered_map<int, bool> parse(int argc, char** argv, bool ignore_unknown_args = false);

	std::string program_name;
	
	bool nogl;
	bool nosound;
	bool nogamma;
	bool debug;
	bool nojoystick;
	bool insecure_lua;

	bool force_fullscreen;
	bool force_windowed;

	bool skip_intro;
	bool editor;

	bool no_chooser;

	std::string replay_directory;

	std::string directory;
	std::vector<std::string> files;

	std::string output;
};

extern ShellOptions shell_options;

#endif
