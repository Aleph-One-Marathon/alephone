/*
GAME_WINDOW.C
Thursday, December 30, 1993 9:51:24 PM

Tuesday, May 24, 1994 6:42:32 PM
	functions beginning with underscores are mac-specific and always have a corresponding portable
	function which sets up game-specific information (see update_compass and _update_compass for
	an example).
Friday, June 10, 1994 3:56:57 AM
	gutted, rewritten.  Much cleaner now.
Sunday, September 4, 1994 6:32:05 PM
	made scroll_inventory() non-static so that shell.c can call it.
Saturday, September 24, 1994 10:33:19 AM
	fixed some things, twice...
Friday, October 7, 1994 3:13:22 PM
	New interface.  Draws panels from PICT resources for memory consumption.  Inventory is 
	different panels, which are switched to whenever you grab an item.  There is no scrolling.
Tuesday, June 6, 1995 3:37:50 PM
	Marathon II modifications.
Tuesday, August 29, 1995 4:02:15 PM
	Reestablishing a level of portablility...

Feb 4, 2000 (Loren Petrich):
	Added SMG display stuff
	
Apr 30, 2000 (Loren Petrich): Added XML parser object for all the screen_drawing data,
	and also essentiall all the data here.

May 28, 2000 (Loren Petrich): Added support for buffering the Heads-Up Display

Jul 2, 2000 (Loren Petrich):
	The HUD is now always buffered
*/

#include "cseries.h"
#include <string.h>
#include <stdio.h>

#include "map.h"
#include "interface.h"
#include "player.h"
#include "screen_drawing.h"
#include "motion_sensor.h"
#include "mysound.h"
#include "items.h"
#include "weapons.h"
#include "game_window.h"
#include "network_games.h"

// LP addition: color parser
#include "ColorParser.h"
#include "screen.h"

#ifdef env68k
	#pragma segment screen
#endif

extern void draw_panels(void);

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
static void set_current_inventory_screen(short player_index, short screen);

#define INVENTORY_IS_DIRTY(p) ((p)->interface_flags & INVENTORY_DIRTY_BIT)
#define SET_INVENTORY_DIRTY_STATE(p, v) ((void)((v)?((p)->interface_flags|=(word)INVENTORY_DIRTY_BIT):((p)->interface_flags&=(word)~INVENTORY_DIRTY_BIT)))

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
	boolean right_to_left; /* Which way do the bullets go as they are used? */
};

struct weapon_interface_data 
{
	short item_id;
	short weapon_panel_shape;
	short weapon_name_start_y;
	short weapon_name_end_y;
	short weapon_name_start_x;	/* NONE means center in the weapon rectangle */
	short weapon_name_end_x;	/* NONE means center in the weapon rectangle */
	short standard_weapon_panel_top;
	short standard_weapon_panel_left;
	boolean multi_weapon;
	struct weapon_interface_ammo_data ammo_data[NUMBER_OF_WEAPON_INTERFACE_ITEMS];
};

struct interface_state_data
{
	boolean ammo_is_dirty;
	boolean weapon_is_dirty;
	boolean shield_is_dirty;
	boolean oxygen_is_dirty;
};

extern void validate_world_window(void);

/* --------- globals */

// LP addition: whether or not the motion sensor is active
static bool MotionSensorActive = true;

// LP addition: whether= to force an update of the HUD display;
// the purpose is to avoid redrawing the HUD more than is necessary.
static bool ForceUpdate = false;

static struct interface_state_data interface_state;

#define MAXIMUM_WEAPON_INTERFACE_DEFINITIONS (sizeof(weapon_interface_definitions)/sizeof(struct weapon_interface_data))

struct weapon_interface_data weapon_interface_definitions[]=
{
	/* Mac, the knife.. */
	{
		_i_knife,
		NONE,
		433, 432,
		NONE, NONE,
		0, 0,
		FALSE,
		{
			{ _unused_interface_data, 0, 0, 0, 0, 0, 0, NONE, NONE, TRUE},
			{ _unused_interface_data, 0, 0, 0, 0, 0, 0, NONE, NONE, TRUE}
		}
	},
	
	/* Harry, the .44 */
	{
		_i_magnum,
		BUILD_DESCRIPTOR(_collection_interface, _magnum_panel),
		432, 444,
		420, NONE,
		366, 517, 
		TRUE,
		{
			{ _uses_bullets, 517, 412, 8, 1, 5, 14, BUILD_DESCRIPTOR(_collection_interface, _magnum_bullet), BUILD_DESCRIPTOR(_collection_interface, _magnum_casing), FALSE},
			{ _uses_bullets, 452, 412, 8, 1, 5, 14, BUILD_DESCRIPTOR(_collection_interface, _magnum_bullet), BUILD_DESCRIPTOR(_collection_interface, _magnum_casing), TRUE}
		}
	},

	/* Ripley, the plasma pistol. */
	{
		_i_plasma_pistol,
		BUILD_DESCRIPTOR(_collection_interface, _zeus_panel),
		431, 443,
		401, NONE,
		366, 475, 
		FALSE,
		{
			{ _uses_energy, 414, 366, 20, 0, 38, 57, _energy_weapon_full_color, _energy_weapon_empty_color, TRUE},
			{ _unused_interface_data, 450, 410, 50, 0, 62, 7, _energy_weapon_full_color, _energy_weapon_empty_color, TRUE}
		}
	},
	
	/* Arnold, the assault rifle */	
	{
		_i_assault_rifle,
		BUILD_DESCRIPTOR(_collection_interface, _assault_panel),
		430, 452,
		439, NONE, //ее
		366, 460, 
		FALSE,
		{
			{ _uses_bullets, 391, 368, 13, 4, 4, 10, BUILD_DESCRIPTOR(_collection_interface, _assault_rifle_bullet), BUILD_DESCRIPTOR(_collection_interface, _assault_rifle_casing), TRUE},
			{ _uses_bullets, 390, 413, 7, 1, 8, 12, BUILD_DESCRIPTOR(_collection_interface, _assault_rifle_grenade), BUILD_DESCRIPTOR(_collection_interface, _assault_rifle_grenade_casing), TRUE},
		}
	},
		
	/* John R., the missile launcher */	
	{
		_i_missile_launcher,
		BUILD_DESCRIPTOR(_collection_interface, _missile_panel),
		433, 445,
		426, NONE,
		365, 419, 
		FALSE,
		{
			{ _uses_bullets, 385, 376, 2, 1, 16, 49, BUILD_DESCRIPTOR(_collection_interface, _missile), BUILD_DESCRIPTOR(_collection_interface, _missile_casing), TRUE},
			{ _unused_interface_data, 0, 0, 0, 0, 0, 0, NONE, NONE, TRUE }
		}
	},

