#ifndef _OGL_MAP_
#define _OGL_MAP_
/*
	
	OpenGL Map Interface File,
	by Loren Petrich,
	July 8, 2000

	This is for drawing the Marathon overhead map with OpenGL, because at large resolutions,
	the main CPU can be too slow for this.
	
	Note, all the context set-up and blit-to-screen is specified in OGL_Render.c/h
*/

// Special stuff for starting and ending a frame
void OGL_StartMap();
void OGL_EndMap();

// For resetting the font-cache info; useful when restarting an OpenGL context
void OGL_ResetMapFonts();

// These are cribbed from overhead_map_macintosh.c

void OGL_draw_overhead_polygon(short vertex_count, short *vertices,
	RGBColor &color);
void OGL_draw_overhead_line(short line_index, RGBColor &color,
	short pen_size);
void OGL_draw_overhead_thing(world_point2d &center, RGBColor &color,
	short shape, short radius);
void OGL_draw_overhead_player(world_point2d &center, angle facing, RGBColor &color,
	short reduce, short front, short rear, short rear_theta);
void OGL_draw_map_text(world_point2d &location, RGBColor &color,
	unsigned char *text_pascal, short font, short face, short size);
void OGL_SetPathDrawing(RGBColor &color);
void OGL_DrawPath(short step, world_point2d &location);

#endif
