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
 *  HUDRenderer.h - HUD rendering base class and data
 *
 *  Written in 2001 by Christian Bauer
 */

#ifndef _HUD_RENDERER_H_
#define _HUD_RENDERER_H_

#include "cseries.h"
#include "map.h"
#include "interface.h"
#include "player.h"
#include "SoundManager.h"
#include "motion_sensor.h"
#include "items.h"
#include "weapons.h"
#include "network_games.h"
#include "screen_drawing.h"

/* --------- constants */

#define TEXT_INSET 2
#define NAME_OFFSET 23
#define TOP_OF_BAR_WIDTH 8

#define MOTION_SENSOR_SIDE_LENGTH 123
#define DELAY_TICKS_BETWEEN_OXYGEN_REDRAW (2*TICKS_PER_SECOND)
#define RECORDING_LIGHT_FLASHING_DELAY (TICKS_PER_SECOND)

#define MICROPHONE_STOP_CLICK_SOUND ((short) 1250)
#define MICROPHONE_START_CLICK_SOUND ((short) 1280)

#define TOP_OF_BAR_HEIGHT 4

/* ---------- flag macros */
/* Note for reference: */
/* enum { _weapon, _ammunition, _powerup, NUMBER_OF_ITEM_TYPES }; */

#define INVENTORY_MASK_BITS 0x0007
#define INVENTORY_DIRTY_BIT 0x0010
#define INTERFACE_DIRTY_BIT 0x0020

#define GET_CURRENT_INVENTORY_SCREEN(p) ((p)->interface_flags & INVENTORY_MASK_BITS)

#define INVENTORY_IS_DIRTY(p) ((p)->interface_flags & INVENTORY_DIRTY_BIT)
#define SET_INVENTORY_DIRTY_STATE(p, v) ((void)((v)?((p)->interface_flags|=(uint16)INVENTORY_DIRTY_BIT):((p)->interface_flags&=(uint16)~INVENTORY_DIRTY_BIT)))

#define INTERFACE_IS_DIRTY(p) ((p)->interface_flags & INTERFACE_DIRTY_BIT)
#define SET_INTERFACE_DIRTY_STATE(p, v) ((v)?((p)->interface_flags |= INTERFACE_DIRTY_BIT):(p)->interface_flags &= ~INTERFACE_DIRTY_BIT)

/* ---------- enums */
/* texture id's */
enum {
	_empty_energy_bar=0,
	_energy_bar,
	_energy_bar_right,
	_double_energy_bar,
	_double_energy_bar_right,
	_triple_energy_bar,
	_triple_energy_bar_right,
	_empty_oxygen_bar,
	_oxygen_bar,
	_oxygen_bar_right,
	_motion_sensor_mount,
	_motion_sensor_virgin_mount,
	_motion_sensor_alien,
	_motion_sensor_friend= _motion_sensor_alien+6,
	_motion_sensor_enemy= _motion_sensor_friend+6,
	_network_panel= _motion_sensor_enemy+6,

	_magnum_bullet,
	_magnum_casing,
	_assault_rifle_bullet,
	_assault_rifle_casing,
	_alien_weapon_panel,
	_flamethrower_panel,
	_magnum_panel,
	_left_magnum,
	_zeus_panel,
	_assault_panel,
	_missile_panel,
	_left_magnum_unusable,
	_assault_rifle_grenade,
	_assault_rifle_grenade_casing,
	_shotgun_bullet,
	_shotgun_casing,
	_single_shotgun,
	_double_shotgun,
	_missile,
	_missile_casing,
	
	_network_compass_shape_nw,
	_network_compass_shape_ne,
	_network_compass_shape_sw,
	_network_compass_shape_se,

	_skull,
	
	// LP additions:
	_smg,
	_smg_bullet,
	_smg_casing,

	
	/* These are NOT done. */
	_mike_button_unpressed,
	_mike_button_pressed
};

enum {
	_unused_interface_data,
	_uses_energy,
	_uses_bullets
};

enum { /* Passed to update_ammo_display_in_panel */
	_primary_interface_ammo,
	_secondary_interface_ammo,
	NUMBER_OF_WEAPON_INTERFACE_ITEMS
};

