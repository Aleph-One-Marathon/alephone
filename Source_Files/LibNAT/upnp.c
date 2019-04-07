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
** This  file defines UPnP functions to control an Internet Gateway Device using
** the Universal Plug and Play protocol.
*/

/* define this to deprecate unsecure string warnings in vs2005.net */
#define _CRT_SECURE_NO_DEPRECATE 1

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "libnat.h"
#include "http_libnat.h"
#include "ssdp.h"
#include "os.h"       /* for getting the local ip */
#include "utility.h"
#include "expat.h"

/***************************************************************
** Discovery/Description Defines                               *
****************************************************************/
#define NUM_SSDP_ATTEMPTS 2

/* Search target for an SSDP query */
#define SSDP_TARGET                 "urn:schemas-upnp-org:service:%s"

/* wanip target */
#define WANIP_TARGET                  "WANIPConnection:1"

/* wanppp target */
#define WANPPP_TARGET                 "WANPPPConnection:1"

/* Header tag for the location of the description url */
#define LOCATION_TAG                  "LOCATION:"

/* xml element for control url */
#define SERVICE_TYPE_ELEMENT          "serviceType"
#define BASE_URL_ELEMENT              "baseURL"
#define CONTROL_URL_ELEMENT           "controlURL"


/******************************************************************
** Action Defines                                                 *
*******************************************************************/
#define SOAP_ACTION_TAG     "SOAPACTION"
#define SOAP_ACTION_VAL     "\"" SSDP_TARGET "#%s\""

#define CONTENT_TYPE_TAG    "CONTENT-TYPE"

#define CONTENT_TYPE_VAL    "text/xml ; charset=\"utf-8\""

#define CONTENT_LENGTH_TAG      "Content-Length"
#define MAX_CONTENT_STRING_LEN  5
#define CONTENT_LENGTH_VAL      "%5d"

#define SOAP_ACTION  "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n"     \
                     "<s:Envelope xmlns:s="                               \
                     "\"http://schemas.xmlsoap.org/soap/envelope/\" "     \
                     "s:encodingStyle="                                   \
                     "\"http://schemas.xmlsoap.org/soap/encoding/\">\r\n" \
                     "<s:Body>\r\n"                                       \
                     "<u:%s xmlns:u="                                     \
                     "\"urn:schemas-upnp-org:service:%s\">\r\n%s"         \
                     "</u:%s>\r\n"                                        \
                     "</s:Body>\r\n"                                      \
                     "</s:Envelope>\r\n"

#define GET_PUBLIC_IP_ACTION_NAME         "GetExternalIPAddress"
#define GET_PUBLIC_IP_RESPONSE_ELEMENT    "NewExternalIPAddress"
#define SET_PORT_MAPPING_ACTION_NAME      "AddPortMapping"
#define REMOVE_PORT_MAPPING_ACTION_NAME   "DeletePortMapping"

#define PORT_MAPPING_LEASE_TIME "0"

#define GET_PUBLIC_IP_PARAMS    ""
#define SET_PORT_MAPPING_PARAMS "<NewRemoteHost></NewRemoteHost>\r\n"      \
                                "<NewExternalPort>%d</NewExternalPort>\r\n"\
                                "<NewProtocol>%s</NewProtocol>\r\n"        \
                                "<NewInternalPort>%d</NewInternalPort>\r\n"\
                                "<NewInternalClient>%s"                    \
                                "</NewInternalClient>\r\n"                 \
                                "<NewEnabled>1</NewEnabled>\r\n"           \
                                "<NewPortMappingDescription>"              \
                                "</NewPortMappingDescription>\r\n"         \
                                "<NewLeaseDuration>"                       \
                                PORT_MAPPING_LEASE_TIME                    \
                                "</NewLeaseDuration>\r\n"

#define REMOVE_PORT_MAPPING_PARAMS "<NewRemoteHost></NewRemoteHost>\r\n" \
                                   "<NewExternalPort>%d"                 \
                                   "</NewExternalPort>\r\n"              \
                                   "<NewProtocol>%s</NewProtocol>\r\n"


/************************************************************************
** Internal Structures 
*************************************************************************/

