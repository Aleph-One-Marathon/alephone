// LP: not sure who originally wrote these cseries files: Bo Lindbergh?
#include "my32bqd.h"

#include <LowMem.h>
#include <Video.h>
#include <Devices.h>

void initialize_my_32bqd(void)
{
}

void myGetGWorld(
	GWorldPtr *gw,
	GDHandle *dev)
{
	GetGWorld(gw,dev);
}

void mySetGWorld(
	GWorldPtr gw,
	GDHandle dev)
{
	SetGWorld(gw,dev);
}

OSErr myNewGWorld(
	GWorldPtr *gw,
	short depth,
	Rect *bounds,
	CTabHandle clut,
	GDHandle dev,
	unsigned long flags)
{
	return NewGWorld(gw,depth,bounds,clut,dev,flags);
}

OSErr myUpdateGWorld(
	GWorldPtr *gw,
	short depth,
	Rect *bounds,
	CTabHandle clut,
	GDHandle dev,
	unsigned long flags)
{
	return UpdateGWorld(gw,depth,bounds,clut,dev,flags);
}

void myDisposeGWorld(
	GWorldPtr gw)
{
	DisposeGWorld(gw);
}

bool myLockPixels(
	GWorldPtr gw)
{
	return LockPixels(GetGWorldPixMap(gw));
}

void myUnlockPixels(
	GWorldPtr gw)
{
	UnlockPixels(GetGWorldPixMap(gw));
}

PixMapHandle myGetGWorldPixMap(
	GWorldPtr gw)
{
	return GetGWorldPixMap(gw);
}

Ptr myGetPixBaseAddr(
	GWorldPtr gw)
{
	return GetPixBaseAddr(GetGWorldPixMap(gw));
}

static short savedmbh;
static RgnHandle savedgray;

void myHideMenuBar(
	GDHandle ignoredDev)
{
	RgnHandle gray,rect;
	GDHandle dev;
	(void)ignoredDev;

	if (savedgray)
		return;
	gray=GetGrayRgn();
	savedgray=NewRgn();
	rect=NewRgn();
	CopyRgn(gray,savedgray);
	for (dev=GetDeviceList(); dev; dev=GetNextDevice(dev)) {
		if (!TestDeviceAttribute(dev,screenDevice)
				|| !TestDeviceAttribute(dev,screenActive))
			continue;
		RectRgn(rect,&(*dev)->gdRect);
		UnionRgn(gray,rect,gray);
	}
	DisposeRgn(rect);
	savedmbh=LMGetMBarHeight();
	LMSetMBarHeight(0);
}

void myShowMenuBar(void)
{
	if (!savedgray)
		return;
	CopyRgn(savedgray,GetGrayRgn());
	LMSetMBarHeight(savedmbh);
	DisposeRgn(savedgray);
	savedgray=NULL;
}

void LowLevelSetEntries(
	short start,
	short count0,
	ColorSpec *specs)
{
	GDHandle dev;
	VDSetEntryRecord se;
	CntrlParam pb;

	se.csTable=specs;
	se.csStart=start;
	se.csCount=count0;
	dev=GetGDevice();
	pb.ioCRefNum=(*dev)->gdRefNum;
	if ((*dev)->gdType==directType) {
		pb.csCode=cscDirectSetEntries;
	} else {
		pb.csCode=cscSetEntries;
	}
	*(VDSetEntryRecord **)pb.csParam=&se;
	PBControlSync((ParmBlkPtr)&pb);
}

