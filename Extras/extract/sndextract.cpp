/*
NORESNAMES.C
Saturday, July 3, 1993 8:19:20 AM
*/

#include "macintosh_cseries.h"
#include "byte_swapping.h"

#include <string.h>

#include "::world.h"
#include "::mysound.h"

#define STATIC_DEFINITIONS
#include "::sound_definitions.h"

/* ---------- macros */

#define ALIGN_LONG(x) ((((x)-1)&0xfffffffc)+4)
					
/* ---------- private code */

static void build_sounds_file(char *destination_filename, short source_count, char **source_filenames);
static void extract_sound_resources(FILE *stream, long *definition_offset, short base_resource,
	struct sound_definition *original_definitions, struct sound_definition *working_definitions);

/* ---------- code */

void main(
	int argc,
	char **argv)
{
	if (argc<=2)
	{
		fprintf(stderr, "Usage: %s <source> <destination>\n", argv[0]);
		exit(1);
	}
	else
	{
		build_sounds_file(argv[1], argc-2, argv+2);
	}
	
	exit(0);
}

/* ---------- private code */

void build_sounds_file(
	char *destination_filename,
	short source_count,
	char **source_filenames)
{
	FILE *stream= fopen(destination_filename, "wb");
	
	if (stream)
	{
		struct sound_definition *original_definitions, *working_definitions;
		long definition_offset= sizeof(struct sound_file_header);
		short source;
		
		original_definitions= malloc(NUMBER_OF_SOUND_DEFINITIONS*sizeof(struct sound_definition));
		working_definitions= malloc(NUMBER_OF_SOUND_DEFINITIONS*sizeof(struct sound_definition));
		assert(original_definitions && working_definitions);
		
		// write the file header and enough room for each of the sound_definition arrays
		{
			struct sound_file_header header;
			short source_index;
			
			memset(&header, 0, sizeof(struct sound_file_header));
			header.version= SOUND_FILE_VERSION;
			header.tag= SOUND_FILE_TAG;
			header.source_count= source_count;
			header.sound_count= NUMBER_OF_SOUND_DEFINITIONS;
			
			fwrite(&header, sizeof(struct sound_file_header), 1, stream);
			
			for (source_index= 0; source_index<NUMBER_OF_SOUND_SOURCES; ++source_index)
			{
				fwrite(sound_definitions, sizeof(struct sound_definition), NUMBER_OF_SOUND_DEFINITIONS, stream);
			}
		}
		
		for (source= 0; source<source_count; ++source)
		{
			short reference_number;
			Str255 filename;
	
			strcpy(filename, source_filenames[source]);
			c2pstr(filename);
			
			reference_number= OpenResFile(filename);
			if (reference_number!=NONE)
			{
				if (!source)
				{
					memcpy(original_definitions, sound_definitions, NUMBER_OF_SOUND_DEFINITIONS*sizeof(struct sound_definition));
					extract_sound_resources(stream, &definition_offset, 0, (struct sound_definition *) NULL, original_definitions);
				}
				else
				{
					memcpy(working_definitions, sound_definitions, NUMBER_OF_SOUND_DEFINITIONS*sizeof(struct sound_definition));
					extract_sound_resources(stream, &definition_offset, 10000, original_definitions, working_definitions);
				}
				
				CloseResFile(reference_number);
			}
			else
			{
				fprintf(stderr, "Error #%d opening '%s'.\n", ResError(), source_filenames[source]);
				exit(1);
			}
		}
	
		free(original_definitions);
		free(working_definitions);
	
		fclose(stream);
	}
	else
	{
		fprintf(stderr, "Error #%d opening '%s'.\n", -1, destination_filename);
		exit(1);
	}
}

static void extract_sound_resources(
	FILE *stream,
	long *definition_offset,
	short base_resource,
	struct sound_definition *original_definitions,
	struct sound_definition *working_definitions)
{
	short i;

	fseek(stream, 0, SEEK_END);

	for (i= 0; i<NUMBER_OF_SOUND_DEFINITIONS; ++i)
	{
		struct sound_definition *definition= working_definitions + i;
		
		if (definition->sound_code==NONE)
		{
			definition->permutations= 0;
		}
		else
		{
			short permutations;
			
			definition->group_offset= ftell(stream);
			definition->total_length= 0;

			for (permutations= 0; permutations<MAXIMUM_PERMUTATIONS_PER_SOUND; ++permutations)
			{
				Handle sound_handle= GetResource('snd ', definition->sound_code + permutations + base_resource);
				
				if (sound_handle)
				{
					ExtSoundHeaderPtr extended_sound_header;
					SoundHeaderPtr sound_header;
					long size;
					
					HLock(sound_handle);
					
					{
						long offset;
	
						GetSoundHeaderOffset((SndListHandle)sound_handle, &offset);
						sound_header= (SoundHeaderPtr) (*sound_handle+offset);
						extended_sound_header= (ExtSoundHeaderPtr) sound_header;
					}

					switch (sound_header->encode)
					{
						case stdSH:
							size= sizeof(SoundHeader) + sound_header->length;
							break;
						
						case extSH:
							size= sizeof(ExtSoundHeader) + extended_sound_header->numFrames*(extended_sound_header->sampleSize>>3);
							break;
						
						default: halt();
					}
					
					size= ALIGN_LONG(size);
					
					definition->sound_offsets[permutations]= definition->total_length;
					fwrite(sound_header, size, 1, stream);
					definition->total_length+= size;
					if (!permutations) definition->single_length= definition->total_length;
					
					ReleaseResource(sound_handle);
				}
				else
				{
					break;
				}
			}
			
			if (!(definition->permutations= permutations))
			{
				if (!original_definitions)
				{
					definition->sound_code= NONE;
				}
				else
				{
					// if this is an alternate source copy missing sounds from the primary source
					*definition= original_definitions[i];
				}
			}
		}
	}

	fseek(stream, *definition_offset, SEEK_SET);
	fwrite(working_definitions, sizeof(struct sound_definition), NUMBER_OF_SOUND_DEFINITIONS, stream);
	*definition_offset+= NUMBER_OF_SOUND_DEFINITIONS*sizeof(struct sound_definition);
	
	return;
}
