/*
 *  SSLP_limited.cpp
 *
 *  An implementation of the Simple Service Location Protocol as described in SSLP_Protocol.h, conforming
 *	to the API described in SSLP_API.h.
 *
 *  Copyright (c) 2001 Woody Zenfell, III.
 *  woodyzenfell@hotmail.com

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 *  This version does things the easy way, which should support current Aleph One needs, but which means
 *	there are several limitations.
 *  This version is designed to receive processing time from the application's main thread by the
 *      main thread calling SSLP_Pump().  This way, there are no threading issues to worry about.  Of
 *      course, proper operation depends on receiving the processor from time to time while a lookup or
 *      discovery-allowance is in effect.
 *
 *  Created by Woody Zenfell, III on Mon Oct 1 2001, from SSLP_limited_threaded.cpp.

 June 15, 2002 (Loren Petrich):
 	Added packing and unpacking, so as to avoid compiler dependency
        
 Jan 18, 2003 (Woody Zenfell):
        Using A1's Logging system instead of older diagnostic-message schemes

 */

// This stuff (or equivalent) should be widely available...
#include	<assert.h>
#include	<string.h>
#include	<stdlib.h>

// We depend on SDL
#include	<SDL2/SDL.h>
#include	<SDL2/SDL_thread.h>
#include	<SDL2/SDL_endian.h>
#include	"SSLP_API.h"
#include	"SSLP_Protocol.h"

// We use the A1 logging facilities
#include	"Logging.h"
#include	"network.h"
#include	"csmisc.h"

// FILE-LOCAL CONSTANTS
// flags for sBehaviorsDesired (tracks what should be going on)
#define	SSLPINT_NONE		0x00
#define	SSLPINT_LOCATING	0x01
#define	SSLPINT_RESPONDING	0x02

// found service instances expire after not hearing from them for this many milliseconds
#define SSLPINT_INSTANCE_TIMEOUT 20000


// FILE-LOCAL TYPES
struct SSLPint_FoundInstance {
    struct SSLP_ServiceInstance*	mInstance;
    uint64_t				mTimestamp;
    struct SSLPint_FoundInstance*	mNext;
};


// LP: must be initialized!

// FILE-LOCAL (STATIC) STORAGE
////////// used all around
static int						sBehaviorsDesired = 0;
static std::unique_ptr<UDPsocket> sSocketDescriptor;

////////// used by packet receiver
static UDPpacket sReceivingPacket;


////////// for discovering services
static UDPpacket sFindPacket;		// sFindPacket->data does not change
static SSLP_Service_Instance_Status_Changed_Callback	sFoundCallback = NULL;
static SSLP_Service_Instance_Status_Changed_Callback	sLostCallback = NULL;
static SSLP_Service_Instance_Status_Changed_Callback	sNameChangedCallback = NULL;
static struct 	SSLPint_FoundInstance*			sFoundInstances = NULL;


// NB: currently, incoming FIND packets' service_types are compared against
// the service_type in this packet to see if a response is warranted.
// (the service_type in this packet is copied from the instance passed in to Allow_Service_Discovery().)
static UDPpacket sResponsePacket;	// sResponsePacket->data does not change


// Packing and unpacking:
static void PackPacket(unsigned char *Packed, SSLP_Packet *Unpacked);
static void UnpackPacket(unsigned char *Packed, SSLP_Packet *Unpacked);


template<class T> void PacketCopyIn(unsigned char* &Ptr, const T& Value)
{
	int Size = sizeof(T);
	memcpy(Ptr,&Value,Size);
	Ptr += Size;
}

template<class T> void PacketCopyOut(unsigned char* &Ptr, T& Value)
{
	int Size = sizeof(T);
	memcpy(&Value,Ptr,Size);
	Ptr += Size;
}

template<class T> void PacketCopyInList(unsigned char* &Ptr, const T *List, int Count)
{
	int Size = Count*sizeof(T);
	memcpy(Ptr,List,Size);
	Ptr += Size;
}

