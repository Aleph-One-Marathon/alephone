/*  network_dialogs_sdl.cpp

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

*/

#if !defined(DISABLE_NETWORKING)

/*
 *  network_dialogs_sdl.cpp - Network game dialogs, SDL implementation
 *
 *  Written in 2000 by Christian Bauer
 *
 *  Nearly complete rewrite in Sept-Nov 2001 by Woody Zenfell, III
 *	(Many comments are mine despite lack of ZZZ.)

Feb 27, 2002 (Br'fin (Jeremy Parsons)):
	Moved shared SDL hint address info to network_dialogs.cpp/.h

Mar 1, 2002 (Woody Zenfell):  Setup Network Game changes.
    Reworked selection of level to pop up a box (w_select_button rather than w_select).
    Added support for changing map file in SNG; no trip to Environment Preferences needed!
    Pruned out some stale code.

Mar 8, 2002 (Woody Zenfell):
    Realtime microphone can be enabled now in setup network game.
    Added UI for testing out a microphone implementation on the local machine.
    
Feb 5, 2003 (Woody Zenfell):
    Dialog should now display found/lost players, gathered players, chat messages, etc.
    without the user generating events.

Apr 10, 2003 (Woody Zenfell):
    Join hinting and autogathering have Preferences entries now

August 27, 2003 (Woody Zenfell):
	SDL UI for selecting netscript

September 17, 2004 (jkvw):
	Changes to accomodate NAT-friendly networking
*/

#include "cseries.h"
#include "sdl_network.h"
#include "sdl_dialogs.h"
#include "sdl_fonts.h"
#include "sdl_widgets.h"
#include	"network_dialog_widgets_sdl.h"
#include    "preferences_widgets_sdl.h"
#include	"network_lookup_sdl.h"

#include "shell.h"
#include "map.h"
#include "player.h"
#include "preferences.h"
#include "PlayerName.h"
#include	"progress.h"
#include    "screen.h"  // clear_screen()
#include    "mysound.h" // resolution of DIALOG_CLICK_SOUND
#include    "wad.h"     // read_wad_file_checksum()
#include	"snprintf.h"

// String-Set Functions (for getting strings from MML rather than compiled-in)
#include	"TextStrings.h"

// Shared dialog item ID constants
#include	"network_dialogs.h"

// get_entry_point_flags_for_game_type
#include	"network_games.h"

#include	"metaserver_dialogs.h"

// LAN game-location services
#include	"network_private.h"

#include	<memory>




// Get player name from outside
// ZZZ random note: I didn't do this part, and I'm not sure it's right.  At least, the
// documentation seems a bit inconsistent.  The MML docs say that it determines the
// default player name in multiplayer.  This is true, but more importantly, it determines
// the service type advertised/sought when trying to get together a game.  I guess this
// could be advantageous, in case MML is used to change the way a game works, in which case
// you wouldn't want folks with their MML set up in different ways trying to play together
// (instant sync problems - just add water.)
#define PLAYER_TYPE GetPlayerName()

// ZZZ: some features that may or may not be there - these are used to control what UI gets drawn.
// (as of my initial submission, only pregame gatherer-to-joiner messaging works.)
// Eventually, someone will make network microphone for SDL, at which point it should be extended
// to work in pre/postgame also, so the text chat becomes less important.
// NOTE - if you enable the POSTGAME_CHAT UI, you will need to edit kPostgameTopMargin in
// w_players_in_game2's implementation to reduce the dialog height it uses.  (network_dialog_widgets_sdl.cpp)
#define	NETWORK_PREGAME_CHAT	// Gatherer can message joiners at pregame
#undef	NETWORK_POSTGAME_CHAT	// Gatherer can message joiners at postgame
#undef	NETWORK_TWO_WAY_CHAT	// Joiners can message whenever the gatherer can

#ifdef	DEBUG
//#define	NETWORK_TEST_POSTGAME_DIALOG	// For testing without having to play a netgame

// Note!  If you use NETWORK_TEST_MICROPHONE_LOCALLY you'll also want to set the microphone
// implementation to local loopback, else who knows what will happen...
//#define NETWORK_TEST_MICROPHONE_LOCALLY // can use this OR test postgame dialog, not both at once.
#endif

#ifdef NETWORK_TEST_MICROPHONE_LOCALLY
#include    "network_speaker_sdl.h"
#include	"network_sound.h"
#endif




// ZZZ: graph types are a dynamically-generated StringSet (not loaded from MML)
enum {
    kGraphTypesStringSetID	= 3180
};

// limit types, 0-based, for w_select-compatible use.  (see also set_limit_type())
enum {    
    kNoLimit			= 0,
    kTimeLimit			= 0x01,
    kScoreLimit			= 0x02,
    kScoreAndTimeLimits		= kScoreLimit | kTimeLimit // currently cannot be selected directly
};

// Some identifiers used only locally.  Hope the numeric equivalents don't conflict!
// (they shouldn't.)
enum {
        iDONT_DO_THIS_USE_SHARED_SYMBOLS= 4242,	// Score limit?  Time limit?  No limit?
        iCHAT_HISTORY,				// Where chat text appears
        iCHAT_ENTRY,				// Where chat text is entered
                
};


static bool sAdvertiseGameOnMetaserver = false;






/*
 *  Network game statistics dialog
 */


