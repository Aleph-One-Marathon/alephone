/*
	
	Overhead-Map OpenGL Class Implementation
	by Loren Petrich,
	August 3, 2000
	
	Subclass of OverheadMapClass for doing rendering with OpenGL
	Code originally from OGL_Map.c; font handling is still MacOS-specific.

[Notes from OGL_Map.c]
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
[End notes]

Aug 6, 2000 (Loren Petrich):
	Added perimeter drawing to drawing commands for the player object;
	this guarantees that this object will always be drawn reasonably correctly
*/

#include "OverheadMap_OGL.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include <agl.h>
#include <math.h>
#include <string.h>
#include "map.h"

// rgb_color straight to OpenGL
inline void SetColor(rgb_color& Color) {glColor3usv((unsigned short *)(&Color));}

// Need to test this so as to find out when the color changes
inline bool ColorsEqual(rgb_color& Color1, rgb_color& Color2)
{
	return
		((Color1.red == Color2.red) &&
			(Color1.green == Color2.green) &&
				(Color1.blue == Color2.blue));
}

// Render context for aglUseFont(); defined in OGL_Render.c
extern AGLContext RenderContext;

void OverheadMap_OGL_Class::begin_overall()
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
	glLineWidth(1);
}

void OverheadMap_OGL_Class::end_overall()
{
	// Reset line width
	glLineWidth(1);
}


void OverheadMap_OGL_Class::begin_polygons()
{
	// Polygons are rendered before lines, and use the endpoint array,
	// so both of them will have it set here. Using the compiled-vertex extension,
	// however, makes everything the same color :-P
	glVertexPointer(2,GL_SHORT,GetVertexStride(),GetFirstVertex());

	// Reset color defaults
	SavedColor.red = SavedColor.green = SavedColor.blue = 0;
	SetColor(SavedColor);
	
	// Reset cache to zero length
	PolygonCache.ResetLength();
}

void OverheadMap_OGL_Class::draw_polygon(
	short vertex_count,
	short *vertices,
	rgb_color& color)
{
	// Test whether the polygon parameters have changed
	bool AreColorsEqual = ColorsEqual(color,SavedColor);
	
	// If any change, then draw the cached lines with the *old* parameters,
	// Set the new parameters
	if (!AreColorsEqual)
	{
		DrawCachedPolygons();
		SavedColor = color;
		SetColor(SavedColor);
	}
	
	// Implement the polygons as triangle fans
	for (int k=2; k<vertex_count; k++)
	{
		assert(PolygonCache.Add(vertices[0]));
		assert(PolygonCache.Add(vertices[k-1]));
		assert(PolygonCache.Add(vertices[k]));
	}
	
	// glDrawElements(GL_POLYGON,vertex_count,GL_UNSIGNED_SHORT,vertices);
}

void OverheadMap_OGL_Class::end_polygons()
{
	DrawCachedPolygons();
}

void OverheadMap_OGL_Class::DrawCachedPolygons()
{
	glDrawElements(GL_TRIANGLES, PolygonCache.GetLength(),
		GL_UNSIGNED_SHORT, PolygonCache.Begin());
	PolygonCache.ResetLength();
}

void OverheadMap_OGL_Class::begin_lines()
{
	// Vertices already set
	
	// Reset color and pen size to defaults
	SetColor(SavedColor);
	SavedPenSize = 1;
	glLineWidth(SavedPenSize);
	
	// Reset cache to zero length
	LineCache.ResetLength();
}


void OverheadMap_OGL_Class::draw_line(
	short *vertices,
	rgb_color& color,
	short pen_size)
{
	// Test whether the line parameters have changed
	bool AreColorsEqual = ColorsEqual(color,SavedColor);
	bool AreLinesEquallyWide = (pen_size == SavedPenSize);
	
	// If any change, then draw the cached lines with the *old* parameters
	if (!AreColorsEqual || !AreLinesEquallyWide) DrawCachedLines();
	
	// Set the new parameters
	if (!AreColorsEqual)
	{
		SavedColor = color;
		SetColor(SavedColor);
	}
	
	if (!AreLinesEquallyWide)
	{
		SavedPenSize = pen_size;
		glLineWidth(SavedPenSize);		
	}
	
	// Add the line's points to the cached line		
	assert(LineCache.Add(vertices[0]));
	assert(LineCache.Add(vertices[1]));
}

