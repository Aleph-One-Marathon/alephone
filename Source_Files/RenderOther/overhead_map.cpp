/*
OVERHEAD_MAP.C

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

Friday, June 10, 1994 2:53:51 AM

Saturday, June 11, 1994 1:27:58 AM
	the portable parts of this file should be moved into RENDER.C 
Friday, August 12, 1994 7:57:00 PM
	invisible polygons and lines are never drawn.
Thursday, September 8, 1994 8:19:15 PM (Jason)
	changed behavior of landscaped lines
Monday, October 24, 1994 4:35:38 PM (Jason)
	only draw the checkpoint at the origin.
Monday, October 31, 1994 3:52:00 PM (Jason)
	draw name of map on overhead map, last.
Monday, August 28, 1995 1:44:43 PM  (Jason)
	toward portability; removed clip region from _render_overhead_map.

Feb 3, 2000 (Loren Petrich):
	Jjaro-goo color is the same as the sewage color

Feb 4, 2000 (Loren Petrich):
	Changed halt() to assert(false) for better debugging

Feb 18, 2000 (Loren Petrich):
	Made VacBobs display properly 

May 2, 2000 (Loren Petrich):
	Added XML setting of overhead-map-display parameters;
	also imported the number of paths for displaying them.
	
	Can display alien monsters, items, projectiles, and paths.

Jul 4, 2000 (Loren Petrich):
	Made XML map-display settings compatible with the map cheat.

Jul 8, 2000 (Loren Petrich):
	Added support for OpenGL rendering;
	in these routines, it's the global flag OGL_MapActive,
	which indicates whether to do so in the overhead map

Jul 16, 2000 (Loren Petrich):
	Added begin/end pairs for polygons and lines,
	so that caching of them can be more efficient (important for OpenGL)

[Loren Petrich: notes for this file moved here]
OVERHEAD_MAP_MAC.C
Monday, August 28, 1995 1:41:36 PM  (Jason)

Feb 3, 2000 (Loren Petrich):
	Jjaro-goo color is the same as the sewage color

Feb 4, 2000 (Loren Petrich):
	Changed halt() to assert(false) for better debugging

Jul 8, 2000 (Loren Petrich):
	Added support for OpenGL rendering, in the form of calls to OpenGL versions

Jul 16, 2000 (Loren Petrich):
	Added begin/end pairs for polygons and lines,
	so that caching of them can be more efficient (important for OpenGL)

Aug 3, 2000 (Loren Petrich):
	All the code here has been transferred to either OverheadMapRenderer.c/h or OverheadMap_QuickDraw.c/h
[End notes for overhead_map_macintosh.c]

Nov 12, 2000 (Loren Petrich):
	Added automap reset function and XML parsing
*/

#include "cseries.h"

#include "shell.h" // for _get_player_color

#include "map.h"
#include "monsters.h"
#include "overhead_map.h"
#include "player.h"
#include "render.h"
#include "flood_map.h"
#include "platforms.h"
#include "media.h"
#include "InfoTree.h"

// Object-oriented setup of overhead-map rendering
#include "OverheadMap_SDL.h"
#include "OverheadMap_OGL.h"

#include <string.h>
#include <stdlib.h>

#ifdef DEBUG
//#define PATH_DEBUG
//#define RENDER_DEBUG
#endif

#ifdef RENDER_DEBUG
extern struct view_data *world_view;
#endif

// Constants moved out to OverheadMapRender.h
// Render flags now in OverheadMapRender.c