// ZZZ: this is based on the eponymous function on the Mac side
static short create_graph_popup_menu(w_select* theMenu)
{
	short index;
	bool has_scores;

        // Clear the graph types stringset
        TS_DeleteStringSet(kGraphTypesStringSetID);

	/* Setup the player names */
	for (index= 0; index<dynamic_world->player_count; index++)
	{
		struct player_data *player= get_player_data(rankings[index].player_index);

                TS_PutCString(kGraphTypesStringSetID, index, player->name);
	}
	
	/* Add in the total carnage.. */
        getcstr(temporary, strNET_STATS_STRINGS, strTOTALS_STRING);
        TS_PutCString(kGraphTypesStringSetID, index, temporary);
        index++;
	
	/* Add in the scores */
	has_scores= get_network_score_text_for_postgame(temporary, false);
	if(has_scores)
	{
                TS_PutCString(kGraphTypesStringSetID, index, temporary);
                index++;
	}
	
	/* If the game has teams, show the team stats. */
	if (!(dynamic_world->game_information.game_options & _force_unique_teams)) 
	{
                getcstr(temporary, strNET_STATS_STRINGS, strTEAM_TOTALS_STRING);
                TS_PutCString(kGraphTypesStringSetID, index, temporary);
                index++;

		if(has_scores)
		{
			get_network_score_text_for_postgame(temporary, true);
                        TS_PutCString(kGraphTypesStringSetID, index, temporary);
                        index++;
		}
	} 

        // Place the newly-constructed StringSet into the graph selection widget.
        theMenu->set_labels_stringset(kGraphTypesStringSetID);

        // Change of behavior here: instead of choosing individual scores, or failing that, Total Carnage,
        // I select team scores, then failing that either team carnage or individual scores, then failing that,
        // Total Carnage.  I think this better reflects what really happens - in a team game, it's *teams*
        // that win things.  You can view the individual results, sure, but the first thing that pops up
        // (who won??) is a team stat.
        theMenu->set_selection(index - 1);        
        
	return index;
}

void
draw_names(DialogPtr &dialog, struct net_rank *ranks, short number_of_bars, short which_player) {
    // This does nothing here - draw_kill_bars or draw_score_bars is assumed to have enough data to work with,
    // and one of those is always called adjacent to a call to draw_names in practice.
}

void
draw_kill_bars(DialogPtr &dialog, struct net_rank *ranks, short num_players, 
               short suicide_index, bool do_totals, bool friendly_fire)
{
    // We don't actually draw here - we just pass the data along to the widget, and it will take care of the rest. 
    w_players_in_game2* wpig2 = dynamic_cast<w_players_in_game2*>(dialog->get_widget_by_id(iDAMAGE_STATS));
    wpig2->set_graph_data(ranks, num_players, suicide_index, (ranks[0].player_index == NONE) ? true : false, false);

    update_carnage_summary(dialog, ranks, num_players, suicide_index, do_totals, friendly_fire);
}

void
draw_score_bars(DialogPtr &dialog, struct net_rank *ranks, short bar_count) {
    // We don't actually draw here - we just pass the data along to the widget, and it will take care of the rest. 
    w_players_in_game2* wpig2 = dynamic_cast<w_players_in_game2*>(dialog->get_widget_by_id(iDAMAGE_STATS));
    wpig2->set_graph_data(ranks, bar_count, NONE, (ranks[0].player_index == NONE) ? true : false, true);

    // clear the summary text
    unsigned char theEmptyString = '\0';
    copy_pstring_to_static_text(dialog, iTOTAL_KILLS, &theEmptyString);
    copy_pstring_to_static_text(dialog, iTOTAL_DEATHS, &theEmptyString);
}

// User clicked on a postgame carnage report element.  If it was a player and we're showing Total Carnage
// or a player vs player graph, switch to showing a player vs player graph according to the player clicked.
static void
respond_to_element_clicked(w_players_in_game2* inWPIG2, bool inTeam, bool inGraph, bool inScore, size_t inDrawIndex,
                           int inPlayerIndexOrTeamColor) {
    if(inGraph && !inTeam && !inScore) {
        w_select*   theGraphMenu = dynamic_cast<w_select*>(inWPIG2->get_owning_dialog()->get_widget_by_id(iGRAPH_POPUP));

        if(theGraphMenu->get_selection() != inDrawIndex)
            theGraphMenu->set_selection(inDrawIndex, true);
    }
}

// User twiddled the iGRAPH_POPUP; draw a new kind of graph in response.
static void
respond_to_graph_type_change(w_select* inGraphMenu) {
    DialogPtr p = inGraphMenu->get_owning_dialog();
    draw_new_graph(p);
}

#ifdef NETWORK_TWO_WAY_CHAT
// There's currently no underlying support for this, so we just do some fakery.
static void
send_text_fake(w_text_entry* te) {
    assert(te != NULL);
    
    dialog* d = te->get_owning_dialog();
    
    w_chat_history* ch = dynamic_cast<w_chat_history*>(d->get_widget_by_id(iCHAT_HISTORY));
    assert(ch != NULL);
    
    int netState = NetState();
    
    if(netState != netUninitialized && netState != netJoining && netState != netDown
        && !(netState == netGathering && NetGetNumberOfPlayers() <= 1))
    {
        ch->append_chat_entry(NULL, "This is not finished yet.  Your text will not be seen by others.");
        player_info* info = (player_info*)NetGetPlayerData(NetGetLocalPlayerIndex());
        ch->append_chat_entry(info, te->get_text());
    
        te->set_text("");
    }
    else {
        ch->append_chat_entry(NULL, "There is nobody in the game to hear you yet.");
    }
}
#endif // NETWORK_TWO_WAY_CHAT

