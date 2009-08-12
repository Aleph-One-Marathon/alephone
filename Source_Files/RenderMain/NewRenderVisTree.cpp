/*
	
	Rendering Visibility-Tree Class
	by Loren Petrich,
	August 6, 2000
	
	Contains the finding of the visibility tree for rendering; from render.c
	
	Made [view_data *view] a member and removed it as an argument

Sep. 15, 2000 (Loren Petrich):
	Fixed stale-pointer bug in cast_render_ray() by using index instead of pointer to parent node.
	Added some code to keep parent and ParentIndex in sync.
	
Oct 13, 2000
	LP: replaced GrowableLists and ResizableLists with STL vectors

Aug 12, 2001 (Ian Rickard):
	Various changes relating to B&B prep or OOzing

Sept 6, 2001 (Ian Rickard):
	Completely re-wrote
*/

/* How this whole thing works:

first, lets talk about the coordinate spaces we're talking about:
world-space		exactly what it says.  this is the map-file coordinates of everything
viewer-spce		world-space rotated and translated so the viewer is at 0,0 looking along
				the +x axis.  +y is to the right and -y is to the left.
screen-space	pixel coordinates.  this is the goal.

How this used to work:
I'm not sure about the details, but its something like this:
First, look outwards on the polygon map by shooting rays outwards from the endpoints of every polygon you
hit.  transparent endpoints are ignored for clipping, and only used for visibility discovery.  Once you
have this tree of visible nodes (polygons), go back and sort it by depth.  At the same time you're sorting,
build clip rectangle for each node.

how it works now:
Starting with the polygon the viewer is on (could be extended to start with multiple polygons for the
chasecam) look at each line in the polygon.  For each line that's horizontal extetns intersect the current
clip window (and is facing toward us, and is transparent), put it in the list of spans to cast with
appropriate extra info.  Repeat for the polygon on the other side of each line until we don't generate any
more lines.  When we're done with this go back and look through the clip windows we generated for each
polygon.  If we can simplify the clip area of the polygon, reparent it to the farthest parent node of the
clip windows being reduced and drop the old clip window.  This results in many more tall slender clip
windows than the old method, but this is ok because vertical surfaces draw little verticle strips just fine.

To draw horizontal surfaces (which draw quite inefficiently when sliced into lots of little vertical
strips) find the top of the clip window, the first group of clip rects (possibly only 1) which reaches
that height (and any neighboring clip rects that also reach that height) and the highest clip rect
neighboring that group.  draw the rect between these heights, set the top of the rendered rects to the
lower height, and repeat.

*/

#include "cseries.h"

#include "map.h"
#include "newRenderVisTree.h"
#include <new> // for bad_alloc
#include "readout_data.h"

#define INITIAL_LINE_QUEUE_SIZE 256
#define INITIAL_NODES 1024
#define INITIAL_LINE_CLIPS 512
#define INITIAL_ENDPOINT_CLIPS 256
#define INITIAL_CLIPPING_WINDOWS 512
#define MAXIMUM_PORTAL_VIEWS 30 // max ~400k of possibly unused LUTs laying around, thats acceptable for the flexibility.

enum
{
	_line_clip_top = 0x0001,
	_line_clip_bottom = 0x0002,
	_line_clip_portal = 0x0004
};

// used as the target of gotos at the end of code blocks
#define DO_NOTHING NULL

portal_view_data::portal_view_data()
{
	endpoint_translation_table = NULL;
	line_translation_table = NULL;
	polygon_node_table = NULL;
}

uint16 *portal_view_data::memBlock;
int portal_view_data::memBlockAlloced, portal_view_data::memBlockSize;

bool portal_view_data::setup(
	portal_reference	portalRef,
	NewVisTree			*visTree,
	uint16				parent_view_index)
{
	int points = EndpointList.size(),
		lines = LineList.size(),
		polys = PolygonList.size();
	
	int lutSize = points + lines + polys;
	
	if (!memBlock)
	{
		memBlock = (uint16*)NewPtr(lutSize * sizeof(uint16) * MAXIMUM_PORTAL_VIEWS);
		if (! memBlock) throw bad_alloc();
		memBlockAlloced = 0;
		memBlockSize = lutSize * MAXIMUM_PORTAL_VIEWS;
	}
	
	if(parent_view_index == UNONE) {
		memBlockAlloced = 0;
	}
	
	if (memBlockAlloced + lutSize > memBlockSize) {
		return false;
	}
	
	endpoint_translation_table = memBlock + memBlockAlloced;
	memBlockAlloced += lutSize;
	memset(endpoint_translation_table, -1, lutSize*sizeof(uint16));
	// ok to set chars since -1 is all-on, two -1 chars is a -1 short.
	line_translation_table = endpoint_translation_table + points;
	polygon_node_table = line_translation_table + lines;
	
	
	portalID = portalRef;
	
	portal_data *portal = portalRef;
	portal_view_data *parent_view = (parent_view_index==UNONE)?NULL:visTree->get_portal_view(parent_view_index);
	
	yaw = normalize_angle((parent_view ? parent_view->yaw : visTree->view->yaw) - portal->yaw);
	pitch = parent_view ? parent_view->pitch : visTree->view->pitch;
	roll = parent_view ? parent_view->roll : visTree->view->roll;
	dtanpitch = parent_view ? parent_view->dtanpitch : visTree->view->dtanpitch;
	landscape_yaw = normalize_angle((parent_view ? parent_view->landscape_yaw : visTree->view->landscape_yaw) - portal->yaw);
	origin = parent_view ? parent_view->origin : visTree->view->origin;
	
	// forward transform (portal space to world space) is rotate then translate, so to take the world-space (technically
	// parent-portal-space) origin into portal space to subtract the translation and then rotate in the opposite direction.
	long_vector2d tempr;
	tempr.i = origin.x - portal->translate.x;
	tempr.j = origin.y - portal->translate.y;
	
	angle theta = normalize_angle(-portal->yaw);
	origin.x = ((tempr.i*cosine_table[theta])>>TRIG_SHIFT) + ((tempr.j*sine_table[theta])>>TRIG_SHIFT);
	origin.y = ((tempr.j*cosine_table[theta])>>TRIG_SHIFT) - ((tempr.i*sine_table[theta])>>TRIG_SHIFT);
	origin.z -= portal->translate.z;
	/* calculate left cone vector */
//	theta= normalize_angle(yaw - visTree->view->half_cone);
//	left_edge.i= cosine_table[theta]; left_edge.j= sine_table[theta];
	
	/* calculate right cone vector */
//	theta= normalize_angle(yaw + visTree->view->half_cone);
//	right_edge.i= cosine_table[theta]; right_edge.j= sine_table[theta];
	
//	top_edge = visTree->view->top_edge;
//	bottom_edge = visTree->view->bottom_edge;
	
	flags = 0;
	
	if (portal->is_z_mirror()) {
		flags ^= _portal_view_z_mirror; // xor in, because a mirror through a mirror is right way around
		origin.z = -origin.z;
	}
	
	parent_portal_view = parent_view_index;
	
	if (parent_view) {
		depth = parent_view->depth + 1;
	} else {
		depth = 0;
	}
	
	return true;
}

