#ifndef _IMAGE_LOADER_
#define _IMAGE_LOADER_
/*

	Copyright (C) 1991-2001 and beyond by Bungie Studios, Inc.
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
	
	Image-Loader Interface File,
	by Loren Petrich,
	October 21, 2000

	This file contains an image-descriptor object and a function for loading the image
	from a file.
	
*/

#include <vector>
#include "cseries.h"
#include "FileHandler.h"
using namespace std;


// Need an object to hold the read-in image.
class ImageDescriptor
{
	int Width;	// along scanlines
	int Height;	// scanline to scanline
	vector<uint32> Pixels;	// in 32-bit format
public:
	
	// Is an image present?
	bool IsPresent() {return !Pixels.empty();}
	
	// Size
	int GetWidth() {return Width;}
	int GetHeight() {return Height;}
	int GetNumPixels() {return Width*Height;}
	
	// Pixel accessors
	uint32& GetPixel(int Horiz, int Vert) {return Pixels[Width*Vert + Horiz];}
	uint32 *GetPixelBasePtr() {return &Pixels[0];}
	
	// Reallocation
	void Resize(int _Width, int _Height)
		{Width = _Width, Height = _Height, Pixels.resize(GetNumPixels());}
	
	// Clearing
	void Clear()
		{Width = Height = 0; Pixels.clear();}
	
	ImageDescriptor(): Width(0), Height(0) {}
};

// What to load: image colors (must be loaded first)
// or image opacity (replaces the default, which is 100% opaque everywhere).
// The image-opacity image must have the same size as the color image;
// it is interpreted as a grayscale image.
enum {
	ImageLoader_Colors,
	ImageLoader_Opacity
};

// Returns whether or not the loading was successful
bool LoadImageFromFile(ImageDescriptor& Img, FileSpecifier& File, int ImgMode);


#endif
