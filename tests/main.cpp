#include <catch2/catch_session.hpp>
#include "shell_options.h"

extern ShellOptions shell_options;

int main(int argc, char* argv[]) {

	auto results = shell_options.parse(argc, argv, true);

	int argc_catch = 0;
	char* argv_catch[64];

	for (int i = 0; i < argc; i++) {

		if (results.find(i) == results.end() || !results.at(i)) {
			argv_catch[argc_catch] = argv[i];
			argc_catch++;
		}
	}

	return Catch::Session().run(argc_catch, argv_catch);
}
