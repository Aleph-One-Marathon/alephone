/*

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

Feb 5, 2002 (Br'fin (Jeremy Parsons)):
	Default to keyboard and mouse control under Carbon
	for there are no InputSprockets

Apr 30, 2002 (Loren Petrich):
	Converting to a MML-based preferences system

May 16, 2002 (Woody Zenfell):
    Added UI/preferences elements for configurable mouse sensitivity
    Support for "don't auto-recenter" behavior modifier
    Routines to let other code disable/reenable/query behavior modification
   
Jul 21, 2002 (Loren Petrich):
	AS had added some code to fix the OSX preferences behavior;
	I modified it so that it would not be used in the Classic version

Apr 10-22, 2003 (Woody Zenfell):
        Join hinting and autogathering have Preferences entries now
        Being less obnoxious with unrecognized Prefs stuff
        Macintosh Enviroprefs popup style can be set in Preferences file

May 22, 2003 (Woody Zenfell):
	Support for preferences for multiple network game protocols; configurable local game port.

 May 27, 2003 (Gregory Smith):
	Preferences for speex netmic

 August 27, 2003 (Woody Zenfell):
	Preferences for netscript.  Some reworking of index-based Mac FSSpec reading along the way.
 */

/*
 *  preferences.cpp - Preferences handling
 */

#include "cseries.h"
#include "FileHandler.h"

#include "map.h"
#include "shell.h" /* For the screen_mode structure */
#include "interface.h"
#include "mysound.h"
#include "music.h"
#include "ISp_Support.h" /* BT: Added April 16, 2000 for Input Sprocket Support */

#include "preferences.h"
#include "preferences_private.h" // ZZZ: added 23 Oct 2001 for sharing of dialog item ID's with SDL.
#include "wad.h"
#include "wad_prefs.h"
#include "game_errors.h"
#include "network.h" // for _ethernet, etc.
#include "find_files.h"
#include "game_wad.h" // for set_map_file
#include "screen.h"
#include "fades.h"
#include "extensions.h"

#include "XML_ElementParser.h"
#include "XML_DataBlock.h"
#include "ColorParser.h"
#include "StarGameProtocol.h"
#include "RingGameProtocol.h"

#include "tags.h"
#include "Logging.h"

#include <string.h>
#include <stdlib.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef mac
// Marathon-engine dialog boxes:
const short FatalErrorAlert = 128;
const short NonFatalErrorAlert = 129;
#endif

static const char* sNetworkGameProtocolNames[] =
{	// These should match up with _network_game_protocol_ring, etc.
	"ring",
	"star"
};

static const size_t NUMBER_OF_NETWORK_GAME_PROTOCOL_NAMES = sizeof(sNetworkGameProtocolNames) / sizeof(sNetworkGameProtocolNames[0]);


// ZZZ: I'm really, really sorry.  But this is way better than what was here... trust me.
#ifdef mac
enum
{
	kEnvMapFileSpecIndex = 0,
	kEnvPhysicsFileSpecIndex = 1,
	kEnvShapesFileSpecIndex = 2,
	kEnvSoundsFileSpecIndex = 3,
	kNetworkScriptFileSpecIndex = 4,
	kNumberOfFileSpecIndices
};

static FSSpec* sFileSpecIndexToFileSpecPtr[kNumberOfFileSpecIndices];

#endif // mac
		


// MML-like Preferences Stuff; it makes obsolete
// w_open_preferences_file(), w_get_data_from_preferences(), and w_write_preferences_file()
// in wad_prefs.*

// The absolute root element ...
static XML_ElementParser PrefsRootParser("");

// This is the canonical root element in the XML-format preference file:
static XML_ElementParser MarathonPrefsParser("mara_prefs");

// Interpreter of read-in file
static XML_DataBlock XML_DataBlockLoader;

// Sets up the parser, of course
static void SetupPrefsParseTree();

// Have the prefs been inited?
static bool PrefsInited = false;


// Global preferences data
struct graphics_preferences_data *graphics_preferences = NULL;
struct serial_number_data *serial_preferences = NULL;
struct network_preferences_data *network_preferences = NULL;
struct player_preferences_data *player_preferences = NULL;
struct input_preferences_data *input_preferences = NULL;
struct sound_manager_parameters *sound_preferences = NULL;
struct environment_preferences_data *environment_preferences = NULL;

// LP: fake portable-files stuff
#ifdef mac
inline short memory_error() {return MemError();}
#else
inline short memory_error() {return 0;}
#endif

// Prototypes
#ifndef SDL
static void *get_player_pref_data(void);
static void *get_input_pref_data(void);
static void *get_sound_pref_data(void);
static void *get_graphics_pref_data(void);
static void *get_environment_pref_data(void);
#endif

static bool ethernet_active(void);
static void get_name_from_system(unsigned char *name);

// LP: getting rid of the (void *) mechanism as inelegant and non-type-safe
static void default_graphics_preferences(graphics_preferences_data *preferences);
static bool validate_graphics_preferences(graphics_preferences_data *preferences);
static void default_serial_number_preferences(serial_number_data *preferences);
static bool validate_serial_number_preferences(serial_number_data *preferences);
static void default_network_preferences(network_preferences_data *preferences);
static bool validate_network_preferences(network_preferences_data *preferences);
static void default_player_preferences(player_preferences_data *preferences);
static bool validate_player_preferences(player_preferences_data *preferences);
static void default_input_preferences(input_preferences_data *preferences);
static bool validate_input_preferences(input_preferences_data *preferences);
static void default_environment_preferences(environment_preferences_data *preferences);
static bool validate_environment_preferences(environment_preferences_data *preferences);


// Include platform-specific file
#if defined(mac)
#include "preferences_macintosh.h"
#elif defined(SDL)
#include "preferences_sdl.h"
#endif



// For writing out boolean values
const char *BoolString(bool B) {return (B ? "true" : "false");}

// For writing out color values
const float CNorm = 1/float(65535);	// Maximum uint16

// These are template classes so as to be able to handle both
// "rgb_color" and "RGBColor" declarations (both are uint16 red, green, blue)

template<class CType> void WriteColor(FILE *F,
	const char *Prefix, CType& Color, const char *Suffix)
{
	fprintf(F,"%s<color red=\"%f\" green=\"%f\" blue=\"%f\"/>%s",
		Prefix,CNorm*Color.red,CNorm*Color.green,CNorm*Color.blue,Suffix);
}

template<class CType> void WriteColorWithIndex(FILE *F,
	const char *Prefix, int Index, CType& Color, const char *Suffix)
{
	fprintf(F,"%s<color index=\"%d\" red=\"%f\" green=\"%f\" blue=\"%f\"/>%s",
		Prefix,Index,CNorm*Color.red,CNorm*Color.green,CNorm*Color.blue,Suffix);
}

// For writing out text strings: have both Pascal and C versions
// These special routines are necessary in order to make the writing-out XML-friendly,
// converting XML's reserved characters into appropriate strings.
void WriteXML_PasString(FILE *F, const char *Prefix, const unsigned char *String, const char *Suffix);
void WriteXML_CString(FILE *F, const char *Prefix, const char *String, int MaxLen, const char *Suffix);
void WriteXML_Char(FILE *F, unsigned char c);
#ifdef mac
void WriteXML_FSSpec(FILE *F, const char *Indent, int Index, FSSpec& Spec);
#endif

/*
 *  Initialize preferences (load from file or setup defaults)
 */

void initialize_preferences(
	void)
{
	logContext("initializing preferences");

	// In case this function gets called more than once...
	if (!PrefsInited)
	{
		SetupPrefsParseTree();
		
		graphics_preferences= new graphics_preferences_data;
		player_preferences= new player_preferences_data;
		input_preferences= new input_preferences_data;
		sound_preferences= new sound_manager_parameters;
		serial_preferences= new serial_number_data;
		network_preferences= new network_preferences_data;
		environment_preferences= new environment_preferences_data;

#ifdef mac
		sFileSpecIndexToFileSpecPtr[kEnvMapFileSpecIndex]	= &(environment_preferences->map_file);
		sFileSpecIndexToFileSpecPtr[kEnvPhysicsFileSpecIndex]	= &(environment_preferences->physics_file);
		sFileSpecIndexToFileSpecPtr[kEnvShapesFileSpecIndex]	= &(environment_preferences->shapes_file);
		sFileSpecIndexToFileSpecPtr[kEnvSoundsFileSpecIndex]	= &(environment_preferences->sounds_file);
		sFileSpecIndexToFileSpecPtr[kNetworkScriptFileSpecIndex]= &(network_preferences->netscript_file);
#endif

		XML_DataBlockLoader.CurrentElement = &PrefsRootParser;
		XML_DataBlockLoader.SourceName = "[Preferences]";
				
		PrefsInited = true;
		
		read_preferences ();
	}
}

