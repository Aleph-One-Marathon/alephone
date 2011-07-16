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
*/

#include <stddef.h>

#if defined(EXPLICIT_CARBON_HEADER)
    #include <Carbon/Carbon.h>
/*
#else
#include <Quickdraw.h>
#include <Memory.h>
*/
#endif

#include "cscluts.h"
#include "FileHandler.h"

RGBColor rgb_black={0x0000,0x0000,0x0000};
RGBColor rgb_white={0xFFFF,0xFFFF,0xFFFF};

RGBColor system_colors[NUM_SYSTEM_COLORS] =
{
	{0x2666,0x2666,0x2666},
	{0xD999,0xD999,0xD999}
};

CTabHandle build_macintosh_color_table(
	color_table *table)
{
	CTabHandle clut;
	int i,n;
	rgb_color *src;
	ColorSpec *dst;

	n=table->color_count;
	if (n<0) {
		n=0;
	} else if (n>256) {
		n=256;
	}
	clut=(CTabHandle)NewHandleClear(offsetof(ColorTable,ctTable)+n*sizeof (ColorSpec));
	if (clut) {
		(*clut)->ctSeed=GetCTSeed();
		(*clut)->ctSize=n-1;

		src=table->colors;
		dst=(*clut)->ctTable;
		for (i=0; i<n; i++) {
			dst->value=i;
			dst->rgb.red=src->red;
			dst->rgb.green=src->green;
			dst->rgb.blue=src->blue;
			src++;
			dst++;
		}
	}
	return clut;
}

void build_color_table(
	color_table *table,
	LoadedResource& clut_rsrc)
{
	int i,n;
	ColorSpec *src;
	rgb_color *dst;
	
	CTabHandle clut = CTabHandle(clut_rsrc.GetHandle());

	n=(*clut)->ctSize+1;
	if (n<0) {
		n=0;
	} else if (n>256) {
		n=256;
	}
	table->color_count=n;
	src=(*clut)->ctTable;
	dst=table->colors;
	for (i=0; i<n; i++) {
		dst->red=src->rgb.red;
		dst->green=src->rgb.green;
		dst->blue=src->rgb.blue;
		src++;
		dst++;
	}
}
