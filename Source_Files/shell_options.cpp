#include "shell_options.h"

#include <iostream>
#include <functional>
#include <sstream>

#include "FileHandler.h"
#include "csstrings.h"

static void print_usage();
static void print_version();

ShellOptions shell_options;

struct ShellOptionsOption {
	bool match(const char* s) {
		if (s[0] == '-') {
			if (s[1] == '-') {
				return long_name == s + 2;
			} else {
				return short_name == s + 1;
			}
		} else {
			return false;
		}
	}
	
	std::string short_name;
	std::string long_name;
	std::string help;
};

static int help_tab_stop = 33;

static std::string spaces(int num_spaces)
{
	std::string s;
	for (auto i = 0; i < num_spaces; ++i) {
		s += " ";
	}
	return s;
}

static std::ostream& operator<<(std::ostream& s, const ShellOptionsOption& o) {
	if (o.help.size())
	{
		s << "\t[-" << o.short_name
		  << " | --" << o.long_name << "]"
		  << spaces(help_tab_stop - 16 - o.short_name.size() - o.long_name.size())
		  << o.help << "\n";
	}

	return s;
}

struct ShellOptionsCommand : public ShellOptionsOption {
	// once we switch to C++17 this can go away and we can use aggregate
	// initializers
	ShellOptionsCommand(std::string short_name, std::string long_name, std::string help, std::function<void()> command_) :
		ShellOptionsOption{short_name, long_name, help},
		command{command_}
		{ }
	
	std::function<void()> command;
};

struct ShellOptionsFlag : public ShellOptionsOption {
	// once we switch to C++17 this can go away and we can use aggregate
	// initializers
	ShellOptionsFlag(std::string short_name, std::string long_name, std::string help, bool& flag_) :
		ShellOptionsOption{short_name, long_name, help},
		flag{flag_}
		{ }
	
	bool& flag;
};

static const std::vector<ShellOptionsCommand> shell_options_commands {
	{"h", "help", "Display this help message", print_usage},
	{"v", "version", "Display the game version", print_version}
};

static const std::vector<ShellOptionsFlag> shell_options_flags {
	{"d", "debug", "Allow saving of core files", shell_options.debug},
	{"f", "fullscreen", "Run the game fullscreen", shell_options.force_fullscreen},
	{"w", "windowed", "Run the game in a window", shell_options.force_windowed},
	{"g", "nogl", "Do not use OpenGL", shell_options.nogl},
	{"s", "nosound", "Do not access the sound card", shell_options.nosound},
	{"m", "nogamma", "Disable gamma table effects (menu fades)", shell_options.nogamma},
	{"j", "nojoystick", "Do not initialize joysticks", shell_options.nojoystick},
	{"i", "insecure_lua", "", shell_options.insecure_lua},
};

bool ShellOptions::parse(int argc, char** argv)
{
	shell_options.program_name = argv[0];
	--argc;
	++argv;

	while (argc > 0)
	{
		bool found = false;
		
		for (auto command : shell_options_commands)
		{
			if (command.match(*argv))
			{
				command.command();
				exit(0);
			}
		}

		for (auto flag : shell_options_flags)
		{
			if (flag.match(*argv))
			{
				found = true;
				flag.flag = true;
				break;
			}
		}

		if (!found)
		{
			if (*argv[0] != '-')
			{
				FileSpecifier f(*argv);
				if (f.IsDir())
				{
					shell_options.directory = *argv;
				}
				else
				{
					shell_options.files.push_back(*argv);
				}
			}
			else
			{
				
				printf("Unrecognized argument '%s'.\n", *argv);
				print_usage();
				exit(0);
			}
		}
			
		--argc;
		++argv;
	}

	return true;
}

void print_version()
{
	char app_name_version[256];
	expand_app_variables(app_name_version, "Aleph One $appLongVersion$");
	std::cout << app_name_version << std::endl;
}

void print_usage()
{
	std::ostringstream oss;

#ifdef __WIN32__
	oss << "Command line switches:\n\n";
#else
	oss << "\nUsage: " << shell_options.program_name << " [options] [directory] [file]\n";
#endif

	for (auto command : shell_options_commands)
	{
		oss << command;
	}

	for (auto flag : shell_options_flags)
	{
		oss << flag;
	}

	oss << "\tdirectory" << spaces(help_tab_stop - strlen("directory") - 8)
		<< "Directory containing scenario data files\n"
		
		<< "\tfile" << spaces(help_tab_stop - strlen("file") - 8)
		<< "Saved game to load or film to play\n"
		<< "\n"
		<< "You can also use the ALEPHONE_DATA environment variable to specify\n"
		<< "the data directory.\n";

#ifdef __WIN32__
	MessageBoxW(NULL, utf8_to_wide(msg).str().c_str(), L"Usage", MB_OK | MB_ICONINFORMATION);
#else
	std::cout << oss.str();
#endif
}
