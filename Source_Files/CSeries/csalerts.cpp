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

Jan 25, 2002 (Br'fin (Jeremy Parsons)):
	Added TARGET_API_MAC_CARBON for Carbon.h
*/
// LP: not sure who originally wrote these cseries files: Bo Lindbergh?
#include <stdlib.h>
#include <string.h>

#if defined(TARGET_API_MAC_CARBON)
    #include <Carbon/Carbon.h>
#else
#include <Dialogs.h>
#include <TextUtils.h>
#endif

#include "csalerts.h"
#include "csstrings.h"

static Str255 alert_text;

void alert_user(
	short severity,
	short resid,
	short item,
	OSErr error)
{
	getpstr(alert_text,resid,item);
	ParamText(alert_text,NULL,"\p","\p");
	NumToString(error,alert_text);
	ParamText(NULL,alert_text,NULL,NULL);
	InitCursor();
	switch (severity) {
	case infoError:
		Alert(129,NULL);
		break;
	case fatalError:
	default:
		Alert(128,NULL);
		exit(1);
	}
}

void pause(void)
{
	Debugger();
}

void vpause(
	char *message)
{
	long len;

	len=strlen(message);
	if (len>255)
		len=255;
	alert_text[0]=len;
	memcpy(alert_text+1,message,len);
	ParamText(alert_text,"\p","\p","\p");
	InitCursor();
	Alert(129,NULL);
}

void halt(void)
{
	Debugger();
	exit(1);
}

void vhalt(
	char *message)
{
	long len;

	len=strlen(message);
	if (len>255)
		len=255;
	alert_text[0]=len;
	memcpy(alert_text+1,message,len);
	ParamText(alert_text,"\p","\p","\p");
	InitCursor();
	Alert(128,NULL);
	exit(1);
}

static char assert_text[256];

void _alephone_assert(
	char *file,
	long line,
	char *what)
{
	vhalt(csprintf(assert_text,"%s:%ld: %s",file,line,what));
}

void _alephone_warn(
	char *file,
	long line,
	char *what)
{
	vpause(csprintf(assert_text,"%s:%ld: %s",file,line,what));
}

