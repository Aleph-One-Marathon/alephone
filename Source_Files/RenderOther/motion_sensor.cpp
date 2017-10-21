/*
MOTION_SENSOR.C

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

Saturday, June 11, 1994 1:37:32 AM

Friday, September 16, 1994 2:04:47 PM  (Jason')
	removed all get_shape_information() calls.
Monday, December 5, 1994 9:19:55 PM  (Jason)
	flickers in magnetic environments

Feb 18, 2000 (Loren Petrich):
	Made VacBobs display properly

Mar 23, 2000 (Loren Petrich):
	Made motion-sensor monster typing more generic;
	since it's now read off of a table, it can easily be changed
	without rebuilding the app.

Sep 2, 2000 (Loren Petrich):
	Idiot-proofed the shapes display, since the shape accessor
	now returns NULL pointers for nonexistent bitmaps.
*/

#include "cseries.h"
#include "map.h"
#include "monsters.h"
#include "render.h"
#include "interface.h"
#include "motion_sensor.h"
#include "player.h"
#include "network_games.h"
#include "InfoTree.h"

#include "HUDRenderer_SW.h"
#include "HUDRenderer_OGL.h"
#include "HUDRenderer_Lua.h"

#include <math.h>
#include <string.h>
#include <stdlib.h>

static short MonsterDisplays[NUMBER_OF_MONSTER_TYPES] =
{
	// Marine
	MType_Friend,
	// Ticks
	MType_Alien,
	MType_Alien,
	MType_Alien,
	// S'pht
	MType_Alien,
	MType_Alien,
	MType_Alien,
	MType_Alien,
	// Pfhor
	MType_Alien,
	MType_Alien,
	MType_Alien,
	MType_Alien,
	// Bob
	MType_Friend,
	MType_Friend,
	MType_Friend,
	MType_Friend,
	// Drone
	MType_Alien,
	MType_Alien,
	MType_Alien,
	MType_Alien,
	MType_Alien,
	// Cyborg
	MType_Alien,
	MType_Alien,
	MType_Alien,
	MType_Alien,
	// Enforcer
	MType_Alien,
	MType_Alien,
	// Hunter
	MType_Alien,
	MType_Alien,
	// Trooper
	MType_Alien,
	MType_Alien,
	// Big Cyborg, Hunter
	MType_Alien,
	MType_Alien,
	// F'lickta
	MType_Alien,
	MType_Alien,
	MType_Alien,
	// S'pht'Kr
	MType_Alien,
	MType_Alien,
	// Juggernauts
	MType_Alien,
	MType_Alien,
	// Tiny ones
	MType_Alien,
	MType_Alien,
	MType_Alien,
	// VacBobs
	MType_Friend,
	MType_Friend,
	MType_Friend,
	MType_Friend,
};

/*
??monsters which translate only on frame changes periodically drop off the motion sensor making it appear jumpy

//when entities first appear on the sensor they are initially visible
//entities are drawn one at a time, so the dark spot from one entity can cover the light spot from another
//sometimes the motion sensor shapes are not masked correctly (fixed from SHAPES.C)
//must save old points in real world coordinates because entities get redrawn when the player turns
*/

#define MAXIMUM_MOTION_SENSOR_ENTITIES 12

#define NUMBER_OF_PREVIOUS_LOCATIONS 6

struct motion_sensor_definition {
	uint16 update_frequency;
	uint16 rescan_frequency;
	uint16 range;
	int16 scale;
};

struct motion_sensor_definition motion_sensor_settings = {
	5,  // update_frequency
	15, // rescan_frequency
	(8 * WORLD_ONE), // range
	64 // scale
};

#define MOTION_SENSOR_UPDATE_FREQUENCY motion_sensor_settings.update_frequency
#define MOTION_SENSOR_RESCAN_FREQUENCY motion_sensor_settings.rescan_frequency
#define MOTION_SENSOR_RANGE motion_sensor_settings.range
#define MOTION_SENSOR_SCALE motion_sensor_settings.scale

#define OBJECT_IS_VISIBLE_TO_MOTION_SENSOR(o) true

#define FLICKER_FREQUENCY 0xe

/* ---------- structures */

