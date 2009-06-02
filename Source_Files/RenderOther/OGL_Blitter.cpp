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
#include "OGL_Setup.h"

#ifdef HAVE_OPENGL

const int OGL_Blitter::tile_size;
set<OGL_Blitter*> OGL_Blitter::m_blitter_registry;

OGL_Blitter::OGL_Blitter() : m_loaded(false) { }

OGL_Blitter::OGL_Blitter(const SDL_Surface& s) : m_loaded(false)
{
	Load(s);
}

OGL_Blitter::OGL_Blitter(const SDL_Surface& s, const SDL_Rect& src) : m_loaded(false)
{
	Load(s, src);
}

OGL_Blitter::OGL_Blitter(const ImageDescriptor& image) : m_loaded(false)
{
	Load(image);
}

bool OGL_Blitter::Load(const ImageDescriptor& image)
{
	Unload();
#ifdef ALEPHONE_LITTLE_ENDIAN
	SDL_Surface *s = SDL_CreateRGBSurfaceFrom(const_cast<uint32 *>(image.GetBuffer()), image.GetWidth(), image.GetHeight(), 32, image.GetWidth() * 4, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
#else
	SDL_Surface *s = SDL_CreateRGBSurfaceFrom(const_cast<uint32 *>(image.GetBuffer()), image.GetWidth(), image.GetHeight(), 32, image.GetWidth() * 4, 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);
#endif
	if (!s)
		return false;
	bool ret = Load(*s);
	SDL_FreeSurface(s);
	return ret;
}

// defined in OGL_Textures.cpp
inline int NextPowerOfTwo(int n);

bool OGL_Blitter::Load(const SDL_Surface& s)
{
	SDL_Rect sr = { 0, 0, s.w, s.h };
	return Load(s, sr);
}

bool OGL_Blitter::Load(const SDL_Surface& s, const SDL_Rect& src)
{
	Unload();
	m_src.x = 0;
	m_src.y = 0;
	m_src.w = src.w;
	m_src.h = src.h;
	
	m_tile_width  = std::min(NextPowerOfTwo(src.w), tile_size);
	m_tile_height = std::min(NextPowerOfTwo(src.h), tile_size);
	
	if (Get_OGL_ConfigureData().Flags & OGL_Flag_TextureFix)
	{
		m_tile_width = std::max(m_tile_width, 128);
		m_tile_height = std::max(m_tile_height, 128);
	}

	SDL_Surface *t;
#ifdef ALEPHONE_LITTLE_ENDIAN
	t = SDL_CreateRGBSurface(SDL_SWSURFACE, m_tile_width, m_tile_height, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
#else
	t = SDL_CreateRGBSurface(SDL_SWSURFACE, m_tile_width, m_tile_height, 32, 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);
#endif
	if (!t)
		return false;
	
	// calculate how many rects we need
	int v_rects = ((src.h + m_tile_height - 1) / m_tile_height);
	int h_rects = ((src.w + m_tile_width - 1) / m_tile_width);
	m_rects.resize(v_rects * h_rects);
	m_refs.resize(v_rects * h_rects);

	// ensure our textures get cleaned up
	m_blitter_registry.insert(this);
	glGenTextures(v_rects * h_rects, &m_refs[0]);

	uint32 rgb_mask = ~(t->format->Amask);

	glEnable(GL_TEXTURE_2D);
	
	// when blitting surface, make sure we copy rather than blend the alpha
	uint8 src_alpha = s.format->alpha;
	uint32 src_flags = s.flags;
	if (src_flags & SDL_SRCALPHA)
		SDL_SetAlpha(const_cast<SDL_Surface *>(&s), src_flags & ~SDL_SRCALPHA, 0);

	int i = 0;
	for (int y = 0; y < v_rects; y++)
	{
		for (int x = 0; x < h_rects; x++)
		{
			m_rects[i].x = x * m_tile_width;
			m_rects[i].y = y * m_tile_height;
			m_rects[i].w = std::min(m_tile_width, src.w - x * m_tile_width);
			m_rects[i].h = std::min(m_tile_height, src.h - y * m_tile_height);

			SDL_Rect sr = { src.x + m_rects[i].x, src.y + m_rects[i].y, m_rects[i].w, m_rects[i].h }; 
			SDL_BlitSurface(const_cast<SDL_Surface *>(&s), &sr, t, NULL);

			// to avoid edge artifacts, smear edge pixels out to texture boundary
			for (int row = 0; row < m_rects[i].h; ++row)
			{
				uint32 *curRow = static_cast<uint32 *>(t->pixels) + (row * m_tile_width);
				for (int col = m_rects[i].w; col < m_tile_width; ++col)
				{
					curRow[col] = curRow[m_rects[i].w - 1] & rgb_mask;
				}
			}
			
			uint32 *lastRow = static_cast<uint32 *>(t->pixels) + ((m_rects[i].h - 1) * m_tile_width);
			for (int row = m_rects[i].h; row < m_tile_height; ++row)
			{
				uint32 *curRow = static_cast<uint32 *>(t->pixels) + (row * m_tile_width);
				for (int col = 0; col < m_tile_width; ++col)
				{
					curRow[col] = lastRow[col] & rgb_mask;
				}
			}
			
			glBindTexture(GL_TEXTURE_2D, m_refs[i]);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_tile_width, m_tile_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, t->pixels);

			i++;
		}
	}
	
	// restore the source surface to its original blend mode
	if (src_flags & SDL_SRCALPHA)
		SDL_SetAlpha(const_cast<SDL_Surface *>(&s), src_flags, src_alpha);

	SDL_FreeSurface(t);
	m_loaded = true;
	return true;
}

void OGL_Blitter::Unload()
{
	if (!m_loaded)
		return;
	m_blitter_registry.erase(this);
	if (m_refs.size())
		glDeleteTextures(m_refs.size(), &m_refs[0]);
	m_refs.clear();
	m_rects.clear();
	m_src.x = m_src.y = m_src.w = m_src.h = 0;
	m_loaded = false;
}

bool OGL_Blitter::Loaded()
{
	return m_loaded;
}

int OGL_Blitter::Width()
{
	return m_src.w;
}

int OGL_Blitter::Height()
{
	return m_src.h;
}

void OGL_Blitter::StopTextures()
{
	set<OGL_Blitter*>::iterator it, end = m_blitter_registry.end();
	for (it = m_blitter_registry.begin(); it != end; it++)
		(*it)->Unload();
}

OGL_Blitter::~OGL_Blitter()
{
	Unload();
}

int OGL_Blitter::ScreenWidth()
{
	return SDL_GetVideoSurface()->w;
}

int OGL_Blitter::ScreenHeight()
{
	return SDL_GetVideoSurface()->h;
}

void OGL_Blitter::BoundScreen()
{	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glViewport(0, 0, ScreenWidth(), ScreenHeight());
	glOrtho(0, ScreenWidth(), ScreenHeight(), 0, -1, 1);
	glMatrixMode(GL_MODELVIEW);
}

void OGL_Blitter::Draw()
{
	Draw(m_src, m_src);
}

void OGL_Blitter::Draw(const SDL_Rect& dst)
{
	Draw(dst, m_src);
}

void OGL_Blitter::Draw(const SDL_Rect& dst, const SDL_Rect& src)
{
	if (!Loaded())
		return;
	
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	
	// disable everything but alpha blending
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_ALPHA_TEST);
	glEnable(GL_BLEND);
	glDisable(GL_FOG);
	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_STENCIL_TEST);
	glEnable(GL_TEXTURE_2D);

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
		int tw = std::min((int) m_rects[i].w, src.x + src.w - m_rects[i].x) - tx;
		int th = std::min((int) m_rects[i].h, src.y + src.h - m_rects[i].y) - ty;
		
		GLdouble VMin = tx / (GLdouble) m_tile_width;
		GLdouble VMax = (tx + tw) / (GLdouble) m_tile_width;
		GLdouble UMin = ty / (GLdouble) m_tile_height;
		GLdouble UMax = (ty + th) / (GLdouble) m_tile_height;
		
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
	
	glPopAttrib();
}

#endif
