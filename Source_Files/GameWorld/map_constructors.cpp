/*
MAP_CONSTRUCTORS.C

	Copyright (C) 1991-2001 and beyond by Bungie Studios, Inc.
	and the "Aleph One" developers.
 
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	This license is contained in the file "COPYING",
	which is included with this source code; it is available online at
	http://www.gnu.org/licenses/gpl.html

Friday, June 3, 1994 1:06:31 PM

Thursday, March 23, 1995 8:53:35 PM  (Jason')
	added guess_side_lightsource_indexes().

Jan 30, 2000 (Loren Petrich):
	Added some typecasts

Feb. 4, 2000 (Loren Petrich):
	Changed halt() to assert(false) for better debugging

Feb 15, 2000 (Loren Petrich):
	Suppressed some assertions designed to check for map consistency;
	this is to get around some Pfhorte bugs.

April 16, 2000 (Loren Petrich):
	Made the incorrect-count vwarns optional

Aug 29, 2000 (Loren Petrich):
	Created packing and unpacking functions for all the
		externally-accessible data types defined here

Dec 14, 2000 (Loren Petrich):
	Added growable lists for lists of intersecting endpoints, lines, and polygons

Aug 12, 2001 (Ian Rickard):
	All sorts of changes for OOzing mostly, some B&B prep
*/

const bool DoIncorrectCountVWarn = true;


#include "cseries.h"
#include "map.h"
#include "flood_map.h"
#include "Packing.h"

#include <limits.h>
#include <vector>

/*
maps of one polygon don’t have their impassability information computed

//detached polygons (i.e., shadows) and their twins will not have their neighbor polygon lists correctly computed
//adjacent polygons should be precalculated in the polygon structure
//intersecting_flood_proc can’t store side information (using sign) with line index zero
//keep_line_segment_out_of_walls() can’t use precalculated height information and should do weird things next to elevators and doors
*/

#ifdef env68k
#pragma segment map
#endif

/* ---------- structures */

#define MAXIMUM_INTERSECTING_INDEXES 64

struct intersecting_flood_data
{
	// This stuff now global:
	/*
	short *line_indexes;
	short line_count;
	
	short *endpoint_indexes;
	short endpoint_count;
	
	short *polygon_indexes;
	short polygon_count;
	*/
	
	short original_polygon_index;
	world_point2d center;
	
	long minimum_separation_squared;
};

/* ---------- globals */
static long map_index_buffer_count= 0l; /* Added due to the dynamic nature of maps */

// LP: Temporary areas for nearby endpoint/line/polygon finding;
// OK for this to be global since they replace only single instances.
static vector<short> LineIndices(MAXIMUM_INTERSECTING_INDEXES);
static vector<short> EndpointIndices(MAXIMUM_INTERSECTING_INDEXES);
static vector<short> PolygonIndices(MAXIMUM_INTERSECTING_INDEXES);


/* ---------- private prototypes */

static short calculate_clockwise_endpoints(short polygon_index, short *buffer);
static void calculate_adjacent_polygons(short polygon_index, short *polygon_indexes);
static void calculate_adjacent_sides(short polygon_index, short *side_indexes);
static long calculate_polygon_area(short polygon_index);

static void add_map_index(short index, short *count);
static void find_intersecting_endpoints_and_lines(short polygon_index, world_distance minimum_separation);
static long intersecting_flood_proc(short source_polygon_index, short line_index,
	short destination_polygon_index, void *data);

static void precalculate_polygon_sound_sources(void);

/* ---------- code */
/* calculates area, clockwise endpoint list, adjacent polygons */
void recalculate_redundant_polygon_data(
	short polygon_index)
{
	struct polygon_data *polygon= get_polygon_data(polygon_index);

	if (!POLYGON_IS_DETACHED(polygon))
	{
		calculate_clockwise_endpoints(polygon_index, polygon->endpoint_indexes);
		calculate_adjacent_polygons(polygon_index, polygon->adjacent_polygon_indexes);
		polygon->area= calculate_polygon_area(polygon_index);

		find_center_of_polygon(polygon_index, &polygon->center);
		calculate_adjacent_sides(polygon_index, polygon->side_indexes);
	}

	// TEMPORARY UNTIL THE EDITOR SETS THESE FIELDS !!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//	polygon->media_lightsource_index= polygon->floor_lightsource_index;
//	polygon->ambient_sound_image_index= NONE;
//	polygon->random_sound_image_index= NONE;
}

/* calculates solidity, highest adjacent floor and lowest adjacent ceiling; not to be called
	at runtime. */
// IR change: OOzing
void endpoint_data::recalculate_redundant_data()
{
	short endpoint_index = MY_INDEX;
//	struct endpoint_data *endpoint= get_endpoint_data(endpoint_index);
	struct line_data *line;
	short line_index;
	bool solid= false;
	bool elevation= false;
	bool transparent= true;
	
	for (line_index= 0, line= map_lines; line_index<dynamic_world->line_count; ++line_index, ++line)
	{
		/* does this line contain our endpoint? */
		// IR change: OOzing
		if (line->endpoint_0().index()==endpoint_index || line->endpoint_1().index()==endpoint_index)
		{
			short polygon_index;
			struct polygon_data *polygon;

			/* if this line is solid, so is the endpoint */			
			// IR changes: OOzing			
			if (line->is_solid()) solid= true;
			if (!line->is_transparent()) transparent= false;
			if (line->is_elevation()) elevation= true;
			
			/* look at adjacent polygons to determine highest floor and lowest ceiling */
			polygon_index= line->clockwise_polygon_owner;
			if (polygon_index!=NONE)
			{
				polygon= get_polygon_data(polygon_index);
				// IR changes: OOzing
				if (highest_adjacent_floor_height<polygon->lowest_floor())
				{
					highest_adjacent_floor_height= polygon->lowest_floor();
					supporting_polygon_index= polygon_index;
				}
				if (lowest_adjacent_ceiling_height>polygon->highest_ceiling())
					lowest_adjacent_ceiling_height= polygon->highest_ceiling();
			}
			polygon_index= line->counterclockwise_polygon_owner;
			if (polygon_index!=NONE)
			{
				polygon= get_polygon_data(polygon_index);
				// IR changse: OOzing
				if (highest_adjacent_floor_height<polygon->lowest_floor())
				{
					highest_adjacent_floor_height= polygon->lowest_floor();
					supporting_polygon_index= polygon_index;
				}
				if (lowest_adjacent_ceiling_height>polygon->highest_ceiling())
					lowest_adjacent_ceiling_height= polygon->highest_ceiling();
			}
		}
	}

<<<<<<< map_constructors.cpp
	set_flag(kSolid, solid);
	set_flag(kTransparent, transparent);
	set_flag(kElevation, elevation);
	highest_adjacent_floor_height= highest_adjacent_floor_height;
	lowest_adjacent_ceiling_height= lowest_adjacent_ceiling_height;
	supporting_polygon_index= supporting_polygon_index;
	
	return;
=======
	SET_ENDPOINT_SOLIDITY(endpoint, solid);
	SET_ENDPOINT_TRANSPARENCY(endpoint, transparent);
	SET_ENDPOINT_ELEVATION(endpoint, elevation);
	endpoint->highest_adjacent_floor_height= highest_adjacent_floor_height;
	endpoint->lowest_adjacent_ceiling_height= lowest_adjacent_ceiling_height;
	endpoint->supporting_polygon_index= supporting_polygon_index;
>>>>>>> 1.11
}

/* calculates line length, highest adjacent floor and lowest adjacent ceiling and calls
	recalculate_redundant_side_data() on the line’s sides */
// IR change: OOzing
void line_data::recalculate_redundant_data()
{
//	struct line_data *line= get_line_data(line_index);
	struct side_data *clockwise_side=NULL, *counterclockwise_side=NULL;
	bool elevation= false;
	bool landscaped= false;
	bool variable_elevation= false;
	bool transparent_texture= false;
	
	/* recalculate line length */
	// IR change: OOzing
	length= distance2d(&endpoint_0()->vertex, &endpoint_1()->vertex);

	/* find highest adjacent floor and lowest adjacent ceiling */
	{
		struct polygon_data *polygon1, *polygon2;
		
		// IR change: OOzing
		polygon1 = get_clockwise_polygon();
		polygon2 = get_counterclockwise_polygon();
		
		if ((polygon1&&polygon1->type==_polygon_is_platform) || (polygon2&&polygon2->type==_polygon_is_platform)) variable_elevation= true;
		
		if (polygon1&&polygon2)
		{
			// IR change: OOzing
			highest_adjacent_floor = MAX(polygon1->lowest_floor(), polygon2->lowest_floor());
			lowest_adjacent_ceiling = MIN(polygon1->highest_ceiling(), polygon2->highest_ceiling());
			if (polygon1->lowest_floor() != polygon2->lowest_floor()) elevation= true;
		}
		else
		{
			elevation= true;
			
			// IR change: swizzled this if around for more linear logic.
			if (polygon1)
			{
				// IR change: OOzing
				highest_adjacent_floor = polygon1->lowest_floor();
				lowest_adjacent_ceiling = polygon1->highest_ceiling();
			}
			else if (polygon2)
			{
				// IR change: OOzing
				highest_adjacent_floor = polygon2->lowest_floor();
				lowest_adjacent_ceiling = polygon2->highest_ceiling();
			}
			else
			{
				highest_adjacent_floor = lowest_adjacent_ceiling = 0;
			}
		}
	}
	
	
	// IR addition: cache this since calculating it requires a divide.
 	short line_index = MY_INDEX;
	
	// IR change: lots of OOzing changes for the next several lines	
	clockwise_side = get_clockwise_side();
	if (clockwise_side)
	{
		recalculate_redundant_side_data(clockwise_polygon_side_index, line_index);
		clockwise_side = get_clockwise_side();
	}
	
	counterclockwise_side = get_counterclockwise_side();
	if (counterclockwise_side)
	{
		recalculate_redundant_side_data(counterclockwise_polygon_side_index, line_index);
		counterclockwise_side = get_counterclockwise_side();
	}

	if ((clockwise_side && clockwise_side->primary_texture.transfer_mode==_xfer_landscape) ||
		(counterclockwise_side && counterclockwise_side->primary_texture.transfer_mode==_xfer_landscape))
	{
		landscaped= true;
	}
	
	if ((clockwise_side && clockwise_side->transparent_texture.texture!=NONE) ||
		(counterclockwise_side && counterclockwise_side->transparent_texture.texture!=NONE))
	{
		transparent_texture= true;
	}

<<<<<<< map_constructors.cpp
	// IR change: OOzing
	set_flag(kElevation, elevation);
	set_flag(kVariableElevation, variable_elevation && !i_am(kSolid));
	set_flag(kLandscape, landscaped);
	set_flag(kTransparentSide, transparent_texture);

	return;
=======
	SET_LINE_ELEVATION(line, elevation);
	SET_LINE_VARIABLE_ELEVATION(line, variable_elevation && !LINE_IS_SOLID(line));
	SET_LINE_LANDSCAPE_STATUS(line, landscaped);
	SET_LINE_HAS_TRANSPARENT_SIDE(line, transparent_texture);
>>>>>>> 1.11
}

