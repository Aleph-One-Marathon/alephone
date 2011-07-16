#ifndef _DIRECTDRAWSURFACE_H_
#define _DIRECTDRAWSURFACE_H_
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
 *  Written in 2005 by Gregory Smith
 *  From the Directx9 reference
 * http://msdn.microsoft.com/archive/default.asp?url=/archive/en-us/directx9_c/directx/graphics/reference/ddsfilereference/ddsfileformat.asp
 */
 
#include "cstypes.h"

#ifndef __DDRAW_INCLUDED__

#define DDSD_CAPS          0x00000001
#define DDSD_HEIGHT        0x00000002
#define DDSD_WIDTH         0x00000004
#define DDSD_PITCH         0x00000008
#define DDSD_PIXELFORMAT   0x00001000
#define DDSD_MIPMAPCOUNT   0x00002000
#define DDSD_LINEARSIZE    0x00008000
#define DDSD_DEPTH         0x00800000

#define DDPF_ALPHAPIXELS   0x00000001
#define DDPF_FOURCC        0x00000004
#define DDPF_RGB           0x00000040

#define DDSCAPS_COMPLEX    0x00000008
#define DDSCAPS_TEXTURE    0x00001000
#define DDSCAPS_MIPMAP     0x00400000

#define DDSCAPS2_CUBEMAP   0x00000200
#define DDSCAPS2_VOLUME    0x00200000

struct DDSURFACEDESC2
{
	uint32 dwSize;
	uint32 dwFlags;
	uint32 dwHeight;
	uint32 dwWidth;
	uint32 dwPitchOrLinearSize;
	uint32 dwDepth;
	uint32 dwMipMapCount;
	uint32 dwReserved1[11];

	struct {
		uint32 dwSize;
		uint32 dwFlags;
		uint32 dwFourCC;
		uint32 dwRGBBitCount;
		uint32 dwRBitMask;
		uint32 dwGBitMask;
		uint32 dwBBitMask;
		uint32 dwRGBAlphaBitMask;
	} ddpfPixelFormat;

	struct {
		uint32 dwCaps1;
		uint32 dwCaps2;
		uint32 Reserved[2];
	} ddsCaps;

	uint32 dwReserved2;
};
	
#endif
#endif
