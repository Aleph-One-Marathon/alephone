/*
NETWORK_DDP.C
Monday, June 27, 1994 1:10:35 PM

Feb. 4, 2000 (Loren Petrich):
	Changed halt() to assert(false) for better debugging
*/

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
		
		error= POpenSkt(myMPPPBPtr, FALSE);
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
			
			error= PCloseSkt(myMPPPBPtr, FALSE);
			
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
	
	return;
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
	AddrBlock *address,
	short protocolType,
	short socket)
{
	MPPPBPtr myMPPPBPtr= &frame->pb;
	OSErr error;

	static long count;
	
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
		myMPPPBPtr->DDP.checksumFlag= FALSE;
		myMPPPBPtr->DDP.ioCompletion= (XPPCompletionUPP) NULL;
		myMPPPBPtr->DDP.u.wdsPointer= (Ptr) &frame->wds;
		error= PWriteDDP(myMPPPBPtr, TRUE);
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

#ifdef OBSOLETE
#ifdef envppc
static Boolean NetDDPSocketListener(
	Ptr SCCAddr1,
	Ptr SCCAddr2,
	Ptr MPPLocalVars,
	Ptr nextFreeByteInRHA,
	Ptr ReadPacketAndReadRestPtr,
	char packetDestinationNumber,
	short numBytesLeftToReadInPacket)
{
	(void) (SCCAddr1,SCCAddr2,MPPLocalVars,nextFreeByteInRHA,ReadPacketAndReadRestPtr,packetDestinationNumber,numBytesLeftToReadInPacket);
	
	// LP change:
	assert(false);
	// halt();
}
#endif
#endif

#ifdef OBSOLETE
static ParmBlkPtr killioPBPtr;

		
		killioPBPtr= (ParmBlkPtr) NewPtrClear(sizeof(ParamBlockRec));
		assert(killioPBPtr);

			if (killioPBPtr->ioParam.ioResult==noErr)
			{
				OSErr error;
				
				killioPBPtr->ioParam.ioRefNum= mppRefNum;
				
				error= PBKillIO(killioPBPtr, TRUE);
				dprintf("PBKillIO(%d) returned %d;g", mppRefNum, error);
			}
			else
			{
				dprintf("last async PBKillIO returned %d", killioPBPtr->ioParam.ioResult);
				if (killioPBPtr->ioParam.ioResult!=asyncUncompleted) killioPBPtr->ioParam.ioResult= noErr;
			}
#endif