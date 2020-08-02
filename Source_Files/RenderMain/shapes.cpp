/*
SHAPES.C

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

Saturday, September 4, 1993 9:26:41 AM

Thursday, May 19, 1994 9:06:28 AM
	unification of wall and object shapes complete, new shading table builder.
Wednesday, June 22, 1994 11:55:07 PM
	we now read data from alain’s shape extractor.
Saturday, July 9, 1994 3:22:11 PM
	lightening_table removed; we now build darkening tables on a collection-by-collection basis
	(one 8k darkening table per clut permutation of the given collection)
Monday, October 3, 1994 4:17:15 PM (Jason)
	compressed or uncompressed collection resources
Friday, June 16, 1995 11:34:08 AM  (Jason)
	self-luminescent colors

Jan 30, 2000 (Loren Petrich):
	Changed "new" to "_new" to make data structures more C++-friendly
	Did some typecasts

Feb 3, 2000 (Loren Petrich):
	Changed _collection_madd to _collection_vacbob (later changed all "vacbob"'s to "civilian_fusion"'s)

Feb 4, 2000 (Loren Petrich):
	Changed halt() to assert(false) for better debugging

Feb 12, 2000 (Loren Petrich):
	Set up a fallback strategy for the colors;
	when there are more colors in all the tables than there are in the general color table,
	then look for the nearest one.

Feb 24, 2000 (Loren Petrich):
	Added get_number_of_collection_frames(), so as to assist in wall-texture error checking

Mar 14, 2000 (Loren Petrich):
	Added accessors for number of bitmaps and which bitmap index for a frame index;
	these will be useful for OpenGL rendering

Mar 23, 2000 (Loren Petrich):
	Made infravision tinting more generic and reassignable

Aug 12, 2000 (Loren Petrich):
	Using object-oriented file handler
	
Aug 14, 2000 (Loren Petrich):
	Turned collection and shading-table handles into pointers,
	because handles are needlessly MacOS-specific,
	and because these are variable-format objects.

Aug 26, 2000 (Loren Petrich):
	Moved get_default_shapes_spec() to preprocess_map_mac.c

Sept 2, 2000 (Loren Petrich):
	Added shapes-file unpacking.

Jan 17, 2001 (Loren Petrich):
	Added support for offsets for OpenGL-rendered substitute textures
*/

/*
//gracefully handle out-of-memory conditions when loading shapes.  it will happen.
//get_shape_descriptors() needs to look at high-level instead of low-level shapes when fetching scenery instead of walls/ceilings/floors
//get_shape_information() is called often, and is quite slow
//it is possible to have more than 255 low-level shapes in a collection, which means the existing shape_descriptor is too small
//must build different shading tables for each collection (even in 8-bit, for alternate color tables)
*/

#include "cseries.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shell.h"
#include "render.h"
#include "interface.h"
#include "collection_definition.h"
#include "screen.h"
#include "game_errors.h"
#include "FileHandler.h"
#include "progress.h"
#include "images.h"

#include "map.h"

// LP addition: OpenGL support
#include "OGL_Render.h"
#include "OGL_LoadScreen.h"

// LP addition: infravision XML setup needs colors
#include "InfoTree.h"

#include "Packing.h"
#include "SW_Texture_Extras.h"

#include <SDL_rwops.h>
#include <memory>

#include <boost/shared_ptr.hpp>

#include "Plugins.h"

/* ---------- constants */

#define iWHITE 1
#ifdef SCREAMING_METAL
#define iBLACK 255
#else
#define iBLACK 18
#endif

/* each collection has a tint table which (fully) tints the clut of that collection to whatever it
	looks like through the light enhancement goggles */
#define NUMBER_OF_TINT_TABLES 1

// Moved from shapes_macintosh.c:

// Possibly of historical interest:
// #define COLLECTIONS_RESOURCE_BASE 128
// #define COLLECTIONS_RESOURCE_BASE16 1128

enum /* collection status */
{
	markNONE,
	markLOAD= 1,
	markUNLOAD= 2,
	markSTRIP= 4 /* we don’t want bitmaps, just high/low-level shape data */,
	markPATCHED = 8 /* force re-load */
};

enum /* flags */
{
	_collection_is_stripped= 0x0001
};

/* ---------- macros */

// LP: fake portable-files stuff
inline short memory_error() {return 0;}

/* ---------- structures */

/* ---------- globals */

extern SDL_Surface* world_pixels;

#include "shape_definitions.h"

static pixel16 *global_shading_table16= (pixel16 *) NULL;
static pixel32 *global_shading_table32= (pixel32 *) NULL;

short number_of_shading_tables, shading_table_fractional_bits, shading_table_size;

// LP addition: opened-shapes-file object
static OpenedFile ShapesFile;
OpenedResourceFile M1ShapesFile;

static enum {
	M1_SHAPES_VERSION = 1,
	M2_SHAPES_VERSION
} shapes_file_version;

bool shapes_file_is_m1() { return shapes_file_version == M1_SHAPES_VERSION; }

/* ---------- private prototypes */

static void update_color_environment(bool is_opengl);
static short find_or_add_color(struct rgb_color_value *color, struct rgb_color_value *colors, short *color_count, bool update_flags);
static void _change_clut(void (*change_clut_proc)(struct color_table *color_table), struct rgb_color_value *colors, short color_count);

static void build_shading_tables8(struct rgb_color_value *colors, short color_count, pixel8 *shading_tables);
static void build_shading_tables16(struct rgb_color_value *colors, short color_count, pixel16 *shading_tables, byte *remapping_table, bool is_opengl);
static void build_shading_tables32(struct rgb_color_value *colors, short color_count, pixel32 *shading_tables, byte *remapping_table, bool is_opengl);
static void build_global_shading_table16(void);
static void build_global_shading_table32(void);

static bool get_next_color_run(struct rgb_color_value *colors, short color_count, short *start, short *count);
static bool new_color_run(struct rgb_color_value *_new, struct rgb_color_value *last);

static int32 get_shading_table_size(short collection_code);

static void build_collection_tinting_table(struct rgb_color_value *colors, short color_count, short collection_index, bool is_opengl);
static void build_tinting_table8(struct rgb_color_value *colors, short color_count, pixel8 *tint_table, short tint_start, short tint_count);
static void build_tinting_table16(struct rgb_color_value *colors, short color_count, pixel16 *tint_table, struct rgb_color *tint_color);
static void build_tinting_table32(struct rgb_color_value *colors, short color_count, pixel32 *tint_table, struct rgb_color *tint_color, bool is_opengl);

static void precalculate_bit_depth_constants(void);

static bool collection_loaded(struct collection_header *header);
static void unload_collection(struct collection_header *header);
static void unlock_collection(struct collection_header *header);
static void lock_collection(struct collection_header *header);
static bool load_collection(short collection_index, bool strip);

static void shutdown_shape_handler(void);
static void close_shapes_file(void);

// static byte *make_stripped_collection(byte *collection);

/* --------- collection accessor prototypes */

// Modified to return NULL for unloaded collections and out-of-range indices for collection contents.
// This is to allow for more graceful degradation.

static struct collection_header *get_collection_header(short collection_index);
/*static*/ struct collection_definition *get_collection_definition(short collection_index);
static void *get_collection_shading_tables(short collection_index, short clut_index);
static void *get_collection_tint_tables(short collection_index, short tint_index);
static struct rgb_color_value *get_collection_colors(short collection_index, short clut_number);
static struct high_level_shape_definition *get_high_level_shape_definition(short collection_index, short high_level_shape_index);
static struct bitmap_definition *get_bitmap_definition(short collection_index, short bitmap_index);


#include <SDL_endian.h>
#include "byte_swapping.h"

/*
 *  Initialize shapes handling
 */

static void initialize_pixmap_handler()
{
	// nothing to do
}


/*
 *  Convert shape to surface
 */

