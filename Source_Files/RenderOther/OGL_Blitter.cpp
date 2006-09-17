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
	
	OpenGL blitter
	written by Gregory Smith, 2006
*/

#include "OGL_Blitter.h"

OGL_Blitter::OGL_Blitter(const SDL_Surface& s, const SDL_Rect& dst, const SDL_Rect& ortho) : m_ortho(ortho)
{
	m_rects.resize(1);
	m_rects[0] = dst;

	SDL_Surface *t;

	t = SDL_CreateRGBSurface(SDL_SWSURFACE, NextPowerOfTwo(s.w), NextPowerOfTwo(s.h), 32,
#ifdef ALEPHONE_LITTLE_ENDIAN
				 0x000000ff,
				 0x0000ff00,
				 0x00ff0000,
				 0xff000000
#else
				 0xff000000,
				 0x00ff0000,
				 0x0000ff00,
				 0x000000ff
#endif
		);
	
	SDL_BlitSurface(const_cast<SDL_Surface *>(&s), NULL, t, NULL);
	UScale = (double) s.h / (double) t->h;
	VScale = (double) s.w / (double) t->w;
	
	m_refs.resize(1);
	glGenTextures(1, &m_refs.front());
	
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, m_refs[0]);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, t->w, t->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, t->pixels);

	SDL_FreeSurface(t);
		      
}

OGL_Blitter::~OGL_Blitter()
{
	glDeleteTextures(1, &m_refs.front());
}

void OGL_Blitter::SetupMatrix()
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	// disable everything but alpha blending
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_ALPHA_TEST);
	glEnable(GL_BLEND);
	glDisable(GL_FOG);
	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_STENCIL_TEST);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	
	// transform ortho into screen co-ordinates
	gluOrtho2D(m_ortho.x, m_ortho.x + m_ortho.w, m_ortho.y + m_ortho.h, m_ortho.y);

}

void OGL_Blitter::Draw()
{
	glBindTexture(GL_TEXTURE_2D, m_refs[0]);
//	glBindTexture(GL_TEXTURE_2D, 0);
	glColor3ub(255, 255, 255);
	glBegin(GL_QUADS);

	glTexCoord2f(0.0, 0.0); glVertex3f(m_rects[0].x, m_rects[0].y, 0);
	glTexCoord2f(VScale, 0.0); glVertex3f(m_rects[0].x + m_rects[0].w, m_rects[0].y, 0);
	glTexCoord2f(VScale, UScale); glVertex3f(m_rects[0].x + m_rects[0].w, m_rects[0].y + m_rects[0].h, 0);
	glTexCoord2f(0.0, UScale); glVertex3f(m_rects[0].x, m_rects[0].y + m_rects[0].h, 0);
	glEnd();
}

void OGL_Blitter::RestoreMatrix()
{
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glPopAttrib();
}