	/* ???, the flame thrower */	
	{
		_i_flamethrower,
		BUILD_DESCRIPTOR(_collection_interface, _flamethrower_panel),
		433, 445,
		398, NONE,
		363, 475, 
		FALSE,
		{
			/* This weapon has 7 seconds of flamethrower carnage.. */
			{ _uses_energy, 427, 369, 7*TICKS_PER_SECOND, 0, 38, 57, _energy_weapon_full_color, _energy_weapon_empty_color, TRUE},
			{ _unused_interface_data, 450, 410, 50, 0, 62, 7, _energy_weapon_full_color, _energy_weapon_empty_color, TRUE}
		}
	},

	/* Predator, the alien shotgun */	
	{
		_i_alien_shotgun,
		BUILD_DESCRIPTOR(_collection_interface, _alien_weapon_panel),
		418, 445,
		395, 575,
		359, 400, 
		FALSE,
		{
			{ _unused_interface_data, 425, 411, 50, 0, 96, 7, _energy_weapon_full_color, _energy_weapon_empty_color, TRUE},
			{ _unused_interface_data, 450, 410, 50, 0, 62, 7, _energy_weapon_full_color, _energy_weapon_empty_color, TRUE}
		}
	},

	/* Shotgun */
	{
		_i_shotgun,
		BUILD_DESCRIPTOR(_collection_interface, _single_shotgun),
		432, 444,
		420, NONE,
		373, 451, 
		TRUE,
		{
			{ _uses_bullets, 483, 411, 2, 1, 12, 16, BUILD_DESCRIPTOR(_collection_interface, _shotgun_bullet), BUILD_DESCRIPTOR(_collection_interface, _shotgun_casing), TRUE},
			{ _uses_bullets, 451, 411, 2, 1, 12, 16, BUILD_DESCRIPTOR(_collection_interface, _shotgun_bullet), BUILD_DESCRIPTOR(_collection_interface, _shotgun_casing), TRUE}
		}
	},

	/* Ball */
	{
		_i_red_ball, // statistically unlikely to be valid (really should be SKULL)
		BUILD_DESCRIPTOR(_collection_interface, _skull),
		432, 444,
		402, NONE,
		366, 465, 
		FALSE,
		{
			{ _unused_interface_data, 451, 411, 2, 1, 12, 16, BUILD_DESCRIPTOR(_collection_interface, _shotgun_bullet), BUILD_DESCRIPTOR(_collection_interface, _shotgun_casing), TRUE},
			{ _unused_interface_data, 483, 411, 2, 1, 12, 16, BUILD_DESCRIPTOR(_collection_interface, _shotgun_bullet), BUILD_DESCRIPTOR(_collection_interface, _shotgun_casing), TRUE}
		}
	},
	
	/* LP addition: SMG (clone of assault rifle) */	
	{
		_i_smg,
		BUILD_DESCRIPTOR(_collection_interface, _smg),
		430, 452,
		439, NONE, //ее
		366, 460, 
		FALSE,
		{
			{ _uses_bullets, 405, 382, 8, 4, 5, 10, BUILD_DESCRIPTOR(_collection_interface, _smg_bullet), BUILD_DESCRIPTOR(_collection_interface, _smg_casing), TRUE},
			{ _unused_interface_data, 390, 413, 7, 1, 8, 12, BUILD_DESCRIPTOR(_collection_interface, _assault_rifle_grenade), BUILD_DESCRIPTOR(_collection_interface, _assault_rifle_grenade_casing), TRUE},
		}
	},
};


/* --------- private prototypes */
static void update_inventory_panel(boolean force_redraw);
static void update_motion_sensor(short time_elapsed);
static void update_weapon_panel(boolean force_redraw);
static void update_ammo_display(boolean force_redraw);
static void	draw_inventory_header(char *text, short offset);
static void	draw_bar(screen_rectangle *rectangle, short actual_height,
	shape_descriptor top_piece, shape_descriptor full_bar,
	shape_descriptor background_piece);

static void calculate_inventory_rectangle_from_offset(screen_rectangle *r, short offset);
static short max_displayable_inventory_lines(void);
static void draw_ammo_display_in_panel(short trigger_id);
static void update_suit_energy(short time_elapsed);
static void update_suit_oxygen(short time_elapsed);
static void draw_inventory_item(char *text, short count, short offset, 
	boolean erase_first, boolean valid_in_this_environment);
static void draw_player_name(void);
static void draw_message_area(short time_elapsed);

void update_everything(short time_elapsed);

/* --------- code */

void initialize_game_window(
	void)
{
	initialize_motion_sensor(
		BUILD_DESCRIPTOR(_collection_interface, _motion_sensor_mount),
		BUILD_DESCRIPTOR(_collection_interface, _motion_sensor_virgin_mount),
		BUILD_DESCRIPTOR(_collection_interface, _motion_sensor_alien),
		BUILD_DESCRIPTOR(_collection_interface, _motion_sensor_friend),
		BUILD_DESCRIPTOR(_collection_interface, _motion_sensor_enemy),
		BUILD_DESCRIPTOR(_collection_interface, _network_compass_shape_nw),
		MOTION_SENSOR_SIDE_LENGTH);

	return;
}

/* draws the entire interface */
void draw_interface(
	void)
{
	if (!game_window_is_full_screen())
	{
		/* draw the frame */
		draw_panels();
	}
		
	validate_world_window();
	
	return;
}

/* updates only what needs changing (time_elapsed==NONE means redraw everything no matter what,
	but skip the interface frame) */
void update_interface(
	short time_elapsed)
{
	if (!game_window_is_full_screen())
	{
		// LP addition: don't force an update unless explicitly requested
		ForceUpdate = (time_elapsed == NONE);
	
		// LP addition: added support for HUD buffer;
		// added origin relocation to make the graphics paint into place
		_set_port_to_HUD();
#ifdef mac
		SetOrigin(0,320);
#endif
		// _set_port_to_screen_window();
		update_everything(time_elapsed);
#ifdef mac
		SetOrigin(0,0);
#endif
		_restore_port();
		
		// Draw the whole thing if doing so is requested
		// (may need some smart way of drawing only what has to be drawn)
		if (ForceUpdate)
		{
			RequestDrawingHUD();
		}
	}

	return;
}

