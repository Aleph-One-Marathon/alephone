/*

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


Sept-Nov 2001 (Woody Zenfell): approximate emulations of originally Mac OS-only routines for
    the SDL dialog system, lets us share more code.  Using API that's a bit more specific, so
    we can split what were originally single functions into several related ones.  Mac OS
    implementation of these "split" functions is still handled by the original function.

Feb 27, 2002 (Br'fin (Jeremy Parsons)):
	Added utility routine GetListBoxListHandle for Carbon
*/

#ifndef _CSERIES_DIALOGS_
#define _CSERIES_DIALOGS_

#define iOK					1
#define iCANCEL				2

#ifdef SDL_RFORK_HACK
//AS thinks this is a great hack
#define dialog CHEESEOFDEATH
#define DialogPtr mDialogPtr
#include <Dialogs.h>
#undef dialog
#undef DialogPtr
#endif 

#if defined(mac)

#define CONTROL_INACTIVE	kControlInactivePart
#define CONTROL_ACTIVE		kControlNoPart
#else //!mac

class dialog;

typedef	dialog*	DialogPtr;

#define	CONTROL_INACTIVE	0
#define	CONTROL_ACTIVE		1

#endif //!mac


// (ZZZ:) Prototypes for both platforms.  I think I wrote some of these (on both platforms)
// so we could use a common API for common operations.  Others I think already existed with
// Mac OS implementations, so I just wrote SDL implementations.
extern long extract_number_from_text_item(
	DialogPtr dlg,
	short item);

extern void insert_number_into_text_item(
	DialogPtr dlg,
	short item,
	long number);

extern void copy_pstring_from_text_field(
        DialogPtr dialog,
        short item,
        unsigned char* pstring);

extern void copy_pstring_to_text_field(
        DialogPtr dialog,
        short item,
        const unsigned char* pstring);

extern void copy_pstring_to_static_text(DialogPtr dialog, short item, const unsigned char* pstring);



// (ZZZ:) For Macs only (some more non-Mac stuff later, read on)
#ifdef mac
#define SCROLLBAR_WIDTH	16

enum {
	centerRect
};

extern void AdjustRect(
	Rect const *frame,
	Rect const *in,
	Rect *out,
	short how);

extern void get_window_frame(
	WindowPtr win,
	Rect *frame);

extern DialogPtr myGetNewDialog(
	short id,
	void *storage,
	WindowPtr before,
	long refcon);

extern pascal Boolean general_filter_proc(
	DialogPtr dlg,
	EventRecord *event,
	short *hit);
extern ModalFilterUPP get_general_filter_upp(void);

extern void set_dialog_cursor_tracking(
	bool tracking);

extern bool hit_dialog_button(
	DialogPtr dlg,
	short item);

extern short get_dialog_control_value(
        DialogPtr dialog,
        short which_control);

extern void modify_control(
	DialogPtr dlg,
	short item,
	short hilite,
	short value);
        
extern void modify_radio_button_family(
	DialogPtr dlg,
	short firstItem,
	short lastItem,
	short activeItem);

typedef void (*dialog_header_proc_ptr)(
	DialogPtr dialog,
	Rect *frame);

extern void set_dialog_header_proc(
	dialog_header_proc_ptr proc);

// For Carbon-events dialogs
#ifdef USES_NIBS

// Auto-allocated window loaded from a nib;
// it will automatically deallocate the window when it goes out of scope

class AutoNibWindow
{
	WindowRef w;
public:

	AutoNibWindow(IBNibRef Nib, CFStringRef Name);
	~AutoNibWindow();

	WindowRef operator()() {return w;}
};

// Gets a control reference from:
//   Window reference
//   Signature (check the Interface Builder)
//   Control ID (check the Interface Builder)
ControlRef GetCtrlFromWindow(
	WindowRef DlgWindow,
	uint32 Signature,
	uint32 ID
	);

// Sets whether a control is active/enabled or inactive/disabled
void SetControlActivity(ControlRef Ctrl, bool Activity);

// All these are for getting and setting the text in static and editable text fields
// Note that the getters all have some maximum length:
//   the maximum number of characters that may be read in

void GetStaticPascalText(ControlRef Ctrl, Str255 Text, int MaxLen = 255);
void SetStaticPascalText(ControlRef Ctrl, ConstStr255Param Text);
void GetStaticCText(ControlRef Ctrl, char *Text, int MaxLen = 255);
void SetStaticCText(ControlRef Ctrl, const char *Text);

void GetEditPascalText(ControlRef Ctrl, Str255 Text, int MaxLen = 255);
void SetEditPascalText(ControlRef Ctrl, ConstStr255Param Text);
void GetEditCText(ControlRef Ctrl, char *Text, int MaxLen = 255);
void SetEditCText(ControlRef Ctrl, const char *Text);


// For adding drawability to a 
// It cleans up when it goes out of scope
class AutoDrawability
{
	ControlUserPaneDrawUPP DrawingUPP;
public:
	
	AutoDrawability();
	~AutoDrawability();
	
	// Needs the control to be made drawable,
	// the function to be used for drawing (can be shared by several controls),
	// and the drawing data for that control.
	// That function will be called with the control that was hit and its drawing data
	void operator()(ControlRef Ctrl,
		void (*DrawFunction)(ControlRef, void *),
		void *DrawData = NULL);
};


// Draws a simple color swatch; "DrawData" above must be a pointer to a RGBColor object
void SwatchDrawer(ControlRef Ctrl, void *Data);


// Picks a color associated with a control
// Needs:
//   Control reference
//   Pointer to color -- must be used in the control's draw routine for doing live updates
//   Prompt (optional)
// Returns whether or not the color was finally changed.
bool PickControlColor(ControlRef Ctrl,
	RGBColor *ClrPtr,
	ConstStr255Param Prompt = NULL
	);


