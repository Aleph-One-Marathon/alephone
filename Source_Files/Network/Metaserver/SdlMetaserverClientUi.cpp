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

using namespace std;
using boost::bind;
using boost::ref;


class SdlMetaserverClientUi : public MetaserverClientUi
{
public:
	SdlMetaserverClientUi()
	{
		d.add(new w_static_text("LOCATE NETWORK GAMES", TITLE_FONT, TITLE_COLOR));

		d.add(new w_spacer());

		w_players_in_room* players_in_room_w = new w_players_in_room(NULL, 260, 8);
		players_in_room_w->set_alignment(widget::kAlignLeft);
		d.add(players_in_room_w);

		w_games_in_room* games_in_room_w = new w_games_in_room(
			bind(&SdlMetaserverClientUi::GameSelected, this, _1),
			320,
			8
		);
		games_in_room_w->set_alignment(widget::kAlignRight);
		games_in_room_w->align_bottom_with_bottom_of(players_in_room_w);
		d.add(games_in_room_w);

		d.add(new w_spacer());

		w_text_box* chat_history_w = new w_text_box(600, 12);
		d.add(chat_history_w);

		w_text_entry* chatentry_w = new w_text_entry("Say:", 240, "");
		chatentry_w->set_with_textbox();
		chatentry_w->set_alignment(widget::kAlignLeft);
		chatentry_w->set_full_width();
		d.add(chatentry_w);

		d.add(new w_spacer());

		w_button* cancel_w = new w_button("CANCEL", NULL, &d);
		d.add(cancel_w);
		
		m_playersInRoomWidget = new PlayerListWidget (players_in_room_w);
		m_gamesInRoomWidget = new GameListWidget (games_in_room_w);
		m_chatEntryWidget = new EditTextWidget (chatentry_w);
		m_textboxWidget = new HistoricTextboxWidget (new TextboxWidget(chat_history_w));
		m_cancelWidget = new ButtonWidget (cancel_w);
	}
	
	~SdlMetaserverClientUi()
	{
		delete m_playersInRoomWidget;
		delete m_gamesInRoomWidget;
		delete m_chatEntryWidget;
		delete m_textboxWidget;
	}

	void Run()
	{
		d.set_processing_function(bind(&SdlMetaserverClientUi::pump, this, _1));

		int result = d.run();

		if(result == -1)
			obj_clear(m_joinAddress);
	}
	
	void Stop()
	{
		dialog_ok(&d);
	}

private:

	void
	pump(dialog* d)
	{
		MetaserverClient::pumpAll();
	}


private:
	dialog d;
};



auto_ptr<MetaserverClientUi>
MetaserverClientUi::Create()
{
	return auto_ptr<MetaserverClientUi>(new SdlMetaserverClientUi);
}

#endif // !defined(DISABLE_NETWORKING)