// ZZZ extension: pass out (if non-NULL) a pointer to a block of pixel data -
// caller should free() that storage after freeing the returned surface.
// Only needed for RLE-encoded shapes.
// Note that default arguments are used to make this function
// source-code compatible with existing usage.
// Note also that inShrinkImage currently only applies to RLE shapes.
SDL_Surface *get_shape_surface(int shape, int inCollection, byte** outPointerToPixelData, float inIllumination, bool inShrinkImage)
{
	// Get shape information
	int collection_index = GET_COLLECTION(GET_DESCRIPTOR_COLLECTION(shape));
	int clut_index = GET_COLLECTION_CLUT(GET_DESCRIPTOR_COLLECTION(shape));
	int low_level_shape_index = GET_DESCRIPTOR_SHAPE(shape);
        
        if(inCollection != NONE) {
            collection_index = GET_COLLECTION(inCollection);
            clut_index = GET_COLLECTION_CLUT(inCollection);
            low_level_shape_index = shape;
        }

	struct collection_definition *collection = get_collection_definition(collection_index);
	struct low_level_shape_definition *low_level_shape = get_low_level_shape_definition(collection_index, low_level_shape_index);
	if (!low_level_shape) return NULL;
	struct bitmap_definition *bitmap;
        SDL_Color colors[256];

        if(inIllumination >= 0) {
            assert(inIllumination <= 1.0f);
        
            // ZZZ: get shading tables to use instead of CLUT, if requested
            void*	shading_tables_as_void;
            extended_get_shape_bitmap_and_shading_table(BUILD_COLLECTION(collection_index, clut_index), low_level_shape_index,
                    &bitmap, &shading_tables_as_void, _shading_normal);
            if (!bitmap) return NULL;
            
            switch(bit_depth) {
                case 16:
                {
                    uint16*	shading_tables	= (uint16*) shading_tables_as_void;
                    shading_tables += 256 * (int)(inIllumination * (number_of_shading_tables - 1));
                    
                    // Extract color table - ZZZ change to use shading table rather than CLUT.  Hope it works.

		    SDL_PixelFormat *fmt = &pixel_format_16;
		    for (int i = 0; i < 256; i++) {
			    SDL_GetRGB(shading_tables[i], fmt, &colors[i].r, &colors[i].g, &colors[i].b);
			    colors[i].a = 0xff;
                    }
                }
                break;
                
                case 32:
                {
                    uint32*	shading_tables	= (uint32*) shading_tables_as_void;
                    shading_tables += 256 * (int)(inIllumination * (number_of_shading_tables - 1));
                    
                    // Extract color table - ZZZ change to use shading table rather than CLUT.  Hope it works.
                    for(int i = 0; i < 256; i++) {
                        colors[i].r = RED32(shading_tables[i]);
                        colors[i].g = GREEN32(shading_tables[i]);
                        colors[i].b = BLUE32(shading_tables[i]);
                        colors[i].a = 0xff;
                    }
                }
                break;
                
                default:
                    vhalt("oops, bit_depth not supported for get_shape_surface with illumination\n");
                break;
            }

        } // inIllumination >= 0
        else { // inIllumination < 0
            bitmap = get_bitmap_definition(collection_index, low_level_shape->bitmap_index);
            if(!bitmap) return NULL;
            
            // Extract color table
            int num_colors = collection->color_count - NUMBER_OF_PRIVATE_COLORS;
            rgb_color_value *src_colors = get_collection_colors(collection_index, clut_index) + NUMBER_OF_PRIVATE_COLORS;
            for (int i=0; i<num_colors && src_colors; i++) {
                    int idx = src_colors[i].value;
                    colors[idx].r = src_colors[i].red >> 8;
                    colors[idx].g = src_colors[i].green >> 8;
                    colors[idx].b = src_colors[i].blue >> 8;
                    colors[idx].a = 0xff;
            }
        } // inIllumination < 0
        
        
	SDL_Surface *s = NULL;
	if (bitmap->bytes_per_row == NONE) {

                // ZZZ: process RLE-encoded shape
                
                // Allocate storage for un-RLE'd pixels
                uint32	theNumberOfStorageBytes = bitmap->width * bitmap->height * sizeof(byte);
                byte*	pixel_storage = (byte*) malloc(theNumberOfStorageBytes);
                memset(pixel_storage, 0, theNumberOfStorageBytes);
                
                // Here, a "run" is a row or column.  An "element" is a single pixel's data.
                // We always go forward through the source data.  Thus, the offsets for where the next run
                // or element goes into the destination data area change depending on the circumstances.
                int16	theNumRuns;
                int16	theDestDataNextRunOffset;
                int16	theDestDataNextElementOffset;

                // Is this row-major or column-major?
                if(bitmap->flags & _COLUMN_ORDER_BIT) {
                    theNumRuns				= bitmap->width;
                    theDestDataNextRunOffset		= (low_level_shape->flags & _X_MIRRORED_BIT) ? -1 : 1;
                    theDestDataNextElementOffset	= (low_level_shape->flags & _Y_MIRRORED_BIT) ? -bitmap->width : bitmap->width;
                }
                else {
                    theNumRuns				= bitmap->height;
                    theDestDataNextElementOffset	= (low_level_shape->flags & _X_MIRRORED_BIT) ? -1 : 1;
                    theDestDataNextRunOffset		= (low_level_shape->flags & _Y_MIRRORED_BIT) ? -bitmap->width : bitmap->width;
                }
                
                // Figure out where our first byte will be written
                byte* theDestDataStartAddress = pixel_storage;
                
                if(low_level_shape->flags & _X_MIRRORED_BIT)
                    theDestDataStartAddress += bitmap->width - 1;
                
                if(low_level_shape->flags & _Y_MIRRORED_BIT)
                    theDestDataStartAddress += bitmap->width * (bitmap->height - 1);
                
                // Walk through runs, un-RLE'ing as we go
                for(int run = 0; run < theNumRuns; run++) {
                    uint16*	theLengthData 					= (uint16*) bitmap->row_addresses[run];
                    uint16	theFirstOpaquePixelElement 			= SDL_SwapBE16(theLengthData[0]);
                    uint16	theFirstTransparentAfterOpaquePixelElement	= SDL_SwapBE16(theLengthData[1]);
                    uint16	theNumberOfOpaquePixels = theFirstTransparentAfterOpaquePixelElement - theFirstOpaquePixelElement;

                    byte*	theOriginalPixelData = (byte*) &theLengthData[2];
                    byte*	theUnpackedPixelData;
                    
                    theUnpackedPixelData = theDestDataStartAddress + run * theDestDataNextRunOffset
                                            + theFirstOpaquePixelElement * theDestDataNextElementOffset;
                    
                    for(int i = 0; i < theNumberOfOpaquePixels; i++) {
                        assert(theUnpackedPixelData >= pixel_storage);
                        assert(theUnpackedPixelData < (pixel_storage + theNumberOfStorageBytes));
                        *theUnpackedPixelData = *theOriginalPixelData;
                        theUnpackedPixelData += theDestDataNextElementOffset;
                        theOriginalPixelData++;
                    }
                }
                
                // Let's shrink the image if the user wants us to.
                // We do this here by discarding every other pixel in each direction.
                // Really, I guess there's probably a library out there that would do nice smoothing
                // for us etc. that we should use here.  I just want to hack something out now and run with it.
                int image_width		= bitmap->width;
                int image_height	= bitmap->height;

                if(inShrinkImage) {
                    int		theLargerWidth		= bitmap->width;
                    int		theLargerHeight		= bitmap->height;
                    byte*	theLargerPixelStorage	= pixel_storage;
                    int		theSmallerWidth		= theLargerWidth / 2 + theLargerWidth % 2;
                    int		theSmallerHeight	= theLargerHeight / 2 + theLargerHeight % 2;
                    byte*	theSmallerPixelStorage	= (byte*) malloc(theSmallerWidth * theSmallerHeight);
                    
                    for(int y = 0; y < theSmallerHeight; y++) {
                        for(int x = 0; x < theSmallerWidth; x++) {
                            theSmallerPixelStorage[y * theSmallerWidth + x] =
                                theLargerPixelStorage[(y * theLargerWidth + x) * 2];
                        }
                    }
                    
                    free(pixel_storage);
                    
                    pixel_storage	= theSmallerPixelStorage;
                    image_width		= theSmallerWidth;
                    image_height	= theSmallerHeight;
                }
                
                // Now we can create a surface from this new storage
		s = SDL_CreateRGBSurfaceFrom(pixel_storage, image_width, image_height, 8, image_width, 0, 0, 0, 0);

                if(s != NULL) {
                    // If caller is not prepared to take this data, it's a coding error.
                    assert(outPointerToPixelData != NULL);
                    *outPointerToPixelData = pixel_storage;

                    // Set color table
                    SDL_SetPaletteColors(s->format->palette, colors, 0, 256);
                    
                    // Set transparent pixel (color #0)
                    SDL_SetColorKey(s, SDL_TRUE, 0);
                }
                
	} else {
		// Row-order shape, we can directly create a surface from it
		if (collection->type == _wall_collection)
		{
			s = SDL_CreateRGBSurfaceFrom(bitmap->row_addresses[0], bitmap->height, bitmap->width, 8, bitmap->bytes_per_row, 0, 0, 0, 0);
		}
		else
		{
			s = SDL_CreateRGBSurfaceFrom(bitmap->row_addresses[0], bitmap->width, bitmap->height, 8, bitmap->bytes_per_row, 0, 0, 0, 0);
		}
                // ZZZ: caller should not dispose of any additional data - just free the surface.
                if(outPointerToPixelData != NULL)
                    *outPointerToPixelData = NULL;

                if(s != NULL) {
                    // Set color table
                    SDL_SetPaletteColors(s->format->palette, colors, 0, 256);
                }
	}

	return s;
}

static void load_collection_definition(collection_definition* cd, SDL_RWops *p)
{
	cd->version = SDL_ReadBE16(p);
	cd->type = SDL_ReadBE16(p);
	cd->flags = SDL_ReadBE16(p);
	cd->color_count = SDL_ReadBE16(p);
	cd->clut_count = SDL_ReadBE16(p);
	cd->color_table_offset = SDL_ReadBE32(p);
	cd->high_level_shape_count = SDL_ReadBE16(p);
	cd->high_level_shape_offset_table_offset = SDL_ReadBE32(p);
	cd->low_level_shape_count = SDL_ReadBE16(p);
	cd->low_level_shape_offset_table_offset = SDL_ReadBE32(p);
	cd->bitmap_count = SDL_ReadBE16(p);
	cd->bitmap_offset_table_offset = SDL_ReadBE32(p);
	cd->pixels_to_world = SDL_ReadBE16(p);
	SDL_ReadBE32(p); // skip size
	SDL_RWseek(p, 253 * sizeof(int16), SEEK_CUR); // unused

	// resize members
	cd->color_tables.resize(cd->clut_count * cd->color_count);
	cd->high_level_shapes.resize(cd->high_level_shape_count);
	cd->low_level_shapes.resize(cd->low_level_shape_count);
	cd->bitmaps.resize(cd->bitmap_count);

}

static void load_clut(rgb_color_value *r, int count, SDL_RWops *p)
{
	for (int i = 0; i < count; i++, r++) 
	{
		SDL_RWread(p, r, 1, 2);
		r->red = SDL_ReadBE16(p);
		r->green = SDL_ReadBE16(p);
		r->blue = SDL_ReadBE16(p);
	}
}

static void load_high_level_shape(std::vector<uint8>& shape, SDL_RWops *p)
{
	int16 type = SDL_ReadBE16(p);
	int16 flags = SDL_ReadBE16(p);
	char name[HIGH_LEVEL_SHAPE_NAME_LENGTH + 2];
	SDL_RWread(p, name, 1, HIGH_LEVEL_SHAPE_NAME_LENGTH + 2);
	int16 number_of_views = SDL_ReadBE16(p);
	int16 frames_per_view = SDL_ReadBE16(p);

	// Convert low-level shape index list
	int num_views;
	switch (number_of_views) {
	case _unanimated:
	case _animated1:
		num_views = 1;
		break;
	case _animated3to4:
	case _animated4:
		num_views = 4;
		break;
	case _animated3to5:
	case _animated5:
		num_views = 5;
		break;
	case _animated2to8:
	case _animated5to8:
	case _animated8:
		num_views = 8;
		break;
	default:
		num_views = number_of_views;
		break;
	}

	shape.resize(sizeof(high_level_shape_definition) + num_views * frames_per_view * sizeof(int16));
		
	high_level_shape_definition *d = (high_level_shape_definition *) &shape[0];
		
	d->type = type;
	d->flags = flags;
	memcpy(d->name, name, HIGH_LEVEL_SHAPE_NAME_LENGTH + 2);
	d->number_of_views = number_of_views;
	d->frames_per_view = frames_per_view;
	d->ticks_per_frame = SDL_ReadBE16(p);
	d->key_frame = SDL_ReadBE16(p);
	d->transfer_mode = SDL_ReadBE16(p);
	d->transfer_mode_period = SDL_ReadBE16(p);
	d->first_frame_sound = SDL_ReadBE16(p);
	d->key_frame_sound = SDL_ReadBE16(p);
	d->last_frame_sound = SDL_ReadBE16(p);
	d->pixels_to_world = SDL_ReadBE16(p);
	d->loop_frame = SDL_ReadBE16(p);
	SDL_RWseek(p, 14 * sizeof(int16), SEEK_CUR);

	// Convert low-level shape index list
	for (int j = 0; j < num_views * d->frames_per_view; j++) {
		d->low_level_shape_indexes[j] = SDL_ReadBE16(p);
	}
}

