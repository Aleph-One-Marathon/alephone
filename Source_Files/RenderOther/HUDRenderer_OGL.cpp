/*

	Copyright (C) 1991-2001 and beyond by Bungie Studios, Inc.
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

*/

/*
 *  HUDRenderer_OGL.cpp - HUD rendering using OpenGL
 *
 *  Written in 2001 by Christian Bauer
 */

#include "HUDRenderer_OGL.h"

#ifdef HAVE_OPENGL

#include "FontHandler.h"

#include "game_window.h"
#include "screen_definitions.h"
#include "images.h"
#include "render.h"
#include "scottish_textures.h"

#include "OGL_Setup.h"
#include "OGL_Textures.h"

#ifdef HAVE_OPENGL
# if defined (__APPLE__) && defined (__MACH__)
#   include <OpenGL/gl.h>
# elif defined mac
#   include <gl.h>
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
static HUD_OGL_Class HUD_OGL;


// MacOS HUD Buffer; defined in screen.cpp
#if defined(mac)
extern GWorldPtr HUD_Buffer;
#endif


/*
 *  Draws the entire interface using OpenGL
 */

// We are using 6 textures to render the 640x160 pixel HUD:
//
//     256      256    128
//  +--------+--------+----+
//  |   0    |   1    | 2  | 128
//  |        |        |    |
//  +--------+--------+----+
//  |   3    |   4    | 5  | 32
//  +--------+--------+----+
const int NUM_TEX = 6;
static GLuint txtr_id[NUM_TEX];

static bool hud_pict_loaded = false;	// HUD backdrop picture loaded and ready
static bool hud_pict_not_found = false;	// HUD backdrop picture not found, don't try again to load it

