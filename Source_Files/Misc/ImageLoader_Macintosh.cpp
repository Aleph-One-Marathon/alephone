/*
	
	Image Loader,
	by Loren Petrich,
	October 31, 2000

	This file contains a function for loading an image;
	it is MacOS-specific, but it should be easy to create an SDL version.
*/


#include <QDOffscreen.h>
#include <QuickTimeComponents.h>
#include "ImageLoader.h"
#include "shell.h"


bool LoadImageFromFile(ImageDescriptor& Img, FileSpecifier& File)
{
	// Needs QT, of course:
	if (!machine_has_quicktime()) return false;
	
	
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
	Img.Resize(Width,Height);
	
	// Set pointers:
	byte *PixMap = (byte *)GetPixBaseAddr(PxlMapHdl);
	int NumRowPixels = int((**PxlMapHdl).rowBytes & 0x7fff);
	byte *RowBegin = PixMap;
	uint32 *DestPxlPtr = Img.GetPixelBasePtr();
	
	for (int h=0; h<Height; h++) {
		byte *PixPtr = RowBegin;
		for (int w=0; w<Width; w++) {
			int32 DestPxl;
			byte *DPCP = (byte *)(&DestPxl);
			// ARGB to RGBA
			PixPtr++;
			DPCP[0] = *(PixPtr++);
			DPCP[1] = *(PixPtr++);
			DPCP[2] = *(PixPtr++);
			DPCP[3] = 0xff;			// Completely opaque
			*(DestPxlPtr++) = DestPxl;
		}
		RowBegin += NumRowPixels;
	}
	
	UnlockPixels(PxlMapHdl);
	DisposeGWorld(ImgGW);
	return true;
}
