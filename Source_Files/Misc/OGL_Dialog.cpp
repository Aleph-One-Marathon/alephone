/*
	March 18, 2000 (Loren Petrich)
	
	Dialog box for setting up OpenGL options
	
	Several changes...
	
	May 27, 2000 (Loren Petrich)
	
	Added support for flat static effect
*/

#include "cseries.h"
#include "OGL_Setup.h"
#include <stdio.h>

enum
{
	OK_Item = 1,
	Cancel_Item = 2,
	
	OpenGL_Dialog = 2000,
	Walls_Item = 5,
	Landscape_Item = 6,
	Inhabitants_Item = 7,
	WeaponsInHand_Item = 8,
	ZBuffer_Item = 9,
	ColorVoid_Item = 10,
	ColorVoidSwatch_Item = 11,
	FlatColorLandscapes_Item = 12,
	
	// The first of 8 items, in order:
	// Day Ground, Day Sky, Night Ground, Night Sky, ...
	LandscapeSwatch_ItemBase = 17,
	
	Fog_Item = 25,
	FogSwatch_Item = 26,
	FogDepth_Item = 28,
	
	SinglePass_Item = 29,
	TwoDimGraphics_Item = 30,
	FlatStaticEffect_Item = 31,
	Fader_Item = 32,
	
	ColorPicker_PromptStrings = 200,
	ColorVoid_String = 0,
	FlatColorLandscapes_StringBase = 1,
	FogColor_String = 9,
	
	OpenGL_Textures_Dialog = 2100,
	WhichOne_Item = 4,
	Near_Item = 5,
	Far_Item = 6,
	Resolution_Item = 7,
	ColorDepth_Item = 8,
	BasedOn_Item = 9,
	
	BasedOn_Menu = 12140
};


inline void ToggleControl(ControlHandle Hdl)
{
	SetControlValue(Hdl, 1 - GetControlValue(Hdl));
}

// Local copies of this stuff, needed in case we cancel
static OGL_Texture_Configure TxtrConfigList[OGL_NUMBER_OF_TEXTURE_TYPES];
static RGBColor VoidColor, FogColor, LscpColors[4][2];
static long FogDepth;

