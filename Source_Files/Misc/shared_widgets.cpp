
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


JoinAddressWidget::JoinAddressWidget (EditTextWidget* joinAddressComponentWidget)
	: m_joinAddressComponentWidget (joinAddressComponentWidget)
{
	set_address (std::string (network_preferences->join_address));
}
		
JoinAddressWidget::~JoinAddressWidget ()
{
	copy_string_to_cstring (get_address (), network_preferences->join_address);
	delete m_joinAddressComponentWidget;
}


JoinByAddressWidget::JoinByAddressWidget (ToggleWidget* joinByAddressComponentWidget)
	: m_joinByAddressComponentWidget (joinByAddressComponentWidget)
{
	set_state (network_preferences->join_by_address);
}
		
JoinByAddressWidget::~JoinByAddressWidget ()
{
	network_preferences->join_by_address = get_state ();
	delete m_joinByAddressComponentWidget;
}


NameWidget::NameWidget (EditTextWidget* nameComponentWidget)
	: m_nameComponentWidget (nameComponentWidget)
{
	set_name (pstring_to_string (player_preferences->name));
}
		
NameWidget::~NameWidget ()
{
	copy_string_to_pstring (get_name (), player_preferences->name);
	delete m_nameComponentWidget;
}


ColourWidget::ColourWidget (SelectorWidget* colourComponentWidget)
	: m_colourComponentWidget (colourComponentWidget)
{
	m_colourComponentWidget->set_labels (kTeamColorsStringSetID);
	set_colour (player_preferences->color);
}
		
ColourWidget::~ColourWidget ()
{
	player_preferences->color = get_colour ();
	delete m_colourComponentWidget;
}

TeamWidget::TeamWidget (SelectorWidget* teamComponentWidget)
	: m_teamComponentWidget (teamComponentWidget)
{
	m_teamComponentWidget->set_labels (kTeamColorsStringSetID);
	set_team (player_preferences->team);
}
		
TeamWidget::~TeamWidget ()
{
	player_preferences->team = get_team ();
	delete m_teamComponentWidget;
}