static void load_low_level_shape(low_level_shape_definition *d, SDL_RWops *p)
{
	d->flags = SDL_ReadBE16(p);
	d->minimum_light_intensity = SDL_ReadBE32(p);
	d->bitmap_index = SDL_ReadBE16(p);
	d->origin_x = SDL_ReadBE16(p);
	d->origin_y = SDL_ReadBE16(p);
	d->key_x = SDL_ReadBE16(p);
	d->key_y = SDL_ReadBE16(p);
	d->world_left = SDL_ReadBE16(p);
	d->world_right = SDL_ReadBE16(p);
	d->world_top = SDL_ReadBE16(p);
	d->world_bottom = SDL_ReadBE16(p);
	d->world_x0 = SDL_ReadBE16(p);
	d->world_y0 = SDL_ReadBE16(p);
	SDL_RWseek(p, 4 * sizeof(int16), SEEK_CUR);
}

static void convert_m1_rle(std::vector<uint8>& bitmap, int scanlines, int scanline_length, SDL_RWops* p)
{
//	std::vector<uint8> bitmap;
	for (int scanline = 0; scanline < scanlines; ++scanline)
	{
		std::vector<uint8> scanline_data(scanline_length + 1);
		uint8* dst = &scanline_data[0];
		uint8* sentry = &scanline_data[scanline_length];

		while (true)
		{
			int16 opcode = SDL_ReadBE16(p);
			if (opcode > 0)
			{
				assert(dst + opcode <= sentry);
				SDL_RWread(p, dst, opcode, 1);
				dst += opcode;
			}
			else if (opcode < 0)
			{
				assert(dst - opcode <= sentry);
				dst -= opcode;
			}
			else
				break;
		}

		assert (dst == sentry);

		// Find M2/oo-format RLE compression;
		// it needs the first nonblank pixel and the last nonblank one + 1
		int16 first = 0;
		int16 last = 0;
		for (int i = 0; i < scanline_length; ++i)
		{
			if (scanline_data[i] != 0)
			{
				first = i;
				break;
			}
		}

		for (int i = scanline_length - 1; i >= 0; --i)
		{
			if (scanline_data[i] != 0)
			{
				last = i + 1;
				break;
			}
		}

		if (last < first) last = first;

		bitmap.push_back(first >> 8);
		bitmap.push_back(first & 0xff);
		bitmap.push_back(last >> 8);
		bitmap.push_back(last & 0xff);
		bitmap.insert(bitmap.end(), &scanline_data[first], &scanline_data[last]);
	}
}

static void load_bitmap(std::vector<uint8>& bitmap, SDL_RWops *p, int version)
{
	bitmap_definition b;

	// Convert bitmap definition
	b.width = SDL_ReadBE16(p);
	b.height = SDL_ReadBE16(p);
	b.bytes_per_row = SDL_ReadBE16(p);
	b.flags = SDL_ReadBE16(p);
	b.bit_depth = SDL_ReadBE16(p);

	// guess how big to make it
	int rows = (b.flags & _COLUMN_ORDER_BIT) ? b.width : b.height;
	int row_len = (b.flags & _COLUMN_ORDER_BIT) ? b.height : b.width;
		
	SDL_RWseek(p, 16, SEEK_CUR);
		
	// Skip row address pointers
	SDL_RWseek(p, (rows + 1) * sizeof(uint32), SEEK_CUR);

	if (b.bytes_per_row == NONE) 
	{
		if (version == M1_SHAPES_VERSION)
		{
			// make enough room for the definition, then append as we convert RLE
			bitmap.resize(sizeof(bitmap_definition) + rows * sizeof(pixel8*));
		}
		else
		{
			// ugly--figure out how big it's going to be
			
			int32 size = 0;
			for (int j = 0; j < rows; j++) {
				int16 first = SDL_ReadBE16(p);
				int16 last = SDL_ReadBE16(p);
				size += 4;
				SDL_RWseek(p, last - first, SEEK_CUR);
				size += last - first;
			}
			
			bitmap.resize(sizeof(bitmap_definition) + rows * sizeof(pixel8*) + size);
			
			// Now, seek back
			SDL_RWseek(p, -size, SEEK_CUR);
		}
	} 
	else
	{
		bitmap.resize(sizeof(bitmap_definition) + rows * sizeof(pixel8*) + rows * b.bytes_per_row);
	}


	uint8* c = &bitmap[0];
	bitmap_definition *d = (bitmap_definition *) &bitmap[0];
	d->width = b.width;
	d->height = b.height;
	d->bytes_per_row = b.bytes_per_row;
	d->flags = b.flags;
	d->flags &= ~_PATCHED_BIT; // Anvil sets unused flags :( we'll set it later
	d->bit_depth = b.bit_depth;
	c += sizeof(bitmap_definition);

	// Skip row address pointers
	c += rows * sizeof(pixel8 *);

	// Copy bitmap data
	if (d->bytes_per_row == NONE) 
	{
		// RLE format

		if (version == M1_SHAPES_VERSION)
		{
			convert_m1_rle(bitmap, rows, row_len, p);
		}
		else
		{
			for (int j = 0; j < rows; j++) {
				int16 first = SDL_ReadBE16(p);
				int16 last = SDL_ReadBE16(p);
				*(c++) = (uint8)(first >> 8);
				*(c++) = (uint8)(first);
				*(c++) = (uint8)(last >> 8);
				*(c++) = (uint8)(last);
				SDL_RWread(p, c, 1, last - first);
				c += last - first;
			}
		}
	} else {
		SDL_RWread(p, c, d->bytes_per_row, rows);
		c += rows * d->bytes_per_row;
	}

}

static void allocate_shading_tables(short collection_index, bool strip)
{
	collection_header *header = get_collection_header(collection_index);
	// Allocate enough space for this collection's shading tables
	if (strip)
		header->shading_tables = NULL;
	else {
		collection_definition *definition = get_collection_definition(collection_index);
		header->shading_tables = (byte *)malloc(get_shading_table_size(collection_index) * definition->clut_count + shading_table_size * NUMBER_OF_TINT_TABLES);
	}
}

/*
 *  Load collection
 */

static bool load_collection(short collection_index, bool strip)
{
	SDL_RWops* p;
	boost::shared_ptr<SDL_RWops> m1_p; // automatic deallocation
	LoadedResource r;
	int32 src_offset;

	collection_header *header = get_collection_header(collection_index);
	
	if (shapes_file_version == M1_SHAPES_VERSION)
	{
		// Collections are stored in .256 resources
		if (!M1ShapesFile.Get('.', '2', '5', '6', 128 + collection_index, r))
		{
			return false;
		}

		m1_p.reset(SDL_RWFromConstMem(r.GetPointer(), r.GetLength()), SDL_FreeRW);
		p = m1_p.get();
		src_offset = 0;
	}
	else
	{
		// Get offset and length of data in source file from header
		
		if (bit_depth == 8 || header->offset16 == -1) {
			if (header->offset == -1)
			{
				return false;
			}
			src_offset = header->offset;
		} else {
			src_offset = header->offset16;
		}

		p = ShapesFile.GetRWops();
		ShapesFile.SetPosition(0);
		src_offset += SDL_RWtell(p);
	}

	// Read collection definition
	std::unique_ptr<collection_definition> cd(new collection_definition);
	SDL_RWseek(p, src_offset, RW_SEEK_SET);
	load_collection_definition(cd.get(), p);
	header->status &= ~markPATCHED;

	// Convert CLUTS
	if (cd->clut_count && cd->color_count) {
		SDL_RWseek(p, src_offset + cd->color_table_offset, RW_SEEK_SET);
		load_clut(&cd->color_tables[0], cd->clut_count * cd->color_count, p);
	}

	// Convert high-level shape definitions
	if (cd->high_level_shape_count) {
		SDL_RWseek(p, src_offset + cd->high_level_shape_offset_table_offset, RW_SEEK_SET);
		std::vector<uint32> t(cd->high_level_shape_count);
		SDL_RWread(p, &t[0], sizeof(uint32), cd->high_level_shape_count);
		byte_swap_memory(&t[0], _4byte, cd->high_level_shape_count);

		for (int i = 0; i < cd->high_level_shape_count; i++) {
			SDL_RWseek(p, src_offset + t[i], RW_SEEK_SET);
			load_high_level_shape(cd->high_level_shapes[i], p);
		}
	}

	// Convert low-level shape definitions
	if (cd->low_level_shape_count) {
		SDL_RWseek(p, src_offset + cd->low_level_shape_offset_table_offset, RW_SEEK_SET);
		std::vector<uint32> t(cd->low_level_shape_count);
		SDL_RWread(p, &t[0], sizeof(uint32), cd->low_level_shape_count);
		byte_swap_memory(&t[0], _4byte, cd->low_level_shape_count);

		for (int i = 0; i < cd->low_level_shape_count; i++) {
			SDL_RWseek(p, src_offset + t[i], RW_SEEK_SET);
			load_low_level_shape(&cd->low_level_shapes[i], p);
		}
	}

	// Convert bitmap definitions
	if (cd->bitmap_count) {
		SDL_RWseek(p, src_offset + cd->bitmap_offset_table_offset, RW_SEEK_SET);
		std::vector<uint32> t(cd->bitmap_count);
		SDL_RWread(p, &t[0], sizeof(uint32), cd->bitmap_count);
		byte_swap_memory(&t[0], _4byte, cd->bitmap_count);

		for (int i = 0; i < cd->bitmap_count; i++) {
			SDL_RWseek(p, src_offset + t[i], RW_SEEK_SET);
			load_bitmap(cd->bitmaps[i], p, shapes_file_version);
		}
	}

	header->collection = cd.release();
	
	if (strip) {
		//!! don't know what to do
		fprintf(stderr, "Stripped shapes not implemented\n");
		abort();
	}

	allocate_shading_tables(collection_index, strip);
	
	if (header->shading_tables == NULL) {
		delete header->collection;
		header->collection = NULL;
		return false;
	}

	// Everything OK
	return true;
}	
			

