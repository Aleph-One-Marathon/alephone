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

// String-Set Functions (for getting strings from MML rather than compiled-in)
#include	"TextStrings.h"

// Shared dialog item ID constants
#include	"network_dialogs.h"

// get_entry_point_flags_for_game_type
#include	"network_games.h"

// LAN game-location services
#include	"network_private.h"




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
    kScoreLimit			= 0x01,
    kTimeLimit			= 0x02,
    kScoreAndTimeLimits		= kScoreLimit | kTimeLimit // currently cannot be selected directly
};

// Some identifiers used only locally.  Hope the numeric equivalents don't conflict!
// (they shouldn't.)
enum {
        iENDCONDITION_TYPE_MENU		= 4242,	// Score limit?  Time limit?  No limit?
        iCHAT_HISTORY,				// Where chat text appears
        iCHAT_ENTRY,				// Where chat text is entered
		iHINT_TOGGLE,			// Join by Address on/off
		iHINT_ADDRESS_ENTRY,		// Join by address address
                iAUTO_GATHER,			// Auto-gather on/off
        iMAP_FILE_SELECTOR			// Choose Map file (not choose level within map)
                
};







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
draw_names(DialogPtr dialog, struct net_rank *ranks, short number_of_bars, short which_player) {
    // This does nothing here - draw_kill_bars or draw_score_bars is assumed to have enough data to work with,
    // and one of those is always called adjacent to a call to draw_names in practice.
}

void
draw_kill_bars(DialogPtr dialog, struct net_rank *ranks, short num_players, 
               short suicide_index, bool do_totals, bool friendly_fire)
{
    // We don't actually draw here - we just pass the data along to the widget, and it will take care of the rest. 
    w_players_in_game2* wpig2 = dynamic_cast<w_players_in_game2*>(dialog->get_widget_by_id(iDAMAGE_STATS));
    wpig2->set_graph_data(ranks, num_players, suicide_index, (ranks[0].player_index == NONE) ? true : false, false);

    update_carnage_summary(dialog, ranks, num_players, suicide_index, do_totals, friendly_fire);
}

void
draw_score_bars(DialogPtr dialog, struct net_rank *ranks, short bar_count) {
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
    draw_new_graph(inGraphMenu->get_owning_dialog());
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
    
    draw_new_graph(&d);
    
    d.run();
}










/*
 *  Game setup dialog
 */

static bool sGathererMayStartGame= false;

// ZZZ: this is kinda like check_setup_information on the Mac.
static bool
is_game_limit_valid(dialog* d) {
    bool limit_is_valid = true;
    
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
    
    return limit_is_valid;
}

static void
update_setup_ok_button_enabled(dialog* d) {
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
}

// This can be plugged in directly for the three entry widgets' callbacks.
static void
update_setup_ok_button_enabled_callback_adaptor(w_text_entry* inWidget) {
    update_setup_ok_button_enabled(inWidget->get_owning_dialog());
}

static void
respond_to_end_condition_type_change(w_select* inWidget) {
    dialog*	theDialog = inWidget->get_owning_dialog();

    switch(inWidget->get_selection()) {
        case kNoLimit:
            setup_for_untimed_game(theDialog);
        break;
        
        case kTimeLimit:
            setup_for_timed_game(theDialog);
        break;
        
        case kScoreLimit:
            setup_for_score_limited_game(theDialog);
        break;
        
        default:
            assert(false);
        break;
    }
    
    // make sure "ok" button state is current.
    update_setup_ok_button_enabled(theDialog);
} // respond_to_end_condition_type_change




