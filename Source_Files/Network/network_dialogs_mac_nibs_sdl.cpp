/*
NETWORK_DIALOGS.C  (now network_dialogs_mac_sdl.cpp)

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

Feb 26, 2002 (Br'fin (Jeremy Parsons)):
	Forked off from network_dialogs_macintosh.cpp to tie to SDL networking apis
        
Feb 14, 2003 (Woody Zenfell):
	Support for resuming saved games as network games.
        
July 03, 2003 (jkvw):
        Lua script selection in gather network game dialog

August 27, 2003 (Woody Zenfell):
	Reworked netscript selection stuff to be more cross-platform and more like other dialog code
	
January 27, 2005 (James Willson):
	Forked off from network_dialogs_mac_sdl.cpp; NIBs code updated for firewall-friendly
	netgame dialogs.
*/

#if !defined(DISABLE_NETWORKING)

/*
 *  network_dialogs_mac_nibs_sdl.cpp - NIBS based network dialogs for Carbon with SDL networking
 */
//#define NETWORK_TEST_POSTGAME_DIALOG
//#define ALSO_TEST_PROGRESS_BAR
//#define NETWORK_TEST_GATHER_DIALOG

#if TARGET_API_MAC_CARBON
	#define LIST_BOX_AS_CONTROL 1
#else
	#define LIST_BOX_AS_CONTROL 0
#endif

// add a taunt window for postgame.
// add a message window for gathering
// Don't allow gather on map with no entry points (marathon bites it)
// _overhead_map_is_omniscient is now _burn_items_on_death

#include "cseries.h"
#include "sdl_network.h"

#include "shell.h"  // for preferences
#include "map.h"    // so i can include player.h
#include "player.h" // for displaying damage statistics after network game.
#include "preferences.h"
#include "interface.h" // for _multiplayer_carnage_entry_point
#include "screen_drawing.h"

#include "network_games.h"
#include "network_lookup_sdl.h"
#include "network_metaserver.h"
#include "metaserver_dialogs.h"

// STL Libraries
#include <vector>
#include <algorithm>

// ZZZ: shared dialog item ID constants
#include "network_dialogs.h"

// LP change: outside handler for the default player name
#include "PlayerName.h"

#include "NibsUiHelpers.h"

//#define TEST_NET_STATS_DIALOG  // for testing the dialog when i don't want to play a net game

#ifdef TEST_NET_STATS_DIALOG
//#define TEST_TEAM_DISPLAY_FOR_DIALOG
#endif

#ifdef env68k
#pragma segment network_dialogs
#endif

struct network_speeds
{
	short updates_per_packet;
	short update_latency;
};

/* ---------- constants */

// LP change: get player name from outside
#define PLAYER_TYPE GetPlayerName()

#define MONSTER_TEAM                8

#define fontTOP_LEVEL_FONT        130
#define menuZONES                1002

// ZZZ: Dialog and string constants moved to network_dialogs.h for sharing between SDL and Mac versions.

#define NAME_BOX_HEIGHT   28
#define NAME_BOX_WIDTH   114

#define BOX_SPACING  8

#define OPTION_KEYCODE             0x3a

/* ---------- globals */

/* from screen_drawing.c */
extern TextSpec *_get_font_spec(short font_index);

#define NAME_BEVEL_SIZE    4
static void draw_beveled_text_box(bool inset, Rect *box, short bevel_size, RGBColor *brightest_color, char *text,short flags, bool name_box);

// ZZZ: moved a few static functions to network_dialogs.h so we can share

static void draw_player_box_with_team(Rect *rectangle, short player_index);

static bool key_is_down(short key_code);
#pragma mark -

/* ---------- code */

extern void NetUpdateTopology(void);


static void PlayerDisplayDrawer(ControlRef Ctrl, void *UserData)
{
	// No need for the window context -- it's assumed
	Rect Bounds = {0,0,0,0};
	
	GetControlBounds(Ctrl, &Bounds);
	
	// Draw background and boundary
	ForeColor(whiteColor);
	PaintRect(&Bounds);
	ForeColor(blackColor);
	FrameRect(&Bounds);

	// Cribbed from update_player_list_item()
	FontInfo finfo;
	GetFontInfo(&finfo);
	short height = finfo.ascent + finfo.descent + finfo.leading;
	MoveTo(Bounds.left + 3, Bounds.top+height);
	short num_players = NetNumberOfPlayerIsValid() ? NetGetNumberOfPlayers() : 0;
	
	Rect name_rect;
	SetRect(&name_rect, Bounds.left, Bounds.top, Bounds.left+NAME_BOX_WIDTH, Bounds.top+NAME_BOX_HEIGHT);
	for (short i = 0; i < num_players; i++)
	{
		draw_player_box_with_team(&name_rect, i);
		if (!(i % 2))
		{
			OffsetRect(&name_rect, NAME_BOX_WIDTH+BOX_SPACING, 0);
		}
		else
		{
			OffsetRect(&name_rect, -(NAME_BOX_WIDTH+BOX_SPACING), NAME_BOX_HEIGHT + BOX_SPACING);
		}
	}
	
}

const double PollingInterval = 1.0/30.0;

/*************************************************************************************************
 *
 * NIBs-specific network gather dialog
 *
 *************************************************************************************************/

