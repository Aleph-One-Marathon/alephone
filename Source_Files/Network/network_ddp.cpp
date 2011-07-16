/*
NETWORK_DDP.C

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

Monday, June 27, 1994 1:10:35 PM

Feb. 4, 2000 (Loren Petrich):
	Changed halt() to assert(false) for better debugging
*/

#if !defined(DISABLE_NETWORKING)

#include "macintosh_cseries.h"
#include "macintosh_network.h"

#ifdef env68k
#pragma segment network
#endif

#define SOCKET_LISTENER_RESOURCE_TYPE 'SOCK'
#define SOCKET_LISTENER_ID 128

enum 
{
	// info for calling the packet handler
	uppPacketHandlerProcInfo = kCStackBased
		| RESULT_SIZE(kNoByteCode)
		| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof (DDPPacketBufferPtr))),
	
	// info for calling the procedure that initializes the ddp socket listener
	uppInitializeListenerProcInfo = kCStackBased
		| RESULT_SIZE(SIZE_CODE(sizeof (DDPSocketListenerUPP)))
		| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof (PacketHandlerUPP)))
		| STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof (DDPPacketBufferPtr)))
};

/* ---------- prototypes/NETWORK_SOCKET_LISTENER.A */

/* ---------- globals */

static DDPPacketBufferPtr ddpPacketBuffer= (DDPPacketBufferPtr) NULL;

static DDPSocketListenerUPP listenerUPP= (DDPSocketListenerUPP) NULL;

/* ---------- code */

/*
----------
NetDDPOpen
----------

	<--- error
	
assure the the .MPP driver is open

-----------
NetDDPClose
-----------

	<--- error
*/

OSErr NetDDPOpen(
	void)
{
	short mpp_driver_reference_number;
	OSErr error;
	
	error= OpenDriver("\p.MPP", &mpp_driver_reference_number);
	if (error==noErr)
	{
		assert(mppRefNum==mpp_driver_reference_number);
	}
	
	return error;
}

OSErr NetDDPClose(
	void)
{
	OSErr error;
	
	error= noErr;
	
	return error;
}

/*
----------------
NetDDPOpenSocket
----------------

	<--- error
	<--- socket number

-----------------
NetDDPCloseSocket
 ----------------

	---> socket number

	<--- error
*/


OSErr NetDDPOpenSocket(
	short *socketNumber,
	PacketHandlerProcPtr packetHandler)
{
	OSErr     error;
	Handle    socket_listener_resource;
	InitializeListenerUPP initializeUPP;
	DDPSocketListenerUPP listenerUPP;
	MPPPBPtr  myMPPPBPtr= (MPPPBPtr) NewPtrClear(sizeof(MPPParamBlock));

	static PacketHandlerUPP handlerUPP = NULL;
	
	assert(packetHandler); /* canÕt have NULL packet handlers */
	assert(!ddpPacketBuffer); /* canÕt have more than one socket listener installed */
	
	socket_listener_resource = GetResource(SOCKET_LISTENER_RESOURCE_TYPE, SOCKET_LISTENER_ID);
	assert(socket_listener_resource);
	HLock(socket_listener_resource);
	HNoPurge(socket_listener_resource);
	
	initializeUPP = (InitializeListenerUPP) StripAddress(*socket_listener_resource);
	
	ddpPacketBuffer= (DDPPacketBufferPtr) NewPtrClear(sizeof(DDPPacketBuffer));
	
	error= MemError();
	if (error==noErr)
	{
		if (handlerUPP == NULL)
		{
			handlerUPP = (PacketHandlerUPP) NewRoutineDescriptor(
				(ProcPtr)packetHandler, uppPacketHandlerProcInfo, GetCurrentISA());
		}
		assert(handlerUPP);

#if 0
		// calling NewRoutineDescriptor with kM68kISA as the final argument is a waste of bits.
		if (initialize_upp == NULL)
		{
			initialize_upp = (initializeListenerUPP) NewRoutineDescriptor(
				initialize_socket_listener,uppInitializeListenerProcInfo, kM68kISA); // it's in a 68k code resource
		}
		assert(initialize_upp);		
#endif

#ifdef env68k  // it seems that we don't have CallUniversalProc() in the library. strange...
		listenerUPP = (*initializeUPP)(
			handlerUPP, ddpPacketBuffer);
#else	
		listenerUPP = (DDPSocketListenerUPP) CallUniversalProc(
			initializeUPP, uppInitializeListenerProcInfo,
			handlerUPP, ddpPacketBuffer);
#endif

#if 0
		// calling NewRoutineDescriptor with kM68kISA as the final argument is still a waste of bits.

		listenerUPP = (DDPSocketListenerUPP) NewRoutineDescriptor((ProcPtr) socket_listener, uppDDPSocketListenerProcInfo, 
			kM68kISA); // have to force it to realize that it's a 68K resource
		assert(listenerUPP);
#endif

		myMPPPBPtr->DDP.socket= 0;
		myMPPPBPtr->DDP.u.listener= listenerUPP;
		
		error= POpenSkt(myMPPPBPtr, false);
		if (error==noErr)
		{
			*socketNumber= myMPPPBPtr->DDP.socket;
		}
		
	}
	DisposePtr((Ptr)myMPPPBPtr);
	
	return error;
}

