#ifndef STATISTICS_H
#define STATISTICS_H

/*
	Copyright (C) 2011 Gregory Smith
 
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

	Collects and uploads game stats
*/

#include "cseries.h"

#include <list>
#include <map>
#include <queue>
#include <string>

#include <SDL_mutex.h>
#include <SDL_thread.h>

#include "sdl_dialogs.h"

class StatsManager {
public:
	static StatsManager* instance() { 
		static StatsManager *instance_ = nullptr;
		if (!instance_) 
			instance_  = new StatsManager();
		return instance_;
	}

	// grabs the current stats from the Lua script and post them
	void Process();

	// are all the stats written / uploaded?
	bool Busy() { return busy_; }

	void Finish();

private:
	struct Entry {
		std::map<std::string, std::string> options;
		std::map<std::string, std::string> parameters;
	};

	StatsManager();

	void CheckForDone(dialog* d);

	std::queue<Entry, std::list<Entry> > entries_;
	
	// thread fun
	SDL_Thread* thread_;
	SDL_mutex* entry_mutex_;
	bool run_;
	bool busy_;
	static int Run(void *);
};

#endif
