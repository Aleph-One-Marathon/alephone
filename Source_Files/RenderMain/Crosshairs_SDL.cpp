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
 *  Crosshairs_SDL.cpp - Crosshairs display, SDL implementation
 */

#include "cseries.h"
#include "Crosshairs.h"
#include "screen_drawing.h"
#include "world.h" // for struct world_point2d :(


/*
 *  Crosshairs status
 */

extern bool use_lua_hud_crosshairs;

static bool _Crosshairs_IsActive = false;

bool Crosshairs_IsActive(void)
{
	return _Crosshairs_IsActive;
}

bool Crosshairs_SetActive(bool NewState)
{
	return _Crosshairs_IsActive = NewState;
}


/*
 *  Draw crosshairs in center of surface
 */

bool Crosshairs_Render(SDL_Surface *s)
{
	if (!_Crosshairs_IsActive)
		return false;
	if (use_lua_hud_crosshairs)
		return false;

	// Get the crosshair data
	CrosshairData &Crosshairs = GetCrosshairData();

	// Get color
	uint32 pixel = SDL_MapRGB(s->format, Crosshairs.Color.red >> 8, Crosshairs.Color.green >> 8, Crosshairs.Color.blue >> 8);

	// Get coordinates
	int xcen = s->w / 2 - 1, ycen = s->h / 2 - 1;

	if (Crosshairs.Shape == CHShape_RealCrosshairs)
	{

		// Left
		SDL_Rect r = {xcen - Crosshairs.FromCenter - Crosshairs.Length, ycen - Crosshairs.Thickness / 2, Crosshairs.Length, Crosshairs.Thickness};
		SDL_FillRect(s, &r, pixel);
		
		// Right
		r.x = xcen + Crosshairs.FromCenter;
		SDL_FillRect(s, &r, pixel);
		
		// Top
		r.x = xcen - Crosshairs.Thickness / 2;
		r.y = ycen - Crosshairs.FromCenter - Crosshairs.Length;
		r.w = Crosshairs.Thickness;
		r.h = Crosshairs.Length;
		SDL_FillRect(s, &r, pixel);
		
		// Bottom
		r.y = ycen + Crosshairs.FromCenter;
		SDL_FillRect(s, &r, pixel);
	}
	else if (Crosshairs.Shape == CHShape_Circle)
	{

		// This will really be an octagon, for OpenGL-rendering convenience
		
		// Precalculate the line endpoints, for convenience

		short octa_points[2][6];
		short len;

		len = Crosshairs.Length;
		octa_points[0][0] = xcen - len;
		octa_points[0][5] = xcen + len;
		octa_points[1][0] = ycen - len;
		octa_points[1][5] = ycen + len;

		len = len / 2;
		octa_points[0][1] = xcen - len;
		octa_points[0][4] = xcen + len;
		octa_points[1][1] = ycen - len;
		octa_points[1][4] = ycen + len;

		len = std::min(len, Crosshairs.FromCenter);
		octa_points[0][2] = xcen - len;
		octa_points[0][3] = xcen + len;
		octa_points[1][2] = ycen - len;
		octa_points[1][3] = ycen + len;

		// We need to do 12 line segments, so we do them in 2*2*3 fashion
		for (int ix = 0; ix < 2; ix++)
		{
			int ixi = (ix > 0) ? 5 : 0;
			int ixid = (ix > 0) ? -1 : 1;
			for (int iy = 0; iy < 2; iy++)
			{
				int iyi = (iy > 0) ? 5 : 0;
				int iyid = (iy > 0) ? -1 : 1;

				world_point2d p1;
				world_point2d p2;

				// Vertical
				p1.x = octa_points[0][ixi];
				p1.y = octa_points[1][iyi + 2 * iyid];
				p2.x = octa_points[0][ixi];
				p2.y = octa_points[1][iyi+iyid];

				draw_line(s, &p1, &p2, pixel, Crosshairs.Thickness);

				// Diagonal
				p1 = p2;
				p2.x = octa_points[0][ixi+ixid];
				p2.y = octa_points[1][iyi];
				draw_line(s, &p1, &p2, pixel, Crosshairs.Thickness);

				// Horizontal
				p1 = p2;
				p2.x = octa_points[0][ixi + 2*ixid];
				p2.y = octa_points[1][iyi];
				draw_line(s, &p1, &p2, pixel, Crosshairs.Thickness);
			}
		}
	}

	return true;
}
