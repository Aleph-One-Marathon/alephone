/*
	preferences_macintosh.c
	
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
	
	Tuesday, June 13, 1995 10:02:50 AM- rdm created.

Jan 30, 2000 (Loren Petrich):
	Added some typecasts

Feb 4, 2000 (Loren Petrich):
	Changed halt() to assert(false) for better debugging

Feb 6, 2000 (Loren Petrich):
	Changed switch/case to if/then/elseif/else for typecodes

Feb 10, 2000 (Loren Petrich):
	Added code to do run/walk and swim/sink switching

Feb 11, 2000 (Loren Petrich):
	Made directory searches recursive;
	also increased MAXIMUM_FIND_FILES to 128

Feb 12, 2000 (Loren Petrich):
	Changed run/walk interchange default to on;
	swim/sink interchange default remains off.

Feb 25, 2000 (Loren Petrich):
	Set up dialog boxes for handling the chase-cam and crosshair parameters

Mar 19, 2000 (Loren Petrich):
	Added OpenGL support

Apr 27, 2000 (Loren Petrich):
	Added Josh Elsasser's "don't switch weapons" patch

May 2, 2000 (Loren Petrich):
	Eliminated "draw every other line" dialog-box option (already grayed out for PowerPC).

May 3, 2000 (Loren Petrich):
	Added Simon Brownlee's patch for handling the "hardware acceleration" button

Jul 7, 2000 (Loren Petrich): Added Ben Thompson's InputSprocket support

Aug 12, 2000 (Loren Petrich):
	Using object-oriented file handler

Aug 25, 2000 (Loren Petrich)
	Confined searches to the app's directory

Oct 14, 2000 (Loren Petrich):
	Added music-volume stuff

Sept 9, 2001 (Loren Petrich):
	Modified the checkbox code to be much more reasonable.
	Also added Ian Rickard's smart-dialog processing.
	And added a routine for setting preferences to single files
	if only single files of some type are present (idiot-proofing)

Jan 25, 2002 (Br'fin (Jeremy Parsons)):
	Added TARGET_API_MAC_CARBON for CoreServices.h
	Attempted to get current user name for Carbon
	Added accessors for datafields now opaque in Carbon
	Disabled ethernet check for Carbon
	Attempting to configure ISP controls now asserts

Jan 25, 2002 (Br'fin (Jeremy Parsons)):
	Included Bytnar inspired changes to disable InputSprocket controls under Carbon

Feb 5, 2002 (Br'fin (Jeremy Parsons)):
	Fixed non-display of network player name in Player Dialog

April 19, 2003 (Woody Zenfell):
        Environment prefs now can show many more items; elements labelled by containing folder
*/

#if defined(EXPLICIT_CARBON_HEADER)
    #include <CoreServices/CoreServices.h>
#endif

#ifdef env68k
	#pragma segment dialogs
#endif

#include <map>
#include <vector>
#include <algorithm>

using std::map;
using std::vector;
using std::sort;

// ZZZ: moved constants to preferences.h for sharing between versions

/* ----------- private prototypes */
static void setup_graphics_dialog(DialogPtr dialog, short first_item, void *prefs);
static void hit_graphics_item(DialogPtr dialog, short first_item, void *prefs, short item_hit);
static bool teardown_graphics_dialog(DialogPtr dialog, short first_item, void *prefs);
static void setup_player_dialog(DialogPtr dialog, short first_item, void *prefs);
static void hit_player_item(DialogPtr dialog, short first_item, void *prefs, short item_hit);
static bool teardown_player_dialog(DialogPtr dialog, short first_item, void *prefs);
static void setup_sound_dialog(DialogPtr dialog, short first_item, void *prefs);
static void hit_sound_item(DialogPtr dialog, short first_item, void *prefs, short item_hit);
static bool teardown_sound_dialog(DialogPtr dialog, short first_item, void *prefs);
static void setup_input_dialog(DialogPtr dialog, short first_item, void *prefs);
static void hit_input_item(DialogPtr dialog, short first_item, void *prefs, short item_hit);
static bool teardown_input_dialog(DialogPtr dialog, short first_item, void *prefs);
static void set_popup_enabled_state(DialogPtr dialog, short item_number, short item_to_affect,
	bool enabled);
static void setup_environment_dialog(DialogPtr dialog, short first_item, void *prefs);
static void hit_environment_item(DialogPtr dialog, short first_item, void *prefs, short item_hit);
static bool teardown_environment_dialog(DialogPtr dialog, short first_item, void *prefs);
static void fill_in_popup_with_filetype(DialogPtr dialog, short item, int type, unsigned long checksum, const FSSpec& file);
static void fill_in_popup_with_filetype(ControlHandle control, int type, unsigned long checksum, const FSSpec& file);
#ifndef TARGET_API_MAC_CARBON
static MenuHandle get_popup_menu_handle(DialogPtr dialog, short item);
#endif
static bool allocate_extensions_memory(void);
static void free_extensions_memory(void);
static void build_extensions_list(void);
static void search_from_directory(DirectorySpecifier& BaseDir);
static unsigned long find_checksum_and_file_spec_from_dialog(DialogPtr dialog, 
	short item_hit, uint32 type, FSSpec *file);
static unsigned long find_checksum_and_file_spec_from_control(ControlHandle control,
	uint32 type, FSSpec *file);
static void SetToLoneFile(Typecode Type, FSSpec& File, unsigned long& Checksum);

/* ---------------- code */

struct preferences_dialog_data prefs_data[]={
	{ strPREFERENCES_GROUPS, graphicsGroup, ditlGRAPHICS, get_graphics_pref_data, setup_graphics_dialog, hit_graphics_item, teardown_graphics_dialog },
	{ strPREFERENCES_GROUPS, playerGroup, ditlPLAYER, get_player_pref_data, setup_player_dialog, hit_player_item, teardown_player_dialog },
	{ strPREFERENCES_GROUPS, soundGroup, ditlSOUND, get_sound_pref_data, setup_sound_dialog, hit_sound_item, teardown_sound_dialog },
	{ strPREFERENCES_GROUPS, inputGroup, ditlINPUT, get_input_pref_data, setup_input_dialog, hit_input_item, teardown_input_dialog },
#ifndef DEMO
	{ strPREFERENCES_GROUPS, environmentGroup, ditlENVIRONMENT, get_environment_pref_data, setup_environment_dialog, hit_environment_item, teardown_environment_dialog }
#endif
};
#define NUMBER_OF_PREFS_PANELS (sizeof(prefs_data)/sizeof(struct preferences_dialog_data))

static bool physics_valid= true;

#ifdef USES_NIBS


enum {
	_hundreds_colors_menu_item= 1,
	_thousands_colors_menu_item,
	_millions_colors_menu_item,
	_billions_colors_menu_item
};


// Global variable since this will be persistent
// Everything else is local, because they get re-created
static int CurrentPrefsPane = graphicsGroup;


// These map between a slider's 0-1 range and the sensitivity values

const float SliderSensRange = 4;

static float PrefToSlider(_fixed PrefVal)
{
	float CenteredSliderVal = (PrefVal > 0) ?
		(log(float(PrefVal)) - log(FIXED_ONE))/log(SliderSensRange) :
		0;
	
	// -1 to 1 -> 0 to 1
	return (CenteredSliderVal+1)/2;
}

static _fixed SliderToPref(float SliderVal)
{
	return _fixed(FIXED_ONE * pow(SliderSensRange, 2*SliderVal-1));
}


struct PreferencesHandlerData
{
	WindowPtr Window;
	ControlRef TabCtrl;
	ControlRef PaneCtrls[NUMBER_OF_PREFS_GROUPS];
};


static void UpdatePanes(PreferencesHandlerData *HDPtr)
{
	// Go from 1-based to 0-based (in the preferences_private defs)
	int WhichPane = GetControl32BitValue(HDPtr->TabCtrl) - 1;
	
	// If no need to change the pane, then quit
	if (WhichPane == CurrentPrefsPane) return;
	
	// Otherwise, update the current pane
	CurrentPrefsPane = WhichPane;
	
	// Hide every pane but the current one
	for (int k=0; k<NUMBER_OF_PREFS_GROUPS; k++)
	{
		if (k != WhichPane)
			SetControlVisibility(HDPtr->PaneCtrls[k],false,true);
	}
	SetControlVisibility(HDPtr->PaneCtrls[WhichPane],true,true);
	
	ClearKeyboardFocus(HDPtr->Window);
	Draw1Control(HDPtr->TabCtrl);
}


