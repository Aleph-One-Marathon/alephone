/*
	network_modem.c

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

	Friday, September 29, 1995 10:07:55 PM- rdm created.

*/

#include <string.h>

#include "macintosh_cseries.h"
#include <Gestalt.h>
#include <CommResources.h>
#include <CTBUtilities.h>
#include <Connections.h>
#include <Traps.h>

#include "macintosh_network.h"
#include "network_modem.h"

#ifdef env68k
	#pragma segment network_modem
#endif

enum {
	strDEFAULT_TOOL_NAMES= 4000,
	appleModemTool= 0
};

enum {
	dlogAWAITING_ANSWER= 30000
};

enum {
	_read_buffer,
	_write_buffer,
	NUMBER_OF_ASYNCHRONOUS_BUFFERS
};

#define kBufferSize 1024
#define kReadTimeout (-1)
#define kWriteTimeout (-1)
#define kCloseTimeout (600) // 10 seconds

static ConnHandle connection_handle= NULL;
static bool listen_completed;
static CMErr listen_error;
static Str255 stored_tool_name;
static byte *asynchronous_buffers[NUMBER_OF_ASYNCHRONOUS_BUFFERS];

/* -------- local prototypes */
static bool communications_toolbox_present(void);
static ConnHandle instantiate_connection_record(void);
static pascal void listen_completion_routine(ConnHandle connection);
static bool establish_connection(bool calling);
static void save_connection_preferences(void);
static void load_connection_tool_name_from_preferences(unsigned char *name);
static void load_connection_preferences(void);

/* -------- code */
OSErr initialize_modem_endpoint(
	void)
{
	OSErr error;
	static bool first_time= true;
	
	assert(communications_toolbox_present());
	if(first_time)
	{
		error= InitCTBUtilities();
		if(!error)
		{
			error= InitCRM();
			if(!error)
			{
				error= InitCM();
			}
		}
	} 
	else 
	{
		error= noErr;
	}
	
	return error;
}

OSErr teardown_modem_endpoint(
	void)
{
	CMErr error= noErr;

	if(connection_handle)
	{
		CMBufferSizes sizes;
		CMStatFlags flags;
		short index;

		error= CMStatus(connection_handle, sizes, &flags);
		vwarn(!error, csprintf(temporary, "CMStatus error: %d", error));
		if(flags & (cmStatusOpen | cmStatusOpening))
		{
			error= CMAbort(connection_handle);
			vwarn(!error, csprintf(temporary, "CMAbort error: %d", error));
			error= CMClose(connection_handle, false, NULL, kCloseTimeout, true);
			vwarn(!error, csprintf(temporary, "Bullshit CMClose error: %d Flags: %x;g", error, flags));
			if(error==4) error= noErr;
		}
		CMDispose(connection_handle);
		connection_handle= NULL;
		
		for(index= 0; index<NUMBER_OF_ASYNCHRONOUS_BUFFERS; ++index)
		{
			DisposePtr((Ptr) asynchronous_buffers[index]);
		}
	}
	
	return error;
}

bool call_modem_player(
	void)
{
	bool success= false;

	connection_handle= instantiate_connection_record();
	if(connection_handle)
	{
		success= establish_connection(true);
	} else {
		// outOfMemory
		alert_user(fatalError, strNETWORK_ERRORS, netErrCantContinue, MemError());
	}

	return success;
}

bool answer_modem_player(
	void)
{
	bool success= false;

	connection_handle= instantiate_connection_record();
	if(connection_handle)
	{
		success= establish_connection(false);
	} else {
		// outOfMemory
		alert_user(fatalError, strNETWORK_ERRORS, netErrCantContinue, MemError());
	}

	return success;
}