class NibsGatherDialog : public GatherDialog
{
public:
	NibsGatherDialog::NibsGatherDialog()
	: m_gatherDialogNib(CFSTR("Gather Network Game"))
	, m_dialog_window(m_gatherDialogNib.nibReference(), CFSTR("Gather Network Game"))
	, m_dialog(m_dialog_window(), false)
	
	{
		m_cancelWidget = new ButtonWidget (GetCtrlFromWindow(m_dialog_window(), 0, iCANCEL));
		m_startWidget = new ButtonWidget (GetCtrlFromWindow(m_dialog_window(), 0, iOK));
		
		m_autogatherWidget = new ToggleWidget (GetCtrlFromWindow(m_dialog_window(), 0, iAUTO_GATHER));
	
		m_ungatheredWidget = new JoiningPlayerListWidget (GetCtrlFromWindow(m_dialog_window(), 0, iNETWORK_LIST_BOX), 
								new ButtonWidget (GetCtrlFromWindow(m_dialog_window(), 0, iADD)));
		m_pigWidget = new PlayersInGameWidget (GetCtrlFromWindow(m_dialog_window(), 0, iPLAYER_DISPLAY_AREA));
		
		m_chatEntryWidget = new EditTextWidget (GetCtrlFromWindow(m_dialog_window(), 0, iGATHER_CHAT_ENTRY));
		m_chatChoiceWidget = new SelectorWidget (GetCtrlFromWindow(m_dialog_window(), 0, iGATHER_CHAT_CHOICE));
		m_chatWidget = new HistoricTextboxWidget (new TextboxWidget(m_dialog_window(), 20, 277, 567, 407));
	}

	virtual bool Run ()
	{
		show_cursor(); // Hidden one way or another
	
		AutoTimer Poller(0, PollingInterval, boost::bind(&NibsGatherDialog::idle, this));
		bool result = m_dialog.Run();
		
		AutoDrawability Drawability;
		Drawability(GetCtrlFromWindow(m_dialog_window(), 0, iPLAYER_DISPLAY_AREA), PlayerDisplayDrawer, NULL);
		
		hide_cursor();
		
		return result;
	}
	
	virtual void Stop(bool result)
	{
		m_dialog.Stop(result);
	}

private:
	AutoNibReference m_gatherDialogNib;
	AutoNibWindow m_dialog_window;
	Modal_Dialog m_dialog;
};

auto_ptr<GatherDialog>
GatherDialog::Create()
{
	return auto_ptr<GatherDialog>(new NibsGatherDialog);
}

/*************************************************************************************************
 *
 * NIBs-specific network join dialog
 *
 *************************************************************************************************/

class NibsJoinDialog : public JoinDialog
{
public:
	NibsJoinDialog::NibsJoinDialog()
	: m_joinDialogNib(CFSTR("Join Network Game"))
	, m_dialog_window(m_joinDialogNib.nibReference(), CFSTR("Join Network Game"))
	, m_dialog(m_dialog_window(), false)
	{
		m_cancelWidget = new ButtonWidget (GetCtrlFromWindow(m_dialog_window(), 0, iCANCEL));
		m_joinWidget = new ButtonWidget (GetCtrlFromWindow(m_dialog_window(), 0, iJOIN));
	
		m_joinMetaserverWidget = new ButtonWidget (GetCtrlFromWindow(m_dialog_window(), 0, iJOIN_BY_METASERVER));
		m_joinAddressWidget = new EditTextWidget (GetCtrlFromWindow(m_dialog_window(), 0, iJOIN_BY_HOST_ADDRESS));
		m_joinByAddressWidget = new ToggleWidget (GetCtrlFromWindow(m_dialog_window(), 0, iJOIN_BY_HOST));
	
		m_nameWidget = new EditTextWidget (GetCtrlFromWindow(m_dialog_window(), 0, iJOIN_NAME));
		m_colourWidget = new SelectorWidget (GetCtrlFromWindow(m_dialog_window(), 0, iJOIN_COLOR));
		m_teamWidget = new SelectorWidget (GetCtrlFromWindow(m_dialog_window(), 0, iJOIN_TEAM));
	
		m_messagesWidget = new StaticTextWidget (GetCtrlFromWindow(m_dialog_window(), 0, iJOIN_MESSAGES));
	
		m_pigWidget = new PlayersInGameWidget (GetCtrlFromWindow(m_dialog_window(), 0, iPLAYER_DISPLAY_AREA));
		
		m_chatEntryWidget = new EditTextWidget (GetCtrlFromWindow(m_dialog_window(), 0, iJOIN_CHAT_ENTRY));
		m_chatChoiceWidget = new SelectorWidget (GetCtrlFromWindow(m_dialog_window(), 0, iJOIN_CHAT_CHOICE));
		m_chatWidget = new HistoricTextboxWidget (new TextboxWidget(m_dialog_window(), 23, 272, 626, 404));
	}

	virtual void Run ()
	{
		show_cursor(); // Hidden one way or another
	
		AutoTimer Poller(0, PollingInterval, boost::bind(&NibsJoinDialog::gathererSearch, this));
		m_dialog.Run();
		
		hide_cursor();
	}
	
	virtual void Stop()
	{
		m_dialog.Stop(false);
	}
	
	virtual ~NibsJoinDialog()
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
		delete m_chatEntryWidget;
		delete m_chatChoiceWidget;
		delete m_chatWidget;
	}

