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

	This license is contained in the file "GNU_GeneralPublicLicense.txt",
	which is included with this source code; it is available online at
	http://www.gnu.org/licenses/gpl.html

 *  Created by Woody Zenfell, III on Thu Oct 11 2001.
 */

#include	"network_data_formats.h"

#pragma mark game_info

void
netcpy(game_info_NET* dest, const game_info* src) {
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
}

void
netcpy(game_info* dest, const game_info_NET* src) {
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
}

#pragma mark -





#pragma mark player_info

void
netcpy(player_info_NET* dest, const player_info* src) {
    memcpy(dest->name,			src->name,			sizeof(dest->name));
    dest->desired_color			= SDL_SwapBE16(src->desired_color);
    dest->team				= SDL_SwapBE16(src->team);
    dest->color				= SDL_SwapBE16(src->color);
    memcpy(dest->long_serial_number,	src->long_serial_number,	sizeof(dest->long_serial_number));
}

void
netcpy(player_info* dest, const player_info_NET* src) {
    memcpy(dest->name,			src->name,			sizeof(dest->name));
    dest->desired_color			= SDL_SwapBE16(src->desired_color);
    dest->team				= SDL_SwapBE16(src->team);
    dest->color				= SDL_SwapBE16(src->color);
    memcpy(dest->long_serial_number,	src->long_serial_number,	sizeof(dest->long_serial_number));
}

#pragma mark -





#pragma mark NetPacketHeader

void
netcpy(NetPacketHeader_NET* dest, const NetPacketHeader* src) {
    dest->tag		= SDL_SwapBE16(src->tag);
    dest->sequence	= SDL_SwapBE32(src->sequence);
}

void
netcpy(NetPacketHeader* dest, const NetPacketHeader_NET* src) {
    dest->tag		= SDL_SwapBE16(src->tag);
    dest->sequence	= SDL_SwapBE32(src->sequence);
}

#pragma mark -





#pragma mark NetPacket

void
netcpy(NetPacket_NET* dest, const NetPacket* src) {
    dest->ring_packet_type		= src->ring_packet_type;
    dest->server_player_index		= src->server_player_index;
    dest->server_net_time		= SDL_SwapBE32(src->server_net_time);
    dest->required_action_flags		= SDL_SwapBE16(src->required_action_flags);
    
    for(int i = 0; i < MAXIMUM_NUMBER_OF_NETWORK_PLAYERS; i++)
        dest->action_flag_count[i]	= SDL_SwapBE16(src->action_flag_count[i]);
}

void
netcpy(NetPacket* dest, const NetPacket_NET* src) {
    dest->ring_packet_type		= src->ring_packet_type;
    dest->server_player_index		= src->server_player_index;
    dest->server_net_time		= SDL_SwapBE32(src->server_net_time);
    dest->required_action_flags		= SDL_SwapBE16(src->required_action_flags);
    
    for(int i = 0; i < MAXIMUM_NUMBER_OF_NETWORK_PLAYERS; i++)
        dest->action_flag_count[i]	= SDL_SwapBE16(src->action_flag_count[i]);
}

#pragma mark -





#pragma mark Action Flags

// (if not ALEPHONE_LITTLE_ENDIAN, this is unnecessary as memcpy is used instead.)
#ifdef ALEPHONE_LITTLE_ENDIAN
void
netcpy(uint32* dest, const uint32* src, size_t length) {
    assert(length % sizeof(uint32) == 0);
    
    int	num_items = length / sizeof(uint32);
    
    for(int i = 0; i < num_items; i++) {
        dest[i] = SDL_SwapBE32(src[i]);
    }
}
#endif

#pragma mark -





#pragma mark NetDistributionPacket

void
netcpy(NetDistributionPacket_NET* dest, const NetDistributionPacket* src) {
    dest->distribution_type	= SDL_SwapBE16(src->distribution_type);
    dest->first_player_index	= SDL_SwapBE16(src->first_player_index);
    dest->data_size		= SDL_SwapBE16(src->data_size);
}

