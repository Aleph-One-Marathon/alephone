#ifndef __MOUSE_H
#define __MOUSE_H

/*
MOUSE.H
Tuesday, January 17, 1995 2:53:17 PM  (Jason')
*/

void enter_mouse(short type);
void test_mouse(short type, uint32 *action_flags, fixed *delta_yaw, fixed *delta_pitch, fixed *delta_velocity);
void exit_mouse(short type);
void mouse_idle(short type);

#endif
