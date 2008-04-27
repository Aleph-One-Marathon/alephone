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
#include <boost/tuple/tuple.hpp>
#endif

#include <string>

/*
 *  Definitions
 */


class font_info {
public:
	virtual uint16 get_ascent(void) const = 0;
	virtual uint16 get_height(void) const = 0;
	virtual uint16 get_line_height(void) const = 0;
	virtual uint16 get_descent(void) const = 0;
	virtual int16 get_leading(void) const = 0;
};

// Font information structure
class sdl_font_info : public font_info {
	friend sdl_font_info *load_sdl_font(const TextSpec &spec);
	friend void unload_sdl_font(sdl_font_info *font);

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

#ifdef HAVE_SDL_TTF
typedef boost::tuple<std::string, uint16, int16> ttf_font_key_t;

class ttf_font_info : public font_info { 
public:
	uint16 get_ascent() const { return TTF_FontAscent(m_ttf); };
	uint16 get_height() const { return TTF_FontHeight(m_ttf); };
	uint16 get_line_height() const { return max(TTF_FontLineSkip(m_ttf), TTF_FontHeight(m_ttf)); }
	uint16 get_descent() const { return TTF_FontDescent(m_ttf); }
	int16 get_leading() const { return get_line_height() - get_ascent() - get_descent(); }

	ttf_font_key_t ttf_key;
	TTF_Font *m_ttf;
};
#endif

/*
 *  Functions
 */

// Initialize font management
extern void initialize_fonts(void);

// Load font, return pointer to font info
extern font_info *load_font_info(const TextSpec &spec);

// Unload font
extern void unload_font(font_info *font);

#endif
