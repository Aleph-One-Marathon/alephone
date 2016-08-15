/*
	images.c

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
#include <memory>

#include "interface.h"
#include "shell.h"
#include "images.h"
#include "screen.h"
#include "wad.h"
#include "screen_drawing.h"
#include "Logging.h"

#include "render.h"
#include "OGL_Render.h"
#include "OGL_Blitter.h"
#include "screen_definitions.h"


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

// Global variables
static image_file_t ImagesFile;
static image_file_t ScenarioFile;
static image_file_t ExternalResourcesFile;
static image_file_t ShapesImagesFile;
static image_file_t SoundsImagesFile;

// Prototypes
static void shutdown_images_handler(void);
static void draw_picture(LoadedResource &PictRsrc);


#include <SDL_endian.h>

#ifdef HAVE_SDL_IMAGE
#include <SDL_image.h>
#endif

#include "byte_swapping.h"
#include "screen_drawing.h"


// From screen_sdl.cpp
extern short interface_bit_depth;

// From screen_drawing_sdl.cpp
extern bool draw_clip_rect_active;
extern screen_rectangle draw_clip_rect;

extern bool shapes_file_is_m1();

/*
 *  Uncompress picture data, returns size of compressed image data that was read
 */

// Uncompress (and endian-correct) scan line compressed by PackBits RLE algorithm
template <class T>
static const uint8 *unpack_bits(const uint8 *src, int row_bytes, T *dst)
{
	// Read source count
	int src_count;
	if (row_bytes > 250) {
		src_count = (src[0] << 8) | src[1];
		src += 2;
	} else
		src_count = *src++;

	while (src_count > 0) {

		// Read flag/count byte
		int c = (int8)*src++;
		src_count--;
		if (c < 0) {

			// RLE compressed run
			int size = -c + 1;
			T data;
			if (sizeof(T) == 1) {
				data = *src++;
				src_count--;
			} else {
				data = (src[0] << 8) | src[1];
				src += 2;
				src_count -= 2;
			}
			for (int i=0; i<size; i++)
				*dst++ = data;

		} else {

			// Uncompressed run
			int size = c + 1;
			for (int i=0; i<size; i++) {
				T data;
				if (sizeof(T) == 1) {
					data = *src++;
					src_count--;
				} else {
					data = (src[0] << 8) | src[1];
					src += 2;
					src_count -= 2;
				}
				*dst++ = data;
			}
		}
	}
	return src;
}

// 8-bit picture, one scan line at a time
static int uncompress_rle8(const uint8 *src, int row_bytes, uint8 *dst, int dst_pitch, int height)
{
	const uint8 *start = src;
	for (int y=0; y<height; y++) {
		src = unpack_bits(src, row_bytes, dst);
		dst += dst_pitch;
	}
	return static_cast<int>(src - start);
}

// 16-bit picture, one scan line at a time, 16-bit chunks
static int uncompress_rle16(const uint8 *src, int row_bytes, uint8 *dst, int dst_pitch, int height)
{
	const uint8 *start = src;
	for (int y=0; y<height; y++) {
		src = unpack_bits(src, row_bytes, (uint16 *)dst);
		dst += dst_pitch;
	}
	return static_cast<int>(src - start);
}

static void copy_component_into_surface(const uint8 *src, uint8 *dst, int count, int component)
{
	if (PlatformIsLittleEndian()) {
		dst += 2 - component;
	} else {
		dst += component + 1;
	}
	while (count--) {
		*dst = *src++;
		dst += 4;
	}
}

// 32-bit picture, one scan line, one component at a time
static int uncompress_rle32(const uint8 *src, int row_bytes, uint8 *dst, int dst_pitch, int height)
{
	uint8 *tmp = (uint8 *)malloc(row_bytes);
	if (tmp == NULL)
		return -1;
	memset(tmp, 0, row_bytes);

	const uint8 *start = src;

	int width = row_bytes / 4; 
	for (int y=0; y<height; y++) {
		src = unpack_bits(src, row_bytes, tmp);

		// "tmp" now contains "width" bytes of red, followed by "width"
		// bytes of green and "width" bytes of blue, so we have to copy them
		// into the surface in the right order
		copy_component_into_surface(tmp, dst, width, 0);
		copy_component_into_surface(tmp + width, dst, width, 1);
		copy_component_into_surface(tmp + width * 2, dst, width, 2);

		dst += dst_pitch;
	}

	free(tmp);

	return static_cast<int>(src - start);
}

