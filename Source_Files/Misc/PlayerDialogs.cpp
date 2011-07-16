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

Jun 26, 2002 (Loren Petrich):
	Added support for additional crosshairs features;
	included revert on bad value for crosshairs dialog
*/

#include "cseries.h"
#include "interface.h"
#include "world.h"
#include "ChaseCam.h"
#include "Crosshairs.h"
#include "OGL_Setup.h"
#include "MacCheckbox.h"
#include "shell.h"
#include <math.h>
#include <stdio.h>

#ifdef USES_NIBS
	#include "NibsUiHelpers.h"
#endif

const float FLOAT_WORLD_ONE = float(WORLD_ONE);

#ifdef USES_NIBS

const CFStringRef Window_Prefs_ChaseCam = CFSTR("Prefs_ChaseCam");

const CFStringRef Window_Prefs_Crosshairs = CFSTR("Prefs_Crosshairs");

#endif

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
	Damping_Item = 16,
	Spring_Item = 18,
	CC_Opacity_Item = 20,
	VerifyNumbers_Item = 21,
	
	Crosshairs_Dialog = 1600,
	Thickness_Item = 5,
	FromCenter_Item = 7,
	Length_Item = 9,
	GetColor_Item = 10,
	Show_Item = 11,
	Preview_Item = 12,
	BkgdColor_Item = 13,
	Shape_Item = 14,
	CH_Opacity_Item = 16
};

// Background color
static RGBColor BkgdColor = rgb_black;


#ifdef USES_NIBS


// These need to return whether the text field was correctly parsed or not

static bool GetShort(ControlRef Ctrl, short& Value)
{
	GetEditCText(Ctrl, temporary);
	return (sscanf(temporary, "%hd", &Value) == 1);
}

static bool GetFloat(ControlRef Ctrl, float& Value)
{
	GetEditCText(Ctrl, temporary);
	return (sscanf(temporary, "%f", &Value) == 1);
}

static void SetShort(ControlRef Ctrl, short Value)
{
	sprintf(temporary, "%hd", Value);
	SetEditCText(Ctrl, temporary);
}

static void SetFloat(ControlRef Ctrl, float Value)
{
	sprintf(temporary, "%f", Value);
	SetEditCText(Ctrl, temporary);
}

static int FloatRoundoff(float x)
{
	return (x >= 0) ? int(x + 0.5) : - int(-x + 0.5);
}


struct Configure_ChaseCam_HandlerData
{
	// All these must be verified
	ControlRef BehindCtrl;
	ControlRef UpwardCtrl;
	ControlRef RightwardCtrl;
	ControlRef DampingCtrl;
	ControlRef SpringCtrl;
	ControlRef CC_OpacityCtrl;
	
	// Void color to be picked
	RGBColor VoidColor;
	
	// Verifies and performs necessary adjustments;
	// returns whether all values were OK
	bool VerifyAndAdjust();
};

bool Configure_ChaseCam_HandlerData::VerifyAndAdjust()
{
	float Temp;
	float Damping, Spring;
	bool IsOK = true;
	
	// First, check the spring-constant factors
	if (GetFloat(DampingCtrl,Damping))
	{
		// Simple damping-factor validation
		float NewTemp = PIN(Damping, -1, 1);
		if (NewTemp != Damping)
		{
			Damping = NewTemp;
			SetFloat(DampingCtrl,Damping);
			IsOK = false;
		}
	}
	else
		IsOK = false;

	IsOK = IsOK && GetFloat(SpringCtrl,Spring);

	if (IsOK)
	{
		// Do validation: will the chase cam be unstable?			

		if (Spring >= 0)
		{
			// Oscillatory case
			float DampSq = Damping*Damping;
			if ((DampSq + Spring) >= 1)
			{
				Spring = 1 - DampSq;
				SetFloat(SpringCtrl,Spring);
				IsOK = false;
			}
		}
		else
		{
			// Overdamped case
			float DampAbs = fabs(Damping);
			if ((DampAbs + sqrt(-Spring)) >= 1)
			{
				float DACmpl = 1 - DampAbs;
				Spring = - DACmpl*DACmpl;
				SetFloat(SpringCtrl,Spring);
				IsOK = false;
			}
		}
	}
	
	// Don't really need these values here
	IsOK = IsOK && GetFloat(BehindCtrl,Temp);
	IsOK = IsOK && GetFloat(UpwardCtrl,Temp);
	IsOK = IsOK && GetFloat(RightwardCtrl,Temp);
	
	// Get the opacity to within a reasonable range
	if (GetFloat(CC_OpacityCtrl,Temp))
	{
		float NewTemp = PIN(Temp, 0, 1);
		if (NewTemp != Temp)
		{
			SetFloat(CC_OpacityCtrl, NewTemp);
			IsOK = false;
		}
	}
	else
		IsOK = false;
	
	if (!IsOK) SysBeep(30);
	
	return IsOK;
}