// For adding hittability to a user-defined control,
// so that a mouse click will register in the way
// that it does on some Apple-defined control (buttons, popups, etc.).
// It cleans up when it goes out of scope
class AutoHittability
{
	ControlUserPaneHitTestUPP HitTesterUPP;
public:
	
	AutoHittability();
	~AutoHittability();
	
	// Needs only the control to be made hittable
	void operator()(ControlRef Ctrl);
};


// Builds a popup menu with the help of:
//   The menu's control ref
//   Menu-builder callback; uses
//     Menu-item number, starting from 1
//     Pascal string, receives name of menu item
//     Boolean, receives whether this item should be initially selected
//     User data
//   Returns boolean:
//     True: use the string and the initial-selection boolean
//     False: ignore both and stop adding menu items
//   Default for initial value is 1
void BuildMenu(
	ControlRef MenuCtrl,
	bool (*BuildMenuItem)(int,uint8 *,bool &,void *),
	void *BuildMenuData = NULL
	);

struct ParsedControl
{
	ControlRef Ctrl;
	ControlID ID;
};

// Runs a modal dialog; needs:
//   Dialog window
//   Whether the dialog will be a sheet if sheets are available
//   Dialog handler; uses
//     ID of the control hit
//     User data
//   Dialog user data; passed to the dialog handler
// There are always two special dialog numbers: iOK (1) and iCANCEL (2)
// Will return 'true' if iOK was pressed and 'false' otherwise
bool RunModalDialog(
	WindowRef DlgWindow,
	bool IsSheet,
	void (*DlgHandler)(ParsedControl &,void *) = NULL,
	void *DlgData = NULL
	);


// Stops a running modal dialog; needs:
//   Dialog window
//   Whether the dialog will be a sheet if sheets are available
void StopModalDialog(
	WindowRef DlgWindow,
	bool IsSheet
	);


// Adds a timer to the current event loop;
// it cleans up when it goes out of scope

class AutoTimer
{
	EventLoopTimerUPP HandlerUPP;
	EventLoopTimerRef Timer;
	
public:
	AutoTimer(
		EventTimerInterval Delay,		// Before the timer starts
		EventTimerInterval Interval,	// How often it fires (0 is once-off)
		EventLoopTimerProcPtr Handler,
		void *HandlerData = NULL
		);
	~AutoTimer();
	
	EventLoopTimerRef operator() () {return Timer;}
};


// Adds a keyboard watcher to a control; useful for catching keystrokes
// It cleans up when it goes out of scope
class AutoKeyboardWatcher
{
	EventHandlerUPP KeyboardHandlerUPP;
	
public:

	AutoKeyboardWatcher(
		EventHandlerProcPtr Handler	// Called for every keystroke
		);
	~AutoKeyboardWatcher();
	
	void Watch(
		ControlRef Ctrl,			// Control to watch keystrokes at
		void *HandlerData = NULL
		);
};


// Convert between control values and floats from 0 to 1.
// Should be especially useful for sliders.
float GetCtrlFloatValue(ControlRef Ctrl);
void SetCtrlFloatValue(ControlRef Ctrl, float Value);

#endif

#endif//mac



// (ZZZ:) Now, some functions I "specialized" for SDL are simply forwarded to the original, more
// general versions on classic Mac...
// These more specific names show better what manipulation is desired.  Also, more importantly,
// they let us patch up some differences between the way the Mac OS handles things and the way
// Christian's dialog code handles things (e.g., a Mac OS selection control (popup) is numbered
// from 1, whereas a boolean control (checkbox) is numbered 0 or 1.  In Christian's code, all
// selection controls (w_select and subclasses, including the boolean control w_toggle) are
// indexed from 0).
// These functions use the Mac OS numbering scheme.
#ifdef mac
__inline__ void modify_selection_control(
	DialogPtr dlg,
	short item,
	short hilite,
	short value) {modify_control(dlg, item, hilite, value); }

__inline__ void modify_control_enabled(
	DialogPtr dlg,
	short item,
	short hilite) {modify_control(dlg, item, hilite, NONE); }

__inline__ void modify_boolean_control(
	DialogPtr dlg,
	short item,
	short hilite,
	short value) {modify_control(dlg, item, hilite, value); }
        
__inline__ short get_selection_control_value(
        DialogPtr dialog,
        short which_control) {return get_dialog_control_value(dialog, which_control); }
        
__inline__ bool get_boolean_control_value(
        DialogPtr dialog,
        short which_control) {return get_dialog_control_value(dialog, which_control); }

#else//!mac

// (ZZZ: Here are the prototypes for the specialized SDL versions.)
extern void modify_selection_control(
	DialogPtr dlg,
	short item,
	short hilite,
	short value);

extern void modify_control_enabled(
	DialogPtr dlg,
	short item,
	short hilite);

extern void modify_boolean_control(
	DialogPtr dlg,
	short item,
	short hilite,
	short value);
        
extern short get_selection_control_value(
        DialogPtr dialog,
        short which_control);
        
extern bool get_boolean_control_value(
        DialogPtr dialog,
        short which_control);
#endif//!mac



// ZZZ: (very) approximate SDL emulation of Mac OS Toolbox routines - see note in implementation
#ifndef mac
void HideDialogItem(DialogPtr dialog, short item_index);
void ShowDialogItem(DialogPtr dialog, short item_index);
#endif // !mac

#if defined(TARGET_API_MAC_CARBON)
// JTP: Get the list manager handle for a listbox control
pascal OSStatus GetListBoxListHandle( ControlHandle control, ListHandle* list );
#endif

#endif//_CSERIES_DIALOGS_
