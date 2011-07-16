/*

	Copyright (C) 1991-2001 and beyond by Bo Lindbergh
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

	Changes:

Jan 30, 2000 (Loren Petrich)
	Did some typecasts

Jan 25, 2002 (Br'fin (Jeremy Parsons)):
	Added TARGET_API_MAC_CARBON for Carbon.h
	Added accessors for datafields now opaque in Carbon
	Carbon proc routines all allocated as UPP's
	GetSlotFromGDevice does nothing under Carbon (Slot field now opaque)

Feb 5, 2002 (Br'fin (Jeremy Parsons)):
	Put screen choosing dialog in a sheet under Carbon

Feb 14, 2002 (Br'fin (Jeremy Parsons)):
	Made the Carbon sheet background transparent
	Changed the background for the screen selector to be an opaque backgrounded image well in Carbon
*/

#include <stdlib.h>
#include <vector>

#include "cseries.h"
#include "shell.h"

#ifdef USES_NIBS
	#include "NibsUiHelpers.h"
#endif

GDHandle BestDevice(
	GDSpecPtr spec)
{
	GDHandle dev;
	GDHandle first;
	Rect bounds;

	first=NULL;
	for (dev=GetDeviceList(); dev; dev=GetNextDevice(dev)) {
		if (!TestDeviceAttribute(dev,screenDevice)
				|| !TestDeviceAttribute(dev,screenActive))
			continue;
		bounds=(*dev)->gdRect;
		if (bounds.right-bounds.left<spec->width
				|| bounds.bottom-bounds.top<spec->height
				|| !HasDepth(dev,spec->bit_depth,1<<gdDevType,spec->flags))
			continue;
		if (GetSlotFromGDevice(dev)==spec->slot) {
			first=dev;
			break;
		}
		if (!first)
			first=dev;
	}
	if (first)
		spec->slot=GetSlotFromGDevice(first);
	return first;
}

GDHandle MatchGDSpec(
	GDSpecPtr spec)
{
	GDHandle dev;

	for (dev=GetDeviceList(); dev; dev=GetNextDevice(dev)) {
		if (!TestDeviceAttribute(dev,screenDevice)
				|| !TestDeviceAttribute(dev,screenActive))
			continue;
		if (GetSlotFromGDevice(dev)==spec->slot)
			return dev;
	}
	return NULL;
}

void SetDepthGDSpec(
	GDSpecPtr spec)
{
	GDHandle dev;

	for (dev=GetDeviceList(); dev; dev=GetNextDevice(dev)) {
		if (!TestDeviceAttribute(dev,screenDevice)
				|| !TestDeviceAttribute(dev,screenActive))
			continue;
		if (GetSlotFromGDevice(dev)==spec->slot)
			break;
	}
	if (!dev)
		return;
	if ((*(*dev)->gdPMap)->pixelSize==spec->bit_depth
			&& !!TestDeviceAttribute(dev,gdDevType)==!!(spec->flags&1<<gdDevType))
		return;
	SetDepth(dev,spec->bit_depth,1<<gdDevType,spec->flags);
}

void BuildGDSpec(
	GDSpecPtr spec,
	GDHandle dev)
{
	Rect bounds;

	spec->slot=GetSlotFromGDevice(dev);
	spec->flags=TestDeviceAttribute(dev,gdDevType) ? 1<<gdDevType : 0;
	spec->bit_depth=(*(*dev)->gdPMap)->pixelSize;
	bounds=(*dev)->gdRect;
	spec->width=bounds.right-bounds.left;
	spec->height=bounds.bottom-bounds.top;
}

bool HasDepthGDSpec(
	GDSpecPtr spec)
{
	GDHandle dev;

	for (dev=GetDeviceList(); dev; dev=GetNextDevice(dev)) {
		if (!TestDeviceAttribute(dev,screenDevice)
				|| !TestDeviceAttribute(dev,screenActive))
			continue;
		if (GetSlotFromGDevice(dev)==spec->slot)
			break;
	}
	if (!dev)
		return false;
/* the comments in screen.c indicate that the colours/grays bit should not be tested */
	return !!HasDepth(dev,spec->bit_depth,0,0);
}

bool EqualGDSpec(
	GDSpecPtr spec1,
	GDSpecPtr spec2)
{
	return spec1->slot==spec2->slot;
}