/* an entity can’t just be jerked from the array, because his signal should fade out, so we
	mark him as ‘being removed’ and wait until his last signal fades away to actually remove
	him */
#define SLOT_IS_BEING_REMOVED_BIT 0x4000
#define SLOT_IS_BEING_REMOVED(e) ((e)->flags&(uint16)SLOT_IS_BEING_REMOVED_BIT)
#define MARK_SLOT_AS_BEING_REMOVED(e) ((e)->flags|=(uint16)SLOT_IS_BEING_REMOVED_BIT)

struct entity_data
{
	uint16 flags; /* [slot_used.1] [slot_being_removed.1] [unused.14] */
	
	short monster_index;
	shape_descriptor shape;
	
	short remove_delay; /* only valid if this entity is being removed [0,NUMBER_OF_PREVIOUS_LOCATIONS) */
	
	point2d previous_points[NUMBER_OF_PREVIOUS_LOCATIONS];
	bool visible_flags[NUMBER_OF_PREVIOUS_LOCATIONS];
	
	world_point3d last_location;
	angle last_facing;
};

struct region_data
{
	short x0, x1;
};

/* ---------- globals */

static short motion_sensor_player_index;

static short motion_sensor_side_length;

static struct region_data *sensor_region;
static struct entity_data *entities;
static short network_compass_state;

static shape_descriptor mount_shape, virgin_mount_shapes, compass_shapes;
static shape_descriptor alien_shapes, friendly_shapes, enemy_shapes;

static bool motion_sensor_changed;
static int32 ticks_since_last_update, ticks_since_last_rescan;

/* ---------- private code */

static void erase_all_entity_blips(void);

static void precalculate_sensor_region(short side_length);

static short find_or_add_motion_sensor_entity(short monster_index);

static shape_descriptor get_motion_sensor_entity_shape(short monster_index);

static void clipped_transparent_sprite_copy(struct bitmap_definition *source, struct bitmap_definition *destination,
	struct region_data *region, short x0, short y0);
static void bitmap_window_copy(struct bitmap_definition *source, struct bitmap_definition *destination,
	short x0, short y0, short x1, short y1);
static void unclipped_solid_sprite_copy(struct bitmap_definition *source,
	struct bitmap_definition *destination, short x0, short y0);

/* ---------- code */

void initialize_motion_sensor(
	shape_descriptor mount,
	shape_descriptor virgin_mounts,
	shape_descriptor aliens,
	shape_descriptor friends,
	shape_descriptor enemies,
	shape_descriptor compasses,
	short side_length)
{
	mount_shape= mount;
	virgin_mount_shapes= virgin_mounts;
	enemy_shapes= enemies;
	friendly_shapes= friends;
	alien_shapes= aliens;
	compass_shapes= compasses;
	
	entities= new entity_data[MAXIMUM_MOTION_SENSOR_ENTITIES];
	assert(entities);
	for (int i = 0; i < MAXIMUM_MOTION_SENSOR_ENTITIES; i++) {
	  entities[i].flags = 0;
	}

	sensor_region= new region_data[side_length];
	assert(sensor_region);
	
	/* precalculate the sensor region */
	precalculate_sensor_region(side_length);	

	/* reset_motion_sensor() should be called before the motion sensor is used, but after it’s
		shapes are loaded (because it will do bitmap copying) */
}

extern bool shapes_file_is_m1();

void reset_motion_sensor(
	short player_index)
{
	struct bitmap_definition *mount, *virgin_mount;
	short i;

	motion_sensor_player_index= player_index;
	ticks_since_last_update= ticks_since_last_rescan= 0;

	if (!shapes_file_is_m1())
	{
		get_shape_bitmap_and_shading_table(mount_shape, &mount, (void **) NULL, NONE);
		if (!mount) return;
		get_shape_bitmap_and_shading_table(virgin_mount_shapes, &virgin_mount, (void **) NULL, NONE);
		if (!virgin_mount) return;
		
		assert(mount->width==virgin_mount->width);
		assert(mount->height==virgin_mount->height);
		bitmap_window_copy(virgin_mount, mount, 0, 0, mount->width, mount->height);
	}

	for (i= 0; i<MAXIMUM_MOTION_SENSOR_ENTITIES; ++i) MARK_SLOT_AS_FREE(entities+i);
	
	network_compass_state= _network_compass_all_off;
}

