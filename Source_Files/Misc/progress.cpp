/*
	progress.c

	Copyright (C) 1991-2001 and beyond by Bungie Studios, Inc.
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

	Saturday, October 28, 1995 10:56:17 PM- rdm created.

Jan 25, 2002 (Br'fin (Jeremy Parsons)):
	Replaced proc pointers with UPP for Carbon
	Added accessors for datafields now opaque in Carbon

Feb 27, 2002 (Br'fin (Jeremy Parsons)):
	Reimplemented for Carbon to use a fancy progress bar control
	Fixed that it wouldn't actually update its window under carbon
*/

#include "macintosh_cseries.h"
#include "progress.h"
#include "shell.h"

#ifdef USES_NIBS
	#include "NibsUiHelpers.h"
#endif

enum {
	dialogPROGRESS= 10002,
	iPROGRESS_BAR= 1,
	iPROGRESS_MESSAGE
};

#ifdef USES_NIBS

const CFStringRef Window_Progress = CFSTR("Progress");

// When not in use, this variable will have value NULL
WindowRef ProgressWindow = NULL;


static void Update()
{
	if (QDIsPortBuffered(GetWindowPort(ProgressWindow)))
		QDFlushPortBuffer(GetWindowPort(ProgressWindow), NULL);
}


void open_progress_dialog(size_t message_id, bool)
{
	OSStatus err;
	
	err = CreateWindowFromNib(GUI_Nib,Window_Progress,&ProgressWindow);
	
	vassert(err == noErr,
		csprintf(temporary,"CreateWindowFromNib error: %d for the progress window",err));
	
	reset_progress_bar();

	if (message_id == _loading)
		SetWindowTitleWithCFString(ProgressWindow, CFSTR("Status")); // not "Network Status"
	set_progress_dialog_message(message_id);
	
	ShowWindow(ProgressWindow);
}

void close_progress_dialog(void)
{
	assert(ProgressWindow);
	
	HideWindow(ProgressWindow);
	DisposeWindow(ProgressWindow);
	
	ProgressWindow = NULL;
}

void set_progress_dialog_message(size_t message_id)
{
	assert(ProgressWindow);
	
	getpstr(ptemporary, strPROGRESS_MESSAGES, message_id);
	
	ControlRef Ctrl = GetCtrlFromWindow(ProgressWindow, 0, iPROGRESS_MESSAGE);
	SetStaticPascalText(Ctrl, ptemporary);
	
	Draw1Control(Ctrl);	// Updates the control's text display
	Update();
}

void draw_progress_bar(size_t sent, size_t total)
{
	assert(ProgressWindow);
	
	ControlRef Ctrl = GetCtrlFromWindow(ProgressWindow, 0, iPROGRESS_BAR);
	
	int CtrlMin = GetControl32BitMinimum(Ctrl);
	int CtrlMax = GetControl32BitMaximum(Ctrl);
	
	int ProportionSent = CtrlMin + ((sent*(CtrlMax-CtrlMin))/total);
	
	SetControl32BitValue(Ctrl, ProportionSent);

	Update();
}

void reset_progress_bar(void)
{
	assert(ProgressWindow);
	
	ControlRef Ctrl = GetCtrlFromWindow(ProgressWindow, 0, iPROGRESS_BAR);
	
	Update();
}


#else

/* ------- structures */
struct progress_data {
	DialogPtr dialog;
	GrafPtr old_port;
	UserItemUPP progress_bar_upp;
#if defined(TARGET_API_MAC_CARBON)
	ControlRef control;
#endif
};

/* ------ private prototypes */
#if !defined(TARGET_API_MAC_CARBON)
static pascal void draw_distribute_progress(DialogPtr dialog, short item_num);
#endif

/* ------ globals */
struct progress_data progress_data;

