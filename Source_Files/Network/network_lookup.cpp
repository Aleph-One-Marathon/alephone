/*
NETWORK_LOOKUP.C

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

Sunday, June 26, 1994 5:21:47 PM

Tuesday, June 28, 1994 8:24:38 PM
  allow lookupUpdateProc and lookupFilterProc to be NULL: they won't be called
Wednesday, October 19, 1994 3:50:46 PM  (Jason')
	zone changes, rewriting jason-style.
*/

#if !defined(DISABLE_NETWORKING)

#include "macintosh_cseries.h"
#include "macintosh_network.h"
#include <string.h>
#include <stdlib.h>

#ifdef env68k
#pragma segment network
#endif

// #define TEST_MODEM

/* ---------- constants */

/* this is the number of calls a named entity can be absent from the network before we remove it
	(about four seconds) */
#define ENTITY_PERSISTENCE 4

/* this is the buffer we give to NBP to fill */
#define NAMES_LIST_BUFFER_SIZE      4096
#define MAXIMUM_LOOKUP_NAME_COUNT     40

#define MAXIMUM_ZONE_NAMES 400

/* ---------- structures */

struct NetLookupEntity
{
	NetEntityName entity;
	NetAddrBlock address;

	short persistence;
};
typedef struct NetLookupEntity NetLookupEntity, *NetLookupEntityPtr;

/* ---------- globals */

static MPPPBPtr lookupMPPPBPtr= (MPPPBPtr) NULL; /* the MPPPBPtr PNBPLookup() is using */
static NetEntityNamePtr lookupEntity; /* the entity PNBPLookup() is matching against */
static Ptr lookupBuffer; /* the buffer PNBPLookup() is filling up */

static short lookupCount; /* number of entities in our list */
static NetLookupEntityPtr lookupEntities; /* the entities in our list */

static lookupUpdateProcPtr lookupUpdateProc; /* to inform the caller when the list changes */
static lookupFilterProcPtr lookupFilterProc; /* to filter entities */

/* ---------- private prototypes */

static int zone_name_compare(void const *elem1, void const *elem2);

/* ---------- code */

/*
-------------
NetLookupOpen
-------------

	---> name to match against (pascal string)
	---> type to match against (pascal string)
	---> zone to look in (pascal string, can be "*")
	---> function to call when the list changes (call NetLookupUpdate(), below, to trigger this) (may be NULL)
	---> function to call to filter names from the list (may be NULL)
	
	<--- error

start the asynchronous PNBPLookup() machine.
*/

OSErr NetLookupOpen(
	unsigned char *name,
	unsigned char *type,
	unsigned char *zone,
	short version,
	lookupUpdateProcPtr updateProc,
	lookupFilterProcPtr filterProc)
{
	OSErr error;
	Str255 type_with_version;

#ifdef TEST_MODEM
	error= ModemLookupOpen(name, type, zone, version, updateProc, filterProc);
#else
	
	assert(!lookupMPPPBPtr);

	/* Note that this utilizes the mac extension for pstrings.. */
	psprintf(type_with_version, "%.*s%d", type[0], type+1, version);
	assert(type_with_version[0]<32);

	/* Type is pstring */
	lookupMPPPBPtr= (MPPPBPtr) NewPtrClear(sizeof(MPPParamBlock));
	lookupEntity= (NetEntityNamePtr) NewPtrClear(sizeof(NetEntityName));
	lookupBuffer= NewPtrClear(NAMES_LIST_BUFFER_SIZE);
	lookupEntities= (NetLookupEntityPtr) NewPtr(MAXIMUM_LOOKUP_NAME_COUNT*sizeof(NetLookupEntity));
	
	error= MemError();
	if (error==noErr)
	{
		NBPSetEntity((Ptr)lookupEntity, name, type_with_version, zone);
		lookupMPPPBPtr->NBP.interval= 4; /* 4*8 ticks == 32 ticks */
		lookupMPPPBPtr->NBP.count= 2; /* 2 retries == 64 ticks */
		lookupMPPPBPtr->NBP.ioCompletion= (XPPCompletionUPP) NULL; /* no completion routine */
		lookupMPPPBPtr->NBP.nbpPtrs.entityPtr= (Ptr) lookupEntity;
		lookupMPPPBPtr->NBP.parm.Lookup.retBuffPtr= lookupBuffer;
		lookupMPPPBPtr->NBP.parm.Lookup.retBuffSize= NAMES_LIST_BUFFER_SIZE;
		lookupMPPPBPtr->NBP.parm.Lookup.maxToGet= MAXIMUM_LOOKUP_NAME_COUNT;
		error= PLookupName(lookupMPPPBPtr, true);	
	
		lookupCount= 0; /* no names, initially */
		lookupUpdateProc= updateProc; /* remember the callerÕs update procedure */
		lookupFilterProc= filterProc; /* remember the callerÕs filter procedure */
	}
#endif
	
	return error;
}