short GetSlotFromGDevice(
	GDHandle cur_dev)
{
#if defined(TARGET_API_MAC_CARBON)
	short count= 0;
	GDHandle dev;
	for (dev=GetDeviceList(); dev; dev=GetNextDevice(dev), ++count) {
		if (!TestDeviceAttribute(dev,screenDevice)
				|| !TestDeviceAttribute(dev,screenActive))
			continue;
		if(dev == cur_dev)
		{
			break;
		}
	}
	return count;
#else
	return (*(AuxDCEHandle)GetDCtlEntry((*dev)->gdRefNum))->dCtlSlot;
#endif
}

enum {
	dlgScreenChooser=20000,

	itemOKButton=1,
	itemCancelButton,
	itemColorsRadio,
	itemGraysRadio,
	itemDesktopUser,
	itemPromptText,
	itemGroupUser,	// In the nib version, contains the gray and color radio buttons
	itemGroupText,
	itemDescriptionText
};

#ifdef USES_NIBS

const CFStringRef Window_Prefs_Monitor = CFSTR("Prefs_Monitor");


// For the grays/colors radio button
enum
{
	rbColor = 1,
	rbGray
};


// For the data to be attached to the monitor controls as properties
const OSType AppTag = 'appl';
const OSType DevInfoTag = 'dvif';

const OSType Monitor_Sig = 'mnsg';

struct DevInfo {
	ControlRef Ctrl;
	GDHandle Dev;
	Rect Bounds;
	bool IsSelected;
	bool HasMenu;
};


struct DesktopDisplayData
{
	vector<DevInfo> Devices;
};


static void MonitorDrawer(ControlRef Ctrl, void *Data)
{
	DevInfo *DI_Ptr = (DevInfo *)Data;
	
	// No need for the window context -- it's assumed
	Rect Bounds = {0,0,0,0};
	
	GetControlBounds(Ctrl, &Bounds);
	
	// Get ready to draw the swatch
	PenNormal();
	
	// Draw!
	const RGBColor SelColor = {0xc000,0xffff,0xc000};
	const RGBColor UnselColor = {0x4000,0x4000,0xffff};
	const RGBColor *ColorPtr = (DI_Ptr->IsSelected) ? &SelColor : &UnselColor;
	
	RGBForeColor(ColorPtr);
	PaintRect(&Bounds);
	ForeColor(blackColor);
	FrameRect(&Bounds);
	
	if (DI_Ptr->HasMenu)
	{
		const short MenuThickness = 4;
		Bounds.bottom = Bounds.top + MenuThickness;
		ForeColor(whiteColor);
		PaintRect(&Bounds);
		ForeColor(blackColor);
		FrameRect(&Bounds);
	}
}


static void MonitorHandler(ParsedControl &Ctrl, void *Data)
{
	DesktopDisplayData *DDPtr = (DesktopDisplayData *)(Data);
	
	if (Ctrl.ID.signature == Monitor_Sig)
	{
		for (int k=0; k<DDPtr->Devices.size(); k++)
		{
			DDPtr->Devices[k].IsSelected = (k == Ctrl.ID.id);
			Draw1Control(DDPtr->Devices[k].Ctrl);
		}
	}
}


struct PositioningInfo
{
	float Center_H;
	float Center_V;
	float Size_H;
	float Size_V;

	PositioningInfo(Rect &R);
};

PositioningInfo::PositioningInfo(Rect &R)
{
	Center_H = 0.5*(R.left + R.right);
	Center_V = 0.5*(R.top  + R.bottom);
	
	Size_H = R.right - R.left;
	Size_V = R.bottom - R.top;
}

