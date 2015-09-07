/*
PATHFINDING.C

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

Monday, January 18, 1993 5:00:40 PM

Friday, July 22, 1994 3:03:18 PM
	as a major change, we now return a path in all cases except the one where there are no free
	path slots or a monster cannot move at all.  monsters unable to reach their destinations or
	monsters whose destinations are too far away will get effectively random and clipped paths,
	respectively.  this was done to prevent out-of-control regeneration of paths.
Friday, November 18, 1994 8:09:38 PM  (Jason)
	if callers who ask for impossible paths need random paths it would be really nice just to
	give them a path to some node we just generated, rather than rebuild the entire node list
	and create a new path.
Wednesday, June 14, 1995 8:35:28 AM  (Jason)
	user data can now be passed into flood_map().

Feb 10, 2000 (Loren Petrich):
	Added dynamic-limits setting of MAXIMUM_PATHS
*/

#include <string.h>
#include <stdlib.h>
#include <limits.h>

#include "cseries.h"
#include "map.h"
#include "flood_map.h"
#include "dynamic_limits.h"

#ifdef DEBUG
//#define VALIDATE_PATH_SPACE
//#define VERIFY_PATH_SYNC
#endif

/*
we should cut corners instead of blindly following map geometry (i.e., separate pathfinding as
	much as possible from the underlying polygon structure of the map), especially the final leg.

//calculate_midpoint_of_shared_line should have some randomness (bad idea?)
//it ocurrs to me that connecting midpoints of shared lines won’t always work (monsters will run through walls)
//shadow polygons screw us up (a specific case of the general "weird polygon geometry" problem)
//random paths aren’t really random, and (even worse) tend to be symmetric
*/

/* ---------- constants */

// LP change: made this settable from the resource fork
#define MAXIMUM_PATHS (get_dynamic_limit(_dynamic_limit_paths))
#define MAXIMUM_POINTS_PER_PATH 63

#define PATH_VALIDATION_AREA_SIZE 64*1024

/* ---------- structures */

struct path_definition /* 256 bytes */
{
	/* NONE is an empty path */
	short current_step;
	short step_count;
	
	world_point2d points[MAXIMUM_POINTS_PER_PATH];
};

/* ---------- globals */

static struct path_definition *paths = NULL;

#ifdef VERIFY_PATH_SYNC
static byte *path_validation_area = NULL;
static int32 path_validation_area_index;
static short path_run_count;
#endif

/* ---------- private prototypes */

static void calculate_midpoint_of_shared_line(short polygon1, short polygon2,
	world_distance minimum_separation, world_point2d *midpoint);

/* ---------- code */

void allocate_pathfinding_memory(
	void)
{
	// Made reentrant because this is called every time MAXIMUM_PATHS is changed
	if (paths) delete []paths;
	paths= new path_definition[MAXIMUM_PATHS];
	assert(paths);

#ifdef VERIFY_PATH_SYNC
	if (path_validation_area) delete []path_validation_area;
	path_validation_area= new byte[PATH_VALIDATION_AREA_SIZE];
	assert(path_validation_area);
	path_run_count= 0;
#endif
}

void reset_paths(
	void)
{
	short path_index;

	for (path_index=0;path_index<MAXIMUM_PATHS;++path_index) paths[path_index].step_count= NONE;

#ifdef VERIFY_PATH_SYNC
	path_run_count+= 1;
	path_validation_area_index= 0;
#endif
}

