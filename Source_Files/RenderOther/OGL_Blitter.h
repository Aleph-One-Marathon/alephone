#ifndef _OGL_BLITTER_
#define _OGL_BLITTER_
/*

	Copyright (C) 2006 and beyond by Bungie Studios, Inc.
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
	
	OpenGL image blitter
	written by Gregory Smith, 2006
*/

#include "cseries.h"
#include "ImageLoader.h"
#include "Image_Blitter.h"

#ifdef HAVE_OPENGL
# if defined (__APPLE__) && defined (__MACH__)
#   include <OpenGL/gl.h>
#   include <OpenGL/glu.h>
# elif defined mac
#   include <gl.h>
#   include <glu.h>
# else
#ifndef __WIN32__
#   ifndef GL_GLEXT_PROTOTYPES
#   define GL_GLEXT_PROTOTYPES 1
#endif
#endif
#   include <GL/gl.h>
#   include <GL/glu.h>
#   include <GL/glext.h>
# endif
#endif

#ifndef SDL
#include <SDL/SDL.h>
#endif

#include <vector>
#include <set>
using namespace std;

#ifdef HAVE_OPENGL
class OGL_Blitter;

class OGL_Blitter : public Image_Blitter
{
public:
	OGL_Blitter();
	
	void Unload();

	void Draw(SDL_Surface *dst_surface, SDL_Rect& dst, SDL_Rect& src) { Draw(dst, src); }
	void Draw(const SDL_Rect& dst) { Draw(dst, crop_rect); }
	void Draw(const SDL_Rect& dst, const SDL_Rect& src);
	
	~OGL_Blitter();
			
	static void StopTextures();
	static void BoundScreen();
	static int ScreenWidth();
	static int ScreenHeight();
		
private:
	
	void _Draw(const SDL_Rect& dst, const SDL_Rect& src);
	void _LoadTextures();
	void _UnloadTextures();

	vector<SDL_Rect> m_rects;
	vector<GLuint> m_refs;
	int m_tile_width, m_tile_height;
	bool m_textures_loaded;
	
	static const int tile_size = 256;
	static set<OGL_Blitter*> m_blitter_registry;
};

#endif

#endif