OSErr NetDDPCloseSocket(
	short socketNumber)
{
	OSErr error= noErr;
	
	if (ddpPacketBuffer)
	{
		MPPPBPtr myMPPPBPtr= (MPPPBPtr) NewPtrClear(sizeof(MPPParamBlock));

		error= MemError();
		if (error==noErr)
		{
			myMPPPBPtr->DDP.socket= socketNumber;
			
			error= PCloseSkt(myMPPPBPtr, false);
			
			DisposePtr((Ptr)ddpPacketBuffer);
			ddpPacketBuffer= (DDPPacketBufferPtr) NULL;

			DisposePtr((Ptr)myMPPPBPtr);
		}
	}
	
	return error;
}

/*
--------------
NetDDPNewFrame
--------------

	<--- ddp frame pointer

------------------
NetDDPDisposeFrame
------------------

	---> ddp frame pointer to dispose
*/

DDPFramePtr NetDDPNewFrame(
	void)
{
	DDPFramePtr frame= (DDPFramePtr) NewPtrClear(sizeof(DDPFrame));
	
	return frame;
}

void NetDDPDisposeFrame(
	DDPFramePtr frame)
{
	DisposePtr((Ptr)frame);
}

/*
---------------
NetDDPSendFrame
---------------

	---> ddp frame pointer
	---> address to send to
	---> ddp protocol type
	---> socket to send through
	
	<--- error

asynchronously sends the given frame to the given address
*/

OSErr NetDDPSendFrame(
	DDPFramePtr frame,
	NetAddrBlock *address,
	short protocolType,
	short socket)
{
	MPPPBPtr myMPPPBPtr= &frame->pb;
	OSErr error;

	static int32 count;
	
	assert(frame->data_size <= ddpMaxData);
	
	error= myMPPPBPtr->DDP.ioResult;
	if (error==noErr||error==excessCollsns||error==abortErr)
	{
		if (count > 2)
		{
//			dprintf("Previous PWriteDDP call was uncompleted for %d times;g", count);
			count = 0;
		}
		BuildDDPwds((Ptr) &frame->wds, (Ptr) &frame->header, (Ptr) &frame->data, *address,
			protocolType, frame->data_size);
	
		myMPPPBPtr->DDP.socket= socket;
		myMPPPBPtr->DDP.checksumFlag= false;
		myMPPPBPtr->DDP.ioCompletion= (XPPCompletionUPP) NULL;
		myMPPPBPtr->DDP.u.wdsPointer= (Ptr) &frame->wds;
		error= PWriteDDP(myMPPPBPtr, true);
		count = 0;
	}
	else
	{
		if (error!=asyncUncompleted)
		{
			dprintf("asynchronous NetDDPSendFrame(%p) returned %d", frame, error);
			myMPPPBPtr->DDP.ioResult= noErr;
		}
		else
		{
			count++;
		}
	}
	
	return error;
}

#endif // !defined(DISABLE_NETWORKING)