/* data to pass to xml description parse events */
struct XmlDescData
{
  char * control_url;           /* control url of the service */
  char * url_base;              /* base of the control url */
  char * correct_service;       /* service type we are looking for */

  int found_url_base;           /* 1 if we found a base element */
  int found_service_type;       /* 1 if we found a service type element */
  int found_correct_service;    /* 1 if the service type was the right one */
  int found_control_url;        /* 1 if we found a control url element */
};
typedef struct XmlDescData XmlDescData;

/* data to pass to xml get public ip parse events */
struct XmlGetPublicIpData
{
  char * ext_ip;      /* the external ip address found in the xml */
  int found_ext_ip;   /* will equal 1 when the external ip element is found */
};
typedef struct XmlGetPublicIpData XmlGetPublicIpData;


/* local file function prototypes */
static int Send_Ssdp_Discover(UpnpController * c, char ** ssdp_response);
static int Get_Description_Url(const char * ssdp_response, char ** desc_url);
static int Get_Description(const char * desc_url, char ** description);
static int Get_Control_Url(UpnpController * c, const char * desc_url, const char * description);
static int Parse_Url(const char * desc_url, char * host, char * resource, unsigned short int * port);
static void Start_Desc_Element(void * userData, const char * name, const char ** atts);
static void End_Desc_Element(void * userData, const char * name);
static void Data_Desc_Handler(void * userData, const char * s, int len);
static void XmlDescData_Free(XmlDescData d);
static int Get_Public_Ip_From_Response(const char * response, char * public_ip, int public_ip_size);
static void Start_Ip_Element(void * userData, const char * name, const char ** atts);
static void End_Ip_Element(void * userData, const char * name);
static void Data_Ip_Handler(void * userData, const char * s, int len);
static int Create_Action_Message(const char * service_type, const char * action_name, const char * action_params, char ** action_message);
static int Send_Action_Message(const UpnpController * c, const char * action_name, const char * body, char ** response);
static int Get_Local_Ip(const UpnpController * c, char ** ip_map);

/********************************************************************************
**                   UPnP Discover Specific Functions                           *
********************************************************************************/

/**
 * Sends a discovery request to search for a UPnP enabled IGD that
 * contains the WANIPConnection service or WANPPPConnection that will 
 * allow us to recieve the public IP address of the IGD, and control 
 * it for forwarding ports.
 *
 * Return the UpnpController for the IGD in the c parameter.
 * User should call LNat_Upnp_Free() to free the memory allocated for
 * this UpnpController object.
 * 
 * Return OK if everything was successful.
 */
LIBNAT_API
int LNat_Upnp_Discover(UpnpController ** c)
{
  int ret;
  char * ssdp_response;
  char * desc_url = NULL;
  char * description;

  /* allocate space for our controller object, and initialize it's members */
  *c = (UpnpController *)malloc(sizeof(UpnpController));
  if(NULL == *c) {
    return BAD_MALLOC;
  }
  (*c)->control_url = NULL;
  (*c)->service_type = NULL;

  /* send a discovery request, and get a response */
  if((ret = Send_Ssdp_Discover(*c, &ssdp_response)) != OK) {
    (void)LNat_Upnp_Controller_Free(c);
    return ret;
  }

  /* get the description URL out of the ssdp discovery request */
  if((ret = Get_Description_Url(ssdp_response, &desc_url)) != OK) {
    free(ssdp_response);
    (void)LNat_Upnp_Controller_Free(c);
    return ret;
  }

  /* retreive the description from the desc_url */
  if((ret = Get_Description(desc_url, &description)) != OK) {
    free(ssdp_response);
    free(desc_url);
    (void)LNat_Upnp_Controller_Free(c);
    return ret;
  }

  /* get the control_url from the description, and store it in c */
  if((ret = Get_Control_Url(*c, desc_url, description)) != OK) {
    free(ssdp_response);
    free(desc_url);
    free(description);
    (void)LNat_Upnp_Controller_Free(c);
    return ret;
  }

  free(ssdp_response);
  free(desc_url);
  free(description);

  return OK;
}


/* Send SSDP discovery requests searching for WANIPConnection and
   WANPPPConnection services on the local network. Return OK if we
   can find one. Also set the serviceType parameter of the
   UpnpController structure. */
