/*

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

Jan 25, 2002 (Br'fin (Jeremy Parsons)):
	Added TARGET_API_MAC_CARBON for Carbon.h
	LowLevelSetEntries simply asserts under Carbon (Device control unavailable)
	Included Steve Bytnar's Carbon menubar handling code
*/

#include "my32bqd.h"

#if defined(EXPLICIT_CARBON_HEADER)
    #include <Carbon/Carbon.h>
/*
#else
#include <LowMem.h>
#include <Video.h>
#include <Devices.h>
*/
#endif

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

#if !defined(TARGET_API_MAC_CARBON)
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
#else
enum eMenuBarState
{
	UNKNOWN,
	HIDDEN,
	SHOWING
};

#include "csalerts.h"
#include "csstrings.h"

static eMenuBarState mbarstate = UNKNOWN;
void myHideMenuBar(
	GDHandle ignoredDev)
{
	(void)ignoredDev;
	switch (mbarstate)
	{
		case UNKNOWN:
		case SHOWING:
			mbarstate = HIDDEN;
			HideMenuBar();
			break;
		case HIDDEN:
			dprintf("HideMenuBar called while menubar was already hidden.");
			break;
	}
}

void myShowMenuBar(void)
{
	switch (mbarstate)
	{
		case UNKNOWN:
		case HIDDEN:
			mbarstate = SHOWING;
			ShowMenuBar();
			break;
		case SHOWING:
			dprintf("ShowMenuBar called while menubar was already showing.");
			break;
	}
}
#endif

void LowLevelSetEntries(
	short start,
	short count0,
	ColorSpec *specs)
{
//#if defined(SUPPRESS_MACOS_CLASSIC)
	assert(0);
/*
#else
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
#endif
*/
}
