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

#include <SDL_endian.h>
#include <map>


// Global variables
typedef pair<int, int> id_and_size_t;
typedef map<id_and_size_t, sdl_font_info *> font_list_t;
static font_list_t font_list;				// List of all loaded fonts


/*
 *  Initialize font management
 */

void initialize_fonts(void)
{
	// Open font resources
	FileSpecifier fonts;
	fonts.SetToGlobalDataDir();
	fonts.AddPart("Fonts");
	if (open_res_file(fonts) == NULL) {
		fprintf(stderr, "Can't open '%s'.\n", fonts.GetPath());
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
	SDL_RWops *p = SDL_RWFromMem(fond.GetPointer(), fond.GetLength());
	assert(p);

	// Look for font size in association table
	SDL_RWseek(p, 52, SEEK_SET);
	int num_assoc = SDL_ReadBE16(p) + 1;
	while (num_assoc--) {
		int size = SDL_ReadBE16(p);
		int style = SDL_ReadBE16(p);
		int id = SDL_ReadBE16(p);
		if (size == spec.size) {

			// Size found, load bitmap font resource
			info = new sdl_font_info;
			if (!get_resource(FOUR_CHARS_TO_INT('N', 'F', 'N', 'T'), id, info->rsrc))
				get_resource(FOUR_CHARS_TO_INT('F', 'O', 'N', 'T'), id, info->rsrc);
			if (info->rsrc.IsLoaded()) {

				// Found, switch stream to font resource
				SDL_RWclose(p);
				p = SDL_RWFromMem(info->rsrc.GetPointer(), info->rsrc.GetLength());
				assert(p);
				void *font_ptr = info->rsrc.GetPointer(true);

				// Read font information
				SDL_RWseek(p, 2, SEEK_CUR);
				info->first_character = SDL_ReadBE16(p);
				info->last_character = SDL_ReadBE16(p);
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
