
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


template<typename tPref>
class Pref
{
public:
	virtual ~Pref () {}
	
	virtual tPref read_pref () = 0;
	virtual void write_pref (tPref value) = 0;
};

class PStringPref : public Pref<std::string>
{
public:
	PStringPref (unsigned char* pref) : m_pref (pref) {};

	virtual std::string read_pref () { return pstring_to_string (m_pref); }
	virtual void write_pref (std::string value) { copy_string_to_pstring (value, m_pref); }
	
protected:
	unsigned char* m_pref;
};

class CStringPref : public Pref<std::string>
{
public:
	CStringPref (char* pref) : m_pref (pref) {};

	virtual std::string read_pref () { return string (m_pref); }
	virtual void write_pref (std::string value) { copy_string_to_cstring (value, m_pref); }

protected:
	char* m_pref;
};

class BoolPref : public Pref<bool>
{
public:
	BoolPref (bool& pref) : m_pref(pref) {}

	virtual bool read_pref () { return m_pref; }
	virtual void write_pref (bool value) { m_pref = value; }

protected:
	bool& m_pref;
};

class Int16Pref : public Pref<int>
{
public:
	Int16Pref (int16& pref) : m_pref (pref) {}
	
	virtual int read_pref () { return m_pref; }
	virtual void write_pref (int value) { m_pref = value; }
	
protected:
	int16& m_pref;
};


template<typename tWidget>
class PrefWidget
{
public:
	PrefWidget (tWidget* componentWidget)
		: m_componentWidget (componentWidget)
		{}
	
	virtual ~PrefWidget () { delete m_componentWidget; }
	
	virtual void update_prefs () = 0;
	
	void activate () { m_componentWidget->activate (); }
	void deactivate () { m_componentWidget->deactivate (); }

protected:
	tWidget* m_componentWidget;
};

class TogglePrefWidget : public PrefWidget<ToggleWidget>
{
public:
	TogglePrefWidget (ToggleWidget* componentWidget)
		: PrefWidget<ToggleWidget> (componentWidget)
		, m_pref (0)
		{}
	
	virtual ~TogglePrefWidget () { update_prefs (); delete m_pref; }
	
	virtual void update_prefs () { m_pref->write_pref (m_componentWidget->get_value ()); }
	virtual void attach_pref (Pref<bool>* pref) { m_pref = pref; m_componentWidget->set_value (m_pref->read_pref()); }
	
	void set_callback (ControlHitCallback callback) { m_componentWidget->set_callback (callback); }

	bool get_value () { return m_componentWidget->get_value (); }
	void set_value (bool value) { m_componentWidget->set_value (value); }

protected:
	Pref<bool>* m_pref;
};

class SelectorPrefWidget : public PrefWidget<SelectorWidget>
{
public:
	SelectorPrefWidget (SelectorWidget* componentWidget)
		: PrefWidget<SelectorWidget> (componentWidget)
		, m_pref (0)
		{}
	
	virtual ~SelectorPrefWidget () { update_prefs (); delete m_pref; }
	
	virtual void update_prefs () { m_pref->write_pref (m_componentWidget->get_value ()); }
	virtual void attach_pref (Pref<int>* pref) { m_pref = pref; m_componentWidget->set_value (m_pref->read_pref()); }

	void set_callback (ControlHitCallback callback) { m_componentWidget->set_callback (callback); }

	void set_labels (int stringset) { m_componentWidget->set_labels (stringset); }
	void set_labels (const std::vector<std::string>& labels) { m_componentWidget->set_labels (labels); }

	int get_value () { return m_componentWidget->get_value (); }
	void set_value (int value) { m_componentWidget->set_value (value); }

protected:
	Pref<int>* m_pref;
};

class EditTextPrefWidget : public PrefWidget<EditTextWidget>
{
public:
	EditTextPrefWidget (EditTextWidget* componentWidget)
		: PrefWidget<EditTextWidget> (componentWidget)
		, m_pref (0)
		{}
	
	virtual ~EditTextPrefWidget () { update_prefs (); delete m_pref; }
	
	virtual void update_prefs () { m_pref->write_pref (m_componentWidget->get_text ()); }
	virtual void attach_pref (Pref<std::string>* pref) { m_pref = pref; m_componentWidget->set_text (m_pref->read_pref()); }

	const string get_text () { return m_componentWidget->get_text (); }
	void set_text (const string& s) { m_componentWidget->set_text (s); }

protected:
	Pref<std::string>* m_pref;
};


class ChatHistory
{
public:
	class NotificationAdapter {
	public:
		virtual void contentAdded (const string& s) = 0;
		virtual void contentCleared () = 0;
		virtual ~NotificationAdapter() {}
	};

	ChatHistory () : m_notificationAdapter (NULL) {}
	
	void appendString (const string& s);
	void clear ();
	const vector<string> getHistory () { return m_history; }
	
	void setObserver (NotificationAdapter* notificationAdapter)
		{ m_notificationAdapter = notificationAdapter; }

private:
	vector<string> m_history;
	NotificationAdapter* m_notificationAdapter;
};

class HistoricTextboxWidget : ChatHistory::NotificationAdapter
{
public:
	HistoricTextboxWidget (TextboxWidget* componentWidget)
		: m_componentWidget (componentWidget)
		, m_history (NULL)
		{}
	
	virtual ~HistoricTextboxWidget ();
	
	void attachHistory (ChatHistory* history);
	
	virtual void contentAdded (const string& s) { m_componentWidget->AppendString (s); }
	virtual void contentCleared () { m_componentWidget->Clear (); }
	
private:
	TextboxWidget* m_componentWidget;
	ChatHistory* m_history;
};

#endif
