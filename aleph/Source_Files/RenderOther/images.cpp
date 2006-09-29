/*
	images.c

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

	Thursday, July 20, 1995 3:29:30 PM- rdm created.

Feb. 4, 2000 (Loren Petrich):
	Changed halt() to assert(false) for better debugging

Feb. 5, 2000 (Loren Petrich):
	Better handling of case of no scenario-file resource fork

Aug 21, 2000 (Loren Petrich):
	Added object-oriented file handling
	
	LoadedResource handles are assumed to always be locked,
	and HLock() and HUnlock() have been suppressed for that reason.

Jul 6, 2001 (Loren Petrich):
	Added Thomas Herzog's changes for loading Win32-version image chunks

Jan 25, 2002 (Br'fin (Jeremy Parsons)):
	Added TARGET_API_MAC_CARBON for Quicktime.h

Jul 31, 2002 (Loren Petrich)
	Added text-resource access in analogy with others' image- and sound-resource access;
	this is for supporting the M2-Win95 file format
 */

#include "cseries.h"
#include "FileHandler.h"

#include <stdlib.h>

#if defined(mac)
#if defined(EXPLICIT_CARBON_HEADER)
#  include <quicktime/Quicktime.h>
#else
#  include <Movies.h>
#endif
#endif

#include "interface.h"
#include "shell.h"
#include "images.h"
#include "screen.h"
#include "wad.h"
#include "screen_drawing.h"
#include "Logging.h"


// Constants
enum {
	_images_file_delta16= 1000,
	_images_file_delta32= 2000,
	_scenario_file_delta16= 10000,
	_scenario_file_delta32= 20000
};

// Structure for open image file
class image_file_t {
public:
	image_file_t() {}
	~image_file_t() {close_file();}

	bool open_file(FileSpecifier &file);
	void close_file(void);
	bool is_open(void);

	int determine_pict_resource_id(int base_id, int delta16, int delta32);

	bool has_pict(int id);
	bool has_clut(int id);

	bool get_pict(int id, LoadedResource &rsrc);
	bool get_clut(int id, LoadedResource &rsrc);
	bool get_snd(int id, LoadedResource &rsrc);
	bool get_text(int id, LoadedResource &rsrc);

private:
	bool has_rsrc(uint32 rsrc_type, uint32 wad_type, int id);
	bool get_rsrc(uint32 rsrc_type, uint32 wad_type, int id, LoadedResource &rsrc);

	bool make_rsrc_from_pict(void *data, size_t length, LoadedResource &rsrc, void *clut_data, size_t clut_length);
	bool make_rsrc_from_clut(void *data, size_t length, LoadedResource &rsrc);

	OpenedResourceFile rsrc_file;
	OpenedFile wad_file;
	wad_header wad_hdr;
};

#ifdef mac
// M2/Win picture structures (do not use in portable code)
typedef struct pict_head {
	screen_rectangle bounds;	/* screen_rectangle from screen_drawing.h */
	short depth;				/* 8 or 16 */
								/* pixel data follows here */
} pict_head;

typedef struct clut_record {
	short count;
	short unknown;
	short id;
	rgb_color colors[256];		/* rgb_color from cscluts.h */
} clut_record;
#endif

// Global variables
static image_file_t ImagesFile;
static image_file_t ScenarioFile;

// Prototypes
static void shutdown_images_handler(void);
static void draw_picture(LoadedResource &PictRsrc);


// Include platform-specific file
#if defined(mac)
#include "images_macintosh.h"
#elif defined(SDL)
#include "images_sdl.h"
#endif


/*
 *  Initialize image manager, open Images file
 */

void initialize_images_manager(void)
{
	FileSpecifier file;

  logContext("loading Images...");

	file.SetNameWithPath(getcstr(temporary, strFILENAMES, filenameIMAGES)); // _typecode_images
	
	if (!file.Exists())
		alert_user(fatalError, strERRORS, badExtraFileLocations, fnfErr);
	
	if (!ImagesFile.open_file(file))
		alert_user(fatalError, strERRORS, badExtraFileLocations, -1);

	atexit(shutdown_images_handler);
}


/*
 *  Shutdown image manager
 */

