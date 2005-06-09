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



template <typename tElement>
class w_items_in_room : public w_list_base
{
public:
	typedef typename boost::function<void (const tElement& item, w_items_in_room<tElement>* sender)> ItemClickedCallback;
	typedef typename std::vector<tElement> ElementVector;
	typedef typename boost::function<const ElementVector ()> GetItemsCallback;

	w_items_in_room(GetItemsCallback getItems, ItemClickedCallback itemClicked, int width, int numRows) :
		w_list_base(width, numRows, 0),
		m_getItems(getItems),
		m_itemClicked(itemClicked)
	{
		assert(m_getItems);
		num_items = 0;
		new_items();
	}

	void collection_changed()
	{
		m_items = m_getItems();
		num_items = m_items.size();
		new_items();
		// do other crap - manage selection, force redraw, etc.
	}

	void item_selected()
	{
		if(m_itemClicked)
			m_itemClicked(m_items[selection], this);
	}

protected:
	void draw_items(SDL_Surface* s) const
	{
		typename ElementVector::const_iterator i = m_items.begin();
		int16 x = rect.x + get_dialog_space(LIST_L_SPACE);
		int16 y = rect.y + get_dialog_space(LIST_T_SPACE);
		uint16 width = rect.w - get_dialog_space(LIST_L_SPACE) - get_dialog_space(LIST_R_SPACE);

		for(size_t n = 0; n < top_item; n++)
			++i;

		for (size_t n=top_item; n<top_item + MIN(shown_items, num_items); n++, ++i, y=y+font_height)
			draw_item(*i, s, x, y, width, n == selection && active);
	}

private:
	GetItemsCallback		m_getItems;
	ElementVector			m_items;
	ItemClickedCallback		m_itemClicked;

	// This should be factored out into a "drawer" object/Strategy
	virtual void draw_item(const tElement& item, SDL_Surface* s,
			int16 x, int16 y, uint16 width, bool selected) const
	{
		y += font->get_ascent();
		set_drawing_clip_rectangle(0, x, static_cast<short>(s->h), x + width);
		draw_text(s, item.name().c_str(), x, y, selected ? get_dialog_color(ITEM_ACTIVE_COLOR) : get_dialog_color(ITEM_COLOR), font, style);
		set_drawing_clip_rectangle(SHRT_MIN, SHRT_MIN, SHRT_MAX, SHRT_MAX);
	}

	w_items_in_room(const w_items_in_room<tElement>&);
	w_items_in_room<tElement>& operator =(const w_items_in_room<tElement>&);
};



typedef w_items_in_room<GameListMessage::GameListEntry> w_games_in_room;



class w_players_in_room : public w_items_in_room<MetaserverPlayerInfo>
{
public:
	w_players_in_room(w_items_in_room<MetaserverPlayerInfo>::GetItemsCallback getItems, w_items_in_room<MetaserverPlayerInfo>::ItemClickedCallback itemClicked, int width, int numRows)
	: w_items_in_room<MetaserverPlayerInfo>(getItems, itemClicked, width, numRows)
	{}

private:
	static const int kTeamColorSwatchWidth = 8;
	static const int kPlayerColorSwatchWidth = 4;
	static const int kSwatchGutter = 2;

	void draw_item(const MetaserverPlayerInfo& item, SDL_Surface* s,
		int16 x, int16 y, uint16 width, bool selected) const
	{
		set_drawing_clip_rectangle(0, x, static_cast<short>(s->h), x + width);

		SDL_Rect r = {x, y + 1, kTeamColorSwatchWidth, font->get_ascent() - 2};
		uint32 pixel = SDL_MapRGB(s->format, 0xff, 0xff, 0x00);
		SDL_FillRect(s, &r, pixel);

		r.x += kTeamColorSwatchWidth;
		r.w = kPlayerColorSwatchWidth;
		pixel = SDL_MapRGB(s->format, 0xff, 0x00, 0x00);
		SDL_FillRect(s, &r, pixel);

		y += font->get_ascent();
		draw_text(s, item.name().c_str(), x + kTeamColorSwatchWidth + kPlayerColorSwatchWidth + kSwatchGutter, y, selected ? get_dialog_color(ITEM_ACTIVE_COLOR) : get_dialog_color(ITEM_COLOR), font, style);

		set_drawing_clip_rectangle(SHRT_MIN, SHRT_MIN, SHRT_MAX, SHRT_MAX);
	}
};



class SdlMetaserverClientUi : public MetaserverClientUi
{
public:
	SdlMetaserverClientUi() { obj_clear(mJoinAddress); }
	~SdlMetaserverClientUi() {}

