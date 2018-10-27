/*
SHAPE_BLITTER.CPP
 
    Copyright (C) 2009 by Jeremiah Morris and the Aleph One developers

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

    Draws Shapes file bitmaps for 2D UI
*/

#include "Shape_Blitter.h"
#include "interface.h"
#include "render.h"
#include "images.h"
#include "shell.h"
#include "scottish_textures.h"

#ifdef HAVE_OPENGL
#include "OGL_Setup.h"
#include "OGL_Textures.h"
#include "OGL_Blitter.h"
#include "OGL_Render.h"

#include "OGL_Headers.h"
#endif

extern bool shapes_file_is_m1();
static bool shape_is_motion_blip(short collection, short frame_index)
{
	if (shapes_file_is_m1())
	{
		return (collection == 0 &&
				((frame_index >= 23 && frame_index <= 34) ||
				 (frame_index >= 46 && frame_index <= 51)));
	}
	return (collection == 0 &&
			(frame_index >= 12 && frame_index <= 29));
}


Shape_Blitter::Shape_Blitter(short collection, short frame_index, short texture_type, short clut_index) : m_coll(BUILD_COLLECTION(collection, clut_index)), m_frame(frame_index), m_type(texture_type), m_surface(NULL), m_scaled_surface(NULL), tint_color_r(1.0), tint_color_g(1.0), tint_color_b(1.0), tint_color_a(1.0), rotation(0.0)
{
	m_src.x = m_src.y = m_src.w = m_src.h = 0;
	m_scaled_src.x = m_scaled_src.y = m_scaled_src.w = m_scaled_src.h = 0;
	crop_rect.x = crop_rect.y = crop_rect.w = crop_rect.h = 0;
    
    byte *pixelsOut = NULL;
    SDL_Surface *tmp = get_shape_surface(m_frame, m_coll, &pixelsOut);
    if (tmp)
    {
        m_src.w = m_scaled_src.w = crop_rect.w = tmp->w;
        m_src.h = m_scaled_src.h = crop_rect.h = tmp->h;
        SDL_FreeSurface(tmp);
        if (pixelsOut)
            free(pixelsOut);
    }
}

void Shape_Blitter::Rescale(float width, float height)
{	
	if (width != m_scaled_src.w)
	{
		crop_rect.x = crop_rect.x * width / m_scaled_src.w;
		crop_rect.w = crop_rect.w * width / m_scaled_src.w;
		m_scaled_src.w = width;
	}
	if (height != m_scaled_src.h)
	{
		crop_rect.y = crop_rect.y * height / m_scaled_src.h;
		crop_rect.h = crop_rect.h * height / m_scaled_src.h;
		m_scaled_src.h = height;
	}
}

float Shape_Blitter::Width()
{
	return m_scaled_src.w;
}

float Shape_Blitter::Height()
{
	return m_scaled_src.h;
}

int Shape_Blitter::UnscaledWidth()
{
	return m_src.w;
}

int Shape_Blitter::UnscaledHeight()
{
	return m_src.h;
}