/* ------------ structures */
struct weapon_interface_ammo_data 
{
	short type;
	short screen_left;
	short screen_top;
	short ammo_across; /* max energy for beam weapons */
	short ammo_down; /* Unused for energy weapons */
	short delta_x; /* Or width, if uses energy */
	short delta_y; /* Or height if uses energy */
	shape_descriptor bullet;	 /* or fill color index */
	shape_descriptor empty_bullet; /* or empty color index */
	bool right_to_left; /* Which way do the bullets go as they are used? */
};

struct weapon_interface_data 
{
	short item_id;
	shape_descriptor weapon_panel_shape;
	short weapon_name_start_y;
	short weapon_name_end_y;
	short weapon_name_start_x;	/* NONE means center in the weapon rectangle */
	short weapon_name_end_x;	/* NONE means center in the weapon rectangle */
	short standard_weapon_panel_top;
	short standard_weapon_panel_left;
	bool multi_weapon;
	struct weapon_interface_ammo_data ammo_data[NUMBER_OF_WEAPON_INTERFACE_ITEMS];
	shape_descriptor multiple_shape;
	shape_descriptor multiple_unusable_shape;
	short multiple_delta_x;
	short multiple_delta_y;
};

struct interface_state_data
{
	bool ammo_is_dirty;
	bool weapon_is_dirty;
	bool shield_is_dirty;
	bool oxygen_is_dirty;
};

#define MAXIMUM_WEAPON_INTERFACE_DEFINITIONS (sizeof(weapon_interface_definitions)/sizeof(struct weapon_interface_data))

extern interface_state_data interface_state;
extern weapon_interface_data weapon_interface_definitions[10];

struct point2d;

// Base class for HUD renderer
class HUD_Class
{
public:
	HUD_Class() : ForceUpdate(false) {}
	virtual ~HUD_Class() {}

	bool update_everything(short time_elapsed);

protected:
	void update_suit_energy(short time_elapsed);
	void update_suit_oxygen(short time_elapsed);
	void update_weapon_panel(bool force_redraw);
	void update_ammo_display(bool force_redraw);
	void update_inventory_panel(bool force_redraw);
	void draw_inventory_header(char *text, short offset);
        void draw_inventory_time(char *text, short offset);
	short max_displayable_inventory_lines(void);
	void calculate_inventory_rectangle_from_offset(screen_rectangle *r, short offset);
	void draw_bar(screen_rectangle *rectangle, short actual_height,
		shape_descriptor top_piece, shape_descriptor full_bar,
		shape_descriptor background_piece);
	void draw_ammo_display_in_panel(short trigger_id);
	void draw_inventory_item(char *text, short count, short offset, 
		bool erase_first, bool valid_in_this_environment);
	void draw_player_name(void);
	virtual void draw_message_area(short time_elapsed);

	void draw_network_compass(void);
	virtual void draw_all_entity_blips(void);

	virtual void update_motion_sensor(short time_elapsed) = 0;
	virtual void render_motion_sensor(short time_elapsed) = 0;
	virtual void draw_or_erase_unclipped_shape(short x, short y, shape_descriptor shape, bool draw) = 0;
	virtual void draw_entity_blip(point2d *location, shape_descriptor shape) = 0;

	virtual void DrawShape(shape_descriptor shape, screen_rectangle *dest, screen_rectangle *src) = 0;
	virtual void DrawShapeAtXY(shape_descriptor shape, short x, short y, bool transparency = false) = 0;
	virtual void DrawText(const char *text, screen_rectangle *dest, short flags, short font_id, short text_color) = 0;
	virtual void FillRect(screen_rectangle *r, short color_index) = 0;
	virtual void FrameRect(screen_rectangle *r, short color_index) = 0;

	virtual void DrawTexture(shape_descriptor shape, short texture_type, short x, short y, int size) = 0;

	virtual void SetClipPlane(int x, int y, int c_x, int c_y, int radius) = 0;
	virtual void DisableClipPlane(void) = 0;

protected:
	bool ForceUpdate;
};

#endif