void recalculate_redundant_side_data(
	short side_index,
	short line_index)
{
	struct side_data *side= get_side_data(side_index);
	struct line_data *line= get_line_data(line_index);
	world_point2d *e0, *e1;

	// TEMPORARY UNTIL THE EDITOR SETS THESE FIELDS !!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//	side->transparent_texture.texture= NONE; // no transparent texture
//	side->ambient_delta= 0;

	if (line->clockwise_polygon_side_index==side_index)
	{
		// IR change: OOzing
		e0= &line->endpoint_0()->vertex;
		e1= &line->endpoint_1()->vertex;
		side->polygon_index= line->clockwise_polygon_owner;
	}
	else
	{
		assert(side_index==line->counterclockwise_polygon_side_index);

		// IR change: OOzing
		e0= &line->endpoint_1()->vertex;
		e1= &line->endpoint_0()->vertex;
		side->polygon_index= line->counterclockwise_polygon_owner;
	}

//	if (line_index==98) dprintf("line sides: %d,%d side_index==%d", line->clockwise_polygon_side_index, line->counterclockwise_polygon_side_index, side_index);
	
	side->exclusion_zone.e0= side->exclusion_zone.e2= *e0;
	side->exclusion_zone.e1= side->exclusion_zone.e3= *e1;
	push_out_line(&side->exclusion_zone.e0, &side->exclusion_zone.e1, MINIMUM_SEPARATION_FROM_WALL, line->length);
	
	side->line_index= line_index;
//	side->direction= arctangent(e0->x - e1->x, e0->y - e1->y);
	
//	if (line_index==98||line_index==64)
//		dprintf("e0(%d,%d) e1(%d,%d) e2(%d,%d) e3(%d,%d)", impassable_side->e0.x, impassable_side->e0.y,
//		 impassable_side->e1.x, impassable_side->e1.y, impassable_side->e2.x, impassable_side->e2.y,
//		 impassable_side->e3.x, impassable_side->e3.y);
	
	// TEMPORARY UNTIL THE EDITOR SETS THESE FIELDS !!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//	guess_side_lightsource_indexes(side_index);
}

void calculate_side_clip_data() {
	for (int i=0 ; i<LineList.size() ; i++) {
		line_data *line = get_line_data(i);
		assert(line);
		
		polygon_data *clock_poly = polygon_reference(line->clockwise_polygon_owner);
		polygon_data *counter_poly = polygon_reference(line->counterclockwise_polygon_owner);
		
		// fix invalid data forge leaves.
		if (!clock_poly) line->clockwise_polygon_side_index = NONE;
		if (!counter_poly) line->counterclockwise_polygon_side_index = NONE;
		
		side_data *clock_side = side_reference(line->clockwise_polygon_side_index);
		side_data *counter_side = side_reference(line->counterclockwise_polygon_side_index);
		
		if (!clock_poly) {
			// solid sides should never get to the point where they want to clip, but still...
			if (counter_side)
				counter_side->flags &= ~(_side_clips_top|_side_clips_bottom);
		} else if (!counter_poly) {
			if (clock_side)
				clock_side->flags &= ~(_side_clips_top|_side_clips_bottom);
		} else {
			// only silhouette edges clip.
			if (clock_poly->lowest_floor() < counter_poly->lowest_floor()) {
				if (!counter_side) {
					// clip here so we need a side to tell the view tree to generate a clip window
					counter_poly->create_side(i);
					counter_side = side_reference(line->counterclockwise_polygon_side_index);
					// and reload this guy incase it changed.
					if (clock_side)
						clock_side = side_reference(line->clockwise_polygon_side_index);
					assert(counter_side);
				}
				counter_side->flags |= _side_clips_bottom;
				if (clock_side)
					clock_side->flags &= ~_side_clips_bottom;
			} else if (clock_poly->lowest_floor() > counter_poly->lowest_floor()) {
				if (!clock_side) {
					// clip here so we need a side to tell the view tree to generate a clip window
					clock_poly->create_side(i);
					clock_side = side_reference(line->clockwise_polygon_side_index);
					// and reload this guy incase it changed.
					if (counter_side)
						counter_side = side_reference(line->counterclockwise_polygon_side_index);
					assert(clock_side);
				}
				clock_side->flags |= _side_clips_bottom;
				if (counter_side)
					counter_side->flags &= ~_side_clips_bottom;
			}
			
			if (clock_poly->highest_ceiling() > counter_poly->highest_ceiling()) {
				if (!counter_side) {
					// clip here so we need a side to tell the view tree to generate a clip window
					counter_poly->create_side(i);
					counter_side = side_reference(line->counterclockwise_polygon_side_index);
					// and reload this guy incase it changed.
					if (clock_side)
						clock_side = side_reference(line->clockwise_polygon_side_index);
					assert(counter_side);
				}
				counter_side->flags |= _side_clips_top;
				if (clock_side)
					clock_side->flags &= ~_side_clips_top;
			} else if (clock_poly->highest_ceiling() < counter_poly->highest_ceiling()) {
				if (!clock_side) {
					// clip here so we need a side to tell the view tree to generate a clip window
					clock_poly->create_side(i);
					clock_side = side_reference(line->clockwise_polygon_side_index);
					// and reload this guy incase it changed.
					if (counter_side)
						counter_side = side_reference(line->counterclockwise_polygon_side_index);
					assert(clock_side);
				}
				clock_side->flags |= _side_clips_top;
				if (counter_side)
					counter_side->flags &= ~_side_clips_top;
			}			
		}
		
		if (line->is_transparent()/* && line->highest_floor() < line->lowest_ceiling()*/)
		{
			if (clock_side)
				clock_side->flags |= _side_is_transparent;
			if (counter_side)
				counter_side->flags |= _side_is_transparent;
		} else if (clock_poly && counter_poly) {
			if (!counter_side) {
				// clip here so we need a side to tell the view tree to generate a clip window
				counter_poly->create_side(i);
				counter_side = side_reference(line->counterclockwise_polygon_side_index);
				assert(counter_side);
			}
			if (!clock_side) {
				// clip here so we need a side to tell the view tree to generate a clip window
				clock_poly->create_side(i);
				clock_side = side_reference(line->clockwise_polygon_side_index);
				assert(clock_side);
			}
			
			clock_side->flags &= ~(_side_is_transparent|_side_clips_top|_side_clips_bottom);
			counter_side->flags &= ~(_side_is_transparent|_side_clips_top|_side_clips_bottom);
		}	
	}
}

void calculate_endpoint_polygon_owners(
	short endpoint_index,
	short *first_index,
	short *index_count)
{
	struct polygon_data *polygon;
	short polygon_index;
	
	*first_index= dynamic_world->map_index_count;
	*index_count= 0;
	
	for (polygon_index= 0, polygon= map_polygons; polygon_index<dynamic_world->polygon_count; ++polygon_index, ++polygon)
	{
		short i;
		
		for (i= 0; i<polygon->vertex_count; ++i)
		{
			if (endpoint_index==polygon->endpoint_indexes[i])
			{
				add_map_index(polygon_index, index_count);
			}
		}
	}
}

void calculate_endpoint_line_owners(
	short endpoint_index,
	short *first_index,
	short *index_count)
{
	short line_index;
	struct line_data *line;
	
	*first_index= dynamic_world->map_index_count;
	*index_count= 0;
	
	for (line_index= 0, line= map_lines; line_index<dynamic_world->line_count; ++line_index, ++line)
	{
		// IR change: OOzing
		if (line->endpoint_0().index()==endpoint_index || line->endpoint_1().index()==endpoint_index)
		{
			add_map_index(line_index, index_count);
		}
	}
}

#define CONTINUOUS_SPLIT_SIDE_HEIGHT WORLD_ONE

void guess_side_lightsource_indexes(
	short side_index)
{
	struct side_data *side= get_side_data(side_index);
	struct line_data *line= get_line_data(side->line_index);
	struct polygon_data *polygon= get_polygon_data(side->polygon_index);
	
	switch (side->type)
	{
		// IR changes: B&B prep side effect
		case _full_side:
			side->primary_texture.lightsource_index= polygon->ceiling_surface.lightsource_index;
			break;
		case _split_side:
			side->secondary_texture.lightsource_index= (line->lowest_ceiling()-line->highest_floor()>CONTINUOUS_SPLIT_SIDE_HEIGHT) ?
				polygon->floor_surface.lightsource_index : polygon->ceiling_surface.lightsource_index;
			/* fall through to high side */
		case _high_side:
			side->primary_texture.lightsource_index= polygon->ceiling_surface.lightsource_index;
			break;
		case _low_side:
			side->primary_texture.lightsource_index= polygon->floor_surface.lightsource_index;
			break;
		
		default:
			assert(false);
			break;
	}
	
<<<<<<< map_constructors.cpp
	// IR change: B&B prep side effect
	side->transparent_texture.lightsource_index= polygon->ceiling_surface.lightsource_index;

	return;
=======
	side->transparent_lightsource_index= polygon->ceiling_lightsource_index;
>>>>>>> 1.11
}

/* Since the map_index buffer is no longer statically sized. */
void set_map_index_buffer_size(
	long length)
{
	map_index_buffer_count= length/sizeof(short);
}

/* ---------- private code */

