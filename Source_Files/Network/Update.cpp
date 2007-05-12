/*

	Copyright (C) 2007 Gregory Smith
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

	Checks for updates online

*/

#include "Update.h"
#include "SDL_net.h"
#include <boost/tokenizer.hpp>
#include <string>
#include "alephversion.h"

Update *Update::m_instance = 0;

Update::Update() : m_status(NoUpdateAvailable), m_thread(0)
{
	StartUpdateCheck();
}

Update::~Update()
{
	if (m_thread)
	{
		int status;
		SDL_WaitThread(m_thread, &status);
		m_thread = 0;
	}
}

int Update::update_thread(void *p)
{
	Update *update = static_cast<Update *>(p);
	update->Thread();
}

void Update::StartUpdateCheck()
{
	if (m_status == CheckingForUpdate) return;
	if (m_thread)
	{
		int status;
		SDL_WaitThread(m_thread, &status);
		m_thread = 0;
	}

	m_status = CheckingForUpdate;
	m_new_date_version.clear();
	m_new_display_version.clear();

	m_thread = SDL_CreateThread(update_thread, this);
	if (!m_thread)
	{
		m_status = UpdateCheckFailed;
	}
}

int Update::Thread()
{
	IPaddress ip;
	if (SDLNet_ResolveHost(&ip, "marathon.sourceforge.net", 80) < 0)
	{
		m_status = UpdateCheckFailed;
		return 1;
	}
	
	TCPsocket sock = SDLNet_TCP_Open(&ip);
	if (!sock)
	{
		m_status = UpdateCheckFailed;
		return 2;
	}

	std::string request = 
		std::string("GET /update_check/") + A1_UPDATE_PLATFORM + ".php HTTP/1.1\r\n" + 
		"Host: marathon.sourceforge.net\r\n" + 
		"Connection: close\r\n" +
		"\r\n";
	
	if (SDLNet_TCP_Send(sock, const_cast<char *>(request.c_str()), request.size()) < request.size())
	{
		m_status = UpdateCheckFailed;
		return 3;
	}
	
	// ghs: SHOULD but doesn't:
	// handle Location: header
	// handle chunked transfers?

	const int MAX_REPLY = 8192;
	char reply[MAX_REPLY];

	int len = SDLNet_TCP_Recv(sock, reply, MAX_REPLY);
	if (len < 0)
	{
		m_status = UpdateCheckFailed;
		return 4;
	}

	reply[len] = 0;

	std::string reply_string = reply;

	typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
	boost::char_separator<char> sep("\r\n");
	tokenizer tokens(reply_string, sep);

	for (tokenizer::iterator tok_iter = tokens.begin(); tok_iter != tokens.end(); ++tok_iter)
	{
		if (tok_iter->compare(0, strlen("A1_DATE_VERSION: "), "A1_DATE_VERSION: ") == 0)
		{
			m_new_date_version = tok_iter->substr(strlen("A1_DATE_VERSION: "));
		}
		else if (tok_iter->compare(0, strlen("A1_DISPLAY_VERSION: "), "A1_DISPLAY_VERSION: ") == 0)
		{
			m_new_display_version = tok_iter->substr(strlen("A1_DISPLAY_VERSION: "));
		}
	}

	if (m_new_date_version.size())
	{
		m_status = m_new_date_version.compare(A1_DATE_VERSION) > 0 ? UpdateAvailable : NoUpdateAvailable;
		return 0;
	}
	else
	{
		m_status = UpdateCheckFailed;
		return 5;
	}
}


