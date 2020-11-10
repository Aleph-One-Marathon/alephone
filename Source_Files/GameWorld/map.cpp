/*
MAP.C

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

Sunday, August 15, 1993 12:13:52 PM

Tuesday, December 7, 1993 9:35:13 AM
	fixed bug in map_index_to_map_point (removed bitwise exclusive-ors).
Sunday, January 2, 1994 10:53:34 PM
	transmogrify_object_shape plays sounds now.
Wednesday, March 9, 1994 4:34:40 PM
	support for lightsourcing/mapping floor ceiling with polygons.
Monday, June 27, 1994 6:52:10 PM
	ajr--push_out_line now takes the length of the line instead of calculating it.
Friday, December 9, 1994 1:31:09 PM  (Jason)
	translate_map_object moves objects leaving the map into the center of their polygon.
Friday, June 9, 1995 2:25:33 PM  (Jason)
	sounds on the other side of a media boundary are obstructed
Monday, September 18, 1995 4:38:30 PM  (Jason)
	the old sound_index is now the landscape_index for a given level

Jan 30, 2000 (Loren Petrich):
	Added some typecasts

Feb 4, 2000 (Loren Petrich):
	Renamed the "pathways/marathon" environment
	Changed halt() to assert(false) for better debugging

Feb 13, 2000 (Loren Petrich):
	Added some idiot-proofing to the tick count in animate_object().

Feb 15, 2000 (Loren Petrich):
	Suppressed some assertions designed to check for map consistency;
	this is to get around some Pfhorte bugs.

Feb 17, 2000 (Loren Petrich):
	Fixed stuff near arctangent() to be long-distance-friendly

Feb 20, 2000 (Loren Petrich): Suppressed height-consistency check in
	change_polygon_height().

Apr 28, 2000 (Loren Petrich): In animate_object(), switched the two tests
	on the current frame so that the Pfhor can teleport out in
	M2's "Charon Doesn't Make Change".

Jul 6, 2000 (Loren Petrich): Readjusted the frame checking yet again, so that both keyframe = 0
	and keyframe = [number of frames] would be detected.

Jul 7, 2000 (Loren Petrich): Did yet another frame-checking readjustment, in order to suppress
	the reactivated Hunter soft-death bug.

Aug 20, 2000 (Loren Petrich): eliminated a "pause()" statement -- some debugging statement?
	
Oct 13, 2000 (Loren Petrich):
	Converted the intersected-objects list into a Standard Template Library vector

Oct 19, 2000 (Loren Petrich):
	Changed get_object_shape_and_transfer_mode() so that it makes data->collection_code equal to NONE
	if it does not find a valid sequence or view.
	
Nov 19, 2000 (Loren Petrich):
	Added XML support for texture-loading control. This contains a switch to indicate whether to load
	the landscape textures, and also stuff for loading the various texture environments.
	Each one of these has slots for several collection ID's to load; one can use a converted M1 map
	directly with this approach.

Feb 8, 2001 (Loren Petrich):
	Had not too long ago changed many of the arrays into dynamically-allocated ones, thus ending the
	limits on the numbers of points, lines, polygons, etc.
	Fixed a *serious* bug in the calculation of the "dynamic world" quantities in recalculate_map_counts() --
	there are some count-down loops, but they ought to count down to the last used entity, not the last unused one.

Feb 3, 2003 (Loren Petrich):
	In attach_parasitic_object(), will transmit the sizing of the host object to the parasite.

 June 14, 2003 (Woody Zenfell):
	New functions for manipulating polygons' object lists (in support of prediction).
 */

/*
find_line_crossed leaving polygon could be sped up considerable by reversing the search direction in some circumstances

//find_line_crossed_leaving_polygon() does weird things when walking along a gridline
//keep_line_segment_out_of_walls() can slide the player slowly along a wall
*/

#include "cseries.h"
#include "map.h"
#include "FilmProfile.h"
#include "interface.h"
#include "monsters.h"
#include "preferences.h"
#include "projectiles.h"
#include "effects.h"
#include "player.h"
#include "platforms.h"
#include "lightsource.h"
#include "lua_script.h"
#include "media.h"
#include "scenery.h"
#include "SoundManager.h"
#include "Console.h"
#include "InfoTree.h"
#include "flood_map.h"

#include <string.h>
#include <stdlib.h>
#include <limits.h>

#include <list>

/* ---------- structures */

/*
struct environment_definition
{
	short *shape_collections; *//* NONE terminated *//*
};
*/

/* ---------- constants */

#define DEFAULT_MAP_MEMORY_SIZE (128*KILO)

/* ---------- globals */

// LP: modified texture-environment management so as to be easier to handle with XML

const int NUMBER_OF_ENVIRONMENTS = 5;
const int NUMBER_OF_ENV_COLLECTIONS = 7;

static short Environments[NUMBER_OF_ENVIRONMENTS][NUMBER_OF_ENV_COLLECTIONS] = 
{
	{_collection_walls1, _collection_scenery1, NONE, NONE, NONE, NONE, NONE},	// Lh'owon Water
	{_collection_walls2, _collection_scenery2, NONE, NONE, NONE, NONE, NONE},	// Lh'owon Lava
	{_collection_walls3, _collection_scenery3, NONE, NONE, NONE, NONE, NONE},	// Lh'owon Sewage
	{_collection_walls4, _collection_scenery4, NONE, NONE, NONE, NONE, NONE},	// Jjaro (originally to be Pathways or Marathon)
	{_collection_walls5, _collection_scenery5, NONE, NONE, NONE, NONE, NONE}	// Pfhor
};

/*
static short e1[]= {_collection_walls1, _collection_scenery1, NONE}; // lh’owon
static short e2[]= {_collection_walls2, _collection_scenery2, NONE}; // lh’owon
static short e3[]= {_collection_walls3, _collection_scenery3, NONE}; // lh’owon
static short e4[]= {_collection_walls4, _collection_scenery4, NONE}; // pathways (or marathon) (LP: jjaro)
static short e5[]= {_collection_walls5, _collection_scenery5, NONE}; // pfhor ship

#define NUMBER_OF_ENVIRONMENTS (sizeof(environment_definitions)/sizeof(struct environment_definition))
static struct environment_definition environment_definitions[]=
{
	{e1},
	{e2},
	{e3},
	{e4},
	{e5}
};
*/

/* ---------- map globals */

// Turned some of these lists into variable arrays;
// took over their maximum numbers as how many of them

struct static_data *static_world = NULL;
struct dynamic_data *dynamic_world = NULL;

// These are allocated here because the numbers of these objects vary as a game progresses.
vector<effect_data> EffectList(MAXIMUM_EFFECTS_PER_MAP);
vector<object_data> ObjectList(MAXIMUM_OBJECTS_PER_MAP);
vector<monster_data> MonsterList(MAXIMUM_MONSTERS_PER_MAP);
vector<projectile_data> ProjectileList(MAXIMUM_PROJECTILES_PER_MAP);
// struct object_data *objects = NULL;
// struct monster_data *monsters = NULL;
// struct projectile_data *projectiles = NULL;

vector<endpoint_data> EndpointList;
vector<line_data> LineList;
vector<side_data> SideList;
vector<polygon_data> PolygonList;
vector<platform_data> PlatformList;
// struct polygon_data *map_polygons = NULL;
// struct side_data *map_sides = NULL;
// struct line_data *map_lines = NULL;
// struct endpoint_data *map_endpoints = NULL;
// struct platform_data *platforms = NULL;

vector<ambient_sound_image_data> AmbientSoundImageList;
vector<random_sound_image_data> RandomSoundImageList;
// struct ambient_sound_image_data *ambient_sound_images = NULL;
// struct random_sound_image_data *random_sound_images = NULL;

vector<int16> MapIndexList;
// short *map_indexes = NULL;

vector<uint8> AutomapLineList;
vector<uint8> AutomapPolygonList;
// byte *automap_lines = NULL;
// byte *automap_polygons = NULL;

vector<map_annotation> MapAnnotationList;
// struct map_annotation *map_annotations = NULL;

vector<map_object> SavedObjectList;
// struct map_object *saved_objects = NULL;
struct item_placement_data *placement_information = NULL;

bool game_is_networked = false;

// This could be a handle
struct map_memory_data {
	byte *memory;
	int32 size;
	int32 index;
};

// static struct map_memory_data map_structure_memory;

// LP addition: growable list of intersected objects
static vector<short> IntersectedObjects;

// Whether or not Marathon 2/oo landscapes had been loaded (switch off for Marathon 1 compatibility)
bool LandscapesLoaded = true;

// The index number of the first texture loaded (should be the main wall texture);
// needed for infravision fog when landscapes are switched off
short LoadedWallTexture = NONE;

/* ---------- private prototypes */

static short _new_map_object(shape_descriptor shape, angle facing);

// ZZZ: factored out some functionality for prediction, but ended up not using this stuff,
// so am not "publishing" it via map.h yet.
// SB: Blah.
void remove_object_from_polygon_object_list(short object_index, short polygon_index);
// The second infers the polygon_index from the object's "polygon" member field.
void add_object_to_polygon_object_list(short object_index, short polygon_index);
inline void add_object_to_polygon_object_list(short object_index)
{ add_object_to_polygon_object_list(object_index, get_object_data(object_index)->polygon); }

short _find_line_crossed_leaving_polygon(short polygon_index, world_point2d *p0, world_point2d *p1, bool *last_line);

/* ---------- code */

// Accessors moved here to shrink the code

object_data *get_object_data(
	const short object_index)
{
	struct object_data *object = GetMemberWithBounds(objects,object_index,MAXIMUM_OBJECTS_PER_MAP);
	
	vassert(object, csprintf(temporary, "object index #%d is out of range", object_index));
	vassert(SLOT_IS_USED(object), csprintf(temporary, "object index #%d is unused", object_index));
	
	return object;
}

polygon_data *get_polygon_data(
	const short polygon_index)
{
	assert(map_polygons);	
	struct polygon_data *polygon = GetMemberWithBounds(map_polygons,polygon_index,dynamic_world->polygon_count);
	
	vassert(polygon, csprintf(temporary, "polygon index #%d is out of range", polygon_index));
	
	return polygon;
}

line_data *get_line_data(
	const short line_index)
{
	assert(map_lines);
	struct line_data *line = GetMemberWithBounds(map_lines,line_index,dynamic_world->line_count);
	
	vassert(line, csprintf(temporary, "line index #%d is out of range", line_index));
	
	return line;
}

side_data *get_side_data(
	const short side_index)
{
	assert(map_sides);
	struct side_data *side = GetMemberWithBounds(map_sides,side_index,dynamic_world->side_count);
	
	vassert(side, csprintf(temporary, "side index #%d is out of range", side_index));
	
	return side;
}

endpoint_data *get_endpoint_data(
	const short endpoint_index)
{
	assert(map_endpoints);
	struct endpoint_data *endpoint = GetMemberWithBounds(map_endpoints,endpoint_index,dynamic_world->endpoint_count);

	vassert(endpoint, csprintf(temporary, "endpoint index #%d is out of range", endpoint_index));
	
	return endpoint;
}

short *get_map_indexes(
	const short index,
	const short count)
{
	assert(map_indexes);
	short *map_index = GetMemberWithBounds(map_indexes,static_cast<unsigned short>(index),static_cast<unsigned short>(dynamic_world->map_index_count)-count+1);
	
	// vassert(map_index, csprintf(temporary, "map_indexes(#%d,#%d) are out of range", index, count));
	
	return map_index;
}

ambient_sound_image_data *get_ambient_sound_image_data(
	const short ambient_sound_image_index)
{
	return GetMemberWithBounds(ambient_sound_images,ambient_sound_image_index,dynamic_world->ambient_sound_image_count);
}

random_sound_image_data *get_random_sound_image_data(
	const short random_sound_image_index)
{
	return GetMemberWithBounds(random_sound_images,random_sound_image_index,dynamic_world->random_sound_image_count);
}

