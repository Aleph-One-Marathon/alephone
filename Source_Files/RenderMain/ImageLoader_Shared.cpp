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
#include "SDL.h"
#include "SDL_endian.h"

#include <stdlib.h>

int ImageDescriptor::GetMipMapSize(int level) const
{

	// make sure the level is valid
	if (Width >> level == 0 && Height >> level == 0) {
		fprintf(stderr, "invalid mip map level size requested\n");
		return 0;
	}
	switch (Format) {
	case ImageDescriptor::RGBA8:
		return (max(1, Width >> level) * max(1, Height >> level) * 4);
		break;
	case ImageDescriptor::DXTC1:
		return (max(1, ((Width >> level) / 4)) * max(1, ((Height >> level) / 4)) * 8);
		break;
	case ImageDescriptor::DXTC3:
	case ImageDescriptor::DXTC5:
		return (max(1, ((Width >> level) / 4)) * max(1, ((Height >> level) / 4)) * 16);
		break;
	default:
		fprintf(stderr, "invalid format!\n");
		assert(false);
	}
}

const uint32 *ImageDescriptor::GetMipMapPtr(int Level) const
{
	int totalSize = 0;
	for (int i = 0; i < Level; i++) {
		totalSize += GetMipMapSize(i);
	}

	if (totalSize < GetBufferSize()) {
		return GetBuffer() + (totalSize / 4);
	} else {
		return NULL;
	}
}

uint32 *ImageDescriptor::GetMipMapPtr(int Level)
{
	int totalSize = 0;
	for (int i = 0; i < Level; i++) {
		totalSize += GetMipMapSize(i);
	}

	if (totalSize < GetBufferSize()) {
		return GetBuffer() + (totalSize / 4);
	} else {
		return NULL;
	}
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
	VScale(copyFrom.VScale),
	UScale(copyFrom.UScale),
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
	VScale = 1.0;
	UScale = 1.0;
	Size = _Width * _Height * 4;
	Pixels = new uint32[_Width * _Height];
	memcpy(Pixels, _Pixels, Size);
	Format = RGBA8;
	MipMapCount = 0;
}

