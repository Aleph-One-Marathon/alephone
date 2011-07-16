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
 *  XML_Loader_SDL.h - Parser for XML files, SDL implementation
 *
 *  Written in 2000 by Christian Bauer
 */
 
 /*
 Oct 24, 2001 (Loren Petrich):
 	Added printing out of current filename in error messages
 */

#ifndef _XML_LOADER_SDL_
#define _XML_LOADER_SDL_

#include "XML_Configure.h"

class FileSpecifier;


class XML_Loader_SDL : public XML_Configure {
	
	// LP: for error-message convenience;
	// initialized to the null string
	char FileName[256];
	
public:
	XML_Loader_SDL() : data(NULL) {FileName[0] = 0;}
	~XML_Loader_SDL() {delete[] data; data = NULL;}

	bool ParseFile(FileSpecifier &file);
	bool ParseDirectory(FileSpecifier &dir);

protected:
	virtual bool GetData();
	virtual void ReportReadError();
	virtual void ReportParseError(const char *ErrorString, int LineNumber);
	virtual void ReportInterpretError(const char *ErrorString);
	virtual bool RequestAbort();

private:
	char *data;
	int32 data_size;
};

#endif
