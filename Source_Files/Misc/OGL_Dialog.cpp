/*
	March 18, 2000 (Loren Petrich)
	
	Dialog box for setting up OpenGL options
	
	Several changes...
	
	May 27, 2000 (Loren Petrich)
	
	Added support for flat static effect
	
	Jun 11, 2000 (Loren Petrich):
	Use a class created for the checkboxes.
	Also added "see through liquids" option

Sep 9, 2000:

	Added checkbox for AppleGL texturing fix

Dec 17, 2000 (Loren Petrich):
	Eliminated fog parameters from the preferences;
	there is still a "fog present" switch, which is used to indicate
	whether fog will not be suppressed.
*/

#include "cseries.h"
#include "OGL_Setup.h"
#include "MacCheckbox.h"
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
		
	SinglePass_Item = 25,
	TwoDimGraphics_Item = 26,
	FlatStaticEffect_Item = 27,
	Fader_Item = 28,
	LiquidSeeThru_Item = 29,
	Map_Item = 30,
	TextureFix_Item = 31,
	AllowFog_Item = 32,
	
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
	
	MacCheckbox ZBuffer_CB(Dialog, ZBuffer_Item, TEST_FLAG(Data.Flags,OGL_Flag_ZBuffer) != 0);
	MacCheckbox ColorVoid_CB(Dialog, ColorVoid_Item, TEST_FLAG(Data.Flags,OGL_Flag_VoidColor) != 0);
		
	ControlHandle ColorVoidSwatch_CHdl;
	GetDialogItem(Dialog, ColorVoidSwatch_Item, &ItemType, (Handle *)&ColorVoidSwatch_CHdl, &Bounds);
	UserItemUPP PaintSwatchUPP = NewUserItemProc(PaintSwatch);
	SetDialogItem(Dialog, ColorVoidSwatch_Item, ItemType, Handle(PaintSwatchUPP), &Bounds);
	
	MacCheckbox FlatColorLandscapes_CB(Dialog, FlatColorLandscapes_Item, TEST_FLAG(Data.Flags,OGL_Flag_FlatLand) != 0);
		
	for (int ile=0; ile<8; ile++)
	{
		short LandscapeSwatch_Item = LandscapeSwatch_ItemBase + ile;
		ControlHandle LandscapeSwatch_CHdl;
		GetDialogItem(Dialog, LandscapeSwatch_Item, &ItemType, (Handle *)&LandscapeSwatch_CHdl, &Bounds);
		SetDialogItem(Dialog, LandscapeSwatch_Item, ItemType, Handle(PaintSwatchUPP), &Bounds);
	}
	
	MacCheckbox Fog_CB(Dialog, AllowFog_Item, TEST_FLAG(Data.Flags,OGL_Flag_Fog) != 0);
		
	MacCheckbox SinglePass_CB(Dialog, SinglePass_Item, TEST_FLAG(Data.Flags,OGL_Flag_SnglPass) != 0);
	MacCheckbox TwoDimGraphics_CB(Dialog, TwoDimGraphics_Item, TEST_FLAG(Data.Flags,OGL_Flag_2DGraphics) != 0);
	MacCheckbox FlatStaticEffect_CB(Dialog, FlatStaticEffect_Item, TEST_FLAG(Data.Flags,OGL_Flag_FlatStatic) != 0);
	MacCheckbox FaderEffect_CB(Dialog, Fader_Item, TEST_FLAG(Data.Flags,OGL_Flag_Fader) != 0);
	MacCheckbox SeeThruLiquids_CB(Dialog, LiquidSeeThru_Item, TEST_FLAG(Data.Flags,OGL_Flag_LiqSeeThru) != 0);
	MacCheckbox Map_CB(Dialog, Map_Item, TEST_FLAG(Data.Flags,OGL_Flag_Map) != 0);
	MacCheckbox TextureFix_CB(Dialog, TextureFix_Item, TEST_FLAG(Data.Flags,OGL_Flag_TextureFix) != 0);
	
	// Load the colors into temporaries
	VoidColor = Data.VoidColor;
	for (int il=0; il<4; il++)
		for (int ie=0; ie<2; ie++)
			LscpColors[il][ie] = Data.LscpColors[il][ie];
	
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
				
		case ColorVoidSwatch_Item:
			getpstr(ptemporary,ColorPicker_PromptStrings,ColorVoid_String);
			if (GetColor(Loc,ptemporary,&VoidColor,&TempColor))
			{
				VoidColor = TempColor;
				DrawDialog(Dialog);
			}
			break;
		
		default:
		
			if (ZBuffer_CB.ToggleIfHit(ItemHit)) break;
			if (ColorVoid_CB.ToggleIfHit(ItemHit)) break;
			if (FlatColorLandscapes_CB.ToggleIfHit(ItemHit)) break;
			if (Fog_CB.ToggleIfHit(ItemHit)) break;
			if (SinglePass_CB.ToggleIfHit(ItemHit)) break;
			if (TwoDimGraphics_CB.ToggleIfHit(ItemHit)) break;
			if (FlatStaticEffect_CB.ToggleIfHit(ItemHit)) break;
			if (FaderEffect_CB.ToggleIfHit(ItemHit)) break;
			if (SeeThruLiquids_CB.ToggleIfHit(ItemHit)) break;
			if (Map_CB.ToggleIfHit(ItemHit)) break;
			if (TextureFix_CB.ToggleIfHit(ItemHit)) break;
			
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
		
		SET_FLAG(Data.Flags,OGL_Flag_ZBuffer,ZBuffer_CB.GetState());
		SET_FLAG(Data.Flags,OGL_Flag_VoidColor,ColorVoid_CB.GetState());
		SET_FLAG(Data.Flags,OGL_Flag_FlatLand,FlatColorLandscapes_CB.GetState());
		SET_FLAG(Data.Flags,OGL_Flag_Fog,Fog_CB.GetState());
		SET_FLAG(Data.Flags,OGL_Flag_SnglPass,SinglePass_CB.GetState());
		SET_FLAG(Data.Flags,OGL_Flag_2DGraphics,TwoDimGraphics_CB.GetState());
		SET_FLAG(Data.Flags,OGL_Flag_FlatStatic,FlatStaticEffect_CB.GetState());
		SET_FLAG(Data.Flags,OGL_Flag_Fader,FaderEffect_CB.GetState());
		SET_FLAG(Data.Flags,OGL_Flag_LiqSeeThru,SeeThruLiquids_CB.GetState());
		SET_FLAG(Data.Flags,OGL_Flag_Map,Map_CB.GetState());
		SET_FLAG(Data.Flags,OGL_Flag_TextureFix,TextureFix_CB.GetState());
		Data.VoidColor = VoidColor;
		for (int il=0; il<4; il++)
			for (int ie=0; ie<2; ie++)
				Data.LscpColors[il][ie] = LscpColors[il][ie];
	}
	
	// Clean up
	HideWindow(Dialog);
	DisposeRoutineDescriptor(UniversalProcPtr(PaintSwatchUPP));
	DisposeDialog(Dialog);
	
	return IsOK;
}