private:
	AutoNibReference m_joinDialogNib;
	AutoNibWindow m_dialog_window;
	Modal_Dialog m_dialog;
};

auto_ptr<JoinDialog>
JoinDialog::Create()
{
	return auto_ptr<JoinDialog>(new NibsJoinDialog);
}

/*************************************************************************************************
 *
 * NIBs-specific setup network game dialog
 *
 *************************************************************************************************/

class NibsSetupNetgameDialog : public SetupNetgameDialog
{
public:
	NibsSetupNetgameDialog ()
	: m_setupNetgameDialogNib (CFSTR ("Setup Network Game"))
	, m_dialog_window (m_setupNetgameDialogNib.nibReference (), CFSTR ("Setup Network Game"))
	, m_dialog (m_dialog_window (), false)
	{
		m_cancelWidget = new ButtonWidget (GetCtrlFromWindow(m_dialog_window(), 0, iCANCEL));
		m_okWidget = new ButtonWidget (GetCtrlFromWindow(m_dialog_window(), 0, iOK_SPECIAL));
	
		m_nameWidget = new EditTextWidget (GetCtrlFromWindow (m_dialog_window(), 0, iGATHER_NAME));
		m_colourWidget = new SelectorWidget (GetCtrlFromWindow (m_dialog_window(), 0, iGATHER_COLOR));
		m_teamWidget = new SelectorWidget (GetCtrlFromWindow (m_dialog_window(), 0, iGATHER_TEAM));
	
		m_mapWidget = new FileChooserWidget (GetCtrlFromWindow (m_dialog_window(), 0, iCHOOSE_MAP),
				GetCtrlFromWindow (m_dialog_window(), 0, iTEXT_MAP_NAME), _typecode_scenario, "Choose Map");
		
		m_levelWidget = new SelectorWidget (GetCtrlFromWindow (m_dialog_window(), 0, iENTRY_MENU));
		m_gameTypeWidget = new SelectorWidget (GetCtrlFromWindow (m_dialog_window(), 0, iGAME_TYPE));
		m_difficultyWidget = new SelectorWidget (GetCtrlFromWindow (m_dialog_window(), 0, iDIFFICULTY_MENU));
	
		m_limitTypeWidget = new SelectorWidget (GetCtrlFromWindow (m_dialog_window(), 0, iRADIO_NO_TIME_LIMIT));
		m_timeLimitWidget = new EditNumberWidget (GetCtrlFromWindow (m_dialog_window(), 0, iTIME_LIMIT));
		m_scoreLimitWidget = new EditNumberWidget (GetCtrlFromWindow (m_dialog_window(), 0, iKILL_LIMIT));
	
		m_aliensWidget = new ToggleWidget (GetCtrlFromWindow (m_dialog_window(), 0, iUNLIMITED_MONSTERS));
		m_allowTeamsWidget = new ToggleWidget (GetCtrlFromWindow (m_dialog_window(), 0, iFORCE_UNIQUE_TEAMS));
		m_deadPlayersDropItemsWidget = new ToggleWidget (GetCtrlFromWindow (m_dialog_window(), 0, iBURN_ITEMS_ON_DEATH));
		m_penalizeDeathWidget = new ToggleWidget (GetCtrlFromWindow (m_dialog_window(), 0, iDYING_PUNISHED));
		m_penalizeSuicideWidget = new ToggleWidget (GetCtrlFromWindow (m_dialog_window(), 0, iSUICIDE_PUNISHED));
	
		m_useMetaserverWidget = new ToggleWidget (GetCtrlFromWindow (m_dialog_window(), 0, iADVERTISE_GAME_ON_METASERVER));
	
		m_useScriptWidget = new ToggleWidget (GetCtrlFromWindow (m_dialog_window(), 0, iUSE_SCRIPT));
		m_scriptWidget = new FileChooserWidget (GetCtrlFromWindow (m_dialog_window(), 0, iCHOOSE_SCRIPT),
				GetCtrlFromWindow (m_dialog_window(), 0, iTEXT_SCRIPT_NAME), _typecode_netscript, "Choose Script");
	
		m_allowMicWidget = new ToggleWidget (GetCtrlFromWindow (m_dialog_window(), 0, iREAL_TIME_SOUND));

		m_liveCarnageWidget = new ToggleWidget (GetCtrlFromWindow (m_dialog_window(), 0, iREALTIME_NET_STATS));
		m_motionSensorWidget = new ToggleWidget (GetCtrlFromWindow (m_dialog_window(), 0, iMOTION_SENSOR_DISABLED));
	
		m_zoomWidget = new ToggleWidget (GetCtrlFromWindow (m_dialog_window(), 0, iALLOW_ZOOM));
		m_crosshairWidget = new ToggleWidget (GetCtrlFromWindow (m_dialog_window(), 0, iALLOW_CROSSHAIRS));
		m_laraCroftWidget = new ToggleWidget (GetCtrlFromWindow (m_dialog_window(), 0, iALLOW_LARA_CROFT));
		
		m_useUpnpWidget = new ToggleWidget (GetCtrlFromWindow (m_dialog_window(), 0, iUSE_UPNP));
	}
	
