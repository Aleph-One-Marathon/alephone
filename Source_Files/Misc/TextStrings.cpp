/*
	Text-String Collection
	by Loren Petrich,
	April 20, 2000
	
	This is the implementation of my replacement for MacOS STR# resources
*/


#include <string.h>
#include "cseries.h"
#include "TextStrings.h"


// Private objects: the string collections, which form a linked list.
class StringSet
{
	short ID;
	unsigned short NumStrings;
	// Pointer to string pointers:
	unsigned char **Strings;

public:
	// What's the ID
	short GetID() {return ID;}
	
	// How many strings (contiguous from index zero)
	unsigned short CountStrings();
	
	// Create a stringset with some ID
	StringSet(short _ID);
	~StringSet();

	// Assumes a MacOS Pascal string; the resulting string will have
	// a null byte at the end.
	void Add(short Index, unsigned char *String);
	
	// Get a string; return NULL if not found
	unsigned char *GetString(short Index);
	
	// Delete a string
	void Delete(short Index);
	
	// For making a linked list (I'll let Rhys Hill find more efficient data structures)
	StringSet *Next;
};


// How many strings (contiguous from index zero)
unsigned short StringSet::CountStrings()
{
	unsigned short StringCount = 0;
	
	for (int k=0; k<NumStrings; k++)
	{
		if (Strings[k])
			StringCount++;
		else
			break;
	}
	
	return StringCount;
}


StringSet::StringSet(short _ID)
{
	// Of course
	ID = _ID;
	
	NumStrings = 16;	// Reasonable starting number; what Sun uses in Java
	Strings = new unsigned char *[NumStrings];
	
	// Set all the string pointers to NULL
	objlist_clear(Strings,NumStrings);
	
	// Last, but not least:
	Next = NULL;
}

StringSet::~StringSet()
{
	for (int k=0; k<NumStrings; k++)
	{
		unsigned char *StringPtr = Strings[k];
		if (StringPtr) delete []StringPtr;
	}
	delete []Strings;
}

// Assumes a MacOS Pascal string; the resulting string will have
// a null byte at the end.
void StringSet::Add(short Index, unsigned char *String)
{
	if (Index < 0) return;
	
	// Replace string list with longer one if necessary
	unsigned short NewNumStrings = NumStrings;
	while (Index >= NewNumStrings) {NewNumStrings <<= 1;}
	
	if (NewNumStrings > NumStrings)
	{
		unsigned char **NewStrings = new unsigned char *[NewNumStrings];
		objlist_clear(NewStrings+NumStrings,(NewNumStrings-NumStrings));
		objlist_copy(NewStrings,Strings,NumStrings);
		delete []Strings;
		Strings = NewStrings;
		NumStrings = NewNumStrings;
	}
	
	// Delete the old string if necessary
	if (Strings[Index]) delete []Strings[Index];
	
	unsigned short Length = String[0];
	unsigned char *_String = new unsigned char[Length+2];
	memcpy(_String,String,Length+2);
	_String[Length+1] = 0;	//  for making an in-place C string
	
	Strings[Index] = _String;
}

// Get a string; return NULL if not found
unsigned char *StringSet::GetString(short Index)
{
	if (Index < 0 || Index >= NumStrings) return NULL;
	
	return Strings[Index];
}

// Delete a string
void StringSet::Delete(short Index)
{
	if (Index < 0 || Index >= NumStrings) return;
	
	unsigned char *StringPtr = Strings[Index];
	if (StringPtr) {delete []StringPtr; Strings[Index] = 0;}
}

static StringSet *StringSetRoot = NULL;

static StringSet *FindStringSet(short ID)
{
	StringSet *PrevStringSet = NULL, *CurrStringSet = StringSetRoot;
	while(CurrStringSet)
	{
		PrevStringSet = CurrStringSet;
		if (CurrStringSet->GetID() == ID) break;
		CurrStringSet = CurrStringSet->Next;
	}
	
	// Add to the end if not found
	if (!CurrStringSet)
	{
		StringSet *NewStringSet = new StringSet(ID);
		assert(NewStringSet);
		if (PrevStringSet)
			PrevStringSet->Next = NewStringSet;
		else
			StringSetRoot = NewStringSet;
		CurrStringSet = NewStringSet;
	}
	
	return CurrStringSet;
}

// Error strings
static const char IndexNotFound[] = "\"index\" attribute not found";


// Parser of a set of strings

class XML_StringSetParser: public XML_ElementParser
{
public:
	// Pointer to current stringset
	StringSet *CurrStringSet;
		
	// Callbacks
	bool Start() {CurrStringSet = NULL; return true;}
	bool HandleAttribute(const char *Tag, const char *Value);
	bool AttributesDone();
	
	XML_StringSetParser(): XML_ElementParser("stringset") {}
};

bool XML_StringSetParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (StringsEqual(Tag,"index"))
	{
		short ID;
		if (ReadInt16Value(Value,ID))
		{
			CurrStringSet = FindStringSet(ID);
			return true;
		}
		else return false;
	}
	UnrecognizedTag();
	return false;
}

bool XML_StringSetParser::AttributesDone()
{
	if (!CurrStringSet)
	{
		ErrorString = IndexNotFound;
		return false;
	}
	return true;
}

static XML_StringSetParser StringSetParser;


// Parser of a single string


class XML_StringParser: public XML_ElementParser
{
	// Check presence of index; having a DTD-using XML parser would
	// make this check unnecessary
	bool IndexPresent;
	
