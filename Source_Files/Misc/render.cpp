/*
RENDER.C
Thursday, September 8, 1994 1:58:20 PM  (Jason')

Friday, September 9, 1994 1:36:15 PM  (Jason')
	on the quads, in the sun.
Sunday, September 11, 1994 7:32:49 PM  (Jason')
	the clock on the 540 was wrong, yesterday was Saturday (not Friday).  on quads again, but
	back home now.  something will draw before i go to bed tonight.  dinner at the nile?
Tuesday, September 13, 1994 2:54:56 AM  (Jason')
	no fair!—— it’s still monday, really.  with the aid of some graphical debugging the clipping
	all works now and i’m trying to have the entire floor/ceiling thing going tonight (the nile
	was closed by the time i got around to taking a shower and heading out).
Friday, September 16, 1994 4:06:17 AM  (Jason')
	walls, floors and ceilings texture, wobble, etc.  contemplating objects ... maybe this will
	work after all.
Monday, September 19, 1994 11:03:49 AM  (Jason')
	unified xz_clip_vertical_polygon() and z_clip_horizontal_polygon() to get rid of the last
	known whitespace problem.  can’t wait to see what others i have.  objects now respect the
	clipping windows of all nodes they cross.
Monday, October 24, 1994 4:35:38 PM (Jason)
	fixed render sorting problem with objects of equal depth (i.e., parasitic objects). 
Tuesday, October 25, 1994 5:14:27 PM  (Jason')
	fixed object sort order with respect to nodes and height.
Wednesday, October 26, 1994 3:18:59 PM (Jason)
	fixed half of the object sort order back so that it worked: in order to correctly handle
	objects below the viewer projecting into higher polygons we need to sort objects inside
	nodes (to be draw after their walls and ceilings but before their floors).
Wednesday, November 2, 1994 3:49:57 PM (Jason)
	the bottom panel of short split sides sometimes takes on the ceiling lightsource.
Tuesday, November 8, 1994 5:29:12 PM  (Jason')
	implemented new transfer modes: _slide, _wander.  _render_effect_earthquake doesn’t work
	yet because the player can shake behind his own shape.
Thursday, December 15, 1994 12:15:55 AM  (Jason)
	the object depth sort order problem ocurrs with multiple objects in the same polygon,
	is some function of relative depths and overlap, and does not seem to involve objects at
	the same depth.  also, it seems to sort many further objects in front of a single closer
	one.
Monday, January 23, 1995 6:53:26 AM  (Jason')
	the way to fix the render object sorting problem is to ignore overlap and sort everything
	by depth, regardless.  imagine: two, far, non-overlapping objects; by the old algorithm their
	drawing order is irrelevant.  when a closer object which overlaps both of them is sorted, it
	only attempts to lie in front of the closest of the two (leaving the farthest one in an
	uncertain position).  unfortunately this will cause us to do unnecessary promotion and might
	look stupid.
Sunday, March 26, 1995 12:57:39 AM  (Jason')
	media modifications for marathon2; the object sort order problem still exists (the above
	solution indeed looked stupid).
Thursday, March 30, 1995 11:23:35 PM  (Jason')
	tried to fix object sort problem by attempting to assure that objects are always drawn in
	depth-order within a node.
Monday, June 5, 1995 8:37:42 AM  (Jason)
	blood and fire (baby).

Jan 30, 2000 (Loren Petrich):
	Added some typecasts
	Increased MAXIMUM_NODE_ALIASES to 32
	Added an "assert" for when DEBUG is off in aliases-building section in sort_render_tree().

Feb 1, 2000 (Loren Petrich):
	Added growable list of node aliases; replaced static-list node-alias code
	
Feb 4, 2000 (Loren Petrich):
	Changed halt() to assert(false) for better debugging

Feb 5, 2000 (Loren Petrich):
	Added growable lists of nodes and also clips for endpoints and lines.

Feb 6, 2000 (Loren Petrich):
	Doing initial allocations of the growable lists of various quantities as a defensive measure
	against memory leaks that seem to occur.

Feb 9, 2000 (Loren Petrich):
	Suppressed ambiguous-clip-flag debugging statement;
	it gets activated only for excessively distant objects

Feb 10, 2000 (Loren Petrich):
	Added dynamic-limits setting of MAXIMUM_RENDER_OBJECTS

Feb 14, 2000 (Loren Petrich):
	Added test for other-side polygon to LINE_IS_TRANSPARENT() check in next_polygon_along_line()

Feb 16, 2000 (Loren Petrich):
	Put in handling of overflow digits for the purpose of doing long distance correctly;
	also turning several horizontal-coordinate short integers into long ones.

Feb 17, 2000 (Loren Petrich):
	Made the sprites long-distance-friendly; there is a bug where they flip around when they
	go past half the world size, but that's a short-integer wraparound, and the relevant routine
	is in map.c.

Feb 18, 2000 (Loren Petrich):
	Added support for conditional display of weapons-in-hand; so as to support third-person
	as well as first-person view.

Feb 21, 2000 (Loren Petrich):
	Idiot-proofed next_polygon_along_line(), making it quit a loop if it searches a whole circle.

Feb 24, 2000 (Loren Petrich):
	Added animated-texture support

Mar 3, 2000 (Loren Petrich):
	Set view to normal in initialize_view_data();
	squashed persistent-extravision bug.

Mar 5, 2000 (Loren Petrich):
	Moved extravision-persistence bug fix out of this file.

Mar 9, 2000 (Loren Petrich):
	Sorted the nodes by polygon index in sort_render_tree() and used them to speed up searches
	for nodes with the same polygon index; maps with slowed-down visibility calculations,
	such as Desla, can become twice as fast.

Mar 12, 2000 (Loren Petrich):
	Added OpenGL support

Mar 14, 2000 (Loren Petrich):
	Modified data transmitted to OpenGL renderer; it's now collection/color/frame for
	both walls and sprites. Also added transmission of view data.

Mar 24, 2000 (Loren Petrich):
	Added landscape_yaw calculation; this is the yaw of the landscapes' left edges

Mar 30, 2000 (Loren Petrich):
	Inspired by Rhys Hill's work, I've created a second tree to contain the visibility nodes;
	in addition to their visibility tree, they have a polygon-sort tree.
	This tree is implemented by setting up some additional members of node_data
	for indicating its structure; there are members for polygon >, polygon <,
	and the next member of a chain that shares polygon-index values.
	As a result, the node-aliases list can now be abolished once and for all.

Jun 11, 2000 (Loren Petrich):
	Added support for see-through liquids; this requires several changes.
	The rendering of each map polygon had to be changed so that there would be a
	separate liquid surface; it would no longer replace the floor or the ceiling.
	Next, the inhabitant objects had to be done in two passes, one other side, and one view side.
	Also, whether there is void on the other side had to be indicated, so that
	waterfalls and the like may look right.

Jun 28, 2000 (Loren Petrich):
	Fixed Aaron Davies bug; if a polygon is completely below a liquid, it will not be rendered
	if the viewpoint is above the liquid; the bug was that it was not rendered if the viewpoint
	was below the liquid. This only happened if semitransparent liquid surfaces was turned off.

Jul 10, 2000 (Loren Petrich):
	Fixed liquid visibility bug in render_tree() that happens when liquid surfaces are not semitransparent;
	rendering is skipped if the viewpoint is under a liquid and the polygon is high and dry,
	or else if the viewpoint is above a liquid and the polygon is submerged.

Jul 17, 2000 (Loren Petrich):
	Suppressed view-effect resetting in initialize_view_data(),
	in order to make teleport view-stretching work correctly.
*/

// For casting pointers to integer values -- use appropriate data type
// The definition here is for 32-bit systems; for 64-bit systems use appropriate longer one.
typedef unsigned long POINTER_DATA;
#define POINTER_CAST(x) ((POINTER_DATA)(x))


// LP addition: typecast for cross-products, since these can be large
typedef double CROSSPROD_TYPE;

#ifdef DEBUG
//#define AUTOMAP_DEBUG
//#define QUICKDRAW_DEBUG
#endif

#ifdef QUICKDRAW_DEBUG
#include "macintosh_cseries.h"
#else
#include "cseries.h"
#endif

#include "map.h"
#include "render.h"
#include "interface.h"
#include "lightsource.h"
#include "media.h"
#include "weapons.h"

// LP additions
#include "GrowableList.h"
#include "dynamic_limits.h"
#include "AnimatedTextures.h"
#include "OGL_Render.h"

#ifdef QUICKDRAW_DEBUG
#include "shell.h"
extern WindowPtr screen_window;
#endif

#include <math.h>
#include <string.h>
#include <stdlib.h>

#ifdef env68k
#pragma segment render
#endif

/* use native alignment */
#if defined (powerc) || defined (__powerc)
#pragma options align=power
#endif

/*
//render transparent walls (if a bit is set or if the transparent texture is non-NULL?)
//use side lightsources instead of taking them from their polygons
//respect dark side bit (darken light intensity by k)
//fix solid/opaque endpoint confusion (solidity does not imply opacity)

there exists a problem where an object can overlap into a polygon which is clipped by something
	behind the object but that will clip the object because clip windows are subtractive; how
	is this solved?
it’s still possible to get ambiguous clip flags, usually in very narrow (e.g., 1 pixel) windows
the renderer has a maximum range beyond which it shits bricks yet which it allows to be exceeded
it’s still possible, especially in high-res full-screen, for points to end up (slightly) off
	the screen (usually discarding these has no noticable effect on the scene)
whitespace results when two adjacent polygons are clipped to different vertical windows.  this
	is not trivially solved with the current implementation, and may be acceptable (?)

//build_base_polygon_index_list() should discard lower polygons for objects above the viewer and
//	higher polygons for objects below the viewer because we certainly don’t sort objects
//	correctly in these cases
//in strange cases, objects are sorted out of order.  this seems to involve players in some way
//	(i.e., parasitic objects).
*/

/* ---------- constants */

// LP change: increased this one to maximum possible
#define POLYGON_QUEUE_SIZE (MAXIMUM_POLYGONS_PER_MAP)
// #define POLYGON_QUEUE_SIZE 256

/* maximum number of vertices a polygon can be world-clipped into (one per clip line) */
#define MAXIMUM_VERTICES_PER_WORLD_POLYGON (MAXIMUM_VERTICES_PER_POLYGON+4)

#define EXPLOSION_EFFECT_RANGE (WORLD_ONE/12)

/* ---------- clip buffer */

#define CLIP_INDEX_BUFFER_SIZE 4096

/* ---------- flagged world points */

struct flagged_world_point2d /* for floors */
{
	// LP change: made this more long-distance friendly
	long x, y;
	// world_distance x, y;
	word flags; /* _clip_left, _clip_right, _clip_top, _clip_bottom are valid */
};
typedef struct flagged_world_point2d flagged_world_point2d;

struct flagged_world_point3d /* for ceilings */
{
	// LP change: made this more long-distance friendly
	long x, y;
	world_distance z;
	// world_distance x, y, z;
	word flags; /* _clip_left, _clip_right, _clip_top, _clip_bottom are valid */
};
typedef struct flagged_world_point3d flagged_world_point3d;

/* ---------- vertical surface definition */

/* it’s not worth putting this into the side_data structure, although the transfer mode should
	be in the side_texture_definition structure */
struct vertical_surface_data
{
	short lightsource_index;
	fixed ambient_delta; /* a delta to the lightsource’s intensity, then pinned to [0,FIXED_ONE] */
	
	world_distance length;
	world_distance h0, h1, hmax; /* h0<h1; hmax<=h1 and is the height where this wall side meets the ceiling */
	// LP change: made this more long-distance friendly
	long_vector2d p0, p1; /* will transform into left, right points on the screen (respectively) */
	// world_point2d p0, p1; /* will transform into left, right points on the screen (respectively) */
	
	struct side_texture_definition *texture_definition;
	short transfer_mode;
};

/* ---------- clipping data */

#define MAXIMUM_LINE_CLIPS 256
#define MAXIMUM_ENDPOINT_CLIPS 64
#define MAXIMUM_CLIPPING_WINDOWS 128

enum /* …_clip_data flags */
{
	_clip_left= 0x0001,
	_clip_right= 0x0002,
	_clip_up= 0x0004,
	_clip_down= 0x0008
};

enum /* left and right sides of screen */
{
	indexLEFT_SIDE_OF_SCREEN,
	indexRIGHT_SIDE_OF_SCREEN,
	NUMBER_OF_INITIAL_ENDPOINT_CLIPS
};

struct endpoint_clip_data
{
	word flags;
	short x; /* screen-space */
	
	// LP change: made into a long vector for long views
	long_vector2d vector;
	// world_vector2d vector; /* viewer-space */
};

enum /* top and bottom sides of screen */
{
	indexTOP_AND_BOTTOM_OF_SCREEN,
	NUMBER_OF_INITIAL_LINE_CLIPS
};

struct line_clip_data
{
	word flags;
	short x0, x1; /* clipping bounds */
	
	// LP change: made more long-distance friendly
	long_vector2d top_vector, bottom_vector;
	// world_vector2d top_vector, bottom_vector; /* viewer-space */
	short top_y, bottom_y; /* screen-space */
};

struct clipping_window_data
{
	// LP change: split into two sets of vectors: left-right and top-bottom
	long_vector2d left, right;
	long_vector2d top, bottom;
	// world_vector2d left, right, top, bottom; /* j is really k for top and bottom */
	short x0, x1, y0, y1;
	
	struct clipping_window_data *next_window;
};

/* ---------- nodes */

#define MAXIMUM_NODES 512

#define MAXIMUM_CLIPPING_ENDPOINTS_PER_NODE 4
#define MAXIMUM_CLIPPING_LINES_PER_NODE (MAXIMUM_VERTICES_PER_POLYGON-2)

struct node_data
{
	word flags;
	short polygon_index;
	
	/* clipping effects all children (and was caused by crossing from parent) */
	short clipping_endpoint_count;
	short clipping_endpoints[MAXIMUM_CLIPPING_LINES_PER_NODE];
	short clipping_line_count;
	short clipping_lines[MAXIMUM_CLIPPING_LINES_PER_NODE];

	struct node_data *parent; /* a pointer to our parent node (NULL for the root) */	
	struct node_data **reference; /* a pointer to the pointer which references us (NULL for the root) */
	struct node_data *siblings; /* pointer to the next sibling */
	struct node_data *children; /* pointer to the head of the child array */
	
	// LP addition: this is inspired by Rhys Hill's tree structures;
	// this is for membership in a polygon-sort tree.
	// As constructed, its root will be the first node in the list, the viewpoint node.
	struct node_data *PS_Greater;	// To nodes with higher polygon indices
	struct node_data *PS_Less;		// To nodes with lower polygon indices
	struct node_data *PS_Shared;	// To other nodes at the same polygon
};

#define INITIALIZE_NODE(node, node_polygon_index, node_flags, node_parent, node_reference) { \
	(node)->flags= (node_flags); \
	(node)->polygon_index= (node_polygon_index); \
	(node)->clipping_endpoint_count= 0; \
	(node)->clipping_line_count= 0; \
	(node)->parent= (node_parent); \
	(node)->reference= (node_reference); \
	(node)->siblings= (struct node_data *) NULL; \
	(node)->children= (struct node_data *) NULL; \
	(node)->PS_Greater = (struct node_data *) NULL; \
	(node)->PS_Less = (struct node_data *) NULL; \
	(node)->PS_Shared = (struct node_data *) NULL; \
	}
	// LP addition above: polygon-sort tree data

/* ---------- sorted nodes */

#define MAXIMUM_SORTED_NODES 128

struct sorted_node_data
{
	short polygon_index;
	
	struct render_object_data *interior_objects;
	struct render_object_data *exterior_objects;
	
	struct clipping_window_data *clipping_windows;
};

/* ---------- render objects */

// LP change: made this settable from the resource fork
#define MAXIMUM_RENDER_OBJECTS (get_dynamic_limit(_dynamic_limit_rendered))
// #define MAXIMUM_RENDER_OBJECTS 72

struct render_object_data
{
	struct sorted_node_data *node; /* node we are being drawn inside */
	struct clipping_window_data *clipping_windows; /* our privately calculated clipping window */
	
	struct render_object_data *next_object; /* the next object in this chain */
	
	struct rectangle_definition rectangle;
	
	short ymedia;
};

/* ---------- macros */

#define WRAP_LOW(n,max) ((n)?((n)-1):(max))
#define WRAP_HIGH(n,max) (((n)==(max))?0:((n)+1))

#define PUSH_POLYGON_INDEX(polygon_index) \
	if (!TEST_RENDER_FLAG(polygon_index, _polygon_is_visible)) \
	{ \
		polygon_queue[polygon_queue_size++]= polygon_index; \
		SET_RENDER_FLAG(polygon_index, _polygon_is_visible); \
	}

/* ---------- globals */

/* every time we find a unique endpoint which clips something, we build one of these for it */
// LP addition: growable list
// Length changed in calculate_endpoint_clipping_information() and ResetEndpointClips()
static GrowableList<endpoint_clip_data> EndpointClips(64);
/*
static short next_endpoint_clip_index;
static struct endpoint_clip_data *endpoint_clips, *next_endpoint_clip;
*/
/* every time we find a unique line which clips something, we build one of these for it (notice
	the translation table from line_indexes on the map to line_clip_indexes in our table for when
	we cross the same clip line again */
// LP addition: growable list
// Length changed in calculate_line_clipping_information() and ResetLineClips()
static GrowableList<line_clip_data> LineClips(256);
/*
static short next_line_clip_index;
static struct line_clip_data *line_clips, *next_line_clip;
*/
static short *line_clip_indexes; /* translates from map indexes to clip indexes, only valid if appropriate render flag is set */

// LP addition: growable lists of accumulations of endpoint and line clips
// used in build_clipping_windows()
static GrowableList<endpoint_clip_data *> AccumulatedEndpointClips(64);
static GrowableList<line_clip_data *> AccumulatedLineClips(64);

// LP addition: growable list of clipping windows
// Length changed in build_clipping_windows(), initialize_clip_data(),
// and build_aggregate_render_object_clipping_window();
// keep sorted-node clipping-window pointers in sync with render-object ones
static GrowableList<clipping_window_data> ClippingWindows(128);
// static short next_clipping_window_index; /* used only when DEBUG is TRUE */
// static struct clipping_window_data *clipping_windows, *next_clipping_window;

// LP addition: growable list of node_data values
// Length changed in cast_render_ray() and initialize_render_tree()
static GrowableList<node_data> Nodes(512);
// static short node_count; /* used only when DEBUG is TRUE */
// static struct node_data *nodes, *next_node;

// LP additions: growable list of sorted nodes
// Length changed in initialize_sorted_render_tree() and sort_render_tree()
// When being built, the render objects are yet to be listed
static GrowableList<sorted_node_data> SortedNodes(128);
// static short sorted_node_count; /* used only when DEBUG is TRUE */
// static struct sorted_node_data *sorted_nodes, *next_sorted_node;

static short polygon_queue_size;
static short *polygon_queue;

// LP additions: growable list of render objects; these are all the inhabitants
// Length changed in build_render_object()
// keep SortedNodes in sync
static GrowableList<render_object_data> RenderObjects(64);
// static short render_object_count;
// static struct render_object_data *render_objects, *next_render_object;

static short *endpoint_x_coordinates; /* gives screen x-coordinates for a map endpoint (only valid if _endpoint_is_visible) */
static struct sorted_node_data **polygon_index_to_sorted_node; /* converts map polygon indexes to sorted nodes (only valid if _polygon_is_visible) */

word *render_flags;

// LP addition: node-alias growable list
// Only used in sort_render_tree()
// Suppressed as unnecessary because of node_data polygon-sorted-tree structure
// static GrowableList<node_data *> NodeAliases(32);

// LP addition: sorting of nodes by polygon index;
// this needs a list of sorted-order node indexes and a growable list of polygon node-location
// structures; the node indexes will be long ints to be safe.
static long *PSNIndexList;
struct PolyNodeLoc
{
	short Polygon;	// Polygon index
	long LastIndex;	// The first index is the previous record's last index + 1
	// This is designed to use an initial placeholder record with contents {NONE, -1}
};
static GrowableList<PolyNodeLoc> PolyNodeLocList;

// This is to indicate the previous size allocated to, so as to be smart about reallocating.
// This list will be reallocated only when the node-list capacity increases.
static int PSNILSize = 0;

/* ---------- private prototypes */

static void update_view_data(struct view_data *view);
static void update_render_effect(struct view_data *view);
static void shake_view_origin(struct view_data *view, world_distance delta);

static void initialize_polygon_queue(struct view_data *view);
static void initialize_sorted_render_tree(void);
static void initialize_render_tree(struct view_data *view);
static void initialize_render_object_list(void);
static void initialize_clip_data(struct view_data *view);

static void build_render_object_list(struct view_data *view);
// LP change to make it more long-distance-friendly:
static struct render_object_data *build_render_object(struct view_data *view, long_point3d *origin,
	fixed ambient_intensity, struct sorted_node_data **base_nodes, short *base_node_count,
	short object_index);
/*
static struct render_object_data *build_render_object(struct view_data *view, world_point3d *origin,
	fixed ambient_intensity, struct sorted_node_data **base_nodes, short *base_node_count,
	short object_index);
*/
static void sort_render_object_into_tree(struct render_object_data *new_render_object,
	struct sorted_node_data **base_nodes, short base_node_count);
static short build_base_node_list(struct view_data *view, short origin_polygon_index,
	world_point3d *origin, world_distance left_distance, world_distance right_distance,
	struct sorted_node_data **base_nodes);
void build_aggregate_render_object_clipping_window(struct render_object_data *render_object,
	struct sorted_node_data **base_nodes, short base_node_count);

static void render_tree(struct view_data *view, struct bitmap_definition *destination);

// LP change: indicate whether the void is present on one side;
// useful for suppressing semitransparency to the void
static void render_node_floor_or_ceiling(struct view_data *view, struct bitmap_definition *destination,
	struct clipping_window_data *window, struct polygon_data *polygon, struct horizontal_surface_data *surface,
	bool void_present);
/*
static void render_node_floor_or_ceiling(struct view_data *view, struct bitmap_definition *destination,
	struct clipping_window_data *window, struct polygon_data *polygon, struct horizontal_surface_data *surface);
*/
static void render_node_side(struct view_data *view, struct bitmap_definition *destination,
	struct clipping_window_data *window, struct vertical_surface_data *surface,
	bool void_present);
/*
static void render_node_side(struct view_data *view, struct bitmap_definition *destination,
	struct clipping_window_data *window, struct vertical_surface_data *surface);
*/
// LP change: add "other side of media" flag, to indicate that the sprite will be rendered
// on the opposite side of the liquid surface from the viewpoint, instead of the same side.
static void render_node_object(struct view_data *view, struct bitmap_definition *destination,
	struct render_object_data *object, bool other_side_of_media);
/*
static void render_node_object(struct view_data *view, struct bitmap_definition *destination,
	struct render_object_data *object);
*/

static void build_render_tree(struct view_data *view);
// LP changes for better long-distance support:
static void cast_render_ray(struct view_data *view, long_vector2d *vector, short endpoint_index,
	struct node_data *parent, short bias);
/*
static void cast_render_ray(struct view_data *view, world_vector2d *vector, short endpoint_index,
	struct node_data *parent, short bias);
*/
static word next_polygon_along_line(short *polygon_index, world_point2d *origin, long_vector2d *vector,
	short *clipping_endpoint_index, short *clipping_line_index, short bias);
/*
static word next_polygon_along_line(short *polygon_index, world_point2d *origin, world_vector2d *vector,
	short *clipping_endpoint_index, short *clipping_line_index, short bias);
*/
static word decide_where_vertex_leads(short *polygon_index, short *line_index, short *side_index, short endpoint_index_in_polygon_list,
	world_point2d *origin, long_vector2d *vector, word clip_flags, short bias);
/*
static word decide_where_vertex_leads(short *polygon_index, short *line_index, short *side_index, short endpoint_index_in_polygon_list,
	world_point2d *origin, world_vector2d *vector, word clip_flags, short bias);
*/

static void calculate_line_clipping_information(struct view_data *view, short line_index, word clip_flags);
static short calculate_endpoint_clipping_information(struct view_data *view, short endpoint_index, word clip_flags);

