#ifndef __PROGRESS_H
#define __PROGRESS_H

/*

	progress.h
	Saturday, October 28, 1995 10:58:57 PM- rdm created.

*/

enum {
	strPROGRESS_MESSAGES= 143,
	_distribute_map_single= 0,
	_distribute_map_multiple,
	_receiving_map,
	_awaiting_map,
	_distribute_physics_single,
	_distribute_physics_multiple,
	_receiving_physics
};

void open_progress_dialog(short message_id);
void close_progress_dialog(void);

void set_progress_dialog_message(short message_id);

void draw_progress_bar(long sent, long total);

void reset_progress_bar(void);

#endif

