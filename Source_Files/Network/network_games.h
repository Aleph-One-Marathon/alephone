#ifndef __NETWORK_GAMES_H
#define __NETWORK_GAMES_H

/*

	network_games.h
	Wednesday, July 19, 1995 11:03:09 AM- rdm created.

*/

struct player_ranking_data {
	short player_index;
	long ranking;
};

void initialize_net_game(void);

/* returns TRUE if the game is over.. */
boolean update_net_game(void);

/* Returns the player net ranking, which may mean different things */
long get_player_net_ranking(short player_index, short *kills, short *deaths,
	boolean game_is_over);

void calculate_player_rankings(struct player_ranking_data *rankings);
void calculate_ranking_text(char *buffer, long ranking);
boolean current_net_game_has_scores(void);
void calculate_ranking_text_for_post_game(char *buffer, long ranking);
boolean get_network_score_text_for_postgame(char *buffer, boolean team_mode);
boolean current_game_has_balls(void);
void get_network_joined_message(char *buffer, short game_type);
long get_entry_point_flags_for_game_type(short game_type);

boolean player_killed_player(short dead_player_index, short aggressor_player_index);

boolean game_is_over(void);

enum
{
	_network_compass_all_off= 0,
	
	_network_compass_nw= 0x0001,
	_network_compass_ne= 0x0002,
	_network_compass_sw= 0x0004,
	_network_compass_se= 0x0008,
	
	_network_compass_all_on= 0x000f
};

short get_network_compass_state(short player_index);

#endif
