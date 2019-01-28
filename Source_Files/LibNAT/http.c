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
** This header file defines HTTP functions to send and receive HTTP requests
** to and from the router.
*/

/* define this to deprecate unsecure string warnings in vs2005.net */
#define _CRT_SECURE_NO_DEPRECATE 1

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "http_libnat.h"
#include "os.h"
#include "utility.h"
#include "error.h"

/* the format string for the host info that resides in an http header */
#define HTTP_HOST               "%s:%d"
#define HTTP_HOST_LEN_WO_VARS   1

/* The format strings for a get request, and the host to send it to. */
#define HTTP_GET                "GET /%s HTTP/1.1\r\nHOST: %s\r\n\r\n"
#define HTTP_GET_LEN_WO_VARS    26

/* The format strings for a post request, and the host to send it to */
#define HTTP_POST                "POST /%s HTTP/1.1\r\nHOST: %s\r\n%s%s\r\n%s"
#define HTTP_POST_LEN_WO_VARS    27

/* The strings to search for in the http header to find the body length */
#define HTTP_CONTENT_LEN_TAG     "CONTENT-LENGTH:"
#define HTTP_CONTENT_LEN_SEARCH  "%d"

/* The fromat string for a request header field for a post message */
#define HTTP_REQUEST_FIELD          "%s: %s\r\n"
#define HTTP_REQUEST_LEN_WO_VARS    4

/* the format string for an entity header field for a post message */
#define HTTP_ENTITY_FIELD           "%s: %s\r\n"
#define HTTP_ENTITY_LEN_WO_VARS     4

/* the terminating sequence of an HTTP header */
#define HTTP_HEADER_TERM           "\r\n\r\n"

/* Maximum size of an http response header */
#define MAX_HTTP_HEADER_LEN        4096

/* For our purposes, if no Content-Length is returned in the http header
   we don't want to receive a document more than 65536 bytes in length */
#define MAX_HTTP_BODY_LEN          65536

/* timeouts in seconds */
#define HTTP_CONNECT_TIMEOUT       2
#define HTTP_RECEIVE_TIMEOUT       10


/* Function Prototypes */
static int LNat_Http_Request(const char * resource, const char * host, short int port, const char * message, char ** response);
static int Send_Http_Request(OsSocket * s, const char * message);
static int Get_Http_Response(OsSocket * s, char ** http_response);
static int Get_Http_Content(OsSocket * s, int content_length, char ** http_content);
static int Get_Http_Content_Length(char * http_header, int * content_length);
static int Get_Http_Header(OsSocket * s, char ** http_response);
static int Http_Header_Terminator_Reached(char * http_response, int size_recv_sofar);
static int Create_Get_Request(GetMessage * gm, char ** http_get_request);
static int Create_Post_Request(PostMessage * pm, char ** http_post_request);
static int Create_Request_Header_String(PostMessage * pm, char ** request_header);
static int Create_Entity_Header_String(PostMessage * pm, char ** entity_header);
static int Create_Http_Header_Host_String(const char * host, short int port, char ** request_host);


/* structure to represent a GetMessage */
struct GetMessage
{
  char * resource;
  char * host;
  short int port;
};

/* structure to represent a Request Header Field
   that will reside in a Post Message */
typedef struct RequestHeader
{
  char * token;
  char * value;
  struct RequestHeader * next;
} RequestHeader;

/* structure to represent an Entity Header Field
   that will reside in a Post Message */
typedef struct EntityHeader
{
  char * token;
  char * value;
  struct EntityHeader * next;
} EntityHeader;

/* a post message */
struct PostMessage
{
  char * resource;
  char * host;
  short int port;
  char * body;

  int num_request_headers;  /* num request headers in the request header linked list */
  int num_entity_headers;   /* num entity headers in the entity header linked list */

  RequestHeader * rh_head; /* head of linked list of request headers */
  EntityHeader * eh_head;  /* head of linked list of entity headers */
};


