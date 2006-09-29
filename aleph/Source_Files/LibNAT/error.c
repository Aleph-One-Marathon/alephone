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
** This implementation file contains the implementation for printing out
** errors in human readable format onto stderr.
*/

#include <stdlib.h>
#include <stdio.h>
#include "error.h"

/* Implementation for printing an error to stderr. Contains
   a huge switch statement with every possible error and a
   description of each error */
void LNat_Print_Internal_Error(int error)
{
  switch(error)
  {
  /* General Errors */
  case OK:
    (void)fprintf(stderr, "No Error Occured.\n");
    break;
  case BAD_MALLOC:
    (void)fprintf(stderr, "Bad Malloc.\n");
    break;
  case BAD_PARAMS:
    (void)fprintf(stderr, "Bad Params.\n");
    break;

  /* Socket Errors */
  case SOCKET_CONNECT_FAILED:
    (void)fprintf(stderr, "Socket Connection Failed.\n");
    break;
  case SOCKET_SEND_TIMEOUT:
    (void)fprintf(stderr, "Socket Send Timeout.\n");
    break;
  case SOCKET_SEND_FAILED:
    (void)fprintf(stderr, "Socket Send Failure.\n");
    break;
  case SOCKET_RECV_TIMEOUT:
    (void)fprintf(stderr, "Socket Recv Timeout.\n");
    break;
  case SOCKET_RECV_FAILED:
    (void)fprintf(stderr, "Socket Recv Failure.\n");
    break;
  case SOCKET_WSASTARTUP_FAILED:
    (void)fprintf(stderr, "WSAStartup Failure.\n");
    break;
  case SOCKET_INVALID_LIB:
    (void)fprintf(stderr, "Invalid Winsock library found.\n");
    break;
  case SOCKET_GETHOSTBYNAME_FAILED:
    (void)fprintf(stderr, "Gethostbyname failed.\n");
    break;
  case SOCKET_INVALID:
    (void)fprintf(stderr, "Invalid Socket.\n");
    break;
  case SOCKET_IOCTL_FAILURE:
    (void)fprintf(stderr, "IOCTL Failed.\n");
    break;
  case SOCKET_GETFL_FAILURE:
    (void)fprintf(stderr, "GETFL Failed.\n");
    break;
  case SOCKET_SETFL_FAILURE:
    (void)fprintf(stderr, "SETFL Failed.\n");
    break;
  case SOCKET_GETSOCKNAME_FAILED:
    (void)fprintf(stderr, "getsockname() Failed.");
    break;

  /* HTTP Errors */
  case HTTP_RECV_FAILED:
    (void)fprintf(stderr, "HTTP Receive failed.\n");
    break;
  case HTTP_RECV_OVER_MAXSIZE:
    (void)fprintf(stderr, "HTTP Received more data than can fit in buffer.\n");
    break;
  case HTTP_PORT_OUT_OF_RANGE:
    (void)fprintf(stderr, "HTTP port given is out of range.\n");
    break;
  case HTTP_NO_CONTENT_LENGTH:
    (void)fprintf(stderr, "No Content-Length found in HTTP Header.\n");
    break;
  case HTTP_HEADER_NOT_OK:
    (void)fprintf(stderr, "Didn't receive an HTTP/1.1 200 OK Header for HTTP.\n");
     break;
  case HTTP_INVALID_URL:
    (void)fprintf(stderr, "URL is not a valid HTTP URL.\n");
    break;

  /* SSDP Errors */
  case SSDP_RECV_FAILED:
    (void)fprintf(stderr, "SSDP Receive failed.\n");
    break;
  case SSDP_RECV_OVER_MAXSIZE:
    (void)fprintf(stderr, "SSDP Received more data than can fit in buffer.\n");
    break;
  case SSDP_HEADER_NOT_OK:
    (void)fprintf(stderr, "Didn't receive an HTTP/1.1 200 OK Header for SSDP.\n");
    break;
  case SSDP_NO_LOCATION_TAG:
    (void)fprintf(stderr, "No location tag found in SSDP header.\n");
    break;
  case SSDP_NO_LOCATION:
    (void)fprintf(stderr, "No location found in SSDP header.\n");
    break;
    
  /* UPnP Errors */
  case UPNP_BAD_DESCRIPTION:
    (void)fprintf(stderr, "Bad Description XML could not be parsed.\n");
    break;
  case UPNP_GET_PUBLIC_IP_FAILED:
    (void)fprintf(stderr, "Get Public IP Failed.\n");
    break;
  case UPNP_BAD_PUBLIC_IP_RESPONSE:
    (void)fprintf(stderr, "Bad XML Response from Get Public IP.\n");
    break;
  case UPNP_URL_OVER_MAX_LEN:
    (void)fprintf(stderr, "URL Over Maximum Allowed Length.\n");
    break;

  default:
    (void)fprintf(stderr, "Some Error.\n");
    break;
  }
}
