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
*/

#include "macintosh_cseries.h"

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
	NUMBER_OF_POLYGON_COLORS
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

enum /* render flags */
{
	_endpoint_on_automap= 0x2000,
	_line_on_automap= 0x4000,
	_polygon_on_automap= 0x8000
};


// LP change: creating an array describing the type assignments
// of the various monsters, so as to allow better customization

static short MonsterDisplays[NUMBER_OF_MONSTER_TYPES] =
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
};

// LP change: This is for objects that have been turned into garbage
// and that are waiting for the garbage collector 

static short DeadMonsterDisplays[NUMBER_OF_COLLECTIONS] =
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
};

// LP addition: what extra to show in the map
static bool ShowAliens = false;
static bool ShowItems = false;
static bool ShowProjectiles = false;
static bool ShowPaths = false;


/* ---------- macros */

#define WORLD_TO_SCREEN_SCALE_ONE 8
#define WORLD_TO_SCREEN(x, x0, scale) (((x)-(x0))>>(WORLD_TO_SCREEN_SCALE_ONE-(scale)))

/* ---------- private code */

// Changed to link properly with code in pathfinding.c
extern world_point2d *path_peek(short path_index, short *step_count);
extern short GetNumberOfPaths();

static void transform_endpoints_for_overhead_map(struct overhead_map_data *data);

static void generate_false_automap(short polygon_index);
static long false_automap_cost_proc(short source_polygon_index, short line_index, short destination_polygon_index, void *caller_data);
static void replace_real_automap(void);

static void draw_overhead_polygon(short vertex_count, short *vertices, short color, short scale);
static void draw_overhead_line(short line_index, short color, short scale);
static void draw_overhead_player(world_point2d *center, angle facing, short color, short scale);
static void draw_overhead_thing(world_point2d *center, angle facing, short color, short scale);
static void draw_overhead_annotation(world_point2d *location, short color, char *text, short scale);
static void draw_map_name(struct overhead_map_data *data, char *name);

/* ---------- machine-specific code */

#ifdef mac
#include "overhead_map_macintosh.c"
#endif

/* ---------- code */

void _render_overhead_map(
	struct overhead_map_data *data)
{
	world_distance x0= data->origin.x, y0= data->origin.y;
	short scale= data->scale;
	world_point2d location;
	short i;
	
	// LP addition: stuff for setting the game options, since they get defaulted to 0
	SET_FLAG(GET_GAME_OPTIONS(),_overhead_map_shows_monsters,ShowAliens);
	SET_FLAG(GET_GAME_OPTIONS(),_overhead_map_shows_items,ShowItems);
	SET_FLAG(GET_GAME_OPTIONS(),_overhead_map_shows_projectiles,ShowProjectiles);
	
	if (data->mode==_rendering_checkpoint_map) generate_false_automap(data->origin_polygon_index);
	
	transform_endpoints_for_overhead_map(data);

#if 0	
	RgnHandle old_clip, new_clip;
	Rect bounds;

	old_clip= NewRgn();
	new_clip= NewRgn();
	assert(old_clip && new_clip);
	
	GetClip(old_clip);
	SetRect(&bounds, data->left, data->top, data->left+data->width, data->top+data->height);
	RectRgn(new_clip, &bounds);
	SetClip(new_clip);
#endif
	
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
					case _polygon_is_platform: color= PLATFORM_IS_SECRET(get_platform_data(polygon->permutation)) ? _polygon_color : _polygon_platform_color; break;
					
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
								// default:
								// LP change:
								// Do nothing
								// assert(false);
								// halt();
							}
						}
					}
				}
				
				draw_overhead_polygon(polygon->vertex_count, polygon->endpoint_indexes, color, scale);

#ifdef RENDER_DEBUG
				find_center_of_polygon(i, &polygon->center);
				location.x= data->half_width + WORLD_TO_SCREEN(polygon->center.x, x0, scale);
				location.y= data->half_height + WORLD_TO_SCREEN(polygon->center.y, y0, scale);
				TextFont(kFontIDMonaco);
				TextFace(normal);
				TextSize(9);
				RGBForeColor(&rgb_white);
				psprintf(ptemporary, "%d", i);
				MoveTo(location.x, location.y);
				DrawString(temporary);
#endif
			}
		}
	}
	
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
						line_color= LINE_IS_LANDSCAPED(line) ? NONE : _elevation_line_color;
					}
				}
			}
			
			if (line_color!=NONE) draw_overhead_line(i, line_color, scale);
		}
	}
	
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
				location.x= data->half_width + WORLD_TO_SCREEN(annotation->location.x, x0, scale);
				location.y= data->half_height + WORLD_TO_SCREEN(annotation->location.y, y0, scale);
				
				draw_overhead_annotation(&location, annotation->type, annotation->text, scale);
			}
		}
	}

	if (ShowPaths)
	{
		short path_index;
		
		// LP change: made this more general
		SetPathDrawing();
		// PenSize(1, 1);
		// RGBForeColor(&rgb_white);
		
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
					location.x= data->half_width + WORLD_TO_SCREEN(points[step].x, x0, scale);
					location.y= data->half_height + WORLD_TO_SCREEN(points[step].y, y0, scale);
					// LP change: made this more general
					DrawPath(step,location);
					// step ? LineTo(location.x, location.y) : MoveTo(location.x, location.y);
				}
			}
		}
	}

