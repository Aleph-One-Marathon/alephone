/*
	crc.c
	Sunday, March 5, 1995 6:21:30 PM

	CRC Checksum generation for a file.

Aug 15, 2000 (Loren Petrich):
	Using object-oriented file handler
*/

#include <stdlib.h>

#include "cseries.h"
#include "FileHandler_Mac.h"
// #include "portable_files.h"
#include "crc.h"

#ifdef env68k
	#pragma segment file_io
#endif

/* ---------- constants */
#define TABLE_SIZE (256)
#define CRC32_POLYNOMIAL 0xEDB88320L
#define BUFFER_SIZE 1024

/* ---------- local data */
static unsigned long *crc_table= NULL;

/* ---------- local prototypes ------- */
static unsigned long calculate_file_crc(unsigned char *buffer, 
	short buffer_size, OpenedFile& OFile);
//	short buffer_size, short refnum);
static unsigned long calculate_buffer_crc(long count, unsigned long crc, void *buffer);
static boolean build_crc_table(void);
static void free_crc_table(void);

/* -------------- Entry Point ----------- */
unsigned long calculate_crc_for_file(FileObject& File)
	// FileDesc *file) 
{
	short refnum;
	unsigned long crc;
	
	OpenedFile_Mac OFile;
	if (File.Open(OFile))
	{
		crc= calculate_crc_for_opened_file(OFile);
		OFile.Close();
	}
	
	/*
	refnum= open_file_for_reading(file);
	if(refnum!=NONE)
	{
		crc= calculate_crc_for_opened_file(refnum);
		close_file(refnum);
	}
	*/
	
	return crc;
}

unsigned long calculate_crc_for_opened_file(OpenedFile& OFile)
	// short refnum) 
{
	unsigned long crc;
	unsigned char *buffer;

	/* Build the crc table */
	if(build_crc_table())
	{
		buffer = new byte[BUFFER_SIZE];
		// buffer= (unsigned char *) malloc(BUFFER_SIZE*sizeof(unsigned char));
		if(buffer) 
		{
			// crc= calculate_file_crc(buffer, BUFFER_SIZE, refnum);
			crc= calculate_file_crc(buffer, BUFFER_SIZE, OFile);
			delete []buffer;
			// free(buffer);
		}
		
		/* free the crc table! */
		free_crc_table();
	}

	return crc;
}

/* Calculate the crc for a file using the given buffer.. */
unsigned long calculate_data_crc(
	unsigned char *buffer,
	long length)
{
	unsigned long crc= 0l;

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
static boolean build_crc_table(
	void)
{
	boolean success= FALSE;

	assert(!crc_table);
	crc_table= (unsigned long *) malloc(TABLE_SIZE*sizeof(unsigned long));
	if(crc_table)
	{
		/* Build the table */
		short index, j;
		unsigned long crc;

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
		
		success= TRUE;
	}
	
	return success;
}

static void free_crc_table(
	void)
{
	assert(crc_table);
	free(crc_table);
	crc_table= NULL;
}

/* Calculate for a block of data incrementally */
static unsigned long calculate_buffer_crc(
	long count, 
	unsigned long crc, 
	void *buffer)
{
	unsigned char *p;
	unsigned long a;
	unsigned long b;

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
static unsigned long calculate_file_crc(
	unsigned char *buffer, 
	short buffer_size,
	OpenedFile& OFile)
	// short refnum)
{
	unsigned long crc;
	long count;
	// FileError err;
	long file_length, initial_position;
	
	/* Save and restore the initial file position */
	assert(OFile.GetPosition(initial_position));
	// initial_position= get_fpos(refnum);

	/* Get the file_length */
	assert(OFile.GetLength(file_length));
	// file_length= get_file_length(refnum);
	
	/* Set to the start of the file */
	assert(OFile.SetPosition(0));
	// set_fpos(refnum, 0l);

	crc = 0xFFFFFFFFL;
	while(file_length) 
	{
		if(file_length>buffer_size)
		{
			count= buffer_size;
		} else {
			count= file_length;
		}

		assert(OFile.ReadObjectList(count,buffer));
		// err= read_file(refnum, count, buffer);
		// vassert(!err, csprintf(temporary, "Error: %d", err));
		
		crc = calculate_buffer_crc(count, crc, buffer);
		file_length -= count;
	}
	
	/* Restore the file position */
	assert(OFile.SetPosition(initial_position));
	// set_fpos(refnum, initial_position);

	return (crc ^= 0xFFFFFFFFL);
}
