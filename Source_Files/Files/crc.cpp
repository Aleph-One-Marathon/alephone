/*
	crc.c

	Copyright (C) 1991-2001 and beyond by Bungie Studios, Inc.
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

/* ---------- constants */
#define TABLE_SIZE (256)
#define CRC32_POLYNOMIAL 0xEDB88320L
#define BUFFER_SIZE 1024

/* ---------- local data */
static uint32 *crc_table= NULL;

/* ---------- local prototypes ------- */
static uint32 calculate_file_crc(unsigned char *buffer, 
	short buffer_size, OpenedFile& OFile);
static uint32 calculate_buffer_crc(int32 count, uint32 crc, void *buffer);
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
	int32 length)
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
	int32 count, 
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
	int32 count;
	int32 file_length, initial_position;
	
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

/*  crcccitt.c - a demonstration of look up table based CRC
 *               computation using the non-reversed CCITT_CRC
 *               polynomial 0x1021 (truncated)
 *
 *  Copyright (C) 2000  Jack Klein
 *                      Macmillan Computer Publishing
 *  Adapted for Aleph One, 2006, by Gregory Smith
 *  License: GPL 2 or newer (as above)
 * 
 *  Jack Klein may be contacted by email at:
 *     The_C_Guru@yahoo.com
 *
 *  ghs: this is from http://home.att.net/~jackklein/C_Unleashed/crcccitt.c
 */

static uint16 crc_ccitt_table [256] =
{
  0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5,
  0x60c6, 0x70e7, 0x8108, 0x9129, 0xa14a, 0xb16b,
  0xc18c, 0xd1ad, 0xe1ce, 0xf1ef, 0x1231, 0x0210,
  0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
  0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c,
  0xf3ff, 0xe3de, 0x2462, 0x3443, 0x0420, 0x1401,
  0x64e6, 0x74c7, 0x44a4, 0x5485, 0xa56a, 0xb54b,
  0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
  0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6,
  0x5695, 0x46b4, 0xb75b, 0xa77a, 0x9719, 0x8738,
  0xf7df, 0xe7fe, 0xd79d, 0xc7bc, 0x48c4, 0x58e5,
  0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
  0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969,
  0xa90a, 0xb92b, 0x5af5, 0x4ad4, 0x7ab7, 0x6a96,
  0x1a71, 0x0a50, 0x3a33, 0x2a12, 0xdbfd, 0xcbdc,
  0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
  0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03,
  0x0c60, 0x1c41, 0xedae, 0xfd8f, 0xcdec, 0xddcd,
  0xad2a, 0xbd0b, 0x8d68, 0x9d49, 0x7e97, 0x6eb6,
  0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
  0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a,
  0x9f59, 0x8f78, 0x9188, 0x81a9, 0xb1ca, 0xa1eb,
  0xd10c, 0xc12d, 0xf14e, 0xe16f, 0x1080, 0x00a1,
  0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
  0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c,
  0xe37f, 0xf35e, 0x02b1, 0x1290, 0x22f3, 0x32d2,
  0x4235, 0x5214, 0x6277, 0x7256, 0xb5ea, 0xa5cb,
  0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
  0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447,
  0x5424, 0x4405, 0xa7db, 0xb7fa, 0x8799, 0x97b8,
  0xe75f, 0xf77e, 0xc71d, 0xd73c, 0x26d3, 0x36f2,
  0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
  0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9,
  0xb98a, 0xa9ab, 0x5844, 0x4865, 0x7806, 0x6827,
  0x18c0, 0x08e1, 0x3882, 0x28a3, 0xcb7d, 0xdb5c,
  0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
  0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0,
  0x2ab3, 0x3a92, 0xfd2e, 0xed0f, 0xdd6c, 0xcd4d,
  0xbdaa, 0xad8b, 0x9de8, 0x8dc9, 0x7c26, 0x6c07,
  0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
  0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba,
  0x8fd9, 0x9ff8, 0x6e17, 0x7e36, 0x4e55, 0x5e74,
  0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
};

uint16 calculate_data_crc_ccitt(unsigned char *data, int32 length)
{
  int32 count;
  uint32 crc = 0xffff;
  uint32 temp;

  for (count = 0; count < length; ++count)
  {
    temp = (*data++ ^ (crc >> 8)) & 0xff;
    crc = crc_ccitt_table[temp] ^ (crc << 8);
  }
  return (uint16)(crc ^ 0);
}
