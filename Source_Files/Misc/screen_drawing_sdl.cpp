/*
 *  screen_drawing_sdl.cpp - Basic screen drawing functions, SDL implementation
 *
 *  Written in 2000 by Christian Bauer
 */

#include "cseries.h"
#include "byte_swapping.h"
#include "resource_manager.h"

#include "map.h"
#include "interface.h"
#include "shell.h"
#include "screen_drawing.h"
#include "fades.h"
#include "screen.h"
#include "ColorParser.h"
#include "FileHandler.h"

#include <SDL_endian.h>
#include <string.h>
#include <map>


// Global variables
static screen_rectangle interface_rectangles[NUMBER_OF_INTERFACE_RECTANGLES];
static SDL_Surface *draw_surface = NULL;	// Target surface for drawing commands
static SDL_Surface *old_draw_surface = NULL;

struct sdl_font_info {
	sdl_font_info() : first_character(0), last_character(0),
		ascent(0), descent(0), leading(0) {}

	uint8 first_character, last_character;
	int16 maximum_kerning;
	int16 rect_width, rect_height;
	int16 ascent, descent, leading;

	uint8 *pixmap;			// Font image (1 byte/pixel)
	int bytes_per_row;		// Bytes per row in pixmap

	uint16 *location_table;	// Table of byte-offsets into pixmap (points into resource)
	int8 *width_table;		// Table of kerning/width info (points into resource)
};

typedef pair<int, int> id_and_size_t;
typedef map<id_and_size_t, sdl_font_info *> font_list_t;
static font_list_t font_list;				// List of all loaded fonts

static struct interface_font_info {
	TextSpec spec;
	const sdl_font_info *font;
	int height;
	int line_spacing;
} interface_fonts[NUMBER_OF_INTERFACE_FONTS] = {
	{{kFontIDMonaco, styleBold, 9}},
	{{kFontIDMonaco, styleBold, 9}},
	{{kFontIDMonaco, styleBold, 9}},
	{{kFontIDMonaco, styleNormal, 9}},
	{{kFontIDCourier, styleNormal, 12}},
	{{kFontIDCourier, styleBold, 14}},
	{{kFontIDMonaco, styleNormal, 9}}
};

bool draw_clip_rect_active = false;			// Flag: clipping rect active
screen_rectangle draw_clip_rect;			// Current clipping rectangle

// From screen_sdl.cpp
extern SDL_Surface *world_pixels, *HUD_Buffer;

// Prototypes
static int _get_font_height(const sdl_font_info *info);
static int _get_font_line_spacing(const sdl_font_info *info);
extern TextSpec *_get_font_spec(short font_index);


/*
 *  Initialize drawing module
 */

void initialize_screen_drawing(void)
{
	// Init drawing surface
	draw_surface = SDL_GetVideoSurface();
	old_draw_surface = NULL;

	// Open font resources
	FileSpecifier fonts;
	fonts.SetToGlobalDataDir();
	fonts.AddPart("Fonts");
	open_res_file(fonts);

	// Init fonts
	for (int i=0; i<NUMBER_OF_INTERFACE_FONTS; i++) {
		interface_fonts[i].font = load_font(interface_fonts[i].spec);
		interface_fonts[i].height = _get_font_height(interface_fonts[i].font);
		interface_fonts[i].line_spacing = _get_font_line_spacing(interface_fonts[i].font);
	}
}


/*
 *  Redirect drawing to screen or offscreen buffer
 */

void _set_port_to_screen_window(void)
{
	assert(old_draw_surface == NULL);
	old_draw_surface = draw_surface;
	draw_surface = SDL_GetVideoSurface();
}

void _set_port_to_gworld(void)
{
	assert(old_draw_surface == NULL);
	old_draw_surface = draw_surface;
	draw_surface = world_pixels;
}

void _set_port_to_HUD(void)
{
	assert(old_draw_surface == NULL);
	old_draw_surface = draw_surface;
	draw_surface = HUD_Buffer;
}

void _restore_port(void)
{
	draw_surface = old_draw_surface;
	old_draw_surface = NULL;
}


/*
 *  Set clipping rectangle
 */

void set_drawing_clip_rectangle(short top, short left, short bottom, short right)
{
	if (top < 0)
		draw_clip_rect_active = false;
	else {
		draw_clip_rect_active = true;
		draw_clip_rect.top = top;
		draw_clip_rect.left = left;
		draw_clip_rect.bottom = bottom;
		draw_clip_rect.right = right;
	}
}


/*
 *  Draw shapes
 */