// LP changes for better long-distance support
static short xy_clip_horizontal_polygon(flagged_world_point2d *vertices, short vertex_count,
	long_vector2d *line, word flag);
/*
static short xy_clip_horizontal_polygon(flagged_world_point2d *vertices, short vertex_count,
	world_vector2d *line, word flag);
*/
static void xy_clip_flagged_world_points(flagged_world_point2d *p0, flagged_world_point2d *p1,
	flagged_world_point2d *clipped, long_vector2d *line);
/*
static void xy_clip_flagged_world_points(flagged_world_point2d *p0, flagged_world_point2d *p1,
	flagged_world_point2d *clipped, world_vector2d *line);
*/

// LP changes for better long-distance support
static short z_clip_horizontal_polygon(flagged_world_point2d *vertices, short vertex_count,
	long_vector2d *line, world_distance height, word flag);
/*
static short z_clip_horizontal_polygon(flagged_world_point2d *vertices, short vertex_count,
	world_vector2d *line, world_distance height, word flag);
*/
static void z_clip_flagged_world_points(flagged_world_point2d *p0, flagged_world_point2d *p1,
	world_distance height, flagged_world_point2d *clipped, long_vector2d *line);
/*
static void z_clip_flagged_world_points(flagged_world_point2d *p0, flagged_world_point2d *p1,
	world_distance height, flagged_world_point2d *clipped, world_vector2d *line);
*/

static void sort_render_tree(struct view_data *view);
// LP change: using node chain instead of list of node aliases
static struct clipping_window_data *build_clipping_windows(struct view_data *view, struct node_data *ChainBegin);
/*
static struct clipping_window_data *build_clipping_windows(struct view_data *view, struct node_data **node_list, short node_count);
*/
static void calculate_vertical_clip_data(struct line_clip_data **accumulated_line_clips,
	short accumulated_line_clip_count, struct clipping_window_data *window, short x0, short x1);

static void instantiate_rectangle_transfer_mode(struct view_data *view,
	struct rectangle_definition *rectangle, short transfer_mode, fixed transfer_phase);
static void instantiate_polygon_transfer_mode(struct view_data *view,
	struct polygon_definition *polygon, short transfer_mode, short transfer_phase, boolean horizontal);

// LP changes for better long-distance support
static short xz_clip_vertical_polygon(flagged_world_point3d *vertices, short vertex_count,
	long_vector2d *line, word flag);
/*
static short xz_clip_vertical_polygon(flagged_world_point3d *vertices, short vertex_count,
	world_vector2d *line, word flag);
*/
static void xz_clip_flagged_world_points(flagged_world_point3d *p0, flagged_world_point3d *p1,
	flagged_world_point3d *clipped, long_vector2d *line);
/*
static void xz_clip_flagged_world_points(flagged_world_point3d *p0, flagged_world_point3d *p1,
	flagged_world_point3d *clipped, world_vector2d *line);
*/

// LP changes for better long-distance support

static short xy_clip_line(flagged_world_point2d *posts, short vertex_count,
	long_vector2d *line, word flag);
/*
static short xy_clip_line(flagged_world_point2d *posts, short vertex_count,
	world_vector2d *line, word flag);
*/

static void render_viewer_sprite_layer(struct view_data *view, struct bitmap_definition *destination);
static void position_sprite_axis(short *x0, short *x1, short scale_width, short screen_width,
	short positioning_mode, fixed position, boolean flip, world_distance world_left, world_distance world_right);

static struct shape_information_data *rescale_shape_information(struct shape_information_data *unscaled,
	struct shape_information_data *scaled, word flags);

// LP addition: resetters for some of the lists:
static void ResetEndpointClips(void);
static void ResetLineClips(void);

#ifdef QUICKDRAW_DEBUG
static void debug_flagged_points(flagged_world_point2d *points, short count);
static void debug_flagged_points3d(flagged_world_point3d *points, short count);
static void debug_vector(world_vector2d *v);
static void debug_x_line(world_distance x);
#endif

/* ---------- code */

void allocate_render_memory(
	void)
{
	assert(NUMBER_OF_RENDER_FLAGS<=16);
	assert(MAXIMUM_LINES_PER_MAP<=RENDER_FLAGS_BUFFER_SIZE);
	assert(MAXIMUM_SIDES_PER_MAP<=RENDER_FLAGS_BUFFER_SIZE);
	assert(MAXIMUM_ENDPOINTS_PER_MAP<=RENDER_FLAGS_BUFFER_SIZE);
	assert(MAXIMUM_POLYGONS_PER_MAP<=RENDER_FLAGS_BUFFER_SIZE);
	render_flags= (word *) malloc(sizeof(word)*RENDER_FLAGS_BUFFER_SIZE);
	assert(render_flags);
	
	// LP addition: check out pointer-arithmetic hack
	assert(sizeof(void *) == sizeof(POINTER_DATA));
	
	// LP change: no longer needed
	/*
	nodes= (struct node_data *) malloc(sizeof(struct node_data)*MAXIMUM_NODES);
	assert(nodes);
	*/

	polygon_queue= (short *) malloc(sizeof(short)*POLYGON_QUEUE_SIZE);
	assert(polygon_queue);

	// LP change: no longer needed
	/*
	sorted_nodes= (struct sorted_node_data *) malloc(sizeof(struct sorted_node_data)*MAXIMUM_SORTED_NODES);
	assert(sorted_nodes);
	*/

	// LP change: no longer needed
	/*
	render_objects= (struct render_object_data *) malloc(sizeof(struct render_object_data)*MAXIMUM_RENDER_OBJECTS);
	assert(render_objects);
	*/

	// LP change: no longer needed
	/*
	endpoint_clips= (struct endpoint_clip_data *)malloc(MAXIMUM_ENDPOINT_CLIPS*sizeof(struct endpoint_clip_data));
	assert(endpoint_clips);

	line_clips= (struct line_clip_data *)malloc(MAXIMUM_LINE_CLIPS*sizeof(struct line_clip_data));
	*/
	line_clip_indexes= (short *)malloc(MAXIMUM_LINES_PER_MAP*sizeof(short));
	assert(line_clip_indexes);
	// assert(line_clips&&line_clip_indexes);

	// LP change: no longer needed
	/*
	clipping_windows= (struct clipping_window_data *)malloc(MAXIMUM_CLIPPING_WINDOWS*sizeof(struct clipping_window_data));
	assert(clipping_windows);
	*/

	endpoint_x_coordinates= (short *)malloc(MAXIMUM_ENDPOINTS_PER_MAP*sizeof(short));
	polygon_index_to_sorted_node= (struct sorted_node_data **)malloc(MAXIMUM_POLYGONS_PER_MAP*sizeof(struct sorted_node *));
	assert(endpoint_x_coordinates&&polygon_index_to_sorted_node);
	
	return;
}

/* just in case anyone was wondering, standard_screen_width will usually be the same as
	screen_width.  the renderer assumes that the given field_of_view matches the standard
	width provided (so if the actual width provided is larger, you'll be able to see more;
	if it's smaller you'll be able to see less).  this allows the destination bitmap to not
	only grow and shrink while maintaining a constant aspect ratio, but to also change in
	geometry without effecting the image being projected onto it.  if you don't understand
	this, pass standard_width==width */
void initialize_view_data(
	struct view_data *view)
{
	double two_pi= 8.0*atan(1.0);
	double half_cone= view->field_of_view*(two_pi/360.0)/2;
	double adjusted_half_cone= asin(view->screen_width*sin(half_cone)/view->standard_screen_width);
	double world_to_screen;
	
	view->half_screen_width= view->screen_width/2;
	view->half_screen_height= view->screen_height/2;
	
	/* if there’s a round-off error in half_cone, we want to make the cone too big (so when we clip
		lines ‘to the edge of the screen’ they’re actually off the screen, thus +1.0) */
	view->half_cone= (angle) (adjusted_half_cone*((double)NUMBER_OF_ANGLES)/two_pi+1.0);
	
	// LP change: find the adjusted yaw for the landscapes;
	// this is the effective yaw value for the left edge.
	// A landscape rotation can also be added if desired.
	view->landscape_yaw = view->yaw - view->half_cone;

	/* calculate world_to_screen; we could calculate this with standard_screen_width/2 and
		the old half_cone and get the same result */
	world_to_screen= view->half_screen_width/tan(adjusted_half_cone);
	view->world_to_screen_x= view->real_world_to_screen_x= (short) ((world_to_screen/view->horizontal_scale)+0.5);
	view->world_to_screen_y= view->real_world_to_screen_y= (short) ((world_to_screen/view->vertical_scale)+0.5);
	
	/* calculate the vertical cone angle; again, overflow instead of underflow when rounding */
	view->half_vertical_cone= (angle) (NUMBER_OF_ANGLES*atan(((double)view->half_screen_height*view->vertical_scale)/world_to_screen)/two_pi+1.0);

	/* calculate left edge vector */
	view->untransformed_left_edge.i= view->world_to_screen_x;
	view->untransformed_left_edge.j= - view->half_screen_width;

	/* calculate right edge vector (negative, so it clips in the right direction) */
	view->untransformed_right_edge.i= - view->world_to_screen_x;
	view->untransformed_right_edge.j= - view->half_screen_width;

	/* reset any active effects */
	// LP: this is now called in render_screen(), so we need to disable the initializing
	// view->effect= NONE;
	
	return;
}

/* origin,origin_polygon_index,yaw,pitch,roll,etc. have probably changed since last call */
void render_view(
	struct view_data *view,
	struct bitmap_definition *destination)
{
	(void) (destination);
	
	update_view_data(view);

	/* clear the render flags */
	memset(render_flags, 0, sizeof(word)*RENDER_FLAGS_BUFFER_SIZE);

#ifdef AUTOMAP_DEBUG
	memset(automap_lines, 0, (dynamic_world->line_count/8+((dynamic_world->line_count%8)?1:0)*sizeof(byte)));
	memset(automap_polygons, 0, (dynamic_world->polygon_count/8+((dynamic_world->polygon_count%8)?1:0)*sizeof(byte)));
#endif
	
	if(view->terminal_mode_active)
	{
		/* Render the computer interface. */
		render_computer_interface(view);
	}
	else
	{
		/* build the render tree, regardless of map mode, so the automap updates while active */
		build_render_tree(view);
		
		if (view->overhead_map_active)
		{
			/* if the overhead map is active, render it */
			render_overhead_map(view);
		}
		else /* do something complicated and difficult to explain */
		{
			// LP addition:
			// Transmit the view data to the OpenGL renderer, so it can calculate
			// perspective properly.
			OGL_SetView(*view);
			
			/* sort the render tree (so we have a depth-ordering of polygons) and accumulate
				clipping information for each polygon */
			sort_render_tree(view);
	
			/* build the render object list by looking at the sorted render tree */
			build_render_object_list(view);
			
			/* render the object list, back to front, doing clipping on each surface before passing
				it to the texture-mapping code */
			render_tree(view, destination);
	
			/* render the player’s weapons, etc. */		
			render_viewer_sprite_layer(view, destination);
		}
	}

	return;
}

void start_render_effect(
	struct view_data *view,
	short effect)
{
	view->effect= effect;
	view->effect_phase= NONE;
	
	return;
}

/* ---------- private code */

static void update_view_data(
	struct view_data *view)
{
	angle theta;

	// LP change: doing all the FOV changes here:
	View_AdjustFOV(view->field_of_view,view->target_field_of_view);
	
	if (view->effect==NONE)
	{
		view->world_to_screen_x= view->real_world_to_screen_x;
		view->world_to_screen_y= view->real_world_to_screen_y;
	}
	else
	{
		update_render_effect(view);
	}
	
	view->untransformed_left_edge.i= view->world_to_screen_x;
	view->untransformed_right_edge.i= - view->world_to_screen_x;
	
	/* calculate world_to_screen_y*tan(pitch) */
	view->dtanpitch= (view->world_to_screen_y*sine_table[view->pitch])/cosine_table[view->pitch];

	/* calculate left cone vector */
	theta= NORMALIZE_ANGLE(view->yaw-view->half_cone);
	view->left_edge.i= cosine_table[theta], view->left_edge.j= sine_table[theta];
	
	/* calculate right cone vector */
	theta= NORMALIZE_ANGLE(view->yaw+view->half_cone);
	view->right_edge.i= cosine_table[theta], view->right_edge.j= sine_table[theta];
	
	/* calculate top cone vector (negative to clip the right direction) */
	view->top_edge.i= - view->world_to_screen_y;
	view->top_edge.j= - (view->half_screen_height + view->dtanpitch); /* ==k */

	/* calculate bottom cone vector */
	view->bottom_edge.i= view->world_to_screen_y;
	view->bottom_edge.j= - view->half_screen_height + view->dtanpitch; /* ==k */

	/* if we’re sitting on one of the endpoints in our origin polygon, move us back slightly (±1) into
		that polygon.  when we split rays we’re assuming that we’ll never pass through a given
		vertex in different directions (because if we do the tree becomes a graph) but when
		we start on a vertex this can happen.  this is a destructive modification of the origin. */
	{
		short i;
		struct polygon_data *polygon= get_polygon_data(view->origin_polygon_index);
		
		for (i= 0;i<polygon->vertex_count;++i)
		{
			struct world_point2d *vertex= &get_endpoint_data(polygon->endpoint_indexes[i])->vertex;
			
			if (vertex->x==view->origin.x && vertex->y==view->origin.y)
			{
				world_point2d *ccw_vertex= &get_endpoint_data(polygon->endpoint_indexes[WRAP_LOW(i, polygon->vertex_count-1)])->vertex;
				world_point2d *cw_vertex= &get_endpoint_data(polygon->endpoint_indexes[WRAP_HIGH(i, polygon->vertex_count-1)])->vertex;
				world_vector2d inset_vector;
				
				inset_vector.i= (ccw_vertex->x-vertex->x) + (cw_vertex->x-vertex->x);
				inset_vector.j= (ccw_vertex->y-vertex->y) + (cw_vertex->y-vertex->y);
				view->origin.x+= SGN(inset_vector.i);
				view->origin.y+= SGN(inset_vector.j);
				
				break;
			}
		}
		
		/* determine whether we are under or over the media boundary of our polygon; we will see all
			other media boundaries from this orientation (above or below) or fail to draw them. */
		if (polygon->media_index==NONE)
		{
			view->under_media_boundary= FALSE;
		}
		else
		{
			struct media_data *media= get_media_data(polygon->media_index);
			
			// LP change: idiot-proofing
			if (media)
			{
				view->under_media_boundary= UNDER_MEDIA(media, view->origin.z);
				view->under_media_index= polygon->media_index;
			} else {
				view->under_media_boundary= FALSE;
			}
		}
	}

	return;
}

static void update_render_effect(
	struct view_data *view)
{
	short effect= view->effect;
	short phase= view->effect_phase==NONE ? 0 : (view->effect_phase+view->ticks_elapsed);
	short period;

	view->effect_phase= phase;

	switch (effect)
	{
		// LP change: suppressed all the FOV changes
		case _render_effect_fold_in: case _render_effect_fold_out: period= TICKS_PER_SECOND/2; break;
		// case _render_effect_going_fisheye: case _render_effect_leaving_fisheye: period= TICKS_PER_SECOND; break;
		case _render_effect_explosion: period= TICKS_PER_SECOND; break;
		// LP additions:
		// case _render_effect_going_tunnel: case _render_effect_leaving_tunnel: period= TICKS_PER_SECOND; break;
		default:
			// LP change:
			assert(false);
			// halt();
	}
	
	if (phase>period)
	{
		view->effect= NONE;
	}
	else
	{
		switch (effect)
		{
			case _render_effect_explosion:
				shake_view_origin(view, EXPLOSION_EFFECT_RANGE - ((EXPLOSION_EFFECT_RANGE/2)*phase)/period);
				break;
			
			case _render_effect_fold_in:
				phase= period-phase;
			case _render_effect_fold_out:
				/* calculate world_to_screen based on phase */
				view->world_to_screen_x= view->real_world_to_screen_x + (4*view->real_world_to_screen_x*phase)/period;
				view->world_to_screen_y= view->real_world_to_screen_y - (view->real_world_to_screen_y*phase)/(period+period/4);
				break;
			// LP change: suppressed all the FOV changes here
			/*
			case _render_effect_leaving_fisheye:
				phase= period-phase;
				// LP addition: goes to normal instead of tunnel vision
				view->tunnel_vision_active = false;
			case _render_effect_going_fisheye:
			*/
				/* calculate field of view based on phase */
				/*
				// LP change: supporting tunnel vision
				if (view->tunnel_vision_active)
					view->field_of_view= TUNNEL_VISION_FIELD_OF_VIEW + ((EXTRAVISION_FIELD_OF_VIEW-TUNNEL_VISION_FIELD_OF_VIEW)*phase)/period;
				else
					view->field_of_view= NORMAL_FIELD_OF_VIEW + ((EXTRAVISION_FIELD_OF_VIEW-NORMAL_FIELD_OF_VIEW)*phase)/period;
				// view->field_of_view= NORMAL_FIELD_OF_VIEW + ((EXTRAVISION_FIELD_OF_VIEW-NORMAL_FIELD_OF_VIEW)*phase)/period;
				initialize_view_data(view);
				view->effect= effect;
				break;
			
			// LP additions:
			case _render_effect_leaving_tunnel:
				phase= period-phase;
			case _render_effect_going_tunnel:
			*/
				/* calculate field of view based on phase */
				/*
				view->field_of_view= NORMAL_FIELD_OF_VIEW + ((TUNNEL_VISION_FIELD_OF_VIEW-NORMAL_FIELD_OF_VIEW)*phase)/period;
				initialize_view_data(view);
				view->effect= effect;
				break;
			*/
		}
	}

	return;
}

/* ---------- building the render tree */

enum /* cast_render_ray(), next_polygon_along_line() biases */
{
	_no_bias, /* will split at the given endpoint or travel clockwise otherwise */
	_clockwise_bias, /* cross the line clockwise from this endpoint */
	_counterclockwise_bias /* cross the line counterclockwise from this endpoint */
};

static void build_render_tree(
	struct view_data *view)
{
	/* initialize the queue where we remember polygons we need to fire at */
	initialize_polygon_queue(view);

	/* initialize our node list to contain the root, etc. */
	initialize_render_tree(view);
	
	/* reset clipping buffers */
	initialize_clip_data(view);
	
	// LP change:
	// Adjusted for long-vector handling
	long_vector2d view_edge;
	short_to_long_2d(view->left_edge,view_edge);
	cast_render_ray(view, &view_edge, NONE, Nodes.Begin(), _counterclockwise_bias);
	short_to_long_2d(view->right_edge,view_edge);
	cast_render_ray(view, &view_edge, NONE, Nodes.Begin(), _clockwise_bias);
	/*
	cast_render_ray(view, &view->left_edge, NONE, nodes, _counterclockwise_bias);
	cast_render_ray(view, &view->right_edge, NONE, nodes, _clockwise_bias);
	*/
	
	/* pull polygons off the queue, fire at all their new endpoints, building the tree as we go */
	while (polygon_queue_size)
	{
		short vertex_index;
		short polygon_index= polygon_queue[--polygon_queue_size];
		struct polygon_data *polygon= get_polygon_data(polygon_index);
		
		assert(!POLYGON_IS_DETACHED(polygon));
		
		for (vertex_index=0;vertex_index<polygon->vertex_count;++vertex_index)
		{
			short endpoint_index= polygon->endpoint_indexes[vertex_index];
			struct endpoint_data *endpoint= get_endpoint_data(endpoint_index);
			
			if (!TEST_RENDER_FLAG(endpoint_index, _endpoint_has_been_visited))
			{
				// LP change: move toward correct handling of long distances
				long_vector2d vector;
				// world_vector2d vector;
				
				/* transform all visited endpoints */
				endpoint->transformed= endpoint->vertex;
				// LP change
				transform_overflow_point2d(&endpoint->transformed, (world_point2d *) &view->origin, view->yaw, &endpoint->flags);
				// transform_point2d(&endpoint->transformed, (world_point2d *) &view->origin, view->yaw);

				/* calculate an outbound vector to this endpoint */
				// LP: changed to do long distance correctly.	
				vector.i= long(endpoint->vertex.x)-long(view->origin.x);
				vector.j= long(endpoint->vertex.y)-long(view->origin.y);
				
				// LP change: compose a true transformed point to replace endpoint->transformed,
				// and use it in the upcoming code
				long_vector2d transformed_endpoint;
				overflow_short_to_long_2d(endpoint->transformed,endpoint->flags,transformed_endpoint);
				
				if (transformed_endpoint.i>0)
				// if (endpoint->transformed.x>0)
				{
					long x= view->half_screen_width + (transformed_endpoint.j*view->world_to_screen_x)/transformed_endpoint.i;
					// long x= view->half_screen_width + (endpoint->transformed.y*view->world_to_screen_x)/endpoint->transformed.x;
					
					endpoint_x_coordinates[endpoint_index]= PIN(x, SHORT_MIN, SHORT_MAX);
					SET_RENDER_FLAG(endpoint_index, _endpoint_has_been_transformed);
				}
				
				/* do two cross products to determine whether this endpoint is in our view cone or not
					(we don’t have to cast at points outside the cone) */
				if ((view->right_edge.i*vector.j - view->right_edge.j*vector.i)<=0 && (view->left_edge.i*vector.j - view->left_edge.j*vector.i)>=0)
				{
					// LP change:
					cast_render_ray(view, &vector, ENDPOINT_IS_TRANSPARENT(endpoint) ? NONE : endpoint_index, Nodes.Begin(), _no_bias);
					// cast_render_ray(view, &vector, ENDPOINT_IS_TRANSPARENT(endpoint) ? NONE : endpoint_index, nodes, _no_bias);
				}
				
				SET_RENDER_FLAG(endpoint_index, _endpoint_has_been_visited);
			}
		}
	}
	
	return;
}

enum /* cast_render_ray() flags */
{
	_split_render_ray= 0x8000
};

