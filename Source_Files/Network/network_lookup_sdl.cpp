/*
 *  network_lookup_sdl.cpp - Network entity lookup functions, SDL implementation
 *
 *  Written in 2000 by Christian Bauer
 */

#include "cseries.h"
#include "sdl_network.h"


/*
 *  Start lookup
 */

OSErr NetLookupOpen(unsigned char *name, unsigned char *type, unsigned char *zone, short version, lookupUpdateProcPtr updateProc, lookupFilterProcPtr filterProc)
{
printf("NetLookupOpen %s, %s, %s, %d\n", name, type, zone, version);
	return 0;
}


/*
 *  Stop lookup
 */

void NetLookupClose(void)
{
printf("NetLookupClose\n");
}


/*
 *  Remove entity from name list
 */

void NetLookupRemove(short index)
{
printf("NetLookupRemove %d\n", index);
}


/*
 *  Get information for entity
 */

void NetLookupInformation(short index, AddrBlock *address, EntityName *entity)
{
printf("NetLookupInformation %d\n", index);
}


/*
 *  Update entity list
 */

void NetLookupUpdate(void)
{
printf("NetLookupUpdate\n");
}


/*
 *  Register entity
 */

OSErr NetRegisterName(unsigned char *name, unsigned char *type, short version, short socketNumber)
{
printf("NetRegisterName %s, %s, %d, %d\n", name, type, version, socketNumber);
	return 0;
}


/*
 *  Unregister entity
 */

OSErr NetUnRegisterName(void)
{
printf("NetUnRegisterName\n");
	return 0;
}