void display_device_dialog(
	GDSpecPtr spec)
{
	OSStatus err;
	
	// Get the window
	AutoNibReference Nib (Window_Prefs_Monitor);
	AutoNibWindow Window (Nib.nibReference (), Window_Prefs_Monitor);
	
	// Get its controls
	
	ControlRef GrayColorCtrl = GetCtrlFromWindow(Window(),0,itemGroupUser);
	SetControl32BitValue(GrayColorCtrl,rbColor);
	
	DesktopDisplayData DesktopData;
	ControlRef DesktopCtrl = GetCtrlFromWindow(Window(),0,itemDesktopUser);
	
	Rect DesktopBounds;
	GetControlBounds(DesktopCtrl, &DesktopBounds);
	
	// Create inset version for putting the monitors in
	const short InsetAmount = 8;
	InsetRect(&DesktopBounds,InsetAmount,InsetAmount);
	
	// For making controls drawable and hittable
	AutoDrawability Drawability;
	AutoHittability Hittability;
	
	// Light gray
	RGBColor DesktopColor = {0xe800, 0xe800, 0xe800};
	
	Drawability(DesktopCtrl, SwatchDrawer, &DesktopColor);
	
	// Compose the monitor representations
	
	GDHandle MainDev = GetMainDevice();
	
	// The total screen area
	Rect TotalBounds;
	TotalBounds.left = TotalBounds.top = 0x7fff;
	TotalBounds.right = TotalBounds.bottom = 0x8000;
	
	// Pick up the device info
	for (GDHandle Dev = GetDeviceList(); Dev; Dev = GetNextDevice(Dev))
	{
		// Skip over non-monitors and inactive monitors
		if (!TestDeviceAttribute(Dev,screenDevice) ||
			!TestDeviceAttribute(Dev,screenActive)) continue;
		
		DevInfo Monitor;
		
		Monitor.Dev = Dev;
		
		Monitor.Bounds = (*Dev)->gdRect;
		UnionRect(&TotalBounds, &Monitor.Bounds, &TotalBounds);
		
		Monitor.IsSelected = (GetSlotFromGDevice(Dev)==spec->slot);
		
		Monitor.HasMenu = (Dev == MainDev);
		
		// Add to list
		DesktopData.Devices.push_back(Monitor);
	}
	
	// Set up scaling factors
	PositioningInfo Total_PInfo(TotalBounds);
	PositioningInfo Desktop_PInfo(DesktopBounds);
	
	float Scale_H = Desktop_PInfo.Size_H/Total_PInfo.Size_H;
	float Scale_V = Desktop_PInfo.Size_V/Total_PInfo.Size_V;
	float Scale = MIN(Scale_H,Scale_V);
	
	// Create monitor controls
	
	int k = 0;
	for (vector<DevInfo>::iterator DI_Iter = DesktopData.Devices.begin();
		DI_Iter < DesktopData.Devices.end();
		DI_Iter++, k++)
	{
		// Find appropriately-scaled bounds for representing the monitor
		Rect Bounds;
		// The 0.5 factor is for rounding off of nonnegative values
		Bounds.left =
			Scale*(DI_Iter->Bounds.left - Total_PInfo.Center_H) + Desktop_PInfo.Center_H + 0.5;
		Bounds.right =
			Scale*(DI_Iter->Bounds.right - Total_PInfo.Center_H) + Desktop_PInfo.Center_H + 0.5;
		Bounds.top =
			Scale*(DI_Iter->Bounds.top - Total_PInfo.Center_V) + Desktop_PInfo.Center_V + 0.5;
		Bounds.bottom =
			Scale*(DI_Iter->Bounds.bottom - Total_PInfo.Center_V) + Desktop_PInfo.Center_V + 0.5;
		
		// Create!
		err = CreateUserPaneControl(
				Window(), &Bounds,
				0, &(DI_Iter->Ctrl)
				);
		vassert(err == noErr, csprintf(temporary,"CreateUserPaneControl error: %d",(int)err));
		
		// Add ID so that the dialog's hit tester can recognize it
		ControlID ID;
		ID.signature = Monitor_Sig;
		ID.id = k;
		
		err = SetControlID(DI_Iter->Ctrl, &ID);
		vassert(err == noErr, csprintf(temporary,"SetControlID error: %d",(int)err));
		
		// OK since the list of device-info data is now set up
		// and will not get reallocated
		Drawability(DI_Iter->Ctrl, MonitorDrawer, &DesktopData.Devices[k]);
		Hittability(DI_Iter->Ctrl);
		
		EmbedControl(DI_Iter->Ctrl, DesktopCtrl);
	}
	
	if (RunModalDialog(Window(), true, MonitorHandler, &DesktopData))
	{
		// Pressed "OK" -- get the selected device
		
		for (vector<DevInfo>::iterator DI_Iter = DesktopData.Devices.begin(); DI_Iter < DesktopData.Devices.end(); DI_Iter++)
		{
			if (DI_Iter->IsSelected)
				spec->slot = GetSlotFromGDevice(DI_Iter->Dev);
		}
		spec->flags = (GetControl32BitValue(GrayColorCtrl) == rbColor) ? 1<<gdDevType : 0;
	}
}


#else

typedef struct dev_info {
	Rect rect;
	GDHandle dev;
} dev_info;

static dev_info *devices;
static short devcnt;
static short mainix;
static short curix;

