#ifndef __SCREEN_DRAWING_H
#define __SCREEN_DRAWING_H

/*
	SCREEN_DRAWING.H
	Monday, August 15, 1994 3:11:38 PM
	
Apr 21, 2000 (Loren Petrich): Added XML parser object for the rectangle set,
	and exporting color data for game_window color parser

Jul 2, 2000 (Loren Petrich):
	The HUD is now always buffered
*/

#include "XML_ElementParser.h"

/* Rectangles for the interface, etc.. */
/* rectangle id's */
enum {
	/* game window rectangles */
	_player_name_rect= 0,
	_oxygen_rect,
	_shield_rect,
	_motion_sensor_rect,
	_microphone_rect,
	_inventory_rect,
	_weapon_display_rect,
	
	/* interface rectangles */
	_new_game_button_rect,
	_load_game_button_rect,
	_gather_button_rect,
	_join_button_rect,
	_prefs_button_rect,
	_replay_last_button_rect,
	_save_last_button_rect,
	_replace_saved_button_rect,
	_credits_button_rect,
	_quit_button_rect,
	_center_button_rect,
	NUMBER_OF_INTERFACE_RECTANGLES
};

/* Colors for drawing.. */
enum {
	_energy_weapon_full_color,
	_energy_weapon_empty_color,
	_black_color,
	_inventory_text_color,
	_inventory_header_background_color,
	_inventory_background_color,
	PLAYER_COLOR_BASE_INDEX,
	
	_white_color= 14,
	_invalid_weapon_color,
	_computer_border_background_text_color,
	_computer_border_text_color,
	_computer_interface_text_color,
	_computer_interface_color_purple,
	_computer_interface_color_red,
	_computer_interface_color_pink,
	_computer_interface_color_aqua,
	_computer_interface_color_yellow,
	_computer_interface_color_brown,
	_computer_interface_color_blue
};

enum { /* justification flags for _draw_screen_text */
	_no_flags,
	_center_horizontal= 0x01,
	_center_vertical= 0x02,
	_right_justified= 0x04,
	_top_justified= 0x08,
	_bottom_justified= 0x10,
	_wrap_text= 0x20
};

enum { /* Fonts for the interface et al.. */
	_interface_font,
	_weapon_name_font,
	_player_name_font,
	_interface_item_count_font,
	_computer_interface_font,
	_computer_interface_title_font,
	_net_stats_font,
	NUMBER_OF_INTERFACE_FONTS
};

/* Structure for portable rectangles.  notice it is exactly same as Rect */
struct screen_rectangle {
	short top, left;
	short bottom, right;
};
typedef struct screen_rectangle screen_rectangle;

/* ------- Prototypes */
void initialize_screen_drawing(void);

void _set_port_to_screen_window(void);
void _set_port_to_gworld(void);
void _restore_port(void);

/* If source==NULL, source= the shapes bounding rectangle */
void _draw_screen_shape(shape_descriptor shape_id, screen_rectangle *destination, 
	screen_rectangle *source);
void _draw_screen_shape_at_x_y(shape_descriptor shape, short x, short y);
void _draw_screen_shape_centered(shape_descriptor shape, screen_rectangle *rectangle,
	short flags);
void _draw_screen_text(char *text, screen_rectangle *destination, 
	short flags, short font_id, short text_color);
short _text_width(char *buffer, short font_id);

void _erase_screen(short color_index);

void _scroll_window(short dy, short rectangle_id, short background_color_index);

void _fill_screen_rectangle(screen_rectangle *rectangle, short color_index);
	
screen_rectangle *get_interface_rectangle(short index);

short _get_font_line_height(short font_index);

void _fill_rect(screen_rectangle *rectangle, short color_index);

void _frame_rect(screen_rectangle *rectangle, short color_index);

void _offset_screen_rect(screen_rectangle *rect, short dx, short dy);

// LP addition: stuff to use a buffer for the Heads-Up Display
void _set_port_to_HUD();

// LP addition: get the parser for the interface rectangles
XML_ElementParser *InterfaceRectangles_GetParser();

// Makes the color parser use screen-drawing stuff
void SetColorParserToScreenDrawing();

#endif
