#ifndef __MOTION_SENSOR_H
#define __MOTION_SENSOR_H

/*
MOTION_SENSOR.H
Friday, June 17, 1994 12:10:02 PM

Jan 30, 2000 (Loren Petrich)
	Changed "friend" to "_friend" to make data structures more C++-friendly

May 1, 2000 (Loren Petrich): Added XML parser object for the stuff here.
*/

#include "XML_ElementParser.h"

/* ---------- prototypes/MOTION_SENSOR.C */

void initialize_motion_sensor(shape_descriptor mount, shape_descriptor virgin_mounts,
	shape_descriptor alien, shape_descriptor _friend, shape_descriptor enemy,
	shape_descriptor network_compass, short side_length);
void reset_motion_sensor(short monster_index);
bool motion_sensor_has_changed(void);
void adjust_motion_sensor_range(void);

// LP addition: get the parser for the motion-sensor elements (name "motion_sensor")
XML_ElementParser *MotionSensor_GetParser();

#endif
