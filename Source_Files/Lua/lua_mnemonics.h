/*
LUA_OBJECTS.CPP

	Copyright (C) 2008 by Gregory Smith
 
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

	Implements the Lua string mnemonics
*/

struct lang_def
{
	char *name;
	int32 value;
};

const lang_def Lua_ControlPanelClass_Mnemonics[] = {
	{"oxygen recharger", 0},
	{"single shield recharger", 1},
	{"double shield recharger", 2},
	{"triple shield recharger", 3},
	{"light switch", 4},
	{"platform switch", 5},
	{"tag switch", 6},
	{"pattern buffer", 7},
	{"terminal", 8},
	{0, 0}
};

const lang_def Lua_DamageType_Mnemonics[] = {
	{"explosion", 0x00},
	{"staff", 0x01},
	{"projectile", 0x02},
	{"absorbed", 0x03},
	{"flame", 0x04},
	{"claws", 0x05},
	{"alien weapon", 0x06},
	{"hulk slap", 0x07},
	{"compiler", 0x08},
	{"fusion", 0x09},
	{"hunter", 0x0A},
	{"fists", 0x0B},
	{"teleporter", 0x0C},
	{"defender", 0x0D},
	{"yeti claws", 0x0E},
	{"yeti projectile", 0x0F},
	{"crushing", 0x10},
	{"lava", 0x11},
	{"suffocation", 0x12},
	{"goo", 0x13},
	{"energy drain", 0x14},
	{"oxygen drain", 0x15},
	{"drone", 0x16},
	{"shotgun", 0x17},
	{0, 0}
};

const lang_def Lua_DifficultyType_Mnemonics[] = {
	{"kindergarten", 0},
	{"easy", 1},
	{"normal", 2},
	{"major damage", 3},
	{"total carnage", 4},
	{0, 0}
};

const lang_def Lua_FadeType_Mnemonics[] = {
	{"red", 0x05},
	{"big red", 0x06},
	{"bonus", 0x07},
	{"bright", 0x08},
	{"long bright", 0x09},
	{"yellow", 0x0A},
	{"big yellow", 0x0B},
	{"purple", 0x0C},
	{"cyan", 0x0D},
	{"white", 0x0E},
	{"big white", 0x0F},
	{"orange", 0x10},
	{"long orange", 0x11},
	{"green", 0x12},
	{"long green", 0x13},
	{"static", 0x14},
	{"negative", 0x15},
	{"big negative", 0x16},
	{"flicker negative", 0x17},
	{"dodge purple", 0x18},
	{"burn cyan", 0x19},
	{"dodge yellow", 0x1A},
	{"burn green", 0x1B},
	{"tint green", 0x1C},
	{"tint blue", 0x1D},
	{"tint orange", 0x1E},
	{"tint gross", 0x1F},
	{"tint jjaro", 0x20},
	{0, 0}
};

const lang_def Lua_GameType_Mnemonics[] = {
	{"kill monsters", 0x00},
	{"cooperative play", 0x01},
	{"capture the flag", 0x02},
	{"king of the hill", 0x03},
	{"kill the man with the ball", 0x04},
	{"defense", 0x05},
	{"rugby", 0x06},
	{"tag", 0x07},
	{0, 0}
};

const lang_def Lua_ItemType_Mnemonics[] = {
	{"knife", 0},
	{"pistol", 1},
	{"pistol ammo", 2},
	{"fusion pistol", 3},
	{"fusion pistol ammo", 4},
	{"assault rifle", 5},
	{"assault rifle ammo", 6},
	{"assault rifle grenades", 7},
	{"missile launcher", 8},
	{"missile launcher ammo", 9},
	{"invisibility", 10},
	{"invincibility", 11},
	{"infravision", 12},
	{"alien weapon", 13},
	{"alien weapon ammo", 14},
	{"flamethrower", 15},
	{"flamethrower ammo", 16},
	{"extravision", 17},
	{"oxygen", 18},
	{"single health", 19},
	{"double health", 20},
	{"triple health", 21},
	{"shotgun", 22},
	{"shotgun ammo", 23},
	{"key", 24},
	{"uplink chip", 25},
	{"light blue ball", 26},
	{"ball", 27},
	{"violet ball", 28},
	{"yellow ball", 29},
	{"brown ball", 30},
	{"orange ball", 31},
	{"blue ball", 32},
	{"green ball", 33},
	{"smg", 34},
	{"smg ammo", 35},
	{0, 0}
};

