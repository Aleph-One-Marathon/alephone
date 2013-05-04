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
	
	Overhead-Map Base-Class Renderer,
	by Loren Petrich,
	August 3, 2000
	
	The code had been moved out of overhead_map.c and overhead_map_macintosh.c;
	all of it is cross-platform except for the font-handling code.
*/


#include "cseries.h"

#include "OverheadMapRenderer.h"
#include "flood_map.h"
#include "media.h"
#include "platforms.h"
#include "player.h"
#include "render.h"

#include <string.h>
#include <stdlib.h>
#include <limits.h>


enum /* render flags */
{
	_endpoint_on_automap= 0x2000,
	_line_on_automap= 0x4000,
	_polygon_on_automap= 0x8000
};

/* ---------- macros */

#define WORLD_TO_SCREEN_SCALE_ONE 8
#define WORLD_TO_SCREEN(x, x0, scale) (((x)-(x0))>>(WORLD_TO_SCREEN_SCALE_ONE-(scale)))


// Externals:
// Changed to link properly with code in pathfinding.c
extern world_point2d *path_peek(short path_index, short *step_count);
extern short GetNumberOfPaths();


// Main rendering routine

void OverheadMapClass::Render(overhead_map_data& Control)
{
	world_distance x0= Control.origin.x, y0= Control.origin.y;
	int xoff= Control.left + Control.half_width, yoff= Control.top + Control.half_height;
	short scale= Control.scale;
	world_point2d location;
	short i;
	
	// LP addition: overall setup
	begin_overall();
		
	// LP addition: stuff for setting the game options, since they get defaulted to 0
	// Made compatible with map cheat
	assert(ConfigPtr);
	if (ConfigPtr->ShowAliens) GET_GAME_OPTIONS() |= _overhead_map_shows_monsters;
	if (ConfigPtr->ShowItems) GET_GAME_OPTIONS() |= _overhead_map_shows_items;
	if (ConfigPtr->ShowProjectiles) GET_GAME_OPTIONS() |= _overhead_map_shows_projectiles;
	
	if (Control.mode==_rendering_checkpoint_map) generate_false_automap(Control.origin_polygon_index);
	
	transform_endpoints_for_overhead_map(Control);
	
	// LP addition
	begin_polygons();
	
	/* shade all visible polygons */
	for (i=0;i<dynamic_world->polygon_count;++i)
	{
		struct polygon_data *polygon= get_polygon_data(i);
		if (POLYGON_IS_IN_AUTOMAP(i) && TEST_STATE_FLAG(i, _polygon_on_automap)
			&&(polygon->floor_transfer_mode!=_xfer_landscape||polygon->ceiling_transfer_mode!=_xfer_landscape))
		{
			
			if (!POLYGON_IS_DETACHED(polygon))
			{
				short color;
				
				switch (polygon->type)
				{
					case _polygon_is_platform:
						color= PLATFORM_IS_SECRET(get_platform_data(polygon->permutation)) ?
							_polygon_color : _polygon_platform_color;
						if (PLATFORM_IS_FLOODED(get_platform_data(polygon->permutation)))
						{
							short adj_index = find_flooding_polygon(i);
							if (adj_index != NONE)
							{
								switch (get_polygon_data(adj_index)->type)
								{
									case _polygon_is_minor_ouch:
										color = _polygon_minor_ouch_color;
										break;
									case _polygon_is_major_ouch:
										color = _polygon_major_ouch_color;
										break;
								}
							}
						}
						break;
					
					case _polygon_is_minor_ouch:
						color = _polygon_minor_ouch_color;
						break;
					
					case _polygon_is_major_ouch:
						color = _polygon_major_ouch_color;
						break;
                        
					case _polygon_is_teleporter:
						color = _polygon_teleporter_color;
						break;
                        
				case _polygon_is_hill:
					color = _polygon_hill_color;
					break;
					
					default:
						color= _polygon_color;
						break;
				}

				if (polygon->media_index!=NONE)
				{
					struct media_data *media= get_media_data(polygon->media_index);
					
					// LP change: idiot-proofing
					if (media)
					{
						if (media->height>=polygon->floor_height)
						{
							switch (media->type)
							{
								case _media_water: color= _polygon_water_color; break;
								case _media_lava: color= _polygon_lava_color; break;
								case _media_goo: color= _polygon_goo_color; break;
								// LP change: separated sewage and JjaroGoo
								case _media_sewage: color= _polygon_sewage_color; break;
								case _media_jjaro: color = _polygon_jjaro_color; break;
							}
						}
					}
				}
				
				draw_polygon(polygon->vertex_count, polygon->endpoint_indexes, color, scale);
			}
		}
	}

	// LP addition
	end_polygons();

	// LP addition
	begin_lines();
	
	/* draw all visible lines */
	for (i=0;i<dynamic_world->line_count;++i)
	{
		short line_color= NONE;
		struct line_data *line= get_line_data(i);
		
		if (LINE_IS_IN_AUTOMAP(i))
		{
			if ((line->clockwise_polygon_owner!=NONE && TEST_STATE_FLAG(line->clockwise_polygon_owner, _polygon_on_automap)) ||
				(line->counterclockwise_polygon_owner!=NONE && TEST_STATE_FLAG(line->counterclockwise_polygon_owner, _polygon_on_automap)))
			{
				struct polygon_data *clockwise_polygon= line->clockwise_polygon_owner==NONE ? NULL : get_polygon_data(line->clockwise_polygon_owner);
				struct polygon_data *counterclockwise_polygon= line->counterclockwise_polygon_owner==NONE ? NULL : get_polygon_data(line->counterclockwise_polygon_owner);

				if (LINE_IS_SOLID(line) || LINE_IS_VARIABLE_ELEVATION(line))
				{
					if (LINE_IS_LANDSCAPED(line))
					{
						if ((!clockwise_polygon||clockwise_polygon->floor_transfer_mode!=_xfer_landscape) &&
							(!counterclockwise_polygon||counterclockwise_polygon->floor_transfer_mode!=_xfer_landscape))
						{
							line_color= _elevation_line_color;
						}
					}
					else
					{
						line_color= _solid_line_color;
					}
				}
				else
				{
					if (clockwise_polygon->floor_height!=counterclockwise_polygon->floor_height)
					{
						line_color= LINE_IS_LANDSCAPED(line) ? NONE : static_cast<short>(_elevation_line_color);
					}
				}
			}
			
			if (line_color!=NONE) draw_line(i, line_color, scale);
		}
	}

	// LP addition
	end_lines();
	
	/* print all visible tags */
	if (scale!=OVERHEAD_MAP_MINIMUM_SCALE)
	{
		struct map_annotation *annotation;
		
		i= 0;
		while ((annotation= get_next_map_annotation(&i))!=NULL)
		{
			if (POLYGON_IS_IN_AUTOMAP(annotation->polygon_index) &&
				TEST_STATE_FLAG(annotation->polygon_index, _polygon_on_automap))
			{
				location.x= xoff + WORLD_TO_SCREEN(annotation->location.x, x0, scale);
				location.y= yoff + WORLD_TO_SCREEN(annotation->location.y, y0, scale);
				
				draw_annotation(&location, annotation->type, annotation->text, scale);
			}
		}
	}

	if (ConfigPtr->ShowPaths)
	{
		short path_index;
		
		// LP change: made this more general
		set_path_drawing();
		
		// LP change: there may be more than 20 paths
		for (path_index=0;path_index<GetNumberOfPaths();path_index++)
		{
			world_point2d *points;
			short step, count;
			
			points= path_peek(path_index, &count);
			if (points)
			{
				for (step= 0; step<count; ++step)
				{
					location.x= xoff + WORLD_TO_SCREEN(points[step].x, x0, scale);
					location.y= yoff + WORLD_TO_SCREEN(points[step].y, y0, scale);
					// LP change: made this more general
					draw_path(step,location);
				}
			}
			// LP addition: indicate when a path is complete
			finish_path();
		}
	}
	
	if (Control.mode!=_rendering_checkpoint_map)	
	{
		struct object_data *object;
		
		for (i=0, object= objects; i<MAXIMUM_OBJECTS_PER_MAP; ++i, ++object)
		{
			if (SLOT_IS_USED(object))
			{
				if (!OBJECT_IS_INVISIBLE(object))
				{
					short thing_type= NONE;
					
					switch (GET_OBJECT_OWNER(object))
					{
						case _object_is_monster:
						{
							struct monster_data *monster= get_monster_data(object->permutation);
							
							if (MONSTER_IS_PLAYER(monster))
							{
								struct player_data *player= get_player_data(monster_index_to_player_index(object->permutation));
	
								if ((GET_GAME_OPTIONS()&_overhead_map_is_omniscient) || local_player->team==player->team)
								{
									location.x= xoff + WORLD_TO_SCREEN(object->location.x, x0, scale);
									location.y= yoff + WORLD_TO_SCREEN(object->location.y, y0, scale);
									
									draw_player(&location, object->facing, player->team, scale);
								}
							}
							else
							{
								// LP: use the lookup system
								switch (ConfigPtr->monster_displays[monster->type])
								{
									case _civilian_thing:
										thing_type= _civilian_thing;
										break;
									
									case _monster_thing:
										if (GET_GAME_OPTIONS()&_overhead_map_shows_monsters)
											thing_type= _monster_thing;
										break;
								}
							}
							break;
						}
	
						case _object_is_projectile:
							if ((GET_GAME_OPTIONS()&_overhead_map_shows_projectiles) && object->shape!=UNONE)
							{
								thing_type= _projectile_thing;
							}
							break;
						
						case _object_is_item:
							if (GET_GAME_OPTIONS()&_overhead_map_shows_items)
							{
								thing_type= _item_thing;
							}
							break;
							
						case _object_is_garbage:
							// LP change: making this more general
							switch (ConfigPtr->dead_monster_displays[GET_COLLECTION(GET_DESCRIPTOR_COLLECTION(object->shape))])
							{
							case _civilian_thing:
								thing_type= _civilian_thing;
								break;
							
							case _monster_thing:
								if (GET_GAME_OPTIONS()&_overhead_map_shows_monsters)
									thing_type= _monster_thing;
								break;
							}
							/*
							if (GET_COLLECTION(GET_DESCRIPTOR_COLLECTION(object->shape))==_collection_civilian)
							{
								thing_type= _civilian_thing;
							}
							*/
							break;
					}
					
					if (thing_type!=NONE)
					{
						// Making this more general, in case we want to see monsters and stuff
						if (thing_type==_projectile_thing || ((dynamic_world->tick_count+i)&8))
						// if (thing_type!=_civilian_thing || ((dynamic_world->tick_count+i)&8))
						{
							location.x= xoff + WORLD_TO_SCREEN(object->location.x, x0, scale);
							location.y= yoff + WORLD_TO_SCREEN(object->location.y, y0, scale);
							
							draw_thing(&location, object->facing, thing_type, scale);
						}
					}
				}
			}
		}
	}
	else
	{
		for (i= 0; i<dynamic_world->initial_objects_count; ++i)
		{
			struct map_object *saved_object= saved_objects + i;
			
			if (saved_object->type==_saved_goal &&
				saved_object->location.x==Control.origin.x && saved_object->location.y==Control.origin.y)
			{
				location.x= xoff + WORLD_TO_SCREEN(saved_object->location.x, x0, scale);
				location.y= yoff + WORLD_TO_SCREEN(saved_object->location.y, y0, scale);
				draw_thing(&location, 0, _checkpoint_thing, scale);
			}
		}
	}

	if (Control.mode==_rendering_game_map) draw_map_name(Control, static_world->level_name);
	if (Control.mode==_rendering_checkpoint_map) replace_real_automap();
	
	// LP addition: overall cleanup
	end_overall();
}