template<class T> void PacketCopyOutList(unsigned char* &Ptr, T *List, int Count)
{
	int Size = Count*sizeof(T);
	memcpy(List,Ptr,Size);
	Ptr += Size;
}


// FILE-LOCAL (STATIC) FUNCTIONS
// returns a pointer if the instance was "new" - i.e. if we didn't have a record of it.
// caller should notify anyone interested that a new instance was found (and should refer to it by the returned pointer).
// if we already knew about the instance, returns NULL.  in any case, the caller may discard
// the ServiceInstance it passed us (but NOT any we return - that one's ours :) )
static struct SSLP_ServiceInstance*
SSLPint_FoundAnInstance(struct SSLP_ServiceInstance* inInstance) {
    logTrace("Found an instance!  %s, %s, %s:%d", inInstance->sslps_type, inInstance->sslps_name,
            inInstance->sslps_address.address().c_str(), inInstance->sslps_address.port());
    // this should (but doesn't) force string termination to appropriate lengths for type and name.
    
    struct SSLPint_FoundInstance*	theCurrentFoundInstance = sFoundInstances;
    
    while(theCurrentFoundInstance != NULL) {
        if(inInstance->sslps_address == theCurrentFoundInstance->mInstance->sslps_address) {
            // Found a match (would have to check service_type if we handled multiple types, but ok here)
            if(strncmp(theCurrentFoundInstance->mInstance->sslps_name, inInstance->sslps_name, SSLP_MAX_NAME_LENGTH) != 0) {
                // name changed - copy new name and notify
                strncpy(theCurrentFoundInstance->mInstance->sslps_name, inInstance->sslps_name, SSLP_MAX_NAME_LENGTH);
                if(sNameChangedCallback != NULL)
                    sNameChangedCallback(theCurrentFoundInstance->mInstance);
            }
                
            // found a match - update timestamp and break the loop
            theCurrentFoundInstance->mTimestamp = machine_tick_count();
            break;
        }
        theCurrentFoundInstance = theCurrentFoundInstance->mNext;
    }
    
    // no match found - must be a new one!  make new serviceInstance and foundInstance and set up appropriately.
    if(theCurrentFoundInstance == NULL) {
        struct SSLP_ServiceInstance*	theServiceInstance = (struct SSLP_ServiceInstance*) malloc(sizeof(struct SSLP_ServiceInstance));
        memcpy(theServiceInstance, inInstance, sizeof(struct SSLP_ServiceInstance));
        
        theCurrentFoundInstance = (struct SSLPint_FoundInstance*) malloc(sizeof(struct SSLPint_FoundInstance));
        theCurrentFoundInstance->mInstance	= theServiceInstance;
        theCurrentFoundInstance->mTimestamp	= machine_tick_count();
        theCurrentFoundInstance->mNext		= sFoundInstances;
        sFoundInstances				= theCurrentFoundInstance;
        
        return theServiceInstance;
    }
    else
        return NULL;
}



void
SSLPint_RemoveTimedOutInstances() {
    logContext("removing stale SSLP service instances");
    
    struct SSLPint_FoundInstance* theCurrentInstance = sFoundInstances;
    struct SSLPint_FoundInstance* thePreviousInstance = NULL;
    
    uint64_t theCurrentTickCount = machine_tick_count();
    
    while(theCurrentInstance != NULL) {
        if(theCurrentTickCount - theCurrentInstance->mTimestamp > SSLPINT_INSTANCE_TIMEOUT) {
            if(sBehaviorsDesired & SSLPINT_LOCATING) {
                if(sLostCallback != NULL) {
                    sLostCallback(theCurrentInstance->mInstance);
                }
            }

            free(theCurrentInstance->mInstance);
            theCurrentInstance->mInstance = NULL;

            if(thePreviousInstance != NULL)
                thePreviousInstance->mNext = theCurrentInstance->mNext;
            else
                sFoundInstances = theCurrentInstance->mNext;
            
            free(theCurrentInstance);
            
            theCurrentInstance = (thePreviousInstance != NULL) ? thePreviousInstance->mNext : sFoundInstances;
        }
        else {
            thePreviousInstance = theCurrentInstance;
            theCurrentInstance = theCurrentInstance->mNext;
        }
    }
}