static int uncompress_picture(const uint8 *src, int row_bytes, uint8 *dst, int dst_pitch, int depth, int height, int pack_type)
{
	// Depths <8 have to be color expanded to depth 8 after uncompressing,
	// so we uncompress into a temporary buffer
	uint8 *orig_dst = dst;
	int orig_dst_pitch = dst_pitch;
	if (depth < 8) {
		dst = (uint8 *)malloc(row_bytes * height);
		dst_pitch = row_bytes;
		if (dst == NULL)
			return -1;
	}

	int data_size = 0;

	if (row_bytes < 8) {

		// Uncompressed data
		const uint8 *p = src;
		uint8 *q = dst;
		for (int y=0; y<height; y++) {
			memcpy(q, p, MIN(row_bytes, dst_pitch));
			p += row_bytes;
			q += dst_pitch;
		}
		data_size = row_bytes * height;

	} else {

		// Compressed data
		if (depth <= 8) {

			// Indexed color
			if (pack_type == 1)
				goto no_packing;
			data_size = uncompress_rle8(src, row_bytes, dst, dst_pitch, height);

		} else {

			// Direct color
			if (pack_type == 0) {
				if (depth == 16)
					pack_type = 3;
				else if (depth == 32)
					pack_type = 4;

			}
			switch (pack_type) {
				case 1: {	// No packing
no_packing:			const uint8 *p = src;
					uint8 *q = dst;
					for (int y=0; y<height; y++) {
						memcpy(q, p, MIN(row_bytes, dst_pitch));
						p += row_bytes;
						q += dst_pitch;
					}
					data_size = row_bytes * height;
					if (depth == 16)
						byte_swap_memory(dst, _2byte, dst_pitch * height / 2);
					else if (depth == 32)
						byte_swap_memory(dst, _4byte, dst_pitch * height / 4);
					break;
				}
				case 3:		// Run-length encoding by 16-bit chunks
					data_size = uncompress_rle16(src, row_bytes, dst, dst_pitch, height);
					break;
				case 4:		// Run-length encoding one component at a time
					data_size = uncompress_rle32(src, row_bytes, dst, dst_pitch, height);
					break;
				default:
					fprintf(stderr, "Unimplemented packing type %d (depth %d) in PICT resource\n", pack_type, depth);
					data_size = -1;
					break;
			}
		}
	}

	// Color expansion 1/2/4->8 bits
	if (depth < 8) {
		const uint8 *p = dst;
		uint8 *q = orig_dst;

		// Source and destination may have different alignment restrictions,
		// don't run off the right of either
		int x_max = row_bytes;
		while (x_max * 8 / depth > orig_dst_pitch)
			x_max--;

		switch (depth) {
			case 1:
				for (int y=0; y<height; y++) {
					for (int x=0; x<x_max; x++) {
						uint8 b = p[x];
						q[x*8+0] = (b & 0x80) ? 0x01 : 0x00;
						q[x*8+1] = (b & 0x40) ? 0x01 : 0x00;
						q[x*8+2] = (b & 0x20) ? 0x01 : 0x00;
						q[x*8+3] = (b & 0x10) ? 0x01 : 0x00;
						q[x*8+4] = (b & 0x08) ? 0x01 : 0x00;
						q[x*8+5] = (b & 0x04) ? 0x01 : 0x00;
						q[x*8+6] = (b & 0x02) ? 0x01 : 0x00;
						q[x*8+7] = (b & 0x01) ? 0x01 : 0x00;
					}
					p += row_bytes;
					q += orig_dst_pitch;
				}
				break;
			case 2:
				for (int y=0; y<height; y++) {
					for (int x=0; x<x_max; x++) {
						uint8 b = p[x];
						q[x*4+0] = (b >> 6) & 0x03;
						q[x*4+1] = (b >> 4) & 0x03;
						q[x*4+2] = (b >> 2) & 0x03;
						q[x*4+3] = b & 0x03;
					}
					p += row_bytes;
					q += orig_dst_pitch;
				}
				break;
			case 4:
				for (int y=0; y<height; y++) {
					for (int x=0; x<x_max; x++) {
						uint8 b = p[x];
						q[x*2+0] = (b >> 4) & 0x0f;
						q[x*2+1] = b & 0x0f;
					}
					p += row_bytes;
					q += orig_dst_pitch;
				}
				break;
		}
		free(dst);
	}

	return data_size;
}

int get_pict_header_width(LoadedResource &rsrc)
{
	SDL_RWops *p = SDL_RWFromMem(rsrc.GetPointer(), (int) rsrc.GetLength());
	if (p)
	{
		SDL_RWseek(p, 8, SEEK_CUR);
		int width = SDL_ReadBE16(p);
		SDL_RWclose(p);
		return width;
	}
	return -1;
}

/*
 *  Convert picture resource to SDL surface
 */

