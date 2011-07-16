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
	
	Image Loader,
	by Loren Petrich,
	October 31, 2000

	This file contains a function for loading an image;
	it is MacOS-specific, but it should be easy to create an SDL version.

Nov 12, 2000 (Loren Petrich):
	Added opacity-loading support

Jan 25, 2002 (Br'fin (Jeremy Parsons)):
	Added TARGET_API_MAC_CARBON for Carbon.h, Quicktime.h
*/


#if defined(EXPLICIT_CARBON_HEADER)
    #include <Carbon/Carbon.h>
    #include <quicktime/QuickTimeComponents.h>
#else
//#include <QDOffscreen.h>
#include <QuickTimeComponents.h>
#endif
#include "ImageLoader.h"
#include "shell.h"

bool ImageDescriptor::LoadFromFile(FileSpecifier& File, int ImgMode, int flags, int actual_height, int actual_width, int maxSize)
{
	// Needs QT, of course:
	if (!machine_has_quicktime()) return false;

	if (flags & ImageLoader_ImageIsAlreadyPremultiplied)
		PremultipliedAlpha = true;
	
	// Don't load opacity if there is no color component:
	switch(ImgMode)
	{
	case ImageLoader_Colors:
		if (LoadDDSFromFile(File, flags, actual_width, actual_height, maxSize)) return true;
		break;
		
	case ImageLoader_Opacity:
		if (!IsPresent()) return false;
		break;
		
	default:
		vassert(false,csprintf(temporary,"Bad image mode for loader: %d",ImgMode));
	}
	
	// Get the graphics-importing component
	GraphicsImportComponent Importer;
	ComponentResult Result = GetGraphicsImporterForFile(&File.GetSpec(), &Importer);
	if (Result != noErr)
	{
		return false;
	}
	
	// Get the image dimensions
	Rect ImgRect;
	GraphicsImportGetBoundsRect(Importer, &ImgRect);
	
	// Create a GWorld for it
	GWorldPtr ImgGW;
	OSErr Err = NewGWorld(&ImgGW, 32, &ImgRect, nil, nil, 0);
	if (Err != noErr)
	{
		CloseComponent(Importer);
		return false;
	}
	
	// Read in the image
	PixMapHandle PxlMapHdl = GetGWorldPixMap(ImgGW);
	LockPixels(PxlMapHdl);
	GraphicsImportSetGWorld(Importer, ImgGW, nil);
	Result = GraphicsImportDraw(Importer);		// Draw into GWorld
	CloseComponent(Importer); 					// Cleanup
	if (Result != noErr)
	{
		UnlockPixels(PxlMapHdl);
		DisposeGWorld(ImgGW);
		return false;
	}
		
	// Get image dimensions and set its size
	int Width = ImgRect.right - ImgRect.left;
	int Height = ImgRect.bottom - ImgRect.top;
	int OriginalWidth = Width;
	int OriginalHeight = Height;
	if (flags & ImageLoader_ResizeToPowersOfTwo) {
		Width = NextPowerOfTwo(Width);
		Height = NextPowerOfTwo(Height);
	}
	switch(ImgMode)
	{
	case ImageLoader_Colors:
		Resize(Width,Height);
		VScale = ((double) OriginalWidth / (double) Width);
		UScale = ((double) OriginalHeight / (double) Height);
		MipMapCount = 1;
		break;
		
	case ImageLoader_Opacity:
		// If the wrong size, then bug out
		if (Width != this->Width || Height != this->Height || ((double) OriginalWidth / Width != VScale || ((double) OriginalHeight / Height != UScale)))
		{
			UnlockPixels(PxlMapHdl);
			DisposeGWorld(ImgGW);
			return false;
		}
		break;
	}
	
	// Set pointers:
	byte *PixMap = (byte *)GetPixBaseAddr(PxlMapHdl);
	int NumRowBytes = int((**PxlMapHdl).rowBytes & 0x7fff);
	byte *RowBegin = PixMap;
	uint32 *DestPxlPtr = GetPixelBasePtr();
	
	switch(ImgMode)
	{
	case ImageLoader_Colors:
		for (int h=0; h<OriginalHeight; h++) {
			byte *PixPtr = RowBegin;
			for (int w=0; w<OriginalWidth; w++) {
				uint32 DestPxl;
				byte *DPCP = (byte *)(&DestPxl);
				// ARGB to RGBA
				PixPtr++;
				DPCP[0] = *(PixPtr++);
				DPCP[1] = *(PixPtr++);
				DPCP[2] = *(PixPtr++);
				DPCP[3] = 0xff;			// Completely opaque
				*(DestPxlPtr++) = DestPxl;
			}
			RowBegin += NumRowBytes;
			DestPxlPtr += (Width - OriginalWidth);
		}
		break;
	
	case ImageLoader_Opacity:
		for (int h=0; h<OriginalHeight; h++) {
			byte *PixPtr = RowBegin;
			for (int w=0; w<OriginalWidth; w++) {
				uint32 DestPxl = *DestPxlPtr;
				byte *DPCP = (byte *)(&DestPxl);
				// ARGB to grayscale value, and then to the opacity
				PixPtr++;
				float Red = float(*(PixPtr++));
				float Green = float(*(PixPtr++));
				float Blue = float(*(PixPtr++));
				float Opacity = (Red + Green + Blue)/3;
				DPCP[3] = PIN(int(Opacity + 0.5),0,255);
				*(DestPxlPtr++) = DestPxl;
			}
			RowBegin += NumRowBytes;
			DestPxlPtr += (Width - OriginalWidth);
		}
		break;
	}
	
	UnlockPixels(PxlMapHdl);
	DisposeGWorld(ImgGW);
	return true;
}