static pascal void draw_desktop(
	DialogPtr dlg,
	short item)
{
	int i;
	short it;
	Handle ih;
	Rect ir;
	static RGBColor fifty={0x7FFF,0x7FFF,0x7FFF};

	GetDialogItem(dlg,item,&it,&ih,&ir);
#if defined(TARGET_API_MAC_CARBON)
	CGrafPtr port = GetWindowPort(GetDialogWindow(dlg));
	SInt16   pixelDepth = (*GetPortPixMap(port))->pixelSize;

	ThemeDrawingState savedState;
	ThemeDrawState curState =
		IsWindowActive(GetDialogWindow(dlg))?kThemeStateActive:kThemeStateInactive;

	GetThemeDrawingState(&savedState);
	SetThemeBackground(
		(curState == kThemeStateActive)?kThemeBrushDialogBackgroundActive:kThemeBrushDialogBackgroundInactive,
		pixelDepth, pixelDepth > 1);
	EraseRect(&ir);
	DrawThemeGenericWell (&ir, curState, false);
	SetThemeDrawingState(savedState, true);
#else
	FrameRect(&ir);
#endif
	for (i=0; i<devcnt; i++) {
		ir=devices[i].rect;
		if (i==curix) {
			PenSize(2,2);
			InsetRect(&ir,2,2);
		} else {
			PenSize(1,1);
			InsetRect(&ir,1,1);
		}
		RGBForeColor(&fifty);
		PaintRect(&ir);
		ForeColor(blackColor);
		FrameRect(&devices[i].rect);
		PenSize(1,1);
		if (i==mainix) {
			ir=devices[i].rect;
			InsetRect(&ir,1,1);
			ir.bottom=ir.top+4;
			ForeColor(whiteColor);
			PaintRect(&ir);
			ForeColor(blackColor);
			MoveTo(ir.left,ir.bottom);
			LineTo(ir.right-1,ir.bottom);
		}
	}
}

#if defined(TARGET_API_MAC_CARBON)
UserItemUPP draw_desktop_upp = NewUserItemUPP(draw_desktop);
#else
#if GENERATINGCFM
static RoutineDescriptor draw_desktop_desc =
	BUILD_ROUTINE_DESCRIPTOR(uppUserItemProcInfo,draw_desktop);
#define draw_desktop_upp (&draw_desktop_desc)
#else
#define draw_desktop_upp draw_desktop
#endif
#endif

static pascal void draw_group(
	DialogPtr dlg,
	short item)
{
	short it;
	Handle ih;
	Rect ir;
	RgnHandle outer,inner;
	static RGBColor third={0x5555,0x5555,0x5555};

	inner=NewRgn();
	outer=NewRgn();
	GetDialogItem(dlg,item,&it,&ih,&ir);
	RectRgn(outer,&ir);
	InsetRect(&ir,1,1);
	RectRgn(inner,&ir);
	DiffRgn(outer,inner,outer);
	GetDialogItem(dlg,item+1,&it,&ih,&ir);
	RectRgn(inner,&ir);
	DiffRgn(outer,inner,outer);
	DisposeRgn(inner);
	RGBForeColor(&third);
	PaintRgn(outer);
	ForeColor(blackColor);
	DisposeRgn(outer);
}

#if defined(TARGET_API_MAC_CARBON)
UserItemUPP draw_group_upp = NewUserItemUPP(draw_group);
#else
#if GENERATINGCFM
static RoutineDescriptor draw_group_desc =
	BUILD_ROUTINE_DESCRIPTOR(uppUserItemProcInfo,draw_group);
#define draw_group_upp (&draw_group_desc)
#else
#define draw_group_upp draw_group
#endif
#endif

static pascal Boolean device_filter(
	DialogPtr dlg,
	EventRecord *event,
	short *hit)
{
	WindowPtr win;
	int item;
	Point where;
	GrafPtr saveport;
	int i;

	switch (event->what) {
	case mouseDown:
		switch (FindWindow(event->where,&win)) {
		case inContent:
			where=event->where;
			GetPort(&saveport);
			SetPort(GetWindowPort(GetDialogWindow(dlg)));

			GlobalToLocal(&where);
			item=FindDialogItem(dlg,where)+1;
			switch (item) {
			case itemDesktopUser:
				for (i=0; i<devcnt; i++) {
					if (PtInRect(where,&devices[i].rect)) {
						if (curix!=i) {
							curix=i;
							draw_desktop(dlg,itemDesktopUser);
						}
						break;
					}
				}
				break;
			}
			SetPort(saveport);
			break;
		}
		break;
	}
	return general_filter_proc(dlg,event,hit);
}

