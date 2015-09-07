
/*

	Copyright (C) 2005 and beyond by the "Aleph One" developers.
 
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
 *  shared_widgets.h - Widgets for carbon and sdl dialogs
 */

// This is where high level widgets with shared implementations on SDL and Carbon go.
// Also chooses which target-specific widget header to include.

#ifndef SHARED_WIDGETS_H
#define SHARED_WIDGETS_H

#include "cseries.h"

#include "sdl_widgets.h"

#include "binders.h"

class CStringPref : public Bindable<std::string>
{
public:
	CStringPref (char* pref, int length) : m_pref (pref), m_length (length) {};

	virtual std::string bind_export () { return string (m_pref); }
	virtual void bind_import (std::string s) { copy_string_to_cstring (s, m_pref, m_length); }

protected:
	char* m_pref;
	int m_length;
};

class BoolPref : public Bindable<bool>
{
public:
	BoolPref (bool& pref) : m_pref(pref) {}

	virtual bool bind_export () { return m_pref; }
	virtual void bind_import (bool value) { m_pref = value; }

protected:
	bool& m_pref;
};

class BitPref : public Bindable<bool>
{
public:
	BitPref (uint16& pref, uint16 mask, bool invert = false)
		: m_pref (pref), m_mask (mask), m_invert (invert) {}

	virtual bool bind_export () { return (m_invert ? !(m_pref & m_mask) : (m_pref & m_mask)); }
	virtual void bind_import (bool value) { (m_invert ? !value : value) ? m_pref |= m_mask : m_pref &= (m_mask ^ 0xFFFF); }

protected:
	uint16& m_pref;
	uint16 m_mask;
	bool m_invert;
};

class Int16Pref : public Bindable<int>
{
public:
	Int16Pref (int16& pref) : m_pref (pref) {}
	
	virtual int bind_export () { return m_pref; }
	virtual void bind_import (int value) { m_pref = value; }
	
protected:
	int16& m_pref;
};

class FilePref : public Bindable<FileSpecifier>
{
public:
	// The buffer should be at least 256
	FilePref (char* pref) : m_pref (pref) {}

	virtual FileSpecifier bind_export () { FileSpecifier f (m_pref); return f; }
	virtual void bind_import (FileSpecifier value) { strncpy (m_pref, value.GetPath (), 255); }
	
protected:
	char* m_pref;
};

class ChatHistory
{
public:
	class NotificationAdapter {
	public:
		virtual void contentAdded (const ColoredChatEntry& e) = 0;
		virtual void contentCleared () = 0;
		virtual ~NotificationAdapter() {}
	};

	ChatHistory () : m_notificationAdapter (NULL) {}
	
	void append(const ColoredChatEntry& e);
	void clear ();
	const vector<ColoredChatEntry> getHistory() { return m_history; }
	
	void setObserver (NotificationAdapter* notificationAdapter)
		{ m_notificationAdapter = notificationAdapter; }

private:
	vector<ColoredChatEntry> m_history;
	NotificationAdapter* m_notificationAdapter;
};

class ColorfulChatWidget : ChatHistory::NotificationAdapter
{
public:
	ColorfulChatWidget(ColorfulChatWidgetImpl* componentWidget)
		: m_componentWidget(componentWidget),
		  m_history(NULL)
		{}

	virtual ~ColorfulChatWidget();

	void attachHistory(ChatHistory* history);

	virtual void contentAdded(const ColoredChatEntry& e);
	virtual void contentCleared() { m_componentWidget->Clear(); }

private:
	ColorfulChatWidgetImpl* m_componentWidget;
	ChatHistory* m_history;
};

#endif