	virtual bool Run ()
	{	
		vector<ControlRef> SNG_panes;
		SNG_panes.push_back (GetCtrlFromWindow (m_dialog_window (), 0, iSNG_GENERAL_TAB));
		SNG_panes.push_back (GetCtrlFromWindow (m_dialog_window (), 0, iSNG_STUFF_TAB));
		AutoTabHandler tab_handler (GetCtrlFromWindow (m_dialog_window (), 0, iSNG_TABS), SNG_panes, m_dialog_window ());
	
		return m_dialog.Run ();
	}

	virtual void Stop (bool result)
	{
		m_dialog.Stop (result);
	}

	virtual bool allLevelsAllowed ()
	{
		return key_is_down (OPTION_KEYCODE);
	}

	virtual void unacceptableInfo ()
	{
		SysBeep (30);
	}

private:
	AutoNibReference m_setupNetgameDialogNib;
	AutoNibWindow m_dialog_window;
	Modal_Dialog m_dialog;
};

auto_ptr<SetupNetgameDialog>
SetupNetgameDialog::Create()
{
	return auto_ptr<SetupNetgameDialog>(new NibsSetupNetgameDialog ());
}



struct EntryPointMenuData
{
	long entry_flags;
	short level_index;
	short default_level;
};


// Cribbed from interface_macintosh.h : LevelNumberMenuBuilder()
static bool EntryPointMenuBuilder(
	int indx, Str255 ItemName, bool &ThisIsInitial, void *Data)
{
	EntryPointMenuData *MDPtr = (EntryPointMenuData *)(Data);
	entry_point entry;
		
	bool UseThis = get_indexed_entry_point(
		&entry, &MDPtr->level_index, MDPtr->entry_flags);
	
	ThisIsInitial = (MDPtr->level_index == MDPtr->default_level);
	
	if (UseThis)
		psprintf(ItemName, "%s",entry.level_name);
	
	return UseThis;
}


void EntryPoints_FillIn(
	DialogPTR dialog,
	long entry_flags,
	short default_level
	)
{
	ControlRef EntryPointCtrl = GetCtrlFromWindow(dialog, 0, iENTRY_MENU);

	EntryPointMenuData MenuData;
	MenuData.entry_flags = entry_flags;
	MenuData.level_index = 0;
	MenuData.default_level = default_level;
	
	BuildMenu(EntryPointCtrl, EntryPointMenuBuilder, &MenuData);
}

void select_entry_point(DialogPtr inDialog, short inItem, int16 inLevelNumber)
{
	modify_selection_control(inDialog, inItem, CONTROL_ACTIVE, inLevelNumber+1);
}

static void calculate_box_colors(
	short color_index,
	RGBColor *highlight_color,
	RGBColor *bar_color,
	RGBColor *shadow_color)
{
	_get_player_color(color_index, highlight_color);

	bar_color->red = (highlight_color->red * 7) / 10;
	bar_color->blue = (highlight_color->blue * 7) / 10;
	bar_color->green = (highlight_color->green * 7) / 10;
	
	shadow_color->red = (highlight_color->red * 2) / 10;
	shadow_color->blue = (highlight_color->blue * 2) / 10;
	shadow_color->green = (highlight_color->green * 2) / 10;
}


// ZZZ: new function used by setup_dialog_for_game_type
void set_limit_text(DialogPTR dialog, short radio_item, short radio_stringset_id, short radio_string_index,
	short units_item, short units_stringset_id, short units_string_index)
{
/*			PLEASE FIX
	Handle item;
	short item_type;
	Rect bounds;

	GetDialogItem(dialog, radio_item, &item_type, &item, &bounds);
	getpstr(ptemporary, radio_stringset_id, radio_string_index);
	SetControlTitle((ControlHandle) item, ptemporary);
	
	GetDialogItem(dialog, units_item, &item_type, &item, &bounds);
	getpstr(ptemporary, units_stringset_id, units_string_index);
	SetDialogItemText(item, ptemporary);
	*/
}

/* For join & gather dialogs. */
static void draw_player_box_with_team(
	Rect *rectangle, 
	short player_index)
{
	player_info *player= (player_info *) NetGetPlayerData(player_index);
	RGBColor highlight_color, bar_color, shadow_color;
	Rect team_badge, color_badge, text_box;
	RGBColor old_color;
	short index;

	/* Save the color */
	GetForeColor(&old_color);

#define TEAM_BADGE_WIDTH 16	
	/* Setup the rectangles.. */
	team_badge= color_badge= *rectangle;
	team_badge.right= team_badge.left+TEAM_BADGE_WIDTH;
	color_badge.left= team_badge.right;

	/* Determine the colors */
	calculate_box_colors(player->team, &highlight_color,
		&bar_color, &shadow_color);

	/* Erase the team badge area. */
	RGBForeColor(&bar_color);
	PaintRect(&team_badge);
	
	/* Draw the highlight for this one. */
	RGBForeColor(&highlight_color);
	for (index = 0; index < NAME_BEVEL_SIZE; index++)
	{
		MoveTo(team_badge.left+index, team_badge.bottom-index);
		LineTo(team_badge.left+index, team_badge.top+index);
		LineTo(team_badge.right, team_badge.top+index);
	}
	
	/* Draw the drop shadow.. */
	RGBForeColor(&shadow_color);
	for (index = 0; index < NAME_BEVEL_SIZE; index++)
	{
		MoveTo(team_badge.left+index, team_badge.bottom-index);
		LineTo(team_badge.right, team_badge.bottom-index);
	}

	/* Now draw the player color. */
	calculate_box_colors(player->color, &highlight_color,
		&bar_color, &shadow_color);

	/* Erase the team badge area. */
	RGBForeColor(&bar_color);
	PaintRect(&color_badge);
	
	/* Draw the highlight for this one. */
	RGBForeColor(&highlight_color);
	for (index = 0; index < NAME_BEVEL_SIZE; index++)
	{
		MoveTo(color_badge.left, color_badge.top+index);
		LineTo(color_badge.right-index, color_badge.top+index);
	}
	
	/* Draw the drop shadow.. */
	RGBForeColor(&shadow_color);
	for (index = 0; index < NAME_BEVEL_SIZE; index++)
	{
		MoveTo(color_badge.left, color_badge.bottom-index);
		LineTo(color_badge.right-index, color_badge.bottom-index);
		LineTo(color_badge.right-index, color_badge.top+index);
	}

	/* Finally, draw the name. */
	text_box= *rectangle;
	InsetRect(&text_box, NAME_BEVEL_SIZE, NAME_BEVEL_SIZE);
	CopyPascalStringToC(player->name, temporary);
	_draw_screen_text(temporary, (screen_rectangle *) &text_box, 
		_center_horizontal|_center_vertical, _net_stats_font, _white_color);		

	/* Restore the color */
	RGBForeColor(&old_color);
}