/*
 *  Unload collection
 */

static void unload_collection(struct collection_header *header)
{
	assert(header->collection);
	delete header->collection;
	free(header->shading_tables);
	header->collection = NULL;
	header->shading_tables = NULL;
}

#define ENDC_TAG FOUR_CHARS_TO_INT('e', 'n', 'd', 'c')
#define CLDF_TAG FOUR_CHARS_TO_INT('c', 'l', 'd', 'f')
#define HLSH_TAG FOUR_CHARS_TO_INT('h', 'l', 's', 'h')
#define LLSH_TAG FOUR_CHARS_TO_INT('l', 'l', 's', 'h')
#define BMAP_TAG FOUR_CHARS_TO_INT('b', 'm', 'a', 'p')
#define CTAB_TAG FOUR_CHARS_TO_INT('c', 't', 'a', 'b')

std::vector<uint8> shapes_patch;
void set_shapes_patch_data(uint8 *data, size_t length)
{
	if (!length) 
	{
		shapes_patch.clear();
	}
	else
	{
		shapes_patch.resize(length);
		memcpy(&shapes_patch[0], data, length);
	}
}

uint8* get_shapes_patch_data(size_t &length)
{
	length = shapes_patch.size();
	return length ? &shapes_patch[0] : 0;
}

void load_shapes_patch(SDL_RWops *p, bool override_replacements)
{
	std::vector<int16> color_counts(MAXIMUM_COLLECTIONS);
	int32 start = SDL_RWtell(p);
	SDL_RWseek(p, 0, SEEK_END);
	int32 end = SDL_RWtell(p);

	SDL_RWseek(p, start, SEEK_SET);
	
	bool done = false;
	while (!done)
	{
		// is there more data to read?
		if (SDL_RWtell(p) < end)
		{
			int32 collection_index = SDL_ReadBE32(p);
			int32 patch_bit_depth = SDL_ReadBE32(p);

			bool collection_end = false;
			while (!collection_end)
			{
				// read a tag
				int32 tag = SDL_ReadBE32(p);
				if (tag == ENDC_TAG) 
				{
					collection_end = true;
				}
				else if (tag == CLDF_TAG)
				{
					// a collection follows directly
					collection_header *header = get_collection_header(collection_index);
					if (collection_loaded(header) && patch_bit_depth == 8)
					{
						load_collection_definition(header->collection, p);
						color_counts[collection_index] = header->collection->color_count;
						allocate_shading_tables(collection_index, false);
						header->status|=markPATCHED;

					} else {
						// get the color count (it's the only way to skip the CTAB_TAG
						SDL_RWseek(p, 6, SEEK_CUR);
						color_counts[collection_index] = SDL_ReadBE16(p);
						SDL_RWseek(p, 544 - 8, SEEK_CUR);
					}
				} 
				else if (tag == HLSH_TAG)
				{
					collection_definition *cd = get_collection_definition(collection_index);
					int32 high_level_shape_index = SDL_ReadBE32(p);
					int32 size = SDL_ReadBE32(p);
					int32 pos = SDL_RWtell(p);
					if (cd && patch_bit_depth == 8 && high_level_shape_index < cd->high_level_shapes.size())
					{
						load_high_level_shape(cd->high_level_shapes[high_level_shape_index], p);
						SDL_RWseek(p, pos + size, SEEK_SET);
						
					}
					else
					{
						SDL_RWseek(p, size, SEEK_CUR);
					}
				}
				else if (tag == LLSH_TAG)
				{
					collection_definition *cd = get_collection_definition(collection_index);
					int32 low_level_shape_index = SDL_ReadBE32(p);
					if (cd && patch_bit_depth == 8 && low_level_shape_index < cd->low_level_shapes.size())
					{
						load_low_level_shape(&cd->low_level_shapes[low_level_shape_index], p);
					}
					else
					{
						SDL_RWseek(p, 36, SEEK_CUR);
					}
				} 
				else if (tag == BMAP_TAG)
				{
					collection_definition *cd = get_collection_definition(collection_index);
					int32 bitmap_index = SDL_ReadBE32(p);
					int32 size = SDL_ReadBE32(p);
					if (cd && patch_bit_depth == 8 && bitmap_index < cd->bitmaps.size())
					{
						load_bitmap(cd->bitmaps[bitmap_index], p, M2_SHAPES_VERSION);
						if (override_replacements)
						{
							get_bitmap_definition(collection_index, bitmap_index)->flags |= _PATCHED_BIT;
						}
					}
					else
					{
						SDL_RWseek(p, size, SEEK_CUR);
					}
				}
				else if (tag == CTAB_TAG)
				{
					collection_definition *cd = get_collection_definition(collection_index);
					int32 color_table_index = SDL_ReadBE32(p);
					if (cd && patch_bit_depth == 8 && (color_table_index * cd->color_count < cd->color_tables.size())) 
					{
						load_clut(&cd->color_tables[color_table_index], cd->color_count, p);
					}
					else
					{
						SDL_RWseek(p, color_counts[collection_index] * sizeof(rgb_color_value), SEEK_CUR);
					}
				}
				else
				{
					fprintf(stderr, "Unrecognized tag in patch file '%c%c%c%c'\n %x", tag >> 24, tag >> 16, tag >> 8, tag, tag);
				}
			}
					

		} else {
			done = true;
		}
	}

	
}

/* ---------- code */

/* --------- private code */

void initialize_shape_handler()
{
	// M1 uses the resource fork, but M2 and Moo use the data fork

	FileSpecifier File;
	get_default_shapes_spec(File);
	open_shapes_file(File);
	if (!ShapesFile.IsOpen() && !M1ShapesFile.IsOpen())
		alert_bad_extra_file(ShapesFile.GetError());
	else
		atexit(shutdown_shape_handler);
	
	initialize_pixmap_handler();
}

void open_shapes_file(FileSpecifier& File)
{
	bool m1_loaded = false;
	if (File.Open(M1ShapesFile) && M1ShapesFile.Check('.','2','5','6',128))
	{
		shapes_file_version = M1_SHAPES_VERSION;
		m1_loaded = true;
	}
	else
	{
		M1ShapesFile.Close();
	}
	
	if (!m1_loaded && File.Open(ShapesFile))
	{
		shapes_file_version = M2_SHAPES_VERSION;
		// Load the collection headers;
		// need a buffer for the packed data
		int Size = MAXIMUM_COLLECTIONS*SIZEOF_collection_header;
		byte *CollHdrStream = new byte[Size];
		if (!ShapesFile.Read(Size,CollHdrStream))
		{
			ShapesFile.Close();
			delete []CollHdrStream;
			return;
		}
		
		// Unpack them
		uint8 *S = CollHdrStream;
		int Count = MAXIMUM_COLLECTIONS;
		collection_header* ObjPtr = collection_headers;
		
		for (int k = 0; k < Count; k++, ObjPtr++)
		{
			StreamToValue(S,ObjPtr->status);
			StreamToValue(S,ObjPtr->flags);
			
			StreamToValue(S,ObjPtr->offset);
			StreamToValue(S,ObjPtr->length);
			StreamToValue(S,ObjPtr->offset16);
			StreamToValue(S,ObjPtr->length16);
			
			S += 6*2;
			
			ObjPtr->collection = NULL;	// so unloading can work properly
			ObjPtr->shading_tables = NULL;	// so unloading can work properly
		}
		
		assert((S - CollHdrStream) == Count*SIZEOF_collection_header);
		
		delete []CollHdrStream;
		
	}
	set_shapes_images_file(File);
}

static void close_shapes_file(void)
{
	if (shapes_file_version == M1_SHAPES_VERSION)
	{
		M1ShapesFile.Close();
	}
	else
	{
		ShapesFile.Close();
	}
}

static void shutdown_shape_handler(void)
{
	close_shapes_file();
}


static bool collection_loaded(
	struct collection_header *header)
{
	return header->collection ? true : false;
}

bool collection_loaded(short collection_index)
{
	collection_header *header = get_collection_header(collection_index);
	return collection_loaded(header);
}

bool can_load_collection(short collection_index)
{
	if (collection_index >= 0 && collection_index < NUMBER_OF_COLLECTIONS)
	{
		collection_header* header = get_collection_header(collection_index);
		if (header) {
			return (header->offset != -1 || header->offset16 != -1);
		}
	}

	return false;
}

static void lock_collection(
	struct collection_header *header)
{
	// nothing to do
}

static void unlock_collection(
	struct collection_header *header)
{
	// nothing to do
}


void unload_all_collections(
	void)
{
	struct collection_header *header;
	short collection_index;
	
	for (collection_index= 0, header= collection_headers; collection_index<MAXIMUM_COLLECTIONS; ++collection_index, ++header)
	{
		if (collection_loaded(header))
		{
			unload_collection(header);
		}
		OGL_UnloadModelsImages(collection_index);
	}
}

void mark_collection(
	short collection_code,
	bool loading)
{
	if (collection_code!=NONE)
	{
		short collection_index= GET_COLLECTION(collection_code);
	
		assert(collection_index>=0&&collection_index<MAXIMUM_COLLECTIONS);
		collection_headers[collection_index].status|= loading ? markLOAD : markUNLOAD;
	}
}

void strip_collection(
	short collection_code)
{
	if (collection_code!=NONE)
	{
		short collection_index= GET_COLLECTION(collection_code);
	
		assert(collection_index>=0&&collection_index<MAXIMUM_COLLECTIONS);
		collection_headers[collection_index].status|= markSTRIP;
	}
}