void allocate_map_memory(
	void)
{
	assert(NUMBER_OF_COLLECTIONS<=MAXIMUM_COLLECTIONS);
	
	static_world= new static_data;
	dynamic_world= new dynamic_data;
	assert(static_world&&dynamic_world);
	obj_clear(*static_world);
	obj_clear(*dynamic_world);

	// monsters= new monster_data[MAXIMUM_MONSTERS_PER_MAP];
	// projectiles= new projectile_data[MAXIMUM_PROJECTILES_PER_MAP];
	// objects= new object_data[MAXIMUM_OBJECTS_PER_MAP];
	// effects= new effect_data[MAXIMUM_EFFECTS_PER_MAP];
	// lights= new light_data[MAXIMUM_LIGHTS_PER_MAP];
	// medias= new media_data[MAXIMUM_MEDIAS_PER_MAP];
	// assert(objects&&monsters&&effects&&projectiles&&lights&&medias);

	// obj_clear(map_structure_memory);
	// reallocate_map_structure_memory(DEFAULT_MAP_MEMORY_SIZE);
	
	// platforms= new platform_data[MAXIMUM_PLATFORMS_PER_MAP];
	// assert(platforms);

	// ambient_sound_images= new ambient_sound_image_data[MAXIMUM_AMBIENT_SOUND_IMAGES_PER_MAP];
	// random_sound_images= new random_sound_image_data[MAXIMUM_RANDOM_SOUND_IMAGES_PER_MAP];
	// assert(ambient_sound_images && random_sound_images);
	
	// map_annotations= new map_annotation[MAXIMUM_ANNOTATIONS_PER_MAP];
	// saved_objects= new map_object[MAXIMUM_SAVED_OBJECTS];
	// assert(map_annotations && saved_objects);

	allocate_player_memory();
}

void initialize_map_for_new_game(
	void)
{
	obj_clear(*dynamic_world);

	initialize_players();
	initialize_monsters();
}

void initialize_map_for_new_level(
	void)
{
	short total_civilians, total_causalties;
	uint32 tick_count;
	uint16 random_seed;
	short player_count;
	struct game_data game_information;

	/* The player count, tick count, and random seed must persist.. */
	/* And the game information! (ajr) */
	player_count= dynamic_world->player_count;
	tick_count= dynamic_world->tick_count;
	random_seed= dynamic_world->random_seed;
	total_civilians= dynamic_world->total_civilian_count + dynamic_world->current_civilian_count;
	total_causalties= dynamic_world->total_civilian_causalties + dynamic_world->current_civilian_causalties;
	game_information= dynamic_world->game_information;
	obj_clear(*dynamic_world);
	dynamic_world->game_information= game_information;
	dynamic_world->player_count= player_count;
	dynamic_world->tick_count= tick_count;
	dynamic_world->random_seed= random_seed;
	dynamic_world->total_civilian_count= total_civilians;
	dynamic_world->total_civilian_causalties= total_causalties;
	dynamic_world->speaking_player_index= NONE;
	dynamic_world->garbage_object_count= 0;

	obj_clear(*static_world);
	Console::instance()->clear_saves();
	
	// Clear all these out -- supposed to be none of the contents of these when starting a level.
	objlist_clear(automap_lines, AutomapLineList.size());
	objlist_clear(automap_polygons, AutomapPolygonList.size());
	objlist_clear(effects, EffectList.size());
	objlist_clear(projectiles,  ProjectileList.size());
	objlist_clear(monsters,  MonsterList.size());
	objlist_clear(objects,  ObjectList.size());

	/* Note that these pointers just point into a larger structure, so this is not a bad thing */
	// map_polygons= NULL;
	// map_sides= NULL;
	// map_lines= NULL;
	// map_endpoints= NULL;
	// automap_lines= NULL;
	// automap_polygons= NULL;
}

static bool map_collections[NUMBER_OF_COLLECTIONS];
static bool media_effects[NUMBER_OF_EFFECT_TYPES];

void mark_map_collections(bool loading)
{
	if (loading)
	{

		for (int collection = 0; collection < NUMBER_OF_COLLECTIONS; collection++)
		{
			map_collections[collection] = false;
		}

		// walls/floors/ceilings
		for (int n = 0; n < dynamic_world->polygon_count; n++)
		{
			polygon_data *polygon = map_polygons + n;
			int coll;
			coll = GET_DESCRIPTOR_COLLECTION(polygon->floor_texture);
			if (coll >= 0 && coll < NUMBER_OF_COLLECTIONS)
				map_collections[coll] = true;

			coll = GET_DESCRIPTOR_COLLECTION(polygon->ceiling_texture);
			if (coll >= 0 && coll < NUMBER_OF_COLLECTIONS)
				map_collections[coll] = true;
			
			for (int i = 0; i < polygon->vertex_count; i++)
			{
				short side_index = polygon->side_indexes[i];
				if (side_index == NONE) continue;
				side_data *side = get_side_data(side_index);
				switch (side->type)
				{
				case _full_side:
					coll = GET_DESCRIPTOR_COLLECTION(side->primary_texture.texture);
					if (coll >= 0 && coll < NUMBER_OF_COLLECTIONS)
						map_collections[coll] = true;
					break;
				case _split_side:
					coll = GET_DESCRIPTOR_COLLECTION(side->secondary_texture.texture);
					if (coll >= 0 && coll < NUMBER_OF_COLLECTIONS)
						map_collections[coll] = true;
					// fall through to the high side case
				case _high_side:
					coll = GET_DESCRIPTOR_COLLECTION(side->primary_texture.texture);
					if (coll >= 0 && coll < NUMBER_OF_COLLECTIONS)
						map_collections[coll] = true;
					break;
				case _low_side:
					coll = GET_DESCRIPTOR_COLLECTION(side->primary_texture.texture);
					if (coll >= 0 && coll < NUMBER_OF_COLLECTIONS)
						map_collections[coll] = true;
				}

				coll = GET_DESCRIPTOR_COLLECTION(side->transparent_texture.texture);
				if (coll >= 0 && coll < NUMBER_OF_COLLECTIONS)
					map_collections[coll] = true;
				
			}
		}

		// media textures and effects
		for (int media_effect = 0; media_effect < NUMBER_OF_EFFECT_TYPES; media_effect++)
		{
			media_effects[media_effect] = false;
		}

		for (int media_index = 0; media_index < MAXIMUM_MEDIAS_PER_MAP; ++media_index)
		{
			if (get_media_data(media_index))
			{
				short collection;
				if (get_media_collection(media_index, collection))
				{
					map_collections[collection] = true;
				}

				for (int detonation_type = 0; detonation_type < NUMBER_OF_MEDIA_DETONATION_TYPES; detonation_type++)
				{
					short detonation_effect;
					get_media_detonation_effect(media_index, detonation_type, &detonation_effect);
					if (detonation_effect >= 0 && detonation_effect < NUMBER_OF_EFFECT_TYPES)
						media_effects[detonation_effect] = true;
				}
			}
		}

		// scenery
		for (int object_index = 0; object_index < dynamic_world->initial_objects_count; object_index++)
		{
			if (saved_objects[object_index].type == _saved_object)
			{
				short collection;
				if (get_scenery_collection(saved_objects[object_index].index, collection))
				{
					map_collections[collection] = true;
				}

				if (get_damaged_scenery_collection(saved_objects[object_index].index, collection))
				{
					map_collections[collection] = true;
				}
			}
		}


		for (int collection = 0; collection < NUMBER_OF_COLLECTIONS; collection++)
		{
			if (map_collections[collection])
			{
				mark_collection_for_loading(collection);
			}
		}


		for (int media_effect = 0; media_effect < NUMBER_OF_EFFECT_TYPES; media_effect++)
		{
			if (media_effects[media_effect])
			{
				mark_effect_collections(media_effect, true);
			}
		}


	} else { // not loading
		for (int collection = 0; collection < NUMBER_OF_COLLECTIONS; collection++)
		{
			if (map_collections[collection])
			{
				mark_collection_for_unloading(collection);
			}
		}

		for (int media_effect = 0; media_effect < NUMBER_OF_EFFECT_TYPES; media_effect++)
		{
			mark_effect_collections(media_effect, false);
		}
		
	}
}

bool collection_in_environment(
	short collection_code,
	short environment_code)
{
	short collection_index= GET_COLLECTION(collection_code);
	bool found= false;
	int i;
	
	if (!(environment_code>=0 && environment_code<NUMBER_OF_ENVIRONMENTS)) return false;
	assert(collection_index>=0 && collection_index<NUMBER_OF_COLLECTIONS);
	
	for (i= 0; i<NUMBER_OF_ENV_COLLECTIONS; ++i)
	{
		if (Environments[environment_code][i]==collection_index) {
			found= true;
			break;
		}
	}
	
	return found;
}

/* mark all of the shape collections belonging to a given environment code for loading or
	unloading */
void mark_environment_collections(
	short environment_code,
	bool loading)
{
	short i;
	short collection;
	
	if (!(environment_code>=0&&environment_code<NUMBER_OF_ENVIRONMENTS)) return;

	// LP change: modified to use new collection-environment management;
	// be sure to set "loaded wall texture" to the first one loaded
	LoadedWallTexture = NONE;
	
	// for (i= 0; (collection= environment_definitions[environment_code].shape_collections[i])!=NONE; ++i)
	for (i= 0; i<NUMBER_OF_ENV_COLLECTIONS; ++i)
	{
		collection = Environments[environment_code][i];
		if (collection != NONE)
		{
			if (LoadedWallTexture == NONE) LoadedWallTexture = collection;
			loading ? mark_collection_for_loading(collection) : mark_collection_for_unloading(collection);
		}
	}
	if (LoadedWallTexture == NONE) LoadedWallTexture = 0;
	
	// Don't load/unload if M1 compatible...
	if (LandscapesLoaded)
		loading ? mark_collection_for_loading(_collection_landscape1+static_world->song_index) :
			mark_collection_for_unloading(_collection_landscape1+static_world->song_index);
}

/* make the object list and the map consistent */
void reconnect_map_object_list(
	void)
{
	short i;
	struct object_data *object;
	struct polygon_data *polygon;

	/* wipe first_object links from polygon structures */
	for (polygon=map_polygons,i=0;i<dynamic_world->polygon_count;--i,++polygon)
	{
		polygon->first_object= NONE;
	}
	
	/* connect objects to their polygons */
	for (object=objects,i=0;i<MAXIMUM_OBJECTS_PER_MAP;++i,++object)
	{
		if (SLOT_IS_USED(object))
		{
			polygon= get_polygon_data(object->polygon);
			
			object->next_object= polygon->first_object;
			polygon->first_object= i;
		}
	}
}

bool valid_point2d(
	world_point2d *p)
{
	return world_point_to_polygon_index(p)==NONE ? false : true;
}

bool valid_point3d(
	world_point3d *p)
{
	short polygon_index= world_point_to_polygon_index((world_point2d *)p);
	bool valid= false;
	
	if (polygon_index!=NONE)
	{
		struct polygon_data *polygon= get_polygon_data(polygon_index);
		
		if (p->z>polygon->floor_height&&p->z<polygon->ceiling_height)
		{
			valid= true;
		}
	}
	
	return valid;
}

short new_map_object(
	struct object_location *location,
	shape_descriptor shape)
{
	struct polygon_data *polygon= get_polygon_data(location->polygon_index);
	world_point3d p= location->p;
	short object_index;
	
	p.z= ((location->flags&_map_object_hanging_from_ceiling) ? polygon->ceiling_height : polygon->floor_height) + p.z;
	
	object_index= new_map_object3d(&p, location->polygon_index, shape, location->yaw);
	if (object_index!=NONE)
	{
		struct object_data *object= get_object_data(object_index);
		
		if (location->flags&_map_object_is_invisible)
			SET_OBJECT_INVISIBILITY(object, true);
	}
	
	return object_index;
}

