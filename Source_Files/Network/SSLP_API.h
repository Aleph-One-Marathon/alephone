/*
 *  SSLP_API.h - applications include this to use SSLP services.
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

 *  Simple Service Location Protocol (hope that name's ok with everyone, did not research it at all)
 *	(Near-)conflict with well-known Secure Socket Layer is unfortunate, but I doubt anyone will mistake them :) .
 *	Someone smart has surely done this better somewhere and has probably proven various characteristics,
 *	but I decided to hack out my own anyway.  Tough.
 *
 *	The current implementation (and a small amount of the interface) relies on the SDL_net cross-platform
 *	IP networking library.  The current implementation (and no part of the interface) relies on my own
 *	"SDL_netx" UDP broadcast companion to SDL_net, as well as some other features of SDL.
 *
 *	for the Aleph One project, to do NBP-ish player location on non-AppleTalk networks.  The current
 *	implementation is a (significant) simplification of what this could or perhaps ought to be, but
 *	it's sufficient for Aleph One's needs, which is all that matters right now.  Besides, like I said,
 *	I'm sure there's something better already out there that's designed for heavyweight application.
 *
 *  Created by Woody Zenfell, III on Tue Sep 11 2001.
 */

#ifndef SSLP_API_H
#define	SSLP_API_H

#include	<SDL_net.h>

// SSLP does not "guarantee" anything about its findings - it's intended merely as an aid.  This means (in particular)
// a host that found a service via SSLP cannot assume that the service definitely exists at the host and port provided...
// but it's a probably a good place to look ;)

// Note: here's one sticky part, at the moment, for the sake of laziness, I share these constants between
// the ServiceInstance (part of the API) and the SSLP_Packet network data format.  Really, they should be
// disentangled, but then I'd have to worry about them being different.  ;)
#define	SSLP_MAX_TYPE_LENGTH	32
#define	SSLP_MAX_NAME_LENGTH	32

// SSLP_ServiceInstance does not get written to network or file, so struct alignment no big whoop
// sslps_address must have both parts in network (big-endian) byte order, though.
struct SSLP_ServiceInstance {
    char		sslps_type[SSLP_MAX_TYPE_LENGTH];
    char		sslps_name[SSLP_MAX_NAME_LENGTH];
    IPaddress		sslps_address;	// "host" part ignored by SSLP_*_Service_Discovery()
};


// N.B.!!: currently, for ease of implementation, only one service can be searched for and only one service can
// be made discoverable at a time.  (You may do one of each simultaneously.)
// No wildcard or regex matches are supported at this time.


////// General functions //////

// If using a non-threaded implementation, you will need to be sure to call this function every once in a while
// to give SSLP some processing time.  Once a second ought to be plenty, but more or less often should also work.
// More frequent calls result in more responsive updates.  Try to avoid taking more than 5 seconds between calls.
// In a threaded implementation, threads will worry about themselves, and you never need to call SSLP_Pump().
void
SSLP_Pump();



////// Trying to locate remote services //////

// SSLP_Locate_Service_Instances: Starts looking for service instances of inServiceType.
// If one is found, inFoundCallback is invoked immediately (potentially in a different thread than was used to call
// Locate_Service_Instances).  Consider the ServiceInstance* to be valid until noted otherwise... you may store a reference
// (no need to copy).  You won't get another foundCallback for the same instance.
// If one is lost, inLostCallback is invoked immediately (again, potentially in a different thread).  You won't get
// a lostCallback unless there had been a matching foundCallback (err, unless you gave NULL for foundCallback, which would
// probably not be very useful).
// Once inLostCallback returns, consider the ServiceInstance* invalid (i.e. hope you copied anything you needed).
// If a service is found with a different name but at the same host and port as an existing one, inNameChangedCallback is invoked.
// (you guessed it... from a different thread.)
// Any Callback(s) may be NULL, in which case you receive no notification of the corresponding event.
// The ServiceInstance pointer may be used to *identify* a service instance, while it's valid... that is,
// no need to "deep compare" the whole records.  In particular, inLostCallback and inNameChangedCallback will get
// a pointer that was previously passed to inFoundCallback.
// NB!! in the current (limited) implementation, you may only have one service-location attempt ongoing at a time.
// Calling this function twice without an intervening call to Stop_Locating_Service_Instances is an ERROR, and may
// result in the termination of the entire application!

