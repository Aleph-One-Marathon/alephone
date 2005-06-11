/*
 *  NibsMetaserverClientUi.cpp - UI for metaserver client, Carbon NIBs specialization

	Copyright (C) 2004 and beyond by Woody Zenfell, III
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

 April 29, 2004 (Woody Zenfell):
	Created.

 May 22, 2005 (Woody Zenfell):
	Split out from metaserver_dialogs.cpp.
 */

#if !defined(DISABLE_NETWORKING)

#include "metaserver_dialogs.h"
#include "NibsUiHelpers.h"

#include <boost/static_assert.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>

enum {
	iPLAYERS_IN_ROOM = 100,
	iGAMES_IN_ROOM = 200,
	iCHAT_HISTORY = 300,
	iCHAT_ENTRY = 400
};

static void MetaserverClientUi_Handler(ParsedControl& control, void* data);
const double PollingInterval = 1.0/30.0;

static pascal void MetaserverClientUi_Poller(EventLoopTimerRef, void*)
{
	MetaserverClient::pumpAll();
}

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
		TXNSetData (textObject, kTXNTextData, s.c_str(), s.size(),
				kTXNEndOffset, kTXNEndOffset);
	}
	
private:
	TXNObject textObject;
	TXNFrameID frameID;

};

class NibsMetaserverClientUi : public MetaserverClientUi, public MetaserverClient::NotificationAdapter
{
public:
	NibsMetaserverClientUi()
	: m_metaserverClientNib(CFSTR("Metaserver Client"))
	, m_dialog(m_metaserverClientNib.nibReference(), CFSTR("Metaserver Client"))
	, m_playersInRoomWidget(GetCtrlFromWindow(m_dialog(), 0, iPLAYERS_IN_ROOM))
	, m_gamesInRoomWidget(GetCtrlFromWindow(m_dialog(), 0, iGAMES_IN_ROOM))
	, m_chatEntryWidget(GetCtrlFromWindow(m_dialog(), 0, iCHAT_ENTRY))
	, m_textboxWidget(m_dialog(), 23, 200, 609, 403)
	, m_used(false)
	{
		m_gamesInRoomWidget.SetItemSelectedCallback(bind(&NibsMetaserverClientUi::GameSelected, this, _1, _2));
	}
	
	~NibsMetaserverClientUi() {}

	const IPaddress GetJoinAddressByRunning()
	{
		// This was designed with one-shot-ness in mind
		assert(!m_used);
		m_used = true;

		obj_clear(m_joinAddress);

		setupAndConnectClient(m_metaserverClient);
		m_metaserverClient.associateNotificationAdapter(this);

		AutoTimer Poller(0, PollingInterval, MetaserverClientUi_Poller, m_dialog());

		AutoKeyboardWatcher ChatEntry_Watcher(respondToChatEvent);
		ChatEntry_Watcher.Watch(m_chatEntryWidget, this);

		RunModalDialog(m_dialog(), false, MetaserverClientUi_Handler, this);

		return m_joinAddress;
	}

	void GameSelected(GameListMessage::GameListEntry game, ListWidget<GameListMessage::GameListEntry> sender)
	{
		memcpy(&m_joinAddress.host, &game.m_ipAddress, sizeof(m_joinAddress.host));
		m_joinAddress.port = game.m_port;
		StopModalDialog(m_dialog(), false);
	}

	void playersInRoomChanged()
	{
		m_playersInRoomWidget.SetItems(m_metaserverClient.playersInRoom());
	}

	void gamesInRoomChanged()
	{
		m_gamesInRoomWidget.SetItems(m_metaserverClient.gamesInRoom());
	}

	void receivedChatMessage(const std::string& senderName, uint32 senderID, const std::string& message)
	{
		m_textboxWidget.AppendString (senderName + ": " + message + "\r");
	}

	void receivedBroadcastMessage(const std::string& message)
	{
		receivedChatMessage("Metaserver", 0, message);
	}

	void sendChat()
	{
		string message = QQ_copy_string_from_text_control(m_dialog(), iCHAT_ENTRY);
		m_metaserverClient.sendChatMessage(message);
		QQ_copy_string_to_text_control(m_dialog(), iCHAT_ENTRY, string());
	}
	
	static pascal OSStatus respondToChatEvent (
			EventHandlerCallRef HandlerCallRef,
			EventRef Event,
			void *UserData)
	{
		// If user pressed return, we send
		if (GetEventKind(Event) == kEventRawKeyDown) {
			char character;
			GetEventParameter(Event, kEventParamKeyMacCharCodes, typeChar,
						NULL, sizeof(char), NULL, &character);
			if (character == '\r') {
				reinterpret_cast<NibsMetaserverClientUi*>(UserData)->sendChat();
				return noErr; 	// Don't allow next handler to be called
			}
		}
		
		// Hand off to the next event handler
		return CallNextEventHandler(HandlerCallRef, Event);
	}

private:
	AutoNibReference				m_metaserverClientNib;
	AutoNibWindow					m_dialog;
	MetaserverClient				m_metaserverClient;
	ListWidget<MetaserverPlayerInfo>		m_playersInRoomWidget;
	ListWidget<GameListMessage::GameListEntry>	m_gamesInRoomWidget;
	ControlRef					m_chatEntryWidget;
	TextboxWidget					m_textboxWidget;
	IPaddress					m_joinAddress;
	bool						m_used;
};

auto_ptr<MetaserverClientUi>
MetaserverClientUi::Create()
{
	return auto_ptr<MetaserverClientUi>(new NibsMetaserverClientUi);
}

// Does nothing, since we have another system for chat sending now
static void MetaserverClientUi_Handler(ParsedControl& control, void* data)
{
	NibsMetaserverClientUi* ui = static_cast<NibsMetaserverClientUi*>(data);
	switch(control.ID.id)
	{
		default:
			break;
	}
}

#endif // !defined(DISABLE_NETWORKING)