short new_map_object2d(
	world_point2d *location,
	short polygon_index,
	shape_descriptor shape,
	angle facing)
{
	world_point3d location3d;
	struct polygon_data *polygon;

	polygon= get_polygon_data(polygon_index);
	location3d.x= location->x, location3d.y= location->y, location3d.z= polygon->floor_height;
	
	return new_map_object3d(&location3d, polygon_index, shape, facing);
}

short new_map_object3d(
	world_point3d *location,
	short polygon_index,
	shape_descriptor shape,
	angle facing)
{
	short object_index;

	object_index= _new_map_object(shape, facing);
	if (object_index!=NONE)
	{
		struct polygon_data *polygon= get_polygon_data(polygon_index);
		struct object_data *object= get_object_data(object_index);
	
		/* initialize object polygon and location */	
		object->polygon= polygon_index;
		object->location= *location;

		/* insert at head of linked list */
		object->next_object= polygon->first_object;
		polygon->first_object= object_index;
	}
	
	return object_index;
}

/* can be NONE if there is no direct route between the two points or the child point is not in any
	polygon */
short find_new_object_polygon(
	world_point2d *parent_location,
	world_point2d *child_location,
	short parent_polygon_index)
{
	short child_polygon_index= parent_polygon_index;
	
	if (child_polygon_index!=NONE)
	{
		short line_index;
		
		do
		{
			line_index= find_line_crossed_leaving_polygon(child_polygon_index, parent_location, child_location);
			if (line_index!=NONE)
			{
				child_polygon_index= find_adjacent_polygon(child_polygon_index, line_index);
			}
		}
		while (line_index!=NONE&&child_polygon_index!=NONE);
	}
	
	return child_polygon_index;
}

short attach_parasitic_object(
	short host_index,
	shape_descriptor shape,
	angle facing)
{
	struct object_data *host_object, *parasite_object;
	short parasite_index;

	/* walk this object’s parasite list until we find the last parasite and then attach there */
	for (host_object= get_object_data(host_index);
			host_object->parasitic_object!=NONE;
			host_index= host_object->parasitic_object, host_object= get_object_data(host_index))
		;
	parasite_index= _new_map_object(shape, facing);
	assert(parasite_index!=NONE);
	
	parasite_object= get_object_data(parasite_index);
	parasite_object->location= host_object->location;
	host_object->parasitic_object= parasite_index;
	
	// So that it will have the same size scaling as its host object
	SET_FLAG(parasite_object->flags, _object_is_enlarged, TEST_FLAG(host_object->flags, _object_is_enlarged));
	SET_FLAG(parasite_object->flags, _object_is_tiny, TEST_FLAG(host_object->flags, _object_is_tiny));
		
	return parasite_index;
}

void remove_parasitic_object(
	short host_index)
{
	struct object_data *host= get_object_data(host_index);
	struct object_data *parasite= get_object_data(host->parasitic_object);

	host->parasitic_object= NONE;
	MARK_SLOT_AS_FREE(parasite);
}

/* look up the index yourself */
void remove_map_object(
	short object_index)
{
	short *next_object;
	struct object_data *object= get_object_data(object_index);
	struct polygon_data *polygon= get_polygon_data(object->polygon);
	
	next_object= &polygon->first_object;
	while (*next_object!=object_index) next_object= &get_object_data(*next_object)->next_object;

	if (object->parasitic_object!=NONE) 
	{
		struct object_data *parasite= get_object_data(object->parasitic_object);
		
		MARK_SLOT_AS_FREE(parasite);
	}

	SoundManager::instance()->OrphanSound(object_index);
	L_Invalidate_Object(object_index);
	*next_object= object->next_object;
	MARK_SLOT_AS_FREE(object);
}



/* remove the object from the old_polygon’s object list*/
void
remove_object_from_polygon_object_list(short object_index, short polygon_index)
{
	struct object_data* object = get_object_data(object_index);

	polygon_data* polygon= get_polygon_data(polygon_index);
	short* next_object= &polygon->first_object;

	assert(*next_object != NONE);

	while (*next_object!=object_index)
	{
		next_object= &get_object_data(*next_object)->next_object;
		assert(*next_object != NONE);
	}

	*next_object= object->next_object;

	object->polygon= NONE;
}

void
remove_object_from_polygon_object_list(short object_index)
{
	remove_object_from_polygon_object_list(object_index, get_object_data(object_index)->polygon);
}



/* add the object to the new_polygon’s object list */
void
add_object_to_polygon_object_list(short object_index, short polygon_index)
{
	struct object_data* object = get_object_data(object_index);
	struct polygon_data* polygon= get_polygon_data(polygon_index);

	object->next_object= polygon->first_object;
	polygon->first_object= object_index;

	object->polygon= polygon_index;
}

typedef std::pair<short, short>	DeferredObjectListInsertion;
typedef std::list<DeferredObjectListInsertion> DeferredObjectListInsertionList;
static DeferredObjectListInsertionList sDeferredObjectListInsertions;

void
deferred_add_object_to_polygon_object_list(short object_index, short index_to_precede)
{
	sDeferredObjectListInsertions.push_back(DeferredObjectListInsertion(object_index, index_to_precede));
}



void
perform_deferred_polygon_object_list_manipulations()
{
	// Loop while the list of insertions is non-empty (we may need to make multiple passes)
	while(!sDeferredObjectListInsertions.empty())
	{
		// Pass over the list of insertions, inserting whatever we can.
		bool something_changed = false;

		for(DeferredObjectListInsertionList::iterator i = sDeferredObjectListInsertions.begin(); i != sDeferredObjectListInsertions.end(); )
		{
			short object_to_insert_index = (*i).first;
			short object_index_to_precede = (*i).second;
			object_data* object = get_object_data(object_to_insert_index);
			polygon_data* polygon = get_polygon_data(object->polygon);

			// Find object index we're supposed to predece... and insert the object before it
			short* next_object_index_p = &(polygon->first_object);
			bool inserted = false;

			while(!inserted)
			{
				if(*next_object_index_p == object_index_to_precede)
				{
					object->next_object = *next_object_index_p;
					*next_object_index_p = object_to_insert_index;
					inserted = true;
				}

				if(*next_object_index_p == NONE)
					break;

				next_object_index_p = &(get_object_data(*next_object_index_p)->next_object);

			} // Insert object before object it's supposed to precede

			// Each branch of this if() will increment i
			if(inserted)
			{
				// Most concise way to correctly remove-and-increment
				sDeferredObjectListInsertions.erase(i++);
				something_changed = true;
			}
			else
				// Perhaps we were trying to insert before another object that's scheduled for insertion.
				// In that case, maybe by the time we come around again, the other object will be inserted.
				// For now, we leave the insertion pending.
				++i;

		} // Pass over the list of insertions, inserting whatever we can.

		// We must make progress, otherwise my algorithm is flawed (we'd loop forever).  Progress here
		// is performing insertions into the polygon object lists and removing the corresponding deferred
		// insertions from the insertion list.
		assert(something_changed);

	} // Loop while the list of insertions is non-empty

} // perform_deferred_polygon_object_list_manipulations



/* if a new polygon index is supplied, it will be used, otherwise we’ll try to find the new
	polygon index ourselves */
bool translate_map_object(
	short object_index,
	world_point3d *new_location,
	short new_polygon_index)
{
	short line_index;
	struct object_data *object= get_object_data(object_index);
	short old_polygon_index= object->polygon;
	bool changed_polygons= false;
	
	/* if new_polygon is NONE, find out what polygon the new_location is in */
	if (new_polygon_index==NONE)
	{
		new_polygon_index= old_polygon_index;
		do
		{
			line_index= find_line_crossed_leaving_polygon(new_polygon_index, (world_point2d *)&object->location, (world_point2d *)new_location);
			if (line_index!=NONE) new_polygon_index= find_adjacent_polygon(new_polygon_index, line_index);
			if (new_polygon_index==NONE)
			{
				*(world_point2d *)new_location= get_polygon_data(old_polygon_index)->center;
				new_polygon_index= old_polygon_index;
				changed_polygons= true; /* tell the caller we switched polygons, even though we didn’t */
				break;
			}
		}
		while (line_index!=NONE);
	}
	
	/* if we changed polygons, update the old and new polygon’s linked lists of objects */
	if (old_polygon_index!=new_polygon_index)
	{
		remove_object_from_polygon_object_list(object_index, old_polygon_index);
		add_object_to_polygon_object_list(object_index, new_polygon_index);		
		changed_polygons= true;
	}
	object->location= *new_location;

	/* move (no saving throw) all parasitic objects along with their host */
	while (object->parasitic_object!=NONE)
	{
		object= get_object_data(object->parasitic_object);
		object->polygon= new_polygon_index;
		object->location= *new_location;
	}

	return changed_polygons;
}

void get_object_shape_and_transfer_mode(
	world_point3d* camera_location,
	short object_index,
	struct shape_and_transfer_mode* data)
{
	get_object_shape_and_transfer_mode(camera_location, get_object_data(object_index), data);
}

void get_object_shape_and_transfer_mode(
	world_point3d *camera_location,
	object_data* object,
	shape_and_transfer_mode *data)
{
	struct shape_animation_data *animation;
	angle theta;
	short view;
	
	animation= get_shape_animation_data(object->shape);
	// Added bug-outs in case of incorrect data; turned asserts into these tests:
	if (!animation)
	{
		data->collection_code = NONE; // Deliberate bad value
		return;
	}
	else if (!(animation->frames_per_view>=1))
	{
		data->collection_code = NONE; // Deliberate bad value
		return;
	}
	// assert(animation->frames_per_view>=1);
	
	/* get correct base shape */
	// LP change: made long-distance friendly
	theta= arctangent(int32(object->location.x) - int32(camera_location->x), int32(object->location.y) - int32(camera_location->y)) - object->facing;
	switch (animation->number_of_views)
	{
		case _unanimated:
		case _animated1:
			view= 0;
			break;
		
		case _animated3to4: /* front, quarter and side views only */
		case _animated4:
			switch (FACING4(theta))
			{
				case 0: view= 3; break; /* 90° (facing left) */
				case 1: view= 0; break; /* 0° (facing forward) */
				case 2: view= 1; break; /* -90° (facing right) */
				case 3: view= 2; break; /* ±180° (facing away) */
				default:
					data->collection_code = NONE; // Deliberate bad value
					return;
			}
			break;

		case _animated3to5:
		case _animated5:
			theta+= HALF_CIRCLE;
			switch (FACING5(theta))
			{
				case 0: view= 4; break;
				case 1: view= 3; break;
				case 2: view= 2; break;
				case 3: view= 1; break;
				case 4: view= 0; break;
				default:
					data->collection_code = NONE; // Deliberate bad value
					return;
			}
			break;
		
		case _animated2to8:			
		case _animated5to8:
		case _animated8:
			switch (FACING8(theta))
			{
				case 0: view= 3; break; /* 135° (facing left) */
				case 1: view= 2; break; /* 90° (facing left) */
				case 2: view= 1; break; /* 45° (facing left) */
				case 3: view= 0; break; /* 0° (facing forward) */
				case 4: view= 7; break; /* -45° (facing right) */
				case 5: view= 6; break; /* -90° (facing right) */
				case 6: view= 5; break; /* -135° (facing right) */
				case 7: view= 4; break; /* ±180° (facing away) */
				default:
					data->collection_code = NONE; // Deliberate bad value
					return;
			}
			break;
		
		default:
			data->collection_code = NONE; // Deliberate bad value
			return;
	}

	/* fill in the structure (transfer modes in the animation override transfer modes in the object */
	data->collection_code= GET_DESCRIPTOR_COLLECTION(object->shape);
	short Frame = GET_SEQUENCE_FRAME(object->sequence);
	data->Frame = Frame;
	
	// Guess next frame
	short NextFrame = Frame + 1;
	if (NextFrame >= animation->frames_per_view)
		NextFrame = animation->loop_frame;
	data->NextFrame = NextFrame;
	
	// Work out the phase in the cycle; be sure to start a cycle with 0 and not 1
	short Phase = GET_SEQUENCE_PHASE(object->sequence);
	short Ticks = animation->ticks_per_frame;
	if (Phase >= Ticks) Phase -= Ticks;
	data->Phase = Phase;
	data->Ticks = Ticks;
	
	// What bitmap, etc.
	data->low_level_shape_index= animation->low_level_shape_indexes[view*animation->frames_per_view + Frame];
	if (animation->transfer_mode==_xfer_normal && object->transfer_mode!=NONE)
	{
		data->transfer_mode= object->transfer_mode;
		data->transfer_phase= object->transfer_period ? INTEGER_TO_FIXED(object->transfer_phase)/object->transfer_period : 0;

//		if (object->transfer_mode==_xfer_fold_out) dprintf("#%d/#%d==%x", object->transfer_phase, object->transfer_period, data->transfer_phase);
	}
	else
	{
		data->transfer_mode= animation->transfer_mode;
		data->transfer_phase= (animation->transfer_mode!=_xfer_normal && animation->transfer_mode_period) ? INTEGER_TO_FIXED(object->transfer_phase)/animation->transfer_mode_period : 0;
	}
}