void mark_interface_collections(
	boolean loading)
{
	loading ? mark_collection_for_loading(_collection_interface) : 
		mark_collection_for_unloading(_collection_interface);
	
	return;
}

void mark_weapon_display_as_dirty(
	void)
{
	interface_state.weapon_is_dirty= TRUE;
}

void mark_ammo_display_as_dirty(
	void)
{
	interface_state.ammo_is_dirty= TRUE;
}

void mark_shield_display_as_dirty(
	void)
{
	interface_state.shield_is_dirty= TRUE;
}

void mark_oxygen_display_as_dirty(
	void)
{
	interface_state.oxygen_is_dirty= TRUE;
}

void mark_player_inventory_screen_as_dirty(
	short player_index,
	short screen)
{
	struct player_data *player= get_player_data(player_index);

	set_current_inventory_screen(player_index, screen);
	SET_INVENTORY_DIRTY_STATE(player, TRUE);
	
	return;
}

void mark_player_inventory_as_dirty(
	short player_index, 
	short dirty_item)
{
	struct player_data *player= get_player_data(player_index);

	/* If the dirty item is not NONE, then goto that item kind display.. */
	if(dirty_item != NONE)
	{
		short item_kind= get_item_kind(dirty_item);
		short current_screen= GET_CURRENT_INVENTORY_SCREEN(player);

		/* Don't change if it is a powerup, or you are in the network statistics screen */		
		if(item_kind != _powerup && item_kind != current_screen) // && current_screen!=_network_statistics)
		{
			/* Goto that type of item.. */
			set_current_inventory_screen(player_index, item_kind);
		}
	}
	SET_INVENTORY_DIRTY_STATE(player, TRUE);
}

void mark_player_network_stats_as_dirty(
	short player_index)
{
	if (GET_GAME_OPTIONS()&_live_network_stats)
	{
		struct player_data *player= get_player_data(player_index);
	
		set_current_inventory_screen(player_index, _network_statistics);
		SET_INVENTORY_DIRTY_STATE(player, TRUE);
	}
	
	return;
}

void set_interface_microphone_recording_state(
	boolean state)
{
	(void) (state);
#ifdef OBSOLETE
	const short sounds[]={MICROPHONE_STOP_CLICK_SOUND, MICROPHONE_START_CLICK_SOUND};
	const short shapes[]={_mike_button_unpressed, _mike_button_pressed};
	screen_rectangle *rectangle= get_interface_rectangle(_microphone_rect);

	play_local_sound(sounds[state]);
	if(!game_window_is_full_screen())
	{
		_draw_screen_shape(BUILD_DESCRIPTOR(_collection_interface, shapes[state]), 
			rectangle, NULL);
	}
#endif
}

void scroll_inventory(
	short dy)
{
	short mod_value, index, current_inventory_screen, section_count, test_inventory_screen;
	short section_items[NUMBER_OF_ITEMS];
	short section_counts[NUMBER_OF_ITEMS];
	
	current_inventory_screen= GET_CURRENT_INVENTORY_SCREEN(current_player);

	if(dynamic_world->player_count>1)
	{
		mod_value= NUMBER_OF_ITEM_TYPES+1;
	} else {
		mod_value= NUMBER_OF_ITEM_TYPES;
	}

	if(dy>0)
	{
		for(index= 1; index<mod_value; ++index)
		{
			test_inventory_screen= (current_inventory_screen+index)%mod_value;
			
			assert(test_inventory_screen>=0 && test_inventory_screen<NUMBER_OF_ITEM_TYPES+1);			
			if(test_inventory_screen != NUMBER_OF_ITEM_TYPES)
			{
				calculate_player_item_array(current_player_index, test_inventory_screen,
					section_items, section_counts, &section_count);
				if(section_count) break; /* Go tho this one! */
			} else {
				/* Network statistics! */
				break; 
			}
		}
		
		current_inventory_screen= test_inventory_screen;
	} else {
		/* Going down.. */
		for(index= mod_value-1; index>0; --index)
		{
			test_inventory_screen= (current_inventory_screen+index)%mod_value;

			assert(test_inventory_screen>=0 && test_inventory_screen<NUMBER_OF_ITEM_TYPES+1);			
			if(test_inventory_screen != NUMBER_OF_ITEM_TYPES)
			{
				calculate_player_item_array(current_player_index, test_inventory_screen,
					section_items, section_counts, &section_count);
				if(section_count) break; /* Go tho this one! */
			} else {
				/* Network statistics! */
				break; 
			}
		}		

		current_inventory_screen= test_inventory_screen;
	}
	set_current_inventory_screen(current_player_index, current_inventory_screen);

	SET_INVENTORY_DIRTY_STATE(current_player, TRUE);	
}

/* This function is only called from macintosh_game_window.c */
void update_everything(
	short time_elapsed)
{
	update_motion_sensor(time_elapsed);
	update_inventory_panel((time_elapsed==NONE) ? TRUE : FALSE);
	update_weapon_panel((time_elapsed==NONE) ? TRUE : FALSE);
	update_ammo_display((time_elapsed==NONE) ? TRUE : FALSE);
	update_suit_energy(time_elapsed);
	update_suit_oxygen(time_elapsed);

	/* Handle the network microphone.. */
//		handle_microphone(local_player_index==dynamic_world->speaking_player_index);

	/* Draw the message area if the player count is greater than one. */
	if(dynamic_world->player_count>1)
	{
		draw_message_area(time_elapsed);
	}
		
	return;
}

