#ifndef XML_ELEMENTPARSER
#define XML_ELEMENTPARSER
/*
	XML-Element Parsing Objects
	by Loren Petrich,
	April 15, 2000
	
	These are subclassed for each kind of element to be parsed.
	
	May 3, 2000
	Added a change to adding a child so as not to add one with the same name twice.
*/


#include <stdio.h>
#include "GrowableList.h"


extern bool XML_GetBooleanValue(const char *String, bool &Value);


class XML_ElementParser
{
	// Its name, of course
	char *Name;
		
	// List of child elements
	GrowableList<XML_ElementParser *> Children;

protected:
	// Designed for easy reading of numerical values:
	// args are the string to read, the format string (for <>scanf),
	// and the numerical-value destination
	template<class T> bool ReadNumericalValue(const char *String, const char *Format,
		T& Value)
	{
		if (sscanf(String,Format,&Value) != 1)
		{
			BadNumericalValue();
			return false;
		}
		return true;
	}
	
	// For reading of bounded numerical values, such as index values
	template<class T> bool ReadBoundedNumericalValue(const char *String, const char *Format,
		T& Value, const T& MinVal, const T& MaxVal)
	{
		if (ReadNumericalValue(String,Format,Value))
		{
			if (Value >= MinVal && Value <= MaxVal)
				return true;
			
			OutOfRange();
			return false;
		}
		return false;
	}
	
	// For reading of Boolean values (true or false)
	template<class T> bool ReadBooleanValue(const char *String, T& Value)
	{
		bool BValue;
		if (!XML_GetBooleanValue(String,BValue))
		{
			BadBooleanValue();
			return false;
		}
		Value = T(BValue);
		return true;
	}
	
public:

	// The element's name:
	char *GetName() {return Name;}
	
	// Does a name match it?
	bool NameMatch(const char *_Name);
	
	// If the next routines routines return "false",
	// then set ErrorString to a character string
	// describing the error.
	
	// Start and end processing of an element
	virtual bool Start() {return true;}
	virtual bool End() {return true;}
	
	// Process an attribute tag-value set;
	// need individual parsing and attribute-finish parsing
	virtual bool HandleAttribute(const char *Tag, const char *Value) {return true;}
	virtual bool AttributesDone() {return true;}
	
	// Handle embedded string data
	virtual bool HandleString(const char *String, int Length) {return true;}
	
	// Error-message string
	char *ErrorString;
	
	// Common kinds of errors:
	void UnrecognizedTag();
	void AttribsMissing();
	void BadNumericalValue();
	void OutOfRange();
	void BadBooleanValue();
	
	// When finished parsing with this object, go to parent element
	XML_ElementParser *Parent;
	
	// Constructor and destructor;
	// needs the element's name.
	XML_ElementParser(const char *_Name);
	~XML_ElementParser();
	
	// Add a child element
	void AddChild(XML_ElementParser *Child);
	
	// Find a child element with a matching name;
	// if one is not found, then this method returns NULL
	XML_ElementParser *FindChild(const char *_Name);
};


#endif