// Here's the main entry point for the postgame carnage report.
void display_net_game_stats(void)
{
//printf("display_net_game_stats\n");

    dialog d;
    
    d.add(new w_static_text("POSTGAME CARNAGE REPORT", TITLE_FONT, TITLE_COLOR));
    
    w_select* graph_type_w = new w_select("Report on", 0, NULL);
    graph_type_w->set_identifier(iGRAPH_POPUP);
    graph_type_w->set_selection_changed_callback(respond_to_graph_type_change);
    graph_type_w->set_alignment(widget::kAlignCenter);
    d.add(graph_type_w);
    
    w_players_in_game2* wpig2 = new w_players_in_game2(true);	// "true": extra space for postgame labels etc.
    wpig2->set_identifier(iDAMAGE_STATS);
    wpig2->set_element_clicked_callback(respond_to_element_clicked);
    wpig2->update_display(true);	// "true": widget gets data from dynamic_world, not topology
    d.add(wpig2);
    
    d.add(new w_spacer());

// these conditionals don't do the right thing for network_postgame_chat && !network_two_way_chat - there's no
// UI for the gatherer to send.  oh well, since that combination seems unlikely at the moment, I'll leave it
// as it; someone can easily fix it if the underlying functionality is added.
#ifdef NETWORK_POSTGAME_CHAT
    w_chat_history* chat_history_w = new w_chat_history(600, 6);
    chat_history_w->set_identifier(iCHAT_HISTORY);
    d.add(chat_history_w);
#ifdef NETWORK_TWO_WAY_CHAT
    w_text_entry*	chatentry_w = new w_text_entry("Say:", 240, "");
    chatentry_w->set_identifier(iCHAT_ENTRY);
    chatentry_w->set_enter_pressed_callback(send_text_fake);
    chatentry_w->set_alignment(widget::kAlignLeft);
    chatentry_w->set_full_width();
    d.add(chatentry_w);
   
    d.add(new w_spacer());
#endif // NETWORK_TWO_WAY_CHAT
#endif // NETWORK_POSTGAME_CHAT

    // (total kills) and (total deaths) will be replaced by update_carnage_summary() or set to "".
    w_static_text*  total_kills_w = new w_static_text("(total kills)");
    total_kills_w->set_identifier(iTOTAL_KILLS);
    total_kills_w->set_alignment(widget::kAlignLeft);
    total_kills_w->set_full_width();
    d.add(total_kills_w);

    w_static_text*  total_deaths_w = new w_static_text("(total deaths)");
    total_deaths_w->set_identifier(iTOTAL_DEATHS);
    total_deaths_w->set_alignment(widget::kAlignLeft);
    total_deaths_w->set_full_width();
    d.add(total_deaths_w);

    // Place OK button in the lower right to save a little vertical space (this is more important when chat UI is present)
    w_button* ok_w = new w_button("OK", dialog_ok, &d);
    ok_w->set_alignment(widget::kAlignRight);
    ok_w->align_bottom_with_bottom_of(total_deaths_w);
    total_deaths_w->reduce_width_by_width_of(ok_w);
    total_kills_w->reduce_width_by_width_of(ok_w);
    d.add(ok_w);

	/* Calculate the rankings (once) for the entire graph */
	calculate_rankings(rankings, dynamic_world->player_count);
	qsort(rankings, dynamic_world->player_count, sizeof(struct net_rank), rank_compare);

	/* Create the graph popup menu */
	create_graph_popup_menu(graph_type_w);

{
	DialogPtr p = &d;
	draw_new_graph(p);
}

	d.run();
}










/*
 *  Game setup dialog
 */

static bool sGathererMayStartGame= false;

// ZZZ: this is kinda like check_setup_information on the Mac.
static bool
is_game_limit_valid(dialog* d) {
// jkvw: don't do this - shared code's responsability
/*    bool limit_is_valid = true;
    
    long limit_type = get_selection_control_value(d, iENDCONDITION_TYPE_MENU) - 1;

    // Benad's _game_of_defense always has two limits.
    if((get_selection_control_value(d, iGAME_TYPE) - 1) == _game_of_defense)
        limit_type = kScoreAndTimeLimits;
    
    if(limit_type & kTimeLimit)
        if(extract_number_from_text_item(d, iTIME_LIMIT) <= 0)
            limit_is_valid = false;
    
    if(limit_type & kScoreLimit)
        if(extract_number_from_text_item(d, iKILL_LIMIT) <= 0)
            limit_is_valid = false;
    
    return limit_is_valid;*/
    return true;
}

static void
update_setup_ok_button_enabled(dialog* d) {
	modify_control_enabled(d, iOK, CONTROL_ACTIVE);
// jkvw: bad bad bad - this is for shared code to deal with
/*
    short	button_state;
    
    // If limit is invalid, don't enable OK.
    if(!is_game_limit_valid(d)) {
        button_state = CONTROL_INACTIVE;
    }
    else {
        // If there's no player name, don't enable OK.
        copy_pstring_from_text_field(d, iGATHER_NAME, ptemporary);
        if(ptemporary[0] == 0) {
            button_state = CONTROL_INACTIVE;
        }
        else {
            // If there's no valid entry point, don't enable OK.
//           if(get_selection_control_value(d, iENTRY_MENU) == 0)
            entry_point theEntryPoint;
            get_selected_entry_point(d, iENTRY_MENU, &theEntryPoint);
            if(theEntryPoint.level_number < 0)
                button_state = CONTROL_INACTIVE;
            else
                button_state = CONTROL_ACTIVE;
        }
    }
    
    modify_control_enabled(d, iOK, button_state);
    */
}


// ZZZ might rework this to use menu indices again someday for the sake of
// cross-platformness - wouldn't be hard at all, just too tired tonight.
#if 0
void menu_index_to_level_entry(
	short menu_index, 
	long /*entry_flags*/,
	struct entry_point *entry)
{
    // We'd better hope the set of entry points in the static vector is right!
    *entry = sEntryPointsForCurrentGameType[menu_index - 1];
}
#endif


void set_limit_text(DialogPtr dialog, short radio_item, short radio_stringset_id, short radio_string_index,
                                short units_item, short units_stringset_id, short units_string_index)
{
        // Get the basic strings from stringsets
        const char*	limit_text = TS_GetCString(radio_stringset_id, radio_string_index);
        const char*	units_text = TS_GetCString(units_stringset_id, units_string_index);
        
        // Update the endcondition types stringset
        TS_PutCString(kEndConditionTypeStringSetID, kScoreLimit, limit_text);

        // Refresh the endcondition selection widget
        w_select*	theEndconditionWidget = dynamic_cast<w_select*>(dialog->get_widget_by_id(iRADIO_NO_TIME_LIMIT));
        
        theEndconditionWidget->set_labels_stringset(kEndConditionTypeStringSetID);
        
        
        // Update the score-limit entry label
        // We need storage that will stick around as long as the widget; let's use a static here.
        static char	score_limit_label[256];
        sprintf(score_limit_label, "%s (%s)", limit_text, units_text);
        
        w_number_entry*	theScoreLimitEntry = dynamic_cast<w_number_entry*>(dialog->get_widget_by_id(iKILL_LIMIT));
        
        theScoreLimitEntry->set_name(score_limit_label);
}


