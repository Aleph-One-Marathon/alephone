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
#include    "network.h"
#include    "network_private.h" // for JoinerSeekingGathererAnnouncer
#include    "FileHandler.h"
#include    "network_metaserver.h"

#include    "shared_widgets.h"

#include    <string>

#include <map>
#include <set>

#ifdef USES_NIBS
const CFStringRef Window_Network_Distribute = CFSTR("Network_Distribute");
#endif

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
    kNetworkGameTypesStringSetID	= 146,
    kEndConditionTypeStringSetID	= 147,
    kScoreLimitTypeStringSetID		= 148,
    kSingleOrNetworkStringSetID		= 149
};


#ifdef USES_NIBS

enum {
	dlogNET_GAME_STATS= 5000,
	// iGRAPH_POPUP moved from #2 because that is the "Cancel" value
	iDAMAGE_STATS = 3,
	iTOTAL_KILLS,
	iTOTAL_DEATHS,
	iGRAPH_POPUP
};

#else

enum {
	dlogNET_GAME_STATS= 5000,
	iGRAPH_POPUP= 2,
	iDAMAGE_STATS,
	iTOTAL_KILLS,
	iTOTAL_DEATHS
};

#endif

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

/* SDL/TCP hinting info. JTP: moved here from network_dialogs_sdl.cpp */
enum {
    kJoinHintingAddressLength = 64,
    kMaximumRecentAddresses = 16
};

extern bool sUserWantsJoinHinting;
extern char sJoinHintingAddress[];

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
	iPLAYER_LIST_TEXT =9,
	iAUTO_GATHER = 19
};

enum {
	dlogJOIN= 10001,
#ifndef USES_NIBS
	iJOIN= 1,
#else
	iJOIN= 101,
#endif
	// iPLAYER_DISPLAY_AREA = 3,
	iJOIN_NAME= 4,
	iJOIN_TEAM,
	iJOIN_COLOR, 
	iJOIN_MESSAGES,
	// Group line = 12
	// iJOIN_NETWORK_TYPE= 13,
	iJOIN_BY_HOST = 14,
	iJOIN_BY_HOST_LABEL,
	iJOIN_BY_HOST_ADDRESS,
	iJOIN_CHAT_ENTRY,
	iJOIN_CHAT_CHOICE,
	iJOIN_BY_METASERVER = 20
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
	iUSE_SCRIPT= 28,
	iCHOOSE_SCRIPT,
	iTEXT_TIME_LIMIT= 35,
	iMICROPHONE_TYPE,
	iTEXT_SCRIPT_NAME,
	iADVERTISE_GAME_ON_METASERVER = 39,
	iCHOOSE_MAP,
	iTEXT_MAP_NAME,
	iALLOW_ZOOM,
	iALLOW_CROSSHAIRS,
	iALLOW_LARA_CROFT,
	iGATHER_CHAT_ENTRY,
	iGATHER_CHAT_CHOICE,
	iSNG_TABS = 400,
	iSNG_GENERAL_TAB,
	iSNG_STUFF_TAB
};

#ifdef USES_NIBS

// For the nib version -- it has radio-button groups that work like popup menus
const int iRADIO_GROUP_DURATION = iRADIO_NO_TIME_LIMIT;
enum {
	duration_no_time_limit = 1,
	duration_time_limit,
	duration_kill_limit
};

enum {
	mic_type_plain = 1,
	mic_type_speex
};

// Because otherwise it would be interpreted as a regular "OK"
const int iOK_SPECIAL = 101;

// For player-display Data Browser control:
const OSType PlayerDisplay_Name = 'name';

// Signature of player-select buttons in player dialog:
const OSType StatsDisplay_Player = 'plyr';

#endif


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

#ifndef USES_NIBS
typedef DialogPtr NetgameSetupData;
typedef DialogPtr NetgameOutcomeData;
#endif

#ifdef USES_NIBS