static void shutdown_images_handler(void)
{
	ScenarioFile.close_file();
	ImagesFile.close_file();
}


/*
 *  Set map file to load images from
 */

void set_scenario_images_file(FileSpecifier &file)
{
	ScenarioFile.open_file(file);
}


/*
 *  Open/close image file
 */

bool image_file_t::open_file(FileSpecifier &file)
{
	close_file();
	
	// Try to open as a resource file
	if (!file.Open(rsrc_file)) {
	
		// This failed, maybe it's a wad file (M2 Win95 style)
		if (!open_wad_file_for_reading(file, wad_file)
		 || !read_wad_header(wad_file, &wad_hdr)) {

			// This also failed, bail out
			wad_file.Close();
			return false;
		}
	} // Try to open wad file, too
	else if (!wad_file.IsOpen()) {
		if (open_wad_file_for_reading(file, wad_file)) {
			if (!read_wad_header(wad_file, &wad_hdr)) {
				
				wad_file.Close();
			}
		}
	}
	
	return true;
}

void image_file_t::close_file(void)
{
	rsrc_file.Close();
	wad_file.Close();
}

bool image_file_t::is_open(void)
{
	return rsrc_file.IsOpen() || wad_file.IsOpen();
}


/*
 *  Get resource from file
 */

bool image_file_t::has_rsrc(uint32 rsrc_type, uint32 wad_type, int id)
{
	// Check for resource in resource file
	if (rsrc_file.IsOpen())
	{
		if (rsrc_file.Check(rsrc_type, id))
			return true;
	}
	
	// Check for resource in wad file
	if (wad_file.IsOpen()) {
		wad_data *d = read_indexed_wad_from_file(wad_file, &wad_hdr, id, true);
		if (d) {
			bool success = false;
			size_t len;
			if (extract_type_from_wad(d, wad_type, &len))
				success = true;
			free_wad(d);
			return success;
		}
	}
	
	return false;
}

bool image_file_t::has_pict(int id)
{
	return has_rsrc(FOUR_CHARS_TO_INT('P','I','C','T'), FOUR_CHARS_TO_INT('p','i','c','t'), id);
}

bool image_file_t::has_clut(int id)
{
	return has_rsrc(FOUR_CHARS_TO_INT('c','l','u','t'), FOUR_CHARS_TO_INT('c','l','u','t'), id);
}

bool image_file_t::get_rsrc(uint32 rsrc_type, uint32 wad_type, int id, LoadedResource &rsrc)
{
	// Get resource from resource file
	if (rsrc_file.IsOpen())
	{
		if (rsrc_file.Get(rsrc_type, id, rsrc))
			return true;
	}
	
	// Get resource from wad file
	if (wad_file.IsOpen()) {
		wad_data *d = read_indexed_wad_from_file(wad_file, &wad_hdr, id, true);
		if (d) {
			bool success = false;
			size_t raw_length;
			void *raw = extract_type_from_wad(d, wad_type, &raw_length);
			if (raw)
			{
				if (rsrc_type == FOUR_CHARS_TO_INT('P','I','C','T'))
				{
					size_t clut_length;
					void *clut_data = extract_type_from_wad(d, FOUR_CHARS_TO_INT('c','l','u','t'), &clut_length);
					success = make_rsrc_from_pict(raw, raw_length, rsrc, clut_data, clut_length);
				}
				else if (rsrc_type == FOUR_CHARS_TO_INT('c','l','u','t'))
					success = make_rsrc_from_clut(raw, raw_length, rsrc);
				else if (rsrc_type == FOUR_CHARS_TO_INT('s','n','d',' '))
				{
					void *snd_data = malloc(raw_length);
					memcpy(snd_data, raw, raw_length);
					rsrc.SetData(snd_data, raw_length);
					success = true;
				}
				else if (rsrc_type == FOUR_CHARS_TO_INT('T','E','X','T'))
				{
					void *text_data = malloc(raw_length);
					memcpy(text_data, raw, raw_length);
					rsrc.SetData(text_data, raw_length);
					success = true;
				}
			}
			free_wad(d);
			return success;
		}
	}
	
	return false;
}

