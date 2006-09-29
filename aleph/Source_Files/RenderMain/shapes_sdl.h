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
 *  shapes_sdl.cpp - Shapes handling, SDL specific stuff (included by shapes.cpp)
 *
 *  Written in 2000 by Christian Bauer
 */

/*
Oct 19, 2000 (Loren Petrich):
	Added graceful degradation in the case of frames or bitmaps not being found;
	get_shape_surface() returns NULL when that happens
        
October 2001 (Woody Zenfell):
        Changes to get_shape_surface() for RLE shapes, shading-table lookups, large
        low-level-shape indices, and quarter-size shapes
        (these for the w_players_in_game2 widget, primarily)
*/

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

		    // ghs: I believe the second case in this if statement
		    // is the way it should always be done; but somehow SDL
		    // screws up when OPENGLBLIT is enabled (big surprise) and
		    // this old behavior seems luckily to work around it

		    // well, ok, ideally the code that builds these tables
		    // should either not use the current video format
		    // (since in the future it may change) or should store
		    // the pixel format it used somewhere so we're sure we've
		    // got the right one
		    if (SDL_GetVideoSurface()->flags & SDL_OPENGLBLIT) {
		      for(int i = 0; i < 256; i++) {
			colors[i].r = RED16(shading_tables[i]) << 3;
			colors[i].g = GREEN16(shading_tables[i]) << 3;
			colors[i].b = BLUE16(shading_tables[i]) << 3;
		      }
		    } else {
		      SDL_PixelFormat *fmt = SDL_GetVideoSurface()->format;
		      for (int i = 0; i < 256; i++) {
			SDL_GetRGB(shading_tables[i], fmt, &colors[i].r, &colors[i].g, &colors[i].b);
		      }
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
            for (int i=0; i<num_colors; i++) {
                    int idx = src_colors[i].value;
                    colors[idx].r = src_colors[i].red >> 8;
                    colors[idx].g = src_colors[i].green >> 8;
                    colors[idx].b = src_colors[i].blue >> 8;
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
                int16	theNumElementsPerRun;
                int16	theDestDataNextRunOffset;
                int16	theDestDataNextElementOffset;

                // Is this row-major or column-major?
                if(bitmap->flags & _COLUMN_ORDER_BIT) {
                    theNumRuns				= bitmap->width;
                    theNumElementsPerRun		= bitmap->height;
                    theDestDataNextRunOffset		= (low_level_shape->flags & _X_MIRRORED_BIT) ? -1 : 1;
                    theDestDataNextElementOffset	= (low_level_shape->flags & _Y_MIRRORED_BIT) ? -bitmap->width : bitmap->width;
                }
                else {
                    theNumRuns				= bitmap->height;
                    theNumElementsPerRun		= bitmap->width;
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
		s = SDL_CreateRGBSurfaceFrom(pixel_storage, image_width, image_height, 8, image_width, 0xff, 0xff, 0xff, 0xff);

                if(s != NULL) {
                    // If caller is not prepared to take this data, it's a coding error.
                    assert(outPointerToPixelData != NULL);
                    *outPointerToPixelData = pixel_storage;

                    // Set color table
                    SDL_SetColors(s, colors, 0, 256);
                    
                    // Set transparent pixel (color #0)
                    SDL_SetColorKey(s, SDL_SRCCOLORKEY, 0);
                }
                
	} else {

		// Row-order shape, we can directly create a surface from it
		s = SDL_CreateRGBSurfaceFrom(bitmap->row_addresses[0], bitmap->width, bitmap->height, 8, bitmap->bytes_per_row, 0xff, 0xff, 0xff, 0xff);
                // ZZZ: caller should not dispose of any additional data - just free the surface.
                if(outPointerToPixelData != NULL)
                    *outPointerToPixelData = NULL;

                if(s != NULL) {
                    // Set color table
                    SDL_SetColors(s, colors, 0, 256);
                }
	}

	return s;
}


/*
 *  Load collection
 */

static bool load_collection(short collection_index, bool strip)
{
	SDL_RWops *p = ShapesFile.GetRWops();	// Source stream
	uint32 *t;								// Offset table pointer

	// Get offset and length of data in source file from header
	collection_header *header = get_collection_header(collection_index);
	long src_offset, src_length;
	if (bit_depth == 8 || header->offset16 == -1) {
		vassert(header->offset != -1, csprintf(temporary, "collection #%d does not exist.", collection_index));
		src_offset = header->offset;
		src_length = header->length;
	} else {
		src_offset = header->offset16;
		src_length = header->length16;
	}

	// Read collection definition
	ShapesFile.SetPosition(src_offset);
	int16 version = SDL_ReadBE16(p);
	int16 type = SDL_ReadBE16(p);
	uint16 flags = SDL_ReadBE16(p);
	int16 color_count = SDL_ReadBE16(p);
	int16 clut_count = SDL_ReadBE16(p);
	int32 color_table_offset = SDL_ReadBE32(p);
	int16 high_level_shape_count = SDL_ReadBE16(p);
	int32 high_level_shape_offset_table_offset = SDL_ReadBE32(p);
	int16 low_level_shape_count = SDL_ReadBE16(p);
	int32 low_level_shape_offset_table_offset = SDL_ReadBE32(p);
	int16 bitmap_count = SDL_ReadBE16(p);
	int32 bitmap_offset_table_offset = SDL_ReadBE32(p);
	int16 pixels_to_world = SDL_ReadBE16(p);
	SDL_ReadBE32(p); // skip size

	// Allocate memory for collection
	int extra_length = 1024 + high_level_shape_count * 4 + low_level_shape_count * 4 + bitmap_count * 2048;
	uint8 *c = (uint8*)malloc(src_length + extra_length);
	if (c == NULL)
		return false;

	// Initialize collection definition
	collection_definition *cd = (collection_definition *)c;
	cd->version = version;
	cd->type = type;
	cd->flags = flags;
	cd->color_count = color_count;
	cd->clut_count = clut_count;
	cd->high_level_shape_count = high_level_shape_count;
	cd->low_level_shape_count = low_level_shape_count;
	cd->bitmap_count = bitmap_count;
	cd->pixels_to_world = pixels_to_world;
//	printf(" index %d, version %d, type %d, %d colors, %d cluts, %d hl, %d ll, %d bitmaps\n", collection_index, version, type, color_count, clut_count, high_level_shape_count, low_level_shape_count, bitmap_count);

	// Set up destination pointer
	size_t dst_offset = 0x220;
#define q (c + dst_offset)

	// Convert CLUTs
	ShapesFile.SetPosition(src_offset + color_table_offset);
	cd->color_table_offset = static_cast<int32>(dst_offset);
	for (int i=0; i<clut_count*color_count; i++) {
		rgb_color_value *r = (rgb_color_value *)q;
		SDL_RWread(p, r, 1, 2);
		r->red = SDL_ReadBE16(p);
		r->green = SDL_ReadBE16(p);
		r->blue = SDL_ReadBE16(p);
		dst_offset += sizeof(rgb_color_value);
	}

	// Convert high-level shape definitions
	ShapesFile.SetPosition(src_offset + high_level_shape_offset_table_offset);
	cd->high_level_shape_offset_table_offset = static_cast<int32>(dst_offset);

	t = (uint32 *)q;	// Offset table
	SDL_RWread(p, t, sizeof(uint32), high_level_shape_count);
	byte_swap_memory(t, _4byte, high_level_shape_count);
	dst_offset += high_level_shape_count * sizeof(uint32);

	for (int i=0; i<high_level_shape_count; i++) {

		// Seek to offset in source file, correct destination offset
		ShapesFile.SetPosition(src_offset + t[i]);
		t[i] = static_cast<int32>(dst_offset);

		// Convert high-level shape definition
		high_level_shape_definition *d = (high_level_shape_definition *)q;
		d->type = SDL_ReadBE16(p);
		d->flags = SDL_ReadBE16(p);
		SDL_RWread(p, d->name, 1, HIGH_LEVEL_SHAPE_NAME_LENGTH + 2);
		d->number_of_views = SDL_ReadBE16(p);
		d->frames_per_view = SDL_ReadBE16(p);
		d->ticks_per_frame = SDL_ReadBE16(p);
		d->key_frame = SDL_ReadBE16(p);
		d->transfer_mode = SDL_ReadBE16(p);
		d->transfer_mode_period = SDL_ReadBE16(p);
		d->first_frame_sound = SDL_ReadBE16(p);
		d->key_frame_sound = SDL_ReadBE16(p);
		d->last_frame_sound = SDL_ReadBE16(p);
		d->pixels_to_world = SDL_ReadBE16(p);
		d->loop_frame = SDL_ReadBE16(p);
		SDL_RWseek(p, 28, SEEK_CUR);

		// Convert low-level shape index list
		int num_views;
		switch (d->number_of_views) {
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
				num_views = d->number_of_views;
				break;
		}
		for (int j=0; j<num_views*d->frames_per_view; j++)
			d->low_level_shape_indexes[j] = SDL_ReadBE16(p);

		dst_offset += sizeof(high_level_shape_definition) + (num_views * d->frames_per_view - 1) * sizeof(int16);
		if (dst_offset & 3)	// Align to 32-bit boundary
			dst_offset += 4 - (dst_offset & 3);
	}

	// Convert low-level shape definitions
	ShapesFile.SetPosition(src_offset + low_level_shape_offset_table_offset);
	cd->low_level_shape_offset_table_offset = static_cast<int32>(dst_offset);

	t = (uint32 *)q;	// Offset table
	SDL_RWread(p, t, sizeof(uint32), low_level_shape_count);
	byte_swap_memory(t, _4byte, low_level_shape_count);
	dst_offset += low_level_shape_count * sizeof(uint32);

	for (int i=0; i<low_level_shape_count; i++) {

		// Seek to offset in source file, correct destination offset
		ShapesFile.SetPosition(src_offset + t[i]);
		t[i] = static_cast<int32>(dst_offset);

		// Convert low-level shape definition
		low_level_shape_definition *d = (low_level_shape_definition *)q;
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
		SDL_RWseek(p, 8, SEEK_CUR);
		dst_offset += sizeof(low_level_shape_definition);
	}

	// Convert bitmap definitions
	ShapesFile.SetPosition(src_offset + bitmap_offset_table_offset);
	cd->bitmap_offset_table_offset = static_cast<int32>(dst_offset);

	t = (uint32 *)q;	// Offset table
	SDL_RWread(p, t, sizeof(uint32), bitmap_count);
	byte_swap_memory(t, _4byte, bitmap_count);
	dst_offset += bitmap_count * sizeof(uint32);
	if (dst_offset & 7)	// Align to 64-bit boundary
		dst_offset += 8 - (dst_offset & 7);

	for (int i=0; i<bitmap_count; i++) {

		// Seek to offset in source file, correct destination offset
		ShapesFile.SetPosition(src_offset + t[i]);
		t[i] = static_cast<int32>(dst_offset);

		// Convert bitmap definition
		bitmap_definition *d = (bitmap_definition *)q;
		d->width = SDL_ReadBE16(p);
		d->height = SDL_ReadBE16(p);
		d->bytes_per_row = SDL_ReadBE16(p);
		d->flags = SDL_ReadBE16(p);
		d->bit_depth = SDL_ReadBE16(p);
		SDL_RWseek(p, 16, SEEK_CUR);
		dst_offset += sizeof(bitmap_definition);

		// Skip row address pointers
		int rows = (d->flags & _COLUMN_ORDER_BIT) ? d->width : d->height;
		SDL_RWseek(p, (rows + 1) * sizeof(uint32), SEEK_CUR);
		dst_offset += rows * sizeof(pixel8 *);

		// Copy bitmap data
		if (d->bytes_per_row == NONE) {
			// RLE format
			for (int j=0; j<rows; j++) {
				int16 first = SDL_ReadBE16(p);
				int16 last = SDL_ReadBE16(p);
				*(c + dst_offset++) = (uint8)(first >> 8);
				*(c + dst_offset++) = (uint8)(first);
				*(c + dst_offset++) = (uint8)(last >> 8);
				*(c + dst_offset++) = (uint8)(last);
				SDL_RWread(p, q, 1, last - first);
				dst_offset += last - first;
			}
		} else {
			// Raw format
			SDL_RWread(p, q, d->bytes_per_row, rows);
			dst_offset += rows * d->bytes_per_row;
		}
		if (dst_offset & 7)	// Align to 64-bit boundary
			dst_offset += 8 - (dst_offset & 7);
	}

	// Set pointer to collection in collection header
	header->collection = cd;
	cd->size = static_cast<int32>(dst_offset);
//	printf(" collection at %p, size %d -> %d\n", cd, src_length, cd->size);
	assert(cd->size <= src_length + extra_length);

	if (strip) {
		//!! don't know what to do
		fprintf(stderr, "Stripped shapes not implemented\n");
		abort();
	}

	// Allocate enough space for this collection's shading tables
	if (strip)
		header->shading_tables = NULL;
	else {
		collection_definition *definition = get_collection_definition(collection_index);
		header->shading_tables = (byte *)malloc(get_shading_table_size(collection_index) * definition->clut_count + shading_table_size * NUMBER_OF_TINT_TABLES);
	}
	if (header->shading_tables == NULL) {
		free(header->collection);
		header->collection = NULL;
		free(c);
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
	free(header->collection);
	free(header->shading_tables);
	header->collection = NULL;
	header->shading_tables = NULL;
}
