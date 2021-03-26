/*

	Copyright (C) 1991-2001 and beyond by Bungie Studios, Inc.
	and the "Aleph One" developers.
 
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

#include "computer_interface.h"
#include "fades.h"
#include "network.h"
#include "OGL_Render.h"
#include "overhead_map.h"
#include "screen.h"
#include <stdarg.h>

#include <chrono>

extern SDL_Surface *world_pixels;

#define DESIRED_SCREEN_WIDTH 640
#define DESIRED_SCREEN_HEIGHT 480

#define DEFAULT_WORLD_WIDTH 640
#define DEFAULT_WORLD_HEIGHT 320

#include "Console.h"
#include "screen_drawing.h"

#include "network_games.h"
#include "Image_Blitter.h"
#include "OGL_Blitter.h"

/* ---------- globals */

struct color_table *uncorrected_color_table; /* the pristine color environment of the game (can be 16bit) */
struct color_table *world_color_table; /* the gamma-corrected color environment of the game (can be 16bit) */
struct color_table *interface_color_table; /* always 8bit, for mixed-mode (i.e., valkyrie) fades */
struct color_table *visible_color_table; /* the color environment the player sees (can be 16bit) */

struct view_data *world_view; /* should be static */

static struct screen_mode_data screen_mode;

class FpsCounter {
public:
	using clock = std::chrono::high_resolution_clock;
	static constexpr auto update_time = std::chrono::milliseconds(250);
	
	FpsCounter() :
		next_update_{clock::now() + update_time},
		sum_{0},
		count_{0},
		fps_{0}
		{
			
		}
	
	void update() {
		auto now = clock::now();
		sum_ += 1.0 / std::chrono::duration_cast<std::chrono::duration<float>>(now - prev_).count();
		++count_;
		prev_ = now;
		
		if (now >= next_update_)
		{
			next_update_ = now + update_time;
			if (count_) {
				fps_ = sum_ / count_;
			} else {
				fps_ = 0.f;
			}
			
			sum_ = 0;
			count_ = 0;
		}
	}
	
	float get() const { return fps_; }
	bool ready() const { return fps_ != 0; }

	void reset() {
		sum_ = 0;
		count_ = 0;
		prev_ = clock::now();

		fps_ = 0.f;
	}

private:
	clock::time_point prev_;
	clock::time_point next_update_;
	
	float sum_;
	int count_;

	bool ready_;
	float fps_;
};

constexpr std::chrono::milliseconds FpsCounter::update_time;

static FpsCounter fps_counter;

bool displaying_fps= false;

// LP addition:
// whether to show one's position
bool ShowPosition = false;
bool ShowScores = false;

// Whether rendering of the HUD has been requested
static bool HUD_RenderRequest = false;
static bool Term_RenderRequest = false;

static bool screen_initialized= false;
static bool nonlocal_script_hud= false;

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

	uint32_t ExpirationTime; // machine ticks the screen message expires at
	char Text[Len];		// Text to display
	
	ScreenMessage(): ExpirationTime(machine_tick_count()) {Text[0] = 0;}
};

static int MostRecentMessage = NumScreenMessages-1;
static ScreenMessage Messages[NumScreenMessages];

/* SB */
static struct ScriptHUDElement {
	/* this needs optimized (sorry, making fun of my grandmother...) */
	/* it's char[4] instead of int32 to make the OpenGL support simpler to implement */
	unsigned char icon[1024];
	bool isicon;
	int color;
	std::string text;
	Image_Blitter sdl_blitter;
#ifdef HAVE_OPENGL	
	OGL_Blitter ogl_blitter;
#endif	
} ScriptHUDElements[MAXIMUM_NUMBER_OF_NETWORK_PLAYERS][MAXIMUM_NUMBER_OF_SCRIPT_HUD_ELEMENTS];
/* /SB */

/* ---------- private prototypes */

static void set_overhead_map_status(bool status);
static void set_terminal_status(bool status);

/* ---------- code */

/* SB */
namespace icon {
	
  static inline char nextc(const char*& p, size_t& rem) {
    if(rem == 0) throw "end of string";
    --rem;
    return *(p++);
  }
	
