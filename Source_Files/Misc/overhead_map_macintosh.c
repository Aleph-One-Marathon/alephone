/*
OVERHEAD_MAP_MAC.C
Monday, August 28, 1995 1:41:36 PM  (Jason)

Feb 3, 2000 (Loren Petrich):
	Jjaro-goo color is the same as the sewage color

Feb 4, 2000 (Loren Petrich):
	Changed halt() to assert(false) for better debugging

Jul 8, 2000 (Loren Petrich):
	Added support for OpenGL rendering, in the form of calls to OpenGL versions

Jul 16, 2000 (Loren Petrich):
	Added begin/end pairs for polygons and lines,
	so that caching of them can be more efficient (important for OpenGL)
*/

/* ---------- private code */

#include "OGL_Map.h"

// #define NUMBER_OF_POLYGON_COLORS (sizeof(polygon_colors)/sizeof(RGBColor))

static RGBColor polygon_colors[NUMBER_OF_POLYGON_COLORS]=
{
	{0, 12000, 0},
	{30000, 0, 0},
	{14*256, 37*256, 63*256},
	{76*256, 27*256, 0},
	{70*256, 90*256, 0},
	{70*256, 90*256, 0},	// LP addition: Jjaro goo
	{137*256, 0, 137*256},	// LP: PfhorSlime moved down here
	{32768, 32768, 0}
};


static void begin_overhead_polygons()
{
	// LP addition: do OpenGL
	if (OGL_MapActive)
	{
		OGL_begin_overhead_polygons();
		return;
	}
}

static void end_overhead_polygons()
{
	// LP addition: do OpenGL
	if (OGL_MapActive)
	{
		OGL_end_overhead_polygons();
		return;
	}
}

static void draw_overhead_polygon(
	short vertex_count,
	short *vertices,
	short color,
	short scale)
{
	// LP addition: do OpenGL
	if (OGL_MapActive)
	{
		OGL_draw_overhead_polygon(vertex_count,vertices,polygon_colors[color]);
		return;
	}

	PolyHandle polygon;
	world_point2d *vertex;
	short i;

	(void) (scale);

	assert(color>=0&&color<NUMBER_OF_POLYGON_COLORS);

	polygon= OpenPoly();
	vertex= &get_endpoint_data(vertices[vertex_count-1])->transformed;
	MoveTo(vertex->x, vertex->y);
	for (i=0;i<vertex_count;++i)
	{
		vertex= &get_endpoint_data(vertices[i])->transformed;
		LineTo(vertex->x, vertex->y);
	}
	ClosePoly();
	PenSize(1, 1);
	RGBForeColor(polygon_colors+color);
	FillPoly(polygon, &qd.black);
	KillPoly(polygon);

	return;
}


struct line_definition
{
	RGBColor color;
	short pen_sizes[OVERHEAD_MAP_MAXIMUM_SCALE-OVERHEAD_MAP_MINIMUM_SCALE+1];
};

// #define NUMBER_OF_LINE_DEFINITIONS (sizeof(line_definitions)/sizeof(struct line_definition))
struct line_definition line_definitions[NUMBER_OF_LINE_DEFINITIONS]=
{
	{{0, 65535, 0}, {1, 2, 2, 4}},
	{{0, 40000, 0}, {1, 1, 1, 2}},
	{{65535, 0, 0}, {1, 2, 2, 4}}
};

static void begin_overhead_lines()
{
	// LP addition: do OpenGL
	if (OGL_MapActive)
	{
		OGL_begin_overhead_lines();
		return;
	}
}

static void end_overhead_lines()
{
	// LP addition: do OpenGL
	if (OGL_MapActive)
	{
		OGL_end_overhead_lines();
		return;
	}
}