/* Ideally, packet data has only a checksum, and stream data knows how to ask */
/*  that the data be resent. */
OSErr write_modem_endpoint(
	void *data,
	uint16 length,
	long timeout)
{
	long data_written;
	CMErr err;
	uint16 data_size= length;

	assert(connection_handle);
	
	data_written= length;
	err= CMWrite(connection_handle, data, &data_written,
		cmData, false, NULL, timeout, 0);
	vassert(!err && data_written==length,
		csprintf(temporary, "Err: %d Data: %d length: %d", err, data_written, length));

	return err;
}

OSErr read_modem_endpoint(
	void *data,
	uint16 length,
	long timeout)
{
	long data_read;
	CMErr err;

	assert(connection_handle);
	
	data_read= length;
	err= CMRead(connection_handle, data, &data_read,
		cmData, false, NULL, timeout, 0);
	vassert(!err && data_read==length,
		csprintf(temporary, "Err: %d Data read: %d requested: %d", 
			err, data_read, length));

	return err;
}

void write_modem_endpoint_asynchronous(
	void *data,
	uint16 length,
	long timeout)
{
	long data_written;
	CMErr err;
	uint16 data_size= length;

	assert(connection_handle);

	assert(length<kBufferSize);
	memcpy(asynchronous_buffers[_write_buffer], data, length);
	data_written= length;
	err= CMWrite(connection_handle, asynchronous_buffers[_write_buffer], 
		&data_written, cmData, true, NULL, timeout, 0);
	assert(!err);
}

bool asynchronous_write_completed(
	void)
{
	CMBufferSizes sizes;
	CMStatFlags flags;
	CMErr error;
	bool complete= true;

	error= CMStatus(connection_handle, sizes, &flags);
	assert(!error);
	
	if(flags & cmStatusDWPend)
	{
		complete= false;
	}
	
	return complete;
}

void idle_modem_endpoint(
	void)
{
	if(connection_handle)
	{
		CMIdle(connection_handle);
	}
}

bool modem_available(
	void)
{
	return communications_toolbox_present();
}

long modem_read_bytes_available(
	void)
{
	CMErr error;
	CMBufferSizes sizes;
	CMStatFlags flags;

	assert(connection_handle);
	error= CMStatus(connection_handle, sizes, &flags);
	assert(sizes[cmDataIn]&&(flags&cmStatusDataAvail) || !sizes[cmDataIn]);
	
	return sizes[cmDataIn];
}
	
/* ------------ local code */
static bool communications_toolbox_present(
	void)
{
	bool installed= true;

	if(NGetTrapAddress(_Unimplemented, OSTrap)==NGetTrapAddress(_CommToolboxDispatch, OSTrap))
	{
		installed= false;
	}
	
	return installed;
}

static ConnHandle instantiate_connection_record(
	void)
{
	short procID;
	ConnHandle connection;
	Str255 tool_name;

	load_connection_tool_name_from_preferences(tool_name);
	procID= CMGetProcID(tool_name);
	if(procID==NONE)
	{
		OSErr err;
		
		err= CRMGetIndToolName(classCM, 1, ptemporary);
		if(!err) 
		{
			procID= CMGetProcID(ptemporary);
		}
	}
	
	if(procID!=NONE) 
	{
		CMBufferSizes buff_sizes;
		short index;

		memset(buff_sizes, 0, sizeof(CMBufferSizes));
		buff_sizes[cmDataIn]= kBufferSize;
		buff_sizes[cmDataOut]= kBufferSize;

		/* refcon/userdata */
		connection= CMNew(procID, cmNoMenus|cmQuiet|cmData, buff_sizes, 0l, 0l);
		assert(connection);

		/* Lock and load.. */
		MoveHHi((Handle) connection);
		HLock((Handle) connection);

		/* Load the preferences. */
		load_connection_preferences();

		/* Create the asynchronous buffers */
		for(index= 0; index<NUMBER_OF_ASYNCHRONOUS_BUFFERS; ++index)
		{
			asynchronous_buffers[index]= (byte *) NewPtr(kBufferSize);
			assert(asynchronous_buffers[index]);
		}
	}

	return connection;
}

