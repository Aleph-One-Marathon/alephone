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
 *  images_sdl.cpp - Image management, SDL implementation (included by images.cpp)
 *
 *  Written in 2000 by Christian Bauer
 */

#include <SDL_endian.h>

#ifdef HAVE_SDL_IMAGE_H
#include <SDL_image.h>
#endif

#include "byte_swapping.h"
#include "screen_drawing.h"


// From screen_sdl.cpp
extern short interface_bit_depth;

// From screen_drawing_sdl.cpp
extern bool draw_clip_rect_active;
extern screen_rectangle draw_clip_rect;


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
#ifdef ALEPHONE_LITTLE_ENDIAN
	dst += 2 - component;
#else
	dst += component + 1;
#endif
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


/*
 *  Convert picture resource to SDL surface
 */

SDL_Surface *picture_to_surface(LoadedResource &rsrc)
{
	if (!rsrc.IsLoaded())
		return NULL;

	SDL_Surface *s = NULL;

	// Open stream to picture resource
	SDL_RWops *p = SDL_RWFromMem(rsrc.GetPointer(), (int)rsrc.GetLength());
	if (p == NULL)
		return NULL;
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
						Rmask = Gmask = Bmask = 0xff;
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
					}
					SDL_SetColors(bm, colors, 0, 256);
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
				if (s)
					SDL_FreeSurface(bm);
				else
					s = bm;
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
				if (s == NULL) {
					s = SDL_CreateRGBSurface(SDL_SWSURFACE, pic_width, pic_height, 24,
#ifdef ALEPHONE_LITTLE_ENDIAN
							0x0000ff, 0x00ff00, 0xff0000,
#else
							0xff0000, 0x00ff00, 0x0000ff,
#endif
							0);
					if (s == NULL) {
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
				SDL_Surface *bm = IMG_LoadTyped_RW(img, true, "JPG");

				// Copy image (band) into surface
				if (bm) {
					SDL_Rect dst_rect = {offset_x, offset_y, bm->w, bm->h};
					SDL_BlitSurface(bm, NULL, s, &dst_rect);
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
		SDL_SetColors(s2, s->format->palette->colors, 0, s->format->palette->ncolors);

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
		SDL_SetColors(s2, s->format->palette->colors, 0, s->format->palette->ncolors);

	return s2;
}


/*
 *  Draw picture resource centered on screen
 */

static void draw_picture(LoadedResource &rsrc)
{
	// Convert picture resource to surface, free resource
	SDL_Surface *s = picture_to_surface(rsrc);
	if (s == NULL)
		return;
	SDL_Surface *video = SDL_GetVideoSurface();

	// Default source rectangle
	SDL_Rect src_rect = {0, 0, s->w, s->h};

	// Center picture on screen
	SDL_Rect dst_rect = {(video->w - s->w) / 2, (video->h - s->h) / 2, s->w, s->h};
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

	// Blit picture to screen
	SDL_BlitSurface(s, &src_rect, video, &dst_rect);
	SDL_FreeSurface(s);

	// Update display and free picture surface
	SDL_UpdateRects(video, 1, &dst_rect);
#ifdef HAVE_OPENGL
		if (video->flags & SDL_OPENGL)
		  SDL_GL_SwapBuffers();
#endif
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
	SDL_Surface *s = picture_to_surface(rsrc);
	if (s == NULL)
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
		SDL_Event event;
		while (SDL_PollEvent(&event)) ;

		// Prepare source and destination rectangles
		SDL_Rect src_rect = {0, 0, scroll_horizontal ? screen_width : picture_width, scroll_vertical ? screen_height : picture_height};
		SDL_Rect dst_rect = {(SDL_GetVideoSurface()->w - screen_width) / 2, (SDL_GetVideoSurface()->h - screen_height) / 2, screen_width, screen_height};

		// Scroll loop
		bool done = false, aborted = false;
		uint32 start_tick = SDL_GetTicks();
		do {

			int32 delta = (SDL_GetTicks() - start_tick) / (text_block ? (2 * SCROLLING_SPEED) : SCROLLING_SPEED);
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
			SDL_BlitSurface(s, &src_rect, SDL_GetVideoSurface(), &dst_rect);
			SDL_UpdateRects(SDL_GetVideoSurface(), 1, &dst_rect);
#ifdef HAVE_OPENGL
			if (SDL_GetVideoSurface()->flags & SDL_OPENGL)
				SDL_GL_SwapBuffers();
#endif

			// Give system time
			global_idle_proc();
			SDL_Delay(10);

			// Check for events to abort
			event.type = SDL_NOEVENT;
			SDL_PollEvent(&event);
			switch (event.type) {
				case SDL_MOUSEBUTTONDOWN:
				case SDL_KEYDOWN:
					aborted = true;
					break;
			}

		} while (!done && !aborted);
	}

	// Free surface
	SDL_FreeSurface(s);
}