/* ---------- private code */
static void update_suit_energy(
	short time_elapsed)
{
	/* time_elapsed==NONE means force redraw */
	if (time_elapsed==NONE || (interface_state.shield_is_dirty))
	{
		// LP addition: display needs to be updated
		ForceUpdate = true;
		
		screen_rectangle *shield_rect= get_interface_rectangle(_shield_rect);
		short width= shield_rect->right-shield_rect->left;
		short actual_width, suit_energy;
		short background_shape_id, bar_shape_id, bar_top_shape_id;

		suit_energy = current_player->suit_energy%PLAYER_MAXIMUM_SUIT_ENERGY;

		if(	!suit_energy && 
			current_player->suit_energy==PLAYER_MAXIMUM_SUIT_ENERGY ||
			current_player->suit_energy==2*PLAYER_MAXIMUM_SUIT_ENERGY || 
			current_player->suit_energy==3*PLAYER_MAXIMUM_SUIT_ENERGY) 
		{
			suit_energy= PLAYER_MAXIMUM_SUIT_ENERGY;
		}

		actual_width= (suit_energy*width)/PLAYER_MAXIMUM_SUIT_ENERGY;

		/* Setup the bars.. */
		if(current_player->suit_energy>2*PLAYER_MAXIMUM_SUIT_ENERGY)
		{
			background_shape_id= _double_energy_bar;
			bar_shape_id= _triple_energy_bar;
			bar_top_shape_id= _triple_energy_bar_right;
		} 
		else if(current_player->suit_energy>PLAYER_MAXIMUM_SUIT_ENERGY)
		{
			background_shape_id= _energy_bar;
			bar_shape_id= _double_energy_bar;
			bar_top_shape_id= _double_energy_bar_right;
		} 
		else 
		{
			background_shape_id= _empty_energy_bar;
			bar_shape_id= _energy_bar;
			bar_top_shape_id= _energy_bar_right;
			if(current_player->suit_energy<0) actual_width= 0;
		} 

		draw_bar(shield_rect, actual_width, 
			BUILD_DESCRIPTOR(_collection_interface, bar_top_shape_id), 
			BUILD_DESCRIPTOR(_collection_interface, bar_shape_id),
			BUILD_DESCRIPTOR(_collection_interface, background_shape_id));

		interface_state.shield_is_dirty= FALSE;
	}
	
	return;
}

static void update_suit_oxygen(
	short time_elapsed)
{
	static short delay_time= 0;

	/* Redraw the oxygen only if the interface is visible and only if enough delay has passed.. */
	if(((delay_time-= time_elapsed)<0) || time_elapsed==NONE || interface_state.oxygen_is_dirty)
	{
		// LP addition: display needs to be updated
		ForceUpdate = true;
		
		screen_rectangle *oxygen_rect= get_interface_rectangle(_oxygen_rect);
		short width, actual_width;
		short suit_oxygen;
		
		suit_oxygen= MIN(current_player->suit_oxygen, PLAYER_MAXIMUM_SUIT_OXYGEN);
		width= oxygen_rect->right-oxygen_rect->left;
		actual_width= (suit_oxygen*width)/PLAYER_MAXIMUM_SUIT_OXYGEN;

		draw_bar(oxygen_rect, actual_width, 
			BUILD_DESCRIPTOR(_collection_interface, _oxygen_bar_right), 
			BUILD_DESCRIPTOR(_collection_interface, _oxygen_bar),
			BUILD_DESCRIPTOR(_collection_interface, _empty_oxygen_bar));
	
		delay_time= DELAY_TICKS_BETWEEN_OXYGEN_REDRAW;
		interface_state.oxygen_is_dirty= FALSE;
	}

	return;
}

static void draw_player_name(
	void)
{
	struct player_data *player= get_player_data(current_player_index);
	screen_rectangle *player_name_rect= get_interface_rectangle(_player_name_rect);

	_draw_screen_text(player->name, player_name_rect, 
		_center_vertical | _center_horizontal, _player_name_font, 
		player->color+PLAYER_COLOR_BASE_INDEX);
}

static void update_motion_sensor(
	short time_elapsed)
{
	if (!MotionSensorActive) GET_GAME_OPTIONS() |= _motion_sensor_does_not_work;
	
	if(!(GET_GAME_OPTIONS() & _motion_sensor_does_not_work))
	{
		if (time_elapsed==NONE)
		{
			// LP addition: display needs to be updated
			ForceUpdate = true;
		
			reset_motion_sensor(current_player_index);
		}

		motion_sensor_scan(time_elapsed);
		
		if (motion_sensor_has_changed())
		{
			// LP addition: display needs to be updated
			ForceUpdate = true;
				
			screen_rectangle *destination= get_interface_rectangle(_motion_sensor_rect);

			_draw_screen_shape_at_x_y(BUILD_DESCRIPTOR(_collection_interface, 
				_motion_sensor_mount), destination->left, destination->top);
		}
	}
		
	return;
}

/* A change of weapon has occurred, change the weapon display panel */
static void update_weapon_panel(
	boolean force_redraw)
{
	if(force_redraw || interface_state.weapon_is_dirty)
	{
		// LP addition: display needs to be updated
		ForceUpdate = true;
		
		char weapon_name[90];
		struct weapon_interface_data *definition;
		screen_rectangle *destination= get_interface_rectangle(_weapon_display_rect);
		screen_rectangle source;
		short desired_weapon= get_player_desired_weapon(current_player_index);

		/* Now we have to erase, because the panel won't do it for us.. */
		_fill_rect(destination, _inventory_background_color);
	
		if(desired_weapon != NONE)
		{
			assert(desired_weapon>=0 && desired_weapon<MAXIMUM_WEAPON_INTERFACE_DEFINITIONS);

			definition= weapon_interface_definitions+desired_weapon;
	
			/* Check if it is a multi weapon - actually special cased for the magnum... */
			if(definition->multi_weapon)
			{
#define MAGNUM_DELTA_X 97
				if(definition->item_id==_i_magnum)
				{
					/* Either way, draw the single */
					_draw_screen_shape_at_x_y(definition->weapon_panel_shape, 
						definition->standard_weapon_panel_left, 
						definition->standard_weapon_panel_top);

					if(current_player->items[definition->item_id]>1)
					{
						_draw_screen_shape_at_x_y(
							BUILD_DESCRIPTOR(_collection_interface, _left_magnum), 
							definition->standard_weapon_panel_left-MAGNUM_DELTA_X, 
							definition->standard_weapon_panel_top);
					} 
					else 
					{
						/* Draw the empty one.. */
						_draw_screen_shape_at_x_y(
							BUILD_DESCRIPTOR(_collection_interface, _left_magnum_unusable), 
							definition->standard_weapon_panel_left-MAGNUM_DELTA_X, 
							definition->standard_weapon_panel_top);
					}
				} 
				else if(definition->item_id==_i_shotgun)
				{
					if(current_player->items[definition->item_id]>1)
					{
						_draw_screen_shape_at_x_y(
							BUILD_DESCRIPTOR(_collection_interface, _double_shotgun), 
							definition->standard_weapon_panel_left, 
							definition->standard_weapon_panel_top-12);
					} else {
						_draw_screen_shape_at_x_y(definition->weapon_panel_shape, 
							definition->standard_weapon_panel_left, 
							definition->standard_weapon_panel_top);
					}
				}
			} else {
				/* Slam it to the screen! */
				if(definition->weapon_panel_shape != NONE)
				{
					_draw_screen_shape_at_x_y(definition->weapon_panel_shape, 
						definition->standard_weapon_panel_left, 
						definition->standard_weapon_panel_top);
				}
			}
		
			/* Get the weapon name.. */
			if(desired_weapon != _weapon_ball)
			{
#define strWEAPON_NAME_LIST 137
				getcstr(weapon_name, strWEAPON_NAME_LIST, desired_weapon);
			} else {
				short item_index;
				
				/* Which ball do they actually have? */
				for(item_index= BALL_ITEM_BASE; item_index<BALL_ITEM_BASE+MAXIMUM_NUMBER_OF_PLAYERS; ++item_index)
				{
					if(current_player->items[item_index]>0) break;
				}
				assert(item_index != BALL_ITEM_BASE+MAXIMUM_NUMBER_OF_PLAYERS);
				get_item_name(weapon_name, item_index, FALSE);
			}

			/* Draw the weapon name.. */
			source= *destination;
			source.top= definition->weapon_name_start_y;
			source.bottom= definition->weapon_name_end_y;
			if(definition->weapon_name_start_x != NONE)
			{
				source.left= definition->weapon_name_start_x;
			}
			
			if(definition->weapon_name_end_x != NONE)
			{
				source.right= definition->weapon_name_end_x;
			}
			
			_draw_screen_text(weapon_name, &source, _center_horizontal|_center_vertical|_wrap_text,
				_weapon_name_font, _inventory_text_color);
				
			/* And make sure that the ammo knows it needs to update */
			interface_state.ammo_is_dirty= TRUE;
		} 
		interface_state.weapon_is_dirty= FALSE;
	}
}