  // we can't use ctype for this because of locales and hexadecimal
  static inline bool isadigit(char p) {
    if(p >= '0' && p <= '9') return true;
    else if(p >= 'A' && p <= 'F') return true;
    else if(p >= 'a' && p <= 'f') return true;
    else return false;
  }
	
  static inline unsigned char digit(char p) {
    if(p >= '0' && p <= '9') return p - '0';
    else if(p >= 'A' && p <= 'F') return p - 'A' + 0xA;
    else if(p >= 'a' && p <= 'f') return p - 'a' + 0xA;
    else throw "invalid digit";
  }
	
  static inline unsigned char readuc(const char*& p, size_t& rem) {
    char a = nextc(p, rem), b;
    b = nextc(p, rem);
    return (digit(a) << 4) | digit(b);
  }
	
  static bool parseicon(const char* p, size_t rem,
			unsigned char palette[1024], int& numcolors,
			unsigned char graphic[256]) {
    char chars[256];
    try {
      char oc, c;
      size_t n, m;
      numcolors = 0;
      while(1) {
	c = nextc(p, rem);
	if(c >= '0' && c <= '9')
	  numcolors = numcolors * 10 + (c - '0');
	else break;
      }
      if(numcolors == 0) return 1;
      oc = c;
      do {
	c = nextc(p, rem);
      } while(c == oc);
      n = 0;
      while(n < numcolors) {
	chars[n] = c;
	palette[n * 4] = readuc(p, rem);
	palette[n * 4 + 1] = readuc(p, rem);
	palette[n * 4 + 2] = readuc(p, rem);
	c = nextc(p, rem); /* ignore a char, UNLESS... */
	if(isadigit(c)) {  /* ...it's a digit */
	  --p; ++rem; /* let readuc see it */
	  palette[n * 4 + 3] = readuc(p, rem);
	  nextc(p, rem); /* remember to ignore another char */
	}
	else
	  palette[n * 4 + 3] = 255;
	++n;
	c = nextc(p, rem);
      }
      n = 0;
      while(n < 256) {
	for(m = 0; m < numcolors; ++m) {
	  if(chars[m] == c) {
	    graphic[n++] = m;
	    break;
	  }
	}
	c = nextc(p, rem);
      }
    } catch(...) {
      return false;
    }
    return true;
  }
	
  void seticon(int player, int idx, unsigned char palette[1024], unsigned char graphic[256]) {
    unsigned char* p1, *p2, px;
    int n;
    p1 = ScriptHUDElements[player][idx].icon;
    p2 = graphic;
    for(n = 0; n < 256; ++n) {
      px = *(p2++);
      *(p1++) = palette[px * 4 + 3];
      *(p1++) = palette[px * 4];
      *(p1++) = palette[px * 4 + 1];
      *(p1++) = palette[px * 4 + 2];
    }
    ScriptHUDElements[player][idx].isicon = true;
	SDL_Surface *srf;
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
	srf = SDL_CreateRGBSurfaceFrom(ScriptHUDElements[player][idx].icon, 16, 16, 32, 64, 0xFF<<8, 0xFF<<16, 0xFF<<24, 0xFF);
#else
	srf = SDL_CreateRGBSurfaceFrom(ScriptHUDElements[player][idx].icon, 16, 16, 32, 64, 0xFF<<16, 0xFF<<8, 0xFF, 0xFF<<24);
#endif
#ifdef HAVE_OPENGL	
	if (OGL_IsActive()) {
		ScriptHUDElements[player][idx].ogl_blitter.Load(*srf);
	} else
#endif	  
	{	  
		ScriptHUDElements[player][idx].sdl_blitter.Load(*srf);
	}
	SDL_FreeSurface(srf);
  }
	
}

bool IsScriptHUDNonlocal() {
  return nonlocal_script_hud;
}

void SetScriptHUDNonlocal(bool nonlocal) {
  nonlocal_script_hud = nonlocal;
}

void SetScriptHUDColor(int player, int idx, int color) {
  player %= MAXIMUM_NUMBER_OF_NETWORK_PLAYERS;
  idx %= MAXIMUM_NUMBER_OF_SCRIPT_HUD_ELEMENTS; /* o_o */
  ScriptHUDElements[player][idx].color = color % 8; /* O_O */
}