/* given a polygon, return its endpoints in clockwise order; always returns polygon->vertex_count */
static short calculate_clockwise_endpoints(
	short polygon_index,
	short *buffer)
{
	struct polygon_data *polygon;
	short i;
	
	polygon= get_polygon_data(polygon_index);
	for (i=0;i<polygon->vertex_count;++i)
	{
		*buffer++= clockwise_endpoint_in_line(polygon_index, polygon->line_indexes[i], 0);
	}
	
	return polygon->vertex_count;
}

static void calculate_adjacent_sides(
	short polygon_index,
	short *side_indexes)
{
	struct polygon_data *polygon= get_polygon_data(polygon_index);
	short i;
	
	for (i=0;i<polygon->vertex_count;++i)
	{
		struct line_data *line= get_line_data(polygon->line_indexes[i]);
		short side_index;
		
		if (line->clockwise_polygon_owner==polygon_index)
		{
			side_index= line->clockwise_polygon_side_index;
		}
		else
		{
			// LP change: get around some Pfhorte bugs
			side_index= line->counterclockwise_polygon_side_index;
		}
		
		*side_indexes++= side_index;
	}
}

static void calculate_adjacent_polygons(
	short polygon_index,
	short *polygon_indexes)
{
	struct polygon_data *polygon;
	short i;
	
	polygon= get_polygon_data(polygon_index);
	for (i=0;i<polygon->vertex_count;++i)
	{
		struct line_data *line= get_line_data(polygon->line_indexes[i]);
		short adjacent_polygon_index= NONE;
		
		if (polygon_index==line->clockwise_polygon_owner)
		{
			adjacent_polygon_index= line->counterclockwise_polygon_owner;
		}
		else
		{
			// LP change: get around some Pfhorte bugs
			adjacent_polygon_index= line->clockwise_polygon_owner;
		}
		
		*polygon_indexes++= adjacent_polygon_index;
	}
}

/* returns area of the given polygon */
static long calculate_polygon_area(
	short polygon_index)
{
	short vertex;
	long area= 0;
	world_point2d *first_point, *point, *next_point;
	struct polygon_data *polygon= get_polygon_data(polygon_index);

	first_point= &get_endpoint_data(polygon->endpoint_indexes[0])->vertex;	
	for (vertex=1;vertex<polygon->vertex_count-1;++vertex)
	{
		point= &get_endpoint_data(polygon->endpoint_indexes[vertex])->vertex;
		next_point= &get_endpoint_data(polygon->endpoint_indexes[vertex+1])->vertex;

		area+= (first_point->x*point->y-point->x*first_point->y) +
			(point->x*next_point->y-next_point->x*point->y) +
			(next_point->x*first_point->y-first_point->x*next_point->y);
	}
	
	/* real area is absolute value of calculated area divided by two */
	area= ABS(area), area>>= 1;
	
	return area;
}

/* ---------- precalculate map indexes */

void precalculate_map_indexes(
	void)
{
	short polygon_index;
	struct polygon_data *polygon;
	
	for (polygon_index=0,polygon=map_polygons;polygon_index<dynamic_world->polygon_count;++polygon,++polygon_index)
	{
		if (!POLYGON_IS_DETACHED(polygon)) /* we’ll handle detached polygons during the second pass */
		{
			// short line_indexes[MAXIMUM_INTERSECTING_INDEXES], endpoint_indexes[MAXIMUM_INTERSECTING_INDEXES],
			// 	polygon_indexes[MAXIMUM_INTERSECTING_INDEXES];
			// short line_count, endpoint_count, polygon_count;
			short i;
	
//			if (polygon_index==17) dprintf("polygon #%d at %p", polygon_index, polygon);
						
			polygon->first_exclusion_zone_index= dynamic_world->map_index_count;
			polygon->line_exclusion_zone_count= polygon->point_exclusion_zone_count= 0;
			find_intersecting_endpoints_and_lines(polygon_index, MINIMUM_SEPARATION_FROM_WALL);
			//	line_indexes, &line_count, endpoint_indexes, &endpoint_count, polygon_indexes,
			//	&polygon_count);
			
			short line_count = LineIndices.size();
			short *line_indexes = &LineIndices[0];
			short endpoint_count = EndpointIndices.size();
			short *endpoint_indexes = &EndpointIndices[0];
			
			for (i=0;i<line_count;++i)	
			{
				add_map_index(line_indexes[i], &polygon->line_exclusion_zone_count);
			}
			
			for (i=0;i<endpoint_count;++i)
			{
				add_map_index(endpoint_indexes[i], &polygon->point_exclusion_zone_count);
			}
			
			polygon->first_neighbor_index= dynamic_world->map_index_count;
			polygon->neighbor_count= 0;
			find_intersecting_endpoints_and_lines(polygon_index, MINIMUM_SEPARATION_FROM_PROJECTILE);
			//	line_indexes, &line_count, endpoint_indexes, &endpoint_count, polygon_indexes,
			//	&polygon_count);
			
//			if (polygon_index==155) dprintf("polygon index #%d has %d neighbors:;dm %x %x;", polygon_index, polygon_count, polygon_indexes, sizeof(short)*polygon_count);
			
			short *polygon_indexes = &PolygonIndices[0];
			short polygon_count = PolygonIndices.size();

			for (i=0;i<polygon_count;++i)
			{
				add_map_index(polygon_indexes[i], &polygon->neighbor_count);
			}
		}
	}

	precalculate_polygon_sound_sources();
}

static void find_intersecting_endpoints_and_lines(
	short polygon_index,
	world_distance minimum_separation)
{
	struct intersecting_flood_data data;

	data.original_polygon_index= polygon_index;
	LineIndices.clear();
	EndpointIndices.clear();
	PolygonIndices.clear();

	data.minimum_separation_squared= minimum_separation*minimum_separation;
	find_center_of_polygon(polygon_index, &data.center);

	polygon_index= flood_map(polygon_index, LONG_MAX, intersecting_flood_proc, _breadth_first, &data);
	while (polygon_index!=NONE)
	{
		polygon_index= flood_map(NONE, LONG_MAX, intersecting_flood_proc, _breadth_first, &data);
	}
}

#ifdef NEW_AND_BROKEN
static long intersecting_flood_proc(
	short source_polygon_index,
	short line_index,
	short destination_polygon_index,
	void *vdata)
{
	struct intersecting_flood_data *data=vdata;
	struct polygon_data *polygon= get_polygon_data(source_polygon_index);
	struct polygon_data *original_polygon= get_polygon_data(data->original_polygon_index);
	bool keep_searching= false; /* don’t flood any deeper unless we find something close enough */
	short i, j;

	(void) (line_index,destination_polygon_index);

	/* we only care about this polygon if it intersects us in z */
	if (polygon->floor_height<=original_polygon->ceiling_height&&polygon->ceiling_height>=original_polygon->floor_height)
	{
		/* check each endpoint to see if it is within the critical distance of any line
			within our original polygon */
		for (i= 0; i<polygon->vertex_count; ++i)
		{
			short endpoint_index= polygon->endpoint_indexes[i];
			world_point2d *p= &get_endpoint_data(endpoint_index)->vertex;
			
			for (j= 0; j<original_polygon->vertex_count; ++j)
			{
				short line_index= polygon->line_indexes[i];
				struct line_data *line= get_line_data(line_index);
				
				if (point_to_line_segment_distance_squared(p, a, b)<data->minimum_separation_squared)
				{
					keep_searching|= try_and_add_endpoint(endpoint_index);
					keep_searching|= try_and_add_line(polygon->line_indexes[i]);
					keep_searching|= try_and_add_line(polygon->line_indexes[i?i-1:polygon->vertex_count-1);
					break;
				}
			}
		}
	}

	/* if any part of this polygon is close enough to our original polygon, remember it’s index */
	if (keep_searching)
	{
		for (j=0;j<data->polygon_count;++j)
		{
			if (data->polygon_indexes[j]==source_polygon_index)
			{
				break; /* found duplicate, ignore */
			}
		}
		if (j==data->polygon_count && data->polygon_count<MAXIMUM_INTERSECTING_INDEXES)
		{
			short detached_twin_index= NONE; //find_undetached_polygons_twin(source_polygon_index);
			
			if (DoIncorrectCountVWarn)
				vwarn(data->polygon_count!=MAXIMUM_INTERSECTING_INDEXES-1, csprintf(temporary, "incomplete neighbor list for polygon#%d", data->original_polygon_index));
			data->polygon_indexes[data->polygon_count++]= source_polygon_index;
			
			/* if this polygon has a detached twin, add it too */
			if (detached_twin_index!=NONE && data->polygon_count<MAXIMUM_INTERSECTING_INDEXES)
			{
				if (DoIncorrectCountVWarn)
					vwarn(data->polygon_count!=MAXIMUM_INTERSECTING_INDEXES-1, csprintf(temporary, "incomplete neighbor list for polygon#%d", data->original_polygon_index));
				data->polygon_indexes[data->polygon_count++]= detached_twin_index;
			}
		}
	}

	/* return area of source polygon as cost */
	return keep_searching ? 1 : -1;
}

void try_and_add_line(
	struct intersecting_flood_data *data,
	short line_index)
{
	struct polygon_data *original_polygon= get_polygon_data(data->original_polygon_index);
	struct line_data *line= get_line_data(line_index);
	bool keep_searching= false;
	short i;
	
		// IR change: OOzing	
	if (line->is_solid() ||
		line_has_variable_height(line_index) ||
		line->lowest_adjacent_ceiling<original_polygon->ceiling_height ||
		line->highest_adjacent_floor>original_polygon->floor_height)
	{
		/* make sure this line isn’t already in the line list */
		for (i=0; i<data->line_count; ++i)
		{
			if (data->line_indexes[i]==line_index)
			{
				keep_searching= true;
				break; /* found duplicate, ignore (but keep looking for others) */
			}
		}
		if (i==data->line_count && data->line_count<MAXIMUM_INTERSECTING_INDEXES)
		{
			bool clockwise= ((b->x-a->x)*(data->center.y-b->y) - (b->y-a->y)*(data->center.x-b->x)>0) ? true : false;

			if (DoIncorrectCountVWarn)
				vwarn(data->line_count!=MAXIMUM_INTERSECTING_INDEXES-1, csprintf(temporary, "incomplete line list for polygon#%d", data->original_polygon_index));
			data->line_indexes[data->line_count++]= clockwise ? polygon->line_indexes[i] : (-polygon->line_indexes[i]-1);
//			if (data->original_polygon_index==23) dprintf("found line %d (%s)", polygon->line_indexes[i], clockwise ? "clockwise" : "counterclockwise");
			keep_searching= true;
			break;
		}
	}
	
	return keep_searching;
}

#if 0
	{
#endif
			
			/* add this endpoint if it isn’t already in the intersecting endpoint list */
			for (j=0;j<data->endpoint_count;++j)
			{
				if (data->endpoint_indexes[j]==polygon->endpoint_indexes[i])
				{
					keep_searching= true;
					break; /* found duplicate, ignore (but keep looking for others) */
				}
			}
			if (j==data->endpoint_count && data->endpoint_count<MAXIMUM_INTERSECTING_INDEXES)
			{
				world_point2d *p= &get_endpoint_data(polygon->endpoint_indexes[i])->vertex;
				
				/* check and see if this endpoint is close enough to any line in our original polygon
					to care about; if it is, add it to our list */
				for (j=0;j<original_polygon->vertex_count;++j)
				{
					struct line_data *line= get_line_data(original_polygon->line_indexes[j]);
					world_point2d *a= &get_endpoint_data(line->endpoint_indexes[0])->vertex;
					world_point2d *b= &get_endpoint_data(line->endpoint_indexes[1])->vertex;
		
					if (point_to_line_segment_distance_squared(p, a, b)<data->minimum_separation_squared)
					{
						if (DoIncorrectCountVWarn)
							vwarn(data->endpoint_count!=MAXIMUM_INTERSECTING_INDEXES-1, csprintf(temporary, "incomplete endpoint list for polygon#%d", data->original_polygon_index));
						data->endpoint_indexes[data->endpoint_count++]= polygon->endpoint_indexes[i];
//						if (data->original_polygon_index==23) dprintf("found endpoint %d", data->endpoint_indexes[data->endpoint_count-1]);
//						switch (data->endpoint_indexes[data->endpoint_count-1])
//						{
//							case 34:
//							case 35:
//								dprintf("found endpoint#%d from polygon#%d", data->endpoint_indexes[data->endpoint_count-1], data->original_polygon_index);
//						}
						break;
					}
				}
			}
		}
	}
