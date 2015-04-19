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
 *  HUDRenderer.cpp - HUD rendering base class and data
 *
 *  Written in 2001 by Christian Bauer
 */

#include "HUDRenderer.h"
#include "lua_script.h" // texture palette

using namespace std;

/*
 *  Move rectangle
 */

static inline void offset_rect(screen_rectangle *rect, short dx, short dy)
{
	rect->top += dy;
	rect->left += dx;
	rect->bottom += dy;
	rect->right += dx;
}


/*
 *  Update all HUD elements
 */

extern bool shapes_file_is_m1();

bool HUD_Class::update_everything(short time_elapsed)
{
	ForceUpdate = false;

	if (!LuaTexturePaletteSize())
	{
		if (!shapes_file_is_m1())
		{
			update_motion_sensor(time_elapsed);
			update_inventory_panel((time_elapsed == NONE) ? true : false);
			update_weapon_panel((time_elapsed == NONE) ? true : false);
			update_ammo_display((time_elapsed == NONE) ? true : false);
			update_suit_energy(time_elapsed);
			update_suit_oxygen(time_elapsed);

			// Draw the message area if the player count is greater than one
			if (dynamic_world->player_count > 1)
				draw_message_area(time_elapsed);
		}
	}
	else
	{
		int size;
		// some good looking break points, based on 640x160
		if (LuaTexturePaletteSize() <= 5)
			size = 128;
		else if (LuaTexturePaletteSize() <= 16)
			size = 80;
		else if (LuaTexturePaletteSize() <= 36)
			size = 53;
		else if (LuaTexturePaletteSize() <= 64)
			size = 40;
		else if (LuaTexturePaletteSize() <= 100)
			size = 32;
		else if (LuaTexturePaletteSize() <= 144)
			size = 26;
		else
			size = 20;

		int rows = 160 / size;
		int cols = 640 / size;

		int x_offset = (640 - cols * size) / 2;
		int y_offset = (160 - rows * size) / 2;
		
		for (int i = 0; i < LuaTexturePaletteSize(); ++i)
		{
			if (LuaTexturePaletteTexture(i) != UNONE)
				DrawTexture(LuaTexturePaletteTexture(i), LuaTexturePaletteTextureType(i), (i % cols) * size + x_offset, 320 + y_offset + (i / cols) * size, size - 1);
		}
		
		if (LuaTexturePaletteSelected() >= 0)
		{
			int i = LuaTexturePaletteSelected();
			screen_rectangle r;
			r.left = (i % cols) * size + x_offset;
			r.right = r.left + size;
			r.top = 320 + y_offset + (i / cols) * size;
			r.bottom = r.top + size;
			FrameRect(&r, _inventory_text_color);
		}

		ForceUpdate = true;
	}

	return ForceUpdate;
}


/*
 *  Update energy bar
 */

void HUD_Class::update_suit_energy(short time_elapsed)
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
		   (current_player->suit_energy==PLAYER_MAXIMUM_SUIT_ENERGY ||
			current_player->suit_energy==2*PLAYER_MAXIMUM_SUIT_ENERGY || 
			current_player->suit_energy==3*PLAYER_MAXIMUM_SUIT_ENERGY))
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

		interface_state.shield_is_dirty= false;
	}
}


/*
 *  Update oxygen bar
 */

void HUD_Class::update_suit_oxygen(short time_elapsed)
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
		interface_state.oxygen_is_dirty= false;
	}
}


/*
 *  A change of weapon has occurred, change the weapon display panel
 */