void Shape_Blitter::OGL_Draw(const Image_Rect& dst)
{
#ifdef HAVE_OPENGL
	// Set up texture
	TextureManager TMgr;
	TMgr.ShapeDesc = BUILD_DESCRIPTOR(m_coll, 0);
    TMgr.LowLevelShape = m_frame;
    extended_get_shape_bitmap_and_shading_table(m_coll, m_frame,  &TMgr.Texture, &TMgr.ShadingTables, _shading_normal);
	TMgr.IsShadeless = false;
	TMgr.TransferMode = _shadeless_transfer;
    
    switch (m_type)
    {
        case Shape_Texture_Wall:
            TMgr.TextureType = OGL_Txtr_Wall;
            break;
        case Shape_Texture_Landscape:
	  {
            TMgr.TextureType = OGL_Txtr_Landscape;
            LandscapeOptions *LandOpts = View_GetLandscapeOptions(TMgr.ShapeDesc);	
            TMgr.LandscapeVertRepeat = LandOpts->VertRepeat;
            TMgr.Landscape_AspRatExp = LandOpts->OGL_AspRatExp;
	  }
            break;
        case Shape_Texture_Sprite:
            TMgr.TextureType = OGL_Txtr_Inhabitant;
            break;
        case Shape_Texture_WeaponInHand:
        case Shape_Texture_Interface:
            TMgr.TextureType = OGL_Txtr_WeaponsInHand;
            break;
    }
	if (!TMgr.Setup())
		return;
    
	// Get dimensions
	GLdouble U_Scale = TMgr.U_Scale;
	GLdouble V_Scale = TMgr.V_Scale;
	GLdouble U_Offset = TMgr.U_Offset;
	GLdouble V_Offset = TMgr.V_Offset;
    
	// Draw shape
	if (Wanting_sRGB && TMgr.TextureType != OGL_Txtr_WeaponsInHand)
	{
		glEnable(GL_FRAMEBUFFER_SRGB_EXT);
		Using_sRGB = true;
	}
	SglColor4f(tint_color_r, tint_color_g, tint_color_b, tint_color_a);
	glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	TMgr.SetupTextureMatrix();
	TMgr.RenderNormal();
    
    bool rotating = (rotation > 0.1 || rotation < -0.1);
	if (rotating)
	{
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glTranslatef((dst.x + dst.w/2.0), (dst.y + dst.h/2.0), 0.0);
		glRotatef(rotation, 0.0, 0.0, 1.0);
		glTranslatef(-(dst.x + dst.w/2.0), -(dst.y + dst.h/2.0), 0.0);
	}

    if (m_type == Shape_Texture_Interface)
    {
        if (crop_rect.x > 0)
            U_Offset += crop_rect.x * U_Scale / static_cast<double>(m_scaled_src.w);
        if (crop_rect.y > 0)
            V_Offset += crop_rect.y * V_Scale / static_cast<double>(m_scaled_src.h);
        if (crop_rect.w < m_scaled_src.w)
            U_Scale *= crop_rect.w / static_cast<double>(m_scaled_src.w);
        if (crop_rect.h < m_scaled_src.h)
            V_Scale *= crop_rect.h / static_cast<double>(m_scaled_src.h);

		OGL_RenderTexturedRect(dst.x, dst.y, dst.w, dst.h,
							   U_Offset, V_Offset,
							   U_Offset + U_Scale,
							   V_Offset + V_Scale);
    }
    else if (m_type == Shape_Texture_Landscape)
    {
        U_Scale = -TMgr.Texture->width / static_cast<double>(TMgr.Texture->height);
        U_Offset = 0.5 - U_Scale/2.0;
        
        if (crop_rect.x > 0)
            V_Offset += crop_rect.x * V_Scale / static_cast<double>(m_scaled_src.w);
        if (crop_rect.y > 0)
            U_Offset += crop_rect.y * U_Scale / static_cast<double>(m_scaled_src.h);
        if (crop_rect.w < m_scaled_src.w)
            V_Scale *= crop_rect.w / static_cast<double>(m_scaled_src.w);
        if (crop_rect.h < m_scaled_src.h)
            U_Scale *= crop_rect.h / static_cast<double>(m_scaled_src.h);
		
		OGL_RenderTexturedRect(dst.x, dst.y, dst.w, dst.h,
							   V_Offset, U_Offset,
							   V_Offset + V_Scale,
							   U_Offset + U_Scale);
    }
    else
    {
        shape_information_data *info = extended_get_shape_information(m_coll, m_frame);
        if (info->flags & _X_MIRRORED_BIT)
        {
            V_Offset += V_Scale;
            V_Scale = -V_Scale;
        }
        if (info->flags & _Y_MIRRORED_BIT)
        {
            U_Offset += U_Scale;
            U_Scale = -U_Scale;
        }

        if (crop_rect.x > 0)
            V_Offset += crop_rect.x * V_Scale / static_cast<double>(m_scaled_src.w);
        if (crop_rect.y > 0)
            U_Offset += crop_rect.y * U_Scale / static_cast<double>(m_scaled_src.h);
        if (crop_rect.w < m_scaled_src.w)
            V_Scale *= crop_rect.w / static_cast<double>(m_scaled_src.w);
        if (crop_rect.h < m_scaled_src.h)
            U_Scale *= crop_rect.h / static_cast<double>(m_scaled_src.h);

		GLfloat texcoords[8] = {
			GLfloat(U_Offset),           GLfloat(V_Offset),
			GLfloat(U_Offset),           GLfloat(V_Offset + V_Scale),
			GLfloat(U_Offset + U_Scale), GLfloat(V_Offset + V_Scale),
			GLfloat(U_Offset + U_Scale), GLfloat(V_Offset)
		};
		GLfloat vertices[8] = {
			dst.x, dst.y,
			dst.x + dst.w, dst.y,
			dst.x + dst.w, dst.y + dst.h,
			dst.x, dst.y + dst.h
		};
		glVertexPointer(2, GL_FLOAT, 0, vertices);
		glTexCoordPointer(2, GL_FLOAT, 0, texcoords);
		glDrawArrays(GL_POLYGON, 0, 4);
	}
    
    if (rotating)
        glPopMatrix();
    
	if (TMgr.IsGlowMapped()) TMgr.RenderGlowing();
	TMgr.RestoreTextureMatrix();
	if (Using_sRGB)
	{
		glDisable(GL_FRAMEBUFFER_SRGB_EXT);
		Using_sRGB = false;
	}
#endif
}

