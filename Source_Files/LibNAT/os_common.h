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
** This file, os_common.h, contains the definitions of the operating system
** specific calls all the operating systems. The operating system specific
** files will include this to get their functionality.
*/

#include "os.h"

#include <stdlib.h>

/* declaration of the operating system specific socket structures. This
   needs to be included in this file because many of the functions in
   here will need to operate on these structures 
   
   because structures are used that are defined in operating system specific
   locations, we need to include the appropriate headers where those are
   defined. These headers are platform dependent, so depending on the platform,
   choose the right one.
*/
#if OS_UNIX
  /* defines for unix */
  #include <fcntl.h>
  #include <time.h>
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <sys/select.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <netdb.h>
  #include <unistd.h>
  
  #include <sys/time.h>
  #include <string.h>
  
  /* this is the unix version of the OsSocket struct */
  struct OsSocket {
    int sock;
  };
#endif
#if OS_WIN
  /* defines for win32 */
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
  #include <winsock.h>
  typedef int socklen_t;
  
  /* this is the win32 version of the OsSocket struct */
  struct OsSocket {
    SOCKET sock;
  };
#endif


/* This function will send data over a udp socket to address host_addr,
   and port port. You need to specify the buffer */
int LNat_Common_Socket_Udp_Send(OsSocket * s, const char * host_addr, short int port, 
                                char * buf, int amt, int * amt_sent);
                                
/* This function will recieve data over a udp socket form address host_addr,
   and port port. You need to specify the buffer to store it in, and the
   amt you are expecting to receive.
*/
int LNat_Common_Socket_Udp_Recv(OsSocket * s, const char * host_addr, short int port,
                                char * buf, int amt, int * amt_recv, int timeout_sec);
                                
/* function to send the data of length amt, in buffer buf, over a connected
socket s. If send is successful, return OK. set the amount actually sent in
amt_sent parameter */
int LNat_Common_Socket_Send(OsSocket * s, char * buf, int amt, int * amt_sent);

/* function to recv the data of length amt, into an already allocated buffer
buf, over a connected socket s. If recv is successful, return oK. Set the
amount actually recieved in amt_recv parameter */
int LNat_Common_Socket_Recv(OsSocket * s, char * buf, int amt, 
                            int * amt_recv, int timeout_sec);


/* forward declaration for the sockaddr_in and hostent structures. these are
   needed for the Initialize_Sockaddr_in function declaration */
struct sockaddr_in;
struct hostent;

/* function to intitialize a sockaddr_in structure */
int Common_Initialize_Sockaddr_in(struct sockaddr_in* server,
                                  struct hostent** hp,
                                  const char * host_addr,
                                  short int port);

/* This function will call select which will block till timeout, 
   or until there is data ready to be read on the socket */
int Select_Till_Readyread(OsSocket * s, int timeout_sec);

/* This function will call seelct which will block till timeout,
   or until the socket is ready to be written to */
int Select_Till_Readywrite(OsSocket * s, int timeout_sec);

/* get the local ip address from a connected socket */
int LNat_Common_Get_Local_Ip(OsSocket * s, char ** local_ip);