void _draw_screen_shape(shape_descriptor shape_id, screen_rectangle *destination, screen_rectangle *source)
{
	// Convert rectangles
	SDL_Rect src_rect;
	if (source) {
		src_rect.x = source->left;
		src_rect.y = source->top;
		src_rect.w = source->right - source->left;
		src_rect.h = source->bottom - source->top;
	}
	SDL_Rect dst_rect = {destination->left, destination->top, destination->right - destination->left, destination->bottom - destination->top};

	// Convert shape to surface
	SDL_Surface *s = get_shape_surface(shape_id);
	if (s == NULL)
		return;

	// Blit the surface
	SDL_BlitSurface(s, source ? &src_rect : NULL, draw_surface, &dst_rect);
	if (draw_surface == SDL_GetVideoSurface())
		SDL_UpdateRects(draw_surface, 1, &dst_rect);

	// Free the surface
	SDL_FreeSurface(s);
}

void _draw_screen_shape_at_x_y(shape_descriptor shape_id, short x, short y)
{
	// Convert shape to surface
	SDL_Surface *s = get_shape_surface(shape_id);
	if (s == NULL)
		return;

	// Setup destination rectangle
	SDL_Rect dst_rect = {x, y, s->w, s->h};

	// Blit the surface
	SDL_BlitSurface(s, NULL, draw_surface, &dst_rect);
	if (draw_surface == SDL_GetVideoSurface())
		SDL_UpdateRects(draw_surface, 1, &dst_rect);

	// Free the surface
	SDL_FreeSurface(s);
}


/*
 *  Draw text
 */

// Calculate width of single character
int char_width(uint8 c, const sdl_font_info *font, uint16 style)
{
	if (font == NULL || c < font->first_character || c > font->last_character)
		return 0;
	int width = font->width_table[(c - font->first_character) * 2 + 1] + ((style & styleBold) ? 1 : 0);
	if (width == -1)	// non-existant character
		width = font->width_table[(font->last_character - font->first_character + 1) * 2 + 1] + ((style & styleBold) ? 1 : 0);
	return width;
}

// Calculate width of text string
int text_width(const char *text, const sdl_font_info *font, uint16 style)
{
	int width = 0;
	char c;
	while ((c = *text++) != 0)
		width += char_width(c, font, style);
	return width;
}

int text_width(const char *text, int length, const sdl_font_info *font, uint16 style)
{
	int width = 0;
	while (length--)
		width += char_width(*text++, font, style); 
	return width;
}

// Determine how many characters of a string fit into a given width
static int trunc_text(char *text, int max_width, const sdl_font_info *font, uint16 style)
{
	int width = 0;
	int num = 0;
	char c;
	while ((c = *text++) != 0) {
		width += char_width(c, font, style);
		if (width > max_width)
			break;
		num++;
	}
	return num;
}

// Draw single glyph at given position in frame buffer, return glyph width
template <class T>
inline static int draw_glyph(uint8 c, int x, int y, T *p, int pitch, int clip_left, int clip_top, int clip_right, int clip_bottom, uint32 pixel, const sdl_font_info *font, bool oblique)
{
	int cpos = c - font->first_character;

	// Calculate source and destination pointers (kerning, ascent etc.)
	uint8 *src = font->pixmap + font->location_table[cpos];
	int width = font->location_table[cpos + 1] - font->location_table[cpos];
	int height = font->rect_height;
	int advance = font->width_table[cpos * 2 + 1];
	y -= font->ascent;
	x += font->maximum_kerning + font->width_table[cpos * 2];
	p += y * pitch / sizeof(T) + x;
	if (oblique)
		p += font->ascent / 2 - 1;

	// Clip on top
	if (y < clip_top) {
		height -= clip_top - y;
		if (height <= 0)
			return advance;
		p += (clip_top - y) * pitch / sizeof(T);
		src += (clip_top - y) * font->bytes_per_row;
	}

	// Clip on bottom
	if (y + height - 1 > clip_bottom) {
		height -= y + height - 1 - clip_bottom;
		if (height <= 0)
			return advance;
	}

	// Clip on left
	if (x < clip_left) {
		width -= (clip_left - x);
		if (width <= 0)
			return advance;
		p += (clip_left - x);
		src += (clip_left - x);
	}

	// Clip on right
	if (x + width - 1 > clip_right) {
		width -= x + width - 1 - clip_right;
		if (width <= 0)
			return advance;
	}

	// Blit glyph to screen
	for (int iy=0; iy<height; iy++) {
		for (int ix=0; ix<width; ix++) {
			if (src[ix])
				p[ix] = pixel;			
		}
		if (oblique && (iy & 1))
			p--;
		src += font->bytes_per_row;
		p += pitch / sizeof(T);
	}

	return advance;
}