static void Configure_ChaseCam_Handler(ParsedControl &Ctrl, void *Data)
{
	Configure_ChaseCam_HandlerData *HDPtr = (Configure_ChaseCam_HandlerData *)(Data);

	switch (Ctrl.ID.id)
	{
	case VoidColorSelect_Item:
		PickControlColor(Ctrl.Ctrl, &HDPtr->VoidColor, "\pSelect Color of Void");
		break;
	
	case VerifyNumbers_Item:
		HDPtr->VerifyAndAdjust();
		break;
	}
}


// True for OK, false for cancel
bool Configure_ChaseCam(ChaseCamData &Data)
{
	// Get the window
	AutoNibReference Nib (Window_Prefs_ChaseCam);
	AutoNibWindow Window (Nib.nibReference (), Window_Prefs_ChaseCam);

	// Get the controls
	Configure_ChaseCam_HandlerData HandlerData;
	
	HandlerData.BehindCtrl = GetCtrlFromWindow(Window(), 0, Behind_Item);
	SetFloat(HandlerData.BehindCtrl,Data.Behind/FLOAT_WORLD_ONE);
	
	HandlerData.UpwardCtrl = GetCtrlFromWindow(Window(), 0, Upward_Item);
	SetFloat(HandlerData.UpwardCtrl,Data.Upward/FLOAT_WORLD_ONE);
	
	HandlerData.RightwardCtrl = GetCtrlFromWindow(Window(), 0, Rightward_Item);
	SetFloat(HandlerData.RightwardCtrl,Data.Rightward/FLOAT_WORLD_ONE);
	
	HandlerData.DampingCtrl = GetCtrlFromWindow(Window(), 0, Damping_Item);
	SetFloat(HandlerData.DampingCtrl,Data.Damping);
	
	HandlerData.SpringCtrl = GetCtrlFromWindow(Window(), 0, Spring_Item);
	SetFloat(HandlerData.SpringCtrl,Data.Spring);
	
	HandlerData.CC_OpacityCtrl = GetCtrlFromWindow(Window(), 0, CC_Opacity_Item);
	SetFloat(HandlerData.CC_OpacityCtrl,Data.Opacity);
	
	ControlRef PassThruWall_Ctrl = GetCtrlFromWindow(Window(), 0, PassThruWall_Item);
	SetControl32BitValue(PassThruWall_Ctrl, !!TEST_FLAG(Data.Flags,_ChaseCam_ThroughWalls));
	
	ControlRef NeverActive_Ctrl = GetCtrlFromWindow(Window(), 0, NeverActive_Item);
	SetControl32BitValue(NeverActive_Ctrl, !!TEST_FLAG(Data.Flags,_ChaseCam_NeverActive));
	
	ControlRef OnWhenEntering_Ctrl = GetCtrlFromWindow(Window(), 0, OnWhenEntering_Item);
	SetControl32BitValue(OnWhenEntering_Ctrl, !!TEST_FLAG(Data.Flags,_ChaseCam_OnWhenEntering));
	
	// Get void color from OpenGL-parameters data
	OGL_ConfigureData& OGLData = Get_OGL_ConfigureData();
	
	ControlRef VoidColorOnOff_Ctrl = GetCtrlFromWindow(Window(), 0, VoidColorOnOff_Item);
	SetControl32BitValue(VoidColorOnOff_Ctrl, !!TEST_FLAG(OGLData.Flags,OGL_Flag_VoidColor));
		
	HandlerData.VoidColor = OGLData.VoidColor;

	// For making the swatch drawable and hittable
	AutoDrawability Drawability;
	AutoHittability Hittability;
	
	ControlRef VoidSwatchCtrl = GetCtrlFromWindow(Window(), 0, VoidColorSelect_Item);
	
	Drawability(VoidSwatchCtrl, SwatchDrawer, &HandlerData.VoidColor);
	Hittability(VoidSwatchCtrl);
		
	bool IsOK = RunModalDialog(Window(), true, Configure_ChaseCam_Handler, &HandlerData);
	
	if (IsOK)
	{
		HandlerData.VerifyAndAdjust();
		
		float Temp;
		
		if (GetFloat(HandlerData.BehindCtrl,Temp))
			Data.Behind = FloatRoundoff(FLOAT_WORLD_ONE*Temp);
		
		if (GetFloat(HandlerData.UpwardCtrl,Temp));
			Data.Upward = FloatRoundoff(FLOAT_WORLD_ONE*Temp);
		
		if (GetFloat(HandlerData.RightwardCtrl,Temp));
			Data.Rightward = FloatRoundoff(FLOAT_WORLD_ONE*Temp);
		
		GetFloat(HandlerData.DampingCtrl, Data.Damping);
		
		GetFloat(HandlerData.SpringCtrl, Data.Spring);
		
		GetFloat(HandlerData.CC_OpacityCtrl, Data.Opacity);
		
		SET_FLAG(Data.Flags,_ChaseCam_ThroughWalls,GetControl32BitValue(PassThruWall_Ctrl));
		
		SET_FLAG(Data.Flags,_ChaseCam_NeverActive,GetControl32BitValue(NeverActive_Ctrl));
		
		SET_FLAG(Data.Flags,_ChaseCam_OnWhenEntering,GetControl32BitValue(OnWhenEntering_Ctrl));
	
		SET_FLAG(OGLData.Flags,OGL_Flag_VoidColor,GetControl32BitValue(VoidColorOnOff_Ctrl));
		
		OGLData.VoidColor = HandlerData.VoidColor;
	}
	
	return IsOK;
}


