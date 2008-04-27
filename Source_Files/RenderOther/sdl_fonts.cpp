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
 *  sdl_fonts.cpp - SDL font handling
 *
 *  Written in 2000 by Christian Bauer
 */

#include "cseries.h"
#include "sdl_fonts.h"
#include "byte_swapping.h"
#include "resource_manager.h"
#include "FileHandler.h"
#include "Logging.h"

#include <SDL_endian.h>
#include <vector>
#include <map>

#ifndef NO_STD_NAMESPACE
using std::vector;
using std::pair;
using std::map;
#endif

#ifdef HAVE_SDL_TTF
#include <boost/tuple/tuple_comparison.hpp>
#include "DejaVuCondensedSansBold.h"
#include "preferences.h" // smooth_font
#endif

// Global variables
typedef pair<int, int> id_and_size_t;
typedef map<id_and_size_t, sdl_font_info *> font_list_t;
static font_list_t font_list;				// List of all loaded fonts

#ifdef HAVE_SDL_TTF
typedef pair<TTF_Font *, int> ref_counted_ttf_font_t;
typedef map<ttf_font_key_t, ref_counted_ttf_font_t> ttf_font_list_t;
static ttf_font_list_t ttf_font_list;
#endif

// From shell_sdl.cpp
extern vector<DirectorySpecifier> data_search_path;


/*
 *  Initialize font management
 */

void initialize_fonts(void)
{
        logContext("initializing fonts");
	// Open font resource files
	bool found = false;
	vector<DirectorySpecifier>::const_iterator i = data_search_path.begin(), end = data_search_path.end();
	while (i != end) {
		FileSpecifier fonts = *i + "Fonts";

		if (open_res_file(fonts))
			found = true;
		i++;
	}
	if (!found) {
		logFatal("Can't open font resource file");
/*
                vector<DirectorySpecifier>::const_iterator i = data_search_path.begin(), end = data_search_path.end();
                while (i != end) {
                        FileSpecifier fonts = *i + "Fonts";
                        fdprintf(fonts.GetPath());
                        i++;
                }
*/                
		exit(1);
	}
}


/*
 *  Load font from resources and allocate sdl_font_info
 */

sdl_font_info *load_sdl_font(const TextSpec &spec)
{
	sdl_font_info *info = NULL;

	// Look for ID/size in list of loaded fonts
	id_and_size_t id_and_size(spec.font, spec.size);
	font_list_t::const_iterator it = font_list.find(id_and_size);
	if (it != font_list.end()) {	// already loaded
		info = it->second;
		info->ref_count++;
		return info;
	}

	// Load font family resource
	LoadedResource fond;
	if (!get_resource(FOUR_CHARS_TO_INT('F', 'O', 'N', 'D'), spec.font, fond)) {
		fprintf(stderr, "Font family resource for font ID %d not found\n", spec.font);
		return NULL;
	}
	SDL_RWops *p = SDL_RWFromMem(fond.GetPointer(), (int)fond.GetLength());
	assert(p);

	// Look for font size in association table
	SDL_RWseek(p, 52, SEEK_SET);
	int num_assoc = SDL_ReadBE16(p) + 1;
	while (num_assoc--) {
		int size = SDL_ReadBE16(p);
		SDL_ReadBE16(p); // skip style
		int id = SDL_ReadBE16(p);
		if (size == spec.size) {

			// Size found, load bitmap font resource
			info = new sdl_font_info;
			if (!get_resource(FOUR_CHARS_TO_INT('N', 'F', 'N', 'T'), id, info->rsrc))
				get_resource(FOUR_CHARS_TO_INT('F', 'O', 'N', 'T'), id, info->rsrc);
			if (info->rsrc.IsLoaded()) {

				// Found, switch stream to font resource
				SDL_RWclose(p);
				p = SDL_RWFromMem(info->rsrc.GetPointer(), (int)info->rsrc.GetLength());
				assert(p);
				void *font_ptr = info->rsrc.GetPointer(true);

				// Read font information
				SDL_RWseek(p, 2, SEEK_CUR);
				info->first_character = static_cast<uint8>(SDL_ReadBE16(p));
				info->last_character = static_cast<uint8>(SDL_ReadBE16(p));
				SDL_RWseek(p, 2, SEEK_CUR);
				info->maximum_kerning = SDL_ReadBE16(p);
				SDL_RWseek(p, 2, SEEK_CUR);
				info->rect_width = SDL_ReadBE16(p);
				info->rect_height = SDL_ReadBE16(p);
				SDL_RWseek(p, 2, SEEK_CUR);
				info->ascent = SDL_ReadBE16(p);
				info->descent = SDL_ReadBE16(p);
				info->leading = SDL_ReadBE16(p);
				int bytes_per_row = SDL_ReadBE16(p) * 2;

				//printf(" first %d, last %d, max_kern %d, rect_w %d, rect_h %d, ascent %d, descent %d, leading %d, bytes_per_row %d\n",
				//	info->first_character, info->last_character, info->maximum_kerning,
				//	info->rect_width, info->rect_height, info->ascent, info->descent, info->leading, bytes_per_row);

				// Convert bitmap to pixmap (1 byte/pixel)
				info->bytes_per_row = bytes_per_row * 8;
				uint8 *src = (uint8 *)font_ptr + SDL_RWtell(p);
				uint8 *dst = info->pixmap = (uint8 *)malloc(info->rect_height * info->bytes_per_row);
				assert(dst);
				for (int y=0; y<info->rect_height; y++) {
					for (int x=0; x<bytes_per_row; x++) {
						uint8 b = *src++;
						*dst++ = (b & 0x80) ? 0xff : 0x00;
						*dst++ = (b & 0x40) ? 0xff : 0x00;
						*dst++ = (b & 0x20) ? 0xff : 0x00;
						*dst++ = (b & 0x10) ? 0xff : 0x00;
						*dst++ = (b & 0x08) ? 0xff : 0x00;
						*dst++ = (b & 0x04) ? 0xff : 0x00;
						*dst++ = (b & 0x02) ? 0xff : 0x00;
						*dst++ = (b & 0x01) ? 0xff : 0x00;
					}
				}
				SDL_RWseek(p, info->rect_height * bytes_per_row, SEEK_CUR);

				// Set table pointers
				int table_size = info->last_character - info->first_character + 3;	// Tables contain 2 additional entries
				info->location_table = (uint16 *)((uint8 *)font_ptr + SDL_RWtell(p));
				byte_swap_memory(info->location_table, _2byte, table_size);
				SDL_RWseek(p, table_size * 2, SEEK_CUR);
				info->width_table = (int8 *)font_ptr + SDL_RWtell(p);

				// Add font information to list of known fonts
				info->ref_count++;
				font_list[id_and_size] = info;

			} else {
				delete info;
				info = NULL;
				fprintf(stderr, "Bitmap font resource ID %d not found\n", id);
			}
		}
	}

	// Free resources
	SDL_RWclose(p);
	return info;
}

