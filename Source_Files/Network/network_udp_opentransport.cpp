/*
    network_udp_opentransport.cpp

    Attempting to bring IPring in-game ring protocol to Classic via OpenTransport
    (will still use SDL_net/SSLP for service location and "streaming" data)

	Copyright (C) 2003 and beyond by Woody Zenfell, III
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

	Jan. 10, 2003 (Woody Zenfell): created

	May 18, 2003 (Woody Zenfell): now uses passed-in port number for local socket
*/


#include "cseries.h"
#include "sdl_network.h"	// trust me, you don't even want to ask...
#include "network_private.h"	// for kPROTOCOL_TYPE
#include "Logging.h"

// ZZZ: guarding with MACH since building this in the Carbon version on Mac OS X
// doesn't like these #includes
#ifndef __MACH__
// LP: had to do these includes to satisfy CodeWarrior's Classic support
#include <OpenTransport.h>
#include <OpenTransportProviders.h>
#endif // __MACH__

// ZZZ: Can define this symbol in a pinch for some additional debug logging.
// Don't do it in most cases because Logging is not thread-safe yet.
//#define UNSAFE_OTUDP_LOGGING

static const int kReserved	= 0;
static const int kUnused	= 0;

static EndpointRef		sEndpoint	= kOTInvalidEndpointRef;
static InetAddress		sBoundAddress;
static InetAddress		sIncomingAddress;
static TUnitData		sIncomingUnitData = {	{ sizeof(sIncomingAddress), sizeof(sIncomingAddress), (UInt8*)&sIncomingAddress }, // addr
                                                        { 0, 0, NULL }, // opt
                                                        { 0, 0, NULL }  // data
                                                    };
static PacketHandlerProcPtr	sPacketHandler	= NULL;
static DDPPacketBuffer		sPacketBufferForPacketHandler;
static OTNotifyUPP		sNotifierUPP;

// Make sure OT is ready
OSErr
NetDDPOpen() {
    OSStatus theResult;
/*
#ifndef __MACH__
    // This seems to not exist in Mac OS X but is probably needed for Mac OS 9 etc.
    // Not sure what preprocessor symbol to test against.
    theResult = InitOpenTransport();
#else
*/
    theResult = InitOpenTransportInContext(kInitOTForApplicationMask, NULL);
    if(theResult == kEINVALErr)	// seems to return this if I init twice - not sure about in Mac OS 9
        theResult = noErr;
//#endif
    
    // According to Inside Mac: Networking with OT, this will automatically patch ExitToShell
    // to make sure CloseOpenTransport() is called.
    // Also, the docs state that if we're using the Apple Shared Library Manager (ASLM), we need
    // to call InitLibraryManager() before calling InitOpenTransport().
    
    // I'm guessing here.  Indeed, it seems OT is probably already going to be initialized
    // by the time we get here since SSLP and the streaming stuff use SDL_net which uses OT.
    // The Apple docs make no statement as to whether multiple initializations are safe.
    
    if(theResult != noErr)
        logError1("cannot initialize Open Transport: returned %d", theResult);

    return theResult;
}



// Nothing special to do
OSErr
NetDDPClose() {
    return noErr;
}



static void
handleDataArrival() {
    OTFlags	theFlags; // currently ignored
    
    // We don't receive directly into the sPacketBufferForPacketHandler in case a spurious T_DATA encourages us to overwrite
    // that data with garbage.  (So we wait to see that it seems like real data before overwriting with copied data.)
    OSStatus theResult = OTRcvUData(sEndpoint, &sIncomingUnitData, &theFlags);
    while(theResult == noErr) {
        if(sIncomingUnitData.udata.len > 0) {
            assert(sIncomingUnitData.udata.len <= ddpMaxData);	// not really safe to assert() here probably, but need to catch this somehow
            memcpy(sPacketBufferForPacketHandler.datagramData, sIncomingUnitData.udata.buf, sIncomingUnitData.udata.len);
            sPacketBufferForPacketHandler.datagramSize = sIncomingUnitData.udata.len;
            sPacketBufferForPacketHandler.protocolType = kPROTOCOL_TYPE;
            InetAddress* theSourceAddress = (InetAddress*)(sIncomingUnitData.addr.buf);
            assert(theSourceAddress->fAddressType = AF_INET);
            sPacketBufferForPacketHandler.sourceAddress.host = theSourceAddress->fHost;
            sPacketBufferForPacketHandler.sourceAddress.port = theSourceAddress->fPort;

            if(sPacketHandler != NULL)
                sPacketHandler(&sPacketBufferForPacketHandler);
        }
        theResult = OTRcvUData(sEndpoint, &sIncomingUnitData, &theFlags);
    }
#ifdef UNSAFE_OTUDP_LOGGING
    if(theResult != kOTNoDataErr)
        logAnomaly1("OTRcvUData returned %d", theResult);
#endif
}