/* This function will generate a GetMessage structure and return
   it in the gm parameter from the host, resource, and port params.
   On success, OK is returned. If error, gm will not be allocated,
   so null is returned in the gm parameter.

   Caller must call LNat_Destroy_Http_Get to destroy the GetMessage
   stucture when done with it.
*/
int LNat_Generate_Http_Get(const char * host,
                           const char * resource,
                           unsigned short int port,
                           GetMessage ** gm)
{
  if(NULL == host || NULL == resource || NULL == gm) {
    return BAD_PARAMS;
  }

  if(NULL == (*gm = (GetMessage *)malloc(sizeof(GetMessage)))) {
    return BAD_MALLOC;
  }

  if(NULL == ((*gm)->resource = (char *)malloc(strlen(resource)+1))) {
    free(*gm);
    return BAD_MALLOC;
  }
  (void)strcpy((*gm)->resource, resource);

  if(NULL == ((*gm)->host = (char *)malloc(strlen(host)+1))) {
    free((*gm)->resource);
    free(*gm);
    return BAD_MALLOC;
  }
  (void)strcpy((*gm)->host, host);

  (*gm)->port = port;
  return OK;
}



/* Destroys a GetMessage structure that was allocated by
   LNat_Generate_Http_Get.
*/
int LNat_Destroy_Http_Get(GetMessage ** gm)
{
  if(NULL == gm || NULL == *gm) {
    return BAD_PARAMS;
  }

  if(NULL != (*gm)->resource) {
    free((*gm)->resource);
  }

  if(NULL != (*gm)->host) {
    free((*gm)->host);
  }

  free(*gm);
  return OK;
}



/* This function will generate a PostMessage structure and return
   it in the pm parameter. It is generated from the host, resource,
   port, and body params. The body is the message to post.

   Caller must call LNat_Destroy_Http_Post to destroy the PostMessage
   structure when done with it. The caller can also add Request
   Header Fields and Entity Header Fields to the post message.
*/
int LNat_Generate_Http_Post(const char * host,
                            const char * resource,
                            unsigned short int port,
                            const char * body,
                            PostMessage ** pm)
{
  if(NULL == host || NULL == resource || NULL == body || NULL == pm) {
    return BAD_PARAMS;
  }

  if(NULL == (*pm = (PostMessage *)malloc(sizeof(PostMessage)))) {
    return BAD_MALLOC;
  }

  if(NULL == ((*pm)->resource = (char *)malloc(strlen(resource)+1))) {
    free(*pm);
    return BAD_MALLOC;
  }
  (void)strcpy((*pm)->resource, resource);

  if(NULL == ((*pm)->host = (char *)malloc(strlen(host)+1))) {
    free((*pm)->resource);
    free(*pm);
    return BAD_MALLOC;
  }
  (void)strcpy((*pm)->host, host);

  (*pm)->port = port;

  if(NULL == ((*pm)->body = (char *)malloc(strlen(body)+1))) {
    free((*pm)->resource);
    free((*pm)->host);
    free(*pm);
    return BAD_MALLOC;
  }
  (void)strcpy((*pm)->body, body);

  (*pm)->num_request_headers = 0;
  (*pm)->num_entity_headers = 0;
  (*pm)->rh_head = NULL;
  (*pm)->eh_head = NULL;
  return OK;
}



/* Add a Request Header Field to the post message */
int LNat_Http_Post_Add_Request_Header(PostMessage * pm,
                                      const char * token,
                                      const char * value)
{
  RequestHeader * new_rheader = (RequestHeader *)malloc(sizeof(RequestHeader));
  if(NULL == new_rheader) {
    return BAD_MALLOC;
  }

  new_rheader->token = (char *)malloc(strlen(token)+1);
  if(NULL == new_rheader->token) {
    free(new_rheader);
    return BAD_MALLOC;
  }
  new_rheader->value = (char *)malloc(strlen(value)+1);
  if(NULL == new_rheader->value) {
    free(new_rheader->token);
    free(new_rheader);
    return BAD_MALLOC;
  }

  new_rheader->next = pm->rh_head;
  pm->rh_head = new_rheader;
  pm->num_request_headers++;
  return OK;
}



