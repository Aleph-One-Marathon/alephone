/*
 *  network_data_formats.h

	Copyright (C) 1991-2001 and beyond by Bungie Studios, Inc.
	and the "Aleph One" developers.
 
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	This license is contained in the file "COPYING",
	which is included with this source code; it is available online at
	http://www.gnu.org/licenses/gpl.html

 *  The purpose of this file is to define structures that will be used with exactly the same
 *  padding, byte ordering, etc. on all platforms, and to declare handy functions to copy data
 *  to or from the corresponding "unpacked" structures already used by the code.  This approach
 *  requires only minimal changes to the original code.
 *
 *  Created by Woody Zenfell, III on Thu Oct 11 2001; structures copied from network_private.h.
 
 	Jan 16, 2002 (Loren Petrich) Replaced compiler-specific packing code with generalized wrappers

    Mar 5, 2002 (Woody Zenfell) added network_audio_header

    Mar 9, 2002 (Woody Zenfell) changed some SIZEOF_'s to be more accurate/specific
 */

#ifndef	NETWORK_DATA_FORMATS_H
#define	NETWORK_DATA_FORMATS_H

#include	"cseries.h"		// Need ALEPHONE_LITTLE_ENDIAN, if appropriate.
#include	"network.h"
#include	"network_private.h"
#include    "network_audio_shared.h"



// LP: All the packed net-packet structs have been turned into wrappers for plain bytes

const int SIZEOF_game_info = MAX_LEVEL_NAME_LENGTH + 25;

struct game_info_NET
{
	uint8 data[SIZEOF_game_info];
};

extern void netcpy(game_info_NET* dest, const game_info* src);
extern void netcpy(game_info* dest, const game_info_NET* src);



const int SIZEOF_player_info = MAX_NET_PLAYER_NAME_LENGTH + LONG_SERIAL_NUMBER_LENGTH + 7;

struct player_info_NET {
	uint8 data[SIZEOF_player_info];
};

extern void netcpy(player_info_NET* dest, const player_info* src);
extern void netcpy(player_info* dest, const player_info_NET* src);



// Note: no further interpretation/manipulation of a packet is attempted here.  That's up
// to whomever actually deals with receiving and interpreting the packet (though they will
// probably make use of the netcpy's for the next couple structures below).

const int SIZEOF_NetPacketHeader = 6;

struct NetPacketHeader_NET
{
	uint8 data[SIZEOF_NetPacketHeader];
};

extern void netcpy(NetPacketHeader_NET* dest, const NetPacketHeader* src);
extern void netcpy(NetPacketHeader* dest, const NetPacketHeader_NET* src);



// Note: we do not attempt any manipulations on the actual action_flags, as we do not claim
// to compute the number that would be there.  (I suppose, knowing that the only stuff in the
// buffer will be action flags, that we could just walk through and swap all of it, but...)
// We'll leave it to whomever interprets or writes to the action_flags data segment to do the
// necessary manipulations.

const int SIZEOF_NetPacket = 2*MAXIMUM_NUMBER_OF_NETWORK_PLAYERS + 8;

struct NetPacket_NET
{
	uint8 data[SIZEOF_NetPacket];
};

extern void netcpy(NetPacket_NET* dest, const NetPacket* src);
extern void netcpy(NetPacket* dest, const NetPacket_NET* src);



// For action flags - note length is in bytes, not number of flags.  This is 'bidirectional',
// i.e. same function is used to copy from _NET to unpacked as the other way around.
// Note, since there is no packing to do - only byte swapping - we can pass along to memcpy if we're
// on a big-endian architecture.
#ifdef ALEPHONE_LITTLE_ENDIAN
extern void netcpy(uint32* dest, const uint32* src, size_t length);
#else
__inline__ void netcpy(uint32* dest, const uint32* src, size_t length) { memcpy(dest, src, length); }
#endif



// Note: we do not attempt any sort of processing on the "data" segment here,
// since we may not understand its format.  Whoever interprets it will have to
// do the necessary packing/unpacking, byte-swapping, etc.