std::unique_ptr<SDL_Surface, decltype(&SDL_FreeSurface)> picture_to_surface(LoadedResource &rsrc)
{
	auto s = std::unique_ptr<SDL_Surface, decltype(&SDL_FreeSurface)>(nullptr, SDL_FreeSurface);

	if (!rsrc.IsLoaded())
		return s;

	// Open stream to picture resource
	SDL_RWops *p = SDL_RWFromMem(rsrc.GetPointer(), (int)rsrc.GetLength());
	if (p == NULL)
		return s;
	SDL_RWseek(p, 6, SEEK_CUR);		// picSize/top/left
	int pic_height = SDL_ReadBE16(p);
	int pic_width = SDL_ReadBE16(p);
	//printf("pic_width %d, pic_height %d\n", pic_width, pic_height);

	// Read and parse picture opcodes
	bool done = false;
	while (!done) {
		uint16 opcode = SDL_ReadBE16(p);
		//printf("%04x\n", opcode);
		switch (opcode) {

			case 0x0000:	// NOP
			case 0x0011:	// VersionOp
			case 0x001c:	// HiliteMode
			case 0x001e:	// DefHilite
			case 0x0038:	// FrameSameRect
			case 0x0039:	// PaintSameRect
			case 0x003a:	// EraseSameRect
			case 0x003b:	// InvertSameRect
			case 0x003c:	// FillSameRect
			case 0x02ff:	// Version
				break;

			case 0x00ff:	// OpEndPic
				done = true;
				break;

			case 0x0001: {	// Clipping region
				uint16 size = SDL_ReadBE16(p);
				if (size & 1)
					size++;
				SDL_RWseek(p, size - 2, SEEK_CUR);
				break;
			}

			case 0x0003:	// TxFont
			case 0x0004:	// TxFace
			case 0x0005:	// TxMode
			case 0x0008:	// PnMode
			case 0x000d:	// TxSize
			case 0x0015:	// PnLocHFrac
			case 0x0016:	// ChExtra
			case 0x0023:	// ShortLineFrom
			case 0x00a0:	// ShortComment
				SDL_RWseek(p, 2, SEEK_CUR);
				break;

			case 0x0006:	// SpExtra
			case 0x0007:	// PnSize
			case 0x000b:	// OvSize
			case 0x000c:	// Origin
			case 0x000e:	// FgColor
			case 0x000f:	// BgColor
			case 0x0021:	// LineFrom
				SDL_RWseek(p, 4, SEEK_CUR);
				break;

			case 0x001a:	// RGBFgCol
			case 0x001b:	// RGBBkCol
			case 0x001d:	// HiliteColor
			case 0x001f:	// OpColor
			case 0x0022:	// ShortLine
				SDL_RWseek(p, 6, SEEK_CUR);
				break;

			case 0x0002:	// BkPat
			case 0x0009:	// PnPat
			case 0x000a:	// FillPat
			case 0x0010:	// TxRatio
			case 0x0020:	// Line
			case 0x0030:	// FrameRect
			case 0x0031:	// PaintRect
			case 0x0032:	// EraseRect
			case 0x0033:	// InvertRect
			case 0x0034:	// FillRect
				SDL_RWseek(p, 8, SEEK_CUR);
				break;

			case 0x0c00:	// HeaderOp
				SDL_RWseek(p, 24, SEEK_CUR);
				break;

			case 0x00a1: {	// LongComment
				SDL_RWseek(p, 2, SEEK_CUR);
				int size = SDL_ReadBE16(p);
				if (size & 1)
					size++;
				SDL_RWseek(p, size, SEEK_CUR);
				break;
			}

			case 0x0098:	// Packed CopyBits
			case 0x0099:	// Packed CopyBits with clipping region
			case 0x009a:	// Direct CopyBits
			case 0x009b: {	// Direct CopyBits with clipping region
				// 1. PixMap
				if (opcode == 0x009a || opcode == 0x009b)
					SDL_RWseek(p, 4, SEEK_CUR);		// pmBaseAddr
				uint16 row_bytes = SDL_ReadBE16(p);	// the upper 2 bits are flags
				//printf(" row_bytes %04x\n", row_bytes);
				bool is_pixmap = ((row_bytes & 0x8000) != 0);
				row_bytes &= 0x3fff;
				uint16 top = SDL_ReadBE16(p);
				uint16 left = SDL_ReadBE16(p);
				uint16 height = SDL_ReadBE16(p) - top;
				uint16 width = SDL_ReadBE16(p) - left;
				uint16 pack_type, pixel_size;
				if (is_pixmap) {
					SDL_RWseek(p, 2, SEEK_CUR);			// pmVersion
					pack_type = SDL_ReadBE16(p);
					SDL_RWseek(p, 14, SEEK_CUR);		// packSize/hRes/vRes/pixelType
					pixel_size = SDL_ReadBE16(p);
					SDL_RWseek(p, 16, SEEK_CUR);		// cmpCount/cmpSize/planeBytes/pmTable/pmReserved
				} else {
					pack_type = 0;
					pixel_size = 1;
				}
				//printf(" width %d, height %d, row_bytes %d, depth %d, pack_type %d\n", width, height, row_bytes, pixel_size, pack_type);

				// Allocate surface for picture
				uint32 Rmask = 0, Gmask = 0, Bmask = 0;
				int surface_depth = 8;
				switch (pixel_size) {
					case 1:
					case 2:
					case 4:
					case 8:
						Rmask = Gmask = Bmask = 0;
						surface_depth = 8;	// SDL surfaces must be at least 8 bits depth, so we expand 1/2/4-bit pictures to 8-bit
						break;
					case 16:
						Rmask = 0x7c00;
						Gmask = 0x03e0;
						Bmask = 0x001f;
						surface_depth = 16;
						break;
					case 32:
						Rmask = 0x00ff0000;
						Gmask = 0x0000ff00;
						Bmask = 0x000000ff;
						surface_depth = 32;
						break;
					default:
						fprintf(stderr, "Unsupported PICT depth %d\n", pixel_size);
						done = true;
						break;
				}
				if (done)
					break;
				SDL_Surface *bm = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, surface_depth, Rmask, Gmask, Bmask, 0);
				if (bm == NULL) {
					done = true;
					break;
				}

				// 2. ColorTable
				if (is_pixmap && (opcode == 0x0098 || opcode == 0x0099)) {
					SDL_Color colors[256];
					SDL_RWseek(p, 4, SEEK_CUR);			// ctSeed
					uint16 flags = SDL_ReadBE16(p);
					int num_colors = SDL_ReadBE16(p) + 1;
					for (int i=0; i<num_colors; i++) {
						uint8 value = SDL_ReadBE16(p) & 0xff;
						if (flags & 0x8000)
							value = i;
						colors[value].r = SDL_ReadBE16(p) >> 8;
						colors[value].g = SDL_ReadBE16(p) >> 8;
						colors[value].b = SDL_ReadBE16(p) >> 8;
						colors[value].a = 0xff;
					}
					SDL_SetPaletteColors(bm->format->palette, colors, 0, 256);
				}

				// 3. source/destination Rect and transfer mode
				SDL_RWseek(p, 18, SEEK_CUR);

				// 4. clipping region
				if (opcode == 0x0099 || opcode == 0x009b) {
					uint16 rgn_size = SDL_ReadBE16(p);
					SDL_RWseek(p, rgn_size - 2, SEEK_CUR);
				}

				// 5. graphics data
				int data_size = uncompress_picture((uint8 *)rsrc.GetPointer() + SDL_RWtell(p), row_bytes, (uint8 *)bm->pixels, bm->pitch, pixel_size, height, pack_type);
				if (data_size < 0) {
					done = true;
					break;
				}
				if (data_size & 1)
					data_size++;
				SDL_RWseek(p, data_size, SEEK_CUR);

				// If there's already a surface, throw away the decoded image
				// (actually, we could have skipped this entire opcode, but the
				// only way to do this is to decode the image data).
				// So we only draw the first image we encounter.
				if (s) {
					SDL_FreeSurface(bm);
				}
				else {
					s.reset(bm);
				}

				break;
			}

#ifdef HAVE_SDL_IMAGE
			case 0x8200: {	// Compressed QuickTime image (we only handle JPEG compression)
				// 1. Header
				uint32 opcode_size = SDL_ReadBE32(p);
				if (opcode_size & 1)
					opcode_size++;
				uint32 opcode_start = SDL_RWtell(p);
				SDL_RWseek(p, 26, SEEK_CUR);	// version/matrix (hom. part)
				int offset_x = SDL_ReadBE16(p);
				SDL_RWseek(p, 2, SEEK_CUR);
				int offset_y = SDL_ReadBE16(p);
				SDL_RWseek(p, 6, SEEK_CUR);	// matrix (remaining part)
				uint32 matte_size = SDL_ReadBE32(p);
				SDL_RWseek(p, 22, SEEK_CUR);	// matteRec/mode/srcRect/accuracy
				uint32 mask_size = SDL_ReadBE32(p);

				// 2. Matte image description
				if (matte_size) {
					uint32 matte_id_size = SDL_ReadBE32(p);
					SDL_RWseek(p, matte_id_size - 4, SEEK_CUR);
				}

				// 3. Matte data
				SDL_RWseek(p, matte_size, SEEK_CUR);

				// 4. Mask region
				SDL_RWseek(p, mask_size, SEEK_CUR);

				// 5. Image description
				uint32 id_start = SDL_RWtell(p);
				uint32 id_size = SDL_ReadBE32(p);
				uint32 codec_type = SDL_ReadBE32(p);
				if (codec_type != FOUR_CHARS_TO_INT('j','p','e','g')) {
					fprintf(stderr, "Unsupported codec type %c%c%c%c\n", codec_type >> 24, codec_type >> 16, codec_type >> 8, codec_type);
					done = true;
					break;
				}
				SDL_RWseek(p, 36, SEEK_CUR);	// resvd1/resvd2/dataRefIndex/version/revisionLevel/vendor/temporalQuality/spatialQuality/width/height/hRes/vRes
				uint32 data_size = SDL_ReadBE32(p);
				SDL_RWseek(p, id_start + id_size, SEEK_SET);

				// Allocate surface for complete (but possibly banded) picture
				if (!s) {
					s.reset(SDL_CreateRGBSurface(SDL_SWSURFACE, pic_width, pic_height, 32,
#ifdef ALEPHONE_LITTLE_ENDIAN
								 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000
#else
								 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff
#endif
							));
					if (!s) {
						done = true;
						break;
					}
				}

				// 6. Compressed image data
				SDL_RWops *img = SDL_RWFromMem((uint8 *)rsrc.GetPointer() + SDL_RWtell(p), data_size);
				if (img == NULL) {
					done = true;
					break;
				}
				SDL_Surface *bm = IMG_LoadTyped_RW(img, true, const_cast<char*>("JPG"));

				// Copy image (band) into surface
				if (bm) {
					SDL_Rect dst_rect = {offset_x, offset_y, bm->w, bm->h};
					SDL_BlitSurface(bm, NULL, s.get(), &dst_rect);
					SDL_FreeSurface(bm);
				}

				SDL_RWseek(p, opcode_start + opcode_size, SEEK_SET);
				break;
			}
#endif

			default:
				if (opcode >= 0x0300 && opcode < 0x8000)
					SDL_RWseek(p, (opcode >> 8) * 2, SEEK_CUR);
				else if (opcode >= 0x8000 && opcode < 0x8100)
					break;
				else {
					fprintf(stderr, "Unimplemented opcode %04x in PICT resource\n", opcode);
					done = true;
				}
				break;
		}
	}

	// Close stream, return surface
	SDL_RWclose(p);
	return s;
}