short new_path(
	world_point2d *source_point,
	short source_polygon_index,
	world_point2d *destination_point,
	short destination_polygon_index,
	world_distance minimum_separation,
	cost_proc_ptr cost,
	void *data)
{
	short path_index;

//	dprintf("#%d(#%d,#%d) #%d(#%d,#%d)", source_polygon_index, source_point->x, source_point->y,
//		destination_polygon_index, destination_point ? destination_point->x : 0,
//		destination_point ? destination_point->y : 0);

	/* used for calculating the source polygon, ages ago */
	(void) (source_point);

#ifdef VALIDATE_PATH_SPACE	
	{
		short i;
		
		for (i=0;i<MAXIMUM_PATHS;++i)
		{
			if (paths[i].step_count!=NONE)
			{
				vassert(paths[i].step_count>=0&&paths[i].step_count<=MAXIMUM_POINTS_PER_PATH&&paths[i].current_step>=0&&paths[i].current_step<=paths[i].step_count,
					csprintf(temporary, "path #%d (%p) is fucked.", i, paths+i));
			}
		}
	}
#endif

	/* find a free path */	
	for (path_index=0;path_index<MAXIMUM_PATHS;++path_index)
	{
		if (paths[path_index].step_count==NONE) break;
	}
	if (path_index==MAXIMUM_PATHS) path_index= NONE;
	
	if (path_index!=NONE)
	{
		bool reached_destination;
		short polygon_index;
		short step_count;
		short depth;

		if (destination_polygon_index!=NONE)
		{
			/* NON-RANDOM PATH: we have a valid destination point: flood out from the source_polygon_index
				until we reach destination_polygon_index or we run out of stack space */
			
			polygon_index= flood_map(source_polygon_index, INT32_MAX, cost, _breadth_first, data);
			while (polygon_index!=NONE&&polygon_index!=destination_polygon_index)
			{
				polygon_index= flood_map(NONE, INT32_MAX, cost, _breadth_first, data);
			}

			/* if we reached destination_polygon_index, extract the path by calling
				reverse_flood_map().  remember to add the destination to the end of the path */
			reached_destination= polygon_index==destination_polygon_index ? true : false;
		}
		else
		{
			/* RANDOM PATH: our destination point is invalid (the caller wants a random path); flood
				out from the source polygon until we run out of stack space or we reach a cost
				of RANDOM_PATH_AREA, whichever comes first.  in fact, our destination_point, if
				not NULL, is a 2d vector specifying a bias in the direction we want to travel
				(usually this will be away from somewhere we don’t want to be) */
			polygon_index= flood_map(source_polygon_index, INT32_MAX, cost, _breadth_first, data);
			while (polygon_index!=NONE)
			{
				polygon_index= flood_map(NONE, INT32_MAX, cost, _breadth_first, data);
			}
			
			choose_random_flood_node((world_vector2d *)destination_point); /* choose a random destination */
			reached_destination= false; /* we didn’t even have one */
		}

		depth= flood_depth();
		if (reached_destination)
		{
			/* a depth of zero yeilds one point (the destination), two and greater 2*depth */
			step_count= depth+1; //depth ? 2*depth : 1;
		}
		else
		{
			/* two points for every polygon change, minus one point because we never walk into the
				last polygon; therefore destinationless paths with a depth of zero are useless */
			step_count= depth; //2*depth-1;
		}
	
		if (step_count>0) /* if we have valid steps, extract the path */
		{
			struct path_definition *path= paths+path_index;
			short last_polygon_index;

//#ifdef DEBUG
			obj_set(*path, 0x80);
//#endif
			
			path->step_count= step_count>MAXIMUM_POINTS_PER_PATH ? MAXIMUM_POINTS_PER_PATH : step_count;
			assert(path->step_count!=NONE); /* this would be bad */
			path->current_step= 0;

			/* if we reached our destination (and it’s not out-of-range), add it */
			if (reached_destination && --step_count<MAXIMUM_POINTS_PER_PATH) path->points[step_count]= *destination_point;
			
			/* add all the points up to but not including the source (if we have room) */
			last_polygon_index= reverse_flood_map();
			while ((polygon_index= reverse_flood_map())!=NONE)
			{
				if (--step_count<MAXIMUM_POINTS_PER_PATH) calculate_midpoint_of_shared_line(last_polygon_index, polygon_index, minimum_separation, path->points+step_count);
//				if (polygon_index!=source_polygon_index&&--step_count<MAXIMUM_POINTS_PER_PATH) find_center_of_polygon(polygon_index, path->points+step_count);
				last_polygon_index= polygon_index;
			}
			assert(!step_count); /* we should be out of points */
	
//			dprintf("path from %d to %d (%d steps);dm #%d #%d;g", source_polygon_index, destination_polygon_index, paths[path_index].step_count,
//				path_points, paths[path_index].step_count*sizeof(world_point2d));

#ifdef VERIFY_PATH_SYNC
			if (path_run_count==1)
			{
				objlist_copy(path_validation_area+path_validation_area_index, path->points, path->step_count);
				path_validation_area_index+= sizeof(world_point2d)*path->step_count;
				assert(path_validation_area_index<PATH_VALIDATION_AREA_SIZE);
			}
			else
			{
				if (memcmp(path_validation_area+path_validation_area_index, path->points, sizeof(world_point2d)*path->step_count))
				{
					dprintf("path #%d at %p didn’t match point list at %p", path_index, path, path_validation_area+path_validation_area_index);
				}
				path_validation_area_index+= sizeof(world_point2d*)*path->step_count;
			}
#endif
		}
		else
		{
			path_index= NONE;
		}
	}

#ifdef VALIDATE_PATH_SPACE
	{
		short i;
		
		for (i=0;i<MAXIMUM_PATHS;++i)
		{
			if (paths[i].step_count!=NONE)
			{
				vassert(paths[i].step_count>=0&&paths[i].step_count<=MAXIMUM_POINTS_PER_PATH&&paths[i].current_step>=0&&paths[i].current_step<=paths[i].step_count,
					csprintf(temporary, "path #%d (%p) is fucked.", i, paths+i));
			}
		}
	}
#endif
	
	return path_index;
}

