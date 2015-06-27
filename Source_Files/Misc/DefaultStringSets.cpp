/*  DefaultStringSets.cpp - support for compiled-in text strings

	Copyright (C) 2002 and beyond by the "Aleph One" developers.
 
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

	Default text StringSets, in case no MML provides them.
	by Woody Zenfell, III
	June 3, 2002

    June 3, 2002: captured text_strings.mml into C++ form for this file.
*/

#include    "cseries.h"
#include    "DefaultStringSets.h"
#include    "TextStrings.h"


static inline void BuildStringSet(short inStringSetID,
                                  const char** inStrings,
                                  short inNumStrings) {
	for (int i = 0; i < inNumStrings; i++)
		TS_PutCString(inStringSetID, i, inStrings[i]);
}

#define NUMBER_OF_STRINGS(sa)   (sizeof(sa) / sizeof((sa)[0]))

#define BUILD_STRINGSET(id, strs)   BuildStringSet(id, strs, NUMBER_OF_STRINGS(strs))



// StringSets from original Bungie resources
// -----------------------------------------------------------------------------------------
// <!-- STR# Resource: "Errors" -->
static const char* sStringSetNumber128[] = {
    "Sorry, $appName$ requires a 68040 processor or higher.",
    "Sorry, $appName$ requires Color QuickDraw.",
    "Sorry, $appName$ requires System Software 7.0 or higher.",
    "Sorry, $appName$ requires at least 3000k of free RAM.",
    "Sorry, $appName$ requires a 13\" monitor (640x480) or larger which can be set to at least 256 colors or grays.",
    "Please be sure the files 'Map', 'Shapes', 'Images' and 'Sounds' are correctly installed and try again.",
    "$appName$ couldn't initialize the sound.",
    "$appName$ has encountered a file system error.  Check to make sure you have enough disk space and that you are not trying to save to a locked volume.",
    "This copy of $appName$ has been modified, perhaps by a virus.  Please re-install it from the original disks.",
    "This beta copy of $appName$ has expired and is no longer functional. Call Bungie at (312) 563-6200 ext. 21 for more information.",
    "Sorry, that key is already used to adjust the sound volume.",
    "Sorry, that key is already used for zooming in the overhead map view.",
    "Sorry, that key is already used for scrolling the inventory.",
    "Sorry, that key is already used for a special game function.",
    "$appName$ has used up all available RAM and cannot continue.  Trying giving it more memory, switching to a lower bit-depth or turning off sounds and try again.",
    "$appName$ is about to load an external physics model.  This could result in erratic performance, inexplicable crashes, corrupted saved games, and inconsistent network games (just a warning).",
    "$appName$ is using maps which were not created with Bungie tools.  This could result in poor performance, inexplicable crashes and corrupted saved games (proceed at your own risk).",
    "A game related error occurred while attempting to read from your map or saved game file.",
    "A system error occurred while attempting to read from your map or saved game file.",
    "An error occurred while saving your game (maybe your hard drive is full, or you tried to save to a locked volume?)",
    "That serial number is invalid.  Please try again.",
    "Sorry, you can't start a network game with more than one copy of the same serial number.  Call 1-800-295-0060 to order more network serial numbers by fax.",
    "Sorry, this copy of $appName$ has been serialized with a network-only serial number.  You cannot play the single-player game with a network-only serial number.",
    "The map you are trying to load has been corrupted.  Please reinstall from a backup copy.",
    "Checkpoint %d was not found!",
    "Picture %d was not found!",
    "This preview copy of $appName$ does not support networking.  A full demo will be available on-line or from Bungie shortly which includes networking (and a whole lot of other cool features).",
    "The gathering computer has quit the game, leaving everyone stranded without the next level.  Perhaps you should tar and feather him.",
    "Sorry, $appName$ was unable to gracefully exit from the network game.  As a result, your romp through the levels has been prematurely halted.",
    "The scenario file that this saved game was from cannot be found.  When you switch levels you will revert to the default map.",
    "$appName$ was unable to find the map that this film was recorded on, so the film cannot be replayed.",
    "Sorry, $appName$ needs 6000k free to play in a networked game.  Give $appName$ more memory and try again.",
    "There appears to be a script conflict.  Perhaps mml and netscript are having differences over who gets to control lua.  Don't be surprised if you get unexpected script behavior or out of sync.",
    "This replay was created with a newer version of $appName$ and cannot be played with this version. Upgrade $appName$ and try again.",
    "Sorry, the scroll wheel can only be used for switching weapons.",
};

