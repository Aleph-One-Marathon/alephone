/*
	File:		CarbonSndPlayDB.h
	
	Description:Headers for routines demonstrating how to create a function that works
				much like the original SndPlayDoubleBuffer but is Carbon compatible
				(which SndPlayDoubleBuffer isn't).

	Author:		MC, ERA

	Copyright: 	© Copyright 1999-2001 Apple Computer, Inc. All rights reserved.
	
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
				
	Change History (most recent first):

*/

#ifdef TARGET_API_MAC_CARBON
// LP: so it will be OK with CodeWarrior
#ifndef __MWERKS__
#include <sys/cdefs.h>
#else
#define __BEGIN_DECLS
#define __END_DECLS
#endif
struct SndDoubleBuffer {
  long                dbNumFrames;
  long                dbFlags;
  long                dbUserInfo[2];
  SInt8               dbSoundData[1];
};
typedef struct SndDoubleBuffer          SndDoubleBuffer;
typedef SndDoubleBuffer *               SndDoubleBufferPtr;

typedef CALLBACK_API( void , SndDoubleBackProcPtr )(SndChannelPtr channel, SndDoubleBufferPtr doubleBufferPtr);
typedef STACK_UPP_TYPE(SndDoubleBackProcPtr)	SndDoubleBackUPP;
// ZZZ fix(?):  the dbhDoubleBack member should really be the following type, in Carbon.  I think.
typedef void (*doubleBackProcT)(SndChannel*, SndDoubleBuffer*);


struct SndDoubleBufferHeader {
  short               dbhNumChannels;
  short               dbhSampleSize;
  short               dbhCompressionID;
  short               dbhPacketSize;
  UnsignedFixed       dbhSampleRate;
  SndDoubleBufferPtr  dbhBufferPtr[2];
  SndDoubleBackUPP    dbhDoubleBack;
};
typedef struct SndDoubleBufferHeader    SndDoubleBufferHeader;
typedef SndDoubleBufferHeader *         SndDoubleBufferHeaderPtr;

struct SndDoubleBufferHeader2 {
  short               dbhNumChannels;
  short               dbhSampleSize;
  short               dbhCompressionID;
  short               dbhPacketSize;
  UnsignedFixed       dbhSampleRate;
  SndDoubleBufferPtr  dbhBufferPtr[2];
  SndDoubleBackUPP    dbhDoubleBack;
  OSType              dbhFormat;
};
typedef struct SndDoubleBufferHeader2   SndDoubleBufferHeader2;
typedef SndDoubleBufferHeader2 *        SndDoubleBufferHeader2Ptr;

enum {
  dbBufferReady                 = 0x00000001, /*double buffer is filled*/
  dbLastBuffer                  = 0x00000004  /*last double buffer to play*/
};

#endif // TARGET_API_MAC_CARBON

#pragma export on
__BEGIN_DECLS
				OSErr	CarbonSndPlayDoubleBuffer (SndChannelPtr chan, SndDoubleBufferHeaderPtr theParams);
				OSErr	MySndDoImmediate (SndChannelPtr chan, SndCommand * cmd);
__END_DECLS
#pragma export off