extern bool MotionSensorActive;

// regardless of frame rate, this will update the positions of the objects
// we are tracking (motion_sensor_scan() is called each tick)
void motion_sensor_scan(void)
{
	if ((GET_GAME_OPTIONS() & _motion_sensor_does_not_work) || !MotionSensorActive) {
		return;
	}
	if (m1_solo_player_in_terminal()) {
		return;
	}

	struct object_data *owner_object= get_object_data(get_player_data(motion_sensor_player_index)->object_index);

	/* if we need to scan for new objects, flood around the owner monster looking for other,
		visible monsters within our range */
	if ((--ticks_since_last_rescan) < 0)
	{
		struct monster_data *monster;
		short monster_index;
		
		for (monster_index=0,monster=monsters;monster_index<MAXIMUM_MONSTERS_PER_MAP;++monster,++monster_index)
		{
			if (SLOT_IS_USED(monster)&&(MONSTER_IS_PLAYER(monster)||MONSTER_IS_ACTIVE(monster)))
			{
				struct object_data *object= get_object_data(monster->object_index);
				world_distance distance= guess_distance2d((world_point2d *) &object->location, (world_point2d *) &owner_object->location);
				
				if (distance<MOTION_SENSOR_RANGE && OBJECT_IS_VISIBLE_TO_MOTION_SENSOR(object))
				{
//					dprintf("found valid monster #%d", monster_index);
					find_or_add_motion_sensor_entity(object->permutation);
					motion_sensor_changed = true;
				}
			}
		}
		
		ticks_since_last_rescan= MOTION_SENSOR_RESCAN_FREQUENCY;
	}

	if ((--ticks_since_last_update) < 0) {
		erase_all_entity_blips();
		
		ticks_since_last_update = MOTION_SENSOR_UPDATE_FREQUENCY;
	}
}

void HUD_SW_Class::render_motion_sensor(short ticks_elapsed)
{
	// Clear the motion sensor and redraw blips
	struct bitmap_definition *mount, *virgin_mount;
		
	get_shape_bitmap_and_shading_table(mount_shape, &mount, (void **) NULL, NONE);
	get_shape_bitmap_and_shading_table(virgin_mount_shapes, &virgin_mount, (void **) NULL, NONE);
	if (mount && virgin_mount)
		bitmap_window_copy(virgin_mount, mount, 0, 0, motion_sensor_side_length, motion_sensor_side_length);

	draw_network_compass();
	draw_all_entity_blips();
}

void HUD_OGL_Class::render_motion_sensor(short ticks_elapsed)
{
	// Draw background
	screen_rectangle *r = get_interface_rectangle(_motion_sensor_rect);
	DrawShapeAtXY(BUILD_DESCRIPTOR(_collection_interface, _motion_sensor_virgin_mount), r->left, r->top);

	// We allways draw all active entities because we have to update the
	// display on every frame
	/*if (dynamic_world->player_count > 1)*/
		draw_network_compass();
	draw_all_entity_blips();
}

void HUD_Lua_Class::render_motion_sensor(short ticks_elapsed)
{
	// If we need to update the motion sensor, draw all active entities
	draw_all_entity_blips();
}


/* the interface code will call this function and only draw the motion sensor if we return true */
bool motion_sensor_has_changed(void)
{
	bool changed = motion_sensor_changed;
	motion_sensor_changed = false;
	return changed;
}

/* toggle through the ranges */
void adjust_motion_sensor_range(void)
{
}

/* ---------- private code */

void HUD_Class::draw_network_compass(void)
{
#if !defined(DISABLE_NETWORKING)
	short new_state= get_network_compass_state(motion_sensor_player_index);
	short difference= (new_state^network_compass_state)|new_state;
	
	if (difference&_network_compass_nw) draw_or_erase_unclipped_shape(36, 36, compass_shapes, (new_state&_network_compass_nw) != 0);
	if (difference&_network_compass_ne) draw_or_erase_unclipped_shape(61, 36, compass_shapes+1, (new_state&_network_compass_ne) != 0);
	if (difference&_network_compass_se) draw_or_erase_unclipped_shape(61, 61, compass_shapes+3, (new_state&_network_compass_se) != 0);
	if (difference&_network_compass_sw) draw_or_erase_unclipped_shape(36, 61, compass_shapes+2, (new_state&_network_compass_sw) != 0);
	
	network_compass_state= new_state;
#endif // !defined(DISABLE_NETWORKING)
}

