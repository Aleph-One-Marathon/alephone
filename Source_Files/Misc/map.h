#ifndef __MAP_H
#define __MAP_H

/*
MAP.H
Sunday, August 15, 1993 12:11:35 PM

Feb 10, 2000 (Loren Petrich):
	Added dynamic-limits setting of MAXIMUM_OBJECTS_PER_MAP

Jul 1, 2000 (Loren Petrich):
	Made all the accessors inline

[Loren Petrich: notes moved here]
MAP_ACCESSORS.C
Friday, June 3, 1994 12:10:10 PM

Thursday, June 16, 1994 7:32:13 PM
	if DEBUG is defined, this file is empty.

Jul 1, 2000 (Loren Petrich):
	moved all its contens out to map.h
[End moved notes]
*/

#include "world.h"
// LP addition:
#include "dynamic_limits.h"

/* ---------- constants */

#define TICKS_PER_SECOND 30
#define TICKS_PER_MINUTE (60*TICKS_PER_SECOND)

#define MAP_INDEX_BUFFER_SIZE 8192
#define MINIMUM_SEPARATION_FROM_WALL (WORLD_ONE/4)
#define MINIMUM_SEPARATION_FROM_PROJECTILE ((3*WORLD_ONE)/4)

#define TELEPORTING_DURATION (2*TELEPORTING_MIDPOINT)
#define TELEPORTING_MIDPOINT (TICKS_PER_SECOND/2)

/* These arrays are the absolute limits, and are used only by the small memory allocating */
/*  arrays.  */
#define MAXIMUM_POLYGONS_PER_MAP (KILO)
#define MAXIMUM_SIDES_PER_MAP (4*KILO)
#define MAXIMUM_ENDPOINTS_PER_MAP (8*KILO)
#define MAXIMUM_LINES_PER_MAP (4*KILO)
#define MAXIMUM_LEVELS_PER_MAP (128)

#define LEVEL_NAME_LENGTH (64+1)

/* ---------- shape descriptors */

#include "shape_descriptors.h"

/* ---------- damage */

enum /* damage types */
{
	_damage_explosion,
	_damage_electrical_staff,
	_damage_projectile,
	_damage_absorbed,
	_damage_flame,
	_damage_hound_claws,
	_damage_alien_projectile,
	_damage_hulk_slap,
	_damage_compiler_bolt,
	_damage_fusion_bolt,
	_damage_hunter_bolt,
	_damage_fist,
	_damage_teleporter,
	_damage_defender,
	_damage_yeti_claws,
	_damage_yeti_projectile,
	_damage_crushing,
	_damage_lava,
	_damage_suffocation,
	_damage_goo,
	_damage_energy_drain,
	_damage_oxygen_drain,
	_damage_hummer_bolt,
	_damage_shotgun_projectile,
	NUMBER_OF_DAMAGE_TYPES
};

enum /* damage flags */
{
	_alien_damage= 0x1 /* will be decreased at lower difficulty levels */
};

struct damage_definition
{
	short type, flags;
	
	short base, random;
	fixed scale;
};

/* ---------- saved objects (initial map locations, etc.) */

#define MAXIMUM_SAVED_OBJECTS 384

enum /* map object types */
{
	_saved_monster,	/* .index is monster type */
	_saved_object,	/* .index is scenery type */
	_saved_item,	/* .index is item type */
	_saved_player,	/* .index is team bitfield */
	_saved_goal,	/* .index is goal number */
	_saved_sound_source /* .index is source type, .facing is sound volume */
};

enum /* map object flags */
{
	_map_object_is_invisible= 0x0001, /* initially invisible */
	_map_object_is_platform_sound= 0x0001,
	_map_object_hanging_from_ceiling= 0x0002, /* used for calculating absolute .z coordinate */
	_map_object_is_blind= 0x0004, /* monster cannot activate by sight */
	_map_object_is_deaf= 0x0008, /* monster cannot activate by sound */
	_map_object_floats= 0x0010, /* used by sound sources caused by media */
	_map_object_is_network_only= 0x0020 /* for items only */
	
	// top four bits is activation bias for monsters
};

#define DECODE_ACTIVATION_BIAS(f) ((f)>>12)
#define ENCODE_ACTIVATION_BIAS(b) ((b)<<12)

struct map_object /* 16 bytes */
{
	short type; /* _saved_monster, _saved_object, _saved_item, ... */
	short index;
	short facing;
	short polygon_index;
	world_point3d location; // .z is a delta
	
	word flags;
};
const int SIZEOF_map_object = 16;

typedef world_point2d saved_map_pt;
typedef struct line_data saved_line;
typedef struct side_data saved_side;
typedef struct polygon_data saved_poly;
typedef struct map_annotation saved_annotation;
typedef struct map_object saved_object;
typedef struct static_data saved_map_data;

/* ---------- map loading/new game structures */

enum { /* entry point types- this is per map level (long). */
	_single_player_entry_point= 0x01,
	_multiplayer_cooperative_entry_point= 0x02,
	_multiplayer_carnage_entry_point= 0x04,
	_capture_the_flag_entry_point= 0x08,
	_king_of_hill_entry_point= 0x10,
	_defense_entry_point= 0x20,
	_rugby_entry_point= 0x40
};

struct entry_point 
{
	short level_number;
	char level_name[64+1];
};

#define MAXIMUM_PLAYER_START_NAME_LENGTH 32

struct player_start_data 
{
	short team;
	short identifier;
	short color;
	char name[MAXIMUM_PLAYER_START_NAME_LENGTH+1]; /* PLAYER_NAME_LENGTH+1 */
};

struct directory_data {
	short mission_flags;
	short environment_flags;
	long entry_point_flags;
	char level_name[LEVEL_NAME_LENGTH];
};
const int SIZEOF_directory_data = 74;

/* ---------- map annotations */

