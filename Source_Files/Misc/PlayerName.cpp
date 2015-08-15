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

	This is for handling the player's name in netgames
*/

#include "cseries.h"

#include "PlayerName.h"
#include "InfoTree.h"
#include <string.h>

static char PlayerName[256];


// Get that name
const char *GetPlayerName() {return PlayerName;}


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
	DeUTF8_C(String,Length,PlayerName,255);
	
	return true;
}

static XML_SimpleStringParser PlayerNameParser;



// Player-name parser: name is "player_name"
XML_ElementParser *PlayerName_GetParser()
{
	const char DefaultPlayerName[] = "Marathon Player";
	size_t Length = strlen(DefaultPlayerName);
	assert(Length == static_cast<size_t>(static_cast<char>(Length)));
	PlayerName[0] = (char)Length;
	memcpy(PlayerName+1,DefaultPlayerName,Length);
	
	return &PlayerNameParser;
}

void reset_mml_player_name()
{
	// no reset
}

void parse_mml_player_name(const InfoTree& root)
{
	boost::optional<std::string> name_opt;
	if ((name_opt = root.get_value_optional<std::string>()))
		DeUTF8_C(name_opt->c_str(), name_opt->size(), PlayerName, 255);
}
