/*
 *  network_data_formats.cpp

	Copyright (C) 2001 and beyond by Woody Zenfell, III
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

 *  Created by Woody Zenfell, III on Thu Oct 11 2001.

	16 Jan 2002 (Loren Petrich): Added packing/unpacking functions
 */

#include	"network_data_formats.h"
#include "Packing.h"

#pragma mark game_info

void
netcpy(game_info_NET* dest, const game_info* src)
{
	uint8 *S = dest->data;
	ValueToStream(S,src->initial_random_seed);
	ValueToStream(S,src->net_game_type);
	ValueToStream(S,src->time_limit);
	ValueToStream(S,src->kill_limit);
	ValueToStream(S,src->game_options);
	ValueToStream(S,src->difficulty_level);
	*(S++) = src->server_is_playing ? 1 : 0;
	*(S++) = src->allow_mic ? 1 : 0;
	ValueToStream(S,src->level_number);
	BytesToStream(S,src->level_name,sizeof(src->level_name));
	ValueToStream(S,src->initial_updates_per_packet);
	ValueToStream(S,src->initial_update_latency);
	assert(S == dest->data + SIZEOF_game_info);

#ifdef OBSOLETE
    dest->initial_random_seed		= SDL_SwapBE16(src->initial_random_seed);
    dest->net_game_type			= SDL_SwapBE16(src->net_game_type);
    dest->time_limit			= SDL_SwapBE32(src->time_limit);
    dest->kill_limit			= SDL_SwapBE16(src->kill_limit);
    dest->game_options			= SDL_SwapBE16(src->game_options);
    dest->difficulty_level		= SDL_SwapBE16(src->difficulty_level);
    dest->server_is_playing		= src->server_is_playing ? 1 : 0;
    dest->allow_mic			= src->allow_mic ? 1 : 0;
    dest->level_number			= SDL_SwapBE16(src->level_number);
    memcpy(dest->level_name, 		src->level_name, 			sizeof(dest->level_name));
    dest->initial_updates_per_packet	= SDL_SwapBE16(src->initial_updates_per_packet);
    dest->initial_update_latency	= SDL_SwapBE16(src->initial_update_latency);
#endif
}

void
netcpy(game_info* dest, const game_info_NET* src)
{
 	uint8 *S = (uint8 *)src->data;
	StreamToValue(S,dest->initial_random_seed);
	StreamToValue(S,dest->net_game_type);
	StreamToValue(S,dest->time_limit);
	StreamToValue(S,dest->kill_limit);
	StreamToValue(S,dest->game_options);
	StreamToValue(S,dest->difficulty_level);
	dest->server_is_playing = *(S++) != 0;
	dest->allow_mic = *(S++) != 0;
	StreamToValue(S,dest->level_number);
	StreamToBytes(S,dest->level_name,sizeof(dest->level_name));
	StreamToValue(S,dest->initial_updates_per_packet);
	StreamToValue(S,dest->initial_update_latency);
	assert(S == src->data + SIZEOF_game_info);

#ifdef OBSOLETE
    dest->initial_random_seed		= SDL_SwapBE16(src->initial_random_seed);
    dest->net_game_type			= SDL_SwapBE16(src->net_game_type);
    dest->time_limit			= SDL_SwapBE32(src->time_limit);
    dest->kill_limit			= SDL_SwapBE16(src->kill_limit);
    dest->game_options			= SDL_SwapBE16(src->game_options);
    dest->difficulty_level		= SDL_SwapBE16(src->difficulty_level);
    dest->server_is_playing		= (src->server_is_playing != 0);
    dest->allow_mic			= (src->allow_mic != 0);
    dest->level_number			= SDL_SwapBE16(src->level_number);
    memcpy(dest->level_name, 		src->level_name, 			sizeof(dest->level_name));
    dest->initial_updates_per_packet	= SDL_SwapBE16(src->initial_updates_per_packet);
    dest->initial_update_latency	= SDL_SwapBE16(src->initial_update_latency);
#endif
}

#pragma mark -





#pragma mark player_info

void
netcpy(player_info_NET* dest, const player_info* src)
{
	uint8 *S = dest->data;
	BytesToStream(S,src->name,sizeof(src->name));
	ValueToStream(S,src->desired_color);
	ValueToStream(S,src->team);
	ValueToStream(S,src->color);
	BytesToStream(S,src->long_serial_number,sizeof(src->long_serial_number));
	assert(S == dest->data + SIZEOF_player_info);
	
#ifdef OBSOLETE
    memcpy(dest->name,			src->name,			sizeof(dest->name));
    dest->desired_color			= SDL_SwapBE16(src->desired_color);
    dest->team				= SDL_SwapBE16(src->team);
    dest->color				= SDL_SwapBE16(src->color);
    memcpy(dest->long_serial_number,	src->long_serial_number,	sizeof(dest->long_serial_number));
#endif
}