bool image_file_t::get_pict(int id, LoadedResource &rsrc)
{
	return get_rsrc(FOUR_CHARS_TO_INT('P','I','C','T'), FOUR_CHARS_TO_INT('p','i','c','t'), id, rsrc);
}

bool image_file_t::get_clut(int id, LoadedResource &rsrc)
{
	return get_rsrc(FOUR_CHARS_TO_INT('c','l','u','t'), FOUR_CHARS_TO_INT('c','l','u','t'), id, rsrc);
}

bool image_file_t::get_snd(int id, LoadedResource &rsrc)
{
	return get_rsrc(FOUR_CHARS_TO_INT('s','n','d',' '), FOUR_CHARS_TO_INT('s','n','d',' '), id, rsrc);
}

bool image_file_t::get_text(int id, LoadedResource &rsrc)
{
	return get_rsrc(FOUR_CHARS_TO_INT('T','E','X','T'), FOUR_CHARS_TO_INT('t','e','x','t'), id, rsrc);
}


/*
 *  Get/draw image from Images file
 */

bool get_picture_resource_from_images(int base_resource, LoadedResource &PictRsrc)
{
	assert(ImagesFile.is_open());

	int RsrcID = ImagesFile.determine_pict_resource_id(
		base_resource, _images_file_delta16, _images_file_delta32);
	return ImagesFile.get_pict(RsrcID, PictRsrc);
}

bool images_picture_exists(int base_resource)
{
	assert(ImagesFile.is_open());

	int RsrcID = ImagesFile.determine_pict_resource_id(
		base_resource, _images_file_delta16, _images_file_delta32);
	return ImagesFile.has_pict(RsrcID);
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
	if (!ScenarioFile.is_open())
		return false;

	int RsrcID = ScenarioFile.determine_pict_resource_id(
		base_resource, _scenario_file_delta16, _scenario_file_delta32);

	bool success = ScenarioFile.get_pict(RsrcID, PictRsrc);
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
	if (!ScenarioFile.is_open())
		return false;

	int RsrcID = ScenarioFile.determine_pict_resource_id(
		base_resource, _scenario_file_delta16, _scenario_file_delta32);
	return ScenarioFile.has_pict(RsrcID);
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
	if (!ScenarioFile.is_open())
		return false;

	bool success = ScenarioFile.get_snd(resource_number, SoundRsrc);
#ifdef mac
	if (success) {
		Handle SndHdl = SoundRsrc.GetHandle();
		if (SndHdl) HNoPurge(SndHdl);
	}
#endif
	return success;
}


// LP: do the same for text resources

