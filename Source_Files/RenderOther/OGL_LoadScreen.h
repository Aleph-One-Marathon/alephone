#ifndef _OGL_LOADSCREEN_
#define _OGL_LOADSCREEN_
/*

	Copyright (C) 2006 and beyond by Bungie Studios, Inc.
	and the "Aleph One" developers.
 
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	This license is contained in the file "COPYING",
	which is included with this source code; it is available online at
	http://www.gnu.org/licenses/gpl.html
	
	OpenGL load screens
	written by Gregory Smith, 2006
*/

#include "cseries.h"

#ifdef HAVE_OPENGL
#include "OGL_Headers.h"
#endif

#include "OGL_Blitter.h"
#include "ImageLoader.h"

#ifdef HAVE_OPENGL
class OGL_LoadScreen
{
public:
	static OGL_LoadScreen *instance();
	
	bool Start();
	void Stop();
	void Progress(const int percent);

	void Set(std::string Path, bool Stretch, bool Scale);
	void Set(std::string Path, bool Stretch, bool Scale, short X, short Y, short W, short H);
	void Clear();

	bool Use() { return use; }

	rgb_color *Colors() { return colors; }

private:
OGL_LoadScreen() : x(0), y(0), w(0), h(0), use(false), useProgress(false), percent(0) { }
	~OGL_LoadScreen();

	std::string path;
	ImageDescriptor image;
	short x, y, w, h;

	OGL_Blitter blitter;
	SDL_Rect m_dst;
	double x_offset, y_offset, x_scale, y_scale;

	bool scale;
	bool stretch;

	bool use;
	bool useProgress;

	rgb_color colors[2];

	short percent;


	GLuint texture_ref;
};
#endif

#endif
