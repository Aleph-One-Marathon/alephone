/*

	Copyright (C) 2005 and beyond by Loren Petrich
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

 */

#ifndef NIBSUIHELPERS_H
#define NIBSUIHELPERS_H

#include <memory>
#include <string>
#include <vector>

#include <boost/function.hpp>

void initialize_MLTE();

class SelfReleasingCFStringRef
{
public:
	SelfReleasingCFStringRef(CFStringRef newlyCreatedStringRef)
		: m_stringRef(newlyCreatedStringRef)
	{}

	CFStringRef StringRef() const {
		return m_stringRef;
	}

	~SelfReleasingCFStringRef()
	{
		CFRelease(m_stringRef);
	}

private:
	CFStringRef m_stringRef;
	
	// No reason these couldn't be implemented, but they're not yet.
	SelfReleasingCFStringRef(const SelfReleasingCFStringRef&);
	SelfReleasingCFStringRef& operator =(const SelfReleasingCFStringRef&);
};



std::auto_ptr<SelfReleasingCFStringRef> StringToCFString(const std::string& s);



// Automatically-Disposed NIB reference.  Not using operator() to get the reference
// (as the other Auto* do) because I find that practice somewhat nauseating.
class AutoNibReference
{
public:
	AutoNibReference(CFStringRef nibName)
{
		OSStatus result = CreateNibReference(nibName, &m_nibReference);
		// Best error-handling strategy evar . . . but when in Rome?
		assert(result == noErr);
}

~AutoNibReference() { DisposeNibReference(m_nibReference); }

const IBNibRef nibReference() const { return m_nibReference; }

private:
IBNibRef m_nibReference;
};


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

void SetGlobalControlSignature(OSType sig);


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


// A replacement for RunModalDialog/StopModalDialog.
// No handler installed here - controls which require them should already have them installed.
// No special casing for the "OK" and "Cancel" buttons.
class Modal_Dialog
{
public:
	Modal_Dialog (WindowRef dialogWindow, bool isSheet);
	
	bool Run ();
	void Stop (bool result); // Whatever we pass in here is returned by Run

private:
	WindowRef m_dialogWindow;
	bool m_isSheet;
	bool m_result;
};

// Adds a timer to the current event loop;
// it cleans up when it goes out of scope

typedef boost::function<void (void)> TimerCallback;
class AutoTimer
{
	EventLoopTimerUPP HandlerUPP;
	EventLoopTimerRef Timer;

public:
	AutoTimer(
		EventTimerInterval Delay,		// Before the timer starts
		EventTimerInterval Interval,		// How often it fires (0 is once-off)
		EventLoopTimerProcPtr Handler,
		void *HandlerData = NULL
	);
	
	AutoTimer(
		EventTimerInterval Delay,		// Before the timer starts
		EventTimerInterval Interval,		// How often it fires (0 is once-off)
		TimerCallback Handler
	);
	
	~AutoTimer();

	EventLoopTimerRef operator() () {return Timer;}

private:
	static void bounce_boosted_callback (EventLoopTimerRef ignored, void* me)
		{ reinterpret_cast<AutoTimer*>(me)->boosted_callback (); }
	
	void boosted_callback () { m_callback (); }
	
	TimerCallback m_callback;
};


class AutoWatcher
{
protected:
	AutoWatcher (ControlRef ctrl, int num_event_types, const EventTypeSpec* event_types);
	virtual ~AutoWatcher () { DisposeEventHandlerUPP (m_EventHandlerUPP); }
	
	virtual OSStatus act (EventHandlerCallRef inCallRef, EventRef inEvent) = 0;

private:
	static pascal OSStatus callback (EventHandlerCallRef inCallRef,
					EventRef inEvent, void* inUserData);

	EventHandlerUPP m_EventHandlerUPP;
};

typedef boost::function<void (void)> ControlHitCallback;
class AutoControlWatcher : public AutoWatcher
{
public:
	AutoControlWatcher (ControlRef ctrl)
		: AutoWatcher (ctrl, 1, ControlWatcherEvents)
		, m_callback (NULL)
		{}

	void set_callback (ControlHitCallback callback)
		{ m_callback = callback; }

protected:
	static const EventTypeSpec ControlWatcherEvents[1];
	// = {{kEventClassControl, kEventControlHit}};

	virtual OSStatus act (EventHandlerCallRef inCallRef, EventRef inEvent)
		{ if (m_callback) m_callback (); return noErr; }

private:
	ControlHitCallback m_callback;
};

// Doesn't identify or respond to modifier keys (currently)
typedef boost::function<void (char)> GotCharacterCallback;
class AutoKeystrokeWatcher : public AutoWatcher
{
public:
	AutoKeystrokeWatcher (ControlRef ctrl)
		: AutoWatcher (ctrl, 2, KeystrokeWatcherEvents)
		, m_callback (NULL)
		{}

	void set_callback (GotCharacterCallback callback)
		{ m_callback = callback; }

protected:
	static const EventTypeSpec KeystrokeWatcherEvents[2];
	// = {{kEventClassKeyboard, kEventRawKeyDown},{kEventClassKeyboard, kEventRawKeyRepeat}};

	virtual OSStatus act (EventHandlerCallRef inCallRef, EventRef inEvent);

private:
	GotCharacterCallback m_callback;
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


// Watch for clicks on a tab control, and make the associated pane visible
class AutoTabHandler : public AutoWatcher
{
public:

	AutoTabHandler (ControlRef in_tab, std::vector<ControlRef> in_panes, WindowRef in_window)
		: AutoWatcher (in_tab, 1, TabControlEvents)
		, tab (in_tab)
		, panes (in_panes)
		, window (in_window)
		{ SetActiveTab (0); }

private:

	static const EventTypeSpec TabControlEvents[1];
	// = {{kEventClassControl, kEventControlHit}};

	virtual OSStatus act (EventHandlerCallRef inCallRef, EventRef inEvent);
	void SetActiveTab (int new_value);
	
	ControlRef tab;
	std::vector<ControlRef> panes;
	int old_value;
	WindowRef window;
};


// Convert between control values and floats from 0 to 1.
// Should be especially useful for sliders.
float GetCtrlFloatValue(ControlRef Ctrl);
void SetCtrlFloatValue(ControlRef Ctrl, float Value);

#endif // NIBSUIHELPERS_H
