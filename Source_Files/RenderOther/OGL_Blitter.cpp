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

OGL_Blitter::OGL_Blitter(const SDL_Surface& s, const SDL_Rect& dst, const SDL_Rect& ortho) : m_ortho(ortho), m_dst(dst)
{
	m_src.x = m_src.y = 0;
	m_src.w = s.w;
	m_src.h = s.h;
	BuildTextures(s);
}

OGL_Blitter::OGL_Blitter(const SDL_Surface& s)
{
	m_dst.x = m_dst.y = m_dst.w = m_dst.h = 0;
	m_ortho.x = m_ortho.y = m_ortho.w = m_ortho.h = 0;
	
	m_src.x = m_src.y = 0;
	m_src.w = s.w;
	m_src.h = s.h;
	BuildTextures(s);
}

void OGL_Blitter::BuildTextures(const SDL_Surface& s)
{
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

	if (m_ortho.w && m_ortho.h)
	{
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
	
		// transform ortho into screen co-ordinates
		gluOrtho2D(m_ortho.x, m_ortho.x + m_ortho.w, m_ortho.y + m_ortho.h, m_ortho.y);
	}
}

void OGL_Blitter::Draw()
{
	Draw(m_dst, m_src);
}

void OGL_Blitter::Draw(const SDL_Rect& dst)
{
	Draw(dst, m_src);
}

void OGL_Blitter::Draw(const SDL_Rect& dst, const SDL_Rect& src)
{
	GLdouble x_scale = dst.w / (GLdouble) src.w;
	GLdouble y_scale = dst.h / (GLdouble) src.h;
	GLdouble x_offset = dst.x;
	GLdouble y_offset = dst.y;
	
	for (int i = 0; i < m_rects.size(); i++)
	{
		if (src.x > m_rects[i].x + m_rects[i].w ||
		    src.x + src.w < m_rects[i].x ||
		    src.y > m_rects[i].y + m_rects[i].h ||
		    src.y + src.h < m_rects[i].y)
			continue;
		
		int tx = std::max(0, src.x - m_rects[i].x);
		int ty = std::max(0, src.y - m_rects[i].y);
		int tw = std::min((int) m_rects[i].w, src.x + src.w) - tx;
		int th = std::min((int) m_rects[i].h, src.y + src.h) - ty;
		
		GLdouble VMin = tx / (GLdouble) tile_size;
		GLdouble VMax = (tx + tw) / (GLdouble) tile_size;
		GLdouble UMin = ty / (GLdouble) tile_size;
		GLdouble UMax = (ty + th) / (GLdouble) tile_size;
		
		GLdouble tleft   = ((m_rects[i].x + tx) * x_scale) + (GLdouble) (dst.x - (src.x * x_scale));
		GLdouble tright  = tleft + (tw * x_scale);
		GLdouble ttop    = ((m_rects[i].y + ty) * y_scale) + (GLdouble) (dst.y - (src.y * y_scale));
		GLdouble tbottom = ttop + (th * y_scale);
		
		glBindTexture(GL_TEXTURE_2D, m_refs[i]);
		glColor3ub(255, 255, 255);
		glBegin(GL_QUADS);
		glTexCoord2f(VMin, UMin); glVertex3f(tleft,  ttop,    0);
		glTexCoord2f(VMax, UMin); glVertex3f(tright, ttop,    0);
		glTexCoord2f(VMax, UMax); glVertex3f(tright, tbottom, 0);
		glTexCoord2f(VMin, UMax); glVertex3f(tleft,  tbottom, 0);
		glEnd();
	}
}

void OGL_Blitter::RestoreMatrix()
{
	if (m_ortho.w && m_ortho.h)
	{
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
	}
	glPopAttrib();
}

#endif