static int Send_Ssdp_Discover(UpnpController * c, char ** ssdp_response)
{
  int i;
  int ret;
  /* allocate space for our search string */
  char * search_target = (char *)malloc(strlen(SSDP_TARGET) +
                                        strlen(WANPPP_TARGET)+
                                        NULL_TERM_LEN);
  if(NULL == search_target) {
    return BAD_MALLOC;
  }

  for(i=0; i<NUM_SSDP_ATTEMPTS; i++) {
    if(i%2 == 0) {
      (void)sprintf(search_target, SSDP_TARGET, WANIP_TARGET);
    } else {
      (void)sprintf(search_target, SSDP_TARGET, WANPPP_TARGET);
    }
    ret = LNat_Ssdp_Discover(search_target, ssdp_response);

    /* if ret == OK, we need to use i with the val it has, so break out */
    if(ret == OK) {
      break;
    }
  }
  if(i == NUM_SSDP_ATTEMPTS) {
    free(search_target);
    return ret;
  }

  /* store the service type variable */
  if(i%2 == 0) {
    c->service_type = (char *)malloc(strlen(WANIP_TARGET) + 
                                     NULL_TERM_LEN);
    if(NULL == c->service_type) {
      free(search_target);
      return BAD_MALLOC;
    }
    (void)strcpy(c->service_type, WANIP_TARGET);
  } else {
    c->service_type = (char *)malloc(strlen(WANPPP_TARGET) + 
                                     NULL_TERM_LEN);
    if(NULL == c->service_type) {
      free(search_target);
      return BAD_MALLOC;
    }
    (void)strcpy(c->service_type, WANPPP_TARGET);
  }
  free(search_target);

  return OK;
}


/* Get the url to retrieve the upnp description from. This URL is parsed
   out of the ssdp response retrieved from an ssdp request. */
static int Get_Description_Url(const char * ssdp_response, char ** desc_url)
{
  const char * fir_rn;
  const char * sec_rn;
  const char * end_loc_tag;
  int end_loc_offset;
  int desc_url_malloc_size;
  char * ssdp_upper_response = (char *)malloc(strlen(ssdp_response) + 
                                              NULL_TERM_LEN);
  if(NULL == ssdp_upper_response) {
    return BAD_MALLOC;
  }
  /* parsing done on all caps version */
  (void)LNat_Str_To_Upper(ssdp_response, ssdp_upper_response);

  /* if we didnt receive an HTTP OK response, return error */
  if(NULL == strstr(ssdp_response, HTTP_OK)) {
    free(ssdp_upper_response);
    return SSDP_HEADER_NOT_OK;
  }

  /* we will scan for successive pairs of /r/n's, and evaluate
     the data between them to see if it is the location tag. if
     it is the location tag, then copy the data between that, and
     the last /r/n. */
  fir_rn = ssdp_upper_response;
  while(1) {
    if((fir_rn = strstr(fir_rn, "\r\n")) == NULL) {
      break;
    }
    fir_rn += strlen("\r\n");
    if((sec_rn = strstr(fir_rn, "\r\n")) == NULL) {
      break;
    }
    end_loc_tag = fir_rn + strlen(LOCATION_TAG);
    end_loc_offset = end_loc_tag - ssdp_upper_response;
    if(!strncmp(fir_rn, LOCATION_TAG, strlen(LOCATION_TAG))) {
      desc_url_malloc_size = (sec_rn - end_loc_tag) + NULL_TERM_LEN;
      *desc_url = (char *)malloc(desc_url_malloc_size);
      if(NULL == *desc_url) {
        free(ssdp_upper_response);
        return BAD_MALLOC;
      }
      sscanf(ssdp_response + end_loc_offset, "%s", *desc_url);
      free(ssdp_upper_response);
      return OK;
    }
  }
  free(ssdp_upper_response);

  /* now make sure that the url size is appropriate */
  if(*desc_url && strlen(*desc_url)+NULL_TERM_LEN > MAX_URL_LEN) {
    free(*desc_url);
    return UPNP_URL_OVER_MAX_LEN;
  }
  return SSDP_NO_LOCATION;
}