// This code is called (potentially at "deferred task time") by OT whenever interesting stuff happens.
static pascal void
udpNotifier(void* inContext, OTEventCode inEventCode, OTResult inResult, void* cookie) {
#ifdef UNSAFE_OTUDP_LOGGING
    // This is fairly unsafe (calling this routine from deferred task time), probably...
    // but if it works even just for now, it can help us understand why networking isn't working right in Classic.
    logTrace2("udpNotifier called with eventCode=0x%x result=0x%x", inEventCode, inResult);
#endif

    switch(inEventCode) {
        case T_UDERR:
            // Need to clear out error even if we don't care about it
            OTRcvUDErr(sEndpoint, NULL);
        break;
        
        case T_GODATA:
            // flow-control has lifted, clear to send data
        break;
        
        case T_DATA:
            handleDataArrival();
        break;
    }
}



// Open the socket ("endpoint provider" in OTspeak)
OSErr
NetDDPOpenSocket(short* ioPortNumber, PacketHandlerProcPtr inPacketHandler) {
    TEndpointInfo	theEndpointInfo;
    OSStatus		theResult;

    // Synchronously create the endpoint
/*
#ifndef __MACH__
    sEndpoint = OTOpenEndpoint(OTCreateConfiguration(kUDPName), kReserved, &theEndpointInfo, &theResult);
#else
*/
    sEndpoint = OTOpenEndpointInContext(OTCreateConfiguration(kUDPName), kReserved, &theEndpointInfo, &theResult, NULL);
//#endif
    
    if(theResult != noErr) {
        logError1("NetDDPOpenSocket: OTOpenEndpoint error (%d)", theResult);
        return theResult;
    }
    
    // Endpoint now is synchronous, nonblocking, and does not acknowledge sends
    // (i.e. it will copy sent data into its own buffer so we can reuse ours)
    
    // Allocate storage for packet
    if(theEndpointInfo.tsdu <= 0) {
        logError1("NetDDPOpenSocket: endpoint tsdu nonpositive (%d)", theEndpointInfo.tsdu);
        theResult = -1;
        goto close_and_return;
    }
    
    if(sIncomingUnitData.udata.buf == NULL) {
        sIncomingUnitData.udata.buf = new UInt8[theEndpointInfo.tsdu];
        sIncomingUnitData.udata.maxlen = theEndpointInfo.tsdu;
        
        if(sIncomingUnitData.udata.buf == NULL) {
            logError1("NetDDPOpenSocket: could not allocate %d bytes for sPacketBuffer", theEndpointInfo.tsdu);
            goto close_and_return;
        }
    }
    else
        logNote("NetDDPOpenSocket: packet buffer already allocated?");

    // Bind the endpoint
    InetAddress theDesiredAddress;
    theDesiredAddress.fAddressType	= AF_INET;
    theDesiredAddress.fPort		= *ioPortNumber;
    theDesiredAddress.fHost		= kOTAnyInetAddress;
    obj_clear(theDesiredAddress.fUnused);

    TBind	theDesiredAddressBind;
    theDesiredAddressBind.addr.buf	= (UInt8*)&theDesiredAddress;
    theDesiredAddressBind.addr.len	= sizeof(theDesiredAddress);
    theDesiredAddressBind.addr.maxlen	= sizeof(theDesiredAddress);
    theDesiredAddressBind.qlen = kUnused;
    
    obj_clear(sBoundAddress);

    TBind	theActualAddressBind;
    theActualAddressBind.addr.buf	= (UInt8*)&sBoundAddress;
    theActualAddressBind.addr.len	= sizeof(sBoundAddress);
    theActualAddressBind.addr.maxlen	= sizeof(sBoundAddress);

    theResult = OTBind(sEndpoint, &theDesiredAddressBind, &theActualAddressBind);
    
    if(theResult != noErr) {
        logError1("NetDDPOpenSocket: OTBind error (%d)", theResult);
        goto dealloc_close_and_return;
    }
    
    // Switch to blocking mode
    theResult = OTSetBlocking(sEndpoint);
    
    if(theResult != noErr) {
        logError1("NetDDPOpenSocket: OTSetBlocking error (%d)", theResult);
        goto unbind_dealloc_close_and_return;
    }
    
    // Switch to asynchronous mode
    theResult = OTSetAsynchronous(sEndpoint);
    
    if(theResult != noErr) {
        logError1("NetDDPOpenSocket: OTSetAsynchronous error (%d)", theResult);
        goto unbind_dealloc_close_and_return;
    }
    
    // Keep reference to caller's desired packet-handler
    sPacketHandler = inPacketHandler;
    
    // Install our notifier
    sNotifierUPP = NewOTNotifyUPP(udpNotifier);
    theResult = OTInstallNotifier(sEndpoint, sNotifierUPP, kUnused);
    
    if(theResult != noErr) {
        logError1("NetDDPOpenSocket: OTInstallNotifier error (%d)", theResult);
        goto unbind_dealloc_close_and_return;
    }
    
    // XXX what if data arrived after we bound but before we installed the notifier?
    // Will we still get a T_DATA message?  Will we when the next packet arrives?
    // Maybe we should call the notifier directly with T_DATA if we somehow detect that there
    // is, in fact, data.
    
    // How about this?  Is this reasonably safe?
    OTEnterNotifier(sEndpoint);
    handleDataArrival();
    OTLeaveNotifier(sEndpoint);
    
    // XXX how is tearing down the connection complicated by async blocking mode?
    
    // Return our port number to caller
    *ioPortNumber = sBoundAddress.fPort;
    
    return theResult;

unbind_dealloc_close_and_return:
    OTUnbind(sEndpoint);
    
    // fall through

dealloc_close_and_return:
    delete [] sIncomingUnitData.udata.buf;
    sIncomingUnitData.udata.buf = NULL;

    // fall through

close_and_return:
    OTCloseProvider(sEndpoint);
    sEndpoint = kOTInvalidEndpointRef;
    
    return theResult;
}
    


