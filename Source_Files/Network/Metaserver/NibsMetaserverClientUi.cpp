/*
 *  NibsMetaserverClientUi.cpp - UI for metaserver client, Carbon NIBs specialization

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

#include "metaserver_dialogs.h"
#include "NibsUiHelpers.h"
#include "shared_widgets.h"

#include <boost/static_assert.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>

enum {
	iPLAYERS_IN_ROOM = 100,
	iGAMES_IN_ROOM = 200,
	iCHAT_HISTORY = 300,
	iCHAT_ENTRY = 400,
	iJOIN_METASERVER_GAME = 500
};

const double PollingInterval = 1.0/30.0;

static pascal void MetaserverClientUi_Poller(EventLoopTimerRef, void*)
{
	MetaserverClient::pumpAll();
}

class NibsMetaserverClientUi : public MetaserverClientUi
{
public:
	NibsMetaserverClientUi ()
	: m_metaserverClientNib(CFSTR("Metaserver Client"))
	, m_dialog_window(m_metaserverClientNib.nibReference(), CFSTR("Metaserver Client"))
	, m_dialog(m_dialog_window(), false)
	{
		m_playersInRoomWidget = new ListWidget<MetaserverPlayerInfo>(GetCtrlFromWindow(m_dialog_window(), 0, iPLAYERS_IN_ROOM));
		m_gamesInRoomWidget = new ListWidget<GameListMessage::GameListEntry>(GetCtrlFromWindow(m_dialog_window(), 0, iGAMES_IN_ROOM),
							new ButtonWidget (GetCtrlFromWindow(m_dialog_window(), 0, iJOIN_METASERVER_GAME)));
		m_chatEntryWidget = new EditTextWidget(GetCtrlFromWindow(m_dialog_window(), 0, iCHAT_ENTRY));
		m_textboxWidget = new HistoricTextboxWidget (new TextboxWidget(m_dialog_window(), 23, 200, 609, 403));
		m_cancelWidget = new ButtonWidget(GetCtrlFromWindow(m_dialog_window(), 0, iCANCEL));
	}
	
	~NibsMetaserverClientUi () { delete_widgets (); }
	
	virtual void Run()
	{
		AutoTimer Poller(0, PollingInterval, MetaserverClientUi_Poller, m_dialog_window());
		m_dialog.Run();
	}
	
	virtual void Stop()
	{
		m_dialog.Stop(false);
	}

private:
	AutoNibReference m_metaserverClientNib;
	AutoNibWindow m_dialog_window;
	Modal_Dialog m_dialog;
};

std::auto_ptr<MetaserverClientUi>
MetaserverClientUi::Create()
{
	return std::auto_ptr<MetaserverClientUi>(new NibsMetaserverClientUi);
}

#endif // !defined(DISABLE_NETWORKING)
