#ifndef _OVERHEAD_MAP_CLASS_
#define _OVERHEAD_MAP_CLASS_
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
	
	Overhead-Map Base Class and Data,
	by Loren Petrich,
	August 3, 2000
	
	The definitions had been moved out of overhead_map.c and overhead_map_macintosh.c

Dec 17, 2000 (Loren Petrich):
	Added font abstraction
*/

#include "cseries.h"
#include "world.h"
#include "map.h"
#include "monsters.h"
#include "overhead_map.h"
#include "shape_descriptors.h"
#include "shell.h"
#include "FontHandler.h"


/* ---------- constants */

enum /* polygon colors */
{
	_polygon_color,
	_polygon_platform_color,
	_polygon_water_color,
	_polygon_lava_color,
	_polygon_sewage_color,
	_polygon_jjaro_color, 	// LP addition
	_polygon_goo_color,		// LP: PfhorSlime moved down here
	_polygon_hill_color,
	_polygon_minor_ouch_color,	// LP, AlexJS: these two added for M1 compatibility
	_polygon_major_ouch_color,
	_polygon_teleporter_color,
	NUMBER_OF_POLYGON_COLORS,
	// For backwards compatibility: all those before the "ouch" colors
	NUMBER_OF_OLD_POLYGON_COLORS = _polygon_hill_color + 1
};

enum /* line colors */
{
	_solid_line_color,
	_elevation_line_color,
	_control_panel_line_color,
	NUMBER_OF_LINE_DEFINITIONS
};

// LP change: Items and monsters were interchanged to get the aliens
// closer to the civilians

enum /* thing colors */
{
	_civilian_thing,
	_monster_thing,
	_item_thing,
	_projectile_thing,
	_checkpoint_thing,
	NUMBER_OF_THINGS
};

enum
{
	_rectangle_thing,
	_circle_thing
};


// Data constituents

// Note: all the colors were changed from RGBColor to rgb_color,
// which has the same members (3 unsigned shorts), but which is intended to be more portable.

struct line_definition
{
	rgb_color color;
	short pen_sizes[OVERHEAD_MAP_MAXIMUM_SCALE-OVERHEAD_MAP_MINIMUM_SCALE+1];
};

struct thing_definition
{
	rgb_color color;
	short shape;
	short radii[OVERHEAD_MAP_MAXIMUM_SCALE-OVERHEAD_MAP_MINIMUM_SCALE+1];
};

struct entity_definition
{
	short front, rear, rear_theta;
};

const int NUMBER_OF_ANNOTATION_SIZES = OVERHEAD_MAP_MAXIMUM_SCALE-OVERHEAD_MAP_MINIMUM_SCALE+1;
struct annotation_definition
{
	rgb_color color;
	FontSpecifier Fonts[NUMBER_OF_ANNOTATION_SIZES];
};

// For some reason, only one annotation color was ever implemented
const int NUMBER_OF_ANNOTATION_DEFINITIONS = 1;

// Collection of the data used to do the map name
struct map_name_definition
{
	rgb_color color;
	FontSpecifier Font;
	short offset_down;	// from top of screen
};

// The configuration-data structure itself
// Note: there is only one definition of the player-entity shape
// instead of one for 

struct OvhdMap_CfgDataStruct
{
	rgb_color polygon_colors[NUMBER_OF_POLYGON_COLORS];
	line_definition line_definitions[NUMBER_OF_LINE_DEFINITIONS];
	thing_definition thing_definitions[NUMBER_OF_THINGS];
	short monster_displays[NUMBER_OF_MONSTER_TYPES];
	short dead_monster_displays[NUMBER_OF_COLLECTIONS];
	entity_definition player_entity;
	annotation_definition annotation_definitions[NUMBER_OF_ANNOTATION_DEFINITIONS];
	map_name_definition map_name_data;
	rgb_color path_color;
	
	// Which of these to show
	bool ShowAliens;
	bool ShowItems;
	bool ShowProjectiles;
	bool ShowPaths;
};


// Now for the overhead-map-renderer class;
// this is to be subclassed for whatever renderers one wants to use.

class OverheadMapClass
{
	// Auxiliary routines:
	// The cost function for the checkpoint automap must be static
	// so it can be called properly
	void transform_endpoints_for_overhead_map(overhead_map_data &Control);
	void generate_false_automap(short polygon_index);
	static int32 false_automap_cost_proc(short source_polygon_index, short line_index, short destination_polygon_index, void *caller_data);
	void replace_real_automap(void);
	
	// For the false automap
	byte *saved_automap_lines, *saved_automap_polygons;

protected:

	// Text justification
	enum
	{
		_justify_left,
		_justify_center
	};
	
	// For special overall things
	virtual void begin_overall() {}
	virtual void end_overall() {}

