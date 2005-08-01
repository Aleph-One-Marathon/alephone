
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

*/

/*
 *  shared_widgets.cpp - Widgets for carbon and sdl dialogs
 *
 */

#include "cseries.h"
#include "preferences.h"
#include "player.h"
#include "shared_widgets.h"
#include <vector>
#include <algorithm>


JoinAddressWidget::JoinAddressWidget (EditTextWidget* componentWidget)
	: EditCStringPrefWidget (componentWidget, network_preferences->join_address) {}

JoinByAddressWidget::JoinByAddressWidget (ToggleWidget* componentWidget)
	: TogglePrefWidget (componentWidget, network_preferences->join_by_address) {}

NameWidget::NameWidget (EditTextWidget* componentWidget)
	: EditPStringPrefWidget (componentWidget, player_preferences->name) {}

ColourWidget::ColourWidget (SelectorWidget* componentWidget)
	: SelectorPrefWidget (componentWidget, player_preferences->color) {}

TeamWidget::TeamWidget (SelectorWidget* componentWidget)
	: SelectorPrefWidget (componentWidget, player_preferences->team) {}

AutogatherWidget::AutogatherWidget (ToggleWidget* componentWidget)
	: TogglePrefWidget (componentWidget, network_preferences->autogather) {}
	
	
void ChatHistory::appendString (const string& s)
{
	m_history.push_back (s);
	if (m_notificationAdapter)
		m_notificationAdapter->contentAdded (s);
}

void ChatHistory::clear ()
{
	m_history.clear ();
	if (m_notificationAdapter)
		m_notificationAdapter->contentCleared ();
}

HistoricTextboxWidget::~HistoricTextboxWidget ()
{
	if (m_history) m_history->setObserver (NULL);
	delete m_componentWidget;
}

void HistoricTextboxWidget::attachHistory (ChatHistory* history)
{
	if (m_history)
		m_history->setObserver (NULL);

	m_history = history;

	m_componentWidget->Clear ();
	if (m_history) {
		const vector<string> &history_vector = m_history->getHistory ();
		for_each (history_vector.begin (), history_vector.end (),
				boost::bind(&TextboxWidget::AppendString, m_componentWidget, _1));
		m_history->setObserver (this);
	}
}