#endif

static long intersecting_flood_proc(
	short source_polygon_index,
	short line_index,
	short destination_polygon_index,
	void *vdata)
{
	struct intersecting_flood_data *data=(struct intersecting_flood_data *)vdata;
	struct polygon_data *polygon= get_polygon_data(source_polygon_index);
	struct polygon_data *original_polygon= get_polygon_data(data->original_polygon_index);
	bool keep_searching= false; /* don’t flood any deeper unless we find something close enough */
	unsigned i, j;

	(void) (line_index);
	(void) (destination_polygon_index);

	/* we only care about this polygon if it intersects us in z */
	// IR change: B&B prep side effect
	if ( polygon->lowest_floor() <= original_polygon->highest_ceiling() && 
	     polygon->highest_ceiling() >= original_polygon->lowest_floor())
	{
		/* update our running line and endpoint lists */	
		for (i=0;i<polygon->vertex_count;++i)
		{
			/* add this line if it isn’t already in the intersecting line list */
			for (j=0;j<LineIndices.size();++j)
			{
				if (LineIndices[j]==polygon->line_indexes[i] ||
					-LineIndices[j]-1==polygon->line_indexes[i])
				{
					keep_searching= true;
					break; /* found duplicate, stop */
				}
			}
			if (j==LineIndices.size())
			{
				short line_index= polygon->line_indexes[i];
				struct line_data *line= get_line_data(line_index);
				
				// IR changes: BYB prep side effects & OOzing				
				if (line->is_solid() ||
					line_has_variable_height(line_index) ||
					line->lowest_ceiling() < original_polygon->highest_ceiling() ||
					line->highest_floor() > original_polygon->lowest_floor())
				{
					world_point2d *a= &line->endpoint_0()->vertex;
					world_point2d *b= &line->endpoint_1()->vertex;
		
					/* check and see if this line is close enough to any point in our original polygon
						to care about; if it is, add it to our list */
					for (j=0;j<original_polygon->vertex_count;++j)
					{
						world_point2d *p= &get_endpoint_data(original_polygon->endpoint_indexes[j])->vertex;
			
						if (point_to_line_segment_distance_squared(p, a, b)<data->minimum_separation_squared)
						{
							bool clockwise= ((b->x-a->x)*(data->center.y-b->y) - (b->y-a->y)*(data->center.x-b->x)>0) ? true : false;
							
							LineIndices.push_back(clockwise ? polygon->line_indexes[i] : (-polygon->line_indexes[i]-1));
							keep_searching= true;
							break;
						}
					}
				}
			}
			
			/* add this endpoint if it isn’t already in the intersecting endpoint list */
			for (j=0;j<EndpointIndices.size();++j)
			{
				if (EndpointIndices[j]==polygon->endpoint_indexes[i])
				{
					keep_searching= true;
					break; /* found duplicate, ignore (but keep looking for others) */
				}
			}
			if (j==EndpointIndices.size())
			{
				world_point2d *p= &get_endpoint_data(polygon->endpoint_indexes[i])->vertex;
				
				/* check and see if this endpoint is close enough to any line in our original polygon
					to care about; if it is, add it to our list */
				for (j=0;j<original_polygon->vertex_count;++j)
				{
					struct line_data *line= get_line_data(original_polygon->line_indexes[j]);
					// IR change: OOzing
					world_point2d *a= &line->endpoint_0()->vertex;
					world_point2d *b= &line->endpoint_1()->vertex;
		
					if (point_to_line_segment_distance_squared(p, a, b)<data->minimum_separation_squared)
					{
						EndpointIndices.push_back(polygon->endpoint_indexes[i]);
						break;
					}
				}
			}
		}
	}

	/* if any part of this polygon is close enough to our original polygon, remember it’s index */
	if (keep_searching)
	{
		for (j=0;j<PolygonIndices.size();++j)
		{
			if (PolygonIndices[j]==source_polygon_index)
			{
				break; /* found duplicate, ignore */
			}
		}
		if (j==PolygonIndices.size())
		{
			short detached_twin_index= NONE; //find_undetached_polygons_twin(source_polygon_index);
			
			PolygonIndices.push_back(source_polygon_index);
			
			/* if this polygon has a detached twin, add it too */
			if (detached_twin_index!=NONE)
			{
				PolygonIndices.push_back(detached_twin_index);
			}
		}
	}

	/* return area of source polygon as cost */
	return keep_searching ? 1 : -1;
}


#ifdef WITH_ORIGINAL_DATA_STRUCTURES
static long intersecting_flood_proc(
	short source_polygon_index,
	short line_index,
	short destination_polygon_index,
	void *vdata)
{
	struct intersecting_flood_data *data=(struct intersecting_flood_data *)vdata;
	struct polygon_data *polygon= get_polygon_data(source_polygon_index);
	struct polygon_data *original_polygon= get_polygon_data(data->original_polygon_index);
	bool keep_searching= false; /* don’t flood any deeper unless we find something close enough */
	short i, j;

	(void) (line_index);
	(void) (destination_polygon_index);

	/* we only care about this polygon if it intersects us in z */
	if (polygon->floor_height<=original_polygon->ceiling_height&&polygon->ceiling_height>=original_polygon->floor_height)
	{
		/* update our running line and endpoint lists */	
		for (i=0;i<polygon->vertex_count;++i)
		{
			/* add this line if it isn’t already in the intersecting line list */
			for (j=0;j<data->line_count;++j)
			{
				if (data->line_indexes[j]==polygon->line_indexes[i] ||
					-data->line_indexes[j]-1==polygon->line_indexes[i])
				{
					keep_searching= true;
					break; /* found duplicate, stop */
				}
			}
			if (j==data->line_count && data->endpoint_count<MAXIMUM_INTERSECTING_INDEXES)
			{
				short line_index= polygon->line_indexes[i];
				struct line_data *line= get_line_data(line_index);
				
//				if (data->original_polygon_index==23&&line_index==104) dprintf("line#%d @ %p", line_index, line);
				
				// IR change: OOzing & B&B prep side effect
				if (line->is_solid() ||
					line_has_variable_height(line_index) ||
					line->lowest_ceiling()<original_polygon->ceiling_height ||
					line->highest_floor()>original_polygon->floor_height)
				{
					world_point2d *a= &get_endpoint_data(line->endpoint_indexes[0])->vertex;
					world_point2d *b= &get_endpoint_data(line->endpoint_indexes[1])->vertex;
		
					/* check and see if this line is close enough to any point in our original polygon
						to care about; if it is, add it to our list */
					for (j=0;j<original_polygon->vertex_count;++j)
					{
						world_point2d *p= &get_endpoint_data(original_polygon->endpoint_indexes[j])->vertex;
			
						if (point_to_line_segment_distance_squared(p, a, b)<data->minimum_separation_squared)
						{
							bool clockwise= ((b->x-a->x)*(data->center.y-b->y) - (b->y-a->y)*(data->center.x-b->x)>0) ? true : false;
							
							if (DoIncorrectCountVWarn)
								vwarn(data->line_count!=MAXIMUM_INTERSECTING_INDEXES-1, csprintf(temporary, "incomplete line list for polygon#%d", data->original_polygon_index));
							data->line_indexes[data->line_count++]= clockwise ? polygon->line_indexes[i] : (-polygon->line_indexes[i]-1);
//							if (data->original_polygon_index==23) dprintf("found line %d (%s)", polygon->line_indexes[i], clockwise ? "clockwise" : "counterclockwise");
							keep_searching= true;
							break;
						}
					}
				}
			}
			
			/* add this endpoint if it isn’t already in the intersecting endpoint list */
			for (j=0;j<data->endpoint_count;++j)
			{
				if (data->endpoint_indexes[j]==polygon->endpoint_indexes[i])
				{
					keep_searching= true;
					break; /* found duplicate, ignore (but keep looking for others) */
				}
			}
			if (j==data->endpoint_count && data->endpoint_count<MAXIMUM_INTERSECTING_INDEXES)
			{
				world_point2d *p= &get_endpoint_data(polygon->endpoint_indexes[i])->vertex;
				
				/* check and see if this endpoint is close enough to any line in our original polygon
					to care about; if it is, add it to our list */
				for (j=0;j<original_polygon->vertex_count;++j)
				{
					struct line_data *line= get_line_data(original_polygon->line_indexes[j]);
					world_point2d *a= &get_endpoint_data(line->endpoint_indexes[0])->vertex;
					world_point2d *b= &get_endpoint_data(line->endpoint_indexes[1])->vertex;
		
					if (point_to_line_segment_distance_squared(p, a, b)<data->minimum_separation_squared)
					{
						if (DoIncorrectCountVWarn)
							vwarn(data->endpoint_count!=MAXIMUM_INTERSECTING_INDEXES-1, csprintf(temporary, "incomplete endpoint list for polygon#%d", data->original_polygon_index));
						data->endpoint_indexes[data->endpoint_count++]= polygon->endpoint_indexes[i];
//						if (data->original_polygon_index==23) dprintf("found endpoint %d", data->endpoint_indexes[data->endpoint_count-1]);
//						switch (data->endpoint_indexes[data->endpoint_count-1])
//						{
//							case 34:
//							case 35:
//								dprintf("found endpoint#%d from polygon#%d", data->endpoint_indexes[data->endpoint_count-1], data->original_polygon_index);
//						}
						break;
					}
				}
			}
		}
	}

	/* if any part of this polygon is close enough to our original polygon, remember it’s index */
	if (keep_searching)
	{
		for (j=0;j<data->polygon_count;++j)
		{
			if (data->polygon_indexes[j]==source_polygon_index)
			{
				break; /* found duplicate, ignore */
			}
		}
		if (j==data->polygon_count && data->polygon_count<MAXIMUM_INTERSECTING_INDEXES)
		{
			short detached_twin_index= NONE; //find_undetached_polygons_twin(source_polygon_index);
			
			if (DoIncorrectCountVWarn)
				vwarn(data->polygon_count!=MAXIMUM_INTERSECTING_INDEXES-1, csprintf(temporary, "incomplete neighbor list for polygon#%d", data->original_polygon_index));
			data->polygon_indexes[data->polygon_count++]= source_polygon_index;
			
			/* if this polygon has a detached twin, add it too */
			if (detached_twin_index!=NONE && data->polygon_count<MAXIMUM_INTERSECTING_INDEXES)
			{
				if (DoIncorrectCountVWarn)
					vwarn(data->polygon_count!=MAXIMUM_INTERSECTING_INDEXES-1, csprintf(temporary, "incomplete neighbor list for polygon#%d", data->original_polygon_index));
				data->polygon_indexes[data->polygon_count++]= detached_twin_index;
			}
		}
	}

	/* return area of source polygon as cost */
	return keep_searching ? 1 : -1;
}
#endif

