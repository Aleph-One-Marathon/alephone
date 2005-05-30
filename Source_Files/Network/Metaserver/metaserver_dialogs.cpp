/*
 *  metaserver_dialogs.cpp - UI for metaserver client

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

#if !defined(DISABLE_NETWORKING)

#include "cseries.h"

#include "metaserver_dialogs.h"
#include "network_private.h" // GAME_PORT
#include "preferences.h"
#include "network_metaserver.h"
#include "map.h" // for _force_unique_teams!?!



const IPaddress
run_network_metaserver_ui()
{
	return MetaserverClientUi::Create()->GetJoinAddressByRunning();
}


void
setupAndConnectClient(MetaserverClient& client)
{
	{
		unsigned char* playerNameCStringStorage = pstrdup(player_preferences->name);
		char* playerNameCString = a1_p2cstr(playerNameCStringStorage);
		client.setPlayerName(playerNameCString);
		free(playerNameCStringStorage);
	}
	
	client.setPlayerTeamName("");
	client.connect("myth.mariusnet.com", 6321, "guest", "0000000000000000");
}



GameAvailableMetaserverAnnouncer::GameAvailableMetaserverAnnouncer(const game_info& info)
{
	setupAndConnectClient(m_client);

	GameDescription description;
	description.m_type = info.net_game_type;
	description.m_timeLimit = info.time_limit;
	description.m_difficulty = info.difficulty_level;
	description.m_mapName = string(info.level_name);
	description.m_name = m_client.playerName() + "'s Game";
	description.m_teamsAllowed = !(info.game_options & _force_unique_teams);

	m_client.announceGame(GAME_PORT, description);
}

#endif // !defined(DISABLE_NETWORKING)