/* returns count, doesn’t fill NULL buffer */
short get_shape_descriptors(
	short shape_type,
	shape_descriptor *buffer)
{
	short collection_index, low_level_shape_index;
	short appropriate_type;
	short count;
	
	switch (shape_type)
	{
		case _wall_shape: appropriate_type= _wall_collection; break;
		case _floor_or_ceiling_shape: appropriate_type= _wall_collection; break;
		default:
			assert(false);
			break;
	}

	count= 0;
	for (collection_index=0;collection_index<MAXIMUM_COLLECTIONS;++collection_index)
	{
		struct collection_definition *collection= get_collection_definition(collection_index);
		// Skip over nonexistent collections, frames, and bitmaps.
		if (!collection) continue;
		
		if (collection&&collection->type==appropriate_type)
		{
			for (low_level_shape_index=0;low_level_shape_index<collection->low_level_shape_count;++low_level_shape_index)
			{
				struct low_level_shape_definition *low_level_shape= get_low_level_shape_definition(collection_index, low_level_shape_index);
				if (!low_level_shape) continue;
				struct bitmap_definition *bitmap= get_bitmap_definition(collection_index, low_level_shape->bitmap_index);
				if (!bitmap) continue;
				
				count+= collection->clut_count;
				if (buffer)
				{
					short clut;
				
					for (clut=0;clut<collection->clut_count;++clut)
					{
						*buffer++= BUILD_DESCRIPTOR(BUILD_COLLECTION(collection_index, clut), low_level_shape_index);
					}
				}
			}
		}
	}
	
	return count;
}

void extended_get_shape_bitmap_and_shading_table(
	short collection_code,
	short low_level_shape_index,
	struct bitmap_definition **bitmap,
	void **shading_tables,
	short shading_mode)
{
//	if (collection_code==_collection_marathon_control_panels) collection_code= 30, low_level_shape_index= 0;
	short collection_index= GET_COLLECTION(collection_code);
	short clut_index= GET_COLLECTION_CLUT(collection_code);
	
	// Forget about it if some one managed to call us with the NONE value
	assert(!(clut_index+1 == MAXIMUM_CLUTS_PER_COLLECTION &&
		collection_index+1 == MAXIMUM_COLLECTIONS &&
		low_level_shape_index+1 == MAXIMUM_SHAPES_PER_COLLECTION));
	
	struct low_level_shape_definition *low_level_shape= get_low_level_shape_definition(collection_index, low_level_shape_index);
	// Return NULL pointers for bitmap and shading table if the frame does not exist
	if (!low_level_shape)
	{
		*bitmap = NULL;
		if (shading_tables) *shading_tables = NULL;
		return;
	}
	
	if (bitmap) *bitmap= get_bitmap_definition(collection_index, low_level_shape->bitmap_index);
	if (shading_tables)
	{
		switch (shading_mode)
		{
			case _shading_normal:
				*shading_tables= get_collection_shading_tables(collection_index, clut_index);
				break;
			case _shading_infravision:
				*shading_tables= get_collection_tint_tables(collection_index, 0);
				break;
			
			default:
				assert(false);
				break;
		}
	}
}

struct shape_information_data *extended_get_shape_information(
	short collection_code,
	short low_level_shape_index)
{
if((GET_COLLECTION(collection_code) < 0) || (GET_COLLECTION(collection_code) >= NUMBER_OF_COLLECTIONS)) return NULL;    if(low_level_shape_index < 0) return NULL;
	short collection_index= GET_COLLECTION(collection_code);
	struct low_level_shape_definition *low_level_shape;

	low_level_shape= get_low_level_shape_definition(collection_index, low_level_shape_index);
	return (struct shape_information_data *) low_level_shape;
}

void process_collection_sounds(
	short collection_code,
	void (*process_sound)(short sound_index))
{
	short collection_index= GET_COLLECTION(collection_code);
	struct collection_definition *collection= get_collection_definition(collection_index);
	// Skip over processing unloaded collections and sequences
	if (!collection) return;
	
	short high_level_shape_index;
	
	for (high_level_shape_index= 0; high_level_shape_index<collection->high_level_shape_count; ++high_level_shape_index)
	{
		struct high_level_shape_definition *high_level_shape= get_high_level_shape_definition(collection_index, high_level_shape_index);
		if (!high_level_shape) return;
		
		process_sound(high_level_shape->first_frame_sound);
		process_sound(high_level_shape->key_frame_sound);
		process_sound(high_level_shape->last_frame_sound);
	}
}

struct shape_animation_data *get_shape_animation_data(
	shape_descriptor shape)
{
	short collection_index, high_level_shape_index;
	struct high_level_shape_definition *high_level_shape;

	collection_index= GET_COLLECTION(GET_DESCRIPTOR_COLLECTION(shape));
	high_level_shape_index= GET_DESCRIPTOR_SHAPE(shape);
	high_level_shape= get_high_level_shape_definition(collection_index, high_level_shape_index);
	if (!high_level_shape) return NULL;
	
	return (struct shape_animation_data *) &high_level_shape->number_of_views;
}

void *get_global_shading_table(
	void)
{
	void *shading_table= (void *) NULL;

	switch (bit_depth)
	{
		case 8:
		{
			/* return the last shading_table calculated */
			short collection_index;
		
			for (collection_index=MAXIMUM_COLLECTIONS-1;collection_index>=0;--collection_index)
			{
				struct collection_definition *collection= get_collection_definition(collection_index);
				
				if (collection)
				{
					shading_table= get_collection_shading_tables(collection_index, 0);
					break;
				}
			}
			
			break;
		}
		
		case 16:
			build_global_shading_table16();
			shading_table= global_shading_table16;
			break;
		
		case 32:
			build_global_shading_table32();
			shading_table= global_shading_table32;
			break;
		
		default:
			assert(false);
			break;
	}
	assert(shading_table);
	
	return shading_table;
}

void load_collections(
	bool with_progress_bar,
	bool is_opengl)
{
	struct collection_header *header;
	short collection_index;

	if (with_progress_bar)
	{
//		open_progress_dialog(_loading_collections);
//		draw_progress_bar(0, 2*MAXIMUM_COLLECTIONS);
	}
	precalculate_bit_depth_constants();
	
	free_and_unlock_memory(); /* do our best to get a big, unfragmented heap */
	
	/* first go through our list of shape collections and dispose of any collections which
		were marked for unloading.  at the same time, unlock all those collections which
		will be staying (so the heap can move around) */
	for (collection_index= 0, header= collection_headers; collection_index<MAXIMUM_COLLECTIONS; ++collection_index, ++header)
	{
//		if (with_progress_bar)
//			draw_progress_bar(collection_index, 2*MAXIMUM_COLLECTIONS);
		if (true)
		{
			if (collection_loaded(header))
			{
				unload_collection(header);
			}
			OGL_UnloadModelsImages(collection_index);
			SW_Texture_Extras::instance()->Unload(collection_index);
		}
	}
	
	/* ... then go back through the list of collections and load any that we were asked to */
	for (collection_index= 0, header= collection_headers; collection_index<MAXIMUM_COLLECTIONS; ++collection_index, ++header)
	{
//		if (with_progress_bar)
//			draw_progress_bar(MAXIMUM_COLLECTIONS+collection_index, 2*MAXIMUM_COLLECTIONS);
		/* don’t reload collections which are already in memory, but do lock them */
		if (collection_loaded(header))
		{
			// In case the substitute images had been changed by some level-specific MML...
//			OGL_LoadModelsImages(collection_index);
			lock_collection(header);
		}
		else
		{
			if (header->status&markLOAD)
			{
				/* load and decompress collection */
				if (!load_collection(collection_index, (header->status&markSTRIP) ? true : false))
				{
					if (shapes_file_version != M1_SHAPES_VERSION)
					{
						alert_out_of_memory();
					}
				}
//				OGL_LoadModelsImages(collection_index);
			}
		}
		
		/* clear action flags */
		header->status= markNONE;
		header->flags= 0;
	}

	Plugins::instance()->load_shapes_patches(is_opengl);

	if (shapes_patch.size())
	{
		SDL_RWops *f = SDL_RWFromMem(&shapes_patch[0], shapes_patch.size());
		load_shapes_patch(f, true);
		SDL_RWclose(f);
	}

	/* remap the shapes, recalculate row base addresses, build our new world color table and
		(finally) update the screen to reflect our changes */
	update_color_environment(is_opengl);

	// load software enhancements
	if (!is_opengl) {
		for (collection_index= 0, header= collection_headers; collection_index < MAXIMUM_COLLECTIONS; ++collection_index, ++header)
		{
			if (collection_loaded(header))
			{
				SW_Texture_Extras::instance()->Load(collection_index);
			}
		}
	}
//	if (with_progress_bar)
//		close_progress_dialog();
}

#ifdef HAVE_OPENGL

int count_replacement_collections()
{
	int total_replacements = 0;
	short collection_index;
	struct collection_header *header;
	for (collection_index = 0, header = collection_headers; collection_index < MAXIMUM_COLLECTIONS; ++collection_index, ++header)
	{
		if (collection_loaded(header))
		{
			total_replacements += OGL_CountModelsImages(collection_index);
		}
	}

	return total_replacements;
}

void load_replacement_collections()
{
	struct collection_header *header;
	short collection_index;

	for (collection_index= 0, header= collection_headers; collection_index < MAXIMUM_COLLECTIONS; ++collection_index, ++header)
	{
		if (collection_loaded(header))
		{
			OGL_LoadModelsImages(collection_index);
		}
	}
}

#endif
		
/* ---------- private code */

static void precalculate_bit_depth_constants(
	void)
{
	switch (bit_depth)
	{
		case 8:
			number_of_shading_tables= 32;
			shading_table_fractional_bits= 5;
//			next_shading_table_shift= 8;
			shading_table_size= PIXEL8_MAXIMUM_COLORS*sizeof(pixel8);
			break;
		case 16:
			number_of_shading_tables= 64;
			shading_table_fractional_bits= 6;
//			next_shading_table_shift= 9;
			shading_table_size= PIXEL8_MAXIMUM_COLORS*sizeof(pixel16);
			break;
		case 32:
			number_of_shading_tables= 256;
			shading_table_fractional_bits= 8;
//			next_shading_table_shift= 10;
			shading_table_size= PIXEL8_MAXIMUM_COLORS*sizeof(pixel32);
			break;
	}
}

/* given a list of RGBColors, find out which one, if any, match the given color.  if there
	aren’t any matches, add a new entry and return that index. */