#else

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

/*
static void ToggleControl(ControlHandle Hdl)
{
	SetControlValue(Hdl, 1 - GetControlValue(Hdl));
}
*/

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
	
	MacCheckbox PassThruWall_CB(Dialog, PassThruWall_Item, TEST_FLAG(Data.Flags,_ChaseCam_ThroughWalls));
	MacCheckbox NeverActive_CB(Dialog, NeverActive_Item, TEST_FLAG(Data.Flags,_ChaseCam_NeverActive));
	MacCheckbox OnWhenEntering_CB(Dialog, OnWhenEntering_Item, TEST_FLAG(Data.Flags,_ChaseCam_OnWhenEntering));
	
	ControlHandle Damping_CHdl;
	GetDialogItem(Dialog, Damping_Item, &ItemType, (Handle *)&Damping_CHdl, &Bounds);
	SetFloat(Damping_CHdl,Data.Damping);
	
	ControlHandle Spring_CHdl;
	GetDialogItem(Dialog, Spring_Item, &ItemType, (Handle *)&Spring_CHdl, &Bounds);
	SetFloat(Spring_CHdl,Data.Spring);
	
	ControlHandle Opacity_CHdl;
	GetDialogItem(Dialog, CC_Opacity_Item, &ItemType, (Handle *)&Opacity_CHdl, &Bounds);
	SetFloat(Opacity_CHdl,Data.Opacity);
	
	// Where to make the color picker
	Point Center = {-1,-1};
	RGBColor NewColor;
	// Get void color from OpenGL-parameters data
	OGL_ConfigureData& OGLData = Get_OGL_ConfigureData();
	MacCheckbox VoidColorOnOff_CB(Dialog, VoidColorOnOff_Item, TEST_FLAG(OGLData.Flags,OGL_Flag_VoidColor));
	
	// Reveal it
#if USE_SHEETS
	SetThemeWindowBackground(GetDialogWindow(Dialog), kThemeBrushSheetBackgroundTransparent, false);
	ShowSheetWindow(GetDialogWindow(Dialog), ActiveNonFloatingWindow());
