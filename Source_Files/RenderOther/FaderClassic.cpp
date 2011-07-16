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

	Loren Petrich:
	Created to implement the Classic faders in a Carbon app
*/

// From csmacros.h
#undef MAX
#define MAX(a,b) ((a)>(b) ? (a) : (b))
#undef MIN
#define MIN(a,b) ((a)<(b) ? (a) : (b))


#pragma export on

// Inspired by cstypes.h
typedef unsigned short uint16;

// From cscluts.h
typedef struct rgb_color {
	uint16 red;
	uint16 green;
	uint16 blue;
} rgb_color;

typedef struct color_table {
	short color_count;
	rgb_color colors[256];
} color_table;

void animate_screen_clut_classic(
	struct color_table *color_table,
	GDHandle DevHdl);

#pragma export off

// Directly manipulate the video-driver color table.
// Written so that the faders will work in MacOS X Classic.
void animate_screen_clut_classic(
	struct color_table *color_table,
	GDHandle DevHdl)
{
	// Check to see if "Control" is loaded:
	if (Control == NULL) return;
		
	// Much of this code is taken from Apple's sample app "MacGamma";
	// the MacOS headers to look in for the MacOS data structures are
	// Devices.h, Quickdraw.h, and Video.h
	
	CntrlParam CParam;					// Set up the control parameters
	CParam.ioCompletion = NULL;
	CParam.ioNamePtr = NULL;
	CParam.ioVRefNum = 0;
	CParam.ioCRefNum = (**DevHdl).gdRefNum;
	CParam.csCode = cscGetGamma;
	
	VDGammaRecord DeviceGammaRec;		// Destination for gamma data; manipulate in place
	
	*(Ptr *)CParam.csParam = (Ptr) &DeviceGammaRec;
	OSErr Err = PBStatus((ParmBlkPtr)&CParam, 0);
	if (Err != noErr) return;
	
	GammaTblPtr GTable = GammaTblPtr(DeviceGammaRec.csGTable);
	if (!GTable) return;
	
	if (GTable->gChanCnt < 3) return;						// Won't do anything if grayscale
	short GTBits = GTable->gDataWidth;						// How many bits per table entry?
	short NumEntries = MIN(GTable->gDataCnt,color_table->color_count);	// How many entries to handle
	
	// Copy over the color data; from chunky 16-bit to planar GTBits-bit
	short Shift = 16 - GTBits;
	if (GTBits > 8)
	{
		unsigned short *EntryPtr, *EntryBase =
			(unsigned short *)(&GTable->gFormulaData + GTable->gFormulaSize); // base of table
		
		// Red
		EntryPtr = EntryBase;
		for (int k=0; k<NumEntries; k++, EntryPtr++)
			*EntryPtr = color_table->colors[k].red >> Shift;
		
		// Green
		EntryPtr = EntryBase + GTable->gDataCnt;
		for (int k=0; k<NumEntries; k++, EntryPtr++)
			*EntryPtr = color_table->colors[k].green >> Shift;
		
		// Blue
		EntryPtr = EntryBase + 2*GTable->gDataCnt;
		for (int k=0; k<NumEntries; k++, EntryPtr++)
			*EntryPtr = color_table->colors[k].blue >> Shift;
	}
	else
	{
		unsigned char *EntryPtr, *EntryBase =
			(unsigned char *)(&GTable->gFormulaData + GTable->gFormulaSize); // base of table
		
		// Red
		EntryPtr = EntryBase;
		for (int k=0; k<NumEntries; k++, EntryPtr++)
			*EntryPtr = color_table->colors[k].red >> Shift;
		
		// Green
		EntryPtr = EntryBase + GTable->gDataCnt;
		for (int k=0; k<NumEntries; k++, EntryPtr++)
			*EntryPtr = color_table->colors[k].green >> Shift;
		
		// Blue
		EntryPtr = EntryBase + 2*GTable->gDataCnt;
		for (int k=0; k<NumEntries; k++, EntryPtr++)
			*EntryPtr = color_table->colors[k].blue >> Shift;
	}
	
	short DevRefNum = (**DevHdl).gdRefNum;
	Ptr DGRPtr = (Ptr) &DeviceGammaRec;
	
	Err = Control(DevRefNum, cscSetGamma, &DGRPtr);
}

