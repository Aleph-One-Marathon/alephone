/*
OVERHEAD_MAP.C
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

// LP addition: to parse the colors:
#include "ColorParser.h"

// Object-oriented setup of overhead-map rendering
#ifdef mac
#include "OverheadMap_QD.h"
#else
#include "OverheadMap_SDL.h"
#endif
#include "OverheadMap_OGL.h"

#include <string.h>
#include <stdlib.h>

#ifdef env68k
#pragma segment shell
#endif

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
		{32768, 32768, 0}			// Hill
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
			{kFontIDMonaco, styleBold, 5},
			{kFontIDMonaco, styleBold, 9},
			{kFontIDMonaco, styleBold, 12},
			{kFontIDMonaco, styleBold, 18}
		}}
	},
	// Map name (color, font)
	{{0, 65535, 0}, {kFontIDMonaco, styleNormal, 18}, 25},
	// Path color
	{65535, 65535, 65535},
	// What to show (aliens, items, projectiles, paths)
	false, false, false, false
};


// Is OpenGL rendering of the map currently active?
// Set this from outside, because we want to make an OpenGL rendering for the main view,
// yet a software rendering for an in-terminal checkpoint view
bool OGL_MapActive = false;

// Software rendering
#ifdef mac
static OverheadMap_QD_Class OverheadMap_SW;
#else
static OverheadMap_SDL_Class OverheadMap_SW;
#endif
// OpenGL rendering
#ifdef HAVE_OPENGL
static OverheadMap_OGL_Class OverheadMap_OGL;
#endif


/* ---------- code */
// LP: most of it has been moved into OverheadMapRenderer.c

void _render_overhead_map(
	struct overhead_map_data *data)
{
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


// Call this from outside
void OGL_ResetMapFonts()
{
#ifdef HAVE_OPENGL
	OverheadMap_OGL.ResetFonts();
#endif
}


// XML elements for parsing motion-sensor specification;
// this is a specification of what monster type gets what
// overhead-map display. It's by monster type for living monsters
// and by collection type for dead monsters.

// Parser for living monsters
class XML_LiveAssignParser: public XML_ElementParser
{
	bool IsPresent[2];
	short Monster, Type;

public:
	bool Start();
	bool HandleAttribute(const char *Tag, const char *Value);
	bool AttributesDone();
		
	XML_LiveAssignParser(): XML_ElementParser("assign_live") {}
};

bool XML_LiveAssignParser::Start()
{
	for (int k=0; k<2; k++)
		IsPresent[k] = false;
	return true;
}

bool XML_LiveAssignParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (strcmp(Tag,"monster") == 0)
	{
		if (ReadBoundedNumericalValue(Value,"%hd",Monster,short(0),short(NUMBER_OF_MONSTER_TYPES-1)))
		{
			IsPresent[0] = true;
			return true;
		}
		else return false;
	}
	else if (strcmp(Tag,"type") == 0)
	{
		// The permissible values, -1, 0, and 1, are
		// NONE
		// _civilian_thing
		// _monster_thing
		if (ReadBoundedNumericalValue(Value,"%hd",Type,short(-1),short(1)))
		{
			IsPresent[1] = true;
			return true;
		}
		else return false;
	}
	UnrecognizedTag();
	return false;
}

bool XML_LiveAssignParser::AttributesDone()
{
	// Verify...
	bool AllPresent = IsPresent[0] && IsPresent[1];
	
	if (!AllPresent)
	{
		AttribsMissing();
		return false;
	}
	
	// Put into place
	OvhdMap_ConfigData.monster_displays[Monster] = Type;
		
	return true;
}

static XML_LiveAssignParser LiveAssignParser;


// Parser for dead monsters
class XML_DeadAssignParser: public XML_ElementParser
{
	bool IsPresent[2];
	short Coll, Type;

public:
	bool Start();
	bool HandleAttribute(const char *Tag, const char *Value);
	bool AttributesDone();
		
	XML_DeadAssignParser(): XML_ElementParser("assign_dead") {}
};

bool XML_DeadAssignParser::Start()
{
	for (int k=0; k<2; k++)
		IsPresent[k] = false;
	return true;
}

bool XML_DeadAssignParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (strcmp(Tag,"coll") == 0)
	{
		if (ReadBoundedNumericalValue(Value,"%hd",Coll,short(0),short(NUMBER_OF_COLLECTIONS-1)))
		{
			IsPresent[0] = true;
			return true;
		}
		else return false;
	}
	else if (strcmp(Tag,"type") == 0)
	{
		// The permissible values, -1, 0, and 1, are
		// NONE
		// _civilian_thing
		// _monster_thing
		if (ReadBoundedNumericalValue(Value,"%hd",Type,short(-1),short(1)))
		{
			IsPresent[1] = true;
			return true;
		}
		else return false;
	}
	UnrecognizedTag();
	return false;
}

bool XML_DeadAssignParser::AttributesDone()
{
	// Verify...
	bool AllPresent = IsPresent[0] && IsPresent[1];
	
	if (!AllPresent)
	{
		AttribsMissing();
		return false;
	}
		
	OvhdMap_ConfigData.dead_monster_displays[Coll] = Type;
	
	return true;
}

static XML_DeadAssignParser DeadAssignParser;


// Boolean-attribute parser: for switching stuff on and off
class XML_OvhdMapBooleanParser: public XML_ElementParser
{
	bool IsPresent;

public:
	bool Start();
	bool HandleAttribute(const char *Tag, const char *Value);
	bool AttributesDone();
	
	// Out here so it can be changed
	short IsOn;
	// Whether this element got used or not
	short GotUsed;
		
	XML_OvhdMapBooleanParser(const char *_Name): XML_ElementParser(_Name) {}
};