void
get_selected_entry_point(dialog* inDialog, short inItem, entry_point* outEntryPoint) {
    w_entry_point_selector* theSelector =
        dynamic_cast<w_entry_point_selector*>(inDialog->get_widget_by_id(inItem));

    *outEntryPoint = theSelector->getEntryPoint();
}

void
select_entry_point(dialog* inDialog, short inItem, int16 inLevelNumber) {
    w_entry_point_selector* theSelector =
        dynamic_cast<w_entry_point_selector*>(inDialog->get_widget_by_id(inItem));

    theSelector->setLevelNumber(inLevelNumber);
}

static void
respond_to_end_condition_type_change(w_select* inWidget) {
	SNG_limit_type_hit (inWidget->get_owning_dialog());
}

static void
respond_to_teams_toggle(w_select* inToggle) {
	SNG_teams_hit (inToggle->get_owning_dialog());
}

static void
respond_to_net_game_type_change(void* dialog) {
	SNG_game_type_hit (reinterpret_cast<DialogPTR>(dialog));
}

static void
respond_to_map_file_change(void* dialog) {
	SNG_choose_map_hit (reinterpret_cast<DialogPTR>(dialog));
}

static void
respond_to_script_twiddle(w_select* inWidget) {
	SNG_use_script_hit (inWidget->get_owning_dialog());
}

static void netgame_setup_dialog_ok(void *arg)
{
	DialogPTR d = reinterpret_cast<DialogPTR>(arg);
	if (SNG_information_is_acceptable (d))
		d->quit(0);
	else
		play_dialog_sound(DIALOG_ERROR_SOUND);
}

