/*
 *  HUDRenderer_SW.cpp - HUD rendering using graphics functions from screen_drawing
 *
 *  Written in 2001 by Christian Bauer
 */

#include "HUDRenderer_SW.h"

extern bool MotionSensorActive;


/*
 *  Update motion sensor
 */

void HUD_SW_Class::update_motion_sensor(short time_elapsed)
{
	if (!MotionSensorActive)
		GET_GAME_OPTIONS() |= _motion_sensor_does_not_work;
	
	if (!(GET_GAME_OPTIONS() & _motion_sensor_does_not_work)) {
		if (time_elapsed == NONE) {
			reset_motion_sensor(current_player_index);
			ForceUpdate = true;
		}

		motion_sensor_scan(time_elapsed);
		
		if (motion_sensor_has_changed()) {
			ForceUpdate = true;
			screen_rectangle *r = get_interface_rectangle(_motion_sensor_rect);
			DrawShapeAtXY(BUILD_DESCRIPTOR(_collection_interface, _motion_sensor_mount), r->left, r->top);
		}
	}
}


/*
 *  Draw shapes
 */

void HUD_SW_Class::DrawShape(shape_descriptor shape, screen_rectangle *dest, screen_rectangle *src)
{
	_draw_screen_shape(shape, dest, src);
}

void HUD_SW_Class::DrawShapeAtXY(shape_descriptor shape, short x, short y, bool transparency)
{
	// "transparency" is only used for OpenGL motion sensor
	_draw_screen_shape_at_x_y(shape, x, y);
}


/*
 *  Draw text
 */

void HUD_SW_Class::DrawText(const char *text, screen_rectangle *dest, short flags, short font_id, short text_color)
{
	_draw_screen_text(text, dest, flags, font_id, text_color);
}


/*
 *  Fill rectangle
 */

void HUD_SW_Class::FillRect(screen_rectangle *r, short color_index)
{
	_fill_rect(r, color_index);
}


/*
 *  Frame rectangle
 */

void HUD_SW_Class::FrameRect(screen_rectangle *r, short color_index)
{
	_frame_rect(r, color_index);
}
