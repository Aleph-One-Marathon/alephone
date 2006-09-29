/*
	File:		CarbonSndPlayDB.c
	
	Description: Routines demonstrating how to create a function that works
				 much like the original SndPlayDoubleBuffer but is Carbon compatible
				 (which SndPlayDoubleBuffer isn't).

	Author:		Quinn, MC, ERA
	
	Version:	1.0.3

	Copyright: 	© Copyright 1999-2002 Apple Computer, Inc. All rights reserved.
	
	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
				copyrights in this original Apple software (the "Apple Software"), to use,
				reproduce, modify and redistribute the Apple Software, with or without
				modifications, in source and/or binary forms; provided that if you redistribute
				the Apple Software in its entirety and without modifications, you must retain
				this notice and the following text and disclaimers in all such redistributions of
				the Apple Software.  Neither the name, trademarks, service marks or logos of
				Apple Computer, Inc. may be used to endorse or promote products derived from the
				Apple Software without specific prior written permission from Apple.  Except as
				expressly stated in this notice, no other rights or licenses, express or implied,
				are granted by Apple herein, including but not limited to any patent rights that
				may be infringed by your derivative works or by other works in which the Apple
				Software may be incorporated.

				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
				WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
				WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
				PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
				COMBINATION WITH YOUR PRODUCTS.

				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
				CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
				GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
				ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
				OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
				(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
				ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
				
	Change History (most recent first): <1> 6/12/02 Fixed OS X crashing bugs per radar 2944451 & 2894327

*/

/* Requirements for using this shim code:

	1) The sound channel's queue must be empty before you call CarbonSndPlayDoubleBuffer
	2) You cannot call MySndDoImmediate until CarbonSndPlayDoubleBuffer returns.  Be
	   careful about calling MySndDoImmediate at interrupt time with a quietCmd.

*/

#define TARGET_API_MAC_CARBON 1

#ifdef __APPLE_CC__
	#include <Carbon/Carbon.h>
#else
	#include <Carbon.h>
#endif

#include "CarbonSndPlayDB.h"
#undef DEBUG
#define DEBUG 0

#define kBufSize					2048

// Structs
struct PerChanInfo {
	QElemPtr 						qLink;						/* next queue entry */
	short 							qType;						/* queue type = 0 */
	short							stopping;
	#if DEBUG
		OSType						magic;
	#endif
	SndCallBackUPP 					usersCallBack;
	SndDoubleBufferHeaderPtr		theParams;
	CmpSoundHeader					soundHeader;
};
typedef struct PerChanInfo			PerChanInfo;
typedef struct PerChanInfo *		PerChanInfoPtr;

// Globals
	Boolean							gNMRecBusy;
	NMRecPtr						gNMRecPtr;
	QHdrPtr							gFreeList;
	Ptr								gSilenceTwos;
	Ptr								gSilenceOnes;
static SndCallBackUPP				gCarbonSndPlayDoubleBufferCallBackUPP = nil;
static SndCallBackUPP				gCarbonSndPlayDoubleBufferCleanUpUPP = nil;

static	pascal	void	CarbonSndPlayDoubleBufferCleanUpProc (SndChannelPtr theChannel, SndCommand * theCallBackCmd);
static	pascal	void	CarbonSndPlayDoubleBufferCallBackProc (SndChannelPtr theChannel, SndCommand * theCmd);
static			void	InsertSndDoCommand (SndChannelPtr chan, SndCommand * theCmd);
static	pascal	void	NMResponseProc (NMRecPtr nmReqPtr);

// Remember this routine could be called at interrupt time, so don't allocate or deallocate memory.
OSErr	MySndDoImmediate (SndChannelPtr chan, SndCommand * cmd) {
	PerChanInfoPtr					perChanInfoPtr;

	// Is this being called on one of the sound channels we are manipulating?
	// If so, we need to pull our callback out of the way so that the user's commands run
	if (gCarbonSndPlayDoubleBufferCallBackUPP == chan->callBack && gCarbonSndPlayDoubleBufferCallBackUPP != nil) {
		if (quietCmd == cmd->cmd || flushCmd == cmd->cmd) {
			// We know that our callBackCmd is the first item in the queue if this is our channel
			perChanInfoPtr = (PerChanInfoPtr)(chan->queue[chan->qHead].param2);
			#if DEBUG
				if (perChanInfoPtr->magic != 'SANE') DebugStr("\pBAD in MySndDoImmediate");
			#endif
			perChanInfoPtr->stopping = true;
			Enqueue ((QElemPtr)perChanInfoPtr, gFreeList);
			if (! OTAtomicSetBit (&gNMRecBusy, 0)) {
				NMInstall (gNMRecPtr);
			}
			chan->callBack = perChanInfoPtr->usersCallBack;
		}
	}

	return (SndDoImmediate (chan, cmd));
}

