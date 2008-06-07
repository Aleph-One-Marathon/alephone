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
 *  HUDRenderer_SW.cpp - HUD rendering using graphics functions from screen_drawing
 *
 *  Written in 2001 by Christian Bauer
 */

#include "HUDRenderer_SW.h"

#include "images.h"
#include "shell.h" // get_shape_surface!?

#if defined(__WIN32__) || defined(__MINGW32__)
#undef DrawText
#endif

extern bool MotionSensorActive;


/*
 *  Update motion sensor
 */

void HUD_SW_Class::update_motion_sensor(short time_elapsed)
{
	if (!(GET_GAME_OPTIONS() & _motion_sensor_does_not_work) && MotionSensorActive) {
		if (time_elapsed == NONE) {
			reset_motion_sensor(current_player_index);
			ForceUpdate = true;
		}

		motion_sensor_scan(time_elapsed);
		
		if (motion_sensor_has_changed()) {
			ForceUpdate = true;
			screen_rectangle *r = get_interface_rectangle(_motion_sensor_rect);
			DrawShapeAtXY(BUILD_DESCRIPTOR(_collection_interface, _motion_sensor_mount), r->left, r->top);
		}
	}
}


/*
 *  Draw shapes
 */

void HUD_SW_Class::DrawShape(shape_descriptor shape, screen_rectangle *dest, screen_rectangle *src)
{
	_draw_screen_shape(shape, dest, src);
}

void HUD_SW_Class::DrawShapeAtXY(shape_descriptor shape, short x, short y, bool transparency)
{
	// "transparency" is only used for OpenGL motion sensor
	_draw_screen_shape_at_x_y(shape, x, y);
}

extern SDL_Surface *HUD_Buffer;

template <class T>
static void rotate(T *src_pixels, int src_pitch, T *dst_pixels, int dst_pitch, int width, int height)
{
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			dst_pixels[x * dst_pitch + y] = src_pixels[y * src_pitch + x];
		}
	}
}

SDL_Surface *rotate_surface(SDL_Surface *s, int width, int height)
{
	if (!s) return 0;

	SDL_Surface *s2 = SDL_CreateRGBSurface(SDL_SWSURFACE, height, width, s->format->BitsPerPixel, s->format->Rmask, s->format->Gmask, s->format->Bmask, s->format->Amask);

	switch (s->format->BytesPerPixel) {
		case 1:
			rotate((pixel8 *)s->pixels, s->pitch, (pixel8 *)s2->pixels, s2->pitch, width, height);
			break;
		case 2:
			rotate((pixel16 *)s->pixels, s->pitch / 2, (pixel16 *)s2->pixels, s2->pitch / 2, width, height);
			break;
		case 4:
			rotate((pixel32 *)s->pixels, s->pitch / 4, (pixel32 *)s2->pixels, s2->pitch / 4, width, height);
			break;
	}

	if (s->format->palette)
		SDL_SetColors(s2, s->format->palette->colors, 0, s->format->palette->ncolors);

	return s2;
}	

void HUD_SW_Class::DrawTexture(shape_descriptor shape, short x, short y, int size)
{
	SDL_Surface *s = get_shape_surface(shape);
	if (!s) return;

	
	if (HUD_Buffer->format->BitsPerPixel == 8) {
		// SDL doesn't seem to be able to handle direct blits between 8-bit surfaces with different cluts
		SDL_Surface *s2 = SDL_DisplayFormat(s);
		SDL_FreeSurface(s);
		s = s2;
	}

	SDL_Surface *s2 = rescale_surface(s, size, size);
	SDL_FreeSurface(s);
	s = s2;

	s2 = rotate_surface(s, size, size);
	SDL_FreeSurface(s);
	s = s2;

	// Setup destination rectangle
	SDL_Rect dst_rect = {x, y, s->w, s->h};

	// Blit the surface
	SDL_BlitSurface(s, NULL, HUD_Buffer, &dst_rect);

	// Free the surface
	SDL_FreeSurface(s);
}

/*
 *  Draw text
 */

void HUD_SW_Class::DrawText(const char *text, screen_rectangle *dest, short flags, short font_id, short text_color)
{
	_draw_screen_text(text, dest, flags, font_id, text_color);
}


/*
 *  Fill rectangle
 */

void HUD_SW_Class::FillRect(screen_rectangle *r, short color_index)
{
	_fill_rect(r, color_index);
}


/*
 *  Frame rectangle
 */

void HUD_SW_Class::FrameRect(screen_rectangle *r, short color_index)
{
	_frame_rect(r, color_index);
}