void OverheadMapClass::transform_endpoints_for_overhead_map(
	struct overhead_map_data& Control)
{
	world_distance x0= Control.origin.x, y0= Control.origin.y;
	int xoff= Control.left + Control.half_width, yoff = Control.top + Control.half_height;
	short scale= Control.scale;
	short i;

	/* transform all our endpoints into screen space, remembering which ones are visible */
	for (i=0;i<dynamic_world->endpoint_count;++i)
	{
		struct endpoint_data *endpoint= get_endpoint_data(i);
		
		endpoint->transformed.x= xoff + WORLD_TO_SCREEN(endpoint->vertex.x, x0, scale);
		endpoint->transformed.y= yoff + WORLD_TO_SCREEN(endpoint->vertex.y, y0, scale);

		if (endpoint->transformed.x >= Control.left &&
            endpoint->transformed.y >= Control.top &&
            endpoint->transformed.y <= Control.top + Control.height &&
            endpoint->transformed.x <= Control.left + Control.width)
		{
			SET_STATE_FLAG(i, _endpoint_on_automap, true);
		}
	}

	/* sweep the polygon array, determining which polygons are visible based on their
		endpoints */
	for (i=0;i<dynamic_world->polygon_count;++i)
	{
		struct polygon_data *polygon= get_polygon_data(i);
		short j;
		
		for (j=0;j<polygon->vertex_count;++j)
		{
			if (TEST_STATE_FLAG(polygon->endpoint_indexes[j], _endpoint_on_automap))
			{
				SET_STATE_FLAG(i, _polygon_on_automap, true);
				break;
			}
		}
	}
}

