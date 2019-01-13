#ifndef _IMAGE_LOADER_
#define _IMAGE_LOADER_
/*

	Copyright (C) 1991-2001 and beyond by Bungie Studios, Inc.
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
	
	Image-Loader Interface File,
	by Loren Petrich,
	October 21, 2000

	This file contains an image-descriptor object and a function for loading the image
	from a file.
	
*/

#include "DDS.h"
#include <vector>
#include "cseries.h"
#include "FileHandler.h"

// Need an object to hold the read-in image.
class ImageDescriptor
{
	int Width;	    // along scanlines
	int Height;	    // scanline to scanline

	double VScale;
	double UScale;

	uint32 *Pixels;
	int Size;

	int MipMapCount;
	
public:
	
	bool IsPresent() const noexcept {return (Pixels != nullptr); }
	bool IsPremultiplied() const noexcept { return (IsPresent() ? PremultipliedAlpha : false); }

	bool LoadFromFile(FileSpecifier& File, int ImgMode, int flags, int actual_width = 0, int actual_height = 0, int maxSize = 0);

	// Size of level 0 image
	int GetWidth() const noexcept {return Width;}
	int GetHeight() const noexcept {return Height;}
	int GetNumPixels() const noexcept {return Width*Height;}

	int GetMipMapCount() const noexcept { return MipMapCount; }
	int GetTotalBytes() const noexcept { return Size; }
	int GetBufferSize() const noexcept { return Size; }
	int GetFormat() const noexcept { return Format; }

	double GetVScale() const noexcept { return VScale; }
	double GetUScale() const noexcept { return UScale; }

	// Pixel accessors
	uint32& GetPixel(int Horiz, int Vert) {return Pixels[Width*(Vert%Height) + (Horiz%Width)];}
	uint32 *GetPixelBasePtr() noexcept {return Pixels;}
	const uint32 *GetBuffer() const noexcept { return Pixels; }
	uint32 *GetBuffer() noexcept { return Pixels; }

	uint32 *GetMipMapPtr(int Level);
	const uint32 *GetMipMapPtr(int Level) const ;
	int GetMipMapSize(int level) const;
	
	// Reallocation
	void Resize(int _Width, int _Height);

	// mipmappy operations
	void Resize(int _Width, int _Height, int _TotalBytes);

	bool Minify();

	bool MakeRGBA();
	bool MakeDXTC3();

	void PremultiplyAlpha();
	bool PremultipliedAlpha; // public so find silhouette version can unset

	// Clearing
	void Clear()
		{Width = Height = Size = 0; delete []Pixels; Pixels = nullptr;}

	ImageDescriptor(const ImageDescriptor &CopyFrom);
	
	ImageDescriptor(): Width(0), Height(0), VScale(1.0), UScale(1.0), Pixels(nullptr), Size(0), PremultipliedAlpha(false) {}

	// asumes RGBA8
	ImageDescriptor(int width, int height, uint32 *pixels);

	enum ImageFormat {
		RGBA8,
		DXTC1,
		DXTC3,
		DXTC5,
		Unknown
	};

	~ImageDescriptor()
	{
		delete []Pixels;
		Pixels = nullptr;
	}
			
private:
	bool LoadDDSFromFile(FileSpecifier& File, int flags, int actual_width = 0, int actual_height = 0, int maxSize = 0);
	bool LoadMipMapFromFile(OpenedFile &File, int flags, int level, DDSURFACEDESC2 &ddsd, int skip);
	bool SkipMipMapFromFile(OpenedFile &File, int flags, int level, DDSURFACEDESC2 &ddsd);

	ImageFormat Format;
};

template <typename T>
class copy_on_edit
{
public:
	copy_on_edit() : _original(nullptr), _copy(nullptr) { };

	void set(const T* original) {
		if (_copy) {
			delete _copy;
			_copy = nullptr;
		}
		_original = original;
	}

	void set(T* original) {
		if (_copy) {
			delete _copy;
			_copy= nullptr;
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
			return _copy;
		} else {
			if (!_copy) {
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
		_original = nullptr;
		_copy = copy;
        return _copy;
	}

	~copy_on_edit() {
		if (_copy) {
			delete _copy;
			_copy = nullptr;
		}
	}

private:
	T* _original;
	T* _copy;
};

using ImageDescriptionManager = copy_on_edit<ImageDescriptor>;
		

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
	ImageLoader_LoadMipMaps = 0x4,
	ImageLoader_LoadDXTC1AsDXTC3 = 0x8,
	ImageLoader_ImageIsAlreadyPremultiplied = 0x10
};
// Returns whether or not the loading was successful
//bool LoadImageFromFile(ImageDescriptor& Img, FileSpecifier& File, int ImgMode, int flags, int maxSize = 0);

uint32 *GetMipMapPtr(uint32 *pixels, int size, int level, int width, int height, int format);

#endif
