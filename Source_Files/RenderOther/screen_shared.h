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

	Created by Loren Petrich,
	Dec. 23, 2000
	Contains everything shared between screen.cpp and screen_sdl.cpp
	
Dec 29, 2000 (Loren Petrich):
	Added stuff for doing screen messages

Mar 19, 2001 (Loren Petrich):
	Added some even bigger screen resolutions

Jan 25, 2002 (Br'fin (Jeremy Parsons)):
	Added accessors for datafields now opaque in Carbon

 Aug 6, 2003 (Woody Zenfell):
	Minor tweaks to screen_printf() mechanism (safer; resets when screen_reset called)
*/

#include <stdarg.h>
#ifndef HAVE_SNPRINTF
#include "snprintf.h"	// for platforms that don't have it
// Maybe someone will work some autoconf/manual config.h magic so we can really only
// include snprintf.h (and snprintf.cpp in the build) when needed.
#endif

#define DESIRED_SCREEN_WIDTH 640
#define DESIRED_SCREEN_HEIGHT 480

// Biggest possible of those defined
#define MAXIMUM_WORLD_WIDTH 1900
#define MAXIMUM_WORLD_HEIGHT 1200

#define DEFAULT_WORLD_WIDTH 640
#define DEFAULT_WORLD_HEIGHT 320


// LP addition: view sizes and display data

struct ViewSizeData
{
	short OverallWidth, OverallHeight;	// Of the display area, so as to properly center everything
	short MainWidth, MainHeight;		// Of the main 3D-rendered view
	short WithHUD, WithoutHUD;			// Corresponding entries that are with the HUD or without it
	bool ShowHUD;						// Will it be visible?
};

const ViewSizeData ViewSizes[NUMBER_OF_VIEW_SIZES] =
{
	{ 640,  480,  320,  160,  _320_160_HUD,   _640_480,	 true},		//   _320_160_HUD
	{ 640,  480,  480,  240,  _480_240_HUD,   _640_480,	 true},		//   _480_240_HUD
	{ 640,  480,  640,  320,  _640_320_HUD,   _640_480,	 true},		//   _640_320_HUD
	{ 640,  480,  640,  480,  _640_320_HUD,   _640_480,	false},		//   _640_480
	{ 800,  600,  800,  400,  _800_400_HUD,   _800_600,	 true},		//   _800_400_HUD
	{ 800,  600,  800,  600,  _800_400_HUD,   _800_600,	false},		//   _800_600
	{1024,  768, 1024,  512, _1024_512_HUD,  _1024_768,	 true},		//  _1024_512_HUD
	{1024,  768, 1024,  768, _1024_512_HUD,  _1024_768,	false},		//  _1024_768
	{1280, 1024, 1280,  640, _1280_640_HUD, _1280_1024,	 true},		//  _1280_640_HUD
	{1280, 1024, 1280, 1024, _1280_640_HUD, _1280_1024,	false},		// _1280_1024
	{1600, 1200, 1600,  800, _1600_800_HUD, _1600_1200,	 true},		//  _1600_800_HUD
	{1600, 1200, 1600, 1200, _1600_800_HUD, _1600_1200,	false},		// _1600_1200
        {1024,  640, 1024,  440, _1024_440_HUD,  _1024_640,	 true},		//   _1024_440_HUD
	{1024,  640, 1024,  640, _1024_440_HUD,  _1024_640,	false},		//   _1024_640
	{1280,  800, 1280,  600, _1280_600_HUD,  _1280_800,	 true},		//  _1280_600_HUD
	{1280,  800, 1280,  800, _1280_600_HUD,  _1280_800,	false},		//  _1280_800
        {1280,  854, 1280,  654, _1280_640WS_HUD,  _1280_854,	 true},		//  _1280_640WS_HUD
        {1280,  854, 1280,  854, _1280_640WS_HUD,  _1280_854,	false},		//  _1280_854
        {1440,  900, 1440,  700, _1440_700_HUD,  _1440_900,	 true},		//  _1440_700_HUD
        {1440,  900, 1440,  700, _1440_700_HUD,  _1440_900,	false},		//  _1440_900
	{1680, 1050, 1680,  840, _1680_840_HUD, _1680_1050,	 true},		//  _1680_840_HUD
	{1680, 1050, 1680,  1050, _1680_840_HUD, _1680_1050,	false},		// _1680_1050
	{1920, 1200, 1920,  950, _1920_950_HUD, _1920_1200,	 true},		//  _1900_950_HUD
	{1920, 1200, 1920,  1200, _1920_950_HUD, _1920_1200,	false},		// _1900_1200
};

// Note: the overhead map will always fill all of the screen except for the HUD,
// and the terminal display will always have a size of 640*320.

/* ---------- globals */

struct color_table *uncorrected_color_table; /* the pristine color environment of the game (can be 16bit) */
struct color_table *world_color_table; /* the gamma-corrected color environment of the game (can be 16bit) */
struct color_table *interface_color_table; /* always 8bit, for mixed-mode (i.e., valkyrie) fades */
struct color_table *visible_color_table; /* the color environment the player sees (can be 16bit) */

struct view_data *world_view; /* should be static */

// Convenient package for the drawing target (contains dimensions and pixel-row pointers)
struct bitmap_definition *world_pixels_structure;

// LP change: added stuff for keeping track of screen sizes;
// this is for forcing the clearing of the screen when resizing.
// These are initialized to improbable values.
short PrevBufferWidth = INT16_MIN, PrevBufferHeight = INT16_MIN,
	PrevOffsetWidth = INT16_MIN, PrevOffsetHeight = INT16_MIN;

static struct screen_mode_data screen_mode;

#define FRAME_SAMPLE_SIZE 20
bool displaying_fps= false;
short frame_count, frame_index;
long frame_ticks[64];

// LP addition:
// whether to show one's position
bool ShowPosition = false;

// Whether rendering of the HUD has been requested
static bool HUD_RenderRequest = false;

static bool screen_initialized= false;

short bit_depth= NONE;
short interface_bit_depth= NONE;

// LP addition: this is defined in overhead_map.c
// It indicates whether to render the overhead map in OpenGL
extern bool OGL_MapActive;


// Current screen messages:
const int NumScreenMessages = 7;
struct ScreenMessage
{
	enum {
		Len = 256
	};

	int TimeRemaining;	// How many more engine ticks until the message expires?
	char Text[Len];		// Text to display
	
	ScreenMessage(): TimeRemaining(0) {Text[0] = 0;}
};

static int MostRecentMessage = NumScreenMessages-1;
static ScreenMessage Messages[NumScreenMessages];


/* ---------- private prototypes */

static void set_overhead_map_status(bool status);
static void set_terminal_status(bool status);

/* ---------- code */

// LP addition: this resets the screen; useful when starting a game
void reset_screen()
{
	// Resetting cribbed from initialize_screen()
	world_view->overhead_map_scale= DEFAULT_OVERHEAD_MAP_SCALE;
	world_view->overhead_map_active= false;
	world_view->terminal_mode_active= false;
	world_view->horizontal_scale= 1, world_view->vertical_scale= 1;
	
	// LP change:
	ResetFieldOfView();

	// ZZZ: reset screen_printf's
	for(int i = 0; i < NumScreenMessages; i++)
		Messages[i].TimeRemaining = 0;
}

// LP change: resets field of view to whatever the player had had when reviving
void ResetFieldOfView()
{
	world_view->tunnel_vision_active = false;

	if (current_player->extravision_duration)
	{
		world_view->field_of_view = EXTRAVISION_FIELD_OF_VIEW;
		world_view->target_field_of_view = EXTRAVISION_FIELD_OF_VIEW;
	}
	else
	{
		world_view->field_of_view = NORMAL_FIELD_OF_VIEW;
		world_view->target_field_of_view = NORMAL_FIELD_OF_VIEW;
	}
}


bool zoom_overhead_map_out(
	void)
{
	bool Success = false;
	if (world_view->overhead_map_scale > OVERHEAD_MAP_MINIMUM_SCALE)
	{
		world_view->overhead_map_scale--;
		Success = true;
	}
	
	return Success;
}

bool zoom_overhead_map_in(
	void)
{
	bool Success = false;
	if (world_view->overhead_map_scale < OVERHEAD_MAP_MAXIMUM_SCALE)
	{
		world_view->overhead_map_scale++;
		Success = true;
	}
	
	return Success;
}

void start_teleporting_effect(
	bool out)
{
	if (View_DoFoldEffect())
		start_render_effect(world_view, out ? _render_effect_fold_out : _render_effect_fold_in);
}

void start_extravision_effect(
	bool out)
{
	// LP change: doing this by setting targets
	world_view->target_field_of_view = out ? EXTRAVISION_FIELD_OF_VIEW : NORMAL_FIELD_OF_VIEW;
}

// LP addition:
void start_tunnel_vision_effect(
	bool out)
{
	// LP change: doing this by setting targets
  world_view->target_field_of_view = (out && NetAllowTunnelVision()) ? TUNNEL_VISION_FIELD_OF_VIEW : 
		((current_player->extravision_duration) ? EXTRAVISION_FIELD_OF_VIEW : NORMAL_FIELD_OF_VIEW);
}

//CP addition: returns the screen info
screen_mode_data *get_screen_mode(
	void)
{
	return &screen_mode;
}

// LP: gets a size ID's related size ID's that show or hide the HUD, respectively
short GetSizeWithHUD(short Size)
{
	assert(Size >= 0 && Size < NUMBER_OF_VIEW_SIZES);
	return ViewSizes[Size].WithHUD;
}

short GetSizeWithoutHUD(short Size)
{
	assert(Size >= 0 && Size < NUMBER_OF_VIEW_SIZES);
	return ViewSizes[Size].WithoutHUD;
}

/* These should be replaced with better preferences control functions */
// LP change: generalizing this
bool game_window_is_full_screen(
	void)
{
	short msize = screen_mode.size;
	assert(msize >= 0 && msize < NUMBER_OF_VIEW_SIZES);
	return (!ViewSizes[msize].ShowHUD);
}


void change_gamma_level(
	short gamma_level)
{
	screen_mode.gamma_level= gamma_level;
	gamma_correct_color_table(uncorrected_color_table, world_color_table, gamma_level);
	stop_fade();
	obj_copy(*visible_color_table, *world_color_table);
	assert_world_color_table(interface_color_table, world_color_table);
	change_screen_mode(&screen_mode, false);
	set_fade_effect(NONE);
}

/* ---------- private code */

// LP addition: routine for displaying text

#ifdef SDL
// Globals for communicating with the SDL contents of DisplayText
static SDL_Surface *DisplayTextDest = NULL;
static sdl_font_info *DisplayTextFont = NULL;
static short DisplayTextStyle = 0;
#endif

/*static*/ void DisplayText(short BaseX, short BaseY, const char *Text)
{
#ifdef HAVE_OPENGL
	// OpenGL version:
	// activate only in the main view, and also if OpenGL is being used for the overhead map
	if((OGL_MapActive || !world_view->overhead_map_active) && !world_view->terminal_mode_active)
		if (OGL_RenderText(BaseX, BaseY, Text)) return;
#endif

#if defined(mac)
	// C to Pascal
	Str255 PasText;
	
	int Len = MIN(strlen(Text),255);
	PasText[0] = Len;
	memcpy(PasText+1,Text,Len);
	
	// LP change: drop shadow in both MacOS and SDL versions
	RGBForeColor(&rgb_black);
	MoveTo(BaseX+1,BaseY+1);
	DrawString(PasText);
	
	RGBForeColor(&rgb_white);
	MoveTo(BaseX,BaseY);
	DrawString(PasText);
	
#elif defined(SDL)

	draw_text(DisplayTextDest, Text, BaseX+1, BaseY+1, SDL_MapRGB(world_pixels->format, 0x00, 0x00, 0x00), DisplayTextFont, DisplayTextStyle);
	draw_text(DisplayTextDest, Text, BaseX, BaseY, SDL_MapRGB(world_pixels->format, 0xff, 0xff, 0xff), DisplayTextFont, DisplayTextStyle);
	
#endif
}


#if defined(mac)
static void update_fps_display(GrafPtr port)
#elif defined(SDL)
static void update_fps_display(SDL_Surface *s)
#endif
{
	if (displaying_fps && !player_in_terminal_mode(current_player_index))
	{
#if defined(mac)
		uint32 ticks= TickCount();
#elif defined(SDL)
		uint32 ticks = SDL_GetTicks();
#endif
		char fps[sizeof("120.00fps")];
		
		frame_ticks[frame_index]= ticks;
		frame_index= (frame_index+1)%FRAME_SAMPLE_SIZE;
		if (frame_count<FRAME_SAMPLE_SIZE)
		{
			frame_count+= 1;
			strcpy(fps, "--");
		}
		else
		{
		    float count = (FRAME_SAMPLE_SIZE * MACHINE_TICKS_PER_SECOND) / float(ticks-frame_ticks[frame_index]);
		    if (count >= TICKS_PER_SECOND)
			sprintf(fps, "%lu%s",(unsigned long)TICKS_PER_SECOND,".00fps");
		    else
			sprintf(fps, "%3.2ffps", count);
		}
		
		FontSpecifier& Font = GetOnScreenFont();
		
#if defined(mac)
		GrafPtr old_port;
		GetPort(&old_port);
		SetPort(port);
		Font.Use();
//#if defined(USE_CARBON_ACCESSORS)
		Rect portRect;
		GetPortBounds(port, &portRect);
		short X0 = portRect.left;
		short Y0 = portRect.bottom;
/*
#else
		short X0 = port->portRect.left;
		short Y0 = port->portRect.bottom;
#endif
*/
#elif defined(SDL)
		DisplayTextDest = s;
		DisplayTextFont = Font.Info;
		DisplayTextStyle = Font.Style;
		short X0 = 0;
		short Y0 = s->h;
#endif
		// The line spacing is a generalization of "5" for larger fonts
		short Offset = Font.LineSpacing / 3;
		short X = X0 + Offset;
		short Y = Y0 - Offset;
		DisplayText(X,Y,fps);
		
#if defined(mac)
		RGBForeColor(&rgb_black);
		SetPort(old_port);
#endif
	}
	else
	{
		frame_count= frame_index= 0;
	}
}


#if defined(mac)
static void DisplayPosition(GrafPtr port)
#elif defined(SDL)
static void DisplayPosition(SDL_Surface *s)
#endif
{
	if (!ShowPosition) return;
		
	FontSpecifier& Font = GetOnScreenFont();
	
#if defined(mac)
	// Push
	GrafPtr old_port;
	GetPort(&old_port);
	SetPort(port);
	Font.Use();
//#if defined(USE_CARBON_ACCESSORS)
	Rect portRect;
	GetPortBounds(port, &portRect);
	short X0 = portRect.left;
	short Y0 = portRect.top;
/*
#else
	short X0 = port->portRect.left;
	short Y0 = port->portRect.top;
#endif
*/
#elif defined(SDL)
	DisplayTextDest = s;
	DisplayTextFont = Font.Info;
	DisplayTextStyle = Font.Style;
	short X0 = 0;
	short Y0 = 0;
#endif
	
	short LineSpacing = Font.LineSpacing;
	short X = X0 + LineSpacing/3;
	short Y = Y0 + LineSpacing;
	const float FLOAT_WORLD_ONE = float(WORLD_ONE);
	const float AngleConvert = 360/float(FULL_CIRCLE);
	sprintf(temporary, "X       = %8.3f",world_view->origin.x/FLOAT_WORLD_ONE);
	DisplayText(X,Y,temporary);
	Y += LineSpacing;
	sprintf(temporary, "Y       = %8.3f",world_view->origin.y/FLOAT_WORLD_ONE);
	DisplayText(X,Y,temporary);
	Y += LineSpacing;
	sprintf(temporary, "Z       = %8.3f",world_view->origin.z/FLOAT_WORLD_ONE);
	DisplayText(X,Y,temporary);
	Y += LineSpacing;
	sprintf(temporary, "Polygon = %8d",world_view->origin_polygon_index);
	DisplayText(X,Y,temporary);
	Y += LineSpacing;
	short Angle = world_view->yaw;
	if (Angle > HALF_CIRCLE) Angle -= FULL_CIRCLE;
	sprintf(temporary, "Yaw     = %8.3f",AngleConvert*Angle);
	DisplayText(X,Y,temporary);
	Y += LineSpacing;
	Angle = world_view->pitch;
	if (Angle > HALF_CIRCLE) Angle -= FULL_CIRCLE;
	sprintf(temporary, "Pitch   = %8.3f",AngleConvert*Angle);
	DisplayText(X,Y,temporary);
	
	// Pop
#ifdef mac
	RGBForeColor(&rgb_black);
	SetPort(old_port);
#endif
}

#if defined(mac)
static void DisplayMessages(GrafPtr port)
#elif defined(SDL)
static void DisplayMessages(SDL_Surface *s)
#endif
{	
	FontSpecifier& Font = GetOnScreenFont();
	
#if defined(mac)
	// Push
	GrafPtr old_port;
	GetPort(&old_port);
	SetPort(port);
	Font.Use();
//#if defined(USE_CARBON_ACCESSORS)
	Rect portRect;
	GetPortBounds(port, &portRect);
	short X0 = portRect.left;
	short Y0 = portRect.top;
/*
#else
	short X0 = port->portRect.left;
	short Y0 = port->portRect.top;
#endif
*/
#elif defined(SDL)
	DisplayTextDest = s;
	DisplayTextFont = Font.Info;
	DisplayTextStyle = Font.Style;
	short X0 = 0;
	short Y0 = 0;
#endif
	
	short LineSpacing = Font.LineSpacing;
	short X = X0 + LineSpacing/3;
	short Y = Y0 + LineSpacing;
	if (ShowPosition) Y += 6*LineSpacing;	// Make room for the position data
	for (int k=0; k<NumScreenMessages; k++)
	{
		int Which = (MostRecentMessage+NumScreenMessages-k) % NumScreenMessages;
		while (Which < 0)
			Which += NumScreenMessages;
		ScreenMessage& Message = Messages[Which];
		if (Message.TimeRemaining <= 0) continue;
		Message.TimeRemaining--;
		
		DisplayText(X,Y,Message.Text);
		Y += LineSpacing;
	}
	
	// Pop
#ifdef mac
	RGBForeColor(&rgb_black);
	SetPort(old_port);
#endif
}


static void set_overhead_map_status( /* it has changed, this is the new status */
	bool status)
{
	world_view->overhead_map_active= status;
}

static void set_terminal_status( /* It has changed, this is the new state.. */
	bool status)
{
	bool restore_effect= false;
	short effect = 0, phase = 0;
	
	if(!status)
	{
		if(world_view->effect==_render_effect_fold_out)
		{
			effect= world_view->effect;
			phase= world_view->effect_phase;
			restore_effect= true;
		}
	}
	world_view->terminal_mode_active= status;
	
	if(restore_effect)
	{
		world_view->effect= effect;
		world_view->effect_phase= phase;
	}

	/* Dirty the view.. */
	dirty_terminal_view(current_player_index);
}

// For getting and setting tunnel-vision mode
bool GetTunnelVision() {return world_view->tunnel_vision_active;}
bool SetTunnelVision(bool TunnelVisionOn)
{
	// LP: simplifying tunnel-vision-activation/deactivation behavior
	world_view->tunnel_vision_active = TunnelVisionOn;
	start_tunnel_vision_effect(TunnelVisionOn);
	return world_view->tunnel_vision_active;
}

// This is for requesting the drawing of the Heads-Up Display;
// this is done because its drawing is now done when the main display is drawn
void RequestDrawingHUD()
{
	HUD_RenderRequest = true;
}

// ZZZ: I feel bad doing this, but ... not sure what best way to handle it is.
// #ifdef __MWERKS__
// using std::vsnprintf;
// #endif

// LP addition: display message on the screen;
// this really puts the current message into a buffer
// Code cribbed from csstrings
void screen_printf(const char *format, ...)
{
	MostRecentMessage = (MostRecentMessage + 1) % NumScreenMessages;
	while (MostRecentMessage < 0)
		MostRecentMessage += NumScreenMessages;
	ScreenMessage& Message = Messages[MostRecentMessage];
	
	Message.TimeRemaining = 3*TICKS_PER_SECOND;

	va_list list;

	va_start(list,format);
	// ZZZ: [v]sprintf is evil, generally: hard to guarantee you don't overflow target buffer
	// using [v]snprintf instead
	vsnprintf(Message.Text,sizeof(Message.Text),format,list);
	va_end(list);
}
