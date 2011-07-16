/*
NETWORK_NAMES.C

	Copyright (C) 1991-2001 and beyond by Bungie Studios, Inc.
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

Sunday, June 26, 1994 5:45:49 PM
Friday, July 15, 1994 11:03:22 AM
	allocated name table entry in the system heap, to prevent problems if the name isn't 
	unregistered before the application exits. (ajr, suggested by jgj.)
*/

#if !defined(DISABLE_NETWORKING)

#include "macintosh_cseries.h"
#include "macintosh_network.h"

#ifdef env68k
#pragma segment network
#endif

// #define MODEM_TEST

/* ---------- constants */

#define strUSER_NAME -16096

/* ---------- types */

typedef NamesTableEntry *NamesTableEntryPtr;

/* ---------- globals */

static NamesTableEntryPtr myNTEName= (NamesTableEntryPtr) NULL;

/* ---------- code */

/*
---------------
NetRegisterName
---------------

allocates and registers and entity name for the given socket; call NetUnRegisterName to unregister
the name.  only one name can be registered at a time through NetRegisterName().

	---> name (pascal string, can be NULL and will be replaced with user name)
	---> type (pascal string)
	---> socket number
	
	<--- error
*/

OSErr NetRegisterName(
	unsigned char *name,
	unsigned char *type,
	short version,
	short socketNumber)
{
	MPPPBPtr myMPPPBPtr= (MPPPBPtr) NewPtrClear(sizeof(MPPParamBlock));
	Str255 adjusted_name;
	Str255 adjusted_type;
	OSErr error;

#ifdef MODEM_TEST
	error= ModemRegisterName(name, type, version, socketNumber);
#else

	assert(!myNTEName);
	// we stick it in the system heap so that if the application crashes/quits while the name
	// is registered, this pointer won't be trashed by another application (unless, of course,
	// it trashes the system heap).
	myNTEName= (NamesTableEntryPtr) NewPtrSysClear(sizeof(NamesTableEntry));
	assert(myNTEName);

	/* get user name if no object name was supplied*/
	pstrcpy(adjusted_name, name ? name : *GetString(strUSER_NAME));

	/* Calculate the adjusted type */
	psprintf(adjusted_type, "%.*s%d", type[0], type+1, version);

	error= MemError();
	if (error==noErr)
	{
		NBPSetNTE((Ptr)myNTEName, adjusted_name, adjusted_type, "\p*", socketNumber); /* build names table entry */

		myMPPPBPtr->NBP.nbpPtrs.ntQElPtr= (Ptr) myNTEName;
		myMPPPBPtr->NBP.parm.verifyFlag= true; /* verify this name doesn’t already exist */
		myMPPPBPtr->NBP.interval= 2; /* retry every 2*8 == 16 ticks */
		myMPPPBPtr->NBP.count= 4; /* retry 4 times ( == 64 ticks) */
	
		error= PRegisterName(myMPPPBPtr, false);
		
		DisposePtr((Ptr)myMPPPBPtr);
	}
#endif
	
	return error;
}

/*

-----------------
NetUnRegisterName
-----------------

	(no parameters)

deallocates and unregisters the entity name previously allocated with NetRegisterName().
*/

OSErr NetUnRegisterName(
	void)
{
	OSErr error= noErr;

#ifdef MODEM_TEST
	error= ModemUnRegisterName();
#else

	if (myNTEName)
	{
		MPPPBPtr myMPPPBPtr= (MPPPBPtr) NewPtrClear(sizeof(MPPParamBlock));

		error= MemError();
		if (error==noErr)
		{
			myMPPPBPtr->NBP.nbpPtrs.entityPtr= (Ptr) &myNTEName->nt.entityData; /* can’t just give back names table entry */
			
			error= PRemoveName(myMPPPBPtr, false);
		
			DisposePtr((Ptr)myMPPPBPtr);
	
			DisposePtr((Ptr)myNTEName);
			myNTEName= (NamesTableEntryPtr) NULL;
		}
	}
#endif

	return error;
}

#endif // !defined(DISABLE_NETWORKING)