void read_preferences ()
{
	// Set to defaults; will be overridden by reading in the XML stuff
	default_graphics_preferences(graphics_preferences);
	default_serial_number_preferences(serial_preferences);
	default_network_preferences(network_preferences);
	default_player_preferences(player_preferences);
	default_input_preferences(input_preferences);
	default_sound_manager_parameters(sound_preferences);
	default_environment_preferences(environment_preferences);

	// Slurp in the file and parse it

	FileSpecifier FileSpec;

#if defined(mac)
	FileSpec.SetParentToPreferences();
	FileSpec.SetName(getcstr(temporary, strFILENAMES, filenamePREFERENCES),'TEXT');
#elif defined(SDL)
	FileSpec.SetToPreferencesDir();
	FileSpec += getcstr(temporary, strFILENAMES, filenamePREFERENCES);
#endif

	OpenedFile OFile;
	bool defaults = false;
	bool opened = FileSpec.Open(OFile);

	if (!opened) {
		defaults = true;
		FileSpec.SetNameWithPath(getcstr(temporary, strFILENAMES, filenamePREFERENCES));
		opened = FileSpec.Open(OFile);
	}

	if (opened) {
		long Len = 0;
		OFile.GetLength(Len);
		if (Len > 0) {
			vector<char> FileContents(Len);

			if (OFile.Read(Len, &FileContents[0])) {
				OFile.Close();
				if (!XML_DataBlockLoader.ParseData(&FileContents[0], Len)) {
#if !defined(TARGET_API_MAC_CARBON)
					if (defaults)
						alert_user("There were default preferences-file parsing errors (see Aleph One Log.txt for details)", infoError);
					else
						alert_user("There were preferences-file parsing errors (see Aleph One Log.txt for details)", infoError);
#endif
				}
			}
		}
	}

	// Check on the read-in prefs
	validate_graphics_preferences(graphics_preferences);
	validate_serial_number_preferences(serial_preferences);
	validate_network_preferences(network_preferences);
	validate_player_preferences(player_preferences);
	validate_input_preferences(input_preferences);
	validate_environment_preferences(environment_preferences);
	
	// jkvw: If we try to load a default file, but can't, we'll have set the game error.
	//       But that's not useful, because we're just going to try loading the file
	//       from user preferences.  It used to be this code was only called in initialisation,
	//       and any game error generated here was simply clobbered by an init time
	//       clear_game_error().  Since I'm using this more generally now, I'm clearing the
	//       error right here, because it's not like we're bothered when we can't load a
	//       default file.
	//       (Problem is SDL specific - socre one for Carbon? :) )
	clear_game_error ();
}


/*
 *  Write preferences to file
 */

