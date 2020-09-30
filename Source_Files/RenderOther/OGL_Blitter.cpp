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
	
	OpenGL blitter
	written by Gregory Smith, 2006
*/

#include "OGL_Blitter.h"

#include <memory>

#include "OGL_Setup.h"
#include "shell.h"
#include "screen.h"

#ifdef HAVE_OPENGL
#include "OGL_Render.h"

const int OGL_Blitter::tile_size;
std::set<OGL_Blitter*> *OGL_Blitter::m_blitter_registry = NULL;

OGL_Blitter::OGL_Blitter() : m_textures_loaded(false)
{
	m_src.x = m_src.y = m_src.w = m_src.h = 0;
	m_scaled_src.x = m_scaled_src.y = m_scaled_src.w = m_scaled_src.h = 0;
	crop_rect.x = crop_rect.y = crop_rect.w = crop_rect.h = 0;
}

// defined in OGL_Textures.cpp
inline int NextPowerOfTwo(int n);

void OGL_Blitter::_LoadTextures()
{
	if (m_textures_loaded)
		return;
	if (!m_surface)
		return;
	
	m_tile_width  = std::min(NextPowerOfTwo(m_src.w), tile_size);
	m_tile_height = std::min(NextPowerOfTwo(m_src.h), tile_size);
	
	if (Get_OGL_ConfigureData().Flags & OGL_Flag_TextureFix)
	{
		m_tile_width = std::max(m_tile_width, 128);
		m_tile_height = std::max(m_tile_height, 128);
	}

	std::unique_ptr<SDL_Surface, decltype(&SDL_FreeSurface)> t(nullptr, SDL_FreeSurface);
	if (PlatformIsLittleEndian()) {
		t.reset(SDL_CreateRGBSurface(SDL_SWSURFACE, m_tile_width, m_tile_height, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000));
	} else {
		t.reset(SDL_CreateRGBSurface(SDL_SWSURFACE, m_tile_width, m_tile_height, 32, 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff));
	}
	if (!t)
		return;
	
	SDL_SetSurfaceBlendMode(t.get(), SDL_BLENDMODE_NONE);
	
	// calculate how many rects we need
	int v_rects = ((m_src.h + m_tile_height - 1) / m_tile_height);
	int h_rects = ((m_src.w + m_tile_width - 1) / m_tile_width);
	m_rects.resize(v_rects * h_rects);
	m_refs.resize(v_rects * h_rects);

	// ensure our textures get cleaned up
	Register(this);

	uint32 rgb_mask = ~(t->format->Amask);

	glEnable(GL_TEXTURE_2D);
	int i = 0;
	for (int y = 0; y < v_rects; y++)
	{
		for (int x = 0; x < h_rects; x++)
		{
			m_rects[i].x = x * m_tile_width;
			m_rects[i].y = y * m_tile_height;
			m_rects[i].w = std::min(m_tile_width, static_cast<int>(m_src.w - x * m_tile_width));
			m_rects[i].h = std::min(m_tile_height, static_cast<int>(m_src.h - y * m_tile_height));

			SDL_Rect sr = { m_rects[i].x, m_rects[i].y, m_rects[i].w, m_rects[i].h }; 
			SDL_BlitSurface(m_surface, &sr, t.get(), NULL);

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
			
			glGenTextures(1, &m_refs[i]);
			glBindTexture(GL_TEXTURE_2D, m_refs[i]);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_tile_width, m_tile_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, t->pixels);

			i++;
		}
	}
	
	m_textures_loaded = true;
	return;
}

void OGL_Blitter::Unload()
{
	Image_Blitter::Unload();
	_UnloadTextures();
}

void OGL_Blitter::_UnloadTextures()
{
	if (!m_textures_loaded)
		return;
	Deregister(this);
	if (m_refs.size())
		glDeleteTextures(m_refs.size(), &m_refs[0]);
	m_refs.clear();
	m_rects.clear();
	m_textures_loaded = false;
}

void OGL_Blitter::StopTextures()
{
	if (!m_blitter_registry)
		return;
	
	std::set<OGL_Blitter*>::iterator it;
	for (it = m_blitter_registry->begin();
	     it != m_blitter_registry->end();
	     it = m_blitter_registry->begin())
		(*it)->_UnloadTextures();
}

OGL_Blitter::~OGL_Blitter()
{
	Unload();
}

int OGL_Blitter::ScreenWidth()
{
	return MainScreenLogicalWidth();
}