void SetScriptHUDText(int player, int idx, const char* text) {
  player %= MAXIMUM_NUMBER_OF_NETWORK_PLAYERS;
  idx %= MAXIMUM_NUMBER_OF_SCRIPT_HUD_ELEMENTS;
  if(!text) ScriptHUDElements[player][idx].text.clear();
  else ScriptHUDElements[player][idx].text = text;
}

bool SetScriptHUDIcon(int player, int idx, const char* text, size_t rem) {
  unsigned char palette[1024], graphic[256];
  int numcolors;
  player %= MAXIMUM_NUMBER_OF_NETWORK_PLAYERS;
  idx %= MAXIMUM_NUMBER_OF_SCRIPT_HUD_ELEMENTS;
  if(text) {
    if(!icon::parseicon(text, rem, palette, numcolors, graphic)) return false;
    icon::seticon(player, idx, palette, graphic);
  } else ScriptHUDElements[player][idx].isicon = false;
  return true;
}

void SetScriptHUDSquare(int player, int idx, int _color) {
  unsigned char palette[4]; /* short, I KNOW. */
  unsigned char graphic[256];
  player %= MAXIMUM_NUMBER_OF_NETWORK_PLAYERS;
  idx %= MAXIMUM_NUMBER_OF_SCRIPT_HUD_ELEMENTS;
  ScriptHUDElements[player][idx].color = _color % 8;
  memset(graphic, 0, 256);
  SDL_Color color;
  _get_interface_color(_color+_computer_interface_text_color, &color);
  palette[0] = color.r;
  palette[1] = color.g;
  palette[2] = color.b;
  palette[3] = 0xff;
  icon::seticon(player, idx, palette, graphic);
}
/* /SB */

void reset_messages()
{
	// ZZZ: reset screen_printf's
	for(int i = 0; i < NumScreenMessages; i++)
		Messages[i].ExpirationTime = machine_tick_count();
	/* SB: reset HUD elements */
	for(int p = 0; p < MAXIMUM_NUMBER_OF_NETWORK_PLAYERS; p++) {
		for(int i = 0; i < MAXIMUM_NUMBER_OF_SCRIPT_HUD_ELEMENTS; i++) {
			ScriptHUDElements[p][i].color = 1;
			ScriptHUDElements[p][i].text.clear();
			ScriptHUDElements[p][i].isicon = false;
		}
	}
        nonlocal_script_hud = false;
}

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

	reset_messages();
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
void start_tunnel_vision_effect(bool out);

//CP addition: returns the screen info
screen_mode_data *get_screen_mode(
	void)
{
	return &screen_mode;
}

