#ifndef __GAME_ERRORS_H
#define __GAME_ERRORS_H

/*
	game_errors.h

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

	This license is contained in the file "COPYING",
	which is included with this source code; it is available online at
	http://www.gnu.org/licenses/gpl.html

	Monday, June 12, 1995 9:24:29 AM- rdm created.

*/

enum { /* types */
	systemError,
	gameError,
	NUMBER_OF_TYPES
};

enum { /* Game Errors */
	errNone= 0,
	errMapFileNotSet,
	errIndexOutOfRange,
	errTooManyOpenFiles,
	errUnknownWadVersion,
	errWadIndexOutOfRange,
	errServerDied,
	errUnsyncOnLevelChange,
	NUMBER_OF_GAME_ERRORS
};

void set_game_error(short type, short error_code);
short get_game_error(short *type);
bool error_pending(void);
void clear_game_error(void);

#endif
