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
	int Width;	    // along scanlines
	int Height;	    // scanline to scanline
	int OriginalWidth;  // before powers of two resize
	int OriginalHeight;
	vector<uint32> Pixels;	// in 32-bit format

	int NumMipMaps;
	int Format;
public:
	
	bool IsPresent() {return !Pixels.empty();}

	bool LoadFromFile(FileSpecifier& File, int ImgMode, int flags, int maxSize = 0);

	// Size of level 0 image
	int GetWidth() {return Width;}
	int GetHeight() {return Height;}
	int GetNumPixels() {return Width*Height;}

	int GetNumMipMaps() { return NumMipMaps; }
	int GetTotalBytes() { return Pixels.size() * 4; }
	int GetFormat() { return Format; }

	void SetFormat(int _Format) { Format = _Format; }
	void SetNumMipMaps(int _NumMipMaps) { NumMipMaps = _NumMipMaps; }

	int GetOriginalWidth() { return OriginalWidth; }
	int GetOriginalHeight() { return OriginalHeight; }

	// Pixel accessors
	uint32& GetPixel(int Horiz, int Vert) {return Pixels[Width*Vert + Horiz];}
	uint32 *GetPixelBasePtr() {return &Pixels[0];}

	uint32 *GetMipMapPtr(int Level);
	
	// Reallocation
	void Resize(int _Width, int _Height)
		{Width = _Width, Height = _Height, Pixels.resize(GetNumPixels());}
	void Original(int _Width, int _Height)
	{OriginalWidth = _Width, OriginalHeight = _Height; }

	// mipmappy operations
	void Resize(int _Width, int _Height, int _TotalBytes)
	{ Width = _Width, Height = _Height, Pixels.resize(_TotalBytes * 4); }

	// Clearing
	void Clear()
		{Width = Height = 0; Pixels.clear();}
	
	ImageDescriptor(): Width(0), Height(0), OriginalWidth(0), OriginalHeight(0) {}

private:
	bool LoadDDSFromFile(FileSpecifier& File, int flags, int maxSize = 0);
};

// What to load: image colors (must be loaded first)
// or image opacity (replaces the default, which is 100% opaque everywhere).
// The image-opacity image must have the same size as the color image;
// it is interpreted as a grayscale image.
enum {
	ImageLoader_Colors,
	ImageLoader_Opacity
};

enum {
	ImageFormat_RGBA,
	ImageFormat_DXTC1,
	ImageFormat_DXTC3,
	ImageFormat_DXTC5
};

enum {
	ImageLoader_ResizeToPowersOfTwo = 0x1,
	ImageLoader_CanUseDXTC = 0x2,
	ImageLoader_LoadMipMaps = 0x4
};
// Returns whether or not the loading was successful
//bool LoadImageFromFile(ImageDescriptor& Img, FileSpecifier& File, int ImgMode, int flags, int maxSize = 0);

uint32 *GetMipMapPtr(uint32 *pixels, int size, int level, int width, int height, int format);

#endif
