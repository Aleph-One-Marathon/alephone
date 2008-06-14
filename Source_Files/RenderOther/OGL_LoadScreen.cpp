/*

	Copyright (C) 2006 and beyond by Bungie Studios, Inc.
	and the "Aleph One" developers.
 
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
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

#include "OGL_LoadScreen.h"
#include "screen.h"

#ifdef HAVE_OPENGL

extern bool OGL_SwapBuffers();

OGL_LoadScreen *OGL_LoadScreen::instance_;

OGL_LoadScreen *OGL_LoadScreen::instance()
{
	if (!instance_) 
		instance_ = new OGL_LoadScreen();
	
	return instance_;
}

extern bool OGL_ClearScreen();

#if defined(mac)
extern WindowPtr screen_window;
#endif
extern void bound_screen();

bool OGL_LoadScreen::Start()
{
	// load the image
	FileSpecifier File;
	if (path.size() == 0) return use = false;
	if (!File.SetNameWithPath(&path[0])) return use = false;
	if (!image.LoadFromFile(File, ImageLoader_Colors, 0)) return use = false;

#ifdef ALEPHONE_LITTLE_ENDIAN
	SDL_Surface *s = SDL_CreateRGBSurfaceFrom(image.GetBuffer(), image.GetWidth(), image.GetHeight(), 32, image.GetWidth() * 4, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
#else
	SDL_Surface *s = SDL_CreateRGBSurfaceFrom(image.GetBuffer(), image.GetWidth(), image.GetHeight(), 32, image.GetWidth() * 4, 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);
#endif

	if (blitter)
		delete blitter;

	int screenWidth, screenHeight;
#if defined(SDL)
	screenWidth = SDL_GetVideoSurface()->w;
	screenHeight = SDL_GetVideoSurface()->h;
#else
	Rect ScreenRect;
	GetPortBounds(GetWindowPort(screen_window), &ScreenRect);
	screenWidth = ScreenRect.right;
	screenHeight = ScreenRect.bottom;
#endif
	bound_screen();
	
	// the true width/height
	int imageWidth = static_cast<int>(image.GetWidth() * image.GetVScale());
	int imageHeight = static_cast<int>(image.GetHeight() * image.GetUScale());

	int scaledScreenWidth, scaledScreenHeight;

	if (stretch)
	{
		scaledScreenWidth = imageWidth;
		scaledScreenHeight = imageHeight;
	}
	else if (imageWidth / imageHeight > screenWidth / screenHeight) 
	{
		scaledScreenWidth = imageWidth;
		scaledScreenHeight = screenHeight * imageWidth / screenWidth;
	} 
	else 
	{
		scaledScreenWidth = screenWidth * imageHeight / screenHeight;
		scaledScreenHeight = imageHeight;
	}

	SDL_Rect dst = { 0, 0, imageWidth, imageHeight };
	SDL_Rect ortho = { 0 - (scaledScreenWidth - imageWidth) / 2,
			   0 - (scaledScreenHeight - imageHeight) / 2,
			   scaledScreenWidth,
			   scaledScreenHeight};
	
		
	blitter = new OGL_Blitter(*s, dst, ortho);
						  
	OGL_ClearScreen();
	
	Progress(0);

	return use = true;
}

void OGL_LoadScreen::Stop()
{
	OGL_ClearScreen();
	OGL_SwapBuffers();
	Clear();
}

void OGL_LoadScreen::Progress(const int progress)
{

	OGL_ClearScreen();

	blitter->SetupMatrix();
	blitter->Draw();

	if (useProgress) 
	{

		// draw the progress bar background
		glBindTexture(GL_TEXTURE_2D, 0);
		glColor3us(colors[0].red, colors[0].green, colors[0].blue);
		
		glBegin(GL_QUADS);
		glVertex3f(x, y, 0);
		glVertex3f(x + w, y, 0);
		glVertex3f(x + w, y + h, 0);
		glVertex3f(x, y + h, 0);
		
		glEnd();
		
		int height = h, width = w;
		int left = x, top = y;
		if (height > width) 
		{
			top = top + height - height * progress / 100;
			height = height * progress / 100;
		}
		else 
		{
			width = width * progress / 100;
		}
			
		// draw the progress bar foreground
		glBindTexture(GL_TEXTURE_2D, 0);
		glColor3us(colors[1].red, colors[1].green, colors[1].blue);
		glBegin(GL_QUADS);
		glVertex3f(left, top, 0);
		glVertex3f(left + width, top, 0);
		glVertex3f(left + width, top + height, 0);
		glVertex3f(left, top + height, 0);
		glEnd();
	}

	blitter->RestoreMatrix();

	OGL_SwapBuffers();

}

void OGL_LoadScreen::Set(const vector<char> Path, bool Stretch)
{
	OGL_LoadScreen::Set(Path, Stretch, 0, 0, 0, 0);
	useProgress = false;
}

void OGL_LoadScreen::Set(const vector<char> Path, bool Stretch, short X, short Y, short W, short H)
{
	path = Path;
	x = X;
	y = Y;
	w = W;
	h = H;
	stretch = Stretch;

	image.Clear();
	use = true;
	useProgress = true;
	percent = 0;
}

void OGL_LoadScreen::Clear()
{
	OGL_ClearScreen();
	OGL_SwapBuffers();
	OGL_ClearScreen();

	use = false;
	useProgress = false;
	path.clear();
	image.Clear();
	if (blitter)
	{
		delete blitter;
		blitter = NULL;
	}
}

#endif
