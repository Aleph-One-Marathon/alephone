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
using namespace std;

#ifdef HAVE_OPENGL
class OGL_Blitter
{
public:
	OGL_Blitter(const SDL_Surface& s, const SDL_Rect& dst, const SDL_Rect& ortho);
	void SetupMatrix();
	void Draw();
	void RestoreMatrix();
	~OGL_Blitter();
private:
	SDL_Rect m_ortho;

	GLdouble x_scale, y_scale;
	int x_offset, y_offset;

	vector<SDL_Rect> m_rects;
	vector<GLuint> m_refs;
	static const int tile_size = 256;
};

#endif

#endif
