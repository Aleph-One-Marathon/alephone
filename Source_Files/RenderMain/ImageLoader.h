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

	uint32 *Pixels;
	int Size;

	int MipMapCount;
	
public:
	
	bool IsPresent() {return (Pixels != NULL); }

	bool LoadFromFile(FileSpecifier& File, int ImgMode, int flags, int maxSize = 0);

	// Size of level 0 image
	int GetWidth() const {return Width;}
	int GetHeight() const {return Height;}
	int GetNumPixels() const {return Width*Height;}

	int GetMipMapCount() const { return MipMapCount; }
	int GetTotalBytes() const { return Size; }
	int GetBufferSize() const { return Size; }
	int GetFormat() const { return Format; }

	int GetOriginalWidth() const { return OriginalWidth; }
	int GetOriginalHeight() const { return OriginalHeight; }

	// Pixel accessors
//	uint32& GetPixel(int Horiz, int Vert) {return Pixels[Width*Vert + Horiz];}
	uint32 *GetPixelBasePtr() {return Pixels;}
	const uint32 *GetBuffer() const { return Pixels; }
	uint32 *GetBuffer() { return Pixels; }

	uint32 *GetMipMapPtr(int Level);
	
	// Reallocation
	void Resize(int _Width, int _Height);
	void Original(int _Width, int _Height)
	{OriginalWidth = _Width, OriginalHeight = _Height; }

	// mipmappy operations
	void Resize(int _Width, int _Height, int _TotalBytes);

	// Clearing
	void Clear()
		{Width = Height = Size = 0; delete []Pixels; Pixels = NULL;}

	ImageDescriptor(const ImageDescriptor &CopyFrom);
	
	ImageDescriptor(): Width(0), Height(0), OriginalWidth(0), OriginalHeight(0), Pixels(NULL), Size(0) {}

	// asumes RGBA8
	ImageDescriptor(int width, int height, uint32 *pixels);

	enum ImageFormat {
		RGBA8,
		DXTC1,
		DXTC3,
		DXTC5
	};

	~ImageDescriptor()
		{
			delete []Pixels;
			Pixels = NULL;
		}
			
			

private:
	bool LoadDDSFromFile(FileSpecifier& File, int flags, int maxSize = 0);

	ImageFormat Format;
};

template <typename T>
class copy_on_edit
{
public:
	copy_on_edit() : _original(NULL), _copy(NULL) { };

	void set(const T* original) {
		if (_copy) {
			delete _copy;
			_copy = NULL;
		}
		_original = original;
	}


	const T* get() {
		if (_copy)
			return (const T*) _copy;
		else
			return (const T*) _original;
	}

	T* edit() {
		if (!_original) {
			return NULL;
		} else {
			if (_copy) {
				_copy = new T(*_original);
			}
			return _copy;
		}
	}

	// takes possession of copy
	T* edit(T* copy) {
		if (_copy) {
			delete _copy;
		}
		_original = NULL;
		_copy = copy;
	}

	~copy_on_edit() {
		if (_copy) {
			delete _copy;
			_copy = NULL;
		}
	}

private:
	T* _original;
	T* _copy;
};

typedef copy_on_edit<ImageDescriptor> ImageDescriptorCache;
		

// What to load: image colors (must be loaded first)
// or image opacity (replaces the default, which is 100% opaque everywhere).
// The image-opacity image must have the same size as the color image;
// it is interpreted as a grayscale image.
enum {
	ImageLoader_Colors,
	ImageLoader_Opacity
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