/* Add an Entity Header Field to a post message */
int LNat_Http_Post_Add_Entity_Header(PostMessage * pm,
                                     const char * token,
                                     const char * value)
{
  EntityHeader * new_eheader = (EntityHeader *)malloc(sizeof(EntityHeader));
  if(NULL == new_eheader) {
    return BAD_MALLOC;
  }

  new_eheader->token = (char *)malloc(strlen(token)+1);
  if(NULL == new_eheader->token) {
    free(new_eheader);
    return BAD_MALLOC;
  }
  strcpy(new_eheader->token, token);

  new_eheader->value = (char *)malloc(strlen(value)+1);
  if(NULL == new_eheader->value) {
    free(new_eheader->token);
    free(new_eheader);
    return BAD_MALLOC;
  }
  strcpy(new_eheader->value , value);

  new_eheader->next = pm->eh_head;
  pm->eh_head = new_eheader;
  pm->num_entity_headers++;
  return OK;
}



/* Destroy a PostMessage structure that was allocated by
   LNat_Generate_Http_Post.
*/
int LNat_Destroy_Http_Post(PostMessage ** pm)
{
  int i;
  RequestHeader * next_rheader;
  EntityHeader * next_eheader;

  if(NULL == pm || NULL == *pm) {
    return BAD_PARAMS;
  }

  if(NULL != (*pm)->resource) {
    free((*pm)->resource);
  }

  if(NULL != (*pm)->host) {
    free((*pm)->host);
  }

  if(NULL != (*pm)->body) {
    free((*pm)->body);
  }

  for(i=0; i<(*pm)->num_request_headers; i++) {
    next_rheader = (*pm)->rh_head->next;
    free((*pm)->rh_head->value);
    free((*pm)->rh_head->token);
    free((*pm)->rh_head);
    (*pm)->rh_head = next_rheader;
  }

  for(i=0; i<(*pm)->num_entity_headers; i++) {
    next_eheader = (*pm)->eh_head->next;
    free((*pm)->eh_head->value);
    free((*pm)->eh_head->token);
    free((*pm)->eh_head);
    (*pm)->eh_head = next_eheader;
  }

  free(*pm);
  return OK;
}


/* This function makes an HTTP Get request to a particular ip
   address and port that is stored in the GetMessage struct. One must
   generate the GetMessage before calling this function using the
   LNat_Generate_Http_Get function. 
   
   This function allocates space for the response, and returns it
   in the response parameter. The caller must free this space with
   free(). Return OK on success.
*/
int LNat_Http_Request_Get(GetMessage * gm,
                          char ** response)
{
  int ret;
  char * http_get_request;

  if((ret = Create_Get_Request(gm, &http_get_request)) != OK) {
    return ret;
  }

  if((ret = LNat_Http_Request(gm->resource, gm->host, gm->port,
                            http_get_request, response)) != OK) {
    free(http_get_request);
    return ret;
  }

  free(http_get_request);
  return OK;
}



/* This function makes an HTTP Post request to a particular ip
   address and port that is stored in the PostMessage structure.
   One must generate the PostMessage structure before calling this
   function using the LNat_Generate_Http_Post function.

   This function allocates space for the response, and returns it
   in the response parameter. The caller must free this space with
   free(). Return OK on success.
*/
int LNat_Http_Request_Post(PostMessage * pm,
                           char ** response)
{
  int ret;
  char * http_post_request;

  if((ret = Create_Post_Request(pm, &http_post_request)) != OK) {
    return ret;
  }

  if((ret = LNat_Http_Request(pm->resource, pm->host, pm->port,
                            http_post_request, response)) != OK) {
    free(http_post_request);
    return ret;
  }

  free(http_post_request);
  return OK;
}



