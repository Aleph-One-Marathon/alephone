/*

	Copyright (C) 1991-2001 and beyond by Bungie Studios, Inc.
	and the "Aleph One" developers.
 
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	This license is contained in the file "COPYING",
	which is included with this source code; it is available online at
	http://www.gnu.org/licenses/gpl.html

Feb 6, 2000 (Loren Petrich):
	This file is for loading MacOS typecodes from the resource fork;
	defaults copied from tags.h


Feb 14, 2000 (Loren Petrich):
	Changed it to read 'FTyp' resource 128

Aug 21, 2000 (Loren Petrich):
	Added preferences typecode
	
Aug 21, 2000 (Loren Petrich):
	Added images typecode

Nov 29, 2000 (Loren Petrich):
	Made copying of FTyp resources more reasonable; partial ones are now copied correctly

Mar 14, 2001 (Loren Petrich):
	Added a music filetype

Jan 25, 2002 (Br'fin (Jeremy Parsons)):
	Added TARGET_API_MAC_CARBON for Carbon.h
*/
#if defined(mac) || ( defined(SDL) && defined(SDL_RFORK_HACK) )
#if defined(TARGET_API_MAC_CARBON)
	#include <Carbon/Carbon.h>
#endif
#include <string.h>
#include "tags.h"


// Global: typecode list

static OSType typecodes[NUMBER_OF_TYPECODES] = {
	 '26.A',	// Creator code: originally '52.4'; changed to special "Aleph One" value
	 'sce2',	// Map/scenario typecode: unchanged for compatibility reasons
	 'sga°',	// Savegame typecode: originally 'sga2'; changed for M° compatibility
	 'fil°',	// Film typecode: originally 'fil2'; changed for M° compatibility
	 'phy°',	// Physics typecode: originally 'phy2'; changed for M° compatibility
	 'shp°',	// Shapes typecode: originally 'shp2'; changed for M° compatibility
	 'snd°',	// Sounds typecode: originally 'snd2'; changed for M° compatibility
	 'pat°',	// Patch typecode: originally 'pat2'; changed for M° compatibility
	 'img2',	// Images typecode
	 'pref',	// Preferences typecode
	 'mus2'		// Music typecode
};


// Initializer: loads from resource fork
void initialize_typecodes()
{
#ifndef SDL_RFORK_HACK
//AS: Mac OS X SDL doesn't get custom typecodes for now. Nobody ever used them, anyway. Maybe we can have magic number checking or something?
	Handle FTypHdl = GetResource('FTyp',128);
	if (FTypHdl == NULL) return;
	int FTHSize = GetHandleSize(FTypHdl);
	if (FTHSize > sizeof(typecodes)) return;
	HLock(FTypHdl);
	memcpy(typecodes,*FTypHdl,FTHSize);
	HUnlock(FTypHdl);
	ReleaseResource(FTypHdl);
#endif
}


// Accessor
OSType get_typecode(int which)
{
	if (which < 0) return '????';
	else if (which >= NUMBER_OF_TYPECODES) return '????';
	return typecodes[which];
}
#endif