/*
 *  Rescale surface to given dimensions
 */

template <class T>
static void rescale(T *src_pixels, int src_pitch, T *dst_pixels, int dst_pitch, int width, int height, uint32 dx, uint32 dy)
{
	// Brute-force rescaling, no interpolation
	uint32 sy = 0;
	for (int y=0; y<height; y++) {
		T *p = src_pixels + src_pitch / sizeof(T) * (sy >> 16);
		uint32 sx = 0;
		for (int x=0; x<width; x++) {
			dst_pixels[x] = p[sx >> 16];
			sx += dx;
		}
		dst_pixels += dst_pitch / sizeof(T);
		sy += dy;
	}
}

SDL_Surface *rescale_surface(SDL_Surface *s, int width, int height)
{
	if (s == NULL)
		return NULL;

	SDL_Surface *s2 = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, s->format->BitsPerPixel, s->format->Rmask, s->format->Gmask, s->format->Bmask, s->format->Amask);
	if (s2 == NULL)
		return NULL;

	uint32 dx = (s->w << 16) / width;
	uint32 dy = (s->h << 16) / height;

	switch (s->format->BytesPerPixel) {
		case 1:
			rescale((pixel8 *)s->pixels, s->pitch, (pixel8 *)s2->pixels, s2->pitch, width, height, dx, dy);
			break;
		case 2:
			rescale((pixel16 *)s->pixels, s->pitch, (pixel16 *)s2->pixels, s2->pitch, width, height, dx, dy);
			break;
		case 4:
			rescale((pixel32 *)s->pixels, s->pitch, (pixel32 *)s2->pixels, s2->pitch, width, height, dx, dy);
			break;
	}

	if (s->format->palette)
		SDL_SetPaletteColors(s2->format->palette, s->format->palette->colors, 0, s->format->palette->ncolors);

	return s2;
}


