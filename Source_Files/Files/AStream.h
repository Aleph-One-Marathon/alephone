/////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////

/*
 *  AStream.h
 *  AlephModular
 *
 *	Class to handle serialization issues. This is equivalent to Packing[.h/.cpp]
 *	in AlephOne. And is derived from those files.
 *
 *	Why are we doing this instead of just using Packing[.h/.cpp]?
 *	Because of 2 things. First of all, Packing.h was less clear then it should
 *	have been, the choice between Big Endian and Little Endian was made at the
 *	time the file was included. And the actual elements used from the file don't
 *	specify endian explicitly. Second of all, I wanted the stream elements to be
 *	clearly typed and encapsulated.
 *
 *  Created by Br'fin on Wed Nov 27 2002.
 *

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

#ifndef __ASTREAM_H
#define __ASTREAM_H

#include <string>
#include <exception>
#include "cstypes.h"

namespace AStream
{
	enum _Aiostate { _M_aiostate_end = 1L << 16 };
	static const short _S_badbit = 0x01;
	static const short _S_failbit = 0x02;

	typedef _Aiostate iostate;
	static const iostate badbit = iostate(_S_badbit);
	static const iostate failbit = iostate(_S_failbit);
	static const iostate goodbit = iostate(0);

	class failure : public std::exception
	{
		public:
			explicit failure(const std::string& __str) noexcept;
			failure(const failure &f);
			~failure() noexcept;
			const char*
			what() const noexcept;

		private:
			char * _M_name;
	};

	template <typename T>
	class basic_astream
	{
	private:
		T *_M_stream_begin;
		T *_M_stream_end;
		iostate _M_state;
		iostate _M_exception;
	protected:
		T *_M_stream_pos;
		bool
		bound_check(uint32 __delta);
		
		uint32
		tell_pos() const
		{ return _M_stream_pos - _M_stream_begin; } 

		uint32
		max_pos() const
		{ return _M_stream_end - _M_stream_begin; }

	public:
		iostate
		rdstate() const
		{ return _M_state; }

		void
		setstate(iostate __state)
		{ _M_state= iostate(this->rdstate() | __state); }
		
		bool
		good() const
		{ return this->rdstate() == 0; }
		
		bool 
		fail() const
		{ return (this->rdstate() & (badbit | failbit)) != 0; }

		bool
		bad() const
		{ return (this->rdstate() & badbit) != 0; }
		
		iostate
		exceptions() const
		{ return _M_exception; }
		
		void
		exceptions(iostate except)
		{ _M_exception= except; }
		
		basic_astream(T* __stream, uint32 __length, uint32 __offset) :
			_M_stream_begin(__stream),
			_M_stream_end(__stream + __length),
			_M_state(goodbit),
			_M_exception(failbit),
			_M_stream_pos(__stream + __offset)
		{ if(_M_stream_pos > _M_stream_end) { this->setstate(badbit); } }

		virtual ~basic_astream() {};
	};
}

/* Input Streams, deserializing */

class AIStream : public AStream::basic_astream<const uint8>
{
public:
	AIStream(const uint8* __stream, uint32 __length, uint32 __offset) :
		AStream::basic_astream<const uint8>(__stream, __length, __offset) {}

	uint32
	tellg() const
	{ return this->tell_pos(); }
			
	uint32
	maxg() const
	{ return this->max_pos(); }

	AIStream&
	operator>>(uint8 &__value);
	
	AIStream&
	operator>>(int8 &__value);
	
	virtual AIStream&
	  operator>>(bool &__value);
  
	virtual AIStream&
	operator>>(uint16 &__value) = 0;
	
	virtual AIStream&
	operator>>(int16 &__value) = 0;
	
	virtual AIStream&
	operator>>(uint32 &__value) = 0;
	
	virtual AIStream&
	operator>>(int32 &__value) = 0;

	AIStream&
	read(char *__ptr, uint32 __count);
	
	AIStream&
	read(unsigned char * __ptr, uint32 __count)
	{ return read((char *) __ptr, __count); }
	
	AIStream&
	read(signed char * __ptr, uint32 __count)
	{ return read((char *) __ptr, __count); }
	
	AIStream&
	ignore(uint32 __count);