	// Was the string loaded? If not, then load a blank string at the end
	bool StringLoaded;

public:
	// Callbacks
	bool Start();
	bool End();
	bool HandleAttribute(const char *Tag, const char *Value);
	bool AttributesDone();
	bool HandleString(const char *String, int Length);
	
	// The string's index value
	short Index;

	XML_StringParser(): XML_ElementParser("string") {}
};


bool XML_StringParser::Start() {

	IndexPresent = false;
	StringLoaded = false;
	return true;
}

bool XML_StringParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (StringsEqual(Tag,"index"))
	{
		if (ReadInt16Value(Value,Index))
		{
			IndexPresent = true;
			return true;
		}
		else return false;
	}
	UnrecognizedTag();
	return false;
}

bool XML_StringParser::HandleString(const char *String, int Length)
{
	// Copy into Pascal string
	Str255 StringBuffer;
	// OK since this is called by value
	if (Length > 255) Length = 255;
	
	StringBuffer[0] = Length;
	memcpy(StringBuffer+1,String,Length);
	
	// Load!
	assert(StringSetParser.CurrStringSet);
	StringSetParser.CurrStringSet->Add(Index,StringBuffer);
	
	StringLoaded = true;
	return true;
}

bool XML_StringParser::AttributesDone()
{
	if (!IndexPresent)
	{
		ErrorString = IndexNotFound;
		return false;
	}
	return true;
}
	
bool XML_StringParser::End()
{
	if (!StringLoaded)
	{
		// Load an empty string
		unsigned char StringBuffer[1];
		StringBuffer[0] = 0;
		assert(StringSetParser.CurrStringSet);
		StringSetParser.CurrStringSet->Add(Index,StringBuffer);
	}
	
	return true;
}

static XML_StringParser StringParser;


// Public routines:


// Set up a string in the repository; a repeated call will replace an old string
void TS_PutString(short ID, short Index, unsigned char *String)
{
	// Search for string set:
	StringSet *CurrStringSet = FindStringSet(ID);
	CurrStringSet->Add(Index,String);
}


// Returns a pointer to a string; if the ID and the index do not point to a valid string,
// this function will then return NULL
unsigned char *TS_GetString(short ID, short Index)
{
	// Search for string set:
	StringSet *PrevStringSet = NULL, *CurrStringSet = StringSetRoot;
	
	while(CurrStringSet)
	{
		if (CurrStringSet->GetID() == ID) break;
		PrevStringSet = CurrStringSet;
		CurrStringSet = CurrStringSet->Next;
	}
	
	if (!CurrStringSet) return NULL;
	
	return CurrStringSet->GetString(Index);
}


// Here is that string in C form
char *TS_GetCString(short ID, short Index)
{
	unsigned char *String = TS_GetString(ID,Index);
	if (!String) return NULL;
	
	// Move away from the length byte to the first content byte
	return (char *)(String+1);
}


// Checks on the presence of a string set
bool TS_IsPresent(short ID)
{
	// Search for string set:
	StringSet *PrevStringSet = NULL, *CurrStringSet = StringSetRoot;
	
	while(CurrStringSet)
	{
		if (CurrStringSet->GetID() == ID) return true;
		PrevStringSet = CurrStringSet;
		CurrStringSet = CurrStringSet->Next;
	}
	
	return false;
}


// Count the strings (contiguous from index zero)
unsigned short TS_CountStrings(short ID)
{
	// Search for string set:
	StringSet *PrevStringSet = NULL, *CurrStringSet = StringSetRoot;
	
	while(CurrStringSet)
	{
		if (CurrStringSet->GetID() == ID) break;
		PrevStringSet = CurrStringSet;
		CurrStringSet = CurrStringSet->Next;
	}
	
	if (!CurrStringSet) return 0;
	
	return CurrStringSet->CountStrings();
}


// Deletes a string, should one ever want to do that
void TS_DeleteString(short ID, short Index)
{
	// Search for string set:
	StringSet *PrevStringSet = NULL, *CurrStringSet = StringSetRoot;
	
	while(CurrStringSet)
	{
		if (CurrStringSet->GetID() == ID) break;
		PrevStringSet = CurrStringSet;
		CurrStringSet = CurrStringSet->Next;
	}

	if (!CurrStringSet) return;
	
	CurrStringSet->Delete(Index);
}


// Deletes the stringset with some ID
void TS_DeleteStringSet(short ID)
{
	// Search for string set:
	StringSet *PrevStringSet = NULL, *CurrStringSet = StringSetRoot;
	
	while(CurrStringSet)
	{
		if (CurrStringSet->GetID() == ID) break;
		PrevStringSet = CurrStringSet;
		CurrStringSet = CurrStringSet->Next;
	}

	if (!CurrStringSet) return;
	
	// Get the next string set:
	StringSet *NextStringSet = CurrStringSet->Next;
	
	// Clip out that string set
	if (PrevStringSet)
		PrevStringSet->Next = NextStringSet;
	else
		StringSetRoot = NextStringSet;
	
	delete CurrStringSet;
}


// Deletes all of the stringsets
void TS_DeleteAllStrings()
{
	StringSet *CurrStringSet = StringSetRoot;
	
	while(CurrStringSet)
	{
		StringSet *NextStringSet = CurrStringSet->Next;
		delete CurrStringSet;
		CurrStringSet = NextStringSet;
	}
	
	StringSetRoot = NULL;
}


// Set up a text-string XML parser and return a pointer to it
// Don't try to delete it when one is finished with it
XML_ElementParser *TS_GetParser()
{
	StringSetParser.AddChild(&StringParser);
	return &StringSetParser;
}