/* ------ calls */
void open_progress_dialog(
	size_t message_id)
{
	Rect item_box;
	short item_type;
	Handle item_handle;
		
	progress_data.dialog= GetNewDialog(dialogPROGRESS, NULL, (WindowPtr) -1);
	assert(progress_data.dialog);
	
#if defined(TARGET_API_MAC_CARBON)
// Carbon code, let's use a progress control!
	GetDialogItem(progress_data.dialog, iPROGRESS_BAR, &item_type, &item_handle, &item_box);
	CreateProgressBarControl(GetDialogWindow(progress_data.dialog), &item_box,
		0, 0, RECTANGLE_WIDTH(&item_box), // Values: current, min, max
		false, &progress_data.control);
	assert(progress_data.control);
#else
// Old code, draws a pretty plain meter
	progress_data.progress_bar_upp= NewUserItemUPP(draw_distribute_progress);
	assert(progress_data.progress_bar_upp);

	GetPort(&progress_data.old_port);
	SetPort(GetWindowPort(GetDialogWindow(progress_data.dialog)));

	GetDialogItem(progress_data.dialog, iPROGRESS_BAR, &item_type, &item_handle, &item_box);
	SetDialogItem(progress_data.dialog, iPROGRESS_BAR, item_type, (Handle) progress_data.progress_bar_upp, &item_box);
#endif
	/* Set the message.. */
	set_progress_dialog_message(message_id);

	ShowWindow(GetDialogWindow(progress_data.dialog));
	DrawDialog(progress_data.dialog);

#if defined(TARGET_API_MAC_CARBON)
	if (QDIsPortBuffered(GetDialogPort(progress_data.dialog)))
		QDFlushPortBuffer(GetDialogPort(progress_data.dialog), NULL);
	// Busy cursor already provided by OSX
#else
	SetCursor(*(GetCursor(watchCursor)));
#endif
}

void set_progress_dialog_message(
	size_t message_id)
{
	short item_type;
	Rect bounds;
	Handle item_handle;

	assert(progress_data.dialog);
	GetDialogItem(progress_data.dialog, iPROGRESS_MESSAGE, &item_type, &item_handle, &bounds);
	getpstr(ptemporary, strPROGRESS_MESSAGES, message_id);
	SetDialogItemText(item_handle, ptemporary);
}

void close_progress_dialog(
	void)
{
	SetPort(progress_data.old_port);

#if !defined(TARGET_API_MAC_CARBON)
	// Unneeded under OSX
	SetCursor(&qd.arrow);
#endif

#if defined(TARGET_API_MAC_CARBON)
	HideWindow(GetDialogWindow(progress_data.dialog));
	DisposeControl(progress_data.control);
	DisposeDialog(progress_data.dialog);
#else
	DisposeDialog(progress_data.dialog);
	DisposeRoutineDescriptor(progress_data.progress_bar_upp);
#endif
}

void draw_progress_bar(
	size_t sent, 
	size_t total)
{
	Rect bounds;
	Handle item;
	short item_type;
	short width;
	
	GetDialogItem(progress_data.dialog, iPROGRESS_BAR, &item_type, &item, &bounds);
	width= (sent*RECTANGLE_WIDTH(&bounds))/total;

#if defined(TARGET_API_MAC_CARBON)
	SetControlValue(progress_data.control, width);
	
	if (QDIsPortBuffered(GetDialogPort(progress_data.dialog)))
		QDFlushPortBuffer(GetDialogPort(progress_data.dialog), NULL);
#else
	bounds.right= bounds.left+width;
	RGBForeColor(system_colors+gray15Percent);
	PaintRect(&bounds);
	ForeColor(blackColor);
#endif
}

void reset_progress_bar(
	void)
{
#if defined(TARGET_API_MAC_CARBON)
	draw_progress_bar(0, 100);
#else
	draw_distribute_progress(progress_data.dialog, iPROGRESS_BAR);
#endif
}

/* --------- private code */
#if !defined(TARGET_API_MAC_CARBON)
static pascal void draw_distribute_progress(
	DialogPtr dialog, 
	short item_num)
{
	Rect item_box;
	short item_type;
	Handle item_handle;
	GrafPtr old_port;
	
	GetPort(&old_port);
	SetPort(GetWindowPort(GetDialogWindow(dialog)));
	
	GetDialogItem(dialog, item_num, &item_type, &item_handle, &item_box);
	PenNormal();
	RGBForeColor(system_colors+windowHighlight);
	PaintRect(&item_box);
	ForeColor(blackColor);
	InsetRect(&item_box, -1, -1);
	FrameRect(&item_box);

	SetPort(old_port);
}
#endif
#endif