static void PreferencesHandler(ParsedControl &Ctrl, void *UserData)
{
	PreferencesHandlerData *HDPtr = (PreferencesHandlerData *)(UserData);
	
	switch(Ctrl.ID.signature)
	{
	case 0:
		
		if (Ctrl.ID.id == iTABS)
		{
			UpdatePanes(HDPtr);
		}
		break;
		
	case Sig_Graphics:
	
		switch(Ctrl.ID.id)
		{
		case iCHOOSE_MONITOR:
			display_device_dialog(&graphics_preferences->device_spec);
			break;
			
		case iOPENGL_OPTIONS:
			OGL_ConfigureDialog(graphics_preferences->OGL_Configure);
			break; 
		}
		break;
	
	case Sig_Sound:
	
		switch(Ctrl.ID.id)
		{
		case iVOLUME:
			sound_preferences->volume = GetControl32BitValue(Ctrl.Ctrl) - 1;
			test_sound_volume(sound_preferences->volume, Sound_AdjustVolume());
			set_sound_manager_parameters(sound_preferences);
			break;
			
		case iMUSIC:
			sound_preferences->music = GetControl32BitValue(Ctrl.Ctrl) - 1;
			set_sound_manager_parameters(sound_preferences);
			music_idle_proc();
			break;
			
		case iRELATIVE_VOLUME:
			SET_FLAG(sound_preferences->flags, _relative_volume_flag, GetControl32BitValue(Ctrl.Ctrl));
			test_sound_volume(sound_preferences->volume, Sound_AdjustVolume());
			set_sound_manager_parameters(sound_preferences);
			break;
		}
		break;
	
	case Sig_Player:
	
		switch(Ctrl.ID.id)
		{
		case iCHASE_CAM:
			Configure_ChaseCam(player_preferences->ChaseCam);
			break;
			
		case iCROSSHAIRS:
			Configure_Crosshairs(player_preferences->Crosshairs);
			break;
		}
		break;
	
	case Sig_Input:
	
		switch(Ctrl.ID.id)
		{
		case iSET_KEYS:
			{
				short key_codes[NUMBER_OF_KEYS];
				
				objlist_copy(key_codes, input_preferences->keycodes, NUMBER_OF_KEYS);
				if(configure_key_setup(key_codes))
				{
					objlist_copy(input_preferences->keycodes, key_codes, NUMBER_OF_KEYS);
					set_keys(key_codes);
				}
			}
		}
	}
}


