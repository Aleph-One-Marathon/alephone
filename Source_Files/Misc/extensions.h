#ifndef __EXTENSIONS_H
#define __EXTENSIONS_H

/*

	extensions.h
	Tuesday, October 31, 1995 11:42:19 AM- rdm created.

Aug 12, 2000 (Loren Petrich):
	Using object-oriented file handler
*/

class FileSpecifier;

#define BUNGIE_PHYSICS_DATA_VERSION 0
#define PHYSICS_DATA_VERSION 1

// LP: don't need this stuff anymore
#if 0

#ifdef EXPORT_STRUCTURE
struct definition_data
{
	long tag;
	void *data;
	short count;
	short size;
};

static struct definition_data definitions[]=
{
	{MONSTER_PHYSICS_TAG, monster_definitions, NUMBER_OF_MONSTER_TYPES, sizeof(struct monster_definition)},
	{EFFECTS_PHYSICS_TAG, effect_definitions, NUMBER_OF_EFFECT_TYPES, sizeof(struct effect_definition)},
	{PROJECTILE_PHYSICS_TAG, projectile_definitions, NUMBER_OF_PROJECTILE_TYPES, sizeof(struct projectile_definition)},
	{PHYSICS_PHYSICS_TAG, physics_models, NUMBER_OF_PHYSICS_MODELS, sizeof(struct physics_constants)},
	{WEAPONS_PHYSICS_TAG, weapon_definitions, NUMBER_OF_WEAPONS, sizeof(struct weapon_definition)}
};
#define NUMBER_OF_DEFINITIONS (sizeof(definitions)/sizeof(definitions[0]))
#else
#ifdef IMPORT_STRUCTURE
struct definition_data 
{
	long tag;
	void *data;
};

static struct definition_data definitions[]=
{
	{MONSTER_PHYSICS_TAG, monster_definitions},
	{EFFECTS_PHYSICS_TAG, effect_definitions},
	{PROJECTILE_PHYSICS_TAG, projectile_definitions},
	{PHYSICS_PHYSICS_TAG, physics_models},
	{WEAPONS_PHYSICS_TAG, weapon_definitions},
};
#define NUMBER_OF_DEFINITIONS (sizeof(definitions)/sizeof(definitions[0]))
#endif
#endif

#endif

/* ------------- prototypes */

/* Set the physics file to read from.. */
void set_physics_file(FileSpecifier& File);

void set_to_default_physics_file(void);

/* Proceses the entire physics file.. */
void import_definition_structures(void);

void *get_network_physics_buffer(long *physics_length);
void process_network_physics_model(void *data);

#endif