/*
--------------
NetLookupClose
--------------

	(no parameters)

kills the pending PNBPLookup, frees buffers and returns.
*/

void NetLookupClose(
	void)
{
	OSErr error= noErr;

#ifdef TEST_MODEM
	ModemLookupClose();
#else
	if (lookupMPPPBPtr)
	{
		MPPPBPtr myMPPPBPtr= (MPPPBPtr) NewPtrClear(sizeof(MPPParamBlock));
		
		myMPPPBPtr->NBPKILL.nKillQEl= (Ptr) lookupMPPPBPtr;
		error= PKillNBP(myMPPPBPtr, false);
		/* presumably cbNotFound means the PNBPLookup has already terminated */
		if (error!=reqAborted&&error!=noErr&&error!=cbNotFound) dprintf("PKillNBP() returned %d", error);
	
		DisposePtr((Ptr)lookupMPPPBPtr);
		DisposePtr((Ptr)lookupEntity);
		DisposePtr(lookupBuffer);
		DisposePtr((Ptr)lookupEntities);
		DisposePtr((Ptr)myMPPPBPtr);
		
		lookupMPPPBPtr= (MPPPBPtr) NULL;
	}
#endif
}

/*
---------------
NetLookupRemove
---------------

	---> index to remove

removes a named entity from the names list before it disappears from the network.  this is used
if an entity didnÕt respond to a packet we sent or we added the entity to our game (calls the
user update procedure with the deleteEntity message).
*/

void NetLookupRemove(
	short index)
{
#ifdef TEST_MODEM
	ModemLookupRemove(index);
#else
	assert(index>=0&&index<lookupCount);
	
	/* compact the entity list on top of the deleted entry, decrement lookupCount */
	BlockMove(lookupEntities+index+1, lookupEntities+index, sizeof(NetLookupEntity)*(lookupCount-index));
	lookupCount-= 1;
	
	/* tell the caller to make the change */
	if (lookupUpdateProc) lookupUpdateProc(removeEntity, index);
#endif
}

/*
--------------------
NetLookupInformation
--------------------

	---> index
	---> pointer to an NetEntityName structure to be filled in, can be NULL
	---> pointer to an NetAddrBlock structure to be filled in, can be NULL

given an index, return an NetEntityName and NetAddrBlock
*/

void NetLookupInformation(
	short index,
	NetAddrBlock *address,
	NetEntityName *entity)
{
#ifdef TEST_MODEM
	ModemLookupInformation(index, address, entity);
#else
	assert(index>=0&&index<lookupCount);

	if (address) *address= lookupEntities[index].address;
	if (entity) *entity= lookupEntities[index].entity;
#endif
}

/*
---------------
NetLookupUpdate
---------------

	(no parameters)

if the PNBPLookup call has terminated, calls the caller-specified procedure to handle changes
(if any) to the state of the NBP world.  otherwise it does nothing.
*/

