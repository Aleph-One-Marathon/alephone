/*
 *  sdl_network.h - Definitions for SDL implementation of networking
 */

#ifndef __SDL__NETWORK_H
#define __SDL__NETWORK_H

#include <SDL_net.h>

#include "network.h"

/* ---------- constants */

#define asyncUncompleted 1	/* ioResult value */

#define strNETWORK_ERRORS 132
enum /* error string for user */
{
	netErrCantAddPlayer,
	netErrCouldntDistribute,
	netErrCouldntJoin,
	netErrServerCanceled,
	netErrMapDistribFailed,
	netErrWaitedTooLongForMap,
	netErrSyncFailed,
	netErrJoinFailed,
	netErrCantContinue
};

/* missing from AppleTalk.h */
#define ddpMaxData 586

/* default IP port number for Marathon */
const uint16 DEFAULT_PORT = 4226;

typedef char EntityName[32];
typedef IPaddress AddrBlock;

/* ---------- DDPFrame and PacketBuffer (DDP) */

struct DDPFrame
{
	short data_size;
	byte data[ddpMaxData];
	UDPsocket socket;
};
typedef struct DDPFrame DDPFrame, *DDPFramePtr;

struct DDPPacketBuffer
{
	byte protocolType;
	AddrBlock sourceAddress;
	
	short datagramSize;
	byte datagramData[ddpMaxData];
};
typedef struct DDPPacketBuffer DDPPacketBuffer, *DDPPacketBufferPtr;

/* ---------- ConnectionEnd (ADSP) */

struct ConnectionEnd
{
	TCPsocket socket, server_socket;
};
typedef struct ConnectionEnd ConnectionEnd, *ConnectionEndPtr;

/* ---------- types */

typedef EntityName *EntityNamePtr;

typedef void (*lookupUpdateProcPtr)(short message, short index);
typedef bool (*lookupFilterProcPtr)(EntityName *entity, AddrBlock *address);
typedef void (*PacketHandlerProcPtr)(DDPPacketBufferPtr packet);

/* ---------- prototypes/NETWORK.C */

short NetState(void);

void NetSetServerIdentifier(short identifier);

/* for giving to NetLookupOpen() as a filter procedure */
bool NetEntityNotInGame(EntityName *entity, AddrBlock *address);

/* ---------- prototypes/NETWORK_NAMES.C */

OSErr NetRegisterName(unsigned char *name, unsigned char *type, short version, short socketNumber);
OSErr NetUnRegisterName(void);

/* ---------- prototypes/NETWORK_LOOKUP.C */

void NetLookupUpdate(void);
void NetLookupClose(void);
OSErr NetLookupOpen(unsigned char *name, unsigned char *type, unsigned char *zone, short version, 
	lookupUpdateProcPtr updateProc, lookupFilterProcPtr filterProc);
void NetLookupRemove(short index);
void NetLookupInformation(short index, AddrBlock *address, EntityName *entity);

/* ---------- prototypes/NETWORK_DDP.C */

OSErr NetDDPOpen(void);
OSErr NetDDPClose(void);

OSErr NetDDPOpenSocket(short *socketNumber, PacketHandlerProcPtr packetHandler);
OSErr NetDDPCloseSocket(short socketNumber);

DDPFramePtr NetDDPNewFrame(void);
void NetDDPDisposeFrame(DDPFramePtr frame);

OSErr NetDDPSendFrame(DDPFramePtr frame, AddrBlock *address, short protocolType, short socket);

/* ---------- prototypes/NETWORK_ADSP.C */

OSErr NetADSPOpen(void);
OSErr NetADSPClose(void);

OSErr NetADSPEstablishConnectionEnd(ConnectionEndPtr *connection);
OSErr NetADSPDisposeConnectionEnd(ConnectionEndPtr connectionEnd);

OSErr NetADSPOpenConnection(ConnectionEndPtr connectionEnd, AddrBlock *address);
OSErr NetADSPCloseConnection(ConnectionEndPtr connectionEnd, bool abort);
OSErr NetADSPWaitForConnection(ConnectionEndPtr connectionEnd);
bool NetADSPCheckConnectionStatus(ConnectionEndPtr connectionEnd, AddrBlock *address);

OSErr NetADSPWrite(ConnectionEndPtr connectionEnd, void *buffer, uint16 *count);
OSErr NetADSPRead(ConnectionEndPtr connectionEnd, void *buffer, uint16 *count);

#endif