// STR# Resource: "Filenames"
static const char* sStringSetNumber129[] = {
    "Shapes",
    "Shapes.16",
    "Sounds",
    "Sounds.16",
    "$appName$ Preferences",
    "Map",
    "Untitled Game",
    "Marathon",
    "$appName$ Recording",
    "Physics Model",
    "Music",
    "Images",
    "Movie",
    "Default",
    "Marathon.appl",
};

// STR# Resource: "Top-Level Interface Items"
static const char* sStringSetNumber130[] = {
    "BEGIN GAME",
    "OPEN SAVED GAME",
    "",
    "REPLAY LAST GAME",
    "REPLAY SAVED RECORDING",
    "SAVE LAST RECORDING",
    "",
    "GATHER NETWORK GAME",
    "JOIN NETWORK GAME",
    "",
    "PREFERENCES",
    "START DEMO",
    "QUIT",
};

// STR# Resource: "Prompts"
static const char* sStringSetNumber131[] = {
    "SAVE GAME",
    "SAVE RECORDING",
    "Select recording as:",
    "Default",
};

// STR# Resource: "Network Errors"
static const char* sStringSetNumber132[] = {
    "Sorry, that player could not be found on the network.  He may have cancelled his Join Game dialog.",

    "One or more of the players in the game could not be found to receive the "
    "map.  The game has been canceled.",

    "The gatherering computer is not responding."
    " It may be behind a firewall, or you may have mistyped the address.",

    "Sorry, the gathering computer has cancelled the game (you should all gang up on him next game).",

    "The map was not received in its entirety, so the game has been canceled.",

    "The gathering computer never sent the map, so the game has been cancelled.  Maybe one of the other machines in the game went down.",
    "$appName$ was unable to start the game.  Maybe one of the other machines in the game went down.",
    "An error ocurred while trying to join a game (an incompatible version of $appName$ may have tried to gather you).  Try again.",
    "Sorry, a network error ocurred and $appName$ is unable to continue.",

    "An error occurred while trying to join a game (the gatherer is using an incompatible version).",

    "The player you just added is using an older version of $appName$ that does not support some advanced features required by the game you're trying to gather.  You will not be allowed to start the game.",
    "The player you just attempted to add is using a version of $appName$ that does not support some advanced features required by the game you're trying to gather.",
    "$appName$ was unable to locate the Map file this level came from.  Some terminals may not display properly, and saving this game on this computer is not recommended.",
    "The connection to the gatherer was lost.",

    "Unable to look up the gatherer. Maybe you typed the address in wrong.",

    "An error occurred receiving the map from the server. Game over.",

    "The gatherer is using the star protocol, but your configuration is set to ring. You will not appear in the list of available players.",

    "The gatherer is using the ring protocol, but your configuration is set to star. You will not appear in the list of available players.",

    "The gatherer is using a Lua netscript, but this version was built without Lua support. You will not appear in the list of available players.",
	
	"There was a problem connecting to the server that tracks Internet games.  Please try again later.",
	
	"Your game could not be advertised on the Internet.  Please distribute your public IP address by other means or try again later.",
	
	"$appName$ failed to configure the firewall/router. You may be unable to gather."
	
};

