#ifndef _IMAGE_LOADER_
#define _IMAGE_LOADER_
/*
	
	Image-Loader Interface File,
	by Loren Petrich,
	October 21, 2000

	This file contains an image-descriptor object and a function for loading the image
	from a file.
	
*/

#include <vector.h>
#include "cseries.h"
#include "FileHandler.h"


// Need an object to hold the read-in image.
class ImageDescriptor
{
	int Width;	// along scanlines
	int Height;	// scanline to scanline
	vector<uint32> Pixels;	// in 32-bit format
public:
	
	// Is an image present?
	bool IsPresent() {return !Pixels.empty();}
	
	// Size
	int GetWidth() {return Width;}
	int GetHeight() {return Height;}
	int GetNumPixels() {return Width*Height;}
	
	// Pixel accessors
	uint32& GetPixel(int Horiz, int Vert) {return Pixels[Width*Vert + Horiz];}
	uint32 *GetPixelBasePtr() {return &Pixels[0];}
	
	// Reallocation
	void Resize(int _Width, int _Height)
		{Width = _Width, Height = _Height, Pixels.resize(GetNumPixels());}
	
	// Clearing
	void Clear()
		{Width = Height = 0; Pixels.clear();}
	
	ImageDescriptor(): Width(0), Height(0) {}
};


// Returns whether or not the loading was successful
bool LoadImageFromFile(ImageDescriptor& Img, FileSpecifier& File);


#endif
