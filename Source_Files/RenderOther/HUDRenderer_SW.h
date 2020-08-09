/*

	Copyright (C) 1991-2001 and beyond by Bungie Studios, Inc.
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

*/

/*
 *  HUDRenderer_SW.h - HUD rendering using graphics functions from screen_drawing
 *
 *  Written in 2001 by Christian Bauer
 */

#ifndef _HUD_RENDERER_SW_H_
#define _HUD_RENDERER_SW_H_

#include "HUDRenderer.h"

class HUD_SW_Class : public HUD_Class
{
public:
	HUD_SW_Class() {}
	~HUD_SW_Class() {}

protected:
	void update_motion_sensor(short time_elapsed);
	void render_motion_sensor(short time_elapsed);
	void draw_or_erase_unclipped_shape(short x, short y, shape_descriptor shape, bool draw);
	void draw_entity_blip(point2d *location, shape_descriptor shape);

	void DrawShape(shape_descriptor shape, screen_rectangle *dest, screen_rectangle *src);
	void DrawShapeAtXY(shape_descriptor shape, short x, short y, bool transparency = false);
	void DrawText(const char *text, screen_rectangle *dest, short flags, short font_id, short text_color);
	void FillRect(screen_rectangle *r, short color_index);
	void FrameRect(screen_rectangle *r, short color_index);

	void DrawTexture(shape_descriptor shape, short texture_type, short x, short y, int size);

	void SetClipPlane(int x, int y, int c_x, int c_y, int radius) {}
	void DisableClipPlane(void) {}
};

#endif