#else
	SelectWindow(GetDialogWindow(Dialog));
	ShowWindow(GetDialogWindow(Dialog));
#endif
	
	bool WillQuit = false;
	bool IsOK = false;
	short New_Behind = 0, New_Upward = 0, New_Rightward = 0;
	float FloatTemp = 0;
	bool BadValue;
	float New_Damping, New_Spring, New_Opacity;
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
			
			if (GetFloat(Damping_CHdl,FloatTemp))
			{
				// Simple validation of the damping factor
				New_Damping = PIN(FloatTemp,-1,1);
				if (New_Damping != FloatTemp)
				{
					BadValue = true;
					SetFloat(Damping_CHdl,New_Damping);
				}
			}
			else
				BadValue = true;
			
			if (GetFloat(Spring_CHdl,FloatTemp))
			{
				New_Spring = FloatTemp;
			}
			else
				BadValue = true;
			
			if (GetFloat(Opacity_CHdl,FloatTemp))
			{
				New_Opacity = PIN(FloatTemp,0,1);
				if (New_Opacity != FloatTemp)
				{
					BadValue = true;
					SetFloat(Opacity_CHdl,New_Opacity);
				}
			}
			else
				BadValue = true;
			
			// Do validation: will the chase cam be unstable?			
			if (!BadValue)
			{
				if (New_Spring >= 0)
				{
					// Oscillatory case
					float NewDampSq = New_Damping*New_Damping;
					BadValue = ((NewDampSq + New_Spring) >= 1);
					if (BadValue)
					{
						New_Spring = 1 - NewDampSq;
						SetFloat(Spring_CHdl,New_Spring);
					}
				}
				else
				{
					// Overdamped case
					float NewDampAbs = fabs(New_Damping);
					BadValue = ((NewDampAbs + sqrt(-New_Spring)) >= 1);
					if (BadValue)
					{
						float Temp = 1 - NewDampAbs;
						New_Spring = - Temp*Temp;
						SetFloat(Spring_CHdl,New_Spring);
					}
				}	
			}
			
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
		Data.Damping = New_Damping;
		Data.Spring = New_Spring;
		Data.Opacity = New_Opacity;
	}
	
	// Clean up
#if USE_SHEETS
	HideSheetWindow(GetDialogWindow(Dialog));
#else
	HideWindow(GetDialogWindow(Dialog));
#endif
	DisposeDialog(Dialog);
	
	return IsOK;
}


#endif


#ifdef USES_NIBS


void PreviewDrawer(ControlRef Ctrl, void *Data)
{
	// Don't need the data arg here
	CrosshairData &Crosshairs = GetCrosshairData();	// An alias for the global crosshair data
	
	// No need for the window context -- it's assumed
	Rect Bounds = {0,0,0,0};
	
	GetControlBounds(Ctrl, &Bounds);
	
	// Get ready to draw!
	PenNormal();
	
	// Draw the background
	RGBForeColor(&BkgdColor);
	PaintRect(&Bounds);
	
	// Clip to inside of box
	ClipRect(&Bounds);
	
	// Draw the crosshairs
	Crosshairs_Render(Bounds);
	
	// Draw the boundary line
	ForeColor(blackColor);
	FrameRect(&Bounds);
}


struct Configure_Crosshair_HandlerData
{
	// Will need to read off of these controls
	ControlRef ThicknessCtrl;
	ControlRef FromCenterCtrl;
	ControlRef LengthCtrl;
	ControlRef OpacityCtrl;
	ControlRef ShapeCtrl;
	
	ControlRef ShowCtrl;
	// Background color is a global
};