/* This function makes an HTTP request to the specified IP address
   and port. This function will return OK on success, or some error
   on failure. It will return an allocated buffer in response. The
   caller must free this buffer when it is through.
*/
static int LNat_Http_Request(const char * resource,
                             const char * host,
                             short int port,
                             const char * message,
                             char ** response)
{
  OsSocket * s;
  int ret;

  /* make the connection to the host and port */
  if((ret = LNat_Os_Socket_Connect(&s, host, port, 
                                    HTTP_CONNECT_TIMEOUT)) != OK ) {
    return ret;
  }

  /* send an HTTP request */
  if((ret = Send_Http_Request(s, message)) != OK) {
    LNat_Os_Socket_Close(&s);
    return ret;
  }

  /* get an HTTP response */
  if((ret = Get_Http_Response(s, response)) != OK) {
    LNat_Os_Socket_Close(&s);
    return ret;
  }

  /* close the connection s is connected to */
  if((ret = LNat_Os_Socket_Close(&s)) != OK) {
    return ret;
  }

  return OK;
}

/* this function will send an HTTP request to the http server that
   the OsSocket object is connected to. Returns OK on success.
*/
static int Send_Http_Request(OsSocket * s, 
                             const char * message)
{
  int amt_sent;
  int ret;

  /* now we will send the request */
  if((ret = LNat_Os_Socket_Send(s, 
      (char *)message, (int)strlen(message), &amt_sent)) != OK) {
    return ret;
  }

  return OK;
}

/* receives an HTTP response form the http server that
   the OsSocket object is connected to. Returns OK on success.
   The response is stored in the http_response parameter. This
   function allocates space for it, so the caller must free it.

   The http_response does not contain the HTTP Header, this function
   will parse that header out.
*/
static int Get_Http_Response(OsSocket * s, 
                             char ** http_response)
{
  int ret = 0;
  int content_length;
  char * http_header;

  if((ret = Get_Http_Header(s, &http_header)) != OK) {
    return ret;
  }

  /* get http data size from the header */
  if((ret = Get_Http_Content_Length(http_header, &content_length)) != OK) {
    free(http_header);
    return ret;
  }
  free(http_header);

  if((ret = Get_Http_Content(s, content_length, http_response)) != OK) {
    return ret;
  }

  return OK;
}


/* get the http content */
static int Get_Http_Content(OsSocket * s, 
                            int content_length, 
                            char ** http_content)
{
  int ret;
  int recv = 0;
  *http_content = (char *)malloc(content_length + NULL_TERM_LEN);
  if(NULL == *http_content) {
    return BAD_MALLOC;
  }

  if((ret = LNat_Os_Socket_Recv(s, *http_content, content_length, 
                                &recv, HTTP_RECEIVE_TIMEOUT)) != OK) {
    free(*http_content);
    return ret;
  }

  /* null terminate it */
  (*http_content)[recv] = NULL_TERM;
  return OK;
}


/* Get the HTTP Content length from the HTTP Header. If content length
   can't be found in the header, it will return -1 in the content_length
   parameter. */
static int Get_Http_Content_Length(char * http_header, 
                                   int * content_length)
{
  char * cl_loc;
  char * http_upper_header = (char *)malloc(strlen(http_header)+NULL_TERM_LEN);
  if(NULL == http_upper_header) {
    return -1;
  }
  (void)LNat_Str_To_Upper(http_header, http_upper_header);

  cl_loc = strstr(http_upper_header, HTTP_CONTENT_LEN_TAG);
  if(NULL == cl_loc) {
    *content_length = MAX_HTTP_BODY_LEN;
    free(http_upper_header);
    return OK;
  }

  /* put the cl_loc in the correct position in the original http_header */
  cl_loc = http_header + (cl_loc - http_upper_header) + 
           strlen(HTTP_CONTENT_LEN_TAG);
  if(sscanf(cl_loc, HTTP_CONTENT_LEN_SEARCH, content_length) != 1) {
    *content_length = MAX_HTTP_BODY_LEN;
    free(http_upper_header);
    return OK;
  }

  if(*content_length > MAX_HTTP_BODY_LEN) {
    *content_length = MAX_HTTP_BODY_LEN;
  }

  free(http_upper_header);
  return OK;
}