// One more note: in the single-threaded SSLP implementation, callbacks will occur in whatever thread calls
// SSLP_Pump(), so synchronization is much less of an issue.
typedef	void (*SSLP_Service_Instance_Status_Changed_Callback)(const SSLP_ServiceInstance* inService);

void
SSLP_Locate_Service_Instances(const char* inServiceType, SSLP_Service_Instance_Status_Changed_Callback inFoundCallback,
        SSLP_Service_Instance_Status_Changed_Callback inLostCallback,
        SSLP_Service_Instance_Status_Changed_Callback inNameChangedCallback);



// SSLP_Stop_Locating_Service_Instances: stop looking for a particular service type.
// If NULL is passed as the argument, stop all ongoing service-location efforts.
// After this call returns, the callback functions will no longer be called.
// After this call returns, ALL ServiceInstances that were passed to the "found" callback are invalid.
// You will not receive "lost" callbacks for them.
// NB!! in the current (limited) implementation, the argument is ignored altogether.  The ongoing
// service-location process is stopped... if there is no process ongoing, calling this function is an
// ERROR and may result in the termination of the entire application!
void
SSLP_Stop_Locating_Service_Instances(const char* inServiceType);



////// Allowing your services to be discovered //////

// Allow_Service_Discovery: let people looking for your type of service find your service-instance.
// ServiceInstance's data is copied, so you may do whatever you want with your storage.
// The "host" part of the address is ignored... the "port" says which port on the local machine the service
// may be contacted at.
// NB!! in the current (limited) implementation, only one service instance may be available for discovery
// at a time... calling this function twice without an intervening Disallow_Service_Discovery is an ERROR and
// may abort the entire application!
void
SSLP_Allow_Service_Discovery(const SSLP_ServiceInstance* inServiceInstance);



// SSLP_Hint_Service_Discovery: start periodically announcing the presence of a service-instance to a named machine and port.
// (this is useful if you need a way to connect machines in different broadcast domains, which ordinarily would not see
// each other by SSLP.)
// If remoteHost->port is 0, the default SSLP_PORT will be used.  This is almost certainly what you want.
// Make sure the data in inRemoteHost are in network (big-endian) byte order!
// If Allow_Service_Discovery was not already called for this service-instance, it's called by this function.
// The data you provide are copied, so you may do whatever you like with your own storage afterwards.
// NB!! in the current (limited) implementation, a call to this function with a service-instance that differs
// from the one provided to Allow_Service_Discovery is an ERROR and may lead to odd results.
// (calling this without having called Allow_Service_Discovery is ok, and makes the other call for you.) 
// Calling this with a different serviceInstance or remote address will change the hinting behavior to reflect
// your wishes.
void
SSLP_Hint_Service_Discovery(const SSLP_ServiceInstance* inServiceInstance, const IPaddress* inRemoteHost);



// SSLP_Disallow_Service_Discovery: stop letting other machines know that the service-instance is available.
// If NULL is provided, the effect is of calling Disallow_Service_Discovery on every currently-discoverable service instance.
// Also stops hinting the applicable service-instance(s), if appropriate.
// NB!! in the current (limited) implementation, the argument provided is ignored altogether.  Any hinting is stopped and
// the service-instance currently being published is unpublished.  It is an ERROR in the current implementation to call this
// twice without an intervening call to Allow_Service_Discovery (possibly via Hint_Service_Discovery), and doing so may
// abort the entire application!
void
SSLP_Disallow_Service_Discovery(const SSLP_ServiceInstance* inServiceInstance);


#endif//SSLP_API_H