static short find_or_add_color(
	struct rgb_color_value *color,
	struct rgb_color_value *colors,
	short *color_count, 
	bool update_flags = true)
{
	short i;
	
	// LP addition: save initial color-table pointer, just in case we overflow
	struct rgb_color_value *colors_saved = colors;
	
	// = 1 to skip the transparent color
	for (i= 1, colors+= 1; i<*color_count; ++i, ++colors)
	{
		if (colors->red==color->red && colors->green==color->green && colors->blue==color->blue)
		{
			if (update_flags) colors->flags= color->flags;
			return i;
		}
	}
	
	// LP change: added a fallback strategy; if there were too many colors,
	// then find the closest one
	if (*color_count >= PIXEL8_MAXIMUM_COLORS)
	{
		// Set up minimum distance, its index
		// Strictly speaking, the distance squared, since that will be
		// what we will be calculating.
		// The color values are data type "word", which is unsigned short;
		// this explains the initial choice of minimum value --
		// as greater than any possible such value.
		double MinDiffSq = 3*double(65536)*double(65536);
		short MinIndx = 0;
		
		// Rescan
		colors = colors_saved;
		for (i= 1, colors+= 1; i<*color_count; ++i, ++colors)
		{
			double RedDiff = double(color->red) - double(colors->red);
			double GreenDiff = double(color->green) - double(colors->green);
			double BlueDiff = double(color->blue) - double(colors->blue);
			double DiffSq = RedDiff*RedDiff + GreenDiff*GreenDiff + BlueDiff*BlueDiff;
			if (DiffSq < MinDiffSq)
			{
				MinIndx = i;
				MinDiffSq = DiffSq;
			}
		}
		return MinIndx;
	}
	
	// assert(*color_count<PIXEL8_MAXIMUM_COLORS);
	*colors= *color;
	
	return (*color_count)++;
}

static void update_color_environment(
	bool is_opengl)
{
	short color_count;
	short collection_index;
	short bitmap_index;
	
	pixel8 remapping_table[PIXEL8_MAXIMUM_COLORS];
	struct rgb_color_value colors[PIXEL8_MAXIMUM_COLORS];

	memset(remapping_table, 0, PIXEL8_MAXIMUM_COLORS*sizeof(pixel8));

	// dummy color to hold the first index (zero) for transparent pixels
	colors[0].red= colors[0].green= colors[0].blue= 65535;
	colors[0].flags= colors[0].value= 0;
	color_count= 1;

	/* loop through all collections, only paying attention to the loaded ones.  we’re
		depending on finding the gray run (white to black) first; so it’s the responsibility
		of the lowest numbered loaded collection to give us this */
	for (collection_index=0;collection_index<MAXIMUM_COLLECTIONS;++collection_index)
	{
		struct collection_definition *collection= get_collection_definition(collection_index);

//		dprintf("collection #%d", collection_index);
		
		if (collection && collection->bitmap_count)
		{
			struct rgb_color_value *primary_colors= get_collection_colors(collection_index, 0)+NUMBER_OF_PRIVATE_COLORS;
			assert(primary_colors);
			short color_index, clut_index;

//			if (collection_index==15) dprintf("primary clut %p", primary_colors);
//			dprintf("primary clut %d entries;dm #%d #%d", collection->color_count, primary_colors, collection->color_count*sizeof(ColorSpec));

			/* add the colors from this collection’s primary color table to the aggregate color
				table and build the remapping table */
			for (color_index=0;color_index<collection->color_count-NUMBER_OF_PRIVATE_COLORS;++color_index)
			{
				primary_colors[color_index].value= remapping_table[primary_colors[color_index].value]= 
					find_or_add_color(&primary_colors[color_index], colors, &color_count);
			}
			
			/* then remap the collection and recalculate the base addresses of each bitmap */
			for (bitmap_index= 0; bitmap_index<collection->bitmap_count; ++bitmap_index)
			{
				struct bitmap_definition *bitmap= get_bitmap_definition(collection_index, bitmap_index);
				assert(bitmap);
				
				/* calculate row base addresses ... */
				bitmap->row_addresses[0]= calculate_bitmap_origin(bitmap);
				precalculate_bitmap_row_addresses(bitmap);

				/* ... and remap it */
				remap_bitmap(bitmap, remapping_table);
			}
			
			/* build a shading table for each clut in this collection */
			for (clut_index= 0; clut_index<collection->clut_count; ++clut_index)
			{
				void *primary_shading_table= get_collection_shading_tables(collection_index, 0);
				short collection_bit_depth= collection->type==_interface_collection ? 8 : bit_depth;

				if (clut_index)
				{
					struct rgb_color_value *alternate_colors= get_collection_colors(collection_index, clut_index)+NUMBER_OF_PRIVATE_COLORS;
					assert(alternate_colors);
					void *alternate_shading_table= get_collection_shading_tables(collection_index, clut_index);
					pixel8 shading_remapping_table[PIXEL8_MAXIMUM_COLORS];
					
					memset(shading_remapping_table, 0, PIXEL8_MAXIMUM_COLORS*sizeof(pixel8));
					
//					dprintf("alternate clut %d entries;dm #%d #%d", collection->color_count, alternate_colors, collection->color_count*sizeof(ColorSpec));
					
					/* build a remapping table for the primary shading table which we can use to
						calculate this alternate shading table */
					for (color_index= 0; color_index<PIXEL8_MAXIMUM_COLORS; ++color_index) shading_remapping_table[color_index]= static_cast<pixel8>(color_index);
					for (color_index= 0; color_index<collection->color_count-NUMBER_OF_PRIVATE_COLORS; ++color_index)
					{
						shading_remapping_table[find_or_add_color(&primary_colors[color_index], colors, &color_count, false)]= 
							find_or_add_color(&alternate_colors[color_index], colors, &color_count);
					}
//					shading_remapping_table[iBLACK]= iBLACK; /* make iBLACK==>iBLACK remapping explicit */

					switch (collection_bit_depth)
					{
						case 8:
							/* duplicate the primary shading table and remap it */
							memcpy(alternate_shading_table, primary_shading_table, get_shading_table_size(collection_index));
							map_bytes((unsigned char *)alternate_shading_table, shading_remapping_table, get_shading_table_size(collection_index));
							break;
						
						case 16:
							build_shading_tables16(colors, color_count, (pixel16 *)alternate_shading_table, shading_remapping_table, is_opengl); break;
							break;
						
						case 32:
							build_shading_tables32(colors, color_count, (pixel32 *)alternate_shading_table, shading_remapping_table, is_opengl); break;
							break;
						
						default:
							assert(false);
							break;
					}
				}
				else
				{
					/* build the primary shading table */
					switch (collection_bit_depth)
					{
					case 8: build_shading_tables8(colors, color_count, (unsigned char *)primary_shading_table); break;
					case 16: build_shading_tables16(colors, color_count, (pixel16 *)primary_shading_table, (byte *) NULL, is_opengl); break;
					case 32: build_shading_tables32(colors, color_count,  (pixel32 *)primary_shading_table, (byte *) NULL, is_opengl); break;
						default:
							assert(false);
							break;
					}
				}
			}
			
			build_collection_tinting_table(colors, color_count, collection_index, is_opengl);
			
			/* 8-bit interface, non-8-bit main window; remember interface CLUT separately */
			if (collection_index==_collection_interface && interface_bit_depth==8 && bit_depth!=interface_bit_depth) _change_clut(change_interface_clut, colors, color_count);
			
			/* if we’re not in 8-bit, we don’t have to carry our colors over into the next collection */
			if (bit_depth!=8) color_count= 1;
		}
	}

#ifdef DEBUG
//	dump_colors(colors, color_count);
#endif

	/* change the screen clut and rebuild our shading tables */
	_change_clut(change_screen_clut, colors, color_count);
}

static void _change_clut(
	void (*change_clut_proc)(struct color_table *color_table),
	struct rgb_color_value *colors,
	short color_count)
{
	struct color_table color_table;
	struct rgb_color *color;
	short i;
	
	color= color_table.colors;
	color_table.color_count= PIXEL8_MAXIMUM_COLORS;
	for (i= 0; i<color_count; ++i, ++color, ++colors)
	{
		*color= *((struct rgb_color *)&colors->red);
	}
	for (i= color_count; i<PIXEL8_MAXIMUM_COLORS; ++i, ++color)
	{
		color->red= color->green= color->blue= 0;
	}
	change_clut_proc(&color_table);
}

#ifndef SCREAMING_METAL
static void build_shading_tables8(
	struct rgb_color_value *colors,
	short color_count,
	pixel8 *shading_tables)
{
	short i;
	short start, count, level, value;
	
	objlist_set(shading_tables, iBLACK, PIXEL8_MAXIMUM_COLORS);
	
	start= 0, count= 0;
	while (get_next_color_run(colors, color_count, &start, &count))
	{
		for (i= 0; i<count; ++i)
		{
			assert(number_of_shading_tables > 1);
			short adjust= start ? 1 : 0;

			for (level= 0; level<number_of_shading_tables; ++level)
			{
				struct rgb_color_value *color= colors + start + i;
				short multiplier= (color->flags&SELF_LUMINESCENT_COLOR_FLAG) ? (level>>1) : level;

				value= i + (multiplier*(count+adjust-i))/(number_of_shading_tables-1);
				if (value>=count) value= iBLACK; else value= start+value;
				shading_tables[PIXEL8_MAXIMUM_COLORS*(number_of_shading_tables-level-1)+start+i]= value;
			}
		}
	}
}
#else
short find_closest_color(
	struct rgb_color_value *color,
	struct rgb_color_value *colors,
	short color_count)
{
	short i;
	int32 closest_delta= INT32_MAX;
	short closest_index= 0;
	
	// = 1 to skip the transparent color
	for (i= 1, colors+= 1; i<color_count; ++i, ++colors)
	{
		int32 delta= (int32)ABS(colors->red-color->red) +
			(int32)ABS(colors->green-color->green) +
			(int32)ABS(colors->blue-color->blue);
		
		if (delta<closest_delta) closest_index= i, closest_delta= delta;
	}

	return closest_index;
}

static void build_shading_tables8(
	struct rgb_color_value *colors,
	short color_count,
	pixel8 *shading_tables)
{
	short i;
	short start, count, level;
	
	objlist_set(shading_tables, iBLACK, PIXEL8_MAXIMUM_COLORS);
	
	start= 0, count= 0;
	while (get_next_color_run(colors, color_count, &start, &count))
	{
		for (i= 0; i<count; ++i)
		{
			for (level= 0; level<number_of_shading_tables; ++level)
			{
				struct rgb_color_value *color= colors + start + i;
				rgb_color_value result;
				
				result.red= (color->red*level)/(number_of_shading_tables-1);
				result.green= (color->green*level)/(number_of_shading_tables-1);
				result.blue= (color->blue*level)/(number_of_shading_tables-1);
				shading_tables[PIXEL8_MAXIMUM_COLORS*level+start+i]=
					find_closest_color(&result, colors, color_count);
			}
		}
	}
}
#endif

