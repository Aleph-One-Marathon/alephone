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
	SDL_RWops *p = SDL_RWFromMem(clut.GetPointer(), clut.GetLength());
	assert(p);

	// Check number of colors
	SDL_RWseek(p, 6, SEEK_CUR);
	int n = SDL_ReadBE16(p) + 1;
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
