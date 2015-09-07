/*
	game_errors.c

	Copyright (C) 1991-2001 and beyond by Bungie Studios, Inc.
	and the "Aleph One" developers.
 
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	This license is contained in the file "COPYING",
	which is included with this source code; it is available online at
	http://www.gnu.org/licenses/gpl.html

	Monday, June 12, 1995 9:25:53 AM- rdm created.

*/

#include "cseries.h"
#include "game_errors.h"

static short last_type= systemError;
static short last_error= 0;

void set_game_error(
	short type, 
	short error_code)
{
	assert(type>=0 && type<NUMBER_OF_TYPES);
	last_type= type;
	last_error= error_code;
#ifdef DEBUG
	if(type==gameError) assert(error_code>=0 && error_code<NUMBER_OF_GAME_ERRORS);
#endif
}

short get_game_error(
	short *type)
{
	if(type)
	{
		*type= last_type;
	}
	
	return last_error;
}

bool error_pending(
	void)
{
	return (last_error!=0);
}

void clear_game_error(
	void)
{
	last_error= 0;
	last_type= 0;
}
