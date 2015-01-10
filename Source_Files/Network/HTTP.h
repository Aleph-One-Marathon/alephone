#ifndef __HTTP_H
#define __HTTP_H

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

	HTTP utilities
*/

#include <map>
#include <string>

class HTTPClient
{
public:
	typedef std::map<std::string, std::string> parameter_map;
	static void Init();

	HTTPClient() { }
	
	bool Get(const std::string& url);
	bool Post(const std::string& url, const parameter_map& parameters);
	std::string Response() { return response_; }

private:
	static size_t WriteCallback(void* buffer, size_t size, size_t nmemb, void* userp);
	std::string response_;
};

#endif
