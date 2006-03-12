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
** This header file contains a #define for every error possible within Libnat.
** Every function returns an error, and each of these errors are propogated
** down the function stack to the caller program. The caller can use
** LNat_Print_Error to detect what exactly went wrong.
*/

#ifndef _LNAT_ERROR_H_
#define _LNAT_ERROR_H_

/* error/status codes defined here */

/* no error */
#define OK 0
#define LNAT_ERROR                  -1

/* generic errors */
#define BAD_MALLOC                  -499
#define BAD_PARAMS                  -498

/* socket errors */
#define SOCKET_CONNECT_FAILED       -999
#define SOCKET_SEND_TIMEOUT         -998
#define SOCKET_SEND_FAILED          -997
#define SOCKET_RECV_TIMEOUT         -996
#define SOCKET_RECV_FAILED          -995
#define SOCKET_WSASTARTUP_FAILED    -994
#define SOCKET_INVALID_LIB          -993
#define SOCKET_GETHOSTBYNAME_FAILED -992
#define SOCKET_INVALID              -991
#define SOCKET_IOCTL_FAILURE        -990
#define SOCKET_GETFL_FAILURE        -989
#define SOCKET_SETFL_FAILURE        -988
#define SOCKET_GETSOCKNAME_FAILED   -987

/* http errors */
#define HTTP_RECV_FAILED            -899
#define HTTP_RECV_OVER_MAXSIZE      -898
#define HTTP_PORT_OUT_OF_RANGE      -897
#define HTTP_NO_CONTENT_LENGTH      -896
#define HTTP_HEADER_NOT_OK          -895
#define HTTP_INVALID_URL            -894

/* SSDP errors */
#define SSDP_RECV_FAILED            -799
#define SSDP_RECV_OVER_MAXSIZE      -798
#define SSDP_HEADER_NOT_OK          -797
#define SSDP_NO_LOCATION_TAG        -796
#define SSDP_NO_LOCATION            -795

/* UPnP errors */
#define UPNP_BAD_DESCRIPTION        -699
#define UPNP_GET_PUBLIC_IP_FAILED   -697
#define UPNP_BAD_PUBLIC_IP_RESPONSE -696
#define UPNP_URL_OVER_MAX_LEN       -695


/* call this to print an error to stderr. */
void LNat_Print_Internal_Error(int error);


#endif 