// The configuration data
static OvhdMap_CfgDataStruct OvhdMap_ConfigData = 
{
	// Polygon colors
	{
		{0, 12000, 0},				// Plain polygon
		{30000, 0, 0},				// Platform
		{14*256, 37*256, 63*256},	// Water
		{76*256, 27*256, 0},		// Lava
		{70*256, 90*256, 0},		// Sewage
		{70*256, 90*256, 0},		// JjaroGoo
		{137*256, 0, 137*256},		// PfhorSlime
		{0, 12000, 0},			// Hill
		{76*256, 27*256, 0},		// Minor Damage
		{137*256, 0, 137*256},		// Major Damage
		{0, 12000, 0}			// Teleporter
	},
	// Line definitions (color, 4 widths)
	{
		{{0, 65535, 0}, {1, 2, 2, 4}},	// Solid
		{{0, 40000, 0}, {1, 1, 1, 2}},	// Elevation
		{{65535, 0, 0}, {1, 2, 2, 4}}	// Control-Panel
	},
	// Thing definitions (color, shape, 4 radii)
	{
		{{0, 0, 65535}, _rectangle_thing, {1, 2, 4, 8}}, /* civilian */
		{{65535, 0, 0}, _rectangle_thing, {1, 2, 4, 8}}, /* non-player monster */
		{{65535, 65535, 65535}, _rectangle_thing, {1, 2, 3, 4}}, /* item */
		{{65535, 65535, 0}, _rectangle_thing, {1, 1, 2, 3}}, /* projectiles */
		{{65535, 0, 0}, _circle_thing, {8, 16, 16, 16}}	// LP note: this is for checkpoint locations
	},
	// Live-monster type assignments
	{
		// Marine
		_civilian_thing,
		// Ticks
		_monster_thing,
		_monster_thing,
		_monster_thing,
		// S'pht
		_monster_thing,
		_monster_thing,
		_monster_thing,
		_monster_thing,
		// Pfhor
		_monster_thing,
		_monster_thing,
		_monster_thing,
		_monster_thing,
		// Bob
		_civilian_thing,
		_civilian_thing,
		_civilian_thing,
		_civilian_thing,
		// Drone
		_monster_thing,
		_monster_thing,
		_monster_thing,
		_monster_thing,
		_monster_thing,
		// Cyborg
		_monster_thing,
		_monster_thing,
		_monster_thing,
		_monster_thing,
		// Enforcer
		_monster_thing,
		_monster_thing,
		// Hunter
		_monster_thing,
		_monster_thing,
		// Trooper
		_monster_thing,
		_monster_thing,
		// Big Cyborg, Hunter
		_monster_thing,
		_monster_thing,
		// F'lickta
		_monster_thing,
		_monster_thing,
		_monster_thing,
		// S'pht'Kr
		_monster_thing,
		_monster_thing,
		// Juggernauts
		_monster_thing,
		_monster_thing,
		// Tiny ones
		_monster_thing,
		_monster_thing,
		_monster_thing,
		// VacBobs
		_civilian_thing,
		_civilian_thing,
		_civilian_thing,
		_civilian_thing,
	},
	// Dead-monster type assignments
	{
		NONE,				// Interface (what one sees in the HUD)
		NONE,				// Weapons in Hand
		
		_monster_thing,		// Juggernaut
		_monster_thing,		// Tick
		_monster_thing,		// Explosion effects
		_monster_thing,		// Hunter
		NONE,				// Player
	
		_monster_thing,		// Items
		_monster_thing,		// Trooper
		_monster_thing,		// Fighter
		_monster_thing,		// S'pht'Kr
		_monster_thing,		// F'lickta
		
		_civilian_thing,	// Bob
		_civilian_thing,	// VacBob
		_monster_thing,		// Enforcer
		_monster_thing,		// Drone
		_monster_thing,		// S'pht
		
		NONE,				// Water
		NONE,				// Lava
		NONE,				// Sewage
		NONE,				// Jjaro
		NONE,				// Pfhor
	
		NONE,				// Water Scenery
		NONE,				// Lava Scenery
		NONE,				// Sewage Scenery
		NONE,				// Jjaro Scenery
		NONE,				// Pfhor Scenery
		
		NONE,				// Day
		NONE,				// Night
		NONE,				// Moon
		NONE,				// Outer Space
		
		_monster_thing		// Cyborg
	},
	// Player-entity definition
	{16, 10, (7*NUMBER_OF_ANGLES)/20},
	// Annotations (color, 4 fonts)
	{
		{{0, 65535, 0},
		{
			{"Monaco",  5, styleBold, 0, "#4"},
			{"Monaco",  9, styleBold, 0, "#4"},
			{"Monaco", 12, styleBold, 0, "#4"},
			{"Monaco", 18, styleBold, 0, "#4"},
		}}
	},
	// Map name (color, font)
	{{0, 65535, 0}, {"Monaco", 18, styleNormal, 0, "#4"}, 25},
	// Path color
	{65535, 65535, 65535},
	// What to show (aliens, items, projectiles, paths)
	false, false, false, false
};
static bool MapFontsInited = false;