static void update_ammo_display(
	boolean force_redraw)
{
	if(force_redraw || interface_state.ammo_is_dirty)
	{
		// LP addition: display needs to be updated
		ForceUpdate = true;
		
		draw_ammo_display_in_panel(_primary_interface_ammo);
		draw_ammo_display_in_panel(_secondary_interface_ammo);
		interface_state.ammo_is_dirty= FALSE;
	}
}

/* changed_item gets erased first.. */
/* This should probably go to a gworld first, or something */
static void update_inventory_panel(
	boolean force_redraw)
{
	short section_items[NUMBER_OF_ITEMS];
	short section_counts[NUMBER_OF_ITEMS];
	short section_count, loop;
	short item_type, current_row;

	if(INVENTORY_IS_DIRTY(current_player) || force_redraw)
	{
		// LP addition: display needs to be updated
		ForceUpdate = true;
		
		screen_rectangle *destination= get_interface_rectangle(_inventory_rect);
		screen_rectangle text_rectangle;
		short total_inventory_line_count= count_inventory_lines(current_player_index);
		short max_lines= max_displayable_inventory_lines();
	
		/* Recalculate and redraw.. */
		item_type= GET_CURRENT_INVENTORY_SCREEN(current_player);
					
		/* Reset the row.. */
		current_row= 0;
		if(item_type!=_network_statistics)
		{
			/* Get the types and names */
			calculate_player_item_array(current_player_index, item_type,
				section_items, section_counts, &section_count);
		}
				
		/* Draw the header. */
		get_header_name(temporary, item_type);
		draw_inventory_header(temporary, current_row++);
	
		/* Erase the panel.. */
		text_rectangle= *destination;
		text_rectangle.top+= _get_font_line_height(_interface_font);
		_fill_rect(&text_rectangle, _inventory_background_color);
				
		if(item_type==_network_statistics)
		{
			struct player_ranking_data rankings[MAXIMUM_NUMBER_OF_PLAYERS];
	
			calculate_player_rankings(rankings);
		
			/* Calculate the network statistics. */
			for(loop= 0; loop<dynamic_world->player_count; ++loop)
			{
				screen_rectangle dest_rect;
				struct player_data *player= get_player_data(rankings[loop].player_index);
				short width;
				
				calculate_inventory_rectangle_from_offset(&dest_rect, current_row++);
				calculate_ranking_text(temporary, rankings[loop].ranking);

				/* Draw the player name.. */
				width= _text_width(temporary, _interface_font);
				dest_rect.right-= width;
				dest_rect.left+= TEXT_INSET;
				_draw_screen_text(player->name, &dest_rect, _center_vertical, 
					_interface_font, PLAYER_COLOR_BASE_INDEX+player->color);

				/* Now draw the ranking_text */
				dest_rect.right+= width;
				dest_rect.left= dest_rect.right-width;
				_draw_screen_text(temporary, &dest_rect, _center_vertical, 
					_interface_font, PLAYER_COLOR_BASE_INDEX+player->color);
			}
		} else {
			/* Draw the items. */
			for(loop= 0; loop<section_count && current_row<max_lines; ++loop)
			{
				boolean valid_in_this_environment;
			
				/* Draw the item */
				get_item_name(temporary, section_items[loop], (section_counts[loop]!=1));
				valid_in_this_environment= item_valid_in_current_environment(section_items[loop]);
				draw_inventory_item(temporary, section_counts[loop], current_row++, FALSE, valid_in_this_environment);
			}
		}
		
		SET_INVENTORY_DIRTY_STATE(current_player, FALSE);
	}
}

/* Draw the text in the rectangle, starting at the given offset, on the */
/* far left.  Headers also have their backgrounds erased first */
static void draw_inventory_header(
	char *text, 
	short offset)
{
	screen_rectangle destination;

	calculate_inventory_rectangle_from_offset(&destination, offset);

	/* Erase.. */
	_fill_rect(&destination, _inventory_header_background_color);

	/* Now draw the text. */	
	destination.left+= TEXT_INSET;
	_draw_screen_text(text, &destination, _center_vertical, _interface_font,
		_inventory_text_color);
}

static void calculate_inventory_rectangle_from_offset(
	screen_rectangle *r, 
	short offset)
{
	screen_rectangle *inventory_rect= get_interface_rectangle(_inventory_rect);
	short line_height= _get_font_line_height(_interface_font);
	
	*r= *inventory_rect;
	r->top += offset*line_height;
	r->bottom= r->top + line_height;
}

static short max_displayable_inventory_lines(
	void)
{
	screen_rectangle *destination= get_interface_rectangle(_inventory_rect);
	
	return (destination->bottom-destination->top)/_get_font_line_height(_interface_font);
}

