/*
 *  metaserver_dialogs.h - UI for metaserver client

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
 */

#ifndef METASERVER_DIALOGS_H
#define METASERVER_DIALOGS_H

#include "network_metaserver.h"
#include <set>

const IPaddress run_network_metaserver_ui();

struct game_info;

class GameAvailableMetaserverAnnouncer
{
public:
	GameAvailableMetaserverAnnouncer(const game_info& info);
	~GameAvailableMetaserverAnnouncer();

	void pump();

	static void pumpAll();

private:
	MetaserverClient	m_client;

	static std::set<GameAvailableMetaserverAnnouncer*> s_instances;
};

#endif // METASERVER_DIALOGS_H
