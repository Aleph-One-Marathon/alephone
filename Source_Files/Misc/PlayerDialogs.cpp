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

	February 21, 2000 (Loren Petrich)
	
	These are some extra dialog routines associated with setting such player stuff
	as the chase cam and the crosshairs.

	Feb 25, 2000 (Loren Petrich):
	
	Split the rendering routines up into routines that need different parameters.
	
	Feb 26, 2000 (Loren Petrich):
	Added support for "on when entering" for the chase cam when a game starts.
	
	Jun 11, 2000 (Loren Petrich):
	Used a class created for the checkboxes

Jan 25, 2002 (Br'fin (Jeremy Parsons)):
	Added accessors for datafields now opaque in Carbon

Feb 5, 2002 (Br'fin (Jeremy Parsons)):
	Put player chase cam and crosshair dialogs in sheets under Carbon

Feb 14, 2002 (Br'fin (Jeremy Parsons)):
	Made the Carbon sheets backgrounds transparent
*/

#include "cseries.h"
#include "interface.h"
#include "world.h"
#include "ChaseCam.h"
#include "Crosshairs.h"
#include "OGL_Setup.h"
#include "MacCheckbox.h"
#include <stdio.h>

const float FLOAT_WORLD_ONE = float(WORLD_ONE);

enum
{
	OK_Item = 1,
	Cancel_Item = 2,
	
	ChaseCam_Dialog = 1500,
	Behind_Item = 5,
	Upward_Item = 7,
	Rightward_Item = 9,
	PassThruWall_Item = 10,
	NeverActive_Item = 11,
	OnWhenEntering_Item = 12,
	VoidColorOnOff_Item = 13,
	VoidColorSelect_Item = 14,
	
	Crosshairs_Dialog = 1600,
	Thickness_Item = 5,
	FromCenter_Item = 7,
	Length_Item = 9,
	GetColor_Item = 10,
	Show_Item = 11,
	Preview_Item = 12,
	BkgdColor_Item = 13
};

// Background color
static RGBColor BkgdColor = rgb_black;

// These need to return whether the text field was correctly parsed or not

static bool GetShort(ControlHandle Hdl, short& Value)
{
	GetDialogItemText((Handle)Hdl,ptemporary);
	ptemporary[ptemporary[0]+1] = 0;
	return (sscanf(temporary+1,"%hd",&Value) == 1);
}

static bool GetFloat(ControlHandle Hdl, float& Value)
{
	GetDialogItemText((Handle)Hdl,ptemporary);
	ptemporary[ptemporary[0]+1] = 0;
	return (sscanf(temporary+1,"%f",&Value) == 1);
}

static void SetShort(ControlHandle Hdl, short Value)
{
	NumToString(Value,ptemporary);
	SetDialogItemText((Handle)Hdl, ptemporary);
}

static void SetFloat(ControlHandle Hdl, float Value)
{
	psprintf(ptemporary, "%f",Value);
	SetDialogItemText((Handle)Hdl, ptemporary);
}

static void ToggleControl(ControlHandle Hdl)
{
	SetControlValue(Hdl, 1 - GetControlValue(Hdl));
}

static int FloatRoundoff(float x)
{
	return (x >= 0) ? int(x + 0.5) : - int(-x + 0.5);
}

// True for OK, false for cancel
bool Configure_ChaseCam(ChaseCamData &Data)
{
	short ItemType;
	Rect Bounds;
	
	DialogPtr Dialog = myGetNewDialog(ChaseCam_Dialog, NULL, (WindowPtr)(-1), 0);
	assert(Dialog);
	
	ControlHandle Behind_CHdl;
	GetDialogItem(Dialog, Behind_Item, &ItemType, (Handle *)&Behind_CHdl, &Bounds);
	SetFloat(Behind_CHdl,Data.Behind/FLOAT_WORLD_ONE);
	
	ControlHandle Upward_CHdl;
	GetDialogItem(Dialog, Upward_Item, &ItemType, (Handle *)&Upward_CHdl, &Bounds);
	SetFloat(Upward_CHdl,Data.Upward/FLOAT_WORLD_ONE);
	
	ControlHandle Rightward_CHdl;
	GetDialogItem(Dialog, Rightward_Item, &ItemType, (Handle *)&Rightward_CHdl, &Bounds);
	SetFloat(Rightward_CHdl,Data.Rightward/FLOAT_WORLD_ONE);
	
	MacCheckbox PassThruWall_CB(Dialog, PassThruWall_Item, TEST_FLAG(Data.Flags,_ChaseCam_ThroughWalls) != 0);
	MacCheckbox NeverActive_CB(Dialog, NeverActive_Item, TEST_FLAG(Data.Flags,_ChaseCam_NeverActive) != 0);
	MacCheckbox OnWhenEntering_CB(Dialog, OnWhenEntering_Item, TEST_FLAG(Data.Flags,_ChaseCam_OnWhenEntering) != 0);
	
	// Where to make the color picker
	Point Center = {-1,-1};
	RGBColor NewColor;
	// Get void color from OpenGL-parameters data
	OGL_ConfigureData& OGLData = Get_OGL_ConfigureData();
	MacCheckbox VoidColorOnOff_CB(Dialog, VoidColorOnOff_Item, TEST_FLAG(OGLData.Flags,OGL_Flag_VoidColor) != 0);
	
	// Reveal it
#if defined(USE_CARBON_ACCESSORS)
#if USE_SHEETS
	SetThemeWindowBackground(GetDialogWindow(Dialog), kThemeBrushSheetBackgroundTransparent, false);
	ShowSheetWindow(GetDialogWindow(Dialog), ActiveNonFloatingWindow());
#else
	SelectWindow(GetDialogWindow(Dialog));
	ShowWindow(GetDialogWindow(Dialog));
#endif
#else
	SelectWindow(Dialog);
	ShowWindow(Dialog);
#endif
	
	bool WillQuit = false;
	bool IsOK = false;
	short New_Behind = 0, New_Upward = 0, New_Rightward = 0;
	float FloatTemp = 0;
	bool BadValue;
	while(!WillQuit)
	{
		short ItemHit;
		ModalDialog(NULL, &ItemHit);
		
		switch(ItemHit)
		{
		case OK_Item:
		// Check before quitting
			BadValue = false;
			
			// Now doing roundoff correctly
			// Using a modification of AlexJLS's corrected version
			
			if (GetFloat(Behind_CHdl,FloatTemp))
				New_Behind = FloatRoundoff(WORLD_ONE * FloatTemp);
			else
				BadValue = true;
			
			if (GetFloat(Upward_CHdl,FloatTemp))
				New_Upward = FloatRoundoff(WORLD_ONE * FloatTemp);
			else
				BadValue = true;
			
			if (GetFloat(Rightward_CHdl,FloatTemp))
				New_Rightward = FloatRoundoff(WORLD_ONE * FloatTemp);
			else
				BadValue = true;
			
			if (BadValue)
			{
				SysBeep(30);
				break;
			}
		
			IsOK = true;
			WillQuit = true;
			break;
			
		case Cancel_Item:
			IsOK = false;
			WillQuit = true;
			break;
			
		case VoidColorSelect_Item:
			// Need to set color here so the preview can work properly
			if (GetColor(Center,"\pWhat color for the void?",&OGLData.VoidColor,&NewColor))
				OGLData.VoidColor = NewColor;
			break;
		
		default:
			if (PassThruWall_CB.ToggleIfHit(ItemHit)) break;
			if (NeverActive_CB.ToggleIfHit(ItemHit)) break;
			if (OnWhenEntering_CB.ToggleIfHit(ItemHit)) break;
			if (VoidColorOnOff_CB.ToggleIfHit(ItemHit)) break;
			break;
		}
	}
	
	if (IsOK)
	{
		Data.Behind = New_Behind;
		Data.Upward = New_Upward;
		Data.Rightward = New_Rightward;
		SET_FLAG(Data.Flags,_ChaseCam_ThroughWalls,PassThruWall_CB.GetState());
		SET_FLAG(Data.Flags,_ChaseCam_NeverActive,NeverActive_CB.GetState());
		SET_FLAG(Data.Flags,_ChaseCam_OnWhenEntering,OnWhenEntering_CB.GetState());
		SET_FLAG(OGLData.Flags,OGL_Flag_VoidColor,VoidColorOnOff_CB.GetState());
	}
	
	// Clean up
#if defined(USE_CARBON_ACCESSORS)
#if USE_SHEETS
	HideSheetWindow(GetDialogWindow(Dialog));
#else
	HideWindow(GetDialogWindow(Dialog));
#endif
#else
	HideWindow(Dialog);
#endif
	DisposeDialog(Dialog);
	
	return IsOK;
}


// Procedure for displaying the preview
static pascal void DoPreview(DialogPtr Dialog, short ItemNo)
{
	// Get essential stuff on the item to receive the preview
	short ItemType;
	Rect Bounds;
	ControlHandle CHdl;
	GetDialogItem(Dialog, ItemNo, &ItemType, (Handle *)&CHdl, &Bounds);

	// Push previous state
	PenState OldPen;
	RGBColor OldBackColor, OldForeColor;
	
	GetPenState(&OldPen);
	GetBackColor(&OldBackColor);
	GetForeColor(&OldForeColor);
	
	// Get ready to do the drawing
	PenNormal();
	
	// Draw the background
	RGBForeColor(&BkgdColor);
	PaintRect(&Bounds);
	
	// Push old crosshair state
	bool OldCrosshairState = Crosshairs_IsActive();
	Crosshairs_SetActive(true);
	
	// Clip to inside of box
	ClipRect(&Bounds);
	
	// Draw the crosshairs
	Crosshairs_Render(Bounds);
	
	// No more clipping
#if defined(USE_CARBON_ACCESSORS)
	Rect portRect;
	GetPortBounds(GetWindowPort(GetDialogWindow(Dialog)), &portRect);
	ClipRect(&portRect);
#else
	ClipRect(&Dialog->portRect);
#endif
	
	// Pop old crosshair state
	Crosshairs_SetActive(OldCrosshairState);
	
	// Draw the boundary line
	RGBForeColor(&rgb_black);
	FrameRect(&Bounds);
	
	// Pop previous state
	SetPenState(&OldPen);
	RGBBackColor(&OldBackColor);
	RGBForeColor(&OldForeColor);
}


// True for OK, false for cancel
bool Configure_Crosshairs(CrosshairData &Data)
{
	short ItemType;
	Rect Bounds;
	
	DialogPtr Dialog = myGetNewDialog(Crosshairs_Dialog, NULL, (WindowPtr)(-1), 0);
	assert(Dialog);
	
	ControlHandle Thickness_CHdl;
	GetDialogItem(Dialog, Thickness_Item, &ItemType, (Handle *)&Thickness_CHdl, &Bounds);
	SetShort(Thickness_CHdl,Data.Thickness);
	
	ControlHandle FromCenter_CHdl;
	GetDialogItem(Dialog, FromCenter_Item, &ItemType, (Handle *)&FromCenter_CHdl, &Bounds);
	SetShort(FromCenter_CHdl,Data.FromCenter);
	
	ControlHandle Length_CHdl;
	GetDialogItem(Dialog, Length_Item, &ItemType, (Handle *)&Length_CHdl, &Bounds);
	SetShort(Length_CHdl,Data.Length);
	
	// Create a UPP for the crosshair preview and store it
	UserItemUPP DoPreviewUPP = NewUserItemUPP(DoPreview);
	ControlHandle Show_CHdl;
	GetDialogItem(Dialog, Show_Item, &ItemType, (Handle *)&Show_CHdl, &Bounds);	
	SetDialogItem(Dialog, Show_Item, ItemType, Handle(DoPreviewUPP), &Bounds);
	
	// Where to make the color picker
	Point Center = {-1,-1};
	
	// Remembering the old values is necessary for the preview to work properly
	
	// Remember old color just in case of cancellation
	// Also, some temporary output for the color picker
	RGBColor OldColor = Data.Color, NewColor;
	
	// Remember other old values in case of cancellation
	short Old_Thickness = Data.Thickness;
	short Old_FromCenter = Data.FromCenter;
	short Old_Length = Data.Length;
	
	// Reveal it
#if defined(USE_CARBON_ACCESSORS)
#if USE_SHEETS
	SetThemeWindowBackground(GetDialogWindow(Dialog), kThemeBrushSheetBackgroundTransparent, false);
	ShowSheetWindow(GetDialogWindow(Dialog), ActiveNonFloatingWindow());
#else
	SelectWindow(GetDialogWindow(Dialog));
	ShowWindow(GetDialogWindow(Dialog));
#endif
#else
	SelectWindow(Dialog);
	ShowWindow(Dialog);
#endif
	
	bool WillQuit = false;
	bool IsOK = false;
	short ShortTemp;
	bool BadValue;
	while(!WillQuit)
	{
		short ItemHit;
		ModalDialog(NULL, &ItemHit);
		
		switch(ItemHit)
		{
		case OK_Item:
		// Check before quitting
			BadValue = false;
			
			if (GetShort(Thickness_CHdl,ShortTemp))
			{
				if (ShortTemp > 0)
					Data.Thickness = ShortTemp;
				else
					BadValue = true;
			} else
				BadValue = true;
			
			if (GetShort(FromCenter_CHdl,ShortTemp))
			{
				if (ShortTemp > 0)
					Data.FromCenter = ShortTemp;
				else
					BadValue = true;
			} else
				BadValue = true;
			
			if (GetShort(Length_CHdl,ShortTemp))
			{
				if (ShortTemp > 0)
					Data.Length = ShortTemp;
				else
					BadValue = true;
			} else
				BadValue = true;
			
			if (BadValue)
			{
				SysBeep(30);
				break;
			}
		
			IsOK = true;
			WillQuit = true;
			break;
			
		case Cancel_Item:
			IsOK = false;
			WillQuit = true;
			break;
		
		case GetColor_Item:
			// Need to set color here so the preview can work properly
			if (GetColor(Center,"\pWhat crosshair color?",&Data.Color,&NewColor))
				Data.Color = NewColor;
			DrawDialog(Dialog);
			break;
		
		case Preview_Item:
			BadValue = false;
			
			if (GetShort(Thickness_CHdl,ShortTemp))
			{
				if (ShortTemp > 0)
					Data.Thickness = ShortTemp;
				else
					BadValue = true;
			} else
				BadValue = true;
			
			if (GetShort(FromCenter_CHdl,ShortTemp))
			{
				if (ShortTemp > 0)
					Data.FromCenter = ShortTemp;
				else
					BadValue = true;
			} else
				BadValue = true;
			
			if (GetShort(Length_CHdl,ShortTemp))
			{
				if (ShortTemp > 0)
					Data.Length = ShortTemp;
				else
					BadValue = true;
			} else
				BadValue = true;
						
			if (BadValue) SysBeep(30);
			
			DrawDialog(Dialog);
			break;
		
		case BkgdColor_Item:
			if (GetColor(Center,"\pWhat preview background color?",&BkgdColor,&NewColor))
				BkgdColor = NewColor;
			DrawDialog(Dialog);
			break;
		}
	}
	
	if (!IsOK)
	{
		// Revert all these values
		Data.Thickness = Old_Thickness;
		Data.FromCenter = Old_FromCenter;
		Data.Length = Old_Length;
		Data.Color = OldColor;
	}
	
	// Clean up
#if defined(USE_CARBON_ACCESSORS)
#if USE_SHEETS
	HideSheetWindow(GetDialogWindow(Dialog));
#else
	HideWindow(GetDialogWindow(Dialog));
#endif
#else
	HideWindow(Dialog);
#endif
	DisposeUserItemUPP(DoPreviewUPP);
	DisposeDialog(Dialog);
	
	return IsOK;
}