// SSLPint_LostAnInstance
// returns a ServiceInstance (which we no longer reference) if we knew about it.
// caller should notify anyone interested, then dispose of the instance we return as well as the one they passed us.
// returns NULL if we didn't know about the instance to begin with.  caller should dispose of the instance
// they passed us.
static struct SSLP_ServiceInstance*
SSLPint_LostAnInstance(struct SSLP_ServiceInstance* inInstance) {
    logTrace("Lost an instance...  %s, %s, %s:%d\n", inInstance->sslps_type, inInstance->sslps_name,
            inInstance->sslps_address.address().c_str(), inInstance->sslps_address.port());
    // this should (but doesn't) force string termination to appropriate lengths for type and name.

    struct SSLPint_FoundInstance* 	theCurrentInstance = sFoundInstances;
    struct SSLPint_FoundInstance* 	thePreviousInstance = NULL;

    struct SSLP_ServiceInstance*	theDoomedInstance = NULL;
    
    while(theCurrentInstance != NULL) {
        if(theCurrentInstance->mInstance->sslps_address == inInstance->sslps_address) {
            theDoomedInstance = theCurrentInstance->mInstance;
                
            theCurrentInstance->mInstance = NULL;

            if(thePreviousInstance != NULL)
                thePreviousInstance->mNext = theCurrentInstance->mNext;
            else
                sFoundInstances = theCurrentInstance->mNext;
            
            free(theCurrentInstance);

            return	theDoomedInstance;
        }
        
        thePreviousInstance = theCurrentInstance;
        theCurrentInstance = theCurrentInstance->mNext;
    }

    return NULL;
}



static void
SSLPint_FlushAllFoundInstances() {
    logContext("flushing out all found SSLP service instances");

    struct SSLPint_FoundInstance*	theInstance;
    
    // Intentional assignment in while() condition.
    while((theInstance = sFoundInstances) != NULL) {
        sFoundInstances = theInstance->mNext;
        
        if(theInstance->mInstance)
            free(theInstance->mInstance);
        
        free(theInstance);
    }
}



