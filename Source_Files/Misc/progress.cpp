/*
	progress.c

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

	Saturday, October 28, 1995 10:56:17 PM- rdm created.

*/

#include "macintosh_cseries.h"
#include "progress.h"

enum {
	dialogPROGRESS= 10002,
	iPROGRESS_BAR= 1,
	iPROGRESS_MESSAGE
};


/* ------- structures */
struct progress_data {
	DialogPtr dialog;
	GrafPtr old_port;
	UserItemUPP progress_bar_upp;
};

/* ------ private prototypes */
static pascal void draw_distribute_progress(DialogPtr dialog, short item_num);

/* ------ globals */
struct progress_data progress_data;

/* ------ calls */
void open_progress_dialog(
	short message_id)
{
	Rect item_box;
	short item_type;
	Handle item_handle;
		
	progress_data.dialog= GetNewDialog(dialogPROGRESS, NULL, (WindowPtr) -1);
	assert(progress_data.dialog);
	progress_data.progress_bar_upp= NewUserItemProc(draw_distribute_progress);
	assert(progress_data.progress_bar_upp);

	GetPort(&progress_data.old_port);
	SetPort(progress_data.dialog);
	GetDialogItem(progress_data.dialog, iPROGRESS_BAR, &item_type, &item_handle, &item_box);
	SetDialogItem(progress_data.dialog, iPROGRESS_BAR, item_type, (Handle) progress_data.progress_bar_upp, &item_box);

	/* Set the message.. */
	set_progress_dialog_message(message_id);

	ShowWindow(progress_data.dialog);
	DrawDialog(progress_data.dialog);
	SetCursor(*(GetCursor(watchCursor)));

	return;
}

void set_progress_dialog_message(
	short message_id)
{
	short item_type;
	Rect bounds;
	Handle item_handle;

	assert(progress_data.dialog);
	GetDialogItem(progress_data.dialog, iPROGRESS_MESSAGE, &item_type, &item_handle, &bounds);
	getpstr(ptemporary, strPROGRESS_MESSAGES, message_id);
	SetDialogItemText(item_handle, ptemporary);
	
	return;
}

void close_progress_dialog(
	void)
{
	SetPort(progress_data.old_port);

	SetCursor(&qd.arrow);
	DisposeDialog(progress_data.dialog);
	DisposeRoutineDescriptor(progress_data.progress_bar_upp);

	return;
}

void draw_progress_bar(
	long sent, 
	long total)
{
	Rect bounds;
	Handle item;
	short item_type;
	short width;
	
	GetDialogItem(progress_data.dialog, iPROGRESS_BAR, &item_type, &item, &bounds);
	width= (sent*RECTANGLE_WIDTH(&bounds))/total;
	
	bounds.right= bounds.left+width;
	RGBForeColor(system_colors+gray15Percent);
	PaintRect(&bounds);
	ForeColor(blackColor);

	return;
}

void reset_progress_bar(
	void)
{
	draw_distribute_progress(progress_data.dialog, iPROGRESS_BAR);

	return;
}

/* --------- private code */
static pascal void draw_distribute_progress(
	DialogPtr dialog, 
	short item_num)
{
	Rect item_box;
	short item_type;
	Handle item_handle;
	GrafPtr old_port;
	
	GetPort(&old_port);
	SetPort(dialog);
	
	GetDialogItem(dialog, item_num, &item_type, &item_handle, &item_box);
	PenNormal();
	RGBForeColor(system_colors+windowHighlight);
	PaintRect(&item_box);
	ForeColor(blackColor);
	InsetRect(&item_box, -1, -1);
	FrameRect(&item_box);

	SetPort(old_port);

	return;
}