/* -------------------------- Statics for PostGame Carnage Report (redone) (sorta) */

/* ------------ constants */
#define KILL_BAR_HEIGHT   21
#define DEATH_BAR_HEIGHT  14
#define GRAPH_LEFT_INSET  10
#define GRAPH_RIGHT_INSET 40
#define GRAPH_TOP_INSET   20
#define GRAPH_BAR_SPACING  5
#define RANKING_INSET      7
#define DEATH_BEVEL_SIZE   3
#define KILL_BEVEL_SIZE    4

/* ---------------------- globals */


/* ---------------------- prototypes */
static void draw_beveled_box(bool inset, Rect *box, short bevel_size, RGBColor *brightest_color);
static void calculate_maximum_bar(NetgameOutcomeData &Data, Rect *kill_bar_rect);

/* ---------------- code */

static void SetupStatsSelectionMenu(ControlRef Ctrl)
{	
	MenuRef graph_popup;
	short index;
	short current_graph_selection;
	bool has_scores;

	/* Clear the graph popup */
	graph_popup= GetControlPopupMenuHandle(Ctrl);
	assert(graph_popup);
	while(CountMenuItems(graph_popup))
		DeleteMenuItem(graph_popup, 1);

	/* Setup the player names */
	for (index= 0; index<dynamic_world->player_count; index++)
	{
		struct player_data *player= get_player_data(rankings[index].player_index);
		
		CopyCStringToPascal(player->name, ptemporary);
		AppendMenu(graph_popup, "\p ");
		SetMenuItemText(graph_popup, index+1, ptemporary); // +1 since it is 1's based
	}
	
	/* Add in the separator line */
	AppendMenu(graph_popup, "\p-");

	/* Add in the total carnage.. */
	AppendMenu(graph_popup, getpstr(ptemporary, strNET_STATS_STRINGS, strTOTALS_STRING));
	current_graph_selection= CountMenuItems(graph_popup);
	
	/* Add in the scores */
	has_scores= get_network_score_text_for_postgame(temporary, false);
	if(has_scores)
	{
		Str255 pscore_temp;
		CopyCStringToPascal(temporary, pscore_temp);
		AppendMenu(graph_popup, pscore_temp);
		current_graph_selection= CountMenuItems(graph_popup);
	}
	
	/* If the game has teams, show the team stats. */
	if (!(dynamic_world->game_information.game_options & _force_unique_teams)) 
	{
		/* Separator line */
		if(has_scores) AppendMenu(graph_popup, "\p-");

		AppendMenu(graph_popup, getpstr(ptemporary, strNET_STATS_STRINGS, strTEAM_TOTALS_STRING));

		if(has_scores)
		{
			get_network_score_text_for_postgame(temporary, true);
			Str255 ppostgame;
			CopyCStringToPascal(temporary, ppostgame);
			AppendMenu(graph_popup, ppostgame);
		}
	}
	
	SetControl32BitMaximum(Ctrl, CountMenuItems(graph_popup));
	SetControl32BitValue(Ctrl, current_graph_selection);
}


static void StatsDisplayDrawer(ControlRef Ctrl, void *UserData)
{
	NetgameOutcomeData *DPtr = (NetgameOutcomeData *)(UserData);
	NetgameOutcomeData &Data = *DPtr;

	// Paint the background
	Rect Bounds;
	GetControlBounds(Data.DisplayCtrl, &Bounds);
	ForeColor(whiteColor);
	PaintRect(&Bounds);

	draw_new_graph(Data);
	
	// Paint the boundary
	ForeColor(blackColor);
	FrameRect(&Bounds);
}


