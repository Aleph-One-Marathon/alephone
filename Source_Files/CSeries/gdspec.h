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

*/


#ifndef _CSERIES_GDSPEC_
#define _CSERIES_GDSPEC_

typedef struct GDSpec {
	short slot;
	short flags;
	short bit_depth;
	short width;
	short height;
} GDSpec,*GDSpecPtr;

extern GDHandle BestDevice(
	GDSpecPtr spec);
extern GDHandle MatchGDSpec(
	GDSpecPtr spec);
extern void SetDepthGDSpec(
	GDSpecPtr spec);
extern void BuildGDSpec(
	GDSpecPtr spec,
	GDHandle dev);
extern bool HasDepthGDSpec(
	GDSpecPtr spec);
extern bool EqualGDSpec(
	GDSpecPtr spec1,
	GDSpecPtr spec2);

extern short GetSlotFromGDevice(
	GDHandle dev);

extern void display_device_dialog(
	GDSpecPtr spec);

#endif
