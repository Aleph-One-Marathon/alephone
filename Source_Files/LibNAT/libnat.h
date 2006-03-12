/* Copyright (c) 2006 Adam Warrington
** $Id$
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in
** all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
** SOFTWARE.
**
******************************************************************************
**
** This header file declares the functions to control an Internet Gateway Device.
** At the moment, only the UPnP functions are declared here, but this file is able
** to be extended to use different protocols.
*/

#ifndef _LNAT_LIBNAT_H_
#define _LNAT_LIBNAT_H_

/* The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the LIBNAT_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// LIBNAT_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.*/
//#ifdef WIN32 0
#if 0
  #ifdef LIBNAT_EXPORTS
    #define LIBNAT_API __declspec(dllexport)
  #else
    #define LIBNAT_API __declspec(dllimport)
  #endif
#else /* not MSVC */
  #define LIBNAT_API
#endif

/* include error.h so it is available to users */
#include "error.h"


/* UpnpController structure definition */
typedef struct LIBNAT_API UpnpController
{
  char * control_url;
  char * service_type;
}UpnpController;

/* make sure this is callable from C++ */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * Sends a discovery request to search for a UPnP enabled IGD that
 * contains the WANIPConnection service or WANPPPConnection that will 
 * allow us to recieve the public IP address of the IGD, and control 
 * it for forwarding ports.
 *
 * Return the UpnpController for the IGD in the c parameter.
 * User should call LNat_Upnp_Free() to free the memory allocated for
 * this UpnpController object.
 * 
 * Return 0 if everything was successful.
 */
LIBNAT_API
int LNat_Upnp_Discover(UpnpController ** c);



/**
 * Gets the IP address from a UPnP enabled IGD that sits on the local
 * network, so when getting the network IP, instead of returning the
 * local network IP, the public IP is retrieved.
 *
 * controller: The UpnpController retrieved from LNat_Upnp_Discover.
 *
 * return the IP address of the network in a null terminated string
 * in the public_ip parameter. public_ip must already be allocated,
 * and the size of it must be passed in the public_ip_size parameter.
 * If the ip received from the router is larger than the public_ip_size,
 * only the first public_ip_size bytes minus the null terminator will
 * be copied into public_ip. The null terminator will be appended to the
 * end of public_ip.
 *
 * Return 0 if everything was successful.
 */
LIBNAT_API
int LNat_Upnp_Get_Public_Ip(const UpnpController * controller,
                            char * public_ip, int public_ip_size);


/**
 * Maps Ports in a UPnP enabled IGD that sits on the local network to.
 * Essentially, this function takes care of the port forwarding so things 
 * like file transfers can work behind NAT firewalls.
 *
 * controller: The UpnpController retrieved from LNat_Upnp_Discover.
 * ip_map:      The ip to add this mapping for. If NULL, this function will
 *             attempt to determine the IP address of this machine.
 * port_map:    The port to map to this client
 * protocol:   The protocol to map, either "TCP" or "UDP"
 *
 * return 0 if successful.
 */
LIBNAT_API
int LNat_Upnp_Set_Port_Mapping(const UpnpController * controller, 
                               const char * ip_map,
                               short int port_map,
                               const char* protocol);

/**
 * Deletes a port mapping in a UPnP enabled IGD that sits on the local network.
 * Essentially, this function takes care of deleting the port forwarding after 
 * they have completed a connection so another client on the local network can 
 * take advantage of the port forwarding
 *
 * controller: The UpnpController retrieved from LNat_Upnp_Discover.
 * ipMap:      The ip to delete this mapping for. If NULL, this function will
 *             attempt to determine the IP address of this machine.
 * portMap:    The port to delete the mapping for
 * protocol :  The protocol the port was mapped to. Either "TCP" or "UDP"
 *
 * return 0 if successful.
 */
LIBNAT_API
int LNat_Upnp_Remove_Port_Mapping(const UpnpController * controller,
                                  short int port_map,
                                  const char* protocol);

/**
 * Destroy a UpnpController object that was allocated by LNat_Upnp_Discover.
 * return 0 if successful.
 */
LIBNAT_API
int LNat_Upnp_Controller_Free(UpnpController ** controller);


/* call this to print an error to stderr. */
LIBNAT_API
void LNat_Print_Error(int error);


#ifdef __cplusplus
}
#endif

#endif
