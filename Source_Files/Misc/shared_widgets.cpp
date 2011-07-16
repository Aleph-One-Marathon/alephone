
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
	
	
void ChatHistory::append(const ColoredChatEntry& e)
{
	m_history.push_back(e);
	if (m_notificationAdapter)
		m_notificationAdapter->contentAdded(e);
}

void ChatHistory::clear ()
{
	m_history.clear ();
	if (m_notificationAdapter)
		m_notificationAdapter->contentCleared ();
}

ColorfulChatWidget::~ColorfulChatWidget()
{
	if (m_history) m_history->setObserver(0);
	delete m_componentWidget;
}

void ColorfulChatWidget::attachHistory(ChatHistory* history)
{
	if (m_history)
		m_history->setObserver(0);

	m_history = history;

	m_componentWidget->Clear();
	if (m_history) {
		const vector<ColoredChatEntry> &history_vector = m_history->getHistory();
		for(vector<ColoredChatEntry>::const_iterator it = history_vector.begin(); it != history_vector.end(); ++it)
		{
			m_componentWidget->Append(*it);
		}

		m_history->setObserver(this);
	}
}

void ColorfulChatWidget::contentAdded(const ColoredChatEntry& e)
{
	m_componentWidget->Append(e);
}