/*
 *  Tile surface to fill given dimensions
 */

template <class T>
static void tile(T *src_pixels, int src_pitch, T *dst_pixels, int dst_pitch, int src_width, int src_height, int dst_width, int dst_height)
{
	T *p = src_pixels;
	int sy = 0;
	for (int y=0; y<dst_height; y++) {
		int sx = 0;
		for (int x=0; x<dst_width; x++) {
			dst_pixels[x] = p[sx];
			sx++;
			if (sx == src_width)
				sx = 0;
		}
		dst_pixels += dst_pitch / sizeof(T);
		sy++;
		if (sy == src_height) {
			sy = 0;
			p = src_pixels;
		} else
			p += src_pitch / sizeof(T);
	}
}

SDL_Surface *tile_surface(SDL_Surface *s, int width, int height)
{
	if (s == NULL)
		return NULL;

	SDL_Surface *s2 = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, s->format->BitsPerPixel, s->format->Rmask, s->format->Gmask, s->format->Bmask, s->format->Amask);
	if (s2 == NULL)
		return NULL;

	switch (s->format->BytesPerPixel) {
		case 1:
			tile((pixel8 *)s->pixels, s->pitch, (pixel8 *)s2->pixels, s2->pitch, s->w, s->h, width, height);
			break;
		case 2:
			tile((pixel16 *)s->pixels, s->pitch, (pixel16 *)s2->pixels, s2->pitch, s->w, s->h, width, height);
			break;
		case 3:
			tile((pixel8 *)s->pixels, s->pitch, (pixel8 *)s2->pixels, s2->pitch, s->w * 3, s->h, width * 3, height);
			break;
		case 4:
			tile((pixel32 *)s->pixels, s->pitch, (pixel32 *)s2->pixels, s2->pitch, s->w, s->h, width, height);
			break;
	}

	if (s->format->palette)
		SDL_SetPaletteColors(s2->format->palette, s->format->palette->colors, 0, s->format->palette->ncolors);

	return s2;
}


/*
 *  Draw picture resource centered on screen
 */

extern SDL_Surface *draw_surface;	// from screen_drawing.cpp
//void draw_intro_screen(void);		// from screen.cpp

static void draw_picture_surface(std::shared_ptr<SDL_Surface> s)
{
	if (!s)
		return;
	_set_port_to_intro();
	SDL_Surface *video = draw_surface;

	// Default source rectangle
	SDL_Rect src_rect = {0, 0, MIN(s->w, 640), MIN(s->h, 480)};

	// Center picture on screen
	SDL_Rect dst_rect = {(video->w - src_rect.w) / 2, (video->h - src_rect.h) / 2, s->w, s->h};
	if (dst_rect.x < 0)
		dst_rect.x = 0;
	if (dst_rect.y < 0)
		dst_rect.y = 0;

	// Clip if desired (only used for menu buttons)
	if (draw_clip_rect_active) {
		src_rect.w = dst_rect.w = draw_clip_rect.right - draw_clip_rect.left;
		src_rect.h = dst_rect.h = draw_clip_rect.bottom - draw_clip_rect.top;
		src_rect.x = draw_clip_rect.left - (640 - s->w) / 2;
		src_rect.y = draw_clip_rect.top - (480 - s->h) / 2;
		dst_rect.x += draw_clip_rect.left- (640 - s->w) / 2;
		dst_rect.y += draw_clip_rect.top - (480 - s->h) / 2;
	} else {
			// Clear destination to black
			SDL_FillRect(video, NULL, SDL_MapRGB(video->format, 0, 0, 0));
	}
	
	SDL_BlitSurface(s.get(), &src_rect, video, &dst_rect);
	_restore_port();
	draw_intro_screen();
}

