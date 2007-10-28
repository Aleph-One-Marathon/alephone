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
#ifdef HAVE_SDL_TTF
#include <SDL_ttf.h>
#endif

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

class ttf_and_sdl_font_info {
public:

	ttf_and_sdl_font_info() : m_sdl_font_info(0)
#ifdef HAVE_SDL_TTF
				, m_ttf_font_info(0) 
#endif
		{ }
	uint16 get_ascent() const;
	uint16 get_height() const;
	uint16 get_line_height() const;
	uint16 get_descent() const;
	int16 get_leading() const;

	void set_sdl_font_info(sdl_font_info *font_info) { m_sdl_font_info = font_info; }
	sdl_font_info* get_sdl_font_info() { return m_sdl_font_info; }
#ifdef HAVE_SDL_TTF
	void set_ttf_font_info(TTF_Font *ttf_font_info) { m_ttf_font_info = ttf_font_info; }
	TTF_Font* get_ttf_font_info() { return m_ttf_font_info; }
	bool is_ttf_font() const { return m_ttf_font_info; }
#endif

private:
	sdl_font_info* m_sdl_font_info;
#ifdef HAVE_SDL_TTF
	TTF_Font* m_ttf_font_info;
#endif

};

	

/*
 *  Functions
 */

// Initialize font management
extern void initialize_fonts(void);

// Load font, return pointer to font info
extern sdl_font_info *load_font(const TextSpec &spec);
extern ttf_and_sdl_font_info *load_ttf_and_sdl_font(const TextSpec &spec);

// Unload font
extern void unload_font(sdl_font_info *font);
extern void unload_font(ttf_and_sdl_font_info *font);

#endif