void
netcpy(player_info* dest, const player_info_NET* src)
{
	uint8 *S = (uint8 *)src->data;
	StreamToBytes(S,dest->name,sizeof(dest->name));
	StreamToValue(S,dest->desired_color);
	StreamToValue(S,dest->team);
	StreamToValue(S,dest->color);
	StreamToBytes(S,dest->long_serial_number,sizeof(dest->long_serial_number));
	assert(S == src->data + SIZEOF_player_info);
	
#ifdef OBSOLETE
    memcpy(dest->name,			src->name,			sizeof(dest->name));
    dest->desired_color			= SDL_SwapBE16(src->desired_color);
    dest->team				= SDL_SwapBE16(src->team);
    dest->color				= SDL_SwapBE16(src->color);
    memcpy(dest->long_serial_number,	src->long_serial_number,	sizeof(dest->long_serial_number));
#endif
}

#pragma mark -





#pragma mark NetPacketHeader

void
netcpy(NetPacketHeader_NET* dest, const NetPacketHeader* src)
{
	uint8 *S = dest->data;
	ValueToStream(S,src->tag);
	ValueToStream(S,src->sequence);
	assert(S == dest->data + SIZEOF_NetPacketHeader);

#ifdef OBSOLETE
    dest->tag		= SDL_SwapBE16(src->tag);
    dest->sequence	= SDL_SwapBE32(src->sequence);
#endif
}

void
netcpy(NetPacketHeader* dest, const NetPacketHeader_NET* src)
{
	uint8 *S = (uint8 *)src->data;
	StreamToValue(S,dest->tag);
	StreamToValue(S,dest->sequence);
	assert(S == src->data + SIZEOF_NetPacketHeader);

#ifdef OBSOLETE
    dest->tag		= SDL_SwapBE16(src->tag);
    dest->sequence	= SDL_SwapBE32(src->sequence);
#endif
}

#pragma mark -





#pragma mark NetPacket

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

#ifdef OBSOLETE
    dest->ring_packet_type		= src->ring_packet_type;
    dest->server_player_index		= src->server_player_index;
    dest->server_net_time		= SDL_SwapBE32(src->server_net_time);
    dest->required_action_flags		= SDL_SwapBE16(src->required_action_flags);
    
    for(int i = 0; i < MAXIMUM_NUMBER_OF_NETWORK_PLAYERS; i++)
        dest->action_flag_count[i]	= SDL_SwapBE16(src->action_flag_count[i]);
#endif
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

#ifdef OBSOLETE
    dest->ring_packet_type		= src->ring_packet_type;
    dest->server_player_index		= src->server_player_index;
    dest->server_net_time		= SDL_SwapBE32(src->server_net_time);
    dest->required_action_flags		= SDL_SwapBE16(src->required_action_flags);
    
    for(int i = 0; i < MAXIMUM_NUMBER_OF_NETWORK_PLAYERS; i++)
        dest->action_flag_count[i]	= SDL_SwapBE16(src->action_flag_count[i]);
#endif
}

#pragma mark -





#pragma mark Action Flags

// (if not ALEPHONE_LITTLE_ENDIAN, this is unnecessary as memcpy is used instead.)
#ifdef ALEPHONE_LITTLE_ENDIAN
void
netcpy(uint32* dest, const uint32* src, size_t length) {
    assert(length % sizeof(uint32) == 0);
    
    int	num_items = length / sizeof(uint32);
	
	uint8 *S = (uint8 *)dest;
	ListToStream(S,src,num_items);
	assert(S == dest + length);
	
#ifdef OBSOLETE
    for(int i = 0; i < num_items; i++) {
        dest[i] = SDL_SwapBE32(src[i]);
#endif
    }
}
#endif

#pragma mark -





#pragma mark NetDistributionPacket

void
netcpy(NetDistributionPacket_NET* dest, const NetDistributionPacket* src)
{
	uint8 *S = dest->data;
	ValueToStream(S,src->distribution_type);
	ValueToStream(S,src->first_player_index);
	ValueToStream(S,src->data_size);
	assert(S == dest->data + SIZEOF_NetDistributionPacket);

#ifdef OBSOLETE
    dest->distribution_type	= SDL_SwapBE16(src->distribution_type);
    dest->first_player_index	= SDL_SwapBE16(src->first_player_index);
    dest->data_size		= SDL_SwapBE16(src->data_size);
#endif
}

