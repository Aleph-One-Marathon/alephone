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
 
 	Jan 16, 2002 (Loren Petrich) Made 
 */

#ifndef	NETWORK_DATA_FORMATS_H
#define	NETWORK_DATA_FORMATS_H

#include	"cseries.h"		// Need ALEPHONE_LITTLE_ENDIAN, if appropriate.
#include	"network.h"
#include	"network_private.h"



// LP: All the packed net-packet structs have been turned into wrappers for plain bytes

const int SIZEOF_game_info = MAX_LEVEL_NAME_LENGTH + 23;

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



const int SIZEOF_NetPlayer = 2*SIZEOF_IPaddress + MAXIMUM_PLAYER_DATA_SIZE + 3;

struct NetPlayer_NET {
	uint8 data[SIZEOF_NetPlayer];
};

extern void netcpy(NetPlayer_NET* dest, const NetPlayer* src);
extern void netcpy(NetPlayer* dest, const NetPlayer_NET* src);



const int SIZEOF_NetTopology = MAXIMUM_GAME_DATA_SIZE +
	MAXIMUM_NUMBER_OF_NETWORK_PLAYERS*SIZEOF_NetPlayer + 6;

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


#if OBSOLETE

// We will fake the Mac version for now, to maintain total compatibility with old formats...
// We do an actual copy rather than pointer reassignment so semantics are the same as "real" netcpy.
#ifdef MAC
// original macros, replaced with inlines below - does this look right?
//#define netcpy(a,b) memcpy((a),(b),sizeof(*(a)))
//#define netcpy(a,b,s) memcpy((a), (b), (s))

template<class Td, class Ts>
__inline__ void netcpy(Td* dest, const Ts* src) { memcpy(dest, src, sizeof(Td)); }

__inline__ void netcpy(uint32* dest, const uint32* src, size_t length) { memcpy(dest, src, length); }

typedef game_info		game_info_NET;
typedef	player_info		player_info_NET;
typedef	NetPacketHeader		NetPacketHeader_NET;
typedef	NetPacket		NetPacket_NET;
typedef	NetDistributionPacket	NetDistributionPacket_NET;
typedef	AddrBlock		IPaddress_NET;
typedef	NetTopology		NetTopology_NET;
typedef	NetPlayer		NetPlayer_NET;
typedef	gather_player_data	gather_player_data_NET;
typedef	accept_gather_data	accept_gather_data_NET;
#ifdef NETWORK_CHAT
typedef	NetChatMessage		NetChatMessage_NET;
#endif

#else // !MAC


// OK, everyone else gets messed with.  :)

#ifdef __GNUC__		// GNU CC compiler
#define PACKED_ATTRIBUTE __attribute__ ((packed))
#else
#define PACKED_ATTRIBUTE
#ifndef _MSC_VER
#error You must arrange for the structures in this file to be close-packed.
#endif
#endif

#ifdef _MSC_VER		// Microsoft Visual C++ compiler
#pragma pack(push,1)
#endif

struct game_info_NET {
	uint16 initial_random_seed;
	int16  net_game_type;
	int32  time_limit;
	int16  kill_limit;
	int16  game_options;
	int16  difficulty_level;
	char   server_is_playing; // if false, then observing
	char   allow_mic;
	
	// where the game takes place
	int16  level_number;
	char   level_name[MAX_LEVEL_NAME_LENGTH+1];
	
	// network parameters
	int16  initial_updates_per_packet;
	int16  initial_update_latency;
} PACKED_ATTRIBUTE;

extern void netcpy(game_info_NET* dest, const game_info* src);
extern void netcpy(game_info* dest, const game_info_NET* src);



struct player_info_NET {
	unsigned char name[MAX_NET_PLAYER_NAME_LENGTH+1];
	int16 desired_color;
	int16 team;   // from player.h
	int16 color;
	byte long_serial_number[10];
} PACKED_ATTRIBUTE;

extern void netcpy(player_info_NET* dest, const player_info* src);
extern void netcpy(player_info* dest, const player_info_NET* src);



