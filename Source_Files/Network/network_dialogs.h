/*
 *  network_dialogs.h
 *

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

 *
 *  Sept 19, 2001 (Woody Zenfell): split this file away from network_dialogs.cpp for sharing
 *	Also made whatever simple changes were needed for it to compile/work.
 *
 *  Sept-Nov 2001 (Woody Zenfell): added some identifiers and prototypes for carnage report
 *	and for better code sharing between the Mac and SDL versions.

Feb 27, 2002 (Br'fin (Jeremy Parsons)):
	Moved shared SDL hint address info here from network_dialogs_sdl.cpp
	Added dialog item definitions for a Join by Host in the join dialog

Mar 1, 2002 (Woody Zenfell):
    SDL dialog uses new level-selection scheme; new interface based on level number, not menu index.
 */

#ifndef NETWORK_DIALOGS_H
#define	NETWORK_DIALOGS_H

#include    "player.h"  // for MAXIMUM_NUMBER_OF_PLAYERS

// ZZZ: Moved here so constants can be shared by Mac and SDL dialog code.
/* ------------------ enums */
enum {	
	strNET_STATS_STRINGS= 153,
	strKILLS_STRING= 0,
	strDEATHS_STRING,
	strSUICIDES_STRING,
	strTOTALS_STRING,
	strMONSTERS_STRING,
	strTOTAL_KILLS_STRING,
	strTOTAL_DEATHS_STRING,
	strINCLUDING_SUICIDES_STRING,
	strTEAM_TOTALS_STRING,
	strFRIENDLY_FIRE_STRING,
	strTOTAL_SCORES,
	strTOTAL_TEAM_SCORES,
// ZZZ: added the following to support my postgame report
    strTEAM_CARNAGE_STRING,
    strKILLS_LEGEND,
    strDEATHS_LEGEND,
    strSUICIDES_LEGEND,
    strFRIENDLY_FIRE_LEGEND
};

enum {
	dlogNET_GAME_STATS= 5000,
	iGRAPH_POPUP= 2,
	iDAMAGE_STATS,
	iTOTAL_KILLS,
	iTOTAL_DEATHS
};

enum /* All the different graph types */
{
	_player_graph,
	_total_carnage_graph,
	_total_scores_graph,
	_total_team_carnage_graph,
	_total_team_scores_graph
};

enum {
	_suicide_color,
	_kill_color,
	_death_color,
	_score_color,
	NUMBER_OF_NET_COLORS
};

#if HAVE_SDL_NET
/* SDL/TCP hinting info. JTP: moved here from network_dialogs_sdl.cpp */
enum {
    kJoinHintingAddressLength = 64
};

extern bool sUserWantsJoinHinting;
extern char sJoinHintingAddress[];
#endif

#define strJOIN_DIALOG_MESSAGES 136
enum /* join dialog string numbers */
{
	_join_dialog_welcome_string,
	_join_dialog_waiting_string,
	_join_dialog_accepted_string
};

enum {
	strSETUP_NET_GAME_MESSAGES= 141,
	killLimitString= 0,
	killsString,
	flagPullsString,
	flagsString,
	pointLimitString,
	pointsString,
	// START Benad
	timeOnBaseString,
	minutesString
	// END Benad
};

enum {
	dlogGATHER= 10000,
	iPLAYER_DISPLAY_AREA= 3,
	iADD,
	iNETWORK_LIST_BOX,
	iZONES_MENU= 8,
	iPLAYER_LIST_TEXT
};

enum {
	dlogJOIN= 10001,
	iJOIN= 1,
	/* iPLAYER_DISPLAY_AREA        3 */
	iJOIN_NAME= 4,
	iJOIN_TEAM,
	iJOIN_COLOR, 
	iJOIN_MESSAGES,
	// Group line = 12
	iJOIN_NETWORK_TYPE= 13,
	iJOIN_BY_HOST = 14,
	iJOIN_BY_HOST_LABEL,
	iJOIN_BY_HOST_ADDRESS
};

