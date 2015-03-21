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
** This file contains some declarations of some utility functions used
** throughout the project.
******************************************************************************/

/* length of a null terminator at the end of a c-string */
#define NULL_TERM_LEN                 1
/* null terminator character */
#define NULL_TERM                     '\0'

/* maximum size of a port number */
#define MAX_PORT_SIZE                 65535
/* maximum string length of a port number string */
#define MAX_PORT_STRING_LEN           5

/* Some HTTP Specific defines */
#define HTTP_OK                       "200 OK"
#define HTTP_PROTOCOL_STRING          "http://"
#define DEFAULT_HTTP_PORT             80

/* Max length of url's, hosts, and resources strings */
#define MAX_URL_LEN                   512
#define MAX_HOST_LEN                  512
#define MAX_RESOURCE_LEN              512


/* this function will convert a NULL terminated char * to all upper case,
   and return it in dest. Return OK on success. */
int LNat_Str_To_Upper(const char * str, char * dest);