// STR# Resource: "Key Codes To Names"
static const char* sStringSetNumber133[] = {
    "A",
    "S",
    "D",
    "F",
    "H",
    "G",
    "Z",
    "X",
    "C",
    "V",
    "0x0A",
    "B",
    "Q",
    "W",
    "E",
    "R",
    "Y",
    "T",
    "1",
    "2",
    "3",
    "4",
    "6",
    "5",
    "=",
    "9",
    "7",
    "-",
    "8",
    "0",
    "]",
    "O",
    "U",
    "[",
    "I",
    "P",
    "Return",
    "L",
    "J",
    "'",
    "K",
    ";",
    "\\",
    ",",
    "/",
    "N",
    "M",
    ".",
    "Tab",
    "Space",
    "`",
    "Delete",
    "0x34",
    "Escape",
    "0x35",
    "Command",
    "Shift",
    "Caps Lock",
    "Option",
    "Control",
    "0x3c",
    "0x3d",
    "0x3e",
    "0x3f",
    "0x40",
    "Keypad .",
    "0x42",
    "Keypad *",
    "0x44",
    "Keypad +",
    "0x46",
    "Clear",
    "0x48",
    "0x49",
    "0x4a",
    "Keypad /",
    "Enter",
    "0x4d",
    "Keypad -",
    "0x4f",
    "0x50",
    "Keypad =",
    "Keypad 0",
    "Keypad 1",
    "Keypad 2",
    "Keypad 3",
    "Keypad 4",
    "Keypad 5",
    "Keypad 6",
    "Keypad 7",
    "0x5a",
    "Keypad 8",
    "Keypad 9",
    "0x5d",
    "0x5e",
    "0x5f",
    "F5",
    "F6",
    "F7",
    "F3",
    "F8",
    "F9",
    "0x66",
    "F11",
    "0x68",
    "F13",
    "0x6a",
    "F14",
    "0x6c",
    "F10",
    "0x6e",
    "F12",
    "0x70",
    "F15",
    "Help",
    "Home",
    "Page Up",
    "Forw. Del.",
    "F4",
    "End",
    "F2",
    "Page Down",
    "F1",
    "Left Arrow",
    "Right Arrow",
    "Down Arrow",
    "Up Arrow",
    "Power",
};

// STR# Resource: "Preferences Advice"
static const char* sStringSetNumber134[] = {
    "Be sure your external speakers or headphones are connected properly, and that you have enabled stereo output from the Sound Control Panel.",
    "Be sure that your Cybermaxx helmet is properly hooked up to the serial port and turned on.",
    "Please check to be sure you have the file \"QuickTime[TM] Musical Instruments\" in your \"Extensions\" folder, because $appName$'s background music will sound really, really stupid without it.",
};

// STR# Resource: "Computer Interface"
static const char* sStringSetNumber135[] = {
    "U.E.S.C. Marathon",
    "Opening Connection to b.4.5-23",
    "CAS.qterm//CyberAcme Systems Inc.",
    "<931.461.60231.14.vt920>",
    "UESCTerm 802.11 (remote override)",
    "PgUp/PgDown/Arrows To Scroll",
    "Return/Enter To Acknowledge",
    "Disconnecting...",
    "Connection Terminated.",
    "%H%M %m.%d.%Y",
};

// STR# Resource: "Join Dialog Messages"
static const char* sStringSetNumber136[] = {
    "Click 'Join' to wait for an invitation into a network game of $appName$.",
    "Now waiting to be gathered into a network game by a server.  Click 'Cancel' to give up.",
    "You have been accepted into a game.  Now waiting for the server to add the remaining players... ",
};

// STR# Resource: "Weapon Names"
static const char* sStringSetNumber137[] = {
    "FISTS",
    ".44 MAGNUM MEGA CLASS A1",
    "ZEUS-CLASS FUSION PISTOL",
    "MA-75B ASSAULT RIFLE/ GRENADE LAUNCHER",
    "SPNKR-X18 SSM LAUNCHER",
    "TOZT-7 BACKPACK NAPALM UNIT",
    "UNKNOWN WEAPON CLASS system error 0xfded",
    "WSTE-M COMBAT SHOTGUN",
    "(somehow related to time of applicability)",
    "KKV-7 10MM FLECHETTE SMG",
};

// STR# Resource: "file search path"
static const char* sStringSetNumber138[] = {
    "Marathon Trilogy:Marathon Infinity \xc4:Marathon Infinity:",
};

