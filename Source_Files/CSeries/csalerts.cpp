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

// IR addition:
#ifdef INIO
#include <time.h>
#endif

#include "csalerts.h"
#include "csstrings.h"

#ifdef INIO
#if __MWERKS__
#include <MetroNubUtils.h> // located in {Compiler}:(Deubgger Extras)
#endif
#endif

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

#ifdef INIO
void pause_debug(void)
#else
void pause(void)
#endif
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

// IR addition: copied this preprocessor junk from Debugging.h
#define LocalLMGetMacJmp() (*(( unsigned long *)0x0120))
#define LocalLMGetMacJmpFlag() (*(( UInt8 *)0x0BFF))

#define ISLOWLEVELDEBUGGERCALLABLE()                                    \
    ( ( LocalLMGetMacJmpFlag() != (UInt8) 0xff ) &&                     \
      ( (LocalLMGetMacJmpFlag() & (UInt8) 0xe0) == (UInt8) 0x60 ) &&    \
      ( LocalLMGetMacJmp() != 0 ) &&                                    \
      ( LocalLMGetMacJmp() != (unsigned long) 0xffffffff ) )

void _alephone_assert(
	char *file,
	long line,
	char *what)
{
#ifdef INIO
	#if __MWERKS__
		if (AmIBeingMWDebugged()) {
			SysBreakStr(psprintf((unsigned char*)assert_text,"assert failed: %s",what));
		}
	#endif
	// IR change: made this a little more informative
	DebugLog(csprintf(assert_text,"assert: %s:%ld: %s",file,line,what));
	vhalt(csprintf(assert_text,"%s:%ld: %s  This has been logged.",file,line,what));
#else
     vhalt(csprintf(assert_text,"%s:%ld: %s",file,line,what));
#endif
}

void _alephone_warn(
	char *file,
	long line,
	char *what)
{
	vpause(csprintf(assert_text,"%s:%ld: %s",file,line,what));
}

#ifdef INIO
// IR added:
void DebugLog(const char* text) {
// IR changed: changed it to use fdprintf.
//	static FILE* ofile = NULL;
//	if (!ofile) {
//		ofile = fopen("aleph.log", "a");
//		if (!ofile) return;
//	}
	char time[100];
	time_t now = ::time(NULL);
	strftime(time, 100, "%b %d %X: ", localtime(&now));
	fdprintf("%s%s", time, text);
//	fflush(ofile);
}
#endif