struct NetgameSetupData
{
	// All the controls will be here, for convenience
	
	ControlRef PlayerNameCtrl;
	ControlRef PlayerColorCtrl;
	ControlRef PlayerTeamCtrl;
	
	ControlRef GameTypeCtrl;
	ControlRef EntryPointCtrl;
	ControlRef DifficultyCtrl;
	
	ControlRef MonstersCtrl;
	ControlRef NoMotionSensorCtrl;
	ControlRef BadDyingCtrl;
	ControlRef BadSuicideCtrl;
	ControlRef UniqTeamsCtrl;
	ControlRef BurnItemsCtrl;
	ControlRef StatsReportingCtrl;
	
	ControlRef DurationCtrl;
	ControlRef TimeLabelCtrl;
	ControlRef TimeTextCtrl;
	ControlRef KillsLabelCtrl;
	ControlRef KillsTextCtrl;
	
	ControlRef UseMicrophoneCtrl;
	ControlRef MicrophoneTypeCtrl;
	
	ControlRef UseScriptCtrl;
	ControlRef ScriptNameCtrl;

	ControlRef CheatsCtrl;

	ControlRef AdvertiseGameOnMetaserverCtrl;
	
	ControlRef OK_Ctrl;
	
	game_info *game_information;
	bool allow_all_levels;
	
	bool IsOK;	// When quitting
};

struct NetgameOutcomeData
{
	ControlRef SelectCtrl;
	ControlRef DisplayCtrl;
	
	// Invisible, but hittable controls;
	// their drawing is done by the drawing callback for DisplayCtrl
	ControlRef PlayerButtonCtrls[MAXIMUM_NUMBER_OF_PLAYERS];
	
	ControlRef KillsTextCtrl;
	ControlRef DeathsTextCtrl;
};

#endif

/* ---------------------- globals */
extern struct net_rank rankings[MAXIMUM_NUMBER_OF_PLAYERS];


class MetaserverClient;
class GatherDialog : public GatherCallbacks, public ChatCallbacks, public MetaserverClient::NotificationAdapter
{
public:
// Abstract factory; concrete type determined at link-time
	static std::auto_ptr<GatherDialog> Create();
	
	bool GatherNetworkGameByRunning ();
	
	virtual ~GatherDialog ();
	
	// Callbacks for network code; final methods
	virtual void JoinSucceeded(const prospective_joiner_info* player);
	virtual void JoiningPlayerDropped(const prospective_joiner_info* player);
	virtual void JoinedPlayerDropped(const prospective_joiner_info* player);
	virtual void JoinedPlayerChanged(const prospective_joiner_info* player);

	virtual void ReceivedMessageFromPlayer(
		const char *player_name,
		const char *message);

protected:
	GatherDialog();
	
	virtual bool Run() = 0;
	virtual void Stop(bool result) = 0;
	
	void idle ();
	
	void StartGameHit ();
	
	void update_ungathered_widget ();
	
	bool player_search (prospective_joiner_info& player);
	bool gathered_player (const prospective_joiner_info& player);
	
	virtual void playersInRoomChanged() {}
	virtual void gamesInRoomChanged() {}
	virtual void receivedChatMessage(const std::string& senderName, uint32 senderID, const std::string& message);
	virtual void receivedBroadcastMessage(const std::string& message);	
	
	void sendChat ();
	void chatTextEntered (char character);
	void chatChoiceHit ();
	
	map<int, prospective_joiner_info> m_ungathered_players;

	ButtonWidget*			m_cancelWidget;
	ButtonWidget*			m_startWidget;
	
	AutogatherWidget*		m_autogatherWidget;
	
	JoiningPlayerListWidget*	m_ungatheredWidget;
	PlayersInGameWidget*		m_pigWidget;
	
	EditTextWidget*			m_chatEntryWidget;
	SelectorWidget*			m_chatChoiceWidget;
	HistoricTextboxWidget*		m_chatWidget;
	