// LP change: make it better able to do long-distance views
static void cast_render_ray(
	struct view_data *view,
	long_vector2d *vector, // world_vector2d *vector,
	short endpoint_index,
	struct node_data *parent, /* nodes==root */
	short bias) /* _clockwise or _counterclockwise for walking endpoints */
{
	short polygon_index= parent->polygon_index;

//	dprintf("shooting at e#%d of p#%d", endpoint_index, polygon_index);
	
	do
	{
		short clipping_endpoint_index= endpoint_index;
		short clipping_line_index;
		word clip_flags= next_polygon_along_line(&polygon_index, (world_point2d *) &view->origin, vector, &clipping_endpoint_index, &clipping_line_index, bias);
		
		if (polygon_index==NONE)
		{
			if (clip_flags&_split_render_ray)
			{
				cast_render_ray(view, vector, endpoint_index, parent, _clockwise_bias);
				cast_render_ray(view, vector, endpoint_index, parent, _counterclockwise_bias);
			}
		}
		else
		{
			struct node_data **node_reference, *node;
			
			/* find the old node referencing this polygon transition or build one */
			for (node_reference= &parent->children;
					*node_reference && (*node_reference)->polygon_index!=polygon_index;
					node_reference= &(*node_reference)->siblings)
				;
			node= *node_reference;
			if (!node)
			{
				// LP change: using growable list
				// Contents get swapped when the length starts to exceed the capacity;
				// prepare extra array to store the contents before swapping.
				// When they are not NULL,
				// "parent", "siblings" and "children" are pointers to members,
				// "reference" is a pointer to a member with an offset.
				// Cast the pointers to whatever size of integer the system uses.
				node_data *SavedNodes;
				POINTER_DATA OldNodePointer;
				int Length = Nodes.GetLength();
				// Will memory get swapped?
				bool DoSwap = Length >= Nodes.GetCapacity();
				if (DoSwap)
				{
					OldNodePointer = POINTER_CAST(Nodes.Begin());
					assert(SavedNodes = new node_data[Length]);
					// Copy the node contents without change
					for (int k=0; k<Length; k++) SavedNodes[k] = Nodes[k];
				}
				assert(Nodes.Add());
				if (DoSwap)
				{
					POINTER_DATA NewNodePointer = POINTER_CAST(Nodes.Begin());
					for (int k=0; k<Length; k++)
					{
						node_data &SavedNode = SavedNodes[k];
						node_data &NewNode = Nodes[k];
						// If NULL, then these pointers were already copied.
						if (SavedNode.parent != NULL)
							NewNode.parent = (node_data *)(NewNodePointer + (POINTER_CAST(SavedNode.parent) - OldNodePointer));
						if (SavedNode.reference != NULL)
							NewNode.reference = (node_data **)(NewNodePointer + (POINTER_CAST(SavedNode.reference) - OldNodePointer));
						if (SavedNode.siblings != NULL)
							NewNode.siblings = (node_data *)(NewNodePointer + (POINTER_CAST(SavedNode.siblings) - OldNodePointer));
						if (SavedNode.children != NULL)
							NewNode.children = (node_data *)(NewNodePointer + (POINTER_CAST(SavedNode.children) - OldNodePointer));
						if (SavedNode.PS_Greater != NULL)
							NewNode.PS_Greater = (node_data *)(NewNodePointer + (POINTER_CAST(SavedNode.PS_Greater) - OldNodePointer));
						if (SavedNode.PS_Less != NULL)
							NewNode.PS_Less = (node_data *)(NewNodePointer + (POINTER_CAST(SavedNode.PS_Less) - OldNodePointer));
						if (SavedNode.PS_Shared != NULL)
							NewNode.PS_Shared = (node_data *)(NewNodePointer + (POINTER_CAST(SavedNode.PS_Shared) - OldNodePointer));
					}
					delete []SavedNodes;
					// Edit parent-node pointer also
					if (parent != NULL)
						parent = (node_data *)(NewNodePointer + (POINTER_CAST(parent) - OldNodePointer));
				}
				node = &Nodes[Length];		// The length here is the "old" length
				/*
				assert(node_count++<MAXIMUM_NODES);
				node= next_node++;
				*/
				
				*node_reference= node;
				INITIALIZE_NODE(node, polygon_index, 0, parent, node_reference);
				
				// Place new node in tree if it has gotten rooted
				if (Length > 0)
				{
				node_data *CurrNode = Nodes.Begin();
				while(true)
				{
					long PolyDiff = long(polygon_index) - long(CurrNode->polygon_index);
					if (PolyDiff > 0)
					{
						node_data *NextNode = CurrNode->PS_Greater;
						if (NextNode)
							// Advance
							CurrNode = NextNode;
						else
						{
							// Attach to end
							CurrNode->PS_Greater = node;
							break;
						}
					}
					else if (PolyDiff < 0)
					{
						node_data *NextNode = CurrNode->PS_Less;
						if (NextNode)
							// Advance
							CurrNode = NextNode;
						else
						{
							// Attach to end
							CurrNode->PS_Less = node;
							break;
						}
					}
					else // Equal
					{
						node_data *NextNode = CurrNode->PS_Shared;
						if (NextNode)
							// Splice node into shared-polygon chain
							node->PS_Shared = NextNode;
						CurrNode->PS_Shared = node;
						break;
					}
				}
				}
			}

			/* update the line clipping information, if necessary, for this node (don’t add
				duplicates */
			if (clipping_line_index!=NONE)
			{
				short i;
				
				if (!TEST_RENDER_FLAG(clipping_line_index, _line_has_clip_data))
					calculate_line_clipping_information(view, clipping_line_index, clip_flags);
				clipping_line_index= line_clip_indexes[clipping_line_index];
				
				for (i=0;
						i<node->clipping_line_count&&node->clipping_lines[i]!=clipping_line_index;
						++i)
					;
				if (i==node->clipping_line_count)
				{
					assert(node->clipping_line_count<MAXIMUM_CLIPPING_LINES_PER_NODE);
					node->clipping_lines[node->clipping_line_count++]= clipping_line_index;
				}
			}
			
			/* update endpoint clipping information for this node if we have a valid endpoint with clip */
			if (clipping_endpoint_index!=NONE && (clip_flags&(_clip_left|_clip_right)))
			{
				clipping_endpoint_index= calculate_endpoint_clipping_information(view, clipping_endpoint_index, clip_flags);
				
				if (node->clipping_endpoint_count<MAXIMUM_CLIPPING_ENDPOINTS_PER_NODE)
					node->clipping_endpoints[node->clipping_endpoint_count++]= clipping_endpoint_index;
			}
			
			parent= node;
		}
	}
	while (polygon_index!=NONE);
	
	return;
}

static void initialize_polygon_queue(
	struct view_data *view)
{
	(void) (view);
	
	polygon_queue_size= 0;
	
	return;
}

enum /* next_polygon_along_line() states */
{
	_looking_for_first_nonzero_vertex,
	_looking_clockwise_for_right_vertex,
	_looking_counterclockwise_for_left_vertex,
	_looking_for_next_nonzero_vertex
};

// LP change: make it better able to do long-distance views
static word next_polygon_along_line(
	short *polygon_index,
	world_point2d *origin, /* not necessairly in polygon_index */
	long_vector2d *vector, // world_vector2d *vector,
	short *clipping_endpoint_index, /* if non-NONE on entry this is the solid endpoint we’re shooting for */
	short *clipping_line_index, /* NONE on exit if this polygon transition wasn’t accross an elevation line */
	short bias)
{
	struct polygon_data *polygon= get_polygon_data(*polygon_index);
	short next_polygon_index, crossed_line_index, crossed_side_index;
	boolean passed_through_solid_vertex= FALSE;
	short vertex_index, vertex_delta;
	word clip_flags= 0;
	short state;

	ADD_POLYGON_TO_AUTOMAP(*polygon_index);
	PUSH_POLYGON_INDEX(*polygon_index);

	state= _looking_for_first_nonzero_vertex;
	vertex_index= 0, vertex_delta= 1; /* start searching clockwise from vertex zero */
	// LP change: added test for looping around:
	// will remember the first vertex examined when the state has changed
	short initial_vertex_index = vertex_index;
	bool changed_state = true;
	do
	{
		// Jump out of loop?
		if (changed_state)
			changed_state = false;
		else if (vertex_index == initial_vertex_index)
		{
			// Attempt to idiot-proof it by returning nothing
			next_polygon_index = NONE;
			crossed_line_index = NONE;
			crossed_side_index = NONE;
			break;
		}
			
		short endpoint_index= polygon->endpoint_indexes[vertex_index];
		world_point2d *vertex= &get_endpoint_data(endpoint_index)->vertex;
		// LP change to make it more long-distance-friendly
		CROSSPROD_TYPE cross_product= CROSSPROD_TYPE(long(vertex->x)-long(origin->x))*vector->j - CROSSPROD_TYPE(long(vertex->y)-long(origin->y))*vector->i;
		// long cross_product= (vertex->x-origin->x)*vector->j - (vertex->y-origin->y)*vector->i;
		
//		dprintf("p#%d, e#%d:#%d, SGN(cp)=#%d, state=#%d", *polygon_index, vertex_index, polygon->endpoint_indexes[vertex_index], SGN(cross_product), state);
		
		switch (SGN(cross_product))
		{
			case 1: /* endpoint is on the left side of our vector */
				switch (state)
				{
					case _looking_for_first_nonzero_vertex:
						/* search clockwise for transition (left to right) */
						state= _looking_clockwise_for_right_vertex;
						// LP change: resetting loop test
						initial_vertex_index = vertex_index;
						changed_state = true;
						//
						break;
					
					case _looking_counterclockwise_for_left_vertex: /* found the transition we were looking for */
						next_polygon_index= polygon->adjacent_polygon_indexes[vertex_index];
						crossed_line_index= polygon->line_indexes[vertex_index];
						crossed_side_index= polygon->side_indexes[vertex_index];
					case _looking_for_next_nonzero_vertex: /* next_polygon_index already set */
						state= NONE;
						break;
				}
				break;
			
			case 0: /* endpoint lies directly on our vector */
				if (state!=_looking_for_first_nonzero_vertex)
				{
					if (endpoint_index==*clipping_endpoint_index) passed_through_solid_vertex= TRUE;
					
					/* if we think we know what’s on the other side of this zero (these zeros)
						change the state: if we don’t find what we’re looking for then the polygon
						is entirely on one side of the line or the other (except for this vertex),
						in any case we need to call decide_where_vertex_leads() to find out what’s
						on the other side of this vertex */
					switch (state)
					{
						case _looking_clockwise_for_right_vertex:
						case _looking_counterclockwise_for_left_vertex:
							next_polygon_index= *polygon_index;
							clip_flags|= decide_where_vertex_leads(&next_polygon_index, &crossed_line_index, &crossed_side_index,
								vertex_index, origin, vector, clip_flags, bias);
							state= _looking_for_next_nonzero_vertex;
							// LP change: resetting loop test
							initial_vertex_index = vertex_index;
							changed_state = true;
							//
							break;
					}
				}
				break;
			
			case -1: /* endpoint is on the right side of our vector */
				switch (state)
				{
					case _looking_for_first_nonzero_vertex:
						/* search counterclockwise for transition (right to left) */
						state= _looking_counterclockwise_for_left_vertex;
						vertex_delta= -1;
						// LP change: resetting loop test
						initial_vertex_index = vertex_index;
						changed_state = true;
						//
						break;
					
					case _looking_clockwise_for_right_vertex: /* found the transition we were looking for */
						{
							short i= WRAP_LOW(vertex_index, polygon->vertex_count-1);
							next_polygon_index= polygon->adjacent_polygon_indexes[i];
							crossed_line_index= polygon->line_indexes[i];
							crossed_side_index= polygon->side_indexes[i];
						}
					case _looking_for_next_nonzero_vertex: /* next_polygon_index already set */
						state= NONE;
						break;
				}
				break;
		}
		
		/* adjust vertex_index (clockwise or counterclockwise, depending on vertex_delta) */
		vertex_index= (vertex_delta<0) ? WRAP_LOW(vertex_index, polygon->vertex_count-1) :
			WRAP_HIGH(vertex_index, polygon->vertex_count-1);
	}
	while (state!=NONE);

//	dprintf("exiting, cli=#%d, npi=#%d", crossed_line_index, next_polygon_index);

	/* if we didn’t pass through the solid vertex we were aiming for, set clipping_endpoint_index to NONE,
		we assume the line we passed through doesn’t clip, and set clipping_line_index to NONE
		(this will be corrected in a few moments if we chose poorly) */
	if (!passed_through_solid_vertex) *clipping_endpoint_index= NONE;
	*clipping_line_index= NONE;
	
	if (crossed_line_index!=NONE)
	{
		struct line_data *line= get_line_data(crossed_line_index);

		/* add the line we crossed to the automap */
		ADD_LINE_TO_AUTOMAP(crossed_line_index);

		/* if the line has a side facing this polygon, mark the side as visible */
		if (crossed_side_index!=NONE) SET_RENDER_FLAG(crossed_side_index, _side_is_visible);

		/* if this line is transparent we need to check for a change in elevation for clipping,
			if it’s not transparent then we can’t pass through it */
		// LP change: added test for there being a polygon on the other side
		if (LINE_IS_TRANSPARENT(line) && next_polygon_index != NONE)
		// if (LINE_IS_TRANSPARENT(line))
		{
			struct polygon_data *next_polygon= get_polygon_data(next_polygon_index);
			
			if (line->highest_adjacent_floor>next_polygon->floor_height ||
				line->highest_adjacent_floor>polygon->floor_height) clip_flags|= _clip_down; /* next polygon floor is lower */
			if (line->lowest_adjacent_ceiling<next_polygon->ceiling_height ||
				line->lowest_adjacent_ceiling<polygon->ceiling_height) clip_flags|= _clip_up; /* next polygon ceiling is higher */
			if (clip_flags&(_clip_up|_clip_down)) *clipping_line_index= crossed_line_index;
		}
		else
		{
			next_polygon_index= NONE;
		}
	}

	/* tell the caller what polygon we ended up in */
	*polygon_index= next_polygon_index;
	
	return clip_flags;
}

// LP change: make it better able to do long-distance views
static word decide_where_vertex_leads(
	short *polygon_index,
	short *line_index,
	short *side_index,
	short endpoint_index_in_polygon_list,
	world_point2d *origin,
	long_vector2d *vector, // world_vector2d *vector,
	word clip_flags,
	short bias)
{
	struct polygon_data *polygon= get_polygon_data(*polygon_index);
	short endpoint_index= polygon->endpoint_indexes[endpoint_index_in_polygon_list];
	short index;
	
	switch (bias)
	{
		case _no_bias:
//			dprintf("splitting at endpoint #%d", endpoint_index);
			clip_flags|= _split_render_ray;
			*polygon_index= *line_index= *side_index= NONE;
			index= NONE;
			break;
		
		case _clockwise_bias:
			index= endpoint_index_in_polygon_list;
			break;
		
		case _counterclockwise_bias:
			index= WRAP_LOW(endpoint_index_in_polygon_list, polygon->vertex_count-1);
			break;
		
		default:
			// LP change:
			assert(false);
			// halt();
	}
	
	if (index!=NONE)
	{
		struct line_data *line;
		struct world_point2d *vertex;
		// LP change:
		CROSSPROD_TYPE cross_product;
		// long cross_product;

		*line_index= polygon->line_indexes[index];
		*side_index= polygon->side_indexes[index];
		*polygon_index= polygon->adjacent_polygon_indexes[index];
		
		line= get_line_data(*line_index);
		if (*polygon_index!=NONE && LINE_IS_TRANSPARENT(line))
		{
			polygon= get_polygon_data(*polygon_index);
			
			/* locate our endpoint in this polygon */
			for (index=0;
					index<polygon->vertex_count && polygon->endpoint_indexes[index]!=endpoint_index;
					++index)
				;
			vassert(index!=polygon->vertex_count, csprintf(temporary, "endpoint #%d not in polygon #%d (from #%d)", endpoint_index, polygon_index, polygon_index));
	
			switch (bias)
			{
				case _clockwise_bias: index= WRAP_HIGH(index, polygon->vertex_count-1); break;
				case _counterclockwise_bias: index= WRAP_LOW(index, polygon->vertex_count-1); break;
				default:
					// LP change:
					assert(false);
					// halt();
			}
			
			vertex= &get_endpoint_data(polygon->endpoint_indexes[index])->vertex;
			// LP change: made more long-distance-friendly
			cross_product= CROSSPROD_TYPE(long(vertex->x)-long(origin->x))*vector->j - CROSSPROD_TYPE(long(vertex->y)-long(origin->y))*vector->i;
			// cross_product= (vertex->x-origin->x)*vector->j - (vertex->y-origin->y)*vector->i;
			
			if ((bias==_clockwise_bias&&cross_product>=0) || (bias==_counterclockwise_bias&&cross_product<=0))
			{
				/* we’re leaving this endpoint, set clip flag in case it’s solid */
				clip_flags|= (bias==_clockwise_bias) ? _clip_left : _clip_right;
			}
		}

//		dprintf("left endpoint #%d via line #%d to polygon #%d (bias==#%d)", endpoint_index, *line_index, *polygon_index, bias);
	}

	return clip_flags;
}

static void initialize_render_tree(
	struct view_data *view)
{
	// LP change: using growable list
	Nodes.ResetLength();
	Nodes.Add();
	INITIALIZE_NODE(Nodes.Begin(), view->origin_polygon_index, 0, (struct node_data *) NULL, (struct node_data **) NULL);
	/*
	node_count= 1;
	next_node= nodes+1;
	INITIALIZE_NODE(nodes, view->origin_polygon_index, 0, (struct node_data *) NULL, (struct node_data **) NULL);
	*/
	
	return;
}

/* ---------- sorting (decomposing) the render tree */

static void initialize_sorted_render_tree(
	void)
{
	// LP change: sorted nodes a growable list
	SortedNodes.ResetLength();
	/*
	sorted_node_count= 0;
	next_sorted_node= sorted_nodes;
	*/
	
	return;
}

/*
tree decomposition:

pick a leaf polygon
	make sure the polygon is everywhere a leaf (don’t walk the tree, search it linearly) 
		if it’s not, pick the node which obstructed it to test next
		if it is, pull it off the tree (destructively) and accumulate it’s clipping information
			pick the one of the node’s siblings (or it’s parent if it has none) to handle next
*/

// LP change: this is the source of transparent-line errors;
// this has been increased to 32 (the value in the Win32 version)
// Feb 1, 2000: Suppressed because of use of growable-list
// #define MAXIMUM_NODE_ALIASES 20
// #define MAXIMUM_NODE_ALIASES 32

static int DiffIndx(node_data *Node) {if (Node) return (Node - Nodes.Begin()); else return -1;}
static void DebugTree(char *Label, int q, node_data &Node) {dprintf("%s %4d %4d %4d %4d %4d",Label,q,Node.polygon_index,DiffIndx(Node.PS_Less),DiffIndx(Node.PS_Greater),DiffIndx(Node.PS_Shared));}

static void sort_render_tree(
	struct view_data *view)
{
	struct node_data *leaf, *last_leaf;

	initialize_sorted_render_tree();
	
	leaf= (struct node_data *) NULL;
	do
	{
		// LP change: no more growable list of aliases,
		// due to the sorted-polygon-tree structure of the nodes.
		/*
		short alias_count= 0;
		struct node_data *aliases[MAXIMUM_NODE_ALIASES];
		*/
		boolean leaf_has_children= FALSE; /* i.e., it’s not a leaf */
		struct node_data *node;

		/* if we don’t have a leaf, find one */
		if (!leaf)
			// LP change:
			for (leaf= Nodes.Begin(); leaf->children; leaf= leaf->children)
			// for (leaf= nodes; leaf->children; leaf= leaf->children)
				;
		last_leaf= leaf;
		
		/* does the current leaf have any children anywhere in the tree? */
		// LP change: Replaced all this code with binary-search code for polygon value,
		// followed by building of node-alias list
		short PolygonToFind = leaf->polygon_index;
		
		// Look for the first node with that polygon index;
		// start search off with the hypothesis of failure
		node_data *FoundNode = NULL;
		node_data *CurrNode = Nodes.Begin();
		while(true)
		{
			long PolyDiff = long(PolygonToFind) - long(CurrNode->polygon_index);
			if (PolyDiff > 0)
			{
				node_data *NextNode = CurrNode->PS_Greater;
				if (NextNode)
					// Advance
					CurrNode = NextNode;
				else
					// Failed
					break;
			}
			else if (PolyDiff < 0)
			{
				node_data *NextNode = CurrNode->PS_Less;
				if (NextNode)
					// Advance
					CurrNode = NextNode;
				else
					// Failed
					break;
			}
			else // Equal: the search was a success
			{
				FoundNode = CurrNode;
				break;
			}
		}
		
		// Now load up the node aliases and check for children
		if (FoundNode)
		{
			// Search along node chain
			for (node = FoundNode; node; node = node->PS_Shared)
			{
				assert(node->polygon_index == PolygonToFind);
				if (node->children)
				{
					leaf_has_children= TRUE;
					break;
				}
			}
		}
		
		/*
		// LP change: don't need this linear-search code anymore.
		for (node= Nodes.Begin(); node<Nodes.End(); ++node)
		// for (node= nodes; node<next_node; ++node)
		{
			if (node->polygon_index==leaf->polygon_index)
			{
				// LP change: using node-alias growable list				
				assert(NodeAliases.Add(node));
				
				if (node->children)
				{
					leaf_has_children= TRUE;
					break;
				}
				if (alias_count<MAXIMUM_NODE_ALIASES)
				{
					aliases[alias_count++]= node;
					
					if (node->children)
					{
						leaf_has_children= TRUE;
						break;
					}
				}
#ifdef DEBUG
				else
				{
					dprintf("exceeded MAXIMUM_NODE_ALIASES; this sucks, Beavis.");
					return;
				}
// LP: Added alternative way of handling this
#else
				else
				{
					assert(alias_count<MAXIMUM_NODE_ALIASES);
					return;
				}
#endif
			}
		}
		*/
		
		if (leaf_has_children) /* something was in our way; see if we can take it out instead */
		{
			leaf= node->children;
//			dprintf("polygon #%d is in the way of polygon #%d", node->polygon_index, leaf->polygon_index);
		}
		else /* this is a leaf, and we can remove it from the tree */
		{
			struct sorted_node_data *sorted_node;
			short alias;
			
//			dprintf("removed polygon #%d (#%d aliases)", leaf->polygon_index, alias_count);
			
			// LP change:
			int Length = SortedNodes.GetLength();
			// Will memory get swapped?
			bool DoSwap = Length >= SortedNodes.GetCapacity();
			assert(SortedNodes.Add());
			// Update dependent quantities; Length is original length
			if (DoSwap) {
				for (int k=0; k<Length; k++) {
					sorted_node = &SortedNodes[k];
					polygon_index_to_sorted_node[sorted_node->polygon_index]= sorted_node;
				}
			}
			sorted_node = &SortedNodes[Length];
			/*
			assert(sorted_node_count++<MAXIMUM_SORTED_NODES);
			sorted_node= next_sorted_node++;
			*/
			
			sorted_node->polygon_index= leaf->polygon_index;
			sorted_node->interior_objects= (struct render_object_data *) NULL;
			sorted_node->exterior_objects= (struct render_object_data *) NULL;
			// LP change: using polygon-sorted node chain
			sorted_node->clipping_windows= build_clipping_windows(view, FoundNode);
			/*
			sorted_node->clipping_windows= build_clipping_windows(view, aliases, alias_count);
			*/
			
			/* remember which sorted nodes correspond to which polygons (only valid if
				_polygon_is_visible) */
			polygon_index_to_sorted_node[sorted_node->polygon_index]= sorted_node;
			
			/* walk this node’s alias list, removing each from the tree */
			// LP change: move down the chain of polygon-sharing nodes
			for (node_data *Alias = FoundNode; Alias; Alias = Alias->PS_Shared)
			/*
			for (alias= 0;alias<alias_count;++alias)
			*/
			{
				// LP change: remember what the node was for when we break out
				node = Alias;
				// node= aliases[alias];

				/* remove this node and update the next node’s reference (if there is a
					reference and if there is a next node) */
				if (node->reference)
				{
					*(node->reference)= node->siblings;
					if (node->siblings) (node->siblings)->reference= node->reference;
				}
			}

			/* try to handle this node’s siblings next (if there aren’t any, then a ‘random’
				node will be chosen) */
			leaf= node->siblings;
		}
	}
	// LP change:
	while (last_leaf!=Nodes.Begin()); /* continue until we remove the root */
	// while (last_leaf!=nodes); /* continue until we remove the root */
	
	return;
}

#define MAXIMUM_CLIPS_PER_NODE 64

enum /* build_clipping_window() window states */
{
	_looking_for_left_clip, /* ignore right clips (we just passed one) */
	_looking_for_right_clip,
	_building_clip_window /* found valid left and right clip, build a window */
};