void handle_preferences(
	void)
{
	/* Save the existing preferences, in case we have to reload them. */
	write_preferences();
	
	// Get the window
	AutoNibWindow Window(GUI_Nib,Window_Preferences);
	
	// Get the desired controls
	PreferencesHandlerData HandlerData;
	
	// Get all the control objects
	HandlerData.Window = Window();
	HandlerData.TabCtrl = GetCtrlFromWindow(Window(),0,iTABS);
	for (int k=0; k<NUMBER_OF_PREFS_GROUPS; k++)
		HandlerData.PaneCtrls[k] = GetCtrlFromWindow(Window(),Sig_Pane,k);

	// Value of control:
	// Checkbox: 0 or 1
	// Popup: 1, 2, 3, ...
	int value;
		
	// Graphics:
	
	// IR-inspired: force to some reasonable value:
	if (graphics_preferences->screen_mode.bit_depth >= 32)
	{
		graphics_preferences->screen_mode.bit_depth = 32;
		value = _millions_colors_menu_item;
	}
	else if (graphics_preferences->screen_mode.bit_depth >= 16)
	{
		graphics_preferences->screen_mode.bit_depth = 16;
		value = _thousands_colors_menu_item;
	}
	else
	{
		graphics_preferences->screen_mode.bit_depth = 8;
		value = _hundreds_colors_menu_item;
		// IR-inspired change; no OpenGL mode in 8-bit color depth
		graphics_preferences->screen_mode.acceleration= _no_acceleration;
	}
	
	ControlRef GRFX_Number_of_Colors = GetCtrlFromWindow(Window(),Sig_Graphics,iNUMBER_OF_COLORS);
	SetControl32BitValue(GRFX_Number_of_Colors,value);

	// No more special 68K support either
	graphics_preferences->screen_mode.draw_every_other_line= false;

	ControlRef GRFX_Window_Size = GetCtrlFromWindow(Window(),Sig_Graphics,iWINDOW_SIZE);
	SetControl32BitValue(GRFX_Window_Size,graphics_preferences->screen_mode.size+1);

	ControlRef GRFX_Detail = GetCtrlFromWindow(Window(),Sig_Graphics,iDETAIL);
	SetControl32BitValue(GRFX_Detail,graphics_preferences->screen_mode.high_resolution ? 1 : 2);

	ControlRef GRFX_Brightness = GetCtrlFromWindow(Window(),Sig_Graphics,iBRIGHTNESS);
	SetControl32BitValue(GRFX_Brightness,graphics_preferences->screen_mode.gamma_level+1);
	
	ControlRef GRFX_Acceleration = GetCtrlFromWindow(Window(),Sig_Graphics,iHARDWARE_ACCELERATION);
	SetControl32BitValue(GRFX_Acceleration,graphics_preferences->screen_mode.acceleration == _opengl_acceleration ? 1 : 0);
	SetControlActivity(GRFX_Acceleration, OGL_IsPresent());
	
	ControlRef GRFX_FillScreen = GetCtrlFromWindow(Window(),Sig_Graphics,iFILL_SCREEN);
	SetControl32BitValue(GRFX_FillScreen,graphics_preferences->screen_mode.fullscreen);
	
	// Sound:
	
	ControlRef SNDS_Volume = GetCtrlFromWindow(Window(),Sig_Sound,iVOLUME);
	SetControl32BitValue(SNDS_Volume,sound_preferences->volume+1);

	ControlRef SNDS_Channels = GetCtrlFromWindow(Window(),Sig_Sound,iCHANNELS);
	SetControl32BitValue(SNDS_Channels,sound_preferences->channel_count);

	ControlRef SNDS_Music = GetCtrlFromWindow(Window(),Sig_Sound,iMUSIC);
	SetControl32BitValue(SNDS_Music,sound_preferences->music+1);
	
	ControlRef SNDS_Stereo = GetCtrlFromWindow(Window(),Sig_Sound,iSTEREO);
	SetControl32BitValue(SNDS_Stereo,TEST_FLAG(sound_preferences->flags,_stereo_flag));
	
	if (!TEST_FLAG(sound_preferences->flags,_stereo_flag))
		SET_FLAG(sound_preferences->flags,_dynamic_tracking_flag,false);

	ControlRef SNDS_Panning = GetCtrlFromWindow(Window(),Sig_Sound,iACTIVE_PANNING);
	SetControl32BitValue(SNDS_Panning,TEST_FLAG(sound_preferences->flags,_dynamic_tracking_flag));

	ControlRef SNDS_16Bit = GetCtrlFromWindow(Window(),Sig_Sound,iHIGH_QUALITY);
	SetControl32BitValue(SNDS_16Bit,TEST_FLAG(sound_preferences->flags,_16bit_sound_flag));

	ControlRef SNDS_Ambient = GetCtrlFromWindow(Window(),Sig_Sound,iAMBIENT_SOUND);
	SetControl32BitValue(SNDS_Ambient,TEST_FLAG(sound_preferences->flags,_ambient_sound_flag));

	ControlRef SNDS_More = GetCtrlFromWindow(Window(),Sig_Sound,iMORE_SOUNDS);
	SetControl32BitValue(SNDS_More,TEST_FLAG(sound_preferences->flags,_more_sounds_flag));

	ControlRef SNDS_Relative_Volume = GetCtrlFromWindow(Window(),Sig_Sound,iRELATIVE_VOLUME);
	SetControl32BitValue(SNDS_Relative_Volume,TEST_FLAG(sound_preferences->flags,_relative_volume_flag));
	
	// Player:
	
	ControlRef PLYR_Difficulty = GetCtrlFromWindow(Window(),Sig_Player,iDIFFICULTY_LEVEL);
	SetControl32BitValue(PLYR_Difficulty,player_preferences->difficulty_level+1);
	
	ControlRef PLYR_Name = GetCtrlFromWindow(Window(),Sig_Player,iNAME);
	SetEditPascalText(PLYR_Name, player_preferences->name);

	ControlRef PLYR_Color = GetCtrlFromWindow(Window(),Sig_Player,iCOLOR);
	SetControl32BitValue(PLYR_Color,player_preferences->color+1);

	ControlRef PLYR_Team = GetCtrlFromWindow(Window(),Sig_Player,iTEAM);
	SetControl32BitValue(PLYR_Team,player_preferences->team+1);
	
	// Input
	
	// Always keyboard and regular mouse, because Carbon does not support the InputSprocket
	input_preferences->input_device = _mouse_yaw_pitch;
	
	ControlRef INPT_Run_Walk = GetCtrlFromWindow(Window(),Sig_Input,iINTERCHANGE_RUN_WALK);
	SetControl32BitValue(INPT_Run_Walk,TEST_FLAG(input_preferences->modifiers, _inputmod_interchange_run_walk));

	ControlRef INPT_Swim_Sink = GetCtrlFromWindow(Window(),Sig_Input,iINTERCHANGE_SWIM_SINK);
	SetControl32BitValue(INPT_Swim_Sink,TEST_FLAG(input_preferences->modifiers, _inputmod_interchange_swim_sink));

	ControlRef INPT_No_Auto_Rctr = GetCtrlFromWindow(Window(),Sig_Input,iDONT_AUTO_RECENTER);
	SetControl32BitValue(INPT_No_Auto_Rctr,TEST_FLAG(input_preferences->modifiers, _inputmod_dont_auto_recenter));

	ControlRef INPT_No_Switch_New = GetCtrlFromWindow(Window(),Sig_Input,iDONT_SWITCH_TO_NEW_WEAPON);
	SetControl32BitValue(INPT_No_Switch_New,TEST_FLAG(input_preferences->modifiers, _inputmod_dont_switch_to_new_weapon));

	ControlRef INPT_Button_Snds = GetCtrlFromWindow(Window(),Sig_Input,iUSE_INTERFACE_BUTTON_SOUNDS);
	SetControl32BitValue(INPT_Button_Snds,TEST_FLAG(input_preferences->modifiers, _inputmod_use_button_sounds));

	ControlRef INPT_Button_Invert = GetCtrlFromWindow(Window(),Sig_Input,iINVERT_MOUSE_VERTICAL);
	SetControl32BitValue(INPT_Button_Invert,TEST_FLAG(input_preferences->modifiers, _inputmod_invert_mouse));

	ControlRef INPT_Button_VertSens = GetCtrlFromWindow(Window(),Sig_Input,iVERTICAL_MOUSE_SENSITIVITY);
	SetCtrlFloatValue(INPT_Button_VertSens, PrefToSlider(input_preferences->sens_vertical));
	
	ControlRef INPT_Button_HorizSens = GetCtrlFromWindow(Window(),Sig_Input,iHORIZONTAL_MOUSE_SENSITIVITY);
	SetCtrlFloatValue(INPT_Button_HorizSens, PrefToSlider(input_preferences->sens_horizontal));
		
	// Environment:
	
	if(allocate_extensions_memory())
		build_extensions_list();
	
	ControlRef ENVR_Map = GetCtrlFromWindow(Window(),Sig_Environment,iMAP);
	fill_in_popup_with_filetype(ENVR_Map, _typecode_scenario,
		environment_preferences->map_checksum, environment_preferences->map_file);
	
	ControlRef ENVR_Physics = GetCtrlFromWindow(Window(),Sig_Environment,iPHYSICS);
	fill_in_popup_with_filetype(ENVR_Physics, _typecode_physics,
		environment_preferences->physics_checksum, environment_preferences->physics_file);
	
	ControlRef ENVR_Shapes = GetCtrlFromWindow(Window(),Sig_Environment,iSHAPES);
	fill_in_popup_with_filetype(ENVR_Shapes, _typecode_shapes,
		environment_preferences->shapes_mod_date, environment_preferences->shapes_file);
	
	ControlRef ENVR_Sounds = GetCtrlFromWindow(Window(),Sig_Environment,iSOUNDS);
	fill_in_popup_with_filetype(ENVR_Sounds, _typecode_sounds,
		environment_preferences->sounds_mod_date, environment_preferences->sounds_file);
	
	// Set to the current prefs pane
	// Go from 0-based to 1-based
	SetControl32BitValue(HandlerData.TabCtrl,CurrentPrefsPane+1);
	CurrentPrefsPane = NONE; // Force-update hack ("previous" pane is none of them)
	UpdatePanes(&HandlerData);
	
	// Preserve the old sound preferences, so that music volume can be previewed,
	// while being able to revert to the original values
	sound_manager_parameters OriginalSoundParameters;
	obj_copy(OriginalSoundParameters,*sound_preferences);
	
	// Remember the old monitor resolution, in case we change it.
	// When we change it, we'll set the monitor frequency to its default value.
	short OldSize = graphics_preferences->screen_mode.size;
	
	// Run!
	if (RunModalDialog(Window(), false, PreferencesHandler, &HandlerData))
	{
		// OK
		
		// Extract the control objects' values
		
		// Graphics:
		
		switch(GetControl32BitValue(GRFX_Number_of_Colors))
		{
			case _hundreds_colors_menu_item:
				graphics_preferences->screen_mode.bit_depth= 8; break;
			case _thousands_colors_menu_item:
				graphics_preferences->screen_mode.bit_depth= 16; break;
			case _millions_colors_menu_item:
			case _billions_colors_menu_item:
			default:
				graphics_preferences->screen_mode.bit_depth= 32;
		}
		
		graphics_preferences->screen_mode.size = GetControl32BitValue(GRFX_Window_Size) - 1;
		
		graphics_preferences->screen_mode.high_resolution = !(GetControl32BitValue(GRFX_Detail) - 1);
		
		graphics_preferences->screen_mode.gamma_level = GetControl32BitValue(GRFX_Brightness) - 1;
		
		graphics_preferences->screen_mode.acceleration =
			GetControl32BitValue(GRFX_Acceleration) ?
				_opengl_acceleration :
				_no_acceleration;
		
		// IR-inspired change; no OpenGL mode in 8-bit color depth
		if (graphics_preferences->screen_mode.bit_depth == 8)
			graphics_preferences->screen_mode.acceleration= _no_acceleration;
		
		graphics_preferences->screen_mode.fullscreen = GetControl32BitValue(GRFX_FillScreen);
		
		// Sound:
	
		sound_preferences->volume = GetControl32BitValue(SNDS_Volume) - 1;
	
		sound_preferences->channel_count = GetControl32BitValue(SNDS_Channels);
	
		sound_preferences->music = GetControl32BitValue(SNDS_Music) - 1;
		
		SET_FLAG(sound_preferences->flags,_stereo_flag,GetControl32BitValue(SNDS_Stereo));
		
		SET_FLAG(sound_preferences->flags,_dynamic_tracking_flag,GetControl32BitValue(SNDS_Panning));

		if (!TEST_FLAG(sound_preferences->flags,_stereo_flag))
			SET_FLAG(sound_preferences->flags,_dynamic_tracking_flag,false);
		
		SET_FLAG(sound_preferences->flags,_16bit_sound_flag,GetControl32BitValue(SNDS_16Bit));
		
		SET_FLAG(sound_preferences->flags,_ambient_sound_flag,GetControl32BitValue(SNDS_Ambient));
		
		SET_FLAG(sound_preferences->flags,_more_sounds_flag,GetControl32BitValue(SNDS_More));
		
		SET_FLAG(sound_preferences->flags,_relative_volume_flag,GetControl32BitValue(SNDS_Relative_Volume));
		
		// Player:
		
		player_preferences->difficulty_level = GetControl32BitValue(PLYR_Difficulty) - 1;
		
		GetEditPascalText(PLYR_Name, player_preferences->name, PREFERENCES_NAME_LENGTH);

		player_preferences->color = GetControl32BitValue(PLYR_Color) - 1;

		player_preferences->team = GetControl32BitValue(PLYR_Team) - 1;
		
		// Input:
	
		SET_FLAG(input_preferences->modifiers, _inputmod_interchange_run_walk, GetControl32BitValue(INPT_Run_Walk));

		SET_FLAG(input_preferences->modifiers, _inputmod_interchange_swim_sink, GetControl32BitValue(INPT_Swim_Sink));

		SET_FLAG(input_preferences->modifiers, _inputmod_dont_auto_recenter, GetControl32BitValue(INPT_No_Auto_Rctr));

		SET_FLAG(input_preferences->modifiers, _inputmod_dont_switch_to_new_weapon, GetControl32BitValue(INPT_No_Switch_New));

		SET_FLAG(input_preferences->modifiers, _inputmod_use_button_sounds, GetControl32BitValue(INPT_Button_Snds));

		SET_FLAG(input_preferences->modifiers, _inputmod_invert_mouse, GetControl32BitValue(INPT_Button_Invert));
	
		input_preferences->sens_vertical = SliderToPref(GetCtrlFloatValue(INPT_Button_VertSens));
		
		input_preferences->sens_horizontal = SliderToPref(GetCtrlFloatValue(INPT_Button_HorizSens));
		
		// Environment:
		
		environment_preferences->map_checksum=
			find_checksum_and_file_spec_from_control(ENVR_Map, _typecode_scenario,
				&environment_preferences->map_file);

		if(physics_valid)
		{
			environment_preferences->physics_checksum=
				find_checksum_and_file_spec_from_control(ENVR_Physics, _typecode_physics,
					&environment_preferences->physics_file);
		}
		
		environment_preferences->shapes_mod_date=
			find_checksum_and_file_spec_from_control(ENVR_Shapes, _typecode_shapes,
				&environment_preferences->shapes_file);
		
		environment_preferences->sounds_mod_date=
			find_checksum_and_file_spec_from_control(ENVR_Sounds, _typecode_sounds,
				&environment_preferences->sounds_file);
		
		// Clean up after dialogs; lone files may not be selected properly by them
		SetToLoneFile(_typecode_scenario,
			environment_preferences->map_file, environment_preferences->map_checksum);
		SetToLoneFile(_typecode_physics,
			environment_preferences->physics_file, environment_preferences->physics_checksum);
		SetToLoneFile(_typecode_shapes,
			environment_preferences->shapes_file, environment_preferences->shapes_mod_date);
		SetToLoneFile(_typecode_sounds,
			environment_preferences->sounds_file, environment_preferences->sounds_mod_date);
		
		/* Save the new ones. */
		if (graphics_preferences->screen_mode.size != OldSize)
			graphics_preferences->refresh_frequency = DEFAULT_MONITOR_REFRESH_FREQUENCY;
		write_preferences();
		set_sound_manager_parameters(sound_preferences);
		load_environment_from_preferences();
	}
	else
	{
		// Cancel
		
		set_sound_manager_parameters(&OriginalSoundParameters);
	}
		
	/* Proceses the entire physics file.. */
	free_extensions_memory();
}
#else
void handle_preferences(
	void)
{
	/* Save the existing preferences, in case we have to reload them. */
	write_preferences();
		
	// Preserve the old sound preferences, so that music volume can be previewed,
	// while being able to revert to the original values
	sound_manager_parameters OriginalSoundParameters;
	obj_copy(OriginalSoundParameters,*sound_preferences);
	
	// Remember the old monitor resolution, in case we change it.
	// When we change it, we'll set the monitor frequency to its default value.
	short OldSize = graphics_preferences->screen_mode.size;

	if(set_preferences(prefs_data, NUMBER_OF_PREFS_PANELS, initialize_preferences))
	{
		/* Save the new ones. */
		if (graphics_preferences->screen_mode.size != OldSize)
			graphics_preferences->refresh_frequency = DEFAULT_MONITOR_REFRESH_FREQUENCY;
		write_preferences();
		set_sound_manager_parameters(sound_preferences);
		load_environment_from_preferences();
	}
	else
		set_sound_manager_parameters(&OriginalSoundParameters);
}
#endif


/*
 *  Get user name
 */

#define strUSER_NAME -16096