	enum { kPregameChat = 0, kMetaserverChat };
};


class JoinDialog : public MetaserverClient::NotificationAdapter, public ChatCallbacks
{
public:
	// Abstract factory; concrete type determined at link-time
	static std::auto_ptr<JoinDialog> Create();

	const int JoinNetworkGameByRunning();

	virtual ~JoinDialog ();

protected:
	JoinDialog();
	
	virtual void Run() = 0;
	virtual void Stop() = 0;

	virtual void respondToJoinHit ();

	void gathererSearch ();
	void attemptJoin ();
	void changeColours ();
	void getJoinAddressFromMetaserver ();
	
	virtual void playersInRoomChanged() {}
	virtual void gamesInRoomChanged() {}
	virtual void receivedChatMessage(const std::string& senderName, uint32 senderID, const std::string& message);
	virtual void receivedBroadcastMessage(const std::string& message);

	// ChatCallbacks
	virtual void ReceivedMessageFromPlayer(const char *player_name, const char *message);

	void sendChat ();
	void chatTextEntered (char character);
	void chatChoiceHit ();
	
	ButtonWidget*		m_cancelWidget;
	ButtonWidget*		m_joinWidget;
	
	ButtonWidget*		m_joinMetaserverWidget;
	JoinAddressWidget*	m_joinAddressWidget;
	JoinByAddressWidget*	m_joinByAddressWidget;
	
	NameWidget*		m_nameWidget;
	ColourWidget*		m_colourWidget;
	TeamWidget*		m_teamWidget;
	
	StaticTextWidget*	m_messagesWidget;
	
	PlayersInGameWidget*	m_pigWidget;
	
	EditTextWidget*		m_chatEntryWidget;
	SelectorWidget*		m_chatChoiceWidget;
	HistoricTextboxWidget*	m_chatWidget;
	
	enum { kPregameChat = 0, kMetaserverChat };
	
	JoinerSeekingGathererAnnouncer join_announcer;
	int join_result;
	bool got_gathered;
};

/* ---------------------- new stuff :) */

// Gather Dialog Goodies
// shared routines
bool gather_dialog_player_search (prospective_joiner_info& player);
bool gather_dialog_gathered_player (const prospective_joiner_info& player);
void gather_dialog_initialise (DialogPTR dialog);
void gather_dialog_save_prefs (DialogPTR dialog);
// non-shared routines
class MetaserverClient;
bool run_network_gather_dialog (MetaserverClient* metaserverClient);

GatherCallbacks *get_gather_callbacks();

// Netgame setup goodies
// shared routines
bool network_game_setup(player_info *player_information, game_info *game_information, bool inResumingGame, bool& outAdvertiseGameOnMetaserver);
short netgame_setup_dialog_initialise (DialogPTR dialog, bool allow_all_levels, bool resuming_game);
void netgame_setup_dialog_extract_information(DialogPTR dialog, player_info *player_information,
	game_info *game_information, bool allow_all_levels, bool resuming_game,
	bool &outAdvertiseGameOnMetaserver);
void setup_for_untimed_game(DialogPTR dialog);
void setup_for_timed_game(DialogPTR dialog);
void setup_for_score_limited_game(DialogPTR dialog);
void setup_dialog_for_game_type(DialogPTR dialog, size_t game_type);
void SNG_limit_type_hit (DialogPTR dialog);
void SNG_teams_hit (DialogPTR dialog);
void SNG_game_type_hit (DialogPTR dialog);
void SNG_choose_map_hit (DialogPTR dialog);
void SNG_use_script_hit (DialogPTR dialog);
bool SNG_information_is_acceptable (DialogPTR dialog);
void update_netscript_file_display(DialogPTR dialog);
void set_dialog_netscript_file(DialogPTR dialog, const FileSpecifier& inFile);
const FileSpecifier& get_dialog_netscript_file(DialogPTR dialog);
// non-shared routines
bool run_netgame_setup_dialog(player_info *player_information, game_info *game_information, bool inResumingGame, bool& outAdvertiseGameOnMetaserver);
void update_netscript_file_display(DialogPTR inDialog);
#ifndef SDL // SDL uses its own private mechanism
void EntryPoints_FillIn(DialogPTR dialog, long entry_flags, short default_level);
#endif