// Draw text at given position in frame buffer, return width
template <class T>
inline static int draw_text(const uint8 *text, int length, int x, int y, T *p, int pitch, int clip_left, int clip_top, int clip_right, int clip_bottom, uint32 pixel, const sdl_font_info *font, uint16 style)
{
	bool oblique = style & styleItalic;
	int total_width = 0;

	uint8 c;
	while (length--) {
		c = *text++;
		if (c < font->first_character || c > font->last_character)
			continue;

		int width = draw_glyph(c, x, y, p, pitch, clip_left, clip_top, clip_right, clip_bottom, pixel, font, oblique);
		if (style & styleBold) {
			draw_glyph(c, x + 1, y, p, pitch, clip_left, clip_top, clip_right, clip_bottom, pixel, font, oblique);
			width++;
		}
		if (style & styleUnderline) {
			for (int i=0; i<width; i++)
				p[y * pitch / sizeof(T) + x + i] = pixel;
		}

		total_width += width;
		x += width;
	}
	return total_width;
}

// Draw text at given coordinates, return total width
int draw_text(SDL_Surface *s, const char *text, int length, int x, int y, uint32 pixel, const sdl_font_info *font, uint16 style)
{
	if (font == NULL)
		return 0;

	// Get clipping rectangle
	int clip_top, clip_bottom, clip_left, clip_right;
	if (draw_clip_rect_active) {
		clip_top = draw_clip_rect.top;
		clip_bottom = draw_clip_rect.bottom - 1;
		clip_left = draw_clip_rect.left;
		clip_right = draw_clip_rect.right - 1;
	} else {
		clip_top = clip_left = 0;
		clip_right = s->w - 1;
		clip_bottom = s->h - 1;
	}

	int width = 0;
	switch (s->format->BytesPerPixel) {
		case 1:
			width = draw_text((const uint8 *)text, length, x, y, (uint8 *)s->pixels, s->pitch, clip_left, clip_top, clip_right, clip_bottom, pixel, font, style);
			break;
		case 2:
			width = draw_text((const uint8 *)text, length, x, y, (uint16 *)s->pixels, s->pitch, clip_left, clip_top, clip_right, clip_bottom, pixel, font, style);
			break;
		case 4:
			width = draw_text((const uint8 *)text, length, x, y, (uint32 *)s->pixels, s->pitch, clip_left, clip_top, clip_right, clip_bottom, pixel, font, style);
			break;
	}
	if (s == SDL_GetVideoSurface())
		SDL_UpdateRect(s, x, y - font->ascent, text_width(text, font, style), font->rect_height);
	return width;
}

static void draw_text(const char *text, int x, int y, uint32 pixel, const sdl_font_info *font, uint16 style)
{
	draw_text(draw_surface, text, strlen(text), x, y, pixel, font, style);
}

void _draw_screen_text(const char *text, screen_rectangle *destination, short flags, short font_id, short text_color)
{
	int x, y;

	// Find font information
	assert(font_id >= 0 && font_id < NUMBER_OF_INTERFACE_FONTS);
	uint16 style = interface_fonts[font_id].spec.style;
	const sdl_font_info *font = interface_fonts[font_id].font;
	if (font == NULL)
		return;

	// Get color
	SDL_Color color;
	_get_interface_color(text_color, &color);

	// Copy the text to draw
	char text_to_draw[256];
	strcpy(text_to_draw, text);

	// Check for wrapping, and if it occurs, be recursive
	if (flags & _wrap_text) {
		int last_non_printing_character = 0, text_width = 0, count = 0;
		while (count < strlen(text_to_draw) && text_width < RECTANGLE_WIDTH(destination)) {
			text_width += char_width(text_to_draw[count], font, style);
			if (text_to_draw[count] == ' ')
				last_non_printing_character = count;
			count++;
		}
		
		if( count != strlen(text_to_draw)) {
			char remaining_text_to_draw[256];
			screen_rectangle new_destination;
			
			// If we ever have to wrap text, we can't also center vertically. Sorry.
			flags &= ~_center_vertical;
			flags |= _top_justified;
			
			// Pass the rest of it back in, recursively, on the next line
			memcpy(remaining_text_to_draw, text_to_draw + last_non_printing_character + 1, strlen(text_to_draw + last_non_printing_character + 1) + 1);
	
			new_destination = *destination;
			new_destination.top += interface_fonts[font_id].line_spacing;
			_draw_screen_text(remaining_text_to_draw, &new_destination, flags, font_id, text_color);
	
			// Now truncate our text to draw
			text_to_draw[last_non_printing_character] = 0;
		}
	}

	// Truncate text if necessary
	int t_width = text_width(text_to_draw, font, style);
	if (t_width > RECTANGLE_WIDTH(destination)) {
		text_to_draw[trunc_text(text_to_draw, RECTANGLE_WIDTH(destination), font, style)] = 0;
		t_width = text_width(text_to_draw, font, style);
	}

	// Horizontal positioning
	if (flags & _center_horizontal)
		x = destination->left + (((destination->right - destination->left) - t_width) / 2);
	else if (flags & _right_justified)
		x = destination->right - t_width;
	else
		x = destination->left;

	// Vertical positioning
	int t_height = interface_fonts[font_id].height;
	if (flags & _center_vertical) {
		if (t_height > RECTANGLE_HEIGHT(destination))
			y = destination->top;
		else {
			y = destination->bottom;
			int offset = RECTANGLE_HEIGHT(destination) - t_height;
			y -= (offset / 2) + (offset & 1) + 1;
		}
	} else if (flags & _top_justified) {
		if (t_height > RECTANGLE_HEIGHT(destination))
			y = destination->bottom;
		else
			y = destination->top + t_height;
	} else
		y = destination->bottom;

	// Now draw it
	draw_text(text_to_draw, x, y, SDL_MapRGB(draw_surface->format, color.r, color.g, color.b), font, style);
}