enum {
	dlogGAME_SETUP= 3000,
	iNETWORK_SPEED= 3,
	iENTRY_MENU,
	iDIFFICULTY_MENU,
	iMOTION_SENSOR_DISABLED,
	iDYING_PUNISHED,
	iBURN_ITEMS_ON_DEATH,
	iREAL_TIME_SOUND,
	iUNLIMITED_MONSTERS,
	iFORCE_UNIQUE_TEAMS,
	iRADIO_NO_TIME_LIMIT,
	iRADIO_TIME_LIMIT,
	iRADIO_KILL_LIMIT,
	iTIME_LIMIT,
	iKILL_LIMIT,
	iREALTIME_NET_STATS,
	iTEXT_KILL_LIMIT,
	iGATHER_NAME= 21,
	iGATHER_TEAM,
	iSUICIDE_PUNISHED= 24,
	iGAME_TYPE,
	iGATHER_COLOR,
	iTEXT_TIME_LIMIT= 35
};



/* ------------------ structures */
struct net_rank
{
	short kills, deaths;
	long ranking;
	long game_ranking;
	
	short player_index;
	short color; // only valid if player_index== NONE!
	short friendly_fire_kills;
};

struct player_info;
struct game_info;


/* ---------------------- globals */
extern struct net_rank rankings[MAXIMUM_NUMBER_OF_PLAYERS];



/* ---------------------- prototypes */
// And now, some shared routines.
extern short
fill_in_game_setup_dialog(
	DialogPtr dialog, 
	player_info *player_information,
	bool allow_all_levels,
        bool resuming_game);

extern void
extract_setup_dialog_information(
	DialogPtr dialog,
	player_info *player_information,
	game_info *game_information,
	short game_limit_type,
	bool allow_all_levels,
        bool resuming_game);

extern void reassign_player_colors(short player_index, short num_players);

extern void setup_for_untimed_game(
	DialogPtr dialog);

extern void setup_for_timed_game(
	DialogPtr dialog);

// ZZZ: new function to parallel the previous two.
extern void setup_for_score_limited_game(
	DialogPtr dialog);

extern void setup_dialog_for_game_type(
	DialogPtr dialog, 
	short game_type);



// (Postgame Carnage Report routines)
extern short find_graph_mode(DialogPtr dialog, short *index);
extern void draw_new_graph(DialogPtr dialog);

extern void draw_player_graph(DialogPtr dialog, short index);
extern void get_net_color(short index, RGBColor *color);

extern short calculate_max_kills(size_t num_players);
extern void draw_totals_graph(DialogPtr dialog);
extern void calculate_rankings(struct net_rank *ranks, short num_players);
extern int rank_compare(void const *rank1, void const *rank2);
extern int team_rank_compare(void const *rank1, void const *ranks2);
extern int score_rank_compare(void const *rank1, void const *ranks2);
extern void draw_team_totals_graph(DialogPtr dialog);
extern void draw_total_scores_graph(DialogPtr dialog);
extern void draw_team_total_scores_graph(DialogPtr dialog);
extern void update_carnage_summary(DialogPtr dialog, struct net_rank *ranks, short num_players,
                                   short suicide_index, bool do_totals, bool friendly_fire);


#ifdef mac
// Mac-only routines called by shared routines.
extern void fill_in_entry_points(DialogPtr dialog, short item, long entry_flags, short default_level);

#else//!mac
// SDL-only routines called by shared routines.
extern void get_selected_entry_point(dialog* inDialog, short inItem, entry_point* outEntryPoint);

#endif//!mac

// Routines with different implementations on different platforms, which are called by
// shared routines.
extern void menu_index_to_level_entry(short index, long entry_flags, struct entry_point *entry);
extern void select_entry_point(DialogPtr inDialog, short inItem, int16 inLevelNumber);

// ZZZ: new function manipulates radio buttons on Mac; changes w_select widget on SDL.
extern void set_limit_type(DialogPtr dialog, short limit_type);
extern void modify_limit_type_choice_enabled(DialogPtr dialog, short inChangeEnable);

// ZZZ: new function manipulates radio button title and units ("Point Limit", "points")
extern void set_limit_text(DialogPtr dialog, short radio_item, short radio_stringset_id, short radio_string_index,
                                short units_item, short units_stringset_id, short units_string_index);

// (Postgame carnage report)
extern void draw_names(DialogPtr dialog, struct net_rank *ranks, short number_of_bars,
	short which_player);

extern void draw_kill_bars(DialogPtr dialog, struct net_rank *ranks, short num_players, 
	short suicide_index, bool do_totals, bool friendly_fire);

extern void draw_score_bars(DialogPtr dialog, struct net_rank *ranks, short bar_count);

#endif//NETWORK_DIALOGS_H