/* --------- the false automap */

static void add_poly_to_false_automap(short polygon_index)
{
	struct polygon_data *polygon= get_polygon_data(polygon_index);
	for (int i = 0; i < polygon->vertex_count; ++i)
            ADD_LINE_TO_AUTOMAP(polygon->line_indexes[i]);
	ADD_POLYGON_TO_AUTOMAP(polygon_index);
}

void OverheadMapClass::generate_false_automap(
	short polygon_index)
{
	int32 automap_line_buffer_size, automap_polygon_buffer_size;

	automap_line_buffer_size= (dynamic_world->line_count/8+((dynamic_world->line_count%8)?1:0))*sizeof(byte);
	automap_polygon_buffer_size= (dynamic_world->polygon_count/8+((dynamic_world->polygon_count%8)?1:0))*sizeof(byte);

	/* allocate memory for the old automap memory */
	saved_automap_lines= new byte[automap_line_buffer_size];
	saved_automap_polygons= new byte[automap_polygon_buffer_size];

	if (saved_automap_lines && saved_automap_polygons)
	{
		memcpy(saved_automap_lines, automap_lines, automap_line_buffer_size);
		memcpy(saved_automap_polygons, automap_polygons, automap_polygon_buffer_size);
		memset(automap_lines, 0, automap_line_buffer_size);
		memset(automap_polygons, 0, automap_polygon_buffer_size);
		
		add_poly_to_false_automap(polygon_index);
		polygon_index= flood_map(polygon_index, INT32_MAX, false_automap_cost_proc, _breadth_first, (void *) NULL);
		do
		{
			polygon_index= flood_map(NONE, INT32_MAX, false_automap_cost_proc, _breadth_first, (void *) NULL);
		}
		while (polygon_index!=NONE);
	}
}