/* Get the description from the description url by making an http request to it.
   The caller is responsible for freeing the space allocated for description. */
static int Get_Description(const char * desc_url, char ** description)
{
  int ret = 0;
  char host[MAX_HOST_LEN];
  unsigned short int port;
  char resource[MAX_RESOURCE_LEN];
  GetMessage * gm;

  if((ret = Parse_Url(desc_url, host, resource, &port)) != OK) {
    return ret;
  }

  if((ret = LNat_Generate_Http_Get(host, resource, port, &gm)) != OK) {
    return ret;
  }
  if((ret = LNat_Http_Request_Get(gm, description)) != OK) {
    (void)LNat_Destroy_Http_Get(&gm);
    return ret;
  }

  (void)LNat_Destroy_Http_Get(&gm);
  return OK;
}


/* get the control url from the description, which is in xml format, and
   set the control_url variable in c equal to the string */
static int Get_Control_Url(UpnpController * c, const char * desc_url, 
                           const char * description)
{
  int ret;
  char host[MAX_HOST_LEN];
  char host_port[MAX_HOST_LEN];
  unsigned short int port;
  XML_Parser parser;
  XmlDescData xdescdat = {0, 0, 0, 0, 0, 0, 0};

  if((ret = Parse_Url(desc_url, host, NULL, &port)) != OK) {
    return ret;
  }
  sprintf(host_port, "%s:%d", host, port);
    
  parser = XML_ParserCreate(NULL);
  xdescdat.correct_service = (char *)malloc(strlen(SSDP_TARGET) +
                                            strlen(c->service_type) +
                                            NULL_TERM_LEN);
  (void)sprintf(xdescdat.correct_service, SSDP_TARGET, c->service_type);

  XML_SetUserData(parser, &xdescdat);
  XML_SetElementHandler(parser, Start_Desc_Element, End_Desc_Element);
  XML_SetCharacterDataHandler(parser, Data_Desc_Handler);

  if(!XML_Parse(parser, description, (int)strlen(description), 1)) {
    XML_ParserFree(parser);
    XmlDescData_Free(xdescdat);
    return UPNP_BAD_DESCRIPTION;
  }

  /* if we didn't find a control url, then bail */
  if(NULL == xdescdat.control_url) {
    XmlDescData_Free(xdescdat);
    return UPNP_BAD_DESCRIPTION;
  }

  /* if we didn't find a base url in the description, then it is the
     same url as the desc_url */
  if(NULL == xdescdat.url_base) {
    xdescdat.url_base = (char *)malloc(strlen(host_port) + NULL_TERM_LEN);
    strcpy(xdescdat.url_base, host_port);
  }

  /* if the control_url contains the http protocol string,
     then don't prepend the base url to it */
  if(strncmp(xdescdat.control_url, HTTP_PROTOCOL_STRING, 
             strlen(HTTP_PROTOCOL_STRING))  == 0) {
    c->control_url = (char *)malloc(strlen(xdescdat.control_url) + 
                                    NULL_TERM_LEN);
    if(NULL == c->control_url) {
      XmlDescData_Free(xdescdat);
      return BAD_MALLOC;
    }
    strcpy(c->control_url, xdescdat.control_url);
  } else {
    c->control_url = (char *)malloc(strlen(xdescdat.url_base)+
                                    strlen(xdescdat.control_url) +
                                    NULL_TERM_LEN);
    if(NULL == c->control_url) {
      XmlDescData_Free(xdescdat);
      return BAD_MALLOC;
    }
    (void)sprintf(c->control_url, "%s%s", xdescdat.url_base, xdescdat.control_url);
  }

  XmlDescData_Free(xdescdat);
  XML_ParserFree(parser);

  /* now make sure that the url size is appropriate */
  if(strlen(c->control_url)+NULL_TERM_LEN > MAX_URL_LEN) {
    free(c->control_url);
    c->control_url = NULL;
    return UPNP_URL_OVER_MAX_LEN;
  }
  return OK;
}


static void XmlDescData_Free(XmlDescData d)
{
  if(d.control_url != NULL) {
    free(d.control_url);
  }

  if(d.url_base != NULL) {
    free(d.url_base);
  }

  if(d.correct_service != NULL) {
    free(d.correct_service);
  }
}