bool ImageDescriptor::LoadMipMapFromFile(OpenedFile& file, int flags, int level, DDSURFACEDESC2 &ddsd)
{
	// total the size so far
	int totalSize = 0;
	for (int i = 0; i < level; i++) {
		totalSize += GetMipMapSize(i);
	}
	
	if (totalSize + GetMipMapSize(level) > GetBufferSize()) {
		fprintf(stderr, "buffer not large enough\n");
		return false;
	}

	int srcWidth = max(1, (int) ddsd.dwWidth >> level);
	int srcHeight = max(1, (int) ddsd.dwHeight >> level);
	int dstWidth = max(1, Width >> level);
	int dstHeight = max(1, Height >> level);

	unsigned char *buffer = (unsigned char *) GetBuffer() + totalSize;

	if (Format == RGBA8) 
	{
		int pitch;
		if (ddsd.dwFlags & DDSD_PITCH) {
			pitch = ddsd.dwPitchOrLinearSize;
		} else {
			pitch = ddsd.dwPitchOrLinearSize / ddsd.dwHeight;
		}
		int depth = pitch / ddsd.dwWidth;
		pitch = srcWidth * depth;
		
		unsigned char img[pitch * srcHeight];
		if (!file.Read(pitch * srcHeight, img)) {
			fprintf(stderr, "failed to read %i bytes\n", pitch * srcHeight);
			return false;
		}

		SDL_Surface *src = SDL_CreateRGBSurfaceFrom(img, srcWidth, srcHeight, ddsd.ddpfPixelFormat.dwRGBBitCount, pitch, ddsd.ddpfPixelFormat.dwRBitMask, ddsd.ddpfPixelFormat.dwGBitMask, ddsd.ddpfPixelFormat.dwBBitMask, (ddsd.ddpfPixelFormat.dwFlags & DDPF_ALPHAPIXELS) ? ddsd.ddpfPixelFormat.dwRGBAlphaBitMask : 0);
		SDL_SetAlpha(src, 0, 0xff); // disable SDL_SRCALPHA
		
#ifdef ALEPHONE_LITTLE_ENDIAN
		SDL_Surface *dst = SDL_CreateRGBSurfaceFrom(buffer, dstWidth, dstHeight, 32, dstWidth * 4, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
#else
		SDL_Surface *dst = SDL_CreateRGBSurfaceFrom(buffer, dstWidth, dstHeight, 32, dstWidth * 4, 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);
#endif
		SDL_BlitSurface(src, NULL, dst, NULL);
		SDL_FreeSurface(src);
		SDL_FreeSurface(dst);
	} 
	else if (Format == DXTC1)
	{

		dstWidth += dstWidth % 4;
		dstHeight += dstHeight % 4;
		srcWidth += srcWidth % 4;
		srcHeight += srcHeight % 4;

		memset(buffer, '\0', (dstWidth / 4) * (dstHeight / 4) * 8);
		for (int row = 0; row < srcHeight / 4; row++)
		{
			if (!file.Read(srcWidth / 4 * 8, &buffer[row * dstWidth / 4 * 8])) {
				fprintf(stderr, "failed to read %i bytes\n", srcWidth / 4 * 8);
				return false;
			}
		}
	}
	else if (Format == DXTC3 || Format == DXTC5)
	{

		dstWidth += dstWidth % 4;
		dstHeight += dstHeight % 4;
		srcWidth += srcWidth % 4;
		srcHeight += srcHeight % 4;

		memset(buffer, '\0', (dstWidth / 4) * (dstHeight / 4) * 16);
		for (int row = 0; row < srcHeight / 4; row++)
		{
			if (!file.Read(srcWidth / 4 * 16, &buffer[row * dstWidth / 4 * 16])) {
				fprintf(stderr, "failed to read %i bytes\n", srcWidth / 4 * 16);
				return false;
			}
		}
	}

	return true;
}
	
	
	

bool ImageDescriptor::LoadDDSFromFile(FileSpecifier& File, int flags, int actual_width, int actual_height, int maxSize)
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

	// figure out the original and final widths
	int OriginalWidth = (actual_width) ? actual_width : ddsd.dwWidth;
	int OriginalHeight = (actual_height) ? actual_height : ddsd.dwHeight;
	
	int Width = ddsd.dwWidth;
	int Height = ddsd.dwHeight;
	if (flags & ImageLoader_ResizeToPowersOfTwo) {
		Width = NextPowerOfTwo(Width);
		Height = NextPowerOfTwo(Height);
	}

	VScale = (double) OriginalWidth / (double) Width;
	UScale = (double) OriginalHeight / (double) Height;

	Format = Unknown;
	if (ddsd.ddpfPixelFormat.dwFlags & DDPF_RGB) {
		Format = RGBA8;
	} else if (ddsd.ddpfPixelFormat.dwFlags & DDPF_FOURCC) {
		if (ddsd.ddpfPixelFormat.dwFourCC == FOUR_CHARS_TO_INT('1', 'T', 'X', 'D')) {
			Format = DXTC1;
		} else if (ddsd.ddpfPixelFormat.dwFourCC == FOUR_CHARS_TO_INT('3', 'T', 'X', 'D')) {
			Format = DXTC3;
		} else if (ddsd.ddpfPixelFormat.dwFourCC == FOUR_CHARS_TO_INT('5', 'T', 'X', 'D')) {
			Format = DXTC5;
		}
	}

	if (Format == Unknown) return false;

	// allocate a buffer

	int bpp = 0;
	if (Format == RGBA8) bpp = 32;
	else if (Format == DXTC1) bpp = 4;
	else if (Format == DXTC3 || Format == DXTC5) bpp = 8;

	this->Width = Width;
	this->Height = Height;

	if (ddsd.ddsCaps.dwCaps1 & DDSCAPS_MIPMAP) {
		
		int OriginalMipMapCount = ddsd.dwMipMapCount;
		
		// we don't handle incomplete mip map chains
		if (OriginalMipMapCount != 1 + floor(log2(max(OriginalWidth, OriginalHeight)))) {
			fprintf(stderr, "incomplete mipmap chain (%ix%i, %ix%i, %i mipmaps\n", Width, Height, ddsd.dwWidth, ddsd.dwHeight, OriginalMipMapCount);
			return false;
		}

		// hehe, resizing to the next power of two could have
		// introduced another mipmap
		MipMapCount = 1 + floor(log2(max(Width, Height)));

		int totalSize = 0;
		for (int i = 0; i < OriginalMipMapCount; i++) {
			totalSize += GetMipMapSize(i);
		}

		for (int i = OriginalMipMapCount; i < MipMapCount; i++) {
			totalSize += GetMipMapSize(i);
		}
		
		Resize(Width, Height, totalSize);

		for (int i = 0; i < OriginalMipMapCount; i++) {
			// read in the base image
			if (!LoadMipMapFromFile(dds_file, flags, i, ddsd)) return false;
		}

		for (int i = OriginalMipMapCount; i < MipMapCount; i++) {
			// what's one pixel among friends?
			memcpy(GetMipMapPtr(i), GetMipMapPtr(OriginalMipMapCount - 1), GetMipMapSize(i));
		}
		
		
	} else {
		MipMapCount = 1;
		
		Resize(Width, Height, GetMipMapSize(0));

		if (!LoadMipMapFromFile(dds_file, flags, 0, ddsd)) return false;
	}

	return true;
}
	