static void	draw_bar(
	screen_rectangle *rectangle,
	short width,
	shape_descriptor top_piece,
	shape_descriptor full_bar,
	shape_descriptor background_texture)
{
	screen_rectangle destination= *rectangle;
	screen_rectangle source;
	screen_rectangle bar_section;

	/* Draw the background (right). */
	destination.left+= width;
	source= destination;

	_offset_screen_rect(&source, -rectangle->left, -rectangle->top);
	_draw_screen_shape(background_texture, &destination, &source);

	/* Draw the top bit.. */
	if(width>2*TOP_OF_BAR_WIDTH)
	{
		_draw_screen_shape_at_x_y(top_piece, rectangle->left+width-TOP_OF_BAR_WIDTH, 
			rectangle->top);
	} else {
		destination= *rectangle;

		/* Gotta take lines off the top, so that the bottom stuff is still visible.. */
		destination.left= rectangle->left+width/2+width%2;
		destination.right= destination.left+width/2;

		source= destination;			
		_offset_screen_rect(&source, -source.left+TOP_OF_BAR_WIDTH-width/2, -source.top);
		_draw_screen_shape(top_piece, &destination, &source);
	}

	/* Copy the bar.. */
	bar_section= *rectangle;
	bar_section.right= bar_section.left+width-TOP_OF_BAR_WIDTH;
	
	if(bar_section.left<bar_section.right)
	{
		screen_rectangle bar_section_source= bar_section;
		
		_offset_screen_rect(&bar_section_source, -rectangle->left, -rectangle->top);
		_draw_screen_shape(full_bar, &bar_section, &bar_section_source);
	}
}

static void draw_ammo_display_in_panel(
	short trigger_id)
{
	struct weapon_interface_data *current_weapon_data;
	struct weapon_interface_ammo_data *current_ammo_data;
	struct player_data *player= get_player_data(current_player_index);
	short ammunition_count;
	short desired_weapon= get_player_desired_weapon(current_player_index);

	/* Based on desired weapon, so we can get ammo updates as soon as we change */
	if(desired_weapon != NONE)
	{
		current_weapon_data= weapon_interface_definitions+desired_weapon;
		current_ammo_data= &current_weapon_data->ammo_data[trigger_id];

		if(trigger_id==_primary_interface_ammo)
		{
			ammunition_count= get_player_weapon_ammo_count(current_player_index, desired_weapon, _primary_weapon);
		} else {
			ammunition_count= get_player_weapon_ammo_count(current_player_index, desired_weapon, _secondary_weapon);
		}
		
		/* IF we have ammo for this trigger.. */
		if(current_ammo_data->type!=_unused_interface_data && ammunition_count!=NONE)
		{
			if(current_ammo_data->type==_uses_energy)
			{
				/* Energy beam weapon-> progress bar type.. */
				short  fill_height;
				screen_rectangle bounds;

				/* Pin it.. */
				ammunition_count= PIN(ammunition_count, 0, current_ammo_data->ammo_across);
				fill_height= (ammunition_count*(current_ammo_data->delta_y-2))/current_ammo_data->ammo_across;
				
				/* Setup the energy left bar... */				
				bounds.left= current_ammo_data->screen_left;
				bounds.right= current_ammo_data->screen_left+current_ammo_data->delta_x;
				bounds.bottom= current_ammo_data->screen_top+current_ammo_data->delta_y;
				bounds.top= current_ammo_data->screen_top;
				
				/* Frame the rectangle */
				_frame_rect(&bounds, current_ammo_data->bullet);
				
				/* Inset the rectangle.. */
				bounds.left+=1; bounds.right-=1; bounds.bottom-= 1; bounds.top+=1;
				
				/* Fill with the full stuff.. */
				bounds.top= bounds.bottom-fill_height;
				_fill_rect(&bounds, current_ammo_data->bullet);

				/* Now erase the rest of the rectangle */
				bounds.bottom= bounds.top;
				bounds.top= current_ammo_data->screen_top+1;
				
				/* Fill it. */
				_fill_rect(&bounds, current_ammo_data->empty_bullet);
				
				/* We be done.. */
			} else {
				/* Uses ammunition, a little trickier.. */
				short row, x, y;
				screen_rectangle destination, source;
				short max, partial_row_count;
				
				x= current_ammo_data->screen_left;
				y= current_ammo_data->screen_top;
				
				destination.left= x;
				destination.top= y;
				
				/* Pin it.. */
				max= current_ammo_data->ammo_down*current_ammo_data->ammo_across;
				ammunition_count= PIN(ammunition_count, 0, max);
									
				/* Draw all of the full rows.. */
				for(row=0; row<(ammunition_count/current_ammo_data->ammo_across); ++row)
				{
					_draw_screen_shape_at_x_y(current_ammo_data->bullet,
						x, y);
					y+= current_ammo_data->delta_y;
				}
				
				/* Draw the partially used row.. */
				partial_row_count= ammunition_count%current_ammo_data->ammo_across;
				if(partial_row_count)
				{
					/* If we use ammo from right to left.. */
					if(current_ammo_data->right_to_left)
					{
						/* Draw the unused part of the row.. */
						destination.left= x, destination.top= y;
						destination.right= x+(partial_row_count*current_ammo_data->delta_x);
						destination.bottom= y+current_ammo_data->delta_y;
						source= destination;
						_offset_screen_rect(&source, -source.left, -source.top);
						_draw_screen_shape(current_ammo_data->bullet, &destination, &source);
						
						/* Draw the used part of the row.. */
						destination.left= destination.right, 
						destination.right= destination.left+(current_ammo_data->ammo_across-partial_row_count)*current_ammo_data->delta_x;
						source= destination;
						_offset_screen_rect(&source, -source.left, -source.top);
						_draw_screen_shape(current_ammo_data->empty_bullet, &destination, &source);
					} else {
						/* Draw the used part of the row.. */
						destination.left= x, destination.top= y;
						destination.right= x+(current_ammo_data->ammo_across-partial_row_count)*current_ammo_data->delta_x;
						destination.bottom= y+current_ammo_data->delta_y;
						source= destination;
						_offset_screen_rect(&source, -source.left, -source.top);
						_draw_screen_shape(current_ammo_data->empty_bullet, &destination, &source);
						
						/* Draw the unused part of the row */
						destination.left= destination.right;
						destination.right= destination.left+(partial_row_count*current_ammo_data->delta_x);
						source= destination;
						_offset_screen_rect(&source, -source.left, -source.top);
						_draw_screen_shape(current_ammo_data->bullet, &destination, &source);
					}
					y+= current_ammo_data->delta_y;
				}
				
				/* Draw the remaining rows. */
				x= current_ammo_data->screen_left;
				for(row=0; row<(max-ammunition_count)/current_ammo_data->ammo_across; ++row)
				{
					_draw_screen_shape_at_x_y(current_ammo_data->empty_bullet,
						x, y);
					y+= current_ammo_data->delta_y;
				}
			}
		}
	}
}

