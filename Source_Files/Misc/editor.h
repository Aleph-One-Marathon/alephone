/*
	EDITOR.H
	Sunday, April 17, 1994 10:50:25 PM

Feb 12, 2000 (Loren Petrich):
	Added MARATHON_INFINITY_DATA_VERSION and set EDITOR_MAP_VERSION to it
*/

#ifndef __EDITOR_H_
#define __EDITOR_H_

#define MARATHON_ONE_DATA_VERSION 0
#define MARATHON_TWO_DATA_VERSION 1
// LP change:
#define MARATHON_INFINITY_DATA_VERSION 2
#define EDITOR_MAP_VERSION (MARATHON_INFINITY_DATA_VERSION)
// #define EDITOR_MAP_VERSION (MARATHON_TWO_DATA_VERSION)

#if 0
/* these are in map.h, why again? */
typedef world_point2d saved_map_pt;
typedef struct line_data saved_line;
typedef struct side_data saved_side;
typedef struct polygon_data saved_poly;
typedef struct map_annotation saved_annotation;
typedef struct map_object saved_object;
typedef struct static_data saved_map_data;
#endif

#define MINIMUM_MAP_X_COORDINATE SHORT_MIN
#define MAXIMUM_MAP_X_COORDINATE SHORT_MAX
#define MINIMUM_MAP_Y_COORDINATE SHORT_MIN
#define MAXIMUM_MAP_Y_COORDINATE SHORT_MAX

#define MINIMUM_FLOOR_HEIGHT (-8*WORLD_ONE)
#define MINIMUM_CEILING_HEIGHT (MINIMUM_FLOOR_HEIGHT+WORLD_ONE)

#define MAXIMUM_FLOOR_HEIGHT (8*WORLD_ONE)
#define MAXIMUM_CEILING_HEIGHT (MAXIMUM_FLOOR_HEIGHT+WORLD_ONE)

#define INVALID_HEIGHT (MINIMUM_FLOOR_HEIGHT-1)

enum {
	_saved_guard_path_is_random= 0x0001
};

struct map_index_data 
{
	char level_name[LEVEL_NAME_LENGTH];
	char unused;
	long level_flags;
};

#define MAXIMUM_GUARD_PATH_CONTROL_POINTS 20

struct saved_path
{
	short point_count;
	word flags;
	world_point2d points[MAXIMUM_GUARD_PATH_CONTROL_POINTS];
	short polygon_indexes[MAXIMUM_GUARD_PATH_CONTROL_POINTS];
};

/* Prevent ridiculous maps.. */
#define MAX_LINES_PER_VERTEX 15
#endif
