#ifndef _OGL_BLITTER_
#define _OGL_BLITTER_
/*

	Copyright (C) 2006 and beyond by Bungie Studios, Inc.
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
	
	OpenGL image blitter
	written by Gregory Smith, 2006
*/

#include "cseries.h"
#include "ImageLoader.h"
#include "Image_Blitter.h"

#ifdef HAVE_OPENGL
#include "OGL_Headers.h"
#endif

#include <vector>
#include <set>

using std::vector;

#ifdef HAVE_OPENGL
class OGL_Blitter;

class OGL_Blitter : public Image_Blitter
{
public:
	OGL_Blitter();
	
	void Unload();

	void Draw(SDL_Surface *dst_surface, const Image_Rect& dst, const Image_Rect& src) { Draw(dst, src); }
	void Draw(const Image_Rect& dst) { Draw(dst, crop_rect); }
	void Draw(const Image_Rect& dst, const Image_Rect& src);
	
	~OGL_Blitter();
			
	static void StopTextures();
	static void BoundScreen(bool in_game = false);
	static void WindowToScreen(int& x, int& y, bool in_game = false);
	static int ScreenWidth();
	static int ScreenHeight();
	
private:
	
	void _LoadTextures();
	void _UnloadTextures();

	// Add or remove an instance from the registry of in-use OpenGL blitters.
	// To recycle OpenGL assets properly on context switches, the set
	// m_blitter_registry tracks all active blitters.
	static void Register(OGL_Blitter *B);
	static void Deregister(OGL_Blitter *B);
	
	vector<SDL_Rect> m_rects;
	vector<GLuint> m_refs;
	int m_tile_width, m_tile_height;
	bool m_textures_loaded;
	
	static const int tile_size = 256;
	static std::set<OGL_Blitter*> *m_blitter_registry;
};

#endif

#endif
