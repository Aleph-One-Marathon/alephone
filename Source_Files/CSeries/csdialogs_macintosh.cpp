/* csdialogs_macintosh.cpp

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

// LP: not sure who originally wrote these cseries files: Bo Lindbergh?

    Sept-Nov 2001 (Woody Zenfell):
        renamed from csdialogs.cpp
        inserted get_dialog_control_value, from network_dialogs.cpp
        new functions copy_pstring_*_text_field()

Jan 25, 2002 (Br'fin (Jeremy Parsons)):
	Added TARGET_API_MAC_CARBON for Carbon.h
	Added accessors for datafields now opaque in Carbon

Feb 3, 2002 (Br'fin (Jeremy Parsons)):
	For carbon, replaced framing of our own group boxes with a theme seperator line at the top

Feb 27, 2002 (Br'fin (Jeremy Parsons)):
	Adjusted modify_control to call the recommended De/ActivateControl
		for controls in a control hierarchy under Carbon
	Added utility routine GetListBoxListHandle for Carbon
*/

#if defined(TARGET_API_MAC_CARBON)
    #include <Carbon/Carbon.h>
#else
#include <Dialogs.h>
#include <TextUtils.h>
#endif

#include "cstypes.h"
#include "csdialogs.h"

extern void update_any_window(
	WindowPtr window,
	EventRecord *event);
extern void activate_any_window(
	WindowPtr window,
	EventRecord *event,
	bool active);

static bool cursor_tracking=true;
static dialog_header_proc_ptr header_proc;

DialogPtr myGetNewDialog(
	short id,
	void *storage,
	WindowPtr before,
	long refcon)
{
	DialogPtr dlg;
	short it;
	Handle ih;
	Rect ir;

	dlg=GetNewDialog(id,storage,before);
	if (dlg) {
#if defined(USE_CARBON_ACCESSORS)
		SetWRefCon(GetDialogWindow(dlg),refcon);
#else
		SetWRefCon(dlg,refcon);
#endif
		GetDialogItem(dlg,iOK,&it,&ih,&ir);
		if (it==kButtonDialogItem) {
			SetDialogDefaultItem(dlg,iOK);
			GetDialogItem(dlg,iCANCEL,&it,&ih,&ir);
			if (it==kButtonDialogItem)
				SetDialogCancelItem(dlg,iCANCEL);
		}
	}
	return dlg;
}

static RGBColor third={0x5555,0x5555,0x5555};

static void frame_useritems(
	DialogPtr dlg)
{
	RgnHandle outer,inner,good,bad;
	int i,n;
	short it;
	Handle ih;
	Rect ir;
	Boolean fillRegion = false;

	good=NewRgn();
	bad=NewRgn();
	outer=NewRgn();
	inner=NewRgn();
	n=CountDITL(dlg);
	for (i=1; i<=n; i++) {
		GetDialogItem(dlg,i,&it,&ih,&ir);
		if ((it&~kItemDisableBit)==kUserDialogItem && !ih) {
#if defined(TARGET_API_MAC_CARBON)
		/*
		 * A user item with no drawing proc installed is assumed to be a group box.
		 * Use the appearance manager to draw a seperator line at the top of the group
		 */
			// And keep it from being painted over
			RectRgn(outer,&ir);
			UnionRgn(bad,outer,bad);

			ThemeDrawState curState =
				IsWindowActive(GetDialogWindow(dlg))?kThemeStateActive:kThemeStateInactive;
			
			ir.bottom = ir.top + 2;
			ir.left += 10;
			
			DrawThemeSeparator(&ir, curState);
#else
		/*
		 * A user item with no drawing proc installed is assumed to be a group box.
		 * Make a 1-pixel-thick frame from its rect and add it to the region to paint.
		 */
			RectRgn(outer,&ir);
			CopyRgn(outer,inner);
			InsetRgn(inner,1,1);
			DiffRgn(outer,inner,outer);
			UnionRgn(good,outer,good);
#endif
		} else {
		/*
		 * For all other items, add its rect to the region _not_ to paint.
		 */
			RectRgn(outer,&ir);
			UnionRgn(bad,outer,bad);
		}
	}
	DisposeRgn(inner);
	DisposeRgn(outer);
	DiffRgn(good,bad,good);
	DisposeRgn(bad);
	if(fillRegion)
	{
		RGBForeColor(&third);
		PaintRgn(good);
	}
	ForeColor(blackColor);
	DisposeRgn(good);
}

