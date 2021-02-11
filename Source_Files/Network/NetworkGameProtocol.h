/*
 *  NetworkGameProtocol.h

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
 *  Interface to a generic game protocol module.
 */

#ifndef NETWORKGAMEPROTOCOL_H
#define NETWORKGAMEPROTOCOL_H

#include "network_private.h"

class NetworkGameProtocol
{
public:
	/* Distribute information to the whole net. */
	virtual bool	Enter(short* inNetStatePtr) = 0;
	virtual void	Exit1() = 0;
	virtual void	Exit2() = 0;
	virtual void	DistributeInformation(short type, void *buffer, short buffer_size, bool send_to_self, bool only_send_to_team) = 0;
	virtual bool	Sync(NetTopology* inTopology, int32 inSmallestGameTick, size_t inLocalPlayerIndex, size_t inServerPlayerIndex) = 0;
	virtual bool	UnSync(bool inGraceful, int32 inSmallestPostgameTick) = 0;
	virtual int32	GetNetTime() = 0;
	virtual void	PacketHandler(DDPPacketBuffer* inPacket) = 0;
	virtual		~NetworkGameProtocol() {}

	// action flags we can use for prediction, but aren't authoritative yet
	virtual int32   GetUnconfirmedActionFlagsCount() = 0;
	virtual uint32  PeekUnconfirmedActionFlag(int32 offset) = 0;
	virtual void    UpdateUnconfirmedActionFlags() = 0;

	virtual bool CheckWorldUpdate() = 0;
};

#endif // NETWORKGAMEPROTOCOL_H
