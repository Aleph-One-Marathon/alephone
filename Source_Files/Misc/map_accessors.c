/*
MAP_ACCESSORS.C
Friday, June 3, 1994 12:10:10 PM

Thursday, June 16, 1994 7:32:13 PM
	if DEBUG is defined, this file is empty.
*/

#include "cseries.h"
#include "map.h"

#ifdef env68k
#pragma segment map
#endif

/* --------- code */

#ifdef DEBUG
struct object_data *get_object_data(
	short object_index)
{
	struct object_data *object;
	
	vassert(object_index>=0&&object_index<MAXIMUM_OBJECTS_PER_MAP, csprintf(temporary, "object index #%d is out of range", object_index));
	
	object= objects+object_index;
	vassert(SLOT_IS_USED(object), csprintf(temporary, "object index #%d is unused", object_index));
	
	return object;
}

struct polygon_data *get_polygon_data(
	short polygon_index)
{
	struct polygon_data *polygon;

	assert(map_polygons);	
	vassert(polygon_index>=0&&polygon_index<dynamic_world->polygon_count, csprintf(temporary, "polygon index #%d is out of range", polygon_index));
	
	polygon= map_polygons+polygon_index;
	
	return polygon;
}

struct line_data *get_line_data(
	short line_index)
{
	struct line_data *line;
	
	assert(map_lines);
	vassert(line_index>=0&&line_index<dynamic_world->line_count, csprintf(temporary, "line index #%d is out of range", line_index));
	
	line= map_lines+line_index;
	
	return line;
}

struct side_data *get_side_data(
	short side_index)
{
	struct side_data *side;
	
	assert(map_sides);
	vassert(side_index>=0&&side_index<dynamic_world->side_count, csprintf(temporary, "side index #%d is out of range", side_index));
	
	side= map_sides+side_index;
	
	return side;
}

struct endpoint_data *get_endpoint_data(
	short endpoint_index)
{
	struct endpoint_data *endpoint;

	assert(map_endpoints);
	vassert(endpoint_index>=0&&endpoint_index<dynamic_world->endpoint_count, csprintf(temporary, "endpoint index #%d is out of range", endpoint_index));

	endpoint= map_endpoints+endpoint_index;
	
	return endpoint;
}

short *get_map_indexes(
	short index,
	short count)
{
	assert(map_indexes);
	vassert(index>=0&&index+count<=dynamic_world->map_index_count, csprintf(temporary, "map_indexes(#%d,#%d) are out of range", index, count));
	
	return map_indexes + index;
}

// LP change: if invalid, return NULL
struct ambient_sound_image_data *get_ambient_sound_image_data(
	short ambient_sound_image_index)
{
	struct ambient_sound_image_data *image;
	
	// LP change:
	if (!(ambient_sound_image_index>=0 && ambient_sound_image_index<dynamic_world->ambient_sound_image_count)) return NULL;
	/*
	vassert(ambient_sound_image_index>=0 && ambient_sound_image_index<dynamic_world->ambient_sound_image_count,
		csprintf(temporary, "ambient_sound_image_index #%d is out of range", ambient_sound_image_index));
	*/
	
	image= ambient_sound_images + ambient_sound_image_index;
	
	return image;
}

// LP change: if invalid, return NULL
struct random_sound_image_data *get_random_sound_image_data(
	short random_sound_image_index)
{
	struct random_sound_image_data *image;
	
	// LP change:
	if(!(random_sound_image_index>=0 && random_sound_image_index<dynamic_world->random_sound_image_count)) return NULL;
	/*
	vassert(random_sound_image_index>=0 && random_sound_image_index<dynamic_world->random_sound_image_count,
		csprintf(temporary, "random_sound_image_index #%d is out of range", random_sound_image_index));
	*/
	
	image= random_sound_images + random_sound_image_index;
	
	return image;
}
#endif