pascal Boolean general_filter_proc(
	DialogPtr dlg,
	EventRecord *event,
	short *hit)
{
	WindowPtr win;
	Rect frame;
	GrafPtr saveport;

	GetPort(&saveport);
	SetDialogTracksCursor(dlg,cursor_tracking);
	switch (event->what) {
	case updateEvt:
		win=(WindowPtr)event->message;
#if defined(USE_CARBON_ACCESSORS)
		SetPort(GetWindowPort(win));
#else
		SetPort(win);
#endif
		if (GetWindowKind(win)==kDialogWindowKind) {
			if (header_proc) {
#if defined(USE_CARBON_ACCESSORS)
				GetPortBounds(GetWindowPort(GetDialogWindow(dlg)), &frame);
				(*header_proc)(GetDialogFromWindow(win),&frame);
#else
				frame=dlg->portRect;
				(*header_proc)(win,&frame);
#endif
			}
			frame_useritems(dlg);
		} else {
			update_any_window(win,event);
		}
		break;
	case activateEvt:
		win=(WindowPtr)event->message;
		if (GetWindowKind(win)!=kDialogWindowKind)
			activate_any_window(win,event,(event->modifiers&activeFlag)!=0);
		break;
	}
	SetPort(saveport);
	return StdFilterProc(dlg,event,hit);
}

#if defined(TARGET_API_MAC_CARBON)
ModalFilterUPP general_filter_upp = NewModalFilterUPP(general_filter_proc);
#else
#if GENERATINGCFM
static RoutineDescriptor general_filter_desc =
	BUILD_ROUTINE_DESCRIPTOR(uppModalFilterProcInfo,general_filter_proc);
#define general_filter_upp (&general_filter_desc)
#else
#define general_filter_upp general_filter_proc
#endif
#endif

ModalFilterUPP get_general_filter_upp(void)
{
	return general_filter_upp;
}

void set_dialog_cursor_tracking(
	bool tracking)
{
	cursor_tracking=tracking;
}

long extract_number_from_text_item(
	DialogPtr dlg,
	short item)
{
	Str255 str;
	long num;
	short it;
	Handle ih;
	Rect ir;

	GetDialogItem(dlg,item,&it,&ih,&ir);
	GetDialogItemText(ih,str);
	StringToNum(str,&num);
	return num;
}

void insert_number_into_text_item(
	DialogPtr dlg,
	short item,
	long number)
{
	Str255 str;
	short it;
	Handle ih;
	Rect ir;

	NumToString(number,str);
	GetDialogItem(dlg,item,&it,&ih,&ir);
	SetDialogItemText(ih,str);
}

bool hit_dialog_button(
	DialogPtr dlg,
	short item)
{
	short it;
	Handle ih;
	Rect ir;
	ControlHandle button;
	unsigned long ignore;

	GetDialogItem(dlg,item,&it,&ih,&ir);
	if (it!=kButtonDialogItem)
		return false;
	button=(ControlHandle)ih;
#if defined(USE_CARBON_ACCESSORS)
	if (GetControlHilite(button)!=kControlNoPart)
		return false;
#else
	if ((*button)->contrlHilite!=kControlNoPart)
		return false;
#endif
	HiliteControl(button,kControlButtonPart);
	Delay(8,&ignore);
	HiliteControl(button,kControlNoPart);
	return true;
}

void modify_control(
	DialogPtr dlg,
	short item,
	short hilite,
	short value)
{
	short it;
	Handle ih;
	Rect ir;
	ControlHandle control;

	GetDialogItem(dlg,item,&it,&ih,&ir);
	control=(ControlHandle)ih;
#if defined(TARGET_API_MAC_CARBON)
	if (hilite!=NONE)
	{
		// JTP: This works well for non-buttons, such as labels and edit text boxes
		// but is useless without a control hierarchy enabled, so we fall back to the old ways
		ControlRef hierarchyControl;
		if((hilite == CONTROL_INACTIVE || hilite == CONTROL_ACTIVE)
			&& (GetDialogItemAsControl( dlg, item, &hierarchyControl ) == noErr))
		{
			if(hilite == CONTROL_INACTIVE)
				DeactivateControl(hierarchyControl);
			else
				ActivateControl(hierarchyControl);
		}
		else
			HiliteControl(control,hilite);
	}
#else	
	if (hilite!=NONE)
		HiliteControl(control,hilite);
#endif
	if (value!=NONE)
		SetControlValue(control,value);
}

