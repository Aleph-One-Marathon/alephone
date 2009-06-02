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
 *  Crosshairs_SDL.cpp - Crosshairs display, SDL implementation
 */

#include "cseries.h"
#include "Crosshairs.h"
#include "screen_drawing.h"
#include "world.h" // for struct world_point2d :(


/*
 *  Crosshairs status
 */

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

SDL_Surface *CrosshairSurface = NULL;
static CrosshairData CrosshairCachedData;

bool Crosshairs_Render(SDL_Surface *s)
{
	if (!_Crosshairs_IsActive)
		return false;
	
	if (!CrosshairSurface)
	{
		// set up surface for cached crosshair
		SDL_Surface *t = SDL_CreateRGBSurface(SDL_SWSURFACE, 128, 128, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
		SDL_SetAlpha(t, SDL_SRCALPHA, 0);
		CrosshairSurface = SDL_DisplayFormatAlpha(t);
		SDL_FreeSurface(t);
		
		// ensure that we redraw into our surface
		memset(&CrosshairCachedData, 0, sizeof(CrosshairCachedData));
	}

	CrosshairData &data = GetCrosshairData();
	if (memcmp(&data, &CrosshairCachedData, sizeof(CrosshairData)))
	{
		// redraw surface
		SDL_FillRect(CrosshairSurface, NULL, SDL_MapRGBA(CrosshairSurface->format, 255, 255, 255, 255));

		// Get color
		uint32 pixel = SDL_MapRGBA(CrosshairSurface->format, data.Color.red >> 8, data.Color.green >> 8, data.Color.blue >> 8, data.Opacity * 255);
		
		// Get coordinates
		int xcen = CrosshairSurface->w / 2 - 1, ycen = CrosshairSurface->h / 2 - 1;
		
		if (data.Shape == CHShape_RealCrosshairs)
		{
			
			// Left
			SDL_Rect r = {xcen - data.FromCenter - data.Length, ycen - data.Thickness / 2, data.Length, data.Thickness};
			SDL_FillRect(CrosshairSurface, &r, pixel);
			
			// Right
			r.x = xcen + data.FromCenter;
			SDL_FillRect(CrosshairSurface, &r, pixel);
			
			// Top
			r.x = xcen - data.Thickness / 2;
			r.y = ycen - data.FromCenter - data.Length;
			r.w = data.Thickness;
			r.h = data.Length;
			SDL_FillRect(CrosshairSurface, &r, pixel);
			
			// Bottom
			r.y = ycen + data.FromCenter;
			SDL_FillRect(CrosshairSurface, &r, pixel);
		}
		else if (data.Shape == CHShape_Circle)
		{
			
			// This will really be an octagon, for OpenGL-rendering convenience
			
			// Precalculate the line endpoints, for convenience
			
			short octa_points[2][6];
			short len;
			
			len = data.Length;
			octa_points[0][0] = xcen - len;
			octa_points[0][5] = xcen + len;
			octa_points[1][0] = ycen - len;
			octa_points[1][5] = ycen + len;
			
			len = len / 2;
			octa_points[0][1] = xcen - len;
			octa_points[0][4] = xcen + len;
			octa_points[1][1] = ycen - len;
			octa_points[1][4] = ycen + len;
			
			len = std::min(len, data.FromCenter);
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
					
					draw_line(CrosshairSurface, &p1, &p2, pixel, data.Thickness);
					
					// Diagonal
					p1 = p2;
					p2.x = octa_points[0][ixi+ixid];
					p2.y = octa_points[1][iyi];
					draw_line(CrosshairSurface, &p1, &p2, pixel, data.Thickness);
					
					// Horizontal
					p1 = p2;
					p2.x = octa_points[0][ixi + 2*ixid];
					p2.y = octa_points[1][iyi];
					draw_line(CrosshairSurface, &p1, &p2, pixel, data.Thickness);
				}
			}
		}
		
		CrosshairCachedData = data;
	}
	
	SDL_Rect sr = { 0, 0, CrosshairSurface->w, CrosshairSurface->h };
	SDL_Rect dr = { (s->w - sr.w) / 2, (s->h - sr.h) / 2, sr.w, sr.h };
	
	SDL_BlitSurface(CrosshairSurface, &sr, s, &dr);

	return true;
}