/* be sure to examine all of a node’s parents for clipping information (gak!) */
static struct clipping_window_data *build_clipping_windows(
	struct view_data *view,
	// LP change: using node chain instead
	struct node_data *ChainBegin)
	// struct node_data **node_list,
	// short node_count)
{
	// LP change: growable lists
	AccumulatedLineClips.ResetLength();
	AccumulatedEndpointClips.ResetLength();
	// short accumulated_line_clip_count= 0, accumulated_endpoint_clip_count= 0;
	// struct line_clip_data *accumulated_line_clips[MAXIMUM_CLIPS_PER_NODE];
	// struct endpoint_clip_data *accumulated_endpoint_clips[MAXIMUM_CLIPS_PER_NODE];
	struct clipping_window_data *first_clipping_window= (struct clipping_window_data *) NULL;
	struct clipping_window_data *last_clipping_window;
	struct endpoint_clip_data *endpoint;
	struct line_clip_data *line;
	short x0, x1; /* ignoring what clipping parameters we’ve gotten, this is the left and right borders of this node on the screen */
	short i, j, k;
	
	/* calculate x0,x1 (real left and right borders of this node) in case the left and right borders
		of the window are sloppy */
	{
		// LP change: look at beginning of chain
		struct polygon_data *polygon= get_polygon_data(ChainBegin->polygon_index); /* all these nodes should be the same */
		// struct polygon_data *polygon= get_polygon_data((*node_list)->polygon_index); /* all these nodes should be the same */
		
		x0= SHORT_MAX, x1= SHORT_MIN;
		for (i= 0;i<polygon->vertex_count;++i)
		{
			short endpoint_index= polygon->endpoint_indexes[i];
			
			if (TEST_RENDER_FLAG(endpoint_index, _endpoint_has_been_transformed))
			{
				short x= endpoint_x_coordinates[endpoint_index];
				
				if (x<x0) x0= x;
				if (x>x1) x1= x;
			}
			else
			{
				x0= SHORT_MIN, x1= SHORT_MAX;
				break;
			}
		}
	}
	
	/* add left, top and bottom of screen */
	// LP change:
	endpoint_clip_data *EndpointClipPtr = &EndpointClips[indexLEFT_SIDE_OF_SCREEN];
	assert(AccumulatedEndpointClips.Add(EndpointClipPtr));
	line_clip_data *LineClipPtr = &LineClips[indexTOP_AND_BOTTOM_OF_SCREEN];
	assert(AccumulatedLineClips.Add(LineClipPtr));
	/*
	accumulated_endpoint_clips[accumulated_endpoint_clip_count++]= endpoint_clips + indexLEFT_SIDE_OF_SCREEN;
	accumulated_line_clips[accumulated_line_clip_count++]= line_clips + indexTOP_AND_BOTTOM_OF_SCREEN;
	*/

	/* accumulate clipping information, left to right, into local arrays */
	// Move along chain
	for (node_data *ChainNode = ChainBegin; ChainNode; ChainNode = ChainNode->PS_Shared)
	// for (k= 0;k<node_count;++k)
	{
		struct node_data *node;
		
		// LP change: use chain node as starting point
		for (node= ChainNode;node;node= node->parent) /* examine this node and all parents! */
		// for (node= node_list[k];node;node= node->parent) /* examine this node and all parents! */
		{
			/* sort in endpoint clips (left to right) */
			for (i= 0;i<node->clipping_endpoint_count;++i)
			{
				// LP change:
				endpoint= &EndpointClips[node->clipping_endpoints[i]];
				// endpoint= endpoint_clips + node->clipping_endpoints[i];
				
				// LP change:
				for (j= 0;j<AccumulatedEndpointClips.GetLength();++j)
				// for (j= 0;j<accumulated_endpoint_clip_count;++j)
				{
					// LP change:
					if (AccumulatedEndpointClips[j]==endpoint) { j= NONE; break; } /* found duplicate */
					if ((AccumulatedEndpointClips[j]->x==endpoint->x&&endpoint->flags==_clip_left) ||
						AccumulatedEndpointClips[j]->x>endpoint->x)
						/*
					if (accumulated_endpoint_clips[j]==endpoint) { j= NONE; break; } *//* found duplicate *//*
					if ((accumulated_endpoint_clips[j]->x==endpoint->x&&endpoint->flags==_clip_left) ||
						accumulated_endpoint_clips[j]->x>endpoint->x)
					*/
					{
						break; /* found sorting position if x is greater or x is equal and this is a left clip */
					}
				}
				
				if (j!=NONE) /* if the endpoint was not a duplicate */
				{
					/* expand the array, if necessary, and add the new endpoint */
					// LP change:
					int Length = AccumulatedEndpointClips.GetLength();
					assert(AccumulatedEndpointClips.Add());
					assert(AccumulatedEndpointClips.GetLength() <= 32767);		// Originally a short value
					if (j!=Length) memmove(&AccumulatedEndpointClips[j+1], &AccumulatedEndpointClips[j],
						(Length-j)*sizeof(struct endpoint_clip_data *));
					AccumulatedEndpointClips[j]= endpoint;
					/*
					assert(accumulated_endpoint_clip_count<MAXIMUM_CLIPS_PER_NODE);
					if (j!=accumulated_endpoint_clip_count) memmove(accumulated_endpoint_clips+j+1, accumulated_endpoint_clips+j,
						(accumulated_endpoint_clip_count-j)*sizeof(struct endpoint_clip_data *));
					accumulated_endpoint_clips[j]= endpoint;
					accumulated_endpoint_clip_count+= 1;
					*/
				}
			}

			/* sort in line clips, avoiding redundancies;  calculate_vertical_line_clip_data(),
				the function which deals with these, does not depend on them being sorted */
			for (i= 0;i<node->clipping_line_count;++i)
			{
				// LP change:
				line= &LineClips[node->clipping_lines[i]];
				// line= line_clips + node->clipping_lines[i];
				
				// LP change:
				for (j= 0;j<AccumulatedLineClips.GetLength();++j) if (AccumulatedLineClips[j]==line) break; /* found duplicate */
				if (j==AccumulatedLineClips.GetLength()) /* if the line was not a duplicate */
				{
					assert(AccumulatedLineClips.Add(line));
					assert(AccumulatedLineClips.GetLength() <= 32767);		// Originally a short value
				}
				/*
				for (j= 0;j<accumulated_line_clip_count;++j) if (accumulated_line_clips[j]==line) break; *//* found duplicate *//*
				if (j==accumulated_line_clip_count) *//* if the line was not a duplicate *//*
				{
					assert(accumulated_line_clip_count<MAXIMUM_CLIPS_PER_NODE);
					accumulated_line_clips[accumulated_line_clip_count++]= line;
				}
				*/
			}
		}
	}
	
//	dprintf("#%d accumulated points @ %p", accumulated_endpoint_clip_count, accumulated_endpoint_clips);
//	dprintf("#%d accumulated lines @ %p", accumulated_line_clip_count, accumulated_line_clips);

	/* add right side of screen */
	// LP change:
	EndpointClipPtr = &EndpointClips[indexRIGHT_SIDE_OF_SCREEN];
	assert(AccumulatedEndpointClips.Add(EndpointClipPtr));
	// assert(accumulated_endpoint_clip_count<MAXIMUM_CLIPS_PER_NODE);
	// accumulated_endpoint_clips[accumulated_endpoint_clip_count++]= endpoint_clips + indexRIGHT_SIDE_OF_SCREEN;

	/* build the clipping windows */
	{
		short state= _looking_for_left_clip;
		struct endpoint_clip_data *left_clip, *right_clip;

		// LP change:
		for (i= 0;i<AccumulatedEndpointClips.GetLength();++i)
		// for (i= 0;i<accumulated_endpoint_clip_count;++i)
		{
			// LP change:
			endpoint= AccumulatedEndpointClips[i];
			// endpoint= accumulated_endpoint_clips[i];
	
			switch (endpoint->flags)
			{
				case _clip_left:
					switch (state)
					{
						case _looking_for_left_clip:
							left_clip= endpoint;
							state= _looking_for_right_clip;
							break;
						case _looking_for_right_clip:
							left_clip= endpoint; /* found more strict clipping point, use it instead */
							break;
					}
					break;
				
				case _clip_right:
					switch (state)
					{
						case _looking_for_right_clip:
							right_clip= endpoint;
							state= _building_clip_window;
							break;
						
						/* ignore _left_clips */
					}
					break;
				
				default:
					// LP change:
					dprintf("render.c: build_clipping_windows(): bad state: %d",state);
					assert(false);
					// halt();
			}

			if (state==_building_clip_window)
			{
				if (left_clip->x<view->screen_width && right_clip->x>0 && left_clip->x<right_clip->x)
				{
					// LP change: clipping windows are in growable list
					int Length = ClippingWindows.GetLength();
					POINTER_DATA OldCWPointer;
					// Will memory get swapped?
					bool DoSwap = Length >= ClippingWindows.GetCapacity();
					if (DoSwap) OldCWPointer = POINTER_CAST(ClippingWindows.Begin());
					assert(ClippingWindows.Add());
					if (DoSwap)
					{
						// Get the clipping windows and sorted nodes into sync; no render objects yet
						POINTER_DATA NewCWPointer = POINTER_CAST(ClippingWindows.Begin());
						for (int k=0; k<ClippingWindows.GetLength(); k++)
						{
							clipping_window_data &ClippingWindow = ClippingWindows[k];
							if (ClippingWindow.next_window != NULL)
								ClippingWindow.next_window = (clipping_window_data *)(NewCWPointer + (POINTER_CAST(ClippingWindow.next_window) - OldCWPointer));
						}
						for (int k=0; k<SortedNodes.GetLength(); k++)
						{
							sorted_node_data &SortedNode = SortedNodes[k];
							if (SortedNode.clipping_windows != NULL)
								SortedNode.clipping_windows = (clipping_window_data *)(NewCWPointer + (POINTER_CAST(SortedNode.clipping_windows) - OldCWPointer));
						}
					}
					struct clipping_window_data *window= &ClippingWindows[Length];
					// struct clipping_window_data *window= next_clipping_window++;
					
					/* handle maintaining the linked list of clipping windows */
					// assert(next_clipping_window_index++<MAXIMUM_CLIPPING_WINDOWS);
					if (!first_clipping_window)
					{
						first_clipping_window= last_clipping_window= window;
					}
					else
					{
						last_clipping_window->next_window= window;
						last_clipping_window= window;
					}
					
					window->x0= left_clip->x, window->x1= right_clip->x;
					window->left= left_clip->vector;
					window->right= right_clip->vector;
					calculate_vertical_clip_data(AccumulatedLineClips.Begin(), AccumulatedLineClips.GetLength(), window,
						MAX(x0, window->x0), MIN(x1, window->x1));
					/*
					calculate_vertical_clip_data(accumulated_line_clips, accumulated_line_clip_count, window,
						MAX(x0, window->x0), MIN(x1, window->x1));
					*/
					window->next_window= (struct clipping_window_data *) NULL;
				}
				
				state= _looking_for_left_clip;
			}
		}
	}

	return first_clipping_window;
}

/* does not care if the given line_clips are sorted or not */
static void calculate_vertical_clip_data(
	struct line_clip_data **accumulated_line_clips,
	short accumulated_line_clip_count,
	struct clipping_window_data *window,
	short x0,
	short x1)
{
	if (x0<x1)
	{
		short i, x;
		struct line_clip_data *highest_line, *locally_highest_line, *line;
	
		/* get the highest top clip covering the requested horizontal run */		
		x= x0;
		highest_line= (struct line_clip_data *) NULL;
		do
		{
			locally_highest_line= (struct line_clip_data *) NULL;
			
			for (i= 0;i<accumulated_line_clip_count;++i)
			{
				line= accumulated_line_clips[i];
				
				if ((line->flags&_clip_up) && x>=line->x0 && x<line->x1 &&
					(!locally_highest_line || locally_highest_line->top_y<line->top_y))
				{
					locally_highest_line= line;
				}
			}
			vassert(locally_highest_line, csprintf(temporary, "didn’t find diddly at #%d [#%d,#%d]", x, x0, x1));
				
			if (!highest_line || locally_highest_line->top_y<highest_line->top_y)
			{
				highest_line= locally_highest_line;
//				dprintf("%p [%d,%d] is new highest top clip line for window [%d,%d]", highest_line, highest_line->x0, highest_line->x1, x0, x1);
			}
			
			x= locally_highest_line->x1;
		}
		while (x<x1);
		
		assert(highest_line);
//		dprintf("%p [%d,%d] is highest top clip line for window [%d,%d]", highest_line, highest_line->x0, highest_line->x1, x0, x1);
		window->top= highest_line->top_vector;
		window->y0= highest_line->top_y;
	
		/* get the lowest bottom clip covering the requested horizontal run */	
		x= x0;
		highest_line= (struct line_clip_data *) NULL;
		do
		{
			locally_highest_line= (struct line_clip_data *) NULL; /* means lowest */
			
			for (i= 0;i<accumulated_line_clip_count;++i)
			{
				line= accumulated_line_clips[i];
				
				if ((line->flags&_clip_down) && x>=line->x0 && x<line->x1 &&
					(!locally_highest_line || locally_highest_line->bottom_y>line->bottom_y))
				{
					locally_highest_line= line;
				}
			}
			vassert(locally_highest_line, csprintf(temporary, "didn’t find diddly at #%d [#%d,#%d]", x, x0, x1));
				
			if (!highest_line || locally_highest_line->bottom_y>highest_line->bottom_y)
			{
				highest_line= locally_highest_line; 
//				dprintf("%p [%d,%d] is new lowest bottom clip line for window [%d,%d]", highest_line, highest_line->x0, highest_line->x1, x0, x1);
			}
			
			x= locally_highest_line->x1;
		}
		while (x<x1);
		
		assert(highest_line);
//		dprintf("%p [%d,%d] is lowest bottom clip line for window [%d,%d]", highest_line, highest_line->x0, highest_line->x1, x0, x1);
		window->bottom= highest_line->bottom_vector;
		window->y1= highest_line->bottom_y;
}
	
	return;
}

/* ---------- initializing and calculating clip data */

static void initialize_clip_data(
	struct view_data *view)
{
	// LP change:
	ResetEndpointClips();
	/*
	next_endpoint_clip_index= NUMBER_OF_INITIAL_ENDPOINT_CLIPS;
	next_endpoint_clip= endpoint_clips + NUMBER_OF_INITIAL_ENDPOINT_CLIPS;
	*/
	
	/* set two default endpoint clips (left and right sides of screen) */
	{
		struct endpoint_clip_data *endpoint;
		
		// LP change:
		endpoint= &EndpointClips[indexLEFT_SIDE_OF_SCREEN];
		// endpoint= endpoint_clips + indexLEFT_SIDE_OF_SCREEN;
		endpoint->flags= _clip_left;
		// LP change:
		short_to_long_2d(view->untransformed_left_edge,endpoint->vector);
		// endpoint->vector= view->untransformed_left_edge;
		endpoint->x= 0;

		// LP change:
		endpoint= &EndpointClips[indexRIGHT_SIDE_OF_SCREEN];
		// endpoint= endpoint_clips + indexLEFT_SIDE_OF_SCREEN;
		endpoint->flags= _clip_right;
		// LP change:
		short_to_long_2d(view->untransformed_right_edge,endpoint->vector);
		// endpoint->vector= view->untransformed_right_edge;
		endpoint->x= view->screen_width;
	}
	
	// LP change:
	ResetLineClips();
	/*
	next_line_clip_index= NUMBER_OF_INITIAL_LINE_CLIPS;
	next_line_clip= line_clips + NUMBER_OF_INITIAL_LINE_CLIPS;
	*/
	
	/* set default line clip (top and bottom of screen) */
	{
		// LP change:
		struct line_clip_data *line= &LineClips[indexTOP_AND_BOTTOM_OF_SCREEN];
		// struct line_clip_data *line= line_clips + indexTOP_AND_BOTTOM_OF_SCREEN;

		line->flags= _clip_up|_clip_down;
		line->x0= 0;
		line->x1= view->screen_width;
		// LP change:
		line->top_y= 0; short_to_long_2d(view->top_edge,line->top_vector);
		line->bottom_y= view->screen_height; short_to_long_2d(view->bottom_edge,line->bottom_vector);
		/*
		line->top_y= 0, line->top_vector= view->top_edge;
		line->bottom_y= view->screen_height, line->bottom_vector= view->bottom_edge;
		*/
	}

	// LP change:
	ClippingWindows.ResetLength();
	/*
	next_clipping_window_index= 0;
	next_clipping_window= clipping_windows;
	*/
	
	return;
}

static void calculate_line_clipping_information(
	struct view_data *view,
	short line_index,
	word clip_flags)
{
	// LP addition: extend the line-clip list
	assert(LineClips.Add());
	unsigned int Length = LineClips.GetLength();
	assert(Length <= 32767);
	short LastIndex = Length-1;
	
	struct line_data *line= get_line_data(line_index);
	// LP change: relabeling p0 and p1 so as not to conflict with later use
	world_point2d p0_orig= get_endpoint_data(line->endpoint_indexes[0])->vertex;
	world_point2d p1_orig= get_endpoint_data(line->endpoint_indexes[1])->vertex;
	// LP addition: place for new line data
	struct line_clip_data *data= &LineClips[LastIndex];
	// struct line_clip_data *data= next_line_clip++;

	/* it’s possible (in fact, likely) that this line’s endpoints have not been transformed yet,
		so we have to do it ourselves */
	// LP change: making the operation long-distance friendly
	word p0_flags = 0, p1_flags = 0;
	transform_overflow_point2d(&p0_orig, (world_point2d *) &view->origin, view->yaw, &p0_flags);
	transform_overflow_point2d(&p1_orig, (world_point2d *) &view->origin, view->yaw, &p1_flags);
	/*
	transform_point2d(&p0, (world_point2d *) &view->origin, view->yaw);
	transform_point2d(&p1, (world_point2d *) &view->origin, view->yaw);	
	*/
	
	// Defining long versions here and copying over
	long_point2d p0, p1;
	long_vector2d *pv0ptr = (long_vector2d*)(&p0), *pv1ptr = (long_vector2d*)(&p1);
	overflow_short_to_long_2d(p0_orig,p0_flags,*pv0ptr);
	overflow_short_to_long_2d(p1_orig,p1_flags,*pv1ptr);
	
	clip_flags&= _clip_up|_clip_down;	
	assert(clip_flags&(_clip_up|_clip_down));
	// LP change:
	// assert(next_line_clip_index<MAXIMUM_LINE_CLIPS);
	assert(!TEST_RENDER_FLAG(line_index, _line_has_clip_data));

	SET_RENDER_FLAG(line_index, _line_has_clip_data);
	// LP change:
	line_clip_indexes[line_index]= LastIndex;
	// line_clip_indexes[line_index]= next_line_clip_index++;
	
	data->flags= 0;

	if (p0.x>0 && p1.x>0)
	{
		// LP change:
		long_point2d *p;
		// world_point2d *p;
		world_distance z;
		long transformed_z;
		long y, y0, y1;
		long x0= view->half_screen_width + (p0.y*view->world_to_screen_x)/p0.x;
		long x1= view->half_screen_width + (p1.y*view->world_to_screen_x)/p1.x;
	
		data->x0= PIN(x0, 0, view->screen_width);
		data->x1= PIN(x1, 0, view->screen_width);
		if (data->x1<data->x0) SWAP(data->x0, data->x1);
		if (data->x1>data->x0)
		{
			if (clip_flags&_clip_up)
			{
				/* precalculate z and transformed_z */
				z= line->lowest_adjacent_ceiling-view->origin.z;
				transformed_z= z*view->world_to_screen_y;
				
				/* calculate and clip y0 and y1 (screen y-coordinates of each side of the line) */
				y0= (p0.x>0) ? (view->half_screen_height - transformed_z/p0.x + view->dtanpitch) : 0;
				y1= (p1.x>0) ? (view->half_screen_height - transformed_z/p1.x + view->dtanpitch) : 0;
		
				/* pick the highest (closest to zero) and pin it to the screen */
				if (y0<y1) y= y0, p= &p0; else y= y1, p= &p1;
				y= PIN(y, 0, view->screen_height);
				
				/* if we’re not useless (clipping up off the top of the screen) set up top-clip information) */
				if (y<=0)
				{
					clip_flags&= ~_clip_up;
				}
				else
				{
					data->top_vector.i= - p->x, data->top_vector.j= - z;
					data->top_y= y;
				}
			}
			
			if (clip_flags&_clip_down)
			{
				z= line->highest_adjacent_floor - view->origin.z;
				transformed_z= z*view->world_to_screen_y;
				
				/* calculate and clip y0 and y1 (screen y-coordinates of each side of the line) */
				y0= (p0.x>0) ? (view->half_screen_height - transformed_z/p0.x + view->dtanpitch) : view->screen_height;
				y1= (p1.x>0) ? (view->half_screen_height - transformed_z/p1.x + view->dtanpitch) : view->screen_height;
				
				/* pick the highest (closest to zero screen_height) and pin it to the screen */
				if (y0>y1) y= y0, p= &p0; else y= y1, p= &p1;
				y= PIN(y, 0, view->screen_height);
				
				/* if we’re not useless (clipping up off the bottom of the screen) set up top-clip information) */
				if (y>=view->screen_height)
				{
					clip_flags&= ~_clip_down;
				}
				else
				{
					data->bottom_vector.i= p->x,  data->bottom_vector.j= z;
					data->bottom_y= y;
				}
			}
	
			data->flags= clip_flags;
//			dprintf("line #%d clips %x @ %p", line_index, clip_flags, data);
		}
	}

	return;
}

/* we can actually rely on the given endpoint being transformed because we only set clipping
	information for endpoints we’re aiming at, and we transform endpoints before firing at them */
static short calculate_endpoint_clipping_information(
	struct view_data *view,
	short endpoint_index,
	word clip_flags)
{
	// LP addition: extend the endpoint-clip list
	assert(EndpointClips.Add());
	unsigned int Length = EndpointClips.GetLength();
	assert(Length <= 32767);
	short LastIndex = Length-1;

	struct endpoint_data *endpoint= get_endpoint_data(endpoint_index);
	// LP change:
	struct endpoint_clip_data *data= &EndpointClips[LastIndex];
	// struct endpoint_clip_data *data= next_endpoint_clip++;
	long x;

	// LP change:
	// assert(next_endpoint_clip_index<MAXIMUM_ENDPOINT_CLIPS);
	assert((clip_flags&(_clip_left|_clip_right))); /* must have a clip flag */
	assert((clip_flags&(_clip_left|_clip_right))!=(_clip_left|_clip_right)); /* but can’t have both */
	assert(!TEST_RENDER_FLAG(endpoint_index, _endpoint_has_clip_data));
	
	// LP change: compose a true transformed point to replace endpoint->transformed,
	// and use it in the upcoming code
	long_vector2d transformed_endpoint;
	overflow_short_to_long_2d(endpoint->transformed,endpoint->flags,transformed_endpoint);
	
	data->flags= clip_flags&(_clip_left|_clip_right);
	switch (data->flags)
	{
		case _clip_left:
			data->vector.i= transformed_endpoint.i;
			data->vector.j= transformed_endpoint.j;
			/*
			data->vector.i= endpoint->transformed.x;
			data->vector.j= endpoint->transformed.y;
			*/
			break;
		case _clip_right: /* negatives so we clip to the correct side */
			data->vector.i= -transformed_endpoint.i;
			data->vector.j= -transformed_endpoint.j;
			/*
			data->vector.i= -endpoint->transformed.x;
			data->vector.j= -endpoint->transformed.y;
			*/
			break;
	}
	warn(data->vector.i);
	
	assert(TEST_RENDER_FLAG(endpoint_index, _endpoint_has_been_transformed));
	x= endpoint_x_coordinates[endpoint_index];

	data->x= PIN(x, 0, view->screen_width);
	
	// LP change:
	return LastIndex;
	// return next_endpoint_clip_index++;
}

/* ---------- initializing, building and sorting the object list */

static void initialize_render_object_list(
	void)
{
	// LP change: using growable list
	RenderObjects.ResetLength();
	/*
	render_object_count= 0;
	next_render_object= render_objects;
	*/
	
	return;
}

#define MAXIMUM_OBJECT_BASE_NODES 6

/* walk our sorted polygon lists, adding every object in every polygon to the render_object list,
	in depth order */
static void build_render_object_list(
	struct view_data *view)
{
	struct sorted_node_data *sorted_node;

	initialize_render_object_list();
	
	// LP change:
	for (sorted_node= SortedNodes.RevBegin();sorted_node>SortedNodes.RevEnd();--sorted_node)
	// for (sorted_node= next_sorted_node-1;sorted_node>=sorted_nodes;--sorted_node)
	{
		struct polygon_data *polygon= get_polygon_data(sorted_node->polygon_index);
		fixed ambient_intensity= get_light_intensity(polygon->floor_lightsource_index);
		short object_index= polygon->first_object;
		
		while (object_index!=NONE)
		{
			short base_node_count;
			struct sorted_node_data *base_nodes[MAXIMUM_OBJECT_BASE_NODES];
			// LP change:
			struct render_object_data *render_object= build_render_object(view, (long_point3d *) NULL, ambient_intensity, base_nodes, &base_node_count, object_index);
			// struct render_object_data *render_object= build_render_object(view, (world_point3d *) NULL, ambient_intensity, base_nodes, &base_node_count, object_index);
			
			if (render_object)
			{
				build_aggregate_render_object_clipping_window(render_object, base_nodes, base_node_count);
				sort_render_object_into_tree(render_object, base_nodes, base_node_count);
			}
			
			object_index= get_object_data(object_index)->next_object;
		}
	}

	return;
}

