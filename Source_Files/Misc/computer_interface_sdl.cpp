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
 *  computer_interface_sdl.cpp - Terminal handling, SDL specific stuff
 *
 *  Written in 2000 by Christian Bauer
 */

#include "sdl_fonts.h"


// Global variables
// static const sdl_font_info *terminal_font = NULL;
static uint32 current_pixel;				// Current color pixel value
static uint16 current_style = styleNormal;	// Current style flags

// From screen_sdl.cpp
extern SDL_Surface *world_pixels;


// Terminal key definitions
static struct terminal_key terminal_keys[]= {
	{SDLK_UP, 0, 0, _terminal_page_up},				// arrow up
	{SDLK_DOWN, 0, 0, _terminal_page_down},			// arrow down
	{SDLK_PAGEUP, 0, 0, _terminal_page_up},			// page up
	{SDLK_PAGEDOWN, 0, 0, _terminal_page_down},		// page down
	{SDLK_TAB, 0, 0, _terminal_next_state},			// tab
	{SDLK_KP_ENTER, 0, 0, _terminal_next_state},	// enter
	{SDLK_RETURN, 0, 0, _terminal_next_state},		// return
	{SDLK_SPACE, 0, 0, _terminal_next_state},		// space
	{SDLK_ESCAPE, 0, 0, _any_abort_key_mask}		// escape
};


// Emulation of MacOS functions
static void SetRect(Rect *r, int left, int top, int right, int bottom)
{
	r->top = top;
	r->left = left;
	r->bottom = bottom;
	r->right = right;
}

static void InsetRect(Rect *r, int dx, int dy)
{
	r->top += dy;
	r->left += dx;
	r->bottom -= dy;
	r->right -= dx;
}

static void OffsetRect(Rect *r, int dx, int dy)
{
	r->top += dy;
	r->left += dx;
	r->bottom += dy;
	r->right += dx;
}


static void	set_text_face(struct text_face_data *text_face)
{
	current_style = styleNormal;

	// Set style
	if (text_face->face & _bold_text)
		current_style |= styleBold;
	if (text_face->face & _italic_text)
		current_style |= styleItalic;
	if (text_face->face & _underline_text)
		current_style |= styleUnderline;

	// Set color
	SDL_Color color;
	_get_interface_color(text_face->color + _computer_interface_text_color, &color);
	current_pixel = SDL_MapRGB(world_pixels->format, color.r, color.g, color.b);
}


static bool calculate_line(char *base_text, short width, short start_index, short text_end_index, short *end_index)
{
	bool done = false;

	if (base_text[start_index]) {
		int index = start_index, running_width = 0;
		
		// terminal_font no longer a global, since it may change
		sdl_font_info *terminal_font = GetInterfaceFont(_computer_interface_font);

		while (running_width < width && base_text[index] && base_text[index] != MAC_LINE_END) {
			running_width += char_width(base_text[index], terminal_font, current_style);
			index++;
		}
		
		// Now go backwards, looking for whitespace to split on
		if (base_text[index] == MAC_LINE_END)
			index++;
		else if (base_text[index]) {
			int break_point = index;

			while (break_point>start_index) {
				if (base_text[break_point] == ' ')
					break; 	// Non printing
				break_point--;	// this needs to be in front of the test
			}
			
			if (break_point != start_index)
				index = break_point+1;	// Space at the end of the line
		}
		
		*end_index= index;
	} else
		done = true;
	
	return done;
}