void NetLookupUpdate(
	void)
{
	short entity_index, entity_count, insertion_point;
	NetEntityName entity;
	NetAddrBlock address;
	OSErr error;

#ifdef TEST_MODEM
	ModemLookupUpdate();
#else
	assert(lookupMPPPBPtr);
	
	/* donÕt do anything if the asynchronous PNBPLookupName() hasnÕt returned */
	if (lookupMPPPBPtr->NBP.ioResult!=asyncUncompleted)
	{
		entity_count= lookupMPPPBPtr->NBP.parm.Lookup.numGotten;
		for (entity_index=0;entity_index<entity_count;++entity_index)
		{
			bool insert;
			
			/* get this entityÕs address and entity structure */
			NBPExtract(lookupBuffer, entity_count, entity_index+1, &entity, &address);
			
			if (lookupFilterProc == NULL || lookupFilterProc(&entity, &address))
			{
				/* see if we can find an old entry in our entity list with the same node, socket and
					network numbers.  if we can, reset his persistence and continue.  */
				insert= true; /* default to inserting this entity */
				for (insertion_point=0;insertion_point<lookupCount;++insertion_point)
				{
					if (address.aNode==lookupEntities[insertion_point].address.aNode &&
						address.aSocket==lookupEntities[insertion_point].address.aSocket &&
						address.aNet==lookupEntities[insertion_point].address.aNet)
					{
						/* this entity is already in the list; reset his persistence and continue */
						lookupEntities[insertion_point].persistence= ENTITY_PERSISTENCE;
						insert= false;
						break;
					}
					if (IUCompString(entity.objStr, lookupEntities[insertion_point].entity.objStr)<0)
					{
						/* the name we are looking at is alphabetically higher than the name we
							just extracted with NBPExtract(), so we might as well give up looking
							for a match (this must be a new name) */
						break;
					}
				}
				
				if (insert&&lookupCount<MAXIMUM_LOOKUP_NAME_COUNT)
				{
					BlockMove(lookupEntities+insertion_point, lookupEntities+insertion_point+1,
						sizeof(NetLookupEntity)*(lookupCount-insertion_point));
					lookupEntities[insertion_point].address= address;
					lookupEntities[insertion_point].entity= entity;
					lookupEntities[insertion_point].persistence= ENTITY_PERSISTENCE;
					lookupCount+= 1;
	
					/* only tell the caller we inserted the new entry after weÕre ready to handle
						him asking us about it */
					if (lookupUpdateProc) lookupUpdateProc(insertEntity, insertion_point);
				}
			}
		}
		
		/* find and remove all entities who havenÕt responded in ENTITY_PERSISTENCE calls */
		for (entity_index=0;entity_index<lookupCount;entity_index+= 1)
		{
			if ((lookupEntities[entity_index].persistence-= 1)<0)
			{
				NetLookupRemove(entity_index);

				/* donÕt advance entity_index this time */
				entity_index-= 1;
			}
		}
		
		/* start another asynchronous PNBPLookup() */
		
		lookupMPPPBPtr->NBP.interval= 4; /* 4*8 ticks == 32 ticks */
		lookupMPPPBPtr->NBP.count= 2; /* 2 retries == 64 ticks */
		lookupMPPPBPtr->NBP.ioCompletion= (XPPCompletionUPP) NULL; /* no completion routine */
		lookupMPPPBPtr->NBP.nbpPtrs.entityPtr= (Ptr) lookupEntity;
		lookupMPPPBPtr->NBP.parm.Lookup.retBuffPtr= lookupBuffer;
		lookupMPPPBPtr->NBP.parm.Lookup.retBuffSize= NAMES_LIST_BUFFER_SIZE;
		lookupMPPPBPtr->NBP.parm.Lookup.maxToGet= MAXIMUM_LOOKUP_NAME_COUNT;
		
		error= PLookupName(lookupMPPPBPtr, true);
		if (error!=noErr) dprintf("Subsequent PLookupName() returned %d", error);
	}
#endif
}

OSErr NetGetZonePopupMenu(
	MenuHandle menu,
	short *local_zone)
{
	Str32 *zone_names;
	short zone_count;
	OSErr error;
	
	/* make sure we have a menu and delete all itÕs items */
	assert(menu);
	while (CountMenuItems(menu))
		DeleteMenuItem(menu, 1);
	
	zone_names= (Str32 *)NewPtr(sizeof (Str32)*MAXIMUM_ZONE_NAMES);
	if (zone_names)
	{
		SetCursor(*(GetCursor(watchCursor)));
		error= NetGetZoneList(zone_names, MAXIMUM_ZONE_NAMES, &zone_count, local_zone);
		(*local_zone)+= 1; /* adjust to be one-based for menu manager */
		
		if (error==noErr && zone_count>1)
		{
			short i;
			
			for (i= 0; i<zone_count; ++i)
			{
				AppendMenu(menu, "\p ");
				SetMenuItemText(menu, i+1, zone_names[i]);
			}

			CheckItem(menu, *local_zone, true);
		}
		
		DisposePtr((Ptr)zone_names);
	}
	else
	{
		error= MemError();
	}
	SetCursor(&qd.arrow);

	return error;
}