void write_preferences(
	void)
{
	// LP: I've used plain stdio here because it's simple to do formatted writing with it.
	
#if defined(mac)
	// Mac stuff: save old default directory
	short OldVRefNum;
	long OldParID;
#if defined(TARGET_API_MAC_CARBON) && defined(__MACH__)
	//AS: evil hack since HSetVol doesn't affect fopen() on OS X, so we fopen an absolute path
	char str[257] = "";
	const char *home = getenv("HOME");
	if (home) strcat(str, home);
	strcat(str,"/Library/Preferences/");
	strcat(str,getcstr(temporary, strFILENAMES, filenamePREFERENCES));
#endif
	HGetVol(nil,&OldVRefNum,&OldParID);
	
	// Set default directory to prefs directory
	FileSpecifier FileSpec;
	if (!FileSpec.SetParentToPreferences()) return;
	if (HSetVol(nil, FileSpec.GetSpec().vRefNum, FileSpec.GetSpec().parID) != noErr) return;
    
	// Open the file
#if defined(TARGET_API_MAC_CARBON) && defined(__MACH__)
	FILE *F = fopen(str,"w");
#else
	FILE *F = fopen(getcstr(temporary, strFILENAMES, filenamePREFERENCES),"w");
#endif

#elif defined(SDL)
	// Fix courtesy of mdadams@ku.edu
	FileSpecifier FileSpec;
	FileSpec.SetToPreferencesDir();
	FileSpec += getcstr(temporary, strFILENAMES, filenamePREFERENCES);
	
	// Open the file
	FILE *F = fopen(FileSpec.GetPath(),"w");
#endif
	
	if (!F)
	{
#ifdef mac
		// Restore the old default directory and quit
		HSetVol(nil,OldVRefNum,OldParID);
#endif
		return;
	}

	fprintf(F,"<!-- Preferences file for the Marathon Open Source \"Aleph One\" engine -->\n\n");
	
	fprintf(F,"<mara_prefs>\n\n");
	
	fprintf(F,"<graphics\n");
	fprintf(F,"  scmode_size=\"%hd\"\n",graphics_preferences->screen_mode.size);
	fprintf(F,"  scmode_accel=\"%hd\"\n",graphics_preferences->screen_mode.acceleration);
	fprintf(F,"  scmode_highres=\"%s\"\n",BoolString(graphics_preferences->screen_mode.high_resolution));
	fprintf(F,"  scmode_fullscreen=\"%s\"\n",BoolString(graphics_preferences->screen_mode.fullscreen));
	fprintf(F,"  scmode_fill_the_screen=\"%s\"\n", BoolString(graphics_preferences->screen_mode.fill_the_screen));
	fprintf(F,"  scmode_bitdepth=\"%hd\"\n",graphics_preferences->screen_mode.bit_depth);
	fprintf(F,"  scmode_gamma=\"%hd\"\n",graphics_preferences->screen_mode.gamma_level);
#ifdef mac
	fprintf(F,"  devspec_slot=\"%hd\"\n",graphics_preferences->device_spec.slot);
	fprintf(F,"  devspec_flags=\"%hd\"\n",graphics_preferences->device_spec.flags);
	fprintf(F,"  devspec_bitdepth=\"%hd\"\n",graphics_preferences->device_spec.bit_depth);
	fprintf(F,"  devspec_width=\"%hd\"\n",graphics_preferences->device_spec.width);
	fprintf(F,"  devspec_height=\"%hd\"\n",graphics_preferences->device_spec.height);
	fprintf(F,"  frequency=\"%f\"\n",graphics_preferences->refresh_frequency);
#endif
	fprintf(F,"  ogl_flags=\"%hu\"\n",graphics_preferences->OGL_Configure.Flags);
        fprintf(F,"  experimental_rendering=\"%s\"\n",BoolString(graphics_preferences->experimental_rendering));
        fprintf(F,"  anisotropy_level=\"%f\"\n", graphics_preferences->OGL_Configure.AnisotropyLevel);
	fprintf(F,"  multisamples=\"%i\"\n", graphics_preferences->OGL_Configure.Multisamples);
	fprintf(F,"  geforce_fix=\"%s\"\n", BoolString(graphics_preferences->OGL_Configure.GeForceFix));
	fprintf(F,"  double_corpse_limit=\"%s\"\n", BoolString(graphics_preferences->double_corpse_limit));
	fprintf(F,">\n");
	fprintf(F,"  <void>\n");
	WriteColor(F,"    ",graphics_preferences->OGL_Configure.VoidColor,"\n");
	fprintf(F,"  </void>\n");
	fprintf(F,"  <landscapes>\n");
	for (int i=0; i<4; i++)
		for (int j=0; j<2; j++)
			WriteColorWithIndex(F,"    ",(2*i+j),
				graphics_preferences->OGL_Configure.LscpColors[i][j],"\n");
	fprintf(F,"  </landscapes>\n");
	for (int k=0; k<OGL_NUMBER_OF_TEXTURE_TYPES; k++)
	{
		OGL_Texture_Configure& TxtrConfig = graphics_preferences->OGL_Configure.TxtrConfigList[k];
		fprintf(F,"  <texture index=\"%hd\" near_filter=\"%hd\" far_filter=\"%hd\" resolution=\"%hd\" color_format=\"%d\" max_size=\"%d\"/>\n",
			k, TxtrConfig.NearFilter, TxtrConfig.FarFilter, TxtrConfig.Resolution, TxtrConfig.ColorFormat, TxtrConfig.MaxSize);
	}
	fprintf(F,"</graphics>\n\n");
	
	fprintf(F,"<player\n");
	WriteXML_PasString(F, "  name=\"",player_preferences->name,"\"\n");
	fprintf(F,"  color=\"%hd\"\n",player_preferences->color);
	fprintf(F,"  team=\"%hd\"\n",player_preferences->team);
	fprintf(F,"  last_time_ran=\"%lu\"\n",player_preferences->last_time_ran);
	fprintf(F,"  difficulty=\"%hd\"\n",player_preferences->difficulty_level);
	fprintf(F,"  bkgd_music=\"%s\"\n",BoolString(player_preferences->background_music_on));
	fprintf(F,">\n");
	ChaseCamData& ChaseCam = player_preferences->ChaseCam;
	fprintf(F,"  <chase_cam behind=\"%hd\" upward=\"%hd\" rightward=\"%hd\" flags=\"%hd\"\n",
		ChaseCam.Behind, ChaseCam.Upward, ChaseCam.Rightward, ChaseCam.Flags);
	fprintf(F,"    damping=\"%f\" spring=\"%f\" opacity=\"%f\"/>\n",
		ChaseCam.Damping, ChaseCam.Spring, ChaseCam.Opacity);
	CrosshairData& Crosshairs = player_preferences->Crosshairs;
	fprintf(F,"  <crosshairs\n");
	fprintf(F,"    thickness=\"%hd\" from_center=\"%hd\" length=\"%hd\"\n",
		Crosshairs.Thickness, Crosshairs.FromCenter, Crosshairs.Length);
	fprintf(F,"    shape=\"%hd\" opacity=\"%f\"\n",
		Crosshairs.Shape, Crosshairs.Opacity);
	fprintf(F,"  >\n"),
	WriteColor(F,"    ",Crosshairs.Color,"\n");
	fprintf(F,"  </crosshairs>\n");
	fprintf(F,"</player>\n\n");
	
	fprintf(F,"<input\n");
	fprintf(F,"  device=\"%hd\"\n",input_preferences->input_device);
	fprintf(F,"  modifiers=\"%hu\"\n",input_preferences->modifiers);
	fprintf(F,"  sens_horizontal=\"%ld\"\n",input_preferences->sens_horizontal); // ZZZ, LP
	fprintf(F,"  sens_vertical=\"%ld\"\n",input_preferences->sens_vertical); // ZZZ, LP
	fprintf(F,"  mouse_acceleration=\"%s\"\n",BoolString(input_preferences->mouse_acceleration)); // SB

	fprintf(F,">\n");
	for (int i = 0; i < MAX_BUTTONS; i++)
		fprintf(F,"  <mouse_button index=\"%hd\" action=\"%s\"/>\n", i,
			input_preferences->mouse_button_actions[i] == _mouse_button_fires_left_trigger ? "left_trigger" : 
			input_preferences->mouse_button_actions[i] == _mouse_button_fires_right_trigger ? "right_trigger" : "none");
#if defined(mac)
	for (int k=0; k<NUMBER_OF_KEYS; k++)
		fprintf(F,"  <mac_key index=\"%hd\" value=\"%hd\"/>\n",
			k,input_preferences->keycodes[k]);
#elif defined(SDL)
	for (int k=0; k<NUMBER_OF_KEYS; k++)
		fprintf(F,"  <sdl_key index=\"%hd\" value=\"%hd\"/>\n",
			k,input_preferences->keycodes[k]);
#endif
	fprintf(F,"</input>\n\n");
	
	fprintf(F,"<sound\n");
	fprintf(F,"  channels=\"%hd\"\n",sound_preferences->channel_count);
	fprintf(F,"  volume=\"%hd\"\n",sound_preferences->volume);
	fprintf(F,"  music_volume=\"%hd\"\n",sound_preferences->music);
	fprintf(F,"  flags=\"%hu\"\n",sound_preferences->flags);
	fprintf(F,"/>\n\n");
	
#if !defined(DISABLE_NETWORKING)
	fprintf(F,"<network\n");
	fprintf(F,"  microphone=\"%s\"\n",BoolString(network_preferences->allow_microphone));
	fprintf(F,"  untimed=\"%s\"\n",BoolString(network_preferences->game_is_untimed));
	fprintf(F,"  type=\"%hd\"\n",network_preferences->type);
	fprintf(F,"  game_type=\"%hd\"\n",network_preferences->game_type);
	fprintf(F,"  difficulty=\"%hd\"\n",network_preferences->difficulty_level);
	fprintf(F,"  game_options=\"%hu\"\n",network_preferences->game_options);
	fprintf(F,"  time_limit=\"%ld\"\n",network_preferences->time_limit);
	fprintf(F,"  kill_limit=\"%hd\"\n",network_preferences->kill_limit);
	fprintf(F,"  entry_point=\"%hd\"\n",network_preferences->entry_point);
        fprintf(F,"  autogather=\"%s\"\n",BoolString(network_preferences->autogather));
        fprintf(F,"  join_by_address=\"%s\"\n",BoolString(network_preferences->join_by_address));
        WriteXML_CString(F, "  join_address=\"",network_preferences->join_address,256,"\"\n");
        fprintf(F,"  local_game_port=\"%hu\"\n",network_preferences->game_port);
	fprintf(F,"  game_protocol=\"%s\"\n",sNetworkGameProtocolNames[network_preferences->game_protocol]);
	fprintf(F,"  use_speex_netmic_encoder=\"%s\"\n", BoolString(network_preferences->use_speex_encoder));
	fprintf(F,"  speex_encoder_quality=\"%hu\"\n", network_preferences->speex_encoder_quality);
	fprintf(F,"  speex_encoder_complexity=\"%hu\"\n", network_preferences->speex_encoder_complexity);
	fprintf(F,"  use_netscript=\"%s\"\n", BoolString(network_preferences->use_netscript));
#ifdef SDL
	WriteXML_CString(F,"  netscript_file=\"", network_preferences->netscript_file, sizeof(network_preferences->netscript_file), "\"\n");
#endif
	fprintf(F,"  cheat_flags=\"%hu\"\n",network_preferences->cheat_flags);
	fprintf(F,"  advertise_on_metaserver=\"%s\"\n",BoolString(network_preferences->advertise_on_metaserver));
	fprintf(F,"  attempt_upnp=\"%s\"\n", BoolString(network_preferences->attempt_upnp));
	fprintf(F,">\n");
#ifndef SDL
	WriteXML_FSSpec(F,"  ", kNetworkScriptFileSpecIndex, network_preferences->netscript_file);
#endif
	WriteStarPreferences(F);
	WriteRingPreferences(F);
	fprintf(F,"</network>\n\n");
#endif // !defined(DISABLE_NETWORKING)

	fprintf(F,"<environment\n");
#ifdef SDL
	WriteXML_CString(F,"  map_file=\"",environment_preferences->map_file,256,"\"\n");
	WriteXML_CString(F,"  physics_file=\"",environment_preferences->physics_file,256,"\"\n");
	WriteXML_CString(F,"  shapes_file=\"",environment_preferences->shapes_file,256,"\"\n");
	WriteXML_CString(F,"  sounds_file=\"",environment_preferences->sounds_file,256,"\"\n");
#if defined(__APPLE__) && defined(__MACH__)
	extern char *bundle_name; // SDLMain.m
	// replace our leading bundle name with generic "AlephOneSDL.app" (we do reverse when loading)
	if (!strncmp(environment_preferences->theme_dir, bundle_name, strlen(bundle_name))) {
		strlcpy(temporary, "AlephOneSDL.app", sizeof(temporary));
		strlcat(temporary, environment_preferences->theme_dir + strlen(bundle_name), sizeof(temporary));
		WriteXML_CString(F,"  theme_dir=\"",temporary,256,"\"\n");
	} else
#endif
	WriteXML_CString(F,"  theme_dir=\"",environment_preferences->theme_dir,256,"\"\n");
#endif
	fprintf(F,"  map_checksum=\"%lu\"\n",environment_preferences->map_checksum);
	fprintf(F,"  physics_checksum=\"%lu\"\n",environment_preferences->physics_checksum);
	fprintf(F,"  shapes_mod_date=\"%lu\"\n",uint32(environment_preferences->shapes_mod_date));
	fprintf(F,"  sounds_mod_date=\"%lu\"\n",uint32(environment_preferences->sounds_mod_date));
	fprintf(F,"  group_by_directory=\"%s\"\n",BoolString(environment_preferences->group_by_directory));
	fprintf(F,"  reduce_singletons=\"%s\"\n",BoolString(environment_preferences->reduce_singletons));
	fprintf(F,"  non_bungie_warning=\"%s\"\n",BoolString(environment_preferences->non_bungie_warning));
	fprintf(F,">\n");
#ifdef mac
	WriteXML_FSSpec(F,"  ",kEnvMapFileSpecIndex,environment_preferences->map_file);
	WriteXML_FSSpec(F,"  ",kEnvPhysicsFileSpecIndex,environment_preferences->physics_file);
	WriteXML_FSSpec(F,"  ",kEnvShapesFileSpecIndex,environment_preferences->shapes_file);
	WriteXML_FSSpec(F,"  ",kEnvSoundsFileSpecIndex,environment_preferences->sounds_file);
#endif
	fprintf(F,"</environment>\n\n");
			
	fprintf(F,"</mara_prefs>\n\n");
	
	fclose(F);
	
#ifdef mac
	// Restore it
	HSetVol(nil,OldVRefNum,OldParID);
#endif
}

/*
 *  Get prefs data from prefs file (or defaults)
 */

#ifndef SDL
static void *get_graphics_pref_data() {return graphics_preferences;}
static void *get_player_pref_data() {return player_preferences;}
static void *get_sound_pref_data() {return sound_preferences;}
static void *get_input_pref_data() {return input_preferences;}
static void *get_environment_pref_data() {return environment_preferences;}
#endif

/*
 *  Setup default preferences
 */

static void default_graphics_preferences(graphics_preferences_data *preferences)
{
  memset(&preferences->screen_mode, '\0', sizeof(screen_mode_data));
	preferences->screen_mode.gamma_level= DEFAULT_GAMMA_LEVEL;

#ifdef mac
	preferences->device_spec.slot= NONE;
	preferences->device_spec.flags= deviceIsColor;
	preferences->device_spec.bit_depth= 32;
	preferences->device_spec.width= 800;
	preferences->device_spec.height= 600;

	preferences->screen_mode.size = _100_percent;
	preferences->screen_mode.fullscreen = false;
	preferences->screen_mode.high_resolution = true;
	
	preferences->refresh_frequency = DEFAULT_MONITOR_REFRESH_FREQUENCY;
	
	if (hardware_acceleration_code(&preferences->device_spec) == _opengl_acceleration)
	{
		preferences->screen_mode.acceleration = _opengl_acceleration;
		preferences->screen_mode.bit_depth = 32;
	}
	else
	{
		preferences->screen_mode.acceleration = _no_acceleration;
		preferences->screen_mode.bit_depth = 32;
	}
	
#else
	preferences->screen_mode.size = _100_percent;
#if defined(__APPLE__) && defined(__MACH__)
	preferences->screen_mode.acceleration = _opengl_acceleration;
#else
	preferences->screen_mode.acceleration = _no_acceleration;
#endif
	preferences->screen_mode.high_resolution = true;
	preferences->screen_mode.fullscreen = true;
	
	const SDL_version *version = SDL_Linked_Version();
	if (SDL_VERSIONNUM(version->major, version->minor, version->patch) >= SDL_VERSIONNUM(1, 2, 10))
		preferences->screen_mode.fill_the_screen = false;
	else
		preferences->screen_mode.fill_the_screen = true;
	preferences->screen_mode.bit_depth = 16;
#endif
	
	preferences->screen_mode.draw_every_other_line= false;
	
	OGL_SetDefaults(preferences->OGL_Configure);

        preferences->experimental_rendering= false;
	preferences->double_corpse_limit= false;
}