static void Configure_Crosshair_Handler(ParsedControl &Ctrl, void *Data)
{
	Configure_Crosshair_HandlerData *HDPtr = (Configure_Crosshair_HandlerData *)Data;
	
	CrosshairData &Crosshairs = GetCrosshairData();	// An alias for the global crosshair data
	
	bool IsOK;
	
	switch(Ctrl.ID.id)
	{
	case Shape_Item:
	case Preview_Item:
	
		// Update the crosshair data
		IsOK = true;
		
		IsOK = IsOK && GetShort(HDPtr->ThicknessCtrl, Crosshairs.Thickness);
		
		IsOK = IsOK && GetShort(HDPtr->FromCenterCtrl, Crosshairs.FromCenter);
		
		IsOK = IsOK && GetShort(HDPtr->LengthCtrl, Crosshairs.Length);
		
		IsOK = IsOK && GetFloat(HDPtr->OpacityCtrl, Crosshairs.Opacity);
		
		if (!IsOK) SysBeep(30);
		
		Crosshairs.Shape = GetControl32BitValue(HDPtr->ShapeCtrl) - 1;
		
		Draw1Control(HDPtr->ShowCtrl);
		
		break;
		
	case GetColor_Item:
		PickControlColor(HDPtr->ShowCtrl, &Crosshairs.Color, "\pSelect Crosshair Color");
		break;
		
	case BkgdColor_Item:
		PickControlColor(HDPtr->ShowCtrl, &BkgdColor, "\pSelect Background Color");
		break;
	}
}


// True for OK, false for cancel
bool Configure_Crosshairs(CrosshairData &Data)
{

	// Get the window
	AutoNibReference Nib (Window_Prefs_Crosshairs);
	AutoNibWindow Window (Nib.nibReference (), Window_Prefs_Crosshairs);

	// Push old crosshair state;
	// will always want to draw it
	bool OldCrosshairActive = Crosshairs_IsActive();
	Crosshairs_SetActive(true);
	CrosshairData &Crosshairs = GetCrosshairData();	// An alias for the global crosshair data
	CrosshairData OldCrosshairs = Crosshairs;
	
	// Get the controls
	Configure_Crosshair_HandlerData HandlerData;
	
	HandlerData.ThicknessCtrl = GetCtrlFromWindow(Window(), 0, Thickness_Item);
	SetShort(HandlerData.ThicknessCtrl, Crosshairs.Thickness);	
	
	HandlerData.FromCenterCtrl = GetCtrlFromWindow(Window(), 0, FromCenter_Item);
	SetShort(HandlerData.FromCenterCtrl, Crosshairs.FromCenter);
	
	HandlerData.LengthCtrl = GetCtrlFromWindow(Window(), 0, Length_Item);
	SetShort(HandlerData.LengthCtrl, Crosshairs.Length);
	
	HandlerData.OpacityCtrl = GetCtrlFromWindow(Window(), 0, CH_Opacity_Item);
	SetFloat(HandlerData.OpacityCtrl, Crosshairs.Opacity);
	
	HandlerData.ShapeCtrl = GetCtrlFromWindow(Window(), 0, Shape_Item);
	SetControl32BitValue(HandlerData.ShapeCtrl, Crosshairs.Shape+1);

	// For making the preview drawable
	AutoDrawability Drawability;
	
	HandlerData.ShowCtrl = GetCtrlFromWindow(Window(), 0, Show_Item);
	
	Drawability(HandlerData.ShowCtrl, PreviewDrawer);
	
	bool IsOK = RunModalDialog(Window(), true, Configure_Crosshair_Handler, &HandlerData);
	
	// Save the new color, in case we want it
	RGBColor Color = Crosshairs.Color;
	
	// Pop old crosshair state
	Crosshairs_SetActive(OldCrosshairActive);
	Crosshairs = OldCrosshairs;
	
	// After that popping, just in case that data is the data being currently edited
	if (IsOK)
	{
		GetShort(HandlerData.ThicknessCtrl, Crosshairs.Thickness);
		
		GetShort(HandlerData.FromCenterCtrl, Crosshairs.FromCenter);
		
		GetShort(HandlerData.LengthCtrl, Crosshairs.Length);
		
		GetFloat(HandlerData.OpacityCtrl, Crosshairs.Opacity);
		
		Crosshairs.Shape = GetControl32BitValue(HandlerData.ShapeCtrl) - 1;
		
		Crosshairs.Color = Color;
	}
	
	// Reset OpenGL precalculation
	Crosshairs.PreCalced = false;
	
	return IsOK;	
}