#define MAXIMUM_ANNOTATIONS_PER_MAP 20
#define MAXIMUM_ANNOTATION_TEXT_LENGTH 64

struct map_annotation
{
	short type; /* turns into color, font, size, style, etc... */
	
	world_point2d location; /* where to draw this (lower left) */
	short polygon_index; /* only displayed if this polygon is in the automap */
	
	char text[MAXIMUM_ANNOTATION_TEXT_LENGTH];
};
const int SIZEOF_map_annotation = 72;

struct map_annotation *get_next_map_annotation(short *count);

/* ---------- ambient sound images */

#define MAXIMUM_AMBIENT_SOUND_IMAGES_PER_MAP 64

// non-directional ambient component
struct ambient_sound_image_data // 16 bytes
{
	word flags;
	
	short sound_index;
	short volume;

	short unused[5];
};
const int SIZEOF_ambient_sound_image_data = 16;

/* ---------- random sound images */

#define MAXIMUM_RANDOM_SOUND_IMAGES_PER_MAP 64

enum // sound image flags
{
	_sound_image_is_non_directional= 0x0001 // ignore direction
};

// possibly directional random sound effects
struct random_sound_image_data // 32 bytes
{
	word flags;
	
	short sound_index;
	
	short volume, delta_volume;
	short period, delta_period;
	angle direction, delta_direction;
	fixed pitch, delta_pitch;
	
	// only used at run-time; initialize to NONE
	short phase;
	
	short unused[3];
};
const int SIZEOF_random_sound_image_data = 16;

/* ---------- object structure */
// LP change: made this settable from the resource fork
#define MAXIMUM_OBJECTS_PER_MAP (get_dynamic_limit(_dynamic_limit_objects))
// #define MAXIMUM_OBJECTS_PER_MAP 384

/* SLOT_IS_USED(), SLOT_IS_FREE(), MARK_SLOT_AS_FREE(), MARK_SLOT_AS_USED() macros are also used
	for monsters, effects and projectiles */
#define SLOT_IS_USED(o) ((o)->flags&(word)0x8000)
#define SLOT_IS_FREE(o) (!SLOT_IS_USED(o))
#define MARK_SLOT_AS_FREE(o) ((o)->flags&=(word)~0x8000)
#define MARK_SLOT_AS_USED(o) ((o)->flags|=(word)0x8000)

#define OBJECT_WAS_RENDERED(o) ((o)->flags&(word)0x4000)
#define SET_OBJECT_RENDERED_FLAG(o) ((o)->flags|=(word)0x4000)
#define CLEAR_OBJECT_RENDERED_FLAG(o) ((o)->flags&=(word)~0x4000)

/* this field is only valid after transmogrify_object_shape is called; in terms of our pipeline, that
	means that itÕs only valid if OBJECT_WAS_RENDERED returns true *and* was cleared before
	the last call to render_scene() ... this means that if OBJECT_WAS_RENDERED returns false,
	the monster and projectile managers will probably call transmogrif_object_shape themselves.
	for reasons beyond this scope of this comment to explain, the keyframe cannot be frame zero!
	also, when any of the flags below are set, the phase of the .sequence field can be examined
	to determine exactly how many ticks the last frame took to animate (that is, .sequence.phase
	is not reset until the next loop). */
#define OBJECT_WAS_ANIMATED(o) ((o)->flags&(word)_obj_animated)
#define GET_OBJECT_ANIMATION_FLAGS(o) ((o)->flags&(word)0x3c00)
#define SET_OBJECT_ANIMATION_FLAGS(o,n) { (o)->flags&= (word)~0x3c00; (o)->flags|= (n); }
enum /* object was animated flags */
{
	_obj_not_animated= 0x0000, /* nothing happened */
	_obj_animated= 0x2000, /* a new frame was reached */
	_obj_keyframe_started= 0x1000, /* the key-frame was reached */
	_obj_last_frame_animated= 0x0800, /* sequence complete, returning to first frame */
	_obj_transfer_mode_finished= 0x0400 /* transfer mode phase is about to loop */
};

#define GET_OBJECT_SCALE_FLAGS(o) (((o)->flags)&OBJECT_SCALE_FLAGS_MASK)
enum /* object scale flags */
{
	_object_is_enlarged= 0x0200,
	_object_is_tiny= 0x0100,

	OBJECT_SCALE_FLAGS_MASK= _object_is_enlarged|_object_is_tiny
};

#define OBJECT_IS_MEDIA_EFFECT(o) ((o)->flags&128)
#define SET_OBJECT_IS_MEDIA_EFFECT(o) ((o)->flags|= 128)

/* ignored by renderer if INVISIBLE */
#define OBJECT_IS_INVISIBLE(o) ((o)->flags&(word)32)
#define OBJECT_IS_VISIBLE(o) (!OBJECT_IS_INVISIBLE(o))
#define SET_OBJECT_INVISIBILITY(o,v) ((void)((v)?((o)->flags|=(word)32):((o)->flags&=(word)~32)))

/* call get_object_dimensions(object_index, &radius, &height) for SOLID objects to get their dimensions */
#define OBJECT_IS_SOLID(o) ((o)->flags&(word)16)
#define SET_OBJECT_SOLIDITY(o,v) ((void)((v)?((o)->flags|=(word)16):((o)->flags&=(word)~16)))

#define GET_OBJECT_STATUS(o) ((o)->flags&(word)8)
#define SET_OBJECT_STATUS(o,v) ((v)?((o)->flags|=(word)8):((o)->flags&=(word)~8))
#define TOGGLE_OBJECT_STATUS(o) ((o)->flags^=(word)8)