	const IPaddress
	GetJoinAddressByRunning()
	{
		obj_clear(mJoinAddress);

		setupAndConnectClient(mMetaserverClient);

		dialog d;

		NotificationAdapter	na(d);
		mMetaserverClient.associateNotificationAdapter(&na);

		d.add(new w_static_text("LOCATE NETWORK GAMES", TITLE_FONT, TITLE_COLOR));

		d.add(new w_spacer());

		w_players_in_room* players_in_room_w = new w_players_in_room(bind(&MetaserverClient::playersInRoom, ref(mMetaserverClient)), NULL, 260, 8);
		players_in_room_w->set_identifier(iPLAYERS_IN_ROOM);
		players_in_room_w->set_alignment(widget::kAlignLeft);
		d.add(players_in_room_w);

		w_games_in_room* games_in_room_w = new w_games_in_room(
			bind(&MetaserverClient::gamesInRoom, ref(mMetaserverClient)),
			bind(&SdlMetaserverClientUi::gameClicked, this, _1, _2),
			320,
			8
		);
		games_in_room_w->set_identifier(iGAMES_IN_ROOM);
		games_in_room_w->set_alignment(widget::kAlignRight);
		games_in_room_w->align_bottom_with_bottom_of(players_in_room_w);
		d.add(games_in_room_w);

		d.add(new w_spacer());

		w_chat_history* chat_history_w = new w_chat_history(600, 12);
		chat_history_w->set_identifier(iCHAT_HISTORY);
		d.add(chat_history_w);

		w_text_entry*	chatentry_w = new w_text_entry("Say:", 240, "");
		chatentry_w->set_identifier(iCHAT_ENTRY);
		chatentry_w->set_enter_pressed_callback(bind(&SdlMetaserverClientUi::send_text, this, _1));
		chatentry_w->set_alignment(widget::kAlignLeft);
		chatentry_w->set_full_width();
		d.add(chatentry_w);

		d.add(new w_spacer());

		d.add(new w_button("CANCEL", dialog_cancel, &d));

		d.set_processing_function(bind(&SdlMetaserverClientUi::pump, this, _1));

		int result = d.run();

		if(result == -1)
			obj_clear(mJoinAddress);

		return mJoinAddress;
	}

private:
	enum
	{
		iPLAYERS_IN_ROOM = 5342,
		iGAMES_IN_ROOM,
		iCHAT_HISTORY,
		iCHAT_ENTRY,
	};

	void
	pump(dialog* d)
	{
		mMetaserverClient.pump();
	}

	void
	gameClicked(const GameListMessage::GameListEntry& entry, w_games_in_room* sender)
	{
		memcpy(&mJoinAddress.host, &entry.m_ipAddress, sizeof(mJoinAddress.host));
		mJoinAddress.port = entry.m_port;
		sender->get_owning_dialog()->quit(0);
	}	

	void
	send_text(w_text_entry* te) {
		assert(te != NULL);

		// Make sure there's something worth sending
		if(strlen(te->get_text()) <= 0)
			return;

		mMetaserverClient.sendChatMessage(std::string(te->get_text()));

		te->set_text("");
	}

	class NotificationAdapter : public MetaserverClient::NotificationAdapter
	{
	public:
		NotificationAdapter(dialog& d) : m_dialog(d) {}

		void playersInRoomChanged()
		{
			itemsInRoomChanged<w_players_in_room>(iPLAYERS_IN_ROOM);
		}

		void gamesInRoomChanged()
		{
			itemsInRoomChanged<w_games_in_room>(iGAMES_IN_ROOM);
		}

		void receivedChatMessage(const std::string& senderName, uint32 senderID, const std::string& message)
		{
			w_chat_history* ch = dynamic_cast<w_chat_history*>(m_dialog.get_widget_by_id(iCHAT_HISTORY));
			assert(ch != NULL);

			ch->append_chat_entry(senderName.c_str(), 0xcccccccc, 0xcccccccc, message.c_str());

			m_dialog.draw_dirty_widgets();
		}

		void receivedBroadcastMessage(const std::string& message)
		{
			w_chat_history* ch = dynamic_cast<w_chat_history*>(m_dialog.get_widget_by_id(iCHAT_HISTORY));
			assert(ch != NULL);

			ch->append_chat_entry(NULL, message.c_str());

			m_dialog.draw_dirty_widgets();
		}

	private:
		dialog&	m_dialog;

		template <typename tWidget>
		void itemsInRoomChanged(uint16 widgetID)
		{
			tWidget* w = dynamic_cast<tWidget*>(m_dialog.get_widget_by_id(widgetID));
			assert(w != NULL);

			w->collection_changed();

			m_dialog.draw_dirty_widgets();
		}

		NotificationAdapter(const NotificationAdapter&);
		NotificationAdapter& operator =(const NotificationAdapter&);
	};	

private:
	IPaddress		mJoinAddress;
	MetaserverClient	mMetaserverClient;
};



auto_ptr<MetaserverClientUi>
MetaserverClientUi::Create()
{
	return auto_ptr<MetaserverClientUi>(new SdlMetaserverClientUi);
}

#endif // !defined(DISABLE_NETWORKING)