bool XML_OvhdMapBooleanParser::Start()
{
	IsPresent = false;
	return true;
}

bool XML_OvhdMapBooleanParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (strcmp(Tag,"on") == 0)
	{
		if (ReadBooleanValue(Value,IsOn))
		{
			IsPresent = true;
			return true;
		}
		else return false;
	}
	UnrecognizedTag();
	return false;
}

bool XML_OvhdMapBooleanParser::AttributesDone()
{
	// Verify...
	if (!IsPresent)
	{
		AttribsMissing();
		return false;
	}
	GotUsed = true;
	return true;
}

static XML_OvhdMapBooleanParser
	ShowAliensParser("aliens"),
	ShowItemsParser("items"),
	ShowProjectilesParser("projectiles"),
	ShowPathsParser("paths");
	

// Subclassed to set the color objects appropriately
const int TOTAL_NUMBER_OF_COLORS =
	NUMBER_OF_POLYGON_COLORS + NUMBER_OF_LINE_DEFINITIONS +
	NUMBER_OF_THINGS + + NUMBER_OF_ANNOTATION_DEFINITIONS + 2;

class XML_OvhdMapParser: public XML_ElementParser
{
	// Extras are:
	// annotation color
	// map-title color
	// path color
	rgb_color Colors[2*TOTAL_NUMBER_OF_COLORS];
	
public:
	bool Start();
	bool HandleAttribute(const char *Tag, const char *Value);
	bool End();
	
	XML_OvhdMapParser(): XML_ElementParser("overhead_map") {}
};

bool XML_OvhdMapParser::Start()
{
	ShowAliensParser.GotUsed = false;
	ShowItemsParser.GotUsed = false;
	ShowProjectilesParser.GotUsed = false;
	ShowPathsParser.GotUsed = false;
	
	// Copy in the colors
	rgb_color *ColorPtr = Colors;
	
	rgb_color *PolyColorPtr = OvhdMap_ConfigData.polygon_colors;
	for (int k=0; k<NUMBER_OF_POLYGON_COLORS; k++)
		*(ColorPtr++) = *(PolyColorPtr++);

	line_definition *LineDefPtr = OvhdMap_ConfigData.line_definitions;
	for (int k=0; k<NUMBER_OF_LINE_DEFINITIONS; k++)
		*(ColorPtr++) = (LineDefPtr++)->color;
	
	thing_definition *ThingDefPtr = OvhdMap_ConfigData.thing_definitions;
	for (int k=0; k<NUMBER_OF_THINGS; k++)
		*(ColorPtr++) = (ThingDefPtr++)->color;
	

	annotation_definition *NoteDefPtr = OvhdMap_ConfigData.annotation_definitions;
	for (int k=0; k<NUMBER_OF_ANNOTATION_DEFINITIONS; k++)
		*(ColorPtr++) = (NoteDefPtr++)->color;
	
	*(ColorPtr++) = OvhdMap_ConfigData.map_name_data.color;
	*(ColorPtr++) = OvhdMap_ConfigData.path_color;
	
	Color_SetArray(Colors,TOTAL_NUMBER_OF_COLORS);
	
	return true;
}

bool XML_OvhdMapParser::HandleAttribute(const char *Tag, const char *Value)
{
	UnrecognizedTag();
	return false;
}

bool XML_OvhdMapParser::End()
{
	if (ShowAliensParser.GotUsed)
		OvhdMap_ConfigData.ShowAliens = (ShowAliensParser.IsOn != 0);
	if (ShowItemsParser.GotUsed)
		OvhdMap_ConfigData.ShowItems = (ShowItemsParser.IsOn != 0);
	if (ShowProjectilesParser.GotUsed)
		OvhdMap_ConfigData.ShowProjectiles = (ShowProjectilesParser.IsOn != 0);
	if (ShowPathsParser.GotUsed)
		OvhdMap_ConfigData.ShowPaths = (ShowPathsParser.IsOn != 0);
		
	// Copy out the colors
	rgb_color *ColorPtr = Colors;
	
	rgb_color *PolyColorPtr = OvhdMap_ConfigData.polygon_colors;
	for (int k=0; k<NUMBER_OF_POLYGON_COLORS; k++)
		*(PolyColorPtr++) = *(ColorPtr++);

	line_definition *LineDefPtr = OvhdMap_ConfigData.line_definitions;
	for (int k=0; k<NUMBER_OF_LINE_DEFINITIONS; k++)
		(LineDefPtr++)->color = *(ColorPtr++);

	thing_definition *ThingDefPtr = OvhdMap_ConfigData.thing_definitions;
	for (int k=0; k<NUMBER_OF_THINGS; k++)
		(ThingDefPtr++)->color = *(ColorPtr++);

	annotation_definition *NoteDefPtr = OvhdMap_ConfigData.annotation_definitions;
	for (int k=0; k<NUMBER_OF_ANNOTATION_DEFINITIONS; k++)
		(NoteDefPtr++)->color = *(ColorPtr++);
	
	OvhdMap_ConfigData.map_name_data.color = *(ColorPtr++);
	OvhdMap_ConfigData.path_color = *(ColorPtr++);
		
	return true;
}

static XML_OvhdMapParser OvhdMapParser;


// LP change: added infravision-parser export
XML_ElementParser *OverheadMap_GetParser()
{
	OvhdMapParser.AddChild(&LiveAssignParser);
	OvhdMapParser.AddChild(&DeadAssignParser);
	OvhdMapParser.AddChild(&ShowAliensParser);
	OvhdMapParser.AddChild(&ShowItemsParser);
	OvhdMapParser.AddChild(&ShowProjectilesParser);
	OvhdMapParser.AddChild(&ShowPathsParser);
	OvhdMapParser.AddChild(Color_GetParser());
	
	return &OvhdMapParser;
}