#ifdef HAVE_SDL_TTF
static TTF_Font *load_ttf_font(const std::string& path, uint16 style, int16 size)
{
	// already loaded? increment reference counter and return pointer
	ttf_font_key_t search_key(path, style, size);
	ttf_font_list_t::iterator it = ttf_font_list.find(search_key);
	if (it != ttf_font_list.end())
	{
		TTF_Font *font = it->second.first;
		it->second.second++;

		return font;
	}

	TTF_Font *font;
	if (path == "gothic")
	{
		font = TTF_OpenFontRW(SDL_RWFromConstMem(deja_vu_condensed_sans_bold, sizeof(deja_vu_condensed_sans_bold)), 0, size);
	}
	else
	{
		font = TTF_OpenFont(path.c_str(), size);
	}

	if (font)
	{
		int ttf_style = TTF_STYLE_NORMAL;
		if (style & styleBold)
			ttf_style |= TTF_STYLE_BOLD;
		if (style & styleItalic)
			ttf_style |= TTF_STYLE_ITALIC;
		if (style & styleUnderline)
			ttf_style |= TTF_STYLE_UNDERLINE;
		
		TTF_SetFontStyle(font, ttf_style);
#ifdef TTF_HINTING_LIGHT
		if (environment_preferences->smooth_text)
			TTF_SetFontHinting(font, TTF_HINTING_LIGHT);
		else
			TTF_SetFontHinting(font, TTF_HINTING_MONO);
#endif
		
		ttf_font_key_t key(path, style, size);
		ref_counted_ttf_font_t value(font, 1);
		
		ttf_font_list[key] = value;
	}
	
	return font;	
}
#endif

font_info *load_font(const TextSpec &spec) {
//	return static_cast<font_info*>(load_font(spec));

#ifdef HAVE_SDL_TTF
	if (spec.normal != "")
	{
		uint16 loaded_style = spec.style & (styleBold | styleItalic | styleUnderline);
		TTF_Font *font = load_ttf_font(spec.normal, loaded_style, spec.size);
		if (font) 
		{
			ttf_font_info *info = new ttf_font_info;
			info->m_ttf = font;
			info->ttf_key = ttf_font_key_t(spec.normal, loaded_style, spec.size);
			return info;
		}
	}
	else
#endif
        if (spec.font != -1)
	{
		return static_cast<font_info *>(load_sdl_font(spec));
	}
	else
		return 0;
}


/*
 *  Unload font, free sdl_font_info
 */