#else


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
	Rect portRect;
	GetPortBounds(GetWindowPort(GetDialogWindow(Dialog)), &portRect);
	ClipRect(&portRect);
	
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

	ControlHandle Shape_CHdl;
	GetDialogItem(Dialog, Shape_Item, &ItemType, (Handle *)&Shape_CHdl, &Bounds);
	SetControlValue(Shape_CHdl,Data.Shape+1);

	ControlHandle Opacity_CHdl;
	GetDialogItem(Dialog, CH_Opacity_Item, &ItemType, (Handle *)&Opacity_CHdl, &Bounds);
	SetFloat(Opacity_CHdl,Data.Opacity);
	
	// Create a UPP for the crosshair preview and store it
	UserItemUPP DoPreviewUPP = NewUserItemUPP(DoPreview);
	ControlHandle Show_CHdl;
	GetDialogItem(Dialog, Show_Item, &ItemType, (Handle *)&Show_CHdl, &Bounds);	
	SetDialogItem(Dialog, Show_Item, ItemType, Handle(DoPreviewUPP), &Bounds);
	
	// Where to make the color picker
	Point Center = {-1,-1};
	
	// Remembering the old values is necessary for the preview to work properly
	CrosshairData SavedData = Data;
	
	// Reveal it
#if USE_SHEETS
	SetThemeWindowBackground(GetDialogWindow(Dialog), kThemeBrushSheetBackgroundTransparent, false);
	ShowSheetWindow(GetDialogWindow(Dialog), ActiveNonFloatingWindow());
#else
	SelectWindow(GetDialogWindow(Dialog));
	ShowWindow(GetDialogWindow(Dialog));
#endif
	
	bool WillQuit = false;
	bool IsOK = false;
	short ShortTemp;
	float FloatTemp;
	RGBColor NewColor;
	bool BadValue, OverallBadValue;
	while(!WillQuit)
	{
		short ItemHit;
		ModalDialog(NULL, &ItemHit);
		
		switch(ItemHit)
		{
		case OK_Item:
		case Preview_Item:
			// Check before quitting or redrawing;
			OverallBadValue = false;
			
			BadValue = false;
			if (GetShort(Thickness_CHdl,ShortTemp))
			{
				if (ShortTemp > 0)
					Data.Thickness = ShortTemp;
				else
					BadValue = true;
			} else
				BadValue = true;
			
			if (BadValue)
			{
				SetShort(Thickness_CHdl,Data.Thickness);
				OverallBadValue = true;
			}
			
			BadValue = false;
			if (GetShort(FromCenter_CHdl,ShortTemp))
			{
				if (ShortTemp >= 0)
					Data.FromCenter = ShortTemp;
				else
					BadValue = true;
			} else
				BadValue = true;
			
			if (BadValue)
			{
				SetShort(FromCenter_CHdl,Data.FromCenter);
				OverallBadValue = true;
			}
			
			BadValue = false;
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
				SetShort(Length_CHdl,Data.Length);
				OverallBadValue = true;
			}

			BadValue = false;
			if (GetFloat(Opacity_CHdl,FloatTemp))
			{
				if (FloatTemp >= 0 && FloatTemp <= 1)
					Data.Opacity = FloatTemp;
				else
					BadValue = true;
			} else
				BadValue = true;
			
			if (BadValue)
			{
				SetFloat(Opacity_CHdl,Data.Opacity);
				OverallBadValue = true;
			}
			
			Data.Shape = GetControlValue(Shape_CHdl)-1;
			
			if (OverallBadValue)
			{
				SysBeep(30);
				DrawDialog(Dialog);	// To do the reversion correctly
				break;
			}
			
			switch(ItemHit)
			{
			case Preview_Item:
				DrawDialog(Dialog);
				break;
				
			case OK_Item:
				IsOK = true;
				WillQuit = true;
				break;
			}
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
		Data = SavedData;
	}
	
	// Clean up
#if USE_SHEETS
	HideSheetWindow(GetDialogWindow(Dialog));
#else
	HideWindow(GetDialogWindow(Dialog));
#endif

	DisposeUserItemUPP(DoPreviewUPP);
	DisposeDialog(Dialog);
	
	return IsOK;
}

#endif
