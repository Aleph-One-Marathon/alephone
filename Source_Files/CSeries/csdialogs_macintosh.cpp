/* csdialogs_macintosh.cpp

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
#include "TextStrings.h"

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
		SetWRefCon(GetDialogWindow(dlg),refcon);
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
		SetPort(GetWindowPort(win));
		if (GetWindowKind(win)==kDialogWindowKind) {
			if (header_proc) {
				GetPortBounds(GetWindowPort(GetDialogWindow(dlg)), &frame);
				(*header_proc)(GetDialogFromWindow(win),&frame);
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

int32 extract_number_from_text_item(
	DialogPtr dlg,
	short item)
{
	Str255 str;
	int32 num;
	Handle ih;

	get_handle_for_dialog_item(dlg, item, &ih);
	GetDialogItemText(ih,str);
	StringToNum(str,&num);
	return num;
}

void insert_number_into_text_item(
	DialogPtr dlg,
	short item,
	int32 number)
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
	if (GetControlHilite(button)!=kControlNoPart)
		return false;
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
	SetPort(GetWindowPort(win));
	GetPortBounds(GetWindowPort(win), &pr);
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


#include "csalerts.h"
#include <stdio.h>
#include <string.h>
#include "NibsUiHelpers.h"

static OSType global_signature = 0;

std::auto_ptr<SelfReleasingCFStringRef>
StringToCFString(const std::string& s)
{
	return std::auto_ptr<SelfReleasingCFStringRef>(
		new SelfReleasingCFStringRef(
			CFStringCreateWithCString(NULL, s.c_str(), kCFStringEncodingMacRoman)
		)
	);
}



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
		csprintf(temporary,"CreateWindowFromNib error: %d for window %s",(int)err,Buffer));
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
	vassert(err == noErr, csprintf(temporary,"GetControlByID error for sig and id: %d - %d %d",(int)err,(int)Signature,(int)ID));
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

void SetGlobalControlSignature(OSType sig)
{
	global_signature = sig;
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
	vassert(err == noErr, csprintf(temporary,"GetControlProperty error: %d",(int)err));
	
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
	vassert(err == noErr, csprintf(temporary,"SetControlData error: %d",(int)err));
	
	err = SetControlProperty(
			Ctrl,
			AppTag, DrawTag,
			sizeof(CtrlDD), &CtrlDD
			);
	vassert(err == noErr, csprintf(temporary,"SetControlProperty error: %d",(int)err));
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
	vassert(err == noErr, csprintf(temporary,"SetControlData error: %d",(int)err));
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
	vassert(err == noErr, csprintf(temporary,"GetEventParameter error: %d",(int)err));
	
	err = GetControlID(Ctrl.Ctrl,&Ctrl.ID);
	vassert(err == noErr, csprintf(temporary,"GetControlID error: %d",(int)err));

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
	vassert(err == noErr, csprintf(temporary,"InstallWindowEventHandler error: %d",(int)err));
	
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


Modal_Dialog::Modal_Dialog(WindowRef dialogWindow, bool isSheet)
	: m_dialogWindow (dialogWindow)
	, m_isSheet (isSheet)
	, m_result (false)
{}

bool Modal_Dialog::Run()
{
#if USE_SHEETS
	if (m_isSheet)
	{
		SetThemeWindowBackground(m_dialogWindow, kThemeBrushSheetBackgroundTransparent, false);
		ShowSheetWindow(m_dialogWindow, ActiveNonFloatingWindow());
	} else
#endif
		ShowWindow(m_dialogWindow);

	RunAppModalLoopForWindow(m_dialogWindow);
	return m_result;
}

void Modal_Dialog::Stop(bool result)
{
	QuitAppModalLoopForWindow(m_dialogWindow);
	m_result = result;

#if USE_SHEETS
	if (m_isSheet)
		HideSheetWindow(m_dialogWindow);
	else
#endif
		HideWindow(m_dialogWindow);
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
	
	vassert(err == noErr, csprintf(temporary, "Error in InstallEventLoopTimer: %d",(int)err));
}

AutoTimer::AutoTimer(
	EventTimerInterval Delay,
	EventTimerInterval Interval,
	TimerCallback Handler
	)
{
	OSStatus err;
	
	EventLoopRef EventLoop = GetCurrentEventLoop();
	HandlerUPP = NewEventLoopTimerUPP(bounce_boosted_callback);
	err = InstallEventLoopTimer(
		EventLoop,
		Delay, Interval,
		HandlerUPP, this,
		&Timer
	);
	
	m_callback = Handler;
	
	vassert(err == noErr, csprintf(temporary, "Error in InstallEventLoopTimer: %d",(int)err));
}

AutoTimer::~AutoTimer()
{
	RemoveEventLoopTimer(Timer);	
	DisposeEventLoopTimerUPP(HandlerUPP);
}


AutoWatcher::AutoWatcher (ControlRef ctrl, int num_event_types, const EventTypeSpec* event_types)
{
	
	m_EventHandlerUPP = NewEventHandlerUPP(callback);
	InstallControlEventHandler(ctrl, m_EventHandlerUPP,
				num_event_types, event_types,
				this, NULL);
}

pascal OSStatus AutoWatcher::callback (EventHandlerCallRef inCallRef,
					EventRef inEvent, void* inUserData)
{
	// Hand off to the next event handler
	CallNextEventHandler(inCallRef, inEvent);

	// Do our thing
	return reinterpret_cast<AutoWatcher*>(inUserData)->act (inCallRef, inEvent);
}

const EventTypeSpec AutoControlWatcher::ControlWatcherEvents[1]
	= {{kEventClassControl, kEventControlHit}};

const EventTypeSpec AutoKeystrokeWatcher::KeystrokeWatcherEvents[2]
	= {{kEventClassKeyboard, kEventRawKeyDown},{kEventClassKeyboard, kEventRawKeyRepeat}};

OSStatus AutoKeystrokeWatcher::act (EventHandlerCallRef inCallRef, EventRef inEvent)
{
	if (m_callback) {
		char character;
		GetEventParameter(inEvent, kEventParamKeyMacCharCodes, typeChar,
					NULL, sizeof(char), NULL, &character);
		m_callback (character);
	}
	
	return noErr;
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
	
	vassert(err == noErr, csprintf(temporary, "Error in InstallControlEventHandler: %d",(int)err));
}

const EventTypeSpec AutoTabHandler::TabControlEvents[1]
	 = {{kEventClassControl, kEventControlHit}};

OSStatus AutoTabHandler::act (EventHandlerCallRef inCallRef, EventRef inEvent)
{
	int new_value = GetControlValue(tab) - 1;
	if (new_value != old_value) {
		SetActiveTab (new_value);
		return (noErr);
	} else
		return (eventNotHandledErr);
}

void AutoTabHandler::SetActiveTab (int new_value)
{
	old_value = new_value;
	
	for (int i = 0; i < panes.size (); ++i)
		if (i != new_value)
			SetControlVisibility (panes[i], false, true);
	
	ClearKeyboardFocus (window);
	
	SetControlVisibility (panes[new_value], true, true);
	
	Draw1Control (tab);
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

void initialize_MLTE()
{
	TXNInitTextension (NULL, 0, 0); 
}



// Similar to GetCtrlFromWindow, but doesn't mind if control doesn't exist
static ControlRef get_control_from_window (DialogPTR dlg, int item)
{
	OSStatus err;
	
	ControlID CtrlID;
	CtrlID.signature = global_signature;
	CtrlID.id = item;
	
	ControlRef control = NULL;
	err = GetControlByID(dlg,&CtrlID,&control);

	return control;	
}

bool QQ_control_exists (DialogPTR dlg, int item)
{
	return (get_control_from_window (dlg, item) != NULL);
}

void QQ_hide_control (DialogPTR dlg, int item)
{
	HideControl (get_control_from_window (dlg, item));
}

void QQ_show_control (DialogPTR dlg, int item)
{
	ShowControl (get_control_from_window (dlg, item));
}

bool QQ_get_boolean_control_value (DialogPTR dlg, int item)
{
	return GetControl32BitValue (get_control_from_window (dlg, item));
}

void QQ_set_boolean_control_value (DialogPTR dlg, int item, bool value)
{
	SetControl32BitValue (get_control_from_window (dlg, item), value ? 1 : 0);
}

int QQ_get_selector_control_value (DialogPTR dlg, int item)
{
	return GetControl32BitValue (get_control_from_window (dlg, item)) - 1;
}

void QQ_set_selector_control_value (DialogPTR dlg, int item, int value)
{
	SetControl32BitValue (get_control_from_window (dlg, item), value + 1);
}

void QQ_set_selector_control_labels (DialogPTR dlg, int item, const std::vector<std::string> labels)
{
	// Possibly should extend to operate on radio groups too?

	// Get the menu
	ControlRef MenuCtrl = get_control_from_window (dlg, item);
	MenuRef Menu = GetControlPopupMenuHandle(MenuCtrl);
	if (!Menu)
		return;
	
	// Get rid of old contents
	while(CountMenuItems(Menu)) DeleteMenuItem(Menu, 1);
	
	// Add in new contents
	for (std::vector<std::string>::const_iterator it = labels.begin (); it != labels.end (); ++it) {
		CFStringRef cfstring = CFStringCreateWithCString(NULL, (*it).c_str (), kCFStringEncodingMacRoman);
		AppendMenuItemTextWithCFString(Menu, cfstring, 0, 0, NULL);
		CFRelease(cfstring);
	}
	
	SetControl32BitMaximum(MenuCtrl,CountMenuItems(Menu));
}

void QQ_set_selector_control_labels_from_stringset (DialogPTR dlg, int item, int stringset_id)
{
	QQ_set_selector_control_labels (dlg, item, build_stringvector_from_stringset (stringset_id));
}

const std::string QQ_copy_string_from_text_control (DialogPTR dlg, int item)
{
	ControlRef control = get_control_from_window (dlg, item);

	if (!control)
		return std::string();
	
	ControlKind kind;
	GetControlKind (control, &kind);
	
	ControlPartCode part;
	ResType tagName;
	
	if (kind.kind == kControlKindEditText)
	{
		part = kControlEditTextPart;
		tagName = kControlEditTextTextTag;
	}
	else if (kind.kind == kControlKindStaticText)
	{
		part = kControlLabelPart;
		tagName = kControlStaticTextTextTag;
	}
	else
	{
		return std::string();
	}

	Size size = 0;
	GetControlDataSize(control, part, tagName, &size);

	if (size == 0)
		return std::string();

	std::vector<char> buffer(size);
	GetControlData(control, part, tagName, buffer.size(), &buffer[0], NULL);
	return std::string(&buffer[0], buffer.size());
}

void QQ_copy_string_to_text_control (DialogPTR dlg, int item, const std::string &s)
{
	ControlRef control = get_control_from_window (dlg, item);
	
	if (!control)
		return;
	
	ControlKind kind;
	GetControlKind (control, &kind);
	
	if (kind.kind == kControlKindEditText) {
		SetControlData(control, kControlEditTextPart, kControlEditTextTextTag, s.length (), s.c_str ());
	}
	
	if (kind.kind == kControlKindStaticText) {
		SetControlData(control, kControlLabelPart, kControlStaticTextTextTag, s.length (), s.c_str ());
	}
	
	Draw1Control (control);
}

extern void QQ_set_control_activity (DialogPTR dlg, int item, bool active)
{
	if (active)
		ActivateControl (get_control_from_window (dlg, item));
	else
		DeactivateControl (get_control_from_window (dlg, item));
}

extern int32 QQ_extract_number_from_text_control (DialogPTR dlg, int item)
{
	long result;

	copy_string_to_pstring(QQ_copy_string_from_text_control(dlg, item), ptemporary);
	StringToNum(ptemporary, &result);
	return result;
}

extern void QQ_insert_number_into_text_control (DialogPTR dlg, int item, int32 number)
{
	NumToString(number, ptemporary);
	QQ_copy_string_to_text_control(dlg, item, pstring_to_string(ptemporary));
}