static void default_serial_number_preferences(serial_number_data *preferences)
{
	memset(preferences, 0, sizeof(struct serial_number_data));
}

static void default_network_preferences(network_preferences_data *preferences)
{
	preferences->type= _ethernet;

	preferences->allow_microphone = true;
	preferences->game_is_untimed = false;
	preferences->difficulty_level = 2;
	preferences->game_options =	_multiplayer_game | _ammo_replenishes | _weapons_replenish
		| _specials_replenish |	_monsters_replenish | _burn_items_on_death | _suicide_is_penalized 
		| _force_unique_teams | _live_network_stats;
	preferences->time_limit = 10 * TICKS_PER_SECOND * 60;
	preferences->kill_limit = 10;
	preferences->entry_point= 0;
	preferences->game_type= _game_of_kill_monsters;
	preferences->autogather= false;
	preferences->join_by_address= false;
	obj_clear(preferences->join_address);
	preferences->game_port= 4226;	// Magic number I guess, but this is the only place it's used
                                // (everyone else uses preferences->game_port)
	preferences->game_protocol= _network_game_protocol_default;
#if !defined(DISABLE_NETWORKING)
	DefaultStarPreferences();
	DefaultRingPreferences();
#endif // !defined(DISABLE_NETWORKING)
	preferences->use_speex_encoder = true;
	preferences->speex_encoder_quality = 2;
	preferences->speex_encoder_complexity = 1;
	preferences->use_netscript = false;
#ifdef mac
	obj_clear(preferences->netscript_file);
#else
	preferences->netscript_file[0] = '\0';
#endif
	preferences->cheat_flags = _allow_tunnel_vision | _allow_crosshair | _allow_behindview;
	preferences->advertise_on_metaserver = false;
	preferences->attempt_upnp = false;
}

static void default_player_preferences(player_preferences_data *preferences)
{
	obj_clear(*preferences);

#ifdef mac
	GetDateTime(&preferences->last_time_ran);
#endif
	preferences->difficulty_level= 2;
	get_name_from_system(preferences->name);
	
	// LP additions for new fields:
	
	preferences->ChaseCam.Behind = 1536;
	preferences->ChaseCam.Upward = 0;
	preferences->ChaseCam.Rightward = 0;
	preferences->ChaseCam.Flags = 0;
	preferences->ChaseCam.Damping = 0.5;
	preferences->ChaseCam.Spring = 0;
	preferences->ChaseCam.Opacity = 1;
	
	preferences->Crosshairs.Thickness = 2;
	preferences->Crosshairs.FromCenter = 8;
	preferences->Crosshairs.Length = 16;
	preferences->Crosshairs.Shape = CHShape_RealCrosshairs;
	preferences->Crosshairs.Color = rgb_white;
	preferences->Crosshairs.Opacity = 0.5;
	preferences->Crosshairs.PreCalced = false;
}

static void default_input_preferences(input_preferences_data *preferences)
{
#if defined(TARGET_API_MAC_CARBON)
	// JTP: No ISP, go with default option
	preferences->input_device= _mouse_yaw_pitch;
#else
	preferences->input_device= _keyboard_or_game_pad;
#endif
	set_default_keys(preferences->keycodes, _standard_keyboard_setup);
	
	// LP addition: set up defaults for modifiers:
	// interchange run and walk, but don't interchange swim and sink.
	preferences->modifiers = _inputmod_interchange_run_walk;

	// LP: split into horizontal and vertical sensitivities
	// ZZZ addition: sensitivity factor starts at 1 (no adjustment)
	preferences->sens_horizontal = FIXED_ONE;
	preferences->sens_vertical = FIXED_ONE;
	
	// SB
	preferences->mouse_acceleration = true;

	// default mouse settings
	preferences->mouse_button_actions[0] = _mouse_button_fires_left_trigger;
	preferences->mouse_button_actions[1] = _mouse_button_fires_right_trigger;
	for (int i = 2; i < MAX_BUTTONS; i++)
		preferences->mouse_button_actions[i] = _mouse_button_does_nothing;
}

static void default_environment_preferences(environment_preferences_data *preferences)
{
	obj_set(*preferences, NONE);

	FileSpecifier DefaultMapFile;
	FileSpecifier DefaultShapesFile;
	FileSpecifier DefaultSoundsFile;
	FileSpecifier DefaultPhysicsFile;
    
#ifdef mac
	// Can use "all in one" routine
	get_default_file_specs(&DefaultMapFile, &DefaultShapesFile, &DefaultSoundsFile, &DefaultPhysicsFile);
#else
	get_default_map_spec(DefaultMapFile);
	get_default_physics_spec(DefaultPhysicsFile);
	get_default_shapes_spec(DefaultShapesFile);
	get_default_sounds_spec(DefaultSoundsFile);
#endif
                
	preferences->map_checksum= read_wad_file_checksum(DefaultMapFile);
#ifdef mac
	obj_copy(preferences->map_file, DefaultMapFile.GetSpec());
#else
	strncpy(preferences->map_file, DefaultMapFile.GetPath(), 256);
	preferences->map_file[255] = 0;
#endif
	
	preferences->physics_checksum= read_wad_file_checksum(DefaultPhysicsFile);
#ifdef mac
	obj_copy(preferences->physics_file, DefaultPhysicsFile.GetSpec());
#else
	strncpy(preferences->physics_file, DefaultPhysicsFile.GetPath(), 256);
	preferences->physics_file[255] = 0;
#endif
	
	preferences->shapes_mod_date = DefaultShapesFile.GetDate();
#ifdef mac
	obj_copy(preferences->shapes_file, DefaultShapesFile.GetSpec());
#else
	strncpy(preferences->shapes_file, DefaultShapesFile.GetPath(), 256);
	preferences->shapes_file[255] = 0;
#endif

	preferences->sounds_mod_date = DefaultSoundsFile.GetDate();
#ifdef mac
	obj_copy(preferences->sounds_file, DefaultSoundsFile.GetSpec());
#else
	strncpy(preferences->sounds_file, DefaultSoundsFile.GetPath(), 256);
	preferences->sounds_file[255] = 0;
#endif

#ifdef SDL
	FileSpecifier DefaultThemeFile;
	get_default_theme_spec(DefaultThemeFile);
	strncpy(preferences->theme_dir, DefaultThemeFile.GetPath(), 256);
	preferences->theme_dir[255] = 0;
#endif

	preferences->group_by_directory = true;
	preferences->reduce_singletons = false;
	preferences->non_bungie_warning = true;
}


/*
 *  Validate preferences
 */

static bool validate_graphics_preferences(graphics_preferences_data *preferences)
{
	bool changed= false;

	// Fix bool options
	preferences->screen_mode.high_resolution = !!preferences->screen_mode.high_resolution;
	preferences->screen_mode.fullscreen = !!preferences->screen_mode.fullscreen;
	preferences->screen_mode.draw_every_other_line = !!preferences->screen_mode.draw_every_other_line;

	if(preferences->screen_mode.gamma_level<0 || preferences->screen_mode.gamma_level>=NUMBER_OF_GAMMA_LEVELS)
	{
		preferences->screen_mode.gamma_level= DEFAULT_GAMMA_LEVEL;
		changed= true;
	}

#ifdef mac
#ifndef __MACH__
	if (preferences->screen_mode.bit_depth==32 && !machine_supports_32bit(&preferences->device_spec))
	{
		preferences->screen_mode.bit_depth= 16;
		changed= true;
	}

	/* Don't change out of 16 bit if we are in valkyrie mode. */
	// LP: good riddance to that old video card :-P
	if (preferences->screen_mode.bit_depth==16 && !machine_supports_16bit(&preferences->device_spec))
	{
		preferences->screen_mode.bit_depth= 8;
		changed= true;
	}
#endif
#else
	// OpenGL requires at least 16 bit color depth
	if (preferences->screen_mode.acceleration == _opengl_acceleration && preferences->screen_mode.bit_depth == 8)
	{
		preferences->screen_mode.bit_depth= 16;
		changed= true;
	}
#endif

	return changed;
}

static bool validate_serial_number_preferences(serial_number_data *preferences)
{
	(void) (preferences);
	return false;
}