bool run_netgame_setup_dialog(player_info *player_information, game_info *game_information, bool inResumingGame, bool& outAdvertiseGameOnMetaserver)
{
    // Save the map file path on entering, so we can restore it if user cancels.
    string  theSavedMapFilePath(environment_preferences->map_file);

	// Create dialog
    // ZZZ note: the initial values here are nice, but now are not too important.
    // the now cross-platform fill_in_game_setup_dialog() will re-set most of them anyway.
	dialog d;
	d.add(new w_static_text("SETUP NETWORK GAME", TITLE_FONT, TITLE_COLOR));
        
	w_tab_popup *tab_w = new w_tab_popup("Section");
	vector<string> tab_strings;
	tab_strings.push_back ("General");
	tab_strings.push_back ("More Stuff");
	tab_w->set_identifier(iSNG_TABS);
	d.add(tab_w);
	QQ_set_selector_control_labels (&d, iSNG_TABS, tab_strings);
	QQ_set_selector_control_value (&d, iSNG_TABS, 0);
	d.set_active_tab(iSNG_GENERAL_TAB);
	
	d.add(new w_spacer());
	
	d.add_to_tab(new w_static_text("Appearance"), iSNG_GENERAL_TAB);

	w_text_entry *name_w = new w_text_entry("Name", PREFERENCES_NAME_LENGTH, "");
        name_w->set_identifier(iGATHER_NAME);
        name_w->set_enter_pressed_callback(dialog_try_ok);
	d.add_to_tab(name_w, iSNG_GENERAL_TAB);

	w_player_color *pcolor_w = new w_player_color("Color", player_preferences->color);
        pcolor_w->set_identifier(iGATHER_COLOR);
	d.add_to_tab(pcolor_w, iSNG_GENERAL_TAB);

	w_player_color *tcolor_w = new w_player_color("Team Color", player_preferences->team);
        tcolor_w->set_identifier(iGATHER_TEAM);
	d.add_to_tab(tcolor_w, iSNG_GENERAL_TAB);

	d.add_to_tab(new w_spacer(), iSNG_GENERAL_TAB);
        d.add_to_tab(new w_static_text("Game Options"), iSNG_GENERAL_TAB);

    // Could eventually store this path in network_preferences somewhere, so to have separate map file
    // prefs for single- and multi-player.
	w_button* map_w = new w_button("CHOOSE MAP", respond_to_map_file_change, &d);
	map_w->set_identifier(iCHOOSE_MAP);
	d.add_to_tab(map_w, iSNG_GENERAL_TAB);
	
	w_static_text* map_name_w = new w_static_text("NAME OF MAP");
	map_name_w->set_identifier(iTEXT_MAP_NAME);
	map_name_w->set_full_width();
	d.add_to_tab(map_name_w, iSNG_GENERAL_TAB);

        w_select_popup* entry_point_w = new w_select_popup("Level");
        entry_point_w->set_full_width();
        entry_point_w->set_identifier(iENTRY_MENU);
        d.add_to_tab(entry_point_w, iSNG_GENERAL_TAB);

	w_select_popup* game_type_w = new w_select_popup("Game Type", respond_to_net_game_type_change, &d);
        game_type_w->set_full_width();
        game_type_w->set_identifier(iGAME_TYPE);
        d.add_to_tab(game_type_w, iSNG_GENERAL_TAB);

	w_select *diff_w = new w_select("Difficulty", network_preferences->difficulty_level, NULL);
        diff_w->set_identifier(iDIFFICULTY_MENU);
	d.add_to_tab(diff_w, iSNG_GENERAL_TAB);

	d.add_to_tab(new w_spacer(), iSNG_GENERAL_TAB);
        
    w_select* endcondition_w    = new w_select("Game Ends At", kTimeLimit, NULL);
    endcondition_w->set_labels_stringset(kEndConditionTypeStringSetID);
    endcondition_w->set_full_width();
    endcondition_w->set_identifier(iRADIO_NO_TIME_LIMIT);
    endcondition_w->set_selection_changed_callback(respond_to_end_condition_type_change);
    d.add_to_tab(endcondition_w, iSNG_GENERAL_TAB);

    w_number_entry*	timelimit_w	= new w_number_entry("Time Limit (minutes)", network_preferences->time_limit);
    timelimit_w->set_identifier(iTIME_LIMIT);
    d.add_to_tab(timelimit_w, iSNG_GENERAL_TAB);

    // The name of this widget (score limit) will be replaced by Kill Limit, Flag Capture Limit, etc.
    w_number_entry*	scorelimit_w	= new w_number_entry("(score limit)", network_preferences->kill_limit);
    scorelimit_w->set_identifier(iKILL_LIMIT);
    d.add_to_tab(scorelimit_w, iSNG_GENERAL_TAB);

	d.add_to_tab(new w_spacer(), iSNG_GENERAL_TAB);

	w_toggle *aliens_w = new w_toggle("Aliens", (network_preferences->game_options & _monsters_replenish) != 0);
        aliens_w->set_identifier(iUNLIMITED_MONSTERS);
	d.add_to_tab(aliens_w, iSNG_GENERAL_TAB);

	w_toggle *teams_w = new w_toggle("Teams", !(network_preferences->game_options & _force_unique_teams));
        teams_w->set_identifier(iFORCE_UNIQUE_TEAMS);
        teams_w->set_selection_changed_callback(respond_to_teams_toggle);
	d.add_to_tab(teams_w, iSNG_GENERAL_TAB);

	w_toggle *drop_w = new w_toggle("Dead Players Drop Items", !(network_preferences->game_options & _burn_items_on_death));
        drop_w->set_identifier(iBURN_ITEMS_ON_DEATH);
	d.add_to_tab(drop_w, iSNG_GENERAL_TAB);

	w_toggle *pen_die_w = new w_toggle("Penalize Dying (10 seconds)", (network_preferences->game_options & _dying_is_penalized) != 0);
        pen_die_w->set_identifier(iDYING_PUNISHED);
	d.add_to_tab(pen_die_w, iSNG_GENERAL_TAB);

	w_toggle *pen_sui_w = new w_toggle("Penalize Suicide (15 seconds)", (network_preferences->game_options & _suicide_is_penalized) != 0);
        pen_sui_w->set_identifier(iSUICIDE_PUNISHED);
	d.add_to_tab(pen_sui_w, iSNG_GENERAL_TAB);

	d.add_to_tab(new w_spacer(), iSNG_GENERAL_TAB);

	w_toggle *advertise_on_metaserver_w = new w_toggle("Advertise Game on Internet", sAdvertiseGameOnMetaserver);
	advertise_on_metaserver_w->set_identifier(iADVERTISE_GAME_ON_METASERVER);
	d.add_to_tab(advertise_on_metaserver_w, iSNG_GENERAL_TAB);

	w_toggle* use_netscript_w = new w_toggle("Use Netscript", false);
	use_netscript_w->set_identifier(iUSE_SCRIPT);
	use_netscript_w->set_selection_changed_callback(respond_to_script_twiddle);
	d.add_to_tab(use_netscript_w, iSNG_STUFF_TAB);
	
	w_static_text* script_name_w = new w_static_text("");
	script_name_w->set_identifier(iTEXT_SCRIPT_NAME);
	script_name_w->set_full_width();
	d.add_to_tab(script_name_w, iSNG_STUFF_TAB);

	d.add_to_tab(new w_spacer(), iSNG_STUFF_TAB);

	w_toggle*   realtime_audio_w = new w_toggle("Allow Microphone", network_preferences->allow_microphone);
	realtime_audio_w->set_identifier(iREAL_TIME_SOUND);
	d.add_to_tab(realtime_audio_w, iSNG_STUFF_TAB);

	w_toggle *live_w = new w_toggle("Live Carnage Reporting", (network_preferences->game_options & _live_network_stats) != 0);
	live_w->set_identifier(iREALTIME_NET_STATS);
	d.add_to_tab(live_w, iSNG_STUFF_TAB);

	w_toggle *sensor_w = new w_toggle("Disable Motion Sensor", (network_preferences->game_options & _motion_sensor_does_not_work) != 0);
        sensor_w->set_identifier(iMOTION_SENSOR_DISABLED);
	d.add_to_tab(sensor_w, iSNG_STUFF_TAB);

	d.add_to_tab(new w_spacer(), iSNG_STUFF_TAB);

	w_toggle *zoom_w = new w_toggle("Allow Zoom", true);
        zoom_w->set_identifier(iALLOW_ZOOM);
	d.add_to_tab(zoom_w, iSNG_STUFF_TAB);
	
	w_toggle *crosshairs_w = new w_toggle("Allow Crosshairs", true);
        crosshairs_w->set_identifier(iALLOW_CROSSHAIRS);
	d.add_to_tab(crosshairs_w, iSNG_STUFF_TAB);
	
	w_toggle *lara_croft_w = new w_toggle("Allow Lara Croft", true);
        lara_croft_w->set_identifier(iALLOW_LARA_CROFT);
	d.add_to_tab(lara_croft_w, iSNG_STUFF_TAB);

	d.add(new w_spacer());	

	w_left_button*	ok_w = new w_left_button("OK", netgame_setup_dialog_ok, &d);
	ok_w->set_identifier(iOK);
	d.add(ok_w);
        
	w_right_button*	cancel_w = new w_right_button("CANCEL", dialog_cancel, &d);
	cancel_w->set_identifier(iCANCEL);
	d.add(cancel_w);

	netgame_setup_dialog_initialise(&d, false, inResumingGame);
    
        // or is it called automatically by fill_in_game_setup_dialog somehow?  the interactions are getting complex... :/
        // in any event, the data SHOULD be ok, or else we wouldn't have let the user save the info in prefs, but....
        // (Hmm, true user should not have been able to save bad data, but if we have a different map file now, we could
        // have an illegal configuration showing.)
        update_setup_ok_button_enabled(&d);

	// Run dialog
	if (d.run() == 0) { // Accepted

        short   theLimitType;

        switch(endcondition_w->get_selection()) {
        case kScoreLimit:
            theLimitType = iRADIO_KILL_LIMIT;
        break;

        case kTimeLimit:
            theLimitType = iRADIO_TIME_LIMIT;
        break;

        case kNoLimit:
            theLimitType = iRADIO_NO_TIME_LIMIT;
        break;

        default:
            // This avoids a compiler warning
            theLimitType = NONE;
            assert(false);
        break;
        }

        // This will write preferences changes (including change of map file if applicable)
        netgame_setup_dialog_extract_information(&d, player_information, game_information, false /*allow all levels*/, inResumingGame, outAdvertiseGameOnMetaserver);

		return true;
	} // d.run() == 0
        
    else {
		return false;
    }
} // network_game_setup







