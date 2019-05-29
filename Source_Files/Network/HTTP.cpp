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

#include "HTTP.h"

#include "cseries.h"
#include "Logging.h"
#include "preferences.h"

#ifdef HAVE_CURL

#ifdef __WIN32__
#define WIN32_LEAN_AND_MEAN // curl.h includes <windows.h>
#endif

#include "curl/curl.h"
#include "curl/easy.h"

#include <boost/shared_ptr.hpp>

void HTTPClient::Init()
{
	curl_global_init(CURL_GLOBAL_ALL);
}

size_t HTTPClient::WriteCallback(void* buffer, size_t size, size_t nmemb, void* p)
{
	HTTPClient* client = reinterpret_cast<HTTPClient*>(p);
	client->response_.append(reinterpret_cast<char*>(buffer), size * nmemb);
	return size * nmemb;
}

bool HTTPClient::Get(const std::string& url)
{
	response_.clear();

	boost::shared_ptr<CURL> handle(curl_easy_init(), curl_easy_cleanup);
	if (!handle)
	{
		logError("CURL init failed");
		return false;
	}

	curl_easy_setopt(handle.get(), CURLOPT_URL, url.c_str());
	curl_easy_setopt(handle.get(), CURLOPT_WRITEFUNCTION, WriteCallback);
	curl_easy_setopt(handle.get(), CURLOPT_WRITEDATA, this);
	curl_easy_setopt(handle.get(), CURLOPT_SSL_VERIFYPEER, network_preferences->verify_https);
	curl_easy_setopt(handle.get(), CURLOPT_FOLLOWLOCATION, 1);

	CURLcode ret = curl_easy_perform(handle.get());
	if (ret == CURLE_OK) 
	{
		return true;
	}
	else 
	{
		logError("HTTP(s) GET from %s failed: %s", url.c_str(), curl_easy_strerror(ret));
		return false;
	}
}

#if LIBCURL_VERSION_NUM >= 0x071504
static std::string escape(CURL* handle, const std::string& s)
#else
static std::string escape(CURL*, const std::string& s)
#endif
{
#if LIBCURL_VERSION_NUM >= 0x071504
	boost::shared_ptr<char> dst(curl_easy_escape(handle, s.c_str(), s.size()), curl_free);
#else
	boost::shared_ptr<char> dst(curl_escape(s.c_str(), s.size()), curl_free);
#endif
	return std::string(dst.get());
}

bool HTTPClient::Post(const std::string& url, const parameter_map& parameters)
{
	response_.clear();

	boost::shared_ptr<CURL> handle(curl_easy_init(), curl_easy_cleanup);
	if (!handle)
	{
		logError("CURL init failed");
		return false;
	}


	std::string parameter_string;
	for (parameter_map::const_iterator it = parameters.begin(); it != parameters.end(); ++it)
	{
		if (parameter_string.size())
		{
			parameter_string.append("&");
		}
		parameter_string.append(escape(handle.get(), it->first));
		parameter_string.append("=");
		parameter_string.append(escape(handle.get(), it->second));
	}

	curl_easy_setopt(handle.get(), CURLOPT_URL, url.c_str());
	curl_easy_setopt(handle.get(), CURLOPT_WRITEFUNCTION, WriteCallback);
	curl_easy_setopt(handle.get(), CURLOPT_WRITEDATA, this);
	curl_easy_setopt(handle.get(), CURLOPT_POST, 1L);
	curl_easy_setopt(handle.get(), CURLOPT_SSL_VERIFYPEER, network_preferences->verify_https);
	curl_easy_setopt(handle.get(), CURLOPT_POSTFIELDS, parameter_string.c_str());

	CURLcode ret = curl_easy_perform(handle.get());
	if (ret == CURLE_OK)
	{
		return true;
	} 
	else
	{
		logError("HTTP(s) POST to %s failed: %s", url.c_str(), curl_easy_strerror(ret));
		return false;
	}
}
#else // we do not HAVE_CURL
void HTTPClient::Init()
{

}

bool HTTPClient::Get(const std::string&)
{
	return false;
}

bool HTTPClient::Post(const std::string&, const std::map<std::string, std::string>&)
{
	return false;
}

#endif

