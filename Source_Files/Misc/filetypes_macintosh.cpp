/*

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
*/

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
	 'pref'		// Preferences typecode
};


// Initializer: loads from resource fork
void initialize_typecodes()
{
	Handle FTypHdl = GetResource('FTyp',128);
	if (FTypHdl == NULL) return;
	int FTHSize = GetHandleSize(FTypHdl);
	if (FTHSize > sizeof(typecodes)) return;
	HLock(FTypHdl);
	memcpy(typecodes,*FTypHdl,FTHSize);
	HUnlock(FTypHdl);
	ReleaseResource(FTypHdl);
}


// Accessor
OSType get_typecode(int which)
{
	if (which < 0) return '????';
	else if (which >= NUMBER_OF_TYPECODES) return '????';
	return typecodes[which];
}
