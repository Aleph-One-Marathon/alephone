/*
 *  HUDRenderer_OGL.cpp - HUD rendering using OpenGL
 *
 *  Written in 2001 by Christian Bauer
 */

#include "HUDRenderer_OGL.h"

#ifdef HAVE_OPENGL

#include "FontHandler.h"

#include "render.h"
#include "scottish_textures.h"

#include "OGL_Setup.h"
#include "OGL_Textures.h"

#include <GL/gl.h>

#include <math.h>

extern bool MotionSensorActive;


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
		glTexCoord2f(U_Offset, V_Offset);
		glVertex2i(x, y);
		glTexCoord2f(U_Offset + U_Scale, V_Offset);
		glVertex2i(x + width, y);
		glTexCoord2f(U_Offset + U_Scale, V_Offset + V_Scale);
		glVertex2i(x + width, y + height);
		glTexCoord2f(U_Offset, V_Offset + V_Scale);
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
		glTexCoord2f(U_Offset, V_Offset);
		glVertex2i(x, y);
		glTexCoord2f(U_Offset + U_Scale, V_Offset);
		glVertex2i(x + width, y);
		glTexCoord2f(U_Offset + U_Scale, V_Offset + V_Scale);
		glVertex2i(x + width, y + height);
		glTexCoord2f(U_Offset, V_Offset + V_Scale);
		glVertex2i(x, y + height);
	glEnd();
}


/*
 *  Draw text
 */

void HUD_OGL_Class::DrawText(const char *text, screen_rectangle *dest, short flags, short font_id, short text_color)
{
	// Get color
	const rgb_color &c = get_interface_color(text_color);
	glColor3f(c.red / 65535.0, c.green / 65535.0, c.blue / 65535.0);

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
	glColor3f(c.red / 65535.0, c.green / 65535.0, c.blue / 65535.0);

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
	glColor3f(c.red / 65535.0, c.green / 65535.0, c.blue / 65535.0);

	// Draw rectangle
	glDisable(GL_TEXTURE_2D);
	glLineWidth(1);
	glBegin(GL_LINE_LOOP);
		glVertex2i(r->left, r->top + 1);
		glVertex2i(r->right - 1, r->top + 1);
		glVertex2i(r->right - 1, r->bottom);
		glVertex2i(r->left, r->bottom);
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
	GLdouble blip_dist = sqrt(x*x+y*y);
	if (blip_dist <= 2.0)
		return;
	GLdouble normal_x = x / blip_dist, normal_y = y / blip_dist;
	GLdouble tan_pt_x = c_x + normal_x * radius, tan_pt_y = c_y + normal_y * radius;

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
