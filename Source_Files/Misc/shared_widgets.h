
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

template<typename tWidget>
class PrefWidget
{
public:
	PrefWidget (tWidget* componentWidget)
		: m_componentWidget (componentWidget)
		{ init_widget (); }
	
	virtual ~PrefWidget () { update_prefs (); delete m_componentWidget; }
	
	virtual void init_widget () {} //= 0;
	virtual void update_prefs () {} //= 0;
	
	void activate () { m_componentWidget->activate (); }
	void deactivate () { m_componentWidget->deactivate (); }

protected:
	tWidget* m_componentWidget;
};

class TogglePrefWidget : public PrefWidget<ToggleWidget>
{
public:
	TogglePrefWidget (ToggleWidget* componentWidget, bool& pref)
		: PrefWidget<ToggleWidget> (componentWidget)
		, m_pref (pref)
		{}
	
	virtual void init_widget () { m_componentWidget->set_value (m_pref); }
	virtual void update_prefs () { m_pref = m_componentWidget->get_value (); }

	bool get_value () { return m_componentWidget->get_value (); }
	void set_value (bool value) { m_componentWidget->set_value (value); }

protected:
	bool& m_pref;
};

class SelectorPrefWidget : public PrefWidget<SelectorWidget>
{
public:
	SelectorPrefWidget (SelectorWidget* componentWidget, int16& pref)
		: PrefWidget<SelectorWidget> (componentWidget)
		, m_pref (pref)
		{}
	
	virtual void init_widget () { m_componentWidget->set_value (m_pref); }
	virtual void update_prefs () { m_pref = m_componentWidget->get_value (); }

	int get_value () { return m_componentWidget->get_value (); }
	void set_value (int value) { m_componentWidget->set_value (value); }

protected:
	int16& m_pref;
};

class EditPStringPrefWidget : public PrefWidget<EditTextWidget>
{
public:
	EditPStringPrefWidget (EditTextWidget* componentWidget, unsigned char* pref)
		: PrefWidget<EditTextWidget> (componentWidget)
		, m_pref (pref)
		{}
	
	virtual void init_widget () { m_componentWidget->set_text (pstring_to_string (m_pref)); }
	virtual void update_prefs () { copy_string_to_pstring (get_text (), m_pref); }

	const string get_text () { return m_componentWidget->get_text (); }
	void set_text (const string& s) { m_componentWidget->set_text (s); }

protected:
	unsigned char* m_pref;
};

class EditCStringPrefWidget : public PrefWidget<EditTextWidget>
{
public:
	EditCStringPrefWidget (EditTextWidget* componentWidget, char* pref)
		: PrefWidget<EditTextWidget> (componentWidget)
		, m_pref (pref)
		{}
	
	virtual void init_widget () { m_componentWidget->set_text (string (m_pref)); }
	virtual void update_prefs () { copy_string_to_cstring (get_text (), m_pref); }

	const string get_text () { return m_componentWidget->get_text (); }
	void set_text (const string& s) { m_componentWidget->set_text (s); }

protected:
	char* m_pref;
};

class JoinAddressWidget : public EditCStringPrefWidget
{
public:
	JoinAddressWidget (EditTextWidget* joinAddressComponentWidget);
};

class JoinByAddressWidget : public TogglePrefWidget
{
public:
	JoinByAddressWidget (ToggleWidget* componentWidget);
};

class NameWidget : public EditPStringPrefWidget
{
public:
	NameWidget (EditTextWidget* componentWidget);
};

class ColourWidget : public SelectorPrefWidget
{
public:
	ColourWidget (SelectorWidget* componentWidget);
};

class TeamWidget : public SelectorPrefWidget
{
public:
	TeamWidget (SelectorWidget* componentWidget);
};

class AutogatherWidget : public TogglePrefWidget
{
public:
	AutogatherWidget (ToggleWidget* componentWidget);
};

#endif