static void
SSLPint_ReceivedPacket() {
    logContext("processing a received SSLP packet");
    
    if(sReceivingPacket.data_size != SIZEOF_SSLP_Packet) {
        logNote("packet has wrong len (%d)", sReceivingPacket.data_size);
        return;
    }
    
    struct SSLP_Packet UnpackedReceivedPacket;
    struct SSLP_Packet*	theReceivedPacket = &UnpackedReceivedPacket;
    UnpackPacket(sReceivingPacket.buffer.data(), theReceivedPacket);
    
    if(theReceivedPacket->sslpp_magic != SDL_SwapBE32(SSLPP_MAGIC)) {
        logNote("wrong magic (%x)", SDL_SwapBE32(theReceivedPacket->sslpp_magic));
        return;
    }
    
    if(theReceivedPacket->sslpp_version != SDL_SwapBE32(SSLPP_VERSION)) {
        logNote("packet has wrong version (%d)", SDL_SwapBE32(theReceivedPacket->sslpp_version));
        return;
    }
    
    switch(SDL_SwapBE32(theReceivedPacket->sslpp_message)) {
    case SSLPP_MESSAGE_FIND:
        {
            logContext("dealing with an SSLP FIND request");
    
            // Someone is looking for services...
            if(sBehaviorsDesired & SSLPINT_RESPONDING) {
                    struct SSLP_Packet UnpackedResponsePacket;
                            struct SSLP_Packet*	theResponsePacket = &UnpackedResponsePacket;
                    UnpackPacket(sResponsePacket.buffer.data(), theResponsePacket);
                
                // We have a service we want discovered...
                if(strncmp(theReceivedPacket->sslpp_service_type,
                    theResponsePacket->sslpp_service_type,/* sDiscoverableService->sslps_type, */
                        SSLP_MAX_TYPE_LENGTH) == 0) {
                    
                    // We have a service of the same type they are looking for.  Let's tell them about us.
                    // Fortunately, we have a packet all ready to go just for this very purpose!  ;)
                    sResponsePacket.address = sReceivingPacket.address;
                    sSocketDescriptor->send(sResponsePacket);                    
                    logTrace("tried to send response");
                }
                else
                {
                    logNote("type mismatch (%s != %s)", theReceivedPacket->sslpp_service_type, theResponsePacket->sslpp_service_type);
                    // note: this printf does not clamp string at 32 chars (i.e. max length in packet)
                }
            }
            else
                logTrace("we are not responding to FIND requests");
    
            return;
        }
    break;
    case SSLPP_MESSAGE_HAVE:
        {
            logContext("processing an SSLP HAVE message");
    
            // Someone reports having an instance of some kind of service type!
            if(sBehaviorsDesired & SSLPINT_LOCATING) {
                // ... ok, and we're interested... so, let's make sure it's the service_type that we're looking for...
                
                    struct SSLP_Packet UnpackedFindPacket;
                            struct SSLP_Packet*	theFindPacket = &UnpackedFindPacket;
                    UnpackPacket(sFindPacket.buffer.data(), theFindPacket);
                    
            if(strncmp(theReceivedPacket->sslpp_service_type, theFindPacket->sslpp_service_type,
                    SSLP_MAX_TYPE_LENGTH) == 0) {
    
                    // It's the right type!  We found an instance out there!  Set up a structure to report our findings.
                    struct SSLP_ServiceInstance	theReceivedInstance;
                    strncpy(theReceivedInstance.sslps_type, theReceivedPacket->sslpp_service_type, SSLP_MAX_TYPE_LENGTH);
                    strncpy(theReceivedInstance.sslps_name, theReceivedPacket->sslpp_service_name, SSLP_MAX_NAME_LENGTH);
                    theReceivedInstance.sslps_address.set_address(sReceivingPacket.address.address());
                    theReceivedInstance.sslps_address.set_port(theReceivedPacket->sslpp_service_port);
                    
                    // Report our findings to the "instance librarian".
                    struct SSLP_ServiceInstance* theReturnedInstance = SSLPint_FoundAnInstance(&theReceivedInstance);
                    
                    // Maybe this is old news (if so, it returns NULL)
                    if(theReturnedInstance != NULL) {
                        // It didn't know about this instance, and has returned a pointer to its (new) permanent copy.  Spread the word.
                        if(sFoundCallback != NULL) {
                            sFoundCallback(theReturnedInstance);
                        }
                        else
                            logNote("no 'found instance' callback registered");
                    }
                    else
                        logTrace("service already known");
                }
                else
                    logNote("wrong service type (%s != %s)", theReceivedPacket->sslpp_service_type, theFindPacket->sslpp_service_type);
                    // note we don't stop the string at SSLP_MAX_NAME_LENGTH as we should.
            }
            else
                logTrace("we are not currently locating instances");

            return;
        }
    break;
    case SSLPP_MESSAGE_LOST:
        {
            logContext("processing an SSLP LOST message");
            
            // Someone reports having lost an instance of some kind of service type.
            if(sBehaviorsDesired & SSLPINT_LOCATING) {
                // ... ok, and we're interested... so, let's make sure it's the service_type that we're looking for...
                
                    struct SSLP_Packet UnpackedFindPacket;
                            struct SSLP_Packet*	theFindPacket = &UnpackedFindPacket;
                    UnpackPacket(sFindPacket.buffer.data(), theFindPacket);
                    
            if(strncmp(theReceivedPacket->sslpp_service_type, theFindPacket->sslpp_service_type,
                    SSLP_MAX_TYPE_LENGTH) == 0) {
    
                    // It's the right type.  Set up a structure to report our findings.
                    struct SSLP_ServiceInstance	theReceivedInstance;
                    strncpy(theReceivedInstance.sslps_type, theReceivedPacket->sslpp_service_type, SSLP_MAX_TYPE_LENGTH);
                    strncpy(theReceivedInstance.sslps_name, theReceivedPacket->sslpp_service_name, SSLP_MAX_NAME_LENGTH);
                    theReceivedInstance.sslps_address.set_address(sReceivingPacket.address.address());
                    theReceivedInstance.sslps_address.set_port(theReceivedPacket->sslpp_service_port);
                    
                    // Report our findings to the "instance librarian".
                    struct SSLP_ServiceInstance*	theReturnedInstance = SSLPint_LostAnInstance(&theReceivedInstance);
                    
                    // It returns NULL if it had no record of that instance to begin with.
                    if(theReturnedInstance != NULL) {
                        // This was news to it.  It has surrendered its pointer to its permanent copy.  Spread the word.
                        if(sLostCallback != NULL) {
                            sLostCallback(theReturnedInstance);
                        }
                        
                        // And now, we kill the instance record.
                        free(theReturnedInstance);
                    }
                }
            }

            return;
        }
    break;
    default:
        logNote("unknown SSLP message type (%x)", SDL_SwapBE32(theReceivedPacket->sslpp_message));

        return;
    break;
    }
}



