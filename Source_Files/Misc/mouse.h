#ifndef __MOUSE_H
#define __MOUSE_H

/*
MOUSE.H

	Copyright (C) 1991-2001 and beyond by Bungie Studios, Inc.
	and the "Aleph One" developers.
 
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	This license is contained in the file "GNU_GeneralPublicLicense.txt",
	which is included with this source code; it is available online at
	http://www.gnu.org/licenses/gpl.html

Tuesday, January 17, 1995 2:53:17 PM  (Jason')
*/

void enter_mouse(short type);
void test_mouse(short type, uint32 *action_flags, _fixed *delta_yaw, _fixed *delta_pitch, _fixed *delta_velocity);
void exit_mouse(short type);
void mouse_idle(short type);
void recenter_mouse(void);

#endif