static void draw_inventory_item(
	char *text, 
	short count, 
	short offset, 
	boolean erase_first,
	boolean valid_in_this_environment)
{
	screen_rectangle destination, text_destination;
	char count_text[10];
	short color;

	calculate_inventory_rectangle_from_offset(&destination, offset);

	/* Select the color for the text.. */
	color= (valid_in_this_environment) ? (_inventory_text_color) : (_invalid_weapon_color);

	/* Erase on items that changed only in count.. */
	if(erase_first)
	{
		_fill_rect(&destination, _inventory_background_color);
	} else {
		/* Unfortunately, we must always erase the numbers.. */
		text_destination= destination;
		text_destination.right= text_destination.left+NAME_OFFSET+TEXT_INSET;
		_fill_rect(&text_destination, _inventory_background_color);
	}

	/* Draw the text name.. */
	text_destination= destination;
	text_destination.left+= NAME_OFFSET+TEXT_INSET;
	_draw_screen_text(text, &text_destination, _center_vertical, _interface_font, color);

	/* Draw the text count-> Change the font!! */
	text_destination= destination;
	text_destination.left+= TEXT_INSET;
	sprintf(count_text, "%3d", count);
	_draw_screen_text(count_text, &text_destination, _center_vertical, _interface_item_count_font, color);
}

/* Draw the message area, and then put messages or player name in the buffer. */
#define MESSAGE_AREA_X_OFFSET 291
#define MESSAGE_AREA_Y_OFFSET 321

static void draw_message_area(
	short time_elapsed)
{
	if(time_elapsed==NONE)
	{
		_draw_screen_shape_at_x_y(
			BUILD_DESCRIPTOR(_collection_interface, _network_panel), 
			MESSAGE_AREA_X_OFFSET, MESSAGE_AREA_Y_OFFSET);
		draw_player_name();
	}
}

static void set_current_inventory_screen(
	short player_index,
	short screen)
{
	struct player_data *player= get_player_data(player_index);
	
	assert(screen>=0 && screen<7);
	
	player->interface_flags&= ~INVENTORY_MASK_BITS;
	player->interface_flags|= screen;
	player->interface_decay= 5*TICKS_PER_SECOND;
	
	return;
}



class XML_AmmoDisplayParser: public XML_ElementParser
{
	// Intended one to replace
	int Index;
	// Ammo data
	weapon_interface_ammo_data Data;
	
	// What is present?
	bool IndexPresent;
	enum {NumberOfValues = 10};
	bool IsPresent[NumberOfValues];
	
public:
	bool Start();
	bool HandleAttribute(const char *Tag, const char *Value);
	bool AttributesDone();

	// Pointer to list of ammo data types, from the weapon parser
	weapon_interface_ammo_data *OrigAmmo;
	
	XML_AmmoDisplayParser(): XML_ElementParser("ammo") {OrigAmmo=0;}
};

bool XML_AmmoDisplayParser::Start()
{
	IndexPresent = false;
	for (int k=0; k<NumberOfValues; k++)
		IsPresent[k] = false;
	return true;
}

bool XML_AmmoDisplayParser::HandleAttribute(const char *Tag, const char *Value)
{
	// Won't be doing the item ID or the shape ID;
	// just the stuff necessary for good placement of weapon and name graphics
	if (strcmp(Tag,"index") == 0)
	{
		if (ReadBoundedNumericalValue(Value,"%d",Index,0,int(NUMBER_OF_WEAPON_INTERFACE_ITEMS)-1))
		{
			IndexPresent = true;
			return true;
		}
		else return false;
	}
	else if (strcmp(Tag,"type") == 0)
	{
		if (ReadNumericalValue(Value,"%hd",Data.type))
		{
			IsPresent[0] = true;
			return true;
		}
		else return false;
	}
	else if (strcmp(Tag,"left") == 0)
	{
		if (ReadNumericalValue(Value,"%hd",Data.screen_left))
		{
			IsPresent[1] = true;
			return true;
		}
		else return false;
	}
	else if (strcmp(Tag,"top") == 0)
	{
		if (ReadNumericalValue(Value,"%hd",Data.screen_top))
		{
			IsPresent[2] = true;
			return true;
		}
		else return false;
	}
	else if (strcmp(Tag,"across") == 0)
	{
		if (ReadNumericalValue(Value,"%hd",Data.ammo_across))
		{
			IsPresent[3] = true;
			return true;
		}
		else return false;
	}
	else if (strcmp(Tag,"down") == 0)
	{
		if (ReadNumericalValue(Value,"%hd",Data.ammo_down))
		{
			IsPresent[4] = true;
			return true;
		}
		else return false;
	}
	else if (strcmp(Tag,"delta_x") == 0)
	{
		if (ReadNumericalValue(Value,"%hd",Data.delta_x))
		{
			IsPresent[5] = true;
			return true;
		}
		else return false;
	}
	else if (strcmp(Tag,"delta_y") == 0)
	{
		if (ReadNumericalValue(Value,"%hd",Data.delta_y))
		{
			IsPresent[6] = true;
			return true;
		}
		else return false;
	}
	else if (strcmp(Tag,"bullet_shape") == 0)
	{
		if (ReadNumericalValue(Value,"%hd",Data.bullet))
		{
			IsPresent[7] = true;
			return true;
		}
		else return false;
	}
	else if (strcmp(Tag,"empty_shape") == 0)
	{
		if (ReadNumericalValue(Value,"%hd",Data.empty_bullet))
		{
			IsPresent[8] = true;
			return true;
		}
		else return false;
	}
	else if (strcmp(Tag,"right_to_left") == 0)
	{
		short RightToLeft;
		if (ReadNumericalValue(Value,"%hd",RightToLeft))
		{
			Data.right_to_left = (RightToLeft != 0);
			IsPresent[9] = true;
			return true;
		}
		else return false;
	}
	
	UnrecognizedTag();
	return false;
}