#if defined(TARGET_API_MAC_CARBON)
ModalFilterUPP device_filter_upp = NewModalFilterUPP(device_filter);
#else
#if GENERATINGCFM
static RoutineDescriptor device_filter_desc =
	BUILD_ROUTINE_DESCRIPTOR(uppModalFilterProcInfo,device_filter);
#define device_filter_upp (&device_filter_desc)
#else
#define device_filter_upp device_filter
#endif
#endif

void display_device_dialog(
	GDSpecPtr spec)
{
	DialogPtr dlg;
	short it;
	Handle ih;
	Rect ir;
	GDHandle dev,maindev;
	int i,cnt;
	Rect one,all,small;
	int width,height;
	ControlHandle graysradio,colorsradio;
	bool done;
	short hit;

	maindev=GetMainDevice();
	dlg=myGetNewDialog(dlgScreenChooser,NULL,(WindowPtr)-1,0);
	assert(dlg);
/*
	GetDialogItem(dlg,itemGroupUser,&it,&ih,&ir);
	SetDialogItem(dlg,itemGroupUser,kUserDialogItem|kItemDisableBit,
		(Handle)draw_group_upp,&ir);
*/
	GetDialogItem(dlg,itemDesktopUser,&it,&ih,&ir);
	SetDialogItem(dlg,itemDesktopUser,kUserDialogItem|kItemDisableBit,
		(Handle)draw_desktop_upp,&ir);
	cnt=0;
	all.left=all.top=0x7FFF;
	all.right=all.bottom=0x8000;
	for (dev=GetDeviceList(); dev; dev=GetNextDevice(dev)) {
		if (!TestDeviceAttribute(dev,screenDevice)
				|| !TestDeviceAttribute(dev,screenActive))
			continue;
		one=(*dev)->gdRect;
		UnionRect(&all,&one,&all);
		cnt++;
	}
	width=all.right-all.left>>4;
	height=all.bottom-all.top>>4;
	small.left=(ir.left+ir.right-width)/2;
	small.top=(ir.top+ir.bottom-height)/2;
	small.right=small.left+width;
	small.bottom=small.top+height;
	devices= new dev_info[cnt];
	assert(devices);
	devcnt=cnt;
	i=0;
	mainix=NONE;
	curix=NONE;
	for (dev=GetDeviceList(); dev; dev=GetNextDevice(dev)) {
		if (!TestDeviceAttribute(dev,screenDevice)
				|| !TestDeviceAttribute(dev,screenActive))
			continue;
		devices[i].rect=(*dev)->gdRect;
		MapRect(&devices[i].rect,&all,&small);
		devices[i].dev=dev;
		if (dev==maindev)
			mainix=i;
		if (GetSlotFromGDevice(dev)==spec->slot)
			curix=i;
		i++;
	}
	if (curix==NONE)
		curix=mainix;
	GetDialogItem(dlg,itemGraysRadio,&it,&ih,&ir);
	graysradio=(ControlHandle)ih;
	GetDialogItem(dlg,itemColorsRadio,&it,&ih,&ir);
	colorsradio=(ControlHandle)ih;
	SetControlValue((spec->flags&1<<gdDevType) ? colorsradio : graysradio,1);
#if USE_SHEETS
	SetThemeWindowBackground(GetDialogWindow(dlg), kThemeBrushSheetBackgroundTransparent, false);
	ShowSheetWindow(GetDialogWindow(dlg), ActiveNonFloatingWindow());
#else
	ShowWindow(GetDialogWindow(dlg));
#endif

	for (done=false; !done; ) {
		ModalDialog(device_filter_upp,&hit);
		switch (hit) {
		case itemOKButton:
		case itemCancelButton:
			done=true;
			break;
		case itemColorsRadio:
			SetControlValue(graysradio,0);
			SetControlValue(colorsradio,1);
			break;
		case itemGraysRadio:
			SetControlValue(colorsradio,0);
			SetControlValue(graysradio,1);
			break;
		}
	}
#if USE_SHEETS
	HideSheetWindow(GetDialogWindow(dlg));
#else
	HideWindow(GetDialogWindow(dlg));
#endif

	if (hit==itemOKButton) {
		spec->slot=GetSlotFromGDevice(devices[curix].dev);
		spec->flags=GetControlValue(colorsradio) ? 1<<gdDevType : 0;
	}
	delete []devices;
	DisposeDialog(dlg);
}

#endif
