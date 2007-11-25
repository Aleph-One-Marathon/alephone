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
#include "urw_gothic.h"
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

sdl_font_info *load_font(const TextSpec &spec)
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

ttf_and_sdl_font_info *load_ttf_and_sdl_font(const DualFontSpec& spec)
{
#ifdef HAVE_SDL_TTF
	if (spec.prefer_old_font && spec.font_id >= 0)
	{
		TextSpec old_spec;
		old_spec.font = spec.font_id;
		old_spec.style = spec.style;
		old_spec.size = spec.size;

		sdl_font_info *info = load_font(old_spec);
		if (info)
		{
			std::auto_ptr<ttf_and_sdl_font_info> ttf_and_sdl_info(new ttf_and_sdl_font_info);
			ttf_and_sdl_info->set_sdl_font_info(info);
			return ttf_and_sdl_info.release();
		}
	}


	// look for loaded fonts
	int loaded_style = spec.ttf_style & (DualFontSpec::styleBold | DualFontSpec::styleItalic | DualFontSpec::styleUnderline);

	ttf_font_key_t search_key(spec.path, loaded_style, spec.ttf_size);
	
	ttf_font_list_t::iterator it = ttf_font_list.find(search_key);
	if (it != ttf_font_list.end()) 
	{
		TTF_Font *font = it->second.first;
		it->second.second++;
		
		std::auto_ptr<ttf_and_sdl_font_info> ttf_and_sdl_info(new ttf_and_sdl_font_info);
		ttf_and_sdl_info->set_ttf_font_info(font);
		ttf_and_sdl_info->ttf_key = search_key;
		return ttf_and_sdl_info.release();
	}
	
	TTF_Font *font;
	if (spec.path == "gothic")
	{
		font = TTF_OpenFontRW(SDL_RWFromConstMem(urw_gothic, sizeof(urw_gothic)), 0, spec.ttf_size);
	}
	else
	{
		font = TTF_OpenFont(spec.path.c_str(), spec.ttf_size);
	}

	if (font)
	{
		int ttf_style = TTF_STYLE_NORMAL;
		if (spec.ttf_style & DualFontSpec::styleBold)
			ttf_style |= TTF_STYLE_BOLD;
		if (spec.ttf_style & DualFontSpec::styleItalic)
			ttf_style |= TTF_STYLE_ITALIC;
		if (spec.ttf_style & DualFontSpec::styleUnderline)
			ttf_style |= TTF_STYLE_UNDERLINE;
		
		TTF_SetFontStyle(font, ttf_style);
#ifdef TTF_HINTING_LIGHT
		TTF_SetFontHinting(font, TTF_HINTING_LIGHT);
#endif
		
		ttf_font_key_t key(spec.path, loaded_style, spec.ttf_size);
		ref_counted_ttf_font_t value(font, 1);

		ttf_font_list[key] = value;

		std::auto_ptr<ttf_and_sdl_font_info> ttf_and_sdl_info(new ttf_and_sdl_font_info);
		ttf_and_sdl_info->set_ttf_font_info(font);
		ttf_and_sdl_info->ttf_key = key;
		return ttf_and_sdl_info.release();
	}
#endif
	// not loaded, try the old font
	TextSpec old_spec;
	old_spec.font = spec.font_id;
	old_spec.style = spec.style;
	old_spec.size = spec.size;
	
	sdl_font_info *info = load_font(old_spec);
	if (info)
	{
		std::auto_ptr<ttf_and_sdl_font_info> ttf_and_sdl_info(new ttf_and_sdl_font_info);
		ttf_and_sdl_info->set_sdl_font_info(info);
		return ttf_and_sdl_info.release();
	}
	
	return 0;
}

ttf_and_sdl_font_info *load_ttf_and_sdl_font(const TextSpec& spec)
{
	sdl_font_info *info = load_font(spec);
	if (info)
	{
		ttf_and_sdl_font_info* ttf_and_sdl_info = new ttf_and_sdl_font_info;
		ttf_and_sdl_info->set_sdl_font_info(info);
		return ttf_and_sdl_info;
	}
	else
	{
		return 0;
	}
}

/*
 *  Unload font, free sdl_font_info
 */

void unload_font(sdl_font_info *info)
{
	// Look for font in list of loaded fonts
	font_list_t::const_iterator i = font_list.begin(), end = font_list.end();
	while (i != end) {
		if (i->second == info) {

			// Found, decrement reference counter and delete
			info->ref_count--;
			if (info->ref_count <= 0) {
				delete info;
				font_list.erase(i->first);
				return;
			}
		}
		i++;
	}
}

void unload_font(ttf_and_sdl_font_info *info)
{
#ifdef HAVE_SDL_TTF
	if (info->get_ttf_font_info())
	{
		ttf_font_list_t::iterator it = ttf_font_list.find(info->ttf_key);
		if (it != ttf_font_list.end())
		{
			--(it->second.second);
			if (it->second.second <= 0)
			{
				TTF_CloseFont(it->second.first);
				ttf_font_list.erase(info->ttf_key);
			}
		}

	}
#endif

	if (info->get_sdl_font_info())
	{
		unload_font(info->get_sdl_font_info());
	}
	
	delete info;
}

uint16 ttf_and_sdl_font_info::get_ascent() const
{
#ifdef HAVE_SDL_TTF
	if (is_ttf_font())
	{
		return TTF_FontAscent(m_ttf_font_info);
	}
	else
#endif
	{
		return m_sdl_font_info->get_ascent();
	}
}

uint16 ttf_and_sdl_font_info::get_height() const
{
#ifdef HAVE_SDL_TTF
	if (is_ttf_font())
	{
		return TTF_FontHeight(m_ttf_font_info);
	}
	else
#endif
	{
		return m_sdl_font_info->get_height();
	}
}

uint16 ttf_and_sdl_font_info::get_line_height() const
{
#ifdef HAVE_SDL_TTF
	if (is_ttf_font())
	{
		return max(TTF_FontLineSkip(m_ttf_font_info), TTF_FontHeight(m_ttf_font_info));;
	}
	else
#endif
	{
		return m_sdl_font_info->get_line_height();
	}
}

uint16 ttf_and_sdl_font_info::get_descent() const
{
#ifdef HAVE_SDL_TTF
	if (is_ttf_font())
	{
		return TTF_FontDescent(m_ttf_font_info);
	}
	else
#endif
	{
		return m_sdl_font_info->get_descent();
	}
}

int16 ttf_and_sdl_font_info::get_leading() const
{
#ifdef HAVE_SDL_TTF
	if (is_ttf_font())
	{
		return get_line_height() - get_ascent() - get_descent();
	}
	else
#endif
	{
		return m_sdl_font_info->get_leading();
	}
}