static bool validate_network_preferences(network_preferences_data *preferences)
{
	bool changed= false;

	// Fix bool options
	preferences->allow_microphone = !!preferences->allow_microphone;
	preferences->game_is_untimed = !!preferences->game_is_untimed;

	if(preferences->type<0||preferences->type>_ethernet)
	{
		if(ethernet_active())
		{
			preferences->type= _ethernet;
		} else {
			preferences->type= _localtalk;
		}
		changed= true;
	}
	
	if(preferences->game_is_untimed != true && preferences->game_is_untimed != false)
	{
		preferences->game_is_untimed= false;
		changed= true;
	}

	if(preferences->allow_microphone != true && preferences->allow_microphone != false)
	{
		preferences->allow_microphone= true;
		changed= true;
	}

	if(preferences->game_type<0 || preferences->game_type >= NUMBER_OF_GAME_TYPES)
	{
		preferences->game_type= _game_of_kill_monsters;
		changed= true;
	}

        // ZZZ: is this relevant anymore now with XML prefs?  if so, should validate autogather, join_by_address, and join_address.

	if(preferences->game_protocol >= NUMBER_OF_NETWORK_GAME_PROTOCOLS)
	{
		preferences->game_protocol= _network_game_protocol_default;
		changed= true;
	}
	
	return changed;
}

static bool validate_player_preferences(player_preferences_data *preferences)
{
	// Fix bool options
	preferences->background_music_on = !!preferences->background_music_on;

	return false;
}

static bool validate_input_preferences(input_preferences_data *preferences)
{
	(void) (preferences);
	return false;
}

static bool validate_environment_preferences(environment_preferences_data *preferences)
{
	(void) (preferences);
	return false;
}


/*
 *  Load the environment
 */

/* Load the environment.. */
void load_environment_from_preferences(
	void)
{
	FileSpecifier File;
	struct environment_preferences_data *prefs= environment_preferences;

#ifdef mac
	File.SetSpec(prefs->map_file);
#else
	File = prefs->map_file;
#endif
	if (File.Exists()) {
		set_map_file(File);
	} else {
		/* Try to find the checksum */
		if(find_wad_file_that_has_checksum(File,
			_typecode_scenario, strPATHS, prefs->map_checksum))	{
			set_map_file(File);
		} else {
			set_to_default_map();
		}
	}

#ifdef mac
	File.SetSpec(prefs->physics_file);
#else
	File = prefs->physics_file;
#endif
	if (File.Exists()) {
		set_physics_file(File);
		import_definition_structures();
	} else {
		if(find_wad_file_that_has_checksum(File,
			_typecode_physics, strPATHS, prefs->physics_checksum)) {
			set_physics_file(File);
			import_definition_structures();
		} else {
			/* Didn't find it.  Don't change them.. */
		}
	}
	
#ifdef mac
	File.SetSpec(prefs->shapes_file);
#else
	File = prefs->shapes_file;
#endif
	if (File.Exists()) {
		open_shapes_file(File);
	} else {
		if(find_file_with_modification_date(File,
			_typecode_shapes, strPATHS, prefs->shapes_mod_date))
		{
			open_shapes_file(File);
		} else {
			/* What should I do? */
		}
	}

#ifdef mac
	File.SetSpec(prefs->sounds_file);
#else
	File = prefs->sounds_file;
#endif
	if (File.Exists()) {
		open_sound_file(File);
	} else {
		if(find_file_with_modification_date(File,
			_typecode_sounds, strPATHS, prefs->sounds_mod_date)) {
			open_sound_file(File);
		} else {
			/* What should I do? */
		}
	}
}


// LP addition: get these from the preferences data
ChaseCamData& GetChaseCamData() {return player_preferences->ChaseCam;}
CrosshairData& GetCrosshairData() {return player_preferences->Crosshairs;}
OGL_ConfigureData& Get_OGL_ConfigureData() {return graphics_preferences->OGL_Configure;}


// ZZZ: override player-behavior modifiers
static bool sStandardizeModifiers = false;


void
standardize_player_behavior_modifiers() {
    sStandardizeModifiers = true;
}


void
restore_custom_player_behavior_modifiers() {
    sStandardizeModifiers = false;
}


bool
is_player_behavior_standard() {
    return (!dont_switch_to_new_weapon() && !dont_auto_recenter());
}


// LP addition: modification of Josh Elsasser's dont-switch-weapons patch
// so as to access preferences stuff here
bool dont_switch_to_new_weapon() {
    // ZZZ: let game require standard modifiers for a while
    if(!sStandardizeModifiers)
	    return TEST_FLAG(input_preferences->modifiers,_inputmod_dont_switch_to_new_weapon);
    else
        return false;
}


// ZZZ addition: like dont_switch_to_new_weapon()
bool
dont_auto_recenter() {
    if(!sStandardizeModifiers)
        return TEST_FLAG(input_preferences->modifiers, _inputmod_dont_auto_recenter);
    else
        return false;
}


// For writing out text strings: have both Pascal and C versions
// These special routines are necessary in order to make the writing-out XML-friendly,
// converting XML's reserved characters into appropriate strings.

void WriteXML_PasString(FILE *F, const char *Prefix, const unsigned char *String, const char *Suffix)
{
	fprintf(F,"%s",Prefix);
	for (int k=1; k<=String[0]; k++)
		WriteXML_Char(F,String[k]);
	fprintf(F,"%s",Suffix);
}

void WriteXML_CString(FILE *F, const char *Prefix, const char *String, int MaxLen, const char *Suffix)
{
	fprintf(F,"%s",Prefix);
	size_t Len = strlen(String);
	for (size_t k=0; k<Len; k++)
		WriteXML_Char(F,String[k]);
	fprintf(F,"%s",Suffix);
}

void WriteXML_Char(FILE *F, unsigned char c)
{
	// Make XML-friendly
	// Are the characters normal ASCII printable characters?
	if (c < 0x20)
	{
		// Turn the bad character into a good one,
		// because Expat dislikes characters below 0x20
		fprintf(F,"$");
	}
	else if (c >= 0x7f)
	{
		// Dump as hex
		fprintf(F,"&#x%x;",int(c));
	}
	else
	{
		switch(c)
		{
		// XML reserved characters
		case '&':
			fprintf(F,"&amp;");
			break;
		
		case '<':
			fprintf(F,"&lt;");
			break;
		
		case '>':
			fprintf(F,"&gt;");
			break;
		
		case '"':
			fprintf(F,"&quot;");
			break;
		
		case '\'':
			fprintf(F,"&apos;");
			break;
		
		// OK character
		default:
			fputc(int(c),F);
			break;
		}
	}
}

#ifdef mac
void WriteXML_FSSpec(FILE *F, const char *Indent, int Index, FSSpec& Spec)
{
	fprintf(F,"%s<mac_fsspec\n",Indent);
	fprintf(F,"%s  index=\"%d\"\n",Indent,Index);
	fprintf(F,"%s  v_ref_num=\"%hd\"\n",Indent,Spec.vRefNum);
	fprintf(F,"%s  par_id=\"%ld\"\n",Indent,Spec.parID);
	fprintf(F,"%s",Indent);
	WriteXML_PasString(F,"  name=\"",Spec.name,"\"\n");
	fprintf(F,"%s/>\n",Indent);
}
#endif



// LP additions: MML-like prefs stuff
// These parsers are intended to work correctly on both Mac and SDL prefs files;
// including one crossing over to the other platform (uninterpreted fields become defaults)

// To get around both RGBColor and rgb_color being used in the code
template<class CType1, class CType2> void CopyColor(CType1& Dest, CType2& Src)
{
	Dest.red = Src.red;
	Dest.green = Src.green;
	Dest.blue = Src.blue;
}


class XML_VoidPrefsParser: public XML_ElementParser
{
	rgb_color Color;

public:
	bool Start();
	bool End();

	XML_VoidPrefsParser(): XML_ElementParser("void") {}
};

bool XML_VoidPrefsParser::Start()
{
	CopyColor(Color,graphics_preferences->OGL_Configure.VoidColor);
	
	Color_SetArray(&Color);
	
	return true;
}

bool XML_VoidPrefsParser::End()
{
	CopyColor(graphics_preferences->OGL_Configure.VoidColor,Color);

	return true;
}

static XML_VoidPrefsParser VoidPrefsParser;


class XML_LandscapePrefsParser: public XML_ElementParser
{
	rgb_color Colors[8];

public:
	bool Start();
	bool End();

	XML_LandscapePrefsParser(): XML_ElementParser("landscapes") {}
};

bool XML_LandscapePrefsParser::Start()
{
	for (int i=0; i<4; i++)
		for (int j=0; j<2; j++)
			CopyColor(Colors[2*i+j],graphics_preferences->OGL_Configure.LscpColors[i][j]);
	
	Color_SetArray(Colors,8);
	
	return true;
}

bool XML_LandscapePrefsParser::End()
{
	for (int i=0; i<4; i++)
		for (int j=0; j<2; j++)
			CopyColor(graphics_preferences->OGL_Configure.LscpColors[i][j],Colors[2*i+j]);

	return true;
}

static XML_LandscapePrefsParser LandscapePrefsParser;


class XML_TexturePrefsParser: public XML_ElementParser
{
	bool IndexPresent, ValuesPresent[5];
	int16 Index, Values[5];
	
public:
	bool Start();
	bool HandleAttribute(const char *Tag, const char *Value);
	bool AttributesDone();

