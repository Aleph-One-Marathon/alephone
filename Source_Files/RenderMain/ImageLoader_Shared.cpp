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

*/

/*
 *  ImageLoader_Shared.cpp - Image file loading extras
 *
 *  Written in 2005 by Gregory Smith
 */

#include "ImageLoader.h"

uint32 *GetMipMapPtr(uint32 *pixels, int size, int level, int width, int height, int format)
{
	if (format == ImageFormat_RGBA) 
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
	return ::GetMipMapPtr(&Pixels[0], Pixels.size(), Level, Width, Height, Format);
}
		