static void respond_to_teams_toggle(w_select* inToggle) {
    modify_control_enabled(inToggle->get_owning_dialog(), iGATHER_TEAM,
        inToggle->get_selection() ? CONTROL_ACTIVE : CONTROL_INACTIVE);
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

void set_limit_type(DialogPtr dialog, short limit_type) {
    switch(limit_type) {
        case iRADIO_NO_TIME_LIMIT:
            modify_selection_control(dialog, iENDCONDITION_TYPE_MENU, CONTROL_ACTIVE, kNoLimit + 1);
        break;
        
        case iRADIO_TIME_LIMIT:
            modify_selection_control(dialog, iENDCONDITION_TYPE_MENU, CONTROL_ACTIVE, kTimeLimit + 1);
        break;
        
        case iRADIO_KILL_LIMIT:
            modify_selection_control(dialog, iENDCONDITION_TYPE_MENU, CONTROL_ACTIVE, kScoreLimit + 1);
        break;
        
        default:
            assert(false);
        break;
    }
}

void modify_limit_type_choice_enabled(DialogPtr dialog, short inChangeEnable)
{
        modify_control_enabled(dialog, iENDCONDITION_TYPE_MENU, inChangeEnable);
}


void set_limit_text(DialogPtr dialog, short radio_item, short radio_stringset_id, short radio_string_index,
                                short units_item, short units_stringset_id, short units_string_index)
{
        // Get the basic strings from stringsets
        const char*	limit_text = TS_GetCString(radio_stringset_id, radio_string_index);
        const char*	units_text = TS_GetCString(units_stringset_id, units_string_index);
        
        // Update the endcondition types stringset
        TS_PutCString(kEndConditionTypeStringSetID, kScoreLimit, limit_text);

        // Refresh the endcondition selection widget
        w_select*	theEndconditionWidget = dynamic_cast<w_select*>(dialog->get_widget_by_id(iENDCONDITION_TYPE_MENU));
        
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


void
set_dialog_netscript_file(DialogPtr inDialog, const FileSpecifier& inFile)
{
	w_file_chooser* theChooser = dynamic_cast<w_file_chooser*>(inDialog->get_widget_by_id(iCHOOSE_SCRIPT));

	theChooser->set_file(inFile);
}

const FileSpecifier&
get_dialog_netscript_file(DialogPtr inDialog)
{
	w_file_chooser* theChooser = dynamic_cast<w_file_chooser*>(inDialog->get_widget_by_id(iCHOOSE_SCRIPT));

	return theChooser->get_file();
}


static void
respond_to_net_game_type_change(w_select* inWidget) {
        // Get the newly-selected game type and calculate stringset ID
        size_t	theSelectedGameType = inWidget->get_selection();

        // Get the level-selection menu
        dialog*	theDialog = inWidget->get_owning_dialog();
        w_entry_point_selector* theSelector = 
            dynamic_cast<w_entry_point_selector*>(theDialog->get_widget_by_id(iENTRY_MENU));
        assert(theSelector != NULL);

        // Update the game type in the level selector
        theSelector->setGameType(theSelectedGameType);
    
        // Adjust default/available options, end condition, etc. (cross-platform)
        setup_dialog_for_game_type(theDialog, theSelectedGameType);
        
        // make sure ok button state is current.
        update_setup_ok_button_enabled(theDialog);
} // respond_to_net_game_type_change


static void
respond_to_map_file_change(w_env_select* inWidget) {
    if(strcmp(environment_preferences->map_file, inWidget->get_path())) {
        strcpy(environment_preferences->map_file, inWidget->get_path());
		environment_preferences->map_checksum = read_wad_file_checksum(inWidget->get_file_specifier());

        load_environment_from_preferences();

        // We don't write_preferences in case user cancels (in which case environment_preferences->map_file
        // and map_checksum are restored).

        dynamic_cast<w_entry_point_selector*>(inWidget->get_owning_dialog()->get_widget_by_id(iENTRY_MENU))->reset();

        // There might not be any levels for the currently selected game-type; need to double-check the OK button.
        update_setup_ok_button_enabled(inWidget->get_owning_dialog());
    }
}


bool network_game_setup(player_info *player_information, game_info *game_information, bool inResumingGame)
{
//printf("network_game_setup\n");

    // Save the map file path on entering, so we can restore it if user cancels.
    string  theSavedMapFilePath(environment_preferences->map_file);

	// Create dialog
    // ZZZ note: the initial values here are nice, but now are not too important.
    // the now cross-platform fill_in_game_setup_dialog() will re-set most of them anyway.
	dialog d;
	d.add(new w_static_text("SETUP NETWORK GAME", TITLE_FONT, TITLE_COLOR));
	d.add(new w_spacer());
        
	d.add(new w_static_text("Appearance"));

	w_text_entry *name_w = new w_text_entry("Name", PREFERENCES_NAME_LENGTH, "");
        name_w->set_identifier(iGATHER_NAME);
        name_w->set_enter_pressed_callback(dialog_try_ok);
        name_w->set_value_changed_callback(update_setup_ok_button_enabled_callback_adaptor);
	d.add(name_w);

	w_player_color *pcolor_w = new w_player_color("Color", player_preferences->color);
        pcolor_w->set_identifier(iGATHER_COLOR);
	d.add(pcolor_w);

	w_player_color *tcolor_w = new w_player_color("Team Color", player_preferences->team);
        tcolor_w->set_identifier(iGATHER_TEAM);
	d.add(tcolor_w);

	d.add(new w_spacer());
        d.add(new w_static_text("Game Options"));
        
	w_select *type_w = new w_select("Game Type", network_preferences->game_type, NULL);
        type_w->set_labels_stringset(kNetworkGameTypesStringSetID);
        type_w->set_identifier(iGAME_TYPE);
        type_w->set_selection_changed_callback(respond_to_net_game_type_change);
	d.add(type_w);

	w_enabling_toggle* use_netscript_w = new w_enabling_toggle("Use Netscript", false);
	use_netscript_w->set_identifier(iUSE_SCRIPT);
	d.add(use_netscript_w);
	
	w_file_chooser* netscript_file_w = new w_file_chooser("Netscript", "SELECT NETSCRIPT", _typecode_netscript);
	netscript_file_w->set_identifier(iCHOOSE_SCRIPT);
	netscript_file_w->set_full_width();
	d.add(netscript_file_w);

	use_netscript_w->add_dependent_widget(netscript_file_w);

    // Could eventually store this path in network_preferences somewhere, so to have separate map file
    // prefs for single- and multi-player.
	    w_env_select *map_w = new w_env_select("Map file", environment_preferences->map_file, "AVAILABLE MAPS", _typecode_scenario, &d);
        map_w->set_selection_made_callback(respond_to_map_file_change);
        map_w->set_full_width();
        map_w->set_identifier(iMAP_FILE_SELECTOR);
	    d.add(map_w);

        w_entry_point_selector* entry_point_w = new w_entry_point_selector("Level", network_preferences->game_type, 0);
        entry_point_w->set_full_width();
        entry_point_w->set_identifier(iENTRY_MENU);
        d.add(entry_point_w);

	w_select *diff_w = new w_select("Difficulty", network_preferences->difficulty_level, NULL);
        diff_w->set_labels_stringset(kDifficultyLevelsStringSetID);
        diff_w->set_identifier(iDIFFICULTY_MENU);
	d.add(diff_w);

	d.add(new w_spacer());
        
    w_select* endcondition_w    = new w_select("Game Ends At", kTimeLimit, NULL);
    endcondition_w->set_labels_stringset(kEndConditionTypeStringSetID);
    endcondition_w->set_full_width();
    endcondition_w->set_identifier(iENDCONDITION_TYPE_MENU);
    endcondition_w->set_selection_changed_callback(respond_to_end_condition_type_change);
    d.add(endcondition_w);

    w_number_entry*	timelimit_w	= new w_number_entry("Time Limit (minutes)", network_preferences->time_limit);
    timelimit_w->set_identifier(iTIME_LIMIT);
        timelimit_w->set_value_changed_callback(update_setup_ok_button_enabled_callback_adaptor);
    d.add(timelimit_w);

    // The name of this widget (score limit) will be replaced by Kill Limit, Flag Capture Limit, etc.
    w_number_entry*	scorelimit_w	= new w_number_entry("(score limit)", network_preferences->kill_limit);
    scorelimit_w->set_identifier(iKILL_LIMIT);
        scorelimit_w->set_value_changed_callback(update_setup_ok_button_enabled_callback_adaptor);
    d.add(scorelimit_w);

	d.add(new w_spacer());

	w_toggle *aliens_w = new w_toggle("Aliens", (network_preferences->game_options & _monsters_replenish) != 0);
        aliens_w->set_identifier(iUNLIMITED_MONSTERS);
	d.add(aliens_w);

    w_toggle*   realtime_audio_w = new w_toggle("Realtime audio (broadband only)", network_preferences->allow_microphone);
    realtime_audio_w->set_identifier(iREAL_TIME_SOUND);
    d.add(realtime_audio_w);

	w_toggle *live_w = new w_toggle("Live Carnage Reporting", (network_preferences->game_options & _live_network_stats) != 0);
        live_w->set_identifier(iREALTIME_NET_STATS);
	d.add(live_w);

	w_toggle *teams_w = new w_toggle("Teams", !(network_preferences->game_options & _force_unique_teams));
        teams_w->set_identifier(iFORCE_UNIQUE_TEAMS);
        teams_w->set_selection_changed_callback(respond_to_teams_toggle);
	d.add(teams_w);

	w_toggle *drop_w = new w_toggle("Dead Players Drop Items", !(network_preferences->game_options & _burn_items_on_death));
        drop_w->set_identifier(iBURN_ITEMS_ON_DEATH);
	d.add(drop_w);

	w_toggle *sensor_w = new w_toggle("Disable Motion Sensor", (network_preferences->game_options & _motion_sensor_does_not_work) != 0);
        sensor_w->set_identifier(iMOTION_SENSOR_DISABLED);
	d.add(sensor_w);

	w_toggle *pen_die_w = new w_toggle("Penalize Dying (10 seconds)", (network_preferences->game_options & _dying_is_penalized) != 0);
        pen_die_w->set_identifier(iDYING_PUNISHED);
	d.add(pen_die_w);

	w_toggle *pen_sui_w = new w_toggle("Penalize Suicide (15 seconds)", (network_preferences->game_options & _suicide_is_penalized) != 0);
        pen_sui_w->set_identifier(iSUICIDE_PUNISHED);
	d.add(pen_sui_w);

	d.add(new w_spacer());

        w_left_button*	ok_w = new w_left_button("OK", dialog_ok, &d);
        ok_w->set_identifier(iOK);
	d.add(ok_w);
        
        w_right_button*	cancel_w = new w_right_button("CANCEL", dialog_cancel, &d);
        cancel_w->set_identifier(iCANCEL);
	d.add(cancel_w);

        fill_in_game_setup_dialog(&d, player_information, false, inResumingGame);

	if(inResumingGame)
        {
                // If resuming, they shouldn't be allowed to change the Map file
                // (this should go in fill_in_game_setup_dialog() if the Mac version gets a Map file selector)
                modify_control_enabled(&d, iMAP_FILE_SELECTOR, CONTROL_INACTIVE);
        }
    
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
        extract_setup_dialog_information(&d, player_information, game_information, theLimitType, false /*allow all levels*/, inResumingGame);

		return true;
	} // d.run() == 0
        
    else {
       // Restore the map file path if it was changed - no need to resave prefs.
        if(strcmp(theSavedMapFilePath.c_str(), map_w->get_path())) {
            strcpy(environment_preferences->map_file, theSavedMapFilePath.c_str());

            FileSpecifier   theFS(theSavedMapFilePath.c_str());

		    environment_preferences->map_checksum = read_wad_file_checksum(theFS);

            load_environment_from_preferences();
       }

		return false;
    }
} // network_game_setup











/*
 *  Gathering dialog
 */


 // Currently only the gatherer should call this one.
static void
send_text(w_text_entry* te) {
    assert(te != NULL);

    // Make sure there's something worth sending
    if(strlen(te->get_text()) <= 0)
        return;

    dialog* d = te->get_owning_dialog();
    
    w_chat_history* ch = dynamic_cast<w_chat_history*>(d->get_widget_by_id(iCHAT_HISTORY));
    assert(ch != NULL);
    
    int netState = NetState();
    
    if(netState != netUninitialized && netState != netJoining && netState != netDown
        && !(netState == netGathering && NetGetNumberOfPlayers() <= 1))
    {
        NetDistributeChatMessage(NetGetPlayerIdentifier(NetGetLocalPlayerIndex()), te->get_text());
        player_info* info = (player_info*)NetGetPlayerData(NetGetLocalPlayerIndex());
        ch->append_chat_entry(info, te->get_text());
    
        te->set_text("");
    }
    else {
        ch->append_chat_entry(NULL, "There is nobody in the game to hear you yet.");
    }
}

// This is called when the user clicks on a found player to attempt to gather him in.
static void
gather_player_callback(w_found_players* foundPlayersWidget, prospective_joiner_info player) {
        assert(foundPlayersWidget != NULL);
    
	// Either gather will succeed, in which case we don't want to see player, or
	// an error will occur in gathering, in which case we also don't want to see player.
        foundPlayersWidget->hide_player(player);
    
        dialog* theDialog = foundPlayersWidget->get_owning_dialog();

        assert(theDialog != NULL);
        
        int theGatherPlayerResult = NetGatherPlayer(player, reassign_player_colors);

        if(theGatherPlayerResult != kGatherPlayerFailed) {
                w_players_in_game2* thePlayersDisplay =
			dynamic_cast<w_players_in_game2*>(theDialog->get_widget_by_id(iPLAYER_DISPLAY_AREA));

		assert(thePlayersDisplay != NULL);

		thePlayersDisplay->update_display();
                
                if(theGatherPlayerResult == kGatheredUnacceptablePlayer)
                {
                        sGathererMayStartGame= false;
                }
                
                modify_control_enabled(theDialog, iOK, sGathererMayStartGame ? CONTROL_ACTIVE : CONTROL_INACTIVE);
	}
        
        theDialog->draw_dirty_widgets();
}


// This is called when the autogather toggle is twiddled.
static void
autogather_callback(w_select* inAutoGather) {
    if(inAutoGather->get_selection() > 0) {
        dialog* theDialog = inAutoGather->get_owning_dialog();
        assert(theDialog != NULL);
        
        w_found_players* theFoundPlayers = dynamic_cast<w_found_players*>(theDialog->get_widget_by_id(iNETWORK_LIST_BOX));
        assert(theFoundPlayers != NULL);
        
        theFoundPlayers->callback_on_all_items();
    }
}


static void
found_player(dialog* inDialog, prospective_joiner_info &player) {
        assert(inDialog != NULL);
        
        w_found_players* theFoundPlayers = dynamic_cast<w_found_players*>(inDialog->get_widget_by_id(iNETWORK_LIST_BOX));
        assert(theFoundPlayers != NULL);
        
        theFoundPlayers->found_player(player);
        
        w_toggle*	theAutoGather = dynamic_cast<w_toggle*>(inDialog->get_widget_by_id(iAUTO_GATHER));
        assert(theAutoGather != NULL);
        
        if(theAutoGather->get_selection() > 0)
                gather_player_callback(theFoundPlayers, player);
                
        inDialog->draw_dirty_widgets();
}


// This is a callback of sorts; the dialog will invoke it during its idle time.
static void
gather_processing_function(dialog* inDialog) {
	GathererAvailableAnnouncer::pump();

	prospective_joiner_info player;

	if (NetCheckForNewJoiner(player))
		found_player(inDialog, player);
}


#ifndef NETWORK_TEST_POSTGAME_DIALOG // because that test code replaces the real gather box
#ifndef NETWORK_TEST_MICROPHONE_LOCALLY // same deal
bool network_gather(bool inResumingGame)
{
//printf("network_gather\n");

	// Display game setup dialog
	game_info myGameInfo;
	player_info myPlayerInfo;
	if (network_game_setup(&myPlayerInfo, &myGameInfo, inResumingGame)) {
		myPlayerInfo.desired_color = myPlayerInfo.color;
		memcpy(myPlayerInfo.long_serial_number, serial_preferences->long_serial_number, 10);

		if (NetEnter()) {               
                        // ZZZ: gather network game dialog
                        dialog d;
                        
                        d.add(new w_static_text("GATHER NETWORK GAME", TITLE_FONT, TITLE_COLOR));
                    
                        d.add(new w_spacer());
                    
                        d.add(new w_static_text("Players on Network"));
                        
                        w_found_players* foundplayers_w = new w_found_players(320, 4);
                        foundplayers_w->set_identifier(iNETWORK_LIST_BOX);
                        foundplayers_w->set_player_selected_callback(gather_player_callback);
                        d.add(foundplayers_w);
        
                        w_toggle*	autogather_w = new w_toggle("Auto-Gather", network_preferences->autogather);
                        autogather_w->set_identifier(iAUTO_GATHER);
                        autogather_w->set_selection_changed_callback(autogather_callback);
                        d.add(autogather_w);
                        
                        d.add(new w_spacer());
        
                        w_players_in_game2* players_w = new w_players_in_game2(false);
                                                players_w->set_identifier(iPLAYER_DISPLAY_AREA);
                                                d.add(players_w);

#ifdef NETWORK_PREGAME_CHAT

#ifdef	NETWORK_TWO_WAY_CHAT
                        w_chat_history* chat_history_w = new w_chat_history(600, 4);
#else
                        // Signal gatherer that he won't be hearing anything back ;)
                        d.add(new w_static_text("Messages sent to other players"));
                        w_chat_history* chat_history_w = new w_chat_history(600, 3);
#endif // NETWORK_TWO_WAY_CHAT

                        chat_history_w->set_identifier(iCHAT_HISTORY);
                        d.add(chat_history_w);
                        
                        w_text_entry*	chatentry_w = new w_text_entry("Say:", 240, "");
                        chatentry_w->set_identifier(iCHAT_ENTRY);
                        chatentry_w->set_enter_pressed_callback(send_text);
                        chatentry_w->set_alignment(widget::kAlignLeft);
                        chatentry_w->set_full_width();
                        d.add(chatentry_w);
                        
                        d.add(new w_spacer());
#endif // NETWORK_PREGAME_CHAT
                    
                        // "Play" (OK) button starts off disabled.  It's enabled in the gather callback when a player is gathered (see above).
                        // This prevents trying to start a net game by yourself (which doesn't work at the moment, whether it ought to or not).
                        w_left_button* play_button_w = new w_left_button("PLAY", dialog_ok, &d);
                        play_button_w->set_identifier(iOK);
                        play_button_w->set_enabled(false);
                        d.add(play_button_w);
                        
                        d.add(new w_right_button("CANCEL", dialog_cancel, &d));

			GathererAvailableAnnouncer announcer;
                        
                        d.set_processing_function(gather_processing_function);
                        
                        sGathererMayStartGame= true;
                        
                        int theDialogResult;
                    
                        if (NetGather(&myGameInfo, sizeof(game_info), (void *)&myPlayerInfo, sizeof(player_info), inResumingGame)) {
                                                        players_w->start_displaying_actual_information();
                                                        players_w->update_display();
                                theDialogResult = d.run();
                        }
                        else {
                                theDialogResult = -1;	// simulate "cancel" clicked if NetGather failed.
                        }
                  
                        if(theDialogResult == 0) {
                                bool new_autogather_setting = autogather_w->get_selection() ? true : false;
                                if(network_preferences->autogather != new_autogather_setting)
                                {
                                        network_preferences->autogather = new_autogather_setting; 
                                        write_preferences();
                                }
				
				// jkvw: Give network code a chance to deal with prospective joiners we chose not to gather.
				// (right now I'm just dropping connection)
				foundplayers_w->handle_ungathered_players();
				
				NetDoneGathering();
				
                                return true;
                        }
                    
                        NetCancelGather();
                        NetExit();
		}
	}
	return false;
}
#endif // ndef NETWORK_TEST_MICROPHONE_LOCALLY
#endif // ndef NETWORK_TEST_POSTGAME_DIALOG










/*
 *  Joining dialog
 */

static void
join_processing_function(dialog* inDialog) {
	JoinerSeekingGathererAnnouncer::pump();

	// The rest of this taken almost directly from the join_dialog_filter_proc in the Mac version:

	static int last_join_state = NONE;
	
	/* check and see if we've gotten any connection requests */
	int join_state= NetUpdateJoinState();

	switch (join_state)
	{
		case NONE: // haven't Joined yet.
			break;

	case netConnecting:
		case netJoining:
			break;

		case netCancelled: /* the server cancelled the game; force bail */
			assert(inDialog != NULL);
			inDialog->quit(-1);

			break;

		case netWaiting: /* if we just changed netJoining to netWaiting change the dialog text */
			modify_control_enabled(inDialog, iCANCEL, CONTROL_INACTIVE);

			break;

		case netStartingUp: /* the game is starting up (we have the network topography) */
			// dialog is finished (successfully) when game is starting.
			assert(inDialog != NULL);
			inDialog->quit(kNetworkJoinedNewGame);

			break;
                        
                case netStartingResumeGame: // like netStartingUp but we're resuming, not starting anew
                        assert(inDialog != NULL);
                        inDialog->quit(kNetworkJoinedResumeGame);
                        
                        break;

		case netPlayerAdded:
			if(last_join_state==netWaiting)
			{
                // Tell user they got accepted.  Hmm, this should eventually be reworked into
                // something that looks nicer and is more informative (we know the end conditions,
                // options, level name, etc. by now if I'm not mistaken - may as well pass that
                // info along to the user).
                game_info *info= (game_info *)NetGetGameData();

				static char	sStringBuffer[240];

				get_network_joined_message(sStringBuffer, info->net_game_type);

				assert(inDialog != NULL);

				w_static_text* join_status_w =
					dynamic_cast<w_static_text*>(inDialog->get_widget_by_id(iJOIN_MESSAGES));

				assert(join_status_w != NULL);

				join_status_w->set_text(sStringBuffer);
            }

			assert(inDialog != NULL);

			{
				w_players_in_game2*	players_w =
					dynamic_cast<w_players_in_game2*>(inDialog->get_widget_by_id(iPLAYER_DISPLAY_AREA));

				assert(players_w != NULL);

				players_w->start_displaying_actual_information();
				players_w->update_display();
			}

                        inDialog->draw_dirty_widgets();
                        
			break;
                        
                case netChatMessageReceived:
                        player_info*	sending_player;
                        char*		chat_message;
                        
                        if(NetGetMostRecentChatMessage(&sending_player, &chat_message)) {
                            w_chat_history*	ch = dynamic_cast<w_chat_history*>(inDialog->get_widget_by_id(iCHAT_HISTORY));
                            ch->append_chat_entry(sending_player, chat_message);
                            inDialog->draw_dirty_widgets();
                        }
                break;

		case netJoinErrorOccurred:
			assert(inDialog != NULL);
			inDialog->quit(-1);
			
			break;
		
		default:
			assert(false);
	}

	last_join_state = join_state;
}


static void
respond_to_hint_toggle(w_select* inToggle) {
    w_text_entry* theHintAddressEntry = dynamic_cast<w_text_entry*>(inToggle->get_owning_dialog()->get_widget_by_id(iHINT_ADDRESS_ENTRY));
    theHintAddressEntry->set_enabled(inToggle->get_selection() ? true : false);
}


int network_join(void)
{
	if (NetEnter())
	{
                dialog d;
                
                d.add(new w_static_text("JOIN NETWORK GAME", TITLE_FONT, TITLE_COLOR));
                d.add(new w_spacer());
                
                d.add(new w_static_text("Appearance"));
        
                w_text_entry *name_w = new w_text_entry("Name", PREFERENCES_NAME_LENGTH, "");
                name_w->set_identifier(iJOIN_NAME);
                name_w->set_enter_pressed_callback(dialog_try_ok);
                name_w->set_value_changed_callback(dialog_disable_ok_if_empty);
                d.add(name_w);
        
                w_player_color *pcolor_w = new w_player_color("Color", player_preferences->color);
                pcolor_w->set_identifier(iJOIN_COLOR);
                d.add(pcolor_w);
        
                w_player_color *tcolor_w = new w_player_color("Team Color", player_preferences->team);
                tcolor_w->set_identifier(iJOIN_TEAM);
                d.add(tcolor_w);
        
                d.add(new w_spacer());

                w_toggle*	hint_w = new w_toggle("Join by address", network_preferences->join_by_address);
                hint_w->set_identifier(iHINT_TOGGLE);
                hint_w->set_selection_changed_callback(respond_to_hint_toggle);
                d.add(hint_w);

                w_text_entry*	hint_address_w = new w_text_entry("Join address", kJoinHintingAddressLength, network_preferences->join_address);
                hint_address_w->set_identifier(iHINT_ADDRESS_ENTRY);
                if(!hint_w->get_selection())
                        hint_address_w->set_enabled(false);
                d.add(hint_address_w);

                d.add(new w_spacer());

                d.add(new w_static_text(TS_GetCString(strJOIN_DIALOG_MESSAGES, _join_dialog_welcome_string)));

                d.add(new w_spacer());
                
                w_left_button* join_w = new w_left_button("JOIN", dialog_ok, &d);
                join_w->set_identifier(iOK);
                d.add(join_w);
                
                d.add(new w_right_button("CANCEL", dialog_cancel, &d));

                // We do this here since it (indirectly) invokes the respond_to_typing callback, which needs iOK in place.
                copy_pstring_to_text_field(&d, iJOIN_NAME, player_preferences->name);

                if(d.run() == 0) {
                        network_preferences->join_by_address = hint_w->get_selection() ? true : false;
                        if(network_preferences->join_by_address) {
                                strncpy(network_preferences->join_address, hint_address_w->get_text(), kJoinHintingAddressLength);
                                network_preferences->join_address[kJoinHintingAddressLength - 1] = '\0';
                        }
                
                        // Set up the player_info
                        player_info myPlayerInfo;
        
                        copy_pstring_from_text_field(&d, iJOIN_NAME, myPlayerInfo.name);
                        myPlayerInfo.color = static_cast<int16>(pcolor_w->get_selection());
                                                myPlayerInfo.team = static_cast<int16>(tcolor_w->get_selection());
                                                myPlayerInfo.desired_color = myPlayerInfo.color;
                        memcpy(myPlayerInfo.long_serial_number, serial_preferences->long_serial_number, 10);
        
                        bool did_join = NetGameJoin(myPlayerInfo.name, PLAYER_TYPE,
                                                        (void *)&myPlayerInfo, sizeof(player_info), get_network_version(),
                                                        hint_w->get_selection() ? hint_address_w->get_text() : NULL);
                        if (did_join) {
                                // Store user prefs
                                pstrcpy(player_preferences->name, myPlayerInfo.name);
                                player_preferences->team = myPlayerInfo.team;
                                player_preferences->color = myPlayerInfo.color;
                                write_preferences();
        
                                // Join network game 2 box (players in game, chat, etc.)
                                dialog d2;
        
                                d2.add(new w_static_text("WAITING FOR GAME", TITLE_FONT, TITLE_COLOR));
        
                                d2.add(new w_spacer());
        
                                d2.add(new w_static_text("Players in Game"));
        
                                d2.add(new w_spacer());

                                w_players_in_game2* players_w = new w_players_in_game2(false);
                                players_w->set_identifier(iPLAYER_DISPLAY_AREA);
                                d2.add(players_w);

                                d2.add(new w_spacer());
#ifdef	NETWORK_PREGAME_CHAT
#ifdef	NETWORK_TWO_WAY_CHAT                
                                d2.add(new w_static_text("Chat"));
#else
                                d2.add(new w_static_text("Messages from Gatherer"));
#endif // NETWORK_TWO_WAY_CHAT
                                d2.add(new w_spacer());

                                w_chat_history* chat_history_w = new w_chat_history(600, 6);
                                chat_history_w->set_identifier(iCHAT_HISTORY);
                                d2.add(chat_history_w);
#ifdef	NETWORK_TWO_WAY_CHAT
                                w_text_entry*	chatentry_w = new w_text_entry("Say:", 240, "");
                                chatentry_w->set_identifier(iCHAT_ENTRY);
                                chatentry_w->set_enter_pressed_callback(send_text_fake);
                                chatentry_w->set_alignment(widget::kAlignLeft);
                                chatentry_w->set_full_width();
                                d2.add(chatentry_w);
                
#endif // NETWORK_TWO_WAY_CHAT
                                d2.add(new w_spacer());
#endif // NETWORK_PREGAME_CHAT
                                w_static_text*	status_w = new w_static_text(
                                        TS_GetCString(strJOIN_DIALOG_MESSAGES, _join_dialog_waiting_string));
                                status_w->set_identifier(iJOIN_MESSAGES);
                                d2.add(status_w);

                                d2.add(new w_spacer());

                                w_button*	cancel_w = new w_button("CANCEL", dialog_cancel, &d2);
                                cancel_w->set_identifier(iCANCEL);
                                d2.add(cancel_w);

				JoinerSeekingGathererAnnouncer announcer(hint_w->get_selection() == 0);
 
                                d2.set_processing_function(join_processing_function);
        
                                int theDialogResult = d2.run(false /*play intro/exit sounds?  no because our retvalues always sound like cancels*/);
                                // d2.run() returns -1 if player clicked cancel or error occured; kNetworkJoined{New|Resume}Game if accepted into game.
                                if (theDialogResult != -1) {
                                        game_info *myGameInfo = (game_info *)NetGetGameData();
                                        NetSetInitialParameters(myGameInfo->initial_updates_per_packet, myGameInfo->initial_update_latency);
                                        
                                        // We make up for muting the dialog proper...
                                        play_dialog_sound(DIALOG_OK_SOUND);
                
                                        return theDialogResult;
                                }// theDialogResult != -1 (accepted into game)

                        }// did_join == true (call to NetGameJoin() worked)

                }// d.run() == 0 (player wanted to join, not cancel, in first box)

		NetExit();
	} // NetEnter() succeeded

	return false;
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