	virtual void begin_polygons() {}
	virtual void draw_polygon(
		short vertex_count,
		short *vertices,
		rgb_color& color) {}
	virtual void end_polygons() {}
	
	virtual void begin_lines() {}
	virtual void draw_line(
		short *vertices,
		rgb_color& color,
		short pen_size) {}
	virtual void end_lines() {}
	
	virtual void begin_things() {}
	virtual void draw_thing(
		world_point2d& center,
		rgb_color& color,
		short shape,
		short radius) {}
	virtual void end_things() {}
	
	virtual void draw_player(
		world_point2d& center,
		angle facing,
		rgb_color& color,
		short shrink,
		short front,
		short rear,
		short rear_theta) {}
	
	virtual void draw_text(
		world_point2d& location,
		rgb_color& color,
		char *text,
		FontSpecifier& FontData,
		short justify) {}
	
	virtual void set_path_drawing(rgb_color& color) {}
	virtual void draw_path(
		short step,	// 0: first point
		world_point2d& location) {}
	virtual void finish_path() {}
	
	// Get vertex with the appropriate transformation:
	static world_point2d& GetVertex(short index) {return get_endpoint_data(index)->transformed;}
	
	// Get pointer to first vertex
	static world_point2d *GetFirstVertex() {return &GetVertex(0);}
	
	// Get the vertex stride, for the convenience of OpenGL
	static int GetVertexStride() {return sizeof(endpoint_data);}

private:
	// Auxiliary functions to be done inline;
	// these are overloads of the corresponding graphics-API-specific virtual functions
	// defined earlier.
	void draw_polygon(
		short vertex_count,
		short *vertices,
		short color,
		short scale)
		{
			(void)(scale);
			if (!(color>=0&&color<NUMBER_OF_POLYGON_COLORS)) return;
			draw_polygon(vertex_count, vertices, ConfigPtr->polygon_colors[color]);
		}
	void draw_line(
		short line_index,
		short color,
		short scale)
		{
			if (!(color>=0&&color<NUMBER_OF_LINE_DEFINITIONS)) return;
			line_definition& LineDef = ConfigPtr->line_definitions[color];
			draw_line(get_line_data(line_index)->endpoint_indexes,
				LineDef.color, LineDef.pen_sizes[scale-OVERHEAD_MAP_MINIMUM_SCALE]);
		}
	void draw_thing(
		world_point2d *center,
		angle facing,
		short color,
		short scale)
		{
			if (!(color>=0&&color<NUMBER_OF_THINGS)) return;
			thing_definition& ThingDef = ConfigPtr->thing_definitions[color];
			draw_thing(*center, ThingDef.color, ThingDef.shape,
				ThingDef.radii[scale-OVERHEAD_MAP_MINIMUM_SCALE]);
		}
	void draw_player(
		world_point2d *center,
		angle facing,
		short color,
		short scale)
		{
			rgb_color PlayerColor;
			_get_player_color(color, (RGBColor*)&PlayerColor);
			
			// Changed to use only one entity shape
			entity_definition& EntityDef = ConfigPtr->player_entity;
			draw_player(*center, facing, PlayerColor,
				OVERHEAD_MAP_MAXIMUM_SCALE-scale,
					EntityDef.front, EntityDef.rear, EntityDef.rear_theta);
		}
	void draw_annotation(
		world_point2d *location,
		short color,
		char *text,
		short scale)
	{
		if (!(color>=0&&color<NUMBER_OF_ANNOTATION_DEFINITIONS)) return;
		if (!(scale>=OVERHEAD_MAP_MINIMUM_SCALE&&scale<=OVERHEAD_MAP_MAXIMUM_SCALE)) return;
		annotation_definition& NoteDef = ConfigPtr->annotation_definitions[color];
		draw_text(*location,NoteDef.color, text,
			NoteDef.Fonts[scale-OVERHEAD_MAP_MINIMUM_SCALE],_justify_left);
	}
	void draw_map_name(
		overhead_map_data &Control,
		char *name)
	{
		map_name_definition& map_name_data = ConfigPtr->map_name_data;
		world_point2d location;
		location.x = Control.left + Control.half_width;
		location.y = Control.top + map_name_data.offset_down;
		draw_text(location, map_name_data.color, name,
			map_name_data.Font, _justify_center);
	}

	void set_path_drawing()
	{
		set_path_drawing(ConfigPtr->path_color);
	}
	
public:
	// Pointer to configuration data
	OvhdMap_CfgDataStruct *ConfigPtr;

	// Needs both the configuration data for displaying the map
	void Render(overhead_map_data& Control);
	
	// Constructor (idiot-proofer)
	OverheadMapClass(): ConfigPtr(NULL) {}

	// Destructor
	virtual ~OverheadMapClass() {}
};


#endif