// SSLPint_Enter - called by API functions if SSLP is inactive
// set up shared resources for lookups and allowing discovery
static bool
SSLPint_Enter() {
    logContext("setting up SSLP");

    sSocketDescriptor = NetGetNetworkInterface()->udp_open_socket(SSLP_PORT);
    if (!sSocketDescriptor) return false;

    if (!sSocketDescriptor->broadcast(true))
        return false;
    
    // (note: if EnableBroadcast failed, it's not the end of the world... but it will be harder to locate services)
    assert(sBehaviorsDesired == SSLPINT_NONE);
    
    // Success
    return true;
}



// SSLPint_Exit - called by API functions if SSLP is no longer needed
// break down shared resources
static int
SSLPint_Exit() {
    logContext("shutting down SSLP");
    assert(sBehaviorsDesired == SSLPINT_NONE);
    sSocketDescriptor.reset();
    return 0;
}




// API FUNCTION DEFINITIONS
// note that all externally-visible functions should be called by the main thread, including SSLP_Pump which does the actual work.
void
SSLP_Locate_Service_Instances(const char* inServiceType, SSLP_Service_Instance_Status_Changed_Callback inFoundCallback,
                                SSLP_Service_Instance_Status_Changed_Callback inLostCallback,
                                SSLP_Service_Instance_Status_Changed_Callback inNameChangedCallback) {

    logContext("starting to locate SSLP service instances");

    assert(inServiceType);
    
    assert(!(sBehaviorsDesired & SSLPINT_LOCATING));
    
    if(!sBehaviorsDesired)	// SSLP is not active at all yet...
        if(!SSLPint_Enter())	// try to activate.  on error, bail
            return;
        
    sFoundCallback		= inFoundCallback;
    sLostCallback		= inLostCallback;
    sNameChangedCallback	= inNameChangedCallback;

    SSLP_Packet UnpackedPacket;
    SSLP_Packet *theFindPacket = &UnpackedPacket;
    
    // set up the "FIND" packet
    sFindPacket.address.set_port(SSLP_PORT);
    sFindPacket.data_size = sizeof(struct SSLP_Packet);

    theFindPacket->sslpp_magic		= SDL_SwapBE32(SSLPP_MAGIC);
    theFindPacket->sslpp_version	= SDL_SwapBE32(SSLPP_VERSION);
    theFindPacket->sslpp_message	= SDL_SwapBE32(SSLPP_MESSAGE_FIND);
    theFindPacket->sslpp_service_port	= 0;				// (service_port is unused in FIND)
    theFindPacket->sslpp_reserved	= 0;				// unused - set to 0
    // note: my strncpy states it fills remaining buffer with 0.  If yours doesn't, fill it yourself.
    strncpy(theFindPacket->sslpp_service_type, inServiceType, SSLP_MAX_TYPE_LENGTH);
	// bzero(theFindPacket->sslpp_service_name, sizeof(theFindPacket->sslpp_service_name));	// (service_name is unused in FIND)

	// Hmm, maybe memset is more widely available than bzero.
	memset(theFindPacket->sslpp_service_name, 0, SSLP_MAX_NAME_LENGTH);
	
    // Allow receiving code to process incoming HAVE messages, and allow "find" broadcaster to broadcast.
    sBehaviorsDesired		|= SSLPINT_LOCATING;
    
    // Load into the "real" packet
    PackPacket(sFindPacket.buffer.data(), theFindPacket);
}


