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

#ifndef BSTREAM_H
#define BSTREAM_H

#include "cseries.h"
#include <streambuf>

class basic_bstream
{
public:
	explicit basic_bstream(std::streambuf* sb) : sb_(sb) { }
	virtual ~basic_bstream() {};

	typedef std::ios_base::failure failure;

	std::streambuf* rdbuf() const { return sb_; }
	std::streambuf* rdbuf(std::streambuf* new_sb) { std::streambuf* old = sb_; sb_ = new_sb; return old; }

private:
	std::streambuf* sb_;
};

class BIStream : public basic_bstream
{
public:
	explicit BIStream(std::streambuf* sb) : basic_bstream(sb) { }

	std::streampos tellg() const;
	std::streampos maxg() const;

	BIStream& operator>>(uint8& value);
	BIStream& operator>>(int8& value);

	virtual BIStream& operator>>(int16& value) = 0;
	virtual BIStream& operator>>(uint16& value) = 0;
	virtual BIStream& operator>>(int32& value) = 0;
	virtual BIStream& operator>>(uint32& value) = 0;
	virtual BIStream& operator>>(double& value) = 0;

	BIStream& read(char *s, std::streamsize n);
	BIStream& ignore(std::streamsize n);
};

class BIStreamBE : public BIStream
{
public:
	explicit BIStreamBE(std::streambuf* sb) : BIStream(sb) {}
	
	BIStream& operator>>(uint8& value) {
		return BIStream::operator>>(value);
	}

	BIStream& operator>>(int8& value) {
		return BIStream::operator>>(value);
	}

	BIStream& operator>>(int16& value);
	BIStream& operator>>(uint16& value);
	BIStream& operator>>(int32& value);
	BIStream& operator>>(uint32& value);
	BIStream& operator>>(double& value);
};
	
class BOStream : public basic_bstream
{
public:
	explicit BOStream(std::streambuf* sb) : basic_bstream(sb) { }
	
	std::streampos tellp() const;
	std::streampos maxp() const;
	
	BOStream& operator<<(uint8 value);
	BOStream& operator<<(int8 value);

	virtual BOStream& operator<<(int16 value) = 0;
	virtual BOStream& operator<<(uint16 value) = 0;
	virtual BOStream& operator<<(int32 value) = 0;
	virtual BOStream& operator<<(uint32 value) = 0;
	virtual BOStream& operator<<(double value) = 0;

	BOStream& write(const char *s, std::streamsize n);
};

class BOStreamBE : public BOStream
{
public:
	explicit BOStreamBE(std::streambuf* sb) : BOStream(sb) {}
	
	BOStream& operator<<(uint8 value) {
		return BOStream::operator<<(value);
	}

	BOStream& operator<<(int8 value) {
		return BOStream::operator<<(value);
	}

	BOStream& operator<<(int16 value);
	BOStream& operator<<(uint16 value);
	BOStream& operator<<(int32 value);
	BOStream& operator<<(uint32 value);
	BOStream& operator<<(double value);
};

#endif
