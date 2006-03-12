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
** This file, os_unix.c, contains the implementation fo the operating system
** specific calls for UNIX operating systems. These calls include
** functionality for sockets and for threads.
*/

#include "error.h"
#include "os_common.h"

/* Helper Function Prototypes */
static int Initialize_Sockaddr_in(struct sockaddr_in* server,
                             struct hostent** hp, const char * host, short int port);
static int Set_Nonblocking(OsSocket * s);


/* This function takes an OsSocket object, and attempts to allocate
   that socket of UDP type. This can then be used to send udp data
   to specified locations.
*/
int LNat_Unix_Socket_Udp_Setup(OsSocket ** s)
{
  int ret;
  
  /* allocate a new OsSocket structure */
  *s = (OsSocket *)malloc(sizeof(OsSocket));
  if(*s == NULL) {
    return BAD_MALLOC;
  }

  /* Set up the sockets */
  if(((*s)->sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    free(*s);
    return SOCKET_INVALID;
  }
  
  /* we want a non-blocking socket so our select calls can timeout */
  if((ret = Set_Nonblocking(*s)) != OK) {
    free(*s);
    return ret;
  }
  
  return OK;
}


/* This function will close a udp socket type and clean it up
   appropriately. */
int LNat_Unix_Socket_Udp_Close(OsSocket ** s)
{
  return LNat_Unix_Socket_Close(s);
}


/* This function will send data over a udp socket to address host_addr,
   and port port. You need to specify the buffer */
int LNat_Unix_Socket_Udp_Send(OsSocket * s, const char * host_addr, short int port, 
                            char * buf, int amt, int * amt_sent)
{
  return LNat_Common_Socket_Udp_Send(s, host_addr, port, buf, amt, amt_sent);
}


/* This function will recieve data over a udp socket form address host_addr,
   and port port. You need to specify the buffer to store it in, and the
   amt you are expecting to receive.
*/
int LNat_Unix_Socket_Udp_Recv(OsSocket * s, const char * host_addr, short int port,
                            char * buf, int amt, int * amt_recv, int timeout_sec)
{
  return LNat_Common_Socket_Udp_Recv(s, host_addr, port, buf, amt, 
                                    amt_recv, timeout_sec);
}  


/* This function takes an OsSocket object, host char*, and port, and
   initializes a connection to the host at port port. The OsSocket's
   variables will be returned through it's parameters. */
int LNat_Unix_Socket_Connect(OsSocket ** s, const char * host_addr, short int port, 
                           int timeout_sec)
{
  struct sockaddr_in server;      /* socket address stuff */
  struct hostent * hp;            /* host stuff */
  int ret;
  
  /* allocate a new OsSocket structure */
  *s = (OsSocket *)malloc(sizeof(OsSocket));
  if(*s == NULL) {
    return BAD_MALLOC;
  }
 
  /* open a tcp socket for the internet */
  if(((*s)->sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    free(*s);
    return SOCKET_INVALID;
  }
  
  /* we want a non-blocking socket so our select calls can timeout */
  if((ret = Set_Nonblocking(*s)) != OK) {
    free(*s);
    return ret;
  }
  
  /* initialize the address variable to tell connect where to connect
     to. It contains things like the port, host, and that it should
     connect over the internet */
  if((ret = Initialize_Sockaddr_in(&server, &hp, host_addr, port)) != OK) {
    free(*s);
    return ret;
  }
  
  /* make the connection */
  connect((*s)->sock, (struct sockaddr *)&server, sizeof(server));
  if(Select_Till_Readywrite(*s, timeout_sec) != OK) {
    free(*s);
    return SOCKET_CONNECT_FAILED;
  }
  
  return OK;
}

/* close the socket and deallocate the OsSocket structure */
int LNat_Unix_Socket_Close(OsSocket ** s)
{
  close((*s)->sock);
  free(*s);
  return OK;
}

/* function to send the data of length amt, in buffer buf, over a connected
socket s. If send is successful, return OK. set the amount actually sent in
amt_sent parameter */
int LNat_Unix_Socket_Send(OsSocket * s, char * buf, int amt, int * amt_sent)
{
  return LNat_Common_Socket_Send(s, buf, amt, amt_sent);
}


/* function to recv the data of length amt, into an already allocated buffer
buf, over a connected socket s. If recv is successful, return oK. Set the
amount actually recieved in amt_recv parameter */
int LNat_Unix_Socket_Recv(OsSocket * s, char * buf, int amt, int * amt_recv,
                        int timeout_sec)
{
  return LNat_Common_Socket_Recv(s, buf, amt, amt_recv, timeout_sec);
}


/* get the local ip address from a connected socket */
int LNat_Unix_Get_Local_Ip(OsSocket * s, char ** local_ip)
{
  return LNat_Common_Get_Local_Ip(s, local_ip);
}


/* initialize the sockaddr_in structure */
static int Initialize_Sockaddr_in(struct sockaddr_in* server,
                                  struct hostent** hp,
                                  const char * host_addr,
                                  short int port)
{
  return Common_Initialize_Sockaddr_in(server, hp, host_addr, port);
}


/* set a socket to be nonblocking */
static int Set_Nonblocking(OsSocket * s)
{
  int opts;
  if((opts = fcntl(s->sock, F_GETFL)) < 0) {
    return SOCKET_GETFL_FAILURE;
  }
  opts = (opts | O_NONBLOCK);
  if (fcntl(s->sock,F_SETFL,opts) < 0) {
    return SOCKET_SETFL_FAILURE;
  }
  return OK;
}