static void get_name_from_system(
	unsigned char *name)
{
#if defined(TARGET_API_MAC_CARBON)
	CFStringEncoding encoding = CFStringGetSystemEncoding();
	CFStringRef username = CSCopyUserName(false);
	Str255 buffer;
	ConstStringPtr ptr;
	ptr = CFStringGetPascalStringPtr(username, encoding);
	if (ptr == NULL) {
		if (CFStringGetPascalString(username, buffer, 256, encoding))
			ptr = buffer;
		}
	CFRelease(username);
	assert(ptr);
	
	pstrcpy(name, (unsigned char *)ptr);
#else
	StringHandle name_handle;
	char old_state;

	name_handle= GetString(strUSER_NAME);
	assert(name_handle);
	
	old_state= HGetState((Handle)name_handle);
	HLock((Handle)name_handle);
	
	pstrcpy(name, *name_handle);
	HSetState((Handle)name_handle, old_state);
#endif
}


/*
 *  Ethernet available?
 */

static bool ethernet_active(
	void)
{
#if defined(TARGET_API_MAC_CARBON)
	// JTP: I'm lame, not doing anything for network foo
	return false;
#else
	short  refnum;
	OSErr  error;

	error= OpenDriver("\p.ENET", &refnum);
	
	return error==noErr ? true : false;
#endif
}


/* ------------- dialog functions */

/* --------- graphics */
#ifndef USES_NIBS
enum {
	_hundreds_colors_menu_item= 1,
	_thousands_colors_menu_item,
	_millions_colors_menu_item,
	_billions_colors_menu_item
};
#endif

static void setup_graphics_dialog(
	DialogPtr dialog,
	short first_item,
	void *prefs)
{
	struct graphics_preferences_data *preferences= (struct graphics_preferences_data *) prefs;
	short value, active;
	
	// IR-inspired: force to some reasonable value:
	if (preferences->screen_mode.bit_depth >= 32)
		preferences->screen_mode.bit_depth = 32;
	else if (preferences->screen_mode.bit_depth >= 16)
		preferences->screen_mode.bit_depth = 16;
	else
		preferences->screen_mode.bit_depth = 8;
	
	if(machine_supports_32bit(&preferences->device_spec))
	{
		active= true;
	} else {
		if(preferences->screen_mode.bit_depth==32) preferences->screen_mode.bit_depth= 16;
		active= false;
	}
	set_popup_enabled_state(dialog, LOCAL_TO_GLOBAL_DITL(iNUMBER_OF_COLORS, first_item), _millions_colors_menu_item, active);

	if(machine_supports_16bit(&preferences->device_spec))
	{
		active= true;
	} else {
		if(preferences->screen_mode.bit_depth==16) preferences->screen_mode.bit_depth= 8;
		active= false;
	}
	set_popup_enabled_state(dialog, LOCAL_TO_GLOBAL_DITL(iNUMBER_OF_COLORS, first_item), _thousands_colors_menu_item, active);
	set_popup_enabled_state(dialog, LOCAL_TO_GLOBAL_DITL(iNUMBER_OF_COLORS, first_item), _billions_colors_menu_item, false);

	/* Make sure it is enabled. */
	modify_control(dialog, LOCAL_TO_GLOBAL_DITL(iNUMBER_OF_COLORS, first_item), 
		CONTROL_ACTIVE, NONE);
	modify_control(dialog, LOCAL_TO_GLOBAL_DITL(iDETAIL, first_item), 
		CONTROL_ACTIVE, NONE);
	modify_control(dialog, LOCAL_TO_GLOBAL_DITL(iCHOOSE_MONITOR, first_item), 
		CONTROL_ACTIVE, NONE);
	
	switch(preferences->screen_mode.bit_depth)
	{
		case 32: value= _millions_colors_menu_item; break;
		case 16: value= _thousands_colors_menu_item; break;
		case 8:	 value= _hundreds_colors_menu_item; break;
		default: value= _hundreds_colors_menu_item;
	}
	modify_control(dialog, LOCAL_TO_GLOBAL_DITL(iNUMBER_OF_COLORS, first_item), NONE, value);

	// No more special 68K support either
	preferences->screen_mode.draw_every_other_line= false;
	active= CONTROL_INACTIVE;
	
	modify_control(dialog, LOCAL_TO_GLOBAL_DITL(iWINDOW_SIZE, first_item), NONE, 
		preferences->screen_mode.size+1);
	modify_control(dialog, LOCAL_TO_GLOBAL_DITL(iDETAIL, first_item), NONE, 
		!preferences->screen_mode.high_resolution+1); 
	modify_control(dialog, LOCAL_TO_GLOBAL_DITL(iBRIGHTNESS, first_item), NONE, 
		preferences->screen_mode.gamma_level+1);
	
	// LP change: adding OpenGL support
	// Simon Brownlee patch: did correct handling of "active"
	active = OGL_IsPresent() ? CONTROL_ACTIVE : CONTROL_INACTIVE;
	if (active!=CONTROL_ACTIVE) preferences->screen_mode.acceleration = _no_acceleration;
	modify_control(dialog, LOCAL_TO_GLOBAL_DITL(iHARDWARE_ACCELERATION, first_item), 
		active, (preferences->screen_mode.acceleration == _opengl_acceleration));
	
	// LP: added full-screen option; will not activate it until it's in good working order
	modify_control(dialog, LOCAL_TO_GLOBAL_DITL(iFILL_SCREEN, first_item), 
		CONTROL_ACTIVE, (preferences->screen_mode.fullscreen));
}

static void hit_graphics_item(
	DialogPtr dialog,
	short first_item,
	void *prefs,
	short item_hit)
{
	struct graphics_preferences_data *preferences= (struct graphics_preferences_data *) prefs;
	ControlHandle control;
	short item_type;
	Rect bounds;
	bool resetup= true;

	switch(GLOBAL_TO_LOCAL_DITL(item_hit, first_item))
	{
		case iCHOOSE_MONITOR:
			display_device_dialog(&preferences->device_spec);
			/* We resetup because the new device might not support millions, etc.. */
			break;
		
		case iHARDWARE_ACCELERATION:
			// LP change: added OpenGL support here
			if(preferences->screen_mode.acceleration == _opengl_acceleration)
			{
				preferences->screen_mode.acceleration= _no_acceleration;
			} else {
				preferences->screen_mode.acceleration= _opengl_acceleration;
				// Ian-Rickard-inspired change
				// OpenGL mode is only reasonable with more than 8-bit
				preferences->screen_mode.bit_depth = MAX(preferences->screen_mode.bit_depth, 16);
			}
			break;
			
		case iNUMBER_OF_COLORS:
			GetDialogItem(dialog, item_hit, &item_type, (Handle *) &control, &bounds);
			switch(GetControlValue(control))
			{
				case _hundreds_colors_menu_item:
					// IR-inspired change; no OpenGL mode in 8-bit color depth
					preferences->screen_mode.bit_depth= 8;
					preferences->screen_mode.acceleration= _no_acceleration;
					break;
				case _thousands_colors_menu_item: preferences->screen_mode.bit_depth= 16; break;
				case _millions_colors_menu_item: preferences->screen_mode.bit_depth= 32; break;
				case _billions_colors_menu_item:
				default: preferences->screen_mode.bit_depth= 8;
			}
			break;

		case iWINDOW_SIZE:		
			GetDialogItem(dialog, item_hit, &item_type, (Handle *) &control, &bounds);
			preferences->screen_mode.size= GetControlValue(control)-1;
			break;
			
		case iDETAIL:
			GetDialogItem(dialog, item_hit, &item_type, (Handle *) &control, &bounds);
			preferences->screen_mode.high_resolution= !(GetControlValue(control)-1);
			break;
		
		case iBRIGHTNESS:
			GetDialogItem(dialog, item_hit, &item_type, (Handle *) &control, &bounds);
			preferences->screen_mode.gamma_level= GetControlValue(control)-1;
			break;
		
		case iOPENGL_OPTIONS:
			OGL_ConfigureDialog(preferences->OGL_Configure);
			break;
		
		case iFILL_SCREEN:
			// LP change: added full-screen support here
			preferences->screen_mode.fullscreen = !preferences->screen_mode.fullscreen;
			break;
			
		default:
			assert(false);
			break;
	}

	if(resetup)
	{
		setup_graphics_dialog(dialog, first_item, prefs);
	}
}
	
static bool teardown_graphics_dialog(
	DialogPtr dialog,
	short first_item,
	void *prefs)
{
	(void)(dialog);
	(void)(first_item);
	(void)(prefs);
	return true;
}

/* --------- player */
static void setup_player_dialog(
	DialogPtr dialog,
	short first_item,
	void *prefs)
{
	struct player_preferences_data *preferences= (struct player_preferences_data *)prefs;
	Handle item;
	short item_type;
	Rect bounds;
	ControlRef prefNameItem;

	/* Setup the difficulty level */
	modify_control(dialog, LOCAL_TO_GLOBAL_DITL(iDIFFICULTY_LEVEL, first_item), NONE, 
		preferences->difficulty_level+1);

	/* Setup the name. */
	// JTP: When embedding is on, you should pass in the control handle produced by a
	// call to GetDialogItemAsControl. If embedding is not on, pass in the handle
	// produced by GetDialogItem.
	// JTP: Embedding is on for this main preferences dialog box.
	GetDialogItem(dialog, LOCAL_TO_GLOBAL_DITL(iNAME, first_item), &item_type, &item, &bounds);	
	GetDialogItemAsControl(dialog, LOCAL_TO_GLOBAL_DITL(iNAME, first_item), &prefNameItem);
	SetDialogItemText((Handle)prefNameItem, preferences->name);
	SelectDialogItemText(dialog, LOCAL_TO_GLOBAL_DITL(iNAME, first_item), 0, SHRT_MAX);

	/* Setup the color */
	GetDialogItem(dialog, LOCAL_TO_GLOBAL_DITL(iCOLOR, first_item), &item_type, &item, &bounds);
	SetControlValue((ControlHandle) item, preferences->color+1);

	/* Setup the team */
	GetDialogItem(dialog, LOCAL_TO_GLOBAL_DITL(iTEAM, first_item), &item_type, &item, &bounds);
	SetControlValue((ControlHandle) item, preferences->team+1);
}