/////// Shared metaserver chat hookup stuff

class PregameDialogNotificationAdapter : public MetaserverClient::NotificationAdapter
{
public:
	PregameDialogNotificationAdapter(w_chat_history& chatHistory)
	: m_chatHistory(chatHistory)
	{
	}

	void receivedChatMessage(const std::string& senderName, uint32 senderID, const std::string& message)
	{
		m_chatHistory.append_chat_entry(senderName.c_str(), 0xaaaaaaaa, 0xaaaaaaaa, message.c_str());
		m_chatHistory.get_owning_dialog()->draw_dirty_widgets();
	}

	void receivedBroadcastMessage(const std::string& message)
	{
		receivedChatMessage("Metaserver", 0, message);
	}

	void playersInRoomChanged() {}
	void gamesInRoomChanged() {}

private:
	w_chat_history&	m_chatHistory;

	PregameDialogNotificationAdapter(const PregameDialogNotificationAdapter&);
	PregameDialogNotificationAdapter& operator =(const PregameDialogNotificationAdapter&);
};


static MetaserverClient* sMetaserverClient = NULL;

static void
send_text(w_text_entry* te) {
	assert(te != NULL);

	// Make sure there's something worth sending
	if(strlen(te->get_text()) <= 0)
		return;

	sMetaserverClient->sendChatMessage(te->get_text());
	te->set_text("");
}

static void
setup_metaserver_chat_ui(
			 dialog& inDialog,
			 MetaserverClient& metaserverClient,
			 int historyLines,
			 auto_ptr<PregameDialogNotificationAdapter>& outNotificationAdapter,
			 auto_ptr<MetaserverClient::NotificationAdapterInstaller>& outNotificationAdapterInstaller
			 )
{
	assert (metaserverClient.isConnected());

	sMetaserverClient = &metaserverClient;

	w_chat_history* chatHistory = new w_chat_history(600, historyLines);
	chatHistory->set_identifier(iCHAT_HISTORY);
	inDialog.add(chatHistory);

	w_text_entry*	chatentry_w = new w_text_entry("Say:", 240, "");
	chatentry_w->set_with_textbox();
	chatentry_w->set_identifier(iCHAT_ENTRY);
	chatentry_w->set_enter_pressed_callback(send_text);
	chatentry_w->set_alignment(widget::kAlignLeft);
	chatentry_w->set_full_width();
	inDialog.add(chatentry_w);

	inDialog.add(new w_spacer());

	outNotificationAdapter.reset(new PregameDialogNotificationAdapter(*chatHistory));
	outNotificationAdapterInstaller.reset(new MetaserverClient::NotificationAdapterInstaller(outNotificationAdapter.get(), metaserverClient));	
}


/*
 *  Gathering dialog
 */

class SdlGatherDialog : public GatherDialog
{
public:
	SdlGatherDialog::SdlGatherDialog()
	{
		m_dialog.add(new w_static_text("GATHER NETWORK GAME", TITLE_FONT, TITLE_COLOR));
	
		m_dialog.add(new w_spacer());
	
		// m_dialog.add(new w_static_text("Players on Network"));
	
		w_joining_players_in_room* foundplayers_w = new w_joining_players_in_room(NULL, 320, 3);
		m_dialog.add(foundplayers_w);
	
		w_toggle* autogather_w = new w_toggle("Auto-Gather", false);
		m_dialog.add(autogather_w);
	
		m_dialog.add(new w_spacer());
	
		w_players_in_game2* players_w = new w_players_in_game2(false);
		m_dialog.add(players_w);
		
		w_left_button* play_button_w = new w_left_button("PLAY");
		m_dialog.add(play_button_w);
	
		w_right_button* cancel_w = new w_right_button("CANCEL");
		m_dialog.add(cancel_w);

		w_select_popup* chat_choice_w = new w_select_popup("chat:");
		m_dialog.add(chat_choice_w);

		w_text_box* chat_history_w = new w_text_box(600, 6);
		m_dialog.add(chat_history_w);

		w_text_entry* chatentry_w = new w_text_entry("Say:", 240, "");
		chatentry_w->set_with_textbox();
		chatentry_w->set_alignment(widget::kAlignLeft);
		chatentry_w->set_full_width();
		m_dialog.add(chatentry_w);
		
		
		m_cancelWidget = new ButtonWidget (cancel_w);
		m_startWidget = new ButtonWidget (play_button_w);
		
		m_autogatherWidget = new AutogatherWidget (new ToggleWidget (autogather_w));
	
		m_ungatheredWidget = new JoiningPlayerListWidget (foundplayers_w);
		m_pigWidget = new PlayersInGameWidget (players_w);
		
		m_chatEntryWidget = new EditTextWidget (chatentry_w);
		m_chatWidget = new HistoricTextboxWidget (new TextboxWidget (chat_history_w));
		m_chatChoiceWidget = new PopupSelectorWidget (chat_choice_w);
	}
	
