
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
 *  carbon_widgets.h - Widgets for carbon dialogs
 *
 */

#ifndef CARBON_WIDGETS_H
#define CARBON_WIDGETS_H

#include	"cseries.h"
#include	"NibsUiHelpers.h"
#include	"metaserver_messages.h"

#include <boost/static_assert.hpp>
#include <boost/bind.hpp>

class NIBsControlWidget
{
public:
	void hide () { HideControl (m_ctrl); }
	void show () { ShowControl (m_ctrl); }

	void activate () { ActivateControl (m_ctrl); }
	void deactivate () { DeactivateControl (m_ctrl); }

protected:
	NIBsControlWidget (ControlRef ctrl) : m_ctrl (ctrl) {}
	
	ControlRef m_ctrl;
};

class ToggleWidget : public NIBsControlWidget
{
public:
	ToggleWidget (ControlRef ctrl)
		: NIBsControlWidget (ctrl)
		, m_control_watcher (m_ctrl)
		{}
	
	void set_callback (ControlHitCallback callback) { m_control_watcher.set_callback (callback); }
	
	bool get_value () { return GetControl32BitValue (m_ctrl) == 0 ? false : true; }
	void set_value (bool value) { SetControl32BitValue (m_ctrl, value ? 1 : 0); }

private:
	AutoControlWatcher m_control_watcher;
};

class SelectorWidget : public NIBsControlWidget
{
public:
	SelectorWidget (ControlRef ctrl)
		: NIBsControlWidget (ctrl)
		, m_control_watcher (m_ctrl)
		{}
	
	void set_callback (ControlHitCallback callback) { m_control_watcher.set_callback (callback); }
	
	int get_value () { return GetControl32BitValue (m_ctrl); }
	void set_value (int value) { SetControl32BitValue (m_ctrl, value); }

private:
	AutoControlWatcher m_control_watcher;
};

class ButtonWidget : public NIBsControlWidget
{
public:
	ButtonWidget (ControlRef ctrl)
		: NIBsControlWidget (ctrl)
		, m_control_watcher (m_ctrl)
		, m_callback (NULL)
		{}

	void set_callback (ControlHitCallback callback)
		{ m_control_watcher.set_callback (callback); m_callback = callback; }
	
	void push () { if (m_callback) m_callback (); }

private:
	AutoControlWatcher m_control_watcher;
	ControlHitCallback m_callback;
};

class StaticTextWidget : public NIBsControlWidget
{
public:
	StaticTextWidget (ControlRef ctrl)
		: NIBsControlWidget (ctrl) {}
	
	void set_text (std::string s);
};

class EditTextWidget : public NIBsControlWidget
{
public:
	EditTextWidget (ControlRef ctrl)
		: NIBsControlWidget (ctrl)
		, m_keystroke_watcher (m_ctrl) {}
	
	void set_callback (GotCharacterCallback callback) { m_keystroke_watcher.set_callback (callback); }
	
	void set_text (std::string s);
	const string get_text ();

private:
	AutoKeystrokeWatcher m_keystroke_watcher;
};

class FileChooserWidget
{
public:
	FileChooserWidget (ButtonWidget* button, StaticTextWidget* text, Typecode type)
		: m_button (button)
		, m_text (text)
		, m_type (type)
		{ m_button->set_callback (boost::bind(&FileChooserWidget::choose_file, this)); }

private:
	void choose_file ();

	ButtonWidget* m_button;
	StaticTextWidget* m_text;
	Typecode m_type;
};

template <typename tElement>
class ListWidget
{
public:
	typedef typename boost::function<void (tElement item, ListWidget<tElement>& sender)> ItemSelectedCallback;

	ListWidget(ControlRef control)
		: m_control(control)
	{
		SetupCallbacks();
	}

	void SetItems(const vector<tElement>& items)
	{
		m_items = items;
		ItemsChanged();
	}

	void SetItemSelectedCallback(const ItemSelectedCallback& itemSelected)
	{
		m_itemSelected = itemSelected;
	}

private:
	vector<tElement>	m_items;
	ControlRef		m_control;
	ItemSelectedCallback	m_itemSelected;