#define GET_OBJECT_OWNER(o) ((o)->flags&(word)7)
#define SET_OBJECT_OWNER(o,n) { assert((n)>=0&&(n)<=7); (o)->flags&= (word)~7; (o)->flags|= (n); }
enum /* object owners (8) */
{
	_object_is_normal, /* normal */
	_object_is_scenery, /* impassable scenery */
	_object_is_monster, /* monster index in .permutation */
	_object_is_projectile, /* active projectile index in .permutation */
	_object_is_effect, /* explosion or something; index in .permutation */
	_object_is_item, /* .permutation is item type */
	_object_is_device, /* status given by bit in flags field, device type in .permutation */
	_object_is_garbage /* will be removed by garbage collection algorithms */
};

/* because of sign problems, we must rip out the values before we modify them; frame is
	in [0,16), phase is in [0,4096) ... this is for shape animations */
#define GET_SEQUENCE_FRAME(s) ((s)>>12)
#define GET_SEQUENCE_PHASE(s) ((s)&4095)
#define BUILD_SEQUENCE(f,p) (((f)<<12)|(p))

enum /* object transfer modes (high-level) */
{
	_xfer_normal,
	_xfer_fade_out_to_black, /* reduce ambient light until black, then tint-fade out */
	_xfer_invisibility,
	_xfer_subtle_invisibility,
	_xfer_pulsate, /* only valid for polygons */
	_xfer_wobble, /* only valid for polygons */
	_xfer_fast_wobble, /* only valid for polygons */
	_xfer_static,
	_xfer_50percent_static,
	_xfer_landscape,
	_xfer_smear, /* repeat pixel(0,0) of texture everywhere */
	_xfer_fade_out_static,
	_xfer_pulsating_static,
	_xfer_fold_in, /* appear */
	_xfer_fold_out, /* disappear */
	_xfer_horizontal_slide,
	_xfer_fast_horizontal_slide,
	_xfer_vertical_slide,
	_xfer_fast_vertical_slide,
	_xfer_wander,
	_xfer_fast_wander,
	_xfer_big_landscape,
	NUMBER_OF_TRANSFER_MODES
};

struct object_location
{
	struct world_point3d p;
	short polygon_index;
	
	angle yaw, pitch;
	
	word flags;
};

struct object_data /* 32 bytes */
{
	/* these fields are in the order of a world_location3d structure, but are missing the pitch
		and velocity fields */
	world_point3d location;
	short polygon;
	
	angle facing;
	
	/* this is not really a shape descriptor: (and this is the only place in the game where you
		find this pseudo-shape_descriptor type) the collection is valid, as usual, but the
		shape index is an index into the animated shape array for that collection. */
	shape_descriptor shape;

	word sequence; /* for shape animation */
	word flags; /* [used_slot.1] [rendered.1] [animated.4] [unused.4] [invisible.1] [solid.1] [status.1] [owner.3] */
	short transfer_mode, transfer_period; /* if NONE take from shape data */
	short transfer_phase; /* for transfer mode animations */
	short permutation; /* usually index into owner array */
	
	short next_object; /* or NONE */
	short parasitic_object; /* or NONE */

	/* used when playing sounds */
	fixed sound_pitch;
};

/* ------------ endpoint definition */

#define ENDPOINT_IS_SOLID(e) ((e)->flags&1)
#define SET_ENDPOINT_SOLIDITY(e,s) ((s)?((e)->flags|=1):((e)->flags&=~(word)1))

#define ENDPOINT_IS_TRANSPARENT(e) ((e)->flags&4)
#define SET_ENDPOINT_TRANSPARENCY(e,s) ((s)?((e)->flags|=4):((e)->flags&=~(word)4))

/* FALSE if all polygons sharing this endpoint have the same height */
#define ENDPOINT_IS_ELEVATION(e) ((e)->flags&2)
#define SET_ENDPOINT_ELEVATION(e,s) ((s)?((e)->flags|=2):((e)->flags&=~(word)2))

struct endpoint_data /* 16 bytes */
{
	word flags;
	world_distance highest_adjacent_floor_height, lowest_adjacent_ceiling_height;
	
	world_point2d vertex;
	world_point2d transformed;
	
	short supporting_polygon_index;
};
const int SIZEOF_endpoint_data = 16;

/* ------------ line definition */

#define SOLID_LINE_BIT 0x4000
#define TRANSPARENT_LINE_BIT 0x2000
#define LANDSCAPE_LINE_BIT 0x1000
#define ELEVATION_LINE_BIT 0x800
#define VARIABLE_ELEVATION_LINE_BIT 0x400
#define LINE_HAS_TRANSPARENT_SIDE_BIT 0x200

#define SET_LINE_SOLIDITY(l,v) ((v)?((l)->flags|=(word)SOLID_LINE_BIT):((l)->flags&=(word)~SOLID_LINE_BIT))
#define LINE_IS_SOLID(l) ((l)->flags&SOLID_LINE_BIT)

#define SET_LINE_TRANSPARENCY(l,v) ((v)?((l)->flags|=(word)TRANSPARENT_LINE_BIT):((l)->flags&=(word)~TRANSPARENT_LINE_BIT))
#define LINE_IS_TRANSPARENT(l) ((l)->flags&TRANSPARENT_LINE_BIT)

#define SET_LINE_LANDSCAPE_STATUS(l,v) ((v)?((l)->flags|=(word)LANDSCAPE_LINE_BIT):((l)->flags&=(word)~LANDSCAPE_LINE_BIT))
#define LINE_IS_LANDSCAPED(l) ((l)->flags&LANDSCAPE_LINE_BIT)

#define SET_LINE_ELEVATION(l,v) ((v)?((l)->flags|=(word)ELEVATION_LINE_BIT):((l)->flags&=(word)~ELEVATION_LINE_BIT))
#define LINE_IS_ELEVATION(l) ((l)->flags&ELEVATION_LINE_BIT)

