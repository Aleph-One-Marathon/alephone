/*

	images.c
	Thursday, July 20, 1995 3:29:30 PM- rdm created.

Feb. 4, 2000 (Loren Petrich):
	Changed halt() to assert(false) for better debugging

Feb. 5, 2000 (Loren Petrich):
	Better handling of case of no scenario-file resource fork

Aug 21, 2000 (Loren Petrich):
	Added object-oriented file handling
	
	LoadedResource handles are assumed to always be locked,
	and HLock() and HUnlock() have been suppressed for that reason.
 */

#include "cseries.h"
#include "FileHandler.h"

#include <stdlib.h>

#include "interface.h"
#include "shell.h"
#include "images.h"
#include "screen.h"


// Constants
enum {
	_images_file_delta16= 1000,
	_images_file_delta32= 2000,
	_scenario_file_delta16= 10000,
	_scenario_file_delta32= 20000
};

// Global variables
static OpenedResourceFile ImagesResources;
static OpenedResourceFile ScenarioResources;

// Prototypes
static void shutdown_images_handler(void);
static int determine_pict_resource_id(OpenedResourceFile& OFile, uint32 pict_resource_type, int base_id, int delta16, int delta32);
static void draw_picture(LoadedResource &PictRsrc);


// Include platform-specific file
#if defined(mac)
#include "images_macintosh.cpp"
#elif defined(SDL)
#include "images_sdl.cpp"
#endif


/*
 *  Initialize image manager, open Images file
 */

void initialize_images_manager(void)
{
	FileSpecifier file;
#ifdef mac
	file.SetToApp();
	file.SetName(getcstr(temporary, strFILENAMES, filenameIMAGES),_typecode_images);
	if (!file.Exists())
		alert_user(fatalError, strERRORS, badExtraFileLocations, fnfErr);
	if (!file.Open(ImagesResources))
		alert_user(fatalError, strERRORS, badExtraFileLocations, -1);
#else
	file = getcstr(temporary, strFILENAMES, filenameIMAGES);
	if (!file.OpenRelative(ImagesResources))
		alert_user(fatalError, strERRORS, badExtraFileLocations, -1);
#endif
	atexit(shutdown_images_handler);
}


/*
 *  Shutdown image manager
 */

static void shutdown_images_handler(void)
{
	ScenarioResources.Close();
	ImagesResources.Close();
}


/*
 *  Set map file to load images from
 */

void set_scenario_images_file(FileSpecifier &file)
{
	file.Open(ScenarioResources);
}


/*
 *  Get/draw image from Images file
 */

bool get_picture_resource_from_images(int base_resource, LoadedResource &PictRsrc)
{
	assert(ImagesResources.IsOpen());

	int RsrcID = determine_pict_resource_id(
		ImagesResources,
		FOUR_CHARS_TO_INT('P', 'I', 'C', 'T'), base_resource,
		_images_file_delta16, _images_file_delta32);
	return ImagesResources.Get('P', 'I', 'C', 'T', RsrcID, PictRsrc);
}

bool images_picture_exists(int base_resource)
{
	assert(ImagesResources.IsOpen());

	int RsrcID = determine_pict_resource_id(
		ImagesResources,
		FOUR_CHARS_TO_INT('P', 'I', 'C', 'T'), base_resource,
		_images_file_delta16, _images_file_delta32);
	return ImagesResources.Check('P', 'I', 'C', 'T', RsrcID);
}

void draw_full_screen_pict_resource_from_images(int pict_resource_number)
{
	LoadedResource PictRsrc;
	get_picture_resource_from_images(pict_resource_number, PictRsrc);
	draw_picture(PictRsrc);
}


/*
 *  Get/draw image from scenario
 */