static void hit_player_item(
	DialogPtr dialog,
	short first_item,
	void *prefs,
	short item_hit)
{
	ControlHandle control;
	short item_type;
	Rect bounds;
	struct player_preferences_data *preferences= (struct player_preferences_data *)prefs;

	switch(GLOBAL_TO_LOCAL_DITL(item_hit, first_item))
	{
		case iDIFFICULTY_LEVEL:
			GetDialogItem(dialog, item_hit, &item_type, (Handle *) &control, &bounds);
			preferences->difficulty_level= GetControlValue(control)-1;
			break;

		case iCOLOR:
			GetDialogItem(dialog, item_hit, &item_type, (Handle *) &control, &bounds);
			preferences->color= GetControlValue(control)-1;
			break;

		case iTEAM:
			GetDialogItem(dialog, item_hit, &item_type, (Handle *) &control, &bounds);
			preferences->team= GetControlValue(control)-1;
			break;
		
		// LP additions:
		case iCHASE_CAM:
			Configure_ChaseCam(preferences->ChaseCam);
			break;
			
		case iCROSSHAIRS:
			Configure_Crosshairs(preferences->Crosshairs);
			break;
	}
}
	
static bool teardown_player_dialog(
	DialogPtr dialog,
	short first_item,
	void *prefs)
{
	Handle control;
	short item_type;
	Rect bounds;
	struct player_preferences_data *preferences= (struct player_preferences_data *)prefs;
	Str255 buffer;
	
	/* Get the player name */
	GetDialogItem(dialog, LOCAL_TO_GLOBAL_DITL(iNAME, first_item), &item_type, &control, &bounds);
	GetDialogItemText(control, buffer);
	if(buffer[0]>PREFERENCES_NAME_LENGTH) buffer[0]= PREFERENCES_NAME_LENGTH;
	memcpy(preferences->name, buffer, buffer[0]+1);

	return true;
}

/* --------- sound */
static void setup_sound_dialog(
	DialogPtr dialog,
	short first_item,
	void *prefs)
{
	struct sound_manager_parameters *preferences= (struct sound_manager_parameters *) prefs;
	short active;
	uint16 available_flags;

	available_flags= available_sound_manager_flags(preferences->flags);

	/* First setup the popups */
	modify_control(dialog, LOCAL_TO_GLOBAL_DITL(iVOLUME, first_item), NONE, 
		preferences->volume+1);
	modify_control(dialog, LOCAL_TO_GLOBAL_DITL(iCHANNELS, first_item), NONE, 
		preferences->channel_count);
	// LP addition:
	modify_control(dialog, LOCAL_TO_GLOBAL_DITL(iMUSIC, first_item), NONE, 
		preferences->music+1);

	active= (available_flags & _stereo_flag) ? CONTROL_ACTIVE : CONTROL_INACTIVE;
	modify_control(dialog, LOCAL_TO_GLOBAL_DITL(iSTEREO, first_item), active, 
		(preferences->flags & _stereo_flag) ? true : false);

	/* Don't do dynamic tracking if you aren't in stereo. */
	if(!(preferences->flags & _stereo_flag))
	{
		preferences->flags &= ~_dynamic_tracking_flag;
	}

	active= (available_flags & _dynamic_tracking_flag) ? CONTROL_ACTIVE : CONTROL_INACTIVE;
	modify_control(dialog, LOCAL_TO_GLOBAL_DITL(iACTIVE_PANNING, first_item), active, 
		(preferences->flags & _dynamic_tracking_flag) ? true : false);

	active= (available_flags & _16bit_sound_flag) ? CONTROL_ACTIVE : CONTROL_INACTIVE;
	modify_control(dialog, LOCAL_TO_GLOBAL_DITL(iHIGH_QUALITY, first_item), active, 
		(preferences->flags & _16bit_sound_flag) ? true : false);

	active= (available_flags & _ambient_sound_flag) ? CONTROL_ACTIVE : CONTROL_INACTIVE;
	modify_control(dialog, LOCAL_TO_GLOBAL_DITL(iAMBIENT_SOUND, first_item), active, 
		(preferences->flags & _ambient_sound_flag) ? true : false);

	active= (available_flags & _more_sounds_flag) ? CONTROL_ACTIVE : CONTROL_INACTIVE;
	modify_control(dialog, LOCAL_TO_GLOBAL_DITL(iMORE_SOUNDS, first_item), active, 
		(preferences->flags & _more_sounds_flag) ? true : false);

	active= (available_flags & _relative_volume_flag) ? CONTROL_ACTIVE : CONTROL_INACTIVE;
	modify_control(dialog, LOCAL_TO_GLOBAL_DITL(iRELATIVE_VOLUME, first_item), active, 
		(preferences->flags & _relative_volume_flag) ? true : false);
}

static void hit_sound_item(
	DialogPtr dialog,
	short first_item,
	void *prefs,
	short item_hit)
{
	ControlHandle control;
	short item_type;
	Rect bounds;
	struct sound_manager_parameters *preferences= (struct sound_manager_parameters *)prefs;

	switch(GLOBAL_TO_LOCAL_DITL(item_hit, first_item))
	{
		case iSTEREO:
			GetDialogItem(dialog, item_hit, &item_type, (Handle *) &control, &bounds);
			SET_FLAG(preferences->flags,_stereo_flag,!GetControlValue(control));
			break;

		case iACTIVE_PANNING:
			GetDialogItem(dialog, item_hit, &item_type, (Handle *) &control, &bounds);
			SET_FLAG(preferences->flags,_dynamic_tracking_flag,!GetControlValue(control));
			break;
			
		case iHIGH_QUALITY:
			GetDialogItem(dialog, item_hit, &item_type, (Handle *) &control, &bounds);
			SET_FLAG(preferences->flags,_16bit_sound_flag,!GetControlValue(control));
			break;

		case iAMBIENT_SOUND:
			GetDialogItem(dialog, item_hit, &item_type, (Handle *) &control, &bounds);
			SET_FLAG(preferences->flags,_ambient_sound_flag,!GetControlValue(control));
			break;

		case iMORE_SOUNDS:
			GetDialogItem(dialog, item_hit, &item_type, (Handle *) &control, &bounds);
			SET_FLAG(preferences->flags,_more_sounds_flag,!GetControlValue(control));
			break;

		case iRELATIVE_VOLUME:
			GetDialogItem(dialog, item_hit, &item_type, (Handle *) &control, &bounds);
			SET_FLAG(preferences->flags,_relative_volume_flag,!GetControlValue(control));
			// Need to hear relative/absolute distinction
			test_sound_volume(preferences->volume, Sound_AdjustVolume());
			// For checking out the music volume
			set_sound_manager_parameters(sound_preferences);
			break;
			
		case iVOLUME:
			GetDialogItem(dialog, item_hit, &item_type, (Handle *) &control, &bounds);
			preferences->volume= GetControlValue(control)-1;
			test_sound_volume(preferences->volume, Sound_AdjustVolume());
			// For checking out the music volume
			set_sound_manager_parameters(sound_preferences);
			break;

		case iCHANNELS:
			GetDialogItem(dialog, item_hit, &item_type, (Handle *) &control, &bounds);
			preferences->channel_count= GetControlValue(control);	
			break;
			
		// LP addition:
		case iMUSIC:
			GetDialogItem(dialog, item_hit, &item_type, (Handle *) &control, &bounds);
			preferences->music= GetControlValue(control)-1;
			// For checking out the music volume
			set_sound_manager_parameters(sound_preferences);
			music_idle_proc();
			break;
			
		default:
			assert(false);
			break;
	}

	setup_sound_dialog(dialog, first_item, prefs);
}
	
static bool teardown_sound_dialog(
	DialogPtr dialog,
	short first_item,
	void *prefs)
{
	(void)(dialog);
	(void)(first_item);
	(void)(prefs);
	return true;
}

/* --------- input */

