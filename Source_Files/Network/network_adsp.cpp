/*
NETWORK_ADSP.C

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

Sunday, June 26, 1994 5:33:43 PM
Monday, July 18, 1994 11:47:15 AM
	NetADSPWrite and NetADSPRead now take a "uint16" for length because ADSP can only write
	64K chunks
*/

#include "macintosh_cseries.h"
#include "macintosh_network.h"

#ifdef env68k
#pragma segment network
#endif

/* ---------- constants */

#define kSendBlocking 0
#define kBadSeqMax 0

#define DSP_QUEUE_SIZE 1024

/* ---------- globals */

static short dspRefNum; /* reference number returned by OpenDriver('.DSP', ... ) */

static DSPPBPtr myDSPPBPtr; /* general-purpose non-asynchronous ADSP parameter block */

/* ---------- code */

/*
-----------
NetADSPOpen
-----------

	<--- error

allocates myDSPPBPtr, opens the '.DSP' driver and saves itÕs reference number

------------
NetADSPClose
------------

	<--- error
*/

OSErr NetADSPOpen(
	void)
{
	OSErr error;
	
	error= OpenDriver("\p.DSP", &dspRefNum);
	if (error==noErr)
	{
		myDSPPBPtr= (DSPPBPtr) NewPtrClear(sizeof(DSPParamBlock));
		
		error= MemError();
		if (error==noErr)
		{
		}
	}
	
	return error;
}

OSErr NetADSPClose(
	void)
{
	OSErr error;

	DisposePtr((Ptr)myDSPPBPtr);
	
	error= noErr;
	
	return error;
}

/*
-----------------------------
NetADSPEstablishConnectionEnd
-----------------------------

	---> pointer to a pointer to a ConnectionEnd (will be filled in if error==noErr)
	
	<--- error

establishes an ADSP connection end

---------------------------
NetADSPDisposeConnectionEnd
---------------------------
	
	---> connectionEndPtr

	<--- error

disposes of an ADSP connection end
*/

OSErr NetADSPEstablishConnectionEnd(
	ConnectionEndPtr *connection)
{
	ConnectionEndPtr connectionEnd= (ConnectionEndPtr) NewPtrClear(sizeof(ConnectionEnd));
	OSErr error;
	
	error= MemError();
	if (error==noErr)
	{
		connectionEnd->dspCCB= (TPCCB) NewPtrClear(sizeof(TRCCB));
		connectionEnd->dspSendQPtr= NewPtrClear(DSP_QUEUE_SIZE);
		connectionEnd->dspRecvQPtr= NewPtrClear(DSP_QUEUE_SIZE);
		connectionEnd->dspAttnBufPtr= NewPtrClear(attnBufSize);

		error= MemError();
		if (error==noErr)
		{
			myDSPPBPtr->csCode= dspInit; /* establish a connection end */
			myDSPPBPtr->ioCRefNum= dspRefNum; /* from OpenDriver('.DSP', ...) */
			
			myDSPPBPtr->u.initParams.localSocket= 0; /* dynamically allocate a socket */
			myDSPPBPtr->u.initParams.userRoutine= (ADSPConnectionEventUPP) NULL; /* no unsolicited connection events */
			myDSPPBPtr->u.initParams.ccbPtr= connectionEnd->dspCCB;
			myDSPPBPtr->u.initParams.sendQSize= DSP_QUEUE_SIZE;
			myDSPPBPtr->u.initParams.sendQueue= connectionEnd->dspSendQPtr;
			myDSPPBPtr->u.initParams.recvQSize= DSP_QUEUE_SIZE;
			myDSPPBPtr->u.initParams.recvQueue= connectionEnd->dspRecvQPtr;
			myDSPPBPtr->u.initParams.attnPtr= connectionEnd->dspAttnBufPtr;
			
			error= PBControl((ParmBlkPtr)myDSPPBPtr, false);
			if (error==noErr)
			{
				connectionEnd->ccbRefNum= myDSPPBPtr->ccbRefNum; /* save CCB reference number */
				connectionEnd->socketNum= myDSPPBPtr->u.initParams.localSocket; /* save socket */
#ifdef env68k
				connectionEnd->a5= get_a5(); /* save a5 for later asynchronous calls to retrieve */
#endif
				
				myDSPPBPtr->csCode= dspOptions;
				myDSPPBPtr->ioCRefNum= dspRefNum;
				myDSPPBPtr->ccbRefNum= connectionEnd->ccbRefNum;
				
				myDSPPBPtr->u.optionParams.sendBlocking= kSendBlocking;
				myDSPPBPtr->u.optionParams.badSeqMax= kBadSeqMax;
				myDSPPBPtr->u.optionParams.useCheckSum= false;
				
				error= PBControl((ParmBlkPtr)myDSPPBPtr, false);
				if (error==noErr)
				{
					/* connection end established and initialized with our options */
					*connection= connectionEnd;
				}
			}
		}
	}
	
	return error;
}