// This must be called at task time.
OSErr	CarbonSndPlayDoubleBuffer (SndChannelPtr chan, SndDoubleBufferHeaderPtr theParams) {
	OSErr							err;
	CompressionInfo					compInfo;
	PerChanInfoPtr					perChanInfoPtr;
	SndCommand						playCmd;
	SndCommand						callBack;

	if (nil == chan) {
		err = badChannel;
		goto exit;
	}

	if (nil == theParams) {
		err = paramErr;
		goto exit;
	}

	if (nil == gFreeList) {
		// This can't ever be disposed since we don't know when we might need to use it (at interrupt time)
		gFreeList = (QHdrPtr)NewPtrClear (sizeof (QHdr));
		err = MemError ();
		if (noErr != err) goto exit;
	}

	if (nil == gSilenceOnes) {
		short		i;
		// This can't ever be disposed since we don't know when we might need to use it (at interrupt time)
		gSilenceOnes = NewPtr (kBufSize);
		err = MemError ();
		if (noErr != err) goto exit;
		for (i = 0; i < kBufSize; i++)
			 gSilenceOnes[i] = (char)0x80;
	}

	if (nil == gSilenceTwos) {
		// This can't ever be disposed since we don't know when we might need to use it (at interrupt time)
		gSilenceTwos = NewPtrClear (kBufSize);
		err = MemError ();
		if (noErr != err) goto exit;
	}

	if (nil == gNMRecPtr) {
		// This can't ever be disposed since we don't know when we might need to use it (at interrupt time)
		gNMRecPtr = (NMRecPtr)NewPtr (sizeof (NMRec));
		err = MemError ();
		if (noErr != err) goto exit;

		// Set up our NMProc info that will dispose of most (but not all) of our memory
		gNMRecPtr->qLink = nil;
		gNMRecPtr->qType = 8;
		gNMRecPtr->nmFlags = 0;
		gNMRecPtr->nmPrivate = 0;
		gNMRecPtr->nmReserved = 0;
		gNMRecPtr->nmMark = nil;
		gNMRecPtr->nmIcon = nil;
		gNMRecPtr->nmSound = nil;
		gNMRecPtr->nmStr = nil;
		#if TARGET_API_MAC_CARBON
			gNMRecPtr->nmResp = NewNMUPP(NMResponseProc);
		#else
			gNMRecPtr->nmResp = NewNMProc(NMResponseProc);
		#endif
		gNMRecPtr->nmRefCon = 0;
	}

	perChanInfoPtr = (PerChanInfoPtr)NewPtr (sizeof (PerChanInfo));
	err = MemError ();
	if (noErr != err) goto exit;

	// Init basic per channel information
	perChanInfoPtr->qLink = nil;
	perChanInfoPtr->qType = 0;				// not used
	perChanInfoPtr->stopping = 0;
	#if DEBUG
		perChanInfoPtr->magic = 'SANE';
	#endif
	
	perChanInfoPtr->theParams = theParams;
	// Have to remember the user's callback function from the sound because
	// we are going to overwrite it with our own callback function.
	perChanInfoPtr->usersCallBack = chan->callBack;

	// Set up the sound header for the bufferCmd that will be used to play
	// the buffers passed in by the SndPlayDoubleBuffer call.
	perChanInfoPtr->soundHeader.samplePtr = (Ptr)(theParams->dbhBufferPtr[0]->dbSoundData);
	perChanInfoPtr->soundHeader.numChannels = theParams->dbhNumChannels;
	perChanInfoPtr->soundHeader.sampleRate = theParams->dbhSampleRate;
	perChanInfoPtr->soundHeader.loopStart = 0;
	perChanInfoPtr->soundHeader.loopEnd = 0;
	perChanInfoPtr->soundHeader.encode = cmpSH;
	perChanInfoPtr->soundHeader.baseFrequency = kMiddleC;
	perChanInfoPtr->soundHeader.numFrames = (unsigned long)theParams->dbhBufferPtr[0]->dbNumFrames;
	//	perChanInfoPtr->soundHeader.AIFFSampleRate = 0;				// unused
	perChanInfoPtr->soundHeader.markerChunk = nil;
	perChanInfoPtr->soundHeader.futureUse2 = nil;
	perChanInfoPtr->soundHeader.stateVars = nil;
	perChanInfoPtr->soundHeader.leftOverSamples = nil;
	perChanInfoPtr->soundHeader.compressionID = theParams->dbhCompressionID;
	perChanInfoPtr->soundHeader.packetSize = (unsigned short)theParams->dbhPacketSize;
	perChanInfoPtr->soundHeader.snthID = 0;
	perChanInfoPtr->soundHeader.sampleSize = (unsigned short)theParams->dbhSampleSize;
	perChanInfoPtr->soundHeader.sampleArea[0] = 0;

	// Is the sound compressed?  If so, we need to treat theParams as a SndDoubleBufferHeader2Ptr.
	if (0 != theParams->dbhCompressionID) {
		// Sound is compressed
		err = GetCompressionInfo(theParams->dbhCompressionID,
								((SndDoubleBufferHeader2Ptr)theParams)->dbhFormat,
								theParams->dbhNumChannels,
								theParams->dbhSampleSize,
								&compInfo);
		if (noErr != err) goto exitDispose;

		perChanInfoPtr->soundHeader.format = compInfo.format;
	} else {
		// Sound is not compressed
		perChanInfoPtr->soundHeader.format = kSoundNotCompressed;
	}

	playCmd.cmd = bufferCmd;
	playCmd.param1 = 0;							// unused
	playCmd.param2 = (long)&perChanInfoPtr->soundHeader;

	callBack.cmd = callBackCmd;
	callBack.param1 = 0;						// which buffer to fill, 0 buffer, 1, 0, ...
	callBack.param2 = (long)perChanInfoPtr;

	// Install our callback function pointer straight into the sound channel structure
	if (nil == gCarbonSndPlayDoubleBufferCallBackUPP) {
		#if TARGET_API_MAC_CARBON
			gCarbonSndPlayDoubleBufferCallBackUPP = NewSndCallBackUPP(CarbonSndPlayDoubleBufferCallBackProc);
		#else
			gCarbonSndPlayDoubleBufferCallBackUPP = NewSndCallBackProc(CarbonSndPlayDoubleBufferCallBackProc);
		#endif
	}

	chan->callBack = gCarbonSndPlayDoubleBufferCallBackUPP;

	if (nil == gCarbonSndPlayDoubleBufferCleanUpUPP) {
		#if TARGET_API_MAC_CARBON
			gCarbonSndPlayDoubleBufferCleanUpUPP = NewSndCallBackUPP(CarbonSndPlayDoubleBufferCleanUpProc);
		#else
			gCarbonSndPlayDoubleBufferCleanUpUPP = NewSndCallBackProc(CarbonSndPlayDoubleBufferCleanUpProc);
		#endif
	}

	err = SndDoCommand (chan, &playCmd, true);
	if (noErr != err) goto exitDispose;

	err = SndDoCommand (chan, &callBack, true);
	if (noErr != err) goto exitDispose;

exit:
	return err;

exitDispose:
	if (nil != perChanInfoPtr)
		DisposePtr ((Ptr)perChanInfoPtr);
	goto exit;
}

