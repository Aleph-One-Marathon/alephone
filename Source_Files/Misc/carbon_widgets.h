
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
 *  carbon_widgets.h - Widgets for carbon dialogs
 *
 */

#ifndef CARBON_WIDGETS_H
#define CARBON_WIDGETS_H

#include	"cseries.h"
#include	"NibsUiHelpers.h"
#include	"metaserver_messages.h" // for GameListMessage
#include	"network.h" // for prospective_joiner_info
#include	"tags.h" // for Typecode
#include	"binders.h"

#include <boost/static_assert.hpp>
#include <boost/bind.hpp>

#include "FileHandler.h"

#include <string>
#include <vector>
using std::string;
using std::vector;

class NIBsControlWidget
{
public:
	virtual ~NIBsControlWidget () {}

	virtual void hide () { HideControl (m_ctrl); }
	virtual void show () { ShowControl (m_ctrl); }

	virtual void activate () { ActivateControl (m_ctrl); }
	virtual void deactivate () { DeactivateControl (m_ctrl); }

protected:
	NIBsControlWidget (ControlRef ctrl) : m_ctrl (ctrl) {}
	
	ControlRef m_ctrl;
};

class ToggleWidget : public NIBsControlWidget, public Bindable<bool>
{
public:
	ToggleWidget (ControlRef ctrl)
		: NIBsControlWidget (ctrl)
		, m_control_watcher (m_ctrl)
		{}
	
	void set_callback (ControlHitCallback callback) { m_control_watcher.set_callback (callback); }
	
	bool get_value () { return GetControl32BitValue (m_ctrl) == 0 ? false : true; }
	void set_value (bool value) { SetControl32BitValue (m_ctrl, value ? 1 : 0); }

	virtual bool bind_export () { return get_value (); }
	virtual void bind_import (bool value) { set_value (value); }

private:
	AutoControlWatcher m_control_watcher;
};

class SelectorWidget : public NIBsControlWidget, public Bindable<int>
{
public:
	SelectorWidget (ControlRef ctrl)
		: NIBsControlWidget (ctrl)
		, m_control_watcher (m_ctrl)
		{}
	
	void set_callback (ControlHitCallback callback) { m_control_watcher.set_callback (callback); }
	
	void set_labels (int stringset) { set_labels (build_stringvector_from_stringset (stringset)); }
	void set_labels (const vector<std::string>& labels);
	
	int get_value () { return GetControl32BitValue (m_ctrl) - 1; }
	void set_value (int value) { SetControl32BitValue (m_ctrl, value + 1); }

	int bind_export () { return get_value (); }
	void bind_import (int value) { set_value (value); }

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
	
	void set_text (const std::string& s);
};

class EditTextOrNumberWidget : public NIBsControlWidget
{
public:
	EditTextOrNumberWidget (ControlRef ctrl, ControlRef label_ctrl = 0)
		: NIBsControlWidget (ctrl)
		, m_keystroke_watcher (m_ctrl)
		, m_label_widget ((label_ctrl) ? new StaticTextWidget (label_ctrl) : 0)
		{}

	~EditTextOrNumberWidget () { delete m_label_widget; }
	
	virtual void hide ();
	virtual void show ();
	
	void set_label (const std::string& s);
	
	void set_callback (GotCharacterCallback callback) { m_keystroke_watcher.set_callback (callback); }
	
	void set_text (const std::string& s);
	const string get_text ();

private:
	AutoKeystrokeWatcher m_keystroke_watcher;
	StaticTextWidget* m_label_widget;
};

class EditTextWidget : public EditTextOrNumberWidget, public Bindable<std::string>
{
public:
	EditTextWidget (ControlRef ctrl, ControlRef label_ctrl = 0)
		: EditTextOrNumberWidget (ctrl, label_ctrl)
		{}

	virtual std::string bind_export () { return get_text (); }
	virtual void bind_import (std::string s) { set_text (s); }
};

class EditNumberWidget : public EditTextOrNumberWidget, public Bindable<int>
{
public:
	EditNumberWidget (ControlRef ctrl, ControlRef label_ctrl = 0)
		: EditTextOrNumberWidget (ctrl, label_ctrl)
		{}
	
	void set_value (int value);
	int get_value ();
	
	virtual int bind_export () { return get_value (); }
	virtual void bind_import (int value) { set_value (value); }
};

class FileChooserWidget : public Bindable<FileSpecifier>
{
public:
	FileChooserWidget (ControlRef button_ctrl, ControlRef text_ctrl, Typecode type, const std::string& prompt)
		: m_button (new ButtonWidget (button_ctrl))
		, m_text (new StaticTextWidget (text_ctrl))
		, m_type (type)
		, m_prompt (prompt)
		, m_callback (0)
		{ m_button->set_callback (boost::bind(&FileChooserWidget::choose_file, this)); }
	
	~FileChooserWidget () { delete m_button; delete m_text; }
	
	void set_callback (ControlHitCallback callback) { m_callback = callback; }

