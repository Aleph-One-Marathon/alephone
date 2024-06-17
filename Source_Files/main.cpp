#include "shell_options.h"
#include "shell.h"
#include "csstrings.h"
#include "Logging.h"
#include "alephversion.h"
#include <SDL2/SDL_main.h>

int main(int argc, char** argv)
{
	// Print banner (don't bother if this doesn't appear when started from a GUI)
	char app_name_version[256];
	expand_app_variables(app_name_version, "Aleph One $appLongVersion$");
	printf("%s\n%s\n\n"
		"Original code by Bungie Software <http://www.bungie.com/>\n"
		"Additional work by Loren Petrich, Chris Pruett, Rhys Hill et al.\n"
		"TCP/IP networking by Woody Zenfell\n"
		"SDL port by Christian Bauer <Christian.Bauer@uni-mainz.de>\n"
#if defined(__MACH__) && defined(__APPLE__)
		"Mac OS X/SDL version by Chris Lovell, Alexander Strange, and Woody Zenfell\n"
#endif
		"\nThis is free software with ABSOLUTELY NO WARRANTY.\n"
		"You are welcome to redistribute it under certain conditions.\n"
		"For details, see the file COPYING.\n"
#if defined(__WIN32__)
		// Windows is statically linked against SDL, so we have to include this:
		"\nSimple DirectMedia Layer (SDL) Library included under the terms of the\n"
		"GNU Library General Public License.\n"
		"For details, see the file COPYING.SDL.\n"
#endif
#if !defined(DISABLE_NETWORKING)
		"\nBuilt with network play enabled.\n"
#endif
		, app_name_version, A1_HOMEPAGE_URL
	);

	shell_options.parse(argc, argv);

	auto code = 0;

	try {

		// Initialize everything
		initialize_application();

		for (std::vector<std::string>::iterator it = shell_options.files.begin(); it != shell_options.files.end(); ++it)
		{
			if (handle_open_document(*it))
			{
				break;
			}
		}

		// Run the main loop
		main_event_loop();

	}
	catch (std::exception& e) {
		try
		{
			logFatal("Unhandled exception: %s", e.what());
		}
		catch (...)
		{
		}
		code = 1;
	}
	catch (...) {
		try
		{
			logFatal("Unknown exception");
		}
		catch (...)
		{
		}
		code = 1;
	}

	try
	{
		shutdown_application();
	}
	catch (...)
	{

	}

	return code;
}