static void setup_input_dialog(
	DialogPtr dialog,
	short first_item,
	void *prefs)
{
	struct input_preferences_data *preferences= (struct input_preferences_data *)prefs;
	short which;
	short active;
	
	// JTP: Bytnar's inspired to disable ISp controls on no ISp systems, like OS X
#if defined(TARGET_API_MAC_CARBON) && __MACH__
//	if (!ISp_IsPresent())
//	{
		active= CONTROL_INACTIVE;
		modify_control(dialog, LOCAL_TO_GLOBAL_DITL(iKEYBOARD_CONTROL, first_item), active, false);
		modify_control(dialog, LOCAL_TO_GLOBAL_DITL(iINPUT_SPROCKET_CONTROL, first_item), active, false);
		modify_control(dialog, LOCAL_TO_GLOBAL_DITL(iSET_INPUT_SPROCKET, first_item), active, false);
		if (preferences->input_device != _mouse_yaw_pitch)
			preferences->input_device = _mouse_yaw_pitch;
//	}
#endif

	// LP change: implemented Ben Thompson ISp support
	which = (preferences->input_device == _mouse_yaw_pitch) ? iMOUSE_CONTROL :
		(preferences->input_device == _keyboard_or_game_pad) ?iKEYBOARD_CONTROL: iINPUT_SPROCKET_CONTROL;
	modify_radio_button_family(dialog, LOCAL_TO_GLOBAL_DITL(iMOUSE_CONTROL, first_item), 
		LOCAL_TO_GLOBAL_DITL(iINPUT_SPROCKET_CONTROL, first_item), 
		LOCAL_TO_GLOBAL_DITL(which, first_item));
	
	// LP addition: handle the input modifiers
		
	active= CONTROL_ACTIVE;
	modify_control(dialog, LOCAL_TO_GLOBAL_DITL(iINTERCHANGE_RUN_WALK, first_item), active, 
		(preferences->modifiers & _inputmod_interchange_run_walk) ? true : false);
	
	active= CONTROL_ACTIVE;
	modify_control(dialog, LOCAL_TO_GLOBAL_DITL(iINTERCHANGE_SWIM_SINK, first_item), active, 
		(preferences->modifiers & _inputmod_interchange_swim_sink) ? true : false);
	
	// LP addition: Woody Zenfell's don't-recenter-view patch
	active= CONTROL_ACTIVE;
	modify_control(dialog, LOCAL_TO_GLOBAL_DITL(iDONT_AUTO_RECENTER, first_item), active, 
		(preferences->modifiers & _inputmod_dont_auto_recenter) ? true : false);
	
	// LP addition: Josh Elsasser's dont-switch-weapons patch
	active= CONTROL_ACTIVE;
	modify_control(dialog, LOCAL_TO_GLOBAL_DITL(iDONT_SWITCH_TO_NEW_WEAPON, first_item), active, 
		(preferences->modifiers & _inputmod_dont_switch_to_new_weapon) ? true : false);
	
	// LP addition: whether interface-button sounds are on (Ian Rickard)
	active= CONTROL_ACTIVE;
	modify_control(dialog, LOCAL_TO_GLOBAL_DITL(iUSE_INTERFACE_BUTTON_SOUNDS, first_item), active, 
		(preferences->modifiers & _inputmod_use_button_sounds) ? true : false);
}

static void hit_input_item(
	DialogPtr dialog,
	short first_item,
	void *prefs,
	short item_hit)
{
	ControlHandle control;
	short item_type;
	Rect bounds;
	struct input_preferences_data *preferences= (struct input_preferences_data *)prefs;

	switch(GLOBAL_TO_LOCAL_DITL(item_hit, first_item))
	{
		// LP change: added ISp-control button action
		case iMOUSE_CONTROL:
		case iKEYBOARD_CONTROL:
		case iINPUT_SPROCKET_CONTROL:
					modify_radio_button_family(dialog, LOCAL_TO_GLOBAL_DITL(iMOUSE_CONTROL, first_item), 
				LOCAL_TO_GLOBAL_DITL(iINPUT_SPROCKET_CONTROL, first_item), item_hit);
			// LP change: added Ben Thompson's ISp support
			preferences->input_device= GLOBAL_TO_LOCAL_DITL(item_hit, first_item)==iMOUSE_CONTROL ?
				_mouse_yaw_pitch : (GLOBAL_TO_LOCAL_DITL(item_hit, first_item)==iINPUT_SPROCKET_CONTROL ? 
				_input_sprocket_only : _keyboard_or_game_pad);
		break;
		
		// Added run/walk and swim/sink interchange; renamed iSET_KEYS (LP: renamed it back)
		case iINTERCHANGE_RUN_WALK:
			GetDialogItem(dialog, item_hit, &item_type, (Handle *) &control, &bounds);
			SET_FLAG(preferences->modifiers,_inputmod_interchange_run_walk,!GetControlValue(control));
			break;
			
		case iINTERCHANGE_SWIM_SINK:
			GetDialogItem(dialog, item_hit, &item_type, (Handle *) &control, &bounds);
			SET_FLAG(preferences->modifiers,_inputmod_interchange_swim_sink,!GetControlValue(control));
			break;
			
		// LP addition: Woody Zenfell's don't-recenter-view patch
		case iDONT_AUTO_RECENTER:
			GetDialogItem(dialog, item_hit, &item_type, (Handle *) &control, &bounds);
			SET_FLAG(preferences->modifiers,_inputmod_dont_auto_recenter,!GetControlValue(control));
			break;
			
		// LP addition: Josh Elsasser's dont-switch-weapons patch
		case iDONT_SWITCH_TO_NEW_WEAPON:
			GetDialogItem(dialog, item_hit, &item_type, (Handle *) &control, &bounds);
			SET_FLAG(preferences->modifiers,_inputmod_dont_switch_to_new_weapon,!GetControlValue(control));
			break;
			
		// LP addition: whether interface-button sounds are on (Ian Rickard)
		case iUSE_INTERFACE_BUTTON_SOUNDS:
			GetDialogItem(dialog, item_hit, &item_type, (Handle *) &control, &bounds);
			SET_FLAG(preferences->modifiers,_inputmod_use_button_sounds,!GetControlValue(control));
			break;
			
		case iSET_KEYS:
			{
				short key_codes[NUMBER_OF_KEYS];
				
				objlist_copy(key_codes, preferences->keycodes, NUMBER_OF_KEYS);
				if(configure_key_setup(key_codes))
				{
					objlist_copy(preferences->keycodes, key_codes, NUMBER_OF_KEYS);
					set_keys(key_codes);
				}
			}
			break;

		// LP change: modification of Ben Thompson's change
		case iSET_INPUT_SPROCKET:
#if defined(TARGET_API_MAC_CARBON)
			// assert(0);
			SysBeep(30);
#else
			ConfigureMarathonISpControls();
#endif
			break;
			
		default:
			break;
	}
	
	setup_input_dialog(dialog, first_item, prefs);
}
	
static bool teardown_input_dialog(
	DialogPtr dialog,
	short first_item,
	void *prefs)
{
	(void)(dialog);
	(void)(first_item);
	(void)(prefs);
	return true;
}

/* ------------------ environment preferences */
// ZZZ: we should now feel better about setting this quite high, since our use
// of vectors should mean we only use what memory we need.  OTOH I feel we may
// as well leave this in with a quite-high limit, just to guard against any
// infinite-recursion-type glitches etc.
#define MAXIMUM_FIND_FILES (8192)

// ZZZ: changing the way all this works so we can label groups of elements by their folder name.
struct file_description {
	// This is now the _typecode_stuff specified in tags.h (abstract file typing)
	Typecode file_type;
	unsigned long checksum;
	unsigned long parent_checksum;
        int directory_index;
        FileSpecifier file;
};

/*
struct directory_with_parent {
        FileSpecifier directory;
        int parent_index;
};
*/

// Holds, for our purposes, names of directories and tree information
static vector<FileSpecifier> directories;

// Holds indices into 'directories'; used during directory tree traversal
// to establish correct 'directory_index' values for the file_descriptions.
static vector<int> current_directory_stack;

// Information about each actual file found along the way, 
static vector<file_description> file_descriptions;

// Maps typecode to vector of indices into file_descriptions
// if index == NONE, means item should not be selectable (disabled folder title, separator, etc.)
static map<int, vector<int> > menu_items;

// Maps directory index to vector of indices into file_descriptions
typedef map<int, vector<int> > files_by_directory_type;
typedef map<int, files_by_directory_type> files_by_directory_by_typecode_type;
files_by_directory_by_typecode_type files_by_directory_by_typecode;

static void setup_environment_dialog(
	DialogPtr dialog,
	short first_item,
	void *prefs)
{
	struct environment_preferences_data *preferences= (struct environment_preferences_data *)prefs;

	if(allocate_extensions_memory())
	{
		SetCursor(*GetCursor(watchCursor));
		
		build_extensions_list();

		/* Fill in the extensions.. */
		fill_in_popup_with_filetype(dialog, LOCAL_TO_GLOBAL_DITL(iMAP, first_item),
			_typecode_scenario, preferences->map_checksum, preferences->map_file);
		fill_in_popup_with_filetype(dialog, LOCAL_TO_GLOBAL_DITL(iPHYSICS, first_item),
			_typecode_physics, preferences->physics_checksum, preferences->physics_file);
		fill_in_popup_with_filetype(dialog, LOCAL_TO_GLOBAL_DITL(iSHAPES, first_item),
			_typecode_shapes, preferences->shapes_mod_date, preferences->shapes_file);
		fill_in_popup_with_filetype(dialog, LOCAL_TO_GLOBAL_DITL(iSOUNDS, first_item),
			_typecode_sounds, preferences->sounds_mod_date, preferences->sounds_file);

//#if defined(USE_CARBON_ACCESSORS)
		Cursor arrow;
		GetQDGlobalsArrow(&arrow);
		SetCursor(&arrow);
/*
#else
		SetCursor(&qd.arrow);
#endif
*/
	} else {
		assert(false);
	}
}

static void hit_environment_item(
	DialogPtr dialog,
	short first_item,
	void *prefs,
	short item_hit)
{
	struct environment_preferences_data *preferences= (struct environment_preferences_data *)prefs;

	(void)(dialog);
	switch(GLOBAL_TO_LOCAL_DITL(item_hit, first_item))
	{
		case iMAP:
			preferences->map_checksum= find_checksum_and_file_spec_from_dialog(dialog, item_hit, 
				_typecode_scenario,	&preferences->map_file);
			break;
			
		case iPHYSICS:
			if(physics_valid)
			{
				preferences->physics_checksum= find_checksum_and_file_spec_from_dialog(dialog, item_hit, 
					_typecode_physics,	&preferences->physics_file);
			}
			break;
			
		case iSHAPES:
			preferences->shapes_mod_date= find_checksum_and_file_spec_from_dialog(dialog, item_hit,
				_typecode_shapes, &preferences->shapes_file);
			break;

		case iSOUNDS:
			preferences->sounds_mod_date= find_checksum_and_file_spec_from_dialog(dialog, item_hit,
				_typecode_sounds, &preferences->sounds_file);
			break;
			
		case iPATCHES_LIST:
			break;
			
		default:
			break;
	}
}