OSErr NetADSPDisposeConnectionEnd(
	ConnectionEndPtr connectionEnd)
{
	OSErr error;

	myDSPPBPtr->csCode= dspRemove; /* remove a connection end */
	myDSPPBPtr->ioCRefNum= dspRefNum; /* from OpenDriver('.DSP', ...) */
	myDSPPBPtr->ccbRefNum= connectionEnd->ccbRefNum; /* from PBControl csCode==dspInit */
	myDSPPBPtr->u.closeParams.abort= true; /* tear down the connection immediately */
	
	error= PBControl((ParmBlkPtr)myDSPPBPtr, false);
	if (error==noErr)
	{
		/* free all memory allocated by the connection end */
		DisposePtr((Ptr)connectionEnd->dspCCB);
		DisposePtr(connectionEnd->dspSendQPtr);
		DisposePtr(connectionEnd->dspRecvQPtr);
		DisposePtr(connectionEnd->dspAttnBufPtr);
		DisposePtr((Ptr)connectionEnd); // oops. can't forget this.
	}

	return error;
}

/*
---------------------
NetADSPOpenConnection
---------------------

	---> ConnectionEndPtr

	<--- error

----------------------
NetADSPCloseConnection
----------------------

	---> ConnectionEndPtr
	
	<--- error

------------------------
NetADSPWaitForConnection
------------------------

	---> ConnectionEndPtr
	---> address to connect with (NetAddrBlock *)
	
	<--- error

----------------------------
NetADSPCheckConnectionStatus
----------------------------

	---> ConnectionEndPtr
	
	<--- connection established (bool)
	<--- address block of remote machine (can be NULL)
*/

OSErr NetADSPOpenConnection(
	ConnectionEndPtr connectionEnd,
	NetAddrBlock *address)
{
	OSErr error;
	
	myDSPPBPtr->csCode= dspOpen; /* open a connection */
	myDSPPBPtr->ioCRefNum= dspRefNum; /* from OpenDriver('.DSP', ...) */
	myDSPPBPtr->ccbRefNum= connectionEnd->ccbRefNum; /* from PBControl csCode==dspInit */
	
	myDSPPBPtr->u.openParams.remoteAddress= *address;
	myDSPPBPtr->u.openParams.filterAddress= *address; /* filter out anybody but our target address */
	myDSPPBPtr->u.openParams.ocMode= ocRequest; /* open connection mode */
	myDSPPBPtr->u.openParams.ocInterval= 0; /* default retry interval */
	myDSPPBPtr->u.openParams.ocMaximum= 0; /* default retry maximum */
	
	error= PBControl((ParmBlkPtr)myDSPPBPtr, false);
	if (error==noErr)
	{
		/* successfully opened connection */
	}
	
	return error;
}

OSErr NetADSPCloseConnection(
	ConnectionEndPtr connectionEnd,
	bool abort)
{
	OSErr error;

	myDSPPBPtr->csCode= dspClose; /* remove a connection end */
	myDSPPBPtr->ioCRefNum= dspRefNum; /* from OpenDriver('.DSP', ...) */
	myDSPPBPtr->ccbRefNum= connectionEnd->ccbRefNum; /* from PBControl csCode==dspInit */
	myDSPPBPtr->u.closeParams.abort= abort; /* tear down the connection immediately if true */
	
	error= PBControl((ParmBlkPtr)myDSPPBPtr, false);
	
	return error;
}

OSErr NetADSPWaitForConnection(
	ConnectionEndPtr connectionEnd)
{
	DSPPBPtr myDSPPBPtr= &connectionEnd->pb; /* use private DSPPBPtr (this will be asynchronous) */
	NetAddrBlock filter_address;
	OSErr error;
	
	myDSPPBPtr->csCode= dspOpen; /* open a connection */
	myDSPPBPtr->ioCRefNum= dspRefNum; /* from OpenDriver('.DSP', ...) */
	myDSPPBPtr->ccbRefNum= connectionEnd->ccbRefNum; /* from PBControl csCode==dspInit */
	myDSPPBPtr->ioCompletion= (ADSPCompletionUPP) NULL; /* no completion routine */
	
	filter_address.aNet= filter_address.aNode= filter_address.aSocket= 0;
	myDSPPBPtr->u.openParams.filterAddress= filter_address; /* accept connections from anybody (?) */
	myDSPPBPtr->u.openParams.ocMode= ocPassive; /* open connection mode */
	myDSPPBPtr->u.openParams.ocInterval= 0; /* default retry interval */
	myDSPPBPtr->u.openParams.ocMaximum= 0; /* default retry maximum */
	
	error= PBControl((ParmBlkPtr)myDSPPBPtr, true);
	if (error==noErr)
	{
		/* weÕre asynchronously waiting for a connection now; nobody had better use this
			parameter block or weÕre screwed */
	}
	
	return error;
}

