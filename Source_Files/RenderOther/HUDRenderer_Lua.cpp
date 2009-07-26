/*
HUD_RENDERER_LUA.CPP

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

    Implements HUD helper class for Lua HUD themes
*/

#include "HUDRenderer_Lua.h"

#ifdef HAVE_OPENGL

#include "FontHandler.h"
#include "Image_Blitter.h"
#include "Shape_Blitter.h"

#include "lua_hud_script.h"
#include "shell.h"
#include "screen.h"

#ifdef HAVE_OPENGL
# if defined (__APPLE__) && defined (__MACH__)
#   include <OpenGL/gl.h>
# else
#   include <GL/gl.h>
# endif
#endif

#include <math.h>

#if defined(__WIN32__) || defined(__MINGW32__)
#undef DrawText
#endif

extern bool MotionSensorActive;


// Rendering object
static HUD_Lua_Class HUD_Lua;


HUD_Lua_Class *Lua_HUDInstance()
{
	return &HUD_Lua;
}

void Lua_DrawHUD(short time_elapsed)
{
	HUD_Lua.update_motion_sensor(time_elapsed);
	HUD_Lua.start_draw();
	L_Call_HUDDraw();
	HUD_Lua.end_draw();
}

/*
 *  Update motion sensor
 */

void HUD_Lua_Class::update_motion_sensor(short time_elapsed)
{
	if (!(GET_GAME_OPTIONS() & _motion_sensor_does_not_work) && MotionSensorActive) {
		if (time_elapsed == NONE)
			reset_motion_sensor(current_player_index);
		motion_sensor_scan(time_elapsed);
	}
}

void HUD_Lua_Class::clear_entity_blips(void)
{
	m_blips.clear();
}

void HUD_Lua_Class::add_entity_blip(short mtype, short intensity, short x, short y)
{
	world_point2d origin, target;
	origin.x = origin.y = 0;
	target.x = x;
	target.y = y;
	
	blip_info info;
	info.mtype = mtype;
	info.intensity = intensity;
	info.distance = distance2d(&origin, &target);
	info.direction = arctangent(x, y);
	
	m_blips.push_back(info);
}

size_t HUD_Lua_Class::entity_blip_count(void)
{
	return m_blips.size();
}

blip_info HUD_Lua_Class::entity_blip(size_t index)
{
	return m_blips[index];
}

void HUD_Lua_Class::start_draw(void)
{
	m_opengl = (get_screen_mode()->acceleration == _opengl_acceleration);
	
	if (m_opengl)
	{
		glPushAttrib(GL_ALL_ATTRIB_BITS);
		glEnable(GL_TEXTURE_2D);
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_ALPHA_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_FOG);
		
		m_surface = NULL;
	}
	else
	{
		if (m_surface &&
				(m_surface->w != SDL_GetVideoSurface()->w ||
				 m_surface->h != SDL_GetVideoSurface()->h))
		{
			SDL_FreeSurface(m_surface);
			m_surface = NULL;
		}
		if (!m_surface)
		{
			m_surface = SDL_DisplayFormatAlpha(SDL_GetVideoSurface());
			SDL_SetAlpha(m_surface, SDL_SRCALPHA, 0);
		}
		SDL_SetClipRect(m_surface, NULL);
		SDL_FillRect(m_surface, NULL, SDL_MapRGBA(m_surface->format, 0, 0, 0, 0));
		
		SDL_SetAlpha(SDL_GetVideoSurface(), SDL_SRCALPHA, 0xff);
	}
	
	
	m_drawing = true;
}

void HUD_Lua_Class::end_draw(void)
{
	m_drawing = false;
	
	if (m_opengl)
	{
		glPopAttrib();
	}
	else if (m_surface)
	{
//		SDL_BlitSurface(m_surface, NULL, SDL_GetVideoSurface(), NULL);
		SDL_SetAlpha(SDL_GetVideoSurface(), 0, 0xff);
		SDL_SetClipRect(SDL_GetVideoSurface(), 0);
	}
}

void HUD_Lua_Class::apply_clip(void)
{
	alephone::Screen *scr = alephone::Screen::instance();
	
	if (m_surface)
	{
		SDL_SetClipRect(SDL_GetVideoSurface(), &scr->lua_clip_rect);
	}
	else if (m_opengl)
	{
		if (scr->lua_clip_rect.x == 0 && scr->lua_clip_rect.y == 0 &&
				scr->lua_clip_rect.w == scr->width() &&
				scr->lua_clip_rect.h == scr->height())
		{
			glDisable(GL_SCISSOR_TEST);
		}
		else
		{
			int top = scr->lua_clip_rect.y;
			int bot = top + scr->lua_clip_rect.h;
			glEnable(GL_SCISSOR_TEST);
			glScissor(scr->lua_clip_rect.x,
								scr->height() - scr->lua_clip_rect.y - scr->lua_clip_rect.h,
								scr->lua_clip_rect.w, scr->lua_clip_rect.h);
		}
	}
}