#ifdef RENDER_DEBUG
	RGBForeColor(&rgb_white);
	PenSize(1, 1);
	
	MoveTo(data->half_width, data->half_height);
	LineTo(data->half_width+world_view->left_edge.i, data->half_height+world_view->left_edge.j);
	
	MoveTo(data->half_width, data->half_height);
	LineTo(data->half_width+world_view->right_edge.i, data->half_height+world_view->right_edge.j);
#endif

	if (data->mode!=_rendering_checkpoint_map)	
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
									location.x= data->half_width + WORLD_TO_SCREEN(object->location.x, x0, scale);
									location.y= data->half_height + WORLD_TO_SCREEN(object->location.y, y0, scale);
									
									draw_overhead_player(&location, object->facing, player->team, scale);
								}
							}
							else
							{
								// LP: use the lookup system
								switch (MonsterDisplays[monster->type])
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
								switch (monster->type)
								{
									case _civilian_crew:
									case _civilian_science:
									case _civilian_security:
									case _civilian_assimilated:
									// LP additions: the VacBobs
									case _civilian_fusion_crew:
									case _civilian_fusion_science:
									case _civilian_fusion_security:
									case _civilian_fusion_assimilated:
										thing_type= _civilian_thing;
										break;
									
									default:
										if (GET_GAME_OPTIONS()&_overhead_map_shows_monsters)
										{
											thing_type= _monster_thing;
										}
										break;
								}
								*/
							}
							break;
						}
	
						case _object_is_projectile:
							if ((GET_GAME_OPTIONS()&_overhead_map_shows_projectiles) && object->shape!=NONE)
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
							switch (DeadMonsterDisplays[GET_COLLECTION(GET_DESCRIPTOR_COLLECTION(object->shape))])
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
							location.x= data->half_width + WORLD_TO_SCREEN(object->location.x, x0, scale);
							location.y= data->half_height + WORLD_TO_SCREEN(object->location.y, y0, scale);
							
							draw_overhead_thing(&location, object->facing, thing_type, scale);
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
				saved_object->location.x==data->origin.x && saved_object->location.y==data->origin.y)
			{
				location.x= data->half_width + WORLD_TO_SCREEN(saved_object->location.x, x0, scale);
				location.y= data->half_height + WORLD_TO_SCREEN(saved_object->location.y, y0, scale);
				draw_overhead_thing(&location, 0, _checkpoint_thing, scale);
			}
		}
	}

	if (data->mode==_rendering_game_map) draw_map_name(data, static_world->level_name);
	if (data->mode==_rendering_checkpoint_map) replace_real_automap();

#if 0
	SetClip(old_clip);
	DisposeRgn(old_clip);
	DisposeRgn(new_clip);
#endif

	return;
}

/* ---------- private code */

static void transform_endpoints_for_overhead_map(
	struct overhead_map_data *data)
{
	world_distance x0= data->origin.x, y0= data->origin.y;
	short screen_width= data->width, screen_height= data->height;
	short scale= data->scale;
	short i;

#ifdef OBSOLETE
	/* find the bounds of our screen in world space */
	left= ((-data->half_width)<<(WORLD_TO_SCREEN_SCALE_ONE-scale)) + x0;
	right= ((data->half_width)<<(WORLD_TO_SCREEN_SCALE_ONE-scale)) + x0;
	top= ((-data->half_height)<<(WORLD_TO_SCREEN_SCALE_ONE-scale)) + y0;
	bottom= ((data->half_height)<<(WORLD_TO_SCREEN_SCALE_ONE-scale)) + y0;
#endif
	
	/* transform all our endpoints into screen space, remembering which ones are visible */
	for (i=0;i<dynamic_world->endpoint_count;++i)
	{
		struct endpoint_data *endpoint= get_endpoint_data(i);
		
		endpoint->transformed.x= data->half_width + WORLD_TO_SCREEN(endpoint->vertex.x, x0, scale);
		endpoint->transformed.y= data->half_height + WORLD_TO_SCREEN(endpoint->vertex.y, y0, scale);

		if (endpoint->transformed.x>=0&&endpoint->transformed.y>=0&&endpoint->transformed.y<=screen_height&&endpoint->transformed.x<=screen_width)
		{
			SET_STATE_FLAG(i, _endpoint_on_automap, TRUE);
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
				SET_STATE_FLAG(i, _polygon_on_automap, TRUE);
				break;
			}
		}
	}

	return;
}

/* --------- the false automap */

static byte *saved_automap_lines, *saved_automap_polygons;