static void build_shading_tables16(
	struct rgb_color_value *colors,
	short color_count,
	pixel16 *shading_tables,
	byte *remapping_table,
	bool is_opengl)
{
	short i;
	short start, count, level;
	
	objlist_set(shading_tables, 0, PIXEL8_MAXIMUM_COLORS);

	SDL_PixelFormat *fmt = &pixel_format_16;
	
	start= 0, count= 0;
	while (get_next_color_run(colors, color_count, &start, &count))
	{
		for (i=0;i<count;++i)
		{
			assert(number_of_shading_tables > 1);
			for (level= 0; level<number_of_shading_tables; ++level)
			{
				struct rgb_color_value *color= colors + (remapping_table ? remapping_table[start+i] : (start+i));
				short multiplier= (color->flags&SELF_LUMINESCENT_COLOR_FLAG) ? ((number_of_shading_tables>>1)+(level>>1)) : level;
				if (!is_opengl)
					// Find optimal pixel value for video display
					shading_tables[PIXEL8_MAXIMUM_COLORS*level+start+i]= 
						SDL_MapRGB(fmt,
						           ((color->red * multiplier) / (number_of_shading_tables-1)) >> 8,
						           ((color->green * multiplier) / (number_of_shading_tables-1)) >> 8,
						           ((color->blue * multiplier) / (number_of_shading_tables-1)) >> 8);
				else
				// Mac xRGB 1555 pixel format
				shading_tables[PIXEL8_MAXIMUM_COLORS*level+start+i]= 
					RGBCOLOR_TO_PIXEL16((color->red*multiplier)/(number_of_shading_tables-1),
						(color->green*multiplier)/(number_of_shading_tables-1),
						(color->blue*multiplier)/(number_of_shading_tables-1));
			}
		}
	}
}

static void build_shading_tables32(
	struct rgb_color_value *colors,
	short color_count,
	pixel32 *shading_tables,
	byte *remapping_table, 
	bool is_opengl)
{
	short i;
	short start, count, level;
	
	objlist_set(shading_tables, 0, PIXEL8_MAXIMUM_COLORS);
	
	SDL_PixelFormat *fmt = &pixel_format_32;

	start= 0, count= 0;
	while (get_next_color_run(colors, color_count, &start, &count))
	{
		for (i= 0; i<count; ++i)
		{
			assert(number_of_shading_tables > 1);
			for (level= 0; level<number_of_shading_tables; ++level)
			{
				struct rgb_color_value *color= colors + (remapping_table ? remapping_table[start+i] : (start+i));
				short multiplier= (color->flags&SELF_LUMINESCENT_COLOR_FLAG) ? ((number_of_shading_tables>>1)+(level>>1)) : level;
				
				if (!is_opengl)
					// Find optimal pixel value for video display
					shading_tables[PIXEL8_MAXIMUM_COLORS*level+start+i]= 
						SDL_MapRGB(fmt,
						           ((color->red * multiplier) / (number_of_shading_tables-1)) >> 8,
						           ((color->green * multiplier) / (number_of_shading_tables-1)) >> 8,
						           ((color->blue * multiplier) / (number_of_shading_tables-1)) >> 8);
				else
				// Mac xRGB 8888 pixel format
				shading_tables[PIXEL8_MAXIMUM_COLORS*level+start+i]= 
					RGBCOLOR_TO_PIXEL32((color->red*multiplier)/(number_of_shading_tables-1),
						(color->green*multiplier)/(number_of_shading_tables-1),
						(color->blue*multiplier)/(number_of_shading_tables-1));
			}
		}
	}
}

static void build_global_shading_table16(
	void)
{
	if (!global_shading_table16)
	{
		short value, shading_table;
		pixel16 *write;

		SDL_PixelFormat *fmt = &pixel_format_16;
		
		global_shading_table16= (pixel16 *) malloc(sizeof(pixel16)*number_of_shading_tables*NUMBER_OF_COLOR_COMPONENTS*(PIXEL16_MAXIMUM_COMPONENT+1));
		assert(global_shading_table16);
		
		write= global_shading_table16;
		for (shading_table= 0; shading_table<number_of_shading_tables; ++shading_table)
		{
			// Under SDL, the components may have different widths and different shifts
			int shift = fmt->Rshift + (3 - fmt->Rloss);
			for (value=0;value<=PIXEL16_MAXIMUM_COMPONENT;++value)
				*write++ = ((value*(shading_table))/(number_of_shading_tables-1))<<shift;
			shift = fmt->Gshift + (3 - fmt->Gloss);
			for (value=0;value<=PIXEL16_MAXIMUM_COMPONENT;++value)
				*write++ = ((value*(shading_table))/(number_of_shading_tables-1))<<shift;
			shift = fmt->Bshift + (3 - fmt->Bloss);
			for (value=0;value<=PIXEL16_MAXIMUM_COMPONENT;++value)
				*write++ = ((value*(shading_table))/(number_of_shading_tables-1))<<shift;
		}
	}
}

static void build_global_shading_table32(
	void)
{
	if (!global_shading_table32)
	{
		short value, shading_table;
		pixel32 *write;
		
		SDL_PixelFormat *fmt = &pixel_format_32;

		global_shading_table32= (pixel32 *) malloc(sizeof(pixel32)*number_of_shading_tables*NUMBER_OF_COLOR_COMPONENTS*(PIXEL32_MAXIMUM_COMPONENT+1));
		assert(global_shading_table32);
		
		write= global_shading_table32;
		for (shading_table= 0; shading_table<number_of_shading_tables; ++shading_table)
		{
			// Under SDL, the components may have different widths and different shifts
			int shift = fmt->Rshift - fmt->Rloss;
			for (value=0;value<=PIXEL32_MAXIMUM_COMPONENT;++value)
				*write++ = ((value*(shading_table))/(number_of_shading_tables-1))<<shift;
			shift = fmt->Gshift - fmt->Gloss;
			for (value=0;value<=PIXEL32_MAXIMUM_COMPONENT;++value)
				*write++ = ((value*(shading_table))/(number_of_shading_tables-1))<<shift;
			shift = fmt->Bshift - fmt->Bloss;
			for (value=0;value<=PIXEL32_MAXIMUM_COMPONENT;++value)
				*write++ = ((value*(shading_table))/(number_of_shading_tables-1))<<shift;
		}
	}
}

static bool get_next_color_run(
	struct rgb_color_value *colors,
	short color_count,
	short *start,
	short *count)
{
	bool not_done= false;
	struct rgb_color_value last_color;
	
	if (*start+*count<color_count)
	{
		*start+= *count;
		for (*count=0;*start+*count<color_count;*count+= 1)
		{
			if (*count)
			{
				if (new_color_run(colors+*start+*count, &last_color))
				{
					break;
				}
			}
			last_color= colors[*start+*count];
		}
		
		not_done= true;
	}
	
	return not_done;
}

static bool new_color_run(
	struct rgb_color_value *_new,
	struct rgb_color_value *last)
{
	if ((int32)last->red+(int32)last->green+(int32)last->blue<(int32)_new->red+(int32)_new->green+(int32)_new->blue)
	{
		return true;
	}
	else
	{
		return false;
	}
}

static int32 get_shading_table_size(
	short collection_code)
{
	int32 size;
	
	switch (bit_depth)
	{
		case 8: size= number_of_shading_tables*shading_table_size; break;
		case 16: size= number_of_shading_tables*shading_table_size; break;
		case 32: size= number_of_shading_tables*shading_table_size; break;
		default:
			assert(false);
			break;
	}
	
	return size;
}

/* --------- light enhancement goggles */

enum /* collection tint colors */
{
	_tint_collection_red,
	_tint_collection_green,
	_tint_collection_blue,
	_tint_collection_yellow,
	NUMBER_OF_TINT_COLORS
};

struct tint_color8_data
{
	short start, count;
};

static struct rgb_color tint_colors16[NUMBER_OF_TINT_COLORS]=
{
	{65535, 0, 0},
	{0, 65535, 0},
	{0, 0, 65535},
	{65535, 65535, 0},
};

static struct tint_color8_data tint_colors8[NUMBER_OF_TINT_COLORS]=
{
	{45, 13},
	{32, 13},
	{96, 13},
	{83, 13},
};


// LP addition to make it more generic;
// the ultimate in this would be to make every collection
// have its own infravision tint.
static short CollectionTints[NUMBER_OF_COLLECTIONS] = 
{
	// Interface
	NONE,
	// Weapons in hand
	_tint_collection_yellow,
	// Juggernaut, tick
	_tint_collection_red,
	_tint_collection_red,
	// Explosion effects
	_tint_collection_yellow,
	// Hunter	
	_tint_collection_red,
	// Player
	_tint_collection_yellow,
	// Items
	_tint_collection_green,
	// Trooper, Pfhor, S'pht'Kr, F'lickta
	_tint_collection_red,
	_tint_collection_red,
	_tint_collection_red,
	_tint_collection_red,
	// Bob and VacBobs
	_tint_collection_yellow,
	_tint_collection_yellow,
	// Enforcer, Drone
	_tint_collection_red,
	_tint_collection_red,
	// S'pht
	_tint_collection_blue,
	// Walls
	_tint_collection_blue,
	_tint_collection_blue,
	_tint_collection_blue,
	_tint_collection_blue,
	_tint_collection_blue,
	// Scenery
	_tint_collection_blue,
	_tint_collection_blue,
	_tint_collection_blue,
	_tint_collection_blue,
	_tint_collection_blue,
	// Landscape
	_tint_collection_blue,
	_tint_collection_blue,
	_tint_collection_blue,
	_tint_collection_blue,
	// Cyborg
	_tint_collection_red
};


