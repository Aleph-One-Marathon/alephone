// LP: not sure who originally wrote these cseries files: Bo Lindbergh?
#ifndef _CSERIES_DIALOGS_
#define _CSERIES_DIALOGS_

#define iOK					1
#define iCANCEL				2

#define CONTROL_INACTIVE	kControlInactivePart
#define CONTROL_ACTIVE		kControlNoPart

#define SCROLLBAR_WIDTH	16

#define RECTANGLE_WIDTH(rectptr) ((rectptr)->right-(rectptr)->left)
#define RECTANGLE_HEIGHT(rectptr) ((rectptr)->bottom-(rectptr)->top)

enum {
	centerRect
};

extern void AdjustRect(
	Rect const *frame,
	Rect const *in,
	Rect *out,
	short how);

#ifdef mac
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
	Boolean tracking);

extern long extract_number_from_text_item(
	DialogPtr dlg,
	short item);

extern void insert_number_into_text_item(
	DialogPtr dlg,
	short item,
	long number);

extern Boolean hit_dialog_button(
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

#endif