void modify_radio_button_family(
	DialogPtr dlg,
	short firstItem,
	short lastItem,
	short activeItem)
{
	int item;
	short it;
	Handle ih;
	Rect ir;
	ControlHandle control;
	int value;

	for (item=firstItem; item<=lastItem; item++) {
		GetDialogItem(dlg,item,&it,&ih,&ir);
		control=(ControlHandle)ih;
		if (item==activeItem) {
			value=kControlRadioButtonCheckedValue;
		} else {
			value=kControlRadioButtonUncheckedValue;
		}
		SetControlValue(control,value);
	}
}

void set_dialog_header_proc(
	dialog_header_proc_ptr proc)
{
	header_proc=proc;
}

void AdjustRect(
	Rect const *frame,
	Rect const *in,
	Rect *out,
	short how)
{
	int dim;

	switch (how) {
	case centerRect:
		dim=in->right-in->left;
		out->left=(frame->left+frame->right-dim)/2;
		out->right=out->left+dim;
		dim=in->bottom-in->top;
		out->top=(frame->top+frame->bottom-dim)/2;
		out->bottom=out->top+dim;
		break;
	}
}

void get_window_frame(
	WindowPtr win,
	Rect *frame)
{
	GrafPtr saveport;
	Rect pr;

	GetPort(&saveport);
#if defined(USE_CARBON_ACCESSORS)
	SetPort(GetWindowPort(win));
	GetPortBounds(GetWindowPort(win), &pr);
#else
	SetPort(win);
	pr=win->portRect;
#endif
	LocalToGlobal((Point *)&pr.top);
	LocalToGlobal((Point *)&pr.bottom);
	SetPort(saveport);
	*frame=pr;
}

// ZZZ: added these to parallel SDL version.  Now we have a common interface.
void
copy_pstring_from_text_field(DialogPtr dialog, short item, unsigned char* pstring) {
	Rect    item_rect;
	short   item_type;
	Handle  item_handle;   

    GetDialogItem(dialog, item, &item_type, &item_handle, &item_rect);
	GetDialogItemText(item_handle, pstring);
}

void
copy_pstring_to_text_field(DialogPtr dialog, short item, const unsigned char* pstring) {
	Rect    item_rect;
	short   item_type;
	Handle  item_handle;   

    GetDialogItem(dialog, item, &item_type, &item_handle, &item_rect);
	SetDialogItemText(item_handle, pstring);
}

void
copy_pstring_to_static_text(DialogPtr dialog, short item, const unsigned char* pstring) {
	Rect    item_rect;
	short   item_type;
	Handle  item_handle;   

    GetDialogItem(dialog, item, &item_type, &item_handle, &item_rect);
	SetDialogItemText(item_handle, pstring);
}
// ZZZ: moved here from network_dialogs_macintosh.cpp
short get_dialog_control_value(DialogPtr dialog, short which_control)
{
	Rect    item_rect;
	short   item_type;
	Handle  item_handle;
	
	GetDialogItem(dialog, which_control, &item_type, &item_handle, &item_rect);
	return GetControlValue((ControlHandle) item_handle);
}

#ifndef mac

// ZZZ: added these to inject a layer of indirection to slip in an SDL implementation and have the two versions share
void
copy_pstring_from_text_field(DialogPtr dialog, short item, unsigned char* pstring) {
    Rect                item_rect;
    short               item_type;
    Handle              item_handle;

    GetDialogItem(dialog, item, &item_type, &item_handle, &item_rect);
    GetDialogItemText(item_handle, pstring);
}

void
copy_pstring_to_text_field(DialogPtr dialog, short item, const unsigned char* pstring) {
    Rect                item_rect;
    short               item_type;
    Handle              item_handle;

    GetDialogItem(dialog, item, &item_type, &item_handle, &item_rect);
    SetDialogItemText(item_handle, pstring);
}

#endif

#if defined(TARGET_API_MAC_CARBON)
// JTP: Taken from an AppearanceManager code sample
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ GetListBoxListHandle
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	Returns the list handle from a list box control.
//
pascal OSStatus
GetListBoxListHandle( ControlHandle control, ListHandle* list )
{
	Size		actualSize;
	OSStatus	err;
	
	if ( control == nil )
		return paramErr;
		
	if ( list == nil )
		return paramErr;
		
	err = GetControlData( control, 0, kControlListBoxListHandleTag, sizeof( ListHandle ),
			 (Ptr)list, &actualSize );
		
	return err;
}
#endif