// Texture-configuration dialog box
static bool TextureConfigureDialog(short WhichTexture)
// True for OK, false for cancel
{
	short ItemType;
	Rect Bounds;
	
	DialogPtr Dialog = myGetNewDialog(OpenGL_Textures_Dialog, NULL, (WindowPtr)(-1), 0);
	assert(Dialog);
	
	// Get a local copy of the data
	OGL_Texture_Configure TxtrConfig = TxtrConfigList[WhichTexture];
	
	ControlHandle Near_CHdl;
	GetDialogItem(Dialog, Near_Item, &ItemType, (Handle *)&Near_CHdl, &Bounds);
	SetControlValue(Near_CHdl, short(TxtrConfig.NearFilter)+1);
	
	ControlHandle Far_CHdl;
	GetDialogItem(Dialog, Far_Item, &ItemType, (Handle *)&Far_CHdl, &Bounds);
	SetControlValue(Far_CHdl, short(TxtrConfig.FarFilter)+1);

	ControlHandle Res_CHdl;
	GetDialogItem(Dialog, Resolution_Item, &ItemType, (Handle *)&Res_CHdl, &Bounds);
	SetControlValue(Res_CHdl, short(TxtrConfig.Resolution)+1);
	
	ControlHandle Color_CHdl;
	GetDialogItem(Dialog, ColorDepth_Item, &ItemType, (Handle *)&Color_CHdl, &Bounds);
	SetControlValue(Color_CHdl, short(TxtrConfig.ColorFormat)+1);
	
	ControlHandle BasedOn_CHdl;
	GetDialogItem(Dialog, BasedOn_Item, &ItemType, (Handle *)&BasedOn_CHdl, &Bounds);
	SetControlValue(BasedOn_CHdl, WhichTexture+1);
	
	// Edit the title
	HLock(Handle(BasedOn_CHdl));
	ControlRecord *PopupPtr = *BasedOn_CHdl;
	
	Handle CtrlDataHdl = PopupPtr->contrlData;
	HLock(CtrlDataHdl);
	PopupPrivateData *PPD = (PopupPrivateData *)(*CtrlDataHdl);
	
	MenuHandle BasedOn_MHdl = PPD->mHandle;
	Str255 WhichTxtrLabel;
	GetMenuItemText(BasedOn_MHdl,WhichTexture+1,WhichTxtrLabel);
	
	Handle WhichOne_Hdl;
	GetDialogItem(Dialog, WhichOne_Item, &ItemType, &WhichOne_Hdl, &Bounds);
	SetDialogItemText(WhichOne_Hdl,WhichTxtrLabel);
	
	HUnlock(CtrlDataHdl);
	HUnlock(Handle(BasedOn_CHdl));
	
	short WhichAltTxtr;	// Which alternative one selected
	
	// Reveal it
	SelectWindow(Dialog);
	ShowWindow(Dialog);
	
	bool WillQuit = false;
	bool IsOK = false;

	while(!WillQuit)
	{
		short ItemHit;
		ModalDialog(NULL, &ItemHit);
		
		switch(ItemHit)
		{
		case OK_Item:
				
			IsOK = true;
			WillQuit = true;
			break;
			
		case Cancel_Item:
			
			IsOK = false;
			WillQuit = true;
			break;
		
		case BasedOn_Item:
			// Copy in from another one (like Forge) if different
			WhichAltTxtr = GetControlValue(BasedOn_CHdl) - 1;
			if (WhichAltTxtr != WhichTexture)
			{
				TxtrConfig = TxtrConfigList[WhichAltTxtr];
				SetControlValue(Near_CHdl, short(TxtrConfig.NearFilter)+1);
				SetControlValue(Far_CHdl, short(TxtrConfig.FarFilter)+1);
				SetControlValue(Res_CHdl, short(TxtrConfig.Resolution)+1);
				SetControlValue(Color_CHdl, short(TxtrConfig.ColorFormat)+1);
			}
			break;
		}
	}
	
	// Change the data structure
	if (IsOK)
	{
		TxtrConfig.NearFilter = GetControlValue(Near_CHdl) - 1;
		TxtrConfig.FarFilter = GetControlValue(Far_CHdl) - 1;
		TxtrConfig.Resolution = GetControlValue(Res_CHdl) - 1;
		TxtrConfig.ColorFormat = GetControlValue(Color_CHdl) - 1;
		TxtrConfigList[WhichTexture] = TxtrConfig;
	}
	
	// Clean up
	HideWindow(Dialog);
	DisposeDialog(Dialog);
	
	return IsOK;
}


// Procedure to draw a color swatch
static pascal void PaintSwatch(DialogPtr DPtr, short ItemNo) {
	short ItemType;
	ControlHandle Hdl;
	Rect Bounds;
	GetDialogItem(DPtr, ItemNo, &ItemType, (Handle *)&Hdl, &Bounds);

	RGBColor* ColorPtr = NULL;
	
	int ile;
	switch(ItemNo)
	{
	case ColorVoidSwatch_Item:
		ColorPtr = &VoidColor;
		break;
	
	case FogSwatch_Item:
		ColorPtr = &FogColor;
		break;
		
	default:
		ile = LandscapeSwatch_ItemBase;
		for (int il=0; il<4; il++)
		{
			for (int ie=0; ie<2; ie++)
			{
				if (ItemNo == ile)
				{
					ColorPtr = &LscpColors[il][ie];
					break;
				}
				if (ColorPtr != NULL) break;
				ile++;
			}
			if (ColorPtr != NULL) break;
		}
	}
	// Must find at least one color
	assert(ColorPtr);
		
	PenState OldPen;
	RGBColor OldBackColor, OldForeColor;

	// Save previous state		
	GetPenState(&OldPen);
	GetBackColor(&OldBackColor);
	GetForeColor(&OldForeColor);
	
	// Get ready to draw the swatch
	PenNormal();
	
	// Draw!
	RGBForeColor(ColorPtr);
	PaintRect(&Bounds);
	ForeColor(blackColor);
	FrameRect(&Bounds);
	
	// Restore previous state
	SetPenState(&OldPen);
	RGBBackColor(&OldBackColor);
	RGBForeColor(&OldForeColor);
}


