/*
 *  network_ring.h

	Copyright (C) 2003 and beyond by Woody Zenfell, III
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

 *  Created by Woody Zenfell, III on Sat May 17 2003.
 *
 *  Interface between the old ring game protocol module and the rest of the game.
 */

#ifndef NETWORK_RING
#define NETWORK_RING

#include "NetworkGameProtocol.h"

#include <stdio.h>

class InfoTree;

class RingGameProtocol : public NetworkGameProtocol
{
public:
	bool	Enter(short* inNetStatePtr);
	void	Exit1();
	void	Exit2();
	void	DistributeInformation(short type, void *buffer, short buffer_size, bool send_to_self, bool only_send_to_team);
	bool	Sync(NetTopology* inTopology, int32 inSmallestGameTick, size_t inLocalPlayerIndex, size_t inServerPlayerIndex);
	bool	UnSync(bool inGraceful, int32 inSmallestPostgameTick);
	int32	GetNetTime();
	void	PacketHandler(DDPPacketBuffer* inPacket);

	int32   GetUnconfirmedActionFlagsCount();
	uint32  PeekUnconfirmedActionFlag(int32 offset);
	void    UpdateUnconfirmedActionFlags();

	static void ParsePreferencesTree(InfoTree prefs, std::string version);

	bool CheckWorldUpdate() override;
};

extern void DefaultRingPreferences();
InfoTree RingPreferencesTree();

#endif // NETWORK_RING