portal_view_data::~portal_view_data() {
}

// Inits everything
NewVisTree::NewVisTree():
	view(NULL)	// Idiot-proofing
{
	LineQueue.preallocate(INITIAL_LINE_QUEUE_SIZE);
	Endpoints.reserve(INITIAL_ENDPOINT_CLIPS);
	LineClips.reserve(INITIAL_LINE_CLIPS);
	ClippingWindows.reserve(INITIAL_CLIPPING_WINDOWS);
	Nodes.reserve(INITIAL_NODES);
}

polygon_reference NewVisTree::get_real_origin(portal_view_data *portalView)
{
	uint16 *translationTable = portalView->endpoint_translation_table;
	polygon_data *polygon = polygon_reference(view->origin_polygon_index);
	
	int i, endpointCount = polygon->vertex_count;
	translated_endpoint_data *endpointArray = &Endpoints[0];
	translated_endpoint_data *polyPoints[MAXIMUM_VERTICES_PER_POLYGON];
	// first, discover this poly's endpoints.
	for (i=0 ; i < endpointCount ; i++) {
		int polyPoint;
		polyPoint = translationTable[polygon->endpoint_indexes[i]];
		if (polyPoint == UNONE) {
			// also calculates transformed endpoint coords.
			polyPoint = translationTable[polygon->endpoint_indexes[i]] = 
					calculate_clip_endpoint(polygon->endpoint_indexes[i], portalView);
			translated_endpoint_data *endpointArray = &Endpoints[0];
		}
		polyPoints[i] = endpointArray + polyPoint;
	}
	
	for (i=0 ; i < endpointCount ; i++) {
		bool flip = false;

		extern bool gDebugKeyDown;
		if (gDebugKeyDown && polygon->line_indexes[i]==531) Debugger();
		
		if (polygon->adjacent_polygon_indexes[i] == NONE)
			continue; // wall.
		
		translated_endpoint_data *leftPoint = polyPoints[i], *rightPoint=polyPoints[WRAP_HIGH(i, endpointCount-1)];
		
		int32 cross = ((leftPoint->vector.i * rightPoint->vector.j) - (leftPoint->vector.j * rightPoint->vector.i));
		
		if (cross <= 0) {
			
			int32 di = rightPoint->vector.i - leftPoint->vector.i;
			int32 dj = rightPoint->vector.j - leftPoint->vector.j;
			
			// if we aren't within the endpoints of this line, this is the wrong line.
			if (    leftPoint->vector.i*di + leftPoint->vector.j*dj > 0
			    ||  rightPoint->vector.i*di + rightPoint->vector.j*dj < 0)
			    continue;
			
			flip = true;
			
			// check if we're on the right side of this line to render across it.  we only render one way
			// across zero-cross-product lines based on their translated position.  This is a completely
			// arbitrary decision and designed to prevent infinite loops in the vis determination caused
			// by the Invisible Polygon Line Barrier of Illogical Infinity
			
			if (cross == 0) {
				flip = false;
				if (dj) {
					if (dj <= 0) flip = true;
				} else {
					if (di <= 0) flip = true;
				}
			}
		}
		
		if (flip) {
			if (cross == 0 && line_reference(polygon->line_indexes[i])->endpoint_0().index() == polygon->endpoint_indexes[i]) {
				uint16 cline = portalView->endpoint_translation_table[polygon->line_indexes[i]];
				
				if (cline == UNONE)
					cline = calculate_clip_line(polygon->line_indexes[i], portalView);
				
				assert (cline < LineClips.size());
				
				line_clip_data *line = &LineClips[cline];
				
				// re-use cline to flip the endpoints
				cline = line->left_endpoint;
				line->left_endpoint = line->right_endpoint;
				line->right_endpoint = cline;
			}
			
			if(polygon->adjacent_polygon_indexes[i] != NONE)
				return polygon->adjacent_polygon_indexes[i];
		} else {
			if (cross == 0 && line_reference(polygon->line_indexes[i])->endpoint_1().index() == polygon->endpoint_indexes[i]) {
				uint16 cline = portalView->endpoint_translation_table[polygon->line_indexes[i]];
				
				if (cline == UNONE)
					cline = calculate_clip_line(polygon->line_indexes[i], portalView);
				
				assert (cline < LineClips.size());
				
				line_clip_data *line = &LineClips[cline];
				
				cline = line->left_endpoint;
				line->left_endpoint = line->right_endpoint;
				line->right_endpoint = cline;
			}
		}
	}
	return view->origin_polygon_index;
}

