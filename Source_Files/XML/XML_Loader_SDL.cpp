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

*/

/*
 *  XML_Loader_SDL.cpp - Parser for XML files, SDL implementation
 *
 *  Written in 2000 by Christian Bauer
 */
 
 /*
 Oct 24, 2001 (Loren Petrich):
 	Added printing out of current filename in error messages
 */

#include "cseries.h"
#include "XML_Loader_SDL.h"
#include "FileHandler.h"

#include <stdio.h>
#include <vector>
#include <algorithm>
#include <boost/algorithm/string/predicate.hpp>


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
	fprintf(stderr, "XML parsing error: %s at line %d in object %s\n", ErrorString, LineNumber, FileName);
}


/*
 *  Reports an interpretation error
 */

const int MaxErrorsToShow = 7;

void XML_Loader_SDL::ReportInterpretError(const char *ErrorString)
{
	if (GetNumInterpretErrors() < MaxErrorsToShow)
		fprintf(stderr, "%s\n", ErrorString);
}


/*
 *  Requests aborting of parsing (reasonable if there were lots of errors)
 */

bool XML_Loader_SDL::RequestAbort()
{
	return (GetNumInterpretErrors() >= MaxErrorsToShow);
}


/*
 *  Parse XML file
 */

bool XML_Loader_SDL::ParseFile(FileSpecifier &file_name)
{
	// Open file
	OpenedFile file;
	if (file_name.Open(file)) {

		// Get file size and allocate buffer
		file.GetLength(data_size);
		data = new char[data_size];
		
		// In case there were errors...
		file_name.GetName(FileName);

		// Read and parse file
		if (file.Read(data_size, data)) {
			if (!DoParse())
				fprintf(stderr, "There were parsing errors in configuration file %s\n", FileName);
		}

		// Delete buffer
		delete[] data;
		data = NULL;
		return true;
	}
	return false;
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
		if (i->name[i->name.length() - 1] == '~') 
			continue;
		// people stick Lua scripts in Scripts/
		if (boost::algorithm::ends_with(i->name, ".lua"))
			continue;

		// Construct full path name
		FileSpecifier file_name = dir + i->name;

		// Parse file
		ParseFile(file_name);
	}

	return true;
}
