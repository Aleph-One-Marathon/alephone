/*
	
	OpenGL Map Renderer,
	by Loren Petrich,
	July 8, 2000

	This is for drawing the Marathon overhead map with OpenGL, because at large resolutions,
	the main CPU can be too slow for this.
	
	Much of this is cribbed from overhead_map_macintosh.c and translated into OpenGL form
	
July 9, 2000:

	Complete this OpenGL renderer. I had to add a font-info cache, so as to avoid
	re-generating the fonts for every frame. The font glyphs and offsets are stored
	as display lists, which appears to be very efficient.

July 16, 2000:

	Added begin/end pairs for line and polygon rendering; the purpose of these is to allow
	more efficient caching.

Jul 17, 2000:

	Paths now cached and drawn as a single line strip per path.
	Lines now cached and drawn in groups with the same width and color;
		that has yielded a significant performance improvement.
	Same for the polygons, but with relatively little improvement.
*/

#include <GL/gl.h>
#include <GL/glu.h>
#include <agl.h>
#include <math.h>
#include "GrowableList.h"
#include "map.h"
#include "OGL_Map.h"


// RGBColor straight to OpenGL
inline void SetColor(RGBColor& Color) {glColor3usv((unsigned short *)(&Color));}

// Render context for aglUseFont(); defined in OGL_Render.c
extern AGLContext RenderContext;

// Font info: all the info needed to construct a font
struct FontInfoData
{
	short font, face, size;
	
	// Equality tester (the == operator)
	bool operator==(FontInfoData &FI) {return ((font==FI.font) && (face==FI.face) && (size==FI.size));}
	
	// Constructor
	FontInfoData() {font=face=size=0;}
	FontInfoData(short _font, short _face, short _size) {font=_font, face=_face, size=_size;}
};

// Font-cache data, so fonts don't have to be repeatedly re-uploaded
struct FontCacheData
{
	FontInfoData Info;		// Info used to construct the font
	GLuint DispList;	// Display list for fonts
	long Timestamp;		// Timestamp for last use (for LRU management)
	bool WasInited;		// Was this dataset inited?
	
	// Build font display list
	void Build();
	
	FontCacheData() {WasInited = false;}
};

void FontCacheData::Build()
{
	if (WasInited)
	{
		glDeleteLists(DispList,256);
	} else {
		WasInited = true;
	}
	
	DispList = glGenLists(256);
	aglUseFont(RenderContext, Info.font, Info.face, Info.size, 0, 256, DispList);
}

const int NumFontCacheEntries = 8;
static FontCacheData FontCacheList[NumFontCacheEntries];

static long FontUse_Timestamp = 0;

void OGL_ResetMapFonts()
{
	for (int ic=0; ic<NumFontCacheEntries; ic++)
		FontCacheList[ic].WasInited = false;
}


void OGL_StartMap()
{
	// Blank out the screen
	glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT);
	
	// Here's for the overhead map
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_FOG);
}

void OGL_EndMap()
{
	// Reset line width
	glLineWidth(1);
}


// Need to test this so as to find out when the color changes
inline bool ColorsEqual(RGBColor& Color1, RGBColor& Color2)
{
	return ((Color1.red == Color2.red) && (Color1.green == Color2.green) && (Color1.blue == Color2.blue));
}

// Stuff for caching polygons to be drawn;
// several polygons with the same color will be drawn at the same time
static RGBColor MapSavedColor = {0, 0, 0};
static GrowableList<unsigned short> MapPolygonCache(16);

static void DrawCachedPolygons()
{
	glDrawElements(GL_TRIANGLES,MapPolygonCache.GetLength(),GL_UNSIGNED_SHORT,MapPolygonCache.Begin());
	MapPolygonCache.ResetLength();
}

void OGL_begin_overhead_polygons()
{
	// Polygons are rendered before lines, and use the endpoint array,
	// so both of them will have it set here. Using the compiled-vertex extension,
	// however, makes everything the same color :-P
	glVertexPointer(2,GL_SHORT,sizeof(endpoint_data),&get_endpoint_data(0)->transformed);

	// Reset color defaults
	SetColor(MapSavedColor);
	
	// Reset cache to zero length
	MapPolygonCache.ResetLength();
}