void NewVisTree::build_render_tree()
{
	// first things first, tear down anything left from the previous frame.
	
	LineQueue.flush();
	Endpoints.clear();
	LineClips.clear();
	ClippingWindows.clear();
	PortalViews.clear();
	Nodes.clear();
	SortedNodes.clear();
	
	gUsageTimers.visTree.Start();

	// build and configure the root portal
	uint16 portal = get_portal_view(portal_reference(kRootPortal), UNONE);
	
	assert (portal != UNONE);
	
	// build the root node
	root_node = new_node();
	render_node_data  *work_node = get_node(root_node);
	
	// and configure it
	work_node->polygon = get_real_origin(get_portal_view(portal));
	{
		short_to_long_2d(view->untransformed_left_edge, work_node->first_clip_window.left);
		work_node->left_render_endpoint = new_endpoint(work_node->first_clip_window.left, 0);
		work_node->first_clip_window.x0 = 0;
		short_to_long_2d(view->untransformed_right_edge, work_node->first_clip_window.right);
		work_node->right_render_endpoint = new_endpoint(work_node->first_clip_window.right, view->screen_width);
		work_node->first_clip_window.x1 = view->screen_width;
		
		short_to_long_2d(view->top_edge,work_node->first_clip_window.top);
		work_node->first_clip_window.y0 = 0;
		short_to_long_2d(view->bottom_edge,work_node->first_clip_window.bottom);
		work_node->first_clip_window.y1 = view->screen_height;
		
		work_node->first_clip_window.next_window = UNONE;
	}
	work_node->clip_line_index = UNONE;
	work_node->parent = work_node->sibling = UNONE;
	work_node->child = UNONE;
	work_node->portal_view = portal;
	// the remaining fields are set up by the constructor
	
	goto process_node;
	// yeah, I know you're not supposed to do this, but what better way would you suggest I code this?
	// the destination of this goto is inside the do{}while() loop below, about 30 lines in.
	
	line_render_data *work_line;
	// have to declare this out here.  ick!  this has to sit around on the stack for the whole loop.
	
	
	while (work_line = LineQueue.pop()) {
		check_tree_integrity();
		
		int16 new_node_index = new_node(); // note: we must call this first before get_node( for the parent because the nodes may move!
		render_node_data  *parent_node = get_node(work_line->parent_node);
		int16 clip_line_index = get_clip_line(work_line->line, parent_node->portal_view);
		
		work_node = get_node(new_node_index);
		// copy the parent's clip window to the new node
		work_node->first_clip_window = parent_node->first_clip_window;
		// we're still building the tree so nodes only ever have one clip window
		
		// now clip the window down by the line we passed through
		if (clip_window_to_line(&work_node->first_clip_window, clip_line_index, work_line->side.not_none() ? work_line->side->flags : _side_is_transparent)) {
			// move along!  nothing to see here! (literally)
			whoops_pop_node();
			continue; // !
		}
		work_node->first_clip_window.next_window = UNONE;
		
		work_node->parent = UNONE;
		work_node->child = UNONE;
		work_node->sibling = UNONE;
		work_node->next_node_for_poly = UNONE;
		check_tree_integrity();

		// set up the node
		work_node->parent = work_line->parent_node;
		work_node->sibling = parent_node->child;
		work_node->child = UNONE;
		parent_node->child = new_node_index;
			check_tree_integrity();
		work_node->portal_view = UNONE;
		
		if (work_line->side.not_none() && work_line->side->is_portal()) {
			work_node->polygon = work_line->side->data.portal.destination_polygon;
			work_node->portal_view = get_portal_view(work_line->side->data.portal.portal, parent_node->portal_view);
		}
		
		if (work_node->portal_view == UNONE) {
			work_node->polygon = work_line->target_poly;
			work_node->portal_view = parent_node->portal_view;
		}
		
		check_tree_integrity();
		
		if (0) {
process_node:
			// little more setup we can't do outside the loop because of variable scope
			new_node_index = root_node;
			check_tree_integrity();
		}
		// first time through we jump straight to here for multiple reasons:
		// 1. setting up a render node is easy, setting up a line to generate a render node would be dificult to impossible with this model
		// 2. moving the line->node conversion to the end of the loop is possible, but would result in equally if not more messed up logic
		// 3. SKRU U BJARNE!
		
		uint16 *translationTable;
		translated_endpoint_data *polyPoints[MAXIMUM_VERTICES_PER_POLYGON];
		uint16 polyPointIndicies[MAXIMUM_VERTICES_PER_POLYGON];
		portal_view_data *portalView = get_portal_view(work_node->portal_view);
		
		translationTable = portalView->endpoint_translation_table;
		
		polygon_data *polygon = polygon_reference(work_node->polygon);
		
		int i, endpointCount = polygon->vertex_count;
		translated_endpoint_data *endpointArray = &Endpoints[0];
		// first, discover this poly's endpoints.
		for (i=0 ; i < endpointCount ; i++) {
			polyPointIndicies[i] = translationTable[polygon->endpoint_indexes[i]];
			if (polyPointIndicies[i] == UNONE) {
				// also calculates transformed endpoint coords.
				polyPointIndicies[i] = translationTable[polygon->endpoint_indexes[i]] = 
						calculate_clip_endpoint(polygon->endpoint_indexes[i], portalView);
				translated_endpoint_data *endpointArray = &Endpoints[0];
			}
			polyPoints[i] = endpointArray + polyPointIndicies[i];
		}
		
		check_tree_integrity();

		// push ourselves onto the list for this polygon in this view.
		work_node->next_node_for_poly = portalView->polygon_node_table[(int)work_node->polygon.index()];
		assert (work_node->next_node_for_poly == UNONE || (work_node->next_node_for_poly != new_node_index && work_node->next_node_for_poly < Nodes.size()));
//		assert((int)work_node->polygon.index() == view->origin_polygon_index || new_node_index > 0);
		portalView->polygon_node_table[(int)work_node->polygon.index()] = new_node_index;
		// If codewarrior doesn't manage to do subexpression elimination with that, somebody is going to die.
		
		check_tree_integrity();
		
		for (i=0 ; i < polygon->vertex_count ; i++) {
			if (polygon->adjacent_polygon_indexes[i] == NONE)
				continue; // wall.
			
			if (polygon->side_indexes[i]!=NONE && !side_reference(polygon->side_indexes[i])->is_transparent())
				continue; // not a transparent side, don't render whats behind it
			
			translated_endpoint_data *leftPoint = polyPoints[i], *rightPoint=polyPoints[WRAP_HIGH(i, endpointCount-1)];
			
			// dot product to check if the side faces us.
			
			int32 cross = ((leftPoint->vector.i * rightPoint->vector.j) - (leftPoint->vector.j * rightPoint->vector.i));
			
			if (cross < 0)
				continue; // backface
			
			if (cross == 0) { // sideways face.
				int di = rightPoint->vector.i - leftPoint->vector.i;
				int dj = rightPoint->vector.j - leftPoint->vector.j;
				
				// we need to throw out this line when crossing from one side, so pick a side and throw it out
				// from that side.  This is a completely arbitrary decision designed to prevent infinite loops
				// in the vis determination caused by Invisible Polygon Line Barrier of Illogical Infinity
				if (dj) {
					if (dj <= 0) continue;
				} else {
					if (di <= 0) continue;
				}
			}
			
			if ((work_node->first_clip_window.right.i*leftPoint->vector.j - work_node->first_clip_window.right.j*leftPoint->vector.i)<=0) {
			// left endpoint clipped by right edge
				// we can only toss it out if ONLY the right edge clips it.  if both edges clip it we fall back to the last test
	     		if ((work_node->first_clip_window.left.i*leftPoint->vector.j - work_node->first_clip_window.left.j*leftPoint->vector.i)>0)
					continue;
			}
			
	     	if ((work_node->first_clip_window.left.i*rightPoint->vector.j - work_node->first_clip_window.left.j*rightPoint->vector.i)<=0) {
			// right endpoint clipped by right edge
				if ((work_node->first_clip_window.right.i*rightPoint->vector.j - work_node->first_clip_window.right.j*rightPoint->vector.i)>0)
	     			continue;
	     	}
			
			
			/*if (leftPoint->x != -1) {
				if (leftPoint->x >= work_node->first_clip_window.x1)
					continue; // clipped by right edge of window
				if (leftPoint->x >= work_node->first_clip_window.x0)
					goto side_visible; // left endpoint inside view frustrum, so side must be visible
			}
			if (rightPoint->x != -1) {
				if (rightPoint->x < work_node->first_clip_window.x0)
					continue; // clipped by left edge of window
				if (rightPoint->x <= work_node->first_clip_window.x1)
					goto side_visible; // right endpoint inside view frustrum, so side must be visible
			}*/
			
			// if both points are outside the view cone, then the line has to cross in front of us (+j) to be visible.
			if (leftPoint->x == -1 && rightPoint->x == -1 &&
			    (cross ^ (rightPoint->vector.j - leftPoint->vector.j)) <= 0)
				continue;
			// ugly? yes.  gets the job done? yes, quickly.
			// doing dot/span gives us the y-intersect of the line, but all we really care about is the sign.
			// instead of dividing, which takes a while, or multiplying, which might overflow, we xor, which has the
			// sign-flipping attributes of dividing or multiplying, without the cost or risk.
			
			
			// in a perfect (floating-point) world this wouldn't be necesary.  but we can get roundoff errors when translating points into viewr
			// space that produce concave polygons which can then link into themselves (#183 in Ne Cede Malis is a good example because not only
			// does it do this, but it has a nearby wall which lines the viewer up right along this line).  This code is to avoid that. In even 
			// rarer cases it can cause a single pixel sliver to remain un-drawn, but that better than locking up hard when trying to with a node
			// thats it's own parent.
			{
				uint16 lineage = new_node_index;
				while (lineage != UNONE) {
					assert (lineage < Nodes.size());
					render_node_data  *ancestor_node = get_node(lineage);
					if (ancestor_node->polygon == polygon->adjacent_polygon_indexes[i]) goto continue_outer_loop;
					lineage = ancestor_node->parent;
				}
			}
			
			side_visible:
			// all conditions check out, its visible, lets do it!
			LineQueue.push(line_render_data(
				line_reference(polygon->line_indexes[i]),
				side_reference(polygon->side_indexes[i]),
				polygon_reference(polygon->adjacent_polygon_indexes[i]),
				new_node_index));
continue_outer_loop:DO_NOTHING;
		}
		
		check_tree_integrity();
	}
	gUsageTimers.visTree.Stop();
	
	gUsageTimers.visFlat.Start();
	check_tree_integrity();
	flatten_render_tree();
	check_tree_integrity();
	rebuild_polygon_lookup();
	gUsageTimers.visFlat.Stop();
	
	return;
}

