/*
 *  network_data_formats.cpp

	Copyright (C) 2001 and beyond by Woody Zenfell, III
	and the "Aleph One" developers.
 
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	This license is contained in the file "COPYING",
	which is included with this source code; it is available online at
	http://www.gnu.org/licenses/gpl.html

 *  Created by Woody Zenfell, III on Thu Oct 11 2001.

	16 Jan 2002 (Loren Petrich): Added packing/unpacking functions

    5 Mar 2002 (Woody Zenfell):  Added network_audio_header netcpy()s.

    9 Mar 2002 (Woody Zenfell):  Made some packing/unpacking functions pack/unpack more data
 */

#if !defined(DISABLE_NETWORKING)

#include "network_data_formats.h"
#include "Packing.h"

void
netcpy(NetPacketHeader_NET* dest, const NetPacketHeader* src)
{
	uint8 *S = dest->data;
	ValueToStream(S,src->tag);
	ValueToStream(S,src->sequence);
	assert(S == dest->data + SIZEOF_NetPacketHeader);
}

void
netcpy(NetPacketHeader* dest, const NetPacketHeader_NET* src)
{
	uint8 *S = (uint8 *)src->data;
	StreamToValue(S,dest->tag);
	StreamToValue(S,dest->sequence);
	assert(S == src->data + SIZEOF_NetPacketHeader);
}

void
netcpy(NetPacket_NET* dest, const NetPacket* src)
{
	uint8 *S = dest->data;
	*(S++) = src->ring_packet_type;
	*(S++) = src->server_player_index;
	ValueToStream(S,src->server_net_time);
	ValueToStream(S,src->required_action_flags);
	for (int i=0; i<MAXIMUM_NUMBER_OF_NETWORK_PLAYERS; i++)
		ValueToStream(S,src->action_flag_count[i]);
	assert(S == dest->data + SIZEOF_NetPacket);
}

void
netcpy(NetPacket* dest, const NetPacket_NET* src)
{
	uint8 *S = (uint8 *)src->data;
	dest->ring_packet_type = *(S++);
	dest->server_player_index = *(S++);
	StreamToValue(S,dest->server_net_time);
	StreamToValue(S,dest->required_action_flags);
	for (int i=0; i<MAXIMUM_NUMBER_OF_NETWORK_PLAYERS; i++)
		StreamToValue(S,dest->action_flag_count[i]);
	assert(S == src->data + SIZEOF_NetPacket);
}

// (if not ALEPHONE_LITTLE_ENDIAN, this is unnecessary as memcpy is used instead.)
#ifdef ALEPHONE_LITTLE_ENDIAN
void
netcpy(uint32* dest, const uint32* src, size_t length) {
    assert(length % sizeof(uint32) == 0);
    
    size_t	num_items = length / sizeof(uint32);
	
	uint8 *S = (uint8 *)dest;
	ListToStream(S,src,num_items);
	assert(S == (uint8 *)dest + length);
}
#endif

void
netcpy(NetDistributionPacket_NET* dest, const NetDistributionPacket* src)
{
	uint8 *S = dest->data;
	ValueToStream(S,src->distribution_type);
	ValueToStream(S,src->first_player_index);
	ValueToStream(S,src->data_size);
	assert(S == dest->data + SIZEOF_NetDistributionPacket);
}

void
netcpy(NetDistributionPacket* dest, const NetDistributionPacket_NET* src)
{
	uint8 *S = (uint8 *)src->data;
	StreamToValue(S,dest->distribution_type);
	StreamToValue(S,dest->first_player_index);
	StreamToValue(S,dest->data_size);
	assert(S == src->data + SIZEOF_NetDistributionPacket);
}

// IP addresses are always in network byte order - do not swap
void
netcpy(IPaddress_NET* dest, const IPaddress* src)
{
	uint8 *S = dest->data;
	memcpy(S,&src->host,4);	// 32-bit integer
	S += 4;
	memcpy(S,&src->port,2);	// 16-bit integer
}

void
netcpy(IPaddress* dest, const IPaddress_NET* src) {
	uint8 *S = (uint8 *)src->data;
	memcpy(&dest->host,S,4);	// 32-bit integer
	S += 4;
	memcpy(&dest->port,S,2);	// 16-bit integer
}

void
netcpy(network_audio_header_NET* dest, const network_audio_header* src) {
    uint8* S = dest->data;
    ValueToStream(S, src->mReserved);
    ValueToStream(S, src->mFlags);
    assert(S == dest->data + SIZEOF_network_audio_header);
}

void
netcpy(network_audio_header* dest, const network_audio_header_NET* src) {
    uint8* S = (uint8*) src->data;
    StreamToValue(S, dest->mReserved);
    StreamToValue(S, dest->mFlags);
    assert(S == src->data + SIZEOF_network_audio_header);
}

#endif // !defined(DISABLE_NETWORKING)

