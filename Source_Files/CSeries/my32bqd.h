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

	This license is contained in the file "COPYING",
	which is included with this source code; it is available online at
	http://www.gnu.org/licenses/gpl.html

*/
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
extern bool myLockPixels(
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