/* receive the HTTP Header from the http server that
   the OsSocket object is connected to. Returns OK on
   success. The response is stored in the http_header
   parameter. This function allocates space for it, so
   the caller must free it
*/
static int Get_Http_Header(OsSocket * s, 
                           char ** http_response)
{
  int ret;
  int recv = 0;
  int recv_sofar = 0;
  *http_response = (char *)malloc(MAX_HTTP_HEADER_LEN);
  if(NULL == *http_response) {
    return BAD_MALLOC;
  }

  /* recieve of size 1 till we hit the \r\n\r\n terminator
     of an HTTP header */
  do {
    ret = LNat_Os_Socket_Recv(s, &((*http_response)[recv_sofar]), 
                              1, &recv, HTTP_RECEIVE_TIMEOUT);
    recv_sofar += recv;
  } while(ret == OK && recv > 0 && recv_sofar < MAX_HTTP_HEADER_LEN &&
          !Http_Header_Terminator_Reached(*http_response, recv_sofar));

  if(ret != OK) {
    free(*http_response);
    return ret;
  }

  if(recv_sofar <= 0) {
    free(*http_response);
    return HTTP_RECV_FAILED;
  }

  if(recv_sofar >= MAX_HTTP_HEADER_LEN) {
    free(*http_response);
    return HTTP_RECV_OVER_MAXSIZE;
  }

  (*http_response)[recv_sofar] = NULL_TERM;
  if(NULL == strstr(*http_response, HTTP_OK)) {
    free(*http_response);
    return HTTP_HEADER_NOT_OK;
  }

  return OK;
}


/* if the data ends with /r/n/r/n, then return 1, else return 0. http_response
   probably doesn't contain a /0 null terminator, so the caller has to make
   sure that there is no buffer overrun going on. */
static int Http_Header_Terminator_Reached(char * http_response, 
                                          int size_recv_sofar)
{
  if(size_recv_sofar < (int)strlen(HTTP_HEADER_TERM)) {
    return 0;
  }

  if(!strncmp(&http_response[size_recv_sofar-strlen(HTTP_HEADER_TERM)], 
              HTTP_HEADER_TERM, strlen(HTTP_HEADER_TERM))) {
    return 1;
  }

  return 0;
}

/* create an HTTP Get request for retreiving a resource from the
   specified host for example...
   GET /rootDesc.xml HTTP/1.1\r\nHost: 192.168.1.1:5678\r\n\r\n */
static int Create_Get_Request(GetMessage * gm, 
                              char ** http_get_request)
{
  int ret;
  char * get_request_host;

  /* make sure params are ok */
  if(NULL == gm || NULL == http_get_request) {
    return BAD_PARAMS;
  }

  /* get the host string that will be in the get request */
  if((ret = Create_Http_Header_Host_String(gm->host, gm->port, 
                                    &get_request_host)) != OK) {
    return ret;
  }

  /* allocate the get request variable */
  *http_get_request = (char *)malloc(HTTP_GET_LEN_WO_VARS +
                                     strlen(gm->resource) +
                                     strlen(get_request_host) +
                                     NULL_TERM_LEN);
  /* make sure the malloc was ok */
  if(NULL == *http_get_request) {
    free(get_request_host);
    return BAD_MALLOC;
  }
  /* fill the array with the appropriate string */
  (void)sprintf(*http_get_request, HTTP_GET, gm->resource, get_request_host);

  free(get_request_host);
  return OK;
}


/* create an http post request from a PostMessage structure and return
   it in the http_post_request param. Caller must free this data when
   done with it */
static int Create_Post_Request(PostMessage * pm,
                               char ** http_post_request)
{
  int ret;
  char * post_request_host;
  char * request_header;
  char * entity_header;

  /* make sure params are ok */
  if(NULL == pm || NULL == http_post_request) {
    return BAD_PARAMS;
  }

  /* get the host string that will be in the post request */
  if((ret = Create_Http_Header_Host_String(pm->host, pm->port, 
                                    &post_request_host)) != OK) {
    return ret;
  } 

  /* create the request header fields */
  if((ret = Create_Request_Header_String(pm, &request_header)) != OK) {
    free(post_request_host);
    return ret;
  }

  /* create the entity header fields */
  if((ret = Create_Entity_Header_String(pm, &entity_header)) != OK) {
    free(post_request_host);
    free(request_header);
    return ret;
  }

  *http_post_request = (char *)malloc(HTTP_POST_LEN_WO_VARS +
                                      strlen(pm->resource) +
                                      strlen(post_request_host) +
                                      strlen(request_header) +
                                      strlen(entity_header) +
                                      strlen(pm->body) +
                                      NULL_TERM_LEN);
  if(NULL == *http_post_request) {
    free(post_request_host);
    free(request_header);
    free(entity_header);
    return BAD_MALLOC;
  }

  (void)sprintf(*http_post_request, HTTP_POST, pm->resource, post_request_host,
                request_header, entity_header, pm->body);

  free(post_request_host);
  free(request_header);
  free(entity_header);
  return OK;
}

