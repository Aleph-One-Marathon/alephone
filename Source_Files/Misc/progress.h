#ifndef __PROGRESS_H
#define __PROGRESS_H

/*
	progress.h

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

	Saturday, October 28, 1995 10:58:57 PM- rdm created.

*/

#include <stddef.h>

enum {
	strPROGRESS_MESSAGES= 143,
	_distribute_map_single= 0,
	_distribute_map_multiple,
	_receiving_map,
	_awaiting_map,
	_distribute_physics_single,
	_distribute_physics_multiple,
	_receiving_physics,
	// non-network ones
	_loading,
	// more network ones
	_opening_router_ports,
	_closing_router_ports,
	_checking_for_updates
};

void open_progress_dialog(size_t message_id, bool show_progress_bar = false);
void close_progress_dialog(void);

void set_progress_dialog_message(size_t message_id);

void draw_progress_bar(size_t sent, size_t total);

void reset_progress_bar(void);

#endif