const int SIZEOF_NetDistributionPacket = 6;

struct NetDistributionPacket_NET
{
	uint8 data[SIZEOF_NetDistributionPacket];
};

extern void netcpy(NetDistributionPacket_NET* dest, const NetDistributionPacket* src);
extern void netcpy(NetDistributionPacket* dest, const NetDistributionPacket_NET* src);


// Note: unlike other _NET structures, neither 'host' nor 'port' here is byte-swapped
// in conversion to/from the non-_NET structures.
// It is believed that both should ALWAYS be dealt with in network byte order.

const int SIZEOF_IPaddress = 6;

struct IPaddress_NET {
   uint8 data[SIZEOF_IPaddress];
};

extern void netcpy(IPaddress_NET* dest, const IPaddress* src);
extern void netcpy(IPaddress* dest, const IPaddress_NET* src);


// ZZZ fix: a NetPlayer holds a player_info, not general player data as might be implied
//const int SIZEOF_NetPlayer = 2*SIZEOF_IPaddress + MAXIMUM_PLAYER_DATA_SIZE + 3;
const int SIZEOF_NetPlayer = 2*SIZEOF_IPaddress + SIZEOF_player_info + 5;

struct NetPlayer_NET {
	uint8 data[SIZEOF_NetPlayer];
};

extern void netcpy(NetPlayer_NET* dest, const NetPlayer* src);
extern void netcpy(NetPlayer* dest, const NetPlayer_NET* src);


// ZZZ fix: NetTopology holds game_info, not general unformatted game data as it might appear.
//const int SIZEOF_NetTopology = MAXIMUM_GAME_DATA_SIZE +
//	MAXIMUM_NUMBER_OF_NETWORK_PLAYERS*SIZEOF_NetPlayer + 6;
const int SIZEOF_NetTopology = SIZEOF_game_info + MAXIMUM_NUMBER_OF_NETWORK_PLAYERS*SIZEOF_NetPlayer + 6;

struct NetTopology_NET
{
	uint8 data[SIZEOF_NetTopology];
};

extern void netcpy(NetTopology_NET* dest, const NetTopology* src);
extern void netcpy(NetTopology* dest, const NetTopology_NET* src);



const int SIZEOF_gather_player_data = 2;

struct gather_player_data_NET {
	uint8 data[SIZEOF_gather_player_data];
};

extern void netcpy(gather_player_data_NET* dest, const gather_player_data* src);
extern void netcpy(gather_player_data* dest, const gather_player_data_NET* src);



const int SIZEOF_accept_gather_data = SIZEOF_NetPlayer + 1;

struct accept_gather_data_NET {
	uint8 data[SIZEOF_accept_gather_data];
};

extern void netcpy(accept_gather_data_NET* dest, const accept_gather_data* src);
extern void netcpy(accept_gather_data* dest, const accept_gather_data_NET* src);


#ifdef NETWORK_CHAT
const int SIZEOF_NetChatMessage = CHAT_MESSAGE_TEXT_BUFFER_SIZE + 2;

struct NetChatMessage_NET {
    uint8 data[SIZEOF_NetChatMessage];
};

extern void netcpy(NetChatMessage_NET* dest, const NetChatMessage* src);
extern void netcpy(NetChatMessage* dest, const NetChatMessage_NET* src);
#endif


const int SIZEOF_network_audio_header = 8;

struct network_audio_header_NET {
    uint8 data[SIZEOF_network_audio_header];
};

extern void netcpy(network_audio_header_NET* dest, const network_audio_header* src);
extern void netcpy(network_audio_header* dest, const network_audio_header_NET* src);


const int SIZEOF_prospective_joiner_info = MAX_NET_PLAYER_NAME_LENGTH + 4;

struct prospective_joiner_info_NET {
    uint8 data[SIZEOF_prospective_joiner_info];
};

extern void netcpy(prospective_joiner_info_NET* dest, const prospective_joiner_info* src);
extern void netcpy(prospective_joiner_info* dest, const prospective_joiner_info_NET* src);

#endif//NETWORK_DATA_FORMATS_H