void OGL_end_overhead_polygons()
{
	DrawCachedPolygons();
}

void OGL_draw_overhead_polygon(short vertex_count, short *vertices,
	RGBColor &color)
{
	// Test whether the polygon parameters have changed
	bool AreColorsEqual = ColorsEqual(color,MapSavedColor);
	
	// If any change, then draw the cached lines with the *old* parameters
	if (!AreColorsEqual) DrawCachedPolygons();
	
	// Set the new parameters
	if (!AreColorsEqual)
	{
		MapSavedColor = color;
		SetColor(MapSavedColor);
	}
	
	// Implement the polygons as triangle fans
	for (int k=2; k<vertex_count; k++)
	{
		assert(MapPolygonCache.Add(vertices[0]));
		assert(MapPolygonCache.Add(vertices[k-1]));
		assert(MapPolygonCache.Add(vertices[k]));
	}
	
	// glDrawElements(GL_POLYGON,vertex_count,GL_UNSIGNED_SHORT,vertices);
}


// Stuff for caching lines to be drawn;
// several lines with the same thickness and color will be drawn at the same time
static short MapSavedPenSize = 1;
static GrowableList<unsigned short> MapLineCache(16);

static void DrawCachedLines()
{
	glDrawElements(GL_LINES,MapLineCache.GetLength(),GL_UNSIGNED_SHORT,MapLineCache.Begin());
	MapLineCache.ResetLength();
}

void OGL_begin_overhead_lines()
{
	// Vertices already set
	
	// Reset color and pen size to defaults
	SetColor(MapSavedColor);
	glLineWidth(MapSavedPenSize);
	
	// Reset cache to zero length
	MapLineCache.ResetLength();
}

void OGL_end_overhead_lines()
{
	DrawCachedLines();
}

void OGL_draw_overhead_line(short line_index, RGBColor &color,
	short pen_size)
{
	// Test whether the line parameters have changed
	bool AreColorsEqual = ColorsEqual(color,MapSavedColor);
	bool AreLinesEquallyWide = (pen_size == MapSavedPenSize);
	
	// If any change, then draw the cached lines with the *old* parameters
	if (!AreColorsEqual || !AreLinesEquallyWide) DrawCachedLines();
	
	// Set the new parameters
	if (!AreColorsEqual)
	{
		MapSavedColor = color;
		SetColor(MapSavedColor);
	}
	
	if (!AreLinesEquallyWide)
	{
		MapSavedPenSize = pen_size;
		glLineWidth(MapSavedPenSize);		
	}
	
	// Add the line's points to the cached line		
	short *Indices = get_line_data(line_index)->endpoint_indexes;
	assert(MapLineCache.Add(Indices[0]));
	assert(MapLineCache.Add(Indices[1]));
}


void OGL_draw_overhead_thing(world_point2d &center, RGBColor &color,
	short shape, short radius)
{
	// Cribbed from overhead_map.c
	enum
	{
		_rectangle_thing,
		_circle_thing
	};
	
	// The rectangle is a square
	const int NumRectangleVertices = 4;
	const GLfloat RectangleShape[NumRectangleVertices][2] =
	{
		{-0.75,-0.75},
		{-0.75,0.75},
		{0.75,0.75},
		{0.75,-0.75}
	};
	// The circle is here an octagon for convenience
	const int NumCircleVertices = 8;
	const GLfloat CircleShape[NumCircleVertices][2] =
	{
		{-0.75,-0.3},
		{-0.75,0.3},
		{-0.3,0.75},
		{0.3,0.75},
		{0.75,0.3},
		{0.75,-0.3},
		{0.3,-0.75},
		{-0.3,-0.75}
	};
	
	SetColor(color);
	// Let OpenGL do the transformation work
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(center.x,center.y,0);
	glScalef(radius,radius,1);
	switch(shape)
	{
	case _rectangle_thing:
		glVertexPointer(2,GL_FLOAT,0,RectangleShape[0]);
		glDrawArrays(GL_POLYGON,0,NumRectangleVertices);
		break;
	case _circle_thing:
		glLineWidth(2);
		glVertexPointer(2,GL_FLOAT,0,CircleShape[0]);
		glDrawArrays(GL_LINE_LOOP,0,NumCircleVertices);
		break;
	default:
		// LP change:
		assert(false);
		// halt();
	}
	glPopMatrix();
}

