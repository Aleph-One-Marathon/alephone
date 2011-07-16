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

April 22, 2003 (Woody Zenfell):
	Now dumping alert text etc. with Logging as well
*/

#include <stdlib.h>
#include <string.h>

#if defined(EXPLICIT_CARBON_HEADER)
	#include <Carbon/Carbon.h>
/*
#else
#include <Dialogs.h>
#include <TextUtils.h>
*/
#endif

#include "csalerts.h"
#include "csstrings.h"

#include "Logging.h"
#include "TextStrings.h"

// Update the compiler to know that ExitToShell doesn't return either
#ifndef __MWERKS__
extern void ExitToShell(void) NORETURN;
#endif

/* ---------- globals */
#ifndef TARGET_API_MAC_CARBON
static Str255 alert_text;
#endif
static char assert_text[256];

/* ---------- code */

void alert_user(char *message, short severity)
{
  InitCursor();
  switch (severity) {
  case infoError:
    SimpleAlert(kAlertNoteAlert, message, NULL);
  case fatalError:
    SimpleAlert(kAlertStopAlert, message, NULL);
  }
}

void alert_user(
	short severity,
	short resid,
	short item,
	OSErr error)
{
#ifdef TARGET_API_MAC_CARBON
	char *Msg = TS_GetCString(resid,item);
	char NumStr[256];
	sprintf(NumStr,"Error ID: %hd",error);
	InitCursor();
	switch(severity)
	{
	case infoError:
		logError2("alert (ID=%hd): %s",error,Msg);
		SimpleAlert(kAlertNoteAlert,Msg,NumStr);
		break;
		
	case fatalError:
		logFatal2("fatal alert (ID=%hd): %s",error,Msg);
		SimpleAlert(kAlertStopAlert,Msg,NumStr);
		ExitToShell();
	}
#else
	getpstr(alert_text,resid,item);
	ParamText(alert_text,NULL,"\p","\p");
	NumToString(error,alert_text);
	ParamText(NULL,alert_text,NULL,NULL);
	InitCursor();
	switch (severity) {
	case infoError:
		logError1("alert: %s",TS_GetCString(resid,item));
		Alert(129,NULL);
		break;
	case fatalError:
	default:
		logFatal1("fatal alert: %s",TS_GetCString(resid,item));
		Alert(128,NULL);
		exit(1);
	}
#endif
}

void vpause(
	char *message)
{
#ifdef TARGET_API_MAC_CARBON
	InitCursor();
	SimpleAlert(kAlertNoteAlert,message);
	logWarning1("vpause: %s", message);
#else
	int32 len=strlen(message);
	if (len>255)
		len=255;
	alert_text[0]=len;
	memcpy(alert_text+1,message,len);
	ParamText(alert_text,"\p","\p","\p");
	InitCursor();
	logWarning1("vpause: %s", message);
	Alert(129,NULL);
#endif
}

void halt(void)
{
	logFatal("halt called");
	Debugger();
	ExitToShell();
}

extern void stop_recording();

void vhalt(
	char *message)
{
	stop_recording();
#ifdef TARGET_API_MAC_CARBON
	InitCursor();
	logFatal1("vhalt: %s", message);
	SimpleAlert(kAlertStopAlert,message);
#else
	int32 len=strlen(message);
	if (len>255)
		len=255;
	alert_text[0]=len;
	memcpy(alert_text+1,message,len);
	ParamText(alert_text,"\p","\p","\p");
	InitCursor();
	logFatal1("vhalt: %s", message);
	Alert(128,NULL);
#endif
	ExitToShell();
}

void _alephone_assert(
	const char *file,
	int32 line,
	const char *what)
{
	vhalt(csprintf(assert_text,"%s:%d: %s",file,line,what));
}

void _alephone_warn(
	const char *file,
	int32 line,
	const char *what)
{
	vpause(csprintf(assert_text,"%s:%d: %s",file,line,what));
}


#ifdef TARGET_API_MAC_CARBON
DialogItemIndex SimpleAlert(AlertType Type, const char *Message, const char *Explain)
{
	Str255 MsgStr;
	Str255 XplStr;
	CopyCStringToPascal(Message,MsgStr);
	CopyCStringToPascal(Explain,XplStr);

	DialogItemIndex Button;
	StandardAlert(Type,MsgStr,XplStr,NULL,&Button);

	return Button;
}
#endif
