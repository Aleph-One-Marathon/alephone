/*
	Changes:

Jan 30, 2000 (Loren Petrich)
	Did some typecasts
*/

#include <stdlib.h>

#include <Dialogs.h>
#include <Palettes.h>
#include <Devices.h>

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

Boolean HasDepthGDSpec(
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

Boolean EqualGDSpec(
	GDSpecPtr spec1,
	GDSpecPtr spec2)
{
	return spec1->slot==spec2->slot;
}

short GetSlotFromGDevice(
	GDHandle dev)
{
	return (*(AuxDCEHandle)GetDCtlEntry((*dev)->gdRefNum))->dCtlSlot;
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
	FrameRect(&ir);
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

#if GENERATINGCFM
static RoutineDescriptor draw_desktop_desc =
	BUILD_ROUTINE_DESCRIPTOR(uppUserItemProcInfo,draw_desktop);
#define draw_desktop_upp (&draw_desktop_desc)
#else
#define draw_desktop_upp draw_desktop
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

#if GENERATINGCFM
static RoutineDescriptor draw_group_desc =
	BUILD_ROUTINE_DESCRIPTOR(uppUserItemProcInfo,draw_group);
#define draw_group_upp (&draw_group_desc)
#else
#define draw_group_upp draw_group
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
			SetPort(dlg);
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

#if GENERATINGCFM
static RoutineDescriptor device_filter_desc =
	BUILD_ROUTINE_DESCRIPTOR(uppModalFilterProcInfo,device_filter);
#define device_filter_upp (&device_filter_desc)
#else
#define device_filter_upp device_filter
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
	Boolean done;
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
	devices=(dev_info *)malloc(cnt*sizeof (dev_info));
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
	ShowWindow(dlg);
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
	HideWindow(dlg);
	if (hit==itemOKButton) {
		spec->slot=GetSlotFromGDevice(devices[curix].dev);
		spec->flags=GetControlValue(colorsradio) ? 1<<gdDevType : 0;
	}
	free(devices);
	DisposeDialog(dlg);
}

