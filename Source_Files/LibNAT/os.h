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
** This header file (together with is companion C source-code file
** "os.c") attempt to abstract the underlying operating system so that
** the libnat library will work on both POSIX and windows systems.
*/
#ifndef _LIBNAT_OS_H_
#define _LIBNAT_OS_H_

/*
** Are we dealing with Unix or Windows?
*/
#if !defined(OS_UNIX)
# ifndef OS_WIN
#   if defined(_WIN32) || defined(WIN32) || defined(__CYGWIN__) || defined(__MINGW32__) || defined(__BORLANDC__)
#     define OS_WIN 1
#     define OS_UNIX 0
#   else
#     define OS_WIN 0
#     define OS_UNIX 1
#  endif
# else
#  define OS_UNIX 0
# endif
#else
# ifndef OS_WIN
#  define OS_WIN 0
# endif
#endif

/*
** Define the interfaces for Unix and for Windows.
*/
#if OS_UNIX
/* Socket UDP Interface  for unix*/
#define LNat_Os_Socket_Udp_Setup          LNat_Unix_Socket_Udp_Setup
#define LNat_Os_Socket_Udp_Close          LNat_Unix_Socket_Udp_Close
#define LNat_Os_Socket_Udp_Send           LNat_Unix_Socket_Udp_Send
#define LNat_Os_Socket_Udp_Recv           LNat_Unix_Socket_Udp_Recv

/* socket TCP Interface for unix */
#define LNat_Os_Socket_Connect            LNat_Unix_Socket_Connect
#define LNat_Os_Socket_Close              LNat_Unix_Socket_Close
#define LNat_Os_Socket_Send               LNat_Unix_Socket_Send
#define LNat_Os_Socket_Recv               LNat_Unix_Socket_Recv

/* utility */
#define LNat_Os_Get_Local_Ip              LNat_Unix_Get_Local_Ip
#endif

#if OS_WIN
/* Socket UDP Interface for windows */
#define LNat_Os_Socket_Udp_Setup          LNat_Win_Socket_Udp_Setup
#define LNat_Os_Socket_Udp_Close          LNat_Win_Socket_Udp_Close
#define LNat_Os_Socket_Udp_Send           LNat_Win_Socket_Udp_Send
#define LNat_Os_Socket_Udp_Recv           LNat_Win_Socket_Udp_Recv

/* socket TCP Interface for windows */
#define LNat_Os_Socket_Connect            LNat_Win_Socket_Connect
#define LNat_Os_Socket_Close              LNat_Win_Socket_Close
#define LNat_Os_Socket_Send               LNat_Win_Socket_Send
#define LNat_Os_Socket_Recv               LNat_Win_Socket_Recv

/* utility */
#define LNat_Os_Get_Local_Ip              LNat_Win_Get_Local_Ip
#endif


/* forward declaration for opaque type */
typedef struct OsSocket OsSocket;


/*
** Prototypes for the operating system interface routings.
*/

/*
** UDP Socket Interface
*/
int LNat_Os_Socket_Udp_Setup(OsSocket ** s);
int LNat_Os_Socket_Udp_Close(OsSocket ** s);
int LNat_Os_Socket_Udp_Send(OsSocket * s, const char * host_addr, short int port, 
                            char * buf, int amt, int * amt_sent);
int LNat_Os_Socket_Udp_Recv(OsSocket * s, const char * host_addr, short int port,
                            char * buf, int amt, int * amt_recv, int timeout_sec);

/*
** TCP Socket Interface
*/
int LNat_Os_Socket_Connect(OsSocket ** s, const char * host_addr, 
                           short int port, int timeout_sec);
int LNat_Os_Socket_Close(OsSocket **);
int LNat_Os_Socket_Send(OsSocket *, char * buf, int amt, int * amt_sent);
int LNat_Os_Socket_Recv(OsSocket *, char * buf, int amt, 
                        int * amt_recv, int timeout_sec);

/*
** Utility functions
*/
/* gets the local ip address from a connected socket */
int LNat_Os_Get_Local_Ip(OsSocket *, char ** local_ip);


#endif
