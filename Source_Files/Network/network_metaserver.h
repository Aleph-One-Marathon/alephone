/*
 *  network_metaserver.h
 *  AlephOne-OSX
 *
 *  Created by Woody Zenfell, III on Wed Jul 16 2003.
 *  Copyright (c) 2003 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef NETWORK_METASERVER_H
#define NETWORK_METASERVER_H

class MetaserverClient	// Singleton
{
public:
	bool	IsConnected() const;
	void	Connect(const char* inAddress);
	void	Disconnect();

	typedef std::vector<MetaserverRoom> RoomCollection;
	typedef typename RoomCollection::iterator RoomCollectionIterator;
	typedef typename RoomCollection::const_iterator RoomCollectionConstIterator;

	void	SetRoom(MetaserverRoom& inRoom);
	void	LeaveRoom();
	bool	IsInARoom() const;
	const MetaserverRoom& CurrentRoom() const;

	
};

class MetaserverPlayer;
class MetaserverGame;

class MetaserverRoom;	// fairly "dumb"; describes a room to client code.  cannot be created by clients.
// Name

class MetaserverConnectedRoom : public MetaserverRoom
{
	// Games
 // Players
 // Send chat msg
};


#endif // NETWORK_METASERVER_H