// STR# Resource: "Preferences Groupings"
static const char* sStringSetNumber139[] = {
    "Graphics",
    "Player",
    "Sound",
    "Controls",
    "Environment",
};

// STR# Resource: "Postgame network game stats"
static const char* sStringSetNumber140[] = {
    "%d flags",
    "%d:%02d",
    "%d points",
    "Team",
    "Time With Ball",
    "Flags Captured",
    "Time 'It'",
    "Goals",
    "Time On Hill",
    "Time On Hill",
    "Points",
    "Time",
};

// STR# Resource: "Net Game Setup"
static const char* sStringSetNumber141[] = {
    "Kill Limit",
    "kills",
    "Capture Limit",
    "flags",
    "Point Limit",
    "points",
    "Time On Hill:",
    "minutes",
    "Points",
    "Time",
};

// STR# Resource: "New Join Dialog Messages"
static const char* sStringSetNumber142[] = {
    "You have been accepted into a game of '%s'.  Now waiting for the server to add the remaining players...",
    "Every Man For Himself",
    "You have been accepted into a cooperative game.  Now waiting for the server to add the remaining players...",
    "Capture the Flag",
    "King of the Hill",
    "Kill the Man With the Ball",
    "Defense",
    "Rugby",
    "Tag",
    "You have been accepted into a custom game. Now waiting for the server to add the remaining players...",
};

// STR# Resource: "Progress strings for networking"
static const char* sStringSetNumber143[] = {
    "Sending map to remote player.",
    "Sending map to remote players.",
    "Receiving map from server.",
    "Waiting for server to send map.",
    "Sending environment information to the other player.",
    "Sending environment information to the other players.",
    "Receiving environment information from server.",
    "Loading...",
    "Attempting to open router ports...",
    "Closing router ports...",
    "Checking for updates..."
};


// Stringsets for SDL w_select widgets.
// -----------------------------------------------------------------------------------------
#include    "network_dialogs.h"  // for stringset ID's

static const char*    sDifficultyLevelsStrings[] = {
    "Kindergarten",
    "Easy",
    "Normal",
    "Major Damage",
    "Total Carnage"
};

static const char*    sNetworkGameTypesStrings[] = {
    "Every Man for Himself",
    "Cooperative Play",
    "Capture the Flag",
    "King of the Hill",
    "Kill the Man With the Ball",
    "Rugby",
    "Tag",
    "Netscript",
};

static const char*    sEndConditionTypeStrings[] = {
//    "No Limit (Alt+Q to quit)",
	"Unlimited",
    "Time Limit",
    "Score Limit",
};

static const char*	sSingleOrNetworkStrings[] = {
    "Single-player game",
    "Network game"
};

// More Mac OS string-resource stringsets.
// -----------------------------------------------------------------------------------------
// STR# Resource: "Item names"
static const char* sStringSetNumber150[] = {
    "FISTS",
    ".44 MAGNUM MEGA CLASS",
    ".44 MAGNUM MEGA CLASS",
    ".44 CLIP (x8)",
    ".44 CLIPS (x8)",
    "ZEUS-CLASS FUSION PISTOL",
    "FUSION BATTERY",
    "FUSION BATTERIES",
    "MA-75B ASSAULT RIFLE",
    "MA-75B CLIP (x52)",
    "MA-75B CLIPS (x52)",
    "MA-75B GRENADES (x7)",
    "MA-75B GRENADES (x7)",
    "SPNKR-X18 SSM LAUNCHER",
    "SSM MISSILE (x2)",
    "SSM MISSILES (x2)",
    "ALIEN WEAPON",
    "SHOTGUN SHELL (x2)",
    "SHOTGUN SHELLS (x2)",
    "TOZT-7 NAPALM UNIT",
    "NAPALM CANISTER",
    "NAPALM CANISTERS",
    "POWERPC 620 CHIP",
    "POWERPC 620 CHIPS",
    "ALIEN ENERGY CONVERTER",
    "WAVE MOTION CANNON",
    "THE PLANS",
    "WSTE-M COMBAT SHOTGUN",
    "WSTE-M COMBAT SHOTGUNS",
    "S\xd5PHT CARD KEY",
    "S\xd5PHT CARD KEYS",
    "UPLINK CHIP",
    "UPLINK CHIPS",
    "Ryan\xd5s Light Blue Ball",
    "SKULL",
    "Violet Ball",
    "Yellow Ball",
    "Brown Ball",
    "Orange Ball",
    "Blue Ball",
    "Green Ball",
    "KKV-7 10MM FLECHETTE SMG",
    "10MM FLECHETTE MAGAZINE",
    "10MM FLECHETTE MAGAZINES",
};