#define SET_LINE_VARIABLE_ELEVATION(l,v) ((v)?((l)->flags|=(word)VARIABLE_ELEVATION_LINE_BIT):((l)->flags&=(word)~VARIABLE_ELEVATION_LINE_BIT))
#define LINE_IS_VARIABLE_ELEVATION(l) ((l)->flags&VARIABLE_ELEVATION_LINE_BIT)

#define SET_LINE_HAS_TRANSPARENT_SIDE(l,v) ((v)?((l)->flags|=(word)LINE_HAS_TRANSPARENT_SIDE_BIT):((l)->flags&=(word)~LINE_HAS_TRANSPARENT_SIDE_BIT))
#define LINE_HAS_TRANSPARENT_SIDE(l) ((l)->flags&LINE_HAS_TRANSPARENT_SIDE_BIT)

struct line_data /* 32 bytes */
{
	short endpoint_indexes[2];
	word flags; /* no permutation field */

	world_distance length;
	world_distance highest_adjacent_floor, lowest_adjacent_ceiling;
	
	/* the side definition facing the clockwise polygon which references this side, and the side
		definition facing the counterclockwise polygon (can be NONE) */
	short clockwise_polygon_side_index, counterclockwise_polygon_side_index;
	
	/* a line can be owned by a clockwise polygon, a counterclockwise polygon, or both (but never
		two of the same) (can be NONE) */
	short clockwise_polygon_owner, counterclockwise_polygon_owner;
	
	short unused[6];
};
const int SIZEOF_line_data = 32;

/* --------------- side definition */

enum /* side flags */
{
	_control_panel_status= 0x0001,
	_side_is_control_panel= 0x0002,
	_side_is_repair_switch= 0x0004, // must be toggled to exit level
	_side_is_destructive_switch= 0x0008, // uses an item
	_side_is_lighted_switch= 0x0010, // switch must be lighted to use
	_side_switch_can_be_destroyed= 0x0020, // projectile hits toggle and destroy this switch
	_side_switch_can_only_be_hit_by_projectiles= 0x0040,

	_editor_dirty_bit= 0x4000 // used by the editor...
};

enum /* control panel side types */
{
	_panel_is_oxygen_refuel,
	_panel_is_shield_refuel,
	_panel_is_double_shield_refuel,
	_panel_is_triple_shield_refuel,
	_panel_is_light_switch, // light index in .permutation
	_panel_is_platform_switch, // platform index in .permutation
	_panel_is_tag_switch, // tag in .permutation (NONE is tagless)
	_panel_is_pattern_buffer,
	_panel_is_computer_terminal,
	NUMBER_OF_CONTROL_PANELS
};

#define SIDE_IS_CONTROL_PANEL(s) ((s)->flags & _side_is_control_panel)
#define SET_SIDE_CONTROL_PANEL(s, t) ((void)((t) ? (s->flags |= (word) _side_is_control_panel) : (s->flags &= (word)~_side_is_control_panel)))

#define GET_CONTROL_PANEL_STATUS(s) ((s)->flags & _control_panel_status)
#define SET_CONTROL_PANEL_STATUS(s, t) ((t) ? (s->flags |= (word) _control_panel_status) : (s->flags &= (word)~_control_panel_status))
#define TOGGLE_CONTROL_PANEL_STATUS(s) ((s)->flags ^= _control_panel_status)

#define SIDE_IS_REPAIR_SWITCH(s) ((s)->flags & _side_is_repair_switch)
#define SET_SIDE_IS_REPAIR_SWITCH(s, t) ((t) ? (s->flags |= (word) _side_is_repair_switch) : (s->flags &= (word)~_side_is_repair_switch))

/* Flags used by Vulcan */
#define SIDE_IS_DIRTY(s) ((s)->flags&_editor_dirty_bit)
#define SET_SIDE_IS_DIRTY(s, t) ((t)?(s->flags|=(word)_editor_dirty_bit):(s->flags&=(word)~_editor_dirty_bit))

enum /* side types (largely redundant; most of this could be guessed for examining adjacent polygons) */
{
	_full_side, /* primary texture is mapped floor-to-ceiling */
	_high_side, /* primary texture is mapped on a panel coming down from the ceiling (implies 2 adjacent polygons) */
	_low_side, /* primary texture is mapped on a panel coming up from the floor (implies 2 adjacent polygons) */
	_composite_side, /* primary texture is mapped floor-to-ceiling, secondary texture is mapped into it (i.e., control panel) */
	_split_side /* primary texture is mapped onto a panel coming down from the ceiling, secondary
		texture is mapped on a panel coming up from the floor */
};

struct side_texture_definition
{
	world_distance x0, y0;
	shape_descriptor texture;
};

struct side_exclusion_zone
{
	world_point2d e0, e1, e2, e3;
};

struct side_data /* 64 bytes */
{
	short type;
	word flags;
	
	struct side_texture_definition primary_texture;
	struct side_texture_definition secondary_texture;
	struct side_texture_definition transparent_texture; /* not drawn if .texture==NONE */

	/* all sides have the potential of being impassable; the exclusion zone is the area near
		the side which cannot be walked through */
	struct side_exclusion_zone exclusion_zone;

	short control_panel_type; /* Only valid if side->flags & _side_is_control_panel */
	short control_panel_permutation; /* platform index, light source index, etc... */
	
	short primary_transfer_mode; /* These should be in the side_texture_definition.. */
	short secondary_transfer_mode;
	short transparent_transfer_mode;

	short polygon_index, line_index;

	short primary_lightsource_index;	
	short secondary_lightsource_index;
	short transparent_lightsource_index;

	fixed ambient_delta;

	short unused[1];
};
const int SIZEOF_side_data = 64;

/* ----------- polygon definition */

#define MAXIMUM_VERTICES_PER_POLYGON 8