// from HUDRenderer_SW.cpp
SDL_Surface *rotate_surface(SDL_Surface *s, int width, int height);

SDL_Surface *flip_surface(SDL_Surface *s, int width, int height)
{
	if (!s) return 0;
    
	SDL_Surface *s2 = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, s->format->BitsPerPixel, s->format->Rmask, s->format->Gmask, s->format->Bmask, s->format->Amask);
    
    int byteWidth = s->pitch;
    pixel8 *dst_pixels = (pixel8 *)s2->pixels;
    pixel8 *src_pixels = (pixel8 *)s->pixels;
    
    for (int y = 0; y < height; y++)
    {
        int yi = height - y - 1;
        for (int x = 0; x < byteWidth; x++)
        {
            dst_pixels[yi * byteWidth + x] = src_pixels[y * byteWidth + x];
        }
    }
    
	if (s->format->palette)
		SDL_SetPaletteColors(s2->format->palette, s->format->palette->colors, 0, s->format->palette->ncolors);
    
	return s2;
}	

void Shape_Blitter::SDL_Draw(SDL_Surface *dst_surface, const Image_Rect& dst)
{
    if (!dst_surface)
		return;
	
    // load shape into surface if necessary
    if (!m_surface)
    {
        byte *pixelsOut = NULL;
        SDL_Surface *tmp = get_shape_surface(m_frame, m_coll, &pixelsOut, (m_type == Shape_Texture_Interface) ? -1.0 : 1.0);
        if (!tmp)
            return;
        
        if (pixelsOut)
        {
			m_surface = SDL_ConvertSurfaceFormat(tmp, SDL_PIXELFORMAT_BGRA8888, 0);
            SDL_FreeSurface(tmp);
            free(pixelsOut);
            pixelsOut = NULL;
        }
        else if (shape_is_motion_blip(m_coll, m_frame))
        {
            // fix transparency on motion sensor blips
            SDL_SetColorKey(tmp, SDL_TRUE, 0);
			m_surface = SDL_ConvertSurfaceFormat(tmp, SDL_PIXELFORMAT_BGRA8888, 0);
            SDL_FreeSurface(tmp);
        }
        else
		{
			m_surface = SDL_ConvertSurfaceFormat(tmp, SDL_PIXELFORMAT_BGRA8888, 0);
			SDL_FreeSurface(tmp);
		}
    }
    if (!m_surface)
        return;
    
    if (!m_scaled_surface ||
        m_scaled_surface->w != m_scaled_src.w ||
        m_scaled_surface->h != m_scaled_src.h)
    {
        if (m_scaled_surface && (m_scaled_surface != m_surface))
            SDL_FreeSurface(m_scaled_surface);
        
        if (m_scaled_src.w != m_src.w || m_scaled_src.h != m_src.h)
            m_scaled_surface = rescale_surface(m_surface, m_scaled_src.w, m_scaled_src.h);
        else
            m_scaled_surface = m_surface;
        
        if (m_type == Shape_Texture_Wall)
        {
            // rotate wall textures
            SDL_Surface *tmp = rotate_surface(m_scaled_surface, m_scaled_surface->w, m_scaled_surface->h);
            if (m_scaled_surface != m_surface)
                SDL_FreeSurface(m_scaled_surface);
            m_scaled_surface = tmp;
        }
        else if (m_type == Shape_Texture_Landscape)
        {
            // flip landscapes vertically
            SDL_Surface *tmp = flip_surface(m_scaled_surface, m_scaled_surface->w, m_scaled_surface->h);
            if (m_scaled_surface != m_surface)
                SDL_FreeSurface(m_scaled_surface);
            m_scaled_surface = tmp;
        }
    }
    
    if (!m_scaled_surface)
        return;
    
	SDL_Rect r = { int(crop_rect.x), int(crop_rect.y), int(crop_rect.w), int(crop_rect.h) };
	SDL_Rect sdst = { int(dst.x), int(dst.y), int(dst.w), int(dst.h) };
	SDL_BlitSurface(m_scaled_surface, &r, dst_surface, &sdst);
}

Shape_Blitter::~Shape_Blitter()
{
    SDL_FreeSurface(m_surface);
    if (m_scaled_surface != m_surface)
    SDL_FreeSurface(m_scaled_surface);
}