static void add_map_index(
	short index,
	short *count)
{
	MapIndexList.push_back(index);
	dynamic_world->map_index_count++;
	*count += 1;
}

#define ZERO_VOLUME_DISTANCE (10*WORLD_ONE)

static void precalculate_polygon_sound_sources(
	void)
{
	short polygon_index;
	struct polygon_data *polygon;
	
	for (polygon_index= 0, polygon= map_polygons; polygon_index<dynamic_world->polygon_count; ++polygon_index, ++polygon)
	{
		short object_index;
		struct map_object *object;
		short sound_sources= 0;
		
		polygon->sound_source_indexes= dynamic_world->map_index_count;
		
		for (object_index= 0, object= saved_objects; object_index<dynamic_world->initial_objects_count; ++object, ++object_index)
		{
			if (object->type==_saved_sound_source)
			{
				short i;
				bool close= false;
				
				for (i= 0; i<polygon->vertex_count; ++i)
				{
					struct endpoint_data *endpoint= get_endpoint_data(polygon->endpoint_indexes[i]);
					struct line_data *line= get_line_data(polygon->line_indexes[i]);
					
					if (guess_distance2d((world_point2d *)&object->location, &endpoint->vertex)<ZERO_VOLUME_DISTANCE ||
						point_to_line_segment_distance_squared((world_point2d *)&object->location,
							// IR change: OOzing
							&line->endpoint_0()->vertex,
							&line->endpoint_1()->vertex)<ZERO_VOLUME_DISTANCE)
					{
						close= true;
						break;
					}
				}
				
				if (close) add_map_index(object_index, &sound_sources);
			}
		}
		
		add_map_index(NONE, &sound_sources);
	}
<<<<<<< map_constructors.cpp
	
	return;
}

#ifdef OBSOLETE
void touch_polygon(
	short polygon_index)
{
	struct polygon_data *polygon= get_polygon_data(polygon_index);

	if (!POLYGON_IS_DETACHED(polygon))
	{
		short i;
		
		recalculate_redundant_polygon_data(polygon_index);
		
		for (i=0;i<polygon->vertex_count;++i)
		{
			recalculate_redundant_line_data(polygon->line_indexes[i]); /* does sides */
			// IR change: OOzing
			endpoint_reference(polygon->endpoint_indexes[i])->recalculate_redundant_data();
		}
	}
	
	return;
=======
>>>>>>> 1.11
}

uint8 *unpack_endpoint_data(uint8 *Stream, endpoint_data *Objects, int Count)
{
	uint8* S = Stream;
	endpoint_data* ObjPtr = Objects;
	
	for (int k = 0; k < Count; k++, ObjPtr++)
	{
		// IR change: moved this into a method.
		S = ObjPtr->unpack(S);
	}
	
	assert((S - Stream) == Count*SIZEOF_endpoint_data);
	return S;
}

// IR addition: OOzing
uint8* endpoint_data::unpack(uint8 *S)
{
	StreamToValue(S,flags);
	StreamToValue(S,highest_adjacent_floor_height);
	StreamToValue(S,lowest_adjacent_ceiling_height);
	
	StreamToValue(S,vertex.x);
	StreamToValue(S,vertex.y);
	short tempload;
	StreamToValue(S,tempload);
	transformedL.x = (int32(tempload)&0x0000FFFF) | (((flags&0xF000)<<16)>>12);
	StreamToValue(S,tempload);
	transformedL.y = (int32(tempload)&0x0000FFFF) | (((flags&0x0F00)<<20)>>12);
	flags &= 0x00FF;
	
	StreamToValue(S,supporting_polygon_index);
	
	return S;
}

uint8 *pack_endpoint_data(uint8 *Stream, endpoint_data *Objects, int Count)
{
	uint8* S = Stream;
	endpoint_data* ObjPtr = Objects;
	
	for (int k = 0; k < Count; k++, ObjPtr++)
	{
		// IR change: moved this into a method.
		S = ObjPtr->pack(S);
	}
	
	assert((S - Stream) == Count*SIZEOF_endpoint_data);
	return S;
}

// IR addition: OOzing
uint8* endpoint_data::pack(uint8 *S)
{
	uint16 extFlags = flags&0x00FF;
	extFlags |= (transformedL.x>4)&0xF000;
	extFlags |= (transformedL.y>8)&0x0F00;
	
	ValueToStream(S,extFlags);
	ValueToStream(S,highest_adjacent_floor_height);
	ValueToStream(S,lowest_adjacent_ceiling_height);
	
	ValueToStream(S,vertex.x);
	ValueToStream(S,vertex.y);
	ValueToStream(S,int16(transformedL.x));
	ValueToStream(S,int16(transformedL.y));
	
	ValueToStream(S,supporting_polygon_index);
	
	return S;
}

uint8 *unpack_line_data(uint8 *Stream, line_data *Objects, int Count)
{
	uint8* S = Stream;
	line_data* ObjPtr = Objects;
	
	for (int k = 0; k < Count; k++, ObjPtr++)
	{
		// IR change: moved this into a method.
		S = ObjPtr->unpack(S);
	}
	
	assert((S - Stream) == Count*SIZEOF_line_data);
	return S;
}

// IR addition: OOzing
uint8* line_data::unpack(uint8 *S)
{
	StreamToList(S,endpoint_indexes,2);
	StreamToValue(S,flags);
	
	StreamToValue(S,length);
	StreamToValue(S,highest_adjacent_floor);
	StreamToValue(S,lowest_adjacent_ceiling);
	
	StreamToValue(S,clockwise_polygon_side_index);
	StreamToValue(S,counterclockwise_polygon_side_index);
	
	StreamToValue(S,clockwise_polygon_owner);
	StreamToValue(S,counterclockwise_polygon_owner);
	
	S += 6*2; // unused data
	return S;
}

uint8 *pack_line_data(uint8 *Stream, line_data *Objects, int Count)
{
	uint8* S = Stream;
	line_data* ObjPtr = Objects;
	
	for (int k = 0; k < Count; k++, ObjPtr++)
	{
		// IR change: moved this into a method.
		S = ObjPtr->pack(S);
	}
	
	assert((S - Stream) == Count*SIZEOF_line_data);
	return S;
}

// IR addition: OOzing
uint8* line_data::pack(uint8 *S)
{
	ListToStream(S,endpoint_indexes,2);
	ValueToStream(S,flags);
	
	ValueToStream(S,length);
	ValueToStream(S,highest_adjacent_floor);
	ValueToStream(S,lowest_adjacent_ceiling);
	
	ValueToStream(S,clockwise_polygon_side_index);
	ValueToStream(S,counterclockwise_polygon_side_index);
	
	ValueToStream(S,clockwise_polygon_owner);
	ValueToStream(S,counterclockwise_polygon_owner);
	
	S += 6*2;
	return S;
}

inline void StreamToSideTxtr(uint8* &S, side_texture_definition& Object)
{
	// IR change: B&B prep
	StreamToValue(S,Object.origin.x);
	StreamToValue(S,Object.origin.y);
	StreamToValue(S,Object.texture);
	/* if (map_version.has_new_side_texture_struct) {
		StreamToValue(S,Object.transfer_mode);
		StreamToValue(S,Object.transfer_mode);
		StreamToValue(S,Object.transfer_mode);
	}*/
}