// LP change: make it better able to do long-distance views
static struct render_object_data *build_render_object(
	struct view_data *view,
	long_point3d *origin, // world_point3d *origin,
	fixed ambient_intensity,
	struct sorted_node_data **base_nodes,
	short *base_node_count,
	short object_index)
{
	struct render_object_data *render_object= (struct render_object_data *) NULL;
	struct object_data *object= get_object_data(object_index);
	
	// LP change: removed upper limit on number (restored it later)
	// if (!OBJECT_IS_INVISIBLE(object))
	if (!OBJECT_IS_INVISIBLE(object) && RenderObjects.GetLength()<MAXIMUM_RENDER_OBJECTS)
	{
		// LP change: made this more long-distance-friendly
		long_point3d transformed_origin;
		// world_point3d transformed_origin;
		
		if (origin)
		{
			// LP change:
			transformed_origin.x = origin->x;
			transformed_origin.y = origin->y;
			transformed_origin.z = origin->z;
			// transformed_origin= *origin;
		}
		else
		{
			// LP change:
			world_point2d temp_tfm_origin;
			temp_tfm_origin.x = object->location.x;
			temp_tfm_origin.y = object->location.y;
			transformed_origin.z = object->location.z - view->origin.z;
			word tfm_origin_flags;
			transform_overflow_point2d(&temp_tfm_origin, (world_point2d *)&view->origin, view->yaw, &tfm_origin_flags);
			long_vector2d *tfm_origin_ptr = (long_vector2d *)(&transformed_origin);
			overflow_short_to_long_2d(temp_tfm_origin,tfm_origin_flags,*tfm_origin_ptr);
			/*
			transformed_origin= object->location;
			transformed_origin.z-= view->origin.z;
			transform_point2d((world_point2d *) &transformed_origin, (world_point2d *)&view->origin, view->yaw);
			*/
		}

		if (transformed_origin.x>MINIMUM_OBJECT_DISTANCE)
		{
			short x0, x1, y0, y1;
			struct shape_and_transfer_mode data;
			struct shape_information_data *shape_information;
			struct shape_information_data scaled_shape_information; // if necessary
			
			get_object_shape_and_transfer_mode(&view->origin, object_index, &data);
			shape_information= rescale_shape_information(
				extended_get_shape_information(data.collection_code, data.low_level_shape_index),
				&scaled_shape_information, GET_OBJECT_SCALE_FLAGS(object));

			/* if the caller wants it, give him the left and right extents of this shape */
			if (base_nodes)
			{
				*base_node_count= build_base_node_list(view, object->polygon, &object->location,
					shape_information->world_left, shape_information->world_right, base_nodes);
			}
			
			x0= view->half_screen_width + ((transformed_origin.y+shape_information->world_left)*view->world_to_screen_x)/transformed_origin.x;
			x1= view->half_screen_width + ((transformed_origin.y+shape_information->world_right)*view->world_to_screen_x)/transformed_origin.x;
			y0=	view->half_screen_height - (view->world_to_screen_y*(transformed_origin.z+shape_information->world_top))/transformed_origin.x + view->dtanpitch;
			y1= view->half_screen_height - (view->world_to_screen_y*(transformed_origin.z+shape_information->world_bottom))/transformed_origin.x + view->dtanpitch;
			if (x0<x1 && y0<y1)
			{
				// LP Change:
				int Length = RenderObjects.GetLength();
				POINTER_DATA OldROPointer;
				// Will memory get swapped?
				bool DoSwap = Length >= RenderObjects.GetCapacity();
				if (DoSwap) OldROPointer = POINTER_CAST(RenderObjects.Begin());
				assert(RenderObjects.Add());
				if (DoSwap)
				{
					// Get the render objects and sorted nodes into sync
					POINTER_DATA NewROPointer = POINTER_CAST(RenderObjects.Begin());
					for (int k=0; k<RenderObjects.GetLength(); k++)
					{
						render_object_data &RenderObject = RenderObjects[k];
						if (RenderObject.next_object != NULL)
							RenderObject.next_object = (render_object_data *)(NewROPointer + (POINTER_CAST(RenderObject.next_object) - OldROPointer));
					}
					for (int k=0; k<SortedNodes.GetLength(); k++)
					{
						sorted_node_data &SortedNode = SortedNodes[k];
						if (SortedNode.interior_objects != NULL)
							SortedNode.interior_objects = (render_object_data *)(NewROPointer + (POINTER_CAST(SortedNode.interior_objects) - OldROPointer));
						if (SortedNode.exterior_objects != NULL)
							SortedNode.exterior_objects = (render_object_data *)(NewROPointer + (POINTER_CAST(SortedNode.exterior_objects) - OldROPointer));
					}
				}
				render_object= &RenderObjects[Length];
				/*
				render_object= next_render_object++;
				render_object_count+= 1;
				*/
				
				render_object->rectangle.flags= 0;
				
				render_object->rectangle.x0= x0;
				render_object->rectangle.x1= x1;
				render_object->rectangle.y0= y0;
				render_object->rectangle.y1= y1;

				{
					// LP change: doing media handling more correctly here:
					short media_index = get_polygon_data(object->polygon)->media_index;
					media_data *media = (media_index != NONE) ? get_media_data(media_index) : NULL;
					
					// LP: the media splashes are clipped as if there was no liquid
					if (media && !OBJECT_IS_MEDIA_EFFECT(object))
					{
						render_object->ymedia= view->half_screen_height - (view->world_to_screen_y*(media->height-view->origin.z))/transformed_origin.x + view->dtanpitch;
					}
					else
					{
						// All the way down
						render_object->ymedia= SHORT_MAX;
					}
					
					/*
					short media_index= view->under_media_boundary ? view->under_media_index : get_polygon_data(object->polygon)->media_index;
					
					if (media_index!=NONE && !OBJECT_IS_MEDIA_EFFECT(object))
					{
						render_object->ymedia= view->half_screen_height - (view->world_to_screen_y*(media->height-view->origin.z))/transformed_origin.x + view->dtanpitch;
					}
					else
					{
						render_object->ymedia= SHORT_MAX;
					}
					*/
				}
				
				extended_get_shape_bitmap_and_shading_table(data.collection_code, data.low_level_shape_index,
					&render_object->rectangle.texture, &render_object->rectangle.shading_tables, view->shading_mode);
		
				// LP change: for the convenience of the OpenGL renderer
				render_object->rectangle.ShapeDesc = BUILD_DESCRIPTOR(data.collection_code,data.low_level_shape_index);
				
				render_object->rectangle.flip_vertical= FALSE;
				render_object->rectangle.flip_horizontal= (shape_information->flags&_X_MIRRORED_BIT) ? TRUE : FALSE;
				
				render_object->rectangle.depth= transformed_origin.x;
				instantiate_rectangle_transfer_mode(view, &render_object->rectangle, data.transfer_mode, data.transfer_phase);
				
				render_object->rectangle.ambient_shade= MAX(shape_information->minimum_light_intensity, ambient_intensity);

				if (view->shading_mode==_shading_infravision) render_object->rectangle.flags|= _SHADELESS_BIT;
				
				render_object->next_object= (struct render_object_data *) NULL;
				if (object->parasitic_object!=NONE)
				{
					struct render_object_data *parasitic_render_object;
					// LP change:
					long_point3d parasitic_origin= transformed_origin;
					// world_point3d parasitic_origin= transformed_origin;
					
					parasitic_origin.z+= shape_information->world_y0;
					parasitic_origin.y+= shape_information->world_x0;
					parasitic_render_object= build_render_object(view, &parasitic_origin, ambient_intensity,
						(struct sorted_node_data **) NULL, (short *) NULL, object->parasitic_object);
					
					if (parasitic_render_object)
					{
						assert(!parasitic_render_object->next_object); /* one parasite only, please */
	
						/* take the maximum intensity of the host and parasite as the intensity of the
							aggregate (does not handle multiple parasites correctly) */
						parasitic_render_object->rectangle.ambient_shade= render_object->rectangle.ambient_shade=
							MAX(parasitic_render_object->rectangle.ambient_shade, render_object->rectangle.ambient_shade);
						
						if (shape_information->flags&_KEYPOINT_OBSCURED_BIT) /* host obscures parasite */
						{
							render_object->next_object= parasitic_render_object;
						}
						else /* parasite obscures host; does not properly handle multiple parasites */
						{
							parasitic_render_object->next_object= render_object;
							render_object= parasitic_render_object;
						}
					}
				}
			}
		}
	}
	
	return render_object;
}

static void sort_render_object_into_tree(
	struct render_object_data *new_render_object, /* null-terminated linked list */
	struct sorted_node_data **base_nodes,
	short base_node_count)
{
	struct render_object_data *render_object, *last_new_render_object;
	struct render_object_data *deep_render_object= (struct render_object_data *) NULL;
	struct render_object_data *shallow_render_object= (struct render_object_data *) NULL;
	struct sorted_node_data *desired_node;
	short i;

	/* find the last render_object in the given list of new objects */
	for (last_new_render_object= new_render_object;
			last_new_render_object->next_object;
			last_new_render_object= last_new_render_object->next_object)
		;

	/* find the two objects we must be lie between */
	// LP change:
	for (render_object= RenderObjects.Begin(); render_object<RenderObjects.End(); ++render_object)
	// for (render_object= render_objects; render_object<new_render_object; ++render_object)
	{
		/* if these two objects intersect... */
		if (render_object->rectangle.x1>new_render_object->rectangle.x0 && render_object->rectangle.x0<new_render_object->rectangle.x1 &&
			render_object->rectangle.y1>new_render_object->rectangle.y0 && render_object->rectangle.y0<new_render_object->rectangle.y1)
		{
			/* update our closest and farthest matches */
			if (render_object->rectangle.depth>new_render_object->rectangle.depth) /* found deeper intersecting object */
			{
				if (!deep_render_object || deep_render_object->rectangle.depth>render_object->rectangle.depth)
				{
					deep_render_object= render_object;
				}
			}
			else
			{
				if (render_object->rectangle.depth<new_render_object->rectangle.depth) /* found shallower intersecting object */
				{
					if (!shallow_render_object || shallow_render_object->rectangle.depth<=render_object->rectangle.depth)
					{
						shallow_render_object= render_object;
					}
				}
			}
		}
	}

	/* find the node we’d like to be in (that is, the node closest to the viewer of all the nodes
		we cross and therefore the latest one in the sorted node list) */
	desired_node= base_nodes[0];
	for (i= 1; i<base_node_count; ++i) if (base_nodes[i]>desired_node) desired_node= base_nodes[i];
	assert(desired_node>=SortedNodes.Begin() && desired_node<SortedNodes.End());
	// assert(desired_node>=sorted_nodes && desired_node<next_sorted_node);
	
	/* adjust desired node based on the nodes of the deep and shallow render object; only
		one of deep_render_object and shallow_render_object will be non-null after this if
		block.  the current object must be sorted with respect to this non-null object inside
		the object list of the desired_node */
	if (shallow_render_object && desired_node>=shallow_render_object->node)
	{
		/* we tried to sort too close to the front of the node list */
		desired_node= shallow_render_object->node;
		deep_render_object= (struct render_object_data *) NULL;
	}
	else
	{
		if (deep_render_object && desired_node<=deep_render_object->node)
		{
			/* we tried to sort too close to the back of the node list */
			desired_node= deep_render_object->node;
			shallow_render_object= (struct render_object_data *) NULL;
		}
		else
		{
			deep_render_object= shallow_render_object= (struct render_object_data *) NULL;
		}
	}
	
	/* update the .node fields of all the objects we’re about to add to reflect their new
		location in the sorted node list */
	for (render_object= new_render_object; render_object; render_object= render_object->next_object)
	{
		render_object->node= desired_node;
	}
	
	if (deep_render_object)
	{
		/* if it turns out that the object after deep_render_object (which we think we should be
			drawn in front of) is also deeper than us, make it the new deep_render_object */
		while ((render_object= deep_render_object->next_object)!=NULL
				&& render_object->rectangle.depth>new_render_object->rectangle.depth)
			deep_render_object= render_object;

		/* sort after deep_render_object object in the given node (so we are drawn in front of it) */
		last_new_render_object->next_object= deep_render_object->next_object;
		deep_render_object->next_object= new_render_object;
	}
	else
	{
//		if (shallow_render_object)
		{
			struct render_object_data **reference;
			
			/* find the reference to the shallow_render_object in the node list first (or the
				first object which is closer than new_render_object) */
			for (reference= &desired_node->exterior_objects;
					*reference!=shallow_render_object && *reference && (*reference)->rectangle.depth>new_render_object->rectangle.depth;
					reference= &(*reference)->next_object)
				;
			assert(!shallow_render_object || *reference);
			
			/* sort before this object in the given node (so we are drawn behind it) */
			last_new_render_object->next_object= *reference;
			*reference= new_render_object;
		}
//		else
//		{
//			/* sort anywhere in the node */
//			last_new_render_object->next_object= desired_node->exterior_objects;
//			desired_node->exterior_objects= new_render_object;
//		}
	}

	return;
}

enum /* build_base_node_list() states */
{
	_casting_left,
	_casting_right
};

/* we once thought it would be a clever idea to use the transformed endpoints, but, not.  we
	now bail if we can’t find a way out of the polygon we are given; usually this happens
	when we’re moving along gridlines */
static short build_base_node_list(
	struct view_data *view,
	short origin_polygon_index,
	world_point3d *origin,
	world_distance left_distance,
	world_distance right_distance,
	struct sorted_node_data **base_nodes)
{
	short cast_state;
	short base_node_count;
	world_distance origin_polygon_floor_height= get_polygon_data(origin_polygon_index)->floor_height;
	
	base_node_count= 1;
	base_nodes[0]= polygon_index_to_sorted_node[origin_polygon_index];

	cast_state= _casting_left;
	do
	{
		world_point2d destination= *((world_point2d *)origin);
		short polygon_index= origin_polygon_index;
		world_vector2d vector;
		
		switch (cast_state)
		{
			case _casting_left:
				translate_point2d(&destination, right_distance, NORMALIZE_ANGLE(view->yaw-QUARTER_CIRCLE));
//				dprintf("%s: (#%d,#%d)==>(#%d,#%d) (by #%d)", cast_state==_casting_left ? "left" : "right", origin->x, origin->y, destination.x, destination.y, cast_state==_casting_left ? left_distance : right_distance);
				cast_state= _casting_right;
				break;
			case _casting_right:
				translate_point2d(&destination, left_distance, NORMALIZE_ANGLE(view->yaw-QUARTER_CIRCLE));
//				dprintf("%s: (#%d,#%d)==>(#%d,#%d) (by #%d)", cast_state==_casting_left ? "left" : "right", origin->x, origin->y, destination.x, destination.y, cast_state==_casting_left ? left_distance : right_distance);
				cast_state= NONE;
				break;
			
			default:
				// LP change:
				assert(false);
				// halt();
		}

		vector.i= destination.x - origin->x;
		vector.j= destination.y - origin->y;
		
		/* move toward the given destination accumulating polygon indexes */
		do
		{
			struct polygon_data *polygon= get_polygon_data(polygon_index);
			short state= _looking_for_first_nonzero_vertex; /* really: testing first vertex state (we don’t have zero vertices) */
			short vertex_index= 0, vertex_delta= 1; /* start searching clockwise from vertex zero */
			world_point2d *vertex, *next_vertex;
			
			do
			{
				vertex= &get_endpoint_data(polygon->endpoint_indexes[vertex_index])->vertex;
		
				if ((vertex->x-origin->x)*vector.j - (vertex->y-origin->y)*vector.i >= 0)
				{
					/* endpoint is on the left side of our vector */
					switch (state)
					{
						case _looking_for_first_nonzero_vertex:
							/* search clockwise for transition (left to right) */
							state= _looking_clockwise_for_right_vertex;
							break;
						
						case _looking_counterclockwise_for_left_vertex: /* found the transition we were looking for */
							next_vertex= &get_endpoint_data(polygon->endpoint_indexes[WRAP_HIGH(vertex_index, polygon->vertex_count-1)])->vertex;
							polygon_index= polygon->adjacent_polygon_indexes[vertex_index];
							state= NONE;
							break;
					}
				}
				else
				{
					/* endpoint is on the right side of our vector */
					switch (state)
					{
						case _looking_for_first_nonzero_vertex:
							/* search counterclockwise for transition (right to left) */
							state= _looking_counterclockwise_for_left_vertex;
							vertex_delta= -1;
							break;
						
						case _looking_clockwise_for_right_vertex: /* found the transition we were looking for */
							next_vertex= vertex;
							vertex= &get_endpoint_data(polygon->endpoint_indexes[WRAP_LOW(vertex_index, polygon->vertex_count-1)])->vertex;
							polygon_index= polygon->adjacent_polygon_indexes[WRAP_LOW(vertex_index, polygon->vertex_count-1)];
							state= NONE;
							break;
					}
				}
				
				/* adjust vertex_index (clockwise or counterclockwise, depending on vertex_delta) */
				vertex_index= (vertex_delta<0) ? WRAP_LOW(vertex_index, polygon->vertex_count-1) :
					WRAP_HIGH(vertex_index, polygon->vertex_count-1);
				if (state!=NONE&&!vertex_index) polygon_index= state= NONE; /* we can’t find a way out; give up */
			}
			while (state!=NONE);
			
			if (polygon_index!=NONE)
			{
				polygon= get_polygon_data(polygon_index);
				
				/* can’t do above clipping (see note in change history) */
				if ((view->origin.z<origin->z && polygon->floor_height<origin_polygon_floor_height) ||
					(view->origin.z>origin->z && origin->z+WORLD_ONE_HALF<polygon->floor_height && polygon->floor_height>origin_polygon_floor_height))
				{
					/* if we’re above the viewer and going into a lower polygon or below the viewer and going
						into a higher polygon, don’t */
//					dprintf("discarding polygon #%d by height", polygon_index);
					polygon_index= NONE;
				}
				else
				{
//					dprintf("  into polygon #%d", polygon_index);
					if (!TEST_RENDER_FLAG(polygon_index, _polygon_is_visible)) polygon_index= NONE; /* don’t have transformed data, don’t even try! */
					if ((destination.x-vertex->x)*(next_vertex->y-vertex->y) - (destination.y-vertex->y)*(next_vertex->x-vertex->x) <= 0) polygon_index= NONE;
					if (polygon_index!=NONE && base_node_count<MAXIMUM_OBJECT_BASE_NODES) base_nodes[base_node_count++]= polygon_index_to_sorted_node[polygon_index];
				}
			}
		}
		while (polygon_index!=NONE);
	}
	while (cast_state!=NONE);

//	dprintf("found #%d polygons @ %p;dm %x %d;", base_polygon_count, base_polygon_indexes, base_polygon_indexes, base_polygon_count*sizeof(short));
	
	return base_node_count;
}

/* find the lowest bottom clip and the highest top clip of all nodes this object crosses.  then
	locate all left and right sides and compile them into one (or several) aggregate windows with
	the same top and bottom */
void build_aggregate_render_object_clipping_window(
	struct render_object_data *render_object,
	struct sorted_node_data **base_nodes,
	short base_node_count)
{
	struct clipping_window_data *first_window= (struct clipping_window_data *) NULL;
	
	if (base_node_count==1)
	{
		/* trivial case of one source window */
		first_window= base_nodes[0]->clipping_windows;
	}
	else
	{
		short i;
		short y0, y1;
		short left, right, left_count, right_count;
		short x0[MAXIMUM_OBJECT_BASE_NODES], x1[MAXIMUM_OBJECT_BASE_NODES]; /* sorted, left to right */
		struct clipping_window_data *window;
		world_distance depth= render_object->rectangle.depth;
		
		/* find the upper and lower bounds of the windows; we could do a better job than this by
			doing the same thing we do when the windows are originally built (i.e., calculating a
			new top/bottom for every window.  but screw that.  */
		left_count= right_count= 0;
		y0= SHORT_MAX, y1= SHORT_MIN;
		for (i= 0; i<base_node_count; ++i)
		{
			short j, k;
			
			window= base_nodes[i]->clipping_windows;
			
			/* update the top and bottom clipping bounds */
			if (window->y0<y0) y0= window->y0;
			if (window->y1>y1) y1= window->y1;
 			
			/* sort in the left side of this window */
			if (ABS(window->left.i)<depth)
			{
				for (j= 0; j<left_count && window->x0>=x0[j]; ++j)
					;
				for (k= j; k<left_count; ++k)
					x0[k+1]= x0[k];
				x0[j]= window->x0;
				left_count+= 1;
			}
			
			/* sort in the right side of this window */
			if (ABS(window->right.i)<depth)
			{
				for (j= 0; j<right_count && window->x1>=x1[j]; ++j)
					;
				for (k= j; k<right_count; ++k)
					x1[k+1]= x1[k];
				x1[j]= window->x1;
				right_count+= 1;
			}
		}
		
		/* build the windows, left to right */
		for (left= 0, right= 0; left<left_count && right<right_count; )
		{
			if (left==left_count-1 || x0[left+1]>x1[right])
			{
				if (x0[left]<x1[right]) /* found one between x0[left] and x1[right] */
				{
					/* allocate it */
					// LP Change:
					int Length = ClippingWindows.GetLength();
					POINTER_DATA OldCWPointer;
					// Will memory get swapped?
					bool DoSwap = Length >= ClippingWindows.GetCapacity();
					if (DoSwap) OldCWPointer = POINTER_CAST(ClippingWindows.Begin());
					assert(ClippingWindows.Add());
					if (DoSwap)
					{
						// Get the sorted nodes into sync
						// Also, the render objects and the parent window
						POINTER_DATA NewCWPointer = POINTER_CAST(ClippingWindows.Begin());
						for (int k=0; k<ClippingWindows.GetLength(); k++)
						{
							clipping_window_data &ClippingWindow = ClippingWindows[k];
							if (ClippingWindow.next_window != NULL)
								ClippingWindow.next_window = (clipping_window_data *)(NewCWPointer + (POINTER_CAST(ClippingWindow.next_window) - OldCWPointer));
						}
						for (int k=0; k<SortedNodes.GetLength(); k++)
						{
							sorted_node_data &SortedNode = SortedNodes[k];
							if (SortedNode.clipping_windows != NULL)
								SortedNode.clipping_windows = (clipping_window_data *)(NewCWPointer + (POINTER_CAST(SortedNode.clipping_windows) - OldCWPointer));
						}
						for (int k=0; k<RenderObjects.GetLength(); k++)
						{
							render_object_data &RenderObject = RenderObjects[k];
							if (RenderObject.clipping_windows != NULL)
								RenderObject.clipping_windows = (clipping_window_data *)(NewCWPointer + (POINTER_CAST(RenderObject.clipping_windows) - OldCWPointer));
						}
						if (first_window != NULL)
							first_window = (clipping_window_data *)(NewCWPointer + (POINTER_CAST(first_window) - OldCWPointer));
					}
					window= &ClippingWindows[Length];
					/*
					assert(next_clipping_window_index++<MAXIMUM_CLIPPING_WINDOWS);
					window= next_clipping_window++;
					*/
					
					/* build it */
					window->x0= x0[left], window->x1= x1[right];
					window->y0= y0, window->y1= y1;
					
					/* link it */
					window->next_window= first_window;
					first_window= window;
				}
				
				/* advance left by one, then advance right until it’s greater than left */
				if (++left<left_count) while (x0[left]>x1[right] && right<right_count) ++right;
			}
			else
			{
				left+= 1;
			}
		}
	}
	
	/* stuff our windows in all objects hanging off our first object (i.e., all parasites) */	
	for (; render_object; render_object= render_object->next_object) render_object->clipping_windows= first_window;

	return;
}

/* ---------- rendering the tree */

static void render_tree(
	struct view_data *view,
	struct bitmap_definition *destination)
{
	struct sorted_node_data *node;
	
	// LP change: added support for semitransparent liquids
	bool SeeThruLiquids = TEST_FLAG(Get_OGL_ConfigureData().Flags,OGL_Flag_LiqSeeThru) != 0;
	