/* parser start element handler for the description xml */
static void Start_Desc_Element(void * userData, const char * name,
                             const char ** atts)
{
  XmlDescData * xdescdat = (XmlDescData *)userData;
  if(strcmp(name, BASE_URL_ELEMENT) == 0) {
    xdescdat->found_url_base = 1;
    return;
  }

  if(strcmp(name, SERVICE_TYPE_ELEMENT) == 0) {
    xdescdat->found_service_type = 1;
    return;
  }

  if(strcmp(name, CONTROL_URL_ELEMENT) == 0) {
    xdescdat->found_control_url = 1;
  }
}

/* parser end element handler for the description xml */
static void End_Desc_Element(void * userData, const char * name)
{
  XmlDescData * xdescdat = (XmlDescData *)userData;

  /* if we found the url base, reset the bit */
  if(xdescdat->found_url_base) {
    xdescdat->found_url_base = 0;
  }

  /* if we found a service element, reset the bit */
  if(xdescdat->found_service_type) {
    xdescdat->found_service_type = 0;
  }
  
  /* only reset the correct service flag if we found the control url */
  if(xdescdat->found_correct_service && xdescdat->found_control_url) {
    xdescdat->found_correct_service = 0;
  }

  /* if we found the control url element, we are done, so reset bit */
  if(xdescdat->found_control_url) {
    xdescdat->found_control_url = 0;
  }
}

/* parser character data handler for the description xml */
static void Data_Desc_Handler(void * userData, const char * s, int len)
{
  XmlDescData * xdescdat = (XmlDescData *)userData;

  /* if it is the url base we found, set it */
  if(xdescdat->found_url_base == 1) {
    xdescdat->url_base = (char *)malloc(len+NULL_TERM_LEN);
    if(NULL == xdescdat->url_base) {
      return;
    }
    strncpy(xdescdat->url_base, s, len);
    xdescdat->url_base[len] = NULL_TERM;
    return;
  }

  /* if it is the service_type we found, see if it is the right one */
  if(xdescdat->found_service_type == 1) {
    /* if we fond the right one, set found correct service */
    if(strncmp(xdescdat->correct_service, s, len) == 0) {
      xdescdat->found_correct_service = 1;
    }
    return;
  }

  /* if we found th control url and have the correct service, store controlurl */
  if(xdescdat->found_correct_service && xdescdat->found_control_url) {
    xdescdat->control_url = (char *)malloc(len+NULL_TERM_LEN);
    if(NULL == xdescdat->control_url) {
      return;
    }
    strncpy(xdescdat->control_url, s, len);
    xdescdat->control_url[len] = NULL_TERM;
    return;
  }
}



/********************************************************************************
**                      UPnP Get Public IP Functions                            *
********************************************************************************/

/**
 * Gets the IP address from a UPnP enabled IGD that sits on the local
 * network, so when getting the network IP, instead of returning the
 * local network IP, the public IP is retrieved.
 *
 * controller: The UpnpController retrieved from LNat_Upnp_Discover.
 *
 * return the IP address of the network in a null terminated string
 * in the public_ip parameter or NULL if something went wrong.
 *
 * Return OK if everything was successful.
 */
LIBNAT_API
int LNat_Upnp_Get_Public_Ip(const UpnpController * c,
                            char * public_ip, int public_ip_size)
{
  int ret;
  char * body;
  char * response;

  /* create the body of the http post request */
  ret = Create_Action_Message(c->service_type, GET_PUBLIC_IP_ACTION_NAME,
                              GET_PUBLIC_IP_PARAMS, &body);
  if(OK != ret) {
    return ret;
  }

  ret = Send_Action_Message(c, GET_PUBLIC_IP_ACTION_NAME, body, &response);
  if(OK != ret) {
    free(body);
    return ret;
  }
  free(body);

  ret = Get_Public_Ip_From_Response(response, public_ip, public_ip_size);
  if(OK != ret) {
    free(response);
    return ret;
  }
  free(response);
  
  return OK;
}

