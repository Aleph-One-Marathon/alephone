/*
 *  Console.h - console utilities for Aleph One

	Copyright (C) 2005 and beyond by Gregory Smith
	and the "Aleph One" developers.
 
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
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
#include <boost/function.hpp>

class Console 
{
 public:
  static Console* instance();

  // called by key handlers
  void enter();
  void abort(); // callback is called with empty string
  void backspace();
  void clear();
  void key(const char);
  const std::string &displayBuffer() { return m_displayBuffer; }

  void activate_input(boost::function<void (const std::string&)> callback,
		const std::string& prompt);
  void deactivate_input(); // like abort, but no callback

  bool input_active() { return m_active; }
  
 private:
  Console() : m_active(false) { } ;
  static Console* m_instance;

  boost::function<void (std::string)> m_callback;
  std::string m_buffer;
  std::string m_displayBuffer;
  std::string m_prompt;
  bool m_active;
};

#endif

 