enum /* polygon types */
{
	_polygon_is_normal,
	_polygon_is_item_impassable,
	_polygon_is_monster_impassable,
	_polygon_is_hill, /* for king-of-the-hill */
	_polygon_is_base, /* for capture the flag, rugby, etc. (team in .permutation) */
	_polygon_is_platform, /* platform index in .permutation */
	_polygon_is_light_on_trigger, /* lightsource index in .permutation */
	_polygon_is_platform_on_trigger, /* polygon index in .permutation */
	_polygon_is_light_off_trigger, /* lightsource index in .permutation */
	_polygon_is_platform_off_trigger, /* polygon index in .permutation */
	_polygon_is_teleporter, /* .permutation is polygon_index of destination */
	_polygon_is_zone_border,
	_polygon_is_goal,
	_polygon_is_visible_monster_trigger,
	_polygon_is_invisible_monster_trigger,
	_polygon_is_dual_monster_trigger,
	_polygon_is_item_trigger, /* activates all items in this zone */
	_polygon_must_be_explored,
	_polygon_is_automatic_exit /* if success conditions are met, causes automatic transport too next level */
};

#define POLYGON_IS_DETACHED_BIT 0x4000
#define POLYGON_IS_DETACHED(p) ((p)->flags&POLYGON_IS_DETACHED_BIT)
#define SET_POLYGON_DETACHED_STATE(p, v) ((v)?((p)->flags|=POLYGON_IS_DETACHED_BIT):((p)->flags&=~POLYGON_IS_DETACHED_BIT))

struct horizontal_surface_data /* should be in polygon structure */
{
	world_distance height;
	short lightsource_index;
	shape_descriptor texture;
	short transfer_mode, transfer_mode_data;
	
	world_point2d origin;
};

struct polygon_data /* 128 bytes */
{
	short type;
	word flags;
	short permutation;

	short vertex_count;
	short endpoint_indexes[MAXIMUM_VERTICES_PER_POLYGON]; /* clockwise */
	short line_indexes[MAXIMUM_VERTICES_PER_POLYGON];
	
	shape_descriptor floor_texture, ceiling_texture;
	world_distance floor_height, ceiling_height;
	short floor_lightsource_index, ceiling_lightsource_index;
	
	long area; /* in world_distance^2 units */
	
	short first_object;
	
	/* precalculated impassability information; each polygon has a list of lines and points
		that anything big (i.e., monsters but not projectiles) inside it must check against when
		ending a move inside it. */
	short first_exclusion_zone_index;
	short line_exclusion_zone_count;
	short point_exclusion_zone_count;

	short floor_transfer_mode;
	short ceiling_transfer_mode;
	
	short adjacent_polygon_indexes[MAXIMUM_VERTICES_PER_POLYGON];
	
	/* a list of polygons within WORLD_ONE of us */
	short first_neighbor_index;
	short neighbor_count;
	
	world_point2d center;
	
	short side_indexes[MAXIMUM_VERTICES_PER_POLYGON];
	
	world_point2d floor_origin, ceiling_origin;
	
	short media_index;
	short media_lightsource_index;
	
	/* NONE terminated list of _saved_sound_source indexes which must be checked while a
		listener is inside this polygon (can be none) */
	short sound_source_indexes;
	
	// either can be NONE
	short ambient_sound_image_index;
	short random_sound_image_index;
	
	short unused[1];
};
const int SIZEOF_polygon_data = 128;

/* ---------- random placement data structures.. */

enum /* game difficulty levels */
{
	_wuss_level,
	_easy_level,
	_normal_level,
	_major_damage_level,
	_total_carnage_level,
	NUMBER_OF_GAME_DIFFICULTY_LEVELS
};

/* ---------- new object frequency structures. */

#define MAXIMUM_OBJECT_TYPES 64

enum // flags for object_frequency_definition
{
	_reappears_in_random_location= 0x0001
};

struct object_frequency_definition
{
	word flags;
	
	short initial_count;   // number that initially appear. can be greater than maximum_count
	short minimum_count;   // this number of objects will be maintained.
	short maximum_count;   // canÕt exceed this, except at the beginning of the level.
	
	short random_count;    // maximum random occurences of the object
	word random_chance;    // in (0, 65535]
};

/* ---------- map */

enum /* mission flags */
{
	_mission_none= 0x0000,
	_mission_extermination= 0x0001,
	_mission_exploration= 0x0002,
	_mission_retrieval= 0x0004,
	_mission_repair= 0x0008,
	_mission_rescue= 0x0010
};

enum /* environment flags */
{
	_environment_normal= 0x0000,
	_environment_vacuum= 0x0001, // prevents certain weapons from working, player uses oxygen
	_environment_magnetic= 0x0002, // motion sensor works poorly
	_environment_rebellion= 0x0004, // makes clients fight pfhor
	_environment_low_gravity= 0x0008, // low gravity

	_environment_network= 0x2000,	// these two pseudo-environments are used to prevent items 
	_environment_single_player= 0x4000 // from arriving in the items.c code.
};

/* current map number is in player->map */
struct static_data
{
	short environment_code;
	
	short physics_model;
	short song_index;
	short mission_flags;
	short environment_flags;
	
	short unused[4];

	char level_name[LEVEL_NAME_LENGTH];
	long entry_point_flags;
};
const int SIZEOF_static_data = 88;

enum /* game options.. */
{
	_multiplayer_game= 0x0001, /* multi or single? */
	_ammo_replenishes= 0x0002, /* Does or doesn't */
	_weapons_replenish= 0x0004, /* Weapons replenish? */
	_specials_replenish= 0x0008, /* Invisibility, Ammo? */
	_monsters_replenish= 0x0010, /* Monsters are lazarus.. */
	_motion_sensor_does_not_work= 0x00020, /* Motion sensor works */
	_overhead_map_is_omniscient=  0x0040, /* Only show teammates on overhead map */
	_burn_items_on_death= 0x0080, /* When you die, you lose everything but the initial crap.. */
	_live_network_stats= 0x0100,
	_game_has_kill_limit= 0x0200,  /* Game ends when the kill limit is reached. */
	_force_unique_teams= 0x0400, /* every player must have a unique team */
	_dying_is_penalized= 0x0800, /* time penalty for dying */
	_suicide_is_penalized= 0x1000, /* time penalty for killing yourselves */
	_overhead_map_shows_items= 0x2000,
	_overhead_map_shows_monsters= 0x4000,
	_overhead_map_shows_projectiles= 0x8000
};

