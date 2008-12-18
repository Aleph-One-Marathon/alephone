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
** This header file defines SSDP functions to send and receive SSDP requests
** to and from the router.
*/

/* define this to deprecate unsecure string warnings in vs2005.net */
#define _CRT_SECURE_NO_DEPRECATE 1

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "ssdp.h"
#include "os.h"
#include "utility.h"
#include "error.h"


/* Address and port of an SSDP request used for discovery */
#define SSDP_HOST_ADDRESS "239.255.255.250"
#define SSDP_HOST_PORT 1900

/* This is the format for the SSDP message that will be broadcast to
   find a router on the network with UPnP enabled. The variable is
   going to be what service we want to search for, since a router can
   be one of two services, we need to search for both. */
#define SEARCH_REQUEST_STRING "M-SEARCH * HTTP/1.1\r\n"            \
                              "MX: 2\r\n"                          \
                              "HOST: 239.255.255.250:1900\r\n"     \
                              "MAN: \"ssdp:discover\"\r\n"         \
                              "ST: %s\r\n"                         \
                              "\r\n"
#define SEARCH_REQUEST_LEN_WO_VARS 86

/* maximum size of a discovery response */
#define MAX_DISCOVERY_RECEIVE_SIZE 1024

/* timeout in seconds of a discovery request */
#define DISCOVERY_TIMEOUT     2


/* local file defines */
static int Send_Discover_Request(OsSocket * s, const char * search_target);
static int Get_Discover_Response(OsSocket * s, char ** response);


/* send an ssdp discover request, and receive a SsdpDiscResp back.
   Caller is responsible for freeing the response with free() */
int LNat_Ssdp_Discover(const char * search_target,
                       char ** response)
{
  int ret;
  OsSocket * s;

  /* setup the udp connection */
  if((ret = LNat_Os_Socket_Udp_Setup(&s)) != OK) {
    return ret;
  }

  /* send the discovery request */
  if((ret = Send_Discover_Request(s, search_target)) != OK) {
    return ret;
  }

  /* get the discovery response */
  if((ret = Get_Discover_Response(s, response)) != OK) {
    LNat_Os_Socket_Udp_Close(&s);
    return ret;
  }
  
  /* close the udp connection */
  if((ret = LNat_Os_Socket_Udp_Close(&s)) != OK) {
    free(*response);
    return ret;
  }

  return OK;
}


/* send a discover request over udp */
static int Send_Discover_Request(OsSocket * s,
                                 const char * search_target)
{
  int ret;
  int amt_sent;
  char * discover_request;

  discover_request = (char *)malloc(SEARCH_REQUEST_LEN_WO_VARS +
                                    strlen(search_target) +
                                    NULL_TERM_LEN);
  if(NULL == discover_request) {
    return BAD_MALLOC;
  }
  (void)sprintf(discover_request, SEARCH_REQUEST_STRING, search_target);

  if((ret = LNat_Os_Socket_Udp_Send(s, SSDP_HOST_ADDRESS, SSDP_HOST_PORT,
                                    discover_request, (int)strlen(discover_request),
                                    &amt_sent)) != OK) {
    free(discover_request);
    return ret;
  }

  free(discover_request);
  return OK;
}


/* receive a discover response */
static int Get_Discover_Response(OsSocket * s,
                                 char ** response)
{
  int ret;
  int amt_recv;
  int recv_sofar = 0;
  
  *response = (char *)malloc(MAX_DISCOVERY_RECEIVE_SIZE +
                             NULL_TERM_LEN);
  if(NULL == *response) {
    return BAD_MALLOC;
  }

  ret = LNat_Os_Socket_Udp_Recv(s, SSDP_HOST_ADDRESS, SSDP_HOST_PORT,
                                  &((*response)[recv_sofar]), 
                                  MAX_DISCOVERY_RECEIVE_SIZE, &amt_recv,
                                  DISCOVERY_TIMEOUT);
  if(ret != OK) {
    free(*response);
    return ret;
  }

  if(amt_recv <= 0) {
    free(*response);
    return SSDP_RECV_FAILED;
  }

  (*response)[amt_recv] = NULL_TERM;
  return OK; 
}




