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
** This file, os_win.c, contains the implementation fo the operating system
** specific calls for the win32 operating system. These calls include
** functionality for sockets and for threads.
*/

#include "error.h"
#include "os_common.h"


/* Helper Function Prototypes */
static int Initialize_Winsock_Library();
static int Is_Valid_Library(WSADATA * wsaData);
static int Initialize_Sockaddr_in(struct sockaddr_in* server, struct hostent** hp, 
                                  const char * host, short int port);



/* This function takes an OsSocket object, and attempts to allocate
   that socket of UDP type. This can then be used to send udp data
   to specified locations.
*/
int LNat_Win_Socket_Udp_Setup(OsSocket ** s)
{
  int ret;
  int blockMode = 1;              /* flag to set socket to nonblock */
  
  /* allocate a new OsSocket structure */
  *s = (OsSocket *)malloc(sizeof(OsSocket));
  if(*s == NULL) {
    return BAD_MALLOC;
  }
  
  /* initialize the winsock library */
  if((ret = Initialize_Winsock_Library()) != OK) {
    free(*s);
    return ret;
  }

  /* Set up the sockets */
  if(((*s)->sock = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
    WSACleanup();   /* call cleanup because user doesn't need to close sock */
    free(*s);
    return SOCKET_INVALID;
  }
  
  /* we want a non-blocking socket so our select calls can timeout */
  /* ioctlsocket returns nonzero on failure */
  if(ioctlsocket((*s)->sock, FIONBIO, (u_long FAR*)&blockMode)) {
    WSACleanup();   /* call cleanup because user doesn't need to close sock */
    return SOCKET_IOCTL_FAILURE;
  }
  
  return OK;
}

/* This function will close a udp socket type and clean it up
   appropriately. */
int LNat_Win_Socket_Udp_Close(OsSocket ** s)
{
  return LNat_Win_Socket_Close(s);
}

/* This function will send data over a udp socket to address host_addr,
   and port port. You need to specify the buffer */
int LNat_Win_Socket_Udp_Send(OsSocket * s, const char * host_addr, short int port, 
                            char * buf, int amt, int * amt_sent)
{
  return LNat_Common_Socket_Udp_Send(s, host_addr, port, buf, amt, amt_sent);
}

/* This function will recieve data over a udp socket form address host_addr,
   and port port. You need to specify the buffer to store it in, and the
   amt you are expecting to receive.
*/
int LNat_Win_Socket_Udp_Recv(OsSocket * s, const char * host_addr, short int port,
                            char * buf, int amt, int * amt_recv, int timeout_sec)
{
  return LNat_Common_Socket_Udp_Recv(s, host_addr, port, buf, amt, 
                                     amt_recv, timeout_sec);
}


/* This function takes an OsSocket object, host char*, and port, and
   initializes a connection to the host at port port. The OsSocket's
   variables will be returned through it's parameters. */
int LNat_Win_Socket_Connect(OsSocket ** s, const char * host_addr, short int port,
                            int timeout_sec)
{
  int blockMode = 1;              /* flag to set socket to nonblock */
  struct sockaddr_in server;      /* socket address stuff */
  struct hostent * hp;            /* host stuff */
  int ret;
  
  /* allocate a new OsSocket structure */
  *s = (OsSocket *)malloc(sizeof(OsSocket));
  if(*s == NULL) {
    return BAD_MALLOC;
  }
 
  /* initialize the winsock library */
  if((ret = Initialize_Winsock_Library()) != OK) {
    free(*s);
    return ret;
  }
  
  /* open a tcp socket for the internet */
  if(((*s)->sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
    WSACleanup();   /* call cleanup because user doesn't need to close sock */
    free(*s);
    return SOCKET_INVALID;
  }
  
  /* we want a non-blocking socket so our select calls can timeout */
  /* ioctlsocket returns nonzero on failure */
  if(ioctlsocket((*s)->sock, FIONBIO, (u_long FAR*)&blockMode)) {
    WSACleanup();   /* call cleanup because user doesn't need to close sock */
    return SOCKET_IOCTL_FAILURE;
  }
  
  /* initialize the address variable to tell connect where to connect
     to. It contains things like the port, host, and that it should
     connect over the internet */
  if((ret = Initialize_Sockaddr_in(&server, &hp, host_addr, port)) != OK) {
    WSACleanup();
    free(*s);
    return ret;
  }
  
  /* make the connection */
  connect((*s)->sock, (struct sockaddr *)&server, sizeof(server));
  if(Select_Till_Readywrite(*s, timeout_sec) != OK) {
    WSACleanup(); /* call cleanup because user doesn't need to close sock */
    free(*s);
    return SOCKET_CONNECT_FAILED;
  }
  
  return OK;
}


/* close the socket and call WSACleanup. WSACleanup will either unload the
   winsock dll, or decrement a reference count to it if multiple WSAStartups
   are still active. Need one cleanup per startup. */
int LNat_Win_Socket_Close(OsSocket ** s)
{
  (void)closesocket((*s)->sock);
  WSACleanup();
  free(*s);
  return OK;
}

/* function to send the data of length amt, in buffer buf, over a connected
socket s. If send is successful, return OK. set the amount actually sent in
amt_sent parameter */
int LNat_Win_Socket_Send(OsSocket * s, char * buf, int amt, int * amt_sent)
{
  return LNat_Common_Socket_Send(s, buf, amt, amt_sent);
}


/* function to recv the data of length amt, into an already allocated buffer
buf, over a connected socket s. If recv is successful, return oK. Set the
amount actually recieved in amt_recv parameter */
int LNat_Win_Socket_Recv(OsSocket * s, char * buf, int amt, 
                         int * amt_recv, int timeout_sec)
{
  return LNat_Common_Socket_Recv(s, buf, amt, amt_recv, timeout_sec);
}


/* get the local ip address from a connected socket */
int LNat_Win_Get_Local_Ip(OsSocket * s, char ** local_ip)
{
  return LNat_Common_Get_Local_Ip(s, local_ip);
}


/* function to initialize the winsock library, and validate
   that we are using the correct version of the library.
   */
static int Initialize_Winsock_Library()
{
  WORD wVersionRequested;     /* socket dll version info */
  WSADATA wsaData;            /* data for socket lib initialization */

  /* we need to call WSAStartup before we try to use any of
     the winsock dll calls. Request version 1.1 of winsock */
  wVersionRequested = MAKEWORD(2, 2);
  if(WSAStartup(wVersionRequested, &wsaData) != 0) {
    WSACleanup(); /* call cleanup because user doesn't need to close sock */
    return SOCKET_WSASTARTUP_FAILED;
  }
  
  /* make sure we are using the correct winsock library */
  if(Is_Valid_Library(&wsaData) != OK) {
    WSACleanup(); /* call cleanup because user doesn't need to close sock */
    return SOCKET_INVALID_LIB;
  }

  return OK;
}


/* function to take in a WSADATA structure that was
   set during the WSAStartup phase, and detect if we
   are using the winsock 1.1 version of the library or
   newer */
static int Is_Valid_Library(WSADATA * wsaData)
{
  float socklib_ver;
  socklib_ver = HIBYTE(wsaData->wVersion)/10.0F;
  socklib_ver += LOBYTE(wsaData->wVersion);
  if(socklib_ver < 2.2) {
    return SOCKET_INVALID_LIB;
  }
  return OK;
}


static int Initialize_Sockaddr_in(struct sockaddr_in* server,
                                  struct hostent** hp,
                                  const char * host_addr,
                                  short int port)
{
  return Common_Initialize_Sockaddr_in(server, hp, host_addr, port);
}