void HUD_Class::update_weapon_panel(bool force_redraw)
{
	if(force_redraw || interface_state.weapon_is_dirty)
	{
		// LP addition: display needs to be updated
		ForceUpdate = true;
		
		char *weapon_name = temporary;
		struct weapon_interface_data *definition;
		screen_rectangle *destination= get_interface_rectangle(_weapon_display_rect);
		screen_rectangle source;
		short desired_weapon= get_player_desired_weapon(current_player_index);

		/* Now we have to erase, because the panel won't do it for us.. */
		FillRect(destination, _inventory_background_color);
	
		if(desired_weapon != NONE)
		{
			assert(desired_weapon>=0 && desired_weapon<short(MAXIMUM_WEAPON_INTERFACE_DEFINITIONS));

			definition= weapon_interface_definitions+desired_weapon;
	
			/* Check if it is a multi weapon */
			if(definition->multi_weapon)
			{
				if (definition->multiple_unusable_shape != UNONE)
				{
					/* always draw the single */
					if (definition->weapon_panel_shape != UNONE)
						DrawShapeAtXY(definition->weapon_panel_shape, 
							      definition->standard_weapon_panel_left, 
							      definition->standard_weapon_panel_top);
					
					if(current_player->items[definition->item_id]>1)
					{
						if (definition->multiple_shape != UNONE)
							DrawShapeAtXY(
								definition->multiple_shape, 
								definition->standard_weapon_panel_left + definition->multiple_delta_x, 
								definition->standard_weapon_panel_top + definition->multiple_delta_y);
					} 
					else 
					{
						/* Draw the empty one.. */
						DrawShapeAtXY(
							definition->multiple_unusable_shape,
							definition->standard_weapon_panel_left + definition->multiple_delta_x, 
							definition->standard_weapon_panel_top + definition->multiple_delta_y);
					}
				} 
				else 
				{
					if(current_player->items[definition->item_id]>1)
					{
						if (definition->multiple_shape != UNONE)
							DrawShapeAtXY(
								definition->multiple_shape,
								definition->standard_weapon_panel_left + definition->multiple_delta_x, 
								definition->standard_weapon_panel_top + definition->multiple_delta_y);
					} else {
						if (definition->weapon_panel_shape != UNONE)
							DrawShapeAtXY(definition->weapon_panel_shape, 
								      definition->standard_weapon_panel_left, 
								      definition->standard_weapon_panel_top);
					}
				}
			} else {
				/* Slam it to the screen! */
				if(definition->weapon_panel_shape != UNONE)
				{
					DrawShapeAtXY(definition->weapon_panel_shape, 
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
				get_item_name(weapon_name, item_index, false);
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
			
			DrawText(weapon_name, &source, _center_horizontal|_center_vertical|_wrap_text,
				_weapon_name_font, _inventory_text_color);
				
			/* And make sure that the ammo knows it needs to update */
			interface_state.ammo_is_dirty= true;
		} 
		interface_state.weapon_is_dirty= false;
	}
}


/*
 *  Update ammunition display
 */

void HUD_Class::update_ammo_display(bool force_redraw)
{
	if(force_redraw || interface_state.ammo_is_dirty)
	{
		// LP addition: display needs to be updated
		ForceUpdate = true;
		
		draw_ammo_display_in_panel(_primary_interface_ammo);
		draw_ammo_display_in_panel(_secondary_interface_ammo);
		interface_state.ammo_is_dirty= false;
	}
}


/*
 *  Update inventory display
 */

void HUD_Class::update_inventory_panel(bool force_redraw)
{
	/* changed_item gets erased first.. */
	/* This should probably go to a gworld first, or something */
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
		FillRect(&text_rectangle, _inventory_background_color);
				
#if !defined(DISABLE_NETWORKING)
		if (item_type==_network_statistics)
		{
			char remaining_time[16];
			int seconds = dynamic_world->game_information.game_time_remaining / TICKS_PER_SECOND;
			if (seconds / 60 < 1000) // start counting down at 999 minutes
			{ 
				sprintf(remaining_time, "%d:%02d", seconds/60, seconds%60);
				draw_inventory_time(remaining_time, current_row-1); // compensate for current_row++ above
			} else if (GET_GAME_OPTIONS() & _game_has_kill_limit) 
			{
				switch (GET_GAME_TYPE())
				{
					case _game_of_kill_monsters:
					case _game_of_cooperative_play:
					case _game_of_king_of_the_hill:
					case _game_of_kill_man_with_ball:
					case _game_of_tag:

						short player_index;
						int kill_limit = INT_MAX;
						for (player_index = 0; player_index < dynamic_world->player_count;++player_index)
						{
							struct player_data *player = get_player_data(player_index);

							int kills_left = dynamic_world->game_information.kill_limit - (player->total_damage_given.kills - player->damage_taken[player_index].kills);
							if (kills_left < kill_limit) kill_limit = kills_left;
						}
						char kills_left[4];
						sprintf(kills_left, "%d", kill_limit);
						draw_inventory_time(kills_left, current_row-1);
						break;
				}
			}

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
				DrawText(player->name, &dest_rect, _center_vertical, 
					 _interface_font, PLAYER_COLOR_BASE_INDEX+player->color);
					
				/* Now draw the ranking_text */
				dest_rect.right+= width;
				dest_rect.left= dest_rect.right-width;
				DrawText(temporary, &dest_rect, _center_vertical, 
					 _interface_font, PLAYER_COLOR_BASE_INDEX+player->color);
			}
		}
		else
#endif // !defined(DISABLE_NETWORKING)
		{
			/* Draw the items. */
			for(loop= 0; loop<section_count && current_row<max_lines; ++loop)
			{
				bool valid_in_this_environment;
			
				/* Draw the item */
				get_item_name(temporary, section_items[loop], (section_counts[loop]!=1));
				valid_in_this_environment= item_valid_in_current_environment(section_items[loop]);
				draw_inventory_item(temporary, section_counts[loop], current_row++, false, valid_in_this_environment);
			}
		}
		
		SET_INVENTORY_DIRTY_STATE(current_player, false);
	}
}


/*
 *  Draw the text in the rectangle, starting at the given offset, on the
 *  far left.  Headers also have their backgrounds erased first.
 */

void HUD_Class::draw_inventory_header(char *text, short offset)
{
	screen_rectangle destination;

	calculate_inventory_rectangle_from_offset(&destination, offset);

	/* Erase.. */
	FillRect(&destination, _inventory_header_background_color);

	/* Now draw the text. */	
	destination.left+= TEXT_INSET;
	DrawText(text, &destination, _center_vertical, _interface_font,
		_inventory_text_color);
}

/*
 * Draw the time in the far right of the rectangle; does not erase the background...should always be called after
 * draw_inventory_header
 */
void HUD_Class::draw_inventory_time(char *text, short offset)
{

    screen_rectangle destination;
        
    calculate_inventory_rectangle_from_offset(&destination, offset);
    
    destination.left = destination.right - _text_width(text, _interface_font) - TEXT_INSET;
    DrawText(text, &destination, _center_vertical, _interface_font, _inventory_text_color);
}


/*
 *  Calculate inventory rectangle
 */

void HUD_Class::calculate_inventory_rectangle_from_offset(screen_rectangle *r, short offset)
{
	screen_rectangle *inventory_rect= get_interface_rectangle(_inventory_rect);
	short line_height= _get_font_line_height(_interface_font);
	
	*r= *inventory_rect;
	r->top += offset*line_height;
	r->bottom= r->top + line_height;
}


/*
 *  Calculate number of visible inventory lines
 */

short HUD_Class::max_displayable_inventory_lines(void)
{
	screen_rectangle *destination= get_interface_rectangle(_inventory_rect);
	return (destination->bottom-destination->top)/_get_font_line_height(_interface_font);
}


/*
 *  Draw health/oxygen bar
 */

void HUD_Class::draw_bar(screen_rectangle *rectangle, short width,
	shape_descriptor top_piece, shape_descriptor full_bar,
	shape_descriptor background_texture)
{
	screen_rectangle destination= *rectangle;
	screen_rectangle source;
	screen_rectangle bar_section;

	/* Draw the background (right). */
	destination.left+= width;
	source= destination;

	offset_rect(&source, -rectangle->left, -rectangle->top);
	DrawShape(background_texture, &destination, &source);

	bar_section= *rectangle;
	bar_section.right= bar_section.left+width-TOP_OF_BAR_WIDTH;
	
	/* Draw the top bit.. */
	if(width>2*TOP_OF_BAR_WIDTH)
	{
		DrawShapeAtXY(top_piece, rectangle->left+width-TOP_OF_BAR_WIDTH, 
			rectangle->top);
	} else {
		destination= *rectangle;

		/* Gotta take lines off the top, so that the bottom stuff is still visible.. */
		destination.left= rectangle->left+width/2+width%2;
		bar_section.right= destination.left;
		destination.right= destination.left+width/2;

		source= destination;			
		offset_rect(&source, -source.left+TOP_OF_BAR_WIDTH-width/2, -source.top);
		DrawShape(top_piece, &destination, &source);
	}

	/* Copy the bar.. */
	if(bar_section.left<bar_section.right)
	{
		screen_rectangle bar_section_source= bar_section;
		
		offset_rect(&bar_section_source, -rectangle->left, -rectangle->top);
		DrawShape(full_bar, &bar_section, &bar_section_source);
	}
}


/*
 *  Draw ammo display
 */

void HUD_Class::draw_ammo_display_in_panel(short trigger_id)
{
	struct weapon_interface_data *current_weapon_data;
	struct weapon_interface_ammo_data *current_ammo_data;
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
				
				/* Draw the full rectangle */
				FillRect(&bounds, current_ammo_data->bullet);
				
				/* Inset the rectangle.. */
				bounds.left+=1; bounds.right-=1; bounds.bottom-= 1; bounds.top+=1;
				
				/* ...and erase the empty part of the rectangle */
				bounds.bottom-= fill_height;
				bounds.top= current_ammo_data->screen_top+1;
				
				/* Fill it. */
				FillRect(&bounds, current_ammo_data->empty_bullet);
				
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
					DrawShapeAtXY(current_ammo_data->bullet,
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
						offset_rect(&source, -source.left, -source.top);
						DrawShape(current_ammo_data->bullet, &destination, &source);
						
						/* Draw the used part of the row.. */
						destination.left= destination.right, 
						destination.right= destination.left+(current_ammo_data->ammo_across-partial_row_count)*current_ammo_data->delta_x;
						source= destination;
						offset_rect(&source, -source.left, -source.top);
						DrawShape(current_ammo_data->empty_bullet, &destination, &source);
					} else {
						/* Draw the used part of the row.. */
						destination.left= x, destination.top= y;
						destination.right= x+(current_ammo_data->ammo_across-partial_row_count)*current_ammo_data->delta_x;
						destination.bottom= y+current_ammo_data->delta_y;
						source= destination;
						offset_rect(&source, -source.left, -source.top);
						DrawShape(current_ammo_data->empty_bullet, &destination, &source);
						
						/* Draw the unused part of the row */
						destination.left= destination.right;
						destination.right= destination.left+(partial_row_count*current_ammo_data->delta_x);
						source= destination;
						offset_rect(&source, -source.left, -source.top);
						DrawShape(current_ammo_data->bullet, &destination, &source);
					}
					y+= current_ammo_data->delta_y;
				}
				
				/* Draw the remaining rows. */
				x= current_ammo_data->screen_left;
				for(row=0; row<(max-ammunition_count)/current_ammo_data->ammo_across; ++row)
				{
					DrawShapeAtXY(current_ammo_data->empty_bullet,
						x, y);
					y+= current_ammo_data->delta_y;
				}
			}
		}
	}
}


