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
** This file, os_common.c, contains the implementations of the operating system
** specific calls all the operating systems. The operating system specific
** files will include this to get their functionality.
*/

/* define this to deprecate unsecure string warnings in vs2005.net */
#define _CRT_SECURE_NO_DEPRECATE 1

#include <errno.h>
#include "os_common.h"
#include "error.h"


/* This function will send data over a udp socket to address host_addr,
   and port port. You need to specify the buffer */
int LNat_Common_Socket_Udp_Send(OsSocket * s, const char * host_addr, 
                                short int port, char * buf, int amt, 
                                int * amt_sent)
{
  int ret;
  int send_ret = 1; /* if not more than 0, will fail the first check */
  int sent_sofar = 0;
  struct sockaddr_in server;
  struct hostent* hp;

  if((ret = Common_Initialize_Sockaddr_in(&server, &hp, host_addr, port)) != OK) {
    return ret;
  }

  /* A send amount of 0 could indicate a socket close, and less means error,
     so loop while there is still stuff to send and send hasn't returned
     one of these values */
  while(amt && !(send_ret <= 0)) {
    send_ret = sendto(s->sock, &(buf[sent_sofar]), amt, 0,
                      (struct sockaddr*)&server,
                      sizeof(struct sockaddr_in));
    if(send_ret > 0) {
      amt -= send_ret;
      sent_sofar += send_ret;
    }
  }
  *amt_sent = sent_sofar; /* now log the amoutn we sent so far param */

  /* if send failed, set amnt_sent to 0, and return socket_send_failed.
     a socket closure for a send indicates failure for us. */
  if(send_ret <= 0) {
    return SOCKET_SEND_FAILED;
  }

  return OK;
}
                                
/* This function will recieve data over a udp socket form address host_addr,
   and port port. You need to specify the buffer to store it in, and the
   amt you are expecting to receive.
*/
int LNat_Common_Socket_Udp_Recv(OsSocket * s, const char * host_addr, 
                                short int port, char * buf, int amt, 
                                int * amt_recv, int timeout_sec)
{
  int ret;
  int recv_ret = 1; /* if not more than 0, will fail the first check */
  int recv_sofar = 0;
  struct sockaddr_in server;
  struct hostent* hp;
  socklen_t sender_addr_sz = sizeof(server);

  if((ret = Common_Initialize_Sockaddr_in(&server, &hp, host_addr, port)) != OK) {
    return ret;
  }
  
  /* The entire packet must be read in a single request, so there is no
     need to loop here. Just wait till ready and receive */
  if(Select_Till_Readyread(s, timeout_sec) == OK) {
    recv_ret = recvfrom(s->sock, &(buf[recv_sofar]), amt, 0,
                    (struct sockaddr*)&server,
                    &sender_addr_sz);
    recv_sofar += recv_ret;
  } else {
    recv_ret = 0;
  }

  /* if recv failed, set amnt_recv to 0, and return socket_recv_failed.
     a socket closure for recv DOES NOT indicate failure for us */
  if(recv_ret < 0) {
    *amt_recv = 0;
    return SOCKET_RECV_FAILED;
  }

  *amt_recv = recv_sofar;
  return OK;
}
                                
/* function to send the data of length amt, in buffer buf, over a connected
socket s. If send is successful, return OK. set the amount actually sent in
amt_sent parameter */
int LNat_Common_Socket_Send(OsSocket * s, char * buf, int amt, int * amt_sent)
{
  int send_ret = 1; /* if not more than 0, will fail the first check */
  int sent_sofar = 0;
  
  /* A send amount of 0 could indicate a socket close, and less means error,
     so loop while there is still stuff to send and send hasn't returned
     one of these values */
  while(amt && !(send_ret <= 0)) {
    send_ret = send(s->sock, &(buf[sent_sofar]), amt, 0);
    if(send_ret > 0) {
      amt -= send_ret;
      sent_sofar += send_ret;
    }
  }
  *amt_sent = sent_sofar; /* log the amount sent so far param */

  /* if send failed, return socket_send_failed.
     a socket closure for a send indicates failure for us. */
  if(send_ret <= 0) {
    return SOCKET_SEND_FAILED;
  }

  return OK;
}