void erase_all_entity_blips(void)
{
	struct object_data *owner_object= get_object_data(get_player_data(motion_sensor_player_index)->object_index);
	struct entity_data *entity;
	short entity_index;

	/* first erase all locations where the entity changed locations and then did not change
		locations, and erase it’s last location */
	for (entity_index=0,entity=entities;entity_index<MAXIMUM_MOTION_SENSOR_ENTITIES;++entity_index,++entity)
	{
		if (SLOT_IS_USED(entity))
		{
			motion_sensor_changed = true;
//			dprintf("entity #%d (%p) valid", entity_index, entity);
			/* see if our monster slot is free; if it is mark this entity as being removed; of
				course this isn’t wholly accurate and we might start tracking a new monster
				which has been placed in our old monster’s slot, but we eat that chance
				without remorse */
			if (SLOT_IS_USED(monsters+entity->monster_index))
			{
				struct object_data *object= get_object_data(get_monster_data(entity->monster_index)->object_index);
				world_distance distance= guess_distance2d((world_point2d *) &object->location, (world_point2d *) &owner_object->location);
				
				/* verify that we’re still in range (and mark us as being removed if we’re not */
				if (distance>MOTION_SENSOR_RANGE || !OBJECT_IS_VISIBLE_TO_MOTION_SENSOR(object))
				{
//					dprintf("removed2");
					MARK_SLOT_AS_BEING_REMOVED(entity);
				}
			}
			else
			{
//				dprintf("removed1");
				MARK_SLOT_AS_BEING_REMOVED(entity);
			}

			/* adjust the arrays to make room for new entries */			
			memmove(entity->visible_flags+1, entity->visible_flags, (NUMBER_OF_PREVIOUS_LOCATIONS-1)*sizeof(bool));
			memmove(entity->previous_points+1, entity->previous_points, (NUMBER_OF_PREVIOUS_LOCATIONS-1)*sizeof(point2d));
			entity->visible_flags[0]= false;
				
			/* if we’re not being removed, make room for a new location and calculate it */
			if (!SLOT_IS_BEING_REMOVED(entity))
			{
				struct monster_data *monster= get_monster_data(entity->monster_index);
				struct object_data *object= get_object_data(monster->object_index);
				
				/* remember if this entity is visible or not */
				if (object->transfer_mode!=_xfer_invisibility && object->transfer_mode!=_xfer_subtle_invisibility &&
					(!(static_world->environment_flags&_environment_magnetic) || !((dynamic_world->tick_count+4*monster->object_index)&FLICKER_FREQUENCY)))
				{
					if (object->location.x!=entity->last_location.x || object->location.y!=entity->last_location.y ||
						object->location.z!=entity->last_location.z || object->facing!=entity->last_facing)
					{
						entity->visible_flags[0]= true;
	
						entity->last_location= object->location;
						entity->last_facing= object->facing;
					}
				}
				
				/* calculate the 2d position on the motion sensor */
				entity->previous_points[0].x= object->location.x;
				entity->previous_points[0].y= object->location.y;
				transform_point2d((world_point2d *)&entity->previous_points[0], (world_point2d *)&owner_object->location, NORMALIZE_ANGLE(owner_object->facing+QUARTER_CIRCLE));
				//entity->previous_points[0].x>>= MOTION_SENSOR_SCALE;
				entity->previous_points[0].x /= MOTION_SENSOR_SCALE;
			//	entity->previous_points[0].y>>= MOTION_SENSOR_SCALE;
				entity->previous_points[0].y /= MOTION_SENSOR_SCALE;
			}
			else
			{
				/* if this is the last point of an entity which was being removed; mark it as unused */
				if ((entity->remove_delay+= 1)>=NUMBER_OF_PREVIOUS_LOCATIONS)
				{
					MARK_SLOT_AS_FREE(entity);
				}
			}
		}
	}
}