	XML_TexturePrefsParser(): XML_ElementParser("texture") {}
};

bool XML_TexturePrefsParser::Start()
{
	IndexPresent = false;
	for (int k=0; k<4; k++)
		ValuesPresent[k] = false;
	
	return true;
}

bool XML_TexturePrefsParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (StringsEqual(Tag,"index"))
	{
		if (ReadBoundedInt16Value(Value,Index,0,OGL_NUMBER_OF_TEXTURE_TYPES-1))
		{
			IndexPresent = true;
			return true;
		}
		else
			return false;
	}
	else if (StringsEqual(Tag,"near_filter"))
	{
		if (ReadInt16Value(Value,Values[0]))
		{
			ValuesPresent[0] = true;
			return true;
		}
		else
			return false;
	}
	else if (StringsEqual(Tag,"far_filter"))
	{
		if (ReadInt16Value(Value,Values[1]))
		{
			ValuesPresent[1] = true;
			return true;
		}
		else
			return false;
	}
	else if (StringsEqual(Tag,"resolution"))
	{
		if (ReadInt16Value(Value,Values[2]))
		{
			ValuesPresent[2] = true;
			return true;
		}
		else
			return false;
	}
	else if (StringsEqual(Tag,"color_format"))
	{
		if (ReadInt16Value(Value,Values[3]))
		{
			ValuesPresent[3] = true;
			return true;
		}
		else
			return false;
	}
	else if (StringsEqual(Tag, "max_size"))
	{
		if (ReadInt16Value(Value, Values[4]))
		{
			ValuesPresent[4] = true;
			return true;
		}
		else
			return false;
	}
	return true;
}


bool XML_TexturePrefsParser::AttributesDone()
{
	// Verify...
	if (!IndexPresent)
	{
		AttribsMissing();
		return false;
	}
	
	if (ValuesPresent[0])
		graphics_preferences->OGL_Configure.TxtrConfigList[Index].NearFilter = Values[0];
	
	if (ValuesPresent[1])
		graphics_preferences->OGL_Configure.TxtrConfigList[Index].FarFilter = Values[1];
	
	if (ValuesPresent[2])
		graphics_preferences->OGL_Configure.TxtrConfigList[Index].Resolution = Values[2];
	
	if (ValuesPresent[3])
		graphics_preferences->OGL_Configure.TxtrConfigList[Index].ColorFormat = Values[3];

	if (ValuesPresent[4])
		graphics_preferences->OGL_Configure.TxtrConfigList[Index].MaxSize = Values[4];
	
	return true;
}

static XML_TexturePrefsParser TexturePrefsParser;


class XML_GraphicsPrefsParser: public XML_ElementParser
{
public:
	bool HandleAttribute(const char *Tag, const char *Value);

	XML_GraphicsPrefsParser(): XML_ElementParser("graphics") {}
};

bool XML_GraphicsPrefsParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (StringsEqual(Tag,"scmode_size"))
	{
		return ReadInt16Value(Value,graphics_preferences->screen_mode.size);
	}
	else if (StringsEqual(Tag,"scmode_accel"))
	{
		return ReadInt16Value(Value,graphics_preferences->screen_mode.acceleration);
	}
	else if (StringsEqual(Tag,"scmode_highres"))
	{
		return ReadBooleanValue(Value,graphics_preferences->screen_mode.high_resolution);
	}
	else if (StringsEqual(Tag,"scmode_fullscreen"))
	{
		return ReadBooleanValue(Value,graphics_preferences->screen_mode.fullscreen);
	}
	else if (StringsEqual(Tag, "scmode_fill_the_screen"))
	{
#ifdef SDL
		const SDL_version *version = SDL_Linked_Version();
		if (SDL_VERSIONNUM(version->major, version->minor, version->patch) >= SDL_VERSIONNUM(1, 2, 10))
#endif
			return ReadBooleanValue(Value, graphics_preferences->screen_mode.fill_the_screen);
#ifdef SDL
		else
		{
			graphics_preferences->screen_mode.fill_the_screen = true;
			return true;
		}
#endif
	}
	else if (StringsEqual(Tag,"scmode_bitdepth"))
	{
		return ReadInt16Value(Value,graphics_preferences->screen_mode.bit_depth);
	}
	else if (StringsEqual(Tag,"scmode_gamma"))
	{
		return ReadInt16Value(Value,graphics_preferences->screen_mode.gamma_level);
	}
	else if (StringsEqual(Tag,"devspec_slot"))
	{
#ifdef mac
		return ReadInt16Value(Value,graphics_preferences->device_spec.slot);
#else
		return true;
#endif
	}
	else if (StringsEqual(Tag,"devspec_flags"))
	{
#ifdef mac
		return ReadInt16Value(Value,graphics_preferences->device_spec.flags);
#else
		return true;
#endif
	}
	else if (StringsEqual(Tag,"devspec_bitdepth"))
	{
#ifdef mac
		return ReadInt16Value(Value,graphics_preferences->device_spec.bit_depth);
#else
		return true;
#endif
	}
	else if (StringsEqual(Tag,"devspec_width"))
	{
#ifdef mac
		return ReadInt16Value(Value,graphics_preferences->device_spec.width);
#else
		return true;
#endif
	}
	else if (StringsEqual(Tag,"devspec_height"))
	{
#ifdef mac
		return ReadInt16Value(Value,graphics_preferences->device_spec.height);
#else
		return true;
#endif
	}
	else if (StringsEqual(Tag,"frequency"))
	{
#ifdef mac
		return ReadFloatValue(Value,graphics_preferences->refresh_frequency);
#else
		return true;
#endif
	}
	else if (StringsEqual(Tag,"ogl_flags"))
	{
		return ReadUInt16Value(Value,graphics_preferences->OGL_Configure.Flags);
	}
        else if (StringsEqual(Tag,"experimental_rendering"))
        {
                return ReadBooleanValue(Value,graphics_preferences->experimental_rendering);
        }
        else if (StringsEqual(Tag,"anisotropy_level"))
	  {
	    return ReadFloatValue(Value, graphics_preferences->OGL_Configure.AnisotropyLevel);
	  }
	else if (StringsEqual(Tag,"multisamples"))
	  {
	    return ReadInt16Value(Value, graphics_preferences->OGL_Configure.Multisamples);
	  }
	else if (StringsEqual(Tag,"geforce_fix"))
	{
		return ReadBooleanValue(Value, graphics_preferences->OGL_Configure.GeForceFix);
	}
	else if (StringsEqual(Tag,"double_corpse_limit"))
	  {
	    return ReadBooleanValue(Value,graphics_preferences->double_corpse_limit);
	  }
	return true;
}

static XML_GraphicsPrefsParser GraphicsPrefsParser;



class XML_ChaseCamPrefsParser: public XML_ElementParser
{
public:
	bool HandleAttribute(const char *Tag, const char *Value);

	XML_ChaseCamPrefsParser(): XML_ElementParser("chase_cam") {}
};

bool XML_ChaseCamPrefsParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (StringsEqual(Tag,"behind"))
	{
		return ReadInt16Value(Value,player_preferences->ChaseCam.Behind);
	}
	else if (StringsEqual(Tag,"upward"))
	{
		return ReadInt16Value(Value,player_preferences->ChaseCam.Upward);
	}
	else if (StringsEqual(Tag,"rightward"))
	{
		return ReadInt16Value(Value,player_preferences->ChaseCam.Rightward);
	}
	else if (StringsEqual(Tag,"flags"))
	{
		return ReadInt16Value(Value,player_preferences->ChaseCam.Flags);
	}
	else if (StringsEqual(Tag,"damping"))
	{
		return ReadFloatValue(Value,player_preferences->ChaseCam.Damping);
	}
	else if (StringsEqual(Tag,"spring"))
	{
		return ReadFloatValue(Value,player_preferences->ChaseCam.Spring);
	}
	else if (StringsEqual(Tag,"opacity"))
	{
		return ReadFloatValue(Value,player_preferences->ChaseCam.Opacity);
	}
	return true;
}

static XML_ChaseCamPrefsParser ChaseCamPrefsParser;


class XML_CrosshairsPrefsParser: public XML_ElementParser
{
	rgb_color Color;

public:
	bool Start();
	bool HandleAttribute(const char *Tag, const char *Value);
	bool End();

	XML_CrosshairsPrefsParser(): XML_ElementParser("crosshairs") {}
};

bool XML_CrosshairsPrefsParser::Start()
{
	CopyColor(Color,player_preferences->Crosshairs.Color);
	Color_SetArray(&Color);

	return true;
}

bool XML_CrosshairsPrefsParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (StringsEqual(Tag,"thickness"))
	{
		return ReadInt16Value(Value,player_preferences->Crosshairs.Thickness);
	}
	else if (StringsEqual(Tag,"from_center"))
	{
		return ReadInt16Value(Value,player_preferences->Crosshairs.FromCenter);
	}
	else if (StringsEqual(Tag,"length"))
	{
		return ReadInt16Value(Value,player_preferences->Crosshairs.Length);
	}
	else if (StringsEqual(Tag,"shape"))
	{
		return ReadInt16Value(Value,player_preferences->Crosshairs.Shape);
	}
	else if (StringsEqual(Tag,"opacity"))
	{
		return ReadFloatValue(Value,player_preferences->Crosshairs.Opacity);
	}
	return true;
}

