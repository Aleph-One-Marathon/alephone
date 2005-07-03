
/*

	Copyright (C) 2005 and beyond by the "Aleph One" developers.
 
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
 *  shared_widgets.h - Widgets for carbon and sdl dialogs
 */

// This is where high level widgets with shared implementations on SDL and Carbon go.
// Also chooses which target-specific widget header to include.

#ifndef SHARED_WIDGETS_H
#define SHARED_WIDGETS_H

#include "cseries.h"

#ifdef SDL
#include "sdl_widgets.h"
#else
#include "carbon_widgets.h"
#endif

class JoinAddressWidget
{
public:
	JoinAddressWidget (EditTextWidget* joinAddressComponentWidget);		
	~JoinAddressWidget ();
	
	std::string get_address () { return m_joinAddressComponentWidget->get_text (); }
	void set_address (std::string s) { m_joinAddressComponentWidget->set_text (s); }
	
	void activate () { m_joinAddressComponentWidget->activate (); }
	void deactivate () { m_joinAddressComponentWidget->deactivate (); }

private:
	EditTextWidget* m_joinAddressComponentWidget;
};

class JoinByAddressWidget
{
public:
	JoinByAddressWidget (ToggleWidget* joinByAddressComponentWidget);		
	~JoinByAddressWidget ();
	
	bool get_state () { return m_joinByAddressComponentWidget->get_value (); }
	void set_state (bool state) { m_joinByAddressComponentWidget->set_value (state); }
	
	void activate () { m_joinByAddressComponentWidget->activate (); }
	void deactivate () { m_joinByAddressComponentWidget->deactivate (); }

private:
	ToggleWidget* m_joinByAddressComponentWidget;
};

class NameWidget
{
public:
	NameWidget (EditTextWidget* nameComponentWidget);		
	~NameWidget ();
	
	std::string get_name () { return m_nameComponentWidget->get_text (); }
	void set_name (std::string s) { m_nameComponentWidget->set_text (s); }
	
	void activate () { m_nameComponentWidget->activate (); }
	void deactivate () { m_nameComponentWidget->deactivate (); }

private:
	EditTextWidget* m_nameComponentWidget;
};

class ColourWidget
{
public:
	ColourWidget (SelectorWidget* colourComponentWidget);		
	~ColourWidget ();
	
	int get_colour () { return m_colourComponentWidget->get_value (); }
	void set_colour (int colour) { m_colourComponentWidget->set_value (colour); }
	
	void activate () { m_colourComponentWidget->activate (); }
	void deactivate () { m_colourComponentWidget->deactivate (); }

private:
	SelectorWidget* m_colourComponentWidget;
};

class TeamWidget
{
public:
	TeamWidget (SelectorWidget* colourComponentWidget);		
	~TeamWidget ();
	
	int get_team () { return m_teamComponentWidget->get_value (); }
	void set_team (int colour) { m_teamComponentWidget->set_value (colour); }
	
	void activate () { m_teamComponentWidget->activate (); }
	void deactivate () { m_teamComponentWidget->deactivate (); }

private:
	SelectorWidget* m_teamComponentWidget;
};

#endif