enum // specifies how the user completed the level. saved in dynamic_data
{
	_level_unfinished, 
	_level_finished,
	_level_failed
};

/* Game types! */
enum {
	_game_of_kill_monsters,		// single player & combative use this
	_game_of_cooperative_play,	// multiple players, working together
	_game_of_capture_the_flag,	// A team game.
	_game_of_king_of_the_hill,
	_game_of_kill_man_with_ball,
	_game_of_defense,
	_game_of_rugby,
	_game_of_tag,
	NUMBER_OF_GAME_TYPES
};
#define GET_GAME_TYPE() (dynamic_world->game_information.game_type)
#define GET_GAME_OPTIONS() (dynamic_world->game_information.game_options)
#define GET_GAME_PARAMETER(x) (dynamic_world->game_information.parameters[(x)])

/* 
	Single player game:
		game_type= _game_of_kill_monsters;
		game_options= 0
*/

struct game_data 
{
	/* Used for the net game, decrement each tick.  Used for the */
	/*  single player game-> set to LONG_MAX, and decremented over time, so */
	/*  that you know how long it took you to solve the game. */
	long game_time_remaining;  
	short game_type; /* One of previous enum's */
	short game_options;
	short kill_limit;
	short initial_random_seed;
	short difficulty_level;
	short parameters[2]; /* Use these later. for now memset to 0 */
};

struct dynamic_data
{
	/* ticks since the beginning of the game */
	long tick_count;
	
	/* the real seed is static in WORLD.C; must call set_random_seed() */
	word random_seed;
	
	/* This is stored in the dynamic_data so that it is valid across */
	/* saves. */
	struct game_data game_information;
	
	short player_count;
	short speaking_player_index;
	
	short unused;
	short platform_count;
	short endpoint_count;
	short line_count;
	short side_count;
	short polygon_count;
	short lightsource_count;
	short map_index_count;
	short ambient_sound_image_count, random_sound_image_count;
	
	/* statistically unlikely to be valid */
	short object_count;
	short monster_count;
	short projectile_count;
	short effect_count;
	short light_count;
	
	short default_annotation_count;
	short personal_annotation_count;
	
	short initial_objects_count;
	
	short garbage_object_count;

	/* used by move_monsters() to decide who gets to generate paths, etc. */	
	short last_monster_index_to_get_time, last_monster_index_to_build_path;

	/* variables used by new_monster() to adjust for different difficulty levels */
	short new_monster_mangler_cookie, new_monster_vanishing_cookie;
	
	/* number of civilians killed by players; periodically decremented */
	short civilians_killed_by_players;

	/* used by the item placement stuff */
	short random_monsters_left[MAXIMUM_OBJECT_TYPES];
	short current_monster_count[MAXIMUM_OBJECT_TYPES];
	short random_items_left[MAXIMUM_OBJECT_TYPES];
	short current_item_count[MAXIMUM_OBJECT_TYPES];

	short current_level_number;   // what level the user is currently exploring.
	
	short current_civilian_causalties, current_civilian_count;
	short total_civilian_causalties, total_civilian_count;
	
	world_point2d game_beacon;
	short game_player_index;
};

/* ---------- map globals */

extern struct static_data *static_world;
extern struct dynamic_data *dynamic_world;

extern struct object_data *objects;

extern struct polygon_data *map_polygons;
extern struct side_data *map_sides;
extern struct line_data *map_lines;
extern struct endpoint_data *map_endpoints;

extern struct ambient_sound_image_data *ambient_sound_images;
extern struct random_sound_image_data *random_sound_images;

extern short *map_indexes;

extern byte *automap_lines;
extern byte *automap_polygons;

extern struct map_annotation *map_annotations;
extern struct map_object *saved_objects;

extern boolean game_is_networked; /* TRUE if this is a network game */

#define ADD_LINE_TO_AUTOMAP(i) (automap_lines[(i)>>3] |= (byte) 1<<((i)&0x07))
#define LINE_IS_IN_AUTOMAP(i) ((automap_lines[(i)>>3]&((byte)1<<((i)&0x07)))?(TRUE):(FALSE))

#define ADD_POLYGON_TO_AUTOMAP(i) (automap_polygons[(i)>>3] |= (byte) 1<<((i)&0x07))
#define POLYGON_IS_IN_AUTOMAP(i) ((automap_polygons[(i)>>3]&((byte)1<<((i)&0x07)))?(TRUE):(FALSE))

/* ---------- prototypes/MARATHON.C */

void initialize_marathon(void);

void leaving_map(void);
boolean entering_map(void);

short update_world(void);

/* Called to activate lights, platforms, etc. (original polygon may be NONE) */
void changed_polygon(short original_polygon_index, short new_polygon_index, short player_index);

short calculate_damage(struct damage_definition *damage);
void cause_polygon_damage(short polygon_index, short monster_index);

short calculate_level_completion_state(void);

/* ---------- prototypes/MAP.C */

void allocate_map_memory(void);
void initialize_map_for_new_game(void);
void initialize_map_for_new_level(void);

void mark_environment_collections(short environment_code, boolean loading);
boolean collection_in_environment(short collection_code, short environment_code);

boolean valid_point2d(world_point2d *p);
boolean valid_point3d(world_point3d *p);