static void draw_overhead_line(
	short line_index,
	short color,
	short scale)
{
	// LP addition: do OpenGL
	if (OGL_MapActive)
	{
		assert(color>=0&&color<NUMBER_OF_LINE_DEFINITIONS);
		line_definition& LineDef = line_definitions[color];
		OGL_draw_overhead_line(line_index,LineDef.color,LineDef.pen_sizes[scale-OVERHEAD_MAP_MINIMUM_SCALE]);
		return;
	}

	struct line_data *line= get_line_data(line_index);
	world_point2d *vertex1= &get_endpoint_data(line->endpoint_indexes[0])->transformed;
	world_point2d *vertex2= &get_endpoint_data(line->endpoint_indexes[1])->transformed;
	struct line_definition *definition;

	assert(color>=0&&color<NUMBER_OF_LINE_DEFINITIONS);
	definition= line_definitions+color;
	
	RGBForeColor(&definition->color);
	PenSize(definition->pen_sizes[scale-OVERHEAD_MAP_MINIMUM_SCALE], definition->pen_sizes[scale-OVERHEAD_MAP_MINIMUM_SCALE]);
	MoveTo(vertex1->x, vertex1->y);
	LineTo(vertex2->x, vertex2->y);
	
#ifdef RENDER_DEBUG
	if (scale==OVERHEAD_MAP_MAXIMUM_SCALE)
	{
		world_point2d location;
		
		TextFont(kFontIDMonaco);
		TextFace(normal);
		TextSize(9);
		RGBForeColor(&rgb_white);

		location.x= (vertex1->x+vertex2->x)/2;
		location.y= (vertex1->y+vertex2->y)/2;
		psprintf(ptemporary, "%d", line_index);
		MoveTo(location.x, location.y);
		DrawString(temporary);
		
		psprintf(ptemporary, "%d", line->endpoint_indexes[0]);
		MoveTo(vertex1->x, vertex1->y);
		DrawString(temporary);

		psprintf(ptemporary, "%d", line->endpoint_indexes[1]);
		MoveTo(vertex2->x, vertex2->y);
		DrawString(temporary);
	}
#endif

	return;
}

struct thing_definition
{
	RGBColor color;
	short shape;
	short radii[OVERHEAD_MAP_MAXIMUM_SCALE-OVERHEAD_MAP_MINIMUM_SCALE+1];
};

// LP change: Items and monsters were interchanged to get the aliens
// closer to the civilians

struct thing_definition thing_definitions[NUMBER_OF_THINGS]=
{
	{{0, 0, 65535}, _rectangle_thing, {1, 2, 4, 8}}, /* civilian */
	{{65535, 0, 0}, _rectangle_thing, {1, 2, 4, 8}}, /* non-player monster */
	{{65535, 65535, 65535}, _rectangle_thing, {1, 2, 3, 4}}, /* item */
	{{65535, 65535, 0}, _rectangle_thing, {1, 1, 2, 3}}, /* projectiles */
	{{65535, 0, 0}, _circle_thing, {8, 16, 16, 16}}	// LP note: this is for checkpoint locations
};

static void draw_overhead_thing(
	world_point2d *center,
	angle facing,
	short color,
	short scale)
{
	// LP addition: do OpenGL
	if (OGL_MapActive)
	{
		assert(color>=0&&color<NUMBER_OF_THINGS);
		thing_definition& ThingDef = thing_definitions[color];
		OGL_draw_overhead_thing(*center,ThingDef.color,ThingDef.shape,ThingDef.radii[scale-OVERHEAD_MAP_MINIMUM_SCALE]);
		return;
	}
	
	Rect bounds;
	short radius;
	struct thing_definition *definition;

	(void) (facing);

	assert(color>=0&&color<NUMBER_OF_THINGS);
	definition= thing_definitions+color;
	radius= definition->radii[scale-OVERHEAD_MAP_MINIMUM_SCALE];

	RGBForeColor(&definition->color);
	// LP change: adjusted object display so that objects get properly centered;
	// they were inadvertently made 50% too large
	short raddown = short(0.75*radius);
	short radup = short(1.5*radius) - raddown;
	SetRect(&bounds, center->x-raddown, center->y-raddown, center->x+radup, center->y+radup);
//	SetRect(&bounds, center->x-(radius>>1), center->y-(radius>>1), center->x+radius, center->y+radius);
	switch (definition->shape)
	{
		case _rectangle_thing:
			PaintRect(&bounds);
			break;
		case _circle_thing:
			PenSize(2, 2);
			FrameOval(&bounds);
			break;
		default:
			// LP change:
			assert(false);
			// halt();
	}

	return;
}