/* we have a response of the ip address, so parse it out of the xml */
static int Get_Public_Ip_From_Response(const char * response, 
                                       char * public_ip, int public_ip_size)
{
  XmlGetPublicIpData dat = {0, 0};
  XML_Parser parser = XML_ParserCreate(NULL);

  XML_SetUserData(parser, &dat);
  XML_SetElementHandler(parser, Start_Ip_Element, End_Ip_Element);
  XML_SetCharacterDataHandler(parser, Data_Ip_Handler);

  if(!XML_Parse(parser, response, (int)strlen(response), 1)) {
    XML_ParserFree(parser);
    return UPNP_BAD_PUBLIC_IP_RESPONSE;
  }

  if(NULL == dat.ext_ip) {
    return UPNP_GET_PUBLIC_IP_FAILED;
  }
  strncpy(public_ip, dat.ext_ip, public_ip_size - NULL_TERM_LEN);
  public_ip[public_ip_size - NULL_TERM_LEN] = NULL_TERM;

  free(dat.ext_ip);
  XML_ParserFree(parser);
  return OK;
}


static void Start_Ip_Element(void * userData, const char * name,
                             const char ** atts)
{
  XmlGetPublicIpData * dat  = userData;
  if(strcmp(name, GET_PUBLIC_IP_RESPONSE_ELEMENT) == 0) {
    dat->found_ext_ip = 1;
  }
}


static void End_Ip_Element(void * userData, const char * name)
{
  XmlGetPublicIpData * dat  = userData;
  dat->found_ext_ip = 0;
}


static void Data_Ip_Handler(void * userData, const char * s, int len)
{
  XmlGetPublicIpData * dat  = userData;
  if(dat->found_ext_ip == 1) {
    dat->ext_ip = (char *)malloc(len+NULL_TERM_LEN);
    // TODO insert a check if the malloc fails
    strncpy(dat->ext_ip, s, len);
    dat->ext_ip[len] = NULL_TERM;
  }
}



/********************************************************************************
**                    UPnP Port Mapping Functions                               *
********************************************************************************/


/**
 * Maps Ports in a UPnP enabled IGD that sits on the local network to.
 * Essentially, this function takes care of the port forwarding so things 
 * like file transfers can work behind NAT firewalls.
 *
 * controller: The UpnpController retrieved from LNat_Upnp_Discover.
 * ipMap:      The ip to add this mapping for. If NULL, this function will
 *             attempt to determine the IP address of this machine.
 * portMap:    The port to map to this client
 * protocol:   The protocol to map, either "TCP" or "UDP"
 *
 * return OK if successful.
 */
LIBNAT_API
int LNat_Upnp_Set_Port_Mapping(const UpnpController * c, 
                               const char * ip_map,
                               short int port_map,
                               const char* protocol)
{
  int ret;
  char * body;
  char * response;
  char * params;
  char * local_ip = NULL;

  /* if ipMap is null, attempt to get own ip address */
  if(ip_map == NULL) {
    if((ret = Get_Local_Ip(c, &local_ip)) != OK) {
      return ret;
    }
    ip_map = local_ip;
  }

  params = (char *)malloc(strlen(SET_PORT_MAPPING_PARAMS) + 
                                 MAX_PORT_STRING_LEN +
                                 strlen(protocol) +
                                 MAX_PORT_STRING_LEN +
                                 strlen(ip_map));
  if(NULL == params) {
    free(local_ip);
    return BAD_MALLOC;
  }
  (void)sprintf(params, SET_PORT_MAPPING_PARAMS, port_map, 
                protocol, port_map, ip_map);

  /* create the body of the http post request */
  ret = Create_Action_Message(c->service_type, SET_PORT_MAPPING_ACTION_NAME,
                              params, &body);
  if(OK != ret) {
    free(local_ip);
    free(params);
    return ret;
  }
  free(params);

  ret = Send_Action_Message(c, SET_PORT_MAPPING_ACTION_NAME, body, &response);
  if(OK != ret) {
    free(local_ip);
    free(body);
    return ret;
  }
  free(local_ip);
  free(body);
  free(response);
  
  return OK;
}

