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

	This license is contained in the file "GNU_GeneralPublicLicense.txt",
	which is included with this source code; it is available online at
	http://www.gnu.org/licenses/gpl.html

*/
/*
 *  network_dialogs_sdl.cpp - Network game dialogs, SDL implementation
 *
 *  Written in 2000 by Christian Bauer
 */

#include "cseries.h"
#include "sdl_network.h"
#include "sdl_dialogs.h"
#include "sdl_fonts.h"
#include "sdl_widgets.h"

#include "shell.h"
#include "map.h"
#include "player.h"
#include "preferences.h"
#include "PlayerName.h"


// Get player name from outside
#define PLAYER_TYPE GetPlayerName()


/*
 *  Network game statistics dialog
 */

void display_net_game_stats(void)
{
printf("display_net_game_stats\n");
}


/*
 *  Game setup dialog
 */

static const char *level_labels[6] = {
	"Kindergarten", "Easy", "Normal", "Major Damage", "Total Carnage", NULL
};

bool network_game_setup(player_info *player_information, game_info *game_information)
{
printf("network_game_setup\n");
	// Create dialog
	dialog d;
	d.add(new w_static_text("SETUP NETWORK GAME", TITLE_FONT, TITLE_COLOR));
	d.add(new w_spacer());
	d.add(new w_static_text("Appearance"));
	w_text_entry *name_w = new w_text_entry("Name", PREFERENCES_NAME_LENGTH, player_preferences->name);
	d.add(name_w);
	w_player_color *pcolor_w = new w_player_color("Color", player_preferences->color);
	d.add(pcolor_w);
	w_player_color *tcolor_w = new w_player_color("Team Color", player_preferences->team);
	d.add(tcolor_w);
	d.add(new w_spacer());
	//!! map (network_preferences->entry_point)
	//!! game type (network_preferences->game_type)
	w_select *level_w = new w_select("Difficulty", network_preferences->difficulty_level, level_labels);
	d.add(level_w);
	d.add(new w_spacer());
	//!! duration (network_preferences->game_options)
	//!! time/kill limit (network_preferences->time/kill_limit)
	d.add(new w_spacer());
	w_toggle *aliens_w = new w_toggle("Aliens", network_preferences->game_options & _monsters_replenish);
	d.add(aliens_w);
	w_toggle *live_w = new w_toggle("Live Carnage Reporting", network_preferences->game_options & _live_network_stats);
	d.add(live_w);
	w_toggle *teams_w = new w_toggle("Teams", !(network_preferences->game_options & _force_unique_teams));
	d.add(teams_w);
	w_toggle *drop_w = new w_toggle("Dead Players Drop Items", !(network_preferences->game_options & _burn_items_on_death));
	d.add(drop_w);
	w_toggle *sensor_w = new w_toggle("Disable Motion Sensor", network_preferences->game_options & _motion_sensor_does_not_work);
	d.add(sensor_w);
	w_toggle *pen_die_w = new w_toggle("Penalize Dying (10 seconds)", network_preferences->game_options & _dying_is_penalized);
	d.add(pen_die_w);
	w_toggle *pen_sui_w = new w_toggle("Penalize Suicide (15 seconds)", network_preferences->game_options & _suicide_is_penalized);
	d.add(pen_sui_w);
	d.add(new w_spacer());
	d.add(new w_left_button("OK", dialog_ok, &d));
	d.add(new w_right_button("CANCEL", dialog_cancel, &d));

	// Run dialog
	if (d.run() == 0) { // Accepted

		const char *name = name_w->get_text();
		int name_length = strlen(name);
		player_information->name[0] = name_length;
		memcpy(player_information->name + 1, name, name_length);
		strcpy(player_preferences->name, name);
		player_preferences->color = player_information->color = pcolor_w->get_selection();
		player_preferences->team = player_information->team = tcolor_w->get_selection();

		game_information->server_is_playing = true;
		//!!network_preferences->net_game_type = game_information->net_game_type =
		//!!network_preferences->game_options = game_information->game_options =
		//!!network_preferences->time_limit = game_information->time_limit =
		//!!network_preferences->kill_limit = game_information->kill_limit =
		//!!network_preferences->entry_point = game_information->level_number =
		//!!game_information->level_name =
		network_preferences->difficulty_level = game_information->difficulty_level = level_w->get_selection();

		game_information->initial_updates_per_packet = 1;
		game_information->initial_update_latency = 0;
		NetSetInitialParameters(game_information->initial_updates_per_packet, game_information->initial_update_latency);
		network_preferences->allow_microphone = game_information->allow_mic = false;

		game_information->initial_random_seed = machine_tick_count();
		if (network_preferences->time_limit <= 0)
			network_preferences->time_limit = 10 * 60 * TICKS_PER_SECOND;
		network_preferences->game_is_untimed == (game_information->time_limit == INT32_MAX);

		write_preferences();

		if (game_information->game_options & _force_unique_teams)
			player_information->team = player_information->color;

		return true;
	} else
		return false;
}


/*
 *  Gathering dialog
 */

bool network_gather(void)
{
printf("network_gather\n");

	// Display game setup dialog
	game_info myGameInfo;
	player_info myPlayerInfo;
	if (network_game_setup(&myPlayerInfo, &myGameInfo)) {
		myPlayerInfo.desired_color = myPlayerInfo.color;
		memcpy(myPlayerInfo.long_serial_number, serial_preferences->long_serial_number, 10);

		if (NetEnter()) {
			if (NetGather(&myGameInfo, sizeof(game_info), (void *)&myPlayerInfo, sizeof(player_info))) {
				//!!
				if (1)
					return NetStart();
				else {
					NetCancelGather();
				}
			}
			NetExit();
		}
	}
	return false;
}


/*
 *  Joining dialog
 */

bool network_join(void)
{
printf("network_join\n");

	if (NetEnter()) {
		player_info myPlayerInfo;
		myPlayerInfo.name[0] = strlen(player_preferences->name);
		memcpy(myPlayerInfo.name + 1, player_preferences->name, MAX_NET_PLAYER_NAME_LENGTH);

		//!!
		if (1) {
			// set myPlayerInfo.name/team/color
			myPlayerInfo.desired_color = myPlayerInfo.color;
			memcpy(myPlayerInfo.long_serial_number, serial_preferences->long_serial_number, 10);
			bool did_join = NetGameJoin(myPlayerInfo.name, PLAYER_TYPE, (void *)&myPlayerInfo, sizeof(player_info), MARATHON_NETWORK_VERSION);
			if (did_join) {
				//!! player_preferences->name
				player_preferences->team = myPlayerInfo.team;
				player_preferences->color = myPlayerInfo.color;
				write_preferences();

				game_info *myGameInfo = (game_info *)NetGetGameData();
				NetSetInitialParameters(myGameInfo->initial_updates_per_packet, myGameInfo->initial_update_latency);

				return true;
			}
		}
		NetExit();
	}
	return false;
}


/*
 *  Progress dialog
 */

void open_progress_dialog(short message_id)
{
printf("open_progress_dialog %d\n", message_id);
}

void set_progress_dialog_message(short message_id)
{
printf("set_progress_dialog_message %d\n", message_id);
}

void close_progress_dialog(void)
{
printf("close_progress_dialog\n");
}

void draw_progress_bar(long sent, long total)
{
printf("draw_progress_bar %ld, %ld", sent, total);
}

void reset_progress_bar(void)
{
printf("reset_progress_bar\n");
}
