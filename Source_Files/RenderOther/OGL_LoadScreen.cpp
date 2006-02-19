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

extern bool OGL_SwapBuffers();

OGL_LoadScreen *OGL_LoadScreen::instance_;

OGL_LoadScreen *OGL_LoadScreen::instance()
{
	if (!instance_) 
		instance_ = new OGL_LoadScreen();
	
	return instance_;
}

bool OGL_LoadScreen::Start()
{
	// load the image
	FileSpecifier File;
	if (path.size() == 0) return use = false;
	if (!File.SetNameWithPath(&path[0])) return use = false;
	if (!image.LoadFromFile(File, ImageLoader_Colors, ImageLoader_ResizeToPowersOfTwo)) return use = false;

	glGenTextures(1, &texture_ref);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture_ref);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.GetWidth(), image.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, image.GetBuffer());
	
	Progress(0);
}

extern bool OGL_ClearScreen();

void OGL_LoadScreen::Stop()
{
	glDeleteTextures(1, &texture_ref);
	OGL_ClearScreen();
}

#if defined(mac)
extern WindowPtr screen_window;
extern void bound_screen();
#endif

void OGL_LoadScreen::Progress(const int progress)
{
 	glMatrixMode(GL_PROJECTION);
 	glLoadIdentity();
	
 	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
 	glLoadIdentity();

	glBindTexture(GL_TEXTURE_2D, texture_ref);
	
	int screenWidth, screenHeight;
#if defined(SDL)
	screenWidth = SDL_GetVideoSurface()->w;
	screenHeight = SDL_GetVideoSurface()->h;
#else
	Rect ScreenRect;
	GetPortBounds(GetWindowPort(screen_window), &ScreenRect);
	screenWidth = ScreenRect.right;
	screenHeight = ScreenRect.bottom;
	bound_screen();
#endif
	
	// the true width/height
	int imageWidth = image.GetWidth() * image.GetVScale();
	int imageHeight = image.GetHeight() * image.GetUScale();

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

	glOrtho(0 - (scaledScreenWidth - imageWidth) / 2, imageWidth + (scaledScreenWidth - imageWidth) / 2, imageHeight + (scaledScreenHeight - imageHeight) / 2, 0 - (scaledScreenHeight - imageHeight) / 2, -1, 1);
	
	glBegin(GL_QUADS);
	glColor3ub(255, 255, 255);

	glTexCoord2f (0.0, 0.0); glVertex3f(0, 0, 0);
	glTexCoord2f (image.GetVScale(), 0.0); glVertex3f(imageWidth, 0, 0);
	glTexCoord2f( image.GetVScale(), image.GetUScale()); glVertex3f(imageWidth, imageHeight, 0);
	glTexCoord2f( 0.0, image.GetUScale()); glVertex3f(0, imageHeight, 0);

	glEnd();

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

	glPopMatrix();

	OGL_SwapBuffers();

	glPopMatrix();

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
	use = false;
	useProgress = false;
	path.clear();
	image.Clear();
}


	
