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

#include "Statistics.h"
#include "HTTP.h"
#include "lua_script.h"

#include "sdl_widgets.h"
#include "alephversion.h"
#include "preferences.h"
#include "sdl_network.h"

#include <sstream>
#include <boost/bind.hpp>

class ScopedMutex
{
public:
	ScopedMutex(SDL_mutex* mutex) : mutex_(mutex) {
		SDL_LockMutex(mutex_);
	}

	~ScopedMutex() { 
		SDL_UnlockMutex(mutex_);
	}
private:
	SDL_mutex* mutex_;
};

StatsManager::StatsManager() : thread_(0), run_(true)
{
	entry_mutex_ = SDL_CreateMutex();

	// do uploads in a separate thread
	thread_ = SDL_CreateThread(Run, "StatsManager_uploadThread", this);
}

void StatsManager::Process()
{
	Entry entry;
	if (CollectLuaStats(entry.options, entry.parameters))
	{
		ScopedMutex mutex(entry_mutex_);
		busy_ = true;
		entries_.push(entry);
	}
}

void StatsManager::CheckForDone(dialog* d)
{
	if (!busy_)
	{
		run_ = false;
		SDL_WaitThread(thread_, NULL);
		thread_ = 0;
		d->quit(0);
	}
}

void StatsManager::Finish()
{
	if (busy_)
	{
		dialog d;
		vertical_placer* placer = new vertical_placer;
		placer->dual_add(new w_static_text("Uploading stats"), d);
		placer->add(new w_spacer, true);
		w_button *button = new w_button("CANCEL", dialog_cancel, &d);
		placer->dual_add(button, d);
		d.set_widget_placer(placer);
		d.activate_widget(button);
		
		d.set_processing_function(boost::bind(&StatsManager::CheckForDone, this, _1));
		d.run();
	}
	else
	{
		run_ = false;
		SDL_WaitThread(thread_, NULL);
		thread_ = 0;
	}
}

static uint32 checksum_string(const std::string& s)
{
	uint32 checksum = 0;
	for (std::string::const_iterator it = s.begin(); it != s.end(); ++it)
	{
		checksum += reinterpret_cast<const unsigned char&>(*it);
	}

	return checksum;
}

int StatsManager::Run(void *pv)
{
	StatsManager* sm = reinterpret_cast<StatsManager*>(pv);
	HTTPClient client;

	while (sm->run_)
	{
		std::unique_ptr<Entry> entry;
		{
			ScopedMutex mutex(sm->entry_mutex_);
			if (sm->entries_.empty())
			{
				sm->busy_ = false;
			}
			else
			{
				entry.reset(new Entry(sm->entries_.front()));
				sm->entries_.pop();
			}
		}

		if (entry.get())
		{
			entry->parameters["platform"] = A1_DISPLAY_PLATFORM;
			if (dynamic_world->player_count > 1)
				entry->parameters["session id"] = NetSessionIdentifier();
			entry->parameters["username"] = network_preferences->metaserver_login;
			entry->parameters["password"] = network_preferences->metaserver_password;
			
			// generate checksum
			uint32 checksum = 0;
			for (std::map<std::string, std::string>::const_iterator it = entry->parameters.begin(); it != entry->parameters.end(); ++it)
			{
				checksum += checksum_string(it->first);
				checksum += checksum_string(it->second);
			}
		
			std::ostringstream oss;
			oss << checksum;
			entry->parameters["checksum"] = oss.str();

			client.Post(A1_STATSERVER_ADD_URL, entry->parameters);
		}
		else
		{
			sleep_for_machine_ticks(MACHINE_TICKS_PER_SECOND / 5);
		}
		
	}

	return 0;
}
