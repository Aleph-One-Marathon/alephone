#ifndef _SHAPE_BLITTER_
#define _SHAPE_BLITTER_
/*
SHAPE_BLITTER.H

    Copyright (C) 2009 by Jeremiah Morris and the Aleph One developers

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

    Draws Shapes file bitmaps for 2D UI
*/

#include "cseries.h"
#include "map.h"

#ifndef SDL
#include <SDL/SDL.h>
#endif

#include <vector>
#include <set>
using namespace std;

// texture types
enum {
    Shape_Texture_Wall,
    Shape_Texture_Landscape,
    Shape_Texture_Sprite,
    Shape_Texture_WeaponInHand,
    Shape_Texture_Interface,
    SHAPE_NUMBER_OF_TEXTURE_TYPES
};
    
class Shape_Blitter
{
public:
	Shape_Blitter(short collection, short texture_index, short texture_type, short clut_index = 0);
		
	void Rescale(int width, int height);
	int Width();
	int Height();
	int UnscaledWidth();
	int UnscaledHeight();
	
    void OGL_Draw(SDL_Rect& dst);
    void SDL_Draw(SDL_Surface *dst_surface, SDL_Rect& dst);
	
    ~Shape_Blitter();
	
	// tint the output image -- (1, 1, 1, 1) is untinted
	float tint_color_r, tint_color_g, tint_color_b, tint_color_a;
	
	// rotate the output image about the center of destination rect
	// (in degrees clockwise)
	float rotation;
	
	// set default cropping rectangle
	SDL_Rect crop_rect;
	
protected:
	
	shape_descriptor m_desc;
    short m_type;
    SDL_Rect m_src;
    SDL_Rect m_scaled_src;
    
    SDL_Surface *m_surface;
    SDL_Surface *m_scaled_surface;
};

#endif