static int Get_Local_Ip(const UpnpController * c, char ** ip_map)
{
  int ret;
  OsSocket * s;
  char host[MAX_HOST_LEN];
  char resource[MAX_RESOURCE_LEN];
  unsigned short int port;

  /* parse the host, resource, and port out of the url */
  ret = Parse_Url(c->control_url, host, resource, &port);
  if(OK != ret) {
    return ret;
  }

  if((ret = LNat_Os_Socket_Connect(&s, host, port, 2)) != OK) {
    return ret;
  }

  if((ret = LNat_Os_Get_Local_Ip(s, ip_map)) != OK) {
    return ret;
  }

  (void)LNat_Os_Socket_Close(&s);
  return OK;
}


/**
 * Deletes a port mapping in a UPnP enabled IGD that sits on the local network.
 * Essentially, this function takes care of deleting the port forwarding after 
 * they have completed a connection so another client on the local network can 
 * take advantage of the port forwarding
 *
 * controller: The UpnpController retrieved from LNat_Upnp_Discover.
 * ipMap:      The ip to delete this mapping for. If NULL, this function will
 *             attempt to determine the IP address of this machine.
 * portMap:    The port to delete the mapping for
 * protocol :  The protocol the port was mapped to. Either "TCP" or "UDP"
 *
 * return OK if successful.
 */
LIBNAT_API
int LNat_Upnp_Remove_Port_Mapping(const UpnpController * c,
                                  short int port_map,
                                  const char* protocol)
{
  int ret;
  char * body;
  char * response;
  char * params;

  params = (char *)malloc(strlen(REMOVE_PORT_MAPPING_PARAMS) + 
                                 MAX_PORT_STRING_LEN +
                                 strlen(protocol));
  if(NULL == params) {
    return BAD_MALLOC;
  }
  (void)sprintf(params, REMOVE_PORT_MAPPING_PARAMS, port_map, protocol);

  /* create the body of the http post request */
  ret = Create_Action_Message(c->service_type, REMOVE_PORT_MAPPING_ACTION_NAME,
                              params, &body);
  if(OK != ret) {
    free(params);
    return ret;
  }
  free(params);

  ret = Send_Action_Message(c, REMOVE_PORT_MAPPING_ACTION_NAME, body, &response);
  if(OK != ret) {
    free(body);
    return ret;
  }
  free(response);
  free(body);
  
  return OK;
}




/********************************************************************************
**                        Other Helper Functions                                *
********************************************************************************/

/**
 * Destroy a UpnpController object that was allocated by LNat_Upnp_Discover.
 * return OK if successful.
 */
LIBNAT_API
int LNat_Upnp_Controller_Free(UpnpController ** controller)
{
  if(NULL == *controller) {
    return OK;
  }
  if(NULL != (*controller)->control_url) {
    free((*controller)->control_url);
  }
  if(NULL != (*controller)->service_type) {
    free((*controller)->service_type);
  }
  free(*controller);
  return OK;
}


/* send an action message using http post */
static int Send_Action_Message(const UpnpController * c,
                               const char * action_name,
                               const char * body,
                               char ** response)
{
  int ret;
  char host[MAX_HOST_LEN];
  char resource[MAX_RESOURCE_LEN];
  unsigned short int port;
  char * soap_action_header;
  char * content_length_header;
  PostMessage * pm;

  /* parse the host, resource, and port out of the url */
  ret = Parse_Url(c->control_url, host, resource, &port);
  if(OK != ret) {
    return ret;
  }

  /* generate an http_post request */
  ret = LNat_Generate_Http_Post(host, resource, port, body, &pm);
  if(OK != ret) {
    return ret;
  }

  /* add some entity headers, start with content length */
  content_length_header = (char *)malloc(MAX_CONTENT_STRING_LEN +
                                         NULL_TERM_LEN);
  if(NULL == content_length_header) {
    LNat_Destroy_Http_Post(&pm);
    return BAD_MALLOC;
  }
  (void)sprintf(content_length_header, CONTENT_LENGTH_VAL, (int)strlen(body));
  ret = LNat_Http_Post_Add_Entity_Header(pm, CONTENT_LENGTH_TAG, content_length_header);
  if(OK != ret) {
    free(content_length_header);
    LNat_Destroy_Http_Post(&pm);
    return ret;
  }
  free(content_length_header);

  /* add content type entity header */
  ret = LNat_Http_Post_Add_Entity_Header(pm, CONTENT_TYPE_TAG, CONTENT_TYPE_VAL);
  if(OK != ret) {
    LNat_Destroy_Http_Post(&pm);
    return ret;
  }

  /* add soap action entity header */
  soap_action_header = (char *)malloc(strlen(SOAP_ACTION_VAL) +
                                      strlen(c->service_type) +
                                      strlen(action_name) +
                                      NULL_TERM_LEN);
  if(NULL == soap_action_header) {
    LNat_Destroy_Http_Post(&pm);
    return BAD_MALLOC;
  }
  (void)sprintf(soap_action_header, SOAP_ACTION_VAL, c->service_type, action_name);
  ret = LNat_Http_Post_Add_Entity_Header(pm, SOAP_ACTION_TAG, soap_action_header);
  if(OK != ret) {
    free(soap_action_header);
    LNat_Destroy_Http_Post(&pm);
    return ret;
  }
  free(soap_action_header);

  /* now send the http request */
  ret = LNat_Http_Request_Post(pm, response);
  if(OK != ret) {
    return ret;
  }

  (void)LNat_Destroy_Http_Post(&pm);
  return OK;
}



