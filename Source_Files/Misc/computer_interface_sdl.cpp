/*
 *  computer_interface_sdl.cpp - Terminal handling, SDL specific stuff
 *
 *  Written in 2000 by Christian Bauer
 */

#include "cseries.h"
#include "computer_interface.h"


// Global variables
byte *map_terminal_data;
long map_terminal_data_length;


void initialize_terminal_manager(void)
{
printf("*** initialize_terminal_manager\n");
	//!!
}


void initialize_player_terminal_info(short player_index)
{
printf("*** initialize_player_terminal_info(%d)\n", player_index);
	//!!
}


void enter_computer_interface(short player_index, short text_number, short completion_flag)
{
printf("*** enter_computer_interface(), player %d, text %d\n", player_index, text_number);
	//!!
}


void _render_computer_interface(struct view_terminal_data *data)
{
printf("*** render_computer_interface()\n");
	//!!
}


void update_player_for_terminal_mode(short player_index)
{
printf("*** update_for_terminal_mode(%d)\n", player_index);
	//!!
}


void update_player_keys_for_terminal(short player_index, long action_flags)
{
printf("*** update_player_keys_for_terminal(%d), flags %08x\n", player_index, action_flags);
	//!!
}


long build_terminal_action_flags(char *keymap)
{
printf("*** build_terminal_action_flags()\n");
	//!!
	return 0;
}


void dirty_terminal_view(short player_index)
{
printf("*** dirty_terminal_view(%d)\n", player_index);
	//!!
}


void abort_terminal_mode(short player_index)
{
printf("*** abort_terminal_mode(%d)\n", player_index);
	//!!
}


boolean player_in_terminal_mode(short player_index)
{
//printf("*** player_in_terminal_mode(%d)\n", player_index);
	//!!
	return false;
}


void *get_terminal_data_for_save_game(void)
{
printf("*** get_terminal_data_for_save_game()\n");
	//!!
	return NULL;
}


long calculate_terminal_data_length(void)
{
printf("*** calculate_terminal_data_length()\n");
	//!!
	return 0;
}


void *get_terminal_information_array(void)
{
printf("*** get_terminal_information_array()\n");
	//!!
	return NULL;
}


long calculate_terminal_information_length(void)
{
printf("*** calculate_terminal_information_length()\n");
	//!!
	return 0;
}