	/* walls, ceilings, interior objects, floors, exterior objects for all nodes, back to front */
	// LP change:
	for (node= SortedNodes.Begin(); node<SortedNodes.End(); ++node)
	// for (node= sorted_nodes; node<next_sorted_node; ++node)
	{
		struct polygon_data *polygon= get_polygon_data(node->polygon_index);
		struct clipping_window_data *window;
		struct render_object_data *object;

		// LP change: moved this stuff out here because it only has to be calculated
		// once per polygon.
		struct horizontal_surface_data floor_surface, ceiling_surface;
		short i;
		
		ceiling_surface.origin= polygon->ceiling_origin;
		ceiling_surface.height= polygon->ceiling_height;
		ceiling_surface.texture= polygon->ceiling_texture;
		ceiling_surface.lightsource_index= polygon->ceiling_lightsource_index;
		ceiling_surface.transfer_mode= polygon->ceiling_transfer_mode;
		ceiling_surface.transfer_mode_data= 0;
		
		floor_surface.origin= polygon->floor_origin;
		floor_surface.height= polygon->floor_height;
		floor_surface.texture= polygon->floor_texture;
		floor_surface.lightsource_index= polygon->floor_lightsource_index;
		floor_surface.transfer_mode= polygon->floor_transfer_mode;
		floor_surface.transfer_mode_data= 0;
		
		// The "continue" conditions are OK to move out here, because a non-drawn polygon's
		// inhabitants will be clipped away.
		
		// LP: get liquid data here for convenience;
		// pointer to it being NULL means no liquid in the polygon
		struct media_data *media = NULL;
		if (polygon->media_index!=NONE)
			media = get_media_data(polygon->media_index);
		
		/* if necessary, replace the ceiling or floor surfaces with the media surface */
		// LP change: don't do this step if liquids are semitransparent
		if (media && !SeeThruLiquids)
		// if (polygon->media_index!=NONE)
		{
			// LP change: moved this get upwards
			// struct media_data *media= get_media_data(polygon->media_index);
			struct horizontal_surface_data *media_surface= (struct horizontal_surface_data *) NULL;
					
			if (view->under_media_boundary)
			{
				// LP change: skip if high and dry
				if (media->height <= polygon->floor_height)
					continue;
				
				if (media->height<polygon->ceiling_height) media_surface= &ceiling_surface;
			}
			else
			{
				// LP change: skip if submerged
				if (media->height >= polygon->ceiling_height)
					continue;
				
				if (media->height>polygon->floor_height) media_surface= &floor_surface;
			}
			
			if (media_surface)
			{
				media_surface->origin= media->origin;
				media_surface->height= media->height;
				media_surface->texture= media->texture;
				media_surface->lightsource_index= polygon->media_lightsource_index;
				media_surface->transfer_mode= media->transfer_mode;
				media_surface->transfer_mode_data= 0;
			}
		}
		// LP change: always render liquids that are semitransparent
		else if (!SeeThruLiquids)
		// else
		{
			// if we’re trying to draw a polygon without media from under a polygon with media, don’t
			if (view->under_media_boundary) continue;
		}
		
		// LP: this loop renders the walls
		for (window= node->clipping_windows; window; window= window->next_window)
		{
			if (ceiling_surface.height>floor_surface.height)
			{
				/* render ceiling if above viewer */
				if (ceiling_surface.height>view->origin.z)
				{
					// LP change: indicated that the void is on other side
					render_node_floor_or_ceiling(view, destination, window, polygon, &ceiling_surface, true);
					// render_node_floor_or_ceiling(view, destination, window, polygon, &ceiling_surface);
				}
				
				/* render visible sides */
				for (i= 0; i<polygon->vertex_count; ++i)
				{
					short side_index= polygon->side_indexes[i];
					
					if (side_index!=NONE && TEST_RENDER_FLAG(side_index, _side_is_visible))
					{
						struct line_data *line= get_line_data(polygon->line_indexes[i]);
						struct side_data *side= get_side_data(side_index);
						struct vertical_surface_data surface;
					
						surface.length= line->length;
						// LP change: expanded the transformed-endpoint access
						endpoint_data *endpoint0 = get_endpoint_data(polygon->endpoint_indexes[i]);
						overflow_short_to_long_2d(endpoint0->transformed,endpoint0->flags,surface.p0);
						endpoint_data *endpoint1 = get_endpoint_data(polygon->endpoint_indexes[WRAP_HIGH(i, polygon->vertex_count-1)]);
						overflow_short_to_long_2d(endpoint1->transformed,endpoint1->flags,surface.p1);
						/*
						surface.p0= get_endpoint_data(polygon->endpoint_indexes[i])->transformed;
						surface.p1= get_endpoint_data(polygon->endpoint_indexes[WRAP_HIGH(i, polygon->vertex_count-1)])->transformed;
						*/
						surface.ambient_delta= side->ambient_delta;
						
						// LP change: indicate in all cases whether the void is on the other side
						switch (side->type)
						{
							case _full_side:
								surface.lightsource_index= side->primary_lightsource_index;
								surface.h0= floor_surface.height - view->origin.z;
								surface.hmax= ceiling_surface.height - view->origin.z;
								surface.h1= polygon->ceiling_height - view->origin.z;
								surface.texture_definition= &side->primary_texture;
								surface.transfer_mode= side->primary_transfer_mode;
								render_node_side(view, destination, window, &surface, true);
								// render_node_side(view, destination, window, &surface);
								break;
							case _split_side: /* render _low_side first */
								surface.lightsource_index= side->secondary_lightsource_index;
								surface.h0= floor_surface.height - view->origin.z;
								surface.h1= MAX(line->highest_adjacent_floor, floor_surface.height) - view->origin.z;
								surface.hmax= ceiling_surface.height - view->origin.z;
								surface.texture_definition= &side->secondary_texture;
								surface.transfer_mode= side->secondary_transfer_mode;
								render_node_side(view, destination, window, &surface, true);
								// render_node_side(view, destination, window, &surface);
								/* fall through and render high side */
							case _high_side:
								surface.lightsource_index= side->primary_lightsource_index;
								surface.h0= MIN(line->lowest_adjacent_ceiling, ceiling_surface.height) - view->origin.z;
								surface.hmax= ceiling_surface.height - view->origin.z;
								surface.h1= polygon->ceiling_height - view->origin.z;
								surface.texture_definition= &side->primary_texture;
								surface.transfer_mode= side->primary_transfer_mode;
								render_node_side(view, destination, window, &surface, true);
								// render_node_side(view, destination, window, &surface);
								break;
							case _low_side:
								surface.lightsource_index= side->primary_lightsource_index;
								surface.h0= floor_surface.height - view->origin.z;
								surface.h1= MAX(line->highest_adjacent_floor, floor_surface.height) - view->origin.z;
								surface.hmax= ceiling_surface.height - view->origin.z;
								surface.texture_definition= &side->primary_texture;
								surface.transfer_mode= side->primary_transfer_mode;
								render_node_side(view, destination, window, &surface, true);
								// render_node_side(view, destination, window, &surface);
								break;
							
							default:
								// LP change:
								assert(false);
								// halt();
						}
						
						if (side->transparent_texture.texture!=NONE)
						{
							surface.lightsource_index= side->transparent_lightsource_index;
							surface.h0= MAX(line->highest_adjacent_floor, floor_surface.height) - view->origin.z;
							surface.h1= line->lowest_adjacent_ceiling - view->origin.z;
							surface.hmax= ceiling_surface.height - view->origin.z;
							surface.texture_definition= &side->transparent_texture;
							surface.transfer_mode= side->transparent_transfer_mode;
							render_node_side(view, destination, window, &surface, false);
							// render_node_side(view, destination, window, &surface);
						}
					}
				}
				
				/* render floor if below viewer */
				if (floor_surface.height<view->origin.z)
				{
					// LP change: indicated that the void is on other side
					render_node_floor_or_ceiling(view, destination, window, polygon, &floor_surface, true);
					// render_node_floor_or_ceiling(view, destination, window, polygon, &floor_surface);
				}
			}
		}

		// LP: this is for objects on the other side of the liquids;
		// render them out here if one can see through the liquids
		if (SeeThruLiquids)
		{
			/* render exterior objects (with their own clipping windows) */
			for (object= node->exterior_objects; object; object= object->next_object)
			{
				render_node_object(view, destination, object, true);
			}
		}
		
		// LP: render the liquid surface after the walls and the stuff behind it
		// and before the stuff before it.
		if (media && SeeThruLiquids)
		{
			
			// Render only if between the floor and the ceiling:
			if (media->height > polygon->floor_height && media->height < polygon->ceiling_height)
			{
			
				// Render the liquids
				struct horizontal_surface_data LiquidSurface;
				
				LiquidSurface.origin= media->origin;
				LiquidSurface.height= media->height;
				LiquidSurface.texture= media->texture;
				LiquidSurface.lightsource_index= polygon->media_lightsource_index;
				LiquidSurface.transfer_mode= media->transfer_mode;
				LiquidSurface.transfer_mode_data= 0;
				
				for (window= node->clipping_windows; window; window= window->next_window)
				{
					render_node_floor_or_ceiling(view, destination, window, polygon, &LiquidSurface, false);
				}
			}
		}
		
		// LP: this is for objects on the view side of the liquids
		/* render exterior objects (with their own clipping windows) */
		for (object= node->exterior_objects; object; object= object->next_object)
		{
			render_node_object(view, destination, object, false);
			// render_node_object(view, destination, object);
		}
	}
	
	return;
}

/* ---------- rendering ceilings and floors */

// LP change: added "void present on other side" flag
static void render_node_floor_or_ceiling(
	struct view_data *view,
	struct bitmap_definition *destination,
	struct clipping_window_data *window,
	struct polygon_data *polygon,
	struct horizontal_surface_data *surface,
	bool void_present)
	// struct horizontal_surface_data *surface)
{
	// LP addition: animated-texture support
	// Extra variable defined so as not to edit the original texture
	shape_descriptor Texture = AnimTxtr_Translate(surface->texture);
	if (Texture!=NONE)
	// if (surface->texture!=NONE)
	{
		struct polygon_definition textured_polygon;
		flagged_world_point2d vertices[MAXIMUM_VERTICES_PER_WORLD_POLYGON];
		world_distance adjusted_height= surface->height-view->origin.z;
		long transformed_height= adjusted_height*view->world_to_screen_y;
		short vertex_count;
		short i;
	
		/* build transformed vertex list */
		vertex_count= polygon->vertex_count;
		for (i=0;i<vertex_count;++i)
		{
			// LP change: expanded the transformed-endpoint access
			long_vector2d temp_vertex;
			endpoint_data *endpoint = get_endpoint_data(polygon->endpoint_indexes[i]);
			overflow_short_to_long_2d(endpoint->transformed,endpoint->flags,temp_vertex);
			vertices[i].x = temp_vertex.i;
			vertices[i].y = temp_vertex.j;
			// *((world_point2d *)(vertices+i))= get_endpoint_data(polygon->endpoint_indexes[i])->transformed;
			vertices[i].flags= 0;
		}
		
		/* reversing the order in which these two were clipped caused weird problems */
		
		/* clip to left and right sides of the window */
		vertex_count= xy_clip_horizontal_polygon(vertices, vertex_count, &window->left, _clip_left);
		vertex_count= xy_clip_horizontal_polygon(vertices, vertex_count, &window->right, _clip_right);
		
		/* clip to top and bottom sides of the window */
		vertex_count= z_clip_horizontal_polygon(vertices, vertex_count, &window->top, adjusted_height, _clip_up);
		vertex_count= z_clip_horizontal_polygon(vertices, vertex_count, &window->bottom, adjusted_height, _clip_down);
	
		if (vertex_count)
		{
			/* transform the points we have into screen-space (backwards for ceiling polygons) */
			for (i=0;i<vertex_count;++i)
			{
				flagged_world_point2d *world= vertices + (adjusted_height>0 ? vertex_count-i-1 : i);
				point2d *screen= textured_polygon.vertices + i;
				
				switch (world->flags&(_clip_left|_clip_right))
				{
					case 0:
						// LP change: making it long-distance friendly
						long screen_x= view->half_screen_width + (world->y*view->world_to_screen_x)/world->x;
						screen->x= PIN(screen_x, 0, view->screen_width);
						/*
						screen->x= view->half_screen_width + (world->y*view->world_to_screen_x)/world->x;
						screen->x= PIN(screen->x, 0, view->screen_width);
						*/
						break;
					case _clip_left: screen->x= window->x0; break;
					case _clip_right: screen->x= window->x1; break;
					default:
						// LP change: suppressing
						// if (window->x1-window->x0>1) dprintf("ambiguous clip flags for window [%d,%d];g;", window->x0, window->x1);
						screen->x= window->x0;
						break;
				}
				
				switch (world->flags&(_clip_up|_clip_down))
				{
					case 0:
						// LP change: making it long-distance friendly
						long screen_y= view->half_screen_height - transformed_height/world->x + view->dtanpitch;
						screen->y= PIN(screen_y, 0, view->screen_height);
						/*
						screen->y= view->half_screen_height - transformed_height/world->x + view->dtanpitch;
						screen->y= PIN(screen->y, 0, view->screen_height);
						*/
						break;
					case _clip_up: screen->y= window->y0; break;
					case _clip_down: screen->y= window->y1; break;
					default:
						// LP change: suppressing
						// if (window->y1-window->y0>1) dprintf("ambiguous clip flags for window [%d,%d];g;", window->y0, window->y1);
						screen->y= window->y0;
						break;
				}
//				vassert(screen->y>=0&&screen->y<=view->screen_height, csprintf(temporary, "horizontal: flags==%x, window @ %p", world->flags, window));
			}
			
			/* setup the other parameters of the textured polygon */
			textured_polygon.flags= 0;
			textured_polygon.origin.x= view->origin.x + surface->origin.x;
			textured_polygon.origin.y= view->origin.y + surface->origin.y;
			textured_polygon.origin.z= adjusted_height;
			// LP change:
			get_shape_bitmap_and_shading_table(Texture, &textured_polygon.texture, &textured_polygon.shading_tables, view->shading_mode);
			textured_polygon.ShapeDesc = Texture;
			// get_shape_bitmap_and_shading_table(surface->texture, &textured_polygon.texture, &textured_polygon.shading_tables, view->shading_mode);
			textured_polygon.ambient_shade= get_light_intensity(surface->lightsource_index);
			textured_polygon.vertex_count= vertex_count;
			instantiate_polygon_transfer_mode(view, &textured_polygon, surface->transfer_mode, view->tick_count + 16*(polygon-map_polygons), TRUE);
			if (view->shading_mode==_shading_infravision) textured_polygon.flags|= _SHADELESS_BIT;
			
			/* and, finally, map it */
			// LP: added OpenGL support; also presence of void on other side
			textured_polygon.VoidPresent = void_present;
			if (!OGL_RenderWall(textured_polygon,false))
				texture_horizontal_polygon(&textured_polygon, destination, view);
		}
	}

	return;
}

/* ---------- rendering sides (walls) */

// LP change: added "void present on other side" flag
static void render_node_side(
	struct view_data *view,
	struct bitmap_definition *destination,
	struct clipping_window_data *window,
	struct vertical_surface_data *surface,
	bool void_present)
	// struct vertical_surface_data *surface)
{
	world_distance h= MIN(surface->h1, surface->hmax);
	
	// LP addition: animated-texture support
	// Extra variable defined so as not to edit the original texture
	shape_descriptor Texture = AnimTxtr_Translate(surface->texture_definition->texture);
	if (h>surface->h0 && Texture!=NONE)
	// if (h>surface->h0 && surface->texture_definition->texture!=NONE)
	{
		struct polygon_definition textured_polygon;
		flagged_world_point2d posts[2];
		flagged_world_point3d vertices[MAXIMUM_VERTICES_PER_WORLD_POLYGON];
		short vertex_count;
		short i;
	
		/* initialize the two posts of our trapezoid */
		vertex_count= 2;
		// LP change:
		posts[0].x= surface->p0.i, posts[0].y= surface->p0.j, posts[0].flags= 0;
		posts[1].x= surface->p1.i, posts[1].y= surface->p1.j, posts[1].flags= 0;
		/*
		posts[0].x= surface->p0.x, posts[0].y= surface->p0.y, posts[0].flags= 0;
		posts[1].x= surface->p1.x, posts[1].y= surface->p1.y, posts[1].flags= 0;
		*/
	
		/* clip to left and right sides of the cone (must happen in the same order as horizontal polygons) */
		vertex_count= xy_clip_line(posts, vertex_count, &window->left, _clip_left);
		vertex_count= xy_clip_line(posts, vertex_count, &window->right, _clip_right);
		
		if (vertex_count)
		{
			/* build a polygon out of the two posts */
			vertex_count= 4;
			vertices[0].z= vertices[1].z= h;
			vertices[2].z= vertices[3].z= surface->h0;
			vertices[0].x= vertices[3].x= posts[0].x, vertices[0].y= vertices[3].y= posts[0].y;
			vertices[1].x= vertices[2].x= posts[1].x, vertices[1].y= vertices[2].y= posts[1].y;
			vertices[0].flags= vertices[3].flags= posts[0].flags;
			vertices[1].flags= vertices[2].flags= posts[1].flags;
		
			/* clip to top and bottom sides of the window; because xz_clip_vertical_polygon accepts
				vertices in clockwise or counterclockwise order, we can set our vertex list up to be
				clockwise on the screen and never worry about it after that */
			vertex_count= xz_clip_vertical_polygon(vertices, vertex_count, &window->top, _clip_up);
			vertex_count= xz_clip_vertical_polygon(vertices, vertex_count, &window->bottom, _clip_down);
			
			if (vertex_count)
			{
				// LP change:
				world_distance dx= surface->p1.i - surface->p0.i;
				world_distance dy= surface->p1.j - surface->p0.j;
				/*
				world_distance dx= surface->p1.x - surface->p0.x;
				world_distance dy= surface->p1.y - surface->p0.y;
				*/
				world_distance x0= WORLD_FRACTIONAL_PART(surface->texture_definition->x0);
				world_distance y0= WORLD_FRACTIONAL_PART(surface->texture_definition->y0);
				
				/* calculate texture origin and direction */	
				textured_polygon.vector.i= (WORLD_ONE*dx)/surface->length;
				textured_polygon.vector.j= (WORLD_ONE*dy)/surface->length;
				textured_polygon.vector.k= -WORLD_ONE;
				// LP change:
				textured_polygon.origin.x= surface->p0.i - (x0*dx)/surface->length;
				textured_polygon.origin.y= surface->p0.j - (x0*dy)/surface->length;
				/*
				textured_polygon.origin.x= surface->p0.x - (x0*dx)/surface->length;
				textured_polygon.origin.y= surface->p0.y - (x0*dy)/surface->length;
				*/
				textured_polygon.origin.z= surface->h1 + y0;
	
				/* transform the points we have into screen-space */
				for (i=0;i<vertex_count;++i)
				{
					flagged_world_point3d *world= vertices + i;
					// LP change:
					point2d *screen= textured_polygon.vertices + i;
					
					switch (world->flags&(_clip_left|_clip_right))
					{
						case 0:
							// LP change: making it long-distance friendly
							long screen_x= view->half_screen_width + (world->y*view->world_to_screen_x)/world->x;
							screen->x= PIN(screen_x, 0, view->screen_width);
							/*
							screen->x= view->half_screen_width + (world->y*view->world_to_screen_x)/world->x;
							screen->x= PIN(screen->x, 0, view->screen_width);
							*/
							break;
						case _clip_left: screen->x= window->x0; break;
						case _clip_right: screen->x= window->x1; break;
						default:
							// LP change: suppressing
							// if (window->x1-window->x0>1) dprintf("ambiguous clip flags for window [%d,%d];g;", window->x0, window->x1);
							screen->x= window->x0;
							break;
					}
					
					switch (world->flags&(_clip_up|_clip_down))
					{
						case 0:
							// LP change: making it long-distance friendly
							long screen_y= view->half_screen_height - (world->z*view->world_to_screen_y)/world->x + view->dtanpitch;
							screen->y= PIN(screen_y, 0, view->screen_height);
							/*
							screen->y= view->half_screen_height - (world->z*view->world_to_screen_y)/world->x + view->dtanpitch;
							screen->y= PIN(screen->y, 0, view->screen_height);
							*/
							break;
						case _clip_up: screen->y= window->y0; break;
						case _clip_down: screen->y= window->y1; break;
						default:
							// LP change: suppressing
							// if (window->y1-window->y0>1) dprintf("ambiguous clip flags for window [%d,%d];g;", window->y0, window->y1);
							screen->y= window->y0;
							break;
					}
//					vassert(screen->y>=0&&screen->y<=view->screen_height, csprintf(temporary, "#%d!in[#0,#%d]: flags==%x, wind@%p #%d w@%p s@%p", screen->y, view->screen_height, world->flags, window, vertex_count, world, screen));
				}
				
				/* setup the other parameters of the textured polygon */
				textured_polygon.flags= 0;
				// LP change:
				get_shape_bitmap_and_shading_table(Texture, &textured_polygon.texture, &textured_polygon.shading_tables, view->shading_mode);
				textured_polygon.ShapeDesc = Texture;
				// get_shape_bitmap_and_shading_table(surface->texture_definition->texture, &textured_polygon.texture, &textured_polygon.shading_tables, view->shading_mode);
				textured_polygon.ambient_shade= get_light_intensity(surface->lightsource_index) + surface->ambient_delta;
				textured_polygon.ambient_shade= PIN(textured_polygon.ambient_shade, 0, FIXED_ONE);
				textured_polygon.vertex_count= vertex_count;
				instantiate_polygon_transfer_mode(view, &textured_polygon, surface->transfer_mode, view->tick_count + (long)window, FALSE);
				if (view->shading_mode==_shading_infravision) textured_polygon.flags|= _SHADELESS_BIT;
				
				/* and, finally, map it */
				// LP: added OpenGL support; also presence of void on other side
				textured_polygon.VoidPresent = void_present;
				if (!OGL_RenderWall(textured_polygon,true))
					texture_vertical_polygon(&textured_polygon, destination, view);
			}
		}
	}

	return;
}

/* ---------- rendering objects */

static void render_node_object(
	struct view_data *view,
	struct bitmap_definition *destination,
	struct render_object_data *object,
	bool other_side_of_media)
	// struct render_object_data *object)
{
	struct clipping_window_data *window;
	
	for (window= object->clipping_windows; window; window= window->next_window)
	{
		object->rectangle.clip_left= window->x0;
		object->rectangle.clip_right= window->x1;
		object->rectangle.clip_top= window->y0;
		object->rectangle.clip_bottom= window->y1;
		
		// This is bitwise XOR, but is presumably OK here
		if (view->under_media_boundary ^ other_side_of_media)
		// if (view->under_media_boundary)
		{
			// Clipping: below a liquid surface
			object->rectangle.clip_top= MAX(object->rectangle.clip_top, object->ymedia);
		}
		else
		{
			// Clipping: above a liquid surface
			object->rectangle.clip_bottom= MIN(object->rectangle.clip_bottom, object->ymedia);
		}
		
		// LP: added OpenGL support
		if (!OGL_RenderSprite(object->rectangle))
			texture_rectangle(&object->rectangle, destination, view);
	}
	
	return;
}

/* ---------- horizontal polygon clipping */

enum /* xy_clip_horizontal_polygon() states */
{
	_testing_first_vertex, /* are we in or out? */
	_searching_cw_for_in_out_transition,
	_searching_ccw_for_out_in_transition,
	_searching_cw_for_out_in_transition
};

