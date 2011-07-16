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
	Added a backup to get_my_fsspec to back out of Carbon bundle
*/

#include <string.h>

#if defined(EXPLICIT_CARBON_HEADER)
    #include <Carbon/Carbon.h>
/*
#else
#include <Errors.h>
#include <Processes.h>
*/
#endif

#include "csfiles.h"
#include "csstrings.h"

extern OSErr get_file_spec(
	FSSpec *spec,
	short listid,
	short item,
	short pathsid)
{
	Str255 itemstr;
	Str255 pathstr;
	int i,n;
	OSErr err;
	int itemlen,pathlen;

	getpstr(itemstr,listid,item);
	err=FSMakeFSSpec(0,0,itemstr,spec);
	if (err==noErr)
		return err;
	itemlen=itemstr[0];
	n=countstr(pathsid);
	for (i=0; i<n; i++) {
		getpstr(pathstr,pathsid,i);
		pathlen=pathstr[0];
		pathstr[0]=pathlen+itemlen;
		memcpy(pathstr+pathlen+1,itemstr+1,itemlen);
		err=FSMakeFSSpec(0,0,itemstr,spec);
		if (err==noErr)
			return err;
	}
	return fnfErr;
}

extern OSErr get_my_fsspec(
	FSSpec *spec)
{
	ProcessSerialNumber psn;
	ProcessInfoRec pir;

	psn.highLongOfPSN=0;
	psn.lowLongOfPSN=kCurrentProcess;
	pir.processInfoLength=sizeof pir;
	pir.processName=NULL;
	pir.processAppSpec=spec;
#if defined(APPLICATION_IS_BUNDLED)
	OSStatus err = GetProcessInformation(&psn,&pir);
	if(err == noErr)
	{
		// Back up out of our bundle
		err = FSMakeFSSpec(spec->vRefNum, spec->parID, "\p:::", spec);
	}
	return err;
#else
	return GetProcessInformation(&psn,&pir);
#endif
}
