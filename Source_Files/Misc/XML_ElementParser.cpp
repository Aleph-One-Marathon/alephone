/*
	XML-Element Parsing Objects
	by Loren Petrich,
	April 15, 2000
	
	This is the base-class implementation of these objects
*/

#include <string.h>
#include <ctype.h>
#include "XML_ElementParser.h"


bool XML_GetBooleanValue(const char *String, bool &Value)
{
	if (strcmp(String,"1") == 0)
	{
		Value = true;
		return true;
	}
	else if (strcmp(String,"t") == 0)
	{
		Value = true;
		return true;
	}
	else if (strcmp(String,"true") == 0)
	{
		Value = true;
		return true;
	}
	else if (strcmp(String,"0") == 0)
	{
		Value = false;
		return true;
	}
	else if (strcmp(String,"f") == 0)
	{
		Value = false;
		return true;
	}
	else if (strcmp(String,"false") == 0)
	{
		Value = false;
		return true;
	}
	return false;
}


// Error strings; these are globals so that they can be referenced without confusion.
static char InitialErrorString[] = "initial error string";
static char UnrecognizedTagString[] = "unrecognized tag";
static char AttribsMissingString[] = "attributes missing";
static char BadNumericalValueString[] = "bad numerical value";
static char OutOfRangeString[] = "out of range";
static char BadBooleanValueString[] = "bad boolean value";


bool XML_ElementParser::NameMatch(const char *_Name)
{
	return (strcmp(Name,_Name) == 0);
}


XML_ElementParser::XML_ElementParser(const char *_Name)
{
	Name = new char[strlen(_Name)+1];
	strcpy(Name,_Name);
	
	// Set it to something reasonable
	ErrorString = InitialErrorString;
}


XML_ElementParser::~XML_ElementParser()
{
	delete []Name;
}

	
// Add a child element; be sure not to add one with the same name twice
void XML_ElementParser::AddChild(XML_ElementParser *Child)
{
	// Check to see if the child has already been added
	char *ChildName = Child->GetName();
	for (int k=0; k<Children.GetLength(); k++)
		if (Children[k]->NameMatch(ChildName)) return;
	
	// Go!
	Children.Add(Child);
}


XML_ElementParser *XML_ElementParser::FindChild(const char *_Name)
{
	XML_ElementParser *FoundChild = NULL;
	
	for (int k=0; k<Children.GetLength(); k++)
	{
		XML_ElementParser *TestChild = Children[k];
		if (TestChild->NameMatch(_Name))
		{
			FoundChild = TestChild;
			break;
		}
	}
	
	return FoundChild;
}


// Error-message emitters; these use the global definitions given above
void XML_ElementParser::UnrecognizedTag() {ErrorString = UnrecognizedTagString;}
void XML_ElementParser::AttribsMissing() {ErrorString = AttribsMissingString;}
void XML_ElementParser::BadNumericalValue() {ErrorString = BadNumericalValueString;}
void XML_ElementParser::OutOfRange() {ErrorString = OutOfRangeString;}
void XML_ElementParser::BadBooleanValue() {ErrorString = BadBooleanValueString;}