bool XML_AmmoDisplayParser::AttributesDone()
{
	if (!IndexPresent)
	{
		AttribsMissing();
		return false;
	}
	assert(OrigAmmo);
	weapon_interface_ammo_data& OrigData = OrigAmmo[Index];
	if (IsPresent[0]) OrigData.type = Data.type;
	if (IsPresent[1]) OrigData.screen_left = Data.screen_left;
	if (IsPresent[2]) OrigData.screen_top = Data.screen_top;
	if (IsPresent[3]) OrigData.ammo_across = Data.ammo_across;
	if (IsPresent[4]) OrigData.ammo_down = Data.ammo_down;
	if (IsPresent[5]) OrigData.delta_x = Data.delta_x;
	if (IsPresent[6]) OrigData.delta_y = Data.delta_y;
	if (IsPresent[7]) OrigData.bullet = Data.bullet;
	if (IsPresent[8]) OrigData.empty_bullet = Data.empty_bullet;
	if (IsPresent[9]) OrigData.right_to_left = Data.right_to_left;
		
	return true;
}

static XML_AmmoDisplayParser AmmoDisplayParser;


class XML_WeaponDisplayParser: public XML_ElementParser
{
	// Intended one to replace
	int Index;
	// Weapon data
	weapon_interface_data Data;
	
	// What is present?
	bool IndexPresent;
	enum {NumberOfValues = 8};
	bool IsPresent[NumberOfValues];
	
public:
	bool Start();
	bool HandleAttribute(const char *Tag, const char *Value);
	bool AttributesDone();
	
	XML_WeaponDisplayParser(): XML_ElementParser("weapon") {}
};

bool XML_WeaponDisplayParser::Start()
{
	IndexPresent = false;
	for (int k=0; k<NumberOfValues; k++)
		IsPresent[k] = false;
	return true;
}

bool XML_WeaponDisplayParser::HandleAttribute(const char *Tag, const char *Value)
{
	// Won't be doing the item ID or the shape ID;
	// just the stuff necessary for good placement of weapon and name graphics
	if (strcmp(Tag,"index") == 0)
	{
		if (ReadBoundedNumericalValue(Value,"%d",Index,0,int(MAXIMUM_WEAPON_INTERFACE_DEFINITIONS)-1))
		{
			IndexPresent = true;
			return true;
		}
		else return false;
	}
	else if (strcmp(Tag,"shape") == 0)
	{
		if (ReadNumericalValue(Value,"%hd",Data.weapon_panel_shape))
		{
			IsPresent[0] = true;
			return true;
		}
		else return false;
	}
	else if (strcmp(Tag,"start_y") == 0)
	{
		if (ReadNumericalValue(Value,"%hd",Data.weapon_name_start_y))
		{
			IsPresent[1] = true;
			return true;
		}
		else return false;
	}
	else if (strcmp(Tag,"end_y") == 0)
	{
		if (ReadNumericalValue(Value,"%hd",Data.weapon_name_end_y))
		{
			IsPresent[2] = true;
			return true;
		}
		else return false;
	}
	else if (strcmp(Tag,"start_x") == 0)
	{
		if (ReadNumericalValue(Value,"%hd",Data.weapon_name_start_x))
		{
			IsPresent[3] = true;
			return true;
		}
		else return false;
	}
	else if (strcmp(Tag,"end_x") == 0)
	{
		if (ReadNumericalValue(Value,"%hd",Data.weapon_name_end_x))
		{
			IsPresent[4] = true;
			return true;
		}
		else return false;
	}
	else if (strcmp(Tag,"top") == 0)
	{
		if (ReadNumericalValue(Value,"%hd",Data.standard_weapon_panel_top))
		{
			IsPresent[5] = true;
			return true;
		}
		else return false;
	}
	else if (strcmp(Tag,"left") == 0)
	{
		if (ReadNumericalValue(Value,"%hd",Data.standard_weapon_panel_left))
		{
			IsPresent[6] = true;
			return true;
		}
		else return false;
	}
	else if (strcmp(Tag,"multiple") == 0)
	{
		short IsMultiple;
		if (ReadNumericalValue(Value,"%hd",IsMultiple))
		{
			Data.multi_weapon = (IsMultiple != 0);
			IsPresent[7] = true;
			return true;
		}
		else return false;
	}
	
	UnrecognizedTag();
	return false;
}

bool XML_WeaponDisplayParser::AttributesDone()
{
	if (!IndexPresent)
	{
		AttribsMissing();
		return false;
	}
	
	weapon_interface_data& OrigData = weapon_interface_definitions[Index];
	
	if (IsPresent[0]) OrigData.weapon_panel_shape = Data.weapon_panel_shape;
	if (IsPresent[1]) OrigData.weapon_name_start_y = Data.weapon_name_start_y;
	if (IsPresent[2]) OrigData.weapon_name_end_y = Data.weapon_name_end_y;
	if (IsPresent[3]) OrigData.weapon_name_start_x = Data.weapon_name_start_x;
	if (IsPresent[4]) OrigData.weapon_name_end_x = Data.weapon_name_end_x;
	if (IsPresent[5]) OrigData.standard_weapon_panel_top = Data.standard_weapon_panel_top;
	if (IsPresent[6]) OrigData.standard_weapon_panel_left = Data.standard_weapon_panel_left;
	if (IsPresent[7]) OrigData.multi_weapon = Data.multi_weapon;
	
	AmmoDisplayParser.OrigAmmo = OrigData.ammo_data;
	
	return true;
}

static XML_WeaponDisplayParser WeaponDisplayParser;


class XML_InterfaceParser: public XML_ElementParser
{
public:
	bool Start()
	{
		SetColorParserToScreenDrawing();
		return true;
	}
	bool HandleAttribute(const char *Tag, const char *Value)
	{
		if (strcmp(Tag,"motion_sensor") == 0)
		{
			return ReadBooleanValue(Value,MotionSensorActive);
		}
		UnrecognizedTag();
		return false;
	}
	
	XML_InterfaceParser(): XML_ElementParser("interface") {}
};


static XML_InterfaceParser InterfaceParser;

// Makes a pointer to a the interface-data parser
XML_ElementParser *Interface_GetParser()
{

	// Add all subobjects:
	// weapon display, rectangles, and colors
	WeaponDisplayParser.AddChild(&AmmoDisplayParser);
	InterfaceParser.AddChild(&WeaponDisplayParser);
	InterfaceParser.AddChild(InterfaceRectangles_GetParser());
	InterfaceParser.AddChild(Color_GetParser());
	
	return &InterfaceParser;
}