static pascal void listen_completion_routine(
	ConnHandle connection)
{
	listen_error= (*connection)->errCode;
	listen_completed= true;
}

#define MODEM_OPEN_TIMEOUT (40*60)
#define MODEM_LISTEN_TIMEOUT (120*60)

static bool establish_connection(
	bool calling)
{
	Point where={40, 50};
	short item_hit;
	bool success= false;

	item_hit= CMChoose(&connection_handle, where, NULL);
	if(item_hit==chooseOKMinor || item_hit==chooseOKMajor)
	{
		bool cancelled= false;
		ConnectionCompletionUPP completion_proc;
		DialogPtr dialog;
		short item_hit;

		/* Lock and load (Again, in case the CMChoose changed it....) */
		MoveHHi((Handle) connection_handle);
		HLock((Handle) connection_handle);

		/* Save the preferences. */	
		save_connection_preferences();

		/* Save off the name of the tool they are using.. */
		save_connection_preferences();
				
		completion_proc= NewConnectionCompletionProc(listen_completion_routine);
		assert(completion_proc);
				
		/* Open the listening dialog. */
		listen_completed= false;
				
		dialog= GetNewDialog(dlogAWAITING_ANSWER, NULL, (WindowPtr) -1l);
		assert(dialog);
				
		/* Start listening.. */
		if(calling)
		{
			listen_error= CMOpen(connection_handle, true, completion_proc, MODEM_OPEN_TIMEOUT);
		} else {
			listen_error= CMListen(connection_handle, true, completion_proc, MODEM_LISTEN_TIMEOUT);
			if(listen_error==cmNotSupported)
			{
				/* Serial connections can't listen, so we have to CMOpen it.. */
				listen_error= CMOpen(connection_handle, true, completion_proc, MODEM_OPEN_TIMEOUT);
			}
		}
		
		while(!listen_completed && !cancelled && !listen_error)
		{
			ModalDialog(NULL, &item_hit);
		
			/* This needs to go into the general idle proc.. */
			idle_modem_endpoint();
			if(item_hit==iCANCEL) cancelled= true;
			
			if(!calling)
			{
				CMBufferSizes sizes;
				CMStatFlags flags;
			
				assert(connection_handle);
				listen_error= CMStatus(connection_handle, sizes, &flags);
dprintf("CMStatus returned: %d", listen_error);
				if(flags & cmStatusIncomingCallPresent)
				{	
					bool accept_call= false;
				
					/* Should we accept this call */
					accept_call= true;
dprintf("Incoming call present!");
					listen_error= CMAccept(connection_handle, accept_call);
dprintf("CMAccept returned: %d", listen_error);
					listen_completed= true;
				}
			}
		}
		
		/* Get rid of the dialog.. */
		DisposeDialog(dialog);
		
		/* Check for errors... */
		if(listen_error==cmNoErr && !cancelled)
		{
			success= true;
			
			/* And save the preferences... */
		} else {
			if(!cancelled)
			{
				/* Display the error! */
				alert_user(fatalError, strNETWORK_ERRORS, netErrCantContinue, listen_error);
			}
		}
		
		DisposeRoutineDescriptor(completion_proc);
	}

	return success;
}

static void save_connection_preferences(
	void)
{
	Ptr data;

	CMGetToolName((*connection_handle)->procID, stored_tool_name);
	data= CMGetConfig(connection_handle);
	assert(data);
	
	/* Save the connection data.. */
	
	DisposePtr(data);
}

static void load_connection_tool_name_from_preferences(
	unsigned char *name)
{
	pstrcpy(name, "\pSerial Tool");
}

static void load_connection_preferences(
	void)
{
	Ptr configuration= NULL;
	
	/* Load the tool name.. */
	
	if(configuration)
	{
		/* Setup the configuration from stored values */
		CMSetConfig(connection_handle, configuration);
		CMValidate(connection_handle);
	}
}