// Load font from resources and allocate sdl_font_info
const sdl_font_info *load_font(const TextSpec &spec)
{
	sdl_font_info *info = NULL;

	// Look for ID/size in list of loaded fonts
	id_and_size_t id_and_size(spec.font, spec.size);
	font_list_t::const_iterator it = font_list.find(id_and_size);
	if (it != font_list.end())
		return it->second;	// already loaded

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
			LoadedResource font;
			if (!get_resource(FOUR_CHARS_TO_INT('N', 'F', 'N', 'T'), id, font))
				get_resource(FOUR_CHARS_TO_INT('F', 'O', 'N', 'T'), id, font);
			if (font.IsLoaded()) {

				// Found, switch stream to font resource
				SDL_RWclose(p);
				p = SDL_RWFromMem(font.GetPointer(), font.GetLength());
				assert(p);
				void *font_ptr = font.GetPointer(true);

				// Read font information
				info = new sdl_font_info;
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
				font_list[id_and_size] = info;
			}
		}
	}

	// Free resources
	SDL_RWclose(p);
	return info;
}

TextSpec *_get_font_spec(short font_id)
{
	return &interface_fonts[font_id].spec;
}

short _get_font_line_height(short font_id)
{
	assert(font_id >= 0 && font_id < NUMBER_OF_INTERFACE_FONTS);
	return interface_fonts[font_id].line_spacing;
}

static int _get_font_height(const sdl_font_info *info)
{
	return info == NULL ? 0 : info->ascent + info->leading;
}

static int _get_font_line_spacing(const sdl_font_info *info)
{
	return info == NULL ? 0 : info->ascent + info->descent + info->leading;
}

int font_line_height(const sdl_font_info *info)
{
	return info == NULL ? 0 : info->ascent + info->descent + info->leading;
}

int font_width(const sdl_font_info *info)
{
	return info == NULL ? 0 : info->rect_width;
}

int font_ascent(const sdl_font_info *info)
{
	return info == NULL ? 0 : info->ascent;
}

short _text_width(const char *text, short font_id)
{
	// Find font information
	assert(font_id >= 0 && font_id < NUMBER_OF_INTERFACE_FONTS);
	uint16 style = interface_fonts[font_id].spec.style;
	const sdl_font_info *font = interface_fonts[font_id].font;
	if (font == NULL)
		return 0;

	// Calculate width
	return text_width(text, font, style);
}


/*
 *  Draw rectangle
 */

void _fill_rect(screen_rectangle *rectangle, short color_index)
{
	// Convert source rectangle
	SDL_Rect r;
	if (rectangle) {
		r.x = rectangle->left;
		r.y = rectangle->top;
		r.w = rectangle->right - rectangle->left;
		r.h = rectangle->bottom - rectangle->top;
	}

	// Get color
	SDL_Color color;
	_get_interface_color(color_index, &color);

	// Fill rectangle
	SDL_FillRect(draw_surface, rectangle ? &r : NULL, SDL_MapRGB(draw_surface->format, color.r, color.g, color.b));
	if (draw_surface == SDL_GetVideoSurface()) {
		if (rectangle)
			SDL_UpdateRects(draw_surface, 1, &r);
		else
			SDL_UpdateRect(draw_surface, 0, 0, 0, 0);
	}
}

void _fill_screen_rectangle(screen_rectangle *rectangle, short color_index)
{
	_fill_rect(rectangle, color_index);
}

void draw_rectangle(SDL_Surface *s, const SDL_Rect *rectangle, uint32 pixel)
{
	bool do_update = (s == SDL_GetVideoSurface());
	SDL_Rect r = {rectangle->x, rectangle->y, rectangle->w, 1};
	SDL_FillRect(s, &r, pixel);
	if (do_update)
		SDL_UpdateRects(s, 1, &r);
	r.y += rectangle->h - 1;
	SDL_FillRect(s, &r, pixel);
	if (do_update)
		SDL_UpdateRects(s, 1, &r);
	r.y = rectangle->y;
	r.w = 1;
	r.h = rectangle->h;
	SDL_FillRect(s, &r, pixel);
	if (do_update)
		SDL_UpdateRects(s, 1, &r);
	r.x += rectangle->w - 1;
	SDL_FillRect(s, &r, pixel);
	if (do_update)
		SDL_UpdateRects(s, 1, &r);
}