extern bool shapes_file_is_m1();

bool randomize_object_sequence(
	short object_index,
	shape_descriptor shape)
{
	struct object_data *object= get_object_data(object_index);
	struct shape_animation_data *animation;
	bool randomized= false;
	
	animation= get_shape_animation_data(shape);
	if (!animation) return false;
	
	switch (shapes_file_is_m1() ? _unanimated : animation->number_of_views)
	{
		case _unanimated:
			object->shape= shape;
			object->sequence= BUILD_SEQUENCE(global_random()%animation->frames_per_view, 0);
			randomized= true;
			break;
	}
	
	return randomized;
}

void set_object_shape_and_transfer_mode(
	short object_index,
	shape_descriptor shape,
	short transfer_mode)
{
	struct object_data *object= get_object_data(object_index);

	if (object->shape!=shape)
	{
		struct shape_animation_data *animation= get_shape_animation_data(shape);
		// Quit if a nonexistent animation
		// assert(animation);
		if (!animation) return;
		
		object->shape= shape;
		if (animation->transfer_mode!=_xfer_normal || object->transfer_mode==NONE) object->transfer_phase= 0;
		object->sequence= BUILD_SEQUENCE(0, 1);
		SET_OBJECT_ANIMATION_FLAGS(object, _obj_not_animated);
		
		play_object_sound(object_index, animation->first_frame_sound);
	}
	
	if (transfer_mode!=NONE)
	{
		if (object->transfer_mode!=transfer_mode)
		{
			object->transfer_mode= transfer_mode;
			object->transfer_phase= 0;
		}
	}
}

void animate_object(short object_index)
{
	animate_object(get_object_data(object_index), object_index);
}

/* no longer called by RENDER.C; must be called by monster, projectile or effect controller;
	now assumes ∂t==1 tick */
void animate_object(
	object_data* object,
	int16_t sound_id)
{
	struct shape_animation_data *animation;
	short animation_type= _obj_not_animated;

	if (!OBJECT_IS_INVISIBLE(object)) /* invisible objects don’t have valid .shape fields */
	{
		animation= get_shape_animation_data(object->shape);
		if (!animation) return;
	
		/* if this animation has frames, animate it */		
		if (animation->frames_per_view>=1 && animation->number_of_views!=_unanimated)
		{
			short frame, phase;
			
			// LP change: added some idiot-proofing to the ticks-per-frame value
			if (animation->ticks_per_frame <= 0)
				animation->ticks_per_frame = 1;
		
			frame= GET_SEQUENCE_FRAME(object->sequence);
			phase= GET_SEQUENCE_PHASE(object->sequence);

			if (sound_id != NONE && !frame && (!phase || phase>=animation->ticks_per_frame)) play_object_sound(sound_id, animation->first_frame_sound);
	
			/* phase is left unadjusted if it goes over ticks_per_frame until the next call */
			if (phase>=animation->ticks_per_frame) phase-= animation->ticks_per_frame;
			if ((phase+= 1)>=animation->ticks_per_frame)
			{
				frame+= 1;
				if (!film_profile.keyframe_fix)
				{
					animation_type|= _obj_animated;
					if (frame==animation->key_frame)
					{
						animation_type|= _obj_keyframe_started;
						if (animation->key_frame_sound!=NONE && sound_id != NONE) play_object_sound(sound_id, animation->key_frame_sound);
					}
					if (frame>=animation->frames_per_view)
					{
						frame= animation->loop_frame;
						animation_type|= _obj_last_frame_animated;
						if (animation->last_frame_sound!=NONE && sound_id != NONE) play_object_sound(sound_id, animation->last_frame_sound);
					}
				}
				else
				{
					// LP change: interchanged these two so that
					// 1: keyframe 0 would get recognized
					// 2: to keep the timing correct in the nonzero case
					// LP change: inverted the order yet again to get more like Moo,
					// but this time, added detection of cases
					// keyframe = 0 and keyframe = [frames per view]
					// Inverted the order yet again (!) to supporess Hunter death bug
					animation_type|= _obj_animated;
					if (frame>=animation->frames_per_view)
					{
						frame= animation->loop_frame;
						animation_type|= _obj_last_frame_animated;
						if (animation->last_frame_sound!=NONE && sound_id != NONE) play_object_sound(sound_id, animation->last_frame_sound);
					}
					short offset_frame = frame + animation->frames_per_view; // LP addition
					if (frame==animation->key_frame || offset_frame==animation->key_frame)
					{
						animation_type|= _obj_keyframe_started;
						if (animation->key_frame_sound!=NONE && sound_id != NONE) play_object_sound(sound_id, animation->key_frame_sound);
					}
				}
			}
	
			object->sequence= BUILD_SEQUENCE(frame, phase);
		}
		
		/* if this object has a transfer animation, update the transfer animation counter */
		{
			short period= (animation->transfer_mode==_xfer_normal && object->transfer_mode!=NONE) ? object->transfer_period : animation->transfer_mode_period;
			
			if (period)
			{
				if ((object->transfer_phase+= 1)>=period)
				{
					animation_type|= _obj_transfer_mode_finished;
					object->transfer_phase= 0;
				}
			}
		}
		
		SET_OBJECT_ANIMATION_FLAGS(object, animation_type);
	}

	/* This allows you to animate parasites of objects that are invisible (though */
	/*  it is questionable if you would ever want to do that) */
	/* if this object has any parasites, animate those too */
	if (object->parasitic_object!=NONE) animate_object(object->parasitic_object);
}

void calculate_line_midpoint(
	short line_index,
	world_point3d *midpoint)
{
	struct line_data *line= get_line_data(line_index);
	struct world_point2d *e0= &get_endpoint_data(line->endpoint_indexes[0])->vertex;
	struct world_point2d *e1= &get_endpoint_data(line->endpoint_indexes[1])->vertex;
	
	midpoint->x= (e0->x+e1->x)>>1;
	midpoint->y= (e0->y+e1->y)>>1;
	midpoint->z= (line->lowest_adjacent_ceiling+line->highest_adjacent_floor)>>1;
}

bool point_in_polygon(
	short polygon_index,
	world_point2d *p)
{
	struct polygon_data *polygon= get_polygon_data(polygon_index);
	bool point_inside= true;
	short i;
	
	for (i=0;i<polygon->vertex_count;++i)
	{
		struct line_data *line= get_line_data(polygon->line_indexes[i]);
		bool clockwise= line->endpoint_indexes[0]==polygon->endpoint_indexes[i];
		world_point2d *e0= &get_endpoint_data(line->endpoint_indexes[0])->vertex;
		world_point2d *e1= &get_endpoint_data(line->endpoint_indexes[1])->vertex;
		int32 cross_product= (p->x-e0->x)*(e1->y-e0->y) - (p->y-e0->y)*(e1->x-e0->x);
		
		if ((clockwise && cross_product>0) || (!clockwise && cross_product<0))
		{
			point_inside= false;
			break;
		}
	}
	
	return point_inside;
}

short clockwise_endpoint_in_line(
	short polygon_index,
	short line_index,
	short index)
{
	struct line_data *line= get_line_data(line_index);
	bool line_is_clockwise= true;

	if (line->clockwise_polygon_owner!=polygon_index)
	{
		// LP change: get around some Pfhorte bugs
		line_is_clockwise= false;
	}
	
	switch (index)
	{
		case 0:
			index= line_is_clockwise ? 0 : 1;
			break;
		case 1:
			index= line_is_clockwise ? 1 : 0;
			break;
		default:
			assert(false);
			break;
	}

	return line->endpoint_indexes[index];
}

short world_point_to_polygon_index(
	world_point2d *location)
{
	short polygon_index;
	struct polygon_data *polygon;
	
	for (polygon_index=0,polygon=map_polygons;polygon_index<dynamic_world->polygon_count;++polygon_index,++polygon)
	{
		if (!POLYGON_IS_DETACHED(polygon))
		{
			if (point_in_polygon(polygon_index, location)) break;
		}
	}
	if (polygon_index==dynamic_world->polygon_count) polygon_index= NONE;

	return polygon_index;
}

/* return the polygon on the other side of the given line from the given polygon (i.e., return
	the polygon adjacent to line_index which isn’t polygon_index).  can return NONE. */
short find_adjacent_polygon(
	short polygon_index,
	short line_index)
{
	struct line_data *line= get_line_data(line_index);
	short new_polygon_index;
	
	if (polygon_index==line->clockwise_polygon_owner)
	{
		new_polygon_index= line->counterclockwise_polygon_owner;
	}
	else
	{
		// LP change: get around some Pfhorte bugs
		new_polygon_index= line->clockwise_polygon_owner;
	}
	
	assert(new_polygon_index!=polygon_index);
	
	return new_polygon_index;
}

static short find_flooding_polygon_helper(short parent, short polygon_index)
{
	auto* polygon = get_polygon_data(polygon_index);

	for (auto i = 0; i < polygon->vertex_count; ++i)
	{
		auto adjacent_index = polygon->adjacent_polygon_indexes[i];
		if (adjacent_index != NONE && adjacent_index != parent)
		{
			auto *adjacent = get_polygon_data(adjacent_index);
			if (adjacent->type == _polygon_is_major_ouch ||
				adjacent->type == _polygon_is_minor_ouch)
			{
				return adjacent_index;
			}
		}
	}

	if (film_profile.m1_platform_flood)
	{
		for (auto i = 0; i < polygon->vertex_count; ++i)
		{
			auto adjacent_index = polygon->adjacent_polygon_indexes[i];
			if (adjacent_index != NONE && adjacent_index != parent)
			{
				auto* adjacent = get_polygon_data(adjacent_index);
				if (adjacent->type == _polygon_is_platform)
				{
					auto* platform = get_platform_data(adjacent->permutation);
					if (platform && PLATFORM_IS_FLOODED(platform))
					{
						auto index = find_flooding_polygon_helper(polygon_index, adjacent_index);
						if (index != NONE)
						{
							return index;
						}
					}
				}
			}
		}
	}

	return NONE;
}

/* Find the polygon whose attributes we'll mimic on a flooded platform */
short find_flooding_polygon(
	short polygon_index)
{
	return find_flooding_polygon_helper(NONE, polygon_index);
}

short find_adjacent_side(
	short polygon_index,
	short line_index)
{
	struct line_data *line= get_line_data(line_index);
	short side_index;
	
	if (line->clockwise_polygon_owner==polygon_index)
	{
		side_index= line->clockwise_polygon_side_index;
	}
	else
	{
		assert(line->counterclockwise_polygon_owner==polygon_index);
		side_index= line->counterclockwise_polygon_side_index;
	}
	
	return side_index;
}

