/*
	February 21, 2000 (Loren Petrich)
	
	These are some extra dialog routines associated with setting such player stuff
	as the chase cam and the crosshairs.

	Feb 25, 2000 (Loren Petrich):
	
	Split the rendering routines up into routines that need different parameters.
	
	Feb 26, 2000 (Loren Petrich):
	Added support for "on when entering" for the chase cam when a game starts.
*/

#include "cseries.h"
#include "interface.h"
#include "world.h"
#include "ChaseCam.h"
#include "Crosshairs.h"
#include "OGL_Setup.h"
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

inline bool GetShort(ControlHandle Hdl, short& Value)
{
	GetDialogItemText((Handle)Hdl,ptemporary);
	ptemporary[ptemporary[0]+1] = 0;
	return (sscanf(temporary+1,"%hd",&Value) == 1);
}

inline bool GetFloat(ControlHandle Hdl, float& Value)
{
	GetDialogItemText((Handle)Hdl,ptemporary);
	ptemporary[ptemporary[0]+1] = 0;
	return (sscanf(temporary+1,"%f",&Value) == 1);
}

inline void SetShort(ControlHandle Hdl, short Value)
{
	NumToString(Value,ptemporary);
	SetDialogItemText((Handle)Hdl, ptemporary);
}

inline void SetFloat(ControlHandle Hdl, float Value)
{
	psprintf(ptemporary, "%f",Value);
	SetDialogItemText((Handle)Hdl, ptemporary);
}

inline void ToggleControl(ControlHandle Hdl)
{
	SetControlValue(Hdl, 1 - GetControlValue(Hdl));
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
	
	ControlHandle PassThruWall_CHdl;
	GetDialogItem(Dialog, PassThruWall_Item, &ItemType, (Handle *)&PassThruWall_CHdl, &Bounds);
	SetControlValue(PassThruWall_CHdl, TEST_FLAG(Data.Flags,_ChaseCam_ThroughWalls) != 0);
	
	ControlHandle NeverActive_CHdl;
	GetDialogItem(Dialog, NeverActive_Item, &ItemType, (Handle *)&NeverActive_CHdl, &Bounds);
	SetControlValue(NeverActive_CHdl, TEST_FLAG(Data.Flags,_ChaseCam_NeverActive) != 0);
	
	ControlHandle OnWhenEntering_CHdl;
	GetDialogItem(Dialog, OnWhenEntering_Item, &ItemType, (Handle *)&OnWhenEntering_CHdl, &Bounds);
	SetControlValue(OnWhenEntering_CHdl, TEST_FLAG(Data.Flags,_ChaseCam_OnWhenEntering) != 0);
	
	// Where to make the color picker
	Point Center = {-1,-1};
	RGBColor NewColor;
	// Get void color from OpenGL-parameters data
	OGL_ConfigureData& OGLData = Get_OGL_ConfigureData();
	ControlHandle VoidColorOnOff_CHdl;
	GetDialogItem(Dialog, VoidColorOnOff_Item, &ItemType, (Handle *)&VoidColorOnOff_CHdl, &Bounds);
	SetControlValue(VoidColorOnOff_CHdl, TEST_FLAG(OGLData.Flags,OGL_Flag_VoidColor) != 0);
	
	// Reveal it
	SelectWindow(Dialog);
	ShowWindow(Dialog);
	
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
			
			if (GetFloat(Behind_CHdl,FloatTemp))
				New_Behind = 1024 * FloatTemp;
			else
				BadValue = true;
			
			if (GetFloat(Upward_CHdl,FloatTemp))
				New_Upward = 1024 * FloatTemp;
			else
				BadValue = true;
			
			if (GetFloat(Rightward_CHdl,FloatTemp))
				New_Rightward = 1024 * FloatTemp;
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
			
		case PassThruWall_Item:
			ToggleControl(PassThruWall_CHdl);
			break;
			
		case NeverActive_Item:
			ToggleControl(NeverActive_CHdl);
			break;
			
		case OnWhenEntering_Item:
			ToggleControl(OnWhenEntering_CHdl);
			break;
		
		case VoidColorOnOff_Item:
			ToggleControl(VoidColorOnOff_CHdl);
			break;
		
		case VoidColorSelect_Item:
			// Need to set color here so the preview can work properly
			if (GetColor(Center,"\pWhat color for the void?",&OGLData.VoidColor,&NewColor))
				OGLData.VoidColor = NewColor;
			break;
		}
	}
	
	if (IsOK)
	{
		Data.Behind = New_Behind;
		Data.Upward = New_Upward;
		Data.Rightward = New_Rightward;
		SET_FLAG(Data.Flags,_ChaseCam_ThroughWalls,GetControlValue(PassThruWall_CHdl));
		SET_FLAG(Data.Flags,_ChaseCam_NeverActive,GetControlValue(NeverActive_CHdl));
		SET_FLAG(Data.Flags,_ChaseCam_OnWhenEntering,GetControlValue(OnWhenEntering_CHdl));
		SET_FLAG(OGLData.Flags,OGL_Flag_VoidColor,GetControlValue(VoidColorOnOff_CHdl));
	}
	
	// Clean up
	HideWindow(Dialog);
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
	ClipRect(&Dialog->portRect);
	
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
	UserItemUPP DoPreviewUPP = NewUserItemProc(DoPreview);
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
	SelectWindow(Dialog);
	ShowWindow(Dialog);
	
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
	HideWindow(Dialog);
	DisposeRoutineDescriptor(UniversalProcPtr(DoPreviewUPP));
	DisposeDialog(Dialog);
	
	return IsOK;
}