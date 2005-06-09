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

class SelfReleasingCFStringRef
{
public:
	SelfReleasingCFStringRef(CFStringRef newlyCreatedStringRef)
		: m_stringRef(newlyCreatedStringRef)
	{}

	CFStringRef StringRef() const {
		return m_stringRef;
	}

	~SelfReleasingCFStringRef()
	{
		CFRelease(m_stringRef);
	}

private:
	CFStringRef m_stringRef;
	
	// No reason these couldn't be implemented, but they're not yet.
	SelfReleasingCFStringRef(const SelfReleasingCFStringRef&);
	SelfReleasingCFStringRef& operator =(const SelfReleasingCFStringRef&);
};

auto_ptr<SelfReleasingCFStringRef>
StringToCFString(const string& s)
{
	return auto_ptr<SelfReleasingCFStringRef>(
		new SelfReleasingCFStringRef(
			CFStringCreateWithCString(NULL, s.c_str(), NULL)
		)
	);
}

template <typename tElement>
class ListWidget
{
public:
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

private:
	vector<tElement>	m_items;
	ControlRef		m_control;

	void SetupCallbacks()
	{
		SetControlReference(m_control, this);

		DataBrowserCallbacks callbacks;
		obj_clear(callbacks);
		callbacks.version = kDataBrowserLatestCallbacks;
		callbacks.u.v1.itemDataCallback = NewDataBrowserItemDataUPP(BounceValueForItemId);
		//Callbacks.u.v1.itemNotificationCallback = NewDataBrowserItemNotificationUPP(PlayerListMemberHit);
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

	const string ValueForItemId(DataBrowserItemID id)
	{
		UInt32 itemIndex = id - 1;

		return (itemIndex < m_items.size()) ? m_items[itemIndex].name() : string();
	}

	static pascal OSStatus BounceValueForItemId(
		ControlRef browser,
		DataBrowserItemID itemId,
		DataBrowserPropertyID propertyId,
		DataBrowserItemDataRef itemData,
		Boolean setValue
	)
	{
		ListWidget<tElement>* listWidget = reinterpret_cast<ListWidget<tElement>*>(GetControlReference(browser));
		
		string value = listWidget->ValueForItemId(itemId);

		auto_ptr<SelfReleasingCFStringRef> valueCfString = StringToCFString(value);

		SetDataBrowserItemDataText(itemData, valueCfString->StringRef());

		return noErr;
	}
};

class NibsMetaserverClientUi : public MetaserverClientUi, public MetaserverClient::NotificationAdapter
{
public:
	NibsMetaserverClientUi()
	: m_metaserverClientNib(CFSTR("Metaserver Client"))
	, m_dialog(m_metaserverClientNib.nibReference(), CFSTR("Metaserver Client"))
	, m_playersInRoomWidget(GetCtrlFromWindow(m_dialog(), 0, iPLAYERS_IN_ROOM))
	, m_gamesInRoomWidget(GetCtrlFromWindow(m_dialog(), 0, iGAMES_IN_ROOM))
	{}
	
	~NibsMetaserverClientUi() {}

	const IPaddress GetJoinAddressByRunning()
	{
		IPaddress address;
		obj_clear(address);

		setupAndConnectClient(m_metaserverClient);
		m_metaserverClient.associateNotificationAdapter(this);

		AutoTimer Poller(0, PollingInterval, MetaserverClientUi_Poller, m_dialog());

		if (RunModalDialog(m_dialog(), false, MetaserverClientUi_Handler, this))
			address.host = (1 << 24) + (2 << 16) + (3 << 8) + (4 << 0);

		return address;
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
		string chatTranscript = QQ_copy_string_from_text_control(m_dialog(), iCHAT_HISTORY);
		chatTranscript += (senderName + ": " + message + "\r");
		QQ_copy_string_to_text_control(m_dialog(), iCHAT_HISTORY, chatTranscript);
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

private:
	AutoNibReference				m_metaserverClientNib;
	AutoNibWindow					m_dialog;
	MetaserverClient				m_metaserverClient;
	ListWidget<MetaserverPlayerInfo>		m_playersInRoomWidget;
	ListWidget<GameListMessage::GameListEntry>	m_gamesInRoomWidget;
};

auto_ptr<MetaserverClientUi>
MetaserverClientUi::Create()
{
	return auto_ptr<MetaserverClientUi>(new NibsMetaserverClientUi);
}

static void MetaserverClientUi_Handler(ParsedControl& control, void* data)
{
	NibsMetaserverClientUi* ui = static_cast<NibsMetaserverClientUi*>(data);
	switch(control.ID.id)
	{
		case iCHAT_ENTRY:
			ui->sendChat();
			break;

		default:
			break;
	}
}

#endif // !defined(DISABLE_NETWORKING)