void HUD_Class::draw_all_entity_blips(void)
{
	struct entity_data *entity;
	short entity_index, intensity;

	for (intensity=NUMBER_OF_PREVIOUS_LOCATIONS-1;intensity>=0;--intensity)
	{
		for (entity_index=0,entity=entities;entity_index<MAXIMUM_MOTION_SENSOR_ENTITIES;++entity_index,++entity)
		{
			if (SLOT_IS_USED(entity))
			{
				if (entity->visible_flags[intensity])
				{
					draw_entity_blip(&entity->previous_points[intensity], entity->shape + intensity);
				}
			}
		}
	}
}

void HUD_Lua_Class::draw_all_entity_blips(void)
{
	struct entity_data *entity;
	short entity_index, intensity;
	
	clear_entity_blips();
	for (intensity=NUMBER_OF_PREVIOUS_LOCATIONS-1;intensity>=0;--intensity)
	{
		for (entity_index=0,entity=entities;entity_index<MAXIMUM_MOTION_SENSOR_ENTITIES;++entity_index,++entity)
		{
			if (SLOT_IS_USED(entity))
			{
				if (entity->visible_flags[intensity])
				{
					short display_type = MType_Alien;
					if (entity->shape == friendly_shapes)
						display_type = MType_Friend;
					else if (entity->shape == enemy_shapes)
						display_type = MType_Enemy;
					add_entity_blip(display_type, intensity,
													entity->previous_points[intensity].x * MOTION_SENSOR_SCALE,
													entity->previous_points[intensity].y * MOTION_SENSOR_SCALE);
				}
			}
		}
	}
}

void HUD_SW_Class::draw_or_erase_unclipped_shape(short x, short y, shape_descriptor shape, bool draw)
{
	struct bitmap_definition *mount, *virgin_mount, *blip;

	get_shape_bitmap_and_shading_table(mount_shape, &mount, (void **) NULL, NONE);
	if (!mount) return;
	get_shape_bitmap_and_shading_table(virgin_mount_shapes, &virgin_mount, (void **) NULL, NONE);
	if (!virgin_mount) return;
	get_shape_bitmap_and_shading_table(shape, &blip, (void **) NULL, NONE);
	if (!blip) return;
	
	draw ?
		unclipped_solid_sprite_copy(blip, mount, x, y) :
		bitmap_window_copy(virgin_mount, mount, x, y, x+blip->width, y+blip->height);
}

void HUD_OGL_Class::draw_or_erase_unclipped_shape(short x, short y, shape_descriptor shape, bool draw)
{
	if (draw) {
		screen_rectangle *r = get_interface_rectangle(_motion_sensor_rect);
		DrawShapeAtXY(shape, x + r->left, y + r->top);
	}
}

void HUD_SW_Class::draw_entity_blip(point2d *location, shape_descriptor shape)
{
	bitmap_definition *mount, *blip;
	
	get_shape_bitmap_and_shading_table(mount_shape, &mount, (void **) NULL, NONE);
	if (!mount) return;
	get_shape_bitmap_and_shading_table(shape, &blip, (void **) NULL, NONE);
	if (!blip) return;

	clipped_transparent_sprite_copy(blip, mount, sensor_region,
		location->x + (motion_sensor_side_length>>1) - (blip->width>>1),
		location->y + (motion_sensor_side_length>>1) - (blip->height>>1));
}

void HUD_OGL_Class::draw_entity_blip(point2d *location, shape_descriptor shape)
{
	bitmap_definition *blip;
	get_shape_bitmap_and_shading_table(shape, &blip, (void **) NULL, NONE);
	if (!blip) return;

	screen_rectangle *r = get_interface_rectangle(_motion_sensor_rect);
	int x = location->x, y = location->y;
	int c_x = r->left + (motion_sensor_side_length >> 1);
	int c_y = r->top + (motion_sensor_side_length >> 1);
	SetClipPlane(x, y, c_x, c_y, motion_sensor_side_length >> 1);
	DrawShapeAtXY(shape,
		x + c_x - (blip->width >> 1),
		y + c_y - (blip->height >> 1),
		true);
	DisableClipPlane();
}