void OGL_DrawHUD(Rect &dest, short time_elapsed)
{
	static const int txtr_width[NUM_TEX] = {256, 256, 128, 256, 256, 128};
	static const int txtr_height[NUM_TEX] = {128, 128, 128, 32, 32, 32};
	static const int txtr_x[NUM_TEX] = {0, 256, 512, 0, 256, 512};
	static const int txtr_y[NUM_TEX] = {0, 0, 0, 128, 128, 128};

	if(hud_pict_loaded && time_elapsed == NONE)
	{
		glDeleteTextures(NUM_TEX, txtr_id);
		hud_pict_loaded= false;
	}
	
	// Load static HUD picture if necessary
	if (!hud_pict_loaded && !hud_pict_not_found) {
		LoadedResource PictRsrc;
		if (get_picture_resource_from_images(INTERFACE_PANEL_BASE, PictRsrc))
			hud_pict_loaded = true;
		else
			hud_pict_not_found = true;

		if (hud_pict_loaded) {
			uint8 *txtr_data[NUM_TEX];
			for (int i=0; i<NUM_TEX; i++)
				txtr_data[i] = new uint8[txtr_width[i] * txtr_height[i] * 4];

#if defined(mac)
			// Using already-rendered HUD buffer (LP)
			if (HUD_Buffer)
			{
				// Get the pixels to copy in
				PixMapHandle Pxls = GetGWorldPixMap(HUD_Buffer);
				LockPixels(Pxls);
				
				// Row-start address and row length
				uint8 *SourceStart = (uint8 *)GetPixBaseAddr(Pxls);
				long StrideBytes = (**Pxls).rowBytes & 0x3fff;
				
				// Special-case it for 
				int SrcBytes = (**Pxls).pixelSize/8;
				switch(SrcBytes)
				{
				case 2:
				for (int i=0; i<NUM_TEX; i++)
				{
					uint8 *SrcLineStart = SourceStart + txtr_y[i]*StrideBytes + txtr_x[i]*SrcBytes;
					uint32 *DestPP = (uint32 *)txtr_data[i];
					for (int y=0; y<txtr_height[i]; y++)
					{
						uint8 *SrcPP = SrcLineStart;
						for (int x=0; x<txtr_width[i]; x++)
						{
							// 1555 ARGB to 8888 RGBA -- with A = 1 always
              				uint16 Intmd = *(SrcPP++);
            			    Intmd <<= 8;
            				Intmd |= uint16(*(SrcPP++));
            				*(DestPP++) = Convert_16to32(Intmd);
						}
						SrcLineStart += StrideBytes;
					}
				}
				break;
				
				case 4:
				for (int i=0; i<NUM_TEX; i++)
				{
					uint8 *SrcLineStart = SourceStart + txtr_y[i]*StrideBytes + txtr_x[i]*SrcBytes;
					uint8 *DestPP = txtr_data[i];
					for (int y=0; y<txtr_height[i]; y++)
					{
						uint8 *SrcPP = SrcLineStart;
						for (int x=0; x<txtr_width[i]; x++)
						{
							// 8888 ARGB to RGBA -- with A = 1 always
							SrcPP++;
							*(DestPP++) = *(SrcPP++);
							*(DestPP++) = *(SrcPP++);
							*(DestPP++) = *(SrcPP++);
							*(DestPP++) = 0xff;
						}
						SrcLineStart += StrideBytes;
					}
				}
				}
				
				// Done!
				UnlockPixels(Pxls);
			}
			
#elif defined(SDL)
			// Render picture into SDL surface, convert to GL textures
			SDL_Surface *hud_pict = picture_to_surface(PictRsrc);
			if (hud_pict) {
				SDL_Surface *hud_pict_rgb = SDL_CreateRGBSurface(SDL_SWSURFACE, 640, 160, 32, 0xff0000, 0x00ff00, 0x0000ff, 0);
				if (hud_pict_rgb) {
					SDL_Rect rect = {0, 0, 640, 160};
					SDL_BlitSurface(hud_pict, &rect, hud_pict_rgb, &rect);
					for (int i=0; i<NUM_TEX; i++) {
						uint32 *p = (uint32 *)((uint8 *)hud_pict_rgb->pixels + txtr_y[i] * hud_pict_rgb->pitch) + txtr_x[i];
						uint8 *q = txtr_data[i];
						for (int y=0; y<txtr_height[i]; y++) {
							for (int x=0; x<txtr_width[i]; x++) {
								uint32 v = p[x];
								*q++ = v >> 16;
								*q++ = v >> 8;
								*q++ = v;
								*q++ = 0xff;
							}
							p = (uint32 *)((uint8 *)p + hud_pict_rgb->pitch);
						}
					}
					SDL_FreeSurface(hud_pict_rgb);
				}
				SDL_FreeSurface(hud_pict);
			}
#endif

			glGenTextures(NUM_TEX, txtr_id);
			for (int i=0; i<NUM_TEX; i++) {
				glBindTexture(GL_TEXTURE_2D, txtr_id[i]);
				glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, txtr_width[i], txtr_height[i],
					0, GL_RGBA, GL_UNSIGNED_BYTE, txtr_data[i]);
				delete[] txtr_data[i];
			}

			hud_pict_loaded = true;
		}
	}

	if (hud_pict_loaded) {

		glPushAttrib(GL_ALL_ATTRIB_BITS);

		glEnable(GL_TEXTURE_2D);
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_ALPHA_TEST);
		glDisable(GL_BLEND);
		glDisable(GL_FOG);

		// Draw static HUD picture
		for (int i=0; i<NUM_TEX; i++) {
			glBindTexture(GL_TEXTURE_2D, txtr_id[i]);
			glColor3f(1.0, 1.0, 1.0);
			glBegin(GL_TRIANGLE_FAN);
				glTexCoord2f(0.0, 0.0);
				glVertex2i(txtr_x[i] + dest.left, txtr_y[i] + dest.top);
				glTexCoord2f(1.0, 0.0);
				glVertex2i(txtr_x[i] + txtr_width[i] + dest.left, txtr_y[i] + dest.top);
				glTexCoord2f(1.0, 1.0);
				glVertex2i(txtr_x[i] + txtr_width[i] + dest.left, txtr_y[i] + txtr_height[i] + dest.top);
				glTexCoord2f(0.0, 1.0);
				glVertex2i(txtr_x[i] + dest.left, txtr_y[i] + txtr_height[i] + dest.top);
			glEnd();
		}

		// Add dynamic elements (redraw everything)
		mark_weapon_display_as_dirty();
		mark_ammo_display_as_dirty();
		mark_shield_display_as_dirty();
		mark_oxygen_display_as_dirty();
		mark_player_inventory_as_dirty(current_player_index, NONE);
		glScissor(dest.left, dest.bottom, 640, 160);
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glTranslated(dest.left, dest.top - 320, 0.0);
		HUD_OGL.update_everything(time_elapsed);
		glPopMatrix();

		glPopAttrib();
	}
}


