/*
 *  SdlMetaserverClientUi.cpp - UI for metaserver client, SDL UI specialization

	Copyright (C) 2004 and beyond by Woody Zenfell, III
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

 April 29, 2004 (Woody Zenfell):
	Created.

 May 22, 2005 (Woody Zenfell):
	Split out from metaserver_dialogs.cpp.
 */

#if !defined(DISABLE_NETWORKING)

#include "cseries.h"

#include "sdl_dialogs.h"
#include "sdl_fonts.h"
#include "sdl_widgets.h"
#include "screen_drawing.h"
#include "network_dialog_widgets_sdl.h" // chat_history widget
#include "network_metaserver.h"
#include "interface.h" // set_drawing_clip_rectangle()
#include "metaserver_dialogs.h"

#include <algorithm>
#include <boost/function.hpp>
#include <boost/bind.hpp>

#include "TextStrings.h"

#include <sstream>

using namespace std;
using boost::bind;
using boost::ref;

extern MetaserverClient* gMetaserverClient;

class SdlMetaserverClientUi : public MetaserverClientUi
{
public:
	SdlMetaserverClientUi() : m_disconnected(false)
	{
		const int kSpace = 4;
		vertical_placer *placer = new vertical_placer(kSpace);
		placer->dual_add(new w_title("LOCATE NETWORK GAMES"), d);

		placer->add(new w_spacer(), true);

		table_placer *players_games_placer = new table_placer(2, get_theme_space(SPACER_WIDGET));
		w_players_in_room* players_in_room_w = new w_players_in_room(NULL, 216, get_theme_space(METASERVER_PLAYERS));
		
		players_games_placer->col_flags(1, placeable::kFill);
		players_games_placer->dual_add(players_in_room_w, d);

		games_in_room_w = new w_games_in_room(
			bind(&SdlMetaserverClientUi::GameSelected, this, _1),
			320,
			get_theme_space(METASERVER_GAMES, w_games_in_room::GAME_ENTRIES)
		);

		players_games_placer->dual_add(games_in_room_w, d);
		
		players_games_placer->add_row(new w_spacer(kSpace), true);

		horizontal_placer *player_button_placer = new horizontal_placer;
		w_tiny_button *mute_w = new w_tiny_button("IGNORE");
		mute_w->set_enabled(false);
		player_button_placer->dual_add(mute_w, d);
		players_games_placer->add(player_button_placer, true);

		horizontal_placer *game_button_placer = new horizontal_placer;
		game_button_placer->add_flags(placeable::kFill);
		game_button_placer->add(new w_spacer, true);
		game_button_placer->add_flags(placeable::kDefault);
		w_tiny_button *w_game_info = new w_tiny_button("INFO");
		w_game_info->set_enabled(false);
		game_button_placer->dual_add(w_game_info, d);

		game_button_placer->add(new w_spacer(kSpace), true);

		w_tiny_button *w_join_game = new w_tiny_button("JOIN");
		w_join_game->set_enabled(false);
		game_button_placer->dual_add(w_join_game, d);

		players_games_placer->add(game_button_placer, true);

		placer->add_flags(placeable::kFill);
		placer->add(players_games_placer, true);
		placer->add_flags();

		w_colorful_chat* chat_history_w = new w_colorful_chat(600, 10);
		placer->dual_add(chat_history_w, d);

		horizontal_placer *entry_cancel_placer = new horizontal_placer(get_theme_space(ITEM_WIDGET));

		w_chat_entry* chatentry_w = new w_chat_entry(240);
		chatentry_w->enable_mac_roman_input();

		entry_cancel_placer->dual_add(chatentry_w->label("Say:"), d);

		entry_cancel_placer->add_flags(placeable::kFill);
		entry_cancel_placer->dual_add(chatentry_w, d);
		entry_cancel_placer->add_flags();
		
		w_tiny_button* cancel_w = new w_tiny_button("CANCEL", NULL, &d);
		entry_cancel_placer->dual_add(cancel_w, d);

		placer->add_flags(placeable::kFill);
		placer->add(entry_cancel_placer, true);
		placer->add_flags();
		
		d.set_widget_placer(placer);
		
		m_playersInRoomWidget = new PlayerListWidget (players_in_room_w);
		m_gamesInRoomWidget = new GameListWidget (games_in_room_w);
		m_chatEntryWidget = new EditTextWidget (chatentry_w);
		m_chatWidget = new ColorfulChatWidget(new ColorfulChatWidgetImpl(chat_history_w));

		m_cancelWidget = new ButtonWidget (cancel_w);
		m_muteWidget = new ButtonWidget(mute_w);
		m_joinWidget = new ButtonWidget(w_join_game);
		m_gameInfoWidget = new ButtonWidget(w_game_info);

		d.activate_widget(chatentry_w);
	}

