/*
 *  images_sdl.cpp - Image management, SDL implementation (included by images.cpp)
 *
 *  Written in 2000 by Christian Bauer
 */

#include <SDL_endian.h>

#include "byte_swapping.h"
#include "screen_drawing.h"


// From screen_sdl.cpp
extern short interface_bit_depth;

// From screen_drawing_sdl.cpp
extern bool draw_clip_rect_active;
extern screen_rectangle draw_clip_rect;


/*
 *  Uncompress picture data
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
static void uncompress_rle8(const uint8 *src, int row_bytes, uint8 *dst, int dst_pitch, int height)
{
	for (int y=0; y<height; y++) {
		src = unpack_bits(src, row_bytes, dst);
		dst += dst_pitch;
	}
}

// 16-bit picture, one scan line at a time, 16-bit chunks
static void uncompress_rle16(const uint8 *src, int row_bytes, uint8 *dst, int dst_pitch, int height)
{
	for (int y=0; y<height; y++) {
		src = unpack_bits(src, row_bytes, (uint16 *)dst);
		dst += dst_pitch;
	}
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
static void uncompress_rle32(const uint8 *src, int row_bytes, uint8 *dst, int dst_pitch, int height)
{
	uint8 *tmp = (uint8 *)malloc(row_bytes);
	if (tmp == NULL)
		return;

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
}

static void uncompress_picture(const uint8 *src, int row_bytes, uint8 *dst, int dst_pitch, int depth, int height, int pack_type)
{
	if (row_bytes < 8) {

		// Uncompressed data
		memcpy(dst, src, row_bytes * height);

	} else {

		// Compressed data
		if (depth == 8) {
			uncompress_rle8(src, row_bytes, dst, dst_pitch, height);
		} else {
			if (pack_type == 0) {
				if (depth == 16)
					pack_type = 3;
				else if (depth == 32)
					pack_type = 4;
			}
			switch (pack_type) {
				case 1:		// No packing
					memcpy(dst, src, row_bytes * height);
					if (depth == 16)
						byte_swap_memory(dst, _2byte, dst_pitch * height / 2);
					else if (depth == 32)
						byte_swap_memory(dst, _4byte, dst_pitch * height / 4);
					break;
				case 3:		// Run-length encoding by 16-bit chunks
					uncompress_rle16(src, row_bytes, dst, dst_pitch, height);
					break;
				case 4:		// Run-length encoding one component at a time
					uncompress_rle32(src, row_bytes, dst, dst_pitch, height);
					break;
				default:
					fprintf(stderr, "Unimplemented packing type %d in PICT resource\n", pack_type);
					break;
			}
		}
	}
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
	SDL_RWops *p = SDL_RWFromMem(rsrc.GetPointer(), rsrc.GetLength());
	if (p == NULL)
		return NULL;
	SDL_RWseek(p, 10, SEEK_CUR);	// skip picSize and picRect

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
				uint16 row_bytes = SDL_ReadBE16(p) & 0x3fff;	// the upper 2 bits are flags
				uint16 top = SDL_ReadBE16(p);
				uint16 left = SDL_ReadBE16(p);
				uint16 height = SDL_ReadBE16(p) - top;
				uint16 width = SDL_ReadBE16(p) - left;
				SDL_RWseek(p, 2, SEEK_CUR);			// pmVersion
				uint16 pack_type = SDL_ReadBE16(p);
				SDL_RWseek(p, 14, SEEK_CUR);		// packSize/hRes/vRes/pixelType
				uint16 pixel_size = SDL_ReadBE16(p);
				SDL_RWseek(p, 16, SEEK_CUR);		// cmpCount/cmpSize/planeBytes/pmTable/pmReserved
				//printf(" width %d, height %d, row_bytes %d, depth %d, pack_type %d\n", width, height, row_bytes, pixel_size, pack_type);

				// Allocate surface for picture
				uint32 Rmask, Gmask, Bmask;
				switch (pixel_size) {
					case 16:
						Rmask = 0x7c00;
						Gmask = 0x03e0;
						Bmask = 0x001f;
						break;
					case 32:
						Rmask = 0x00ff0000;
						Gmask = 0x0000ff00;
						Bmask = 0x000000ff;
						break;
					default:
						Rmask = Gmask = Bmask = 0xff;
						break;
				}
				s = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, pixel_size, Rmask, Gmask, Bmask, 0);
				if (s == NULL) {
					done = true;
					break;
				}

				// 2. ColorTable
				if (opcode == 0x0098 || opcode == 0x0099) {
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
					SDL_SetColors(s, colors, 0, 256);
				}

				// 3. source/destination Rect and transfer mode
				SDL_RWseek(p, 18, SEEK_CUR);

				// 4. clipping region
				if (opcode == 0x0099 || opcode == 0x009b) {
					uint16 rgn_size = SDL_ReadBE16(p);
					SDL_RWseek(p, rgn_size - 2, SEEK_CUR);
				}

				// 5. graphics data
				uncompress_picture((uint8 *)rsrc.GetPointer() + SDL_RWtell(p), row_bytes, (uint8 *)s->pixels, s->pitch, pixel_size, height, pack_type);

				done = true;
				break;
			}

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
 *  Rescale surface
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
			rescale((uint8 *)s->pixels, s->pitch, (uint8 *)s2->pixels, s2->pitch, width, height, dx, dy);
			break;
		case 2:
			rescale((uint16 *)s->pixels, s->pitch, (uint16 *)s2->pixels, s2->pitch, width, height, dx, dy);
			break;
		case 4:
			rescale((uint32 *)s->pixels, s->pitch, (uint32 *)s2->pixels, s2->pitch, width, height, dx, dy);
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

	// Center picture on screen
	SDL_Rect dest_rect = {(SDL_GetVideoSurface()->w - s->w) / 2, (SDL_GetVideoSurface()->h - s->h) / 2, s->w, s->h};
	if (dest_rect.x < 0)
		dest_rect.x = 0;
	if (dest_rect.y < 0)
		dest_rect.y = 0;

	// Set clipping rectangle if desired
	if (draw_clip_rect_active)
		SDL_SetClipping(s, draw_clip_rect.top, draw_clip_rect.left, draw_clip_rect.bottom, draw_clip_rect.right);

	// Blit picture to screen
	SDL_BlitSurface(s, NULL, SDL_GetVideoSurface(), &dest_rect);

	// Reset clipping rectangle
	if (draw_clip_rect_active)
		SDL_SetClipping(s, 0, 0, 0, 0);

	// Update display and free picture surface
	SDL_UpdateRects(SDL_GetVideoSurface(), 1, &dest_rect);
#ifdef HAVE_OPENGL
	if (SDL_GetVideoSurface()->flags & SDL_OPENGL)
		SDL_GL_SwapBuffers();
#endif
	SDL_FreeSurface(s);
}


/*
 *  Get system color table
 */

struct color_table *build_8bit_system_color_table(void)
{
	// RGB 332 color cube
	color_table *table = new color_table;
	table->color_count = 256;
	int index = 0;
	for (int red=0; red<8; red++) {
		for (int green=0; green<8; green++) {
			for (int blue=0; blue<4; blue++) {
				int r = (red * 0x24) | (red >> 1);
				int g = (green * 0x24) | (green >> 1);
				int b = blue * 0x55;
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

			uint32 delta = (SDL_GetTicks() - start_tick) / (text_block ? (2 * SCROLLING_SPEED) : SCROLLING_SPEED);
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