void _frame_rect(screen_rectangle *rectangle, short color_index)
{
	// Get color
	SDL_Color color;
	_get_interface_color(color_index, &color);
	uint32 pixel = SDL_MapRGB(draw_surface->format, color.r, color.g, color.b);

	// Draw rectangle
	SDL_Rect r = {rectangle->left, rectangle->top, rectangle->right - rectangle->left, rectangle->bottom - rectangle->top};
	draw_rectangle(draw_surface, &r, pixel);
}

void _erase_screen(short color_index)
{
	_fill_rect(NULL, color_index);
}


/*
 *  Move rectangle
 */

void _offset_screen_rect(screen_rectangle *rect, short dx, short dy)
{
	rect->top += dy;
	rect->left += dx;
	rect->bottom += dy;
	rect->right += dx;
}


/*
 *  Draw line
 */

static inline uint8 cs_code(const world_point2d *p, int clip_top, int clip_bottom, int clip_left, int clip_right)
{
	uint8 code = 0;
	if (p->x < clip_left)
		code |= 1;
	if (p->x > clip_right)
		code |= 2;
	if (p->y < clip_top)
		code |= 4;
	if (p->y > clip_bottom)
		code |= 8;
	return code;
}

template <class T>
static inline void draw_thin_line_noclip(T *p, int pitch, const world_point2d *v1, const world_point2d *v2, uint32 pixel)
{
	int xdelta = v2->x - v1->x;
	int ydelta = v2->y - v1->y;

	if (abs(xdelta) > ydelta) {	// X axis is major axis
		int32 y = v1->y << 16;
		int32 delta = (xdelta == 0 ? 0 : (ydelta << 16) / xdelta);
		int x = v1->x;
		p += v1->x;
		if (xdelta < 0) {		// Line going left
			while (true) {
				p[(y >> 16) * pitch / sizeof(T)] = pixel;
				if (x == v2->x)
					break;
				x--;
				p--;
				y -= delta;
			}
		} else {
			while (true) {
				p[(y >> 16) * pitch / sizeof(T)] = pixel;
				if (x == v2->x)
					break;
				x++;
				p++;
				y += delta;
			}
		}
	} else {					// Y axis is major axis
		int32 x = v1->x << 16;
		int32 delta = (ydelta == 0 ? 0 : (xdelta << 16) / ydelta);
		p += v1->y * pitch / sizeof(T);
		int y = v1->y;
		while (true) {
			p[x >> 16] = pixel;
			if (y == v2->y)
				break;
			y++;
			x += delta;
			p += pitch / sizeof(T);
		}
	}
}

