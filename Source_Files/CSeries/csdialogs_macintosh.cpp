/* csdialogs_macintosh.cpp

	Copyright (C) 1991-2001 and beyond by Bo Lindbergh
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

#if defined(EXPLICIT_CARBON_HEADER)
    #include <Carbon/Carbon.h>
/*
#else
#include <Dialogs.h>
#include <TextUtils.h>
*/
#endif

#include "cstypes.h"
#include "csdialogs.h"
#include "csmacros.h"
#include "csstrings.h"

extern void update_any_window(
	WindowPtr window,
	EventRecord *event);
extern void activate_any_window(
	WindowPtr window,
	EventRecord *event,
	bool active);

static bool cursor_tracking=true;
static dialog_header_proc_ptr header_proc;
static OSErr get_handle_for_dialog_item(DialogPtr, short, Handle *);

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
//#if defined(USE_CARBON_ACCESSORS)
		SetWRefCon(GetDialogWindow(dlg),refcon);
/*
#else
		SetWRefCon(dlg,refcon);
#endif
*/
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
//#if defined(USE_CARBON_ACCESSORS)
		SetPort(GetWindowPort(win));
/*
#else
		SetPort(win);
#endif
*/
		if (GetWindowKind(win)==kDialogWindowKind) {
			if (header_proc) {
//#if defined(USE_CARBON_ACCESSORS)
				GetPortBounds(GetWindowPort(GetDialogWindow(dlg)), &frame);
				(*header_proc)(GetDialogFromWindow(win),&frame);
/*
#else
				frame=dlg->portRect;
				(*header_proc)(win,&frame);
#endif
*/
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
	Handle ih;

	get_handle_for_dialog_item(dlg, item, &ih);
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
	Handle ih;

	NumToString(number,str);
	get_handle_for_dialog_item(dlg, item, &ih);
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
//#if defined(USE_CARBON_ACCESSORS)
	if (GetControlHilite(button)!=kControlNoPart)
		return false;
/*
#else
	if ((*button)->contrlHilite!=kControlNoPart)
		return false;
#endif
*/
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
#if TARGET_API_MAC_CARBON
	if (hilite!=NONE)
	{
		// JTP: This works well for non-buttons, such as labels and edit text boxes
		// but is useless without a control hierarchy enabled, so we fall back to the old ways
		ControlRef hierarchyControl;
		if((hilite == CONTROL_INACTIVE || hilite == CONTROL_ACTIVE)
			&& (GetDialogItemAsControl( dlg, item, &hierarchyControl ) == noErr))
		{
			if(hilite == CONTROL_INACTIVE)
				#ifdef __MACH__
					DisableControl(hierarchyControl);
				#else
					DeactivateControl(hierarchyControl);
				#endif
			else
				#ifdef __MACH__
					EnableControl(hierarchyControl);
				#else
					ActivateControl(hierarchyControl);
				#endif
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
//#if defined(USE_CARBON_ACCESSORS)
	SetPort(GetWindowPort(win));
	GetPortBounds(GetWindowPort(win), &pr);
/*
#else
	SetPort(win);
	pr=win->portRect;
#endif
*/
	LocalToGlobal((Point *)&pr.top);
	LocalToGlobal((Point *)&pr.bottom);
	SetPort(saveport);
	*frame=pr;
}

// ZZZ: added these to parallel SDL version.  Now we have a common interface.
void
copy_pstring_from_text_field(DialogPtr dialog, short item, unsigned char* pstring) {
	Handle item_handle;

	get_handle_for_dialog_item(dialog, item, &item_handle);
	GetDialogItemText(item_handle, pstring);
}

void
copy_pstring_to_text_field(DialogPtr dialog, short item, const unsigned char* pstring) {
	Handle item_handle;

	get_handle_for_dialog_item(dialog, item, &item_handle);
	SetDialogItemText(item_handle, pstring);
}

void
copy_pstring_to_static_text(DialogPtr dialog, short item, const unsigned char* pstring) {
	Handle item_handle;

	get_handle_for_dialog_item(dialog, item, &item_handle);
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

static OSErr get_handle_for_dialog_item(DialogPtr __dialog, short __item, Handle *__handle)
{
	ControlRef control_ref;	
	// If control embedding is turned on, then use appropriate variants
	if(GetDialogItemAsControl(__dialog, __item, &control_ref) == noErr)
	{
		*__handle= reinterpret_cast<Handle>(control_ref);
	} else {
		Rect    item_rect;
		short   item_type;
		Handle  item_handle;   

		GetDialogItem(__dialog, __item, &item_type, &item_handle, &item_rect);
		*__handle= item_handle;
	}
	return noErr;
}

#if TARGET_API_MAC_CARBON
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


// For Carbon-events dialogs
#ifdef USES_NIBS

#include "csalerts.h"
#include <stdio.h>
#include <string.h>


AutoNibWindow::AutoNibWindow(IBNibRef Nib, CFStringRef Name)
{
	OSStatus err;
	
	err = CreateWindowFromNib(Nib,Name,&w);
	
	const int BufferSize = 192;	// Smaller than 256, the size of "temporary"
	char Buffer[BufferSize];
	if (err != noErr)
	{
		CFStringGetCString(Name, Buffer, BufferSize, kCFStringEncodingMacRoman);
		Buffer[BufferSize-1] = 0; // Null terminator byte
	}	
	vassert(err == noErr,
		csprintf(temporary,"CreateWindowFromNib error: %d for window %s",err,Buffer));
}


AutoNibWindow::~AutoNibWindow() {DisposeWindow(w);}


ControlRef GetCtrlFromWindow(WindowRef DlgWindow, uint32 Signature, uint32 ID)
{
	OSStatus err;
	
	// Set up the popup menu
	ControlID CtrlID;
	CtrlID.signature = Signature;
	CtrlID.id = ID;
	
	ControlRef Ctrl;
	err = GetControlByID(DlgWindow,&CtrlID,&Ctrl);
	vassert(err == noErr, csprintf(temporary,"GetControlByID error for sig and id: %d - %d %d",err,Signature,ID));
	assert(Ctrl);
	
	return Ctrl;	
}


void SetControlActivity(ControlRef Ctrl, bool Activity)
{

#ifdef __MACH__
	if(Activity)
		EnableControl(Ctrl);
	else
		DisableControl(Ctrl);
#else
	if(Activity)
		ActivateControl(Ctrl);
	else
		DeactivateControl(Ctrl);
#endif
}

void GetStaticPascalText(ControlRef Ctrl, Str255 Text, int MaxLen)
{
	const int BufferLen = 255;
	char Buffer[BufferLen];
	Size ActualLen = 0;
	
	GetControlData(Ctrl,
		kControlLabelPart,
		kControlStaticTextTextTag,
		BufferLen,
		Buffer,
		&ActualLen
		);
	
	if (MaxLen < ActualLen) ActualLen = MaxLen;
	Text[0] = ActualLen;
	memcpy(Text+1,Buffer,ActualLen);
}

void SetStaticPascalText(ControlRef Ctrl, ConstStr255Param Text)
{
	SetControlData(Ctrl,
		kControlLabelPart,
		kControlStaticTextTextTag,
		Text[0],
		Text+1
		);
}

void GetStaticCText(ControlRef Ctrl, char *Text, int MaxLen)
{
	const int BufferLen = 255;
	char Buffer[BufferLen];
	Size ActualLen = 0;
	
	GetControlData(Ctrl,
		kControlLabelPart,
		kControlStaticTextTextTag,
		BufferLen,
		Buffer,
		&ActualLen
		);
	
	if (MaxLen < ActualLen) ActualLen = MaxLen;
	Text[ActualLen] = 0;
	memcpy(Text,Buffer,ActualLen);
}

void SetStaticCText(ControlRef Ctrl, const char *Text)
{
	SetControlData(Ctrl,
		kControlLabelPart,
		kControlStaticTextTextTag,
		strlen(Text),
		Text
		);
}

void GetEditPascalText(ControlRef Ctrl, Str255 Text, int MaxLen)
{
	const int BufferLen = 255;
	char Buffer[BufferLen];
	Size ActualLen = 0;
	
	GetControlData(Ctrl,
		kControlEditTextPart,
		kControlEditTextTextTag,
		BufferLen,
		Buffer,
		&ActualLen
		);
	
	if (MaxLen < ActualLen) ActualLen = MaxLen;
	Text[0] = ActualLen;
	memcpy(Text+1,Buffer,ActualLen);
}

void SetEditPascalText(ControlRef Ctrl, ConstStr255Param Text)
{
	SetControlData(Ctrl,
		kControlEditTextPart,
		kControlEditTextTextTag,
		Text[0],
		Text+1
		);
}

void GetEditCText(ControlRef Ctrl, char *Text, int MaxLen)
{
	const int BufferLen = 255;
	char Buffer[BufferLen];
	Size ActualLen = 0;
	
	GetControlData(Ctrl,
		kControlEditTextPart,
		kControlEditTextTextTag,
		BufferLen,
		Buffer,
		&ActualLen
		);
	
	if (MaxLen < ActualLen) ActualLen = MaxLen;
	Text[ActualLen] = 0;
	memcpy(Text,Buffer,ActualLen);
}

void SetEditCText(ControlRef Ctrl, const char *Text)
{
	SetControlData(Ctrl,
		kControlEditTextPart,
		kControlEditTextTextTag,
		strlen(Text),
		Text
		);
}


// Will not support any fancy hit testing;
// a user-defined control will effectively be one object
const int UserControlPart = 1;

const OSType AppTag = 'this';
const OSType DrawTag = 'draw';


struct ControlDrawerData
{
	void (*DrawFunction)(ControlRef, void *);
	void *DrawData;
};


static pascal void ControlDrawer(ControlRef Ctrl, short Part)
{
	OSStatus err;
	unsigned long ActualSize;
	ControlDrawerData CtrlDD;
	
	err = GetControlProperty(Ctrl,
			AppTag, DrawTag,
			sizeof(CtrlDD), &ActualSize, &CtrlDD);
	vassert(err == noErr, csprintf(temporary,"GetControlProperty error: %d",err));
	
	assert(CtrlDD.DrawFunction);
	CtrlDD.DrawFunction(Ctrl,CtrlDD.DrawData);
}


AutoDrawability::AutoDrawability()
{
	DrawingUPP = NewControlUserPaneDrawUPP(ControlDrawer);
}

AutoDrawability::~AutoDrawability()
{
	DisposeControlUserPaneDrawUPP(DrawingUPP);
}

void AutoDrawability::operator()(ControlRef Ctrl,
		void (*DrawFunction)(ControlRef, void *),
		void *DrawData)
{
	OSStatus err;
	
	ControlDrawerData CtrlDD;
	
	CtrlDD.DrawFunction = DrawFunction;
	CtrlDD.DrawData = DrawData;
	
	err = SetControlData(
			Ctrl, 0,
			kControlUserPaneDrawProcTag,
			sizeof(DrawingUPP), &DrawingUPP
			);
	vassert(err == noErr, csprintf(temporary,"SetControlData error: %d",err));
	
	err = SetControlProperty(
			Ctrl,
			AppTag, DrawTag,
			sizeof(CtrlDD), &CtrlDD
			);
	vassert(err == noErr, csprintf(temporary,"SetControlProperty error: %d",err));
}


void SwatchDrawer(ControlRef Ctrl, void *Data)
{
	RGBColor *SwatchColorPtr = (RGBColor *)(Data);
	
	// No need for the window context -- it's assumed
	Rect Bounds = {0,0,0,0};
	
	GetControlBounds(Ctrl, &Bounds);
	
	// Get ready to draw the swatch
	PenNormal();
	
	// Draw!
	RGBForeColor(SwatchColorPtr);
	PaintRect(&Bounds);
	ForeColor(blackColor);
	FrameRect(&Bounds);
}


// Globals the most convenient mechanism here,
// since an int32 is NOT a void * (generic pointer)
static ControlRef PickCtrl;
static RGBColor *PickClrPtr;


static pascal void PickColorChangedFunc(int32 Num, PMColor *NewColorPtr)
{
	// Transmit the color to the control
	CMRGBColor *ChgClrPtr = &NewColorPtr->color.rgb;		
	PickClrPtr->red = ChgClrPtr->red;
	PickClrPtr->green = ChgClrPtr->green;
	PickClrPtr->blue = ChgClrPtr->blue;
	
	Draw1Control(PickCtrl);
}


bool PickControlColor(ControlRef Ctrl,
	RGBColor *ClrPtr,
	ConstStr255Param Prompt
	)
{
	// Save old color so we can revert if necessary
	RGBColor SavedColor = *ClrPtr;
	
	// Set up the globals:
	PickCtrl = Ctrl;
	PickClrPtr = ClrPtr;
	
	// For live updating
	ColorChangedUPP ClrChg = NewColorChangedUPP(PickColorChangedFunc);
	
	// Fill in all those blanks...
	ColorPickerInfo PickerData;
	obj_clear(PickerData);	// 0 is default for everything in it
		
	// Need to load the color and the prompt into the picker data
	CMRGBColor *PkrClrPtr = &PickerData.theColor.color.rgb;		
	PkrClrPtr->red = ClrPtr->red;
	PkrClrPtr->green = ClrPtr->green;
	PkrClrPtr->blue = ClrPtr->blue;
	
	PickerData.placeWhere = kCenterOnMainScreen;
	pstrcpy(PickerData.prompt, Prompt);
	
	PickerData.colorProc = ClrChg;
	
	// Go!
	PickColor(&PickerData);
	
	bool IsOK = PickerData.newColorChosen;
	
	if (IsOK)
	{
		// Get the changed color
		ClrPtr->red = PkrClrPtr->red;
		ClrPtr->green = PkrClrPtr->green;
		ClrPtr->blue = PkrClrPtr->blue;
	}
	else
	{
		// Revert to the saved color
		*ClrPtr = SavedColor;
	}
	// Need to update
	Draw1Control(Ctrl);
	
	DisposeColorChangedUPP(ClrChg);
	return IsOK;
}


// Little more than a placeholder
static pascal ControlPartCode HitTester(ControlRef Ctrl, Point Loc)
{
	return UserControlPart;
}


AutoHittability::AutoHittability()
{
	HitTesterUPP = NewControlUserPaneHitTestUPP(HitTester);
}

AutoHittability::~AutoHittability()
{
	DisposeControlUserPaneHitTestUPP(HitTesterUPP);
}

void AutoHittability::operator()(ControlRef Ctrl)
{
	OSStatus err;
	
	err = SetControlData(
			Ctrl, UserControlPart,
			kControlUserPaneHitTestProcTag,
			sizeof(HitTesterUPP), &HitTesterUPP
			);
	vassert(err == noErr, csprintf(temporary,"SetControlData error: %d",err));
}


void BuildMenu(
	ControlRef MenuCtrl,
	bool (*BuildMenuItem)(int,uint8 *,bool &,void *),
	void *BuildMenuData
	)
{
	// Extract the menu
	MenuRef Menu = GetControlPopupMenuHandle(MenuCtrl);
	assert(Menu);
	
	// Get rid of old contents
	while(CountMenuItems(Menu)) DeleteMenuItem(Menu, 1);
	
	if(!BuildMenuItem) return;
	
	int Initial = 1;
	
	Str255 ItemName;
	for(int indx=1; indx<=255; indx++)
	{
		bool ThisIsInitial = false;
		if (!BuildMenuItem(indx,ItemName,ThisIsInitial,BuildMenuData)) break;
		if (ThisIsInitial) Initial = indx;
		
		AppendMenu(Menu, "\pReplace Me");
		SetMenuItemText(Menu, indx, ItemName);
	}
	
	SetControl32BitMaximum(MenuCtrl,CountMenuItems(Menu));
	SetControl32BitValue(MenuCtrl, Initial);
}


struct ModalDialogHandlerData
{
	WindowRef DlgWindow;
	bool IsSheet;
	void (*DlgHandler)(ParsedControl &,void *);
	void *DlgData;
	bool IsOK;	// Return whether OK was pressed
};

static pascal OSStatus ModalDialogHandler(
	EventHandlerCallRef HandlerCallRef,
	EventRef Event,
	void *Data
	)
{
	(void)(HandlerCallRef);
	
	OSStatus err;
	ParsedControl Ctrl;
	
	err = GetEventParameter(Event,
		kEventParamDirectObject,typeControlRef,
		NULL, sizeof(ControlRef), NULL, &Ctrl.Ctrl);
	vassert(err == noErr, csprintf(temporary,"GetEventParameter error: %d",err));
	
	err = GetControlID(Ctrl.Ctrl,&Ctrl.ID);
	vassert(err == noErr, csprintf(temporary,"GetControlID error: %d",err));

	ModalDialogHandlerData *HDPtr = (ModalDialogHandlerData *)(Data);
	if (HDPtr->DlgHandler)
		HDPtr->DlgHandler(Ctrl,HDPtr->DlgData);
	
	// For quitting the dialog box
	if (Ctrl.ID.signature == 0 && (Ctrl.ID.id == iOK || Ctrl.ID.id == iCANCEL))
	{
		// Which one pressed
		HDPtr->IsOK = (Ctrl.ID.id == iOK);
		
		// Done with the window
		StopModalDialog(HDPtr->DlgWindow,HDPtr->IsSheet);
	}
	
	return noErr;
}


bool RunModalDialog(
	WindowRef DlgWindow,
	bool IsSheet,
	void (*DlgHandler)(ParsedControl &,void *),
	void *DlgData
	)
{
	OSStatus err;
	ModalDialogHandlerData HandlerData;
	HandlerData.DlgWindow = DlgWindow;
	HandlerData.IsSheet = IsSheet;
	HandlerData.DlgHandler = DlgHandler;
	HandlerData.DlgData = DlgData;
	HandlerData.IsOK = false;
	
	const int NumEventTypes = 1;
	const EventTypeSpec EventTypes[NumEventTypes] =
		{
			{kEventClassControl, kEventControlHit}
		};
	
	EventHandlerUPP HandlerUPP = NewEventHandlerUPP(ModalDialogHandler);
	err = InstallWindowEventHandler(DlgWindow, HandlerUPP,
		NumEventTypes, EventTypes,
		&HandlerData, NULL);
	vassert(err == noErr, csprintf(temporary,"InstallWindowEventHandler error: %d",err));
	
	if (IsSheet)
#if USE_SHEETS
	{
		SetThemeWindowBackground(DlgWindow, kThemeBrushSheetBackgroundTransparent, false);
		ShowSheetWindow(DlgWindow, ActiveNonFloatingWindow());
	}
#else
		ShowWindow(DlgWindow);
#endif
	else
		ShowWindow(DlgWindow);
	RunAppModalLoopForWindow(DlgWindow);
	
	DisposeEventHandlerUPP(HandlerUPP);
	
	return HandlerData.IsOK;
}


void StopModalDialog(
	WindowRef DlgWindow,
	bool IsSheet
	)
{
	QuitAppModalLoopForWindow(DlgWindow);
	if (IsSheet)
#if USE_SHEETS
		HideSheetWindow(DlgWindow);
#else
		HideWindow(DlgWindow);
#endif
	else
		HideWindow(DlgWindow);
}


AutoTimer::AutoTimer(
	EventTimerInterval Delay,
	EventTimerInterval Interval,
	EventLoopTimerProcPtr Handler,
	void *HandlerData
	)
{
	OSStatus err;
	
	EventLoopRef EventLoop = GetCurrentEventLoop();
	HandlerUPP = NewEventLoopTimerUPP(Handler);
	err = InstallEventLoopTimer(
		EventLoop,
		Delay, Interval,
		HandlerUPP, HandlerData,
		&Timer
	);
	
	vassert(err == noErr, csprintf(temporary, "Error in InstallEventLoopTimer: %d",err));
}

AutoTimer::~AutoTimer()
{
	RemoveEventLoopTimer(Timer);	
	DisposeEventLoopTimerUPP(HandlerUPP);
}


AutoKeyboardWatcher::AutoKeyboardWatcher(
	EventHandlerProcPtr Handler		// Called for every keystroke
	)
{
	KeyboardHandlerUPP = NewEventHandlerUPP(Handler);
}

AutoKeyboardWatcher::~AutoKeyboardWatcher()
{
	DisposeEventHandlerUPP(KeyboardHandlerUPP);
}

void AutoKeyboardWatcher::Watch(
	ControlRef Ctrl,				// Control to watch keystrokes fat
	void *HandlerData
	)
{
	OSStatus err;
	
	// From Br'fin's keyboard event handler
	const int NumKeyboardEvents = 3;
	const EventTypeSpec KeyboardEvents[NumKeyboardEvents] = {
		{kEventClassKeyboard, kEventRawKeyDown},
		{kEventClassKeyboard, kEventRawKeyModifiersChanged},
		{kEventClassKeyboard, kEventRawKeyRepeat}
	};
	
	err = InstallControlEventHandler(Ctrl, KeyboardHandlerUPP,
			NumKeyboardEvents, KeyboardEvents,
			HandlerData, NULL);
	
	vassert(err == noErr, csprintf(temporary, "Error in InstallControlEventHandler: %d",err));
}

// Convert between control values and floats from 0 to 1.
// Should be especially useful for sliders.

float GetCtrlFloatValue(ControlRef Ctrl)
{
	int32 MinVal = GetControl32BitMinimum(Ctrl);
	int32 MaxVal = GetControl32BitMaximum(Ctrl);
	assert(MaxVal != MinVal);
	
	int32 Value = GetControl32BitValue(Ctrl);
	
	return float(Value - MinVal) / float(MaxVal - MinVal);
}

void SetCtrlFloatValue(ControlRef Ctrl, float Value)
{
	int32 MinVal = GetControl32BitMinimum(Ctrl);
	int32 MaxVal = GetControl32BitMaximum(Ctrl);
	assert(MaxVal != MinVal);

	// Round instead of truncating
	float FVal = MinVal + (MaxVal - MinVal)*Value;
	int IVal = FVal > 0 ? int(FVal + 0.5) : -int(-FVal + 0.5);
	
	SetControl32BitValue(Ctrl, IVal);
}

#endif