int OGL_Blitter::ScreenHeight()
{
	return MainScreenLogicalHeight();
}

void OGL_Blitter::BoundScreen(bool in_game)
{
	alephone::Screen::instance()->bound_screen(in_game);
}

void OGL_Blitter::WindowToScreen(int& x, int& y, bool in_game)
{
	alephone::Screen::instance()->window_to_screen(x, y);
}

void OGL_Blitter::Draw(const Image_Rect& dst, const Image_Rect& raw_src)
{
	if (!Loaded())
		return;
	_LoadTextures();
	if (!m_textures_loaded)
		return;

	glPushAttrib(GL_ALL_ATTRIB_BITS);
	
	// disable everything but alpha blending and clipping
	glDisable(GL_DEPTH_TEST);
//	glDisable(GL_ALPHA_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_FOG);
//	glDisable(GL_SCISSOR_TEST);
//	glDisable(GL_STENCIL_TEST);
	glEnable(GL_TEXTURE_2D);

	Image_Rect src;
	if (m_src.w != m_scaled_src.w)
	{
		src.x = raw_src.x * m_src.w / m_scaled_src.w;
		src.w = raw_src.w * m_src.w / m_scaled_src.w;
	}
	else
	{
		src.x = raw_src.x;
		src.w = raw_src.w;
	}
	if (m_src.h != m_scaled_src.h)
	{
		src.y = raw_src.y * m_src.h / m_scaled_src.h;
		src.h = raw_src.h * m_src.h / m_scaled_src.h;
	}
	else
	{
		src.y = raw_src.y;
		src.h = raw_src.h;
	}

	GLdouble x_scale = dst.w / (GLdouble) src.w;
	GLdouble y_scale = dst.h / (GLdouble) src.h;
	
	bool rotating = (rotation > 0.1 || rotation < -0.1);
	if (rotating)
	{
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glTranslatef((dst.x + dst.w/2.0), (dst.y + dst.h/2.0), 0.0);
		glRotatef(rotation, 0.0, 0.0, 1.0);
		glTranslatef(-(dst.x + dst.w/2.0), -(dst.y + dst.h/2.0), 0.0);
	}
	
	glColor4f(tint_color_r, tint_color_g, tint_color_b, tint_color_a);
	
	for (int i = 0; i < m_rects.size(); i++)
	{
		if (src.x > m_rects[i].x + m_rects[i].w ||
		    src.x + src.w < m_rects[i].x ||
		    src.y > m_rects[i].y + m_rects[i].h ||
		    src.y + src.h < m_rects[i].y)
			continue;
		
		GLdouble tx = MAX(0, src.x - m_rects[i].x);
		GLdouble ty = MAX(0, src.y - m_rects[i].y);
		GLdouble tw = MIN(m_rects[i].w, src.x + src.w - m_rects[i].x) - tx;
		GLdouble th = MIN(m_rects[i].h, src.y + src.h - m_rects[i].y) - ty;
		
		GLdouble VMin = tx / (GLdouble) m_tile_width;
		GLdouble VMax = (tx + tw) / (GLdouble) m_tile_width;
		GLdouble UMin = ty / (GLdouble) m_tile_height;
		GLdouble UMax = (ty + th) / (GLdouble) m_tile_height;
		
		GLdouble tleft   = ((m_rects[i].x + tx) * x_scale) + (GLdouble) (dst.x - (src.x * x_scale));
		GLdouble tright  = tleft + (tw * x_scale);
		GLdouble ttop    = ((m_rects[i].y + ty) * y_scale) + (GLdouble) (dst.y - (src.y * y_scale));
		GLdouble tbottom = ttop + (th * y_scale);
		
		glBindTexture(GL_TEXTURE_2D, m_refs[i]);
		
		OGL_RenderTexturedRect(tleft, ttop, tright - tleft, tbottom - ttop,
							   VMin, UMin, VMax, UMax);
	}
	
	if (rotating)
		glPopMatrix();
	glPopAttrib();
}

void OGL_Blitter::Register(OGL_Blitter *B)
{
	if (!m_blitter_registry)
		m_blitter_registry = new std::set<OGL_Blitter*>;
	m_blitter_registry->insert(B);
}

void OGL_Blitter::Deregister(OGL_Blitter *B)
{
	if (m_blitter_registry)
		m_blitter_registry->erase(B);
	// we could delete registry here, but why bother?
}

#endif
