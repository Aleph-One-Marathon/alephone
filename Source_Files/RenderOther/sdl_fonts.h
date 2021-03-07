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
 *  sdl_fonts.h - SDL font handling
 *
 *  Written in 2000 by Christian Bauer
 */

#if defined _MSC_VER
#define NOMINMAX
#include <algorithm>
#endif

#ifndef SDL_FONTS_H
#define SDL_FONTS_H

#include "csfonts.h"
#include "FileHandler.h"
#include <SDL_ttf.h>
#include <boost/tuple/tuple.hpp>

#include <string>

/*
 *  Definitions
 */


class font_info {
	friend void unload_font(font_info *font);
public:
	virtual uint16 get_ascent(void) const = 0;
	virtual uint16 get_height(void) const = 0;
	virtual uint16 get_line_height(void) const = 0;
	virtual uint16 get_descent(void) const = 0;
	virtual int16 get_leading(void) const = 0;

	int draw_text(SDL_Surface *s, const char *text, size_t length, int x, int y, uint32 pixel, uint16 style, bool utf8 = false) const;
	uint16 text_width(const char *text, size_t length, uint16 style, bool utf8 = false) const;
	uint16 text_width(const char *text, uint16 style, bool utf8 = false) const;
	int trunc_text(const char *text, int max_width, uint16 style) const;
	virtual int8 char_width(uint8 c, uint16 style) const = 0;

	int draw_styled_text(SDL_Surface *s, const std::string& text, size_t length, int x, int y, uint32 pixel, uint16 initial_style, bool utf = false) const;
	int styled_text_width(const std::string& text, size_t length, uint16 initial_style, bool utf8 = false) const;
	int trunc_styled_text(const std::string& text, int max_width, uint16 style) const;
	std::string style_at(const std::string& text, std::string::const_iterator pos, uint16 style) const;
	virtual ~font_info() = default;
protected:
	virtual int _draw_text(SDL_Surface *s, const char *text, size_t length, int x, int y, uint32 pixel, uint16 style, bool utf8) const = 0;
	virtual uint16 _text_width(const char *text, size_t length, uint16 style, bool utf8) const = 0;
	virtual uint16 _text_width(const char *text, uint16 style, bool utf8) const = 0;
	virtual int _trunc_text(const char *text, int max_width, uint16 style) const = 0;
private:
	// wrapped behind unload_font, because we call delete this!
	virtual void _unload() = 0;
};

// Font information structure
class sdl_font_info : public font_info {
	friend sdl_font_info *load_sdl_font(const TextSpec &spec);
	friend void unload_sdl_font(sdl_font_info *font);

public:
	sdl_font_info() : first_character(0), last_character(0),
		ascent(0), descent(0), leading(0), pixmap(NULL), ref_count(0) {}
	virtual ~sdl_font_info() {if (pixmap) free(pixmap);}

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

	int8 char_width(uint8 c, uint16 style) const;

protected:
	virtual int _draw_text(SDL_Surface *s, const char *text, size_t length, int x, int y, uint32 pixel, uint16 style, bool utf8) const;
	virtual uint16 _text_width(const char *text, size_t length, uint16 style, bool utf8) const;
	virtual uint16 _text_width(const char *text, uint16 style, bool utf8) const;
	virtual int _trunc_text(const char *text, int max_width, uint16 style) const;
private:
	virtual void _unload();
	int ref_count;
	LoadedResource rsrc;
};

typedef boost::tuple<std::string, uint16, int16> ttf_font_key_t;

class ttf_font_info : public font_info { 
public:
	uint16 get_ascent() const { return TTF_FontAscent(m_styles[styleNormal]); };
	uint16 get_height() const { return TTF_FontHeight(m_styles[styleNormal]); };
	uint16 get_line_height() const { return m_line_height + m_adjust_height; }
	uint16 get_descent() const { return -TTF_FontDescent(m_styles[styleNormal]); }
	int16 get_leading() const { return get_line_height() - get_ascent() - get_descent(); }

	TTF_Font* m_styles[styleUnderline];
	ttf_font_key_t m_keys[styleUnderline];
	int m_adjust_height;
	int m_line_height;

	int8 char_width(uint8, uint16) const;

	ttf_font_info() { 
		for (int i = 0; i < styleUnderline; i++) { m_styles[i] = 0; } 
	}
	virtual ~ttf_font_info() = default;
protected:
	virtual int _draw_text(SDL_Surface *s, const char *text, size_t length, int x, int y, uint32 pixel, uint16 style, bool utf8) const;
	virtual uint16 _text_width(const char *text, size_t length, uint16 style, bool utf8) const;
	virtual uint16 _text_width(const char *text, uint16 style, bool utf8) const;	
	virtual int _trunc_text(const char *text, int max_width, uint16 style) const;
private:
	char *process_printable(const char *src, int len) const;
	uint16 *process_macroman(const char *src, int len) const;
	TTF_Font *get_ttf(uint16 style) const { return m_styles[style & (styleBold | styleItalic)]; }
	virtual void _unload();
};

/*
 *  Functions
 */

// Initialize font management
extern void initialize_fonts(bool last_chance);

// Load font, return pointer to font info
extern font_info *load_font(const TextSpec &spec);

// Unload font
extern void unload_font(font_info *font);

#endif
