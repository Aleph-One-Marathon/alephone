#ifndef SHELL_OPTIONS_H
#define SHELL_OPTIONS_H

#include <string>
#include <vector>

struct ShellOptions {
	bool parse(int argc, char** argv);

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
	bool export_film;
	bool editor;

	std::string directory;
	std::vector<std::string> files;

	std::string output;
	std::string view_player;
};

extern ShellOptions shell_options;

#endif
