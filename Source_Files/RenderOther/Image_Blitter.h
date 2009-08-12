#ifndef _IMAGE_BLITTER_
#define _IMAGE_BLITTER_
/*
IMAGE_BLITTER.H

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

    Implements images for 2D UI
*/

#include "cseries.h"
#include "ImageLoader.h"

#ifndef SDL
#include <SDL/SDL.h>
#endif

#include <vector>
#include <set>
using namespace std;

class Image_Blitter
{
public:
	Image_Blitter();
	
	bool Load(const ImageDescriptor& image);
    bool Load(int picture_resource);
	bool Load(const SDL_Surface& s);
	bool Load(const SDL_Surface& s, const SDL_Rect& src);
	virtual void Unload();
	bool Loaded();
	
	void Rescale(int width, int height);
	int Width();
	int Height();
	int UnscaledWidth();
	int UnscaledHeight();
	
	virtual void Draw(SDL_Surface *dst_surface, SDL_Rect& dst) { Draw(dst_surface, dst, crop_rect); }
	virtual void Draw(SDL_Surface *dst_surface, SDL_Rect& dst, SDL_Rect& src);
		
	virtual ~Image_Blitter();
	
	// tint the output image -- (1, 1, 1, 1) is untinted
	float tint_color_r, tint_color_g, tint_color_b, tint_color_a;
	
	// rotate the output image about the center of destination rect
	// (in degrees clockwise)
	float rotation;
	
	// set default cropping rectangle
	SDL_Rect crop_rect;
	
protected:
	
	SDL_Surface *m_surface;
    SDL_Surface *m_disp_surface;
	SDL_Surface *m_scaled_surface;
	SDL_Rect m_src, m_scaled_src;
};

#endif