bool XML_CrosshairsPrefsParser::End()
{
	CopyColor(player_preferences->Crosshairs.Color,Color);

	return true;
}

static XML_CrosshairsPrefsParser CrosshairsPrefsParser;


class XML_PlayerPrefsParser: public XML_ElementParser
{
public:
	bool HandleAttribute(const char *Tag, const char *Value);

	XML_PlayerPrefsParser(): XML_ElementParser("player") {}
};

bool XML_PlayerPrefsParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (StringsEqual(Tag,"name"))
	{
		// Copy in as Pascal string
		DeUTF8_Pas(Value,strlen(Value),player_preferences->name,PREFERENCES_NAME_LENGTH);
		return true;
	}
	else if (StringsEqual(Tag,"color"))
	{
		return ReadInt16Value(Value,player_preferences->color);
	}
	else if (StringsEqual(Tag,"team"))
	{
		return ReadInt16Value(Value,player_preferences->team);
	}
	else if (StringsEqual(Tag,"last_time_ran"))
	{
		return ReadUInt32Value(Value,player_preferences->last_time_ran);
	}
	else if (StringsEqual(Tag,"difficulty"))
	{
		return ReadInt16Value(Value,player_preferences->difficulty_level);
	}
	else if (StringsEqual(Tag,"bkgd_music"))
	{
		return ReadBooleanValue(Value,player_preferences->background_music_on);
	}
	return true;
}

static XML_PlayerPrefsParser PlayerPrefsParser;


class XML_MouseButtonPrefsParser: public XML_ElementParser
{
	bool IndexPresent, ActionPresent;
	int16 Index, Action;
	
public:
	bool Start();
	bool HandleAttribute(const char *Tag, const char *Value);
	bool AttributesDone();

	XML_MouseButtonPrefsParser(const char *_Name): XML_ElementParser(_Name) {}
};

bool XML_MouseButtonPrefsParser::Start()
{
	IndexPresent = ActionPresent = false;
	
	return true;
}

bool XML_MouseButtonPrefsParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (StringsEqual(Tag,"index"))
	{
		if (ReadBoundedInt16Value(Value,Index,0,MAX_BUTTONS-1))
		{
			IndexPresent = true;
			return true;
		}
		else return false;
	}
	else if (StringsEqual(Tag,"action"))
	{
		if (StringsEqual(Value, "none"))
		{
			Action = _mouse_button_does_nothing;
			ActionPresent = true;
			return true;
		}
		else if (StringsEqual(Value, "left_trigger"))
		{
			Action = _mouse_button_fires_left_trigger;
			ActionPresent = true;
			return true;
		}
		else if (StringsEqual(Value, "right_trigger"))
		{
			Action = _mouse_button_fires_right_trigger;
			ActionPresent = true;
			return true;
		}
		else return false;
	}
	return true;
}

bool XML_MouseButtonPrefsParser::AttributesDone()
{
	// Verify...
	if (!(IndexPresent && ActionPresent))
	{
		AttribsMissing();
		return false;
	}
	
	input_preferences->mouse_button_actions[Index] = Action;
			
	return true;
}

static XML_MouseButtonPrefsParser MouseButtonPrefsParser("mouse_button");


class XML_KeyPrefsParser: public XML_ElementParser
{
	bool IndexPresent, KeyValPresent;
	int16 Index, KeyVal;
	
public:
	bool Start();
	bool HandleAttribute(const char *Tag, const char *Value);
	bool AttributesDone();

	XML_KeyPrefsParser(const char *_Name): XML_ElementParser(_Name) {}
};

bool XML_KeyPrefsParser::Start()
{
	IndexPresent = KeyValPresent = false;
	
	return true;
}

bool XML_KeyPrefsParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (StringsEqual(Tag,"index"))
	{
		if (ReadBoundedInt16Value(Value,Index,0,NUMBER_OF_KEYS-1))
		{
			IndexPresent = true;
			return true;
		}
		else return false;
	}
	if (StringsEqual(Tag,"value"))
	{
		if (ReadInt16Value(Value,KeyVal))
		{
			KeyValPresent = true;
			return true;
		}
		else return false;
	}
	return true;
}

bool XML_KeyPrefsParser::AttributesDone()
{
	// Verify...
	if (!(IndexPresent && KeyValPresent))
	{
		AttribsMissing();
		return false;
	}
	
	input_preferences->keycodes[Index] = KeyVal;
			
	return true;
}


// This compilation trick guarantees that both Mac and SDL versions will ignore the other's
// key values; for each platform, the parser of the other platform's key values
// is a dummy parser.
#if defined(mac)
static XML_KeyPrefsParser MacKeyPrefsParser("mac_key");
static XML_ElementParser SDLKeyPrefsParser("sdl_key");
#elif defined(SDL)
static XML_ElementParser MacKeyPrefsParser("mac_key");
static XML_KeyPrefsParser SDLKeyPrefsParser("sdl_key");
#endif


class XML_InputPrefsParser: public XML_ElementParser
{
public:
	bool HandleAttribute(const char *Tag, const char *Value);

	XML_InputPrefsParser(): XML_ElementParser("input") {}
};

bool XML_InputPrefsParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (StringsEqual(Tag,"device"))
	{
		return ReadInt16Value(Value,input_preferences->input_device);
	}
	else if (StringsEqual(Tag,"modifiers"))
	{
		return ReadUInt16Value(Value,input_preferences->modifiers);
	}
	// ZZZ: sensitivity scaling factor
	// LP addition: split into separate horizontal and vertical sensitivities
	else if (StringsEqual(Tag, "sensitivity"))
	{
		_fixed sensitivity;
		if (ReadInt32Value(Value, sensitivity))
		{
			input_preferences->sens_horizontal = sensitivity;
			input_preferences->sens_vertical = sensitivity;
			return true;
		}
        else
        	return false;
	}
	else if (StringsEqual(Tag, "sens_horizontal"))
	{
		return ReadInt32Value(Value, input_preferences->sens_horizontal);
	}
	else if (StringsEqual(Tag, "sens_vertical"))
	{
		return ReadInt32Value(Value, input_preferences->sens_vertical);
	}
	else if(StringsEqual(Tag, "mouse_acceleration")) {
		return ReadBooleanValue(Value, input_preferences->mouse_acceleration);
	}
	return true;
}

static XML_InputPrefsParser InputPrefsParser;


class XML_SoundPrefsParser: public XML_ElementParser
{
public:
	bool HandleAttribute(const char *Tag, const char *Value);

	XML_SoundPrefsParser(): XML_ElementParser("sound") {}
};

bool XML_SoundPrefsParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (StringsEqual(Tag,"channels"))
	{
		return ReadInt16Value(Value,sound_preferences->channel_count);
	}
	else if (StringsEqual(Tag,"volume"))
	{
		return ReadInt16Value(Value,sound_preferences->volume);
	}
	else if (StringsEqual(Tag,"music_volume"))
	{
		return ReadInt16Value(Value,sound_preferences->music);
	}
	else if (StringsEqual(Tag,"flags"))
	{
		return ReadUInt16Value(Value,sound_preferences->flags);
	}
	return true;
}

static XML_SoundPrefsParser SoundPrefsParser;

class XML_NetworkPrefsParser: public XML_ElementParser
{
public:
	bool HandleAttribute(const char *Tag, const char *Value);

	XML_NetworkPrefsParser(): XML_ElementParser("network") {}
};

bool XML_NetworkPrefsParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (StringsEqual(Tag,"microphone"))
	{
		return ReadBooleanValue(Value,network_preferences->allow_microphone);
	}
	else if (StringsEqual(Tag,"untimed"))
	{
		return ReadBooleanValue(Value,network_preferences->game_is_untimed);
	}
	else if (StringsEqual(Tag,"type"))
	{
		return ReadInt16Value(Value,network_preferences->type);
	}
	else if (StringsEqual(Tag,"game_type"))
	{
		return ReadInt16Value(Value,network_preferences->game_type);
	}
	else if (StringsEqual(Tag,"difficulty"))
	{
		return ReadInt16Value(Value,network_preferences->difficulty_level);
	}
	else if (StringsEqual(Tag,"game_options"))
	{
		return ReadUInt16Value(Value,network_preferences->game_options);
	}
	else if (StringsEqual(Tag,"time_limit"))
	{
		return ReadInt32Value(Value,network_preferences->time_limit);
	}
	else if (StringsEqual(Tag,"kill_limit"))
	{
		return ReadInt16Value(Value,network_preferences->kill_limit);
	}
	else if (StringsEqual(Tag,"entry_point"))
	{
		return ReadInt16Value(Value,network_preferences->entry_point);
	}
        else if (StringsEqual(Tag,"autogather"))
        {
                return ReadBooleanValue(Value,network_preferences->autogather);
        }
        else if (StringsEqual(Tag,"join_by_address"))
        {
                return ReadBooleanValue(Value,network_preferences->join_by_address);
        }
        else if (StringsEqual(Tag,"join_address"))
        {
                DeUTF8_C(Value,strlen(Value),network_preferences->join_address,255);
                return true;
        }
        else if (StringsEqual(Tag,"local_game_port"))
        {
                return ReadUInt16Value(Value,network_preferences->game_port);
        }
	else if (StringsEqual(Tag,"game_protocol"))
	{
		size_t i;
		for(i = 0; i < NUMBER_OF_NETWORK_GAME_PROTOCOL_NAMES; i++)
		{
			if(StringsEqual(Value,sNetworkGameProtocolNames[i]))
				break;
		}
		if(i < NUMBER_OF_NETWORK_GAME_PROTOCOL_NAMES)
		{
			network_preferences->game_protocol= i;
			return true;
		}
		else
			return false;
	}
	else if (StringsEqual(Tag,"use_speex_netmic_encoder"))
	{
		return ReadBooleanValue(Value,network_preferences->use_speex_encoder);
	}
	else if (StringsEqual(Tag,"speex_encoder_quality"))
	{
		return ReadUInt16Value(Value,network_preferences->speex_encoder_quality);
	}
	else if (StringsEqual(Tag,"speex_encoder_complexity"))
	{
		return ReadUInt16Value(Value,network_preferences->speex_encoder_complexity);
	}
	else if (StringsEqual(Tag,"use_netscript"))
	{
		return ReadBooleanValue(Value,network_preferences->use_netscript);
	}
	else if (StringsEqual(Tag,"netscript_file"))
	{
#ifdef SDL
		DeUTF8_C(Value,strlen(Value),network_preferences->netscript_file,sizeof(network_preferences->netscript_file));
#endif
		return true;
	}
	else if (StringsEqual(Tag,"cheat_flags"))
        {
		return ReadUInt16Value(Value,network_preferences->cheat_flags);
	}
	else if (StringsEqual(Tag,"advertise_on_metaserver"))
	{
		return ReadBooleanValue(Value,network_preferences->advertise_on_metaserver);
	} 
	else if (StringsEqual(Tag,"attempt_upnp"))
	{
		return ReadBooleanValue(Value, network_preferences->attempt_upnp);
	}
	return true;
}