static void draw_picture(LoadedResource &rsrc)
{
    draw_picture_surface(picture_to_surface(rsrc));
}


/*
 *  Get system color table
 */

struct color_table *build_8bit_system_color_table(void)
{
	// 6*6*6 RGB color cube
	color_table *table = new color_table;
	table->color_count = 6*6*6;
	int index = 0;
	for (int red=0; red<6; red++) {
		for (int green=0; green<6; green++) {
			for (int blue=0; blue<6; blue++) {
				uint8 r = red * 0x33;
				uint8 g = green * 0x33;
				uint8 b = blue * 0x33;
				table->colors[index].red = (r << 8) | r;
				table->colors[index].green = (g << 8) | g;
				table->colors[index].blue = (b << 8) | b;
				index++;
			}
		}
	}
	return table;
}


/*
 *  Scroll image across screen
 */

#define SCROLLING_SPEED (MACHINE_TICKS_PER_SECOND / 20)

void scroll_full_screen_pict_resource_from_scenario(int pict_resource_number, bool text_block)
{
	// Convert picture resource to surface, free resource
	LoadedResource rsrc;
	get_picture_resource_from_scenario(pict_resource_number, rsrc);
	auto s = picture_to_surface(rsrc);
	if (!s)
		return;

	// Find out in which direction to scroll
	int picture_width = s->w;
	int picture_height = s->h;
	int screen_width = 640;
	int screen_height = 480;
	bool scroll_horizontal = picture_width > screen_width;
	bool scroll_vertical = picture_height > screen_height;

	if (scroll_horizontal || scroll_vertical) {

		// Flush events
		SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);

		// Prepare source and destination rectangles
		SDL_Rect src_rect = {0, 0, scroll_horizontal ? screen_width : picture_width, scroll_vertical ? screen_height : picture_height};
		SDL_Rect dst_rect = {0, 0, screen_width, screen_height};

		// Scroll loop
		bool done = false, aborted = false;
		uint32 start_tick = machine_tick_count();
		do {

			int32 delta = (machine_tick_count() - start_tick) / (text_block ? (2 * SCROLLING_SPEED) : SCROLLING_SPEED);
			if (scroll_horizontal && delta > picture_width - screen_width) {
				delta = picture_width - screen_width;
				done = true;
			}
			if (scroll_vertical && delta > picture_height - screen_height) {
				delta = picture_height - screen_height;
				done = true;
			}

			// Blit part of picture
			src_rect.x = scroll_horizontal ? delta : 0;
			src_rect.y = scroll_vertical ? delta : 0;
			_set_port_to_intro();
			SDL_BlitSurface(s.get(), &src_rect, draw_surface, &dst_rect);
			_restore_port();
			draw_intro_screen();

			// Give system time
			global_idle_proc();
			yield();

			// Check for events to abort
			SDL_Event event;
			if (SDL_PollEvent(&event)) {
				switch (event.type) {
					case SDL_MOUSEBUTTONDOWN:
					case SDL_KEYDOWN:
					case SDL_CONTROLLERBUTTONDOWN:
						aborted = true;
						break;
				}
			}

		} while (!done && !aborted);
	}
}


/*
 *  Initialize image manager, open Images file
 */

void initialize_images_manager(void)
{
	FileSpecifier file;

  logContext("loading Images...");

	file.SetNameWithPath(getcstr(temporary, strFILENAMES, filenameIMAGES)); // _typecode_images
	
	if (!file.Exists())
        logContext("Images file not found");
	
	if (!ImagesFile.open_file(file))
        logContext("Images file could not be opened");

	atexit(shutdown_images_handler);
}


/*
 *  Shutdown image manager
 */