void OGL_draw_overhead_player(world_point2d &center, angle facing, RGBColor &color,
	short reduce, short front, short rear, short rear_theta)
{
	SetColor(color);
	
	// The player is a simple triangle
	GLfloat PlayerShape[3][2];
	
	double rear_theta_rads = rear_theta*(8*atan(1.0)/FULL_CIRCLE);
	float rear_x = rear*cos(rear_theta_rads);
	float rear_y = rear*sin(rear_theta_rads);
	PlayerShape[0][0] = front;
	PlayerShape[0][1] = 0;
	PlayerShape[1][0] = rear_x;
	PlayerShape[1][1] = - rear_y;
	PlayerShape[2][0] = rear_x;
	PlayerShape[2][1] = rear_y;
	
	// Let OpenGL do the transformation work
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(center.x,center.y,0);
	glRotatef(facing*(360.0/FULL_CIRCLE),0,0,1);
	float scale = 1/float(1 << reduce);
	glScalef(scale,scale,1);
	glVertexPointer(2,GL_FLOAT,0,PlayerShape[0]);
	glDrawArrays(GL_POLYGON,0,3);
	glPopMatrix();
}

void OGL_draw_map_text(world_point2d &location, RGBColor &color,
	unsigned char *text_pascal, short font, short face, short size)
{
	SetColor(color);
	glRasterPos2sv((short *)(&location));
	
	// Create this nice little object
	FontInfoData Info(font,face,size);
	GLuint MapFontDisplayList = 0;
	
	// Increase timestamp value; this is used for LRU updating
	FontUse_Timestamp++;
	
	// Look for one with equal font info
	bool EqualFound = false;
	for (int ic=0; ic<NumFontCacheEntries; ic++)
	{
		FontCacheData& Entry = FontCacheList[ic];
		if (Entry.WasInited && Entry.Info == Info)
		{
			EqualFound = true;
			Entry.Timestamp = FontUse_Timestamp;
			MapFontDisplayList = Entry.DispList;
			break;
		}
	}
	
	// If no equal ones were found, then do 
	if (!EqualFound)
	{
		bool UnusedFound = false;
		for (int ic=0; ic<NumFontCacheEntries; ic++)
		{
			FontCacheData& Entry = FontCacheList[ic];
			if (!Entry.WasInited)
			{
				UnusedFound = true;
				Entry.Info = Info;
				Entry.Build();
				Entry.Timestamp = FontUse_Timestamp;
				MapFontDisplayList = Entry.DispList;
				break;
			}
		}
		
		// If no equal ones were found, then do a LRU replacement
		if (!UnusedFound)
		{
			long LowestTime = FontUse_Timestamp;
			int IndexToReplace = 0;
			for (int ic=0; ic<NumFontCacheEntries; ic++)
			{
				FontCacheData& Entry = FontCacheList[ic];
				if (Entry.Timestamp <= LowestTime)
				{
					LowestTime = Entry.Timestamp;
					IndexToReplace = ic;
				}
			}
			FontCacheData& Entry = FontCacheList[IndexToReplace];
			Entry.Info = Info;
			Entry.Build();
			Entry.Timestamp = FontUse_Timestamp;
			MapFontDisplayList = Entry.DispList;
		}
	}
	
	for (int b=1; b<=text_pascal[0]; b++)
	{
		GLuint ByteDisplayList = MapFontDisplayList + text_pascal[b];
		glCallLists(1,GL_UNSIGNED_INT,&ByteDisplayList);
	}
}

// For drawing monster paths
static GrowableList<world_point2d> MapPathPoints(16);

void OGL_SetPathDrawing(RGBColor &color)
{
	SetColor(color);
	glLineWidth(1);
}

void OGL_DrawPath(short step, world_point2d &location)
{
	// At first step, reset the length
	if (step <= 0) MapPathPoints.ResetLength();
	
	// Add the point
	assert(MapPathPoints.Add(location));
}

void OGL_FinishPath()
{
	glVertexPointer(2,GL_SHORT,sizeof(world_point2d),&(MapPathPoints.Begin()->x));
	glDrawArrays(GL_LINE_STRIP,0,MapPathPoints.GetLength());
}
