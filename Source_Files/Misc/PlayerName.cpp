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

	Player-name object and parser
	by Loren Petrich,
	April 30, 2000

	This is for handling the player's name in netgames
*/

#include "cseries.h"

#include "PlayerName.h"
#include <string.h>

static unsigned char PlayerName[256];


// Get that name
unsigned char *GetPlayerName() {return PlayerName;}


class XML_SimpleStringParser: public XML_ElementParser
{
	// Was the string loaded? If not, then load a blank string at the end
	bool StringLoaded;

public:
	// Callbacks
	bool HandleString(const char *String, int Length);
	
	// The string's index value
	short Index;

	XML_SimpleStringParser(): XML_ElementParser("player_name") {}
};

bool XML_SimpleStringParser::HandleString(const char *String, int Length)
{
	// Copy into Pascal string
	// OK since this is called by value
	if (Length > 255) Length = 255;
	
	memcpy(PlayerName+1,String,Length);
	
	return true;
}

static XML_SimpleStringParser PlayerNameParser;



// Player-name parser: name is "player_name"
XML_ElementParser *PlayerName_GetParser()
{
	const char DefaultPlayerName[] = "Marathon Player";
	int Length = strlen(DefaultPlayerName);
	PlayerName[0] = Length;
	memcpy(PlayerName+1,DefaultPlayerName,Length);
	
	return &PlayerNameParser;
}