/*
 *  Reset HUD textures
 */

void OGL_ResetHUDFonts(bool IsStarting)
{
	get_interface_font(_interface_font).OGL_Reset(IsStarting);
	get_interface_font(_interface_item_count_font).OGL_Reset(IsStarting);
	get_interface_font(_weapon_name_font).OGL_Reset(IsStarting);
	get_interface_font(_player_name_font).OGL_Reset(IsStarting);

	if (IsStarting && hud_pict_loaded) {
		glDeleteTextures(NUM_TEX, txtr_id);
		hud_pict_loaded = false;
	}
}


/*
 *  Update motion sensor
 */

void HUD_OGL_Class::update_motion_sensor(short time_elapsed)
{
	if (!MotionSensorActive)
		GET_GAME_OPTIONS() |= _motion_sensor_does_not_work;
	
	if (!(GET_GAME_OPTIONS() & _motion_sensor_does_not_work)) {
		if (time_elapsed == NONE)
			reset_motion_sensor(current_player_index);
		motion_sensor_scan(time_elapsed);
	}
}


/*
 *  Draw shapes
 */

void HUD_OGL_Class::DrawShape(shape_descriptor shape, screen_rectangle *dest, screen_rectangle *src)
{
	// Set up texture
	TextureManager TMgr;
	TMgr.ShapeDesc = shape;
	get_shape_bitmap_and_shading_table(shape, &TMgr.Texture, &TMgr.ShadingTables, _shading_normal);
	TMgr.IsShadeless = true;
	TMgr.TransferMode = _shadeless_transfer;
	TMgr.TextureType = OGL_Txtr_WeaponsInHand;
	if (!TMgr.Setup())
		return;

	// Get dimensions
	int orig_width = TMgr.Texture->width, orig_height = TMgr.Texture->height;
	int x = dest->left, y = dest->top;
	int width = dest->right - dest->left, height = dest->bottom - dest->top;
	GLdouble U_Scale = TMgr.U_Scale * (src->right - src->left) / orig_width;
	GLdouble V_Scale = TMgr.V_Scale * (src->bottom - src->top) / orig_height;
	GLdouble U_Offset = TMgr.U_Offset + TMgr.U_Scale * src->left / orig_width;
	GLdouble V_Offset = TMgr.V_Offset + TMgr.V_Scale * src->top / orig_height;

	// Draw shape
	glColor3f(1.0, 1.0, 1.0);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	TMgr.RenderNormal();
	glBegin(GL_TRIANGLE_FAN);
		glTexCoord2d(U_Offset, V_Offset);
		glVertex2i(x, y);
		glTexCoord2d(U_Offset + U_Scale, V_Offset);
		glVertex2i(x + width, y);
		glTexCoord2d(U_Offset + U_Scale, V_Offset + V_Scale);
		glVertex2i(x + width, y + height);
		glTexCoord2d(U_Offset, V_Offset + V_Scale);
		glVertex2i(x, y + height);
	glEnd();
}