static void shutdown_images_handler(void)
{
	SoundsImagesFile.close_file();
	ExternalResourcesFile.close_file();
	ShapesImagesFile.close_file();
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

void unset_scenario_images_file()
{
	ScenarioFile.close_file();
}

void set_shapes_images_file(FileSpecifier &file)
{
	ShapesImagesFile.open_file(file);
}

void set_external_resources_images_file(FileSpecifier &file)
{
    // fail here, instead of above, if Images is missing
	if ((!file.Exists() || !ExternalResourcesFile.open_file(file)) &&
        !ImagesFile.is_open())
        alert_bad_extra_file();
        
}

void set_sounds_images_file(FileSpecifier &file)
{
	SoundsImagesFile.open_file(file);
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
	return has_rsrc(FOUR_CHARS_TO_INT('P','I','C','T'), FOUR_CHARS_TO_INT('P','I','C','T'), id) || has_rsrc(FOUR_CHARS_TO_INT('P','I','C','T'), FOUR_CHARS_TO_INT('p','i','c','t'), id);
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
					if (wad_type == FOUR_CHARS_TO_INT('P','I','C','T'))
					{
						void *pict_data = malloc(raw_length);
						memcpy(pict_data, raw, raw_length);
						rsrc.SetData(pict_data, raw_length);
						success = true;
					}
					else
					{
						size_t clut_length;
						void *clut_data = extract_type_from_wad(d, FOUR_CHARS_TO_INT('c','l','u','t'), &clut_length);
						success = make_rsrc_from_pict(raw, raw_length, rsrc, clut_data, clut_length);
					}
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
	return get_rsrc(FOUR_CHARS_TO_INT('P','I','C','T'), FOUR_CHARS_TO_INT('P','I','C','T'), id, rsrc) || get_rsrc(FOUR_CHARS_TO_INT('P','I','C','T'), FOUR_CHARS_TO_INT('p','i','c','t'), id, rsrc);
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
    bool found = false;
    
    if (!found && ImagesFile.is_open())
        found = ImagesFile.get_pict(ImagesFile.determine_pict_resource_id(base_resource, _images_file_delta16, _images_file_delta32), PictRsrc);
    if (!found && ExternalResourcesFile.is_open())
        found = ExternalResourcesFile.get_pict(base_resource, PictRsrc);
    if (!found && ShapesImagesFile.is_open())
        found = ShapesImagesFile.get_pict(base_resource, PictRsrc);
    
    return found;
}

bool get_sound_resource_from_images(int resource_number, LoadedResource &SoundRsrc)
{
    bool found = false;
    
    if (!found && ImagesFile.is_open())
        found = ImagesFile.get_snd(resource_number, SoundRsrc);
    if (!found && SoundsImagesFile.is_open())
    {
        // Marathon 1 case: only one sound used for intro
        if (resource_number == 1111 || resource_number == 1114)
            found = SoundsImagesFile.get_snd(1240, SoundRsrc);
    }
    
    return found;
}

bool images_picture_exists(int base_resource)
{
	if (shapes_file_is_m1() && (base_resource == MAIN_MENU_BASE || base_resource == MAIN_MENU_BASE+1))
        return true;
    
    LoadedResource PictRsrc;
    return get_picture_resource_from_images(base_resource, PictRsrc);
}


// In the first Marathon, the main menu is drawn from multiple
// shapes in collection 10, instead of a single image. We handle
// this special case by creating the composite images in code,
// and returning these surfaces when the picture is requested.

static auto m1_menu_unpressed = std::shared_ptr<SDL_Surface>(nullptr, SDL_FreeSurface);
static auto m1_menu_pressed = std::shared_ptr<SDL_Surface>(nullptr, SDL_FreeSurface);

static void create_m1_menu_surfaces(void)
{
    if (m1_menu_unpressed || m1_menu_pressed)
        return;
    
    auto s = std::unique_ptr<SDL_Surface>(nullptr);
	if (PlatformIsLittleEndian()) {
    	s.reset(SDL_CreateRGBSurface(SDL_SWSURFACE, 640, 480, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0));
	} else {
    	s.reset(SDL_CreateRGBSurface(SDL_SWSURFACE, 640, 480, 32, 0xff000000, 0x00ff0000, 0x0000ff00, 0));
	}
    if (!s)
        return;

    SDL_FillRect(s.get(), NULL, SDL_MapRGB(s->format, 0, 0, 0));
    
    SDL_Rect src, dst;
    src.x = src.y = 0;

    // in comments you can see how the hard-coded numbers were arrived at for
    // Marathon--but for third party scenarios, the math doesn't work, so
    // hard-code the offsets instead
    
//    int top = 0;
//    int bottom = s->h;
    
    SDL_Surface *logo = get_shape_surface(0, 10);
    if (!logo)
    {
        // did it fail because we haven't loaded the menu shapes?
        mark_collection_for_loading(10);
        load_collections(false, false);
        logo = get_shape_surface(0, 10);
    }
    if (logo)
    {
        src.w = dst.w = logo->w;
        src.h = dst.h = logo->h;
//        dst.x = (s->w - logo->w)/2;
//        dst.y = 0;
        dst.x = 75;
        dst.y = 0;
        SDL_BlitSurface(logo, &src, s.get(), &dst);
//        top += logo->h;
        SDL_FreeSurface(logo);
    }
    
    SDL_Surface *credits = get_shape_surface(19, 10);
    if (credits)
    {
        src.w = dst.w = credits->w;
        src.h = dst.h = credits->h;
//        dst.x = (s->w - credits->w)/2;
//        dst.y = s->h - credits->h;
        dst.x = 191;
        dst.y = 466;
        SDL_BlitSurface(credits, &src, s.get(), &dst);
//        bottom -= credits->h;
        SDL_FreeSurface(credits);
    }
    
    SDL_Surface *widget = get_shape_surface(1, 10);
    if (widget)
    {
        src.w = dst.w = widget->w;
        src.h = dst.h = widget->h;
//        dst.x = (s->w - widget->w)/2;
//        dst.y = top + (bottom - top - widget->h)/2;
        dst.x = 102;
        dst.y = 117;
        SDL_BlitSurface(widget, &src, s.get(), &dst);
        SDL_FreeSurface(widget);
    }
    m1_menu_unpressed = std::move(s);
    
    // now, add pressed buttons to copy of this surface
    s.reset(SDL_ConvertSurface(m1_menu_unpressed.get(), m1_menu_unpressed.get()->format, SDL_SWSURFACE));
    
    std::vector<std::pair<int, int> > button_shapes;
    button_shapes.push_back(std::pair<int, int>(_new_game_button_rect, 11));
    button_shapes.push_back(std::pair<int, int>(_load_game_button_rect, 12));
    button_shapes.push_back(std::pair<int, int>(_gather_button_rect, 3));
    button_shapes.push_back(std::pair<int, int>(_join_button_rect, 4));
    button_shapes.push_back(std::pair<int, int>(_prefs_button_rect, 5));
    button_shapes.push_back(std::pair<int, int>(_replay_last_button_rect, 6));
    button_shapes.push_back(std::pair<int, int>(_save_last_button_rect, 7));
    button_shapes.push_back(std::pair<int, int>(_replay_saved_button_rect, 8));
    button_shapes.push_back(std::pair<int, int>(_credits_button_rect, 9));
    button_shapes.push_back(std::pair<int, int>(_quit_button_rect, 10));
    button_shapes.push_back(std::pair<int, int>(_center_button_rect, 2));
    for (std::vector<std::pair<int, int> >::const_iterator it = button_shapes.begin(); it != button_shapes.end(); ++it)
    {
        screen_rectangle *r = get_interface_rectangle(it->first);
        SDL_Surface *btn = get_shape_surface(it->second, 10);
        if (btn)
        {
            src.w = dst.w = btn->w;
            src.h = dst.h = btn->h;
            dst.x = r->left;
            dst.y = r->top;
            SDL_BlitSurface(btn, &src, s.get(), &dst);
            SDL_FreeSurface(btn);
        }
    }
    
    m1_menu_pressed = std::move(s);
}

static bool m1_draw_full_screen_pict_resource_from_images(int pict_resource_number)
{
    if (!shapes_file_is_m1())
        return false;
    if (pict_resource_number == MAIN_MENU_BASE)
    {
        create_m1_menu_surfaces();
        draw_picture_surface(m1_menu_unpressed);
        return true;
    }
    else if (pict_resource_number == MAIN_MENU_BASE+1)
    {
        create_m1_menu_surfaces();
        draw_picture_surface(m1_menu_pressed);
        return true;
    }
    return false;
}

void draw_full_screen_pict_resource_from_images(int pict_resource_number)
{
	if (m1_draw_full_screen_pict_resource_from_images(pict_resource_number))
		return;
    
    LoadedResource PictRsrc;
    if (get_picture_resource_from_images(pict_resource_number, PictRsrc))
        draw_picture(PictRsrc);
}


/*
 *  Get/draw image from scenario
 */

bool get_picture_resource_from_scenario(int base_resource, LoadedResource &PictRsrc)
{
    bool found = false;
    
    if (!found && ScenarioFile.is_open())
        found = ScenarioFile.get_pict(ScenarioFile.determine_pict_resource_id(base_resource, _scenario_file_delta16, _scenario_file_delta32), PictRsrc);
    if (!found && ShapesImagesFile.is_open())
        found = ShapesImagesFile.get_pict(base_resource, PictRsrc);
    
    return found;
}

bool scenario_picture_exists(int base_resource)
{
    LoadedResource PictRsrc;
    return get_picture_resource_from_scenario(base_resource, PictRsrc);
}

void draw_full_screen_pict_resource_from_scenario(int pict_resource_number)
{
	LoadedResource PictRsrc;
	if (get_picture_resource_from_scenario(pict_resource_number, PictRsrc))
        draw_picture(PictRsrc);
}


/*
 *  Get sound resource from scenario
 */

bool get_sound_resource_from_scenario(int resource_number, LoadedResource &SoundRsrc)
{
    bool found = false;
    
    if (!found && ScenarioFile.is_open())
        found = ScenarioFile.get_snd(resource_number, SoundRsrc);
    if (!found && SoundsImagesFile.is_open())
        // Marathon 1 case: only one sound used for chapter screens
        found = SoundsImagesFile.get_snd(1240, SoundRsrc);
    
    return found;
}


// LP: do the same for text resources

bool get_text_resource_from_scenario(int resource_number, LoadedResource &TextRsrc)
{
	if (!ScenarioFile.is_open())
		return false;

	bool success = ScenarioFile.get_text(resource_number, TextRsrc);
	return success;
}


/*
 *  Calculate color table for image
 */

struct color_table *calculate_picture_clut(int CLUTSource, int pict_resource_number)
{
	struct color_table *picture_table = NULL;

#if 1
    // with TRUE_COLOR_ONLY turned on, specific cluts don't matter
    picture_table = build_8bit_system_color_table();
    build_direct_color_table(picture_table, interface_bit_depth);
#else
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

		// Allocate color table
		picture_table = new color_table;

		// Convert MacOS CLUT resource to color table
		if (interface_bit_depth == 8)
			build_color_table(picture_table, CLUT_Rsrc);
		else
			build_direct_color_table(picture_table, interface_bit_depth);
	}

#endif
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