uint16 NewVisTree::get_portal_view(portal_reference portal, uint16 parent_view) {
	vector<portal_view_data> *v = &PortalViews;
	// Now, in most of the time we have fewer than a dosen portal views and only a couple dosen
	// portal lines visible so doing a linear search here isn't a big deal.
	for (int i=0 ; i<v->size() ; i++) {
		portal_view_data *this_view = &v[0][i];
		if ((this_view->portalID == portal) && (this_view->parent_portal_view == parent_view))
			return i;
	}
	uint16 result = v->size();
	v->insert(v->end(), portal_view_data()) - v->begin();
	if (! v[0][result].setup(portal, this, parent_view)) {
		result = UNONE;
	}
	return result;
}

int16 NewVisTree::calculate_clip_endpoint(endpoint_reference map_index, portal_view_data *portal_view) {
	vector<translated_endpoint_data> *e = &Endpoints;
	uint16 ep_index = e->size();
	assert(ep_index < UNONE);
	e->push_back(translated_endpoint_data());


	endpoint_data *endpoint= map_index;
	
	translated_endpoint_data *data= &e[0][ep_index];
	
	endpoint->transformedL = long_point2d(endpoint->vertex);
	
	transform_long_point2d(&endpoint->transformedL, &portal_view->origin, portal_view->yaw);
	
	data->vector = long_vector2d(endpoint->transformedL);
	
	//warn(data->vector.i);
	
	if (    (view->untransformed_right_edge.i*data->vector.j - view->untransformed_right_edge.j*data->vector.i)<0
	     || (view->untransformed_left_edge.i*data->vector.j - view->untransformed_left_edge.j*data->vector.i)<0)
	{
		data->x = -1;
	} else {
		int32 x = int32(view->half_screen_width) + (data->vector.j * int32(view->world_to_screen_x))/data->vector.i;
		data->x= PIN(x, 0, view->screen_width);
	}
	
	portal_view->endpoint_translation_table[(int)map_index.index()] = ep_index;
	
	return ep_index;
}