inline void SideTxtrToStream(uint8* &S, side_texture_definition& Object)
{
	// IR change: B&B prep
	ValueToStream(S,Object.origin.x);
	ValueToStream(S,Object.origin.y);
	ValueToStream(S,Object.texture);	
	/* if (map_version.has_new_side_texture_struct) {
		ValueToStream(S,Object.transfer_mode);
		ValueToStream(S,Object.transfer_mode);
		ValueToStream(S,Object.transfer_mode);
	}*/
}


void StreamToSideExclZone(uint8* &S, side_exclusion_zone& Object);
void StreamToSideExclZone(uint8* &S, side_exclusion_zone& Object)
{
	StreamToValue(S,Object.e0.x);
	StreamToValue(S,Object.e0.y);
	StreamToValue(S,Object.e1.x);
	StreamToValue(S,Object.e1.y);
	StreamToValue(S,Object.e2.x);
	StreamToValue(S,Object.e2.y);
	StreamToValue(S,Object.e3.x);
	StreamToValue(S,Object.e3.y);
}

void SideExclZoneToStream(uint8* &S, side_exclusion_zone& Object);
void SideExclZoneToStream(uint8* &S, side_exclusion_zone& Object)
{
	ValueToStream(S,Object.e0.x);
	ValueToStream(S,Object.e0.y);
	ValueToStream(S,Object.e1.x);
	ValueToStream(S,Object.e1.y);
	ValueToStream(S,Object.e2.x);
	ValueToStream(S,Object.e2.y);
	ValueToStream(S,Object.e3.x);
	ValueToStream(S,Object.e3.y);
}


uint8 *unpack_side_data(uint8 *Stream, side_data *Objects, int Count)
{
	uint8* S = Stream;
	side_data* ObjPtr = Objects;
	
	for (int k = 0; k < Count; k++, ObjPtr++)
	{
		StreamToValue(S,ObjPtr->type);
		StreamToValue(S,ObjPtr->flags);
		// if (!map_version.has_side_clip_data)
		ObjPtr->flags &= ~_editor_dirty_bit; // this bit is never accessed.  Now used as part of the clip data.
		
		StreamToSideTxtr(S,ObjPtr->primary_texture);
		StreamToSideTxtr(S,ObjPtr->secondary_texture);
		StreamToSideTxtr(S,ObjPtr->transparent_texture);
		
		// if (map_version.has_exclusion_zones)
		StreamToSideExclZone(S,ObjPtr->exclusion_zone);
		
		/*if ( map_version.has_portals && (flags & _side_is_portal)) {
			StreamToValue(S,ObjPtr->data.portal.portal);
			StreamToValue(S,ObjPtr->data.portal.destination_polygon);
		} else {*/
			StreamToValue(S,ObjPtr->data.control_panel.type);
			StreamToValue(S,ObjPtr->data.control_panel.permutation);
		//}
		
		// if (!map_version.has_new_side_texture_struct) {
			StreamToValue(S,ObjPtr->primary_texture.transfer_mode);
			StreamToValue(S,ObjPtr->secondary_texture.transfer_mode);
			StreamToValue(S,ObjPtr->transparent_texture.transfer_mode);
		// }
		
		StreamToValue(S,ObjPtr->polygon_index);
		StreamToValue(S,ObjPtr->line_index);
		
		// if (!map_version.has_new_side_texture_struct) {
			StreamToValue(S,ObjPtr->primary_texture.lightsource_index);
			StreamToValue(S,ObjPtr->secondary_texture.lightsource_index);
			StreamToValue(S,ObjPtr->transparent_texture.lightsource_index);
		// }
		
		StreamToValue(S,ObjPtr->ambient_delta);
		
		S += 1*2/*map_version.unused_side_bytes*/;
	}
	
	assert((S - Stream) == Count*SIZEOF_side_data/*map_version.sizeof_side*/);
	return S;
}

uint8 *pack_side_data(uint8 *Stream, side_data *Objects, int Count)
{
	uint8* S = Stream;
	side_data* ObjPtr = Objects;
	
	for (int k = 0; k < Count; k++, ObjPtr++)
	{
		ValueToStream(S,ObjPtr->type);
		// if (!map_version.has_side_clip_data)
		ObjPtr->flags &= ~(_side_clips_top|_side_clips_bottom);
		ValueToStream(S,ObjPtr->flags);
		
		SideTxtrToStream(S,ObjPtr->primary_texture);
		SideTxtrToStream(S,ObjPtr->secondary_texture);
		SideTxtrToStream(S,ObjPtr->transparent_texture);
		
		SideExclZoneToStream(S,ObjPtr->exclusion_zone);
		
		/*if ( map_version.has_portals && (flags & _side_is_portal)) {
			ValueToStream(S,ObjPtr->data.portal.portal);
			ValueToStream(S,ObjPtr->data.portal.destination_polygon);
		} else {*/
			ValueToStream(S,ObjPtr->data.control_panel.type);
			ValueToStream(S,ObjPtr->data.control_panel.permutation);
		//}
		
		// if (!map_version.has_new_side_texture_struct) {
			ValueToStream(S,ObjPtr->primary_texture.transfer_mode);
			ValueToStream(S,ObjPtr->secondary_texture.transfer_mode);
			ValueToStream(S,ObjPtr->transparent_texture.transfer_mode);
		//}
		
		ValueToStream(S,ObjPtr->polygon_index);
		ValueToStream(S,ObjPtr->line_index);
		
		// if (!map_version.has_new_side_texture_struct) {
			ValueToStream(S,ObjPtr->primary_texture.lightsource_index);
			ValueToStream(S,ObjPtr->secondary_texture.lightsource_index);
			ValueToStream(S,ObjPtr->transparent_texture.lightsource_index);
		//}
		
		ValueToStream(S,ObjPtr->ambient_delta);
		
		S += 1*2/*map_version.unused_side_bytes*/;
	}
	
	assert((S - Stream) == Count*SIZEOF_side_data/*map_version.sizeof_side*/);
	return S;
}


uint8 *unpack_polygon_data(uint8 *Stream, polygon_data *Objects, int Count)
{
	uint8* S = Stream;
	polygon_data* ObjPtr = Objects;
	
	for (int k = 0; k < Count; k++, ObjPtr++)
	{
		// IR change: moved this into a method.
		S = ObjPtr->unpack(S);
	}
	
	assert((S - Stream) == Count*SIZEOF_polygon_data);
	return S;
}

// IR addition: OOzing
uint8* polygon_data::unpack(uint8* S)
{
	StreamToValue(S,type);
	StreamToValue(S,flags);
	StreamToValue(S,permutation);
	
	StreamToValue(S,vertex_count);
	StreamToList(S,endpoint_indexes,MAXIMUM_VERTICES_PER_POLYGON);
	StreamToList(S,line_indexes,MAXIMUM_VERTICES_PER_POLYGON);
	
	StreamToValue(S,floor_surface.texture);
	StreamToValue(S,ceiling_surface.texture);
	StreamToValue(S,floor_surface.height);
	StreamToValue(S,ceiling_surface.height);
	StreamToValue(S,floor_surface.lightsource_index);
	StreamToValue(S,ceiling_surface.lightsource_index);
	
	StreamToValue(S,area);
	
	StreamToValue(S,first_object);
	
	StreamToValue(S,first_exclusion_zone_index);
	StreamToValue(S,line_exclusion_zone_count);
	StreamToValue(S,point_exclusion_zone_count);
	
	StreamToValue(S,floor_surface.transfer_mode);
	StreamToValue(S,ceiling_surface.transfer_mode);
	
	StreamToList(S,adjacent_polygon_indexes,MAXIMUM_VERTICES_PER_POLYGON);
	
	StreamToValue(S,first_neighbor_index);
	StreamToValue(S,neighbor_count);
	
	StreamToValue(S,center.x);
	StreamToValue(S,center.y);
	
	StreamToList(S,side_indexes,MAXIMUM_VERTICES_PER_POLYGON);
	
	StreamToValue(S,floor_surface.origin.x);
	StreamToValue(S,floor_surface.origin.y);
	StreamToValue(S,ceiling_surface.origin.x);
	StreamToValue(S,ceiling_surface.origin.y);
	
	StreamToValue(S,media_index);
	StreamToValue(S,media_lightsource_index);
	
	StreamToValue(S,sound_source_indexes);
	
	StreamToValue(S,ambient_sound_image_index);
	StreamToValue(S,random_sound_image_index);
	
	// unused	
	S += 1*2;
	return S;
}

uint8 *pack_polygon_data(uint8 *Stream, polygon_data *Objects, int Count)
{
	uint8* S = Stream;
	polygon_data* ObjPtr = Objects;
	
	for (int k = 0; k < Count; k++, ObjPtr++)
	{
		// IR change: moved this into a method.
		S = ObjPtr->pack(S);
	}
	
	assert((S - Stream) == Count*SIZEOF_polygon_data);
	return S;
}

// IR addition: OOzing
uint8* polygon_data::pack(uint8* S) {
	ValueToStream(S,type);
	ValueToStream(S,flags);
	ValueToStream(S,permutation);
	
	ValueToStream(S,vertex_count);
	ListToStream(S,endpoint_indexes,MAXIMUM_VERTICES_PER_POLYGON);
	ListToStream(S,line_indexes,MAXIMUM_VERTICES_PER_POLYGON);
	
	ValueToStream(S,floor_surface.texture);
	ValueToStream(S,ceiling_surface.texture);
	ValueToStream(S,floor_surface.height);
	ValueToStream(S,ceiling_surface.height);
	ValueToStream(S,floor_surface.lightsource_index);
	ValueToStream(S,ceiling_surface.lightsource_index);
	
	ValueToStream(S,area);
	
	ValueToStream(S,first_object);
	
	ValueToStream(S,first_exclusion_zone_index);
	ValueToStream(S,line_exclusion_zone_count);
	ValueToStream(S,point_exclusion_zone_count);
	
	ValueToStream(S,floor_surface.transfer_mode);
	ValueToStream(S,ceiling_surface.transfer_mode);
	
	ListToStream(S,adjacent_polygon_indexes,MAXIMUM_VERTICES_PER_POLYGON);
	
	ValueToStream(S,first_neighbor_index);
	ValueToStream(S,neighbor_count);
	
	ValueToStream(S,center.x);
	ValueToStream(S,center.y);
	
	ListToStream(S,side_indexes,MAXIMUM_VERTICES_PER_POLYGON);
	
	ValueToStream(S,floor_surface.origin.x);
	ValueToStream(S,floor_surface.origin.y);
	ValueToStream(S,ceiling_surface.origin.x);
	ValueToStream(S,ceiling_surface.origin.y);
	
	ValueToStream(S,media_index);
	ValueToStream(S,media_lightsource_index);
	
	ValueToStream(S,sound_source_indexes);
	
	ValueToStream(S,ambient_sound_image_index);
	ValueToStream(S,random_sound_image_index);
	
	S += 1*2;
	return S;
}