	void SetupCallbacks()
	{
		// To be 64-bit clean, will need to come up with a different way of associating 'this' with the control
		BOOST_STATIC_ASSERT((sizeof(ListWidget<tElement>*) == sizeof(SInt32)));
		SetControlReference(m_control, reinterpret_cast<SInt32>(this));

		DataBrowserCallbacks callbacks;
		obj_clear(callbacks);
		callbacks.version = kDataBrowserLatestCallbacks;
		callbacks.u.v1.itemDataCallback = NewDataBrowserItemDataUPP(BounceSetDataForItemId);
		callbacks.u.v1.itemNotificationCallback = NewDataBrowserItemNotificationUPP(BounceHandleItemNotification);
		SetDataBrowserCallbacks(m_control, &callbacks);
	}
	
	void ClearItemsFromControl()
	{
		const DataBrowserItemID container = kDataBrowserNoItem;
		const UInt32 numItems = 0;
		const DataBrowserItemID* items = NULL;
		const DataBrowserPropertyID presortedByProperty = kDataBrowserItemNoProperty;
		RemoveDataBrowserItems(m_control, container, numItems, items, presortedByProperty);
	}

	void InstallItemsInControl()
	{
		const DataBrowserItemID container = kDataBrowserNoItem;
		UInt32 numItems = m_items.size();
		vector<DataBrowserItemID> itemIds(numItems);
		for (size_t i = 0; i < itemIds.size(); i++)
			itemIds[i] = i + 1;
		DataBrowserItemID* items = &(itemIds[0]);
		const DataBrowserPropertyID presortedByProperty = kDataBrowserItemNoProperty;
		AddDataBrowserItems(m_control, container, numItems, items, presortedByProperty);
	}

	void ItemsChanged()
	{
		ClearItemsFromControl();
		InstallItemsInControl();
	}

	tElement* ItemForItemId(DataBrowserItemID id)
	{
		UInt32 itemIndex = id - 1;
		return (itemIndex < m_items.size()) ? &(m_items[itemIndex]) : NULL;
	}

	const string ValueForItem(const tElement* element)
	{
		return element == NULL ? string() : element->name();
	}

	OSStatus SetDataForItemId(
		DataBrowserItemID itemId,
		DataBrowserPropertyID propertyId,
		DataBrowserItemDataRef itemData,
		Boolean setValue
	)
	{
		string value = ValueForItem(ItemForItemId(itemId));
		auto_ptr<SelfReleasingCFStringRef> valueCfString = StringToCFString(value);
		SetDataBrowserItemDataText(itemData, valueCfString->StringRef());
		return noErr;
	}

	static pascal OSStatus BounceSetDataForItemId(
		ControlRef browser,
		DataBrowserItemID itemId,
		DataBrowserPropertyID propertyId,
		DataBrowserItemDataRef itemData,
		Boolean setValue
	)
	{
		ListWidget<tElement>* listWidget = reinterpret_cast<ListWidget<tElement>*>(GetControlReference(browser));
		return listWidget->SetDataForItemId(itemId, propertyId, itemData, setValue);
	}

	void HandleItemNotification(DataBrowserItemID itemId, DataBrowserItemNotification message)
	{
		if (message == kDataBrowserItemDoubleClicked && m_itemSelected)
		{
			tElement* item = ItemForItemId(itemId);
			if (item != NULL)
				m_itemSelected(*item, *this);
		}
	}

	static pascal void BounceHandleItemNotification(ControlRef browser, DataBrowserItemID item, DataBrowserItemNotification message)
	{
		ListWidget<tElement>* listWidget = reinterpret_cast<ListWidget<tElement>*>(GetControlReference(browser));
		listWidget->HandleItemNotification(item, message);
	}
};

typedef ListWidget<MetaserverPlayerInfo> PlayerListWidget;
typedef ListWidget<GameListMessage::GameListEntry> GameListWidget;

class TextboxWidget
{
public:
	TextboxWidget (WindowRef window, int left, int top, int right, int bottom)
	{
		Rect frame = {top, left, bottom, right};
		TXNNewObject (NULL, window, &frame,
			kTXNWantVScrollBarMask | kTXNReadOnlyMask,
			kTXNTextEditStyleFrameType,
			kTXNTextensionFile,
			kTXNSystemDefaultEncoding,
			&textObject,
			&frameID,
			0);

		TXNActivate (textObject, frameID, kScrollBarsAlwaysActive);
	}

	void AppendString (const string& s)
	{
		TXNSetData (textObject, kTXNTextData, (s + '\r').c_str(), s.size() + 1,
				kTXNEndOffset, kTXNEndOffset);
	}
	
private:
	TXNObject textObject;
	TXNFrameID frameID;

};

#endif