const lang_def Lua_MediaType_Mnemonics[] = {
	{"water", 0},
	{"lava", 1},
	{"goo", 2},
	{"sewage", 3},
	{"jjaro", 4},
	{0, 0}
};

const lang_def Lua_MonsterClass_Mnemonics[] = {
	{"player", 0x0001},
	{"bob", 0x0002},
	{"madd", 0x0004},
	{"possessed drone", 0x0008},
	{"defender", 0x0010},
	{"fighter", 0x0020},
	{"trooper", 0x0040},
	{"hunter", 0x0080},
	{"enforcer", 0x0100},
	{"juggernaut", 0x0200},
	{"drone", 0x0400},
	{"compiler", 0x0800},
	{"cyborg", 0x1000},
	{"explodabob", 0x2000},
	{"tick", 0x4000},
	{"yeti", 0x8000},
	{0, 0}
};

const lang_def Lua_MonsterAction_Mnemonics[] = {
	{"stationary", 0x00},
	{"waiting to attack again", 0x01},
	{"moving", 0x02},
	{"attacking close", 0x03},
	{"attacking far", 0x04},
	{"being hit", 0x05},
	{"dying hard", 0x06},
	{"dying soft", 0x07},
	{"dying flaming", 0x08},
	{"teleporting", 0x09},
	{"teleporting in", 0x0A},
	{"teleporting out", 0x0B},
	{0, 0}
};

const lang_def Lua_MonsterMode_Mnemonics[] = {
	{"locked", 0},
	{"losing lock", 1},
	{"lost lock", 2},
	{"unlocked", 3},
	{"running", 4},
	{0, 0}
};

const lang_def Lua_MonsterType_Mnemonics[] = {
	{"player", 0x00},
	{"minor tick", 0x01},
	{"major tick", 0x02},
	{"kamikaze tick", 0x03},
	{"minor compiler", 0x04},
	{"major compiler", 0x05},
	{"minor invisible compiler", 0x06},
	{"major invisible compiler", 0x07},
	{"minor fighter", 0x08},
	{"major fighter", 0x09},
	{"minor projectile fighter", 0x0A},
	{"major projectile fighter", 0x0B},
	{"green bob", 0x0C},
	{"blue bob", 0x0D},
	{"security bob", 0x0E},
	{"explodabob", 0x0F},
	{"minor drone", 0x10},
	{"major drone", 0x11},
	{"big minor drone", 0x12},
	{"big major drone", 0x13},
	{"possessed drone", 0x14},
	{"minor cyborg", 0x15},
	{"major cyborg", 0x16},
	{"minor flame cyborg", 0x17},
	{"major flame cyborg", 0x18},
	{"minor enforcer", 0x19},
	{"major enforcer", 0x1A},
	{"minor hunter", 0x1B},
	{"major hunter", 0x1C},
	{"minor trooper", 0x1D},
	{"major trooper", 0x1E},
	{"mother of all cyborgs", 0x1F},
	{"mother of all hunters", 0x20},
	{"sewage yeti", 0x21},
	{"water yeti", 0x22},
	{"lava yeti", 0x23},
	{"minor defender", 0x24},
	{"major defender", 0x25},
	{"minor juggernaut", 0x26},
	{"major juggernaut", 0x27},
	{"tiny pfhor", 0x28},
	{"tiny bob", 0x29},
	{"tiny yeti", 0x2A},
	{"green vacbob", 0x2B},
	{"blue vacbob", 0x2C},
	{"security vacbob", 0x2D},
	{"explodavacbob", 0x2E},
	{0, 0}
};

const lang_def Lua_OverlayColor_Mnemonics[] = {
	{"green", 0},
	{"white", 1},
	{"red", 2},
	{"dark green", 3},
	{"cyan", 4},
	{"yellow", 5},
	{"dark red", 6},
	{"blue", 7},
	{0, 0}
};		

const lang_def Lua_PlayerColor_Mnemonics[] = {
	{"slate", 0},
	{"red", 1},
	{"violet", 2},
	{"yellow", 3},
	{"white", 4},
	{"orange", 5},
	{"blue", 6},
	{"green", 7},
	{0, 0}
};

const lang_def Lua_PolygonType_Mnemonics[] = {
	{"normal", 0x00},
	{"item impassable", 0x01},
	{"monster impassable", 0x02},
	{"hill", 0x03},
	{"base", 0x04},
	{"platform", 0x05},
	{"light on trigger", 0x06},
	{"platform on trigger", 0x07},
	{"light off trigger", 0x08},
	{"platform off trigger", 0x09},
	{"teleporter", 0x0A},
	{"zone border", 0x0B},
	{"goal", 0x0C},
	{"visible monster trigger", 0x0D},
	{"invisible monster trigger", 0x0E},
	{"dual monster trigger", 0x0F},
	{"item trigger", 0x10},
	{"must be explored", 0x11},
	{"automatic exit", 0x12},
	{"minor ouch", 0x13},
	{"major ouch", 0x14},
	{"glue", 0x15},
	{"glue trigger", 0x16},
	{"superglue", 0x17},
	{0, 0}
};