bool get_text_resource_from_scenario(int resource_number, LoadedResource &TextRsrc)
{
	if (!ScenarioFile.is_open())
		return false;

	bool success = ScenarioFile.get_text(resource_number, TextRsrc);
#ifdef mac
	if (success) {
		Handle TxtHdl = TextRsrc.GetHandle();
		if (TxtHdl) HNoPurge(TxtHdl);
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
	image_file_t *OFilePtr = NULL;
	switch (CLUTSource) {
		case CLUTSource_Images:
			OFilePtr = &ImagesFile;
			break;
		
		case CLUTSource_Scenario:
			OFilePtr = &ScenarioFile;
			break;
	
		default:
			vassert(false, csprintf(temporary, "Invalid resource-file selector: %d", CLUTSource));
			break;
	}
	
	// Load CLUT resource
	LoadedResource CLUT_Rsrc;
	if (OFilePtr->get_clut(pict_resource_number, CLUT_Rsrc)) {

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

int image_file_t::determine_pict_resource_id(int base_id, int delta16, int delta32)
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
		
		if (has_pict(actual_id))
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


/*
 *  Convert picture and CLUT data from wad file to PICT resource
 */

#ifdef mac

// MacOS version
bool image_file_t::make_rsrc_from_pict(void *data, size_t length, LoadedResource &rsrc, void *clut_data, size_t clut_length)
{
	pict_head		*head;
	PicHandle		picture;
	Rect			bounds;
	short			i;
	GWorldPtr		gworld;
	PixMapHandle	pixmap;
	void			*pict_data;
	void			*clut;
	uint8			*p, *q;
	CTabHandle		clut_handle;
	CGrafPtr		saveport;
	
	if (length < 10)
		return false;
	
	head = (pict_head *)data;
	
	bounds.left		= head->bounds.left;
	bounds.top		= head->bounds.top;
	bounds.right	= head->bounds.right;
	bounds.bottom	= head->bounds.bottom;
	
	if (head->depth == 8)
	{
		
		clut = malloc(8 + 256 * 8);
		
		memset(clut, 0, 8 + 256 * 8);
		
		p = (uint8 *)clut_data;
		q = (uint8 *)clut;
		
		// 1. Header
		q[6] = p[0]; // color count
		q[7] = p[1];
		p += 6;
		q += 8;

		// 2. Color table
		for (i = 0; i < 256; i++)
		{
			q++;
			*q++ = i;		// value
			*q++ = *p++;	// red
			*q++ = *p++;
			*q++ = *p++;	// green
			*q++ = *p++;
			*q++ = *p++;	// blue
			*q++ = *p++;
		}
		
		clut_handle = (CTabHandle)NewHandleClear(8 + 256 * 8);
		
		PtrToHand(clut, (Handle *)&clut_handle, 8 + 256 * 8);
		
		QTNewGWorldFromPtr(&gworld, k8IndexedPixelFormat, &bounds, clut_handle, NULL, 0, (uint8 *)data + 10, head->bounds.right - head->bounds.left);
		
		free(clut);
		DisposeHandle((Handle)clut_handle);
	}
	else if (head->depth == 16)
	{
		QTNewGWorldFromPtr(&gworld, k16BE555PixelFormat, &bounds, NULL, NULL, 0, (uint8 *)data + 10, (head->bounds.right - head->bounds.left) * 2);
	}
	else if (head->depth == 24)
	{
		QTNewGWorldFromPtr(&gworld, k24RGBPixelFormat, &bounds, NULL, NULL, 0, (uint8 *)data + 10, (head->bounds.right - head->bounds.left) * 3);
	}
	else if (head->depth == 32)
	{
		QTNewGWorldFromPtr(&gworld, k32ARGBPixelFormat, &bounds, NULL, NULL, 0, (uint8 *)data + 10, (head->bounds.right - head->bounds.left) * 4);
	}
	else
	{
		return false;
	}
	
	PenNormal();
	
	GetGWorld(&saveport, NULL);
	SetGWorld(gworld, NULL);
	
	ForeColor(blackColor);
	BackColor(whiteColor);
	
	pixmap = GetGWorldPixMap(gworld);
	LockPixels(pixmap);
	
	picture = OpenPicture(&bounds);
	
	if (picture != NULL)
    {
        CopyBits((BitMap *)*pixmap, (BitMap *)*pixmap, &bounds, &bounds, srcCopy, NULL);
        ClosePicture();
    }
    
    if (picture != NULL)
    {
	    UnlockPixels(pixmap);
	    SetGWorld(saveport, NULL);
	    DisposeGWorld(gworld);
		
		HLock((Handle)picture);
		pict_data = malloc(GetHandleSize((Handle)picture));
		memcpy(pict_data, *picture, GetHandleSize((Handle)picture));
		
		rsrc.SetData(pict_data, GetHandleSize((Handle)picture));
		KillPicture(picture);
	}
	
	return true;
}

#else

// SDL version
bool image_file_t::make_rsrc_from_pict(void *data, size_t length, LoadedResource &rsrc, void *clut_data, size_t clut_length)
{
	if (length < 10)
		return false;

	// Extract size and depth
	uint8 *p = (uint8 *)data;
	int height = (p[4] << 8) + p[5];
	int width = (p[6] << 8) + p[7];
	int depth = (p[8] << 8) + p[9];
	if (depth != 8 && depth != 16)
		return false;

	// 8-bit depth requires CLUT
	if (depth == 8) {
		if (clut_data == NULL || clut_length != 6 + 256 * 6)
			return false;
	}

	// size(2), rect(8), versionOp(2), version(2), headerOp(26)
	int output_length = 2 + 8 + 2 + 2 + 26;
	int row_bytes;
	if (depth == 8) {
		// opcode(2), pixMap(46), colorTable(8+256*8), srcRect/dstRect/mode(18), data(variable)
		row_bytes = width;
		output_length += 2 + 46 + 8+256*8 + 18;
	} else {
		// opcode(2), pixMap(50), srcRect/dstRect/mode(18), data(variable)
		row_bytes = width * 2;
		output_length += 2 + 50 + 18;
	}
	// data(variable), opEndPic(2)
	output_length += row_bytes * height + 2;

	// Allocate memory for Mac PICT resource
	void *pict_rsrc = malloc(output_length);
	if (pict_rsrc == NULL)
		return false;
	memset(pict_rsrc, 0, output_length);

	// Convert pict tag to Mac PICT resource
	uint8 *q = (uint8 *)pict_rsrc;

	// 1. PICT header
	q[0] = output_length >> 8;
	q[1] = output_length;
	memcpy(q + 2, p, 8);
	q += 10;

	// 2. VersionOp/Version/HeaderOp
	q[0] = 0x00; q[1] = 0x11; // versionOp
	q[2] = 0x02; q[3] = 0xff; // version
	q[4] = 0x0c; q[5] = 0x00; // headerOp
	q[6] = 0xff; q[7] = 0xfe; // header version
	q[11] = 0x48; // hRes
	q[15] = 0x48; // vRes
	memcpy(q + 18, p, 8);
	q += 30;

	// 3. opcode
	if (depth == 8) {
		q[0] = 0x00; q[1] = 0x98;	// PackBitsRect
		q += 2;
	} else {
		q[0] = 0x00; q[1] = 0x9a;	// DirectBitsRect
		q += 6; // skip pmBaseAddr
	}

	// 4. PixMap
	q[0] = (row_bytes >> 8) | 0x80;
	q[1] = row_bytes;
	memcpy(q + 2, p, 8);
	q[13] = 0x01; // packType = unpacked
	q[19] = 0x48; // hRes
	q[23] = 0x48; // vRes
	q[27] = (depth == 8 ? 0 : 0x10); // pixelType
	q[29] = depth; // pixelSize
	q[31] = (depth == 8 ? 1 : 3); // cmpCount
	q[33] = (depth == 8 ? 8 : 5); // cmpSize
	q += 46;

	// 5. ColorTable
	if (depth == 8) {
		q[7] = 0xff; // ctSize
		q += 8;
		uint8 *p = (uint8 *)clut_data + 6;
		for (int i=0; i<256; i++) {
			q++;
			*q++ = i;	// value
			*q++ = *p++;	// red
			*q++ = *p++;
			*q++ = *p++;	// green
			*q++ = *p++;
			*q++ = *p++;	// blue
			*q++ = *p++;
		}
	}

	// 6. source/destination Rect and transfer mode
	memcpy(q, p, 8);
	memcpy(q + 8, p, 8);
	q += 18;

	// 7. graphics data
	memcpy(q, p + 10, row_bytes * height);
	q += row_bytes * height;

	// 8. OpEndPic
	q[0] = 0x00;
	q[1] = 0xff;

	rsrc.SetData(pict_rsrc, output_length);
	return true;
}

#endif

bool image_file_t::make_rsrc_from_clut(void *data, size_t length, LoadedResource &rsrc)
{
	const size_t input_length = 6 + 256 * 6;	// 6 bytes header, 256 entries with 6 bytes each
	const size_t output_length = 8 + 256 * 8;	// 8 bytes header, 256 entries with 8 bytes each

	if (length != input_length)
		return false;

	// Allocate memory for Mac CLUT resource
	void *clut_rsrc = malloc(output_length);
	if (clut_rsrc == NULL)
		return false;
	memset(clut_rsrc, 0, output_length);

	// Convert clut tag to Mac CLUT resource
	uint8 *p = (uint8 *)data;
	uint8 *q = (uint8 *)clut_rsrc;

	// 1. Header
	q[6] = p[0]; // color count
	q[7] = p[1];
	p += 6;
	q += 8;

	// 2. Color table
	for (int i=0; i<256; i++) {
		q++;
		*q++ = i;		// value
		*q++ = *p++;	// red
		*q++ = *p++;
		*q++ = *p++;	// green
		*q++ = *p++;
		*q++ = *p++;	// blue
		*q++ = *p++;
	}

	rsrc.SetData(clut_rsrc, output_length);
	return true;
}
