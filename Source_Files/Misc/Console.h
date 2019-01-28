/*
 *  Console.h - console utilities for Aleph One

	Copyright (C) 2005 and beyond by Gregory Smith
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

*/

#ifndef CONSOLE_H
#define CONSOLE_H

#include <string>
#include <map>
#include <boost/function.hpp>
#include "preferences.h"

class CommandParser
{
public:
	CommandParser() { }
	virtual ~CommandParser() = default;
	void register_command(std::string command, boost::function<void(const std::string&)> f);
	void register_command(std::string command, const CommandParser& command_parser);
	void unregister_command(std::string command);

	virtual void parse_and_execute(const std::string& command_string);
private:
	typedef std::map<std::string, boost::function<void(const std::string&)> > command_map;
	command_map m_commands;
};

class Console : public CommandParser
{
public:
	static Console* instance();

	// called by key handlers
	void enter();
	void abort(); // callback is called with empty string
	void del();
	void backspace();
	void clear();
	void forward_clear();
	void transpose();
	void delete_word();
	void textEvent(const SDL_Event &e);
	void up_arrow();
	void down_arrow();
	void left_arrow();
	void right_arrow();
	void line_home();
	void line_end();
	const std::string &displayBuffer() { return m_displayBuffer; }

	void activate_input(boost::function<void (const std::string&)> callback,
			    const std::string& prompt);
	void deactivate_input(); // like abort, but no callback

	bool input_active() { return m_active; }
	int cursor_position();

	void register_macro(std::string macro, std::string replacement);
	void unregister_macro(std::string macro);
	void clear_macros();

	// carnage reporting
	void set_carnage_message(int16 projectile_type, const std::string& on_kill, const std::string& on_suicide = "");
	void report_kill(int16 player_index, int16 aggressor_player_index, int16 projectile_index);
	void clear_carnage_messages();

	bool use_lua_console() { return m_use_lua_console || environment_preferences->use_solo_lua; };
	void use_lua_console(bool f_use) { m_use_lua_console = f_use; }

	// clear last saved level name
	void clear_saves();

private:
	Console();

	boost::function<void (std::string)> m_callback;
	std::string m_buffer;
	std::string m_displayBuffer;
	std::string m_prompt;
	bool m_active;
	
	std::vector<std::string> m_prev_commands;
	std::vector<std::string>::iterator m_command_iter;
	void set_command(std::string command);
	
	int m_cursor_position;

	std::map<std::string, std::string> m_macros;

	bool m_carnage_messages_exist;
	std::vector<std::pair<std::string, std::string> > m_carnage_messages;

	bool m_use_lua_console;

	void register_save_commands();
};

class InfoTree;
void parse_mml_console(const InfoTree& root);
void reset_mml_console();

#endif

 