void
SSLP_Stop_Locating_Service_Instances(const char* inServiceType) {
    // We ignore inServiceType since we only track one service at a time for now
    // truly, semantics should be: pointer == NULL, stop all location;  pointer == p, stop locating p.
    
    logContext("ceasing attempts to locate SSLP service instances");

    assert(sBehaviorsDesired & SSLPINT_LOCATING);

    // Indicate we no longer want finding code to run
    sBehaviorsDesired &= ~SSLPINT_LOCATING;

    sFoundCallback		= NULL;
    sLostCallback		= NULL;
    sNameChangedCallback	= NULL;

    SSLPint_FlushAllFoundInstances();

    // If all SSLP services are done, clean up more
    if(sBehaviorsDesired == SSLPINT_NONE)
        SSLPint_Exit();
}



void
SSLP_Allow_Service_Discovery(const struct SSLP_ServiceInstance* inServiceInstance) {
    logContext("starting to allow SSLP service discovery");

    assert(inServiceInstance != NULL);
    
    assert(!(sBehaviorsDesired & SSLPINT_RESPONDING));
    
    if(sBehaviorsDesired == SSLPINT_NONE)
        if(!SSLPint_Enter())
            return;

    SSLP_Packet UnpackedPacket;
    SSLP_Packet* theResponsePacket = &UnpackedPacket;
    
    // set up the "HAVE" packet
    sResponsePacket.address.set_port(SSLP_PORT); // port is used for initial broadcast, though.
    sResponsePacket.data_size = sizeof(struct SSLP_Packet);

    theResponsePacket->sslpp_magic		= SDL_SwapBE32(SSLPP_MAGIC);
    theResponsePacket->sslpp_version		= SDL_SwapBE32(SSLPP_VERSION);
    theResponsePacket->sslpp_message		= SDL_SwapBE32(SSLPP_MESSAGE_HAVE);
    theResponsePacket->sslpp_service_port	= inServiceInstance->sslps_address.port();
    theResponsePacket->sslpp_reserved		= 0;				// unused - set to 0
    // note: my strncpy states it fills remaining buffer with 0.  If yours doesn't, fill it yourself.
    strncpy(theResponsePacket->sslpp_service_type, inServiceInstance->sslps_type, SSLP_MAX_TYPE_LENGTH);
    strncpy(theResponsePacket->sslpp_service_name, inServiceInstance->sslps_name, SSLP_MAX_NAME_LENGTH);

    // Load into the "real" packet
    PackPacket(sResponsePacket.buffer.data(), theResponsePacket);
    
    // Broadcast the HAVE once to speed things up, maybe
    sSocketDescriptor->broadcast_send(sResponsePacket);
    
    // Allow receiving code to respond to incoming FIND messages
    sBehaviorsDesired		|= SSLPINT_RESPONDING;
}


