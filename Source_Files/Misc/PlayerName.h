#ifndef _PLAYER_NAME
#define _PLAYER_NAME
/*

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

	Player-name object and parser
	by Loren Petrich,
	April 30, 2000

	This is for handling the default name in netgames
*/


// Get that name
const char *GetPlayerName();

class InfoTree;
void parse_mml_player_name(const InfoTree& root);
void reset_mml_player_name();

#endif
