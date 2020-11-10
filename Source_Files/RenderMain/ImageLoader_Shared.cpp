/*

	Copyright (C) 2005 and beyond by Bungie Studios, Inc.
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

*/

/*
 *  ImageLoader_Shared.cpp - Image file loading extras
 *
 *  Written in 2005 by Gregory Smith
 * 
 *  DXTC decompression functions (C) 2000-2002 by Denton Woods
 *          adapted from DevIL (openil.sourceforge.net)
 */

#if defined(_MSC_VER)
#define NOMINMAX
#include <algorithm>
#endif

#include "AStream.h"
#include "cstypes.h"
#include "DDS.h"
#include "ImageLoader.h"
#include "SDL.h"
#include "SDL_endian.h"
#include "Logging.h"


#ifdef HAVE_OPENGL
#include "OGL_Headers.h"
#include "OGL_Setup.h" // OGL_IsActive
#endif

#include <cmath>
#include <stdlib.h>

using std::min;
using std::max;

static inline float log2(int x) { return std::log((float) x) / std::log(2.0); }

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
		return (max(1, (((Width >> level) + 3) / 4)) * max(1, (((Height >> level) + 3) / 4)) * 8);
		break;
	case ImageDescriptor::DXTC3:
	case ImageDescriptor::DXTC5:
		return (max(1, (((Width >> level) + 3) / 4)) * max(1, (((Height >> level)  + 3) / 4)) * 16);
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

bool ImageDescriptor::Minify()
{
	if (MipMapCount > 1)
	{
		int newSize = Size - GetMipMapSize(0);
		
		uint32 *newPixels = new uint32[newSize / 4];
		memcpy(newPixels, GetMipMapPtr(1), newSize);
		MipMapCount--;
		Width = MAX(1, Width >> 1);
		Height = MAX(1, Height >> 1);
		Size = newSize;
		delete []Pixels;
		Pixels = newPixels;
		return true;
	}  
	else if (Format == RGBA8)
	{
		if (!(Width > 1 || Height > 1)) return false;
		int newWidth = Width >> 1;
		int newHeight = Height >> 1;
#ifdef HAVE_OPENGL
		if (OGL_IsActive())
		{
			
			uint32 *newPixels = new uint32[newWidth * newHeight];
			gluScaleImage(GL_RGBA, Width, Height, GL_UNSIGNED_BYTE, Pixels, newWidth, newHeight, GL_UNSIGNED_BYTE, newPixels);
			delete []Pixels;
			Pixels = newPixels;
			Width = newWidth;
			Height = newHeight;
			Size = newWidth * newHeight;
			return true;
			
		} 
		else 
#endif
		{
			fprintf(stderr, "GL not active\n");
			return false;
		}
	} 
	else 
	{
		return false;
	}
}
	

ImageDescriptor::ImageDescriptor(const ImageDescriptor &copyFrom) :
	Width(copyFrom.Width),
	Height(copyFrom.Height),
	VScale(copyFrom.VScale),
	UScale(copyFrom.UScale),
	Size(copyFrom.Size),
	MipMapCount(copyFrom.MipMapCount),
	Format(copyFrom.Format),
	PremultipliedAlpha(copyFrom.PremultipliedAlpha)
{
	if (copyFrom.Pixels) {
		Pixels = new uint32[copyFrom.Size];
		memcpy(Pixels, copyFrom.Pixels, copyFrom.Size);
	} else {
		Pixels = NULL;
	}
}

ImageDescriptor::ImageDescriptor(int _Width, int _Height, uint32 *_Pixels) : 
	Width(_Width), 
	Height(_Height), 
	VScale(1.0), 
	UScale(1.0), 
	Pixels(_Pixels), 
	Format(RGBA8), 
	MipMapCount(0), 
	PremultipliedAlpha(false)
{
	Size = _Width * _Height * 4;
}

static inline int padfour(int x)
{
	return (x + 3) / 4 * 4;
}