void
SSLP_Disallow_Service_Discovery(const struct SSLP_ServiceInstance* inInstance) {
    // Officially, we would walk through a list to find the right one to disallow (or, if NULL is passed in,
    // we'd walk through to disallow all).  For now, since we're cheating, we assume they want to disallow
    // the one instance that could be discovered, and stop that one (without even looking at what they passed).

    logContext("disallowing SSLP service discovery");

    assert(sBehaviorsDesired & SSLPINT_RESPONDING);

    // Indicate we no longer want to allow discovery
    sBehaviorsDesired &= ~SSLPINT_RESPONDING;

    // Broadcast a LOST packet, as a courtesy
	struct SSLP_Packet UnpackedResponsePacket;
	struct SSLP_Packet*	theResponsePacket = &UnpackedResponsePacket;
    UnpackPacket(sResponsePacket.buffer.data(), theResponsePacket);
    		
    theResponsePacket->sslpp_message = SDL_SwapBE32(SSLPP_MESSAGE_LOST);
    PackPacket(sResponsePacket.buffer.data(), theResponsePacket);
    sResponsePacket.address.set_port(SSLP_PORT);

    sSocketDescriptor->broadcast_send(sResponsePacket);

    // If all SSLP services are done, clean up more...
    if(sBehaviorsDesired == SSLPINT_NONE)
        SSLPint_Exit();
}



// Call this function every once in a while to allow SSLP processing to occur
void
SSLP_Pump() {
    // Do nothing if we're supposed to do nothing :)
    if(sBehaviorsDesired == SSLPINT_NONE)
        return;
        
    logContext("pumping SSLP protocol activity");
    
    static uint64_t	theTimeLastWorked = 0;
    
    uint64_t		theCurrentTime = machine_tick_count();
    
    if(sBehaviorsDesired & SSLPINT_LOCATING) {

        // Do some work only once every five seconds
        if(theCurrentTime - theTimeLastWorked >= 5000) {

            // Do broadcasting work
            sSocketDescriptor->broadcast_send(sFindPacket);
            SSLPint_RemoveTimedOutInstances();

            theTimeLastWorked = theCurrentTime;
        }
    }
    
    // Do receiving work every time
    if(sBehaviorsDesired & (SSLPINT_LOCATING | SSLPINT_RESPONDING)) {
        while (sSocketDescriptor->check_receive() > 0 && sSocketDescriptor->receive(sReceivingPacket) > 0) {
            SSLPint_ReceivedPacket();
        }
    }
}


// Packing and unpacking
void PackPacket(unsigned char *Packed, SSLP_Packet *Unpacked)
{
	PacketCopyIn(Packed,Unpacked->sslpp_magic);
	PacketCopyIn(Packed,Unpacked->sslpp_version);
	PacketCopyIn(Packed,Unpacked->sslpp_message);
	PacketCopyIn(Packed,Unpacked->sslpp_service_port);
	PacketCopyIn(Packed,Unpacked->sslpp_reserved);
	PacketCopyInList(Packed,Unpacked->sslpp_service_type,SSLP_MAX_TYPE_LENGTH);
	PacketCopyInList(Packed,Unpacked->sslpp_service_name,SSLP_MAX_NAME_LENGTH);
}

void UnpackPacket(unsigned char *Packed, SSLP_Packet *Unpacked)
{
	PacketCopyOut(Packed,Unpacked->sslpp_magic);
	PacketCopyOut(Packed,Unpacked->sslpp_version);
	PacketCopyOut(Packed,Unpacked->sslpp_message);
	PacketCopyOut(Packed,Unpacked->sslpp_service_port);
	PacketCopyOut(Packed,Unpacked->sslpp_reserved);
	PacketCopyOutList(Packed,Unpacked->sslpp_service_type,SSLP_MAX_TYPE_LENGTH);
	PacketCopyOutList(Packed,Unpacked->sslpp_service_name,SSLP_MAX_NAME_LENGTH);
}

