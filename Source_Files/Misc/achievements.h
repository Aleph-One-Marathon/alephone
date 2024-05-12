#ifndef ACHIEVEMENTS_H
#define ACHIEVEMENTS_H

/*
	Copyright (C) 2024 Gregory Smith
 
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

    Collects achievements
 */

#include <cstdint>
#include <string>

class Achievements {
public:
	static Achievements* instance();

	std::string get_lua();
	void set(const std::string& key);
	void set_disabled_reason(const std::string& reason) { disabled_reason = reason; }
	const std::string& get_disabled_reason() const { return disabled_reason; }

private:
	std::string disabled_reason;
};

#endif