bool NetADSPCheckConnectionStatus(
	ConnectionEndPtr connectionEnd,
	NetAddrBlock *address)
{
	DSPPBPtr myDSPPBPtr= &connectionEnd->pb; /* use private DSPPBPtr (ocPassive call was asynchronous) */
	bool connectionEstablished= false;
	OSErr error;
	
	/* check to make sure weÕre waiting for a connection like we expect */
	
	error= myDSPPBPtr->ioResult;
	if (error!=asyncUncompleted)
	{
		if (error==noErr)
		{
			if (address) *address= myDSPPBPtr->u.openParams.remoteAddress; /* get remote address */
			connectionEstablished= true; /* got one! */
		}
		else
		{
			dprintf("dspOpen(ocPassive, ...) returned %d asynchronously", error);
			
			error= NetADSPWaitForConnection(connectionEnd); /* connection failed, try again */
			if (error!=noErr) dprintf("subsequent NetADSPWaitForConnection() returned %d", error);
		}
	}
	
	return connectionEstablished;
}

/*
------------
NetADSPWrite
------------

	---> connectionEnd pointer
	---> buffer to send
	---> number of bytes to send
	
	<--- error

executed synchronously (not safe at interrupt time)
*/

OSErr NetADSPWrite(
	ConnectionEndPtr connectionEnd,
	void *buffer,
	uint16 *count)
{
	DSPPBPtr myDSPPBPtr= &connectionEnd->pb; /* use private DSPPBPtr */
	OSErr error;

	error= myDSPPBPtr->ioResult;
	if (error==asyncUncompleted)
	{
		dprintf("found previously uncompleted PBControl call, exiting NetADSPWrite() without action");
	}
	else
	{
		if (error==noErr)
		{
			myDSPPBPtr->csCode= dspWrite; /* write data to connection */
			myDSPPBPtr->ioCRefNum= dspRefNum; /* from OpenDriver('.DSP', ...) */
			myDSPPBPtr->ccbRefNum= connectionEnd->ccbRefNum; /* from PBControl csCode==dspInit */
			myDSPPBPtr->ioCompletion= (ADSPCompletionUPP) NULL; /* no completion routine */
			
			myDSPPBPtr->u.ioParams.reqCount= *count;
			myDSPPBPtr->u.ioParams.dataPtr= buffer;
			myDSPPBPtr->u.ioParams.eom= true; /* logical end-of-message */
			myDSPPBPtr->u.ioParams.flush= true; /* flush immediately */
		
			error= PBControl((ParmBlkPtr)myDSPPBPtr, false);
		}
		else
		{
			dprintf("found %d in ioResult during NetADSPWrite()", error);
 		}
	}

	return error;
}

/*
-----------
NetADSPRead
-----------

	---> connectionEnd pointer
	---> buffer to receive into
	<--> number of bytes to receive (returns number of bytes actually read)
	
	<--- error

this is fundamentally broken because it doesnÕt handle timeouts right now; so we can hang
waiting for data.  we should make a dspOptions call to check and see how much data is
available.
*/

OSErr NetADSPRead(
	ConnectionEndPtr connectionEnd,
	void *buffer,
	uint16 *count)
{
	DSPPBPtr myDSPPBPtr= &connectionEnd->pb; /* use private DSPPBPtr (this will be asynchronous) */
	OSErr error;

	if (myDSPPBPtr->ioResult==asyncUncompleted)
	{
		dprintf("waiting for previous uncompleted asynchronous PBControl call in NetADSPRead()");
		while (myDSPPBPtr->ioResult==asyncUncompleted)
			;
	}
	
	myDSPPBPtr->csCode= dspRead; /* read data from connection */
	myDSPPBPtr->ioCRefNum= dspRefNum; /* from OpenDriver('.DSP', ...) */
	myDSPPBPtr->ccbRefNum= connectionEnd->ccbRefNum; /* from PBControl csCode==dspInit */
	myDSPPBPtr->ioCompletion= (ADSPCompletionUPP) NULL; /* no completion routine */
	
	myDSPPBPtr->u.ioParams.reqCount= *count;
	myDSPPBPtr->u.ioParams.dataPtr= buffer;
	
	error= PBControl((ParmBlkPtr)myDSPPBPtr, false);
	if (error==noErr)
	{
		/* return the actual amount of data read */
		*count= myDSPPBPtr->u.ioParams.actCount;
	}
	
	return error;
}
