#ifndef _PLAYER_NAME
#define _PLAYER_NAME
/*
	Player-name object and parser
	by Loren Petrich,
	April 30, 2000

	This is for handling the default name in netgames
*/


#include "XML_ElementParser.h"

// Get that name
unsigned char *GetPlayerName();

// Player-name parser: name is "player_name"
XML_ElementParser *PlayerName_GetParser();

#endif
