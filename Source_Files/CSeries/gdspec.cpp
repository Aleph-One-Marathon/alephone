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

*/
// LP: not sure who originally wrote these cseries files: Bo Lindbergh?
/*
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

#if defined(TARGET_API_MAC_CARBON)
    #include <Carbon/Carbon.h>
#else
#include <Dialogs.h>
#include <Palettes.h>
#include <Devices.h>
#endif

#include "cstypes.h"
#include "csdialogs.h"
#include "csalerts.h"
#include "gdspec.h"

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
	GDHandle dev)
{
#if defined(TARGET_API_MAC_CARBON)
	return NONE;
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
	itemGroupUser,
	itemGroupText,
	itemDescriptionText
};

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
#if defined(USE_CARBON_ACCESSORS)
			SetPort(GetWindowPort(GetDialogWindow(dlg)));
#else
			SetPort(dlg);
#endif
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
#if defined(USE_CARBON_ACCESSORS)
#if USE_SHEETS
	SetThemeWindowBackground(GetDialogWindow(dlg), kThemeBrushSheetBackgroundTransparent, false);
	ShowSheetWindow(GetDialogWindow(dlg), ActiveNonFloatingWindow());
#else
	ShowWindow(GetDialogWindow(dlg));
#endif
#else
	ShowWindow(dlg);
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
#if defined(USE_CARBON_ACCESSORS)
#if USE_SHEETS
	HideSheetWindow(GetDialogWindow(dlg));
#else
	HideWindow(GetDialogWindow(dlg));
#endif
#else
	HideWindow(dlg);
#endif
	if (hit==itemOKButton) {
		spec->slot=GetSlotFromGDevice(devices[curix].dev);
		spec->flags=GetControlValue(colorsradio) ? 1<<gdDevType : 0;
	}
	delete []devices;
	DisposeDialog(dlg);
}