bool line_is_landscaped(
	short polygon_index,
	short line_index,
	world_distance z)
{
	bool landscaped= false;
	short side_index= find_adjacent_side(polygon_index, line_index);
	
	if (side_index!=NONE)
	{
		struct line_data *line= get_line_data(line_index);
		struct side_data *side= get_side_data(side_index);
		
		switch (side->type)
		{
			case _full_side:
				landscaped= side->primary_transfer_mode==_xfer_landscape;
				break;
			case _split_side: /* render _low_side first */
				if (z<line->highest_adjacent_floor)
				{
					landscaped= side->secondary_transfer_mode==_xfer_landscape;
					break;
				}
			case _high_side:
				landscaped= z>line->lowest_adjacent_ceiling ?
					side->primary_transfer_mode==_xfer_landscape :
					side->transparent_transfer_mode==_xfer_landscape;
				break;
			case _low_side:
				landscaped= z<line->highest_adjacent_floor ?
					side->primary_transfer_mode==_xfer_landscape :
					side->transparent_transfer_mode==_xfer_landscape;
				break;
			
			default:
				assert(false);
				break;
		}
	}

	return landscaped;
}

/* return the line_index where the two polygons meet (or NONE if they don’t meet) */
short find_shared_line(
	short polygon_index1,
	short polygon_index2)
{
	struct polygon_data *polygon= get_polygon_data(polygon_index1);
	short shared_line_index= NONE;
	short i;
	
	for (i=0;i<polygon->vertex_count;++i)
	{
		struct line_data *line= get_line_data(polygon->line_indexes[i]);
		if (line->clockwise_polygon_owner==polygon_index2||line->counterclockwise_polygon_owner==polygon_index2)
		{
			shared_line_index= polygon->line_indexes[i];
			break;
		}
	}
	
	return shared_line_index;
}

_fixed get_object_light_intensity(
	short object_index)
{
	struct object_data *object= get_object_data(object_index);
	struct polygon_data *polygon= get_polygon_data(object->polygon);

	return get_light_intensity(polygon->floor_lightsource_index);
}

/* returns the line_index of the line we intersected to leave this polygon, or NONE if destination
	is in the given polygon */
short find_line_crossed_leaving_polygon(
	short polygon_index,
	world_point2d *p0, /* origin (not necessairly in polygon_index) */
	world_point2d *p1) /* destination (not necessairly in polygon_index) */
{
	struct polygon_data *polygon= get_polygon_data(polygon_index);
	short intersected_line_index= NONE;
	short i;
	
	for (i= 0; i<polygon->vertex_count; ++i)
	{
		/* e1 is clockwise from e0 */
		world_point2d *e0= &get_endpoint_data(polygon->endpoint_indexes[i])->vertex;
		world_point2d *e1= &get_endpoint_data(polygon->endpoint_indexes[i==polygon->vertex_count-1?0:i+1])->vertex;
		
		/* if e0p1 cross e0e1 is negative, p1 is on the outside of edge e0e1 (a result of zero
			means p1 is on the line e0e1) */
		if ((p1->x-e0->x)*(e1->y-e0->y) - (p1->y-e0->y)*(e1->x-e0->x) > 0)
		{
			/* if p0e1 cross p0p1 is positive, p0p1 crosses e0e1 to the left of e1 */
			if ((e1->x-p0->x)*(p1->y-p0->y) - (e1->y-p0->y)*(p1->x-p0->x) <= 0)
			{
				/* if p0e0 cross p0p1 is negative or zero, p0p1 crosses e0e1 on or to the right of e0 */
				if ((e0->x-p0->x)*(p1->y-p0->y) - (e0->y-p0->y)*(p1->x-p0->x) >= 0)
				{
					intersected_line_index= polygon->line_indexes[i];
					break;
				}
			}
		}
	}
	
	return intersected_line_index;
}

/* calculate the 3d intersection of the line segment p0p1 with the line e0e1 */
_fixed find_line_intersection(
	world_point2d *e0,
	world_point2d *e1,
	world_point3d *p0,
	world_point3d *p1,
	world_point3d *intersection)
{
	world_distance dx, dy, dz, line_dx, line_dy;
	int32 numerator, denominator;
	_fixed t;
	
	/* calculate line deltas */
	dx= p1->x-p0->x, dy= p1->y-p0->y, dz= p1->z-p0->z;
	line_dx= e1->x-e0->x, line_dy= e1->y-e0->y;
	
	/* calculate the numerator and denominator to compute t; our basic strategy here is to
		shift the numerator up by eight bits and the denominator down by eight bits, yeilding
		a fixed number in [0,FIXED_ONE] for t.  this won’t work if the numerator is greater
		than or equal to 2^24, or the numerator is less than 2^8.  the first case can’t be
		fixed in any decent way and shouldn’t happen if we have small deltas.  the second case
		is approximated with a denominator of 1 or -1 (depending on the sign of the old
		denominator, although notice here that numbers in [1,2^8) will get downshifted to
		zero and then set to one, while numbers in (-2^8,-1] will get downshifted to -1 and
		left there) */
	numerator= line_dx*(e0->y-p0->y) + line_dy*(p0->x-e0->x);
	denominator= line_dx*dy - line_dy*dx;
	while (numerator>=(1<<24)||numerator<=((-1)<<24)) numerator>>= 1, denominator>>= 1;
	assert(numerator<(1<<24));
	numerator<<= 8;
	if (!(denominator>>= 8)) denominator= 1;
	t= numerator/denominator;
	
	intersection->x = p0->x + FIXED_INTEGERAL_PART(int32(1LL*t*dx));
	intersection->y = p0->y + FIXED_INTEGERAL_PART(int32(1LL*t*dy));
	intersection->z = p0->z + FIXED_INTEGERAL_PART(int32(1LL*t*dz));
	
	return t;
}

/* closest_point may be the same as p; if we’re within 1 of our source point in either
	direction assume that we are actually at the source point */
_fixed closest_point_on_line(
	world_point2d *e0,
	world_point2d *e1,
	world_point2d *p,
	world_point2d *closest_point)
{
	world_distance line_dx, line_dy, dx, dy;
	world_point2d calculated_closest_point;
	int32 numerator, denominator;
	_fixed t;
	
	/* calculate dx,dy and line_dx,line_dy */
	dx= p->x-e0->x, dy= p->y-e0->y;
	line_dx= e1->x-e0->x, line_dy= e1->y-e0->y;
	
	/* same comment as above for calculating t; this is not wholly accurate */
	numerator= line_dx*dx + line_dy*dy;
	denominator= line_dx*line_dx + line_dy*line_dy;
	while (numerator>=(1<<23)||numerator<=(-1<<23)) numerator>>= 1, denominator>>= 1;
	numerator<<= 8;
	if (!(denominator>>= 8)) denominator= 1;
	t= numerator/denominator;

	/* if we’ve only changed by ±1 in x and y, return the original p to avoid sliding down
		the edge on successive calls */
	calculated_closest_point.x= e0->x + FIXED_INTEGERAL_PART(t*line_dx);
	calculated_closest_point.y= e0->y + FIXED_INTEGERAL_PART(t*line_dy);
	switch (calculated_closest_point.x-p->x)
	{
		case -1:
		case 0:
		case 1:
			switch (calculated_closest_point.y-p->y)
			{
				case -1:
				case 0:
				case 1:
					calculated_closest_point= *p;
					break;
			}
	}
	*closest_point= calculated_closest_point;

	return t;
}

void closest_point_on_circle(
	world_point2d *c,
	world_distance radius,
	world_point2d *p,
	world_point2d *closest_point)
{
	world_distance dx= p->x - c->x;
	world_distance dy= p->y - c->y;
	world_distance magnitude= isqrt(dx*dx + dy*dy);

	if (magnitude)
	{
		closest_point->x= c->x + (dx*radius)/magnitude;
		closest_point->y= c->y + (dy*radius)/magnitude;
	}
	else
	{
		*closest_point= *p;
	}
}

void find_center_of_polygon(
	short polygon_index,
	world_point2d *center)
{
	struct polygon_data *polygon= get_polygon_data(polygon_index);
	int32 x= 0, y= 0;
	short i;
	
	for (i=0;i<polygon->vertex_count;++i)
	{
		world_point2d *p= &get_endpoint_data(polygon->endpoint_indexes[i])->vertex;
		
		x+= p->x, y+= p->y;
	}
	
    // polygon->vertex_count could possibly be zero, unsure of what to do here
    // making a note.
	center->x= x/polygon->vertex_count;
	center->y= y/polygon->vertex_count;
}

/* calculate 3d intersection of the line p0p1 with the plane z=h */
_fixed find_floor_or_ceiling_intersection(
	world_distance h,
	world_point3d *p0,
	world_point3d *p1,
	world_point3d *intersection)
{
	_fixed t;
	world_distance dx, dy, dz;
	
	dx= p1->x-p0->x, dy= p1->y-p0->y, dz= p1->z-p0->z;
	t= dz ? INTEGER_TO_FIXED(h-p0->z)/dz : 0; /* if dz==0, return (p0.x,p0.y,h) */
	
	intersection->x= p0->x + FIXED_INTEGERAL_PART(int32(1LL*t*dx));
	intersection->y= p0->y + FIXED_INTEGERAL_PART(int32(1LL*t*dy));
	intersection->z= h;
	
	return t;
}

enum /* keep out states */
{
	_first_line_pass,
	_second_line_pass, /* if _first_line_pass yeilded more than one collision we have to go back
		and make sure we don’t hit anything (or only hit one thing which we hit the first time) */
	_second_line_pass_made_contact, /* we’ve already hit one thing we hit last time, if we hit
		anything else then we abort */
	_aborted, /* if we hit two lines on the second pass, we give up */
	_point_pass /* checking against all points (and hit as many as we can) */
};

