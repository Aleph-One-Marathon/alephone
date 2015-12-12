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


#ifndef _CSERIES_ALERTS_
#define _CSERIES_ALERTS_

#include "cseries.h"  // need OSErr

#if defined(__GNUC__)
#define NORETURN __attribute__((noreturn))
#else
#define NORETURN
#endif

enum {
	infoError,
	fatalError
};

extern void alert_user(const char *message, short severity = infoError);

extern void alert_user(
	short severity,
	short resid,
	short item,
	OSErr error);

extern bool alert_choose_scenario(char *chosen_dir);

extern void launch_url_in_browser(const char *url);

extern void pause_debug(void);
extern void vpause(
	const char *message);

extern void halt(void) NORETURN;
extern void vhalt(
	const char *message) NORETURN;

extern void _alephone_assert(
	const char *file,
	int32 line,
	const char *what) NORETURN;
extern void _alephone_warn(
	const char *file,
	int32 line,
	const char *what);


#undef assert
#ifdef DEBUG
#define assert(what) ((what) ? (void)0 : _alephone_assert(__FILE__,__LINE__,"Assertion failed: " #what))
#define vassert(what,message) ((what) ? (void)0 : _alephone_assert(__FILE__,__LINE__,(message)))
#define warn(what) ((what) ? (void)0 : _alephone_warn(__FILE__,__LINE__,"Assertion failed: " #what))
#define vwarn(what,message) ((what) ? (void)0 : _alephone_warn(__FILE__,__LINE__,(message)))
#else
#define assert(what) ((void) 0)
#define vassert(what,message) ((void) 0)
#define warn(what) ((void) 0)
#define vwarn(what,message) ((void) 0)
#endif

#endif