// Gets the fog-depth value; beeps and returns "false" if invalid
static bool GetFogDepth(ControlHandle FDHdl)
{
	GetDialogItemText((Handle)FDHdl,ptemporary);
	// Pascal to C string in place
	ptemporary[MIN(int(ptemporary[0])+1,255)] = 0;
	double Temp;
	if (sscanf(temporary+1,"%lf",&Temp) == 1)
	{
		// Insure that fog has positive depth
		long NewFogDepth = 1024*Temp + 0.5;
		if (NewFogDepth > 0)
		{
			FogDepth = NewFogDepth;
			return true;
		}
	}

	psprintf(ptemporary,"%lf",FogDepth/double(1024));
	SetDialogItemText((Handle)FDHdl,ptemporary);
	SysBeep(1);
	return false;
}


// True for OK, false for cancel
bool OGL_ConfigureDialog(OGL_ConfigureData& Data)
{
	short ItemType;
	Rect Bounds;
	RGBColor TempColor;
	Point Loc = {-1,-1};	// Color-picker dialog-box location: in the center of the screen
	
	DialogPtr Dialog = myGetNewDialog(OpenGL_Dialog, NULL, (WindowPtr)(-1), 0);
	assert(Dialog);
	
	// Get the dialog-item features and make lots of local copies
	for (int k=0; k<OGL_NUMBER_OF_TEXTURE_TYPES; k++)
	{
		TxtrConfigList[k] = Data.TxtrConfigList[k];
	}
	
	ControlHandle ZBuffer_CHdl;
	GetDialogItem(Dialog, ZBuffer_Item, &ItemType, (Handle *)&ZBuffer_CHdl, &Bounds);
	SetControlValue(ZBuffer_CHdl,TEST_FLAG(Data.Flags,OGL_Flag_ZBuffer) != 0);
	
	ControlHandle ColorVoid_CHdl;
	GetDialogItem(Dialog, ColorVoid_Item, &ItemType, (Handle *)&ColorVoid_CHdl, &Bounds);
	SetControlValue(ColorVoid_CHdl,TEST_FLAG(Data.Flags,OGL_Flag_VoidColor) != 0);
	
	ControlHandle ColorVoidSwatch_CHdl;
	GetDialogItem(Dialog, ColorVoidSwatch_Item, &ItemType, (Handle *)&ColorVoidSwatch_CHdl, &Bounds);
	UserItemUPP PaintSwatchUPP = NewUserItemProc(PaintSwatch);
	SetDialogItem(Dialog, ColorVoidSwatch_Item, ItemType, Handle(PaintSwatchUPP), &Bounds);
	
	ControlHandle FlatColorLandscapes_CHdl;
	GetDialogItem(Dialog, FlatColorLandscapes_Item, &ItemType, (Handle *)&FlatColorLandscapes_CHdl, &Bounds);
	SetControlValue(FlatColorLandscapes_CHdl,TEST_FLAG(Data.Flags,OGL_Flag_FlatLand) != 0);
	
	for (int ile=0; ile<8; ile++)
	{
		short LandscapeSwatch_Item = LandscapeSwatch_ItemBase + ile;
		ControlHandle LandscapeSwatch_CHdl;
		GetDialogItem(Dialog, LandscapeSwatch_Item, &ItemType, (Handle *)&LandscapeSwatch_CHdl, &Bounds);
		SetDialogItem(Dialog, LandscapeSwatch_Item, ItemType, Handle(PaintSwatchUPP), &Bounds);
	}
	
	ControlHandle Fog_CHdl;
	GetDialogItem(Dialog, Fog_Item, &ItemType, (Handle *)&Fog_CHdl, &Bounds);
	SetControlValue(Fog_CHdl,TEST_FLAG(Data.Flags,OGL_Flag_Fog) != 0);

	ControlHandle FogSwatch_CHdl;
	GetDialogItem(Dialog, FogSwatch_Item, &ItemType, (Handle *)&FogSwatch_CHdl, &Bounds);
	SetDialogItem(Dialog, FogSwatch_Item, ItemType, Handle(PaintSwatchUPP), &Bounds);

	ControlHandle FogDepth_CHdl;
	GetDialogItem(Dialog, FogDepth_Item, &ItemType, (Handle *)&FogDepth_CHdl, &Bounds);
	FogDepth = Data.FogDepth;
	psprintf(ptemporary,"%lf",FogDepth/double(1024));
	SetDialogItemText((Handle)FogDepth_CHdl,ptemporary);
	
	ControlHandle SinglePass_CHdl;
	GetDialogItem(Dialog, SinglePass_Item, &ItemType, (Handle *)&SinglePass_CHdl, &Bounds);
	SetControlValue(SinglePass_CHdl,TEST_FLAG(Data.Flags,OGL_Flag_SnglPass) != 0);
	
	ControlHandle TwoDimGraphics_CHdl;
	GetDialogItem(Dialog, TwoDimGraphics_Item, &ItemType, (Handle *)&TwoDimGraphics_CHdl, &Bounds);
	SetControlValue(TwoDimGraphics_CHdl,TEST_FLAG(Data.Flags,OGL_Flag_2DGraphics) != 0);
	
	ControlHandle FlatStaticEffect_CHdl;
	GetDialogItem(Dialog, FlatStaticEffect_Item, &ItemType, (Handle *)&FlatStaticEffect_CHdl, &Bounds);
	SetControlValue(FlatStaticEffect_CHdl,TEST_FLAG(Data.Flags,OGL_Flag_FlatStatic) != 0);
	
	ControlHandle FaderEffect_CHdl;
	GetDialogItem(Dialog, Fader_Item, &ItemType, (Handle *)&FaderEffect_CHdl, &Bounds);
	SetControlValue(FaderEffect_CHdl,TEST_FLAG(Data.Flags,OGL_Flag_Fader) != 0);
	
	// Load the colors into temporaries
	VoidColor = Data.VoidColor;
	for (int il=0; il<4; il++)
		for (int ie=0; ie<2; ie++)
			LscpColors[il][ie] = Data.LscpColors[il][ie];
	FogColor = Data.FogColor;
	
	// Reveal it
	SelectWindow(Dialog);
	ShowWindow(Dialog);
	
	bool WillQuit = false;
	bool IsOK = false;
	
	int ile;
	bool Escape;
	while(!WillQuit)
	{
		short ItemHit;
		ModalDialog(NULL, &ItemHit);
		
		switch(ItemHit)
		{
		case OK_Item:
			if (!GetFogDepth(FogDepth_CHdl)) break;				
			IsOK = true;
			WillQuit = true;
			break;
			
		case Cancel_Item:
			
			IsOK = false;
			WillQuit = true;
			break;
		
		case Walls_Item:
			TextureConfigureDialog(OGL_Txtr_Wall);
			break;
		
		case Landscape_Item:
			TextureConfigureDialog(OGL_Txtr_Landscape);
			break;
		
		case Inhabitants_Item:
			TextureConfigureDialog(OGL_Txtr_Inhabitant);
			break;
		
		case WeaponsInHand_Item:
			TextureConfigureDialog(OGL_Txtr_WeaponsInHand);
			break;
		
		case ZBuffer_Item:
			ToggleControl(ZBuffer_CHdl);
			break;
		
		case ColorVoid_Item:
			ToggleControl(ColorVoid_CHdl);
			break;
		
		case ColorVoidSwatch_Item:
			getpstr(ptemporary,ColorPicker_PromptStrings,ColorVoid_String);
			if (GetColor(Loc,ptemporary,&VoidColor,&TempColor))
			{
				VoidColor = TempColor;
				DrawDialog(Dialog);
			}
			break;
		
		case FlatColorLandscapes_Item:
			ToggleControl(FlatColorLandscapes_CHdl);
			break;
			
		case Fog_Item:
			ToggleControl(Fog_CHdl);
			break;
		
		case FogSwatch_Item:
			getpstr(ptemporary,ColorPicker_PromptStrings,FogColor_String);
			if (GetColor(Loc,ptemporary,&FogColor,&TempColor))
			{
				FogColor = TempColor;
				DrawDialog(Dialog);
			}
			break;
		
		case FogDepth_Item:
			GetFogDepth(FogDepth_CHdl);
			break;
		
		case SinglePass_Item:
			ToggleControl(SinglePass_CHdl);
			break;
		
		case TwoDimGraphics_Item:
			ToggleControl(TwoDimGraphics_CHdl);
			break;
		
		case FlatStaticEffect_Item:
			ToggleControl(FlatStaticEffect_CHdl);
			break;
		
		case Fader_Item:
			ToggleControl(FaderEffect_CHdl);
			break;
		
		default:
			ile = 0;
			Escape = false;
			for (int il=0; il<4; il++)
			{
				for (int ie=0; ie<2; ie++)
				{
					if (ItemHit == ile+LandscapeSwatch_ItemBase)
					{
					getpstr(ptemporary,ColorPicker_PromptStrings,FlatColorLandscapes_StringBase+ile);
					if (GetColor(Loc,ptemporary,&LscpColors[il][ie],&TempColor))
						{
							LscpColors[il][ie] = TempColor;
							DrawDialog(Dialog);
						}
						Escape = true;
						break;
					}
					ile++;
				}
				if (Escape) break;
			}
		}
	}
	
	// Change the data structure
	if (IsOK)
	{
		for (int k=0; k<OGL_NUMBER_OF_TEXTURE_TYPES; k++)
		{
			 Data.TxtrConfigList[k] = TxtrConfigList[k];
		}
		
		SET_FLAG(Data.Flags,OGL_Flag_ZBuffer,GetControlValue(ZBuffer_CHdl));
		SET_FLAG(Data.Flags,OGL_Flag_VoidColor,GetControlValue(ColorVoid_CHdl));
		SET_FLAG(Data.Flags,OGL_Flag_FlatLand,GetControlValue(FlatColorLandscapes_CHdl));
		SET_FLAG(Data.Flags,OGL_Flag_Fog,GetControlValue(Fog_CHdl));
		SET_FLAG(Data.Flags,OGL_Flag_SnglPass,GetControlValue(SinglePass_CHdl));
		SET_FLAG(Data.Flags,OGL_Flag_2DGraphics,GetControlValue(TwoDimGraphics_CHdl));
		SET_FLAG(Data.Flags,OGL_Flag_FlatStatic,GetControlValue(FlatStaticEffect_CHdl));
		SET_FLAG(Data.Flags,OGL_Flag_Fader,GetControlValue(FaderEffect_CHdl));
		Data.VoidColor = VoidColor;
		for (int il=0; il<4; il++)
			for (int ie=0; ie<2; ie++)
				Data.LscpColors[il][ie] = LscpColors[il][ie];
		Data.FogColor = FogColor;
		Data.FogDepth = FogDepth;
	}
	
	// Clean up
	HideWindow(Dialog);
	DisposeRoutineDescriptor(UniversalProcPtr(PaintSwatchUPP));
	DisposeDialog(Dialog);
	
	return IsOK;
}