int16 NewVisTree::calculate_clip_line(line_reference map_index, portal_view_data *portal_view) {
	vector<line_clip_data> *l = &LineClips;
	uint16 line_index = l->insert(l->end(), line_clip_data()) - l->begin();
	line_data *line= map_index;
	
	line_clip_data *data= &l[0][line_index];
	
	uint16 point0, point1;
	
	point0 = portal_view->endpoint_translation_table[(int)line->endpoint_0().index()];
	if (point0 == UNONE)
		point0 = calculate_clip_endpoint(line->endpoint_0(), portal_view);
	assert(point0 != UNONE);
	
	point1 = portal_view->endpoint_translation_table[(int)line->endpoint_1().index()];
	if (point1 == UNONE)
		point1 = calculate_clip_endpoint(line->endpoint_1(), portal_view);
	assert(point1 != UNONE);
	
	translated_endpoint_data *e = &Endpoints[0];
	
	translated_endpoint_data *p0 = e+point0, *p1 = e+point1;
	
	int32 cross = ((p0->vector.i * p1->vector.j) - (p0->vector.j * p1->vector.i));
	
	if (cross >= 0) {
		data->left_endpoint = point0;
		data->right_endpoint = point1;
	} else {
		data->left_endpoint = point1;
		data->right_endpoint = point0;
	}
	
	data->top = line->lowest_ceiling() - portal_view->origin.z;
	data->bottom = line->highest_floor() - portal_view->origin.z;
	
	portal_view->line_translation_table[(int)map_index.index()] = line_index;
	
	return line_index;
}

