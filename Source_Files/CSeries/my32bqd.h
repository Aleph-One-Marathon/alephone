// LP: not sure who originally wrote these cseries files: Bo Lindbergh?
#ifndef _MY_32BIT_QUICKDRAW_
#define _MY_32BIT_QUICKDRAW_

#include <QDOffscreen.h>


extern void initialize_my_32bqd(void);

extern void myGetGWorld(
	GWorldPtr *gw,
	GDHandle *dev);
extern void mySetGWorld(
	GWorldPtr gw,
	GDHandle dev);
extern OSErr myNewGWorld(
	GWorldPtr *gw,
	short depth,
	Rect *bounds,
	CTabHandle clut,
	GDHandle dev,
	unsigned long flags);
extern OSErr myUpdateGWorld(
	GWorldPtr *gw,
	short depth,
	Rect *bounds,
	CTabHandle clut,
	GDHandle dev,
	unsigned long flags);
extern void myDisposeGWorld(
	GWorldPtr gw);
extern Boolean myLockPixels(
	GWorldPtr gw);
extern void myUnlockPixels(
	GWorldPtr gw);
extern PixMapHandle myGetGWorldPixMap(
	GWorldPtr gw);
extern Ptr myGetPixBaseAddr(
	GWorldPtr gw);

extern void myHideMenuBar(
	GDHandle dev);
extern void myShowMenuBar(void);

extern void LowLevelSetEntries(
	short start,
	short count0,
	ColorSpec *specs);

#endif