static void NetgameOutcome_Handler(ParsedControl& Ctrl, void *UserData)
{
	NetgameOutcomeData *DPtr = (NetgameOutcomeData *)(UserData);
	NetgameOutcomeData &Data = *DPtr;
	
	short MaxID;
	
	switch(Ctrl.ID.signature)
	{
	case StatsDisplay_Player:
		
		/* Find if they clicked in an area.. */
		switch(find_graph_mode(Data, NULL))
		{
		case _player_graph:
		case _total_carnage_graph:
		case _total_scores_graph:
			MaxID= dynamic_world->player_count;
			break;
			
		case _total_team_carnage_graph:
		case _total_team_scores_graph:
		default:
			MaxID= 0; /* Don't let them click in any of these. (what would you do?) */
			break;
		}
		if (Ctrl.ID.id >= 0 && Ctrl.ID.id < MaxID)	
		{
			if (Ctrl.ID.id != (GetControl32BitValue(Data.SelectCtrl)-1))
			{
				SetControl32BitValue(Data.SelectCtrl, Ctrl.ID.id+1);
				Draw1Control(Data.DisplayCtrl);
			}
		}	
		
		break;
	
	case 0:
		switch(Ctrl.ID.id)
		{
		case iGRAPH_POPUP:
			Draw1Control(Data.DisplayCtrl);
			break;
		}
		break;
	}	
}


void display_net_game_stats()
{
	// eat all stray keypresses
	FlushEvents(everyEvent, 0);

	AutoNibReference postgameCarnageReportDialogNib(CFSTR("Postgame Carnage Report"));
	AutoNibWindow Window(postgameCarnageReportDialogNib.nibReference(), CFSTR("Postgame Carnage Report"));
	
	NetgameOutcomeData Data;
	
	Data.SelectCtrl = GetCtrlFromWindow(Window(), 0, iGRAPH_POPUP);
	
	/* Calculate the rankings (once) for the entire graph */
	calculate_rankings(rankings, dynamic_world->player_count);
	qsort(rankings, dynamic_world->player_count, sizeof(struct net_rank), rank_compare);
	
	SetupStatsSelectionMenu(Data.SelectCtrl);
	
	Data.DisplayCtrl = GetCtrlFromWindow(Window(), 0, iDAMAGE_STATS);
	
	AutoDrawability Drawability;
	Drawability(Data.DisplayCtrl, StatsDisplayDrawer, &Data);
	
	// Overall bounds of stats display
	Rect Bounds;
	GetControlBounds(Data.DisplayCtrl, &Bounds);
	
	// Bounds of each player button in that display
	Rect CtrlBounds;
	SetRect(&CtrlBounds, 0, 0, NAME_BOX_WIDTH, NAME_BOX_HEIGHT);
	OffsetRect(&CtrlBounds, Bounds.left+GRAPH_LEFT_INSET, Bounds.top+GRAPH_TOP_INSET);
	
	AutoHittability Hittability;
	for (int k=0; k<MAXIMUM_NUMBER_OF_PLAYERS; k++)
	{
		OSStatus err;
		
		// Create a button for each player
		err = CreateUserPaneControl(
				Window(), &CtrlBounds,
				0, &Data.PlayerButtonCtrls[k]
				);
		vassert(err == noErr, csprintf(temporary,"CreateUserPaneControl error: %d",(int)err));
		
		// Add ID so that the dialog's hit tester can recognize it
		ControlID ID;
		ID.signature = StatsDisplay_Player;
		ID.id = k;
		
		err = SetControlID(Data.PlayerButtonCtrls[k], &ID);
		vassert(err == noErr, csprintf(temporary,"SetControlID error: %d",(int)err));
		
		// Make it hittable -- clicking on it will create an item-hit event
		Hittability(Data.PlayerButtonCtrls[k]);
		
		// It lives in the stats-display area, of course!
		EmbedControl(Data.PlayerButtonCtrls[k], Data.DisplayCtrl);
		
		// Advance to the next position
		OffsetRect(&CtrlBounds, 0, RECTANGLE_HEIGHT(&CtrlBounds)+GRAPH_BAR_SPACING);	
	}
	
	Data.KillsTextCtrl = GetCtrlFromWindow(Window(), 0, iTOTAL_KILLS);
	Data.DeathsTextCtrl = GetCtrlFromWindow(Window(), 0, iTOTAL_DEATHS);
	
	RunModalDialog(Window(), false, NetgameOutcome_Handler, &Data);
}

/* This function takes a rank structure because the rank structure contains the team & is */
/*  sorted.  */
void draw_names(
	NetgameOutcomeData &Data,
	struct net_rank *ranks, 
	short number_of_bars,
	short which_player)
{
	Rect item_rect, name_rect;
	short i;
	RGBColor color;

	SetRect(&name_rect, 0, 0, NAME_BOX_WIDTH, NAME_BOX_HEIGHT);
	GetControlBounds(Data.DisplayCtrl, &item_rect);
	OffsetRect(&name_rect, item_rect.left+GRAPH_LEFT_INSET, item_rect.top+GRAPH_TOP_INSET);
	for (i = 0; i <number_of_bars; i++)
	{
		if (ranks[i].player_index != NONE)
		{
			struct player_data *player= get_player_data(ranks[i].player_index);

			_get_player_color(ranks[i].color, &color);
			draw_beveled_text_box(which_player==i, &name_rect, NAME_BEVEL_SIZE, &color, player->name, 
				_center_horizontal|_center_vertical, true);
		}
		else
		{
			_get_player_color(ranks[i].color, &color);
			draw_beveled_box(false, &name_rect, NAME_BEVEL_SIZE, &color);
		}
		OffsetRect(&name_rect, 0, RECTANGLE_HEIGHT(&name_rect)+GRAPH_BAR_SPACING);
	}
	
	return;
}


