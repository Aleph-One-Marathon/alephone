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
#include "OGL_Blitter.h"

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
static HUD_OGL_Class HUD_OGL;


static OGL_Blitter HUD_Blitter;  // HUD backdrop storage
static bool hud_pict_not_found = false;	// HUD backdrop picture not found, don't try again to load it

extern int LuaTexturePaletteSize();

void OGL_DrawHUD(Rect &dest, short time_elapsed)
{	
	// Load static HUD picture if necessary
	if (!HUD_Blitter.Loaded() && !hud_pict_not_found) {
		LoadedResource PictRsrc;
		if (get_picture_resource_from_images(INTERFACE_PANEL_BASE, PictRsrc)) {
			// Render picture into SDL surface, convert to GL textures
			SDL_Surface *hud_pict = picture_to_surface(PictRsrc);
			if (hud_pict) {
				HUD_Blitter.Load(*hud_pict);
				SDL_FreeSurface(hud_pict);
			}
		}
		else
			hud_pict_not_found = true;
	}

	glPushAttrib(GL_ALL_ATTRIB_BITS);

	if (LuaTexturePaletteSize())
		glDisable(GL_TEXTURE_2D);
	else
		glEnable(GL_TEXTURE_2D);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_FOG);

	// Draw static HUD picture
	if (HUD_Blitter.Loaded() && !LuaTexturePaletteSize())
	{
		SDL_Rect hud_dest = { dest.left, dest.top, dest.right - dest.left, dest.bottom - dest.top };
		HUD_Blitter.Draw(hud_dest);
	}
	else
	{
		glColor3ub(0, 0, 0);
		glBegin(GL_QUADS);
		glVertex2i(dest.left,  dest.top);
		glVertex2i(dest.right, dest.top);
		glVertex2i(dest.right, dest.bottom);
		glVertex2i(dest.left,  dest.bottom);
		glEnd();
	}
	
	GLdouble x_scale = (dest.right - dest.left) / 640.0;
	GLdouble y_scale = (dest.bottom - dest.top) / 160.0;
	glScissor(dest.left, dest.bottom, 640.0 * x_scale, 160.0 * y_scale);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslated(dest.left, dest.top - (320.0 * y_scale), 0.0);
	glScaled(x_scale, y_scale, 1.0);

	// Add dynamic elements (redraw everything)
	mark_weapon_display_as_dirty();
	mark_ammo_display_as_dirty();
	mark_shield_display_as_dirty();
	mark_oxygen_display_as_dirty();
	mark_player_inventory_as_dirty(current_player_index, NONE);
	HUD_OGL.update_everything(time_elapsed);
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glPopAttrib();
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
}


/*
 *  Update motion sensor
 */

void HUD_OGL_Class::update_motion_sensor(short time_elapsed)
{
	if (!(GET_GAME_OPTIONS() & _motion_sensor_does_not_work) && MotionSensorActive) {
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
	TMgr.SetupTextureMatrix();
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
	TMgr.RestoreTextureMatrix();
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
	TMgr.SetupTextureMatrix();
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
	TMgr.RestoreTextureMatrix();
}

void HUD_OGL_Class::DrawTexture(shape_descriptor shape, short x, short y, int size)
{
	// Set up texture
	TextureManager TMgr;
	TMgr.ShapeDesc = shape;
	get_shape_bitmap_and_shading_table(shape, &TMgr.Texture, &TMgr.ShadingTables, _shading_normal);
	TMgr.IsShadeless = false;
	TMgr.TransferMode = _shadeless_transfer;
	TMgr.TextureType = OGL_Txtr_Wall;
	if (!TMgr.Setup())
		return;

	// Get dimensions
	int width = size, height = size;
//	int width = TMgr.Texture->width, height = TMgr.Texture->height;
	GLdouble U_Scale = TMgr.U_Scale;
	GLdouble V_Scale = TMgr.V_Scale;
	GLdouble U_Offset = TMgr.U_Offset;
	GLdouble V_Offset = TMgr.V_Offset;

	// Draw shape
	glColor3f(1.0, 1.0, 1.0);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	TMgr.SetupTextureMatrix();
	TMgr.RenderNormal();
	glBegin(GL_TRIANGLE_FAN);
		glTexCoord2d(U_Offset, V_Offset);
		glVertex2i(x, y);
		glTexCoord2d(U_Offset, V_Offset + V_Scale);
		glVertex2i(x + width, y);
		glTexCoord2d(U_Offset + U_Scale, V_Offset + V_Scale);
		glVertex2i(x + width, y + height);
		glTexCoord2d(U_Offset + U_Scale, V_Offset);
		glVertex2i(x, y + height);
	glEnd();
	TMgr.RestoreTextureMatrix();
	if (TMgr.IsGlowMapped()) TMgr.RenderGlowing();
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

#define MESSAGE_AREA_X_OFFSET 291
#define MESSAGE_AREA_Y_OFFSET 321

void HUD_OGL_Class::draw_message_area(short)
{
	{
		DrawShapeAtXY(
			BUILD_DESCRIPTOR(_collection_interface, _network_panel), 
			MESSAGE_AREA_X_OFFSET, MESSAGE_AREA_Y_OFFSET);
		draw_player_name();
	}
}


#endif // def HAVE_OPENGL