	void set_file (const FileSpecifier& file);
	FileSpecifier get_file () { return m_file; }

	virtual FileSpecifier bind_export () { return get_file (); }
	virtual void bind_import (FileSpecifier f) { set_file (f); }

	void hide () { m_button->hide (); m_text->hide (); }
	void show () { m_button->show (); m_text->show (); }

	void activate () { m_button->activate (); m_text->activate (); }
	void deactivate () { m_button->deactivate (); m_text->deactivate (); }

private:
	void choose_file ();

	FileSpecifier m_file;
	ButtonWidget* m_button;
	StaticTextWidget* m_text;
	Typecode m_type;
	std::string m_prompt;
	ControlHitCallback m_callback;
};


const string ListWidgetValueForItem(const MetaserverPlayerInfo* element);
const string ListWidgetValueForItem(const prospective_joiner_info* element);
const string ListWidgetValueForItem(const GameListMessage::GameListEntry* element);

template <typename tElement>
class ListWidget
{
public:
	typedef typename boost::function<void (tElement item, ListWidget<tElement>& sender)> ItemSelectedCallback;

	ListWidget(ControlRef control, ButtonWidget* button = NULL)
		: m_control(control)
		, m_button(button)
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

	virtual ~ListWidget() { if (m_button) delete m_button; }

private:
	vector<tElement>	m_items;
	ControlRef		m_control;
	ItemSelectedCallback	m_itemSelected;
	ButtonWidget*		m_button;

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
		
		if (m_button)
			m_button->set_callback (boost::bind(&ListWidget<tElement>::ButtonHit, this));
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

	OSStatus SetDataForItemId(
		DataBrowserItemID itemId,
		DataBrowserPropertyID propertyId,
		DataBrowserItemDataRef itemData,
		Boolean setValue
	)
	{
		string value = ListWidgetValueForItem(ItemForItemId(itemId));
		std::auto_ptr<SelfReleasingCFStringRef> valueCfString = StringToCFString(value);
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
	
	void ButtonHit ()
	{
		// Find which items are selected
		Handle ItemsHdl = NewHandle(0);
		GetDataBrowserItems(m_control,
			NULL, false, kDataBrowserItemIsSelected,
			ItemsHdl);

		int NumItems = GetHandleSize(ItemsHdl)/sizeof(DataBrowserItemID);
	
		HLock(ItemsHdl);
		DataBrowserItemID* ItemsPtr = (DataBrowserItemID *)(*ItemsHdl);
	
		// Call our item selected callback for each selected item
		for (int k=0; k<NumItems; k++)
		{
			tElement* item = ItemForItemId(ItemsPtr[k]);
			if (item != NULL)
				m_itemSelected(*item, *this);
                }
	}
};

typedef ListWidget<MetaserverPlayerInfo> PlayerListWidget;
typedef ListWidget<prospective_joiner_info> JoiningPlayerListWidget;
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
		
		TXNBackground bgInfo;
		bgInfo.bgType = 1;
		bgInfo.bg.color.red = 0xffff;
		bgInfo.bg.color.blue = 0xffff;
		bgInfo.bg.color.green = 0xffff;
		TXNSetBackground(textObject, &bgInfo);

		TXNActivate (textObject, frameID, kScrollBarsAlwaysActive);
	}

	virtual TextboxWidget::~TextboxWidget ()
	{
		TXNDeleteObject (textObject);
	}

	void AppendString (const string& s)
	{
		TXNSetData (textObject, kTXNTextData, (s + '\r').c_str(), s.size() + 1,
				kTXNEndOffset, kTXNEndOffset);
		
		TXNDraw (textObject, NULL);
	}
	
	void Clear ()
	{
		TXNSetData (textObject, kTXNTextData, "", 0,
				kTXNStartOffset, kTXNEndOffset);
	}
	
private:
	TXNObject textObject;
	TXNFrameID frameID;

};

class PlayersInGameWidget : NIBsControlWidget
{
public:
	PlayersInGameWidget (ControlRef ctrl)
		: NIBsControlWidget (ctrl)
		{ m_pigDrawer (ctrl, pigDrawer, NULL); }

	void redraw () { Draw1Control (m_ctrl); }

private:
	AutoDrawability m_pigDrawer;
	
	static void pigDrawer (ControlRef Ctrl, void* ignored);
};

class ColourPickerWidget : public NIBsControlWidget, public Bindable<RGBColor>
{
public:
	ColourPickerWidget (ControlRef ctrl);

	RGBColor get_value () { return m_colour; }
	void set_value (RGBColor value) { m_colour = value; }

	virtual RGBColor bind_export () { return get_value (); }
	virtual void bind_import (RGBColor value) { set_value (value); }

private:
	AutoDrawability m_colourDrawer;
	AutoHittability m_colourHitter;
	AutoControlWatcher m_control_watcher;
	
	void chooseColour ();
	
	RGBColor m_colour;
};

#endif
