
/*
	Font handler
	by Loren Petrich,
	December 17, 2000
	
	This is for specifying and working with text fonts
*/

#include <string.h>
#include "FontHandler.h"


// Font-specifier equality and assignment:

bool FontSpecifier::operator==(FontSpecifier& F)
{
	if (strncmp(NameSet,F.NameSet,NameSetLen) != 0) return false;
	if (Size != F.Size) return false;
	if (Style != F.Style) return false;
	return true;
}

FontSpecifier& FontSpecifier::operator=(FontSpecifier& F)
{
	memcpy(NameSet,F.NameSet,NameSetLen);
	Size = F.Size;
	Style = F.Style;
	return *this;
}

// Initializer: call before using because of difficulties in setting up a proper constructor:

void FontSpecifier::Init()
{
	Update();
}


// MacOS-specific:
// Cribbed from _get_font_line_spacing() and similar functions in screen_drawing.cpp

void FontSpecifier::Update()
{
	// csfonts -- push old font
	TextSpec OldFont;
	GetFont(&OldFont);
	
	// Parse the font spec to find the font ID;
	// if it is not set, then it is assumed to be the system font
	ID = 0;
	
	Str255 Name;
	
	char *NamePtr = FindNextName(NameSet);
	
	while(NamePtr)
	{
		char *NameEndPtr = FindNameEnd(NamePtr);
		
		// Make a Pascal string out of the name
		int NameLen = MIN(NameEndPtr - NamePtr, 255);
		Name[0] = NameLen;
		memcpy(Name+1,NamePtr,NameLen);
		Name[NameLen+1] = 0;
		// dprintf("Name, Len: <%s>, %d",Name+1,NameLen);
		
		// MacOS ID->name translation
		GetFNum(Name,&ID);
		// dprintf("ID = %d",ID);
		if (ID != 0) break;
		
		NamePtr = FindNextName(NameEndPtr);
	}
	
	// Get the other font features
	Use();
	FontInfo Info;
	GetFontInfo(&Info);
	
	Height = Info.ascent+Info.leading;
	LineSpacing = Info.ascent+Info.descent+Info.leading;
	
	// pop old font
	SetFont(&OldFont);
}


void FontSpecifier::Use()
{
	// MacOS-specific:
	TextFont(ID);
	TextFace(Style);
	TextSize(Size);
}

	
// Given a pointer to somewhere in a name set, returns the pointer
// to the start of the next name, or NULL if it did not find any
char *FontSpecifier::FindNextName(char *NamePtr)
{
	char *NextNamePtr = NamePtr;
	char c;
	
	// Continue as long as one's in the string
	while((c = *NextNamePtr) != 0)
	{
		// dprintf("Nxt %c",c);
		// Check for whitespace and commas and semicolons
		if (c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == ',' || c == ';')
		{
			NextNamePtr++;
			continue;
		}
		return NextNamePtr;
	}
	return NULL;
}

// Given a pointer to the beginning of a name, finds the pointer to the character
// just after the end of that name
char *FontSpecifier::FindNameEnd(char *NamePtr)
{
	char *NameEndPtr = NamePtr;
	char c;
	
	// Continue as long as one's in the string
	while((c = *NameEndPtr) != 0)
	{
		// dprintf("End %c",c);
		// Delimiter: comma or semicolon
		if (c == ',' || c == ';') break;
		NameEndPtr++;
	}
	return NameEndPtr;
}


// Font-parser object:
class XML_FontParser: public XML_ElementParser
{
	FontSpecifier TempFont;
	int Index;
	bool IsPresent[4];

public:
	FontSpecifier *FontList;
	int NumFonts;
	
	bool Start();
	bool HandleAttribute(const char *Tag, const char *Value);
	bool AttributesDone();
	
	XML_FontParser(): XML_ElementParser("font"), FontList(NULL) {}
};

bool XML_FontParser::Start()
{
	for (int k=0; k<4; k++)
		IsPresent[k] = false;
	return true;
}

bool XML_FontParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (NumFonts > 0)
	{
	if (strcmp(Tag,"index") == 0)
	{
		if (ReadBoundedNumericalValue(Value,"%d",Index,0,NumFonts-1))
		{
			IsPresent[3] = true;
			return true;
		}
		else return false;
	}
	}
	if (strcmp(Tag,"name") == 0)
	{
		IsPresent[0] = true;
		strncpy(TempFont.NameSet,Value,FontSpecifier::NameSetLen);
		TempFont.NameSet[FontSpecifier::NameSetLen-1] = 0;	// For making the set always a C string
		return true;
	}
	else if (strcmp(Tag,"size") == 0)
	{
		if (ReadNumericalValue(Value,"%hd",TempFont.Size))
		{
			IsPresent[1] = true;
			return true;
		}
		else return false;
	}
	else if (strcmp(Tag,"style") == 0)
	{
		float CVal;
		if (ReadNumericalValue(Value,"%hd",TempFont.Style))
		{
			IsPresent[2] = true;
			return true;
		}
		else return false;
	}
	UnrecognizedTag();
	return false;
}

bool XML_FontParser::AttributesDone()
{
	// Verify...
	bool AllPresent = true;
	if (NumFonts <= 0)
	{
		IsPresent[3] = true;	// Convenient fakery: no index -- always present
		Index = 0;
	}	
	if (!IsPresent[3])
	{
		AttribsMissing();
		return false;
	}
	
	// Put into place and update if necessary
	assert(FontList);
	bool DoUpdate = false;
	if (IsPresent[0])
	{
		strncpy(FontList[Index].NameSet,TempFont.NameSet,FontSpecifier::NameSetLen);
		DoUpdate = true;
	}
	if (IsPresent[1])
	{
		FontList[Index].Size = TempFont.Size;
		DoUpdate = true;
	}
	if (IsPresent[2])
	{
		FontList[Index].Style = TempFont.Style;
		DoUpdate = true;
	}
	if (DoUpdate) FontList[Index].Update();
	return true;
}

static XML_FontParser FontParser;



// Returns a parser for the colors;
// several elements may have colors, so this ought to be callable several times.
XML_ElementParser *Font_GetParser() {return &FontParser;}

// This sets the array of colors to be read into.
// Its args are the pointer to that array and the number of colors in it.
// If that number is <= 0, then the color value is assumed to be non-indexed,
// and no "index" attribute will be searched for.
void Font_SetArray(FontSpecifier *FontList, int NumFonts)
{FontParser.FontList = FontList; FontParser.NumFonts = NumFonts;}
