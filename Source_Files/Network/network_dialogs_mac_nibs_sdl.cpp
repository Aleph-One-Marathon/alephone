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

// ZZZ: this is used to communicate between the join game dialog filter proc
// and the join game entry point.  kNetworkJoinFailed is used in this variable
// not to indicate failure, but to indicate that no game has started yet.
static int sStartJoinedGameResult;
static ListHandle network_list_box= (ListHandle) NULL;

/* from screen_drawing.c */
extern TextSpec *_get_font_spec(short font_index);


static pascal void update_player_list_item(DialogPtr dialog, short item_num);
static void found_player(const prospective_joiner_info* player);
static void lost_player(const prospective_joiner_info* player);

#define NAME_BEVEL_SIZE    4
static void draw_beveled_text_box(bool inset, Rect *box, short bevel_size, RGBColor *brightest_color, char *text,short flags, bool name_box);

// ZZZ: moved a few static functions to network_dialogs.h so we can share

static MenuHandle get_popup_menu_handle(DialogPtr dialog, short item);
static void draw_player_box_with_team(Rect *rectangle, short player_index);

static short get_game_duration_radio(DialogPtr dialog);

static bool key_is_down(short key_code);
#pragma mark -

static bool CheckSetupInformation(
	NetgameSetupData& Data, 
	short game_limit_type);

/* ---------- code */

extern void NetUpdateTopology(void);

// JTP: Cribbed from network_dialogs_widgets_sdl, if I can't do it right do it compatibly.
// Actually, as it turns out, there should be a generic STL algorithm that does this, I think.
// Well, w_found_players ought to be using a set<> or similar anyway, much more natural.
// Shrug, this was what I came up with before I knew anything about STL, and I'm too lazy to change it.
template<class T>
static const int
find_item_index_in_vector(const T& inItem, const vector<T>& inVector)
{
	typedef typename std::vector<T>::const_iterator const_iterator;
	const_iterator i(inVector.begin());
	const_iterator end(inVector.end());
	int index	= 0;

	while(i != end) {
		if(*i == inItem)
			return index;
		
		index++;
		i++;
	}
	
	// Didn't find it
	return -1;
}



/*************************************************************************************************
 *
 * Function: network_gather
 * Purpose:  do the game setup and gather dialogs
 *
 *************************************************************************************************/

// Ought to be a property of the player-list control...
NetgameGatherData *GatherDataPtr;

static pascal OSStatus SetPlayerListMember(
		ControlRef Browser,
		DataBrowserItemID ItemID,
		DataBrowserPropertyID PropertyID,
        DataBrowserItemDataRef ItemData,
        Boolean SetValue
		)
{
	NetgameGatherData& Data = *GatherDataPtr;
	
	// Find the player entry
	map<DataBrowserItemID, const prospective_joiner_info*>::iterator Entry = 
		Data.FoundPlayers.find(ItemID);
	
	// Was it really found? if not, quit
	if (Entry == Data.FoundPlayers.end()) return noErr;
	
	const byte *NameBytes = (const byte *)Entry->second->name;
	
	CFStringRef NameStr = CFStringCreateWithBytes(
					NULL, NameBytes+1, NameBytes[0],
					NULL, false);
	
	SetDataBrowserItemDataText(ItemData, NameStr);
	
	CFRelease(NameStr);
	
	return noErr;
}

static UInt32 HowManySelected(ControlRef Browser)
{
	// Count how many selected;
	// from Apple's sample code in TechNote #2009
	// (a valuable source on the Data Browser control in general)
	UInt32 Count = 0;
	GetDataBrowserItemCount(Browser,
		kDataBrowserNoItem ,		/* start searching at the root item */
		true,						/* recursively search all subitems */
		kDataBrowserItemIsSelected,	/* only return selected items */
		&Count);
	
	return Count;
}


static void Fake_ADD_Button_Press();


