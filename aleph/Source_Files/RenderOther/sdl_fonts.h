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

*/

/*
 *  sdl_fonts.h - SDL font handling
 *
 *  Written in 2000 by Christian Bauer
 */

#ifndef SDL_FONTS_H
#define SDL_FONTS_H

#include "FileHandler.h"


/*
 *  Definitions
 */

// Font information structure
class sdl_font_info {
	friend sdl_font_info *load_font(const TextSpec &spec);
	friend void unload_font(sdl_font_info *font);

public:
	sdl_font_info() : first_character(0), last_character(0),
		ascent(0), descent(0), leading(0), pixmap(NULL), ref_count(0) {}
	~sdl_font_info() {if (pixmap) free(pixmap);}

	uint16 get_ascent(void) const {return ascent;}
	uint16 get_height(void) const {return ascent + descent;}
	uint16 get_line_height(void) const {return ascent + descent + leading;}
	uint16 get_descent(void) const {return descent; }
	int16 get_leading(void) const { return leading;}

	uint8 first_character, last_character;
	int16 maximum_kerning;
	int16 rect_width, rect_height;
	uint16 ascent, descent;
	int16 leading;

	uint8 *pixmap;			// Font image (1 byte/pixel)
	int bytes_per_row;		// Bytes per row in pixmap

	uint16 *location_table;	// Table of byte-offsets into pixmap (points into resource)
	int8 *width_table;		// Table of kerning/width info (points into resource)

private:
	int ref_count;
	LoadedResource rsrc;
};


/*
 *  Functions
 */

// Initialize font management
extern void initialize_fonts(void);

// Load font, return pointer to font info
extern sdl_font_info *load_font(const TextSpec &spec);

// Unload font
extern void unload_font(sdl_font_info *font);

#endif