/* returns height at clipped p1 */
bool keep_line_segment_out_of_walls(
	short polygon_index, /* where we started */
	world_point3d *p0,
	world_point3d *p1,
	world_distance maximum_delta_height, /* the maximum positive change in height we can tolerate */
	world_distance height, /* the height of the object being moved */
	world_distance *adjusted_floor_height,
	world_distance *adjusted_ceiling_height,
	short *supporting_polygon_index)
{
	struct polygon_data *polygon= get_polygon_data(polygon_index);
	short *indexes= get_map_indexes(polygon->first_exclusion_zone_index, polygon->line_exclusion_zone_count+polygon->point_exclusion_zone_count);
	int32 line_collision_bitmap;
	bool clipped= false;
	short state;
	short i;

	// Skip the whole thing if exclusion-zone indexes were not found
	if (!indexes)
	{
		polygon->line_exclusion_zone_count = 0;
		polygon->point_exclusion_zone_count = 0;
		return clipped;
	}

//	if (polygon_index==23) dprintf("#%d lines, #%d endpoints at %p", polygon->line_exclusion_zone_count, polygon->point_exclusion_zone_count, indexes);

	state= _first_line_pass;
	line_collision_bitmap= 0;
	*supporting_polygon_index= polygon_index;
	*adjusted_floor_height= polygon->floor_height;
	*adjusted_ceiling_height= polygon->ceiling_height;
	do
	{
		for (i=0;i<polygon->line_exclusion_zone_count&&state!=_aborted;++i)
		{
			short signed_line_index= indexes[i];
			short unsigned_line_index= signed_line_index<0 ? -signed_line_index-1 : signed_line_index;
			
			// If there is some map-index screwup...
			if (unsigned_line_index >= dynamic_world->line_count)
				continue;
			
			struct line_data *line= get_line_data(unsigned_line_index);
			short side_index= signed_line_index<0 ? line->counterclockwise_polygon_side_index : line->clockwise_polygon_side_index;
	
//			if (unsigned_line_index==104) dprintf("checking against #%d", unsigned_line_index);
				
			if (side_index!=NONE)
			{
				struct side_exclusion_zone *zone= &get_side_data(side_index)->exclusion_zone;

				if ((p1->x-zone->e0.x)*(zone->e1.y-zone->e0.y) - (p1->y-zone->e0.y)*(zone->e1.x-zone->e0.x) > 0 &&
					(p1->x-zone->e2.x)*(zone->e0.y-zone->e2.y) - (p1->y-zone->e2.y)*(zone->e0.x-zone->e2.x) > 0 &&
					(p1->x-zone->e1.x)*(zone->e3.y-zone->e1.y) - (p1->y-zone->e1.y)*(zone->e3.x-zone->e1.x) > 0)
				{
					short adjacent_polygon_index= signed_line_index<0 ? line->clockwise_polygon_owner : line->counterclockwise_polygon_owner;
					struct polygon_data *adjacent_polygon= adjacent_polygon_index==NONE ? NULL : get_polygon_data(adjacent_polygon_index);
					world_distance lowest_ceiling;
					world_distance highest_floor;

					if (adjacent_polygon) {
						lowest_ceiling= adjacent_polygon->ceiling_height<polygon->ceiling_height ? adjacent_polygon->ceiling_height : polygon->ceiling_height;
						highest_floor= adjacent_polygon->floor_height>polygon->floor_height ? adjacent_polygon->floor_height : polygon->floor_height;
					} else {
						lowest_ceiling = highest_floor = 0;
					}

					/* if a) this line is solid, b) the new polygon is farther than maximum_delta height
						above our feet, or c) the new polygon is lower than the top of our head then
						we can’t move into the new polygon */
					if (LINE_IS_SOLID(line) || adjacent_polygon == NULL ||
						adjacent_polygon->floor_height-p1->z>maximum_delta_height ||
						adjacent_polygon->ceiling_height-p1->z<height ||
						lowest_ceiling-highest_floor<height)
					{
					//	if (unsigned_line_index==104) dprintf("inside solid line #%d (%p) in polygon #%d", unsigned_line_index, line, polygon_index);
						
						switch (state)
						{
							case _first_line_pass:
								/* first pass: set the flag and do the clip */
								line_collision_bitmap|= 1<<i;
								closest_point_on_line(&zone->e0, &zone->e1, (world_point2d*)p1, (world_point2d*)p1);
								clipped= true;
								break;
							
							case _second_line_pass:
								if (line_collision_bitmap&(1<<i))
								{
									/* we hit this line before, change states (we can only hit one thing we hit before) */
									closest_point_on_line(&zone->e0, &zone->e1, (world_point2d*)p1, (world_point2d*)p1);
									state= _second_line_pass_made_contact;
								}
								else
								{
									/* forget it; we hit something we didn’t hit the first time */
									state= _aborted;
								}
								break;
							
							case _second_line_pass_made_contact:
								/* we have no tolerance for hitting two things during the second pass */
								state= _aborted;
								break;
							
							default:
								assert(false);
						}
					}
					else
					{
						if (adjacent_polygon->floor_height>*adjusted_floor_height) {
							*adjusted_floor_height= adjacent_polygon->floor_height;
							*supporting_polygon_index= adjacent_polygon_index;
						}
						if (adjacent_polygon->ceiling_height>*adjusted_ceiling_height) {
							*adjusted_ceiling_height= adjacent_polygon->ceiling_height;
						}
					}
				}
			}
		}
		
		switch (state)
		{
			case _first_line_pass:
				state= _second_line_pass; break;
			case _second_line_pass:
			case _second_line_pass_made_contact:
				state= _point_pass; break;
		}
	}
	while (state==_second_line_pass);

	/* if we didn’t abort while clipping lines, try clipping against points... */
	if (state!=_aborted)
	{
		for (i=0;i<polygon->point_exclusion_zone_count;++i)
		{
			short endpoint_index = indexes[polygon->line_exclusion_zone_count+i];
			
			// If there is some map-index screwup...
			if (endpoint_index < 0 || endpoint_index >= dynamic_world->endpoint_count)
				continue;
			
			struct endpoint_data *endpoint= get_endpoint_data(endpoint_index);
			world_distance dx= endpoint->vertex.x-p1->x;
			world_distance dy= endpoint->vertex.y-p1->y;
			int32 distance_squared= dx*dx+dy*dy;
			
//			switch (indexes[polygon->line_exclusion_zone_count+i])
//			{
//				case 34:
//				case 35:
//					dprintf("endpoint#%d is %d away", indexes[polygon->line_exclusion_zone_count+i], distance_squared);
//			}
			
			if (distance_squared<MINIMUM_SEPARATION_FROM_WALL*MINIMUM_SEPARATION_FROM_WALL)
			{
				if (endpoint->highest_adjacent_floor_height-p1->z>maximum_delta_height ||
					endpoint->lowest_adjacent_ceiling_height-p1->z<height ||
					ENDPOINT_IS_SOLID(endpoint))
				{
					closest_point_on_circle(&endpoint->vertex, MINIMUM_SEPARATION_FROM_WALL, (world_point2d*)p1, (world_point2d*)p1);
					clipped= true;
				}
				else
				{
					if (endpoint->highest_adjacent_floor_height>*adjusted_floor_height) *adjusted_floor_height= endpoint->highest_adjacent_floor_height, *supporting_polygon_index= endpoint->supporting_polygon_index;
					if (endpoint->lowest_adjacent_ceiling_height>*adjusted_ceiling_height) *adjusted_ceiling_height= endpoint->lowest_adjacent_ceiling_height;
				}
			}
		}
	}

	if (state==_aborted) p1->x= p0->x, p1->y= p0->y;
	return clipped;
}

/* take the line e0e1 and destructively move it perpendicular to itself ("to the left" when looking
	along e0e1) by the given distance d */
void push_out_line(
	world_point2d *e0,
	world_point2d *e1,
	world_distance d,
	world_distance line_length)
{
	world_distance line_dx, line_dy;
	world_distance dx, dy;
	
	/* if line_length is zero, calculate it */
	if (!line_length) {
		line_length= distance2d(e0, e1);
		if (!line_length)
			return;
	}
	
	/* calculate dx, dy (a vector of length d perpendicular (outwards) to the line e0e1 */
	line_dx= e1->x-e0->x, line_dy= e1->y-e0->y;
	dx= - (d*line_dy)/line_length, dy= (d*line_dx)/line_length;
	
	/* adjust the line */
	e0->x+= dx, e0->y+= dy;
	e1->x+= dx, e1->y+= dy;
}

/* given the ray p0,theta,d, calculate a point p1 such that p1 is on the ray but still inside
	the [-32k,32k] bounds of our map. p0 can be the same as p1 */
void ray_to_line_segment(
	world_point2d *p0,
	world_point2d *p1,
	angle theta,
	world_distance d)
{
	short dx= cosine_table[theta], dy= sine_table[theta];
	int32 x= (int32)p0->x + (int32)((d*dx)>>TRIG_SHIFT);
	int32 y= (int32)p0->y + (int32)((d*dy)>>TRIG_SHIFT);
	
	if (x<INT16_MIN) x= INT16_MIN, y= (int32)p0->y + (dy*(INT16_MIN-p0->x))/dx;
	if (x>INT16_MAX) x= INT16_MAX, y= (int32)p0->y + (dy*(INT16_MAX-p0->x))/dx;
	if (y<INT16_MIN) y= INT16_MIN, x= (int32)p0->x + (dx*(INT16_MIN-p0->y))/dy;
	if (y>INT16_MAX) y= INT16_MAX, x= (int32)p0->x + (dx*(INT16_MAX-p0->y))/dy;

	p1->x= x;
	p1->y= y;
}

/* computes the squared distance from p to the line segment e0e1 */
int32 point_to_line_segment_distance_squared(
	world_point2d *p,
	world_point2d *a,
	world_point2d *b)
{
	world_distance abx= b->x-a->x, aby= b->y-a->y;
	world_distance apx= p->x-a->x, apy= p->y-a->y;
	world_distance bpx= p->x-b->x, bpy= p->y-b->y;
	int32 distance;
	
	/* if AB dot BP is greather than or equal to zero, d is the distance between B and P */
	if (abx*bpx+aby*bpy>=0)
	{
		distance= bpx*bpx + bpy*bpy;
	}
	else
	{
		/* if BA dot AP is greather than or equal to zero, d is the distance between A and P
			(we don’t calculate BA and use -AB instead */
		if (abx*apx+aby*apy<=0)
		{
			distance= apx*apx + apy*apy;
		}
		else
		{
			distance= point_to_line_distance_squared(p, a, b);
		}
	}
	
	return distance;
}

int32 point_to_line_distance_squared(
	world_point2d *p,
	world_point2d *a,
	world_point2d *b)
{
	world_distance abx= b->x-a->x, aby= b->y-a->y;
	world_distance apx= p->x-a->x, apy= p->y-a->y;
	int32 signed_numerator;
	uint32 numerator, denominator;
	
	/* numerator is absolute value of the cross product of AB and AP, denominator is the
		magnitude of AB squared */
	signed_numerator= apx*aby - apy*abx;
	numerator= ABS(signed_numerator);
	denominator= abx*abx + aby*aby;

	/* before squaring numerator we make sure that it is smaller than fifteen bits (and we
		adjust the denominator to compensate).  if denominator==0 then we make it ==1.  */
	while (numerator>=(1<<16)) numerator>>= 1, denominator>>= 2;
	if (!denominator) denominator= 1;
	
	return (numerator*numerator)/denominator;
}

struct map_annotation *get_next_map_annotation(
	short *count)
{
	struct map_annotation *annotation= (struct map_annotation *) NULL;

	if (*count<dynamic_world->default_annotation_count) annotation= map_annotations + (*count)++;
	
	return annotation;
}

/* for saving or whatever; finds the highest used index plus one for objects, monsters, projectiles
	and effects */
void recalculate_map_counts(
	void)
{
	struct object_data *object;
	struct monster_data *monster;
	struct projectile_data *projectile;
	struct effect_data *effect;
	struct light_data *light;
	size_t count;
	
	// LP: fixed serious bug in the counting logic
	
	for (count=MAXIMUM_OBJECTS_PER_MAP,object=objects+MAXIMUM_OBJECTS_PER_MAP-1;
			count>0&&(!SLOT_IS_USED(object));
			--count,--object)
		;
	dynamic_world->object_count= static_cast<int16>(count);
	
	for (count=MAXIMUM_MONSTERS_PER_MAP,monster=monsters+MAXIMUM_MONSTERS_PER_MAP-1;
			count>0&&(!SLOT_IS_USED(monster));
			--count,--monster)
		;
	dynamic_world->monster_count= static_cast<int16>(count);
	
	for (count=MAXIMUM_PROJECTILES_PER_MAP,projectile=projectiles+MAXIMUM_PROJECTILES_PER_MAP-1;
			count>0&&(!SLOT_IS_USED(projectile));
			--count,--projectile)
		;
	dynamic_world->projectile_count= static_cast<int16>(count);
	
	for (count=MAXIMUM_EFFECTS_PER_MAP,effect=effects+MAXIMUM_EFFECTS_PER_MAP-1;
			count>0&&(!SLOT_IS_USED(effect));
			--count,--effect)
		;
	dynamic_world->effect_count= static_cast<int16>(count);
	
	for (count=MAXIMUM_LIGHTS_PER_MAP,light=lights+MAXIMUM_LIGHTS_PER_MAP-1;
			count>0&&(!SLOT_IS_USED(light));
			--count,--light)
		;
	dynamic_world->light_count= static_cast<int16>(count);
}

bool change_polygon_height(
	short polygon_index,
	world_distance new_floor_height,
	world_distance new_ceiling_height,
	struct damage_definition *damage)
{
	bool legal_change;
	