void
netcpy(NetDistributionPacket* dest, const NetDistributionPacket_NET* src)
{
	uint8 *S = (uint8 *)src->data;
	StreamToValue(S,dest->distribution_type);
	StreamToValue(S,dest->first_player_index);
	StreamToValue(S,dest->data_size);
	assert(S == src->data + SIZEOF_NetDistributionPacket);

#ifdef OBSOLETE
    dest->distribution_type	= SDL_SwapBE16(src->distribution_type);
    dest->first_player_index	= SDL_SwapBE16(src->first_player_index);
    dest->data_size		= SDL_SwapBE16(src->data_size);
#endif
}

#pragma mark -





#pragma mark IPaddress

// IP addresses are always in network byte order - do not swap
void
netcpy(IPaddress_NET* dest, const IPaddress* src)
{
	uint8 *S = dest->data;
	memcpy(S,&src->host,4);	// 32-bit integer
	S += 4;
	memcpy(S,&src->port,2);	// 16-bit integer

#ifdef OBSOLETE
    dest->host	= src->host;
    dest->port	= src->port;
#endif
}

void
netcpy(IPaddress* dest, const IPaddress_NET* src) {
	uint8 *S = (uint8 *)src->data;
	memcpy(&dest->host,S,4);	// 32-bit integer
	S += 4;
	memcpy(&dest->port,S,2);	// 16-bit integer

#ifdef OBSOLETE
    dest->host	= src->host;
    dest->port	= src->port;
#endif
}

#pragma mark -





#pragma mark NetPlayer

void
netcpy(NetPlayer_NET* dest, const NetPlayer* src)
{
	uint8 *S = dest->data;
	IPaddress_NET TempIPAddr;
	netcpy(&TempIPAddr,&src->dspAddress);
	BytesToStream(S,&TempIPAddr.data,SIZEOF_IPaddress);
	netcpy(&TempIPAddr,&src->ddpAddress);
	BytesToStream(S,&TempIPAddr.data,SIZEOF_IPaddress);
	ValueToStream(S,src->identifier);
	*(S++) = src->net_dead ? 1 : 0;
	BytesToStream(S,src->player_data,MAXIMUM_PLAYER_DATA_SIZE);
	assert(S == dest->data + SIZEOF_NetPlayer);

#ifdef OBSOLETE
    netcpy(&dest->dspAddress,	&src->dspAddress);
    netcpy(&dest->ddpAddress,	&src->ddpAddress);
    dest->identifier		= SDL_SwapBE16(src->identifier);
    dest->net_dead		= src->net_dead ? 1 : 0;
    netcpy(&dest->player_data,	(player_info*)&src->player_data);
#endif
}

void
netcpy(NetPlayer* dest, const NetPlayer_NET* src)
{
	uint8 *S = (uint8 *)src->data;
	IPaddress_NET TempIPAddr;
	StreamToBytes(S,&TempIPAddr.data,SIZEOF_IPaddress);
	netcpy(&dest->dspAddress,&TempIPAddr);
	StreamToBytes(S,&TempIPAddr.data,SIZEOF_IPaddress);
	netcpy(&dest->ddpAddress,&TempIPAddr);
	StreamToValue(S,dest->identifier);
	dest->net_dead = *(S++) != 0;
	StreamToBytes(S,dest->player_data,MAXIMUM_PLAYER_DATA_SIZE);
	assert(S == src->data + SIZEOF_NetPlayer);

#ifdef OBSOLETE
    netcpy(&dest->dspAddress,			&src->dspAddress);
    netcpy(&dest->ddpAddress,			&src->ddpAddress);
    dest->identifier				= SDL_SwapBE16(src->identifier);
    dest->net_dead				= (src->net_dead != 0);
    netcpy((player_info*)&dest->player_data,	&src->player_data);
#endif
}

#pragma mark -





#pragma mark NetTopology

void
netcpy(NetTopology_NET* dest, const NetTopology* src)
{
	uint8 *S = dest->data;
	ValueToStream(S,src->tag);
	ValueToStream(S,src->player_count);
	ValueToStream(S,src->nextIdentifier);
	BytesToStream(S,src->game_data,MAXIMUM_GAME_DATA_SIZE);
	for (int i=0; i<MAXIMUM_NUMBER_OF_NETWORK_PLAYERS; i++)
	{
		NetPlayer_NET TempPlyrData;
		netcpy(&TempPlyrData,&src->players[i]);
		BytesToStream(S,TempPlyrData.data,SIZEOF_NetPlayer);
	}
	assert(S == dest->data + SIZEOF_NetTopology);

#ifdef OBSOLETE
    dest->tag			= SDL_SwapBE16(src->tag);
    dest->player_count		= SDL_SwapBE16(src->player_count);
    dest->nextIdentifier	= SDL_SwapBE16(src->nextIdentifier);
    netcpy(&dest->game_data,	(game_info*)&src->game_data);

    for(int i = 0; i < MAXIMUM_NUMBER_OF_NETWORK_PLAYERS; i++)
        netcpy(&dest->players[i],	&src->players[i]);
#endif
}

