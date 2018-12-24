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
** This header file declares HTTP functions to send and receive HTTP requests
** to and from the router.
*/

#ifndef _HTTP_H_
#define _HTTP_H_

/* forward declaration of GetMessage */
typedef struct GetMessage GetMessage;
typedef struct PostMessage PostMessage;


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
                           GetMessage ** gm);

/* Destroys a GetMessage structure that was allocated by
   LNat_Generate_Http_Get.
*/
int LNat_Destroy_Http_Get(GetMessage ** gm);

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
                            PostMessage ** pm);

/* Add a Request Header Field to the post message */
int LNat_Http_Post_Add_Request_Header(PostMessage * pm,
                                      const char * token,
                                      const char * value);

/* Add an Entity Header Field to a post message */
int LNat_Http_Post_Add_Entity_Header(PostMessage * pm,
                                     const char * token,
                                     const char * value);

/* Destroy a PostMessage structure that was allocated by
   LNat_Generate_Http_Post.
*/
int LNat_Destroy_Http_Post(PostMessage ** pm);


/* This function makes an HTTP Get request to a particular ip
   address and port that is stored in the GetMessage struct. One must
   generate the GetMessage before calling this function using the
   LNat_Generate_Http_Get function. 
   
   This function allocates space for the response, and returns it
   in the response parameter. The caller must free this space with
   free(). Return OK on success.
*/
int LNat_Http_Request_Get(GetMessage * gm,
                          char ** response);

/* This function makes an HTTP Post request to a particular ip
   address and port that is stored in the PostMessage structure.
   One must generate the PostMessage structure before calling this
   function using the LNat_Generate_Http_Post function.

   This function allocates space for the response, and returns it
   in the response parameter. The caller must free this space with
   free(). Return OK on success.
*/
int LNat_Http_Request_Post(PostMessage * pm,
                           char ** response);

#endif