	/* returns false if a monster prevented the given change from ocurring (and probably did damage
		to him or maybe even caused him to pop) */
	legal_change= legal_polygon_height_change(polygon_index, new_floor_height, new_ceiling_height, damage);

	/* if this was a legal change, adjust all objects (handle monsters separately) */	
	if (legal_change)
	{
		struct polygon_data *polygon= get_polygon_data(polygon_index);
		short object_index= polygon->first_object;

		/* Change the objects heights... */		
		while (object_index!=NONE)
		{
			struct object_data *object= get_object_data(object_index);
			
			if (OBJECT_IS_VISIBLE(object))
			{
				switch (GET_OBJECT_OWNER(object))
				{
					case _object_is_monster:
						adjust_monster_for_polygon_height_change(object->permutation, polygon_index, new_floor_height, new_ceiling_height);
						break;
					
					default:
						if (object->location.z==polygon->floor_height) object->location.z= new_floor_height;
						break;
				}
			}
			
			object_index= object->next_object;
		}

		/* slam the polygon heights, directly */
		polygon->floor_height= new_floor_height;
		polygon->ceiling_height= new_ceiling_height;
		
		/* the highest_adjacent_floor, lowest_adjacent_ceiling and supporting_polygon_index fields
			of all of this polygon’s endpoints and lines are potentially invalid now.  to assure
			that they are correct, recalculate them using the appropriate redundant functions.
			to do things quickly, slam them yourself.  only these three fields of are invalid,
			nothing else is effected by the height change. */
	}
	
	return legal_change;
}

/* we used to check to see that the point in question was within the player’s view
	cone, but that was queer because stuff would appear behind him all the time
	(which was completely inconvient when this happened to monsters) */
/*
	Added max_players, because this could be called during initial player creation,
	when dynamic_world->player_count was not valid.
*/
bool point_is_player_visible(
	short max_players,
	short polygon_index,
	world_point2d *p,
	int32 *distance)
{
	short player_index;
	bool visible= false;
	
	*distance= INT32_MAX; /* infinite */
	for (player_index=0;player_index<max_players;++player_index)
	{
		struct player_data *player= get_player_data(player_index);
		struct monster_data *monster= get_monster_data(player->monster_index);
		struct object_data *object= get_object_data(monster->object_index);

		if (!line_is_obstructed(object->polygon, (world_point2d*)&object->location, polygon_index, p))
		{
			int32 this_distance= guess_distance2d((world_point2d*)&object->location, p);
			
			if (*distance>this_distance) *distance= this_distance;
			visible= true;
		}
	}
	
	return visible;
}

bool point_is_monster_visible(
	short polygon_index,
	world_point2d *p,
	int32 *distance)
{
	size_t  object_count;
	
	*distance = INT32_MAX; /* infinite */
	
	// LP change:
	IntersectedObjects.clear();
	possible_intersecting_monsters(&IntersectedObjects, LOCAL_INTERSECTING_MONSTER_BUFFER_SIZE, polygon_index, false);
	object_count = IntersectedObjects.size();
	
	for (size_t i=0;i<object_count;++i)
	{
		// LP change:
		struct object_data *object= get_object_data(IntersectedObjects[i]);
		int32 this_distance;
		
		this_distance = guess_distance2d((world_point2d*)&object->location, p);
		if (*distance>this_distance) *distance= this_distance;
	}

	return *distance!=INT32_MAX;
}

bool line_is_obstructed(
	short polygon_index1,
	world_point2d *p1,
	short polygon_index2,
	world_point2d *p2)
{
	short polygon_index= polygon_index1;
	bool obstructed= false;
	short line_index;
	
	do
	{
		bool last_line = false;
		if (film_profile.line_is_obstructed_fix)
		{
			line_index = find_line_crossed_leaving_polygon(polygon_index, (world_point2d *)p1, (world_point2d *)p2);
		}
		else
		{
			line_index= _find_line_crossed_leaving_polygon(polygon_index, (world_point2d *)p1, (world_point2d *)p2, &last_line);
		}

		if (line_index!=NONE)
		{
			if (last_line && polygon_index==polygon_index2) break;
			if (!LINE_IS_SOLID(get_line_data(line_index)))
			{
				/* transparent line, find adjacent polygon */
				polygon_index= find_adjacent_polygon(polygon_index, line_index);
				assert(polygon_index!=NONE);
			}
			else
			{
				obstructed= true; /* non-transparent line */
			}
			if (last_line)
			{
				if (polygon_index==polygon_index2) break;
				obstructed= true;
				break;
			}
		}
		else
		{
			/* the polygon we ended up in is different than the polygon the caller thinks the
				destination point is in; this probably means that the source is on a different
				level than the caller, but it could also easily mean that we’re dealing with
				weird boundary conditions of find_line_crossed_leaving_polygon() */
			if (polygon_index!=polygon_index2) 
			{
				obstructed= true;
				if (film_profile.line_is_obstructed_fix)
				{
					polygon_data* polygon = get_polygon_data(polygon_index);
					polygon_data* polygon2 = get_polygon_data(polygon_index2);
					for (int i = 0; i < polygon->vertex_count; ++i)
					{
						for (int j = 0; j < polygon2->vertex_count; ++j)
						{
							if (polygon->endpoint_indexes[i] == polygon2->endpoint_indexes[j])
							{
								// if our destination polygon shares any endpoints with our actual destination, we're ok
								obstructed = false;
								break;
							}
						}
					}
				}
			}
		}
	}
	while (!obstructed&&line_index!=NONE);

	return obstructed;
}

#define MAXIMUM_GARBAGE_OBJECTS_PER_MAP 256
#define MAXIMUM_GARBAGE_OBJECTS_PER_POLYGON 10

void turn_object_to_shit( /* garbage that is, garbage */
	short garbage_object_index)
{
	struct object_data *garbage_object= get_object_data(garbage_object_index);
	struct polygon_data *polygon= get_polygon_data(garbage_object->polygon);
	short garbage_objects_in_polygon, random_garbage_object_index = 0, object_index;

	struct object_data *object;
	
	/* count the number of garbage objects in this polygon */
	garbage_objects_in_polygon= 0;
	for (object_index=polygon->first_object;object_index!=NONE;object_index=object->next_object)
	{
		object= get_object_data(object_index);
		if (GET_OBJECT_OWNER(object)==_object_is_garbage)
		{
			random_garbage_object_index= object_index;
			garbage_objects_in_polygon+= 1;
		}
	}
	
	if (garbage_objects_in_polygon> (graphics_preferences->double_corpse_limit ? MAXIMUM_GARBAGE_OBJECTS_PER_POLYGON * 2 : MAXIMUM_GARBAGE_OBJECTS_PER_POLYGON))
	{
		/* there are too many garbage objects in this polygon, remove the last (oldest?) one in
			the linked list */
		remove_map_object(random_garbage_object_index);
	}
	else
	{
		/* see if we have overflowed the maximum allowable garbage objects per map; if we have then
			remove an existing piece of shit to make room for the new one (this sort of removal
			could be really obvious... but who pays attention to dead bodies anyway?) */
	  if (dynamic_world->garbage_object_count>= (graphics_preferences->double_corpse_limit? MAXIMUM_GARBAGE_OBJECTS_PER_MAP * 2 : MAXIMUM_GARBAGE_OBJECTS_PER_MAP))
	    {
			/* find a garbage object to remove, and do so (we’re certain that many exist) */
			for (object_index= garbage_object_index, object= garbage_object;
					SLOT_IS_FREE(object) || GET_OBJECT_OWNER(object)!=_object_is_garbage;
					object_index= (object_index==MAXIMUM_OBJECTS_PER_MAP-1) ? 0 : (object_index+1), object= objects+object_index)
				;
			remove_map_object(object_index);
		}
		else
		{
			dynamic_world->garbage_object_count+= 1;
		}
	}
	
	SET_OBJECT_OWNER(garbage_object, _object_is_garbage);
}

/* find an (x,y) and polygon_index for a random point on the given circle, at the same height
	as the center point */
void random_point_on_circle(
	world_point3d *center,
	short center_polygon_index,
	world_distance radius,
	world_point3d *random_point,
	short *random_polygon_index)
{
	world_distance adjusted_floor_height, adjusted_ceiling_height, supporting_polygon_index; /* not used */
	
	*random_point= *center;
	translate_point2d((world_point2d *)random_point, radius, global_random()&(NUMBER_OF_ANGLES-1));
	keep_line_segment_out_of_walls(center_polygon_index, center, random_point, 0, WORLD_ONE/12,
		&adjusted_floor_height, &adjusted_ceiling_height, &supporting_polygon_index);
	*random_polygon_index= find_new_object_polygon((world_point2d *)center,
		(world_point2d *)random_point, center_polygon_index);
	if (*random_polygon_index!=NONE)
	{
		struct polygon_data *center_polygon= get_polygon_data(center_polygon_index);
		struct polygon_data *random_polygon= get_polygon_data(*random_polygon_index);
		
		if (center_polygon->floor_height!=random_polygon->floor_height) *random_polygon_index= NONE;
	}
}

/* ---------- private code */

/* returns the line_index of the line we intersected to leave this polygon, or NONE if destination
	is in the given polygon */
short _find_line_crossed_leaving_polygon(
	short polygon_index,
	world_point2d *p0, /* origin (not necessairly in polygon_index) */
	world_point2d *p1, /* destination (not necessairly in polygon_index) */
	bool *last_line) /* set if p1 is on the line leaving the last polygon */
{
	struct polygon_data *polygon= get_polygon_data(polygon_index);
	short intersected_line_index= NONE;
	short i;
	
	for (i= 0; i<polygon->vertex_count; ++i)
	{
		/* e1 is clockwise from e0 */
		world_point2d *e0= &get_endpoint_data(polygon->endpoint_indexes[i])->vertex;
		world_point2d *e1= &get_endpoint_data(polygon->endpoint_indexes[i==polygon->vertex_count-1?0:i+1])->vertex;
		int32 not_on_line;
		
		/* if e0p1 cross e0e1 is negative, p1 is on the outside of edge e0e1 (a result of zero
			means p1 is on the line e0e1) */
		if ((not_on_line= (p1->x-e0->x)*(e1->y-e0->y) - (p1->y-e0->y)*(e1->x-e0->x)) >= 0)
		{
			/* if p0e1 cross p0p1 is positive, p0p1 crosses e0e1 to the left of e1 */
			if ((e1->x-p0->x)*(p1->y-p0->y) - (e1->y-p0->y)*(p1->x-p0->x) <= 0)
			{
				/* if p0e0 cross p0p1 is negative or zero, p0p1 crosses e0e1 on or to the right of e0 */
				if ((e0->x-p0->x)*(p1->y-p0->y) - (e0->y-p0->y)*(p1->x-p0->x) >= 0)
				{
					intersected_line_index= polygon->line_indexes[i];
					*last_line= !not_on_line;
					break;
				}
			}
		}
	}
	
	return intersected_line_index;
}

static short _new_map_object(
	shape_descriptor shape,
	angle facing)
{
	struct object_data *object;
	short object_index;
	
	for (object_index=0,object=objects;object_index<MAXIMUM_OBJECTS_PER_MAP;++object_index,++object)
	{
		if (SLOT_IS_FREE(object))
		{
			/* initialize the object_data structure.  the defaults result in a normal (i.e., scenery),
				non-solid object.  the rendered, animated and status flags are initially clear. */
			object->polygon= NONE;
			object->shape= shape;
			object->facing= facing;
			object->transfer_mode= NONE;
			object->transfer_phase= 0;
			object->permutation= 0;
			object->sequence= 0;
			object->flags= 0;
			object->next_object= NONE;
			object->parasitic_object= NONE;
			object->sound_pitch= FIXED_ONE;
			
			MARK_SLOT_AS_USED(object);
				
			/* Objects with a shape of UNONE are invisible. */
			if(shape==UNONE)
			{
				SET_OBJECT_INVISIBILITY(object, true);
			}
	
			break;
		}
	}
	if (object_index==MAXIMUM_OBJECTS_PER_MAP) object_index= NONE;
	
	return object_index;
}