	~SdlMetaserverClientUi () { delete_widgets (); }

	int Run()
	{
		d.set_processing_function(bind(&SdlMetaserverClientUi::pump, this, _1));
		int result = d.run();

		if(result == -1)
			obj_clear(m_joinAddress);

		return result;
	}
	
	void Stop()
	{
		dialog_ok(&d);
	}

	void InfoClicked()
	{
		const GameListMessage::GameListEntry *game = gMetaserverClient->find_game(gMetaserverClient->game_target());
		if (game)
		{
			GameListMessage::GameListEntry cachedGame = *game;
			game = &cachedGame;

			dialog info_dialog;
			vertical_placer *placer = new vertical_placer;
			placer->dual_add(new w_title("GAME INFO"), info_dialog);
			placer->add(new w_spacer(), true);
			table_placer *table = new table_placer(2, get_theme_space(ITEM_WIDGET), true);
			table->col_flags(0, placeable::kAlignRight);
			table->col_flags(1, placeable::kAlignLeft);

			table->dual_add_row(new w_static_text("Gatherer"), info_dialog);
			table->dual_add(new w_label("Name"), info_dialog);
			const MetaserverPlayerInfo *player = gMetaserverClient->find_player(game->m_hostPlayerID);
			if (player)
			{
				table->dual_add(new w_styled_text(player->name().c_str()), info_dialog);
			}
			else
			{
				table->add(new w_spacer(), true);
			}
			table->dual_add(new w_label("Version"), info_dialog);
			table->dual_add(new w_static_text(game->m_description.m_alephoneBuildString.c_str()), info_dialog);

			table->add_row(new w_spacer(), true);
			table->dual_add_row(new w_static_text("Scenario"), info_dialog);
			table->dual_add(new w_label("Name"), info_dialog);
			table->dual_add(new w_static_text(game->m_description.m_scenarioName.c_str()), info_dialog);
			table->dual_add(new w_label("Version"), info_dialog);
			table->dual_add(new w_static_text(game->m_description.m_scenarioVersion.c_str()), info_dialog);


			table->add_row(new w_spacer(), true);
			table->dual_add_row(new w_static_text("Game"), info_dialog);
			table->dual_add(new w_label("Name"), info_dialog);
			table->dual_add(new w_styled_text(game->name().c_str()), info_dialog);
			table->dual_add(new w_label("Level"), info_dialog);
			table->dual_add(new w_static_text(game->m_description.m_mapName.c_str()), info_dialog);
			table->dual_add(new w_label("Pack"), info_dialog);
			table->dual_add(new w_static_text(game->m_description.m_mapFileName.c_str()), info_dialog);
			table->dual_add(new w_label("Difficulty"), info_dialog);
			if (TS_GetCString(kDifficultyLevelsStringSetID, game->m_description.m_difficulty))
			{
				table->dual_add(new w_static_text(TS_GetCString(kDifficultyLevelsStringSetID, game->m_description.m_difficulty)), info_dialog);
			}
			else
			{
				table->add(new w_spacer(), true);
			}
			table->add_row(new w_spacer(), true);
			table->dual_add(new w_label("Type"), info_dialog);
			int type = game->m_description.m_type - (game->m_description.m_type > 5 ? 1 : 0);
			if (TS_GetCString(kNetworkGameTypesStringSetID, type))
			{
				table->dual_add(new w_static_text(TS_GetCString(kNetworkGameTypesStringSetID, type)), info_dialog);
			}
			else
			{
				table->add(new w_spacer(), true);
			}
			table->dual_add(new w_label("Netscript"), info_dialog);
			table->dual_add(new w_static_text(game->m_description.m_netScript.c_str()), info_dialog);
			table->dual_add(new w_label("Physics"), info_dialog);
			table->dual_add(new w_static_text(game->m_description.m_physicsName.c_str()), info_dialog);
			table->add_row(new w_spacer(), true);
			table->dual_add_row(new w_static_text("Options"), info_dialog);
			if (game->m_description.m_hasGameOptions && (game->m_description.m_gameOptions & _game_has_kill_limit))
			{
				const char *s;
				switch (game->m_description.m_type)
				{
				case _game_of_capture_the_flag:
					s = TS_GetCString(strSETUP_NET_GAME_MESSAGES, flagPullsString);
					break;
				case _game_of_rugby:
				case _game_of_custom:
					s = TS_GetCString(strSETUP_NET_GAME_MESSAGES, pointLimitString);
					break;
				default:
					s = TS_GetCString(strSETUP_NET_GAME_MESSAGES, killLimitString);
				}

				table->dual_add(new w_label(s), info_dialog);
				ostringstream os;
				os << game->m_description.m_killLimit;
				table->dual_add(new w_label(os.str().c_str()), info_dialog);
			}
			else
			{
				table->dual_add(new w_label("Time Limit"), info_dialog);
				if (game->m_description.m_timeLimit && !(game->m_description.m_timeLimit == INT32_MAX || game->m_description.m_timeLimit == -1))
				{
					char minutes[32];
					snprintf(minutes, 32, "%i minutes", game->m_description.m_timeLimit / 60 / TICKS_PER_SECOND);
					minutes[31] = '\0';
					table->dual_add(new w_static_text(minutes), info_dialog);
				}
				else
				{
					table->dual_add(new w_static_text("No"), info_dialog);
					
				}
			}
			if (game->m_description.m_hasGameOptions)
			{
				table->dual_add(new w_label("Aliens"), info_dialog);
				w_toggle *aliens_w = new w_toggle(game->m_description.m_gameOptions & _monsters_replenish);
				aliens_w->set_enabled(false);
				table->dual_add(aliens_w, info_dialog);

				table->dual_add(new w_label("Teams"), info_dialog);
				w_toggle *teams_w = new w_toggle(game->m_description.m_teamsAllowed);
				teams_w->set_enabled(false);
				table->dual_add(teams_w, info_dialog);
				
				table->dual_add(new w_label("Dead Players Drop Items"), info_dialog);
				w_toggle *dpdi_w = new w_toggle(!(game->m_description.m_gameOptions & _burn_items_on_death));
				dpdi_w->set_enabled(false);
				table->dual_add(dpdi_w, info_dialog);

				table->dual_add(new w_label("Disable Motion Sensor"), info_dialog);
				w_toggle *motion_w = new w_toggle(game->m_description.m_gameOptions & _motion_sensor_does_not_work);
				motion_w->set_enabled(false);
				table->dual_add(motion_w, info_dialog);

				table->dual_add(new w_label("Death Penalty"), info_dialog);
				w_toggle *death_penalty_w = new w_toggle(game->m_description.m_gameOptions & _dying_is_penalized);
				death_penalty_w->set_enabled(false);
				table->dual_add(death_penalty_w, info_dialog);

				table->dual_add(new w_label("Suicide Penalty"), info_dialog);
				w_toggle *suicide_penalty_w = new w_toggle(game->m_description.m_gameOptions & _suicide_is_penalized);
				suicide_penalty_w->set_enabled(false);
				table->dual_add(suicide_penalty_w, info_dialog);
				
			}
			else
			{
				table->dual_add(new w_label("Teams"), info_dialog);
				w_toggle *teams_w = new w_toggle(game->m_description.m_teamsAllowed);
				teams_w->set_enabled(false);
				table->dual_add(teams_w, info_dialog);	
			}

			placer->add(table, true);
			placer->add(new w_spacer(), true);
			horizontal_placer *button_placer = new horizontal_placer;
			w_button *join_w = new w_button("JOIN", dialog_ok, &info_dialog);
			button_placer->dual_add(join_w, info_dialog);
			if (game->running() || !Scenario::instance()->IsCompatible(game->m_description.m_scenarioID))
			{
				join_w->set_enabled(false);
			}
			button_placer->dual_add(new w_button("CANCEL", dialog_cancel, &info_dialog), info_dialog);
			placer->add(button_placer, true);
			
			info_dialog.set_widget_placer(placer);
			info_dialog.set_processing_function(bind(&SdlMetaserverClientUi::pump, this, _1));
			if (info_dialog.run() == 0 && gMetaserverClient->find_game(gMetaserverClient->game_target()))
			{
				JoinGame(*game);
			}
			else
			{
				// deselect
				GameSelected(*game);
				
			}
		}
	}

private:

	w_games_in_room* games_in_room_w;

	void
	pump(dialog* d)
	{
		static uint32 last_update = 0;
		uint32 ticks = machine_tick_count();
		if (ticks > last_update + 5000)
		{
			last_update = ticks;
			games_in_room_w->refresh();
		}
		if (gMetaserverClient->isConnected())
			MetaserverClient::pumpAll();
		else if (!m_disconnected)
		{ 
			alert_user("Connection to room lost.", 0);
			m_disconnected = true;
			Stop();
		}
	}


private:
	dialog d;
	std::unique_ptr<dialog> m_info_dialog;
	bool m_disconnected;
};



std::unique_ptr<MetaserverClientUi>
MetaserverClientUi::Create()
{
	return std::unique_ptr<MetaserverClientUi>(new SdlMetaserverClientUi);
}

#endif // !defined(DISABLE_NETWORKING)
