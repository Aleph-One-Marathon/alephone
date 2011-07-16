#ifndef _NEW_RENDER_VISIBILITY_TREE_CLASS_
#define _NEW_RENDER_VISIBILITY_TREE_CLASS_
/*
	
	Rendering Visibility-Tree Class
	by Loren Petrich,
	August 6, 2000
	
	Defines a class for doing rendering visibility; from render.c
	
	Made [view_data *view] a member and removed it as an argument
	
Sep. 11, 2000 (Loren Petrich):
	Made POINTER_DATA more general -- byte *, so one can do pointer arithmetic on it correctly.

Sep. 15, 2000 (Loren Petrich):
	Fixed stale-pointer bug in cast_render_ray() by using index instead of pointer to parent node
	
Oct 13, 2000
	LP: replaced GrowableLists and ResizableLists with STL vectors
	
Sept 06, 2001 (Ian Rickard)
	re-wrote for newclip.
*/

#include <vector>
#include "world.h"
#include "render.h"
#include "WorkQueue.h"

// ---

class NewVisTree;

// ---

typedef double CROSSPROD_TYPE;

// ---

enum // used in rasterizer, but here for consistency.
{
	_clip_left= 0x0001, // for endpoints means behind left clip plane
	_clip_right= 0x0002,
	_clip_up= 0x0004,
	_clip_down= 0x0008
};

enum // portal view flags.
{
	_portal_view_z_mirror = 0x0001
};

// ---

struct clipping_window_data : public rasterize_window
{
	long_vector2d left, right;
	long_vector2d top, bottom; /* j is really k for top and bottom */
	// inerited:
	// short x0, x1, y0, y1;
	uint16 next_window;
};

struct render_node_data {
	polygon_reference polygon;
	uint16 left_render_endpoint, right_render_endpoint; // transformed endpoint indicies that clip the left and right side of this node.
	uint16 clip_line_index; // real clip is intersection of all clip lines above this.
	uint16 parent, child, sibling;
	uint16 portal_view; // portal view this node is in.
	uint16 next_node_for_poly; // next node for this poly in this portal view
	clipping_window_data first_clip_window; // nodes always have at least one clip window, so build it into the struct.
		

	render_node_data() {first_clip_window.next_window = next_node_for_poly = UNONE; }
};

struct sorted_node_data {
	render_node_data *node;
	uint16 interior_objects, exterior_objects; // indicies in the PlaceObject manager's list
	uint16 order;
	sorted_node_data() {interior_objects = exterior_objects = UNONE;};
	sorted_node_data(render_node_data *n, uint16 o) {node = n; order = o; interior_objects = exterior_objects = UNONE;}
};

struct portal_view_data
{
	static uint16 *memBlock;
	static int memBlockAlloced, memBlockSize;
	
	portal_reference portalID; // portal ID this view is through
	uint16 parent_portal_view; // index in the vis tree's portal view list of the portal we're being
	// viewed through.  UNONE if this is the root portal.
	uint16 depth;
	short flags;

	uint16 *endpoint_translation_table; // lookup tables from map indicies to indicies in the vis
	uint16 *line_translation_table; // tree's lists.
	uint16 *polygon_node_table; // first node for each polygon
	
//	world_vector2d left_edge, right_edge, top_edge, bottom_edge; // in world-space

	angle yaw, pitch, roll; // pitch and roll are the same as the root view's, yaw can change tho
	short dtanpitch; // vertical offset
	world_point3d origin; // translated view origin
	angle landscape_yaw;
	
	portal_view_data();
	bool setup(portal_reference portal, NewVisTree *visTree, uint16 parent_view_index);
	~portal_view_data();
};

struct translated_endpoint_data
{
	short x; /* screen-space */
	long_vector2d vector; /* viewer-space */
	translated_endpoint_data(){}
	translated_endpoint_data(short X, const long_vector2d &Vector) {x = X; vector = Vector;}
};

// ---

