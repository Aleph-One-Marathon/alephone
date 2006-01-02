/*

	Copyright (C) 2005 and beyond by Bungie Studios, Inc.
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

*/

/*
 *  ImageLoader_Shared.cpp - Image file loading extras
 *
 *  Written in 2005 by Gregory Smith
 */

#include "AStream.h"
#include "cstypes.h"
#include "DDS.h"
#include "ImageLoader.h"
#include "SDL_endian.h"

uint32 *GetMipMapPtr(uint32 *pixels, int size, int level, int width, int height, ImageDescriptor::ImageFormat format)
{
	if (format == ImageDescriptor::RGBA8) 
	{
		uint32 *result = pixels;
		while (level--)
		{
			result += (width * height);
			width /= 2;
			height /= 2;
		}
		if (result - pixels < size)
		{
			return result;
		} 
		else
		{
			return NULL;
		}
	}
}

uint32 *ImageDescriptor::GetMipMapPtr(int Level)
{
	return ::GetMipMapPtr(&Pixels[0], Size, Level, Width, Height, Format);
}

void ImageDescriptor::Resize(int _Width, int _Height)
{
	Width = _Width;
	Height = _Height;
	Size = _Width * _Height * 4;
	delete []Pixels;
	Pixels = new uint32[_Width * Height];
}

void ImageDescriptor::Resize(int _Width, int _Height, int _TotalBytes)
{
	Width = _Width;
	Height = _Height;
	Size = _TotalBytes;
	delete []Pixels;
	Pixels = new uint32[_TotalBytes];
}

ImageDescriptor::ImageDescriptor(const ImageDescriptor &copyFrom) :
	Width(copyFrom.Width),
	Height(copyFrom.Height),
	OriginalWidth(copyFrom.OriginalWidth),
	OriginalHeight(copyFrom.OriginalHeight),
	Size(copyFrom.Size),
	MipMapCount(copyFrom.MipMapCount),
	Format(copyFrom.Format)
{
	if (copyFrom.Pixels) {
		Pixels = new uint32[copyFrom.Size];
		memcpy(Pixels, copyFrom.Pixels, copyFrom.Size);
	} else {
		Pixels = NULL;
	}
}

ImageDescriptor::ImageDescriptor(int _Width, int _Height, uint32 *_Pixels)
{
	Width = _Width;
	Height = _Height;
	OriginalWidth = _Width;
	OriginalHeight = _Height;
	Size = _Width * _Height * 4;
	Pixels = new uint32[_Width * _Height];
	memcpy(Pixels, _Pixels, Size);
	Format = RGBA8;
}

