/*
 *  filetypes_sdl.cpp - File type handling, SDL implementation
 *
 *  Written in 2000 by Christian Bauer
 */

#include "cseries.h"
#include "resource_manager.h"
#include "FileHandler.h"
#include "byte_swapping.h"
#include "tags.h"


// Global typecode list
static uint32 typecodes[NUMBER_OF_TYPECODES] = {
	0x32362e41,	// Creator code: originally '52.4'; changed to special "Aleph One" value
	0x73636532,	// Map/scenario typecode: unchanged for compatibility reasons
	0x736761b0,	// Savegame typecode: originally 'sga2'; changed for Minf compatibility
	0x66696cb0,	// Film typecode: originally 'fil2'; changed for Minf compatibility
	0x706879b0,	// Physics typecode: originally 'phy2'; changed for Minf compatibility
	0x736870b0,	// Shapes typecode: originally 'shp2'; changed for Minf compatibility
	0x736e64b0,	// Sounds typecode: originally 'snd2'; changed for Minf compatibility
	0x706174b0,	// Patch typecode: originally 'pat2'; changed for Minf compatibility
	0x696d6732,	// Images typecode
	0x70726566	// Preferences typecode
};


// Initializer: loads from resource fork
void initialize_typecodes(void)
{
	LoadedResource rsrc;
	if (!get_resource('FTyp', 128, rsrc))
		return;
	if (rsrc.GetLength() > sizeof(typecodes))
		return;
	memcpy(typecodes, rsrc.GetPointer(), rsrc.GetLength());
	byte_swap_memory(typecodes, _4byte, sizeof(typecodes) / sizeof(uint32));
}


// Accessor
uint32 get_typecode(int which)
{
	return typecodes[which];
}