uint8 *unpack_map_annotation(uint8 *Stream, map_annotation* Objects, int Count)
{
	uint8* S = Stream;
	map_annotation* ObjPtr = Objects;
	
	for (int k = 0; k < Count; k++, ObjPtr++)
	{
		StreamToValue(S,ObjPtr->type);
		
		StreamToValue(S,ObjPtr->location.x);
		StreamToValue(S,ObjPtr->location.y);
		StreamToValue(S,ObjPtr->polygon_index);
		
		StreamToBytes(S,ObjPtr->text,MAXIMUM_ANNOTATION_TEXT_LENGTH);
	}
	
	assert((S - Stream) == Count*SIZEOF_map_annotation);
	return S;
}

uint8 *pack_map_annotation(uint8 *Stream, map_annotation* Objects, int Count)
{
	uint8* S = Stream;
	map_annotation* ObjPtr = Objects;
	
	for (int k = 0; k < Count; k++, ObjPtr++)
	{
		ValueToStream(S,ObjPtr->type);
		
		ValueToStream(S,ObjPtr->location.x);
		ValueToStream(S,ObjPtr->location.y);
		ValueToStream(S,ObjPtr->polygon_index);
		
		BytesToStream(S,ObjPtr->text,MAXIMUM_ANNOTATION_TEXT_LENGTH);
	}
	
	assert((S - Stream) == Count*SIZEOF_map_annotation);
	return S;
}


uint8 *unpack_map_object(uint8 *Stream, map_object* Objects, int Count)
{
	uint8* S = Stream;
	map_object* ObjPtr = Objects;
	
	for (int k = 0; k < Count; k++, ObjPtr++)
	{
		StreamToValue(S,ObjPtr->type);
		StreamToValue(S,ObjPtr->index);
		StreamToValue(S,ObjPtr->facing);
		StreamToValue(S,ObjPtr->polygon_index);
		StreamToValue(S,ObjPtr->location.x);
		StreamToValue(S,ObjPtr->location.y);
		StreamToValue(S,ObjPtr->location.z);
		
		StreamToValue(S,ObjPtr->flags);
	}
	
	assert((S - Stream) == Count*SIZEOF_map_object);
	return S;
}

uint8 *pack_map_object(uint8 *Stream, map_object* Objects, int Count)
{
	uint8* S = Stream;
	map_object* ObjPtr = Objects;
	
	for (int k = 0; k < Count; k++, ObjPtr++)
	{
		ValueToStream(S,ObjPtr->type);
		ValueToStream(S,ObjPtr->index);
		ValueToStream(S,ObjPtr->facing);
		ValueToStream(S,ObjPtr->polygon_index);
		ValueToStream(S,ObjPtr->location.x);
		ValueToStream(S,ObjPtr->location.y);
		ValueToStream(S,ObjPtr->location.z);
		
		ValueToStream(S,ObjPtr->flags);
	}
	
	assert((S - Stream) == Count*SIZEOF_map_object);
	return S;
}


uint8 *unpack_object_frequency_definition(uint8 *Stream, object_frequency_definition* Objects, int Count)
{
	uint8* S = Stream;
	object_frequency_definition* ObjPtr = Objects;
	
	for (int k = 0; k < Count; k++, ObjPtr++)
	{
		StreamToValue(S,ObjPtr->flags);
		
		StreamToValue(S,ObjPtr->initial_count);
		StreamToValue(S,ObjPtr->minimum_count);
		StreamToValue(S,ObjPtr->maximum_count);
		
		StreamToValue(S,ObjPtr->random_count);
		StreamToValue(S,ObjPtr->random_chance);
	}
	
	assert((S - Stream) == Count*SIZEOF_object_frequency_definition);
	return S;
}

uint8 *pack_object_frequency_definition(uint8 *Stream, object_frequency_definition* Objects, int Count)
{
	uint8* S = Stream;
	object_frequency_definition* ObjPtr = Objects;
	
	for (int k = 0; k < Count; k++, ObjPtr++)
	{
		ValueToStream(S,ObjPtr->flags);
		
		ValueToStream(S,ObjPtr->initial_count);
		ValueToStream(S,ObjPtr->minimum_count);
		ValueToStream(S,ObjPtr->maximum_count);
		
		ValueToStream(S,ObjPtr->random_count);
		ValueToStream(S,ObjPtr->random_chance);
	}
	
	assert((S - Stream) == Count*SIZEOF_object_frequency_definition);
	return S;
}


uint8 *unpack_static_data(uint8 *Stream, static_data* Objects, int Count)
{
	uint8* S = Stream;
	static_data* ObjPtr = Objects;
	
	for (int k = 0; k < Count; k++, ObjPtr++)
	{
		StreamToValue(S,ObjPtr->environment_code);
		
		StreamToValue(S,ObjPtr->physics_model);
		StreamToValue(S,ObjPtr->song_index);
		StreamToValue(S,ObjPtr->mission_flags);
		StreamToValue(S,ObjPtr->environment_flags);
		
		S += 4*2;
		
		StreamToBytes(S,ObjPtr->level_name,LEVEL_NAME_LENGTH);
		StreamToValue(S,ObjPtr->entry_point_flags);
	}
	
	assert((S - Stream) == Count*SIZEOF_static_data);
	return S;
}

uint8 *pack_static_data(uint8 *Stream, static_data* Objects, int Count)
{
	uint8* S = Stream;
	static_data* ObjPtr = Objects;
	
	for (int k = 0; k < Count; k++, ObjPtr++)
	{
		ValueToStream(S,ObjPtr->environment_code);
		
		ValueToStream(S,ObjPtr->physics_model);
		ValueToStream(S,ObjPtr->song_index);
		ValueToStream(S,ObjPtr->mission_flags);
		ValueToStream(S,ObjPtr->environment_flags);
		
		S += 4*2;
		
		BytesToStream(S,ObjPtr->level_name,LEVEL_NAME_LENGTH);
		ValueToStream(S,ObjPtr->entry_point_flags);
	}
	
	assert((S - Stream) == Count*SIZEOF_static_data);
	return S;
}


uint8 *unpack_ambient_sound_image_data(uint8 *Stream, ambient_sound_image_data* Objects, int Count)
{
	uint8* S = Stream;
	ambient_sound_image_data* ObjPtr = Objects;
	
	for (int k = 0; k < Count; k++, ObjPtr++)
	{
		StreamToValue(S,ObjPtr->flags);
		
		StreamToValue(S,ObjPtr->sound_index);
		StreamToValue(S,ObjPtr->volume);
		
		S += 5*2;
	}
	
	assert((S - Stream) == Count*SIZEOF_ambient_sound_image_data);
	return S;
}

uint8 *pack_ambient_sound_image_data(uint8 *Stream, ambient_sound_image_data* Objects, int Count)
{
	uint8* S = Stream;
	ambient_sound_image_data* ObjPtr = Objects;
	
	for (int k = 0; k < Count; k++, ObjPtr++)
	{
		ValueToStream(S,ObjPtr->flags);
		
		ValueToStream(S,ObjPtr->sound_index);
		ValueToStream(S,ObjPtr->volume);
		
		S += 5*2;
	}
	
	assert((S - Stream) == Count*SIZEOF_ambient_sound_image_data);
	return S;
}


uint8 *unpack_random_sound_image_data(uint8 *Stream, random_sound_image_data* Objects, int Count)
{
	uint8* S = Stream;
	random_sound_image_data* ObjPtr = Objects;
	
	for (int k = 0; k < Count; k++, ObjPtr++)
	{
		StreamToValue(S,ObjPtr->flags);
		
		StreamToValue(S,ObjPtr->sound_index);

		StreamToValue(S,ObjPtr->volume);
		StreamToValue(S,ObjPtr->delta_volume);
		StreamToValue(S,ObjPtr->period);
		StreamToValue(S,ObjPtr->delta_period);
		StreamToValue(S,ObjPtr->direction);
		StreamToValue(S,ObjPtr->delta_direction);
		StreamToValue(S,ObjPtr->pitch);
		StreamToValue(S,ObjPtr->delta_pitch);
		
		StreamToValue(S,ObjPtr->phase);
		
		S += 3*2;
	}
	
	assert((S - Stream) == Count*SIZEOF_random_sound_image_data);
	return S;
}

uint8 *pack_random_sound_image_data(uint8 *Stream, random_sound_image_data* Objects, int Count)
{
	uint8* S = Stream;
	random_sound_image_data* ObjPtr = Objects;
	
	for (int k = 0; k < Count; k++, ObjPtr++)
	{
		ValueToStream(S,ObjPtr->flags);
		
		ValueToStream(S,ObjPtr->sound_index);

		ValueToStream(S,ObjPtr->volume);
		ValueToStream(S,ObjPtr->delta_volume);
		ValueToStream(S,ObjPtr->period);
		ValueToStream(S,ObjPtr->delta_period);
		ValueToStream(S,ObjPtr->direction);
		ValueToStream(S,ObjPtr->delta_direction);
		ValueToStream(S,ObjPtr->pitch);
		ValueToStream(S,ObjPtr->delta_pitch);
		
		ValueToStream(S,ObjPtr->phase);
		
		S += 3*2;
	}
	
	assert((S - Stream) == Count*SIZEOF_random_sound_image_data);
	return S;
}


static void StreamToGameData(uint8* &S, game_data& Object)
{
	StreamToValue(S,Object.game_time_remaining);
	StreamToValue(S,Object.game_type);
	StreamToValue(S,Object.game_options);
	StreamToValue(S,Object.kill_limit);
	StreamToValue(S,Object.initial_random_seed);
	StreamToValue(S,Object.difficulty_level);
	StreamToList(S,Object.parameters,2);
}