void
netcpy(NetDistributionPacket* dest, const NetDistributionPacket_NET* src) {
    dest->distribution_type	= SDL_SwapBE16(src->distribution_type);
    dest->first_player_index	= SDL_SwapBE16(src->first_player_index);
    dest->data_size		= SDL_SwapBE16(src->data_size);
}

#pragma mark -





#pragma mark IPaddress

// IP addresses are always in network byte order - do not swap
void
netcpy(IPaddress_NET* dest, const IPaddress* src) {
    dest->host	= src->host;
    dest->port	= src->port;
}

void
netcpy(IPaddress* dest, const IPaddress_NET* src) {
    dest->host	= src->host;
    dest->port	= src->port;
}

#pragma mark -





#pragma mark NetPlayer

void
netcpy(NetPlayer_NET* dest, const NetPlayer* src) {
    netcpy(&dest->dspAddress,	&src->dspAddress);
    netcpy(&dest->ddpAddress,	&src->ddpAddress);
    dest->identifier		= SDL_SwapBE16(src->identifier);
    dest->net_dead		= src->net_dead ? 1 : 0;
    netcpy(&dest->player_data,	(player_info*)&src->player_data);
}

void
netcpy(NetPlayer* dest, const NetPlayer_NET* src) {
    netcpy(&dest->dspAddress,			&src->dspAddress);
    netcpy(&dest->ddpAddress,			&src->ddpAddress);
    dest->identifier				= SDL_SwapBE16(src->identifier);
    dest->net_dead				= (src->net_dead != 0);
    netcpy((player_info*)&dest->player_data,	&src->player_data);
}

#pragma mark -





#pragma mark NetTopology

void
netcpy(NetTopology_NET* dest, const NetTopology* src) {
    dest->tag			= SDL_SwapBE16(src->tag);
    dest->player_count		= SDL_SwapBE16(src->player_count);
    dest->nextIdentifier	= SDL_SwapBE16(src->nextIdentifier);
    netcpy(&dest->game_data,	(game_info*)&src->game_data);

    for(int i = 0; i < MAXIMUM_NUMBER_OF_NETWORK_PLAYERS; i++)
        netcpy(&dest->players[i],	&src->players[i]);
}

void
netcpy(NetTopology* dest, const NetTopology_NET* src) {
    dest->tag					= SDL_SwapBE16(src->tag);
    dest->player_count				= SDL_SwapBE16(src->player_count);
    dest->nextIdentifier			= SDL_SwapBE16(src->nextIdentifier);
    netcpy((game_info*)&dest->game_data,	&src->game_data);

    for(int i = 0; i < MAXIMUM_NUMBER_OF_NETWORK_PLAYERS; i++)
        netcpy(&dest->players[i],	&src->players[i]);
}

#pragma mark -





#pragma mark gather_player_data

void
netcpy(gather_player_data_NET* dest, const gather_player_data* src) {
    dest->new_local_player_identifier	= SDL_SwapBE16(src->new_local_player_identifier);
}

void
netcpy(gather_player_data* dest, const gather_player_data_NET* src) {
    dest->new_local_player_identifier	= SDL_SwapBE16(src->new_local_player_identifier);
}

#pragma mark -





#pragma mark accept_gather_data

void
netcpy(accept_gather_data_NET* dest, const accept_gather_data* src) {
    dest->accepted		= src->accepted ? 1 : 0;
    netcpy(&dest->player,	&src->player);
}

void
netcpy(accept_gather_data* dest, const accept_gather_data_NET* src) {
    dest->accepted		= (src->accepted != 0);
    netcpy(&dest->player,	&src->player);
}

#pragma mark -





#ifdef NETWORK_CHAT
#pragma mark NetChatMessage

void
netcpy(NetChatMessage_NET* dest, const NetChatMessage* src) {
    dest->sender_identifier	= SDL_SwapBE16(src->sender_identifier);
    strncpy(dest->text,		src->text,				CHAT_MESSAGE_TEXT_BUFFER_SIZE);
}

void
netcpy(NetChatMessage* dest, const NetChatMessage_NET* src) {
    dest->sender_identifier	= SDL_SwapBE16(src->sender_identifier);
    strncpy(dest->text,		src->text,				CHAT_MESSAGE_TEXT_BUFFER_SIZE);
}

#endif // NETWORK_CHAT