void draw_kill_bars(
	NetgameOutcomeData &Data,
	struct net_rank *ranks, 
	short num_players, 
	short suicide_index, 
	bool do_totals, 
	bool friendly_fire)
{
	char kill_string_format[65], death_string_format[65], suicide_string_format[65];
	Rect item_rect, kill_bar_rect, death_bar_rect, suicide_bar_rect;
	short i;
	short max_kills, max_width;
	RGBColor kill_color, suicide_color, death_color;

	get_net_color(_kill_color, &kill_color);
	get_net_color(_suicide_color, &suicide_color);
	get_net_color(_death_color, &death_color);

	getcstr(kill_string_format, strNET_STATS_STRINGS, strKILLS_STRING);
	getcstr(death_string_format, strNET_STATS_STRINGS, strDEATHS_STRING);
	getcstr(suicide_string_format, strNET_STATS_STRINGS, strSUICIDES_STRING);

	GetControlBounds(Data.DisplayCtrl, &item_rect);
	kill_bar_rect.left = item_rect.left + GRAPH_LEFT_INSET + NAME_BOX_WIDTH + GRAPH_BAR_SPACING;
	kill_bar_rect.top = item_rect.top + GRAPH_TOP_INSET;
	kill_bar_rect.bottom = kill_bar_rect.top + KILL_BAR_HEIGHT;
	
	death_bar_rect.left = item_rect.left + GRAPH_LEFT_INSET + NAME_BOX_WIDTH + GRAPH_BAR_SPACING;
	death_bar_rect.top = item_rect.top + GRAPH_TOP_INSET + NAME_BOX_HEIGHT - DEATH_BAR_HEIGHT;
	death_bar_rect.bottom = death_bar_rect.top + DEATH_BAR_HEIGHT;
	
	if (do_totals)
	{
		for (i = 0, max_kills = 0; i < num_players; i++)
		{
			if (ranks[i].kills > max_kills) max_kills = ranks[i].kills;
			if (ranks[i].deaths > max_kills) max_kills = ranks[i].deaths;
		}
	}
	else
	{
		max_kills= calculate_max_kills(num_players);
	}
	max_width = item_rect.right - GRAPH_RIGHT_INSET - kill_bar_rect.left;
	
	for (i = 0; i < num_players; i++)
	{
		if (max_kills)
			kill_bar_rect.right = kill_bar_rect.left + ((ranks[i].kills * max_width) / max_kills);
		else
			kill_bar_rect.right = kill_bar_rect.left;
		if (max_kills)
			death_bar_rect.right = death_bar_rect.left + ((ranks[i].deaths * max_width) / max_kills);
		else
			death_bar_rect.right = death_bar_rect.left;

		if (suicide_index != i)
		{
			Rect              ranking_rect;
			short             diff = ranks[i].kills - ranks[i].deaths;
			screen_rectangle  rr;
			
			if (ranks[i].kills)
			{
				sprintf(temporary, kill_string_format, ranks[i].kills);
				draw_beveled_text_box(false, &kill_bar_rect, KILL_BEVEL_SIZE, &kill_color, temporary, _right_justified|_top_justified, false);
				if (diff > 0)
				{
					ranking_rect = kill_bar_rect;
					ranking_rect.top += KILL_BEVEL_SIZE;
					ranking_rect.bottom -= KILL_BEVEL_SIZE;
				}
				
			}
			if (ranks[i].deaths)
			{
				sprintf(temporary, death_string_format, ranks[i].deaths);
				draw_beveled_text_box(false, &death_bar_rect, DEATH_BEVEL_SIZE, &death_color, temporary, _right_justified|_center_vertical, false);
				if (diff < 0)
				{
					ranking_rect = death_bar_rect;
					ranking_rect.top += DEATH_BEVEL_SIZE;
					ranking_rect.bottom -= DEATH_BEVEL_SIZE;
				}
			}
			if (diff == 0)
			{
				ranking_rect.top = kill_bar_rect.top;
				ranking_rect.bottom = death_bar_rect.bottom;
				ranking_rect.left = kill_bar_rect.left;
				ranking_rect.right = MAX(kill_bar_rect.right, death_bar_rect.right);
			}
			rr.top = ranking_rect.top;
			rr.bottom = ranking_rect.bottom;
			rr.left = ranking_rect.right + RANKING_INSET;
			rr.right = rr.left + 1000; // just make it big enough.
			
			sprintf(temporary, "%+d", diff);
			_draw_screen_text(temporary, &rr, _center_vertical, _net_stats_font, _black_color);
		}
		else
		{
			if (ranks[i].kills)
			{
				SetRect(&suicide_bar_rect, kill_bar_rect.left, kill_bar_rect.top, kill_bar_rect.right , death_bar_rect.bottom);
				sprintf(temporary, suicide_string_format, ranks[i].kills);
				draw_beveled_text_box(false, &suicide_bar_rect, KILL_BEVEL_SIZE, &suicide_color, temporary, _right_justified|_center_vertical, false);
			}
		}
		OffsetRect(&kill_bar_rect, 0, NAME_BOX_HEIGHT + GRAPH_BAR_SPACING);
		OffsetRect(&death_bar_rect, 0, NAME_BOX_HEIGHT + GRAPH_BAR_SPACING);
	}

    // ZZZ: ripped this out into a new function for sharing with SDL version
    update_carnage_summary(Data, ranks, num_players, suicide_index, do_totals, friendly_fire);

	return;
}