static void generate_false_automap(
	short polygon_index)
{
	long automap_line_buffer_size, automap_polygon_buffer_size;

	automap_line_buffer_size= (dynamic_world->line_count/8+((dynamic_world->line_count%8)?1:0))*sizeof(byte);
	automap_polygon_buffer_size= (dynamic_world->polygon_count/8+((dynamic_world->polygon_count%8)?1:0))*sizeof(byte);

	/* allocate memory for the old automap memory */
	saved_automap_lines= (byte *) malloc(automap_line_buffer_size);
	saved_automap_polygons= (byte *) malloc(automap_polygon_buffer_size);

	if (saved_automap_lines && saved_automap_polygons)
	{
		memcpy(saved_automap_lines, automap_lines, automap_line_buffer_size);
		memcpy(saved_automap_polygons, automap_polygons, automap_polygon_buffer_size);
		memset(automap_lines, 0, automap_line_buffer_size);
		memset(automap_polygons, 0, automap_polygon_buffer_size);
		
		polygon_index= flood_map(polygon_index, LONG_MAX, false_automap_cost_proc, _breadth_first, (void *) NULL);
		do
		{
			polygon_index= flood_map(NONE, LONG_MAX, false_automap_cost_proc, _breadth_first, (void *) NULL);
		}
		while (polygon_index!=NONE);
	}
	
	return;
}

static void replace_real_automap(
	void)
{
	if (saved_automap_lines)
	{
		long automap_line_buffer_size= (dynamic_world->line_count/8+((dynamic_world->line_count%8)?1:0))*sizeof(byte);
		memcpy(automap_lines, saved_automap_lines, automap_line_buffer_size);
		free(saved_automap_lines);
		saved_automap_lines= (byte *) NULL;
	}
	
	if (saved_automap_polygons)
	{
		long automap_polygon_buffer_size= (dynamic_world->polygon_count/8+((dynamic_world->polygon_count%8)?1:0))*sizeof(byte);
	
		memcpy(automap_polygons, saved_automap_polygons, automap_polygon_buffer_size);
		free(saved_automap_polygons);
		saved_automap_polygons= (byte *) NULL;
	}
	
	return;
}

static long false_automap_cost_proc(
	short source_polygon_index,
	short line_index,
	short destination_polygon_index,
	void *caller_data)
{
	struct polygon_data *destination_polygon= get_polygon_data(destination_polygon_index);
	struct polygon_data *source_polygon= get_polygon_data(source_polygon_index);
	long cost= 1;
	short i;

	(void) (line_index, caller_data);
	
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

	/* add the source polygon and all its lines to the automap */	
	for (i= 0; i<source_polygon->vertex_count; ++i) ADD_LINE_TO_AUTOMAP(source_polygon->line_indexes[i]);
	ADD_POLYGON_TO_AUTOMAP(source_polygon_index);
	
	return cost;
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
	MonsterDisplays[Monster] = Type;
		
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
		
	DeadMonsterDisplays[Coll] = Type;
	
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
const int TOTAL_NUMBER_OF_COLORS = NUMBER_OF_POLYGON_COLORS + NUMBER_OF_LINE_DEFINITIONS + NUMBER_OF_THINGS + 3;
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
	
	RGBColor *PolyColorPtr = polygon_colors;
	for (int k=0; k<NUMBER_OF_POLYGON_COLORS; k++)
		GetParserColor(*(PolyColorPtr++),*(ColorPtr++));

	line_definition *LineDefPtr = line_definitions;
	for (int k=0; k<NUMBER_OF_LINE_DEFINITIONS; k++)
		GetParserColor((LineDefPtr++)->color,*(ColorPtr++));

	thing_definition *ThingDefPtr = thing_definitions;
	for (int k=0; k<NUMBER_OF_THINGS; k++)
		GetParserColor((ThingDefPtr++)->color,*(ColorPtr++));

	GetParserColor(annotation_definitions[0].color,*(ColorPtr++));
	GetParserColor(map_name_color,*(ColorPtr++));
	GetParserColor(PathColor,*(ColorPtr++));
	
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
		ShowAliens = (ShowAliensParser.IsOn != 0);
	if (ShowItemsParser.GotUsed)
		ShowItems = (ShowItemsParser.IsOn != 0);
	if (ShowProjectilesParser.GotUsed)
		ShowProjectiles = (ShowProjectilesParser.IsOn != 0);
	if (ShowPathsParser.GotUsed)
		ShowPaths = (ShowPathsParser.IsOn != 0);
		
	// Copy out the colors
	rgb_color *ColorPtr = Colors;
	
	RGBColor *PolyColorPtr = polygon_colors;
	for (int k=0; k<NUMBER_OF_POLYGON_COLORS; k++)
		PutParserColor(*(ColorPtr++),*(PolyColorPtr++));

	line_definition *LineDefPtr = line_definitions;
	for (int k=0; k<NUMBER_OF_LINE_DEFINITIONS; k++)
		PutParserColor(*(ColorPtr++),(LineDefPtr++)->color);

	thing_definition *ThingDefPtr = thing_definitions;
	for (int k=0; k<NUMBER_OF_THINGS; k++)
		PutParserColor(*(ColorPtr++),(ThingDefPtr++)->color);

	PutParserColor(*(ColorPtr++),annotation_definitions[0].color);
	PutParserColor(*(ColorPtr++),map_name_color);
	PutParserColor(*(ColorPtr++),PathColor);
		
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