static bool teardown_environment_dialog(
	DialogPtr dialog,
	short first_item,
	void *prefs)
{
	// Clean up after dialogs; lone files may not be selected properly by them
	struct environment_preferences_data *preferences= (struct environment_preferences_data *)prefs;
	SetToLoneFile(_typecode_scenario, preferences->map_file, preferences->map_checksum);
	SetToLoneFile(_typecode_physics, preferences->physics_file, preferences->physics_checksum);
	SetToLoneFile(_typecode_shapes, preferences->shapes_file, preferences->shapes_mod_date);
	SetToLoneFile(_typecode_sounds, preferences->sounds_file, preferences->sounds_mod_date);

	(void)(dialog);
	(void)(first_item);
	/* Proceses the entire physics file.. */
	free_extensions_memory();
	
	return true;
}

/*
static unsigned long get_file_modification_date(
	FSSpec *file)
{
	CInfoPBRec pb;
	OSErr error;
	unsigned long modification_date= 0l;
	
	memset(&pb, 0, sizeof(pb));
	pb.hFileInfo.ioVRefNum= file->vRefNum;
	pb.hFileInfo.ioNamePtr= file->name;
	pb.hFileInfo.ioDirID= file->parID;
	pb.hFileInfo.ioFDirIndex= 0;
	error= PBGetCatInfoSync(&pb);
	if(!error)
	{
		modification_date= pb.hFileInfo.ioFlMdDat;
	}

	return modification_date;
}
*/

/* ---------------- miscellaneous */
static void set_popup_enabled_state(
	DialogPtr dialog,
	short item_number,
	short item_to_affect,
	bool enabled)
{
	MenuHandle menu;
	ControlHandle control;
	short item_type;
	Rect bounds;
	
	/* Get the menu handle */
	GetDialogItem(dialog, item_number, &item_type, (Handle *) &control, &bounds);
	assert(item_type&ctrlItem);

//#if defined(USE_CARBON_ACCESSORS)
	menu= GetControlPopupMenuHandle(control);
/*
#else
	*/
	/* I don't know how to assert that it is a popup control... <sigh> */
	/*
	struct PopupPrivateData **privateHndl;
	privateHndl= (PopupPrivateData **) ((*control)->contrlData);
	assert(privateHndl);
	
	menu= (*privateHndl)->mHandle;
#endif
*/
	assert(menu);
	
#if defined(TARGET_API_MAC_CARBON)
	if(enabled)
	{
		EnableMenuItem(menu, item_to_affect);
	} else {
		DisableMenuItem(menu, item_to_affect);
	}
#else	
	if(enabled)
	{
		EnableItem(menu, item_to_affect);
	} else {
		DisableItem(menu, item_to_affect);
	}
#endif
}

static void
clear_extensions_state()
{
        files_by_directory_by_typecode.clear();
        file_descriptions.clear();
        directories.clear();
        current_directory_stack.clear();
        menu_items.clear();
}

static bool allocate_extensions_memory(
	void)
{
        // ZZZ: now using vector to hold these, no pre-allocation

        // Make sure there's nothing stale hanging around
        clear_extensions_state();

        // current directory stack will use NONE for A1's root directory
        current_directory_stack.push_back(NONE);
        
        return true;
}

static void free_extensions_memory(
	void)
{
        // ZZZ: now using vector to hold these, no explicit alloc/free
        // (though we could I suppose ask these vectors to jettison their storage
        //  if we're worried about them taking up space the game needs)

        // Make sure there's nothing stale hanging around
        clear_extensions_state();
}

static bool
directory_change_callback(FileSpecifier& File, bool EnteringDirectory, void* data)
{
        if(EnteringDirectory)
        {
//                directory_with_parent theEntry;
//                theEntry.directory = File;
//                theEntry.parent_index = current_directory_stack.back();
                current_directory_stack.push_back(directories.size());
                directories.push_back(File);
        }
        else
        {
                // Since we shouldn't ever exit the root directory,
                assert(current_directory_stack.back() != NONE);
                assert(current_directory_stack.size() > 1);

                if(environment_preferences->reduce_singletons)
                {
                        for(files_by_directory_by_typecode_type::iterator ti = files_by_directory_by_typecode.begin();
                            ti != files_by_directory_by_typecode.end();
                            ti++)
                        {
                                files_by_directory_type& files_by_directory = ti->second;
                        
                                files_by_directory_type::iterator i = files_by_directory.find(current_directory_stack.back());
                                if(i != files_by_directory.end() && i->second.size() == 1)
                                {
                                        files_by_directory[*(current_directory_stack.rbegin() + 1)].push_back(i->second[0]);
                                        files_by_directory.erase(i);
                                }
                        }
                }
                
                current_directory_stack.pop_back();
        }

        // Keep going
        return true;
}

static bool file_is_extension_and_add_callback(
	FileSpecifier& File,
	void *data)
{
	unsigned long checksum;
	CInfoPBRec *pb= (CInfoPBRec *) data;
	
	if(file_descriptions.size()<MAXIMUM_FIND_FILES)
	{
                bool should_insert = false;
                file_description element_to_insert;
                
		// LP change, since the filetypes are no longer constants
		Typecode Filetype = File.GetType();
		if (Filetype == _typecode_scenario || Filetype == _typecode_physics)
		{
			checksum= read_wad_file_checksum(File);
			// checksum= read_wad_file_checksum((FileDesc *) file);
			if(checksum != 0) /* error. */
			{
                                element_to_insert.file= File;
                                element_to_insert.file_type= Filetype;
                                element_to_insert.checksum= checksum;
                                element_to_insert.directory_index= current_directory_stack.back();
                                should_insert = true;
                        }
		}
		else if (Filetype == _typecode_patch)
		{
			checksum= read_wad_file_checksum(File);
			if(checksum != 0) /* error. */
			{
				unsigned long parent_checksum= read_wad_file_parent_checksum(File);
                                element_to_insert.file= File;
                                element_to_insert.file_type= Filetype;
                                element_to_insert.checksum= checksum;
                                element_to_insert.parent_checksum= parent_checksum;
                                element_to_insert.directory_index= current_directory_stack.back();
                                should_insert = true;
                                // ZZZ note: the code I'm replacing inserted two file_descriptions[] entries here;
                                // that behavior seemed erroneous to me so I'm doing otherwise.
                                // (besides, I don't think we use "patch" files anyway, so...)
			}
		}
		else if (Filetype == _typecode_shapes || Filetype == _typecode_sounds)
		{
                        element_to_insert.file= File;
                        element_to_insert.file_type= Filetype;
                        element_to_insert.checksum=  pb->hFileInfo.ioFlMdDat;
                        element_to_insert.directory_index= current_directory_stack.back();
                        should_insert = true;
		}

                if(should_insert)
                {
                        files_by_directory_by_typecode[element_to_insert.file_type][current_directory_stack.back()].push_back(file_descriptions.size());
                        file_descriptions.push_back(element_to_insert);
                }
	}

        // Keep looking...
	return true;
}

static void build_extensions_list(
	void)
{
	DirectorySpecifier BaseDir;
	Files_GetRootDirectory(BaseDir);
	search_from_directory(BaseDir);
}

static void search_from_directory(DirectorySpecifier& BaseDir)
//	FSSpec *file)
{
	FileFinder pb;

	pb.Clear();	
	pb.version= 0;
	// LP change: always recurse
	pb.flags= _ff_recurse | _ff_callback_with_catinfo;
#if 0
#ifdef FINAL
	pb.flags= _ff_recurse | _ff_callback_with_catinfo;
#else
	pb.flags= _ff_callback_with_catinfo;
#endif
#endif
	pb.search_type= _callback_only;
	pb.BaseDir = BaseDir;
	pb.Type= WILDCARD_TYPE;
	pb.buffer= NULL;
	pb.max= MAXIMUM_FIND_FILES;
	pb.callback= file_is_extension_and_add_callback;
        pb.directory_change_callback= environment_preferences->group_by_directory ? directory_change_callback : NULL;
	pb.user_data= NULL;
	pb.count= 0;

	bool seek_ok= pb.Find();
	if(!seek_ok)
		vassert(seek_ok, csprintf(temporary, "Error: %d", pb.GetError()));
}

static bool
FSSpecs_equal(const FSSpec& spec1, const FSSpec& spec2)
{
        return spec1.vRefNum == spec2.vRefNum && spec1.parID == spec2.parID && memcmp(spec1.name, spec2.name, spec1.name[0] + 1) == 0;
}

static inline int
pstrcmp(const unsigned char* s1, const unsigned char* s2)
{
        int theResult = memcmp(&(s1[1]), &(s2[1]), min(s1[0], s2[0]));
        if(theResult == 0)
        {
                theResult = (s1[0] < s2[0]) ? -1 : ((s1[0] == s2[0]) ? 0 : 1);
        }
        return theResult;
}

class indexed_file_description_less : public binary_function<int, int, bool> {
public:
        bool operator()(int d1, int d2)
        {
                return pstrcmp(file_descriptions[d1].file.GetSpec().name, file_descriptions[d2].file.GetSpec().name) < 0;
        }
};

class indexed_directory_less : public binary_function<int, int, bool> {
public:
        bool operator()(int d1, int d2)
        {
                return pstrcmp(directories[d1].GetSpec().name, directories[d2].GetSpec().name) < 0;
        }
};