static void draw_beveled_text_box(
	bool inset, 
	Rect *box,
	short bevel_size,
	RGBColor *brightest_color,
	char *text,
	short flags,
	bool name_box)
{
	Rect inset_box;
	short color;
	screen_rectangle text_box;
	
	draw_beveled_box(inset, box, bevel_size, brightest_color);

	color = inset || !name_box ? _black_color : _white_color;
	inset_box = *box; 
	InsetRect(&inset_box, bevel_size, bevel_size);
	text_box.top = inset_box.top; text_box.bottom = inset_box.bottom;
	text_box.left = inset_box.left; text_box.right = inset_box.right;
	_draw_screen_text(text, &text_box, flags, _net_stats_font, color);		
}

static void draw_beveled_box(
	bool inset, 
	Rect *box,
	short bevel_size,
	RGBColor *brightest_color)
{
	short i;
	RGBColor second_color, third_color, old_color;
	
	GetForeColor(&old_color);
	
	second_color.red = (brightest_color->red * 7) / 10;
	second_color.blue = (brightest_color->blue * 7) / 10;
	second_color.green = (brightest_color->green * 7) / 10;
	
	third_color.red = (brightest_color->red * 2) / 10;
	third_color.blue = (brightest_color->blue * 2) / 10;
	third_color.green = (brightest_color->green * 2) / 10;
	
	RGBForeColor(&second_color);
	PaintRect(box);
	
	if (RECTANGLE_WIDTH(box) > 2*bevel_size && RECTANGLE_HEIGHT(box) > 2*bevel_size)
	{
		if (inset)
		{
			RGBForeColor(&third_color);
		}
		else
		{
			RGBForeColor(brightest_color);
		}
		
		for (i = 0; i < bevel_size; i++)
		{
			MoveTo(box->left+i, box->top+i);
			LineTo(box->right-i, box->top+i);
			MoveTo(box->left+i, box->top+i);
			LineTo(box->left+i, box->bottom-i);
		}
		
		if (inset)
		{
			RGBForeColor(brightest_color);
		}
		else
		{
			RGBForeColor(&third_color);
		}
		
		for (i = 0; i < bevel_size; i++)
		{
			MoveTo(box->right-i, box->bottom-i);
			LineTo(box->left+i, box->bottom-i);
			MoveTo(box->right-i, box->bottom-i);
			LineTo(box->right-i, box->top+i);
		}
		
	}

	RGBForeColor(&old_color);
	return;
}

static void calculate_maximum_bar(	
	NetgameOutcomeData &Data,
	Rect *kill_bar_rect)
{
	Rect item_rect;

	GetControlBounds(Data.DisplayCtrl, &item_rect);
	kill_bar_rect->left = item_rect.left + GRAPH_LEFT_INSET + NAME_BOX_WIDTH + GRAPH_BAR_SPACING;
	kill_bar_rect->right = item_rect.right - GRAPH_RIGHT_INSET;
	kill_bar_rect->top = item_rect.top + GRAPH_TOP_INSET;
	kill_bar_rect->bottom = kill_bar_rect->top + NAME_BOX_HEIGHT;
}

void draw_score_bars(
	NetgameOutcomeData &Data,
	struct net_rank *ranks, 
	short bar_count)
{
	short index, maximum_width;
	long highest_ranking= LONG_MIN;
	long lowest_ranking= LONG_MAX;
	Rect maximum_bar, bar;
	RGBColor color;
	
	for(index= 0; index<bar_count; ++index)
	{
		if(ranks[index].game_ranking>highest_ranking) highest_ranking= ranks[index].game_ranking;
		if(ranks[index].game_ranking<lowest_ranking) lowest_ranking= ranks[index].game_ranking;
	}

	calculate_maximum_bar(Data, &maximum_bar);
	bar= maximum_bar;
	maximum_width= RECTANGLE_WIDTH(&bar);

	get_net_color(_score_color, &color);

	// Perform adjustments if ranking goes under 0.
	// It likely means a scoring system like golf or for tag, where the person with
	// the least score wins.
	if(lowest_ranking < 0)
	{
		highest_ranking = lowest_ranking;
	}
	
	if(highest_ranking)
	{
		for(index= 0; index<bar_count; ++index)
		{
			/* Get the text. */
			calculate_ranking_text_for_post_game(temporary, ranks[index].game_ranking);
	
			/* Build the bar. */		
			bar.right= bar.left + (ranks[index].game_ranking*maximum_width)/highest_ranking;
	
			/* Draw it! */
			draw_beveled_text_box(false, &bar, NAME_BEVEL_SIZE, &color, temporary, 
				_right_justified|_center_vertical, false);
	
			OffsetRect(&bar, 0, NAME_BOX_HEIGHT + GRAPH_BAR_SPACING);
		}
	}

	/* And clear the text. */
	SetStaticCText(Data.KillsTextCtrl,"");
	SetStaticCText(Data.DeathsTextCtrl,"");
}

/* Stupid function, here as a hack.. */
static bool key_is_down(
	short key_code)
{
	KeyMap key_map;
	
	GetKeys(key_map);
	return ((((byte*)key_map)[key_code>>3] >> (key_code & 7)) & 1);
}


#endif // !defined(DISABLE_NETWORKING)

