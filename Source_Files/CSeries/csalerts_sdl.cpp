/*

	Copyright (C) 1991-2001 and beyond by Bungie Studios, Inc.
	and the "Aleph One" developers.
 
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	This license is contained in the file "GNU_GeneralPublicLicense.txt",
	which is included with this source code; it is available online at
	http://www.gnu.org/licenses/gpl.html

*/
/*
 *  csalerts_sdl.cpp - Game alerts and debugging support, SDL implementation
 *
 *  Written in 2000 by Christian Bauer
 */

#include "cseries.h"

#include <stdio.h>

extern "C" void debugger(const char *message);


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

void _alephone_assert(char *file, long line, char *what)
{
	vhalt(csprintf(assert_text, "%s:%ld: %s", file, line, what));
}

void _alephone_warn(char *file, long line, char *what)
{
	vpause(csprintf(assert_text, "%s:%ld: %s", file, line, what));
}
