#ifndef _FONT_HANDLER_
#define _FONT_HANDLER_

/*
	Font handler
	by Loren Petrich,
	December 17, 2000
	
	This is for specifying and working with text fonts;
	specifying them, changing them with XML, and creating a font brush
	for OpenGL rendering
*/


#include "cseries.h"
#include "XML_ElementParser.h"


struct FontSpecifier
{
	enum {
		// Text styles (additive); these are MacOS style-flag values
		Normal = 0,
		Bold = 1,
		Italic = 2,
		Underline = 4,
		// How many characters
		NameSetLen = 64
	};
	// Name in HTML-fontname style "font1, font2, font3";
	// use the first of these fonts that are present,
	// otherwise use some sensible default
	char NameSet[NameSetLen];
	short Size;
	short Style;
	
	// How tall is the font?
	short GetHeight();
	
	// How much from line to line?
	short GetLineSpacing();
	
	// Use this font; has this form because MacOS Quickdraw has a single global font setting
	void Use();

	// Equality and assignment operators
	bool operator==(FontSpecifier& F);
	bool operator!=(FontSpecifier& F)
		{return (!((*this) == F));}
	FontSpecifier& operator=(FontSpecifier& F);
	
	// Search functions for names; call these in alternation on a pointer to the current name,
	// which is initialized to the name-string pointer above
	
	// Given a pointer to somewhere in a name set, returns the pointer
	// to the start of the next name, or NULL if it did not find any
	static char *FindNextName(char *NamePtr);
	
	// Given a pointer to the beginning of a name, finds the pointer to the character
	// just after the end of that name
	static char *FindNameEnd(char *NamePtr);
	
	// Find the MacOS font ID from the font name
	short GetFontID();
};

// Returns a parser for the fonts;
// several elements may have colors, so this ought to be callable several times.
XML_ElementParser *Font_GetParser();

// This sets the list of fonts to be read into.
// Its args are the pointer to that list and the number of fonts in it.
// If that number is <= 0, then the color value is assumed to be non-indexed,
// and no "index" attribute will be searched for.
void Font_SetArray(FontSpecifier *FontList, int NumColors = 0);

#endif