/* These should be replaced with better preferences control functions */
// LP change: generalizing this
bool game_window_is_full_screen(
	void)
{
	return !alephone::Screen::instance()->hud();
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

// Globals for communicating with the SDL contents of DisplayText
static SDL_Surface *DisplayTextDest = NULL;
static font_info *DisplayTextFont = NULL;
static short DisplayTextStyle = 0;

/*static*/ void DisplayText(short BaseX, short BaseY, const char *Text, unsigned char r = 0xff, unsigned char g = 0xff, unsigned char b = 0xff)
{
#ifdef HAVE_OPENGL
	// OpenGL version:
	// activate only in the main view, and also if OpenGL is being used for the overhead map
	if((OGL_MapActive || !world_view->overhead_map_active) && !world_view->terminal_mode_active)
		if (OGL_RenderText(BaseX, BaseY, Text, r, g, b)) return;
#endif

	draw_text(DisplayTextDest, Text, BaseX+1, BaseY+1, SDL_MapRGB(world_pixels->format, 0x00, 0x00, 0x00), DisplayTextFont, DisplayTextStyle);
	draw_text(DisplayTextDest, Text, BaseX, BaseY, SDL_MapRGB(world_pixels->format, r, g, b), DisplayTextFont, DisplayTextStyle);	

}

/*static*/ void DisplayTextCursor(SDL_Surface *s, short BaseX, short BaseY, const char *Text, short Offset, unsigned char r = 0xff, unsigned char g = 0xff, unsigned char b = 0xff)
{
	SDL_Rect cursor_rect;
	cursor_rect.x = BaseX + text_width(Text, Offset, DisplayTextFont, DisplayTextStyle);
	cursor_rect.w = 1;
	cursor_rect.y = BaseY - DisplayTextFont->get_ascent();
	cursor_rect.h = DisplayTextFont->get_height();
	
	SDL_Rect shadow_rect = cursor_rect;
	shadow_rect.x += 1;
	shadow_rect.y += 1;
	
#ifdef HAVE_OPENGL
	// OpenGL version:
	// activate only in the main view, and also if OpenGL is being used for the overhead map
	if((OGL_MapActive || !world_view->overhead_map_active) && !world_view->terminal_mode_active)
		if (OGL_RenderTextCursor(cursor_rect, r, g, b)) return;
#endif
	
	SDL_FillRect(s, &shadow_rect, SDL_MapRGB(world_pixels->format, 0x00, 0x00, 0x00));
	SDL_FillRect(s, &cursor_rect, SDL_MapRGB(world_pixels->format, r, g, b));
}

uint16 DisplayTextWidth(const char *Text)
{
	return text_width(Text, DisplayTextFont, DisplayTextStyle);
}

static void update_fps_display(SDL_Surface *s)
{
	if (displaying_fps && !player_in_terminal_mode(current_player_index))
	{
		uint32 ticks = machine_tick_count();
		char fps[sizeof("1000 fps (10000 ms)")];
		char ms[sizeof("(10000 ms)")];

		fps_counter.update();

		if (!fps_counter.ready())
		{
			strcpy(fps, "--");
		}
		else
		{
			
			int latency = NetGetLatency();
			if (latency > -1)
				sprintf(ms, "(%i ms)", latency);
			else
				ms[0] = '\0';
			
			sprintf(fps, "%0.f fps %s", fps_counter.get(), ms);
		}

		FontSpecifier& Font = GetOnScreenFont();
		
		DisplayTextDest = s;
		DisplayTextFont = Font.Info;
		DisplayTextStyle = Font.Style;

		auto text_margins = alephone::Screen::instance()->lua_text_margins;
		short X0 = text_margins.left;
		short Y0 = s->h - text_margins.bottom;

		// The line spacing is a generalization of "5" for larger fonts
		short Offset = Font.LineSpacing / 3;
		short X = X0 + Offset;
		short Y = Y0 - Offset;
		if (Console::instance()->input_active())
		{
			Y -= Font.LineSpacing;
		}
		DisplayText(X,Y,fps);
		
	}
	else
	{
		fps_counter.reset();
	}
}


static void DisplayPosition(SDL_Surface *s)
{
	if (!ShowPosition) return;
		
	FontSpecifier& Font = GetOnScreenFont();
	
	DisplayTextDest = s;
	DisplayTextFont = Font.Info;
	DisplayTextStyle = Font.Style;

	auto text_margins = alephone::Screen::instance()->lua_text_margins;
	short X0 = text_margins.left;
	short Y0 = text_margins.top;
	
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
	
}

static void DisplayInputLine(SDL_Surface *s)
{
  if (Console::instance()->input_active() && 
      !Console::instance()->displayBuffer().empty()) {
    FontSpecifier& Font = GetOnScreenFont();
    
  DisplayTextDest = s;
  DisplayTextFont = Font.Info;
  DisplayTextStyle = Font.Style;
  
  auto text_margins = alephone::Screen::instance()->lua_text_margins;
  short X0 = text_margins.left;
  short Y0 = s->h - text_margins.bottom;

  short Offset = Font.LineSpacing / 3;
  short X = X0 + Offset;
  short Y = Y0 - Offset;
  const char *buf = Console::instance()->displayBuffer().c_str();
  DisplayText(X, Y, buf);
  DisplayTextCursor(s, X, Y, buf, Console::instance()->cursor_position());
  }
}

static void DisplayMessages(SDL_Surface *s)
{	
	FontSpecifier& Font = GetOnScreenFont();
	
	DisplayTextDest = s;
	DisplayTextFont = Font.Info;
	DisplayTextStyle = Font.Style;

	auto text_margins = alephone::Screen::instance()->lua_text_margins;
	short X0 = text_margins.left;
	short Y0 = text_margins.top;
	
	short LineSpacing = Font.LineSpacing;
	short X = X0 + LineSpacing/3;
	short Y = Y0 + LineSpacing;
	if (ShowPosition) Y += 6*LineSpacing;	// Make room for the position data
	/* SB */
	short view = nonlocal_script_hud ? local_player_index : current_player_index;
	for(int i = 0; i < MAXIMUM_NUMBER_OF_SCRIPT_HUD_ELEMENTS; ++i) {
		if(!ScriptHUDElements[view][i].text.empty()) {
			short x2 = X, sk = Font.TextWidth("AAAAAAAAAAAAAA"),
				icon_skip, icon_drop;
			switch(get_screen_mode()->hud_scale_level) {
			case 0:
				icon_drop = 2;
				break;
			case 1:
				if(MainScreenLogicalHeight() >= 960)
					icon_drop = 4;
				else
					icon_drop = 2;
				break;
			case 2:
				if(MainScreenLogicalHeight() >= 480)
					icon_drop = MainScreenLogicalHeight() * 2 / 480;
				else
					icon_drop = 2;
				break;
			}
			bool had_icon = false;
			/* Yes, I KNOW this is the same i as above. I know what I'm doing. */
			for(i = 0; i < MAXIMUM_NUMBER_OF_SCRIPT_HUD_ELEMENTS; ++i) {
				if(ScriptHUDElements[view][i].text.empty()) continue;
				if(ScriptHUDElements[view][i].isicon) {
					had_icon = true;

					SDL_Rect rect;
					rect.x = x2;
					rect.y = Y - Font.Ascent + Font.Leading;
					rect.w = rect.h = 16;
                                        icon_skip = 20;
					switch(get_screen_mode()->hud_scale_level) {
					case 1:
						if(MainScreenLogicalHeight() >= 960) {
							rect.w *= 2;
							rect.h *= 2;
							icon_skip *= 2;
						}
						break;
					case 2:
						if(MainScreenLogicalHeight() > 480) {
							rect.w = rect.w * MainScreenLogicalHeight() / 480;
							rect.h = rect.h * MainScreenLogicalHeight() / 480;
							icon_skip = icon_skip * MainScreenLogicalHeight() / 480;
						}
						break;
					}
#ifdef HAVE_OPENGL
					if(OGL_IsActive()) {
						ScriptHUDElements[view][i].ogl_blitter.Draw(rect);
					}
					else 
#endif 
					{
						ScriptHUDElements[view][i].sdl_blitter.Draw(s, rect);
					}
					x2 += icon_skip;
				}
				SDL_Color color;
				_get_interface_color(ScriptHUDElements[view][i].color+_computer_interface_text_color, &color);
				DisplayText(x2,Y + (ScriptHUDElements[view][i].isicon ? icon_drop : 0),ScriptHUDElements[view][i].text.c_str(), color.r, color.g, color.b);
				x2 += sk;
				if(ScriptHUDElements[view][i].isicon)
					x2 -= icon_skip;
			}
			Y += LineSpacing;
			if(had_icon)
				Y += icon_drop;
			break;
		}
	}
	/* /SB */
	//	for (int k=0; k<NumScreenMessages; k++)
	for (int k = NumScreenMessages - 1; k >= 0; k--)
	{
	  int Which = (MostRecentMessage+NumScreenMessages-k) % NumScreenMessages;
		while (Which < 0)
			Which += NumScreenMessages;
		ScreenMessage& Message = Messages[Which];
		if (static_cast<int32_t>(Message.ExpirationTime - machine_tick_count()) < 0)
		{
			continue;
		}
		
		DisplayText(X,Y,Message.Text);
		Y += LineSpacing;
	}

}

extern short local_player_index;
extern bool game_is_networked;

static void DisplayNetMicStatus(SDL_Surface *s)
{
	if (!game_is_networked) return;

	// the net mic status is a message, and a colored text "icon"
	string icon;
	string status;
	SDL_Color iconColor;

	if (!current_netgame_allows_microphone())
	{
		if (dynamic_world->speaking_player_index == local_player_index)
		{
			status = "disabled";
			icon = "  x";
			iconColor.r = 0xff;
			iconColor.g = iconColor.b = 0x0;
			iconColor.a = 0xff;

		}
		else
		{
			return;
		}
	}
	else if (dynamic_world->speaking_player_index == local_player_index)
	{
		if (GET_GAME_OPTIONS() & _force_unique_teams)
			status = "all";
		else
			status = "team";
		icon = "<!>";

		player_data *player = get_player_data(dynamic_world->speaking_player_index);
		if (GET_GAME_OPTIONS() & _force_unique_teams)
			_get_interface_color(PLAYER_COLOR_BASE_INDEX + player->color, &iconColor);
		else
			_get_interface_color(PLAYER_COLOR_BASE_INDEX + player->team, &iconColor);
	} 
	else if (dynamic_world->speaking_player_index != NONE)
	{
		// find the name and color of the person who is speaking
		player_data *player = get_player_data(dynamic_world->speaking_player_index);
		status = player->name;
		_get_interface_color(PLAYER_COLOR_BASE_INDEX + player->color, &iconColor);
		icon = ">!<";
	}
	else
	{
		return;
	}

	FontSpecifier& Font = GetOnScreenFont();

	DisplayTextDest = s;
	DisplayTextFont = Font.Info;
	DisplayTextStyle = Font.Style;

	auto text_margins = alephone::Screen::instance()->lua_text_margins;
	short Y = s->h - text_margins.bottom - Font.LineSpacing / 3;
	if (Console::instance()->input_active())
	{
		Y -= Font.LineSpacing;
	}
	short Xicon = s->w - text_margins.right - DisplayTextWidth(icon.c_str()) - Font.LineSpacing / 3;
	short Xstatus = Xicon - DisplayTextWidth(" ") - DisplayTextWidth(status.c_str());

	DisplayText(Xicon, Y, icon.c_str(), iconColor.r, iconColor.g, iconColor.b);
	DisplayText(Xstatus, Y, status.c_str());
}

static const SDL_Color Green = { 0x0, 0xff, 0x0, 0xff };
static const SDL_Color Yellow = { 0xff, 0xff, 0x0, 0xff };
static const SDL_Color Red = { 0xff, 0x0, 0x0, 0xff };
static const SDL_Color Gray = { 0x7f, 0x7f, 0x7f, 0xff };

static void DisplayScores(SDL_Surface *s)
{
	if (!game_is_networked || !ShowScores) return;

	// assume a proportional font
	int CWidth = DisplayTextWidth("W");

	// field widths
	static const int kNameWidth = 20;
	int WName = CWidth * kNameWidth;
	static const int kScoreWidth = 5;
	int WScore = CWidth * kScoreWidth;
	static const int kPingWidth = 7;
	int WPing = CWidth * kPingWidth;
	int WJitter = CWidth * kPingWidth;
	int WErrors = CWidth * kPingWidth;
	static const int kIdWidth = 2;
	int WId = CWidth * kIdWidth;

	FontSpecifier& Font = GetOnScreenFont();

	DisplayTextDest = s;
	DisplayTextFont = Font.Info;
	DisplayTextStyle = Font.Style;

	int H = Font.LineSpacing * (dynamic_world->player_count + 1);
	int W = WName + WScore + WPing + WJitter + WErrors + WId;

	auto text_margins = alephone::Screen::instance()->lua_text_margins;
	int X = text_margins.left + (s->w - text_margins.right - W) / 2;
	int Y = std::max(text_margins.top + (s->h - text_margins.bottom - H) / 2, Font.LineSpacing * NumScreenMessages) + Font.LineSpacing;

	int XName = X;
	int XScore = XName + WName + CWidth;
	int XPing = XScore + WScore + CWidth;
	int XJitter = XPing + WPing + CWidth;
	int XErrors = XJitter + WPing + CWidth;
	int XId = XErrors + WPing + CWidth;

	// draw headers
	DisplayText(XName, Y, "Name", 0xbf, 0xbf, 0xbf);
	DisplayText(XScore + WScore - DisplayTextWidth("Score"), Y, "Score", 0xbf, 0xbf, 0xbf);
	DisplayText(XPing + WPing - DisplayTextWidth("Delay"), Y, "Delay", 0xbf, 0xbf, 0xbf);
	DisplayText(XJitter + WPing - DisplayTextWidth("Jitter"), Y, "Jitter", 0xbf, 0xbf, 0xbf);
	DisplayText(XErrors + WPing - DisplayTextWidth("Errors"), Y, "Errors", 0xbf, 0xbf, 0xbf);
	DisplayText(XId + WId - DisplayTextWidth("ID"), Y, "ID", 0xbf, 0xbf, 0xbf);
	Y += Font.LineSpacing;
	player_ranking_data rankings[MAXIMUM_NUMBER_OF_PLAYERS];
	calculate_player_rankings(rankings);
	for (int i = 0; i < dynamic_world->player_count; ++i)
	{
		player_data *player = get_player_data(rankings[i].player_index);

		SDL_Color color;
		_get_interface_color(PLAYER_COLOR_BASE_INDEX + player->color, &color);

		strncpy(temporary, player->name, 256);
		temporary[kNameWidth + 1] = '\0';
		DisplayText(XName, Y, temporary, color.r, color.g, color.b);

		calculate_ranking_text(temporary, rankings[i].ranking);
		temporary[kScoreWidth + 1] = '\0';
		DisplayText(XScore + WScore - DisplayTextWidth(temporary), Y, temporary, color.r, color.g, color.b);

		const NetworkStats& stats = NetGetStats(rankings[i].player_index);

		if (stats.latency == NetworkStats::invalid)
		{
			strncpy(temporary, " ", 256);
		}
		else if (stats.latency == NetworkStats::disconnected)
		{
			strncpy(temporary, "DC", 256);
		}
		else
		{
			sprintf(temporary, "%i ms", stats.latency);
		}
		SDL_Color color2;
		if (stats.latency == NetworkStats::invalid || stats.latency == NetworkStats::disconnected) 
			color2 = Gray;
		else if (stats.latency < 150)
			color2 = Green;
		else if (stats.latency < 350)
			color2 = Yellow;
		else 
			color2 = Red;

		temporary[kPingWidth + 1] = '\0';
		DisplayText(XPing + WPing - DisplayTextWidth(temporary), Y, temporary, color2.r, color2.g, color2.b);
		
		if (stats.jitter == NetworkStats::invalid)
		{
			strncpy(temporary, " ", 256);
		}
		else if (stats.jitter == NetworkStats::disconnected)
		{
			strncpy(temporary, "DC", 256);
		}
		else
		{
			sprintf(temporary, "%i ms", stats.jitter);
		}
		if (stats.jitter == NetworkStats::invalid || stats.jitter == NetworkStats::disconnected)
		{
			color2 = Gray;
		}
		else if (stats.jitter < 75)
		{
			color2 = Green;
		}
		else if (stats.jitter < 150)
		{
			color2 = Yellow;
		}
		else
		{
			color2 = Red;
		}
		temporary[kPingWidth + 1] = '\0';
		DisplayText(XJitter + WPing - DisplayTextWidth(temporary), Y, temporary, color2.r, color2.g, color2.b);

		sprintf(temporary, "%i", stats.errors);
		temporary[kPingWidth + 1] = '\0';
		if (stats.errors > 0) 
			color2 = Yellow;
		else
			color2 = Green;
		DisplayText(XErrors + WPing - DisplayTextWidth(temporary), Y, temporary, color2.r, color2.g, color2.b);

		sprintf(temporary, "%i", rankings[i].player_index);
		DisplayText(XId + WId - DisplayTextWidth(temporary), Y, temporary, color.r, color.g, color.b);

		Y += Font.LineSpacing;
	}
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

// This is for requesting the drawing of the Terminal;
// this is done because its drawing is now done when the main display is drawn
void RequestDrawingTerm()
{
	Term_RenderRequest = true;
}

// LP addition: display message on the screen;
// this really puts the current message into a buffer
// Code cribbed from csstrings
void screen_printf(const char *format, ...)
{
	MostRecentMessage = (MostRecentMessage + 1) % NumScreenMessages;
	while (MostRecentMessage < 0)
		MostRecentMessage += NumScreenMessages;
	ScreenMessage& Message = Messages[MostRecentMessage];

	Message.ExpirationTime = machine_tick_count() + 7 * MACHINE_TICKS_PER_SECOND;

	va_list list;

	va_start(list,format);
	// ZZZ: [v]sprintf is evil, generally: hard to guarantee you don't overflow target buffer
	// using [v]snprintf instead
	vsnprintf(Message.Text,sizeof(Message.Text),format,list);
	va_end(list);
}