void draw_line(SDL_Surface *s, const world_point2d *v1, const world_point2d *v2, uint32 pixel, int pen_size)
{
	// Make line going downwards
	if (v1->y > v2->y) {
		const world_point2d *tmp = v1;
		v1 = v2;
		v2 = tmp;
	}

	if (pen_size == 1) {

		// Thin line, clip with Cohen/Sutherland and draw with DDA

		// Get clipping rectangle
		int clip_top, clip_bottom, clip_left, clip_right;
		if (draw_clip_rect_active) {
			clip_top = draw_clip_rect.top;
			clip_bottom = draw_clip_rect.bottom - 1;
			clip_left = draw_clip_rect.left;
			clip_right = draw_clip_rect.right - 1;
		} else {
			clip_top = clip_left = 0;
			clip_right = s->w - 1;
			clip_bottom = s->h - 1;
		}

		// Get codes for start/end points
		uint8 code1 = cs_code(v1, clip_top, clip_bottom, clip_left, clip_right);
		uint8 code2 = cs_code(v2, clip_top, clip_bottom, clip_left, clip_right);

		world_point2d clip_start, clip_end;

clip_line:
		if ((code1 | code2) == 0) {

			// Line completely visible, draw it
			switch (s->format->BytesPerPixel) {
				case 1:
					draw_thin_line_noclip((uint8 *)s->pixels, s->pitch, v1, v2, pixel);
					break;
				case 2:
					draw_thin_line_noclip((uint16 *)s->pixels, s->pitch, v1, v2, pixel);
					break;
				case 4:
					draw_thin_line_noclip((uint32 *)s->pixels, s->pitch, v1, v2, pixel);
					break;
			}

		} else if ((code1 & code2) == 0) {

			// Line partially visible, clip it
#define clipx(p, clip, v, code) \
	p.y = v1->y + (v2->y - v1->y) * (clip - v1->x) / (v2->x - v1->x); \
	p.x = clip; \
	v = &p; \
	if (p.y < clip_top) \
		code = 4; \
	else if (p.y > clip_bottom) \
		code = 8; \
	else \
		code = 0;

#define clipy(p, clip, v, code) \
	p.x = v1->x + (v2->x - v1->x) * (clip - v1->y) / (v2->y - v1->y); \
	p.y = clip; \
	v = &p; \
	if (p.x < clip_left) \
		code = 1; \
	else if (p.x > clip_right) \
		code = 2; \
	else \
		code = 0;

			if (code1) {					// Clip start point
				if (code1 & 1) {			// Left
					clipx(clip_start, clip_left, v1, code1);
				} else if (code1 & 2) {		// Right
					clipx(clip_start, clip_right, v1, code1);
				} else {					// Top (bottom can't happen because the line goes downwards)
					clipy(clip_start, clip_top, v1, code1);
				}
			} else {			 			// Clip end point
				if (code2 & 1) {			// Left
					clipx(clip_end, clip_left, v2, code2);
				} else if (code2 & 2) {		// Right
					clipx(clip_end, clip_right, v2, code2);
				} else {					// Bottom (top can't happen because the line goes downwards)
					clipy(clip_end, clip_bottom, v2, code2);
				}
			}

			goto clip_line;
		}

	} else {

		// Thick line: to emulate the QuickDraw behaviour of moving a
		// rectangular pen along a line, we convert the line into a hexagon

		// Calculate hexagon points
		world_point2d hexagon[6];
		hexagon[0].x = v1->x - pen_size / 2;
		hexagon[1].x = hexagon[0].x + pen_size - 1;
		hexagon[0].y = hexagon[1].y = v1->y - pen_size / 2;
		hexagon[4].x = v2->x - pen_size / 2;
		hexagon[3].x = hexagon[4].x + pen_size - 1;
		hexagon[3].y = hexagon[4].y = v2->y - pen_size / 2 + pen_size - 1;
		if (v1->x > v2->x) {	// Line going to the left
			hexagon[2].x = hexagon[1].x;
			hexagon[2].y = hexagon[1].y + pen_size - 1;
			hexagon[5].x = hexagon[4].x;
			hexagon[5].y = hexagon[4].y - pen_size + 1;
			if (v1->x - v2->y > v2->y - v1->y)	// Pixels missing from polygon filler
				draw_line(s, hexagon + 0, hexagon + 5, pixel, 1);
		} else {				// Line going to the right
			hexagon[2].x = hexagon[3].x;
			hexagon[2].y = hexagon[3].y - pen_size + 1;
			hexagon[5].x = hexagon[0].x;
			hexagon[5].y = hexagon[0].y + pen_size - 1;
			if (v2->x - v1->y > v2->y - v1->y)	// Pixels missing from polygon filler
				draw_line(s, hexagon + 1, hexagon + 2, pixel, 1);
		}

		// Draw hexagon
		draw_polygon(s, hexagon, 6, pixel);
	}
}


/*
 *  Draw clipped, filled, convex polygon
 */