/* create an action message for one of the action requests, such as
   Get_Public_Ip, Set_Port_Mapping, and Remove_Port_Mapping */
static int Create_Action_Message(const char * service_type,
                                 const char * action_name, 
                                 const char * action_params,
                                 char ** action_message)
{
  *action_message = (char *)malloc(strlen(SOAP_ACTION) +
                                   strlen(service_type) +
                                   strlen(action_name) +
                                   strlen(action_params) +
                                   strlen(action_name) +
                                   NULL_TERM_LEN);
  if(NULL == *action_message) {
    return BAD_MALLOC;
  }

  (void)sprintf(*action_message, SOAP_ACTION, action_name,
                service_type, action_params, action_name);
  return OK;
}




static int Parse_Url(const char * url, char * host, 
                     char * resource, unsigned short int * port)
{
  char * loc_of_semicolon;
  char * loc_of_slash;
  char * end_of_host_loc;
  char * loc_of_resource;
  char * loc_of_port;
  char url_wo_http[MAX_HOST_LEN];

  if(sscanf(url, "http://%255s", url_wo_http) != 1) {
    if(sscanf(url, "%255s", url_wo_http) != 1) {
      return HTTP_INVALID_URL;
    }
  }

  loc_of_semicolon = strchr(url_wo_http, ':');
  loc_of_slash = strchr(url_wo_http, '/');

  /* we need a slash for the resource */
  if(NULL == loc_of_slash) {
    return HTTP_INVALID_URL;
  } else {
    loc_of_resource = loc_of_slash+1;
  }

  /* dont necessarily need a port, so if we don't have one default port is 80 */
  if(NULL == loc_of_semicolon || loc_of_semicolon > loc_of_slash) {
    loc_of_semicolon = NULL;
    if(NULL != port) {
      *port = DEFAULT_HTTP_PORT;
    }
    end_of_host_loc = loc_of_slash;
  } else {
    /* port is right after the semicolon */
    loc_of_port = loc_of_semicolon + 1;
    /* make sure it is not larger than MAX_PORT_STRING_LENGTH */
    if((loc_of_slash - loc_of_port) > MAX_PORT_STRING_LEN) {
      return HTTP_INVALID_URL;
    }
    end_of_host_loc = loc_of_semicolon;
  }
  
  /* store the host */
  if(NULL != host) {
    strncpy(host, url_wo_http, end_of_host_loc - url_wo_http);
    host[end_of_host_loc - url_wo_http] = NULL_TERM;
  }
  /* extract the port */
  if(NULL != loc_of_semicolon) {
    if(sscanf(loc_of_port, "%hu", port) != 1) {
      *port = DEFAULT_HTTP_PORT;
    }
    if(*port > MAX_PORT_SIZE) {
      return HTTP_INVALID_URL;
    }
  }
  /* extract the resource */
  if(NULL != resource) {
    strcpy(resource, loc_of_resource);
  }

  return OK;
}


