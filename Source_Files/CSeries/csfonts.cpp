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
	Added accessors for datafields now opaque in Carbon
*/

#if defined(EXPLICIT_CARBON_HEADER)
    #include <Carbon/Carbon.h>
/*
#else
#include <Resources.h>
#include <Quickdraw.h>
*/
#endif

#include "csfonts.h"

static TextSpec null_text_spec;

void GetNewTextSpec(
	TextSpec *spec,
	short resid,
	short item)
{
	Handle res;
	int cnt;
	TextSpec *src;

	res=GetResource('finf',resid);
	if (!res)
		goto notfound;
	if (!*res)
		LoadResource(res);
	if (!*res)
		goto notfound;
	cnt=*(short *)*res;
	if (item<0 || item>=cnt)
		goto notfound;
	src=(TextSpec *)(*res+sizeof (short));
	*spec=src[item];
	return;
notfound:
	*spec=null_text_spec;
}

void GetFont(
	TextSpec *spec)
{
	GrafPtr port;

	GetPort(&port);
	spec->font=GetPortTextFont(port);
	spec->style=GetPortTextFace(port);
	spec->size=GetPortTextSize(port);
}

void SetFont(
	TextSpec *spec)
{
	TextFont(spec->font);
	TextFace(spec->style);
	TextSize(spec->size);
}