static void build_collection_tinting_table(
	struct rgb_color_value *colors,
	short color_count,
	short collection_index,
	bool is_opengl)
{
	struct collection_definition *collection= get_collection_definition(collection_index);
	if (!collection) return;
	
	void *tint_table= get_collection_tint_tables(collection_index, 0);
	short tint_color;

	/* get the tint color */
	// LP change: look up a table
	tint_color = CollectionTints[collection_index];
	// Idiot-proofing:
	if (tint_color >= NUMBER_OF_TINT_COLORS)
		tint_color = NONE;
	else
		tint_color = MAX(tint_color,NONE);

	/* build the tint table */	
	if (tint_color!=NONE)
	{
		// LP addition: OpenGL support
		rgb_color &Color = tint_colors16[tint_color];
#ifdef HAVE_OPENGL
		OGL_SetInfravisionTint(collection_index,true,Color.red/65535.0F,Color.green/65535.0F,Color.blue/65535.0F);
#endif
		switch (bit_depth)
		{
			case 8:
				build_tinting_table8(colors, color_count, (unsigned char *)tint_table, tint_colors8[tint_color].start, tint_colors8[tint_color].count);
				break;
			case 16:
				build_tinting_table16(colors, color_count, (pixel16 *)tint_table, tint_colors16+tint_color);
				break;
			case 32:
				build_tinting_table32(colors, color_count, (pixel32 *)tint_table, tint_colors16+tint_color, is_opengl);
				break;
		}
	}
	else
	{
		// LP addition: OpenGL support
#ifdef HAVE_OPENGL
		OGL_SetInfravisionTint(collection_index,false,1,1,1);
#endif
	}
}

static void build_tinting_table8(
	struct rgb_color_value *colors,
	short color_count,
	pixel8 *tint_table,
	short tint_start,
	short tint_count)
{
	short start, count;
	
	start= count= 0;
	while (get_next_color_run(colors, color_count, &start, &count))
	{
		short i;

		for (i=0; i<count; ++i)
		{
			short adjust= start ? 0 : 1;
			short value= (i*(tint_count+adjust))/count;
			
			value= (value>=tint_count) ? iBLACK : tint_start + value;
			tint_table[start+i]= value;
		}
	}
}

// Return intensity(base)*tint, with M2-style rounding behavior
static rgb_color m2_apply_tint(rgb_color_value base, rgb_color tint)
{
	const uint16 base_mag = (int32{base.red} + base.green + base.blue) / 3;
	auto scale = [base_mag](uint16 comp) { return uint16(int32(1LL*base_mag*comp) / 65535); };
	return {scale(tint.red), scale(tint.green), scale(tint.blue)};
}

static void build_tinting_table16(
	struct rgb_color_value *colors,
	short color_count,
	pixel16 *tint_table,
	struct rgb_color *tint_color)
{
	short i;

	SDL_PixelFormat *fmt = &pixel_format_16;

	for (i= 0; i<color_count; ++i, ++colors)
	{
		const rgb_color tinted_color = m2_apply_tint(*colors, *tint_color);
		
		// Find optimal pixel value for video display
		*tint_table++ = SDL_MapRGB(fmt, tinted_color.red >> 8, tinted_color.green >> 8, tinted_color.blue >> 8);
	}
}

static void build_tinting_table32(
	struct rgb_color_value *colors,
	short color_count,
	pixel32 *tint_table,
	struct rgb_color *tint_color,
	bool is_opengl)
{
	short i;

	SDL_PixelFormat *fmt = &pixel_format_32;

	for (i= 0; i<color_count; ++i, ++colors)
	{
		const rgb_color tinted_color = m2_apply_tint(*colors, *tint_color);
		
		// Find optimal pixel value for video display
		if (!is_opengl)
			*tint_table++ = SDL_MapRGB(fmt, tinted_color.red >> 8, tinted_color.green >> 8, tinted_color.blue >> 8);
		else
		// Mac xRGB 8888 pixel format
			*tint_table++ = RGBCOLOR_TO_PIXEL32(tinted_color.red, tinted_color.green, tinted_color.blue);
	}
}


/* ---------- collection accessors */
// Some originally from shapes_macintosh.c

static struct collection_header *get_collection_header(
	short collection_index)
{
	// This one is intended to bomb because collection indices can only be from 1 to 31,
	// short of drastic changes in how collection indices are specified (a bigger structure
	// than shape_descriptor, for example).
	collection_header *header = GetMemberWithBounds(collection_headers,collection_index,MAXIMUM_COLLECTIONS);
	vassert(header,csprintf(temporary,"Collection index out of range: %d",collection_index));
	
	return header;
	
	/*
	assert(collection_index>=0 && collection_index<MAXIMUM_COLLECTIONS);
	
	return collection_headers + collection_index;
	*/
}

/*static*/ struct collection_definition *get_collection_definition(
	short collection_index)
{
	return get_collection_header(collection_index)->collection;
}

static struct rgb_color_value *get_collection_colors(
	short collection_index,
	short clut_number)
{
	collection_definition *definition = get_collection_definition(collection_index);
	if (!definition) return NULL;
	if (!(clut_number >= 0 && clut_number < definition->clut_count))
		return NULL;
	
	return &definition->color_tables[clut_number * definition->color_count];
}

struct rgb_color_value *get_collection_colors(
	short collection_index,
	short clut_number,
	int &num_colors)
{
	collection_definition *definition = get_collection_definition(collection_index);
	if (!definition) return NULL;
	if (!(clut_number >=0 && clut_number < definition->clut_count)) return NULL;
	num_colors = definition->color_count;
	return &definition->color_tables[clut_number * definition->color_count];
}

static struct high_level_shape_definition *get_high_level_shape_definition(
	short collection_index,
	short high_level_shape_index)
{
	struct collection_definition *definition = get_collection_definition(collection_index);
	if (!definition) return NULL;

	if (!(high_level_shape_index >= 0 && high_level_shape_index<definition->high_level_shapes.size()))
		return NULL;
	
	if (definition->high_level_shapes[high_level_shape_index].empty())
		return NULL;

	return (high_level_shape_definition *) &definition->high_level_shapes[high_level_shape_index][0];
}

struct low_level_shape_definition *get_low_level_shape_definition(
	short collection_index,
	short low_level_shape_index)
{
	collection_definition *definition = get_collection_definition(collection_index);
	if (!definition) return NULL;
	if (low_level_shape_index >= 0 && low_level_shape_index < definition->low_level_shapes.size())
	{
		return &definition->low_level_shapes[low_level_shape_index];
	}
	else
		return NULL;
}

static struct bitmap_definition *get_bitmap_definition(
	short collection_index,
	short bitmap_index)
{
	collection_definition *definition = get_collection_definition(collection_index);
	if (!definition) return NULL;
	if (!(bitmap_index >= 0 && bitmap_index < definition->bitmaps.size()))
		return NULL;
	
	if (definition->bitmaps[bitmap_index].empty())
		return NULL;

	return (bitmap_definition *) &definition->bitmaps[bitmap_index][0];
}

static void *get_collection_shading_tables(
	short collection_index,
	short clut_index)
{
	void *shading_tables= get_collection_header(collection_index)->shading_tables;

	shading_tables = (uint8 *)shading_tables + clut_index*get_shading_table_size(collection_index);
	
	return shading_tables;
}

static void *get_collection_tint_tables(
	short collection_index,
	short tint_index)
{
	struct collection_definition *definition= get_collection_definition(collection_index);
	if (!definition) return NULL;
	
	void *tint_table= get_collection_header(collection_index)->shading_tables;

	tint_table = (uint8 *)tint_table + get_shading_table_size(collection_index)*definition->clut_count + shading_table_size*tint_index;
	
	return tint_table;
}

// LP additions:

// Whether or not collection is present
bool is_collection_present(short collection_index)
{
	collection_header *CollHeader = get_collection_header(collection_index);
	if (!CollHeader) return false;
	return collection_loaded(CollHeader);
}

// Number of texture frames in a collection (good for wall-texture error checking)
short get_number_of_collection_frames(short collection_index)
{
	struct collection_definition *Collection = get_collection_definition(collection_index);
	if (!Collection) return 0;
	return Collection->low_level_shape_count;
}

// Number of bitmaps in a collection (good for allocating texture information for OpenGL)
short get_number_of_collection_bitmaps(short collection_index)
{
	struct collection_definition *Collection = get_collection_definition(collection_index);
	if (!Collection) return 0;
	return Collection->bitmap_count;
}

// Which bitmap index for a frame (good for OpenGL texture rendering)
short get_bitmap_index(short collection_index, short low_level_shape_index)
{
	struct low_level_shape_definition *low_level_shape= get_low_level_shape_definition(collection_index, low_level_shape_index);
	if (!low_level_shape) return NONE;
	return low_level_shape->bitmap_index;
}


// XML elements for parsing infravision specification
short *OriginalCollectionTints = NULL;
struct rgb_color *original_tint_colors16 = NULL;

void reset_mml_infravision()
{
	if (original_tint_colors16) {
		for (int i = 0; i < NUMBER_OF_TINT_COLORS; i++)
			tint_colors16[i] = original_tint_colors16[i];
		free(original_tint_colors16);
		original_tint_colors16 = NULL;
	}

	if (OriginalCollectionTints) {
		for (int i = 0; i < NUMBER_OF_COLLECTIONS; i++)
			CollectionTints[i] = OriginalCollectionTints[i];
		free(OriginalCollectionTints);
		OriginalCollectionTints = NULL;
	}
}

void parse_mml_infravision(const InfoTree& root)
{
	// back up old values first
	if (!original_tint_colors16) {
		original_tint_colors16 = (struct rgb_color *) malloc(sizeof(struct rgb_color) * NUMBER_OF_TINT_COLORS);
		assert(original_tint_colors16);
		for (int i = 0; i < NUMBER_OF_TINT_COLORS; i++)
			original_tint_colors16[i] = tint_colors16[i];
	}
	
	if (!OriginalCollectionTints) {
		OriginalCollectionTints = (short *) malloc(sizeof(short) * NUMBER_OF_COLLECTIONS);
		assert(OriginalCollectionTints);
		for (int i = 0; i < NUMBER_OF_COLLECTIONS; i++)
			OriginalCollectionTints[i] = CollectionTints[i];
	}

	BOOST_FOREACH(InfoTree color, root.children_named("color"))
	{
		int16 index;
		if (!color.read_indexed("index", index, NUMBER_OF_TINT_COLORS))
			continue;
		color.read_color(tint_colors16[index]);
	}
	
	BOOST_FOREACH(InfoTree assign, root.children_named("assign"))
	{
		int16 coll, color;
		if (!assign.read_indexed("coll", coll, NUMBER_OF_COLLECTIONS) ||
			!assign.read_indexed("color", color, NUMBER_OF_TINT_COLORS))
			continue;
		CollectionTints[coll] = color;
	}
}