void reconnect_map_object_list(void);
short new_map_object2d(world_point2d *location, short polygon_index, shape_descriptor shape, angle facing);
short new_map_object3d(world_point3d *location, short polygon_index, shape_descriptor shape, angle facing);
short new_map_object(struct object_location *location, shape_descriptor shape);
short attach_parasitic_object(short host_index, shape_descriptor shape, angle facing);
void remove_parasitic_object(short host_index);
boolean translate_map_object(short object_index, world_point3d *new_location, short new_polygon_index);
short find_new_object_polygon(world_point2d *parent_location, world_point2d *child_location, short parent_polygon_index);

void remove_map_object(short index);

struct shape_and_transfer_mode
{
	/* extended shape descriptor */
	short collection_code, low_level_shape_index;
	
	short transfer_mode;
	fixed transfer_phase; /* [0,FIXED_ONE] */
};

void get_object_shape_and_transfer_mode(world_point3d *camera_location, short object_index, struct shape_and_transfer_mode *data);
void set_object_shape_and_transfer_mode(short object_index, shape_descriptor shape, short transfer_mode);
void animate_object(short object_index); /* assumes ¶t==1 tick */
boolean randomize_object_sequence(short object_index, shape_descriptor shape);

void play_object_sound(short object_index, short sound_code);
void play_polygon_sound(short polygon_index, short sound_code);
void _play_side_sound(short side_index, short sound_code, fixed pitch);
void play_world_sound(short polygon_index, world_point3d *origin, short sound_code);

#define play_side_sound(side_index, sound_code) _play_side_sound(side_index, sound_code, FIXED_ONE)

void handle_random_sound_image(void);

void initialize_map_for_new_player(void);
void generate_map(short level);

short world_point_to_polygon_index(world_point2d *location);
short clockwise_endpoint_in_line(short polygon_index, short line_index, short index);

short find_adjacent_polygon(short polygon_index, short line_index);
short find_adjacent_side(short polygon_index, short line_index);
short find_shared_line(short polygon_index1, short polygon_index2);
boolean line_is_landscaped(short polygon_index, short line_index, world_distance z);
short find_line_crossed_leaving_polygon(short polygon_index, world_point2d *p0, world_point2d *p1);
boolean point_in_polygon(short polygon_index, world_point2d *p);
void find_center_of_polygon(short polygon_index, world_point2d *center);

long point_to_line_segment_distance_squared(world_point2d *p, world_point2d *a, world_point2d *b);
long point_to_line_distance_squared(world_point2d *p, world_point2d *a, world_point2d *b);

fixed closest_point_on_line(world_point2d *e0, world_point2d *e1, world_point2d *p, world_point2d *closest_point);
void closest_point_on_circle(world_point2d *c, world_distance radius, world_point2d *p, world_point2d *closest_point);

fixed find_line_intersection(world_point2d *e0, world_point2d *e1, world_point3d *p0,
	world_point3d *p1, world_point3d *intersection);
fixed find_floor_or_ceiling_intersection(world_distance h, world_point3d *p0, world_point3d *p1, world_point3d *intersection);

void ray_to_line_segment(world_point2d *p0, world_point2d *p1, angle theta, world_distance d);

void push_out_line(world_point2d *e0, world_point2d *e1, world_distance d, world_distance line_length);
boolean keep_line_segment_out_of_walls(short polygon_index, world_point3d *p0,
	world_point3d *p1, world_distance maximum_delta_height, world_distance height, world_distance *adjusted_floor_height,
	world_distance *adjusted_ceiling_height, short *supporting_polygon_index);

fixed get_object_light_intensity(short object_index);

boolean line_has_variable_height(short line_index);

void recalculate_map_counts(void);

boolean change_polygon_height(short polygon_index, world_distance new_floor_height,
	world_distance new_ceiling_height, struct damage_definition *damage);

boolean line_is_obstructed(short polygon_index1, world_point2d *p1, short polygon_index2, world_point2d *p2);
boolean point_is_player_visible(short max_players, short polygon_index, world_point2d *p, long *distance);
boolean point_is_monster_visible(short polygon_index, world_point2d *p, long *distance);

void turn_object_to_shit(short garbage_object_index);

void random_point_on_circle(world_point3d *center, short center_polygon_index,
	world_distance radius, world_point3d *random_point, short *random_polygon_index);

void calculate_line_midpoint(short line_index, world_point3d *midpoint);

void *get_map_structure_chunk(long chunk_size);
void reallocate_map_structure_memory(long size);

/* ---------- prototypes/MAP_ACCESSORS.C */

// LP changed: inlined all of these;  when the index is out of range,
// the geometry ones make failed asserts,
// while the sound ones return null pointers.

inline struct object_data *get_object_data(
	const short object_index)
{
	struct object_data *object = GetMemberWithBounds(objects,object_index,MAXIMUM_OBJECTS_PER_MAP);
	
	vassert(object, csprintf(temporary, "object index #%d is out of range", object_index));
	vassert(SLOT_IS_USED(object), csprintf(temporary, "object index #%d is unused", object_index));
	
	return object;
}

inline struct polygon_data *get_polygon_data(
	const short polygon_index)
{
	assert(map_polygons);	
	struct polygon_data *polygon = GetMemberWithBounds(map_polygons,polygon_index,dynamic_world->polygon_count);
	
	vassert(map_polygons, csprintf(temporary, "polygon index #%d is out of range", polygon_index));
	
	return polygon;
}

inline struct line_data *get_line_data(
	const short line_index)
{
	assert(map_lines);
	struct line_data *line = GetMemberWithBounds(map_lines,line_index,dynamic_world->line_count);
	
	vassert(line, csprintf(temporary, "line index #%d is out of range", line_index));
	
	return line;
}

inline struct side_data *get_side_data(
	const short side_index)
{
	assert(map_sides);
	struct side_data *side = GetMemberWithBounds(map_sides,side_index,dynamic_world->side_count);
	
	vassert(side, csprintf(temporary, "side index #%d is out of range", side_index));
	
	return side;
}