static pascal void PlayerListMemberHit(
		ControlRef Browser,
		DataBrowserItemID Item,
		DataBrowserItemNotification Message
		)
{
	NetgameGatherData& Data = *GatherDataPtr;

	switch(Message)
	{
	case kDataBrowserItemSelected:
		SetControlActivity(Data.AddCtrl, NetGetNumberOfPlayers() <= MAXIMUM_NUMBER_OF_PLAYERS);
		break;
		
	case kDataBrowserItemDoubleClicked:
		SetControlActivity(Data.AddCtrl, NetGetNumberOfPlayers() <= MAXIMUM_NUMBER_OF_PLAYERS);
		Fake_ADD_Button_Press();
		break;
		
	case kDataBrowserItemDeselected:
	case kDataBrowserItemRemoved:
		SetControlActivity(Data.AddCtrl, HowManySelected(Data.NetworkDisplayCtrl) > 0);
		break;
	}
}


static void PlayerDisplayDrawer(ControlRef Ctrl, void *UserData)
{
	//NetgameGatherData *DPtr = (NetgameGatherData *)(UserData);
	//NetgameGatherData& Data = *DPtr;

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


static void NetgameGather_Handler(ParsedControl& Ctrl, void *UserData)
{
	NetgameGatherData *DPtr = (NetgameGatherData *)(UserData);
	NetgameGatherData& Data = *DPtr;

	// This is only an experiment
	MetaserverClient::pumpAll();
	
	Handle ItemsHdl;
	DataBrowserItemID *ItemsPtr;
	
	switch(Ctrl.ID.id)
	{
	case iADD:
		// Find which players were selected
		
		ItemsHdl = NewHandle(0);
		GetDataBrowserItems(Data.NetworkDisplayCtrl,
			NULL, false, kDataBrowserItemIsSelected,
			ItemsHdl);
		
		int NumItems = GetHandleSize(ItemsHdl)/sizeof(DataBrowserItemID);
		
		HLock(ItemsHdl);
		ItemsPtr = (DataBrowserItemID *)(*ItemsHdl);
		
		for (int k=0; k<NumItems; k++)
		{
			// No need to try if we've gathered enough players
			if (NetGetNumberOfPlayers() >= MAXIMUM_NUMBER_OF_PLAYERS)
				continue;
			
			// Examine each entry in the list
			map<DataBrowserItemID, const prospective_joiner_info*>::iterator Entry = 
				Data.FoundPlayers.find(ItemsPtr[k]);
			
			// Was it really found? if not, then try the next one
			if (Entry != Data.FoundPlayers.end()) {
	
				const prospective_joiner_info *player = Entry->second;
			
				// Remove player from lists
				lost_player(player);
			
				// Gather player
				if (gather_dialog_gathered_player (*player))
					Draw1Control(Data.PlayerDisplayCtrl);
			}
		}
		
		// Note: ADD button is handled in list-box callback
		SetControlActivity(Data.OK_Ctrl, (NetGetNumberOfPlayers() > 1) && Data.AllPlayersOK);
		
		break;	
	}
}


void Fake_ADD_Button_Press()
{
	ParsedControl FakeAddCtrl;
	FakeAddCtrl.ID.id = iADD;

	NetgameGather_Handler(FakeAddCtrl, GatherDataPtr);
}


const double PollingInterval = 1.0/30.0;

static pascal void NetgameGather_Poller(EventLoopTimerRef Timer, void *UserData)
{	
	NetgameGatherData *DPtr = (NetgameGatherData *)(UserData);
	NetgameGatherData& Data = *DPtr;

	MetaserverClient::pumpAll();

	prospective_joiner_info info;
	if (gather_dialog_player_search(info)) {
		prospective_joiner_info* infoPtr = new prospective_joiner_info;
		*infoPtr = info;
		found_player(infoPtr);
	}
	
	if (QQ_get_boolean_control_value (Data.dialog, iAUTO_GATHER)) {
		map<DataBrowserItemID, const prospective_joiner_info*>::iterator it;
		it = Data.FoundPlayers.begin ();
		while (it != Data.FoundPlayers.end () && NetGetNumberOfPlayers() < MAXIMUM_NUMBER_OF_PLAYERS) {
			gather_dialog_gathered_player (*((*it).second));
			lost_player((*(it++)).second);
		}
	}
	
	Draw1Control(Data.PlayerDisplayCtrl);
}

class MacNIBSSDLGatherCallbacks : public GatherCallbacks
{
public:
	~MacNIBSSDLGatherCallbacks() { }
	static MacNIBSSDLGatherCallbacks *instance();
	void JoinSucceeded(const prospective_joiner_info *player);
  void JoiningPlayerDropped(const prospective_joiner_info *player);
  void JoinedPlayerDropped(const prospective_joiner_info *player);
private:
	MacNIBSSDLGatherCallbacks() { }
	static MacNIBSSDLGatherCallbacks *m_instance;
};

MacNIBSSDLGatherCallbacks *MacNIBSSDLGatherCallbacks::m_instance = NULL;

MacNIBSSDLGatherCallbacks *MacNIBSSDLGatherCallbacks::instance() {
	if (!m_instance) {
		m_instance = new MacNIBSSDLGatherCallbacks();
	}
	return m_instance;
}

void MacNIBSSDLGatherCallbacks::JoinSucceeded(const prospective_joiner_info *) {
	if (GatherDataPtr) {
		SetControlActivity(GatherDataPtr->OK_Ctrl, (NetGetNumberOfPlayers() > 1) && GatherDataPtr->AllPlayersOK);
	}
}

void MacNIBSSDLGatherCallbacks::JoiningPlayerDropped(const prospective_joiner_info *player) {
  lost_player(player);
}

void MacNIBSSDLGatherCallbacks::JoinedPlayerDropped(const prospective_joiner_info *) {
  	if (GatherDataPtr) {
		SetControlActivity(GatherDataPtr->OK_Ctrl, (NetGetNumberOfPlayers() > 1) && GatherDataPtr->AllPlayersOK);
	}
}

#ifndef NETWORK_TEST_POSTGAME_DIALOG

GatherCallbacks *get_gather_callbacks() { 
	return static_cast<GatherCallbacks *>(MacNIBSSDLGatherCallbacks::instance());
}

bool run_network_gather_dialog(MetaserverClient*)
{
	bool successful= false;

	show_cursor(); // JTP: Hidden one way or another

	AutoNibReference gatherDialogNib(CFSTR("Gather Network Game"));
	AutoNibWindow Window(gatherDialogNib.nibReference(), CFSTR("Gather Network Game"));
	
	NetgameGatherData Data;
	GatherDataPtr = &Data;

	// Actually a Data Browser control, a sort of super list box introduced in Carbon/OSX
	Data.NetworkDisplayCtrl = GetCtrlFromWindow(Window(), 0, iNETWORK_LIST_BOX);
	
	Data.dialog = Window ();
		
	DataBrowserCallbacks Callbacks;
	obj_clear(Callbacks);	// Makes everything NULL
	Callbacks.version = kDataBrowserLatestCallbacks;
	Callbacks.u.v1.itemDataCallback = NewDataBrowserItemDataUPP(SetPlayerListMember);
	Callbacks.u.v1.itemNotificationCallback = NewDataBrowserItemNotificationUPP(PlayerListMemberHit);
	SetDataBrowserCallbacks(Data.NetworkDisplayCtrl, &Callbacks);
	
	Data.ItemID = 1; 	// Initial values
	Data.AllPlayersOK = true;
	
	Data.PlayerDisplayCtrl = GetCtrlFromWindow(Window(), 0, iPLAYER_DISPLAY_AREA);
	
	AutoDrawability Drawability;
	Drawability(Data.PlayerDisplayCtrl, PlayerDisplayDrawer, &Data);
	
	Data.AddCtrl = GetCtrlFromWindow(Window(), 0, iADD);
	Data.OK_Ctrl = GetCtrlFromWindow(Window(), 0, iOK);
	
	SetControlActivity(Data.AddCtrl,false);
	SetControlActivity(Data.OK_Ctrl,false);

	AutoTimer Poller(0, PollingInterval, NetgameGather_Poller, &Data);
	
	gather_dialog_initialise (Window());
	
	successful = RunModalDialog(Window(), false, NetgameGather_Handler, &Data);
	
	if (successful)
		gather_dialog_save_prefs (Window());
	
	// Free the joiner infos we collected
	set<const prospective_joiner_info*>::iterator it;
	for (it = Data.SeenPlayers.begin(); it != Data.SeenPlayers.end(); ++it)
		delete *it;
	Data.SeenPlayers.clear ();
	
	DisposeDataBrowserItemDataUPP(Callbacks.u.v1.itemDataCallback);
	DisposeDataBrowserItemNotificationUPP(Callbacks.u.v1.itemNotificationCallback);
	
	return successful;
}

#endif //ndef NETWORK_TEST_POSTGAME_DIALOG

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
		m_joinAddressWidget = new JoinAddressWidget (new EditTextWidget (GetCtrlFromWindow(m_dialog_window(), 0, iJOIN_BY_HOST_ADDRESS)));
		m_joinByAddressWidget = new JoinByAddressWidget (new ToggleWidget (GetCtrlFromWindow(m_dialog_window(), 0, iJOIN_BY_HOST)));
	
		m_nameWidget = new NameWidget (new EditTextWidget (GetCtrlFromWindow(m_dialog_window(), 0, iJOIN_NAME)));
		m_colourWidget = new ColourWidget (new SelectorWidget (GetCtrlFromWindow(m_dialog_window(), 0, iJOIN_COLOR)));
		m_teamWidget = new TeamWidget (new SelectorWidget (GetCtrlFromWindow(m_dialog_window(), 0, iJOIN_TEAM)));
		m_colourChangeWidget = new ButtonWidget (GetCtrlFromWindow(m_dialog_window(), 0, iJOIN_CHANGE_COLORS));
	
		m_messagesWidget = new StaticTextWidget (GetCtrlFromWindow(m_dialog_window(), 0, iJOIN_MESSAGES));
	
		m_pigWidget = new PlayersInGameWidget (GetCtrlFromWindow(m_dialog_window(), 0, iPLAYER_DISPLAY_AREA));
	}

	virtual void Run ()
	{
		show_cursor(); // Hidden one way or another
	
		AutoTimer Poller(0, PollingInterval, boost::bind(&JoinDialog::gathererSearch, this));
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
		delete m_colourChangeWidget;
		delete m_messagesWidget;
		delete m_pigWidget;
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

/* ---------- private code */

/*************************************************************************************************
 *
 * Function: network_game_setup
 * Purpose:  handle the dialog to setup a network game.
 *
 *************************************************************************************************/

static bool sng_success;

static pascal OSStatus Setup_PlayerNameWatcher(
	EventHandlerCallRef HandlerCallRef,
	EventRef Event,
	void *UserData
	)
{
	NetgameSetupData *DPtr = (NetgameSetupData *)(UserData);
	NetgameSetupData& Data = *DPtr;
	
	// Hand off to the next event handler
	OSStatus err = CallNextEventHandler(HandlerCallRef, Event);
	
	// Need this order because we want to check the text field
	// after it's been changed, not before.
	// Adjust the OK button's activity as needed
	GetEditPascalText(Data.PlayerNameCtrl, ptemporary);
	SetControlActivity(Data.OK_Ctrl, ptemporary[0] != 0);
	
	return err;
}


static void NetgameSetup_Handler(ParsedControl& Ctrl, void *UserData)
{
	DialogPTR dialog = reinterpret_cast<DialogPTR>(UserData);
	
	switch(Ctrl.ID.id)
	{
	case iRADIO_NO_TIME_LIMIT:
		SNG_limit_type_hit (dialog);
		break;
		
	case iFORCE_UNIQUE_TEAMS:
		SNG_teams_hit (dialog);
		break;

	case iGAME_TYPE:
		SNG_game_type_hit (dialog);
		break;
		
	case iUSE_SCRIPT:
		SNG_use_script_hit (dialog);
		break;
		
	case iCHOOSE_MAP:
		SNG_choose_map_hit (dialog);
		break;
					
	case iOK_SPECIAL:
		if (SNG_information_is_acceptable (dialog)) {
			StopModalDialog(ActiveNonFloatingWindow(),false);
			sng_success = true;
		} else
			SysBeep(30);
	}
}



bool run_netgame_setup_dialog(
	player_info *player_information,
	game_info *game_information,
  bool ResumingGame,
	bool& outAdvertiseGameOnMetaserver)
{
	bool allow_all_levels= key_is_down(OPTION_KEYCODE);

	AutoNibReference setupNetworkGameNib(CFSTR("Setup Network Game"));
	AutoNibWindow Window(setupNetworkGameNib.nibReference(), CFSTR("Setup Network Game"));
		
	AutoKeyboardWatcher Watcher(Setup_PlayerNameWatcher);
	
	Watcher.Watch(GetCtrlFromWindow(Window(), 0, iGATHER_NAME), Window ());
	
	vector<ControlRef> SNG_panes;
	SNG_panes.push_back(GetCtrlFromWindow(Window(), 0, iSNG_GENERAL_TAB));
	SNG_panes.push_back(GetCtrlFromWindow(Window(), 0, iSNG_STUFF_TAB));
	AutoTabHandler tab_handler(GetCtrlFromWindow(Window(), 0, iSNG_TABS), SNG_panes);
	
	game_information->net_game_type =
		netgame_setup_dialog_initialise(Window (), allow_all_levels, ResumingGame);

	sng_success = false;
	RunModalDialog(Window(), false, NetgameSetup_Handler, Window ());
	
	if (sng_success)
	{
		netgame_setup_dialog_extract_information(Window (), player_information,
			game_information, allow_all_levels, ResumingGame, outAdvertiseGameOnMetaserver);
	}
	
	return sng_success;
}



static short get_game_duration_radio(
	DialogPtr dialog)
{
	short items[]= {iRADIO_NO_TIME_LIMIT, iRADIO_TIME_LIMIT, iRADIO_KILL_LIMIT};
	unsigned short index, item_hit;
	
	for(index= 0; index<sizeof(items)/sizeof(items[0]); ++index)
	{
		short item_type;
		ControlHandle control;
		Rect bounds;
	
		GetDialogItem(dialog, items[index], &item_type, (Handle *) &control, &bounds);
		if(GetControlValue(control)) 
		{
			item_hit= items[index];
			break;
		}
	}
	
	assert(index!=sizeof(items)/sizeof(items[0]));
	
	return item_hit;
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



// JTP: Routines initially copied from network_dialogs_sdl.cpp

static void
found_player(const prospective_joiner_info* player)
{
	NetgameGatherData& Data = *GatherDataPtr;

	DataBrowserItemID ItemID = Data.ItemID++;	// Get current value, then move to next one
	Data.FoundPlayers[ItemID] = player;
	Data.ReverseFoundPlayers[player] = ItemID;
	
	Data.SeenPlayers.insert(player);
	
	AddDataBrowserItems(Data.NetworkDisplayCtrl,
		kDataBrowserNoItem,
		1, &ItemID,
		NULL
		);
}


static void
lost_player(const prospective_joiner_info* player)
{
	NetgameGatherData& Data = *GatherDataPtr;

	map<const prospective_joiner_info*, DataBrowserItemID>::iterator Entry =
		Data.ReverseFoundPlayers.find(player);
	
	// Could the player entry be found?
	if (Entry == Data.ReverseFoundPlayers.end()) return;

	DataBrowserItemID ItemID = Entry->second;
	Data.FoundPlayers.erase(ItemID);
	Data.ReverseFoundPlayers.erase(Entry);
	
	RemoveDataBrowserItems(Data.NetworkDisplayCtrl,
		kDataBrowserNoItem,
		1, &ItemID,
		NULL
		);
}

/* jkvw: unused function

static void
player_name_changed_callback(const SSLP_ServiceInstance* player)
{
	NetgameGatherData& Data = *GatherDataPtr;
	
	map<const SSLP_ServiceInstance*, DataBrowserItemID>::iterator Entry =
		Data.ReverseFoundPlayers.find(player);

	// Could the player entry be found?
	if (Entry == Data.ReverseFoundPlayers.end()) return;
	
	DataBrowserItemID ItemID = Entry->second;
		
	UpdateDataBrowserItems(Data.NetworkDisplayCtrl,
		kDataBrowserNoItem,
		1, &ItemID,
		NULL, NULL
		);
}
*/



/*************************************************************************************************
 *
 * Function: update_player_list_item
 * Purpose:
 *
 *************************************************************************************************/
static pascal void update_player_list_item(
	DialogPtr dialog, 
	short item_num)
{
	Rect         item_rect, name_rect;
	short        i, num_players;
	short        item_type;
	short        height;
	Handle       item_handle;
	GrafPtr      old_port;
	FontInfo     finfo;

// LP: the Classic version I've kept theme-less for simplicity
#if TARGET_API_MAC_CARBON
	ThemeDrawingState savedState;
	ThemeDrawState curState =
		IsWindowActive(GetDialogWindow(dialog))?kThemeStateActive:kThemeStateInactive;
#endif

	GetPort(&old_port);
	SetPort(GetWindowPort(GetDialogWindow(dialog)));
	
#if TARGET_API_MAC_CARBON
	GetThemeDrawingState(&savedState);
#endif
	
	GetDialogItem(dialog, item_num, &item_type, &item_handle, &item_rect);
	
#if TARGET_API_MAC_CARBON
	DrawThemePrimaryGroup (&item_rect, curState);
#endif
	
	GetFontInfo(&finfo);
	height = finfo.ascent + finfo.descent + finfo.leading;
	MoveTo(item_rect.left + 3, item_rect.top+height);
	num_players = NetNumberOfPlayerIsValid() ? NetGetNumberOfPlayers() : 0;
	SetRect(&name_rect, item_rect.left, item_rect.top, item_rect.left+NAME_BOX_WIDTH, item_rect.top+NAME_BOX_HEIGHT);
	for (i = 0; i < num_players; i++)
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
	
#if TARGET_API_MAC_CARBON
	SetThemeDrawingState(savedState, true);
#endif
	
	SetPort(old_port);
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



static MenuHandle get_popup_menu_handle(
	DialogPtr dialog,
	short item)
{
	MenuHandle menu;
	short item_type;
	ControlHandle control;
	Rect bounds;

	/* Add the maps.. */
	GetDialogItem(dialog, item, &item_type, (Handle *) &control, &bounds);

	menu= GetControlPopupMenuHandle(control);
	assert(menu);

	return menu;
}



#ifdef TEST_NET_STATS_DIALOG
static void fake_initialize_stat_data(void)
{
	short i, j;
	
	for (i = 0; i < MAXIMUM_NUMBER_OF_PLAYERS; i++)
	{
		(players+i)->monster_damage_taken.damage = abs(Random()%200);
		(players+i)->monster_damage_taken.kills = abs(Random()%30);
		(players+i)->monster_damage_given.damage = abs(Random()%200);
		(players+i)->monster_damage_given.kills = abs(Random()%30);
		
		(players+i)->netgame_parameters[0] = abs(Random()%5000000);
		(players+i)->netgame_parameters[1] = abs(Random()%6);
		
		for (j = 0; j < MAXIMUM_NUMBER_OF_PLAYERS; j++)
		{
			(players+i)->damage_taken[j].damage = abs(Random()%200);
			(players+i)->damage_taken[j].kills = abs(Random()%6);
		}
	}
}
#endif

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
static pascal Boolean display_net_stats_proc(DialogPtr dialog, EventRecord *event, short *item_hit);
static void update_damage_item(WindowPtr dialog);
static pascal void update_damage_item_proc(DialogPtr dialog, short item_num);
static short create_graph_popup_menu(DialogPtr dialog, short item);
static void draw_beveled_box(bool inset, Rect *box, short bevel_size, RGBColor *brightest_color);
static void calculate_maximum_bar(NetgameOutcomeData &Data, Rect *kill_bar_rect);
static bool will_new_mode_reorder_dialog(short new_mode, short previous_mode);

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
		vassert(err == noErr, csprintf(temporary,"CreateUserPaneControl error: %d",err));
		
		// Add ID so that the dialog's hit tester can recognize it
		ControlID ID;
		ID.signature = StatsDisplay_Player;
		ID.id = k;
		
		err = SetControlID(Data.PlayerButtonCtrls[k], &ID);
		vassert(err == noErr, csprintf(temporary,"SetControlID error: %d",err));
		
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
	short item_type, i;
	Handle item_handle;
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
	short item_type, max_kills, max_width;
	Handle item_handle;
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
	short item_type;
	Handle item_handle;
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
	short item_type;
	Handle item_handle;
	
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

static bool will_new_mode_reorder_dialog(
	short new_mode,
	short previous_mode)
{
	bool may_reorder= false;
		
	switch(new_mode)
	{
		case _player_graph:
			switch(previous_mode)
			{
				case _player_graph:
				case _total_carnage_graph:
					may_reorder= false; 
					break;
					
				case _total_scores_graph:
				case _total_team_carnage_graph:
				case _total_team_scores_graph:
					may_reorder= true;
					break;
			}
			break;
		
		case _total_carnage_graph:
			switch(previous_mode)
			{
				case _player_graph:
				case _total_carnage_graph:
					may_reorder= false; 
					break;
					
				case _total_scores_graph:
				case _total_team_carnage_graph:
				case _total_team_scores_graph:
					may_reorder= true;
					break;
			}
			break;
			
		case _total_scores_graph:
			switch(previous_mode)
			{
				case _total_scores_graph:
					may_reorder= false; 
					break;
					
				case _total_carnage_graph:
				case _player_graph:
				case _total_team_carnage_graph:
				case _total_team_scores_graph:
					may_reorder= true;
					break;
			}
			break;
	
		case _total_team_carnage_graph:
			switch(previous_mode)
			{
				case _total_team_carnage_graph:
					may_reorder= false;
					break;
	
				case _player_graph:
				case _total_carnage_graph:
				case _total_scores_graph:
				case _total_team_scores_graph:
					may_reorder= true; 
					break;
			}
			break;
			
		case _total_team_scores_graph:
			switch(previous_mode)
			{
				case _total_team_scores_graph:
					may_reorder= false;
					break;
	
				case _player_graph:
				case _total_carnage_graph:
				case _total_scores_graph:
				case _total_team_carnage_graph:
					may_reorder= true; 
					break;
			}
			break;
			
		default:
			// LP change:
			assert(false);
			// halt();
			break;
	}
	
	return may_reorder;
}

/* Stupid function, here as a hack.. */
static bool key_is_down(
	short key_code)
{
	KeyMap key_map;
	
	GetKeys(key_map);
	return ((((byte*)key_map)[key_code>>3] >> (key_code & 7)) & 1);
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
	"<<<-ZED-<<<"
};

// THIS ONE IS FAKE - used to test postgame report dialog without going through a game.
bool network_gather(bool inResumingGame) {
	short i, j;
	player_info thePlayerInfo;
	game_info   theGameInfo;

	show_cursor(); // JTP: Hidden one way or another
	if(network_game_setup(&thePlayerInfo, &theGameInfo, false)) {

	// Test the progress bar while we're at it
	#ifdef ALSO_TEST_PROGRESS_BAR
	#include "progress.h"
	open_progress_dialog(_distribute_physics_single);
	for(i=0; i < 100; i++)
	{
		//nanosleep(&(timespec){0,50000000},NULL);
		//idle_progress_bar();
		int TC = TickCount();	// Busy-waiting; dumb
		while (TickCount() < TC+4);
		draw_progress_bar(i, 100);
	}
	set_progress_dialog_message(_distribute_map_single);
	reset_progress_bar();
	for(i=0; i < 100; i++)
	{
		//nanosleep(&(timespec){0,90000000},NULL);
		int TC = TickCount();	// Busy-waiting; dumb
		while (TickCount() < TC+4);
		draw_progress_bar(i, 100);
	}
	close_progress_dialog();
	#endif
	
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
	hide_cursor();
	return false;
}
#endif

#endif // !defined(DISABLE_NETWORKING)

