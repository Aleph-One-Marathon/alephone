#include <Dialogs.h>
#include <TextUtils.h>
//#include <ControlDefinitions.h>

#include "cstypes.h"
#include "csdialogs.h"

extern void update_any_window(
	WindowPtr window,
	EventRecord *event);
extern void activate_any_window(
	WindowPtr window,
	EventRecord *event,
	boolean active);

static Boolean cursor_tracking=true;
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
		SetWRefCon(dlg,refcon);
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

	good=NewRgn();
	bad=NewRgn();
	outer=NewRgn();
	inner=NewRgn();
	n=CountDITL(dlg);
	for (i=1; i<=n; i++) {
		GetDialogItem(dlg,i,&it,&ih,&ir);
		if ((it&~kItemDisableBit)==kUserDialogItem && !ih) {
		/*
		 * A user item with no drawing proc installed is assumed to be a group box.
		 * Make a 1-pixel-thick frame from its rect and add it to the region to paint.
		 */
			RectRgn(outer,&ir);
			CopyRgn(outer,inner);
			InsetRgn(inner,1,1);
			DiffRgn(outer,inner,outer);
			UnionRgn(good,outer,good);
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
	RGBForeColor(&third);
	PaintRgn(good);
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
		SetPort(win);
		if (GetWindowKind(win)==kDialogWindowKind) {
			if (header_proc) {
				frame=dlg->portRect;
				(*header_proc)(win,&frame);
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

#if GENERATINGCFM
static RoutineDescriptor general_filter_desc =
	BUILD_ROUTINE_DESCRIPTOR(uppModalFilterProcInfo,general_filter_proc);
#define general_filter_upp (&general_filter_desc)
#else
#define general_filter_upp general_filter_proc
#endif

ModalFilterUPP get_general_filter_upp(void)
{
	return general_filter_upp;
}

void set_dialog_cursor_tracking(
	Boolean tracking)
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

Boolean hit_dialog_button(
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
	if ((*button)->contrlHilite!=kControlNoPart)
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
	if (hilite!=NONE)
		HiliteControl(control,hilite);
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
	SetPort(win);
	pr=win->portRect;
	LocalToGlobal((Point *)&pr.top);
	LocalToGlobal((Point *)&pr.bottom);
	SetPort(saveport);
	*frame=pr;
}

