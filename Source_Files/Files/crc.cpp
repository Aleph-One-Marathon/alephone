/*
	crc.c

	Copyright (C) 1991-2001 and beyond by Bungie Studios, Inc.
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

	Sunday, March 5, 1995 6:21:30 PM

	CRC Checksum generation for a file.

Aug 15, 2000 (Loren Petrich):
	Using object-oriented file handler

Nov 6, 2000 (Loren Petrich);
	Suppressed commented-out FileSpecifier function
*/

#include <stdlib.h>

#include "cseries.h"
#include "FileHandler.h"
#include "crc.h"

#ifdef env68k
	#pragma segment file_io
#endif

/* ---------- constants */
#define TABLE_SIZE (256)
#define CRC32_POLYNOMIAL 0xEDB88320L
#define BUFFER_SIZE 1024

/* ---------- local data */
static uint32 *crc_table= NULL;

/* ---------- local prototypes ------- */
static uint32 calculate_file_crc(unsigned char *buffer, 
	short buffer_size, OpenedFile& OFile);
static uint32 calculate_buffer_crc(long count, uint32 crc, void *buffer);
static bool build_crc_table(void);
static void free_crc_table(void);

/* -------------- Entry Point ----------- */
uint32 calculate_crc_for_file(FileSpecifier& File)
{
	uint32 crc = 0;
	
	OpenedFile OFile;
	if (File.Open(OFile))
	{
		crc= calculate_crc_for_opened_file(OFile);
		OFile.Close();
	}
	
	return crc;
}

uint32 calculate_crc_for_opened_file(OpenedFile& OFile)
{
	uint32 crc = 0;
	unsigned char *buffer;

	/* Build the crc table */
	if(build_crc_table())
	{
		buffer = new byte[BUFFER_SIZE];
		if(buffer) 
		{
			crc= calculate_file_crc(buffer, BUFFER_SIZE, OFile);
			delete []buffer;
		}
		
		/* free the crc table! */
		free_crc_table();
	}

	return crc;
}

/* Calculate the crc for a file using the given buffer.. */
uint32 calculate_data_crc(
	unsigned char *buffer,
	long length)
{
	uint32 crc = 0;

	assert(buffer);
	
	/* Build the crc table */
	if(build_crc_table())
	{
		/* The odd permutions ensure that we get the same crc as for a file */
		crc = 0xFFFFFFFFL;
		crc = calculate_buffer_crc(length, crc, buffer);
		crc ^= 0xFFFFFFFFL;

		/* free the crc table! */
		free_crc_table();
	}

	return crc;
}

/* ---------------- Private Code --------------- */
static bool build_crc_table(
	void)
{
	bool success= false;

	assert(!crc_table);
	crc_table= new uint32[TABLE_SIZE];
	if(crc_table)
	{
		/* Build the table */
		short index, j;
		uint32 crc;

		for(index= 0; index<TABLE_SIZE; ++index)
		{
			crc= index;
			for(j=0; j<8; j++)
			{
				if(crc & 1) crc=(crc>>1) ^ CRC32_POLYNOMIAL;
				else crc>>=1;
			}
			crc_table[index] = crc;
		}
		
		success= true;
	}
	
	return success;
}

static void free_crc_table(
	void)
{
	assert(crc_table);
	delete []crc_table;
	crc_table= NULL;
}

/* Calculate for a block of data incrementally */
static uint32 calculate_buffer_crc(
	long count, 
	uint32 crc, 
	void *buffer)
{
	unsigned char *p;
	uint32 a;
	uint32 b;

	p= (unsigned char *) buffer;
	while (count--) 
	{
		a= (crc >> 8) & 0x00FFFFFFL;
		b= crc_table[((int) crc ^ *p++) & 0xff];
		crc= a^b;
	}
	return crc;
}

/* Calculate the crc for a file using the given buffer.. */
static uint32 calculate_file_crc(
	unsigned char *buffer, 
	short buffer_size,
	OpenedFile& OFile)
{
	uint32 crc;
	long count;
	long file_length, initial_position;
	
	/* Save and restore the initial file position */
	if (!OFile.GetPosition(initial_position))
		return 0;

	/* Get the file_length */
	if (!OFile.GetLength(file_length))
		return 0;
	
	/* Set to the start of the file */
	if (!OFile.SetPosition(0))
		return 0;

	crc = 0xFFFFFFFFL;
	while(file_length) 
	{
		if(file_length>buffer_size)
		{
			count= buffer_size;
		} else {
			count= file_length;
		}

		if (!OFile.Read(count, buffer))
			return 0;

		crc = calculate_buffer_crc(count, crc, buffer);
		file_length -= count;
	}
	
	/* Restore the file position */
	OFile.SetPosition(initial_position);

	return (crc ^= 0xFFFFFFFFL);
}