class NewVisTree
{
	struct line_render_data {
		line_reference line;
		side_reference side;
		polygon_reference target_poly;
		uint16 parent_node;
		
		line_render_data(){}
		line_render_data(line_reference l, side_reference s, polygon_reference p, uint16 n)
			{ line = l; side = s; target_poly = p; parent_node = n; }
	};
		
	struct line_clip_data
	{
		uint16 left_endpoint, right_endpoint;
		world_distance top, bottom;
	};
	
	
	int16 calculate_clip_endpoint(endpoint_reference map_index, portal_view_data *portal_view);
	int16 calculate_clip_line(line_reference map_index, portal_view_data *portal_view);
	bool clip_window_to_line(clipping_window_data *window, uint16 line, uint16 side_flags);
			// returns true if the resulting clip window is empty
	
	uint16	get_clip_endpoint(endpoint_reference map_index, uint16 portal_index) {
		uint16 localIndex = PortalViews[portal_index].endpoint_translation_table[(int)map_index.index()];
		if (localIndex == UNONE)
			localIndex = calculate_clip_endpoint(map_index, get_portal_view(portal_index));
		return localIndex;
	}
	
	uint16 get_clip_line(line_reference map_index, uint16 portal_index) {
		uint16 localIndex = PortalViews[portal_index].line_translation_table[(int)map_index.index()];
		if (localIndex == UNONE)
			localIndex = calculate_clip_line(map_index, get_portal_view(portal_index));
		return localIndex;
	}
	
	uint16 new_node() {vector<render_node_data> *n = &Nodes; n->push_back(render_node_data()); return n->size()-1;}
	void whoops_pop_node() {Nodes.pop_back();}
	uint16 new_endpoint(const long_vector2d &heading, short x) {
		vector<translated_endpoint_data> *n = &Endpoints;
		n->push_back(translated_endpoint_data(x, heading));
		return n->size()-1;
	}
	uint16 get_portal_view(portal_reference portal, uint16 parent_view);
	uint16 new_window() {vector<clipping_window_data> *n = &ClippingWindows; n->push_back(clipping_window_data()); return n->size()-1;}
	
	line_clip_data* get_clip_line(uint16 i) {assert(i != UNONE && i < LineClips.size()); return &LineClips[i];}
	translated_endpoint_data* get_clip_endpoint(uint16 i) {assert(i != UNONE && i < Endpoints.size()); return &Endpoints[i];}
	
	WorkQueue<line_render_data> LineQueue;
	
	/* every time we find a unique line which clips something, we build one of these for it (notice
		the translation table from line_indexes on the map to line_clip_indexes in our table for when
		we cross the same clip line again */
	vector<line_clip_data> LineClips;
 	
 	polygon_reference get_real_origin(portal_view_data *portalView);
 	
 	void flatten_render_tree();
 	
 	void rebuild_polygon_lookup();
 	
 	void check_tree_integrity();
 	
public:
	// Pointer to view
	view_data *view;
		
	/* sorted back to front */
	vector<sorted_node_data> SortedNodes;
	/* every time we find a unique endpoint/portalView in a visible polygon, make one of these. */
	vector<translated_endpoint_data> Endpoints;
	/* every time we pass through an elevation line, make one of these */
	vector<clipping_window_data> ClippingWindows;
	/* every time we find a portal through a unique portal path, we build a new one of these. */
	vector<portal_view_data> PortalViews;
	/* every time we hit a polygon we build a new one of these */
	vector<render_node_data> Nodes;

	// Builds everything
	uint16 root_node;
	
	render_node_data* get_node(uint16 i) {assert(i != UNONE && i < Nodes.size()); return &Nodes[i];}
	portal_view_data* get_portal_view(uint16 i) {assert(i != UNONE && i < PortalViews.size()); return &PortalViews[i];}
	clipping_window_data* get_window(uint16 i) {assert(i != UNONE && i < ClippingWindows.size()); return &ClippingWindows[i];}
	
 	void build_render_tree();
 	
  	// Inits everything
 	NewVisTree();	
};

#endif