// STR# Resource: "Item types"
static const char* sStringSetNumber151[] = {
    "WEAPONS",
    "AMMUNITION",
    "POWERUPS",
    "ITEMS",
    "WEAPON POWERUPS",
    "BALLS",
    "NETWORK STATISTICS",
};

// STR# Resource: "Net Statistics Strings"
static const char* sStringSetNumber153[] = {
    "%d kills",
    "%d deaths",
    "%d suicides",
    "Total Carnage",
    "Monsters",
    "Total Kills: %d (%.2f kpm)",
    "Total Deaths: %d (%.2f dpm)",
    " including %d suicides",
    "Total Team Carnage",
    " including %d friendly-fire kills",
    "Total Scores",
    "Total Team Scores",
    // ZZZ: added for team vs. team carnage in postgame report
    "%s's team",
    // ZZZ: added for legend/key in SDL postgame report
    "kills",
    "deaths",
    "suicides",
    "friendly-fire kills",
};

// STR# Resource: "OpenGL-Option Color-Picker Prompts"
static const char* sStringSetNumber200[] = {
    "What color for the Void?",
    "What day ground color?",
    "What day sky color?",
    "What night ground color?",
    "What night sky color?",
    "What moon ground color?",
    "What moon sky color?",
    "What space ground color?",
    "What space sky color?",
    "What fog color?",
};

#include "player.h" // for kTeamColorsStringSetID

static const char* sTeamColorNamesStrings[] = {
    "Slate",
    "Red",
    "Violet",
    "Yellow",
    "White",
    "Orange",
    "Blue",
    "Green",
};


void InitDefaultStringSets() {
	BUILD_STRINGSET(128, sStringSetNumber128);
	BUILD_STRINGSET(129, sStringSetNumber129);
	BUILD_STRINGSET(130, sStringSetNumber130);
	BUILD_STRINGSET(131, sStringSetNumber131);
	BUILD_STRINGSET(132, sStringSetNumber132);
	BUILD_STRINGSET(133, sStringSetNumber133);
	BUILD_STRINGSET(134, sStringSetNumber134);
	BUILD_STRINGSET(135, sStringSetNumber135);
	BUILD_STRINGSET(136, sStringSetNumber136);
	BUILD_STRINGSET(137, sStringSetNumber137);
	BUILD_STRINGSET(138, sStringSetNumber138);
	BUILD_STRINGSET(139, sStringSetNumber139);
	BUILD_STRINGSET(140, sStringSetNumber140);
	BUILD_STRINGSET(141, sStringSetNumber141);
	BUILD_STRINGSET(142, sStringSetNumber142);
	BUILD_STRINGSET(143, sStringSetNumber143);
	BUILD_STRINGSET(kDifficultyLevelsStringSetID,   sDifficultyLevelsStrings);
	BUILD_STRINGSET(kNetworkGameTypesStringSetID,   sNetworkGameTypesStrings);
	BUILD_STRINGSET(kEndConditionTypeStringSetID,   sEndConditionTypeStrings);
	BUILD_STRINGSET(kSingleOrNetworkStringSetID,	sSingleOrNetworkStrings);
	BUILD_STRINGSET(150, sStringSetNumber150);
	BUILD_STRINGSET(151, sStringSetNumber151);
	BUILD_STRINGSET(153, sStringSetNumber153);
	BUILD_STRINGSET(200, sStringSetNumber200);
	BUILD_STRINGSET(kTeamColorsStringSetID, sTeamColorNamesStrings);
}
