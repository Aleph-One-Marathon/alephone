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

#ifdef HAVE_OPENGL

const int OGL_Blitter::tile_size;

OGL_Blitter::OGL_Blitter(const SDL_Surface& s, const SDL_Rect& dst, const SDL_Rect& ortho) : m_ortho(ortho)
{
	x_scale = (dst.w / (GLdouble) s.w);
	y_scale = (dst.h / (GLdouble) s.h);
	x_offset = dst.x;
	y_offset = dst.y;

	// calculate how many rects we need
	int v_rects = ((s.h + tile_size -1) / tile_size);
	int h_rects = ((s.w + tile_size - 1) / tile_size);
	m_rects.resize(v_rects * h_rects);
	m_refs.resize(v_rects * h_rects);

	glGenTextures(v_rects * h_rects, &m_refs.front());

	SDL_Surface *t;
#ifdef ALEPHONE_LITTLE_ENDIAN
	t = SDL_CreateRGBSurface(SDL_SWSURFACE, tile_size, tile_size, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
#else
	t = SDL_CreateRGBSurface(SDL_SWSURFACE, tile_size, tile_size, 32, 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);
#endif

	glEnable(GL_TEXTURE_2D);

	int i = 0;
	for (int y = 0; y < v_rects; y++)
	{
		for (int x = 0; x < h_rects; x++)
		{
			m_rects[i].x = x * tile_size;
			m_rects[i].y = y * tile_size;
			m_rects[i].w = std::min(tile_size, s.w - x * tile_size);
			m_rects[i].h = std::min(tile_size, s.h - y * tile_size);

			SDL_BlitSurface(const_cast<SDL_Surface *>(&s), &m_rects[i], t, NULL);

			glBindTexture(GL_TEXTURE_2D, m_refs[i]);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tile_size, tile_size, 0, GL_RGBA, GL_UNSIGNED_BYTE, t->pixels);

			i++;
		}
	}

	SDL_FreeSurface(t);
		      
}

OGL_Blitter::~OGL_Blitter()
{
	glDeleteTextures(m_refs.size(), &m_refs.front());
	m_refs.clear();
	m_rects.clear();
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
	for (int i = 0; i < m_rects.size(); i++)
	{
		glBindTexture(GL_TEXTURE_2D, m_refs[i]);
		glColor3ub(255, 255, 255);
		glBegin(GL_QUADS);

		GLdouble VScale = (double) m_rects[i].w / (double) tile_size;
		GLdouble UScale = (double) m_rects[i].h / (double) tile_size;

		glTexCoord2f(0.0, 0.0); glVertex3f((m_rects[i].x + x_offset) * x_scale, (m_rects[i].y + y_offset) * y_scale, 0);
		glTexCoord2f(VScale, 0.0); glVertex3f((m_rects[i].x + x_offset + m_rects[i].w) * x_scale, (m_rects[i].y + y_offset) * y_scale, 0);
		glTexCoord2f(VScale, UScale); glVertex3f((m_rects[i].x + x_offset + m_rects[i].w) * x_scale, (m_rects[i].y + y_offset + m_rects[i].h) * y_scale, 0);
		glTexCoord2f(0.0, UScale); glVertex3f((m_rects[i].x + x_offset) * x_scale, (m_rects[i].y + y_offset + m_rects[i].h) * y_scale, 0);
		glEnd();
	}
}

void OGL_Blitter::RestoreMatrix()
{
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glPopAttrib();
}

#endif
