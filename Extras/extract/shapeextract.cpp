/*
SHAPEEXTRACT.C
Tuesday, August 29, 1995 9:54:17 AM  (Jason)
*/

#include "macintosh_cseries.h"

#include <string.h>

#include "::shape_descriptors.h"
#include "::shape_definitions.h"

/* ---------- macros */

/* ---------- private code */

static void extract_shape_resources(FILE *stream);
static void add_resource(short id, long *offset, long *length, FILE *stream);

/* ---------- code */

void main(
	int argc,
	char **argv)
{
	Str255 filename;
	short reference_number;

	if (argc<=2)
	{
		fprintf(stderr, "Usage: %s <source> <destination>\n", argv[0]);
		exit(1);
	}
	else
	{
		strcpy(filename, argv[1]);
		c2pstr(filename);
		reference_number= OpenResFile(filename);
		if (reference_number!=NONE)
		{
			FILE *stream= fopen(argv[2], "wb");
			
			if (stream)
			{
				extract_shape_resources(stream);
				fclose(stream);
			}
			else
			{
				fprintf(stderr, "Error #%d opening %s.\n", -1, argv[2]);
				exit(1);
			}
			
			CloseResFile(reference_number);
		}
		else
		{
			fprintf(stderr, "Error #%d opening %s.\n", ResError(), argv[1]);
			exit(1);
		}
	}
	
	exit(0);
}

/* ---------- private code */

static void extract_shape_resources(
	FILE *stream)
{
	short i;

	fwrite(collection_headers, sizeof(struct collection_header), MAXIMUM_COLLECTIONS, stream);
	
	for (i= 0; i<MAXIMUM_COLLECTIONS; ++i)
	{
		struct collection_header *header= collection_headers + i;

		add_resource(128+i, &header->offset, &header->length, stream);
		add_resource(1128+i, &header->offset16, &header->length16, stream);
	}

	fseek(stream, 0, SEEK_SET);
	fwrite(collection_headers, sizeof(struct collection_header), MAXIMUM_COLLECTIONS, stream);
	
	return;
}

static void add_resource(
	short id,
	long *offset,
	long *length,
	FILE *stream)
{
	Handle collection= GetResource('.256', id);
	
	if (collection)
	{
		HLock(collection);
		*offset= ftell(stream), *length= GetHandleSize(collection);
		fwrite(*collection, *length, 1, stream);

		ReleaseResource(collection);
	}
	else
	{
		*offset= -1, *length= 0;
	}

	return;
}