	virtual bool Run ()
	{
		m_dialog.set_processing_function (boost::bind(&SdlGatherDialog::idle, this));
		return (m_dialog.run() == 0);
	}
	
	virtual void Stop (bool result)
	{
		if (result)
			m_dialog.quit (0);
		else
			m_dialog.quit (-1);
	}
	
private:
	dialog m_dialog;
};

auto_ptr<GatherDialog>
GatherDialog::Create()
{
	return auto_ptr<GatherDialog>(new SdlGatherDialog);
}


/*
 *  Joining dialog
 */

class SdlJoinDialog : public JoinDialog
{
public:
	SdlJoinDialog::SdlJoinDialog()
	{
		m_dialog.add(new w_static_text("JOIN NETWORK GAME", TITLE_FONT, TITLE_COLOR));
		m_dialog.add(new w_spacer());

		w_text_entry *name_w = new w_text_entry("Name", PREFERENCES_NAME_LENGTH, "");
		m_dialog.add(name_w);
	
		w_player_color *pcolor_w = new w_player_color("Color", 0);
		m_dialog.add(pcolor_w);

		w_player_color *tcolor_w = new w_player_color("Team Color", 0);
		m_dialog.add(tcolor_w);
	
		m_dialog.add(new w_spacer());

		w_toggle* hint_w = new w_toggle("Join by address", false);
                m_dialog.add_to_tab(hint_w, iJOIN_PREJOIN_TAB);

		w_text_entry* hint_address_w = new w_text_entry("Join address", kJoinHintingAddressLength, "");
		m_dialog.add_to_tab(hint_address_w, iJOIN_PREJOIN_TAB);

		m_dialog.add_to_tab(new w_spacer(), iJOIN_PREJOIN_TAB);

		w_static_text* join_messages_w = new w_static_text("");
		join_messages_w->set_full_width ();
		// jkvw: add it to dialog, but never show it.
		//       Two things which we need don't work:
		//       1) w_static_text can't handle text longer than the dialog width
		//       2) widgets in dialog don't update layout position once dialog starts to run
		//       If we get solutions to these issues, then we can show the join messages.
		m_dialog.add_to_tab(join_messages_w, iJOIN_NEVERSHOW_TAB);

		m_dialog.add_to_tab(new w_spacer(), iJOIN_PREJOIN_TAB);

		w_button* join_by_metaserver_w = new w_button("FIND INTERNET GAME");
		m_dialog.add_to_tab(join_by_metaserver_w, iJOIN_PREJOIN_TAB);

		m_dialog.add_to_tab(new w_spacer(), iJOIN_PREJOIN_TAB);

		w_players_in_game2* players_w = new w_players_in_game2(false);
		m_dialog.add_to_tab(players_w, iJOIN_POSTJOIN_TAB);

		w_select_popup* chat_choice_w = new w_select_popup("chat:");
		m_dialog.add_to_tab(chat_choice_w, iJOIN_POSTJOIN_TAB);

		w_text_box* chat_history_w = new w_text_box(600, 7);
		m_dialog.add_to_tab(chat_history_w, iJOIN_POSTJOIN_TAB);

		w_text_entry* chatentry_w = new w_text_entry("Say:", 240, "");
		chatentry_w->set_with_textbox();
		chatentry_w->set_alignment(widget::kAlignLeft);
		chatentry_w->set_full_width();
		m_dialog.add_to_tab(chatentry_w, iJOIN_POSTJOIN_TAB);

		w_left_button* join_w = new w_left_button("JOIN");
		m_dialog.add(join_w);
	
		w_right_button* cancel_w = new w_right_button("CANCEL");
		m_dialog.add(cancel_w);

		m_cancelWidget = new ButtonWidget (cancel_w);
		m_joinWidget = new ButtonWidget (join_w);
	
		m_joinMetaserverWidget = new ButtonWidget (join_by_metaserver_w);
		m_joinAddressWidget = new JoinAddressWidget (new EditTextWidget (hint_address_w));
		m_joinByAddressWidget = new JoinByAddressWidget (new ToggleWidget (hint_w));
	
		m_nameWidget = new NameWidget (new EditTextWidget (name_w));
		m_colourWidget = new ColourWidget (new ColourSelectorWidget (pcolor_w));
		m_teamWidget = new TeamWidget (new ColourSelectorWidget (tcolor_w));
	
		m_messagesWidget = new StaticTextWidget (join_messages_w);
	
		m_pigWidget = new PlayersInGameWidget (players_w);
		
		m_chatEntryWidget = new EditTextWidget (chatentry_w);
		m_chatWidget = new HistoricTextboxWidget (new TextboxWidget (chat_history_w));
		m_chatChoiceWidget = new PopupSelectorWidget (chat_choice_w);
	}

	virtual void Run ()
	{
		m_dialog.set_processing_function (boost::bind(&SdlJoinDialog::gathererSearch, this));
		m_dialog.run();
	}
	
	virtual void Stop()
	{
		if (join_result == kNetworkJoinFailedUnjoined || join_result == kNetworkJoinFailedJoined)
			m_dialog.quit(-1);
		else
			m_dialog.quit(0);
	}
	
	virtual void respondToJoinHit()
	{
		play_dialog_sound(DIALOG_OK_SOUND);
		m_dialog.set_active_tab (iJOIN_POSTJOIN_TAB);
		JoinDialog::respondToJoinHit();
	}
	
	virtual ~SdlJoinDialog()
	{
		delete m_cancelWidget;
		delete m_joinWidget;
		delete m_joinMetaserverWidget;
		delete m_joinAddressWidget;
		delete m_joinByAddressWidget;
		delete m_nameWidget;
		delete m_colourWidget;
		delete m_teamWidget;
		delete m_messagesWidget;
		delete m_pigWidget;
		delete m_chatChoiceWidget;
		delete m_chatEntryWidget;
		delete m_chatWidget;
	}

private:
	enum {
		iJOIN_PREJOIN_TAB,
		iJOIN_POSTJOIN_TAB,
		iJOIN_NEVERSHOW_TAB
	};