static XML_NetworkPrefsParser NetworkPrefsParser;


// Make this child-entity class a dummy class for SDL
#ifdef mac

class XML_MacFSSpecPrefsParser: public XML_ElementParser
{
	FSSpec Spec;
	int32 Index;
	// Four attributes: index, vRefNum, parID, name, which must be present
	bool IsPresent[4];

public:
	bool Start();
	bool HandleAttribute(const char *Tag, const char *Value);
	bool AttributesDone();

	XML_MacFSSpecPrefsParser(): XML_ElementParser("mac_fsspec") {}
};

bool XML_MacFSSpecPrefsParser::Start()
{
	// Initially, no attributes are seen
	for (int k=0; k<4; k++)
		IsPresent[k] = false;
	
	return true;
}


bool XML_MacFSSpecPrefsParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (StringsEqual(Tag,"index"))
	{
		if (ReadInt32Value(Value,Index))
		{
			IsPresent[0] = true;
			return true;
		}
		else
			return false;
	}
	else if (StringsEqual(Tag,"v_ref_num"))
	{
		if (ReadInt16Value(Value,Spec.vRefNum))
		{
			IsPresent[1] = true;
			return true;
		}
		else
			return false;
	}
	else if (StringsEqual(Tag,"par_id"))
	{
		if (ReadInt32Value(Value,Spec.parID))
		{
			IsPresent[2] = true;
			return true;
		}
		else
			return false;
	}
	else if (StringsEqual(Tag,"name"))
	{
		// Copy in as Pascal string (Classic: length 31; Carbon: length ?)
		DeUTF8_Pas(Value,strlen(Value),Spec.name,31);
		
		IsPresent[3] = true;
		return true;
	}
	return true;
}

bool XML_MacFSSpecPrefsParser::AttributesDone()
{
	// Verify ...
	// All four attributes (index, vRefNum, parID, and name) must be present
	for (int k=0; k<4; k++)
	{
		if (!IsPresent[k])
		{
			AttribsMissing();
			return false;
		}
	}

	if(Index < kNumberOfFileSpecIndices && Index >= 0)
		*(sFileSpecIndexToFileSpecPtr[Index]) = Spec;
	
	return true;
}

static XML_MacFSSpecPrefsParser MacFSSpecPrefsParser;

#else

static XML_ElementParser MacFSSpecPrefsParser("mac_fsspec");

#endif


class XML_EnvironmentPrefsParser: public XML_ElementParser
{
public:
	bool HandleAttribute(const char *Tag, const char *Value);

	XML_EnvironmentPrefsParser(): XML_ElementParser("environment") {}
};

bool XML_EnvironmentPrefsParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (StringsEqual(Tag,"map_file"))
	{
#ifdef SDL
		DeUTF8_C(Value,strlen(Value),environment_preferences->map_file,255);
#endif
		return true;
	}
	else if (StringsEqual(Tag,"physics_file"))
	{
#ifdef SDL
		DeUTF8_C(Value,strlen(Value),environment_preferences->physics_file,255);
#endif
		return true;
	}
	else if (StringsEqual(Tag,"shapes_file"))
	{
#ifdef SDL
		DeUTF8_C(Value,strlen(Value),environment_preferences->shapes_file,255);
#endif
		return true;
	}
	else if (StringsEqual(Tag,"sounds_file"))
	{
#ifdef SDL
		DeUTF8_C(Value,strlen(Value),environment_preferences->sounds_file,255);
#endif
		return true;
	}
	else if (StringsEqual(Tag,"theme_dir"))
	{
#ifdef SDL
		DeUTF8_C(Value,strlen(Value),environment_preferences->theme_dir,255);
#if defined(__APPLE__) && defined(__MACH__)
		extern char *bundle_name; // SDLMain.m
		// replace leading "AlephOneSDL.app" with our actual bundle name (we do reverse when saving)
		if (!strncmp(environment_preferences->theme_dir, "AlephOneSDL.app", 15)) {
			strlcpy(temporary, bundle_name, sizeof(temporary));
			strlcat(temporary, environment_preferences->theme_dir + 15, sizeof(temporary));
			strlcpy(environment_preferences->theme_dir, temporary, 255);			
		}
#endif
#endif
		return true;
	}
	else if (StringsEqual(Tag,"map_checksum"))
	{
		return ReadUInt32Value(Value,environment_preferences->map_checksum);
	}
	else if (StringsEqual(Tag,"physics_checksum"))
	{
		return ReadUInt32Value(Value,environment_preferences->physics_checksum);
	}
	else if (StringsEqual(Tag,"shapes_mod_date"))
	{
		uint32 ModDate;
		if (ReadUInt32Value(Value,ModDate))
		{
			environment_preferences->shapes_mod_date = TimeType(ModDate);
			return true;
		}
		else
			return false;
	}
	else if (StringsEqual(Tag,"sounds_mod_date"))
	{
		uint32 ModDate;
		if (ReadUInt32Value(Value,ModDate))
		{
			environment_preferences->sounds_mod_date = TimeType(ModDate);
			return true;
		}
		else
			return false;
	}
	else if (StringsEqual(Tag,"group_by_directory"))
	{
		return ReadBooleanValue(Value,environment_preferences->group_by_directory);
	}
	else if (StringsEqual(Tag,"reduce_singletons"))
	{
		return ReadBooleanValue(Value,environment_preferences->reduce_singletons);
	}
	else if (StringsEqual(Tag,"non_bungie_warning"))
	{
		return ReadBooleanValue(Value,environment_preferences->non_bungie_warning);
	}
	return true;
}

static XML_EnvironmentPrefsParser EnvironmentPrefsParser;



void SetupPrefsParseTree()
{
	// Add the root object here
	PrefsRootParser.AddChild(&MarathonPrefsParser);
	
	// Add all the others
	
	VoidPrefsParser.AddChild(Color_GetParser());
	GraphicsPrefsParser.AddChild(&VoidPrefsParser);
	LandscapePrefsParser.AddChild(Color_GetParser());
	GraphicsPrefsParser.AddChild(&LandscapePrefsParser);
	GraphicsPrefsParser.AddChild(&TexturePrefsParser);
	MarathonPrefsParser.AddChild(&GraphicsPrefsParser);
	
	PlayerPrefsParser.AddChild(&ChaseCamPrefsParser);
	CrosshairsPrefsParser.AddChild(Color_GetParser());
	PlayerPrefsParser.AddChild(&CrosshairsPrefsParser);
	MarathonPrefsParser.AddChild(&PlayerPrefsParser);

	InputPrefsParser.AddChild(&MouseButtonPrefsParser);
	InputPrefsParser.AddChild(&MacKeyPrefsParser);
	InputPrefsParser.AddChild(&SDLKeyPrefsParser);
	MarathonPrefsParser.AddChild(&InputPrefsParser);
	
	MarathonPrefsParser.AddChild(&SoundPrefsParser);

#if !defined(DISABLE_NETWORKING)
	NetworkPrefsParser.AddChild(StarGameProtocol::GetParser());
	NetworkPrefsParser.AddChild(RingGameProtocol::GetParser());
#endif // !defined(DISABLE_NETWORKING)
	NetworkPrefsParser.AddChild(&MacFSSpecPrefsParser);
	MarathonPrefsParser.AddChild(&NetworkPrefsParser);
	
	EnvironmentPrefsParser.AddChild(&MacFSSpecPrefsParser);
	MarathonPrefsParser.AddChild(&EnvironmentPrefsParser);
}