void HUD_Lua_Class::fill_rect(float x, float y, float w, float h,
															float r, float g, float b, float a)
{
	if (!m_drawing)
		return;
	
	apply_clip();
	if (m_opengl)
	{
		glColor4f(r, g, b, a);
		glDisable(GL_TEXTURE_2D);
		glBegin(GL_QUADS);

		glVertex2f(x,     y);
		glVertex2f(x + w, y);
		glVertex2f(x + w, y + h);
		glVertex2f(x,     y + h);

		glEnd();
		glEnable(GL_TEXTURE_2D);
	}
	else if (m_surface)
	{
		SDL_Rect rect;
		rect.x = x;
		rect.y = y;
		rect.w = w;
		rect.h = h;
		SDL_FillRect(m_surface, &rect,
								 SDL_MapRGBA(m_surface->format, r * 255, g * 255, b * 255, a * 255));
		SDL_BlitSurface(m_surface, &rect, SDL_GetVideoSurface(), &rect);
	}
}	

void HUD_Lua_Class::frame_rect(float x, float y, float w, float h,
												 			 float r, float g, float b, float a,
															 float t)
{
	if (!m_drawing)
		return;
		
	apply_clip();
	if (m_opengl)
	{
		glColor4f(r, g, b, a);
		glDisable(GL_TEXTURE_2D);
		glBegin(GL_QUADS);
		
		glVertex2f(x,     y);
		glVertex2f(x + w, y);
		glVertex2f(x + w, y + t);
		glVertex2f(x,     y + t);
		
		glVertex2f(x,     y + h - t);
		glVertex2f(x + w, y + h - t);
		glVertex2f(x + w, y + h);
		glVertex2f(x,     y + h);
		
		glVertex2f(x,     y + t);
		glVertex2f(x + t, y + t);
		glVertex2f(x + t, y + h - t);
		glVertex2f(x,     y + h - t);
		
		glVertex2f(x + w - t, y + t);
		glVertex2f(x + w,     y + t);
		glVertex2f(x + w,     y + h - t);
		glVertex2f(x + w - t, y + h - t);
		
		glEnd();
		glEnable(GL_TEXTURE_2D);
	}
	else if (m_surface)
	{
		Uint32 color = SDL_MapRGBA(m_surface->format, r * 255, g * 255, b * 255, a * 255);
		SDL_Rect rect;
		rect.x = x;
		rect.w = w;
		rect.y = y;
		rect.h = t;
		SDL_FillRect(m_surface, &rect, color);
		SDL_BlitSurface(m_surface, &rect, SDL_GetVideoSurface(), &rect);
		rect.x = x;
		rect.w = w;
		rect.y = y + h - t;
		rect.h = t;
		SDL_FillRect(m_surface, &rect, color);
		SDL_BlitSurface(m_surface, &rect, SDL_GetVideoSurface(), &rect);
		rect.x = x;
		rect.w = t;
		rect.y = y + t;
		rect.h = h - t - t;
		SDL_FillRect(m_surface, &rect, color);
		SDL_BlitSurface(m_surface, &rect, SDL_GetVideoSurface(), &rect);
		rect.x = x + w - t;
		rect.w = t;
		rect.y = y + t;
		rect.h = h - t - t;
		SDL_FillRect(m_surface, &rect, color);
		SDL_BlitSurface(m_surface, &rect, SDL_GetVideoSurface(), &rect);
	}
}	

void HUD_Lua_Class::draw_text(FontSpecifier *font, const char *text,
															float x, float y,
															float r, float g, float b, float a)
{
	if (!m_drawing)
		return;
	
	apply_clip();
	if (m_opengl)
	{
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glTranslatef(x, y + font->Height, 0);
		glColor4f(r, g, b, a);
		font->OGL_Render(text);
		glColor4f(1, 1, 1, 1);
		glPopMatrix();
	}
	else if (m_surface)
	{
		SDL_Rect rect;
		rect.x = x;
		rect.y = y;
		rect.w = font->TextWidth(text);
		rect.h = font->LineSpacing;
		SDL_BlitSurface(SDL_GetVideoSurface(), &rect, m_surface, &rect);
		font->Info->draw_text(m_surface, text, strlen(text),
												  x, y + font->Height,
												  SDL_MapRGBA(m_surface->format,
																		  r * 255, g * 255, b * 255, a * 255),
												  font->Style);
	  SDL_BlitSurface(m_surface, &rect, SDL_GetVideoSurface(), &rect);
	}
}

void HUD_Lua_Class::draw_image(Image_Blitter *image, float x, float y)
{
	if (!m_drawing)
		return;
	
	SDL_Rect r;
	r.x = x;
	r.y = y;
	r.w = image->crop_rect.w;
	r.h = image->crop_rect.h;

	apply_clip();
	image->Draw(SDL_GetVideoSurface(), r);
}

void HUD_Lua_Class::draw_shape(Shape_Blitter *shape, float x, float y)
{
	if (!m_drawing)
		return;
	
	SDL_Rect r;
	r.x = x;
	r.y = y;
	r.w = shape->crop_rect.w;
	r.h = shape->crop_rect.h;
    
	apply_clip();
    if (m_opengl)
    {
        shape->OGL_Draw(r);
    }
    else if (m_surface)
    {
        shape->SDL_Draw(SDL_GetVideoSurface(), r);
    }
}


#endif // def HAVE_OPENGL
