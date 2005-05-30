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
	iCHAT_HISTORY = 300,
	iCHAT_ENTRY = 400
};

static void MetaserverClientUi_Handler(ParsedControl& control, void* data);
const double PollingInterval = 1.0/30.0;

static pascal void MetaserverClientUi_Poller(EventLoopTimerRef, void*)
{
	MetaserverClient::pumpAll();
}

class NibsMetaserverClientUi : public MetaserverClientUi, public MetaserverClient::NotificationAdapter
{
public:
	NibsMetaserverClientUi()
	: m_metaserverClientNib(CFSTR("Metaserver Client"))
	, m_dialog(m_metaserverClientNib.nibReference(), CFSTR("Metaserver Client"))
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
	}

	void gamesInRoomChanged()
	{
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
	AutoNibReference	m_metaserverClientNib;
	AutoNibWindow		m_dialog;
	MetaserverClient	m_metaserverClient;
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
