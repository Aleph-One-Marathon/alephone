#ifndef __RENDER_H
#define __RENDER_H

/*
RENDER.H
Thursday, September 8, 1994 5:40:59 PM  (Jason)

Feb 18, 2000 (Loren Petrich):
	Added "show_weapons_in_hand" to "view_data", so as to have support third-person
	as well as first-person view

Feb 25, 2000 (Loren Petrich):
	Added support for tunnel-vision mode (sniper mode);
	this includes "tunnel_vision_active" in "view_data"

May 22, 2000 (Loren Petrich):
	Moved field-of-view definitions into ViewControl.c;
	using accessors to that file to get field-of-view parameters
*/

#include "world.h"	
#include "textures.h"
// Stuff to control the view
#include "ViewControl.h"

/* ---------- constants */

/* the distance behind which we are not required to draw objects */
#define MINIMUM_OBJECT_DISTANCE ((short)(WORLD_ONE/20))

#define MINIMUM_VERTICES_PER_SCREEN_POLYGON ((short)3)
#define MAXIMUM_VERTICES_PER_SCREEN_POLYGON ((short)16)

// LP change: suppressed going/leaving states, because of alternative way of
// adjusting the FOV.
enum /* render effects */
{
	_render_effect_fold_in,
	_render_effect_fold_out,
	// _render_effect_going_fisheye,
	// _render_effect_leaving_fisheye,
	_render_effect_explosion,
	// LP additions:
	// _render_effect_going_tunnel,
	// _render_effect_leaving_tunnel
};

enum /* shading tables */
{
	_shading_normal, /* to black */
	_shading_infravision /* false color */
};

/*
// LP addition: tunnel vision
#define TUNNEL_VISION_FIELD_OF_VIEW 30
#define NORMAL_FIELD_OF_VIEW 80
#define EXTRAVISION_FIELD_OF_VIEW 130
*/
// LP change: using accessors instead
#define TUNNEL_VISION_FIELD_OF_VIEW View_FOV_TunnelVision()
#define NORMAL_FIELD_OF_VIEW View_FOV_Normal()
#define EXTRAVISION_FIELD_OF_VIEW View_FOV_ExtraVision()


/* ---------- structures */

struct point2d
{
	short x, y;
};
typedef struct point2d point2d;

struct definition_header
{
	short tag;
	short clip_left, clip_right;
};

struct view_data
{
	// LP change: specifying current and target field-of-view as floats;
	// one changes the field of view by setting a new target then adjusting
	// the current FOV toward it
	float field_of_view;
	float target_field_of_view;
	// short field_of_view; /* width of the view cone, in degrees (!) */
	short standard_screen_width; /* this is *not* the width of the projected image (see initialize_view_data() in RENDER.C */
	short screen_width, screen_height; /* dimensions of the projected image */
	short horizontal_scale, vertical_scale;
	
	short half_screen_width, half_screen_height;
	short world_to_screen_x, world_to_screen_y;
	short dtanpitch; /* world_to_screen*tan(pitch) */
	angle half_cone; /* often ==field_of_view/2 (when screen_width==standard_screen_width) */
	angle half_vertical_cone;

	world_vector2d untransformed_left_edge, untransformed_right_edge;
	world_vector2d left_edge, right_edge, top_edge, bottom_edge;

	short ticks_elapsed;
	long tick_count; /* for effects and transfer modes */
	short origin_polygon_index;
	angle yaw, pitch, roll;
	world_point3d origin;
	fixed maximum_depth_intensity; /* in fixed units */

	short shading_mode;

	short effect, effect_phase;
	short real_world_to_screen_x, real_world_to_screen_y;
	
	boolean overhead_map_active;
	short overhead_map_scale;

	boolean under_media_boundary;
	short under_media_index;
	
	boolean terminal_mode_active;
	
	// LP addition: this indicates whether to show weapons-in-hand display;
	// this is on in first-person, off in third-person
	boolean show_weapons_in_hand;
	
	// LP: Indicates whether or not tunnel vision is active
	boolean tunnel_vision_active;
	
	// LP addition: value of yaw used by landscapes; this is so that the center
	// can stay stationary
	angle landscape_yaw;
};

/* ---------- render flags */

#define TEST_STATE_FLAG(i,f) TEST_RENDER_FLAG(i,f)
#define SET_STATE_FLAG(i,f,v) SET_RENDER_FLAG(i,f)

#define TEST_RENDER_FLAG(index, flag) (render_flags[index]&(flag))
#define SET_RENDER_FLAG(index, flag) render_flags[index]|= (flag)
#define RENDER_FLAGS_BUFFER_SIZE (8*KILO)
enum /* render flags */
{
	_polygon_is_visible_bit, /* some part of this polygon is horizontally in the view cone */
	_endpoint_has_been_visited_bit, /* we’ve already tried to cast a ray out at this endpoint */
	_endpoint_is_visible_bit, /* this endpoint is horizontally in the view cone */
	_side_is_visible_bit, /* this side was crossed while building the tree and should be drawn */
	_line_has_clip_data_bit, /* this line has a valid clip entry */
	_endpoint_has_clip_data_bit, /* this endpoint has a valid clip entry */
	_endpoint_has_been_transformed_bit, /* this endpoint has been transformed into screen-space */
	NUMBER_OF_RENDER_FLAGS, /* should be <=16 */

	_polygon_is_visible= 1<<_polygon_is_visible_bit,
	_endpoint_has_been_visited= 1<<_endpoint_has_been_visited_bit,
	_endpoint_is_visible= 1<<_endpoint_is_visible_bit,
	_side_is_visible= 1<<_side_is_visible_bit,
	_line_has_clip_data= 1<<_line_has_clip_data_bit,
	_endpoint_has_clip_data= 1<<_endpoint_has_clip_data_bit,
	_endpoint_has_been_transformed= 1<<_endpoint_has_been_transformed_bit
};

/* ---------- globals */

extern uint16 *render_flags;

/* ---------- prototypes/RENDER.C */

void allocate_render_memory(void);

void initialize_view_data(struct view_data *view);
void render_view(struct view_data *view, struct bitmap_definition *destination);

void start_render_effect(struct view_data *view, short effect);


/* ----------- prototypes/SCREEN.C */
void render_overhead_map(struct view_data *view);
void render_computer_interface(struct view_data *view);

#include "scottish_textures.h"

// LP: definitions moved up here because they are referred to
// outside of render.c, where they are defined.

void instantiate_rectangle_transfer_mode(view_data *view,
	rectangle_definition *rectangle, short transfer_mode, fixed transfer_phase);

void instantiate_polygon_transfer_mode(view_data *view,
	polygon_definition *polygon, short transfer_mode, short transfer_phase, boolean horizontal);

#endif