bool ImageDescriptor::LoadMipMapFromFile(OpenedFile& file, int flags, int level, DDSURFACEDESC2 &ddsd, int skip)
{
	// total the size so far
	int totalSize = 0;
	for (int i = skip; i < level; i++) {
		totalSize += GetMipMapSize(i);
	}
	
	if (totalSize + GetMipMapSize(level) > GetBufferSize()) {
		fprintf(stderr, "buffer not large enough\n");
		if (flags & ImageLoader_LoadMipMaps) {
			fprintf(stderr, "(loading mipmaps\n");
		}
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

		std::vector<unsigned char> img;
		img.resize(pitch * srcHeight);
		if (!file.Read(pitch * srcHeight, &img.front())) {
			return false;
		}

		SDL_Surface *src = SDL_CreateRGBSurfaceFrom(&img.front(), srcWidth, srcHeight, ddsd.ddpfPixelFormat.dwRGBBitCount, pitch, ddsd.ddpfPixelFormat.dwRBitMask, ddsd.ddpfPixelFormat.dwGBitMask, ddsd.ddpfPixelFormat.dwBBitMask, (ddsd.ddpfPixelFormat.dwFlags & DDPF_ALPHAPIXELS) ? ddsd.ddpfPixelFormat.dwRGBAlphaBitMask : 0);
		SDL_SetSurfaceBlendMode(src, SDL_BLENDMODE_NONE); // disable SDL_SRCALPHA
		
		SDL_Surface *dst = nullptr;
		if (PlatformIsLittleEndian()) {
			dst = SDL_CreateRGBSurfaceFrom(buffer, dstWidth, dstHeight, 32, dstWidth * 4, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
		} else {
			dst = SDL_CreateRGBSurfaceFrom(buffer, dstWidth, dstHeight, 32, dstWidth * 4, 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);
		}
		SDL_BlitSurface(src, NULL, dst, NULL);
		SDL_FreeSurface(src);
		SDL_FreeSurface(dst);
	} 
	else if (Format == DXTC1)
	{

		dstWidth =  padfour(dstWidth);
		dstHeight = padfour(dstHeight);
		srcWidth = padfour(srcWidth);
		srcHeight = padfour(srcHeight);

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

		dstWidth = padfour(dstWidth);
		dstHeight = padfour(dstHeight);
		srcWidth = padfour(srcWidth);
		srcHeight = padfour(srcHeight);

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
	
bool ImageDescriptor::SkipMipMapFromFile(OpenedFile& File, int flags, int level, DDSURFACEDESC2 &ddsd)
{
	int srcWidth = max(1, (int) ddsd.dwWidth >> level);
	int srcHeight = max(1, (int) ddsd.dwHeight >> level);

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

		int32 position;
		if (!File.GetPosition(position)) return false;
		return File.SetPosition(position + (pitch * srcHeight));
	} 
	else if (Format == DXTC1)
	{
		srcWidth = padfour(srcWidth);
		srcHeight = padfour(srcHeight);

		int32 position;
		if (!File.GetPosition(position)) return false;
		return File.SetPosition(position + (srcWidth / 4 * srcHeight / 4 * 8));
	} 
	else if (Format == DXTC3 || Format == DXTC5)
	{
		srcWidth = padfour(srcWidth);
		srcHeight = padfour(srcHeight);
		
		int32 position;
		if (!File.GetPosition(position)) return false;
		return File.SetPosition(position + (srcWidth / 4 * srcHeight / 4 * 16));
	}
    
    return false;
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
		if ((ddsd.ddpfPixelFormat.dwFlags & DDPF_RGB) && ddsd.ddpfPixelFormat.dwRGBBitCount != 32 && ddsd.ddpfPixelFormat.dwRGBBitCount != 24) return false;
		
 		inputStream.read((char *) &ddsd.ddpfPixelFormat.dwRBitMask, 4);		
 		inputStream.read((char *) &ddsd.ddpfPixelFormat.dwGBitMask, 4);		
 		inputStream.read((char *) &ddsd.ddpfPixelFormat.dwBBitMask, 4);		
 		inputStream.read((char *) &ddsd.ddpfPixelFormat.dwRGBAlphaBitMask, 4);
		
		if (!PlatformIsLittleEndian()) {
			if (ddsd.ddpfPixelFormat.dwRGBBitCount == 24) 
			{
				// the masks are in the correct order, but will be in the wrong place...move them down
				ddsd.ddpfPixelFormat.dwRBitMask >>= 8;
				ddsd.ddpfPixelFormat.dwGBitMask >>= 8;
				ddsd.ddpfPixelFormat.dwBBitMask >>= 8;
			}
		}

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

	this->Width = Width;
	this->Height = Height;

	if (ddsd.ddsCaps.dwCaps1 & DDSCAPS_MIPMAP) {
		
		int OriginalMipMapCount = ddsd.dwMipMapCount;
		
		int ExpectedMipMapCount = 1 + floor(log2(max(OriginalWidth, OriginalHeight)));
		// we don't handle incomplete mip map chains
		// if we're only missing one, that's OK; XBLA textures do that
		if (!(OriginalMipMapCount == ExpectedMipMapCount || OriginalMipMapCount == (ExpectedMipMapCount - 1))) {
			logWarning("incomplete mipmap chain (%ix%i, %ix%i, %i mipmaps)", Width, Height, ddsd.dwWidth, ddsd.dwHeight, OriginalMipMapCount);
			return false;
		}

		// hehe, resizing to the next power of two could have
		// introduced another mipmap
		MipMapCount = static_cast<int>(1 + floor(log2(max(Width, Height))));
		
		int skip  = 0;
		if (maxSize) {
			while ((Width >> skip) > maxSize || (Height >> skip) > maxSize) {
				skip++;
			}
		}
			    

		if (flags & ImageLoader_LoadMipMaps) {
			int totalSize = 0;
			for (int i = skip; i < OriginalMipMapCount; i++) {
				totalSize += GetMipMapSize(i);
			}
			
			for (int i = OriginalMipMapCount; i < MipMapCount; i++) {
				totalSize += GetMipMapSize(i);
			}
			
			Resize(Width, Height, totalSize);
			
			for (int i = 0; i < skip; i++) {
				if (!SkipMipMapFromFile(dds_file, flags, i, ddsd)) return false;
			}
			
			for (int i = skip; i < OriginalMipMapCount; i++) {
				// read in the base image
				if (!LoadMipMapFromFile(dds_file, flags, i, ddsd, skip)) return false;
			}
			
			this->Height = max(1, this->Height >> skip);
			this->Width = max(1, this->Width >> skip);
			if (skip && (Format == DXTC1 || Format == DXTC3 || Format == DXTC5)) {
				this->Height += this->Height % 4;
				this->Width += this->Width % 4;
			}
			MipMapCount -= skip;
			
			for (int i = OriginalMipMapCount - skip; i < MipMapCount; i++) {
				// what's one pixel among friends?
				memcpy(GetMipMapPtr(i), GetMipMapPtr(OriginalMipMapCount - skip - 1), GetMipMapSize(i));
			}
		} else {
			int totalSize = GetMipMapSize(skip);
			Resize(Width, Height, totalSize);

			for (int i = 0; i < skip; i++) {
				if (!SkipMipMapFromFile(dds_file, flags, i, ddsd)) return false;
			}

			if (!LoadMipMapFromFile(dds_file, flags, skip, ddsd, skip)) return false;
			this->Height = max(1, this->Height >> skip);
			this->Width = max(1, this->Width >> skip);
			if (skip && (Format == DXTC1 || Format == DXTC3 || Format == DXTC5)) {
				this->Height = padfour(this->Height);
				this->Width = padfour(this->Width);
			}
			
			MipMapCount = 1;
		}
	} else {
		if ((Width > maxSize || Height > maxSize) && (Format == DXTC1 || Format == DXTC3 || Format == DXTC5)) return false;

		MipMapCount = 1;
		
		Resize(Width, Height, GetMipMapSize(0));

		if (!LoadMipMapFromFile(dds_file, flags, 0, ddsd, 0)) return false;

		while (this->Width > maxSize || this->Height > maxSize)
		{
			if (!Minify()) return false;
		}
	}
	
	if (!(flags & ImageLoader_CanUseDXTC)) MakeRGBA();

	if (Format == DXTC1 && (flags & ImageLoader_LoadDXTC1AsDXTC3)) MakeDXTC3();

	return true;
}

bool ImageDescriptor::MakeDXTC3()
{
	if (Format != DXTC1) return false;

	uint32 *NewPixels = new uint32[Size * 2];

	uint32 *oldpixels = Pixels;
	uint32 *newpixels = NewPixels;
	
	for (int i = 0; i < (Size / 8); i++) {
		memset(newpixels, 0xff, 8);
		newpixels += 2;
		memcpy(newpixels, oldpixels, 8);
		oldpixels += 2;
		newpixels += 2;
	}

	Size = Size * 2;
	delete []Pixels;
	Pixels = NewPixels;
	Format = DXTC3;
	return true;
}

static bool DecompressDXTC1(uint32 *out, int width, int height, uint32 *in);
static bool DecompressDXTC3(uint32 *out, int width, int height, uint32 *in);
static bool DecompressDXTC5(uint32 *out, int width, int height, uint32 *in);
	
bool ImageDescriptor::MakeRGBA()
{
	ImageDescriptor RGBADesc;
	RGBADesc.Width = Width;
	RGBADesc.Height = Height;
	RGBADesc.Format = RGBA8;
	RGBADesc.MipMapCount = MipMapCount;
	
	RGBADesc.Size = 0;
	for (int i = 0; i < MipMapCount; i++) {
		RGBADesc.Size += RGBADesc.GetMipMapSize(i);
	}

	RGBADesc.Pixels = new uint32[RGBADesc.Size / 4];
	
	for (int i = 0; i < MipMapCount; i++) {
		if (Format == DXTC1) {
			if (!DecompressDXTC1(RGBADesc.GetMipMapPtr(i), MAX(1, Width >> i), MAX(1, Height >> i), GetMipMapPtr(i))) return false;
		} else if (Format == DXTC3) {
			if (!DecompressDXTC3(RGBADesc.GetMipMapPtr(i), MAX(1, Width >> i), MAX(1, Height >> i), GetMipMapPtr(i))) return false;
		} else if (Format == DXTC5) {
			if (!DecompressDXTC5(RGBADesc.GetMipMapPtr(i), MAX(1, Width >> i), MAX(1, Height >> i), GetMipMapPtr(i))) return false;
		} else {
			return false;
		}
	}
	
	delete []Pixels;
	Pixels = RGBADesc.Pixels;
	Size = RGBADesc.Size;
	Format = RGBADesc.Format;
	RGBADesc.Pixels = NULL;

	return true;
}

void ImageDescriptor::PremultiplyAlpha()
{
	if (PremultipliedAlpha) return;
	for (int i = 0; i < GetNumPixels(); i++)
	{
		// do these two optimizations without unpacking
		constexpr uint32 alphaMask = PlatformIsLittleEndian() ? 0xff000000 : 0x000000ff;

		if ((Pixels[i] & alphaMask) == alphaMask)
			continue;
		if ((Pixels[i] & alphaMask) == 0) {
			Pixels[i] = 0;
			continue;
		}
		
		// boo, have to do math
		short r, g, b, a;
		uint8 *PxlPtr = (uint8 *)&Pixels[i];

		r = PxlPtr[0];
		g = PxlPtr[1];
		b = PxlPtr[2];
		a = PxlPtr[3];
		
		r = (a * r + 127) / 255;
		g = (g * r + 127) / 255;
		b = (b * r + 127) / 255;

		PxlPtr[0] = (unsigned char) r;
		PxlPtr[1] = (unsigned char) g;
		PxlPtr[2] = (unsigned char) b;
	}

	PremultipliedAlpha = true;
}

// DXTC decompression code adapted from DevIL (openil.sourceforge.net)

typedef struct Color8888
{
	unsigned char r; // change the order of names to change the 
	unsigned char g; //  order of the output ARGB or BGRA, etc...
	unsigned char b; //  Last one is MSB, 1st is LSB.
	unsigned char a;
} Color8888;


typedef struct Color565
{
	unsigned nBlue  : 5; // order of names changes
	unsigned nGreen : 6; //	byte order of output to 32 bit
	unsigned nRed	: 5;
} Color565;

static bool DecompressDXTC1(uint32 *out, int width, int height, uint32 *in)
{
	int			x, y, i, j, k, Select;
	unsigned char		*Temp;
	Color565	*color_0, *color_1;
	Color8888	colours[4], *col;
	uint32		bitmask, Offset;
	unsigned char *data = (unsigned char *) out;
	
	
	Temp = (unsigned char *) in;
	for (y = 0; y < height; y += 4) {
		for (x = 0; x < width; x += 4) {

			*((Uint16 *)Temp) = SDL_SwapLE16(*((Uint16 *)Temp));
			color_0 = ((Color565*)Temp);
			Temp += 2;
			*((Uint16 *)Temp) = SDL_SwapLE16(*((Uint16 *)Temp));
			color_1 = ((Color565*)(Temp));
			Temp += 2;
			bitmask = SDL_SwapLE32(((uint32*)Temp)[0]);
			Temp += 4;
			
			colours[0].r = color_0->nRed << 3;
			colours[0].g = color_0->nGreen << 2;
			colours[0].b = color_0->nBlue << 3;
			colours[0].a = 0xFF;
			
			colours[1].r = color_1->nRed << 3;
			colours[1].g = color_1->nGreen << 2;
			colours[1].b = color_1->nBlue << 3;
			colours[1].a = 0xFF;
			
			
			if (*((uint16*)color_0) > *((uint16*)color_1)) {
				// Four-color block: derive the other two colors.    
				// 00 = color_0, 01 = color_1, 10 = color_2, 11 = color_3
				// These 2-bit codes correspond to the 2-bit fields 
				// stored in the 64-bit block.
				colours[2].b = (2 * colours[0].b + colours[1].b + 1) / 3;
				colours[2].g = (2 * colours[0].g + colours[1].g + 1) / 3;
				colours[2].r = (2 * colours[0].r + colours[1].r + 1) / 3;
				colours[2].a = 0xFF;
				
				colours[3].b = (colours[0].b + 2 * colours[1].b + 1) / 3;
				colours[3].g = (colours[0].g + 2 * colours[1].g + 1) / 3;
				colours[3].r = (colours[0].r + 2 * colours[1].r + 1) / 3;
				colours[3].a = 0xFF;
			}    
			else { 
				// Three-color block: derive the other color.
				// 00 = color_0,  01 = color_1,  10 = color_2,
				// 11 = transparent.
				// These 2-bit codes correspond to the 2-bit fields 
				// stored in the 64-bit block. 
				colours[2].b = (colours[0].b + colours[1].b) / 2;
				colours[2].g = (colours[0].g + colours[1].g) / 2;
				colours[2].r = (colours[0].r + colours[1].r) / 2;
				colours[2].a = 0xFF;
				
				colours[3].b = (colours[0].b + 2 * colours[1].b + 1) / 3;
				colours[3].g = (colours[0].g + 2 * colours[1].g + 1) / 3;
				colours[3].r = (colours[0].r + 2 * colours[1].r + 1) / 3;
				colours[3].a = 0x00;
			}
			
			for (j = 0, k = 0; j < 4; j++) {
				for (i = 0; i < 4; i++, k++) {
					
					Select = (bitmask & (0x03 << k*2)) >> k*2;
					col = &colours[Select];
					
					if (((x + i) < width) && ((y + j) < height)) {
						Offset = (y + j) * (width * 4) + (x + i) * 4;
						// this make absolutely no sense to me, but it works on my G4...
						if (PlatformIsLittleEndian()) {
							data[Offset + 0] = col->r;
							data[Offset + 1] = col->g;
							data[Offset + 2] = col->b;
							data[Offset + 3] = col->a;
						} else {
							data[Offset + 0] = col->b;
							data[Offset + 1] = col->g;
							data[Offset + 2] = col->r;
							data[Offset + 3] = col->a;
						}
					}
				}
			}
		}
	}

	return true;
}

typedef struct DXTColBlock
{
	uint16 col0;
	uint16 col1;

	// no bit fields - use bytes
	unsigned char row[4];
} DXTColBlock;

typedef struct DXTAlphaBlockExplicit
{
	uint16 row[4];
} DXTAlphaBlockExplicit;

typedef struct DXTAlphaBlock3BitLinear
{
	unsigned char alpha0;
	unsigned char alpha1;

	unsigned char stuff[6];
} DXTAlphaBlock3BitLinear;

static bool DecompressDXTC3(uint32 *out, int width, int height, uint32 *in)
{
	int			x, y, i, j, k, Select;
	unsigned char		*Temp;
	Color565	*color_0, *color_1;
	Color8888	colours[4], *col;
	uint32		bitmask, Offset;
	uint16	word;
	DXTAlphaBlockExplicit *alpha;
	unsigned char *data = (unsigned char *) out;

	assert(in);
	Temp = (unsigned char *) in;
	for (y = 0; y < height; y += 4) {
		for (x = 0; x < width; x += 4) {
			alpha = (DXTAlphaBlockExplicit*)Temp;
			Temp += 8;
			*((Uint16 *)Temp) = SDL_SwapLE16(*((Uint16 *)Temp));
			color_0 = ((Color565*)Temp);
			Temp+= 2;
			*((Uint16 *)Temp) = SDL_SwapLE16(*((Uint16 *)Temp));
			color_1 = ((Color565*)(Temp));
			Temp += 2;
			bitmask = SDL_SwapLE32(((uint32*)Temp)[0]);
			Temp += 4;
			
			colours[0].r = color_0->nRed << 3;
			colours[0].g = color_0->nGreen << 2;
			colours[0].b = color_0->nBlue << 3;
			colours[0].a = 0xFF;
			
			colours[1].r = color_1->nRed << 3;
			colours[1].g = color_1->nGreen << 2;
			colours[1].b = color_1->nBlue << 3;
			colours[1].a = 0xFF;
			
			// Four-color block: derive the other two colors.    
			// 00 = color_0, 01 = color_1, 10 = color_2, 11 = color_3
			// These 2-bit codes correspond to the 2-bit fields 
			// stored in the 64-bit block.
			colours[2].b = (2 * colours[0].b + colours[1].b + 1) / 3;
			colours[2].g = (2 * colours[0].g + colours[1].g + 1) / 3;
			colours[2].r = (2 * colours[0].r + colours[1].r + 1) / 3;
			colours[2].a = 0xFF;
			
			colours[3].b = (colours[0].b + 2 * colours[1].b + 1) / 3;
			colours[3].g = (colours[0].g + 2 * colours[1].g + 1) / 3;
			colours[3].r = (colours[0].r + 2 * colours[1].r + 1) / 3;
			colours[3].a = 0xFF;
			
			k = 0;
			for (j = 0; j < 4; j++) {
				for (i = 0; i < 4; i++, k++) {
					
					Select = (bitmask & (0x03 << k*2)) >> k*2;
					col = &colours[Select];
					
					if (((x + i) < width) && ((y + j) < height)) {
						Offset = (y + j) * (width * 4) + (x + i) * 4;
						if (PlatformIsLittleEndian()) {
							data[Offset + 0] = col->r;
							data[Offset + 1] = col->g;
							data[Offset + 2] = col->b;
						} else {
							data[Offset + 0] = col->b;
							data[Offset + 1] = col->g;
							data[Offset + 2] = col->r;
						}
					}
				}
			}
			
			for (j = 0; j < 4; j++) {
				word = SDL_SwapLE16(alpha->row[j]);
				for (i = 0; i < 4; i++) {
					if (((x + i) < width) && ((y + j) < height)) {
						Offset = (y + j) * (width * 4) + (x + i) * 4 + 3;
						data[Offset] = word & 0x0F;
						data[Offset] = data[Offset] | data[Offset] << 4;
					}
					word >>= 4;
				}
			}
			
		}
		
	}
	
	return true;
}

static bool DecompressDXTC5(uint32 *out, int width, int height, uint32 *in)
{
	int			x, y, i, j, k, Select;
	unsigned char		*Temp;
	Color565	*color_0, *color_1;
	Color8888	colours[4], *col;
	uint32		bitmask, Offset;
	unsigned char		alphas[8], *alphamask;
	uint32		bits;

	Temp = (unsigned char *) in;
	unsigned char *data = (unsigned char *) out;

	for (y = 0; y < height; y += 4) {
		for (x = 0; x < width; x += 4) {
			if (y >= height || x >= width)
				break;
			alphas[0] = Temp[0];
			alphas[1] = Temp[1];
			alphamask = Temp + 2;
			Temp += 8;
			*((Uint16 *)Temp) = SDL_SwapLE16(*((Uint16 *)Temp));
			color_0 = ((Color565*)Temp);
			Temp += 2;
			*((Uint16 *)Temp) = SDL_SwapLE16(*((Uint16 *)Temp));
			color_1 = ((Color565*)(Temp));
			Temp += 2;
			bitmask = SDL_SwapLE32(((uint32*)Temp)[0]);
			Temp += 4;

			colours[0].r = color_0->nRed << 3;
			colours[0].g = color_0->nGreen << 2;
			colours[0].b = color_0->nBlue << 3;
			colours[0].a = 0xFF;

			colours[1].r = color_1->nRed << 3;
			colours[1].g = color_1->nGreen << 2;
			colours[1].b = color_1->nBlue << 3;
			colours[1].a = 0xFF;

			// Four-color block: derive the other two colors.    
			// 00 = color_0, 01 = color_1, 10 = color_2, 11 = color_3
			// These 2-bit codes correspond to the 2-bit fields 
			// stored in the 64-bit block.
			colours[2].b = (2 * colours[0].b + colours[1].b + 1) / 3;
			colours[2].g = (2 * colours[0].g + colours[1].g + 1) / 3;
			colours[2].r = (2 * colours[0].r + colours[1].r + 1) / 3;
			colours[2].a = 0xFF;

			colours[3].b = (colours[0].b + 2 * colours[1].b + 1) / 3;
			colours[3].g = (colours[0].g + 2 * colours[1].g + 1) / 3;
			colours[3].r = (colours[0].r + 2 * colours[1].r + 1) / 3;
			colours[3].a = 0xFF;

			k = 0;
			for (j = 0; j < 4; j++) {
				for (i = 0; i < 4; i++, k++) {

					Select = (bitmask & (0x03 << k*2)) >> k*2;
					col = &colours[Select];

					// only put pixels out < width or height
					if (((x + i) < width) && ((y + j) < height)) {
						Offset = (y + j) * (width * 4) + (x + i) * 4;
						if (PlatformIsLittleEndian()) {
							data[Offset + 0] = col->r;
							data[Offset + 1] = col->g;
							data[Offset + 2] = col->b;
						} else {
							data[Offset + 0] = col->b;
							data[Offset + 1] = col->g;
							data[Offset + 2] = col->r;
						}
					}
				}
			}

			// 8-alpha or 6-alpha block?    
			if (alphas[0] > alphas[1]) {    
				// 8-alpha block:  derive the other six alphas.    
				// Bit code 000 = alpha_0, 001 = alpha_1, others are interpolated.
				alphas[2] = (6 * alphas[0] + 1 * alphas[1] + 3) / 7;	// bit code 010
				alphas[3] = (5 * alphas[0] + 2 * alphas[1] + 3) / 7;	// bit code 011
				alphas[4] = (4 * alphas[0] + 3 * alphas[1] + 3) / 7;	// bit code 100
				alphas[5] = (3 * alphas[0] + 4 * alphas[1] + 3) / 7;	// bit code 101
				alphas[6] = (2 * alphas[0] + 5 * alphas[1] + 3) / 7;	// bit code 110
				alphas[7] = (1 * alphas[0] + 6 * alphas[1] + 3) / 7;	// bit code 111  
			}    
			else {  
				// 6-alpha block.    
				// Bit code 000 = alpha_0, 001 = alpha_1, others are interpolated.
				alphas[2] = (4 * alphas[0] + 1 * alphas[1] + 2) / 5;	// Bit code 010
				alphas[3] = (3 * alphas[0] + 2 * alphas[1] + 2) / 5;	// Bit code 011
				alphas[4] = (2 * alphas[0] + 3 * alphas[1] + 2) / 5;	// Bit code 100
				alphas[5] = (1 * alphas[0] + 4 * alphas[1] + 2) / 5;	// Bit code 101
				alphas[6] = 0x00;										// Bit code 110
				alphas[7] = 0xFF;										// Bit code 111
			}

			// Note: Have to separate the next two loops,
			//	it operates on a 6-byte system.

			// First three bytes
			bits = SDL_SwapLE32(*((int32*)alphamask));
			for (j = 0; j < 2; j++) {
				for (i = 0; i < 4; i++) {
					// only put pixels out < width or height
					if (((x + i) < width) && ((y + j) < height)) {
						Offset = (y + j) * (width * 4) + (x + i) * 4 + 3;
						data[Offset] = alphas[bits & 0x07];
					}
					bits >>= 3;
				}
			}

			// Last three bytes
			bits = SDL_SwapLE32(*((int32*)&alphamask[3]));
			for (j = 2; j < 4; j++) {
				for (i = 0; i < 4; i++) {
					// only put pixels out < width or height
					if (((x + i) < width) && ((y + j) < height)) {
						Offset = (y + j) * (width * 4) + (x + i) * 4 + 3;
						data[Offset] = alphas[bits & 0x07];
					}
					bits >>= 3;
				}
			}
		}
	}

	return true;
}