void draw_polygon(SDL_Surface *s, const world_point2d *vertex_array, int vertex_count, uint32 pixel)
{
	if (vertex_count == 0)
		return;

	// Reallocate temporary vertex lists if necessary
	static world_point2d *va1 = NULL, *va2 = NULL;
	static int max_vertices = 0;
	if (vertex_count > max_vertices) {
		delete[] va1;
		delete[] va2;
		va1 = new world_point2d[vertex_count * 2];	// During clipping, each vertex can become two vertices
		va2 = new world_point2d[vertex_count * 2];
		max_vertices = vertex_count;
	}

	// Get clipping rectangle
	int clip_top, clip_bottom, clip_left, clip_right;
	if (draw_clip_rect_active) {
		clip_top = draw_clip_rect.top;
		clip_bottom = draw_clip_rect.bottom - 1;
		clip_left = draw_clip_rect.left;
		clip_right = draw_clip_rect.right - 1;
	} else {
		clip_top = clip_left = 0;
		clip_right = s->w - 1;
		clip_bottom = s->h - 1;
	}

	// Clip polygon
	const world_point2d *v1, *v2;
	world_point2d *vp;
	world_point2d clip_point;
	int new_vertex_count;

#define clip_min(X, Y, clip, dst_array) \
	clip_point.Y = clip; \
	v1 = vertex_array + vertex_count - 1; \
	v2 = vertex_array; \
	vp = dst_array; \
	new_vertex_count = 0; \
	for (int i=0; i<vertex_count; i++, v1 = v2, v2++) { \
		if (v1->Y < clip) { \
			if (v2->Y < clip) { 		/* Edge completely clipped */ \
				continue; \
			} else {		 			/* Clipped edge going down, find clip point */ \
				clip_point.X = v1->X + (v2->X - v1->X) * (clip - v1->Y) / (v2->Y - v1->Y); \
				*vp++ = clip_point;		/* Add clip point to array */ \
				*vp++ = *v2;			/* Add visible endpoint to array */ \
				new_vertex_count += 2; \
			} \
		} else { \
			if (v2->Y < clip) {			/* Clipped edge going up, find clip point */ \
				clip_point.X = v2->X + (v1->X - v2->X) * (clip - v2->Y) / (v1->Y - v2->Y); \
				*vp++ = clip_point;		/* Add clip point to array */ \
				new_vertex_count++; \
			} else {					/* Edge completely visible, add endpoint to array */ \
				*vp++ = *v2; \
				new_vertex_count++; \
			} \
		} \
	} \
	vertex_count = new_vertex_count; \
	if (vertex_count == 0) \
		return;		/* Polygon completely clipped */ \
	vertex_array = dst_array;

#define clip_max(X, Y, clip, dst_array) \
	clip_point.Y = clip; \
	v1 = vertex_array + vertex_count - 1; \
	v2 = vertex_array; \
	vp = dst_array; \
	new_vertex_count = 0; \
	for (int i=0; i<vertex_count; i++, v1 = v2, v2++) { \
		if (v1->Y < clip) { \
			if (v2->Y < clip) {			/* Edge completely visible, add endpoint to array */ \
				*vp++ = *v2; \
				new_vertex_count++; \
			} else {		 			/* Clipped edge going down, find clip point */ \
				clip_point.X = v1->X + (v2->X - v1->X) * (clip - v1->Y) / (v2->Y - v1->Y); \
				*vp++ = clip_point;		/* Add clip point to array */ \
				new_vertex_count++; \
			} \
		} else { \
			if (v2->Y < clip) {			/* Clipped edge going up, find clip point */ \
				clip_point.X = v2->X + (v1->X - v2->X) * (clip - v2->Y) / (v1->Y - v2->Y); \
				*vp++ = clip_point;		/* Add clip point to array */ \
				*vp++ = *v2;			/* Add visible endpoint to array */ \
				new_vertex_count += 2; \
			} else {					/* Edge completely clipped */ \
				continue; \
			} \
		} \
	} \
	vertex_count = new_vertex_count; \
	if (vertex_count == 0) \
		return;		/* Polygon completely clipped */ \
	vertex_array = dst_array;

	clip_min(x, y, clip_top, va1);
	clip_max(x, y, clip_bottom, va2);
	clip_min(y, x, clip_left, va1);
	clip_max(y, x, clip_right, va2);

	// Reallocate span list if necessary
	struct span_t {
		int left, right;
	};
	static span_t *span = NULL;
	static int max_spans = 0;
	if (s->h > max_spans) {
		delete[] span;
		span = new span_t[s->h];
		max_spans = s->h;
	}

	// Scan polygon edges and build span list
	v1 = vertex_array + vertex_count - 1;
	v2 = vertex_array;
	int xmin = INT16_MAX, xmax = INT16_MIN;
	int ymin = INT16_MAX, ymax = INT16_MIN;
	for (int i=0; i<vertex_count; i++, v1 = v2, v2++) {

		if (v1->x < xmin)	// Find minimum and maximum coordinates
			xmin = v1->x;
		if (v1->x > xmax)
			xmax = v1->x;
		if (v1->y < ymin)
			ymin = v1->y;
		if (v1->y > ymax)
			ymax = v1->y;

		int x1x2 = v1->x - v2->x;
		int y1y2 = v1->y - v2->y;

		if (y1y2 == 0)				// Horizontal edge
			continue;
		else if (y1y2 < 0) {		// Edge going down -> left span boundary
			int32 x = v1->x << 16;	// 16.16 fixed point
			int32 delta = (x1x2 << 16) / y1y2;
			for (int y=v1->y; y<=v2->y; y++) {
				span[y].left = x >> 16;
				x += delta;			// DDA line drawing
			}
		} else {					// Edge going up -> right span boundary
			int32 x = v2->x << 16;
			int32 delta = (x1x2 << 16) / y1y2;
			for (int y=v2->y; y<=v1->y; y++) {
				span[y].right = x >> 16;
				x += delta;			// Draw downwards to ensure that adjacent polygon fits perfectly
			}
		}
	}

	// Fill spans
	SDL_Rect r = {0, 0, 0, 1};
	for (int y=ymin; y<=ymax; y++) {
		int left = span[y].left, right = span[y].right;
		if (left == right)
			continue;
		else if (left < right) {
			r.x = left;
			r.y = y;
			r.w = right - r.x + 1;
		} else {
			r.x = right;
			r.y = y;
			r.w = left - r.x + 1;
		}
		SDL_FillRect(s, &r, pixel);
	}

	if (draw_surface == SDL_GetVideoSurface())
		SDL_UpdateRect(draw_surface, xmin, ymin, xmax - xmin + 1, ymax - ymin + 1);
}