/* function to recv the data of length amt, into an already allocated buffer
buf, over a connected socket s. If recv is successful, return oK. Set the
amount actually recieved in amt_recv parameter */
int LNat_Common_Socket_Recv(OsSocket * s, char * buf, int amt, 
                            int * amt_recv, int timeout_sec)
{
  int recv_ret = 1;   /* must be greater than 0 or will fail first check */
  int recv_sofar = 0;
  
  /* A recv amount of 0 will indicate a socket close, and less means error,
     so loop while there is still stuff to recv, or until the connection
     closed */
  while(amt && !(recv_ret <= 0)) {
    if(Select_Till_Readyread(s, timeout_sec) == OK) {
      recv_ret = recv(s->sock, &(buf[recv_sofar]), amt, 0);
      amt -= recv_ret;
      recv_sofar += recv_ret;
    } else {
      recv_ret = 0;
    }
  }
  /* if recv failed, set amnt_recv to 0, and return socket_recv_failed.
     a socket closure for recv DOES NOT indicate failure for us */
  if(recv_ret < 0) {
    *amt_recv = 0;
    return SOCKET_RECV_FAILED;
  }
  
  *amt_recv = recv_sofar;
  return OK;
}


/* function to intitialize a sockaddr_in structure */
int Common_Initialize_Sockaddr_in(struct sockaddr_in* server,
                                  struct hostent** hp,
                                  const char * host_addr,
                                  short int port)
{
  memset(server, 0, sizeof(struct sockaddr));
  server->sin_family = AF_INET;
  if((*hp = gethostbyname(host_addr)) == NULL) {
    return SOCKET_GETHOSTBYNAME_FAILED;
  }
  memcpy(&server->sin_addr,
        (*hp)->h_addr_list[0],
        (*hp)->h_length);
  server->sin_port = htons(port);

  return OK;
}

/* This function will call select which will block till timeout, 
   or until there is data ready to be read on the socket */
int Select_Till_Readyread(OsSocket * s,
                          int timeout_sec)
{
  int rv;
  fd_set read_fds;
  fd_set error_fds;
  struct timeval tv;

  do {
    tv.tv_sec = timeout_sec;
    tv.tv_usec = 0;
    FD_ZERO(&read_fds);
    FD_ZERO(&error_fds);
    FD_SET(s->sock, &read_fds);
    FD_SET(s->sock, &error_fds);

    rv = select((int)(s->sock+1), &read_fds, NULL, &error_fds, &tv);
  } while(rv < 0 && errno == EINTR);

  /* if data is ready to be read, return OK */
  if(FD_ISSET(s->sock, &read_fds)) {
    return OK;
  } else {
    return SOCKET_RECV_TIMEOUT;
  }
}


/* This function will call seelct which will block till timeout,
   or until the socket is ready to be written to */
int Select_Till_Readywrite(OsSocket * s,
                           int timeout_sec)
{
  int rv;
  fd_set write_fds;
  fd_set error_fds;
  struct timeval tv;

  do {
    tv.tv_sec = timeout_sec;
    tv.tv_usec = 0;
    FD_ZERO(&write_fds);
    FD_ZERO(&error_fds);
    FD_SET(s->sock, &write_fds);
    FD_SET(s->sock, &error_fds);

    rv = select((int)(s->sock+1), NULL, &write_fds, &error_fds, &tv);
  } while(rv < 0 && errno == EINTR);

  /* if data is ready to be written, return OK */
  if(FD_ISSET(s->sock, &write_fds)) {
    return OK;
  } else {
    return SOCKET_SEND_TIMEOUT;
  }
}


/* get the local ip address from a connected socket */
int LNat_Common_Get_Local_Ip(OsSocket * s, char ** local_ip)
{
  struct sockaddr_in local;
  socklen_t saSize = sizeof(struct sockaddr);
  if(getsockname(s->sock,(struct sockaddr *)&local,&saSize)) {
    return SOCKET_GETSOCKNAME_FAILED;
  }
  *local_ip = (char *)malloc(strlen(inet_ntoa(local.sin_addr))+1);
  if(NULL == *local_ip) {
    return BAD_MALLOC;
  }
  strcpy(*local_ip, inet_ntoa(local.sin_addr));

  return OK;
}
