/*
	Player-name object and parser
	by Loren Petrich,
	April 30, 2000

	This is for handling the player's name in netgames
*/

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