bool NewVisTree::clip_window_to_line(clipping_window_data *window, uint16 line, uint16 side_flags) {
	translated_endpoint_data *e = &Endpoints[0];
	line_clip_data *clip_line = &LineClips[line];
	bool clipped_left = true, clipped_right = true;
	
	translated_endpoint_data *left_point = e+clip_line->left_endpoint;
	if (left_point->x != -1 && left_point->x > window->x0) {
		window->x0 = left_point->x;
		window->left = left_point->vector;
		clipped_left = false;
	}
	
	translated_endpoint_data *right_point = e+clip_line->right_endpoint;
	if (right_point->x != -1 && right_point->x < window->x1) {
		window->x1 = right_point->x;
		window->right.i = -right_point->vector.i; // negative so we clip to the right (left) side.
		window->right.j = -right_point->vector.j; // "no, your other left"
		clipped_right = false;
	}
	
	// check if we're clipped out of existance.
	if (window->x0 >= window->x1) return true;
	
	// calculate line intersection with window and clip vertically...
	bool tight_top=false, tight_bottom = false, needs_chop = false;;
	
	if (clip_line->bottom > 0 || side_flags & _side_clips_bottom) {
	 // agressively clip to bottom
		tight_bottom = true;
	}
	
	if (clip_line->top < 0 || side_flags & _side_clips_top) {
	 // agressively clip to top
		tight_top = true;
	}
	
	long_vector2d tightLeft, tightRight, looseLeft, looseRight;
	if (tight_top || tight_bottom) {
		if (clipped_left) {
			find_line_vector_intersection(&left_point->vector, &right_point->vector, &window->left, &tightLeft);
		} else {
			tightLeft = left_point->vector;
		}
		if (clipped_right) {
			find_line_vector_intersection(&left_point->vector, &right_point->vector, &window->right, &tightRight);
		} else {
			tightRight = right_point->vector;
		}
		if (tightLeft.i <= 0 || tightRight.i <= 0) return false; // line hits the origin, don't try to clip.
	}
	
	if (!(tight_top && tight_bottom)) {
		if (left_point->vector.i < 0) {
			looseLeft.i = view->untransformed_left_edge.i;
			looseLeft.j = view->untransformed_left_edge.j;
			find_line_vector_intersection(&left_point->vector, &right_point->vector, &looseLeft, &looseLeft);
		} else {
			looseLeft.i = left_point->vector.i;
			looseLeft.j = left_point->vector.j;
		}
		if (right_point->vector.i < 0) {
			looseRight.i = view->untransformed_right_edge.i;
			looseRight.j = view->untransformed_right_edge.j;
			find_line_vector_intersection(&left_point->vector, &right_point->vector, &looseRight, &looseRight);
		} else {
			looseRight.i = right_point->vector.i;
			looseRight.j = right_point->vector.j;
		}
	}
	
	
	long_vector2d *bottom_clip, *top_clip;
	
	if (left_point->vector.i <= right_point->vector.i) {
		// left end is closer
		if (tight_bottom) {
			bottom_clip = (clip_line->bottom < 0) ? &tightLeft : &tightRight;
		} else {
			bottom_clip = (clip_line->bottom < 0) ? &looseLeft : &looseRight;
		}
		if (tight_top) {
			top_clip = (clip_line->top > 0) ? &tightLeft : &tightRight;
		} else {
			top_clip = (clip_line->top > 0) ? &looseLeft : &looseRight;
		}
	} else {
		// left end is farter
		if (tight_bottom) {
			bottom_clip = (clip_line->bottom > 0) ? &tightLeft : &tightRight;
		} else {
			bottom_clip = (clip_line->bottom > 0) ? &looseLeft : &looseRight;
		}
		if (tight_top) {
			top_clip = (clip_line->top < 0) ? &tightLeft : &tightRight;
		} else {
			top_clip = (clip_line->top < 0) ? &looseLeft : &looseRight;
		}
	}
	
	int y;
	if (bottom_clip->i > 0) {
		y = view->half_screen_height - (clip_line->bottom*view->world_to_screen_y)/bottom_clip->i + view->dtanpitch;
		if (y < window->y1) {
			if (y <= window->y0) return true;
			window->y1 = y;
		//	window->bottom.i = bottom_clip->i;
		//	window->bottom.j = clip_line->bottom;
		}
	}
	
	if (top_clip->i > 0) {
		y = view->half_screen_height - (clip_line->top*view->world_to_screen_y)/top_clip->i + view->dtanpitch;
		if (y > window->y0) {
			if (y >= window->y1) return true;
			window->y0 = y;
		//	window->top.i = top_clip->i;
		//	window->top.j = clip_line->top;
		}
	}
	
	if (window->y0 >= window->y1) return true;
	
	return false;
}

static void qsort_windows(clipping_window_data **windows, short *x0s, int count, short partition);
// x0s wil be munged on return.