// LP change: make it better able to do long-distance views
static short xy_clip_horizontal_polygon(
	flagged_world_point2d *vertices,
	short vertex_count,
	long_vector2d *line, // world_vector2d *line,
	word flag)
{
#ifdef QUICKDRAW_DEBUG
	debug_flagged_points(vertices, vertex_count);
	debug_vector(line);
#endif
//	dprintf("clipping %p (#%d vertices) to vector %x,%x (slope==%x)", vertices, vertex_count, line->i, line->j, slope);
	
	if (vertex_count)
	{
		short state= _testing_first_vertex;
		short vertex_index= 0, vertex_delta= 1, first_vertex= 0;
		short entrance_vertex= NONE, exit_vertex= NONE; /* exiting the clipped area and entering the clipped area */
		boolean clipped_exit_vertex= TRUE, clipped_entrance_vertex= TRUE; /* will be FALSE if these points lie directly on a vertex */
		
		do
		{
			// LP change:
			CROSSPROD_TYPE cross_product= CROSSPROD_TYPE(line->i)*vertices[vertex_index].y - CROSSPROD_TYPE(line->j)*vertices[vertex_index].x;
			// long cross_product= line->i*vertices[vertex_index].y - line->j*vertices[vertex_index].x;
			
			switch (SGN(cross_product))
			{
				case -1: /* inside (i.e., will be clipped) */
//					dprintf("vertex#%d is inside s==#%d", vertex_index, state);
					switch (state)
					{
						case _testing_first_vertex:
							first_vertex= vertex_index;
							state= _searching_cw_for_in_out_transition; /* the exit point from the clip area */
							break;
						
						case _searching_ccw_for_out_in_transition: /* found exit point from clip area */
							state= _searching_cw_for_out_in_transition; /* the entrance point to the clipped area */
							vertex_delta= 1;
							if (exit_vertex==NONE) exit_vertex= WRAP_HIGH(vertex_index, vertex_count-1);
							vertex_index= first_vertex; /* skip vertices we already know are out */
							break;
						
						case _searching_cw_for_out_in_transition: /* found entrance point to clipped area */
							if (entrance_vertex==NONE) entrance_vertex= vertex_index;
							state= NONE;
							break;
					}
					break;
				
				case 0: /* clip line passed directly through a vertex */
//					dprintf("vertex#%d is on the clip line s==#%d", vertex_index, state);
					switch (state)
					{
						/* if we’re testing the first vertex, this tells us nothing */
						
						case _searching_cw_for_out_in_transition:
							entrance_vertex= vertex_index;
							clipped_entrance_vertex= FALSE; /* remember if this passes through vertex */
							break;
						
						case _searching_cw_for_in_out_transition:
							exit_vertex= WRAP_HIGH(vertex_index, vertex_count-1);
							clipped_exit_vertex= FALSE;
							break;
						case _searching_ccw_for_out_in_transition:
							exit_vertex= WRAP_HIGH(vertex_index, vertex_count-1);
							clipped_exit_vertex= FALSE; /* remember if this passes through vertex */
							break;
					}
					break;
				
				case 1: /* outside (i.e., will not be clipped) */
//					dprintf("vertex#%d is outside s==#%d", vertex_index, state);
					switch (state)
					{
						case _testing_first_vertex:
							first_vertex= vertex_index;
							state= _searching_ccw_for_out_in_transition; /* the exit point from the clipped area */
							vertex_delta= -1;
							break;
						
						case _searching_cw_for_in_out_transition: /* found exit point from clipped area */
							state= _searching_cw_for_out_in_transition; /* the entrance point to the clipped area */
							if (exit_vertex==NONE) exit_vertex= vertex_index;
							break;
					}
					break;
			}

			/* adjust vertex_index (clockwise or counterclockwise, depending on vertex_delta)
				if we’ve come back to the first vertex without finding an entrance point we’re
				either all the way in or all the way out */
			vertex_index= (vertex_delta<0) ? WRAP_LOW(vertex_index, vertex_count-1) :
				WRAP_HIGH(vertex_index, vertex_count-1);
			if (vertex_index==first_vertex) /* we came full-circle without hitting anything */
			{
				switch (state)
				{
					case _searching_cw_for_in_out_transition: /* never found a way out: clipped into nothing */
						vertex_count= 0;
					case _testing_first_vertex: /* is this the right thing to do? */
					case _searching_ccw_for_out_in_transition: /* never found a way in: no clipping */
						exit_vertex= NONE;
						state= NONE;
						break;
				}
			}
		}
		while (state!=NONE);
		
		if (exit_vertex!=NONE) /* we’ve got clipping to do */
		{
			flagged_world_point2d new_entrance_point, new_exit_point;
			
//			dprintf("entrance_vertex==#%d (%s), exit_vertex==#%d (%s)", entrance_vertex, clipped_entrance_vertex ? "clipped" : "unclipped", exit_vertex, clipped_exit_vertex ? "clipped" : "unclipped");
			
			/* clip the entrance to the clipped area */
			if (clipped_entrance_vertex)
			{
				xy_clip_flagged_world_points(vertices + WRAP_LOW(entrance_vertex, vertex_count-1), vertices + entrance_vertex,
					&new_entrance_point, line);
			}
			else
			{
				new_entrance_point= vertices[entrance_vertex];
			}
			new_entrance_point.flags|= flag;
			
			/* clip the exit from the clipped area */
			if (clipped_exit_vertex)
			{
				xy_clip_flagged_world_points(vertices + WRAP_LOW(exit_vertex, vertex_count-1), vertices + exit_vertex,
					&new_exit_point, line);
			}
			else
			{
				new_exit_point= vertices[WRAP_LOW(exit_vertex, vertex_count-1)];
			}
			new_exit_point.flags|= flag;
			
			/* adjust for the change in number of vertices */
			vertex_delta= entrance_vertex - exit_vertex;
			if (vertex_delta<0)
			{
				if (vertex_delta!=-2) memmove(vertices+entrance_vertex+2, vertices+exit_vertex, (vertex_count-exit_vertex)*sizeof(flagged_world_point2d));
				vertex_delta= vertex_count+vertex_delta;
			}
			else
			{
				/* move down by exit_vertex, add new vertices to end */
				assert(vertex_delta);
				if (exit_vertex)
				{
					memmove(vertices, vertices+exit_vertex, vertex_delta*sizeof(flagged_world_point2d));
					entrance_vertex-= exit_vertex;
				}
			}
			
			vertex_count= vertex_delta+2;
			vwarn(vertex_count>=3 && vertex_count<=MAXIMUM_VERTICES_PER_WORLD_POLYGON,
				csprintf(temporary, "vertex overflow or underflow (#%d);g;", vertex_count));
				
			if (vertex_count<3 || vertex_count>MAXIMUM_VERTICES_PER_WORLD_POLYGON)
			{
				vertex_count= 0;
			}
			else
			{
				/* and, finally, add the new vertices */
				vertices[entrance_vertex]= new_entrance_point;
				vertices[entrance_vertex+1]= new_exit_point;
			}
		}
	}

#ifdef QUICKDRAW_DEBUG
	debug_flagged_points(vertices, vertex_count);
	debug_vector(line);
#endif
//	dprintf("result == %p (#%d vertices)", vertices, vertex_count);

	return vertex_count;
}

/* sort points before clipping to assure consistency; there is a way to make this more accurate
	but it requires the downshifting game, as played in SCOTTISH_TEXTURES.C.  it’s tempting to
	think that having a smaller scale for our world coordinates would help here (i.e., less bits
	per distance) but then wouldn’t we be screwed when we tried to rotate? */
// LP change: make it better able to do long-distance views
static void xy_clip_flagged_world_points(
	flagged_world_point2d *p0,
	flagged_world_point2d *p1,
	flagged_world_point2d *clipped,
	long_vector2d *line) // world_vector2d *line)
{
	boolean swap= (p1->y>p0->y) ? FALSE : ((p0->y==p1->y) ? (p1->x<p0->x) : TRUE);
	flagged_world_point2d *local_p0= swap ? p1 : p0;
	flagged_world_point2d *local_p1= swap ? p0 : p1;
	world_distance dx= local_p1->x - local_p0->x;
	world_distance dy= local_p1->y - local_p0->y;
	long numerator= line->j*local_p0->x - line->i*local_p0->y;
	long denominator= line->i*dy - line->j*dx;
	short shift_count= FIXED_FRACTIONAL_BITS;
	fixed t;

	/* give numerator 16 significant bits over denominator and then calculate t==n/d;  MPW’s PPCC
		didn’t seem to like (LONG_MIN>>1) and i had to substitute 0xc0000000 instead (hmmm) */
	while (numerator<=(long)0x3fffffff && numerator>=(long)0xc0000000 && shift_count--) numerator<<= 1;
	if (shift_count>0) denominator>>= shift_count;
	t= numerator/denominator;

	/* calculate the clipped point */
	clipped->x= local_p0->x + FIXED_INTEGERAL_PART(t*dx);
	clipped->y= local_p0->y + FIXED_INTEGERAL_PART(t*dy);
	clipped->flags= local_p0->flags&local_p1->flags;

	return;
}

/* almost wholly identical to xz_clip_vertical_polygon() except that this works off 2d points
	in the xy-plane and a height */
// LP change: make it better able to do long-distance views
static short z_clip_horizontal_polygon(
	flagged_world_point2d *vertices,
	short vertex_count,
	long_vector2d *line, // world_vector2d *line,/* i==x, j==z */
	world_distance height,
	word flag)
{
	// LP change:
	CROSSPROD_TYPE heighti= CROSSPROD_TYPE(line->i)*height;
	// long heighti= line->i*height;
	
#ifdef QUICKDRAW_DEBUG
	debug_flagged_points(vertices, vertex_count);
	debug_x_line(line->j ? (line->i*height)/line->j : (height<0 ? LONG_MIN : LONG_MAX));
#endif
//	dprintf("clipping %p (#%d vertices) to vector %x,%x", vertices, vertex_count, line->i, line->j);
	
	if (vertex_count)
	{
		short state= _testing_first_vertex;
		short vertex_index= 0, vertex_delta= 1, first_vertex= 0;
		short entrance_vertex= NONE, exit_vertex= NONE; /* exiting the clipped area and entering the clipped area */
		boolean clipped_exit_vertex= TRUE, clipped_entrance_vertex= TRUE; /* will be FALSE if these points lie directly on a vertex */
		
		do
		{
			// LP change:
			CROSSPROD_TYPE cross_product= heighti - CROSSPROD_TYPE(line->j)*vertices[vertex_index].x;
			// long cross_product= heighti - line->j*vertices[vertex_index].x;

			if (cross_product<0) /* inside (i.e., will be clipped) */
			{
				switch (state)
				{
					case _testing_first_vertex:
						first_vertex= vertex_index;
						state= _searching_cw_for_in_out_transition; /* the exit point from the clip area */
						break;
					
					case _searching_ccw_for_out_in_transition: /* found exit point from clip area */
						state= _searching_cw_for_out_in_transition; /* the entrance point to the clipped area */
						vertex_delta= 1;
						if (exit_vertex==NONE) exit_vertex= WRAP_HIGH(vertex_index, vertex_count-1);
						vertex_index= first_vertex; /* skip vertices we already know are out */
						break;
					
					case _searching_cw_for_out_in_transition: /* found entrance point to clipped area */
						if (entrance_vertex==NONE) entrance_vertex= vertex_index;
						state= NONE;
						break;
				}
			}
			else
			{
				if (cross_product>0) /* outside (i.e., will not be clipped) */
				{
					switch (state)
					{
						case _testing_first_vertex:
							first_vertex= vertex_index;
							state= _searching_ccw_for_out_in_transition; /* the exit point from the clipped area */
							vertex_delta= -1;
							break;
						
						case _searching_cw_for_in_out_transition: /* found exit point from clipped area */
							state= _searching_cw_for_out_in_transition; /* the entrance point to the clipped area */
							if (exit_vertex==NONE) exit_vertex= vertex_index;
							break;
					}
				}
				else /* clip line passed directly through a vertex */
				{
					switch (state)
					{
						/* if we’re testing the first vertex (_testing_first_vertex), this tells us nothing */
						
						case _searching_cw_for_out_in_transition:
							entrance_vertex= vertex_index;
							clipped_entrance_vertex= FALSE; /* remember if this passes through vertex */
							break;
						
						case _searching_cw_for_in_out_transition:
							exit_vertex= WRAP_HIGH(vertex_index, vertex_count-1);
							clipped_exit_vertex= FALSE;
							break;
						case _searching_ccw_for_out_in_transition:
							exit_vertex= WRAP_HIGH(vertex_index, vertex_count-1);
							clipped_exit_vertex= FALSE; /* remember if this passes through vertex */
							break;
					}
				}
			}

			/* adjust vertex_index (clockwise or counterclockwise, depending on vertex_delta)
				if we’ve come back to the first vertex without finding an entrance point we’re
				either all the way in or all the way out */
			vertex_index= (vertex_delta<0) ? WRAP_LOW(vertex_index, vertex_count-1) :
				WRAP_HIGH(vertex_index, vertex_count-1);
			if (vertex_index==first_vertex) /* we came full-circle without hitting anything */
			{
				switch (state)
				{
					case _searching_cw_for_in_out_transition: /* never found a way out: clipped into nothing */
						vertex_count= 0;
					case _testing_first_vertex: /* is this the right thing to do? */
					case _searching_ccw_for_out_in_transition: /* never found a way in: no clipping */
						exit_vertex= NONE;
						state= NONE;
						break;
				}
			}
		}
		while (state!=NONE);
		
		if (exit_vertex!=NONE) /* we’ve got clipping to do */
		{
			flagged_world_point2d new_entrance_point, new_exit_point;
			
//			dprintf("entrance_vertex==#%d (%s), exit_vertex==#%d (%s)", entrance_vertex, clipped_entrance_vertex ? "clipped" : "unclipped", exit_vertex, clipped_exit_vertex ? "clipped" : "unclipped");
			
			/* clip the entrance to the clipped area */
			if (clipped_entrance_vertex)
			{
				z_clip_flagged_world_points(vertices + WRAP_LOW(entrance_vertex, vertex_count-1), vertices + entrance_vertex,
					height, &new_entrance_point, line);
			}
			else
			{
				new_entrance_point= vertices[entrance_vertex];
			}
			new_entrance_point.flags|= flag;
			
			/* clip the exit from the clipped area */
			if (clipped_exit_vertex)
			{
				z_clip_flagged_world_points(vertices + WRAP_LOW(exit_vertex, vertex_count-1), vertices + exit_vertex,
					height, &new_exit_point, line);
			}
			else
			{
				new_exit_point= vertices[WRAP_LOW(exit_vertex, vertex_count-1)];
			}
			new_exit_point.flags|= flag;
			
			/* adjust for the change in number of vertices */
			vertex_delta= entrance_vertex - exit_vertex;
			if (vertex_delta<0)
			{
				if (vertex_delta!=-2) memmove(vertices+entrance_vertex+2, vertices+exit_vertex, (vertex_count-exit_vertex)*sizeof(flagged_world_point2d));
				vertex_delta= vertex_count+vertex_delta;
			}
			else
			{
				/* move down by exit_vertex, add new vertices to end */
				assert(vertex_delta);
				if (exit_vertex)
				{
					memmove(vertices, vertices+exit_vertex, vertex_delta*sizeof(flagged_world_point2d));
					entrance_vertex-= exit_vertex;
				}
			}
			vertex_count= vertex_delta+2;

			vwarn(vertex_count>=3 && vertex_count<=MAXIMUM_VERTICES_PER_WORLD_POLYGON,
				csprintf(temporary, "vertex overflow or underflow (#%d);g;", vertex_count));
				
			if (vertex_count<3 || vertex_count>MAXIMUM_VERTICES_PER_WORLD_POLYGON)
			{
				vertex_count= 0;
			}
			else
			{
				/* and, finally, add the new vertices */
				vertices[entrance_vertex]= new_entrance_point;
				vertices[entrance_vertex+1]= new_exit_point;
			}
		}
	}

#ifdef QUICKDRAW_DEBUG
	debug_flagged_points(vertices, vertex_count);
	debug_x_line(line->j ? (line->i*height)/line->j : (height<0 ? LONG_MIN : LONG_MAX));
#endif
//	dprintf("result == %p (#%d vertices)", vertices, vertex_count);

	return vertex_count;
}

/* sort points before clipping to assure consistency; this is almost identical to xz_clip…()
	except that it clips 2d points in the xy-plane at the given height. */
// LP change: make it better able to do long-distance views
static void z_clip_flagged_world_points(
	flagged_world_point2d *p0,
	flagged_world_point2d *p1,
	world_distance height,
	flagged_world_point2d *clipped,
	long_vector2d *line) // world_vector2d *line)
{
	boolean swap= (p1->y>p0->y) ? FALSE : ((p0->y==p1->y) ? (p1->x<p0->x) : TRUE);
	flagged_world_point2d *local_p0= swap ? p1 : p0;
	flagged_world_point2d *local_p1= swap ? p0 : p1;
	world_distance dx= local_p1->x - local_p0->x;
	world_distance dy= local_p1->y - local_p0->y;
	long numerator= line->j*local_p0->x - line->i*height;
	long denominator= - line->j*dx;
	short shift_count= FIXED_FRACTIONAL_BITS;
	fixed t;

	/* give numerator 16 significant bits over denominator and then calculate t==n/d;  MPW’s PPCC
		didn’t seem to like (LONG_MIN>>1) and i had to substitute 0xc0000000 instead (hmmm) */
	while (numerator<=(long)0x3fffffff && numerator>=(long)0xc0000000 && shift_count--) numerator<<= 1;
	if (shift_count>0) denominator>>= shift_count;
	t= numerator/denominator;

	/* calculate the clipped point */
	clipped->x= local_p0->x + FIXED_INTEGERAL_PART(t*dx);
	clipped->y= local_p0->y + FIXED_INTEGERAL_PART(t*dy);
	clipped->flags= local_p0->flags&local_p1->flags;

	return;
}

/* ---------- vertical polygon clipping */

// LP change: make it better able to do long-distance views
static short xy_clip_line(
	flagged_world_point2d *posts,
	short vertex_count,
	long_vector2d *line, // world_vector2d *line,
	word flag)
{
#ifdef QUICKDRAW_DEBUG
//	debug_flagged_points(posts, vertex_count);
//	debug_vector(line);
#endif
//	dprintf("clipping %p (#%d) to line (%d,%d)", posts, vertex_count, line->i, line->j);
	
	if (vertex_count)
	{
		// LP change:
		CROSSPROD_TYPE cross_product0= CROSSPROD_TYPE(line->i)*posts[0].y - CROSSPROD_TYPE(line->j)*posts[0].x;
		CROSSPROD_TYPE cross_product1= CROSSPROD_TYPE(line->i)*posts[1].y - CROSSPROD_TYPE(line->j)*posts[1].x;
		/*
		long cross_product0= line->i*posts[0].y - line->j*posts[0].x;
		long cross_product1= line->i*posts[1].y - line->j*posts[1].x;
		*/
		
		if (cross_product0<0)
		{
			if (cross_product1<0) /* clipped out of existence */
			{
				vertex_count= 0;
			}
			else
			{
				xy_clip_flagged_world_points(&posts[0], &posts[1], &posts[0], line);
				posts[0].flags|= flag;
			}
		}
		else
		{
			if (cross_product1<0)
			{
				xy_clip_flagged_world_points(&posts[0], &posts[1], &posts[1], line);
				posts[1].flags|= flag;
			}
		}
	}

#ifdef QUICKDRAW_DEBUG
//	debug_flagged_points(posts, vertex_count);
//	debug_vector(line);
#endif
//	dprintf("result #%d vertices", vertex_count);
	
	return vertex_count;
}

// LP change: make it better able to do long-distance views
static short xz_clip_vertical_polygon(
	flagged_world_point3d *vertices,
	short vertex_count,
	long_vector2d *line, // world_vector2d *line, /* i==x, j==z */
	word flag)
{
#ifdef QUICKDRAW_DEBUG
//	debug_flagged_points3d(vertices, vertex_count);
//	debug_vector(line);
#endif
//	dprintf("clipping %p (#%d vertices) to vector %x,%x", vertices, vertex_count, line->i, line->j);
	
	if (vertex_count)
	{
		short state= _testing_first_vertex;
		short vertex_index= 0, vertex_delta= 1, first_vertex= 0;
		short entrance_vertex= NONE, exit_vertex= NONE; /* exiting the clipped area and entering the clipped area */
		boolean clipped_exit_vertex= TRUE, clipped_entrance_vertex= TRUE; /* will be FALSE if these points lie directly on a vertex */
		
		do
		{	
			// LP change:
			CROSSPROD_TYPE cross_product= CROSSPROD_TYPE(line->i)*vertices[vertex_index].z - CROSSPROD_TYPE(line->j)*vertices[vertex_index].x;
			// long cross_product= line->i*vertices[vertex_index].z - line->j*vertices[vertex_index].x;
			
			switch (SGN(cross_product))
			{
				case -1: /* inside (i.e., will be clipped) */
//					dprintf("vertex#%d is inside s==#%d", vertex_index, state);
					switch (state)
					{
						case _testing_first_vertex:
							first_vertex= vertex_index;
							state= _searching_cw_for_in_out_transition; /* the exit point from the clip area */
							break;
						
						case _searching_ccw_for_out_in_transition: /* found exit point from clip area */
							state= _searching_cw_for_out_in_transition; /* the entrance point to the clipped area */
							vertex_delta= 1;
							if (exit_vertex==NONE) exit_vertex= WRAP_HIGH(vertex_index, vertex_count-1);
							vertex_index= first_vertex; /* skip vertices we already know are out */
							break;
						
						case _searching_cw_for_out_in_transition: /* found entrance point to clipped area */
							if (entrance_vertex==NONE) entrance_vertex= vertex_index;
							state= NONE;
							break;
					}
					break;
				
				case 0: /* clip line passed directly through a vertex */
//					dprintf("vertex#%d is on the clip line s==#%d", vertex_index, state);
					switch (state)
					{
						/* if we’re testing the first vertex, this tells us nothing */
						
						case _searching_cw_for_out_in_transition:
							entrance_vertex= vertex_index;
							clipped_entrance_vertex= FALSE; /* remember if this passes through vertex */
							break;
						
						case _searching_cw_for_in_out_transition:
							exit_vertex= WRAP_HIGH(vertex_index, vertex_count-1);
							clipped_exit_vertex= FALSE;
							break;
						case _searching_ccw_for_out_in_transition:
							exit_vertex= WRAP_HIGH(vertex_index, vertex_count-1);
							clipped_exit_vertex= FALSE; /* remember if this passes through vertex */
							break;
					}
					break;
				
				case 1: /* outside (i.e., will not be clipped) */
//					dprintf("vertex#%d is outside s==#%d", vertex_index, state);
					switch (state)
					{
						case _testing_first_vertex:
							first_vertex= vertex_index;
							state= _searching_ccw_for_out_in_transition; /* the exit point from the clipped area */
							vertex_delta= -1;
							break;
						
						case _searching_cw_for_in_out_transition: /* found exit point from clipped area */
							state= _searching_cw_for_out_in_transition; /* the entrance point to the clipped area */
							if (exit_vertex==NONE) exit_vertex= vertex_index;
							break;
					}
					break;
			}

			/* adjust vertex_index (clockwise or counterclockwise, depending on vertex_delta)
				if we’ve come back to the first vertex without finding an entrance point we’re
				either all the way in or all the way out */
			vertex_index= (vertex_delta<0) ? WRAP_LOW(vertex_index, vertex_count-1) :
				WRAP_HIGH(vertex_index, vertex_count-1);
			if (vertex_index==first_vertex) /* we came full-circle without hitting anything */
			{
				switch (state)
				{
					case _searching_cw_for_in_out_transition: /* never found a way out: clipped into nothing */
						vertex_count= 0;
					case _testing_first_vertex: /* is this the right thing to do? */
					case _searching_ccw_for_out_in_transition: /* never found a way in: no clipping */
						exit_vertex= NONE;
						state= NONE;
						break;
				}
			}
		}
		while (state!=NONE);
		
		if (exit_vertex!=NONE) /* we’ve got clipping to do */
		{
			flagged_world_point3d new_entrance_point, new_exit_point;
			
//			dprintf("entrance_vertex==#%d (%s), exit_vertex==#%d (%s)", entrance_vertex, clipped_entrance_vertex ? "clipped" : "unclipped", exit_vertex, clipped_exit_vertex ? "clipped" : "unclipped");
			
			/* clip the entrance to the clipped area */
			if (clipped_entrance_vertex)
			{
				xz_clip_flagged_world_points(vertices + WRAP_LOW(entrance_vertex, vertex_count-1), vertices + entrance_vertex,
					&new_entrance_point, line);
			}
			else
			{
				new_entrance_point= vertices[entrance_vertex];
			}
			new_entrance_point.flags|= flag;
			
			/* clip the exit from the clipped area */
			if (clipped_exit_vertex)
			{
				xz_clip_flagged_world_points(vertices + WRAP_LOW(exit_vertex, vertex_count-1), vertices + exit_vertex,
					&new_exit_point, line);
			}
			else
			{
				new_exit_point= vertices[WRAP_LOW(exit_vertex, vertex_count-1)];
			}
			new_exit_point.flags|= flag;
			
			/* adjust for the change in number of vertices */
			vertex_delta= entrance_vertex - exit_vertex;
			if (vertex_delta<0)
			{
				if (vertex_delta!=-2) memmove(vertices+entrance_vertex+2, vertices+exit_vertex, (vertex_count-exit_vertex)*sizeof(flagged_world_point3d));
				vertex_delta= vertex_count+vertex_delta;
			}
			else
			{
				/* move down by exit_vertex, add new vertices to end */
				assert(vertex_delta);
				if (exit_vertex)
				{
					memmove(vertices, vertices+exit_vertex, vertex_delta*sizeof(flagged_world_point3d));
					entrance_vertex-= exit_vertex;
				}
			}
			vertex_count= vertex_delta+2;

			vwarn(vertex_count>=3 && vertex_count<=MAXIMUM_VERTICES_PER_WORLD_POLYGON,
				csprintf(temporary, "vertex overflow or underflow (#%d);g;", vertex_count));
				
			if (vertex_count<3 || vertex_count>MAXIMUM_VERTICES_PER_WORLD_POLYGON)
			{
				vertex_count= 0;
			}
			else
			{
				/* and, finally, add the new vertices */
				vertices[entrance_vertex]= new_entrance_point;
				vertices[entrance_vertex+1]= new_exit_point;
			}
		}
	}

#ifdef QUICKDRAW_DEBUG
//	debug_flagged_points3d(vertices, vertex_count);
//	debug_vector(line);
#endif
//	dprintf("result == %p (#%d vertices)", vertices, vertex_count);

	return vertex_count;
}

