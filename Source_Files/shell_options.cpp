#include "shell_options.h"

#include <iostream>
#include <functional>
#include <sstream>

#include "FileHandler.h"
#include "Logging.h"
#include "csstrings.h"

#ifdef __WIN32__
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef CreateDirectory
#endif

static void print_usage();
static void print_version();

ShellOptions shell_options;

struct ShellOptionsOption {
	bool match(const std::string& s) {
		if (s[0] == '-') {
			if (s[1] == '-') {
				return long_name == s.substr(2);
			} else if (short_name.size()) {
				return short_name == s.substr(1);
			}
		}

		return false;
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
		auto num_spaces = help_tab_stop - 12 - o.long_name.size();
		
		s << "\t[";
		if (o.short_name.size())
		{
			num_spaces -= 4;
			num_spaces -= o.short_name.size();
			s << "-" << o.short_name << " | ";
		}
		
		s << "--" << o.long_name << "]" << spaces(num_spaces) << o.help << "\n";
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

struct ShellOptionsString : public ShellOptionsOption {
	// once we switch to C++17 this can go away and we can use aggregate
	// initializers
	ShellOptionsString(std::string short_name, std::string long_name, std::string help, std::string& string_) :
		ShellOptionsOption{short_name, long_name, help},
		string{string_}
		{ }

	std::string& string;
};

static const std::vector<ShellOptionsCommand> shell_options_commands {
	{"h", "help", "このヘルプメッセージを表示します", print_usage},
	{"v", "version", "ゲームのバージョンを表示します", print_version}
};

static const std::vector<ShellOptionsFlag> shell_options_flags {
	{"d", "debug", "コアダンプを出力するようにします", shell_options.debug},
	{"f", "fullscreen", "ゲームをフルスクリーンで起動します", shell_options.force_fullscreen},
	{"w", "windowed", "ゲームをウィンドウモードで起動します", shell_options.force_windowed},
	{"g", "nogl", "OpenGLを使用せずに起動します", shell_options.nogl},
	{"s", "nosound", "サウンドを無効化します", shell_options.nosound},
	{"m", "nogamma", "（メニューのフェードなどの）ガンマエフェクトを無効化します", shell_options.nogamma},
	{"j", "nojoystick", "ジョイスティックの初期化を行いません", shell_options.nojoystick},
	{"i", "insecure_lua", "", shell_options.insecure_lua},
	{"Q", "skip-intro", "イントロ画面を飛ばします", shell_options.skip_intro},
	{"e", "editor", "エディタ設定を使い、マップに直接ジャンプします", shell_options.editor}
};

static const std::vector<ShellOptionsString> shell_options_strings {
	{"o", "output", "-eと一緒に、ゲーム終了時に[file]へ出力して終わります", shell_options.output}
};

bool ShellOptions::parse(int argc, char** argv)
{
	shell_options.program_name = argv[0];
	--argc;
	++argv;

    std::vector<std::string> args;
    while (argc > 0)
    {
        if (strncmp(*argv, "-C", 2) == 0)
        {
            args.push_back(*argv + 2);
        }
        else
        {
            args.push_back(*argv);
        }

        --argc;
        ++argv;
    }

    for (auto it = args.begin(); it != args.end(); ++it)
    {
		bool found = false;

		for (auto command : shell_options_commands)
		{
			if (command.match(*it))
			{
				command.command();
				exit(0);
			}
		}

		for (auto flag : shell_options_flags)
		{
			if (flag.match(*it))
			{
				found = true;
				flag.flag = true;
				break;
			}
		}

		for (auto option : shell_options_strings)
		{
			if (option.match(*it))
			{
                if (it != args.end() && (*(++it))[0] != '-')
                {
					found = true;
                    option.string = *it;
                }
                else
                {
					logFatal("%s requires an additional argument", it->c_str());
                    printf("%s requires an additional argument\n", it->c_str());
                    print_usage();
                    exit(1);
                }
			}
		}

		if (!found)
		{
			if ((*it)[0] != '-')
			{
				FileSpecifier f(*it);
				if (f.IsDir())
				{
					shell_options.directory = *it;
				}
				else
				{
					shell_options.files.push_back(*it);
				}
			}
			else
			{
				logFatal("Unrecognized argument '%s'.", it->c_str());
				printf("Unrecognized argument '%s'.\n", it->c_str());
				print_usage();
				exit(1);
			}
		}
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
	oss << "コマンドラインスイッチ：\n\n";
#else
	oss << "\n使用方法: " << shell_options.program_name << " [オプション] [ディレクトリ] [ファイル]\n";
#endif

	for (auto command : shell_options_commands)
	{
		oss << command;
	}

	for (auto flag : shell_options_flags)
	{
		oss << flag;
	}

	for (auto option : shell_options_strings)
	{
		oss << option;
	}

	oss << "\tディレクトリ" << spaces(help_tab_stop - 12 - 8)
		<< "データーが含まれているディレクトリ\n"
		
		<< "\tfile" << spaces(help_tab_stop - 8 - 8)
		<< "保存されたゲームやフィルムの再生\n"
		<< "\n"
		<< "Yこの他にも、環境変数「ALEPHONE_DATA」の値を変更することで、\n"
		<< "データディレクトリを指定することができます。\n";

#ifdef __WIN32__
	MessageBoxW(NULL, utf8_to_wide(oss.str()).c_str(), L"使用方", MB_OK | MB_ICONINFORMATION);
#else
	std::cout << oss.str();
#endif
}