OSErr
NetDDPCloseSocket(short socketNumber) {
    if(sEndpoint != kOTInvalidEndpointRef) {
        // This is pretty rudimentary (optimistic) currently...
        //assert(socketNumber == kSocketNum);
    
        OTRemoveNotifier(sEndpoint);
        
        DisposeOTNotifyUPP(sNotifierUPP);
        
        OTSetSynchronous(sEndpoint);
        
        OTSetNonBlocking(sEndpoint);
    
        OTUnbind(sEndpoint);
        
        delete [] sIncomingUnitData.udata.buf;
        sIncomingUnitData.udata.buf = NULL;
    
        OTCloseProvider(sEndpoint);
        sEndpoint = kOTInvalidEndpointRef;
    }
    
    return noErr;
}



DDPFramePtr
NetDDPNewFrame(void) {
    DDPFramePtr theFrame = new DDPFrame;
    
    if(theFrame != NULL) {
        obj_clear(*theFrame);
    }
    
    return theFrame;
}



void
NetDDPDisposeFrame(DDPFramePtr inFrame) {
    delete inFrame;
}



OSErr
NetDDPSendFrame(DDPFramePtr inFrame, NetAddrBlock* inAddress, short inProtocolType, short inSocket) {
//    assert(inSocket == kSocketNum);
//    assert(inFrame->socket == kSocketNum);
    assert(inProtocolType == kPROTOCOL_TYPE);
    assert(inFrame->data_size <= ddpMaxData);
    
    InetAddress theAddress;
    theAddress.fAddressType = AF_INET;
    theAddress.fPort = inAddress->port;
    theAddress.fHost = inAddress->host;
    
    TUnitData theOutgoingData;
    theOutgoingData.addr.buf = (UInt8*)&theAddress;
    theOutgoingData.addr.len = sizeof(theAddress);
    theOutgoingData.opt.len = 0;
    theOutgoingData.udata.buf = inFrame->data;
    theOutgoingData.udata.len = inFrame->data_size;

    OSErr theResult = OTSndUData(sEndpoint, &theOutgoingData);
#ifdef UNSAFE_OTUDP_LOGGING
    if(theResult != noErr)
        logAnomaly1("NetDDPSendFrame: OTSndUData returned %d", theResult);
#endif
    return theResult;
}