void OverheadMap_OGL_Class::end_lines()
{
	DrawCachedLines();
}

void OverheadMap_OGL_Class::DrawCachedLines()
{
	glDrawElements(GL_LINES,LineCache.GetLength(),
		GL_UNSIGNED_SHORT,LineCache.Begin());
	LineCache.ResetLength();
}


void OverheadMap_OGL_Class::draw_thing(
	world_point2d& center,
	rgb_color& color,
	short shape,
	short radius)
{
	SetColor(color);
	
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
		break;
	}
	glPopMatrix();
}

void OverheadMap_OGL_Class::draw_player(
	world_point2d& center,
	angle facing,
	rgb_color& color,
	short shrink,
	short front,
	short rear,
	short rear_theta)
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
	float scale = 1/float(1 << shrink);
	glScalef(scale,scale,1);
	glVertexPointer(2,GL_FLOAT,0,PlayerShape[0]);
	glDrawArrays(GL_POLYGON,0,3);
	glDrawArrays(GL_LINE_LOOP,0,3);	// LP addition: perimeter drawing makes small version easier to see
	glPopMatrix();
}

	
// Text justification: 0=left, 1=center
void OverheadMap_OGL_Class::draw_text(
	world_point2d& location,
	rgb_color& color,
	char *text,
	FontDataStruct& FontData,
	short which,
	short justify)
{
	// Pascalify the name; watch out for buffer overflows
	Str255 pascal_text;
	strncpy((char *)pascal_text, text, 255);
	c2pstr((char *)pascal_text);
	
	// Needed up here for finding the width of the text string
	TextFont(FontData.font);
	TextFace(FontData.face);
	TextSize(FontData.size);

	// Find the left-side location
	world_point2d left_location = location;
	switch(justify)
	{
	case _justify_left:
		break;
		
	case _justify_center:
		left_location.x -= (StringWidth(pascal_text)>>1);
		break;
		
	default:
		return;
	}
	
	// Set color and location	
	SetColor(color);
	glRasterPos2sv((short *)(&left_location));
	
	FontCacheData& CacheMember = FontCache[which];
	
	// Is an update of the font info desired?
	bool RequestUpdate = !CacheMember.WasInited;
	if (!RequestUpdate)
		RequestUpdate = (CacheMember.FontData != FontData);
	
	// If so, then do it
	if (RequestUpdate)
	{
		CacheMember.FontData = FontData;
		CacheMember.Update();
	}
	
	for (int b=1; b<=pascal_text[0]; b++)
	{
		GLuint ByteDisplayList = CacheMember.DispList + pascal_text[b];
		glCallLists(1,GL_UNSIGNED_INT,&ByteDisplayList);
	}
}
	
void OverheadMap_OGL_Class::set_path_drawing(rgb_color& color)
{
	SetColor(color);
	glLineWidth(1);
}

void OverheadMap_OGL_Class::draw_path(
	short step,	// 0: first point
	world_point2d &location)
{
	// At first step, reset the length
	if (step <= 0) PathPoints.ResetLength();
	
	// Add the point
	assert(PathPoints.Add(location));
}

void OverheadMap_OGL_Class::finish_path()
{
	glVertexPointer(2,GL_SHORT,sizeof(world_point2d),PathPoints.Begin());
	glDrawArrays(GL_LINE_STRIP,0,PathPoints.GetLength());
}


// Font-cache handling

// This is for updating the font display list when the font info has changed
void FontCacheData::Update()
{
	if (WasInited)
	{
		glDeleteLists(DispList,256);
	} else {
		WasInited = true;
	}
	
	DispList = glGenLists(256);
	aglUseFont(RenderContext, FontData.font, FontData.face, FontData.size, 0, 256, DispList);
}

// This is for clearing the font display list
void FontCacheData::Clear()
{
	if (WasInited)
	{
		glDeleteLists(DispList,256);
		WasInited = false;
	}
}

// This is for resetting the fonts
void OverheadMap_OGL_Class::ResetFonts()
{
	for (int ic=0; ic<FONT_CACHE_SIZE; ic++)
		FontCache[ic].Clear();
}