static pascal void	CarbonSndPlayDoubleBufferCleanUpProc (SndChannelPtr theChannel, SndCommand * theCallBackCmd) {
	PerChanInfoPtr					perChanInfoPtr;

	perChanInfoPtr = (PerChanInfoPtr)(theCallBackCmd->param2);
	#if DEBUG
		if (perChanInfoPtr->magic != 'SANE') DebugStr("\pBAD in CarbonSndPlayDoubleBufferCleanUpProc");
	#endif

	// Put our per channel data on the free queue so we can clean up later
	Enqueue ((QElemPtr)perChanInfoPtr, gFreeList);
	
	// Have to put the user's callback proc back so they get called when the next buffer finishes
	theChannel->callBack = perChanInfoPtr->usersCallBack;
	
	// Have to install our Notification Manager routine so that we can clean up the gFreeList
	if (! OTAtomicSetBit (&gNMRecBusy, 0)) {
		NMInstall (gNMRecPtr);
	}
}


static pascal void	CarbonSndPlayDoubleBufferCallBackProc (SndChannelPtr theChannel, SndCommand * theCallBackCmd) {
	SndDoubleBufferHeaderPtr		theParams;
	SndDoubleBufferPtr				emptyBuf;
	SndDoubleBufferPtr				nextBuf;
	PerChanInfoPtr					perChanInfoPtr;
	SndCommand						playCmd;

	perChanInfoPtr = (PerChanInfoPtr)(theCallBackCmd->param2);
	#if DEBUG
		if (perChanInfoPtr->magic != 'SANE') DebugStr("\pBAD in CarbonSndPlayDoubleBufferCallBackProc");
	#endif
	if (true == perChanInfoPtr->stopping) goto exit;

	theParams = perChanInfoPtr->theParams;

	// The buffer that just played and needs to be filled
	emptyBuf = theParams->dbhBufferPtr[theCallBackCmd->param1];

	// Clear the ready flag
	emptyBuf->dbFlags ^= dbBufferReady;

	// This is the buffer to play now
	nextBuf = theParams->dbhBufferPtr[!theCallBackCmd->param1];

	// Check to see if it is ready, or if we have to wait a bit
	if (nextBuf->dbFlags & dbBufferReady) {
		perChanInfoPtr->soundHeader.numFrames = (unsigned long)nextBuf->dbNumFrames;
		perChanInfoPtr->soundHeader.samplePtr = (Ptr)(nextBuf->dbSoundData);

		// Flip the bit telling us which buffer is next
		theCallBackCmd->param1 = !theCallBackCmd->param1;

		// If this isn't the last buffer, call the user's fill routine
		if (!(nextBuf->dbFlags & dbLastBuffer)) {
			#if TARGET_API_MAC_CARBON
				// Declare a function pointer to the user's double back proc
				doubleBackProcT doubleBackProc;

				// Call user's double back proc
				doubleBackProc = (doubleBackProcT)theParams->dbhDoubleBack;
				(*doubleBackProc) (theChannel, emptyBuf);
			#else
				CallSndDoubleBackProc (theParams->dbhDoubleBack, theChannel, emptyBuf);
			#endif
		} else {
			// Call our clean up proc when the last buffer finishes
			theChannel->callBack = gCarbonSndPlayDoubleBufferCleanUpUPP;
		}
	} else {
		// We have to wait for the buffer to become ready.
		// The real SndPlayDoubleBuffer would play a short bit of silence waiting for
		// the user to read the audio from disk, so that's what we do here.
		#if DEBUG
			DebugStr ("\p buffer is not ready!");
		#endif
		// Play a short section of silence so that we can check the ready flag again
		if (theParams->dbhSampleSize == 8) {
			perChanInfoPtr->soundHeader.numFrames = (UInt32)(kBufSize / theParams->dbhNumChannels);
			perChanInfoPtr->soundHeader.samplePtr = gSilenceOnes;
		} else {
			perChanInfoPtr->soundHeader.numFrames = (UInt32)(kBufSize / (theParams->dbhNumChannels * (theParams->dbhSampleSize / 8)));
			perChanInfoPtr->soundHeader.samplePtr = gSilenceTwos;
		}
	}

	// Insert our callback command
	InsertSndDoCommand (theChannel, theCallBackCmd);

	// Play the next buffer
	playCmd.cmd = bufferCmd;
	playCmd.param1 = 0;
	playCmd.param2 = (long)&(perChanInfoPtr->soundHeader);
	InsertSndDoCommand (theChannel, &playCmd);

exit:
	return;
}

static void	InsertSndDoCommand (SndChannelPtr chan, SndCommand * newCmd) {
	if (-1 == chan->qHead) {
		chan->qHead = chan->qTail;
	}

	if (1 <= chan->qHead) {
		chan->qHead--;
	} else {
		// chan->qHead = chan->qTail;
		chan->qHead = chan->qLength-1; // Fix radar 2944451
	}

	chan->queue[chan->qHead] = *newCmd;
}

static pascal void NMResponseProc (NMRecPtr nmReqPtr) {
	PerChanInfoPtr					perChanInfoPtr;
	OSErr							err;

	NMRemove (nmReqPtr);
	gNMRecBusy = false;

	do {
		perChanInfoPtr = (PerChanInfoPtr)gFreeList->qHead;
		if (nil != perChanInfoPtr) {
			err = Dequeue ((QElemPtr)perChanInfoPtr, gFreeList);
			if (noErr == err) {
				DisposePtr ((Ptr)perChanInfoPtr);
			}
		}
	} while (nil != perChanInfoPtr && noErr == err);
}
