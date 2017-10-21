/*

	Copyright (C) 2009 by Gregory Smith
 
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

	BStream: serialization to/from a streambuf -- meant to replace AStream
*/

#include "BStream.h"
#include <SDL_endian.h>

std::streampos BIStream::tellg() const
{
	return rdbuf()->pubseekoff(0, std::ios_base::cur, std::ios_base::in);
}

std::streampos BIStream::maxg() const
{
	std::streampos cur = tellg();
	std::streampos max = rdbuf()->pubseekoff(0, std::ios_base::end, std::ios_base::in);
	rdbuf()->pubseekpos(cur, std::ios_base::in);
	return max;
}

BIStream& BIStream::read(char *s, std::streamsize n)
{
	if (rdbuf()->sgetn(s, n) != n)
	{
		throw failure("serialization bound check failed");
	}

	return *this;
}

BIStream& BIStream::ignore(std::streamsize n)
{
	if (rdbuf()->pubseekoff(n, std::ios_base::cur, std::ios_base::in) < 0)
	{
		throw failure("serialization bounc check failed");
	}

	return *this;
}

BIStream& BIStream::operator>>(uint8& value)
{
	return read(reinterpret_cast<char*>(&value), 1);
}

BIStream& BIStream::operator>>(int8& value)
{
	uint8 uvalue;
	operator>>(uvalue);
	value = static_cast<int8>(uvalue);
	return *this;
}

BIStream& BIStreamBE::operator>>(uint16& value)
{
	read(reinterpret_cast<char*>(&value), 2);
	value = SDL_SwapBE16(value);
	return *this;
}

BIStream& BIStreamBE::operator>>(int16& value)
{
	uint16 uvalue;
	operator>>(uvalue);
	value = static_cast<int16>(uvalue);
	return *this;
}

BIStream& BIStreamBE::operator>>(uint32& value)
{
	read(reinterpret_cast<char*>(&value), 4);
	value = SDL_SwapBE32(value);
	return *this;
}

BIStream& BIStreamBE::operator>>(int32& value)
{
	uint32 uvalue;
	operator>>(uvalue);
	value = static_cast<int32>(uvalue);
	return *this;
}

BIStream& BIStreamBE::operator>>(double& value)
{
	Uint64 ivalue;
	read(reinterpret_cast<char*>(&ivalue), 8);
	ivalue = SDL_SwapBE64(ivalue);
	memcpy(reinterpret_cast<char*>(&value), reinterpret_cast<char*>(&ivalue), 8);
	return *this;
}

std::streampos BOStream::tellp() const 
{
	return rdbuf()->pubseekoff(0, std::ios_base::cur, std::ios_base::out);
}

std::streampos BOStream::maxp() const
{
	std::streampos cur = tellp();
	std::streampos max = rdbuf()->pubseekoff(0, std::ios_base::end, std::ios_base::out);
	rdbuf()->pubseekpos(cur, std::ios_base::out);
	return max;
}

BOStream& BOStream::write(const char *s, std::streamsize n)
{
	if (rdbuf()->sputn(s, n) != n)
	{
		throw failure("serialization bound check failed");
	}

	return *this;
}

BOStream& BOStream::operator<<(uint8 value)
{
	return write(reinterpret_cast<char*>(&value), 1);
}

BOStream& BOStream::operator<<(int8 value)
{
	return operator<<(static_cast<uint8>(value));
}

BOStream& BOStreamBE::operator<<(uint16 value)
{
	value = SDL_SwapBE16(value);
	return write(reinterpret_cast<char*>(&value), 2);
}

BOStream& BOStreamBE::operator<<(int16 value)
{
	return operator<<(static_cast<uint16>(value));
}

BOStream& BOStreamBE::operator<<(uint32 value)
{
	value = SDL_SwapBE32(value);
	return write(reinterpret_cast<char*>(&value), 4);
}

BOStream& BOStreamBE::operator<<(int32 value)
{
	return operator<<(static_cast<uint32>(value));
}

BOStream& BOStreamBE::operator<<(double value)
{
	Uint64 ivalue;
	memcpy(reinterpret_cast<char*>(&ivalue), reinterpret_cast<char*>(&value), 8);
	ivalue = SDL_SwapBE64(ivalue);
	return write(reinterpret_cast<char*>(&ivalue), 8);
}
