#ifndef __GAME_WINDOW_H
#define __GAME_WINDOW_H

/*

	game_window.h
	Tuesday, June 6, 1995 3:36:27 PM- rdm created.

Apr 30, 2000 (Loren Petrich): Added XML parser object for all the interface stuff.

*/

void initialize_game_window(void);

void draw_interface(void);
void update_interface(short time_elapsed);
void scroll_inventory(short dy);

void OGL_DrawHUD(Rect &dest, short time_elapsed);

void mark_ammo_display_as_dirty(void);
void mark_shield_display_as_dirty(void);
void mark_oxygen_display_as_dirty(void);
void mark_weapon_display_as_dirty(void);
void mark_player_inventory_screen_as_dirty(short player_index, short screen);
void mark_player_inventory_as_dirty(short player_index, short dirty_item);
void mark_interface_collections(bool loading);
void mark_player_network_stats_as_dirty(short player_index);

void set_interface_microphone_recording_state(bool state);

// LP addition: get the parser for the interface elements (name "interface")
class XML_ElementParser;
XML_ElementParser *Interface_GetParser();

#endif