inline struct endpoint_data *get_endpoint_data(
	const short endpoint_index)
{
	assert(map_endpoints);
	struct endpoint_data *endpoint = GetMemberWithBounds(map_endpoints,endpoint_index,dynamic_world->endpoint_count);

	vassert(endpoint, csprintf(temporary, "endpoint index #%d is out of range", endpoint_index));
	
	return endpoint;
}

inline short *get_map_indexes(
	const short index,
	const short count)
{
	assert(map_indexes);
	short *map_index = GetMemberWithBounds(map_indexes,index,dynamic_world->map_index_count-count+1);
	
	vassert(map_index, csprintf(temporary, "map_indexes(#%d,#%d) are out of range", index, count));
	
	return map_index;
}

inline struct ambient_sound_image_data *get_ambient_sound_image_data(
	const short ambient_sound_image_index)
{
	return GetMemberWithBounds(ambient_sound_images,ambient_sound_image_index,dynamic_world->ambient_sound_image_count);
}

inline struct random_sound_image_data *get_random_sound_image_data(
	const short random_sound_image_index)
{
	return GetMemberWithBounds(random_sound_images,random_sound_image_index,dynamic_world->random_sound_image_count);
}

/*
#ifdef DEBUG
struct object_data *get_object_data(short object_index);
struct polygon_data *get_polygon_data(short polygon_index);
struct line_data *get_line_data(short line_index);
struct side_data *get_side_data(short side_index);
struct endpoint_data *get_endpoint_data(short endpoint_index);
struct ambient_sound_image_data *get_ambient_sound_image_data(short ambient_sound_image_index);
struct random_sound_image_data *get_random_sound_image_data(short random_sound_image_index);
short *get_map_indexes(short index, short count);
#else
#define get_object_data(i) (objects+(i))
#define get_polygon_data(i) (map_polygons+(i))
#define get_line_data(i) (map_lines+(i))
#define get_side_data(i) (map_sides+(i))
#define get_endpoint_data(i) (map_endpoints+(i))
#define get_map_indexes(i, c) (map_indexes+(i))
#define get_ambient_sound_image_data(i) (ambient_sound_images+(i))
#define get_random_sound_image_data(i) (random_sound_images+(i))
#endif
*/

/* ---------- prototypes/MAP_CONSTRUCTORS.C */

short new_map_endpoint(world_point2d *where);
short duplicate_map_endpoint(short old_endpoint_index);
short new_map_line(short a, short b, short poly_a, short poly_b, short side_a, short side_b);
short duplicate_map_line(short old_line_index);
short new_map_polygon(short *line_indexes, short line_count, short floor_height,
	short ceiling_height, short floor_texture, short ceiling_texture, short lightsource_index);

void precalculate_map_indexes(void);

void touch_polygon(short polygon_index);
void recalculate_redundant_polygon_data(short polygon_index);
void recalculate_redundant_endpoint_data(short endpoint_index);
void recalculate_redundant_line_data(short line_index);
void recalculate_redundant_side_data(short side_index, short line_index);

void calculate_endpoint_polygon_owners(short endpoint_index, short *first_index, short *index_count);
void calculate_endpoint_line_owners(short endpoint_index, short *first_index, short *index_count);

void guess_side_lightsource_indexes(short side_index);

void set_map_index_buffer_size(long length);

/* ---------- prototypes/PLACEMENT.C */

void load_placement_data(struct object_frequency_definition *monsters, struct object_frequency_definition *items);
struct object_frequency_definition *get_placement_info(void);
void place_initial_objects(void);
void recreate_objects(void);
void object_was_just_added(short object_class, short object_type);
void object_was_just_destroyed(short object_class, short object_type);
short get_random_player_starting_location_and_facing(short max_player_index, short team, struct object_location *location);

void mark_all_monster_collections(boolean loading);
void load_all_monster_sounds(void);

/* ---------- prototypes/GAME_DIALOGS.C */

/* --------- prototypes/LIGHTSOURCE.C */

void update_lightsources(void);
short new_lightsource_from_old(short old_source);
void entered_polygon(short index);
void left_polygon(short index);
/* Only send _light_turning_on, _light_turning_off, _light_toggle */
void change_light_state(short lightsource_index, short state);

/* ---------- prototypes/DEVICES.C */

void mark_control_panel_shapes(boolean load);
void initialize_control_panels_for_level(void); 
void update_control_panels(void);

boolean control_panel_in_environment(short control_panel_type, short environment_code);

void change_device_state(short device_index, boolean active);
short new_device(world_point2d *location, short initial_polygon_index, 
	short type, short extra_data, boolean active);
void update_action_key(short player_index, boolean triggered);

boolean untoggled_repair_switches_on_level(void);

void assume_correct_switch_position(short switch_type, short permutation, boolean new_state);

void try_and_toggle_control_panel(short polygon_index, short line_index);

/* ---------- prototypes/GAME_WAD.C */

struct map_identifier {
	long scenario_checksum;
	short level_index;
};

void set_to_default_map(void);

/* Return TRUE if it finds the file, and it sets the mapfile to that file. */
/* Otherwise it returns FALSE, meaning that we need have the file sent to us. */
boolean use_map_file(long checksum);
boolean load_level_from_map(short level_index);
unsigned long get_current_map_checksum(void);
boolean select_map_to_use(void);

/* Call with location of NULL to get the number of start locations for a */
/* given team or player */
short get_player_starting_location_and_facing(short team, short index, 
	struct object_location *location);

void pause_game(void);
void resume_game(void);

boolean get_indexed_entry_point(struct entry_point *entry_point, 
	short *index, long type);
boolean new_game(short number_of_players, boolean network, 
	struct game_data *game_information,
	struct player_start_data *player_start_information, 
	struct entry_point *entry_point);
boolean goto_level(struct entry_point *entry, boolean new_game);
#endif
