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

	This license is contained in the file "GNU_GeneralPublicLicense.txt",
	which is included with this source code; it is available online at
	http://www.gnu.org/licenses/gpl.html

*/
// LP: not sure who originally wrote these cseries files: Bo Lindbergh?
#ifndef _CSERIES_DIALOGS_
#define _CSERIES_DIALOGS_

#define iOK					1
#define iCANCEL				2

#define CONTROL_INACTIVE	kControlInactivePart
#define CONTROL_ACTIVE		kControlNoPart

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

extern long extract_number_from_text_item(
	DialogPtr dlg,
	short item);

extern void insert_number_into_text_item(
	DialogPtr dlg,
	short item,
	long number);

extern bool hit_dialog_button(
	DialogPtr dlg,
	short item);

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

#endif
