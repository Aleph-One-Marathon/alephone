/*

	Copyright (C) 2007 Gregory Smith
	and the "Aleph One" developers.
 
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

	A pool for non-blocking outbound TCP connections

*/

#include "ConnectPool.h"
#include <utility>


NonblockingConnect::NonblockingConnect(const std::string& address, uint16 port)
	: m_address(address), m_port(port), m_thread(0), m_ipSpecified(false)
{
	connect();
}

NonblockingConnect::NonblockingConnect(const IPaddress& ip)
	: m_ip(ip), m_ipSpecified(true)
{
	connect();
}

NonblockingConnect::~NonblockingConnect()
{
	if (m_thread)
	{
		int status;
		SDL_WaitThread(m_thread, &status);
		m_thread = 0;
	}
}

void NonblockingConnect::connect()
{
	m_status = Connecting;
	m_thread = SDL_CreateThread(connect_thread, "NonblockingConnect_thread", this);
	if (!m_thread)
	{
		m_status = ConnectFailed;
	}

}

int NonblockingConnect::connect_thread(void *p)
{
	NonblockingConnect *nbc = static_cast<NonblockingConnect *>(p);
	return nbc->Thread();
}

int NonblockingConnect::Thread()
{
	if (!m_ipSpecified)
	{
		if (SDLNet_ResolveHost(&m_ip, const_cast<char*>(m_address.c_str()), m_port) < 0)
		{
			m_status = ResolutionFailed;
			return 1;
		}

	}

	std::unique_ptr<CommunicationsChannel> channel(new CommunicationsChannel);

	channel->connect(m_ip);
	if (channel->isConnected())
	{
		m_channel = std::move(channel);
		m_status = Connected;
		return 0;
	}
	else
	{
		m_status = ConnectFailed;
		return 2;
	}
}

ConnectPool::ConnectPool()
{
	for (int i = 0; i < kPoolSize; i++)
	{
		m_pool[i] = std::pair<NonblockingConnect *, bool>(0, true);
	}
}

void ConnectPool::fast_free()
{
	for (int i = 0; i < kPoolSize; i++)
	{
		if (m_pool[i].second)
		{
			if (m_pool[i].first && m_pool[i].first->done())
			{
				delete m_pool[i].first;
				m_pool[i].first = 0;
			}
		}
	}
}

NonblockingConnect* ConnectPool::connect(const std::string& address, uint16 port)
{
	fast_free();

	for (int i = 0; i < kPoolSize; i++)
	{
		if (m_pool[i].second && !m_pool[i].first)
		{
			m_pool[i].second = false;
			m_pool[i].first = new NonblockingConnect(address, port);
			return m_pool[i].first;
		}
	}

	return 0;
}

NonblockingConnect* ConnectPool::connect(const IPaddress& ip)
{
	fast_free();

	for (int i = 0; i < kPoolSize; i++)
	{
		if (m_pool[i].second && !m_pool[i].first)
		{
			m_pool[i].second = false;
			m_pool[i].first = new NonblockingConnect(ip);
			return m_pool[i].first;
		}
	}

	return 0;
}

void ConnectPool::abandon(NonblockingConnect *nbc)
{
	for (int i = 0; i < kPoolSize; i++)
	{
		if (m_pool[i].first == nbc)
		{
			m_pool[i].second = true;
		}
	}
}

ConnectPool::~ConnectPool()
{
	// uncomment these to allow clean-up at exit
// 	for (int i = 0; i < kPoolSize; i++)
// 	{
// 		if (m_pool[i].first)
// 		{
// 			delete m_pool[i];
// 		}
// 	}
}
	