bool ImageDescriptor::LoadDDSFromFile(FileSpecifier& File, int flags, int maxSize)
{
	OpenedFile dds_file;
	if (!File.Open(dds_file)) {
		return false;
	}

	Uint32 dwMagic;
	if (!dds_file.Read(4, &dwMagic)) return false;
	
	if (SDL_SwapLE32(dwMagic) != FOUR_CHARS_TO_INT(' ', 'S', 'D', 'D')) {
		return false;
	}

	assert(sizeof(DDSURFACEDESC2) == 124);

	unsigned char header[124];
	if (!dds_file.Read(124, header)) return false;

	DDSURFACEDESC2 ddsd;

	fprintf(stderr, "DDS file detected\n");
	AIStreamLE inputStream(header, 124);
	try {
		inputStream >> ddsd.dwSize;
		if (ddsd.dwSize != 124) return false;

		inputStream >> ddsd.dwFlags;
		inputStream >> ddsd.dwHeight;
		inputStream >> ddsd.dwWidth;
		inputStream >> ddsd.dwPitchOrLinearSize;
		inputStream >> ddsd.dwDepth;
		inputStream >> ddsd.dwMipMapCount;
		inputStream.ignore(44);

		inputStream >> ddsd.ddpfPixelFormat.dwSize;
		if (ddsd.ddpfPixelFormat.dwSize != 32) return false;

		inputStream >> ddsd.ddpfPixelFormat.dwFlags;
		inputStream >> ddsd.ddpfPixelFormat.dwFourCC;
		inputStream >> ddsd.ddpfPixelFormat.dwRGBBitCount;
		inputStream >> ddsd.ddpfPixelFormat.dwRBitMask;
		inputStream >> ddsd.ddpfPixelFormat.dwGBitMask;
		inputStream >> ddsd.ddpfPixelFormat.dwBBitMask;
		inputStream >> ddsd.ddpfPixelFormat.dwRGBAlphaBitMask;
// 		inputStream.read(&ddsd.ddpfPixelFormat.dwRBitMask, 4);		
// 		inputStream.read(&ddsd.ddpfPixelFormat.dwGBitMask, 4);		
// 		inputStream.read(&ddsd.ddpfPixelFormat.dwBBitMask, 4);		
// 		inputStream.read(&ddsd.ddpfPixelFormat.dwRGBAlphaBitMask, 4);


		inputStream >> ddsd.ddsCaps.dwCaps1;
		inputStream >> ddsd.ddsCaps.dwCaps2;
		inputStream.ignore(8);

		inputStream.ignore(4);
	} catch (AStream::failure f) {
		fprintf(stderr, "exception %s, returning false\n", f.what());
		return false;
	}
	
	// textures only please
	if ((ddsd.ddsCaps.dwCaps2 & DDSCAPS2_CUBEMAP) ||
	    (ddsd.ddsCaps.dwCaps2 & DDSCAPS2_VOLUME)) {
		fprintf(stderr, "cubemap or volume\n");
		return false;
	}
	if ((!ddsd.ddsCaps.dwCaps1 & DDSCAPS_TEXTURE) ||
	    (ddsd.ddsCaps.dwCaps1 & DDSCAPS_MIPMAP) ||
	    (ddsd.ddsCaps.dwCaps1 & DDSCAPS_COMPLEX)) {
		fprintf(stderr, "complex or mipmap\n");
		return false;
	}

	if (ddsd.ddpfPixelFormat.dwFlags & DDPF_RGB) {
		fprintf(stderr, "uncompressed RGB\n");
		int pitch;
		if (ddsd.dwFlags & DDSD_PITCH) {
			pitch = ddsd.dwPitchOrLinearSize;
		} else if (ddsd.dwFlags & DDSD_LINEARSIZE) {
			// nvidia DDS file plugin writes this for some reason
			pitch = ddsd.dwPitchOrLinearSize / ddsd.dwHeight;
		} else {
			pitch = ddsd.dwWidth * ddsd.ddpfPixelFormat.dwRGBBitCount / 8;
		}

		unsigned char img[pitch * ddsd.dwHeight];
		if (!dds_file.Read(pitch * ddsd.dwHeight, img)) return false;

		SDL_Surface *s = SDL_CreateRGBSurfaceFrom(img, ddsd.dwWidth, ddsd.dwHeight, ddsd.ddpfPixelFormat.dwRGBBitCount, pitch, ddsd.ddpfPixelFormat.dwRBitMask, ddsd.ddpfPixelFormat.dwGBitMask, ddsd.ddpfPixelFormat.dwBBitMask, (ddsd.ddpfPixelFormat.dwFlags & DDPF_ALPHAPIXELS) ? ddsd.ddpfPixelFormat.dwRGBAlphaBitMask : 0);

		Resize(ddsd.dwWidth, ddsd.dwHeight);
		Original(ddsd.dwWidth, ddsd.dwHeight);

#ifdef ALEPHONE_LITTLE_ENDIAN
		SDL_Surface *rgba = SDL_CreateRGBSurface(SDL_SWSURFACE, Width, Height, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
#else
		SDL_Surface *rgba = SDL_CreateRGBSurface(SDL_SWSURFACE, Width, Height, 32, 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);
#endif	
		SDL_BlitSurface(s, NULL, rgba, NULL);
		SDL_FreeSurface(s);

		memcpy(GetPixelBasePtr(), rgba->pixels, Width * Height * 4);
		SDL_FreeSurface(rgba);

		Format = RGBA8;

		return true;
	} else if (ddsd.ddpfPixelFormat.dwFlags & DDPF_FOURCC) {
		fprintf(stderr, "fourcc\n");
		if (ddsd.ddpfPixelFormat.dwFourCC == FOUR_CHARS_TO_INT('1', 'T', 'X', 'D')) {
			fprintf(stderr, "DXT1 texture\n");
			Resize(ddsd.dwWidth, ddsd.dwHeight, ddsd.dwPitchOrLinearSize);
			Original(ddsd.dwWidth, ddsd.dwHeight);

			if (!dds_file.Read(ddsd.dwPitchOrLinearSize, &Pixels[0])) {
				return false;
			}
			
			Format = DXTC1;
			return true;
		} else {
			return false;
		}
	}

	return false;
}
	