const lang_def Lua_ProjectileType_Mnemonics[] = {
	{"missile", 0x00},
	{"grenade", 0x01},
	{"pistol bullet", 0x02},
	{"rifle bullet", 0x03},
	{"shotgun bullet", 0x04},
	{"staff", 0x05},
	{"staff bolt", 0x06},
	{"flamethrower burst", 0x07},
	{"compiler bolt minor", 0x08},
	{"compiler bolt major", 0x09},
	{"alien weapon", 0x0A},
	{"fusion bolt minor", 0x0B},
	{"fusion bolt major", 0x0C},
	{"hunter", 0x0D},
	{"fist", 0x0E},
	{"armageddon sphere", 0x0F},
	{"armageddon electricity", 0x10},
	{"juggernaut rocket", 0x11},
	{"trooper bullet", 0x12},
	{"trooper grenade", 0x13},
	{"minor defender", 0x14},
	{"major defender", 0x15},
	{"juggernaut missile", 0x16},
	{"minor energy drain", 0x17},
	{"major energy drain", 0x18},
	{"oxygen drain", 0x19},
	{"minor hummer", 0x1A},
	{"major hummer", 0x1B},
	{"durandal hummer", 0x1C},
	{"minor cyborg ball", 0x1D},
	{"major cyborg ball", 0x1E},
	{"ball", 0x1F},
	{"minor fusion dispersal", 0x20},
	{"major fusion dispersal", 0x21},
	{"overloaded fusion dispersal", 0x22},
	{"yeti", 0x23},
	{"sewage yeti", 0x24},
	{"lava yeti", 0x25},
	{"smg bullet", 0x26},
	{0, 0}
};

const lang_def Lua_SceneryType_Mnemonics[] = {
	{"light dirt", 0},
	{"dark dirt", 1},
	{"lava bones", 2},
	{"lava bone", 3},
	{"ribs", 4},
	{"skull", 5},
	{"hanging light 1", 6},
	{"hanging light 2", 7},
	{"small cylinder", 8},
	{"large cylinder", 9},
	{"block 1", 10},
	{"block 2", 11},
	{"block 3", 12},

	{"pistol clip", 13},
	{"water short light", 14},
	{"water long light", 15},
	{"siren", 16},
	{"rocks", 17},
	{"blood drops", 18},
	{"water thing", 19},
	{"gun", 20},
	{"bob remains", 21},
	{"puddles", 22},
	{"big puddles", 23},
	{"security monitor", 24},
	{"water alien supply can", 25},
	{"machine", 26},
	{"staff", 27},
	
	{"sewage short light", 28},
	{"sewage long light", 29},
	{"junk", 30},
	{"antenna", 31},
	{"big antenna", 32},
	{"sewage alien supply can", 33},
	{"sewage bones", 34},
	{"sewage big bones", 35},
	{"pfhor pieces", 36},
	{"bob pieces", 37},
	{"bob blood", 38},
	
	{"alien short light", 39},
	{"alien long light", 40},
	{"alien ceiling rod light", 41},
	{"bulbous yellow alien object", 42},
	{"square gray organic object", 43},
	{"pfhor skeleton", 44},
	{"pfhor mask", 45},
	{"green stuff", 46},
	{"hunter shield", 47},
	{"alien bones", 48},
	{"alien sludge", 49},

	{"jjaro short light", 50},
	{"jjaro long light", 51},
	{"weird rod", 52},
	{"pfhor ship", 53},
	{"sun", 54},
	{"large glass container", 55},
	{"nub 1", 56},
	{"nub 2", 57},
	{"lh'owon", 58},
	{"floor whip antenna", 59},
	{"ceiling whip antenna", 60},
	{0, 0}
};

const lang_def Lua_WeaponType_Mnemonics[] = {
	{"fist", 0},
	{"pistol", 1},
	{"fusion pistol", 2},
	{"assault rifle", 3},
	{"missile launcher", 4},
	{"flamethrower", 5},
	{"alien weapon", 6},
	{"shotgun", 7},
	{"ball", 8},
	{"smg", 9},
	{0, 0}
};