// Wrapper that extracts the control
static void fill_in_popup_with_filetype(
	DialogPtr dialog, 
	short item,
	int type,
	unsigned long checksum,
	const FSSpec& file)
{
	ControlHandle control;
	short item_type;
	Rect bounds;
	GetDialogItem(dialog, item, &item_type, (Handle *) &control, &bounds);
	
	fill_in_popup_with_filetype(control, type, checksum, file); 
}

/* Note that we are going to assume that things don't change while they are in this */
/*  dialog- ie no one is copying files to their machine, etc. */
// Now intended to use the _typecode_stuff in tags.h (abstract filetypes)
// ZZZ: this now tries to match by both file and checksum, in case there are multiple
// files with the same checksum (e.g. lots of map files created with non-checksum-aware
// Map editors).  The idea is if we find a "good match" (same file and same checksum)
// we prefer that.  If we find a "file_match" (same file only), we take that next.
// If we have neither a file_match nor a good_match, then if there are any with the
// correct checksum, we choose one of those (the last one, currently).  If there are
// no matches for either the file or the checksum, we select the first one we find.
// Originally, the Bungie code attempted neither good_match nor file_match functionality.
// Hmm, since there ought to be only one possible file_match, it's probably a bit
// much to have both file_match and good_match.  Oh well.
static void fill_in_popup_with_filetype(
	ControlHandle control,
	int type,
	unsigned long checksum,
	const FSSpec& file)
{
	MenuHandle menu;
        short value= NONE;
        short file_match_value= NONE;
        bool good_match= false;
	short count;
	//Rect bounds;

        // if our housekeeping vector for some reason isn't empty, empty it
        menu_items[type].clear();
        
        // insert NONE at index 0 to account for Mac OS 1-based indexing
        menu_items[type].push_back(NONE);

	/* Get the menu */
	// menu= get_popup_menu_handle(dialog, item);
	
	//#if defined(USE_CARBON_ACCESSORS)
	menu= GetControlPopupMenuHandle(control);
/*
#else
	*/
	/* I don't know how to assert that it is a popup control... <sigh> */
	/*
	PopupPrivateData **privateHndl= (PopupPrivateData **) ((*control)->contrlData);
	assert(privateHndl);

	menu= (*privateHndl)->mHandle;
#endif
*/

	
	/* Remove whatever it had */
#if defined(TARGET_API_MAC_CARBON)
	while(CountMenuItems(menu)) DeleteMenuItem(menu, 1);
#else
	while(CountMItems(menu)) DeleteMenuItem(menu, 1);
#endif

        count = 0;
        
        // Note one more option for this area would be like "coalesce on directory name"
        // so if a user had Marathon 2:Map Folder:(whole bunch of maps) and Marathon Infinity:Map Folder:(whole bunch of maps)
        // they would all land in one group called Map Folder.  Alternatively, we could
        // name the two separate groups (as we generate now) by their parents' names, e.g.
        // Marathon 2:Map Folder and Marathon Infinity:Map Folder.  We'd do that though
        // probably only when there's a group-name collision.
        // As James Willson points out, much of this complexity can be thrown out if we
        // just let users pick using a standard get file box.  In that case, we'd probably
        // retain the popup (as an alternative), but as a totally flat, sorted-by-Map-name list.

        // In fact the real solution (for network game players) is to expand the Levels popup in
        // Setup Network Game to show levels from all available Map files (perhaps grouped by Map
        // file).  But that's for another day... and maybe another person.  :)
        
        
        // Sort groups
        // I think NONE should always appear first, but not taking any chances (i.e. forcing it to)
        vector<int> sorted_directory_indices;
        sorted_directory_indices.push_back(NONE);

        files_by_directory_type& files_by_directory = files_by_directory_by_typecode[type];
        
        for(files_by_directory_type::iterator i = files_by_directory.begin(); i != files_by_directory.end(); i++)
        {
                if(i->first != NONE)
                {
                        sorted_directory_indices.push_back(i->first);
                }
        }

        sort(sorted_directory_indices.begin() + 1, sorted_directory_indices.end(), indexed_directory_less());
        
        // Loop over groups
        for(unsigned int i = 0; i < sorted_directory_indices.size(); i++)
        {
                int index = sorted_directory_indices[i];
                if(index != NONE)
                {
                        AppendMenu(menu, "\p ");
                        count++;
                        
                        SetMenuItemText(menu, count, directories[index].GetSpec().name);
#if TARGET_API_MAC_CARBON
                        DisableMenuItem(menu, count);
#else
                        DisableItem(menu, count);
#endif

                        // record this menu item as unusable
                        menu_items[type].push_back(NONE);
                }

                vector<int>& file_indices = files_by_directory[index];

                // Sort group items
                sort(file_indices.begin(), file_indices.end(), indexed_file_description_less());
                
                // Loop over items within group
                for(unsigned int j = 0; j < file_indices.size(); j++)
                {
                        file_description& description = file_descriptions[file_indices[j]];

                        AppendMenu(menu, "\p ");
                        count++;
                        
                        SetMenuItemText(menu, count, description.file.GetSpec().name);

                        // record this menu item
                        menu_items[type].push_back(file_indices[j]);

                        bool specs_equal = FSSpecs_equal(description.file.GetSpec(), file);
        
                        if(!good_match && description.checksum==checksum)
			{
				value= count;

                                if(specs_equal)
                                        good_match = true;
			}

                        if(!good_match && specs_equal)
                        {
                                file_match_value= count;
                        }                                
		}
	} 

	/* Set the max value */
	// GetDialogItem(dialog, item, &item_type, (Handle *) &control, &bounds);

	if(count==0)
	{
		// LP change, since the filetypes are no longer constants
		if (type == _typecode_physics)
		{
			set_to_default_physics_file();
			// Get the default string for an empty physics menu.
			// and then add the formatting to italicize it
			char local_temp[sizeof(temporary)];
			sprintf(local_temp, "<I%s", getcstr(temporary, strPROMPTS, _default_prompt));
			CopyCStringToPascal(local_temp, ptemporary);
			AppendMenu(menu, ptemporary);
			value= 1;
			physics_valid= false;
			count++;
                        menu_items[type].push_back(NONE);
		}
	}
	else
	{
		if (type == _typecode_physics)
		{
			physics_valid= true;
		}
	} 
	
	SetControlMaximum(control, count);

        if(!good_match && file_match_value != NONE)
                value = file_match_value;
        
	if(value != NONE)
	{
		SetControlValue(control, value);
	} else {
		/* Select the default one, somehow.. */
                for(int i = 1; i < count; i++)
                {
                        if(menu_items[type][i] != NONE)
                        {
                                SetControlValue(control, i);
                                break;
                        }
                }
	}
}


static unsigned long find_checksum_and_file_spec_from_dialog(
	DialogPtr dialog, 
	short item_hit, 
	uint32 type,
	FSSpec *file)
{
	ControlHandle control;
	short item_type;
	Rect bounds;

	GetDialogItem(dialog, item_hit, &item_type, (Handle *) &control, &bounds);
	
	return find_checksum_and_file_spec_from_control(control, type, file);
}

static unsigned long find_checksum_and_file_spec_from_control(
	ControlHandle control,
	uint32 type,
	FSSpec *file)
{
	short value;
	unsigned long checksum;
	
	value= GetControlValue(control);

        int index= menu_items[type][value];
        assert(index != NONE);

        *file= file_descriptions[index].file.GetSpec();
        checksum= file_descriptions[index].checksum;

	return checksum;
}

static void SetToLoneFile(Typecode Type, FSSpec& File, unsigned long& Checksum)
{
	int LoneFileIndex = NONE;	// Null value (-1)
	
	for (unsigned int index=0; index<file_descriptions.size(); index++)
	{
		if (file_descriptions[index].file_type == Type)
		{
			if (LoneFileIndex != NONE)
				// No need to continue if duplicated
				return;
			else
				// Don't quit here; check for another file with this type
				LoneFileIndex = index;
		}
	}
	
	// Do the setting
	if (LoneFileIndex != NONE)
	{
		File = file_descriptions[LoneFileIndex].file.GetSpec();
		Checksum = file_descriptions[LoneFileIndex].checksum;
	}
}


#ifndef TARGET_API_MAC_CARBON
static MenuHandle get_popup_menu_handle(
	DialogPtr dialog,
	short item)
{
/*
#if !defined(USE_CARBON_ACCESSORS)
	struct PopupPrivateData **privateHndl;
#endif
*/
	MenuHandle menu;
	short item_type;
	ControlHandle control;
	Rect bounds;

	/* Add the maps.. */
	GetDialogItem(dialog, item, &item_type, (Handle *) &control, &bounds);

//#if defined(USE_CARBON_ACCESSORS)
	menu= GetControlPopupMenuHandle(control);
/*
#else
	*/
	/* I don't know how to assert that it is a popup control... <sigh> */
	/*
	privateHndl= (PopupPrivateData **) ((*control)->contrlData);
	assert(privateHndl);

	menu= (*privateHndl)->mHandle;
#endif
*/
	assert(menu);

	return menu;
}
#endif

#if 0
static bool control_strip_installed(
	void)
{
	bool installed= false;
	long control_strip_version;
	OSErr error;
	
	error= Gestalt(gestaltControlStripVersion, &control_strip_version);
	if(!error)
	{
		installed= true;
	}
	
	return installed;
}

static void hide_control_strip(
	void)
{
	assert(control_strip_installed());
	if(SBIsControlStripVisible())
	{
		SBShowHideControlStrip(false);
	}
}

static void show_control_strip(
	void)
{
	assert(control_strip_installed());
	if(!SBIsControlStripVisible())
	{
		SBShowHideControlStrip(true);
	}
}
#endif
