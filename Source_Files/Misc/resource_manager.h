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
class LoadedResource;

extern void initialize_resources(FileSpecifier &global_resources);

extern SDL_RWops *open_res_file(FileSpecifier &file);
extern void close_res_file(SDL_RWops *file);
extern SDL_RWops *cur_res_file(void);
extern void use_res_file(SDL_RWops *file);

extern int count_1_resources(uint32 type);
extern int count_resources(uint32 type);

extern void get_1_resource_id_list(uint32 type, vector<int> &ids);
extern void get_resource_id_list(uint32 type, vector<int> &ids);

extern bool get_1_resource(uint32 type, int id, LoadedResource &rsrc);
extern bool get_resource(uint32 type, int id, LoadedResource &rsrc);

extern bool get_1_ind_resource(uint32 type, int index, LoadedResource &rsrc);
extern bool get_ind_resource(uint32 type, int index, LoadedResource &rsrc);

extern bool has_1_resource(uint32 type, int id);
extern bool has_resource(uint32 type, int id);

#endif