bool move_along_path(
	short path_index,
	world_point2d *p)
{
	struct path_definition *path;
	bool end_of_path= false;
	
	assert(path_index>=0&&path_index<MAXIMUM_PATHS);
	path= paths+path_index;

	assert(path->step_count!=NONE);
	vassert(path->current_step>=0&&path->current_step<=path->step_count, csprintf(temporary, "invalid current path step: #%d/#%d", path->current_step, path->step_count));
	
	if (path->current_step==path->step_count)
	{
		path->step_count= NONE;
		end_of_path= true;
	}
	else
	{
		*p= path->points[path->current_step++];
//		vwarn(valid_point2d(p), csprintf(temporary, "step #%d (%d,%d) of path %p looks bad;g;", path->current_step-1, p->x, p->y, path));
	}
	
	return end_of_path;
}

void delete_path(
	short path_index)
{
	assert(path_index>=0&&path_index<MAXIMUM_PATHS);
	assert(paths[path_index].step_count!=NONE);
	vassert(paths[path_index].current_step>=0&&paths[path_index].current_step<=paths[path_index].step_count, csprintf(temporary, "invalid current path step: #%d/#%d", paths[path_index].current_step, paths[path_index].step_count));
	
	paths[path_index].step_count= NONE;
}

/* ---------- private code */

static void calculate_midpoint_of_shared_line(
	short polygon1,
	short polygon2,
	world_distance minimum_separation,
	world_point2d *midpoint)
{
	short shared_line_index;
	world_distance range, origin;
	struct line_data *shared_line;
	struct endpoint_data *endpoint0, *endpoint1;
	
	shared_line_index= find_shared_line(polygon1, polygon2);
	assert(shared_line_index!=NONE);
	shared_line= get_line_data(shared_line_index);

	endpoint0= get_endpoint_data(shared_line->endpoint_indexes[0]);
	endpoint1= get_endpoint_data(shared_line->endpoint_indexes[1]);

	origin= 0;
	range= shared_line->length;
	if (ENDPOINT_IS_ELEVATION(endpoint0)) origin+= minimum_separation, range-= minimum_separation;
	if (ENDPOINT_IS_ELEVATION(endpoint1)) range-= minimum_separation;
	if (range<=0)
	{
		/* uhh... this line is really too small for us to pass through */
		midpoint->x= endpoint0->vertex.x + (endpoint1->vertex.x-endpoint0->vertex.x)/2;
		midpoint->y= endpoint0->vertex.y + (endpoint1->vertex.y-endpoint0->vertex.y)/2;
	}
	else
	{
		world_distance dx= endpoint1->vertex.x-endpoint0->vertex.x;
		world_distance dy= endpoint1->vertex.y-endpoint0->vertex.y;
		world_distance offset= origin + ((global_random()*range)>>16);
		
		midpoint->x= endpoint0->vertex.x + (offset*dx)/shared_line->length;
		midpoint->y= endpoint0->vertex.y + (offset*dy)/shared_line->length;
	}
}

/* for debug purposes only (called from OVERHEAD_MAP.C) */
// LP: making these available for those wanting to check out the monster AI
world_point2d *path_peek(
	short path_index,
	short *step_count)
{
	struct path_definition *path;
	world_point2d *points= 0;
	
	path= paths+path_index;
	if (path->step_count!=NONE)
	{
		*step_count= path->step_count;
		points= path->points;
	}
	
	return points;
}

// LP addition: the total number of paths
short GetNumberOfPaths() {return MAXIMUM_PATHS;}
