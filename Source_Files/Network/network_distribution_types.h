/*
 *  network_distribution_types.h
 *  created for Marathon: Aleph One <http://source.bungie.org/>

	Copyright (C) 2002 and beyond by Woody Zenfell, III
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

 *  Centralized location for distribution types (for NetDistributeInformation,
 *  NetAddDistributionFunction, etc.) helps avoid potential conflicts.
 *
 *  Created by woody Mar 3-8, 2002.
 */

#ifndef NETWORK_DISTRIBUTION_TYPES_H
#define NETWORK_DISTRIBUTION_TYPES_H

enum {
        kOriginalNetworkAudioDistributionTypeID = 0,    // for compatibility with older versions
        kNewNetworkAudioDistributionTypeID = 1          // new-style realtime network audio data
};

#endif // NETWORK_DISTRIBUTION_TYPES_H
