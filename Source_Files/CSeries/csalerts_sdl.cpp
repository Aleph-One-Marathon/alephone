/*
 *  csalerts_sdl.cpp - Game alerts and debugging support, SDL implementation
 *
 *  Written in 2000 by Christian Bauer
 */

#include "cseries.h"

#include <stdio.h>

extern "C" void debugger(const char *message);

/*
 *  Display alert message
 */

void alert_user(short severity, short resid, short item, OSErr error)
{
	char str[256];
	fprintf(stderr, "%s: %s (error %d)\n", severity == infoError ? "INFO" : "FATAL", getcstr(str, resid, item), error);
	if (severity != infoError)
		exit(1);
}


/*
 *  Jump into debugger (and return)
 */

void pause_debug(void)
{
	fprintf(stderr, "pause\n");
}


/*
 *  Display message
 */

void vpause(char *message)
{
	fprintf(stderr, "vpause %s\n", message);
}


/*
 *  Jump into debugger (and don't return)
 */

void halt(void)
{
	fprintf(stderr, "halt\n");
	abort();
}


/*
 *  Display message and halt
 */

void vhalt(char *message)
{
	fprintf(stderr, "vhalt %s\n", message);
	abort();
}


/*
 *  Assertion failed
 */

static char assert_text[256];

void _assert(char *file, long line, char *what)
{
	vhalt(csprintf(assert_text, "%s:%ld: %s", file, line, what));
}

void _warn(char *file, long line, char *what)
{
	vpause(csprintf(assert_text, "%s:%ld: %s", file, line, what));
}