	dialog m_dialog;
};

auto_ptr<JoinDialog>
JoinDialog::Create()
{
	return auto_ptr<JoinDialog>(new SdlJoinDialog);
}




/*
 *  Progress dialog (ZZZ)
 */



// This should really be done better, I guess, but most people will never see it long enough to read it.
// Currently no actual bar is drawn (just a box with message), and no effort is made to make sure all messages
// will physically fit into the box.  (should probably somehow force the dialog to a width and set_full_width on
// the text widget.  Maybe alter its justification also.)
dialog*		sProgressDialog 	= NULL;
w_static_text*	sProgressMessage	= NULL;
//widget* 	sProgressBar		= NULL;

void open_progress_dialog(size_t message_id)
{
//printf("open_progress_dialog %d\n", message_id);

    assert(sProgressDialog == NULL);
    
    sProgressDialog 	= new dialog;
    sProgressMessage	= new w_static_text(TS_GetCString(strPROGRESS_MESSAGES, message_id));
//    sProgressBar	= new w_progress_bar;
    
    sProgressDialog->add(sProgressMessage);
//    sProgressDialog->add(sProgressBar);
    
    sProgressDialog->start(false);

    bool done = sProgressDialog->process_events();
    assert(!done);
}


void set_progress_dialog_message(size_t message_id)
{
//printf("set_progress_dialog_message %d\n", message_id);
    assert(sProgressMessage != NULL);

    sProgressMessage->set_text(TS_GetCString(strPROGRESS_MESSAGES, message_id));
    
    bool done = sProgressDialog->process_events();
    assert(!done);
}

void close_progress_dialog(void)
{
//printf("close_progress_dialog\n");

    assert(sProgressDialog != NULL);
    
    sProgressDialog->quit(0);
    
    bool done = sProgressDialog->process_events();
    
    assert(done);
    
    int result = sProgressDialog->finish(false);
    
    assert(result == 0);
    
    delete sProgressDialog;
    
    sProgressDialog	= NULL;
    sProgressMessage	= NULL;
//    sProgressBar	= NULL;
}

void draw_progress_bar(size_t sent, size_t total)
{
//printf("draw_progress_bar %ld, %ld", sent, total);
}

void reset_progress_bar(void)
{
//printf("reset_progress_bar\n");
}










#ifdef NETWORK_TEST_POSTGAME_DIALOG
static const char*    sTestingNames[] = {
        "Doctor Burrito",
        "Carnage Asada",
        "Bongo Bob",
        "The Napalm Man",
        "Kissy Monster",
        "lala",
        "Prof. Windsurf",
        "-ZED-"
};

// THIS ONE IS FAKE - used to test postgame report dialog without going through a game.
bool network_gather(void) {
    short i, j;
    player_info thePlayerInfo;
    game_info   theGameInfo;
	
    if(network_game_setup(&thePlayerInfo, &theGameInfo)) {

	for (i = 0; i < MAXIMUM_NUMBER_OF_PLAYERS; i++)
	{
        // make up a name
        /*int theNameLength = (local_random() % MAXIMUM_PLAYER_NAME_LENGTH) + 1;
        for(int n = 0; n < theNameLength; n++)
            players[i].name[n] = 'a' + (local_random() % ('z' - 'a'));

        players[i].name[theNameLength] = '\0';
*/
        strcpy(players[i].name, sTestingNames[i]);

        // make up a team and color
        players[i].color = local_random() % 8;
        int theNumberOfTeams = 2 + (local_random() % 3);
        players[i].team  = local_random() % theNumberOfTeams;

		(players+i)->monster_damage_taken.damage = abs(local_random()%200);
		(players+i)->monster_damage_taken.kills = abs(local_random()%30);
		(players+i)->monster_damage_given.damage = abs(local_random()%200);
		(players+i)->monster_damage_given.kills = abs(local_random()%30);
                
                players[i].netgame_parameters[0] = local_random() % 200;
                players[i].netgame_parameters[1] = local_random() % 200;
		
		for (j = 0; j < MAXIMUM_NUMBER_OF_PLAYERS; j++)
		{
			(players+i)->damage_taken[j].damage = abs(local_random()%200);
			(players+i)->damage_taken[j].kills = abs(local_random()%6);
		}
	}

    dynamic_world->player_count = MAXIMUM_NUMBER_OF_PLAYERS;

    game_data& game_information = dynamic_world->game_information;
    game_info* network_game_info = &theGameInfo;

	game_information.game_time_remaining= network_game_info->time_limit;
	game_information.kill_limit= network_game_info->kill_limit;
	game_information.game_type= network_game_info->net_game_type;
	game_information.game_options= network_game_info->game_options;
	game_information.initial_random_seed= network_game_info->initial_random_seed;
	game_information.difficulty_level= network_game_info->difficulty_level;

    display_net_game_stats();
    } // if setup box was OK'd
    return false;
}
#endif // NETWORK_TEST_POSTGAME_DIALOG





#ifdef NETWORK_TEST_MICROPHONE_LOCALLY
static void
respond_to_microphone_toggle(w_select* inWidget) {
    set_network_microphone_state(inWidget->get_selection() != 0);
}

bool
network_gather(bool) {
    open_network_speaker();
    open_network_microphone();

    dialog d;

    d.add(new w_static_text("TEST MICROPHONE", TITLE_FONT, TITLE_COLOR));

    w_toggle*   onoff_w = new w_toggle("Active", 0);
    onoff_w->set_selection_changed_callback(respond_to_microphone_toggle);
    d.add(onoff_w);

    d.add(new w_button("DONE", dialog_ok, &d));

    d.run();

    close_network_microphone();
    close_network_speaker();

    return false;
}
#endif

#endif // !defined(DISABLE_NETWORKING)