/*
 *  Draw inventory item
 */

void HUD_Class::draw_inventory_item(char *text, short count,
	short offset, bool erase_first, bool valid_in_this_environment)
{
	screen_rectangle destination, text_destination;
	char *count_text = temporary;
	short color;

	calculate_inventory_rectangle_from_offset(&destination, offset);

	/* Select the color for the text.. */
	color= (valid_in_this_environment) ? (_inventory_text_color) : (_invalid_weapon_color);

	/* Erase on items that changed only in count.. */
	if(erase_first)
	{
		FillRect(&destination, _inventory_background_color);
	} else {
		/* Unfortunately, we must always erase the numbers.. */
		text_destination= destination;
		text_destination.right= text_destination.left+NAME_OFFSET+TEXT_INSET;
		FillRect(&text_destination, _inventory_background_color);
	}

	/* Draw the text name.. */
	text_destination= destination;
	text_destination.left+= NAME_OFFSET+TEXT_INSET;
	DrawText(text, &text_destination, _center_vertical, _interface_font, color);

	/* Draw the text count-> Change the font!! */
	text_destination= destination;
	text_destination.left+= TEXT_INSET;
	sprintf(count_text, "%3d", count);
	DrawText(count_text, &text_destination, _center_vertical, _interface_item_count_font, color);
}


/*
 *  Draw player name
 */

void HUD_Class::draw_player_name(void)
{
	struct player_data *player= get_player_data(current_player_index);
	screen_rectangle *player_name_rect= get_interface_rectangle(_player_name_rect);

	DrawText(player->name, player_name_rect, 
		_center_vertical | _center_horizontal, _player_name_font, 
		player->color+PLAYER_COLOR_BASE_INDEX);
}


/*
 * Draw the message area, and then put messages or player name in the buffer
 */

#define MESSAGE_AREA_X_OFFSET -9
#define MESSAGE_AREA_Y_OFFSET -5

void HUD_Class::draw_message_area(short time_elapsed)
{
	if(time_elapsed == NONE)
	{
        screen_rectangle *player_name_rect = get_interface_rectangle(_player_name_rect);
		DrawShapeAtXY(
			BUILD_DESCRIPTOR(_collection_interface, _network_panel), 
			player_name_rect->left + MESSAGE_AREA_X_OFFSET, player_name_rect->top + MESSAGE_AREA_Y_OFFSET);
		draw_player_name();
	}
}