/*
 *  Interface color management
 */

const int NumInterfaceColors = 26;

static rgb_color InterfaceColors[NumInterfaceColors] = 
{
	{0, 65535, 0},
	{0, 5140, 0},
	{0, 0, 0},
	
	{0, 65535, 0},
	{0, 12956, 0},
	{0, 5100, 0},
	
	{9216, 24320, 41728},
	{65535, 0, 0},
	{45056, 0, 24064},
	{65535, 65535, 0},
	{60000, 60000, 60000},
	{62976, 22528, 0},
	{3072, 0, 65535},
	{0, 65535, 0},
	
	{65535, 65535, 65535},
	{0, 5140, 0},
	
	{10000, 0, 0},
	{65535, 0, 0},
	
	{0, 65535, 0},
	{65535, 65535, 65535},
	{65535, 0, 0},
	{0, 40000, 0},
	{0, 45232, 51657},
	{65535, 59367, 0},
	{45000, 0, 0},
	{3084, 0, 65535}
};

void _get_interface_color(int color_index, SDL_Color *color)
{	
	assert(color_index>=0 && color_index<NumInterfaceColors);
	
	rgb_color &c = InterfaceColors[color_index];
	color->r = c.red >> 8;
	color->g = c.green >> 8;
	color->b = c.blue >> 8;
}

#define NUMBER_OF_PLAYER_COLORS 8

void _get_player_color(short color_index, RGBColor *color)
{
	assert(color_index>=0 && color_index<NUMBER_OF_PLAYER_COLORS);

	rgb_color &c = InterfaceColors[color_index + PLAYER_COLOR_BASE_INDEX];
	color->red = c.red;
	color->green = c.green;
	color->blue = c.blue;
}


/*
 *  Rectangle XML parser
 */

screen_rectangle *get_interface_rectangle(short index)
{
	assert(index>=0 && index<NUMBER_OF_INTERFACE_RECTANGLES);
	return interface_rectangles + index;
}

// Rectangle-parser object:
class XML_RectangleParser: public XML_ElementParser
{
	screen_rectangle TempRect;
	int Index;
	bool IsPresent[5];

public:
	screen_rectangle *RectList;
	int NumRectangles;
	
	bool Start();
	bool HandleAttribute(const char *Tag, const char *Value);
	bool AttributesDone();
	
	XML_RectangleParser(): XML_ElementParser("rect") {}
};

bool XML_RectangleParser::Start()
{
	for (int k=0; k<5; k++)
		IsPresent[k] = false;
	return true;
}

bool XML_RectangleParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (strcmp(Tag,"index") == 0)
	{
		if (ReadBoundedNumericalValue(Value,"%d",Index,0,NumRectangles-1))
		{
			IsPresent[4] = true;
			return true;
		}
		else return false;
	}
	else if (strcmp(Tag,"top") == 0)
	{
		if (ReadNumericalValue(Value,"%hd",TempRect.top))
		{
			IsPresent[0] = true;
			return true;
		}
		else return false;
	}
	else if (strcmp(Tag,"left") == 0)
	{
		if (ReadNumericalValue(Value,"%hd",TempRect.left))
		{
			IsPresent[1] = true;
			return true;
		}
		else return false;
	}
	else if (strcmp(Tag,"bottom") == 0)
	{
		if (ReadNumericalValue(Value,"%hd",TempRect.bottom))
		{
			IsPresent[2] = true;
			return true;
		}
		else return false;
	}
	else if (strcmp(Tag,"right") == 0)
	{
		if (ReadNumericalValue(Value,"%hd",TempRect.right))
		{
			IsPresent[3] = true;
			return true;
		}
		else return false;
	}
	UnrecognizedTag();
	return false;
}

bool XML_RectangleParser::AttributesDone()
{
	// Verify...
	bool AllPresent = true;
	for (int k=0; k<5; k++)
		if (!IsPresent[k]) AllPresent = false;
	
	if (!AllPresent)
	{
		AttribsMissing();
		return false;
	}
	
	// Put into place
	RectList[Index] = TempRect;
	return true;
}

static XML_RectangleParser RectangleParser;

// LP addition: set up the parser for the interface rectangles and get it
XML_ElementParser *InterfaceRectangles_GetParser()
{
	RectangleParser.RectList = interface_rectangles;
	RectangleParser.NumRectangles = NUMBER_OF_INTERFACE_RECTANGLES;
	return &RectangleParser;
}


void SetColorParserToScreenDrawing()
{
	Color_SetArray(InterfaceColors, NumInterfaceColors);
}