	// Uses >> instead of operator>> so as to pick up friendly operator>>
	template<class T>
	inline AIStream&
	read(T* __list, uint32 __count)
	{
		T* ValuePtr = __list;
		for (unsigned int k=0; k<__count; k++)
			*this >> *(ValuePtr++);
  
		return *this;
	}
  
};

class AIStreamBE : public AIStream
{
public:
	AIStreamBE(const uint8* __stream, uint32 __length, uint32 __offset = 0) :
		AIStream(__stream, __length, __offset) {};

	AIStream& 
	operator>>(uint8 &__value)
		{ return AIStream::operator>>(__value); }

	AIStream&
	operator>>(int8 &__value)
		{ return AIStream::operator>>(__value); }
	
	AIStream&
	operator>>(uint16 &__value);
	
	AIStream&
	operator>>(int16 &__value);
	
	AIStream&
	operator>>(uint32 &__value);
	
	AIStream&
	operator>>(int32 &__value);
};

class AIStreamLE : public AIStream
{
public:
	AIStreamLE(const uint8* __stream, uint32 __length, uint32 __offset = 0) :
		AIStream(__stream, __length, __offset) {};

  	AIStream& 
	operator>>(uint8 &__value)
		{ return AIStream::operator>>(__value); }

	AIStream&
	operator>>(int8 &__value)
		{ return AIStream::operator>>(__value); }

	AIStream&
	operator>>(uint16 &__value);
	
	AIStream&
	operator>>(int16 &__value);
	
	AIStream&
	operator>>(uint32 &__value);
	
	AIStream&
	operator>>(int32 &__value);
};

/* Output Streams, serializing */

class AOStream : public AStream::basic_astream<uint8>
{
public:
	AOStream(uint8* __stream, uint32 __length, uint32 __offset) :
		AStream::basic_astream<uint8>(__stream, __length, __offset) {}

	uint32
	tellp() const
	{ return this->tell_pos(); }
			
	uint32
	maxp() const
	{ return this->max_pos(); }
		
	AOStream&
	operator<<(uint8 __value);
	
	AOStream&
	operator<<(int8 __value);

	virtual AOStream&
        operator<<(bool __value);  

	virtual AOStream&
	operator<<(uint16 __value) = 0;
	
	virtual AOStream&
	operator<<(int16 __value) = 0;
	
	virtual AOStream&
	operator<<(uint32 __value) = 0;
	
	virtual AOStream&
	operator<<(int32 __value) = 0;


	AOStream&
	write(char *__ptr, uint32 __count);
	
	AOStream&
	write(unsigned char * __ptr, uint32 __count)
	{ return write((char *) __ptr, __count); }
	
	AOStream&
	write(signed char * __ptr, uint32 __count)
	{ return write((char *) __ptr, __count); }
	
	AOStream& ignore(uint32 __count);

	// Uses << instead of operator>> so as to pick up friendly operator<<
	template<class T>
	inline AOStream&
	write(T* __list, uint32 __count)
	{
		T* ValuePtr = __list;
		for (unsigned int k=0; k<__count; k++)
			*this << *(ValuePtr++);
    
		return *this;
	}
};

class AOStreamBE: public AOStream
{
public:
	AOStreamBE(uint8* __stream, uint32 __length, uint32 __offset = 0) :
		AOStream(__stream, __length, __offset) {};

	AOStream&
	operator<<(uint8 __value)
		{ return AOStream::operator<<(__value); }

	AOStream&
	operator<<(int8 __value)
		{ return AOStream::operator<<(__value); }
  
	AOStream&
	operator<<(uint16 __value);
	
	AOStream&
	operator<<(int16 __value);
	
	AOStream&
	operator<<(uint32 __value);
	
	AOStream&
	operator<<(int32 __value);
};

class AOStreamLE: public AOStream
{
public:
	AOStreamLE(uint8* __stream, uint32 __length, uint32 __offset = 0) :
		AOStream(__stream, __length, __offset) {}

  	AOStream&
	operator<<(uint8 __value)
		{ return AOStream::operator<<(__value); }

	AOStream&
	operator<<(int8 __value)
		{ return AOStream::operator<<(__value); }
 
	AOStream&
	operator<<(uint16 __value);
	
	AOStream&
	operator<<(int16 __value);
	
	AOStream&
	operator<<(uint32 __value);
	
	AOStream&
	operator<<(int32 __value);
};

#endif