static void GameDataToStream(uint8* &S, game_data& Object)
{
	ValueToStream(S,Object.game_time_remaining);
	ValueToStream(S,Object.game_type);
	ValueToStream(S,Object.game_options);
	ValueToStream(S,Object.kill_limit);
	ValueToStream(S,Object.initial_random_seed);
	ValueToStream(S,Object.difficulty_level);
	ListToStream(S,Object.parameters,2);
}


uint8 *unpack_dynamic_data(uint8 *Stream, dynamic_data* Objects, int Count)
{
	uint8* S = Stream;
	dynamic_data* ObjPtr = Objects;
	
	for (int k = 0; k < Count; k++, ObjPtr++)
	{
		StreamToValue(S,ObjPtr->tick_count);

		StreamToValue(S,ObjPtr->random_seed);

		StreamToGameData(S,ObjPtr->game_information);
		
		StreamToValue(S,ObjPtr->player_count);
		StreamToValue(S,ObjPtr->speaking_player_index);
		
		S += 2;
		StreamToValue(S,ObjPtr->platform_count);
		StreamToValue(S,ObjPtr->endpoint_count);
		StreamToValue(S,ObjPtr->line_count);
		StreamToValue(S,ObjPtr->side_count);
		StreamToValue(S,ObjPtr->polygon_count);
		StreamToValue(S,ObjPtr->lightsource_count);
		StreamToValue(S,ObjPtr->map_index_count);
		StreamToValue(S,ObjPtr->ambient_sound_image_count);
		StreamToValue(S,ObjPtr->random_sound_image_count);
		
		StreamToValue(S,ObjPtr->object_count);
		StreamToValue(S,ObjPtr->monster_count);
		StreamToValue(S,ObjPtr->projectile_count);
		StreamToValue(S,ObjPtr->effect_count);
		StreamToValue(S,ObjPtr->light_count);
		
		StreamToValue(S,ObjPtr->default_annotation_count);
		StreamToValue(S,ObjPtr->personal_annotation_count);
		
		StreamToValue(S,ObjPtr->initial_objects_count);
		
		StreamToValue(S,ObjPtr->garbage_object_count);
		
		StreamToValue(S,ObjPtr->last_monster_index_to_get_time);
		StreamToValue(S,ObjPtr->last_monster_index_to_build_path);
		
		StreamToValue(S,ObjPtr->new_monster_mangler_cookie);
		StreamToValue(S,ObjPtr->new_monster_vanishing_cookie);	
		
		StreamToValue(S,ObjPtr->civilians_killed_by_players);
		
		StreamToList(S,ObjPtr->random_monsters_left,MAXIMUM_OBJECT_TYPES);
		StreamToList(S,ObjPtr->current_monster_count,MAXIMUM_OBJECT_TYPES);
		StreamToList(S,ObjPtr->random_items_left,MAXIMUM_OBJECT_TYPES);
		StreamToList(S,ObjPtr->current_item_count,MAXIMUM_OBJECT_TYPES);

		StreamToValue(S,ObjPtr->current_level_number);
		
		StreamToValue(S,ObjPtr->current_civilian_causalties);
		StreamToValue(S,ObjPtr->current_civilian_count);
		StreamToValue(S,ObjPtr->total_civilian_causalties);
		StreamToValue(S,ObjPtr->total_civilian_count);
		
		StreamToValue(S,ObjPtr->game_beacon.x);
		StreamToValue(S,ObjPtr->game_beacon.y);
		StreamToValue(S,ObjPtr->game_player_index);
	}
	
	assert((S - Stream) == Count*SIZEOF_dynamic_data);
	return S;
}

uint8 *pack_dynamic_data(uint8 *Stream, dynamic_data* Objects, int Count)
{
	uint8* S = Stream;
	dynamic_data* ObjPtr = Objects;
	
	for (int k = 0; k < Count; k++, ObjPtr++)
	{
		ValueToStream(S,ObjPtr->tick_count);

		ValueToStream(S,ObjPtr->random_seed);

		GameDataToStream(S,ObjPtr->game_information);
		
		ValueToStream(S,ObjPtr->player_count);
		ValueToStream(S,ObjPtr->speaking_player_index);
		
		S += 2;
		ValueToStream(S,ObjPtr->platform_count);
		ValueToStream(S,ObjPtr->endpoint_count);
		ValueToStream(S,ObjPtr->line_count);
		ValueToStream(S,ObjPtr->side_count);
		ValueToStream(S,ObjPtr->polygon_count);
		ValueToStream(S,ObjPtr->lightsource_count);
		ValueToStream(S,ObjPtr->map_index_count);
		ValueToStream(S,ObjPtr->ambient_sound_image_count);
		ValueToStream(S,ObjPtr->random_sound_image_count);
		
		ValueToStream(S,ObjPtr->object_count);
		ValueToStream(S,ObjPtr->monster_count);
		ValueToStream(S,ObjPtr->projectile_count);
		ValueToStream(S,ObjPtr->effect_count);
		ValueToStream(S,ObjPtr->light_count);
		
		ValueToStream(S,ObjPtr->default_annotation_count);
		ValueToStream(S,ObjPtr->personal_annotation_count);
		
		ValueToStream(S,ObjPtr->initial_objects_count);
		
		ValueToStream(S,ObjPtr->garbage_object_count);
		
		ValueToStream(S,ObjPtr->last_monster_index_to_get_time);
		ValueToStream(S,ObjPtr->last_monster_index_to_build_path);
		
		ValueToStream(S,ObjPtr->new_monster_mangler_cookie);
		ValueToStream(S,ObjPtr->new_monster_vanishing_cookie);	
		
		ValueToStream(S,ObjPtr->civilians_killed_by_players);
		
		ListToStream(S,ObjPtr->random_monsters_left,MAXIMUM_OBJECT_TYPES);
		ListToStream(S,ObjPtr->current_monster_count,MAXIMUM_OBJECT_TYPES);
		ListToStream(S,ObjPtr->random_items_left,MAXIMUM_OBJECT_TYPES);
		ListToStream(S,ObjPtr->current_item_count,MAXIMUM_OBJECT_TYPES);

		ValueToStream(S,ObjPtr->current_level_number);
		
		ValueToStream(S,ObjPtr->current_civilian_causalties);
		ValueToStream(S,ObjPtr->current_civilian_count);
		ValueToStream(S,ObjPtr->total_civilian_causalties);
		ValueToStream(S,ObjPtr->total_civilian_count);
		
		ValueToStream(S,ObjPtr->game_beacon.x);
		ValueToStream(S,ObjPtr->game_beacon.y);
		ValueToStream(S,ObjPtr->game_player_index);
	}
	
	assert((S - Stream) == Count*SIZEOF_dynamic_data);
	return S;
}


uint8 *unpack_object_data(uint8 *Stream, object_data* Objects, int Count)
{
	uint8* S = Stream;
	object_data* ObjPtr = Objects;
	
	for (int k = 0; k < Count; k++, ObjPtr++)
	{
		StreamToValue(S,ObjPtr->location.x);
		StreamToValue(S,ObjPtr->location.y);
		StreamToValue(S,ObjPtr->location.z);
		StreamToValue(S,ObjPtr->polygon);
		
		StreamToValue(S,ObjPtr->facing);
		
		StreamToValue(S,ObjPtr->shape);
		
		StreamToValue(S,ObjPtr->sequence);
		StreamToValue(S,ObjPtr->flags);
		StreamToValue(S,ObjPtr->transfer_mode);
		StreamToValue(S,ObjPtr->transfer_period);
		StreamToValue(S,ObjPtr->transfer_phase);
		StreamToValue(S,ObjPtr->permutation);
		
		StreamToValue(S,ObjPtr->next_object);
		StreamToValue(S,ObjPtr->parasitic_object);
		
		StreamToValue(S,ObjPtr->sound_pitch);
	}
	
	assert((S - Stream) == Count*SIZEOF_object_data);
	return S;
}

uint8 *pack_object_data(uint8 *Stream, object_data* Objects, int Count)
{
	uint8* S = Stream;
	object_data* ObjPtr = Objects;
	
	for (int k = 0; k < Count; k++, ObjPtr++)
	{
		ValueToStream(S,ObjPtr->location.x);
		ValueToStream(S,ObjPtr->location.y);
		ValueToStream(S,ObjPtr->location.z);
		ValueToStream(S,ObjPtr->polygon);
		
		ValueToStream(S,ObjPtr->facing);
		
		ValueToStream(S,ObjPtr->shape);
		
		ValueToStream(S,ObjPtr->sequence);
		ValueToStream(S,ObjPtr->flags);
		ValueToStream(S,ObjPtr->transfer_mode);
		ValueToStream(S,ObjPtr->transfer_period);
		ValueToStream(S,ObjPtr->transfer_phase);
		ValueToStream(S,ObjPtr->permutation);
		
		ValueToStream(S,ObjPtr->next_object);
		ValueToStream(S,ObjPtr->parasitic_object);
		
		ValueToStream(S,ObjPtr->sound_pitch);
	}
	
	assert((S - Stream) == Count*SIZEOF_object_data);
	return S;
}


uint8 *unpack_damage_definition(uint8 *Stream, damage_definition* Objects, int Count)
{
	uint8* S = Stream;
	damage_definition* ObjPtr = Objects;
	
	for (int k = 0; k < Count; k++, ObjPtr++)
	{
		StreamToValue(S,ObjPtr->type);
		StreamToValue(S,ObjPtr->flags);
		
		StreamToValue(S,ObjPtr->base);
		StreamToValue(S,ObjPtr->random);
		StreamToValue(S,ObjPtr->scale);
	}
	
	assert((S - Stream) == Count*SIZEOF_damage_definition);
	return S;
}

uint8 *pack_damage_definition(uint8 *Stream, damage_definition* Objects, int Count)
{
	uint8* S = Stream;
	damage_definition* ObjPtr = Objects;
	
	for (int k = 0; k < Count; k++, ObjPtr++)
	{
		ValueToStream(S,ObjPtr->type);
		ValueToStream(S,ObjPtr->flags);
		
		ValueToStream(S,ObjPtr->base);
		ValueToStream(S,ObjPtr->random);
		ValueToStream(S,ObjPtr->scale);
	}
	
	assert((S - Stream) == Count*SIZEOF_damage_definition);
	return S;
}