bool line_has_variable_height(
	short line_index)
{
	struct line_data *line= get_line_data(line_index);
	struct polygon_data *polygon;

	if(line->clockwise_polygon_owner != NONE)
	{
		if(line->counterclockwise_polygon_owner != NONE)
		{
			polygon= get_polygon_data(line->counterclockwise_polygon_owner);
			if (polygon->type==_polygon_is_platform)
			{
				return true;
			}
		}
		
		polygon= get_polygon_data(line->clockwise_polygon_owner);
		if (polygon->type==_polygon_is_platform)
		{
			return true;
		}		
	}
	
	return false;
}

/* ---------- sound code */

void play_object_sound(
	short object_index,
	short sound_code)
{
	struct object_data *object= get_object_data(object_index);
	world_location3d *location= GET_OBJECT_OWNER(object)==_object_is_monster ?
		(world_location3d *) &get_monster_data(object->permutation)->sound_location : 
		(world_location3d *) &object->location;

	SoundManager::instance()->PlaySound(sound_code, location, object_index, object->sound_pitch);
}

void play_polygon_sound(
	short polygon_index,
	short sound_code)
{
	struct polygon_data *polygon= get_polygon_data(polygon_index);
	world_location3d source;
	
	find_center_of_polygon(polygon_index, (world_point2d *)&source.point);
	source.point.z= polygon->floor_height;
	source.polygon_index= polygon_index;
	
	SoundManager::instance()->PlaySound(sound_code, &source, NONE);
}

void _play_side_sound(
	short side_index,
	short sound_code,
	_fixed pitch)
{
	struct side_data *side= get_side_data(side_index);
	world_location3d source;

	calculate_line_midpoint(side->line_index, &source.point);
	source.polygon_index= side->polygon_index;

	SoundManager::instance()->PlaySound(sound_code, &source, NONE, pitch);
}

void play_world_sound(
	short polygon_index,
	world_point3d *origin,
	short sound_code)
{
	world_location3d source;
	
	source.point= *origin;
	source.polygon_index= polygon_index;
	SoundManager::instance()->PlaySound(sound_code, &source, NONE);
}

world_location3d *_sound_listener_proc(
	void)
{
	return (world_location3d *) ((get_game_state()==_game_in_progress) ?
		&current_player->camera_location :
//		&get_object_data(get_monster_data(current_player->monster_index)->object_index)->location :
		NULL);
}

// stuff floating on top of media is above it
uint16 _sound_obstructed_proc(
	world_location3d *source)
{
	world_location3d *listener= _sound_listener_proc();
	uint16 flags= 0;
	
	if (listener)
	{
		if (line_is_obstructed(source->polygon_index, (world_point2d *)&source->point,
			listener->polygon_index, (world_point2d *)&listener->point))
		{
			flags|= _sound_was_obstructed;
		}
		else
		{
			struct polygon_data *source_polygon= get_polygon_data(source->polygon_index);
			struct polygon_data *listener_polygon= get_polygon_data(listener->polygon_index);
			bool source_under_media= false, listener_under_media= false;
			
			// LP change: idiot-proofed the media handling
			if (source_polygon->media_index!=NONE)
			{
				media_data *media = get_media_data(source_polygon->media_index);
				if (media)
				{
					if (source->point.z<media->height)
					{
						source_under_media= true;
					}
				}
			}
			
			if (listener_polygon->media_index!=NONE)
			{
				media_data *media = get_media_data(listener_polygon->media_index);
				if (media)
				{
					if (listener->point.z<media->height)
					{
						listener_under_media= true;
					}
				}
			}
			
			if (source_under_media)
			{
				if (!listener_under_media || source_polygon->media_index!=listener_polygon->media_index)
				{
					flags|= _sound_was_media_obstructed;
				}
				else
				{
					flags|= _sound_was_media_muffled;
				}
			}
			else
			{
				if (listener_under_media)
				{
					flags|= _sound_was_media_obstructed;
				}
			}
		}
	}
	
	return flags;
}

// for current player
void _sound_add_ambient_sources_proc(
	void *data,
	add_ambient_sound_source_proc_ptr add_one_ambient_sound_source)
{
	struct world_location3d *listener= _sound_listener_proc();
	
	if (listener)
	{
		struct polygon_data *listener_polygon= get_polygon_data(listener->polygon_index);
		struct media_data *media= listener_polygon->media_index!=NONE ? get_media_data(listener_polygon->media_index) : (struct media_data *) NULL;
		short *indexes= get_map_indexes(listener_polygon->sound_source_indexes, 0);
		world_location3d source;
		bool under_media= false;
		short index;
		
		// add ambient sound image
		if (media && listener->point.z<media->height)
		{
			// if we’re under media don’t play the ambient sound image
			add_one_ambient_sound_source((struct ambient_sound_data *)data, (world_location3d *) NULL, listener,
				get_media_sound(listener_polygon->media_index, _media_snd_ambient_under), MAXIMUM_SOUND_VOLUME);
			under_media= true;
		}
		else
		{
			// if we have an ambient sound image, play it
			if (listener_polygon->ambient_sound_image_index!=NONE)
			{
				struct ambient_sound_image_data *image= get_ambient_sound_image_data(listener_polygon->ambient_sound_image_index);
				
				// LP change: returning NULL means this is invalid; do some editing if necessary
				if (image)
					add_one_ambient_sound_source((struct ambient_sound_data *)data, (world_location3d *) NULL, listener, image->sound_index, image->volume);
				else
					listener_polygon->ambient_sound_image_index = NONE;
			}

			// if we’re over media, play that ambient sound image
			if (media && (media->height>=listener_polygon->floor_height || !MEDIA_SOUND_OBSTRUCTED_BY_FLOOR(media)))
			{
				source= *listener, source.point.z= media->height;
				add_one_ambient_sound_source((struct ambient_sound_data *)data, &source, listener,
					get_media_sound(listener_polygon->media_index, _media_snd_ambient_over), MAXIMUM_SOUND_VOLUME);
			}
		}

		// add ambient sound image from platform
		if (listener_polygon->type==_polygon_is_platform)
		{
			struct platform_data *platform= get_platform_data(listener_polygon->permutation);
			
			if (PLATFORM_IS_ACTIVE(platform) && PLATFORM_IS_MOVING(platform))
			{
				source= *listener, source.point.z= listener_polygon->floor_height;
				add_one_ambient_sound_source((struct ambient_sound_data *)data, &source, listener,
					get_platform_moving_sound(listener_polygon->permutation), MAXIMUM_SOUND_VOLUME);
			}
		}

		// add ambient sound sources
		// do only if indexes were found
		if (indexes)
		{
		while ((index= *indexes++)!=NONE && index < MAXIMUM_SAVED_OBJECTS)
		{
			struct map_object *object= saved_objects + index; // gross, sorry
			struct polygon_data *polygon= get_polygon_data(object->polygon_index);
			struct media_data *media= polygon->media_index!=NONE ? get_media_data(polygon->media_index) : (struct media_data *) NULL;
			short sound_type= object->index;
			short sound_volume= object->facing;
			bool active= true;

			if (sound_volume<0)
			{
				sound_volume= get_light_intensity(-sound_volume)>>8;
			}
			
			// yaw, pitch are irrelevant
			source.point= object->location;
			source.polygon_index= object->polygon_index;
			if (object->flags&_map_object_hanging_from_ceiling)
			{
				source.point.z+= polygon->ceiling_height;
			}
			else
			{
				if ((object->flags&_map_object_floats) && media)
				{
					source.point.z+= media->height;
				}
				else
				{
					source.point.z+= polygon->floor_height;
				}
			}
			
			// adjust source if necessary (like, for a platform)
			if (object->flags&_map_object_is_platform_sound)
			{
				if (polygon->type==_polygon_is_platform && PLATFORM_IS_MOVING(get_platform_data(polygon->permutation)))
				{
					sound_type= get_platform_moving_sound(polygon->permutation);
					source.point.z= listener->point.z; // always on our level
				}
				else
				{
					active= false;
				}
			}

			// .index is environmental sound type, .facing is volume
			// CB: added check for media != NULL because it sometimes crashed here when being underwater
			if (active && (!under_media || (media && source.point.z<media->height && polygon->media_index==listener_polygon->media_index)))
			{
				add_one_ambient_sound_source((struct ambient_sound_data *)data, &source, listener, sound_type, sound_volume);
			}
		}
		}
	}
}

void handle_random_sound_image(
	void)
{
	struct polygon_data *polygon= get_polygon_data(current_player->camera_polygon_index);
	
	if (polygon->random_sound_image_index!=NONE)
	{
		struct random_sound_image_data *image= get_random_sound_image_data(polygon->random_sound_image_index);
		
		// LP change: returning NULL means this is invalid; do some editing if necessary
		if (image)
		{
		// play a random sound
		if (!image->phase)
		{
			short volume= image->volume;
			angle direction= image->direction;
			_fixed pitch= image->pitch;
			
			if (image->delta_volume) volume+= local_random()%image->delta_volume;
			if (image->delta_direction) direction= NORMALIZE_ANGLE(direction + local_random()%image->delta_direction);
			if (image->delta_pitch) pitch+= local_random()%image->delta_pitch;

			SoundManager::instance()->DirectPlaySound(SoundManager::instance()->RandomSoundIndexToSoundIndex(image->sound_index), (image->flags & _sound_image_is_non_directional) ? NONE : direction, volume, pitch);
		}
		
		// lower phase and reset if necessary
		if ((image->phase-= 1)<0)
		{
			image->phase= image->period;
			if (image->delta_period) image->phase+= local_random()%image->delta_period;
		}
		}
		else
			polygon->random_sound_image_index = NONE;
	}
}


// XML elements for parsing the texture-loading specification
// Uses an attribute for loading the landscapes
// and a subelement for specifying which texture in an environment

short **OriginalEnvironments = NULL;

void reset_mml_texture_loading()
{
	LandscapesLoaded = true;
	if (OriginalEnvironments) {
		for (int i = 0; i < NUMBER_OF_ENVIRONMENTS; i++) {
			for (int j = 0; j < NUMBER_OF_ENV_COLLECTIONS; j++)
				Environments[i][j] = OriginalEnvironments[i][j];
			free(OriginalEnvironments[i]);
		}
		free(OriginalEnvironments);
		OriginalEnvironments = NULL;
	}
}

void parse_mml_texture_loading(const InfoTree& root)
{
	// back up old values first
	if (!OriginalEnvironments) {
		OriginalEnvironments = (short **) malloc(sizeof(short *) * NUMBER_OF_ENVIRONMENTS);
		assert(OriginalEnvironments);
		for (int i = 0; i < NUMBER_OF_ENVIRONMENTS; i++) {
			OriginalEnvironments[i] = (short *) malloc(sizeof(short) * NUMBER_OF_ENV_COLLECTIONS);
			assert(OriginalEnvironments[i]);
			for (int j = 0; j < NUMBER_OF_ENV_COLLECTIONS; j++)
				OriginalEnvironments[i][j] = Environments[i][j];
		}
	}
	
	root.read_attr("landscapes", LandscapesLoaded);
	
	BOOST_FOREACH(InfoTree env, root.children_named("texture_env"))
	{
		int16 index, which, coll;
		if (env.read_indexed("index", index, NUMBER_OF_ENVIRONMENTS) &&
			env.read_indexed("which", which, NUMBER_OF_ENV_COLLECTIONS) &&
			env.read_indexed("coll", coll, MAXIMUM_COLLECTIONS, true))
		{
			Environments[index][which] = coll;
		}
	}
}