// Note: no further interpretation/manipulation of a packet is attempted here.  That's up
// to whomever actually deals with receiving and interpreting the packet (though they will
// probably make use of the netcpy's for the next couple structures below).
struct NetPacketHeader_NET
{
	int16  tag;
	int32  sequence;
	
	/* data */
} PACKED_ATTRIBUTE;

extern void netcpy(NetPacketHeader_NET* dest, const NetPacketHeader* src);
extern void netcpy(NetPacketHeader* dest, const NetPacketHeader_NET* src);



// Note: we do not attempt any manipulations on the actual action_flags, as we do not claim
// to compute the number that would be there.  (I suppose, knowing that the only stuff in the
// buffer will be action flags, that we could just walk through and swap all of it, but...)
// We'll leave it to whomever interprets or writes to the action_flags data segment to do the
// necessary manipulations.
struct NetPacket_NET
{
	byte ring_packet_type;         // typeSYNC_RING_PACKET, etc...
	byte server_player_index;
	int32 server_net_time;
	int16 required_action_flags;                         // handed down from on high (the server)
	int16 action_flag_count[MAXIMUM_NUMBER_OF_NETWORK_PLAYERS];  // how many each player actually has.
//	uint32 action_flags[1];
} PACKED_ATTRIBUTE;

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
struct NetDistributionPacket_NET
{
	int16 distribution_type;  // type of information
	int16 first_player_index; // who sent the information
	int16 data_size;          // how much they're sending.
//	byte  data[2];            // the chunk Õo shit to send
} PACKED_ATTRIBUTE;

extern void netcpy(NetDistributionPacket_NET* dest, const NetDistributionPacket* src);
extern void netcpy(NetDistributionPacket* dest, const NetDistributionPacket_NET* src);



// Note: unlike other _NET structures, neither 'host' nor 'port' here is byte-swapped
// in conversion to/from the non-_NET structures.
// It is believed that both should ALWAYS be dealt with in network byte order.
struct IPaddress_NET {
    uint32	host;
    uint16	port;
} PACKED_ATTRIBUTE;

extern void netcpy(IPaddress_NET* dest, const IPaddress* src);
extern void netcpy(IPaddress* dest, const IPaddress_NET* src);



struct NetPlayer_NET {
	IPaddress_NET dspAddress;
    IPaddress_NET ddpAddress;
	
	int16 identifier;

	char net_dead; // only valid if you are the server.

	player_info_NET	player_data;
} PACKED_ATTRIBUTE;

extern void netcpy(NetPlayer_NET* dest, const NetPlayer* src);
extern void netcpy(NetPlayer* dest, const NetPlayer_NET* src);



struct NetTopology_NET
{
	int16 tag;
	int16 player_count;
	
	int16 nextIdentifier;
	
	game_info_NET game_data;
	
	NetPlayer_NET players[MAXIMUM_NUMBER_OF_NETWORK_PLAYERS];
} PACKED_ATTRIBUTE;

extern void netcpy(NetTopology_NET* dest, const NetTopology* src);
extern void netcpy(NetTopology* dest, const NetTopology_NET* src);



struct gather_player_data_NET {
	short new_local_player_identifier;
} PACKED_ATTRIBUTE;

extern void netcpy(gather_player_data_NET* dest, const gather_player_data* src);
extern void netcpy(gather_player_data* dest, const gather_player_data_NET* src);



struct accept_gather_data_NET {
	char accepted;
	NetPlayer_NET player;
} PACKED_ATTRIBUTE;

extern void netcpy(accept_gather_data_NET* dest, const accept_gather_data* src);
extern void netcpy(accept_gather_data* dest, const accept_gather_data_NET* src);



#ifdef NETWORK_CHAT
struct NetChatMessage_NET {
    uint16  sender_identifier;
    char    text[CHAT_MESSAGE_TEXT_BUFFER_SIZE];
} PACKED_ATTRIBUTE;

extern void netcpy(NetChatMessage_NET* dest, const NetChatMessage* src);
extern void netcpy(NetChatMessage* dest, const NetChatMessage_NET* src);
#endif



#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif// ndef MAC

#endif // OBSOLETE

#endif//NETWORK_DATA_FORMATS_H
