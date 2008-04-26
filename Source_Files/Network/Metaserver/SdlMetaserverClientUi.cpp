/*
 *  SdlMetaserverClientUi.cpp - UI for metaserver client, SDL UI specialization

	Copyright (C) 2004 and beyond by Woody Zenfell, III
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

using namespace std;
using boost::bind;
using boost::ref;

extern MetaserverClient* gMetaserverClient;

class SdlMetaserverClientUi : public MetaserverClientUi
{
public:
	SdlMetaserverClientUi() : m_disconnected(false)
	{
		vertical_placer *placer = new vertical_placer(4);
		placer->dual_add(new w_static_text("LOCATE NETWORK GAMES", TITLE_FONT, TITLE_COLOR), d);

		placer->add(new w_spacer(), true);

		table_placer *players_games_placer = new table_placer(2, get_dialog_space(SPACER_HEIGHT));
		w_players_in_room* players_in_room_w = new w_players_in_room(NULL, 216, 8);
		
		players_games_placer->col_flags(1, placeable::kFill);
		players_games_placer->dual_add(players_in_room_w, d);

		games_in_room_w = new w_games_in_room(
			bind(&SdlMetaserverClientUi::GameSelected, this, _1),
			320,
			3
		);

		players_games_placer->dual_add(games_in_room_w, d);
		horizontal_placer *player_button_placer = new horizontal_placer;
		w_tiny_button *mute_w = new w_tiny_button("IGNORE");
		mute_w->set_enabled(false);
		player_button_placer->dual_add(mute_w, d);
		players_games_placer->add(player_button_placer, true);

		horizontal_placer *game_button_placer = new horizontal_placer;
		w_tiny_button *w_game_info = new w_tiny_button("INFO");
		w_game_info->set_enabled(false);
		game_button_placer->dual_add(w_game_info, d);

		w_tiny_button *w_join_game = new w_tiny_button("JOIN");
		w_join_game->set_enabled(false);
		game_button_placer->dual_add(w_join_game, d);

		players_games_placer->add(game_button_placer, true);

		placer->add_flags(placeable::kFill);
		placer->add(players_games_placer, true);
		placer->add_flags();

		w_colorful_chat* chat_history_w = new w_colorful_chat(600, 10);
		placer->dual_add(chat_history_w, d);

		horizontal_placer *entry_cancel_placer = new horizontal_placer(get_dialog_space(LABEL_ITEM_SPACE));

		w_text_entry* chatentry_w = new w_text_entry(240, "");
		chatentry_w->set_with_textbox();
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
			dialog info_dialog;
			vertical_placer *placer = new vertical_placer;
			placer->dual_add(new w_static_text("GAME INFO", TITLE_FONT, TITLE_COLOR), info_dialog);
			placer->add(new w_spacer(), true);
			table_placer *table = new table_placer(2, get_dialog_space(LABEL_ITEM_SPACE), true);
			table->col_flags(0, placeable::kAlignRight);
			table->col_flags(1, placeable::kAlignLeft);

			table->dual_add_row(new w_static_text("Gatherer"), info_dialog);
			table->dual_add(new w_label("Name"), info_dialog);
			const MetaserverPlayerInfo *player = gMetaserverClient->find_player(game->m_hostPlayerID);
			if (player)
			{
				table->dual_add(new w_static_text(player->name().c_str()), info_dialog);
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
			table->dual_add(new w_static_text(game->name().c_str()), info_dialog);
			table->dual_add(new w_label("Level"), info_dialog);
			table->dual_add(new w_static_text(game->m_description.m_mapName.c_str()), info_dialog);
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
			table->add_row(new w_spacer(), true);
			table->dual_add(new w_label("Teams"), info_dialog);
			table->dual_add(new w_static_text(game->m_description.m_teamsAllowed ? "Yes" : "No"), info_dialog);
			table->dual_add(new w_label("Time Limit"), info_dialog);
			if (game->m_description.m_timeLimit && !(game->m_description.m_timeLimit == INT32_MAX || game->m_description.m_timeLimit == -1))
			{
				char minutes[32];
				snprintf(minutes, 32, "%i minutes", game->m_description.m_timeLimit / 60 / TICKS_PER_SECOND);
				minutes[32] = '\0';
				table->dual_add(new w_static_text(minutes), info_dialog);
			}
			else
			{
				table->dual_add(new w_static_text("No"), info_dialog);
				
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
			if (info_dialog.run() == 0)
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
		uint32 ticks = SDL_GetTicks();
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
	std::auto_ptr<dialog> m_info_dialog;
	bool m_disconnected;
};



auto_ptr<MetaserverClientUi>
MetaserverClientUi::Create()
{
	return auto_ptr<MetaserverClientUi>(new SdlMetaserverClientUi);
}

#endif // !defined(DISABLE_NETWORKING)