bool get_picture_resource_from_scenario(int base_resource, LoadedResource &PictRsrc)
{
	if (!ScenarioResources.IsOpen())
		return false;

	int RsrcID = determine_pict_resource_id(
		ScenarioResources,
		FOUR_CHARS_TO_INT('P', 'I', 'C', 'T'), base_resource,
		_scenario_file_delta16, _scenario_file_delta32);
	bool success = ScenarioResources.Get('P', 'I', 'C', 'T', RsrcID, PictRsrc);
#ifdef mac
	if (success) {
		Handle PictHdl = PictRsrc.GetHandle();
		if (PictHdl) HNoPurge(PictHdl);
	}
#endif
	return success;
}

bool scenario_picture_exists(int base_resource)
{
	if (!ScenarioResources.IsOpen())
		return false;

	int RsrcID = determine_pict_resource_id(
		ScenarioResources,
		FOUR_CHARS_TO_INT('P', 'I', 'C', 'T'), base_resource,
		_scenario_file_delta16, _scenario_file_delta32);
	return ScenarioResources.Check('P', 'I', 'C', 'T', RsrcID);
}

void draw_full_screen_pict_resource_from_scenario(int pict_resource_number)
{
	LoadedResource PictRsrc;
	get_picture_resource_from_scenario(pict_resource_number, PictRsrc);
	draw_picture(PictRsrc);
}


/*
 *  Get sound resource from scenario
 */

bool get_sound_resource_from_scenario(int resource_number, LoadedResource &SoundRsrc)
{
	if (!ScenarioResources.IsOpen())
		return false;

	bool success = ScenarioResources.Get('s', 'n', 'd', ' ', resource_number, SoundRsrc);
#ifdef mac
	if (success) {
		Handle SndHdl = SoundRsrc.GetHandle();
		if (SndHdl) HNoPurge(SndHdl);
	}
#endif
	return success;
}


/*
 *  Calculate color table for image
 */

struct color_table *calculate_picture_clut(int CLUTSource, int pict_resource_number)
{
	struct color_table *picture_table = NULL;

	// Select the source
	OpenedResourceFile *OFilePtr = NULL;
	switch (CLUTSource) {
		case CLUTSource_Images:
			OFilePtr = &ImagesResources;
			break;
		
		case CLUTSource_Scenario:
			OFilePtr = &ScenarioResources;
			break;
	
		default:
			vassert(false, csprintf(temporary, "Invalid resource-file selector: %d", CLUTSource));
			break;
	}
	
	// Load CLUT resource
	LoadedResource CLUT_Rsrc;
	if (OFilePtr->Get('c', 'l', 'u', 't', pict_resource_number, CLUT_Rsrc)) {

#ifdef mac
		Handle resource = CLUT_Rsrc.GetHandle();
		HNoPurge(resource);
#endif

		// Allocate color table
		picture_table = new color_table;

		// Convert MacOS CLUT resource to color table
		if (interface_bit_depth == 8)
			build_color_table(picture_table, CLUT_Rsrc);
		else
			build_direct_color_table(picture_table, interface_bit_depth);
	}

	return picture_table;
}


/*
 *  Determine ID for picture resource
 */

static int determine_pict_resource_id(OpenedResourceFile &OFile, uint32 pict_resource_type, int base_id, int delta16, int delta32)
{
	int actual_id = base_id;
	bool done = false;
	int bit_depth = interface_bit_depth;

	while (!done) {
		int next_bit_depth;
	
		actual_id = base_id;
		switch(bit_depth) {
			case 8:	
				next_bit_depth = 0; 
				break;
				
			case 16: 
				next_bit_depth = 8;
				actual_id += delta16; 
				break;
				
			case 32: 
				next_bit_depth = 16;
				actual_id += delta32;	
				break;
				
			default: 
				assert(false);
				break;
		}
		
		if (OFile.Check(pict_resource_type, actual_id))
			done = true;

		if (!done) {
			if (next_bit_depth)
				bit_depth = next_bit_depth;
			else {
				// Didn't find it. Return the 8 bit version and bail..
				done = true;
			}
		}
	}
	return actual_id;
}