/* sort points before clipping to assure consistency */
// LP change: make it better able to do long-distance views
static void xz_clip_flagged_world_points(
	flagged_world_point3d *p0,
	flagged_world_point3d *p1,
	flagged_world_point3d *clipped,
	long_vector2d *line) // world_vector2d *line)
{
	boolean swap= (p1->y>p0->y) ? FALSE : ((p0->y==p1->y) ? (p1->x<p0->x) : TRUE);
	flagged_world_point3d *local_p0= swap ? p1 : p0;
	flagged_world_point3d *local_p1= swap ? p0 : p1;
	world_distance dx= local_p1->x - local_p0->x;
	world_distance dy= local_p1->y - local_p0->y;
	world_distance dz= local_p1->z - local_p0->z;
	long numerator= line->j*local_p0->x - line->i*local_p0->z;
	long denominator= line->i*dz - line->j*dx;
	short shift_count= FIXED_FRACTIONAL_BITS;
	fixed t;

	/* give numerator 16 significant bits over denominator and then calculate t==n/d;  MPW’s PPCC
		didn’t seem to like (LONG_MIN>>1) and i had to substitute 0xc0000000 instead (hmmm) */
	while (numerator<=(long)0x3fffffff && numerator>=(long)0xc0000000 && shift_count--) numerator<<= 1;
	if (shift_count>0) denominator>>= shift_count;
	t= numerator/denominator;

	/* calculate the clipped point */
	clipped->x= local_p0->x + FIXED_INTEGERAL_PART(t*dx);
	clipped->y= local_p0->y + FIXED_INTEGERAL_PART(t*dy);
	clipped->z= local_p0->z + FIXED_INTEGERAL_PART(t*dz);
	clipped->flags= local_p0->flags&local_p1->flags;

	return;
}

/* ---------- transfer modes */

/* given a transfer mode and phase, cause whatever changes it should cause to a rectangle_definition
	structure */
static void instantiate_rectangle_transfer_mode(
	struct view_data *view,
	struct rectangle_definition *rectangle,
	short transfer_mode,
	fixed transfer_phase)
{
	switch (transfer_mode)
	{
		case _xfer_invisibility:
		case _xfer_subtle_invisibility:
			if (view->shading_mode!=_shading_infravision)
			{
				rectangle->transfer_mode= _tinted_transfer;
				rectangle->shading_tables= get_global_shading_table();
				rectangle->transfer_data= (transfer_mode==_xfer_invisibility) ? 0x000f : 0x0018;
				break;
			}
			/* if we have infravision, fall through to _textured_transfer (i see you...) */
		case _xfer_normal:
			rectangle->transfer_mode= _textured_transfer;
			break;
		
		case _xfer_static:
		case _xfer_50percent_static:
			rectangle->transfer_mode= _static_transfer;
			rectangle->transfer_data= (transfer_mode==_xfer_static) ? 0x0000 : 0x8000;
			break;

		case _xfer_fade_out_static:
			rectangle->transfer_mode= _static_transfer;
			rectangle->transfer_data= transfer_phase;
			break;
			
		case _xfer_pulsating_static:
			rectangle->transfer_mode= _static_transfer;
			rectangle->transfer_data= 0x8000+((0x6000*sine_table[FIXED_INTEGERAL_PART(transfer_phase*NUMBER_OF_ANGLES)])>>TRIG_SHIFT);
			break;

		case _xfer_fold_in:
			transfer_phase= FIXED_ONE-transfer_phase; /* do everything backwards */
		case _xfer_fold_out:
			{
				short delta= FIXED_INTEGERAL_PART((((rectangle->x1-rectangle->x0)>>1)-1)*transfer_phase);
				
				rectangle->transfer_mode= _static_transfer;
				rectangle->transfer_data= (transfer_phase>>1);
				rectangle->x0+= delta;
				rectangle->x1-= delta;
			}
			break;

#if 0		
		case _xfer_fade_out_to_black:
			rectangle->shading_tables= get_global_shading_table();
			if (transfer_phase<FIXED_ONE_HALF)
			{
				/* fade to black */
				rectangle->ambient_shade= (rectangle->ambient_shade*(transfer_phase-FIXED_ONE_HALF))>>(FIXED_FRACTIONAL_BITS-1);
				rectangle->transfer_mode= _textured_transfer;
			}
			else
			{
				/* vanish */
				rectangle->transfer_mode= _tinted_transfer;
				rectangle->transfer_data= 0x1f - ((0x1f*(FIXED_ONE_HALF-transfer_phase))>>(FIXED_FRACTIONAL_BITS-1));
			}
			break;
#endif
		
		// LP change: made an unrecognized mode act like normal
		default:
			rectangle->transfer_mode= _textured_transfer;
			// vhalt(csprintf(temporary, "rectangles don’t support render mode #%d", transfer_mode));
	}
	
	return;
}

/* given a transfer mode and phase, cause whatever changes it should cause to a polygon_definition
	structure (unfortunately we need to know whether this is a horizontal or vertical polygon) */
static void instantiate_polygon_transfer_mode(
	struct view_data *view,
	struct polygon_definition *polygon,
	short transfer_mode,
	short transfer_phase,
	boolean horizontal)
{
	world_distance x0, y0;
	world_distance vector_magnitude;
	short alternate_transfer_phase;

	polygon->transfer_mode= _textured_transfer;
	switch (transfer_mode)
	{
		case _xfer_fast_horizontal_slide:
		case _xfer_horizontal_slide:
		case _xfer_vertical_slide:
		case _xfer_fast_vertical_slide:
		case _xfer_wander:
		case _xfer_fast_wander:
			x0= y0= 0;
			transfer_phase= view->tick_count;
			switch (transfer_mode)
			{
				case _xfer_fast_horizontal_slide: transfer_phase<<= 1;
				case _xfer_horizontal_slide: x0= (transfer_phase<<2)&(WORLD_ONE-1); break;
				
				case _xfer_fast_vertical_slide: transfer_phase<<= 1;
				case _xfer_vertical_slide: y0= (transfer_phase<<2)&(WORLD_ONE-1); break;
				
				case _xfer_fast_wander: transfer_phase<<= 1;
				case _xfer_wander:
					alternate_transfer_phase= transfer_phase%(10*FULL_CIRCLE);
					transfer_phase= transfer_phase%(6*FULL_CIRCLE);
					x0= (cosine_table[NORMALIZE_ANGLE(alternate_transfer_phase)] +
						(cosine_table[NORMALIZE_ANGLE(2*alternate_transfer_phase)]>>1) +
						(cosine_table[NORMALIZE_ANGLE(5*alternate_transfer_phase)]>>1))>>(WORLD_FRACTIONAL_BITS-TRIG_SHIFT+2);
					y0= (sine_table[NORMALIZE_ANGLE(transfer_phase)] +
						(sine_table[NORMALIZE_ANGLE(2*transfer_phase)]>>1) +
						(sine_table[NORMALIZE_ANGLE(3*transfer_phase)]>>1))>>(WORLD_FRACTIONAL_BITS-TRIG_SHIFT+2);
					break;
			}
			if (horizontal)
			{
				polygon->origin.x+= x0;
				polygon->origin.y+= y0;
			}
			else
			{
				vector_magnitude= isqrt(polygon->vector.i*polygon->vector.i + polygon->vector.j*polygon->vector.j);
				polygon->origin.x+= (polygon->vector.i*x0)/vector_magnitude;
				polygon->origin.y+= (polygon->vector.j*x0)/vector_magnitude;
				polygon->origin.z-= y0;
			}
			break;
		
		case _xfer_pulsate:
		case _xfer_wobble:
		case _xfer_fast_wobble:
			if (transfer_mode==_xfer_fast_wobble) transfer_phase*= 15;
			transfer_phase&= WORLD_ONE/16-1;
			transfer_phase= (transfer_phase>=WORLD_ONE/32) ? (WORLD_ONE/32+WORLD_ONE/64 - transfer_phase) : (transfer_phase - WORLD_ONE/64);
			if (horizontal)
			{
				polygon->origin.z+= transfer_phase;
			}
			else
			{
				if (transfer_mode==_xfer_pulsate) /* translate .origin perpendicular to .vector */
				{
					world_vector2d offset;
					world_distance vector_magnitude= isqrt(polygon->vector.i*polygon->vector.i + polygon->vector.j*polygon->vector.j);
	
					offset.i= (polygon->vector.j*transfer_phase)/vector_magnitude;
					offset.j= (polygon->vector.i*transfer_phase)/vector_magnitude;
	
					polygon->origin.x+= offset.i;
					polygon->origin.y+= offset.j;
				}
				else /* ==_xfer_wobble, wobble .vector */
				{
					polygon->vector.i+= transfer_phase;
					polygon->vector.j+= transfer_phase;
				}
			}
			break;

		case _xfer_normal:
			break;
		
		case _xfer_smear:
			polygon->transfer_mode= _solid_transfer;
			break;
			
		case _xfer_static:
			polygon->transfer_mode= _static_transfer;
			polygon->transfer_data= 0x0000;
			break;
		
		case _xfer_landscape:
			polygon->transfer_mode= _big_landscaped_transfer;
			break;
//		case _xfer_big_landscape:
//			polygon->transfer_mode= _big_landscaped_transfer;
//			break;
			
		default:
			// LP change: made an unrecognized mode act like normal
			break;
			// vhalt(csprintf(temporary, "polygons don’t support render mode #%d", transfer_mode));
	}

	return;
}

/* ---------- viewer sprite layer (i.e., weapons) */

static void render_viewer_sprite_layer(
	struct view_data *view,
	struct bitmap_definition *destination)
{
	struct rectangle_definition textured_rectangle;
	struct weapon_display_information display_data;
	struct shape_information_data *shape_information;
	short count;
	
	// LP change: bug out if weapons-in-hand are not to be displayed
	if (!view->show_weapons_in_hand) return;

	/* get_weapon_display_information() returns TRUE if there is a weapon to be drawn.  it
		should initially be passed a count of zero.  it returns the weapon’s texture and
		enough information to draw it correctly. */
	count= 0;
	while (get_weapon_display_information(&count, &display_data))
	{
		/* fetch relevant shape data */
		shape_information= extended_get_shape_information(display_data.collection, display_data.low_level_shape_index);
		
		if (shape_information->flags&_X_MIRRORED_BIT) display_data.flip_horizontal= !display_data.flip_horizontal;

		/* calculate shape rectangle */
		position_sprite_axis(&textured_rectangle.x0, &textured_rectangle.x1, view->screen_height, view->screen_width, display_data.horizontal_positioning_mode,
			display_data.horizontal_position, display_data.flip_horizontal, shape_information->world_left, shape_information->world_right);
		position_sprite_axis(&textured_rectangle.y0, &textured_rectangle.y1, view->screen_height, view->screen_height, display_data.vertical_positioning_mode,
			display_data.vertical_position, display_data.flip_vertical, -shape_information->world_top, -shape_information->world_bottom);

		/* set rectangle bitmap and shading table */
		extended_get_shape_bitmap_and_shading_table(display_data.collection, display_data.low_level_shape_index, &textured_rectangle.texture, &textured_rectangle.shading_tables, view->shading_mode);
		
		// LP change: for the convenience of the OpenGL renderer
		textured_rectangle.ShapeDesc = BUILD_DESCRIPTOR(display_data.collection,display_data.low_level_shape_index);

		textured_rectangle.flags= 0;

		/* initialize clipping window to full screen */
		textured_rectangle.clip_left= 0;
		textured_rectangle.clip_right= view->screen_width;
		textured_rectangle.clip_top= 0;
		textured_rectangle.clip_bottom= view->screen_height;

		/* copy mirror flags */
		textured_rectangle.flip_horizontal= display_data.flip_horizontal;
		textured_rectangle.flip_vertical= display_data.flip_vertical;
		
		/* lighting: depth of zero in the camera’s polygon index */
		textured_rectangle.depth= 0;
		textured_rectangle.ambient_shade= get_light_intensity(get_polygon_data(view->origin_polygon_index)->floor_lightsource_index);
		textured_rectangle.ambient_shade= MAX(shape_information->minimum_light_intensity, textured_rectangle.ambient_shade);
		if (view->shading_mode==_shading_infravision) textured_rectangle.flags|= _SHADELESS_BIT;

		/* make the weapon reflect the owner’s transfer mode */
		instantiate_rectangle_transfer_mode(view, &textured_rectangle, display_data.transfer_mode, display_data.transfer_phase);
		
		/* and draw it */
		// LP: added OpenGL support
		if (!OGL_RenderSprite(textured_rectangle))
			texture_rectangle(&textured_rectangle, destination, view);
	}
	
	return;
}

static void position_sprite_axis(
	short *x0,
	short *x1,
	short scale_width,
	short screen_width,
	short positioning_mode,
	fixed position,
	boolean flip,
	world_distance world_left,
	world_distance world_right)
{
	short origin;

	/* if this shape is mirrored, reverse the left/right world coordinates */
	if (flip)
	{
		world_distance swap= world_left;
		world_left= -world_right, world_right= -swap;
	}
	
	switch (positioning_mode)
	{
		case _position_center:
			/* origin is the screen coordinate where the logical center of the shape will be drawn */
			origin= (screen_width*position)>>FIXED_FRACTIONAL_BITS;
			break;
		case _position_low:
		case _position_high:
			/* origin is in [0,WORLD_ONE] and represents the amount of the weapon visible off the side */
			origin= ((world_right-world_left)*position)>>FIXED_FRACTIONAL_BITS;
			break;
		
		default:
			// LP change:
			assert(false);
			// halt();
	}
	
	switch (positioning_mode)
	{
		case _position_high:
			*x0= screen_width - ((origin*scale_width)>>WORLD_FRACTIONAL_BITS);
			*x1= *x0 + (((world_right-world_left)*scale_width)>>WORLD_FRACTIONAL_BITS);
			break;
		case _position_low:
			*x1= ((origin*scale_width)>>WORLD_FRACTIONAL_BITS);
			*x0= *x1 - (((world_right-world_left)*scale_width)>>WORLD_FRACTIONAL_BITS);
			break;
		
		case _position_center:
			*x0= origin + ((world_left*scale_width)>>WORLD_FRACTIONAL_BITS);
			*x1= origin + ((world_right*scale_width)>>WORLD_FRACTIONAL_BITS);
			break;
		
		default:
			// LP change:
			assert(false);
			// halt();
	}
	
	return;
}

static void shake_view_origin(
	struct view_data *view,
	world_distance delta)
{
	world_point3d new_origin= view->origin;
	short half_delta= delta>>1;
	
	new_origin.x+= half_delta - ((delta*sine_table[NORMALIZE_ANGLE((view->tick_count&~3)*(7*FULL_CIRCLE))])>>TRIG_SHIFT);
	new_origin.y+= half_delta - ((delta*sine_table[NORMALIZE_ANGLE(((view->tick_count+5*TICKS_PER_SECOND)&~3)*(7*FULL_CIRCLE))])>>TRIG_SHIFT);
	new_origin.z+= half_delta - ((delta*sine_table[NORMALIZE_ANGLE(((view->tick_count+7*TICKS_PER_SECOND)&~3)*(7*FULL_CIRCLE))])>>TRIG_SHIFT);

	/* only use the new origin if we didn’t cross a polygon boundary */
	if (find_line_crossed_leaving_polygon(view->origin_polygon_index, (world_point2d *) &view->origin,
		(world_point2d *) &new_origin)==NONE)
	{
		view->origin= new_origin;
	}
	
	return;
}

/* ---------- mac-specific debugging calls */

#ifdef QUICKDRAW_DEBUG

#define SCALEF 5

static void debug_flagged_points(
	flagged_world_point2d *points,
	short count)
{
	short i;
	
	SetPort(screen_window);
	PenSize(1, 1);
	RGBForeColor(&rgb_black);
	RGBBackColor(&rgb_white);
	EraseRect(&screen_window->portRect);
	SetOrigin(-640/2, -480/2);
	MoveTo(-320, 0); LineTo(320, 0);
	MoveTo(0, 240); LineTo(0, -240);
	PenSize(2, 2);
	MoveTo(points[count-1].y>>SCALEF, - (points[count-1].x>>SCALEF));
	for (i=0;i<count;++i)
	{
		LineTo(points[i].y>>SCALEF, - (points[i].x>>SCALEF));
		psprintf(ptemporary, "%d", i);
		DrawString(temporary);
		MoveTo(points[i].y>>SCALEF, - (points[i].x>>SCALEF));
	}
}

static void debug_flagged_points3d(
	flagged_world_point3d *points,
	short count)
{
	short i;
	
	SetPort(screen_window);
	PenSize(1, 1);
	RGBForeColor(&rgb_black);
	RGBBackColor(&rgb_white);
	EraseRect(&screen_window->portRect);
	SetOrigin(-640/2, -480/2);
	MoveTo(-320, 0); LineTo(320, 0);
	MoveTo(0, 240); LineTo(0, -240);
	PenSize(2, 2);
	MoveTo(points[count-1].z>>SCALEF, - (points[count-1].x>>SCALEF));
	for (i=0;i<count;++i)
	{
		LineTo(points[i].z>>SCALEF, - (points[i].x>>SCALEF));
		psprintf(ptemporary, "%d", i);
		DrawString(temporary);
		MoveTo(points[i].z>>SCALEF, - (points[i].x>>SCALEF));
	}
}

static void debug_vector(
	world_vector2d *v)
{
	PenSize(1, 1);
	MoveTo(0, 0);
	LineTo(v->j, - v->i);
	MoveTo(0, 0);
	LineTo(- v->j, v->i);
	
	while (!Button()); while (Button());
}

static void debug_x_line(
	world_distance x)
{
	PenSize(1, 1);
	MoveTo(-320, - x>>SCALEF);
	LineTo(320, - x>>SCALEF);
	
	while (!Button()); while (Button());
}

#endif /* QUICKDRAW DEBUG */

#define NUMBER_OF_SCALED_VALUES 6

static struct shape_information_data *rescale_shape_information(
	struct shape_information_data *unscaled,
	struct shape_information_data *scaled,
	word flags)
{
	if (flags)
	{
		world_distance *scaled_values= &scaled->world_left;
		world_distance *unscaled_values= &unscaled->world_left;
		short i;
		
		scaled->flags= unscaled->flags;
		scaled->minimum_light_intensity= unscaled->minimum_light_intensity;
		
		if (flags&_object_is_enlarged)
		{
			for (i= 0; i<NUMBER_OF_SCALED_VALUES; ++i)
			{
				*scaled_values++= *unscaled_values + (*unscaled_values>>2), unscaled_values+= 1;
			}
		}
		else
		{
			if (flags&_object_is_tiny)
			{
				for (i= 0; i<NUMBER_OF_SCALED_VALUES; ++i)
				{
					*scaled_values++= (*unscaled_values>>1), unscaled_values+= 1;
				}
			}
		}
	}
	else
	{
		scaled= unscaled;
	}
	
	return scaled;
}
			
#ifdef OBSOLETE
static void sort_render_object_into_tree(
	struct render_object_data *new_render_object, /* null-terminated linked list */
	struct sorted_node_data **base_nodes,
	short base_node_count)
{
	struct render_object_data *render_object, *last_new_render_object;
	struct render_object_data *deep_render_object= (struct render_object_data *) NULL;
	struct render_object_data *shallow_render_object= (struct render_object_data *) NULL;
	struct sorted_node_data *desired_node;
	short i;

	/* find the last render_object in the given list of new objects */
	for (last_new_render_object= new_render_object;last_new_render_object->next_object;
		last_new_render_object= last_new_render_object->next_object);

	/* find the two objects we must be lie between */	
	for (render_object= render_objects;render_object<new_render_object;++render_object)
	{
		/* if these two objects intersect... */
		if (render_object->rectangle.x1>new_render_object->rectangle.x0 && render_object->rectangle.x0<new_render_object->rectangle.x1 &&
			render_object->rectangle.y1>new_render_object->rectangle.y0 && render_object->rectangle.y0<new_render_object->rectangle.y1)
		{
			/* update our closest and farthest matches */
			if (render_object->rectangle.depth>new_render_object->rectangle.depth) /* found deeper intersecting object */
			{
				if (!deep_render_object || deep_render_object->rectangle.depth>render_object->rectangle.depth)
				{
					deep_render_object= render_object;
				}
			}
			else
			{
				if (render_object->rectangle.depth<new_render_object->rectangle.depth) /* found shallower intersecting object */
				{
					if (!shallow_render_object || shallow_render_object->rectangle.depth<=render_object->rectangle.depth)
					{
						shallow_render_object= render_object;
					}
				}
			}
		}
	}

	/* find the node we’d like to be in (ignoring polygons which were not lit up by the view cone) */
	desired_node= base_nodes[0];
	for (i= 1;i<base_node_count;++i) if (base_nodes[i]>desired_node) desired_node= base_nodes[i];
	assert(desired_node>=sorted_nodes && desired_node<next_sorted_node);
	
	/* adjust desired node based on the nodes of the deep and shallow render object; only
		one of deep_render_object and shallow_render_object will be non-null after this if
		block.  the current object must be sorted with respect to this non-null object inside
		the object list of the desired_node */
	if (shallow_render_object && desired_node>=shallow_render_object->node)
	{
		/* we tried to sort too close to the front of the node list */
		desired_node= shallow_render_object->node;
		deep_render_object= (struct render_object_data *) NULL;
	}
	else
	{
		if (deep_render_object && desired_node<=deep_render_object->node)
		{
			/* we tried to sort too close to the back of the node list */
			desired_node= deep_render_object->node;
			shallow_render_object= (struct render_object_data *) NULL;
		}
		else
		{
			deep_render_object= shallow_render_object= (struct render_object_data *) NULL;
		}
	}
	
	/* update the .node fields of all the objects we’re about to add to reflect their new
		location in the sorted node list */
	for (render_object= new_render_object;render_object;render_object= render_object->next_object)
	{
		render_object->node= desired_node;
	}
	
	if (deep_render_object)
	{
		/* sort after all objects as deep as the deep render object in this node */
		while (deep_render_object->next_object && deep_render_object->next_object->rectangle.depth==deep_render_object->rectangle.depth) deep_render_object= deep_render_object->next_object;
		last_new_render_object->next_object= deep_render_object->next_object;
		deep_render_object->next_object= new_render_object;
	}
	else
	{
		if (shallow_render_object)
		{
			struct render_object_data **reference;
			
			/* find the reference to the first object as shallow as the shallow render object in this node */
			for (reference= &desired_node->exterior_objects;
				(*reference)->rectangle.depth!=shallow_render_object->rectangle.depth && *reference;
				reference= &(*reference)->next_object);
			assert(*reference);
			
			/* sort before this object in the given node */
			last_new_render_object->next_object= *reference;
			*reference= new_render_object;
		}
		else
		{
			/* sort anywhere in the node */
			last_new_render_object->next_object= desired_node->exterior_objects;
			desired_node->exterior_objects= new_render_object;
		}
	}

	return;
}
#endif


// LP addition: resetters for some of the lists:
void ResetEndpointClips(void)
{
	EndpointClips.ResetLength();
	for (int k=0; k<NUMBER_OF_INITIAL_ENDPOINT_CLIPS; k++)
		EndpointClips.Add();
}

void ResetLineClips(void)
{
	LineClips.ResetLength();
	for (int k=0; k<NUMBER_OF_INITIAL_LINE_CLIPS; k++)
		LineClips.Add();
}