/* ---------------------- prototypes */
// And now, some shared routines.

extern void reassign_player_colors(short player_index, short num_players);




// (Postgame Carnage Report routines)
extern short find_graph_mode(NetgameOutcomeData &outcome, short *index);
extern void draw_new_graph(NetgameOutcomeData &outcome);

extern void draw_player_graph(NetgameOutcomeData &outcome, short index);
extern void get_net_color(short index, RGBColor *color);

extern short calculate_max_kills(size_t num_players);
extern void draw_totals_graph(NetgameOutcomeData &outcome);
extern void calculate_rankings(struct net_rank *ranks, short num_players);
extern int rank_compare(void const *rank1, void const *rank2);
extern int team_rank_compare(void const *rank1, void const *ranks2);
extern int score_rank_compare(void const *rank1, void const *ranks2);
extern void draw_team_totals_graph(NetgameOutcomeData &outcome);
extern void draw_total_scores_graph(NetgameOutcomeData &outcome);
extern void draw_team_total_scores_graph(NetgameOutcomeData &outcome);
extern void update_carnage_summary(NetgameOutcomeData &outcome, struct net_rank *ranks,
	short num_players, short suicide_index, bool do_totals, bool friendly_fire);

#ifdef mac
// Mac-only routines called by shared routines.
extern void fill_in_entry_points(DialogPtr dialog, short item, long entry_flags, short default_level);

#else//!mac
// SDL-only routines called by shared routines.
extern void get_selected_entry_point(dialog* inDialog, short inItem, entry_point* outEntryPoint);

#endif//!mac

// Routines
extern void menu_index_to_level_entry(short index, long entry_flags, struct entry_point *entry);
extern int level_index_to_menu_index(int level_index, int32 entry_flags);
extern void select_entry_point(DialogPtr inDialog, short inItem, int16 inLevelNumber);

// ZZZ: new function manipulates radio buttons on Mac; changes w_select widget on SDL.
extern void set_limit_type(DialogPTR dialog, short limit_type);
extern int get_limit_type(DialogPTR dialog);

// ZZZ: new function manipulates radio button title and units ("Point Limit", "points")
extern void set_limit_text(DialogPTR dialog, short radio_item, short radio_stringset_id, short radio_string_index,
                                short units_item, short units_stringset_id, short units_string_index);

extern void set_dialog_netscript_file(DialogPTR inDialog, const FileSpecifier& inFile);
extern const FileSpecifier& get_dialog_netscript_file(DialogPTR inDialog);

// (Postgame carnage report)
extern void draw_names(NetgameOutcomeData &outcome, struct net_rank *ranks,
	short number_of_bars, short which_player);
extern void draw_kill_bars(NetgameOutcomeData &outcome, struct net_rank *ranks, short num_players, 
	short suicide_index, bool do_totals, bool friendly_fire);
extern void draw_score_bars(NetgameOutcomeData &outcome, struct net_rank *ranks, short bar_count);



// For manipulating the list of recent host addresses:

// Sets it to empty, of course
void ResetHostAddresses_Reset();

// Returns whether adding an address could be done
// without knocking an existing address off the list
bool RecentHostAddresses_Add(const char *Address);

// Start iterating over it
void RecentHostAddresses_StartIter();

// Returns the next member in sequence;
// if it ran off the end, then it returns NULL
char *RecentHostAddresses_NextIter();

#endif//NETWORK_DIALOGS_H