struct entity_definition
{
	short front, rear, rear_theta;
};

#define NUMBER_OF_ENTITY_DEFINITIONS (sizeof(entity_definitions)/sizeof(struct entity_definition))
struct entity_definition entity_definitions[]=
{
	{16, 10, (7*NUMBER_OF_ANGLES)/20},
	{16, 10, (7*NUMBER_OF_ANGLES)/20},
	{16, 10, (7*NUMBER_OF_ANGLES)/20},
	{16, 10, (7*NUMBER_OF_ANGLES)/20},
	{16, 10, (7*NUMBER_OF_ANGLES)/20},
	{16, 10, (7*NUMBER_OF_ANGLES)/20},
	{16, 10, (7*NUMBER_OF_ANGLES)/20},
	{16, 10, (7*NUMBER_OF_ANGLES)/20},
};

static void draw_overhead_player(
	world_point2d *center,
	angle facing,
	short color,
	short scale)
{
	// LP addition: do OpenGL
	if (OGL_MapActive)
	{
		assert(color>=0&&color<NUMBER_OF_ENTITY_DEFINITIONS);
		RGBColor PlayerColor;
		_get_player_color(color, &PlayerColor);
		entity_definition& EntityDef = entity_definitions[color];
		OGL_draw_overhead_player(*center,facing,PlayerColor,OVERHEAD_MAP_MAXIMUM_SCALE-scale,EntityDef.front,EntityDef.rear,EntityDef.rear_theta);
		return;
	}
	
	short i;
	PolyHandle polygon;
	world_point2d triangle[3];
	struct entity_definition *definition;
	RGBColor rgb_color;

	assert(color>=0&&color<NUMBER_OF_ENTITY_DEFINITIONS);
	definition= entity_definitions+color;

	/* Use our universal clut */
	_get_player_color(color, &rgb_color);

	triangle[0]= triangle[1]= triangle[2]= *center;
	translate_point2d(triangle+0, definition->front>>(OVERHEAD_MAP_MAXIMUM_SCALE-scale), facing);
	translate_point2d(triangle+1, definition->rear>>(OVERHEAD_MAP_MAXIMUM_SCALE-scale), normalize_angle(facing+definition->rear_theta));
	translate_point2d(triangle+2, definition->rear>>(OVERHEAD_MAP_MAXIMUM_SCALE-scale), normalize_angle(facing-definition->rear_theta));
	
	if (scale < 2)
	{
		RGBForeColor(&rgb_color);
		MoveTo(triangle[0].x, triangle[0].y);
		if (triangle[1].x != triangle[0].x || triangle[1].y != triangle[0].y)
			LineTo(triangle[1].x, triangle[1].y);
		else
			LineTo(triangle[2].x, triangle[2].y);
	}
	else
	{
		polygon= OpenPoly();
		MoveTo(triangle[2].x, triangle[2].y);
		for (i=0;i<3;++i) LineTo(triangle[i].x, triangle[i].y);
		ClosePoly();
		PenSize(1, 1);
		RGBForeColor(&rgb_color);
		FillPoly(polygon, &qd.black);
		KillPoly(polygon);
	}	
	
	return;
}

struct annotation_definition
{
	RGBColor color;
	short font, face;
	
	short sizes[OVERHEAD_MAP_MAXIMUM_SCALE-OVERHEAD_MAP_MINIMUM_SCALE+1];
};

