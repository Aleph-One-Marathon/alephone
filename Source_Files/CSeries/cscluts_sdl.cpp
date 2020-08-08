/*

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

*/
/*
 *  cscluts_sdl.cpp - CLUT handling, SDL implementation
 *
 *  Written in 2000 by Christian Bauer
 */

#include "cseries.h"
#include "FileHandler.h"
#include "SDL_endian.h"


// Global variables
RGBColor rgb_black = {0x0000, 0x0000, 0x0000};
RGBColor rgb_white = {0xffff, 0xffff, 0xffff};

RGBColor system_colors[NUM_SYSTEM_COLORS] =
{
	{0x2666, 0x2666, 0x2666},
	{0xd999, 0xd999, 0xd999}
};


/*
 *  Convert Mac CLUT resource to color_table
 */

void build_color_table(color_table *table, LoadedResource &clut)
{
	// Open stream to CLUT resource
	SDL_RWops *p = SDL_RWFromMem(clut.GetPointer(), (int)clut.GetLength());
	assert(p);

	// Check number of colors
	SDL_RWseek(p, 6, SEEK_CUR);
	int n = SDL_ReadBE16(p) + 1;
	// SDL_ReadBE16 returns a Uint16 and thus can never be negative. At least,
	// that is what it seems.
	// TODO Eliminate the n < 0 check as it will always evaluate to false
	if (n < 0)
		n = 0;
	else if (n > 256)
		n = 256;
	table->color_count = n;

	// Convert color data
	rgb_color *dst = table->colors;
	for (int i=0; i<n; i++) {
		SDL_RWseek(p, 2, SEEK_CUR);
		dst->red = SDL_ReadBE16(p);
		dst->green = SDL_ReadBE16(p);
		dst->blue = SDL_ReadBE16(p);
		dst++;
	}

	// Close stream
	SDL_RWclose(p);
}