// Is OpenGL rendering of the map currently active?
// Set this from outside, because we want to make an OpenGL rendering for the main view,
// yet a software rendering for an in-terminal checkpoint view
bool OGL_MapActive = false;

// Software rendering
static OverheadMap_SDL_Class OverheadMap_SW;
// OpenGL rendering
#ifdef HAVE_OPENGL
static OverheadMap_OGL_Class OverheadMap_OGL;
#endif

// Overhead-map-rendering mode
enum {
	OverheadMap_Normal,
	OverheadMap_CurrentlyVisible,
	OverheadMap_All,
	NUMBER_OF_OVERHEAD_MAP_MODES
};
static short OverheadMapMode = OverheadMap_Normal;


/* ---------- code */
// LP: most of it has been moved into OverheadMapRenderer.c

static void InitMapFonts()
{
	// Init the fonts the first time through
	if (!MapFontsInited)
	{
		for (int i=0; i<NUMBER_OF_ANNOTATION_DEFINITIONS; i++)
		{
			annotation_definition& NoteDef = OvhdMap_ConfigData.annotation_definitions[i];
			for (int j=0; j<NUMBER_OF_ANNOTATION_SIZES; j++)
				NoteDef.Fonts[j].Init();
		}
		OvhdMap_ConfigData.map_name_data.Font.Init();
		MapFontsInited = true;
	}
}

void _render_overhead_map(
	struct overhead_map_data *data)
{
	InitMapFonts();
		
	// Select which kind of rendering (OpenGL or software)
	OverheadMapClass *OvhdMapPtr;
#ifdef HAVE_OPENGL
	if (OGL_MapActive)
		OvhdMapPtr = &OverheadMap_OGL;
	else
#endif
		OvhdMapPtr = &OverheadMap_SW;
	
	// Do the rendering
	OvhdMapPtr->ConfigPtr = &OvhdMap_ConfigData;
	OvhdMapPtr->Render(*data);
}


void ResetOverheadMap()
{
	// Default: nothing (mapping is cumulative)
	switch(OverheadMapMode)
	{
	case OverheadMap_CurrentlyVisible:
		// No previous visibility is carried over
		memset(automap_lines, 0, (dynamic_world->line_count/8+((dynamic_world->line_count%8)?1:0)*sizeof(byte)));
		memset(automap_polygons, 0, (dynamic_world->polygon_count/8+((dynamic_world->polygon_count%8)?1:0)*sizeof(byte)));
		
		break;
		
	case OverheadMap_All:
		// Everything is assumed visible
		memset(automap_lines, 0xff, (dynamic_world->line_count/8+((dynamic_world->line_count%8)?1:0)*sizeof(byte)));
		memset(automap_polygons, 0xff, (dynamic_world->polygon_count/8+((dynamic_world->polygon_count%8)?1:0)*sizeof(byte)));
		
		break;
	};
}



// XML elements for parsing motion-sensor specification

// Subclassed to set the color objects appropriately
const int TOTAL_NUMBER_OF_COLORS =
	NUMBER_OF_POLYGON_COLORS + NUMBER_OF_LINE_DEFINITIONS +
	NUMBER_OF_THINGS + NUMBER_OF_ANNOTATION_DEFINITIONS + 2;

const int TOTAL_NUMBER_OF_FONTS = 
	NUMBER_OF_ANNOTATION_DEFINITIONS*(OVERHEAD_MAP_MAXIMUM_SCALE-OVERHEAD_MAP_MINIMUM_SCALE + 1) + 1;

static OvhdMap_CfgDataStruct original_OvhdMap_ConfigData = OvhdMap_ConfigData;
static short original_OverheadMapMode = OverheadMapMode;

void reset_mml_overhead_map()
{
	OverheadMapMode = original_OverheadMapMode;
	OvhdMap_ConfigData = original_OvhdMap_ConfigData;
}