#define NUMBER_OF_ANNOTATION_DEFINITIONS (sizeof(annotation_definitions)/sizeof(struct annotation_definition))
struct annotation_definition annotation_definitions[]=
{
	{{0, 65535, 0}, kFontIDMonaco, bold, {5, 9, 12, 18}},
};

static void draw_overhead_annotation(
	world_point2d *location,
	short color,
	char *text,
	short scale)
{
	
	struct annotation_definition *definition;
	Str255 pascal_text;

	vassert(color>=0&&color<NUMBER_OF_ANNOTATION_DEFINITIONS, csprintf(temporary, "#%d is not a supported annotation type", color));
	definition= annotation_definitions+color;
	
	strcpy((char *)pascal_text, text);
	c2pstr((char *)pascal_text);
	
	// LP addition: do OpenGL
	if (OGL_MapActive)
	{
		annotation_definition& NoteDef = *definition;
		OGL_draw_map_text(*location,NoteDef.color,pascal_text,NoteDef.font,NoteDef.face,NoteDef.sizes[scale-OVERHEAD_MAP_MINIMUM_SCALE]);
		return;
	}
	
	MoveTo(location->x, location->y);
	TextFont(definition->font);
	TextFace(definition->face);
	TextSize(definition->sizes[scale-OVERHEAD_MAP_MINIMUM_SCALE]);
	RGBForeColor(&definition->color);
	DrawString(pascal_text);
	
	return;
}

static RGBColor map_name_color= {0, 0xffff, 0};

static void draw_map_name(
	struct overhead_map_data *data,
	char *name)
{
	Str255 pascal_name;
	
	strcpy((char *)pascal_name, name);
	c2pstr((char *)pascal_name);
	
	// LP change: allowed for easier generalization
	const short font = kFontIDMonaco;
	const short face = normal;
	const short size = 18;
	// Need this stuff here so as to calculate the width of the title string
	TextFont(font);
	TextFace(face);
	TextSize(size);
	world_point2d location;
	location.x = data->half_width - (StringWidth(pascal_name)>>1);
	location.y = 25; 
	// LP addition: do OpenGL
	if (OGL_MapActive)
	{
		OGL_draw_map_text(location,map_name_color,pascal_name,font,face,size);
		return;
	}
	RGBForeColor(&map_name_color);
	MoveTo(location.x,location.y);
	/*
	TextFont(kFontIDMonaco);
	TextFace(normal);
	TextSize(18);
	RGBForeColor(&map_name_color);
	MoveTo(data->half_width - (StringWidth(pascal_name)>>1), 25);
	*/
	DrawString(pascal_name);
	
	return;
}


// LP change: MacOS-specific drawing routines moved here

static RGBColor PathColor = {0xffff, 0xffff, 0xffff};

static void SetPathDrawing()
{
	// LP addition: do OpenGL
	if (OGL_MapActive)
	{
		OGL_SetPathDrawing(PathColor);
		return;
	}
	PenSize(1, 1);
	RGBForeColor(&PathColor);
}

static void DrawPath(short step, world_point2d &location)
{
	if (OGL_MapActive)
	{
		OGL_DrawPath(step, location);
		return;
	}
	step ? LineTo(location.x, location.y) : MoveTo(location.x, location.y);
}

static void FinishPath()
{
	if (OGL_MapActive)
	{
		OGL_FinishPath();
		return;
	}
}


// For getting a color from this file to the parser
static void GetParserColor(RGBColor& InColor, rgb_color &OutColor)
{
	OutColor.red = InColor.red;
	OutColor.green = InColor.green;
	OutColor.blue = InColor.blue;
}

// For getting a color from the parser to this file
static void PutParserColor(rgb_color& InColor, RGBColor &OutColor)
{
	OutColor.red = InColor.red;
	OutColor.green = InColor.green;
	OutColor.blue = InColor.blue;
}