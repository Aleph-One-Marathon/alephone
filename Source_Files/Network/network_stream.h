#ifndef __NETWORK_STREAM_H
#define __NETWORK_STREAM_H

/*

	network_stream.h
	Saturday, September 30, 1995 8:01:13 PM- rdm created.

*/

enum {
	kNetworkTransportType= 0,
	kModemTransportType,
	NUMBER_OF_TRANSPORT_TYPES
};

boolean NetTransportAvailable(short type);

OSErr NetStreamEstablishConnectionEnd(void);
OSErr NetStreamDisposeConnectionEnd(void);

OSErr NetOpenStreamToPlayer(short player_index);
OSErr NetCloseStreamConnection(boolean abort);

OSErr NetSendStreamPacket(short packet_type, void *packet_data);
OSErr NetReceiveStreamPacket(short *packet_type, void *buffer);

short NetGetTransportType(void);
void NetSetTransportType(short type);

boolean NetStreamCheckConnectionStatus(void);
OSErr NetStreamWaitForConnection(void);

void NetGetStreamAddress(AddrBlock *address);
short NetGetStreamSocketNumber(void);

/* ------ application must supply these */
word MaxStreamPacketLength(void);
word NetStreamPacketLength(short packet_type);
AddrBlock *NetGetPlayerADSPAddress(short player_index);

#endif