/* if we find an entity that is being removed, we continue with the removal process and ignore
	the new signal; the new entity will probably not be added to the sensor again for a full
	second or so (the range should be set so that this is reasonably hard to do) */
static short find_or_add_motion_sensor_entity(
	short monster_index)
{
	struct entity_data *entity;
	short entity_index, best_unused_index;
	
	best_unused_index= NONE;
	for (entity_index=0,entity=entities;entity_index<MAXIMUM_MOTION_SENSOR_ENTITIES;++entity_index,++entity)
	{
		if (SLOT_IS_USED(entity))
		{
			if (entity->monster_index==monster_index) break;
		}
		else
		{
			if (best_unused_index==NONE) best_unused_index= entity_index;
		}
	}

	if (entity_index==MAXIMUM_MOTION_SENSOR_ENTITIES)
	{
		/* not found; add new entity if we can */
		
		if (best_unused_index!=NONE)
		{
			struct monster_data *monster= get_monster_data(monster_index);
			struct object_data *object= get_object_data(monster->object_index);
			short i;

			entity= entities+best_unused_index;
			
			entity->flags= 0;
			entity->monster_index= monster_index;
			entity->shape= get_motion_sensor_entity_shape(monster_index);
			for (i=0;i<NUMBER_OF_PREVIOUS_LOCATIONS;++i) entity->visible_flags[i]= false;
			entity->last_location= object->location;
			entity->last_facing= object->facing;
			entity->remove_delay= 0;
			MARK_SLOT_AS_USED(entity);
			
//			dprintf("new index, pointer: %d, %p", best_unused_index, entity);
		}
		
		entity_index= best_unused_index;
	}
	
	return entity_index;
}

static void precalculate_sensor_region(
	short side_length)
{
	double half_side_length= side_length/2.0;
	double r= half_side_length + 1.0;
	short i;

	/* save length for assert() during rendering */
	motion_sensor_side_length= side_length;
	
	/* precompute [x0,x1] clipping values for each y value in the circular sensor */
	for (i=0;i<side_length;++i)
	{
		double y= i - half_side_length;
		double x= sqrt(r*r-y*y);
		
		if (x>=r) x= r-1.0;
		sensor_region[i].x0= int16(half_side_length-x);
		sensor_region[i].x1= int16(x+half_side_length);
	}
}

/* (x0,y0) and (x1,y1) specify a window to be copied from the source to (x2,y2) in the destination.
	pixel index zero is transparent (handles clipping) */
static void bitmap_window_copy(
	struct bitmap_definition *source,
	struct bitmap_definition *destination,
	short x0,
	short y0,
	short x1,
	short y1)
{
	short count;
	short y;
	
	assert(x0<=x1&&y0<=y1);

	if (x0<0) x0= 0;
	if (y0<0) y0= 0;
	if (x1>source->width) x1= source->width;
	if (y1>source->height) y1= source->height;
	
	assert(source->width==destination->width);
	assert(source->height==destination->height);
	assert(destination->width==motion_sensor_side_length);
	assert(destination->height==motion_sensor_side_length);
	
	for (y=y0;y<y1;++y)
	{
		pixel8 *read= source->row_addresses[y]+x0;
		pixel8 *write= destination->row_addresses[y]+x0;
		
		for (count=x1-x0;count>0;--count) *write++= *read++;
	}
}

static void clipped_transparent_sprite_copy(
	struct bitmap_definition *source,
	struct bitmap_definition *destination,
	struct region_data *region,
	short x0,
	short y0)
{
	short height, y;
	
	assert(destination->width==motion_sensor_side_length);
	assert(destination->height==motion_sensor_side_length);

	y= 0;
	height= source->height;
	if (y0+height>destination->height) height= destination->height-y0;
	if (y0<0)
	{
		y= -y0;
		height+= y0;
	}

	while ((height-= 1)>=0)	
	{
		pixel8 pixel, *read, *write;
		short width= source->width;
		short clip_left= region[y0+y].x0, clip_right= region[y0+y].x1;
		short offset= 0;
		
		if (x0<clip_left) offset= clip_left-x0, width-= offset;
		if (x0+offset+width>clip_right) width= clip_right-x0-offset;

		assert(y>=0&&y<source->height);
		assert(y0+y>=0&&y0+y<destination->height);
		
		read= source->row_addresses[y]+offset;
		write= destination->row_addresses[y0+y]+x0+offset;
		
		while ((width-= 1)>=0)
		{
			if ((pixel= *read++)!=0) {
				*write++= pixel;
			} else {
				write+= 1;
			}
		}

		y+= 1;
	}
}

