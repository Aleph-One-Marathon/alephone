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
*/

#include <GL/gl.h>
#include <GL/glu.h>
#include <agl.h>
#include <math.h>
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
}

void OGL_EndMap()
{
	// Reset line width
	glLineWidth(1);
}

void OGL_draw_overhead_polygon(short vertex_count, short *vertices,
	RGBColor &color)
{
	SetColor(color);
	glVertexPointer(2,GL_SHORT,sizeof(endpoint_data),&get_endpoint_data(0)->transformed);
	glDrawElements(GL_POLYGON,vertex_count,GL_UNSIGNED_SHORT,vertices);
}

void OGL_draw_overhead_line(short line_index, RGBColor &color,
	short pen_size)
{
	SetColor(color);
	glLineWidth(pen_size);
	glVertexPointer(2,GL_SHORT,sizeof(endpoint_data),&get_endpoint_data(0)->transformed);
	struct line_data *line= get_line_data(line_index);
	glDrawElements(GL_LINES,2,GL_UNSIGNED_SHORT,line->endpoint_indexes);
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

void OGL_SetPathDrawing(RGBColor &color)
{
	SetColor(color);
	glLineWidth(1);
}

void OGL_DrawPath(short step, world_point2d &location)
{
	// Save previous location
	static world_point2d prev_location;
	
	// One-at-a-time drawing if step is nonzero
	if (step)
	{
		glBegin(GL_LINES);
		glVertex2sv((short *)(&prev_location));
		glVertex2sv((short *)(&location));
		glEnd();
	}
	prev_location = location;
}
