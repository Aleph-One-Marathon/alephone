/*
 *  XML_Loader_SDL.cpp - Parser for XML files, SDL implementation
 *
 *  Written in 2000 by Christian Bauer
 */

#include "cseries.h"
#include "XML_Loader_SDL.h"
#include "FileHandler.h"

#include <stdio.h>
#include <vector>
#include <algorithm>


/*
 *  Gets some XML data to parse
 */

bool XML_Loader_SDL::GetData()
{
	if (data == NULL)
		return false;

	Buffer = data;
	BufLen = data_size;
	LastOne = true;

	return true;
}


/*
 *  Reports a read error
 */

void XML_Loader_SDL::ReportReadError()
{
	fprintf(stderr, "Error in reading resources\n");
	exit(1);
}


/*
 *  Reports an XML parsing error
 */

void XML_Loader_SDL::ReportParseError(const char *ErrorString, int LineNumber)
{
	fprintf(stderr, "XML parsing error: %s at line %d\n", ErrorString, LineNumber);
	exit(1);
}


/*
 *  Reports an interpretation error
 */

const int MaxErrorsToShow = 7;

void XML_Loader_SDL::ReportInterpretError(const char *ErrorString)
{
	if (GetNumInterpretErrors() < MaxErrorsToShow)
		fprintf(stderr, ErrorString);
}


/*
 *  Requests aborting of parsing (reasonable if there were lots of errors)
 */

bool XML_Loader_SDL::RequestAbort()
{
	return (GetNumInterpretErrors() >= MaxErrorsToShow);
}


/*
 *  Parse all XML files in the specified directory
 */

bool XML_Loader_SDL::ParseDirectory(FileSpecifier &dir)
{
	// Get sorted list of files in directory
	vector<dir_entry> de;
	if (!dir.ReadDirectory(de))
		return false;
	sort(de.begin(), de.end());

	// Parse each file
	vector<dir_entry>::const_iterator i, end = de.end();
	for (i=de.begin(); i!=end; i++) {
		if (i->is_directory)
			continue;

		// Construct full path name
		data = new char[data_size = i->size];
		FileSpecifier file_name = dir;
		file_name.AddPart(i->name);

		// Open and parse file
		OpenedFile file;
		if (file_name.Open(file) && file.Read(data_size, data)) {
			if (!DoParse()) {
				fprintf(stderr, "There were configuration file parsing errors\n");
				exit(1);
			}
		}

		delete[] data;
		data = NULL;
	}

	return true;
}