void
netcpy(NetTopology* dest, const NetTopology_NET* src)
{
	uint8 *S = (uint8 *)src->data;
	StreamToValue(S,dest->tag);
	StreamToValue(S,dest->player_count);
	StreamToValue(S,dest->nextIdentifier);
	StreamToBytes(S,dest->game_data,MAXIMUM_GAME_DATA_SIZE);
	for (int i=0; i<MAXIMUM_NUMBER_OF_NETWORK_PLAYERS; i++)
	{
		NetPlayer_NET TempPlyrData;
		StreamToBytes(S,TempPlyrData.data,SIZEOF_NetPlayer);
		netcpy(&dest->players[i],&TempPlyrData);
	}
	assert(S == src->data + SIZEOF_NetTopology);

#ifdef OBSOLETE
    dest->tag					= SDL_SwapBE16(src->tag);
    dest->player_count				= SDL_SwapBE16(src->player_count);
    dest->nextIdentifier			= SDL_SwapBE16(src->nextIdentifier);
    netcpy((game_info*)&dest->game_data,	&src->game_data);

    for(int i = 0; i < MAXIMUM_NUMBER_OF_NETWORK_PLAYERS; i++)
        netcpy(&dest->players[i],	&src->players[i]);
#endif
}

#pragma mark -





#pragma mark gather_player_data

void
netcpy(gather_player_data_NET* dest, const gather_player_data* src)
{
	uint8 *S = dest->data;
	ValueToStream(S,src->new_local_player_identifier);
	assert(S == dest->data + SIZEOF_gather_player_data);
	
#ifdef OBSOLETE
    dest->new_local_player_identifier	= SDL_SwapBE16(src->new_local_player_identifier);
#endif
}

void
netcpy(gather_player_data* dest, const gather_player_data_NET* src)
{
	uint8 *S = (uint8 *)src->data;
	StreamToValue(S,dest->new_local_player_identifier);
	assert(S == src->data + SIZEOF_gather_player_data);
	
#ifdef OBSOLETE
    dest->new_local_player_identifier	= SDL_SwapBE16(src->new_local_player_identifier);
#endif
}

#pragma mark -





#pragma mark accept_gather_data

void
netcpy(accept_gather_data_NET* dest, const accept_gather_data* src)
{
	uint8 *S = dest->data;
	*(S++) = src->accepted ? 1 : 0;
	NetPlayer_NET TempPlyrData;
	netcpy(&TempPlyrData,&src->player);
	BytesToStream(S,TempPlyrData.data,SIZEOF_NetPlayer);
	assert(S == dest->data + SIZEOF_accept_gather_data);
	
#ifdef OBSOLETE
    dest->accepted		= src->accepted ? 1 : 0;
    netcpy(&dest->player,	&src->player);
#endif
}

void
netcpy(accept_gather_data* dest, const accept_gather_data_NET* src)
{
	uint8 *S = (uint8 *)src->data;
	dest->accepted = *(S++) != 0;
	NetPlayer_NET TempPlyrData;
	StreamToBytes(S,TempPlyrData.data,SIZEOF_NetPlayer);
	netcpy(&dest->player,&TempPlyrData);
	assert(S == src->data + SIZEOF_accept_gather_data);

#ifdef OBSOLETE
    dest->accepted		= (src->accepted != 0);
    netcpy(&dest->player,	&src->player);
#endif
}

#pragma mark -





#ifdef NETWORK_CHAT
#pragma mark NetChatMessage

void
netcpy(NetChatMessage_NET* dest, const NetChatMessage* src)
{
	uint8 *S = dest->data;
	ValueToStream(S,src->sender_identifier);
	BytesToStream(S,src->text,CHAT_MESSAGE_TEXT_BUFFER_SIZE);
	assert(S == dest->data + SIZEOF_NetChatMessage);

#ifdef OBSOLETE
    dest->sender_identifier	= SDL_SwapBE16(src->sender_identifier);
    strncpy(dest->text,		src->text,				CHAT_MESSAGE_TEXT_BUFFER_SIZE);
#endif
}

void
netcpy(NetChatMessage* dest, const NetChatMessage_NET* src)
{
	uint8 *S = src->data;
	StreamToValue(S,dest->sender_identifier);
	StreamToBytes(S,dest->text,CHAT_MESSAGE_TEXT_BUFFER_SIZE);
	assert(S == src->data + SIZEOF_NetChatMessage);

    dest->sender_identifier	= SDL_SwapBE16(src->sender_identifier);
    strncpy(dest->text,		src->text,				CHAT_MESSAGE_TEXT_BUFFER_SIZE);
}

#endif // NETWORK_CHAT
