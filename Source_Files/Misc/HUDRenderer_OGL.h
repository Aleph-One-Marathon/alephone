/*
 *  HUDRenderer_OGL.h - HUD rendering using OpenGL
 *
 *  Written in 2001 by Christian Bauer
 */

#ifndef _HUD_RENDERER_OGL_H_
#define _HUD_RENDERER_OGL_H_

#include "HUDRenderer.h"

class HUD_OGL_Class : public HUD_Class
{
public:
	HUD_OGL_Class() {}
	~HUD_OGL_Class() {}

protected:
	void update_motion_sensor(short time_elapsed);
	void render_motion_sensor(short time_elapsed);
	void draw_or_erase_unclipped_shape(short x, short y, shape_descriptor shape, bool draw);
	void erase_entity_blip(point2d *location, shape_descriptor shape) {}
	void draw_entity_blip(point2d *location, shape_descriptor shape);

	void DrawShape(shape_descriptor shape, screen_rectangle *dest, screen_rectangle *src);
	void DrawShapeAtXY(shape_descriptor shape, short x, short y, bool transparency = false);
	void DrawText(const char *text, screen_rectangle *dest, short flags, short font_id, short text_color);
	void FillRect(screen_rectangle *r, short color_index);
	void FrameRect(screen_rectangle *r, short color_index);
};

#endif