void sdl_font_info::_unload()
{
	// Look for font in list of loaded fonts
	font_list_t::const_iterator i = font_list.begin(), end = font_list.end();
	while (i != end) {
		if (i->second == this) {

			// Found, decrement reference counter and delete
			ref_count--;
			if (ref_count <= 0) {
				delete this; // !
				font_list.erase(i->first);
				return;
			}
		}
		i++;
	}
}

#ifdef HAVE_SDL_TTF
void ttf_font_info::_unload()
{
	ttf_font_list_t::iterator it = ttf_font_list.find(ttf_key);
	if (it != ttf_font_list.end())
	{
		--(it->second.second);
		if (it->second.second <= 0)
		{
			TTF_CloseFont(it->second.first);
			ttf_font_list.erase(ttf_key);
		}
	}

	delete this;
}
#endif

void unload_font(font_info *info)
{
	info->_unload();
}

int8 sdl_font_info::char_width(uint8 c, uint16 style) const
{
	if (c < first_character || c > last_character)
		return 0;
	int8 width = width_table[(c - first_character) * 2 + 1] + ((style & styleBold) ? 1 : 0);
	if (width == -1)	// non-existant character
		width = width_table[(last_character - first_character + 1) * 2 + 1] + ((style & styleBold) ? 1 : 0);
	return width;
}

uint16 sdl_font_info::_text_width(const char *text, uint16 style, bool) const
{
	int width = 0;
	char c;
	while ((c = *text++) != 0)
		width += char_width(c, style);
	assert(0 <= width);
	assert(width == static_cast<int>(static_cast<uint16>(width)));
	return width;
}

uint16 sdl_font_info::_text_width(const char *text, size_t length, uint16 style, bool) const
{
	int width = 0;
	while (length--)
		width += char_width(*text++, style); 
	assert(0 <= width);
	assert(width == static_cast<int>(static_cast<uint16>(width)));
	return width;
}

int sdl_font_info::_trunc_text(const char *text, int max_width, uint16 style) const
{
	int width = 0;
	int num = 0;
	char c;
	while ((c = *text++) != 0) {
		width += char_width(c, style);
		if (width > max_width)
			break;
		num++;
	}
	return num;
}

// sdl_font_info::_draw_text is in screen_drawing.cpp

#ifdef HAVE_SDL_TTF
uint16 ttf_font_info::_text_width(const char *text, uint16, bool utf8) const
{
	int width = 0;
	if (utf8)
		TTF_SizeUTF8(m_ttf, text, &width, 0);
	else
	{
		uint16 *temp = new uint16[strlen(text) + 1];
		mac_roman_to_unicode(text, temp);
		TTF_SizeUNICODE(m_ttf, temp, &width, 0);
		delete[] temp;
	}
	return width;
}

uint16 ttf_font_info::_text_width(const char *text, size_t length, uint16 style, bool utf8) const
{
	char *s = strdup(text);
	if (strlen(s) > length)
		s[length] = '\0';
	int w = text_width(s, style, utf8);
	free(s);
	return w;
}

int ttf_font_info::_trunc_text(const char *text, int max_width, uint16 style) const
{
	int width;
	uint16 *temp = new uint16[strlen(text) + 1];
	mac_roman_to_unicode(text, temp);
	TTF_SizeUNICODE(m_ttf, temp, &width, 0);
	if (width < max_width) return strlen(text);

	int num = strlen(text) - 1;

	while (num > 0 && width > max_width)
	{
		num--;
		temp[num] = 0x0;
		TTF_SizeUNICODE(m_ttf, temp, &width, 0);
	}

	delete temp;
	return num;
}

// ttf_font_info::_draw_text is in screen_drawing.cpp
#endif

uint16 font_info::text_width(const char *text, uint16 style, bool utf8) const
{
	if (style & styleShadow)
		return _text_width(text, style, utf8) + 1;
	else
		return _text_width(text, style, utf8);
}

uint16 font_info::text_width(const char *text, size_t length, uint16 style, bool utf8) const
{
	if (style & styleShadow)
		return _text_width(text, length, style, utf8) + 1;
	else
		return _text_width(text, length, style, utf8);
}

int font_info::draw_text(SDL_Surface *s, const char *text, size_t length, int x, int y, uint32 pixel, uint16 style, bool utf8) const
{
	if (style & styleShadow)
	{
		_draw_text(s, text, length, x + 1, y + 1, SDL_MapRGB(s->format, 0x0, 0x0, 0x0), style, utf8);
	}

	_draw_text(s, text, length, x, y, pixel, style, utf8);
}

int font_info::trunc_text(const char *text, int max_width, uint16 style) const
{
	if (style & styleShadow)
		return _trunc_text(text, max_width - 1, style);
	else
		return _trunc_text(text, max_width, style);
}