/* This function will generate the request header field string
   needed by an http post message. This string will be returned
   in request_header, and must be deallocated by the caller */
static int Create_Request_Header_String(PostMessage * pm,
                                        char ** request_header)
{
  int request_header_fields_size = 0;
  RequestHeader * rh;

  for(rh = pm->rh_head; rh != NULL; rh = rh->next) {
    request_header_fields_size += HTTP_REQUEST_LEN_WO_VARS + 
                                  (int)strlen(rh->token) +
                                  (int)strlen(rh->value) +
                                  NULL_TERM_LEN;
  }
  if(0 == request_header_fields_size) {
    *request_header = (char *)malloc(1);
    if (NULL == *request_header) {
    	return BAD_MALLOC;
    }
    (*request_header)[0] = NULL_TERM;
    return OK;
  }

  *request_header = (char *)malloc(request_header_fields_size);
  if(NULL == *request_header) {
    return BAD_MALLOC;
  }
  (*request_header)[0] = NULL_TERM;

  for(rh = pm->rh_head; rh != NULL; rh = rh->next) {
    (void)sprintf(&((*request_header)[strlen(*request_header)]),
                  HTTP_REQUEST_FIELD, rh->token, rh->value);
  }
  return OK;
}

/* this function will generate the entity header field string
   needed by an http post message. This string will be returned
   in entity_header, and must be deallocated by the caller */
static int Create_Entity_Header_String(PostMessage * pm,
                                       char ** entity_header)
{
  int entity_header_fields_size = 0;
  EntityHeader * eh;

  for(eh = pm->eh_head; eh != NULL; eh = eh->next) {
    entity_header_fields_size += HTTP_ENTITY_LEN_WO_VARS +
                                 (int)strlen(eh->token) +
                                 (int)strlen(eh->value) +
                                 NULL_TERM_LEN;
  }
  if(0 == entity_header_fields_size) {
    *entity_header = (char *)malloc(1);
    if (NULL == *entity_header) {
    	return BAD_MALLOC;
    }
    (*entity_header)[0] = NULL_TERM;
    return OK;
  }

  *entity_header = (char *)malloc(entity_header_fields_size);
  if(NULL == *entity_header) {
    return BAD_MALLOC;
  }
  (*entity_header)[0] = NULL_TERM;

  for(eh = pm->eh_head; eh != NULL; eh = eh->next) {
    (void)sprintf(&((*entity_header)[strlen(*entity_header)]),
                  HTTP_ENTITY_FIELD, eh->token, eh->value);
  }
  return OK;
}

/* this function generate the header info common between a post
   header and a get header message for http */
static int Create_Http_Header_Host_String(const char * host,
                                          short int port,
                                          char ** request_host)
{
  /* check to make sure port number isn't out of range */
  if(port > MAX_PORT_SIZE) {
    return HTTP_PORT_OUT_OF_RANGE;
  }

  /* allocate the host:port variable to store in the get request 
     a check on the size of the port number is done first to make
     sure we con't get a buffer overflow */
  *request_host = (char *)malloc(HTTP_HOST_LEN_WO_VARS + 
                                 strlen(host) +
                                 MAX_PORT_STRING_LEN +
                                 NULL_TERM_LEN);
  /* make sure malloc was ok */
  if(NULL == *request_host) {
    return BAD_MALLOC;
  }
  /* fill the get_request_host variable with appropriate string */
  (void)sprintf(*request_host, HTTP_HOST, host, port);

  return OK;
}