void OverheadMapClass::replace_real_automap(
	void)
{
	if (saved_automap_lines)
	{
		int32 automap_line_buffer_size= (dynamic_world->line_count/8+((dynamic_world->line_count%8)?1:0))*sizeof(byte);
		memcpy(automap_lines, saved_automap_lines, automap_line_buffer_size);
		delete []saved_automap_lines;
		saved_automap_lines= (byte *) NULL;
	}
	
	if (saved_automap_polygons)
	{
		int32 automap_polygon_buffer_size= (dynamic_world->polygon_count/8+((dynamic_world->polygon_count%8)?1:0))*sizeof(byte);
	
		memcpy(automap_polygons, saved_automap_polygons, automap_polygon_buffer_size);
		delete []saved_automap_polygons;
		saved_automap_polygons= (byte *) NULL;
	}
}


int32 OverheadMapClass::false_automap_cost_proc(
	short source_polygon_index,
	short line_index,
	short destination_polygon_index,
	void *caller_data)
{
	struct polygon_data *destination_polygon= get_polygon_data(destination_polygon_index);
	struct polygon_data *source_polygon= get_polygon_data(source_polygon_index);
	int32 cost= 1;

	(void) (line_index);
	(void) (caller_data);
	
	/* canÕt leave secret platforms */
	if (source_polygon->type==_polygon_is_platform &&
		PLATFORM_IS_SECRET(get_platform_data(source_polygon->permutation)))
	{
		cost= -1;
	}
	
	/* canÕt enter secret platforms which are also doors */
	if (destination_polygon->type==_polygon_is_platform)
	{
		struct platform_data *platform= get_platform_data(destination_polygon->permutation);
		
		if (PLATFORM_IS_DOOR(platform))
		{
			if (PLATFORM_IS_SECRET(platform)) cost= -1;
		}
	}

	/* add the destination polygon and all its lines to the automap */
	if (cost > 0)
		add_poly_to_false_automap(destination_polygon_index);
	
	return cost;
}
