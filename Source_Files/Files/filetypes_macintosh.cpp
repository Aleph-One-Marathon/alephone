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

April 16, 2003 (Woody Zenfell):
        Can now map multiple Mac OS file types to a single A1 typecode
        (will let enviroprefs find M2-typed stuff)
*/
#if defined(mac) || ( defined(SDL) && defined(SDL_RFORK_HACK) )
#if defined(EXPLICIT_CARBON_HEADER)
	#include <Carbon/Carbon.h>
#endif
#include <string.h>
#include "csalerts.h"
#include "tags.h"
#include <map>

using std::map;
using std::vector;

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
	 'mus2',	// Music typecode
	 '????',	// Theme pseudo-typecode
	 '????'		// NetScript pseudo-typecode
};

struct file_type_to_a1_typecode_rec
{
        OSType	file_type;
        Typecode	typecode;
};

static file_type_to_a1_typecode_rec additional_typecodes[] =
{
        // Additional mappings to let M2 files be found in Environment prefs etc.
        { 'sga2', _typecode_savegame },
        { 'fil2', _typecode_film },
        { 'phy2', _typecode_physics },
        { 'shp2', _typecode_shapes },
        { 'snd2', _typecode_sounds },
        { 'pat2', _typecode_patch },

        // Additional mappings in case A1 wants its own filetypes at some point
        { 'sceA', _typecode_scenario },
        { 'sgaA', _typecode_savegame },
        { 'filA', _typecode_film },
        { 'phyA', _typecode_physics },
        { 'shpA', _typecode_shapes },
        { 'sndA', _typecode_sounds },
        { 'patA', _typecode_patch }
};

static const int NUMBER_OF_ADDITIONAL_TYPECODES = sizeof(additional_typecodes) / sizeof(additional_typecodes[0]);

typedef map<OSType, Typecode> file_type_to_a1_typecode_t;
static file_type_to_a1_typecode_t file_type_to_a1_typecode;


// Initializer: loads from resource fork
void initialize_typecodes()
{
#ifndef SDL_RFORK_HACK
//AS: Mac OS X SDL doesn't get custom typecodes for now. Nobody ever used them, anyway. Maybe we can have magic number checking or something?
	Handle FTypHdl = GetResource('FTyp',128);
	if (FTypHdl != NULL)
	{
		size_t FTHSize = GetHandleSize(FTypHdl);
		if (FTHSize <= sizeof(typecodes))
		{
			HLock(FTypHdl);
			memcpy(typecodes,*FTypHdl,FTHSize);
			HUnlock(FTypHdl);
		}
		ReleaseResource(FTypHdl);
	}
#endif
        file_type_to_a1_typecode.clear();
        
        for(int i = 0; i < NUMBER_OF_TYPECODES; i++)
        {
                file_type_to_a1_typecode[typecodes[i]] = static_cast<Typecode>(i);
        }

        for(int i = 0; i < NUMBER_OF_ADDITIONAL_TYPECODES; i++)
        {
                file_type_to_a1_typecode[additional_typecodes[i].file_type] = additional_typecodes[i].typecode;
        }
}


// Accessors
OSType get_typecode(Typecode which)
{
	if (which < 0) return '????';
	else if (which >= NUMBER_OF_TYPECODES) return '????';
	return typecodes[which];
}

void set_typecode(Typecode which, OSType _type)
{
	if (which < 0) return;
	else if (which > NUMBER_OF_TYPECODES) return;
	else typecodes[which] = _type;
}

Typecode
get_typecode_for_file_type(OSType inType)
{
        assert(!file_type_to_a1_typecode.empty());

        file_type_to_a1_typecode_t::iterator entry = file_type_to_a1_typecode.find(inType);
        if(entry == file_type_to_a1_typecode.end())
                return _typecode_unknown;
        else
                return entry->second;
}

const vector<OSType>
get_all_file_types_for_typecode (Typecode which)
{
	vector<OSType> result;

	if (which < NUMBER_OF_TYPECODES && which > 0)
		if (typecodes [which] != '????')
			result.push_back (typecodes [which]);
	
	for (int i = 0; i < NUMBER_OF_ADDITIONAL_TYPECODES; ++i)
		if (additional_typecodes [i].typecode == which)
			result.push_back (additional_typecodes [i].file_type);
	
	return result;
}
#endif
