#ifndef __NETWORK_GAMES_H
#define __NETWORK_GAMES_H

/*
	network_games.h

	Copyright (C) 1991-2001 and beyond by Bungie Studios, Inc.
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

	Wednesday, July 19, 1995 11:03:09 AM- rdm created.

*/

#include "player.h"

struct player_ranking_data {
	short player_index;
	long ranking;
};

extern int32 team_netgame_parameters[NUMBER_OF_TEAM_COLORS][2];

void initialize_net_game(void);

/* returns true if the game is over.. */
bool update_net_game(void);

/* Returns the player net ranking, which may mean different things */
long get_player_net_ranking(short player_index, short *kills, short *deaths,
	bool game_is_over);
long get_team_net_ranking(short team, short *kills, short *deaths,
			  bool game_is_over);

void calculate_player_rankings(struct player_ranking_data *rankings);
void calculate_ranking_text(char *buffer, long ranking);
bool current_net_game_has_scores(void);
void calculate_ranking_text_for_post_game(char *buffer, long ranking);
bool get_network_score_text_for_postgame(char *buffer, bool team_mode);
bool current_game_has_balls(void);
void get_network_joined_message(char *buffer, short game_type);
long get_entry_point_flags_for_game_type(size_t game_type);

bool player_killed_player(short dead_player_index, short aggressor_player_index);

bool game_is_over(void);

enum
{
	_network_compass_all_off= 0,
	
	_network_compass_nw= 0x0001,
	_network_compass_ne= 0x0002,
	_network_compass_sw= 0x0004,
	_network_compass_se= 0x0008,
	
	_network_compass_all_on= 0x000f,
	
	_network_compass_use_beacon = 0x0010
};

short get_network_compass_state(short player_index);

#endif