void parse_mml_overhead_map(const InfoTree& root)
{
	root.read_indexed("mode", OverheadMapMode, NUMBER_OF_OVERHEAD_MAP_MODES);
	root.read_attr("title_offset", OvhdMap_ConfigData.map_name_data.offset_down);

	BOOST_FOREACH(InfoTree assign, root.children_named("assign_live"))
	{
		int16 monster;
		if (!assign.read_indexed("monster", monster, NUMBER_OF_MONSTER_TYPES))
			continue;
		assign.read_attr_bounded<int16>("type", OvhdMap_ConfigData.monster_displays[monster], -1, 1);
	}
	
	BOOST_FOREACH(InfoTree assign, root.children_named("assign_dead"))
	{
		int16 coll;
		if (!assign.read_indexed("coll", coll, NUMBER_OF_COLLECTIONS))
			continue;
		assign.read_attr_bounded<int16>("type", OvhdMap_ConfigData.dead_monster_displays[coll], -1, 1);
	}
	
	BOOST_FOREACH(InfoTree child, root.children_named("aliens"))
	{
		child.read_attr("on", OvhdMap_ConfigData.ShowAliens);
	}
	BOOST_FOREACH(InfoTree child, root.children_named("items"))
	{
		child.read_attr("on", OvhdMap_ConfigData.ShowItems);
	}
	BOOST_FOREACH(InfoTree child, root.children_named("projectiles"))
	{
		child.read_attr("on", OvhdMap_ConfigData.ShowProjectiles);
	}
	BOOST_FOREACH(InfoTree child, root.children_named("paths"))
	{
		child.read_attr("on", OvhdMap_ConfigData.ShowPaths);
	}

	BOOST_FOREACH(InfoTree line, root.children_named("line_width"))
	{
		int16 index;
		if (!line.read_indexed("index", index, NUMBER_OF_LINE_DEFINITIONS))
			continue;
		
		int16 scale;
		if (!line.read_indexed("scale", scale, OVERHEAD_MAP_MAXIMUM_SCALE-OVERHEAD_MAP_MINIMUM_SCALE))
			continue;
		
		line.read_attr("width", OvhdMap_ConfigData.line_definitions[index].pen_sizes[scale]);
	}

	BOOST_FOREACH(InfoTree color, root.children_named("color"))
	{
		int16 index;
		if (!color.read_indexed("index", index, TOTAL_NUMBER_OF_COLORS))
			continue;
		
		if (index < NUMBER_OF_OLD_POLYGON_COLORS)
		{
			color.read_color(OvhdMap_ConfigData.polygon_colors[index]);
			continue;
		}
		index -= NUMBER_OF_OLD_POLYGON_COLORS;
		
		if (index < NUMBER_OF_LINE_DEFINITIONS)
		{
			color.read_color(OvhdMap_ConfigData.line_definitions[index].color);
			continue;
		}
		index -= NUMBER_OF_LINE_DEFINITIONS;
		
		if (index < NUMBER_OF_THINGS)
		{
			color.read_color(OvhdMap_ConfigData.thing_definitions[index].color);
			continue;
		}
		index -= NUMBER_OF_THINGS;
		
		if (index < NUMBER_OF_ANNOTATION_DEFINITIONS)
		{
			color.read_color(OvhdMap_ConfigData.annotation_definitions[index].color);
			continue;
		}
		index -= NUMBER_OF_ANNOTATION_DEFINITIONS;
		
		if (index == 0)
		{
			color.read_color(OvhdMap_ConfigData.map_name_data.color);
			continue;
		}
		--index;

		if (index == 0)
		{
			color.read_color(OvhdMap_ConfigData.path_color);
			continue;
		}
		--index;

		index += NUMBER_OF_OLD_POLYGON_COLORS;
		if (index < NUMBER_OF_POLYGON_COLORS)
		{
			color.read_color(OvhdMap_ConfigData.polygon_colors[index]);
			continue;
		}
		index -= NUMBER_OF_POLYGON_COLORS;
	}
	
	BOOST_FOREACH(InfoTree font, root.children_named("font"))
	{
		int16 index;
		if (!font.read_indexed("index", index, TOTAL_NUMBER_OF_FONTS))
			continue;
		
		bool found = false;
		for (int i = 0; !found && i < NUMBER_OF_ANNOTATION_DEFINITIONS; ++i)
		{
			if (index < NUMBER_OF_ANNOTATION_SIZES)
			{
				font.read_font(OvhdMap_ConfigData.annotation_definitions[i].Fonts[index]);
				found = true;
			}
			index -= NUMBER_OF_ANNOTATION_SIZES;
		}
		if (found)
			continue;
		
		if (index == 0)
		{
			font.read_font(OvhdMap_ConfigData.map_name_data.Font);
			continue;
		}
		--index;
	}
}