void NewVisTree::flatten_render_tree() {
	uint16 node = root_node;
	render_node_data *nodes = &Nodes[0];
	
	int nodeCount = Nodes.size();
	uint16 sibling, twin;
	
	int sortedNodeCount = 0;
	
	check_tree_integrity();
	
	do {
continue_outer_loop:// up here instead of at the bottom because of language nuances.
		uint16 next_node;
		
		check_tree_integrity();

		assert (node < nodeCount);
		// check for children if THIS node.
		if (nodes[node].child != UNONE) {
			assert (nodes[node].child < nodeCount);
			node = nodes[node].child;
			check_tree_integrity();
			continue;
		}
		
		assert (PortalViews[nodes[node].portal_view].polygon_node_table[(int)nodes[node].polygon.index()] < nodeCount);
		// if this node proper doesn't have any children, switch to the first node for this poly.
		node = PortalViews[nodes[node].portal_view].polygon_node_table[(int)nodes[node].polygon.index()];
		
		assert (node < nodeCount);
		
		twin = node;
		
		// before we do anything, check if this whole poly is a leaf within this portal_view
		do {
			assert (twin < nodeCount);
			if (nodes[twin].child != UNONE) {
				node = nodes[twin].child;
				assert (node < nodeCount);
				check_tree_integrity();
				goto continue_outer_loop; // lands at top of loop
			}
			twin = nodes[twin].next_node_for_poly;
		} while (twin != UNONE);
		
		check_tree_integrity();
		
		// put this node into the render object list.
		SortedNodes.push_back(sorted_node_data(nodes+node, sortedNodeCount));
		sortedNodeCount++;
		
		if (nodes[node].parent == UNONE) { // root node.
			check_tree_integrity();
			assert(node == root_node);
			assert(nodes[node].sibling == UNONE); // root node is always single child
			break;
		}
		assert(nodes[node].parent < nodeCount);
		
		if (nodes[node].next_node_for_poly == UNONE)
		{
			check_tree_integrity();
			// only have one window, no need to do all this expensive setup & teardown.
			sibling = nodes[nodes[node].parent].child;
			assert (sibling < nodeCount);
			
			// and delink it from the parent's child list.
			if (sibling == node) {
				
				// we're the first child of this branch.
				assert (nodes[node].sibling < nodeCount || nodes[node].sibling == UNONE);
				nodes[nodes[node].parent].child = nodes[node].sibling;
			} else {
				while (nodes[sibling].sibling != node) {
					assert (sibling < nodeCount);
					sibling = nodes[sibling].sibling;
				}
				assert (nodes[sibling].sibling == node);
				assert (nodes[node].sibling < nodeCount || nodes[node].sibling == UNONE);
				nodes[sibling].sibling = nodes[node].sibling;
			}
			next_node = nodes[node].parent;
			nodes[node].parent = UNONE;
			nodes[node].first_clip_window.next_window = UNONE;
			
			check_tree_integrity();
		} else {
			vector<clipping_window_data*> windowsV;
			
			clipping_window_data firstWindow = nodes[node].first_clip_window; // save this because we clobber it later.
			windowsV.push_back(&firstWindow);
			
			int sumx0=0;
			
			
			
			twin = node;
			
			check_tree_integrity();
			
			goto skip_window_push; // just inside the loop.
			do {
				// push the window into the list.
				windowsV.push_back(&nodes[twin].first_clip_window);
				
skip_window_push:
				
				sumx0 += nodes[twin].first_clip_window.x0;
				
				check_tree_integrity();
				
				// get the first child of this twin's parent
				assert(nodes[twin].parent < nodeCount);
				sibling = nodes[nodes[twin].parent].child;
				assert (sibling < nodeCount);
				
				// and delink it from the parent's child list.
				if (sibling == twin) {
					
					// we're the first child of this branch.
					assert (nodes[twin].sibling < nodeCount || nodes[twin].sibling == UNONE);
					nodes[nodes[twin].parent].child = nodes[twin].sibling;
				} else {
					while (nodes[sibling].sibling != twin) {
						assert (sibling < nodeCount);
						sibling = nodes[sibling].sibling;
					}
					assert (nodes[sibling].sibling == twin);
					assert (nodes[twin].sibling < nodeCount || nodes[twin].sibling == UNONE);
					nodes[sibling].sibling = nodes[twin].sibling;
				}
				next_node = nodes[twin].parent;
				nodes[twin].parent = UNONE;
				check_tree_integrity();
				
				twin = nodes[twin].next_node_for_poly;
			} while (twin != UNONE);
			
			check_tree_integrity();
			
			// null-terminate the list to make the optimization logic below simpler.
			windowsV.push_back(NULL);
			
			clipping_window_data **windows = &windowsV[0];
			int windowCount = windowsV.size()-1; // don't sort the null
			
			{
				short *x0s = new short[windowCount];
				for(int i=0; i<windowCount ; i++)
					x0s[i] = windows[i]->x0; // double-dereference these once instead of log2(n) times.
				
				// sort the windows by their left edge.
				qsort_windows(windows, x0s, windowCount, sumx0/(windowCount));
				//x0s are undefined after that, so scope them to this little segment
			}
			
			// copy the first sorted clip window into the node's internal window. (this is why we save it seperately above.)
			nodes[node].first_clip_window = *windows[0];
			
			
			// these are setup for the commented out code.  its is commented to simplify bug location.
			clipping_window_data *work_window = &nodes[node].first_clip_window;
			clipping_window_data *next_window = windows[1];
			
			// build remianing clip windows.
			for(int i=1; 1 ; i++) {
				// set forward link
				// check if we can merge the next window into the current window.
				if (work_window->x0 == next_window->x0 && work_window->x1 == next_window->x1 && next_window->y0 <= work_window->y1) {
					if (work_window->y1 < next_window->y1) {
						work_window->y1 = next_window->y1;
					//	work_window->bottom = next_window->bottom;
					}
					
				} else if (work_window->y0 == next_window->y0 && work_window->y1 == next_window->y1 && next_window->x0 <= work_window->x1) {
					if (work_window->x1 < next_window->x1) {
						work_window->x1 = next_window->x1;
						work_window->right = next_window->right;
					}
				} else {
					work_window->next_window = ClippingWindows.size();
					ClippingWindows.push_back(*next_window);
					work_window = &ClippingWindows[work_window->next_window];
				}
				
				next_window = windows[i+1];
				if (!next_window) break;
				
			}
			exit_reduction_loop: DO_NOTHING;
			
			// end the list of windows
			work_window->next_window = UNONE;
		}
		
		assert (next_node < nodeCount || next_node == UNONE);
		node = next_node; // work up the tree.
		check_tree_integrity();
	} while (1);
	
	assert(nodes[root_node].child == UNONE);
}

void NewVisTree::rebuild_polygon_lookup() {
	portal_view_data *portals = PortalViews[0];
	assert(portals);
	int i=0
	for (i=0 ; i<PortalViews.size ; i++) {
		memset(portals[i].polygon_node_table, -1, PolygonList.size()*sizeof(uint16));
	}
	
	sorted_node_data *snode;
	for(i=0, snode = &SortedNodes.front() ; snode < &PortalViews.back() ; snode++, i++) {
		render_node_data node = snode->node;
		portals[node-->portalView].polygon_node_table[node->polygon] = i;
	}
}

void NewVisTree::check_tree_integrity() {
	return;
	
	render_node_data *nodes = &Nodes[0];
	int node;
	uint16 work, twin;
	int nodeCount = Nodes.size();
				
	for (node = 0 ; node < nodeCount ; node++) {
		// if this is the root node or a orphaned leaf, bail out.
		if (nodes[node].parent == UNONE) continue;
		// sanity check.
		dassert(nodes[node].parent < nodeCount);
		
		// because of the way the tree is built, parent nodes always have lower indicies than their children, check this.
		dassert(nodes[node].parent < nodeCount);
		
		// check that we're in the tree (climb it till we hit the root)
		// don't have to worry about being stuck in loops since we check front-to-back.
		// this is to make sure that we actually get to the root (aren't orphaned) and that we don't have nodes linking back on themselves.
		work = nodes[node].parent;
		while (work < nodeCount && work != root_node && work != node) {
			assert(nodes[work].polygon.index() != nodes[node].polygon.index());
			work = nodes[work].parent;
		}
		dassert(work == root_node);
		assert(nodes[work].polygon.index() != nodes[node].polygon.index());
		
		// check that the nxt node for this poly is really for the same poly
		if (nodes[node].next_node_for_poly != UNONE) {
			dassert(nodes[node].next_node_for_poly < nodeCount);
			dassert(nodes[node].polygon == nodes[nodes[node].next_node_for_poly].polygon);
		}
		
		// check that our parent has us as a child
		work = nodes[nodes[node].parent].child;
		while (work < nodeCount && work != node)
			work = nodes[work].sibling;
		dassert(work == node);
	}
}