OSErr NetGetZoneList(
	Str32 *zone_names,
	short maximum_zone_names,
	short *zone_count,
	short *local_zone)
{
	XPPParmBlkPtr xpb= (XPPParmBlkPtr) NewPtrClear(sizeof(XPPParamBlock));
	Ptr zip_buffer= NewPtrClear(578);
	OSErr error;

	if (zone_names && zip_buffer && xpb)
	{
		xpb->XCALL.zipInfoField[0]= 0;
		xpb->XCALL.zipInfoField[1]= 0;
		xpb->XCALL.ioCompletion= (XPPCompletionUPP) NULL;		
		xpb->XCALL.ioVRefNum= 0;
		xpb->XCALL.ioRefNum= xppRefNum;
		xpb->XCALL.csCode= xCall;
		xpb->XCALL.xppSubCode= zipGetZoneList;
		xpb->XCALL.xppTimeout= 4;
		xpb->XCALL.xppRetry= 8;
		xpb->XCALL.zipBuffPtr= (Ptr) zip_buffer;

		/* get a list of all zones */		
		*zone_count= 0;
		do
		{
			error= PBControl((ParmBlkPtr) xpb, false);
			if (error==noErr)
			{
				byte *read;
				short i;
				
				for (i= 0, read= (byte *)zip_buffer;
						i<xpb->XCALL.zipNumZones && *zone_count<maximum_zone_names;
						++i, read+= *read+1)
				{
					pstrcpy(zone_names[(*zone_count)], read);
					(*zone_count)+= 1;
				}
			}
		}
		while(!xpb->XCALL.zipLastFlag && error==noErr && *zone_count<maximum_zone_names);

		if (error==noErr)
		{
			Str32 local_zone_name;
			short i;
			
			/* sort the list */
			qsort(zone_names, *zone_count, sizeof(Str32), zone_name_compare);
	
			/* get our local zone name and locate it in the list */
			error= NetGetLocalZoneName(local_zone_name);
			if (error==noErr)
			{
				*local_zone= 0;
				for (i= 0; i<*zone_count; ++i)
				{
					if (!IUCompString(local_zone_name, zone_names[i]))
					{
						*local_zone= i;
						break;
					}
				}
				vwarn(i!=*zone_count, csprintf(temporary, "couldnÕt find local zone '%p'", local_zone_name));
			}
		}
			
		DisposePtr(zip_buffer);
		DisposePtr((Ptr)xpb);
	}
	else
	{
		error= MemError();
	}
	
	return error;
}

OSErr NetGetLocalZoneName(
	Str32 local_zone_name)
{
	XPPParmBlkPtr xpb= (XPPParmBlkPtr) NewPtrClear(sizeof(XPPParamBlock));
	OSErr error;

	if (xpb)
	{
		xpb->XCALL.csCode= xCall;
		xpb->XCALL.ioRefNum= xppRefNum;
		xpb->XCALL.xppSubCode= zipGetMyZone;
		xpb->XCALL.zipBuffPtr= (Ptr) local_zone_name;
		xpb->XCALL.zipInfoField[0]= 0;
		xpb->XCALL.zipInfoField[1]= 0;
		
		error= PBControl((ParmBlkPtr) xpb, false);

		DisposePtr((Ptr)xpb);
	}
	else
	{
		error= MemError();
	}
	
	return error;
}

/* ----------- private code */

static int zone_name_compare(
	void const *elem1,
	void const *elem2)
{
	return IUCompString((unsigned char const *)elem1, (unsigned char const *)elem2);
}

#endif // !defined(DISABLE_NETWORKING)

