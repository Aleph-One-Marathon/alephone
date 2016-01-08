#ifndef __MOTION_SENSOR_H
#define __MOTION_SENSOR_H

/*
MOTION_SENSOR.H

	Copyright (C) 1991-2001 and beyond by Bungie Studios, Inc.
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

Friday, June 17, 1994 12:10:02 PM

Jan 30, 2000 (Loren Petrich)
	Changed "friend" to "_friend" to make data structures more C++-friendly

May 1, 2000 (Loren Petrich): Added XML parser object for the stuff here.
*/

#include "shape_descriptors.h"

enum {
	MType_Friend,	// What you, friendly players, and the Bobs are
	MType_Alien,	// What the other critters are
	MType_Enemy,	// What hostile players are
	NUMBER_OF_MDISPTYPES
};


/* ---------- prototypes/MOTION_SENSOR.C */

void initialize_motion_sensor(shape_descriptor mount, shape_descriptor virgin_mounts,
	shape_descriptor alien, shape_descriptor _friend, shape_descriptor enemy,
	shape_descriptor network_compass, short side_length);
void reset_motion_sensor(short monster_index);
void motion_sensor_scan(void);
bool motion_sensor_has_changed(void);
void adjust_motion_sensor_range(void);

class InfoTree;
void parse_mml_motion_sensor(const InfoTree& root);
void reset_mml_motion_sensor();

#endif