static void unclipped_solid_sprite_copy(
	struct bitmap_definition *source,
	struct bitmap_definition *destination,
	short x0,
	short y0)
{
	short height, y;

	y= 0;
	height= source->height;

	while ((height-= 1)>=0)	
	{
		pixel8 *read, *write;
		short width= source->width;

		assert(y>=0&&y<source->height);
		assert(y0+y>=0&&y0+y<destination->height);
		
		read= source->row_addresses[y];
		write= destination->row_addresses[y0+y]+x0;
		
		while ((width-= 1)>=0) *write++= *read++;

		y+= 1;
	}
}

static shape_descriptor get_motion_sensor_entity_shape(
	short monster_index)
{
	struct monster_data *monster= get_monster_data(monster_index);
	shape_descriptor shape;
	
	if (MONSTER_IS_PLAYER(monster))
	{
		struct player_data *player= get_player_data(monster_index_to_player_index(monster_index));
		struct player_data *owner= get_player_data(motion_sensor_player_index);
		
		shape= ((player->team==owner->team && !(GET_GAME_OPTIONS()&_force_unique_teams)) || GET_GAME_TYPE()==_game_of_cooperative_play) ?
			friendly_shapes : enemy_shapes;
	}
	else
	{
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
				shape= friendly_shapes;
				break;
			
			default:
				shape= alien_shapes;
				break;
		}
		*/
		switch(MonsterDisplays[monster->type])
		{
		case MType_Friend:
			shape = friendly_shapes;
			break;
		
		case MType_Alien:
			shape = alien_shapes;
			break;
			
		case MType_Enemy:
		default:
			shape = enemy_shapes;
			break;
		}
	}
	
	return shape;
}

// XML elements for parsing motion-sensor specification
short *OriginalMonsterDisplays = NULL;
struct motion_sensor_definition *original_motion_sensor_settings = NULL;

void reset_mml_motion_sensor()
{
	if (original_motion_sensor_settings) {
		motion_sensor_settings = *original_motion_sensor_settings;
		free(original_motion_sensor_settings);
		original_motion_sensor_settings = NULL;
	}

	if (OriginalMonsterDisplays) {
		for (int i = 0; i < NUMBER_OF_MONSTER_TYPES; i++)
			MonsterDisplays[i] = OriginalMonsterDisplays[i];
		free(OriginalMonsterDisplays);
		OriginalMonsterDisplays = NULL;
	}
}

void parse_mml_motion_sensor(const InfoTree& root)
{
	// back up old values first
	if (!original_motion_sensor_settings) {
		original_motion_sensor_settings = (struct motion_sensor_definition *) malloc(sizeof(struct motion_sensor_definition));
		assert(original_motion_sensor_settings);
		*original_motion_sensor_settings = motion_sensor_settings;
	}
	
	if (!OriginalMonsterDisplays) {
		OriginalMonsterDisplays = (short *) malloc(sizeof(short) * NUMBER_OF_MONSTER_TYPES);
		assert(OriginalMonsterDisplays);
		for (int i = 0; i < NUMBER_OF_MONSTER_TYPES; i++)
			OriginalMonsterDisplays[i] = MonsterDisplays[i];
	}

	root.read_attr("scale", MOTION_SENSOR_SCALE);
	short range;
	if (root.read_wu("range", range))
		MOTION_SENSOR_RANGE = range;
	root.read_attr("update_frequency", MOTION_SENSOR_UPDATE_FREQUENCY);
	root.read_attr("rescan_frequency", MOTION_SENSOR_RESCAN_FREQUENCY);
	
	BOOST_FOREACH(InfoTree assign, root.children_named("assign"))
	{
		int16 index;
		if (!assign.read_indexed("monster", index, NUMBER_OF_MONSTER_TYPES))
			continue;
		assign.read_indexed("type", MonsterDisplays[index], NUMBER_OF_MDISPTYPES);
	}
}
