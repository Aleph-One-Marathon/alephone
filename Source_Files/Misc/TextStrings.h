#ifndef _TEXT_STRINGS_
#define _TEXT_STRINGS_
/*
	Text-String Collection Interface
	by Loren Petrich,
	April 20, 2000
	
	These functions replace the getting of MacOS STR# resources,
	and are called in much the same fashion.
	
	Each string is a MacOS Pascal string (length byte + characters)
	with a null byte at the end, so it can also be treated as a C string.
	[length] [characters] [null]
	
	They are referenced by resource-ID number, which defines a string set,
	and an index inside that string set, which starts from 0.
*/


#include "XML_ElementParser.h"


// Set up a string in the repository; a repeated call will replace an old string
void TS_PutString(short ID, short Index, unsigned char *String);

// Returns a pointer to a string in Pascal form;
// if the ID and the index do not point to a valid string,
// this function will then return NULL
unsigned char *TS_GetString(short ID, short Index);

// Here is that string in C form
char *TS_GetCString(short ID, short Index);

// Checks on the presence of a string set
bool TS_IsPresent(short ID);

// Count the strings (contiguous from index zero)
unsigned short TS_CountStrings(short ID);

// Deletes a string, should one ever want to do that
void TS_DeleteString(short ID, short Index);

// Deletes the stringset with some ID
void TS_DeleteStringSet(short ID);

// Deletes all of the stringsets
void TS_DeleteAllStrings();


// Set up a text-string XML parser and return a pointer to it.
// Its name is "stringset".
// Don't try to delete it when one is finished with it
XML_ElementParser *TS_GetParser();

#endif