void HUD_OGL_Class::DrawShapeAtXY(shape_descriptor shape, short x, short y, bool transparency)
{
	// Set up texture
	TextureManager TMgr;
	TMgr.ShapeDesc = shape;
	get_shape_bitmap_and_shading_table(shape, &TMgr.Texture, &TMgr.ShadingTables, _shading_normal);
	TMgr.IsShadeless = true;
	TMgr.TransferMode = _shadeless_transfer;
	TMgr.TextureType = OGL_Txtr_WeaponsInHand;
	if (!TMgr.Setup())
		return;

	// Get dimensions
	int width = TMgr.Texture->width, height = TMgr.Texture->height;
	GLdouble U_Scale = TMgr.U_Scale;
	GLdouble V_Scale = TMgr.V_Scale;
	GLdouble U_Offset = TMgr.U_Offset;
	GLdouble V_Offset = TMgr.V_Offset;

	// Draw shape
	glColor3f(1.0, 1.0, 1.0);
	glEnable(GL_TEXTURE_2D);
	if (transparency) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	} else
		glDisable(GL_BLEND);
	TMgr.RenderNormal();
	glBegin(GL_TRIANGLE_FAN);
		glTexCoord2d(U_Offset, V_Offset);
		glVertex2i(x, y);
		glTexCoord2d(U_Offset + U_Scale, V_Offset);
		glVertex2i(x + width, y);
		glTexCoord2d(U_Offset + U_Scale, V_Offset + V_Scale);
		glVertex2i(x + width, y + height);
		glTexCoord2d(U_Offset, V_Offset + V_Scale);
		glVertex2i(x, y + height);
	glEnd();
}


/*
 *  Draw text
 */

// WZ: Work around some Win32 oddness
#ifdef DrawText
#undef DrawText
#endif

void HUD_OGL_Class::DrawText(const char *text, screen_rectangle *dest, short flags, short font_id, short text_color)
{
	// Get color
	const rgb_color &c = get_interface_color(text_color);
	glColor3f(c.red / 65535.0F, c.green / 65535.0F, c.blue / 65535.0F);

	// Get font information
	FontSpecifier &FontData = get_interface_font(font_id);

	// Draw text
	FontData.OGL_DrawText(text, *dest, flags);
}


/*
 *  Fill rectangle
 */

void HUD_OGL_Class::FillRect(screen_rectangle *r, short color_index)
{
	// Get color
	const rgb_color &c = get_interface_color(color_index);
	glColor3f(c.red / 65535.0F, c.green / 65535.0F, c.blue / 65535.0F);

	// Draw rectangle
	glDisable(GL_TEXTURE_2D);
	glRecti(r->left, r->top, r->right, r->bottom);
}


/*
 *  Frame rectangle
 */

void HUD_OGL_Class::FrameRect(screen_rectangle *r, short color_index)
{
	// Get color
	const rgb_color &c = get_interface_color(color_index);
	glColor3f(c.red / 65535.0F, c.green / 65535.0F, c.blue / 65535.0F);

	// Draw rectangle
	glDisable(GL_TEXTURE_2D);
	glLineWidth(1);
	glBegin(GL_LINE_LOOP);
	glVertex2f(r->left + 0.5, r->top + 0.5);
	glVertex2f(r->right - 0.5, r->top + 0.5);
	glVertex2f(r->right - 0.5, r->bottom - 0.5);
	glVertex2f(r->left + 0.5, r->bottom - 0.5);
	glEnd();
}


/*
 *  Set clip plane for rendering a blip at (x, y) on the motion sensor.
 *  The plane gets attached tangential to the circle with the specified
 *  radius and center (this circle covers the entire motion sensor area).
 *  This should be a sufficient approximation to a circular clipping region
 *  for small blips.
 */

void HUD_OGL_Class::SetClipPlane(int x, int y, int c_x, int c_y, int radius)
{
	GLdouble blip_dist = sqrt(static_cast<float>(x*x+y*y));
	if (blip_dist <= 2.0)
		return;
	GLdouble normal_x = x / blip_dist, normal_y = y / blip_dist;
	GLdouble tan_pt_x = c_x + normal_x * radius + 0.5, tan_pt_y = c_y + normal_y * radius + 0.5;

	glEnable(GL_CLIP_PLANE0);

	GLdouble eqn[4] = {
		-normal_x, -normal_y, 0,
		normal_x * tan_pt_x + normal_y * tan_pt_y
	};
	glClipPlane(GL_CLIP_PLANE0, eqn);
}


/*
 *  Disable clip plane
 */

void HUD_OGL_Class::DisableClipPlane(void)
{
	glDisable(GL_CLIP_PLANE0);
}

#endif // def HAVE_OPENGL
