/*
 *  resource_manager.h - MacOS resource handling for non-Mac platforms
 *
 *  Written in 2000 by Christian Bauer
 */

#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <stdio.h>
#include <vector>

class FileSpecifier;

extern void initialize_resources(FileSpecifier &global_resources);

extern SDL_RWops *OpenResFile(FileSpecifier &file);
extern void CloseResFile(SDL_RWops *file);
extern SDL_RWops *CurResFile(void);
extern void UseResFile(SDL_RWops *file);

extern int Count1Resources(uint32 type);
extern int CountResources(uint32 type);

extern void Get1ResourceIDList(uint32 type, vector<int> &ids);
extern void GetResourceIDList(uint32 type, vector<int> &ids);

extern void *Get1Resource(uint32 type, int id, uint32 *size_ret = NULL);
extern void *GetResource(uint32 type, int id, uint32 *size_ret = NULL);

extern void *Get1IndResource(uint32 type, int index, uint32 *size_ret = NULL);
extern void *GetIndResource(uint32 type, int index, uint32 *size_ret = NULL);

extern bool Has1Resource(uint32 type, int id);
extern bool HasResource(uint32 type, int id);

#endif