static void sort_windows_y(clipping_window_data **windows, int count);

static void qsort_windows(clipping_window_data **windows, short *x0s, int count, short partition) {
	assert (count>1);
	
	if (count==2)
	{
		if ( x0s[0] >  x0s[1] )
		{
			clipping_window_data *temp = windows[0];
			windows[0] = windows[1];
			windows[1] = temp;
		}
		return;
	}
	
	short minlow=INT16_MAX, maxlow=0;
	short minhi=INT16_MAX, maxhi=0, thisx0;
	
	int sortfront = 0, sortback = count-1;
	// ok, case of poorly named variables here, but "sort" is a lot shorter than "partition"
	
	while (sortfront <= sortback)
	{
		// advance through the front side of the list until we hit something we need to move to the back side
		while (thisx0 = x0s[sortfront],
		           thisx0 <= partition  // we implicitly round down when deciding the partition point
		        && sortfront < count)
		{
			assert(windows[sortfront]->x0 == x0s[sortfront]);
			sortfront++;
			if (thisx0 < minlow) minlow = thisx0;
			if (thisx0 > maxlow) maxlow = thisx0;
		}
		
		// reverse through the back end of the list until we find something we need to partition to the front side
		while (thisx0 = x0s[sortback],
		           thisx0 > partition  // we implicitly round down when deciding the partition point
		        && sortback >= 0)
		{
			assert(windows[sortback]->x0 == x0s[sortback]);
			sortback--;
			if (thisx0 < minhi) minhi = thisx0;
			if (thisx0 > maxhi) maxhi = thisx0;
		}
		
		// if they aren't allready cleanly partitioned.
		if (sortfront < sortback)
		{
			// we have items to swap!
			
			clipping_window_data *tempWind = windows[sortfront];
			windows[sortfront] = windows[sortback];
			windows[sortback] = tempWind;
			
			short tempX = x0s[sortfront];
			x0s[sortfront] = x0s[sortback];
			x0s[sortback] = tempX;
			
			assert(windows[sortfront]->x0 == x0s[sortfront]);
			assert(windows[sortback]->x0 == x0s[sortback]);
			
			thisx0 = x0s[sortfront];
			sortfront++;
			if (thisx0 < minlow) minlow = thisx0;
			if (thisx0 > maxlow) maxlow = thisx0;
			
			thisx0 = x0s[sortback];
			sortback--;
			if (thisx0 < minhi) minhi = thisx0;
			if (thisx0 > maxhi) maxhi = thisx0;
		} else {
			// sort points have hit each other, make sure they have passed each other and break out of the loop
			assert (sortfront > sortback);
			break;
		}
	}
	
	if (sortfront > 1)
	{
		assert(minlow <= maxlow);
		
		if (minlow == maxlow)
		{
			sort_windows_y(windows, sortfront);
		} else {
			assert (partition > (minlow + maxlow)/2); // make sure we don't re-call with the same partition, which would be very stupid
			qsort_windows(windows, x0s, sortfront, (minlow + maxlow)/2);
		}
	}
	
	if ((count-sortfront) > 1)
	{
		assert(minhi <= maxhi);
		
		if (minhi == maxhi)
		{
			sort_windows_y(windows+sortfront, (count-sortfront));
		} else {
			assert (partition < (minhi + maxhi)/2); // make sure we don't re-call with the same partition, which would be very stupid
			qsort_windows(windows+sortfront, x0s+sortfront, (count-sortfront), (minhi + maxhi)/2);
		}
	}
	
	#ifdef DEBUG
/*	for (int i=0 ; i<count-1 ; i++) {
		assert (windows[i]->x0 < windows[i+1]->x0
		        || (    windows[i]->x0 == windows[i+1]->x0
		             && windows[i]->y0 <= windows[i+1]->y0));
	}*/
	#endif
}

static void sort_windows_y(clipping_window_data **windows, int count) {
	// just a classic sort.  we're dealing with <10 items even in extreme cases so a qsort wouldn't be woth it
	for (int i=0 ; i<count-1 ; i++,count--) {
		
		short min = windows[i]->y0, max = windows[count-1]->y0;
		int minIndex = i, maxIndex = count-1;
		for (int j=i ; j<count ; j++)
		{
			if (windows[j]->y0 < min)
			{
				minIndex = j;
				min = windows[j]->y0;
			} else if (windows[j]->y0 > max)
			{
				maxIndex = j;
				max = windows[j]->y0;
			}
		}
		
		if (minIndex > i)
		{
			clipping_window_data *temp = windows[i];
			windows[i] = windows[minIndex];
			windows[minIndex] = temp;
		}
		
		if (maxIndex < count-1)
		{
			clipping_window_data *temp = windows[count-1];
			windows[count-1] = windows[maxIndex];
			windows[maxIndex] = temp;
		}
	}
}

















