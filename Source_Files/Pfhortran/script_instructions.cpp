/* script_instructions.c
	2/16/00 Created - Chris Pruett
	
	Aug 10, 2000 (Loren Petrich): renamed dynamic_fog_depth to FogDepth
	
	10/20/00 - Mark Levin
	Functions created for monster support

	Apr 27, 2001 (Loren Petrich):
		Modified access to OpenGL color data; Pfhortran now affects only the above-liquid color,
		though below-liquid color should be easy to control with appropriate modifications
*/

 

#ifdef __MVCPP__

#include <windows.h>
#include "sdl_cseries.h"			// for type 'Rect'
#include "screen.h"					// for change_screen_mode()

#endif  
  
#include <math.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_OPENGL
#include <GL/gl.h>
#else
typedef float GLfloat;
#endif

#include "cseries.h"
#include "tags.h"
#include "map.h"
#include "interface.h"
#include "game_wad.h"
#include "game_errors.h"
#include "player.h"
#include "render.h"
#include "platforms.h"
#include "lightsource.h"
#include "items.h"
#include "game_window.h"
#include "weapons.h"
#include "screen.h"
#include "shell.h"
#include "monsters.h"
#include "mysound.h"
#include "OGL_Render.h"

//#include "Soundtrack.h"

//we need to be able to access monster definitions

#include "monster_definitions.h"

#include "flood_map.h"

#include "script_parser.h"
#include "script_instructions.h"

#define sign(a) (a) < 0 ? -1 : 1


/*extern'd references to the rest of the mara code */
 
extern struct view_data *world_view;
extern bool ready_weapon(short player_index, short weapon_index);
extern void draw_panels(void);
extern struct monster_data *get_monster_data(short monster_index);


// ML addition: to use possible_intersecting_monsters
static vector<short> IntersectedObjects;
//extern struct monster_definition monster_definitions[NUMBER_OF_MONSTER_TYPES];
//trying to access the monster type list


/*extern'd references to the script code (to avoid circular includes) */
extern  void free_script(void);
extern	void set_instruction_decay(long);
extern 	void jump_to_line(int);
extern 	void add_variable(int);
extern 	float get_variable(int);
extern 	void set_variable(int, float);
extern void stack_push(int val);
extern int stack_pop(void);
extern bool script_in_use(void);
extern bool do_next_instruction(void);
extern int get_next_instruction(void);

extern bool is_startup;
extern short camera_count;
extern short current_camera;
extern script_camera *cameras;
extern int current_instruction;
extern short current_trap;
extern short path_count;
extern short current_path;
extern path_header *script_paths;
extern short current_path_point;
extern path_list camera_point;




world_point3d offset_position;
//for camera timing
//int lastTicks;		//the tickCount at which we last updated the path
short offset_polygon_index;
float offset_yaw;
float offset_pitch;
short offset_count;
short roll_count;

//float current_yaw;
//float current_pitch;

bool s_camera_Control;
short old_size = _640_320_HUD;
void (*instruction_lookup[NUMBER_OF_INSTRUCTIONS])(script_instruction);


/* function prototypes */

void s_Camera_Move(script_instruction inst);
void s_Camera_Look(script_instruction inst);

void s_Wait_Ticks(script_instruction inst);
void s_Inflict_Dammage(script_instruction inst);
void s_Jump(script_instruction inst);
void s_Enable_Player(script_instruction inst);
void s_Disable_Player(script_instruction inst);

void s_Script_End(script_instruction inst);

void s_Hide_Interface(script_instruction inst);
void s_Show_Interface(script_instruction inst);

void s_Set_Tag_State(script_instruction inst);
void s_Get_Tag_State(script_instruction inst);

void s_Define(script_instruction inst);
void s_sAdd(script_instruction inst);
void s_sSubtract(script_instruction inst);
void s_If_Equal(script_instruction inst);
void s_Set(script_instruction inst);
void s_Call(script_instruction inst);
void s_Return(script_instruction inst);
void s_If_Greater(script_instruction inst);
void s_If_Less(script_instruction inst);
void s_If_Not_Equal(script_instruction inst);

void s_Get_Life(script_instruction inst);
void s_Set_Life(script_instruction inst);
void s_Get_Oxygen(script_instruction inst);
void s_Set_Oxygen(script_instruction inst);

void s_End(script_instruction inst);
void s_Get_My_Value(script_instruction inst);

void s_Add_Item(script_instruction inst);
void s_Select_Weapon(script_instruction inst);

void s_Block_Start(script_instruction inst);
void s_Block_End(script_instruction inst);

void s_Init_Cameras(script_instruction inst);
void s_Select_Camera(script_instruction inst);
void s_Set_Camera_Poly(script_instruction inst);
void s_Set_Camera_Pos(script_instruction inst);
void s_Set_Camera_YP(script_instruction inst);
void s_Activate_Camera(script_instruction inst);
void s_Use_Camera(script_instruction inst);

void s_Init_Paths(script_instruction inst);
void s_New_Path(script_instruction inst);
void s_Dispose_Path(script_instruction inst);
void s_Select_Path(script_instruction inst);
void s_Set_Path_Move_Speed(script_instruction inst);
void s_Select_Point(script_instruction inst);
void s_Set_Point_Poly(script_instruction inst);
void s_Set_Point_Pos(script_instruction inst);
void s_Set_Point_YP(script_instruction inst);
void s_Start_Camera_On_Path(script_instruction inst);
void s_Set_Path_Roll_Speed(script_instruction inst);
void s_Wait_For_Path(script_instruction inst);

void s_Change_Soundtrack(script_instruction inst);
void s_Set_Fog_Depth(script_instruction inst);
void s_Set_Fog_Color(script_instruction inst);
void s_Get_Fog_Depth(script_instruction inst);
void s_Get_Fog_Color(script_instruction inst);


void s_Teleport_Player(script_instruction inst);

void s_Monster_New(script_instruction inst);
void s_Monster_Sleep(script_instruction inst);
void s_Monster_Hurt(script_instruction inst);
void s_Monster_Attack(script_instruction inst);
void s_Monster_Move(script_instruction inst);
void s_Monster_Select(script_instruction inst);
void s_Monster_Get_Immunity(script_instruction inst);
void s_Monster_Set_Immunity(script_instruction inst);
void s_Monster_Get_Weakness(script_instruction inst);
void s_Monster_Set_Weakness(script_instruction inst);
void s_Monster_Get_Friend(script_instruction inst);
void s_Monster_Set_Friend(script_instruction inst);
void s_Monster_Get_Enemy(script_instruction inst);
void s_Monster_Set_Enemy(script_instruction inst);
void s_Monster_Get_Item(script_instruction inst);
void s_Monster_Set_Item(script_instruction inst);
void s_Monster_Get_Nuke(script_instruction inst);
void s_Monster_Set_Nuke(script_instruction inst);

/*-------------------------------------------*/

//Steal the monster_pathfinding_data structure?

struct monster_pathfinding_data
{
	struct monster_definition *definition;
	struct monster_data *monster;
	
	bool cross_zone_boundaries;
};

//steal this mb


//Steal the monster functions

extern void advance_monster_path(short monster_index);
extern long monster_pathfinding_cost_function(short source_polygon_index, short line_index,
	short destination_polygon_index, void *data);
extern void set_monster_action(short monster_index, short action);
extern void set_monster_mode(short monster_index, short new_mode, short target_index);


/*init_instructions sets up the instruction_lookup array so that
functions can be called by casting back the pointer.  This is only
called once.*/
void init_instructions(void)
{

	//dprintf("pfhortran inited\n");

	instruction_lookup[Camera_Move] = s_Camera_Move;
	instruction_lookup[Camera_Look] = s_Camera_Look;
	instruction_lookup[Wait_Ticks] = s_Wait_Ticks;
	instruction_lookup[Inflict_Dammage] = s_Inflict_Dammage;
	instruction_lookup[Jump] = s_Jump;
	instruction_lookup[Enable_Player] = s_Enable_Player;
	instruction_lookup[Disable_Player] = s_Disable_Player;
	instruction_lookup[Script_End] = s_Script_End;
	instruction_lookup[Hide_Interface] = s_Hide_Interface;
	instruction_lookup[Show_Interface] = s_Show_Interface;
	instruction_lookup[Set_Tag_State] = s_Set_Tag_State;
	instruction_lookup[Get_Tag_State] = s_Get_Tag_State;
	instruction_lookup[Define] = s_Define;
	instruction_lookup[sAdd] = s_sAdd;
	instruction_lookup[sSubtract] = s_sSubtract;
	instruction_lookup[If_Equal] = s_If_Equal;
	instruction_lookup[Set] = s_Set;
	instruction_lookup[Call] = s_Call;
	instruction_lookup[Return] = s_Return;
	instruction_lookup[If_Greater] = s_If_Greater;
	instruction_lookup[If_Less] = s_If_Less;
	instruction_lookup[If_Not_Equal] = s_If_Not_Equal;
	instruction_lookup[Get_Life] = s_Get_Life;
	instruction_lookup[Set_Life] = s_Set_Life;
	instruction_lookup[Get_Oxygen] = s_Get_Oxygen;
	instruction_lookup[Set_Oxygen] = s_Set_Oxygen;
	instruction_lookup[End] = s_End;
	instruction_lookup[Get_My_Value] = s_Get_My_Value;
	instruction_lookup[Add_Item] = s_Add_Item;
	instruction_lookup[Select_Weapon] = s_Select_Weapon;
	instruction_lookup[Block_Start] = s_Block_Start;
	instruction_lookup[Block_End] = s_Block_End;
	instruction_lookup[Init_Cameras] = s_Init_Cameras;
	instruction_lookup[Select_Camera] = s_Select_Camera;
	instruction_lookup[Set_Camera_Poly] = s_Set_Camera_Poly;
	instruction_lookup[Set_Camera_Pos] = s_Set_Camera_Pos;
	instruction_lookup[Set_Camera_YP] = s_Set_Camera_YP;
	instruction_lookup[Activate_Camera] = s_Activate_Camera;
	instruction_lookup[Use_Camera] = s_Use_Camera;
	instruction_lookup[Init_Paths] = s_Init_Paths;
	instruction_lookup[New_Path] = s_New_Path;
	instruction_lookup[Dispose_Path] = s_Dispose_Path;
	instruction_lookup[Select_Path] = s_Select_Path;
	instruction_lookup[Set_Path_Move_Speed] = s_Set_Path_Move_Speed;
	instruction_lookup[Select_Point] = s_Select_Point;
	instruction_lookup[Set_Point_Poly] = s_Set_Point_Poly;
	instruction_lookup[Set_Point_Pos] = s_Set_Point_Pos;
	instruction_lookup[Set_Point_YP] = s_Set_Point_YP;
	instruction_lookup[Start_Camera_On_Path] = s_Start_Camera_On_Path;
	instruction_lookup[Set_Path_Roll_Speed] = s_Set_Path_Roll_Speed;
	instruction_lookup[Wait_For_Path] = s_Wait_For_Path;
	
	//Matthew Hielscher's additions ----
	instruction_lookup[Change_Soundtrack] = s_Change_Soundtrack;
	instruction_lookup[Set_Fog_Depth] = s_Set_Fog_Depth;
	instruction_lookup[Set_Fog_Color] = s_Set_Fog_Color;
	instruction_lookup[Get_Fog_Depth] = s_Get_Fog_Depth;
	instruction_lookup[Get_Fog_Color] = s_Get_Fog_Color;
	// ----
	
	instruction_lookup[Teleport_Player] = s_Teleport_Player;

	//monster funcs added by Mark Levin
	
	instruction_lookup[Monster_Hurt] = s_Monster_Hurt;
	instruction_lookup[Monster_New] = s_Monster_New;
	instruction_lookup[Monster_Move] = s_Monster_Move;
	instruction_lookup[Monster_Sleep] = s_Monster_Sleep;
	instruction_lookup[Monster_Attack] = s_Monster_Attack;
	instruction_lookup[Monster_Select] = s_Monster_Select;
	instruction_lookup[Monster_Get_Immunity] = s_Monster_Get_Immunity;
	instruction_lookup[Monster_Set_Immunity] = s_Monster_Set_Immunity;
	instruction_lookup[Monster_Get_Weakness] = s_Monster_Get_Weakness;
	instruction_lookup[Monster_Set_Weakness] = s_Monster_Set_Weakness;
	instruction_lookup[Monster_Get_Friend] = s_Monster_Get_Friend;
	instruction_lookup[Monster_Set_Friend] = s_Monster_Set_Friend;
	instruction_lookup[Monster_Get_Enemy] = s_Monster_Get_Enemy;
	instruction_lookup[Monster_Set_Enemy] = s_Monster_Set_Enemy;
	instruction_lookup[Monster_Get_Item] = s_Monster_Get_Item;
	instruction_lookup[Monster_Set_Item] = s_Monster_Set_Item;
	instruction_lookup[Monster_Get_Nuke] = s_Monster_Get_Nuke;
	instruction_lookup[Monster_Set_Nuke] = s_Monster_Set_Nuke;



	}

#pragma mark -

/* ----------------------------------------- */

void update_path_camera(void)
{
	path_point old_point;
	const float AngleConvert = 360/float(FULL_CIRCLE);
	int currentTicks;
	
	/* copy over old location info so we can calculate new polygon index later */
	
	old_point.yaw = camera_point.location.yaw;
	old_point.pitch = camera_point.location.pitch;
	old_point.position.x = camera_point.location.position.x;
	old_point.position.y = camera_point.location.position.y;
	old_point.position.z = camera_point.location.position.z;
	old_point.polygon_index = camera_point.location.polygon_index;
	
	//we are now at this time
//	currentTicks = machine_tick_count();	//this should be from 1 to many ticks after lastTicks
	
	if (offset_count)
	{
		if (camera_point.location.position.x < 
			script_paths[cameras[current_camera].path].the_path[cameras[current_camera].point].location.position.x)
			camera_point.location.position.x += offset_position.x;

		if (camera_point.location.position.y < 
			script_paths[cameras[current_camera].path].the_path[cameras[current_camera].point].location.position.y)
			camera_point.location.position.y += offset_position.y;

		if (camera_point.location.position.z < 
			script_paths[cameras[current_camera].path].the_path[cameras[current_camera].point].location.position.z)
			camera_point.location.position.z += offset_position.z;
			

		if (camera_point.location.position.x >
			script_paths[cameras[current_camera].path].the_path[cameras[current_camera].point].location.position.x)
			camera_point.location.position.x -= offset_position.x;

		if (camera_point.location.position.y > 
			script_paths[cameras[current_camera].path].the_path[cameras[current_camera].point].location.position.y)
			camera_point.location.position.y -= offset_position.y;

		if (camera_point.location.position.z > 
			script_paths[cameras[current_camera].path].the_path[cameras[current_camera].point].location.position.z)
			camera_point.location.position.z -= offset_position.z;
	}

	if (roll_count)
	{
		short check_yaw, new_yaw;
		
		check_yaw = camera_point.location.yaw;
		new_yaw = script_paths[cameras[current_camera].path].the_path[cameras[current_camera].point].location.yaw;
		
		/*if (check_yaw > HALF_CIRCLE)
			check_yaw -= FULL_CIRCLE;
			
		if (new_yaw > HALF_CIRCLE)
			new_yaw -= FULL_CIRCLE;*/
			
		
		if (check_yaw < new_yaw && new_yaw - check_yaw <= HALF_CIRCLE)
			camera_point.location.yaw = normalize_angle(camera_point.location.yaw  + (offset_yaw * sign(check_yaw)));
		else if (new_yaw - check_yaw > HALF_CIRCLE)
			camera_point.location.yaw = normalize_angle(camera_point.location.yaw  - (offset_yaw * sign(check_yaw)));

		if (check_yaw > new_yaw && check_yaw - new_yaw <= HALF_CIRCLE)
			camera_point.location.yaw = normalize_angle(camera_point.location.yaw  - (offset_yaw * sign(check_yaw)));
		else if (check_yaw - new_yaw > HALF_CIRCLE)
			camera_point.location.yaw = normalize_angle(camera_point.location.yaw  + (offset_yaw * sign(check_yaw)));

		
		
		
		/*if (abs(check_yaw) + abs(new_yaw) > (180 - abs(check_yaw)) + (180 - abs(new_yaw)))
			camera_point.location.yaw = normalize_angle((abs(check_yaw) + (180 - abs(check_yaw)) + (180 - abs(new_yaw)) / roll_count));
		else
			camera_point.location.yaw = normalize_angle((abs(check_yaw) + abs(new_yaw))/ roll_count);*/

		if (normalize_angle(camera_point.location.pitch) < normalize_angle(script_paths[cameras[current_camera].path].the_path[cameras[current_camera].point].location.pitch))
		{
			camera_point.location.pitch = normalize_angle(angle(camera_point.location.pitch + offset_pitch));
		}

		if (normalize_angle(camera_point.location.pitch) > normalize_angle(script_paths[cameras[current_camera].path].the_path[cameras[current_camera].point].location.pitch))
		{
			camera_point.location.pitch = normalize_angle(angle(camera_point.location.pitch - offset_pitch));
		}
	}
	
	/*if (roll_count)
	{
		if (camera_point.location.yaw < script_paths[cameras[current_camera].path].the_path[cameras[current_camera].point].location.yaw)
		{
			camera_point.location.yaw = normalize_angle(((float)(camera_point.location.yaw * AngleConvert) - offset_yaw) / AngleConvert);
		}

		if (camera_point.location.yaw > script_paths[cameras[current_camera].path].the_path[cameras[current_camera].point].location.yaw)
		{
			camera_point.location.yaw = normalize_angle(((float)(camera_point.location.yaw * AngleConvert) + offset_yaw) / AngleConvert);
		}

		if (camera_point.location.pitch < script_paths[cameras[current_camera].path].the_path[cameras[current_camera].point].location.pitch)
		{
			camera_point.location.pitch = normalize_angle(((float)(camera_point.location.pitch * AngleConvert) - offset_pitch) / AngleConvert);
		}

		if (camera_point.location.pitch > script_paths[cameras[current_camera].path].the_path[cameras[current_camera].point].location.pitch)
		{
			camera_point.location.pitch = normalize_angle(((float)(camera_point.location.pitch * AngleConvert) + offset_pitch) / AngleConvert);
		}
	}*/
	
	
		
	/* Loren Petrich's code... thanks Loren! */
	
	short CurrentPolygonIndex;
	short NextPolygonIndex;
	
	// Set CurrentPosition to the old position of the camera
	// Set TargetPosition to the new position of the camera
	// Set CurrentPolygonIndex to the old polygon membership of the camera

/*	world_point2d CurrentPosition, TargetPosition;

	TargetPosition.x = camera_point.location.position.x;
	TargetPosition.y = camera_point.location.position.y;
	CurrentPosition.x = old_point.position.x;
	CurrentPosition.y = old_point.position.y; */
	
	CurrentPolygonIndex = camera_point.location.polygon_index;

	while(true)
	{
		// Which line got crossed in order to get to the target position
		/*short LineIndex = find_line_crossed_leaving_polygon(
			CurrentPolygonIndex,
			&TargetPosition, 
			&CurrentPosition);*/
			
			
		short LineIndex = find_line_crossed_leaving_polygon(
			CurrentPolygonIndex,
			(world_point2d *)&camera_point.location.position, 
			(world_point2d *)&old_point.position);
			
		if (LineIndex != NONE)
		{
			// Crossed a line, so must move to the next polygon,
			// if there is one on the other side.
			// But first, we must update the current position to
			// where it hit the line
			struct line_data *Line= get_line_data(LineIndex);
			world_point3d Intersection;
			find_line_intersection(
				&get_endpoint_data(Line->endpoint_indexes[0])->vertex,
				&get_endpoint_data(Line->endpoint_indexes[1])->vertex,
				&camera_point.location.position,
				&old_point.position,
				&Intersection);
			
			NextPolygonIndex = find_adjacent_polygon(
				CurrentPolygonIndex,
				LineIndex);
			if (NextPolygonIndex != NONE)
			{
				// Successfully entered another polygon
				CurrentPolygonIndex = NextPolygonIndex;
			}
			else
			{
				// Entered the void through a wall;
				// won't change the membership
				break;
			}
		}
		else
			break; // Intersected with no line, so it's quitting time
	}

	// Most of these functions are, I think, in map.c/h		
	
	if (CurrentPolygonIndex != camera_point.location.polygon_index)
		camera_point.location.polygon_index = CurrentPolygonIndex;
	else 
	{
		NextPolygonIndex = world_point_to_polygon_index((world_point2d *)&camera_point.location.position);
		if ( NextPolygonIndex != -1)
			camera_point.location.polygon_index = NextPolygonIndex;
		else
			camera_point.location.polygon_index = CurrentPolygonIndex;

	}
}


bool script_Camera_Active(void)
{
	const float AngleConvert = 360/float(FULL_CIRCLE);
	
	
	if (!script_in_use())
		return false;

	if (cameras)
	{
		if (current_camera == 0)
		{
			//ChaseCam_SetActive(script_FALSE);
			world_view->show_weapons_in_hand = script_TRUE;
			s_camera_Control = false;
		}
		else
		{
			//ChaseCam_SetActive(script_TRUE);
			world_view->show_weapons_in_hand = script_FALSE;
			s_camera_Control = true;
			
			if (cameras[current_camera].path == -1)
			{
				world_view->origin_polygon_index= cameras[current_camera].location.polygon_index;
				world_view->yaw = cameras[current_camera].location.yaw;
				world_view->pitch = cameras[current_camera].location.pitch;
				world_view->origin = cameras[current_camera].location.position;
			} else
			{
				/* if (camera_point.location.yaw == script_paths[cameras[current_camera].path].the_path[cameras[current_camera].point].location.yaw
					&& camera_point.location.pitch == script_paths[cameras[current_camera].path].the_path[cameras[current_camera].point].location.pitch
					&& camera_point.location.position.x == script_paths[cameras[current_camera].path].the_path[cameras[current_camera].point].location.position.x
					&& camera_point.location.position.y == script_paths[cameras[current_camera].path].the_path[cameras[current_camera].point].location.position.y
					&& camera_point.location.position.z == script_paths[cameras[current_camera].path].the_path[cameras[current_camera].point].location.position.z)
			
				*/
				
				if (offset_count == 0 && roll_count == 0) /* path end point reached */
					current_path_point = cameras[current_camera].point;
					
				if (current_path_point == cameras[current_camera].point)
				{
					cameras[current_camera].point++;
					if (cameras[current_camera].point > script_paths[cameras[current_camera].path].length) /*path is finished */
					{
						current_path_point = 0;
						cameras[current_camera].point = 0;		//step back to end
						cameras[current_camera].path = -1;
						current_camera = 0;
						s_camera_Control = false;
						return s_camera_Control;
					}
					
					/* figure out values for camera_point*/
					
					if (cameras[current_camera].point == 1)
					{
						camera_point.location.polygon_index= script_paths[cameras[current_camera].path].the_path[current_path_point].location.polygon_index;
						camera_point.location.yaw= script_paths[cameras[current_camera].path].the_path[current_path_point].location.yaw;
						camera_point.location.pitch= script_paths[cameras[current_camera].path].the_path[current_path_point].location.pitch;
						camera_point.location.position= script_paths[cameras[current_camera].path].the_path[current_path_point].location.position;
					}
				
					offset_count = script_paths[cameras[current_camera].path].move_speed;
					roll_count = script_paths[cameras[current_camera].path].roll_speed;
					
							
					offset_yaw = abs(camera_point.location.yaw - 
						script_paths[cameras[current_camera].path].the_path[cameras[current_camera].point].location.yaw);
						
					if (offset_yaw / roll_count < 1)
						offset_yaw *= offset_yaw;
						
					offset_yaw /= roll_count;
					
								
					offset_pitch = abs(camera_point.location.pitch - 
						script_paths[cameras[current_camera].path].the_path[cameras[current_camera].point].location.pitch);
					
					if (offset_pitch / roll_count < 1)
						offset_pitch *= offset_pitch;
						
					offset_pitch /= roll_count;
					
					
					
						
					offset_position.x = (abs(
						camera_point.location.position.x - script_paths[cameras[current_camera].path].the_path[cameras[current_camera].point].location.position.x)
						/ offset_count);
					
					offset_position.y = (abs(
						camera_point.location.position.y - script_paths[cameras[current_camera].path].the_path[cameras[current_camera].point].location.position.y)
						/ offset_count);
					
					offset_position.z = (abs(
						camera_point.location.position.z - script_paths[cameras[current_camera].path].the_path[cameras[current_camera].point].location.position.z)
						/ offset_count);
	
				
										
				}
				
				
				update_path_camera();
				
						
				if (offset_count > 0)
					offset_count--;
					
					
				if (roll_count > 0)
					roll_count--;
					
					
				world_view->origin_polygon_index= camera_point.location.polygon_index;
				world_view->yaw = camera_point.location.yaw;
				world_view->pitch = camera_point.location.pitch;
				world_view->origin = camera_point.location.position;
			}
			
		}
	} else
		s_camera_Control = false;

	
	return s_camera_Control;
}

void s_Camera_Move(script_instruction inst)
{
 
}
 
void s_Camera_Look(script_instruction inst)
{
 
}

void s_Wait_Ticks(script_instruction inst)
{
 
 	float temp;
	
	temp = inst.op1;
	
	if (is_startup)
		return;
	

	if (inst.mode == 1)
	{
		temp = get_variable(int(inst.op1));
	}
		
		
	set_instruction_decay(uint32(machine_tick_count() + temp));
	//set_instruction_decay(uint32(dynamic_world->tick_count + temp));
}

void s_Inflict_Dammage(script_instruction inst)
{
	if (PLAYER_IS_DEAD(current_player) || PLAYER_IS_TOTALLY_DEAD(current_player))
		return;
		
	
	
	
		
	struct damage_definition damage;
	
	float temp;
	
	temp = inst.op1;
	
	
	if (inst.mode == 1)
	{
		temp = get_variable(int(inst.op1));
	}
				

	damage.flags= _alien_damage;
	damage.type= _damage_crushing;
	damage.base= int16(temp);
	damage.random= 0;
	damage.scale= FIXED_ONE;

	damage_player(current_player->monster_index, NONE, NONE, &damage);
}


void s_Jump(script_instruction inst)
{
	float temp;
	
	temp = inst.op1;
	
	
	if (inst.mode == 1)
	{
		temp = get_variable(int(inst.op1));
	}
		
		
	jump_to_line((int)floor(temp));

}


void s_Enable_Player(script_instruction inst)
{
	SET_PLAYER_ZOMBIE_STATUS(current_player,script_FALSE);
}

void s_Disable_Player(script_instruction inst)
{
	if (PLAYER_IS_DEAD(current_player) || PLAYER_IS_TOTALLY_DEAD(current_player))
		return;
		
	SET_PLAYER_ZOMBIE_STATUS(current_player,script_TRUE);
}

void s_Script_End(script_instruction inst)
{
	free_script();
}

void s_Hide_Interface(script_instruction inst)
{
	screen_mode_data *the_mode;
	the_mode = get_screen_mode();
	old_size = the_mode->size;
	// LP: Modifying Mark Levin's patch to be more general
	short new_size = GetSizeWithoutHUD(old_size);
	if(the_mode->size != new_size)
	{
		the_mode->size = new_size;
		change_screen_mode(the_mode,true);
	}
	/*
	if(the_mode->size != _full_screen)
	{
		the_mode->size = _full_screen;
		change_screen_mode(the_mode,true);
	}
	*/
}

void s_Show_Interface(script_instruction inst)
{
	screen_mode_data *the_mode;
	the_mode = get_screen_mode();
	// LP: Modifying Mark Levin's patch to be more general
	short the_size = the_mode->size;
	if(the_size == GetSizeWithoutHUD(the_size))
	{
		the_mode->size = old_size;
		change_screen_mode(the_mode,true);
		draw_panels();
	}
	/*
	if(the_mode->size == _full_screen)
	{
		the_mode->size = old_size;
		change_screen_mode(the_mode,true);
		draw_panels();
	}
	*/
}

void s_Set_Tag_State(script_instruction inst)
{
	float temp,temp2;
	
	temp = inst.op1;
	temp2 = inst.op2;
	
	if (inst.mode != 0)
		switch(inst.mode)
		{
			case 1:
				temp = get_variable(int(inst.op1));
				break;
			case 2:
				temp2 = get_variable(int(inst.op2));
				break;
			case 3:
				temp = get_variable(int(inst.op1));
				temp2 = get_variable(int(inst.op2));
				break;
		}
		
		
	if (set_tagged_light_statuses(int16(temp), temp2));
	if (try_and_change_tagged_platform_states(int16(temp), temp2)); 
	
	assume_correct_switch_position(_panel_is_tag_switch, int16(temp), temp2);
	
}

void s_Get_Tag_State(script_instruction inst)
{

	short light_index;
	struct light_data *light;
	struct platform_data *platform;
	short platform_index;
	
	bool changed= false;
	float temp,temp2;
	int tag;
	
	
	temp = inst.op1;
	temp2 = inst.op2;
	
	if (inst.mode != 0)
		switch(inst.mode)
		{
			
			case 2:
				temp2 = get_variable(int(inst.op2));
				break;
			case 3:
				temp = get_variable(int(inst.op1));
				temp2 = get_variable(int(inst.op2));
				break;
		}
		
		
		
	tag = (int)floor(temp);
	
	
	for (light_index= 0, light= lights; light_index<MAXIMUM_LIGHTS_PER_MAP; ++light_index, ++light)
	{
		if (light->static_data.tag==tag)
		{
			if (get_light_status(light_index))
			{
				changed= true;
			}
		}
	}
	
	
	if (!changed)
	{
		for (platform_index= 0, platform= platforms; platform_index<dynamic_world->platform_count; ++platform_index, ++platform)
		{
			if (platform->tag==tag)
			{
				if (PLATFORM_IS_ACTIVE(platform))
				{
					changed= true;
				}
			}
		}
	
	}
	
	if (changed)
		set_variable(int(inst.op2), 1);
	else
		set_variable(int(inst.op2), 0);
}

void s_Define(script_instruction inst)
{
	add_variable(int(inst.op1));
}

void s_sAdd(script_instruction inst)
{
	float temp;
	
	if (inst.mode == 0)
		return;
	
	switch(inst.mode)
	{
		case 1:
				temp = get_variable(int(inst.op1));
				set_variable(int(inst.op1),temp+inst.op2);
				break;
		case 3:
				temp = get_variable(int(inst.op1));
				set_variable(int(inst.op1),temp+get_variable(int(inst.op2)));
				break;
		default:
				break;
	}
 	 
	 
}

void s_sSubtract(script_instruction inst)
{
	float temp;
	
	if (inst.mode == 0)
		return;
	
	switch(inst.mode)
	{
		case 1:
				temp = get_variable(int(inst.op1));
				set_variable(int(inst.op1),temp-inst.op2);
				break;
		case 3:
				temp = get_variable(int(inst.op1));
				set_variable(int(inst.op1),temp-get_variable(int(inst.op2)));
				break;
		default:
				break;
	}
 	 
	 
}

void s_If_Equal(script_instruction inst)
{
	float temp;
	
	if (inst.mode > 3)
		temp = get_variable(int(inst.op3));
	else
		temp = inst.op3;
		
	switch(inst.mode)
	{
		case 0:
		case 4:
//				dprintf("%f = %f? if so, go to %f\n", inst.op1, inst.op2,temp);
			if (inst.op1 == inst.op2)
				{
				jump_to_line((int)floor(temp));
				}
			break;
		case 1:
		case 5:
//				dprintf("%f = %f? if so, go to %f\n", get_variable(int(inst.op1)), inst.op2,temp);
			if (get_variable(int(inst.op1)) == inst.op2)
				{
				jump_to_line((int)floor(temp));
				}
			break;
		case 2:
		case 6:
//				dprintf("%f = %f? if so, go to %f\n", inst.op1,get_variable(int(inst.op2)), temp);
			if (get_variable(int(inst.op2)) == inst.op1)
				{
				jump_to_line((int)floor(temp));
				}
			break;
		case 3:
		case 7:
//				dprintf("%f = %f? if so, go to %f\n", get_variable(int(inst.op1)), get_variable(int(inst.op2)),temp);
			if (get_variable(int(inst.op1)) == get_variable(int(inst.op2)))
				{
				jump_to_line((int)floor(temp));
				}
			break;
		default:
				break;
	}
	 
}

void s_Set(script_instruction inst)
{
	float temp;
	
	if (inst.mode == 0)
		return;
	
	switch(inst.mode)
	{
		case 1:
				temp = get_variable(int(inst.op1));
				set_variable(int(inst.op1),inst.op2);
				break;
		case 3:
				temp = get_variable(int(inst.op1));
				set_variable(int(inst.op1),get_variable(int(inst.op2)));
				break;
		default:
				break;
	}
 	 
	 
}


void s_Call(script_instruction inst)
{
	stack_push(current_instruction);
	s_Jump(inst);

}


void s_Return(script_instruction inst)
{
	int new_offset;
	
	new_offset = stack_pop();
	if (new_offset != -1)
		current_instruction = new_offset;
}


void s_If_Greater(script_instruction inst)
{
	float temp;
	
	if (inst.mode > 3)
		temp = get_variable(int(inst.op3));
	else
		temp = inst.op3;
		
	switch(inst.mode)
	{
		case 0:
		case 4:
			if (inst.op1 > inst.op2)
				jump_to_line((int)floor(temp));
			break;
		case 1:
		case 5:
			if (get_variable(int(inst.op1)) > inst.op2)
				jump_to_line((int)floor(temp));
			
			break;
		case 2:
		case 6:
			if (get_variable(int(inst.op2)) > inst.op1)
				jump_to_line((int)floor(temp));
			
			break;
		case 3:
		case 7:
			if (get_variable(int(inst.op1)) > get_variable(int(inst.op2)))
				jump_to_line((int)floor(temp));
			break;
		default:
				break;
	}
 	 
	 
}

void s_If_Less(script_instruction inst)
{
	float temp;
	
	if (inst.mode > 3)
		temp = get_variable(int(inst.op3));
	else
		temp = inst.op3;
		
	switch(inst.mode)
	{
		case 0:
		case 4:
			if (inst.op1 < inst.op2)
				jump_to_line((int)floor(temp));
			break;
		case 1:
		case 5:
			if (get_variable(int(inst.op1)) < inst.op2)
				jump_to_line((int)floor(temp));
			
			break;
		case 2:
		case 6:
			if (get_variable(int(inst.op2)) < inst.op1)
				jump_to_line((int)floor(temp));
			
			break;
		case 3:
		case 7:
			if (get_variable(int(inst.op1)) < get_variable(int(inst.op2)))
				jump_to_line((int)floor(temp));
			break;
		default:
				break;
	}
 	 
	 
}

void s_If_Not_Equal(script_instruction inst)
{
	float temp;
	
	if (inst.mode > 3)
		temp = get_variable(int(inst.op3));
	else
		temp = inst.op3;
		
	switch(inst.mode)
	{
		case 0:
		case 4:
			if (inst.op1 != inst.op2)
				jump_to_line((int)floor(temp));
			break;
		case 1:
		case 5:
			if (get_variable(int(inst.op1)) != inst.op2)
				jump_to_line((int)floor(temp));
			
			break;
		case 2:
		case 6:
			if (get_variable(int(inst.op2)) != inst.op1)
				jump_to_line((int)floor(temp));
			
			break;
		case 3:
		case 7:
			if (get_variable(int(inst.op1)) != get_variable(int(inst.op2)))
				jump_to_line((int)floor(temp));
			break;
		default:
				break;
	}
 	 
	 
}

void s_Get_Life(script_instruction inst)
{		
	if (inst.mode == 0)
		return;
		
	set_variable(int(inst.op1),current_player->suit_energy);

}

void s_Set_Life(script_instruction inst)
{
		
	switch(inst.mode)
	{
		case 0:
				current_player->suit_energy = (int)floor(inst.op1);
				break;
		case 1:
				current_player->suit_energy = (int)floor(get_variable(int(inst.op1)));
				break;
		default:
				break;
	}
	
	mark_shield_display_as_dirty();
	

}

void s_Get_Oxygen(script_instruction inst)
{
	if (inst.mode == 0)
		return;
		
	set_variable(int(inst.op1),current_player->suit_oxygen);

}

void s_Set_Oxygen(script_instruction inst)
{
	switch(inst.mode)
	{
		case 0:
				current_player->suit_oxygen = (int)floor(inst.op1);
				break;
		case 1:
				current_player->suit_oxygen = (int)floor(get_variable(int(inst.op1)));
				break;
		default:
				break;
	}
	
	mark_oxygen_display_as_dirty();

}


void s_End(script_instruction inst)
{
	reset_trap(current_trap);
}

void s_Get_My_Value(script_instruction inst)
{
	if (inst.mode == 0)
		return;
		
	set_variable(int(inst.op1),get_trap_value(current_trap));
}

void s_Add_Item(script_instruction inst)
{
	switch(inst.mode)
	{
		case 0:
				if (!try_and_add_player_item(player_identifier_to_player_index(current_player->identifier), (int)floor(inst.op1)))
					; /* this sucks */
				break;
		case 1:
				if (!try_and_add_player_item(player_identifier_to_player_index(current_player->identifier), (int)floor(get_variable(int(inst.op1)))))
					; /* this sucks */
				break;
		default:
				break;
	}
	
	

}

void s_Select_Weapon(script_instruction inst)
{

	int temp,weapon_index;
	
	switch(inst.mode)
	{
		case 0:
				temp= (int)floor(inst.op1);
				break;
		case 1:
				temp= (int)floor(get_variable(int(inst.op1)));
				break;
		default:
				temp= 0;
				break;
	}
	
	switch(temp)
	{
		case _i_knife:
			weapon_index = _weapon_fist;
			break;
		case _i_magnum:
			weapon_index = _weapon_pistol;
			break;
		case _i_plasma_pistol:
			weapon_index = _weapon_plasma_pistol;
			break;
		case _i_assault_rifle:
			weapon_index = _weapon_assault_rifle;
			break;
		case _i_missile_launcher:
			weapon_index = _weapon_missile_launcher;
			break;
		case _i_alien_shotgun:
			weapon_index = _weapon_alien_shotgun;
			break;
		case _i_flamethrower:
			weapon_index = _weapon_flamethrower;
			break;
		case _i_shotgun:
			weapon_index = _weapon_shotgun;
			break;
		case _i_smg:
			weapon_index = _weapon_smg;
			break;
		default:
			weapon_index = _weapon_fist;
			break;
	}	
	
	if (!ready_weapon(player_identifier_to_player_index(current_player->identifier), weapon_index))
		; /* this sucks */
		
	

}


void s_Block_Start(script_instruction inst)
{
	
	set_trap_instruction(current_trap, current_instruction);
	
	is_startup = true;
	
	bool success = true;
	do
		success = do_next_instruction();
	while (is_startup && script_in_use() && success);
}

void s_Block_End(script_instruction inst)
{
	is_startup = false;
}


void s_Init_Cameras(script_instruction inst)
{
	int temp;
	
	switch(inst.mode)
	{
		case 0:
				temp= (int)floor(inst.op1);
				break;
		case 1:
				temp= (int)floor(get_variable(int(inst.op1)));
				break;
		default:
				temp= 0;
				break;
	}
	
	if (cameras || camera_count > 0)
		return;
		
	camera_count = 0;
	current_camera = 0;
	
	cameras = (script_camera *)malloc(sizeof(script_camera) * (temp+1));
	
	memset(cameras,0,sizeof(script_camera) * (temp+1));
	
	cameras[0].location.polygon_index = world_view->origin_polygon_index;
	cameras[0].location.yaw = world_view->yaw;
	cameras[0].location.pitch = world_view->pitch;
	cameras[0].location.position = world_view->origin;
	cameras[0].index = 0;
	cameras[0].path = -1;
	cameras[0].point = 0;
	
}

void s_Select_Camera(script_instruction inst)
{
	int temp;
	
	switch(inst.mode)
	{
		case 0:
				temp= (int)floor(inst.op1);
				break;
		case 1:
				temp= (int)floor(get_variable(int(inst.op1)));
				break;
		default:
				temp= 0;
				break;
	}
	
	current_camera = temp;
	
	if (cameras)
	{
		cameras[current_camera].path = -1;
		
		if (cameras[current_camera].index == 0 && temp != 0)
		{
			camera_count++;
			cameras[current_camera].index = camera_count;
			//cameras[current_camera].path = -1;
		}
	}
	
}

void s_Set_Camera_Poly(script_instruction inst)
{
	int temp;
	
	switch(inst.mode)
	{
		case 0:
				temp= (int)floor(inst.op1);
				break;
		case 1:
				temp= (int)floor(get_variable(int(inst.op1)));
				break;
		default:
				temp= 0;
				break;
	}
	
	if (cameras)
		cameras[current_camera].location.polygon_index = temp;
}

void s_Set_Camera_Pos(script_instruction inst)
{
	float x,y,z;
		
	x= inst.op1;
	y= inst.op2;
	z= inst.op3;
				
	switch(inst.mode)
	{
		case 1:
				x= get_variable(int(inst.op1));
				break;
		case 2:
				y= get_variable(int(inst.op2));
				break;
		case 3:
				x= get_variable(int(inst.op1));
				y= get_variable(int(inst.op2));
				break;
		case 4:
				z= get_variable(int(inst.op3));
				break;
		case 5:
				x= get_variable(int(inst.op1));
				z= get_variable(int(inst.op3));
				break;
		case 6:
				y= get_variable(int(inst.op2));
				z= get_variable(int(inst.op3));
				break;
		case 7:
				x= get_variable(int(inst.op1));
				y= get_variable(int(inst.op2));
				z= get_variable(int(inst.op3));
				break;
		default:
				break;
	}
	
	if (cameras)
	{
		cameras[current_camera].location.position.x = world_distance(x*WORLD_ONE);
		cameras[current_camera].location.position.y = world_distance(y*WORLD_ONE);
		cameras[current_camera].location.position.z = world_distance(z*WORLD_ONE);
	}
	
}

void s_Set_Camera_YP(script_instruction inst)
{
	float temp,temp2;
	
	const float AngleConvert = 360/float(FULL_CIRCLE);
	
	temp = inst.op1;
	temp2 = inst.op2;
	
	if (inst.mode != 0)
		switch(inst.mode)
		{
			case 1:
				temp = get_variable(int(inst.op1));
				break;
			case 2:
				temp2 = get_variable(int(inst.op2));
				break;
			case 3:
				temp = get_variable(int(inst.op1));
				temp2 = get_variable(int(inst.op2));
				break;
		}
		
	if (cameras)
	{
		cameras[current_camera].location.yaw = normalize_angle(angle(temp/AngleConvert));
		
		cameras[current_camera].location.pitch = normalize_angle(angle(temp2/AngleConvert));
	}
}

void s_Activate_Camera(script_instruction inst)
{
	s_camera_Control = true;
}

void s_Use_Camera(script_instruction inst)
{
	s_Select_Camera(inst);
	s_Activate_Camera(inst);
	
}


void s_Init_Paths(script_instruction inst)
{
	int temp;
	
	switch(inst.mode)
	{
		case 0:
				temp= (int)floor(inst.op1);
				break;
		case 1:
				temp= (int)floor(get_variable(int(inst.op1)));
				break;
		default:
				temp= 0;
				break;
	}
	
	if (script_paths || path_count > 0)
		return;
		
	path_count = temp;
	current_path = -1;
	current_path_point = 0;
	
	script_paths = (path_header *)malloc(sizeof(path_header) * (temp));
	
	memset(script_paths,0,sizeof(path_header) * (temp));
	
	for (int x = 0; x < temp; x++)
	{	
		script_paths[x].the_path = NULL;
	}
	
}

void s_New_Path(script_instruction inst)
{
	int path_num, path_length;
	
	switch(inst.mode)
	{
		case 0:
				path_num= (int)floor(inst.op1);
				path_length= (int)floor(inst.op2);
				break;
		case 1:
				path_num= (int)floor(get_variable(int(inst.op1)));
				path_length= (int)floor(inst.op2);
				break;
		case 2:
				path_num= (int)floor(inst.op1);
				path_length= (int)floor(get_variable(int(inst.op2)));
				break;
		case 3:
				path_num= (int)floor(get_variable(int(inst.op1)));
				path_length= (int)floor(get_variable(int(inst.op2)));
				break;
		default:
				path_num = path_length = 0;
				break;
	}
	
	
	if (!script_paths || path_count <= 0)
		return;
	
	if (script_paths[path_num-1].the_path)
		return;

	if (path_num > path_count)
		return;
		
	script_paths[path_num-1].the_path = (path_list *)malloc(sizeof(path_list) * (path_length));
	
	script_paths[path_num-1].length = path_length-1;
	
	memset(script_paths[path_num-1].the_path,0,sizeof(path_list) * (path_length));
	
}

void s_Dispose_Path(script_instruction inst)
{
	int temp;
	
	switch(inst.mode)
	{
		case 0:
				temp= (int)floor(inst.op1);
				break;
		case 1:
				temp= (int)floor(get_variable(int(inst.op1)));
				break;
		default:
				temp= 0;
				break;
	}
	
	if (script_paths[temp-1].the_path)
	{
		free(script_paths[temp-1].the_path);
		script_paths[temp-1].the_path = NULL;
	}
	
}


void s_Select_Path(script_instruction inst)
{
	int temp;
	
	switch(inst.mode)
	{
		case 0:
				temp= (int)floor(inst.op1);
				break;
		case 1:
				temp= (int)floor(get_variable(int(inst.op1)));
				break;
		default:
				temp= 0;
				break;
	}
	
	current_path = temp-1;
	
}

void s_Set_Path_Move_Speed(script_instruction inst)
{
	float temp;
	
	switch(inst.mode)
	{
		case 0:
				temp= inst.op1;
				break;
		case 1:
				temp= get_variable(int(inst.op1));
				break;
		default:
				temp= 0.0;
				break;
	}
	
	if (script_paths)
		script_paths[current_path].move_speed = int16(temp);
}

void s_Set_Path_Roll_Speed(script_instruction inst)
{
	float temp;
	
	switch(inst.mode)
	{
		case 0:
				temp= inst.op1;
				break;
		case 1:
				temp= get_variable(int(inst.op1));
				break;
		default:
				temp= 0.0;
				break;
	}
	
	if (script_paths)
		script_paths[current_path].roll_speed = int16(temp);
}

void s_Select_Point(script_instruction inst)
{
	int temp;
	
	switch(inst.mode)
	{
		case 0:
				temp= (int)floor(inst.op1);
				break;
		case 1:
				temp= (int)floor(get_variable(int(inst.op1)));
				break;
		default:
				temp= 0;
				break;
	}
	
	current_path_point = temp;
	
}

void s_Set_Point_Poly(script_instruction inst)
{
	int temp;
	
	switch(inst.mode)
	{
		case 0:
				temp= (int)floor(inst.op1);
				break;
		case 1:
				temp= (int)floor(get_variable(int(inst.op1)));
				break;
		default:
				temp= 0;
				break;
	}
	
	if (script_paths)
		script_paths[current_path].the_path[current_path_point].location.polygon_index = temp;
}

void s_Set_Point_Pos(script_instruction inst)
{
	float x,y,z;
		
	x= inst.op1;
	y= inst.op2;
	z= inst.op3;
				
	switch(inst.mode)
	{
		case 1:
				x= get_variable(int(inst.op1));
				break;
		case 2:
				y= get_variable(int(inst.op2));
				break;
		case 3:
				x= get_variable(int(inst.op1));
				y= get_variable(int(inst.op2));
				break;
		case 4:
				z= get_variable(int(inst.op3));
				break;
		case 5:
				x= get_variable(int(inst.op1));
				z= get_variable(int(inst.op3));
				break;
		case 6:
				y= get_variable(int(inst.op2));
				z= get_variable(int(inst.op3));
				break;
		case 7:
				x= get_variable(int(inst.op1));
				y= get_variable(int(inst.op2));
				z= get_variable(int(inst.op3));
				break;
		default:
				break;
	}
	
	if (script_paths)
	{
		script_paths[current_path].the_path[current_path_point].location.position.x = world_distance(x*WORLD_ONE);
		script_paths[current_path].the_path[current_path_point].location.position.y = world_distance(y*WORLD_ONE);
		script_paths[current_path].the_path[current_path_point].location.position.z = world_distance(z*WORLD_ONE);
	}
	
}

void s_Set_Point_YP(script_instruction inst)
{
	float temp,temp2;
	
	const float AngleConvert = 360/float(FULL_CIRCLE);
	
	temp = inst.op1;
	temp2 = inst.op2;
	
	if (inst.mode != 0)
		switch(inst.mode)
		{
			case 1:
				temp = get_variable(int(inst.op1));
				break;
			case 2:
				temp2 = get_variable(int(inst.op2));
				break;
			case 3:
				temp = get_variable(int(inst.op1));
				temp2 = get_variable(int(inst.op2));
				break;
		}
		
	if (script_paths)
	{
		script_paths[current_path].the_path[current_path_point].location.yaw = normalize_angle(angle(temp/AngleConvert));
		
		script_paths[current_path].the_path[current_path_point].location.pitch = normalize_angle(angle(temp2/AngleConvert));
	}
}


void s_Start_Camera_On_Path(script_instruction inst)
{
	int temp;
	
	switch(inst.mode)
	{
		case 0:
				temp= (int)floor(inst.op1);
				break;
		case 1:
				temp= (int)floor(get_variable(int(inst.op1)));
				break;
		default:
				temp= 0;
				break;
	}
	
	if (cameras && script_paths)
	{
		cameras[current_camera].path = temp-1;
		cameras[current_camera].point = 0;
		
		//try some timing
		
	/*	cameras[current_camera].location.position = script_paths[cameras[current_camera].path].the_path[0].location.position;
		cameras[current_camera].location.polygon_index = script_paths[cameras[current_camera].path].the_path[0].location.polygon_index;
		cameras[current_camera].location.yaw = script_paths[cameras[current_camera].path].the_path[0].location.yaw;
		cameras[current_camera].location.pitch = script_paths[cameras[current_camera].path].the_path[0].location.pitch;
	*/
		current_path_point = 0;
		
//		lastTicks = machine_tick_count();	//set this as the start point
		
		s_Activate_Camera(inst);
	}
}

void s_Change_Soundtrack(script_instruction inst)
{
/*	int temp;

	switch(inst.mode)
	{
		case 0:
			temp= (int)floor(inst.op1);
			break;
		case 1:
			temp= (int)floor(get_variable(inst.op1));
			break;
		default:
			temp= 0;
			break;
	}

	MidiTrackOff();
	MidiLoadTrack(temp); */
}

void s_Set_Fog_Depth(script_instruction inst)
{
	int temp;

	switch(inst.mode)
	{
		case 0:
			temp= (int)floor(inst.op1);
			break;
		case 1:
			temp= (int)floor(get_variable(int(inst.op1)));
			break;
		default:
			temp= 0;
			break;
 	}
	OGL_GetFogData(OGL_Fog_AboveLiquid)->Depth = temp;
}

void s_Set_Fog_Color(script_instruction inst)
{
	float r,g,b;

	r= inst.op1;
	g= inst.op2;
	b= inst.op3;

	switch(inst.mode)
	{
		case 1:
			r= get_variable(int(inst.op1));
			break;
		case 2:
			g= get_variable(int(inst.op2));
			break;
		case 3:
			r= get_variable(int(inst.op1));
			g= get_variable(int(inst.op2));
			break;
		case 4:
			b= get_variable(int(inst.op3));
			break;
		case 5:
			r= get_variable(int(inst.op1));
			b= get_variable(int(inst.op3));
			break;
		case 6:
			g= get_variable(int(inst.op2));
			b= get_variable(int(inst.op3));
			break;
		case 7:
			r= get_variable(int(inst.op1));
			g= get_variable(int(inst.op2));
			b= get_variable(int(inst.op3));
			break;
		default:
			break;
	}
	
	rgb_color& Color = OGL_GetFogData(OGL_Fog_AboveLiquid)->Color;

	Color.red = PIN(int(65535*r+0.5),0,65535);
	Color.green = PIN(int(65535*g+0.5),0,65535);
	Color.blue = PIN(int(65535*b+0.5),0,65535);
}

void s_Get_Fog_Depth(script_instruction inst)
{
	if (inst.mode != 1)
		return;

	set_variable(int(inst.op1),OGL_GetFogData(OGL_Fog_AboveLiquid)->Depth);

}

void s_Get_Fog_Color(script_instruction inst)
{
	if (inst.mode != 7)
		return;
	
	rgb_color& Color = OGL_GetFogData(OGL_Fog_AboveLiquid)->Color;
	
	set_variable(int(inst.op1),Color.red/65535.0);
	set_variable(int(inst.op2),Color.green/65535.0);
	set_variable(int(inst.op3),Color.blue/65535.0);

}

void s_Teleport_Player(script_instruction inst)
{
	int dest;
	monster_data *monster= get_monster_data(current_player->monster_index);
	
	
	switch(inst.mode)
	{
		case 0:
				dest = (int)floor(inst.op1);
				break;
		case 1:
				dest = (int)floor(get_variable(int(inst.op1)));
				break;
		default:
				dest = 0;
				break;
	}
	
	
	
	SET_PLAYER_TELEPORTING_STATUS(current_player, true);
	monster->action= _monster_is_teleporting;
	current_player->teleporting_phase= 0;
	current_player->delay_before_teleport= 0;
	
	current_player->teleporting_destination= dest;
	start_teleporting_effect(true);
	play_object_sound(current_player->object_index, Sound_TeleportOut());
}

void s_Wait_For_Path(script_instruction inst)
{
	if (is_startup)
		return;
		
	if (cameras[current_camera].path != -1)
	{
		current_instruction--;
	
	}

}


//op1: Return the monster's index number
//op2: The type of monster to create
//op3: The polygon to put him in (center)

//THIS IS BROKEN
//DO NOT USE

void s_Monster_New(script_instruction inst)
{
	object_location theLocation;		//where we will put him
	struct polygon_data *destination;
	world_point3d theDestination;
	short index;
	short type, where;
	
	dprintf("I TOLD YOU NOT TO USE THIS INSTRUCTION DAMMIT");
	set_variable(int(inst.op1), -1);
	return;
	
	switch(inst.mode)			//requires: var, both, both
	{
	case 1: 
		type = (short)inst.op2;
		where= (short)inst.op3;
		break;
	case 3:
		type = (short)get_variable(int(inst.op2));
		where = (short)inst.op3;
		break;
	case 5:
		type = (short)inst.op2;
		where = (short)get_variable(int(inst.op3));
		break;
	case 7:
		type = (short)get_variable(int(inst.op2));
		where = (short)get_variable(int(inst.op3));	
		break;
	default:
	return;			//if we don't have a variable as element 1, return
	}
	
	theLocation.polygon_index = (int16)where;
	destination = NULL;
	destination= get_polygon_data(theLocation.polygon_index);
	if(destination==NULL)
		return;
	*((world_point2d *)&theDestination)= destination->center;	//stolen, assuming it works
	theDestination.z= destination->floor_height;
	theLocation.p = theDestination;
	theLocation.yaw = 0;
	theLocation.pitch=0;
	theLocation.flags = 0;//(monster_placement_info+type)->flags;			//so far
	
	index = new_monster(&theLocation, (short)type);
//	dprintf("We have created monster %d", index);
	set_variable(int(inst.op1), index);
	
	return;			//blank for now
}

//op1: The monster to euthanize
void s_Monster_Sleep(script_instruction inst)
{
	
	short theMonster;
	struct monster_data *theMonsterData;
	//requires: var
	switch(inst.mode)
	{
	case 1:
	case 3:
	case 5:
	case 7:
		theMonster = (short)get_variable(int(inst.op1));
		break;
	default:
		dprintf("Argument of Monster_Sleep must be a variable\n");
		return;		//was not a var
	}
//	dprintf("Trying to put monster %d to sleep\n", theMonster);
	if(theMonster == -1) return;

	theMonsterData = GetMemberWithBounds(monsters,theMonster,MAXIMUM_MONSTERS_PER_MAP);

	if(SLOT_IS_USED(theMonsterData))		//Only hurt existing monsters
		{
//		dprintf("monster %d snoozing\n", theMonster);
		if(MONSTER_IS_ACTIVE(theMonsterData))
			deactivate_monster(theMonster);
		}
	return;			//blank for now
}


//op1: The monster to damage
//op2: The damage taken
//[op3: the type of damage]
void s_Monster_Hurt(script_instruction inst)
{
	
	struct damage_definition theDamage;		//create some damage to deal
	struct monster_data *theMonster;
	short damage, target;
	
	//required: var, any
	switch(inst.mode)
	{
/*	case 0:
	case 4:
		target= (short)inst.op1;
		damage = (short)inst.op2;
		break; */
	case 1:
	case 5:
		target = (short)get_variable(int(inst.op1));
		damage = (short)inst.op2;
		break;
/*	case 2:
	case 6:
		target = (short)inst.op1;
		damage = (short)get_variable(int(inst.op2));
		break;	*/
	case 3:
	case 7:
		target = (short)get_variable(int(inst.op1));	
		damage = (short)get_variable(int(inst.op2));
		break;
	default:
		dprintf("Argument 1 of Monster_Hurt must be a variable\n");
		return;
	}	
	theDamage.type = _damage_fist;			//we do Fist of God damage :)
	theDamage.base = damage;		//user selects damage
	theDamage.random = 0;
	theDamage.flags = 0;
	theDamage.scale = 65536;				//something involving fixed-point math
//	dprintf("trying to damage monster %d\n", inst.op1);
//	SysBeep(30);
//	exit(1);
//	dprintf("We want monster at address %d\n", SLOT_IS_USED(theMonster));
	if(target == -1) return;
	theMonster= GetMemberWithBounds(monsters,target,MAXIMUM_MONSTERS_PER_MAP);
	if(!SLOT_IS_USED(theMonster)) return;		//Only hurt existing monsters
	damage_monster(target, NONE, NONE, &(get_monster_data(target)->sound_location), &theDamage);
}


//op1: The monster that will attack
//op2: the monster that will be attacked
void s_Monster_Attack(script_instruction inst)
{
	struct monster_data *bully, *poorHelplessVictim;
	short attacker, attackee;
	
	
	switch(inst.mode)				//requires: var, var
	{
/*	case 0:
	case 4:
		attacker = (short)inst.op1;
		attackee = (short)inst.op2;
		break;
	case 1:
	case 5:
		attacker = (short)get_variable(int(inst.op1));
		attackee = (short)inst.op2;
		break;
	case 2:
	case 6:
		attacker = (short)inst.op1;
		attackee = (short)get_variable(int(inst.op2));
		break; */
	case 3:
	case 7:
		attacker = (short)get_variable(int(inst.op1));
		attackee = (short)get_variable(int(inst.op2));
		break;
	default:
		dprintf("Arguments of Monster_Attack must be variables\n");
		return;
	}
	
	if(attacker == -1 || attackee ==-1) return;	
	bully = GetMemberWithBounds(monsters,attacker,MAXIMUM_MONSTERS_PER_MAP);
	poorHelplessVictim = GetMemberWithBounds(monsters,attackee,MAXIMUM_MONSTERS_PER_MAP);
	if(!SLOT_IS_USED(bully) || !SLOT_IS_USED(poorHelplessVictim)) return;
//	dprintf("monster %d attacks monster %d", attacker, attackee);
	change_monster_target(attacker, attackee);
	
	return;			//blank for now
}

//op1: The monster to move
//op2: The polygon to move to. Monster heads for center.
//[op3: other options]
void s_Monster_Move(script_instruction inst)
{
	world_point2d theStart, *theEnd;	//where we start from. start is monster's pos, end is center of dest
	short startPoly, endPoly;		//poly indexes for new_path
	world_distance minSpace;		//3*monster_definition->radius
	struct monster_pathfinding_data thePath;		//used by new_path
	struct monster_data *theRealMonster;			//the monster data
	struct monster_definition *theDef;				//the def of the monster
	struct object_data *theObject;						//the monster's physical form
	short theMonster;
	
	//dprintf("we are moving a monster\n");
	//required: var, any
	switch(inst.mode)
	{
	case 1:
	case 5:
		theMonster = (short)get_variable(int(inst.op1));
		endPoly = (short)inst.op2;
		break;
	case 3:
	case 7:
		theMonster = (short)get_variable(int(inst.op1));
		endPoly = (short)get_variable(int(inst.op2));
		break;
	default:
		dprintf("Argument 1 of Monster_Move must be a variable\n");	
	return;			//blank for now
	}
	if(theMonster == -1) return;
	theRealMonster = GetMemberWithBounds(monsters,theMonster,MAXIMUM_MONSTERS_PER_MAP);
	if(!SLOT_IS_USED(theRealMonster)) return;		//we must have a legal monster
	theDef = get_monster_definition_external(theRealMonster->type);
	theObject = get_object_data(theRealMonster->object_index);
	//some checks stolen from generate_new_path_for_monster
	if(!MONSTER_IS_ACTIVE(theRealMonster))
		activate_monster(theMonster);
	if (theRealMonster->path!=NONE)
		{
		delete_path(theRealMonster->path);
		theRealMonster->path= NONE;
		}
	SET_MONSTER_NEEDS_PATH_STATUS(theRealMonster, false);
	
	thePath.definition = get_monster_definition_external(theRealMonster->type);		//we want to move this type of monster
	thePath.monster = theRealMonster;	//specifically, this one
	thePath.cross_zone_boundaries = true;	//because we have a poly...
	
	theEnd = &(get_polygon_data(endPoly)->center);
	

	
	theRealMonster->path = new_path((world_point2d *)&theObject->location, theObject->polygon, theEnd,
		endPoly, 3*get_monster_definition_external(theRealMonster->type)->radius, monster_pathfinding_cost_function, &thePath);
	if (theRealMonster->path==NONE)
		{
		if(theRealMonster->action!=_monster_is_being_hit || MONSTER_IS_DYING(theRealMonster)) set_monster_action(theMonster, _monster_is_stationary);
		set_monster_mode(theMonster, _monster_unlocked, NONE);
		return;
		}
	advance_monster_path(theMonster);	//this seems to be called


}

//op1: return the mionster selected
//op2: the monster type to get
//op3: the polygon to look in
void s_Monster_Select(script_instruction inst)
{
	//we are using IntersectedObjects
	int found, i;
	int objectCount;
	struct monster_data * theMonster;
	short type, where;
//	dprintf("Selecting...\n");
	switch(inst.mode)			//requires: var, any, any
	{
	case 1: 
		type = (short)inst.op2;
		where= (short)inst.op3;
		break;
	case 3:
		type = (short)get_variable(int(inst.op2));
		where = (short)inst.op3;
		break;
	case 5:
		type = (short)inst.op2;
		where = (short)get_variable(int(inst.op3));
		break;
	case 7:
		type = (short)get_variable(int(inst.op2));
		where = (short)get_variable(int(inst.op3));	
		break;
	default:
	dprintf("Argument 1 of Monster_Select must be a variable\n");
	return;			//if we don't have a variable as element 1, return
	}
//	dprintf("searching polygon %d for monsters of type %d\n", where, type);
	found = possible_intersecting_monsters(&IntersectedObjects, LOCAL_INTERSECTING_MONSTER_BUFFER_SIZE, (short)where, false);
	//now we have a list of stuff
	objectCount = IntersectedObjects.size();
	set_variable(int(inst.op1), -1);
	for(i=0;i<objectCount;i++)
		{
		theMonster = GetMemberWithBounds(monsters,get_object_data(IntersectedObjects[i])->permutation,MAXIMUM_MONSTERS_PER_MAP); //please tell me this works
//		dprintf("checking a monster found in polygon %d\n", theMonster->sound_polygon_index);
		if((theMonster->type == type) && SLOT_IS_USED(theMonster) && theMonster->unused[0] != where)		//if we have a monster of the same type
			{
			if(get_object_data(theMonster->object_index)->polygon == where)		//double if because get_object_data only works if the monster is legal in the first place
				{
//				dprintf("We have selected monster %d\n", get_object_data(IntersectedObjects[i])->permutation);
				theMonster->unused[0]=where;
				set_variable(int(inst.op1), get_object_data(IntersectedObjects[i])->permutation);
				return;
				}
			}
		}
//	dprintf("No monster found, returning -1\n");
	set_variable(int(inst.op1), -1);
//	dprintf("we return %d\n", (short)get_variable(int(inst.op1)));
	return;
}

//op1: the monster to edit (this is global for all monsters of this type)
//op2: the damage type to check
//op3: the value of the damage type (var)
void s_Monster_Get_Immunity(script_instruction inst)
{
	struct monster_data *theMonster;
	short monsterIndex;
	struct monster_definition *theDef;
	int16 damageType;
	int theValue;
//	dprintf("trying vulnerability\n");
	switch(inst.mode)
	{
	case 5:
		monsterIndex = (short)get_variable(int(inst.op1));
		damageType = (int16)inst.op2;
		break;
	case 7:
		monsterIndex = (short)get_variable(int(inst.op1));
		damageType = (int16)get_variable(int(inst.op2));
		break;
	default:
		dprintf("First and last arguments of Monster_Get_Immunity must be variables\n");
	return; 
	}
	
//	dprintf("checking type %d of monster %d\n", damageType, monsterIndex);
	if(monsterIndex == -1) return;
	theMonster = GetMemberWithBounds(monsters,monsterIndex,MAXIMUM_MONSTERS_PER_MAP);
	if(SLOT_IS_USED(theMonster))
		{
		theDef = get_monster_definition_external(theMonster->type);
		set_variable((int)inst.op3, theDef->immunities & 1<<damageType);
//		dprintf("invulnerability was %d\n", theDef->immunities & 1<<damageType);
		}
}

//op1: the monster to edit (this is global for all monsters of this type)
//op2: the damage type to set
//op3: the value of the damage type
void s_Monster_Set_Immunity(script_instruction inst)
{
	struct monster_data *theMonster;
	short monsterIndex;
	struct monster_definition *theDef;
	int16 damageType;
	int theValue;
//	dprintf("trying vulnerability\n");
	switch(inst.mode)
	{
	case 1:
		monsterIndex = (short)get_variable(int(inst.op1));
		damageType = (int16)inst.op2;
		theValue = (int)inst.op3;
		break;
	case 3:
		monsterIndex = (short)get_variable(int(inst.op1));
		damageType = (int16)get_variable(int(inst.op2));
		theValue = (int)inst.op3;
	case 5:
		monsterIndex = (short)get_variable(int(inst.op1));
		damageType = (int16)inst.op2;
		theValue = (int)get_variable(int(inst.op3));
		break;
	case 7:
		monsterIndex = (short)get_variable(int(inst.op1));
		damageType = (int16)get_variable(int(inst.op2));
		theValue = (int)get_variable(int(inst.op3));
		break;
	default:
		dprintf("First and last arguments of Monster_Set_Immunity must be variables\n");
	return; 
	}
	
//	dprintf("checking type %d of monster %d\n", damageType, monsterIndex);
	if(monsterIndex == -1) return;
	theMonster = GetMemberWithBounds(monsters,monsterIndex,MAXIMUM_MONSTERS_PER_MAP);
	if(SLOT_IS_USED(theMonster))
		{
		if(theValue == 1)
		{
			theDef = get_monster_definition_external(theMonster->type);
//			dprintf("invulnerability was %d\n", theDef->immunities & 1<<damageType);
			theDef->immunities = theDef->immunities | 1<<damageType;
//			dprintf("invulnerability is now %d\n", theDef->immunities & 1<<damageType);
			return;
		}
		if(theValue == 0)
		{
			theDef = get_monster_definition_external(theMonster->type);
//			dprintf("invulnerability was %d\n", theDef->immunities & 1<<damageType);
			theDef->immunities = theDef->immunities & ~(1<<damageType);
//			dprintf("invulnerability is now %d\n", theDef->immunities & 1<<damageType);
			return;
		}
		dprintf("Invulnerability value must be 0 or 1\n");
		}
}

//op1: the monster to edit (this is global for all monsters of this type)
//op2: the damage type to check
//op3: the value of the damage type (var)
void s_Monster_Get_Weakness(script_instruction inst)
{
	struct monster_data *theMonster;
	short monsterIndex;
	struct monster_definition *theDef;
	int16 damageType;
	int theValue;
//	dprintf("trying weakness\n");
	switch(inst.mode)
	{
	case 5:
		monsterIndex = (short)get_variable(int(inst.op1));
		damageType = (int16)inst.op2;
		break;
	case 7:
		monsterIndex = (short)get_variable(int(inst.op1));
		damageType = (int16)get_variable(int(inst.op2));
		break;
	default:
		dprintf("First and last arguments of Monster_Get_Weakness must be variables\n");
	return; 
	}
	
//	dprintf("checking type %d of monster %d\n", damageType, monsterIndex);
	if(monsterIndex == -1) return;
	theMonster = GetMemberWithBounds(monsters,monsterIndex,MAXIMUM_MONSTERS_PER_MAP);
	if(SLOT_IS_USED(theMonster))
		{
		theDef = get_monster_definition_external(theMonster->type);
		set_variable((int)inst.op3, theDef->weaknesses & 1<<damageType);
//		dprintf("weakness was %d\n", theDef->weaknesses & 1<<damageType);
		}
}

//op1: the monster to edit (this is global for all monsters of this type)
//op2: the damage type to check
//op3: the value of the damage type (var)
void s_Monster_Set_Weakness(script_instruction inst)
{
	struct monster_data *theMonster;
	short monsterIndex;
	struct monster_definition *theDef;
	int16 damageType;
	int theValue;
//	dprintf("trying weakness set\n");
	switch(inst.mode)
	{
	case 1:
		monsterIndex = (short)get_variable(int(inst.op1));
		damageType = (int16)inst.op2;
		theValue = (int)inst.op3;
		break;
	case 3:
		monsterIndex = (short)get_variable(int(inst.op1));
		damageType = (int16)get_variable(int(inst.op2));
		theValue = (int)inst.op3;
	case 5:
		monsterIndex = (short)get_variable(int(inst.op1));
		damageType = (int16)inst.op2;
		theValue = (int)get_variable(int(inst.op3));
		break;
	case 7:
		monsterIndex = (short)get_variable(int(inst.op1));
		damageType = (int16)get_variable(int(inst.op2));
		theValue = (int)get_variable(int(inst.op3));
		break;
	default:
		dprintf("First and last arguments of Monster_Set_Weakness must be variables\n");
	return; 
	}
	
//	dprintf("checking type %d of monster %d\n", damageType, monsterIndex);
	if(monsterIndex == -1) return;
	theMonster = GetMemberWithBounds(monsters,monsterIndex,MAXIMUM_MONSTERS_PER_MAP);
	if(SLOT_IS_USED(theMonster))
		{
		if(theValue == 1)
		{
			theDef = get_monster_definition_external(theMonster->type);
//			dprintf("weakness was %d\n", theDef->weaknesses & 1<<damageType);
			theDef->weaknesses = theDef->weaknesses | 1<<damageType;
//			dprintf("weakness is now %d\n", theDef->weaknesses & 1<<damageType);
			return;
		}
		if(theValue == 0)
		{
			theDef = get_monster_definition_external(theMonster->type);
//			dprintf("weaknesses was %d\n", theDef->weaknesses & 1<<damageType);
			theDef->weaknesses = theDef->weaknesses & ~(1<<damageType);
//			dprintf("weaknesses is now %d\n", theDef->weaknesses & 1<<damageType);
			return;
		}
		dprintf("weaknesses value must be 0 or 1\n");
		}
}

//op1: the monster to edit (this is global for all monsters of this type)
//op2: the type to check
//op3: the value of the friend setting (var)
void s_Monster_Get_Friend(script_instruction inst)
{
	struct monster_data *theMonster;
	short monsterIndex;
	struct monster_definition *theDef;
	int32 monsterClass;
	int theValue;
//	dprintf("trying firends\n");
	switch(inst.mode)
	{
	case 5:
		monsterIndex = (short)get_variable(int(inst.op1));
		monsterClass = (int16)inst.op2;
		break;
	case 7:
		monsterIndex = (short)get_variable(int(inst.op1));
		monsterClass = (int16)get_variable(int(inst.op2));
		break;
	default:
		dprintf("First and last arguments of Monster_Get_Friend must be variables\n");
	return; 
	}
	
//	dprintf("checking friend %d of monster %d\n", monsterClass, monsterIndex);
	theMonster = GetMemberWithBounds(monsters,monsterIndex,MAXIMUM_MONSTERS_PER_MAP);
	if(monsterIndex == -1) return;
	if(SLOT_IS_USED(theMonster))
		{
		theDef = get_monster_definition_external(theMonster->type);
		set_variable((int)inst.op3, theDef->friends & 1<<monsterClass);
//		dprintf("weakness was %d\n", theDef->friends & 1<<monsterClass);
		}

}

//op1: the monster to edit (this is global for all monsters of this type)
//op2: the damage type to check
//op3: the value of the damage type (var)
void s_Monster_Set_Friend(script_instruction inst)
{
	struct monster_data *theMonster;
	short monsterIndex;
	struct monster_definition *theDef;
	int32 monsterClass;
	int theValue;
//	dprintf("trying weakness set\n");
	switch(inst.mode)
	{
	case 1:
		monsterIndex = (short)get_variable(int(inst.op1));
		monsterClass = (int32)inst.op2;
		theValue = (int)inst.op3;
		break;
	case 3:
		monsterIndex = (short)get_variable(int(inst.op1));
		monsterClass = (int16)get_variable(int(inst.op2));
		theValue = (int)inst.op3;
	case 5:
		monsterIndex = (short)get_variable(int(inst.op1));
		monsterClass = (int16)inst.op2;
		theValue = (int)get_variable(int(inst.op3));
		break;
	case 7:
		monsterIndex = (short)get_variable(int(inst.op1));
		monsterClass = (int16)get_variable(int(inst.op2));
		theValue = (int)get_variable(int(inst.op3));
		break;
	default:
		dprintf("First argument of Monster_Set_Weakness must be variables\n");
	return; 
	}
	
//	dprintf("checking type %d of monster %d\n", monsterClass, monsterIndex);
	if(monsterIndex == -1) return;
	theMonster = GetMemberWithBounds(monsters,monsterIndex,MAXIMUM_MONSTERS_PER_MAP);
	if(SLOT_IS_USED(theMonster))
		{
		if(theValue == 1)
		{
			theDef = get_monster_definition_external(theMonster->type);
//			dprintf("friendliness was %d\n", theDef->friends & 1<<monsterClass);
			theDef->friends = theDef->friends | 1<<monsterClass;
			//theDef->friends | FLAG(monsterClass);
//			dprintf("friendliness is now %d\n", theDef->friends & 1<<monsterClass);
			return;
		}
		if(theValue == 0)
		{
			theDef = get_monster_definition_external(theMonster->type);
//			dprintf("friendliness was %d\n", theDef->friends & 1<<monsterClass);
			theDef->friends = theDef->friends & ~(1<<monsterClass);
//			dprintf("friendliness is now %d\n", theDef->friends & 1<<monsterClass);
			return;
		}
		dprintf("friendliness value must be 0 or 1\n");
		}
}

//op1: the monster to edit (this is global for all monsters of this type)
//op2: the damage type to check
//op3: the value of the damage type (var)
void s_Monster_Get_Enemy(script_instruction inst)
{
	struct monster_data *theMonster;
	short monsterIndex;
	struct monster_definition *theDef;
	int32 monsterClass;
	int theValue;
//	dprintf("trying enemies\n");
	switch(inst.mode)
	{
	case 5:
		monsterIndex = (short)get_variable(int(inst.op1));
		monsterClass = (int16)inst.op2;
		break;
	case 7:
		monsterIndex = (short)get_variable(int(inst.op1));
		monsterClass = (int16)get_variable(int(inst.op2));
		break;
	default:
		dprintf("First and last arguments of Monster_Get_Enemy must be variables\n");
	return; 
	}
	
	//"SHOW ME, EPYON! WHO IS MY ENEMY?"
	
//	dprintf("checking enemy %d of monster %d\n", monsterClass, monsterIndex);
	if(monsterIndex == -1) return;
	theMonster = GetMemberWithBounds(monsters,monsterIndex,MAXIMUM_MONSTERS_PER_MAP);
	if(SLOT_IS_USED(theMonster))
		{
		theDef = get_monster_definition_external(theMonster->type);
		set_variable((int)inst.op3, theDef->enemies & 1<<monsterClass);
//		dprintf("enemy was %d\n", theDef->friends & 1<<monsterClass);
		}
}

//op1: The monster to edit (this is global for all monsters of this type)
//op2: The class to make friend or enemy
//op3: What to make it (true is checked, false is unchecked)
void s_Monster_Set_Enemy(script_instruction inst)
{
	struct monster_data *theMonster;
	short monsterIndex;
	struct monster_definition *theDef;
	int32 monsterClass;
	int theValue;
//	dprintf("trying to set enemy set\n");
	switch(inst.mode)
	{
	case 1:
		monsterIndex = (short)get_variable(int(inst.op1));
		monsterClass = (int32)inst.op2;
		theValue = (int)inst.op3;
		break;
	case 3:
		monsterIndex = (short)get_variable(int(inst.op1));
		monsterClass = (int16)get_variable(int(inst.op2));
		theValue = (int)inst.op3;
	case 5:
		monsterIndex = (short)get_variable(int(inst.op1));
		monsterClass = (int16)inst.op2;
		theValue = (int)get_variable(int(inst.op3));
		break;
	case 7:
		monsterIndex = (short)get_variable(int(inst.op1));
		monsterClass = (int16)get_variable(int(inst.op2));
		theValue = (int)get_variable(int(inst.op3));
		break;
	default:
		dprintf("First argument of Monster_Set_Enemy must be variables\n");
	return; 
	}
	
//	dprintf("checking enemy %d of monster %d\n", monsterClass, monsterIndex);
	if(monsterIndex == -1) return;
	theMonster = GetMemberWithBounds(monsters,monsterIndex,MAXIMUM_MONSTERS_PER_MAP);
	if(SLOT_IS_USED(theMonster))
		{
		if(theValue == 1)
		{
			theDef = get_monster_definition_external(theMonster->type);
//			dprintf("eneminess was %d\n", theDef->enemies & 1<<monsterClass);
			theDef->enemies = theDef->enemies | 1<<monsterClass;
			//theDef->friends | FLAG(monsterClass);
//			dprintf("eneminess is now %d\n", theDef->enemies & 1<<monsterClass);
			return;
		}
		if(theValue == 0)
		{
			theDef = get_monster_definition_external(theMonster->type);
//			dprintf("eneminess was %d\n", theDef->enemies & 1<<monsterClass);
			theDef->enemies = theDef->enemies & ~(1<<monsterClass);
//			dprintf("eneminess is now %d\n", theDef->enemies & 1<<monsterClass);
			return;
		}
		dprintf("eneminess value must be 0 or 1\n");
		}

}

//op1: The monster to edit (this is global for all monsters of this type)
//op2: The class to make friend or enemy
//op3: What to make it (true is checked, false is unchecked)
void s_Monster_Get_Item(script_instruction inst)
{
	struct monster_data *theMonster;
	short monsterIndex;
	struct monster_definition *theDef;
	int32 monsterClass;
//	dprintf("trying enemies\n");
	switch(inst.mode)
	{
	case 3:
	case 7:
		monsterIndex = (short)get_variable(int(inst.op1));
	default:
		dprintf("First and second arguments of Monster_Get_Item must be variables\n");
	return; 
	}

//	dprintf("checking enemy %d of monster %d\n", monsterClass, monsterIndex);
	if(monsterIndex == -1) return;
	theMonster = GetMemberWithBounds(monsters,monsterIndex,MAXIMUM_MONSTERS_PER_MAP);
	if(SLOT_IS_USED(theMonster))
		{
		theDef = get_monster_definition_external(theMonster->type);
		set_variable((int)inst.op2, theDef->carrying_item_type);
//		dprintf("enemy was %d\n", theDef->friends & 1<<monsterClass);
		}

}

//op1: The monster to edit (this is global for all monsters of this type)
//op2: The class to make friend or enemy
//op3: What to make it (true is checked, false is unchecked)
void s_Monster_Set_Item(script_instruction inst)
{
	struct monster_data *theMonster;
	short monsterIndex;
	struct monster_definition *theDef;
	int16 itemClass;
//	dprintf("trying enemies\n");
	switch(inst.mode)
	{
	case 1:
	case 5:
		monsterIndex = (short)get_variable(int(inst.op1));
		itemClass = (int16)inst.op2;
		break;
	case 3:
	case 7:
		monsterIndex = (short)get_variable(int(inst.op1));
		itemClass = (int16)get_variable(int(inst.op2));
		break;
	default:
		dprintf("First and last arguments of Monster_Get_Item must be variables\n");
	return; 
	}

//	dprintf("checking enemy %d of monster %d\n", monsterClass, monsterIndex);
	if(monsterIndex == -1) return;
	theMonster = GetMemberWithBounds(monsters,monsterIndex,MAXIMUM_MONSTERS_PER_MAP);
	if(SLOT_IS_USED(theMonster))
		{
		theDef = get_monster_definition_external(theMonster->type);
		theDef->carrying_item_type = itemClass;
//		dprintf("enemy was %d\n", theDef->friends & 1<<monsterClass);
		}


}

//op1: The monster to edit (this is global for all monsters of this type)
//op2: The class to make friend or enemy
//op3: What to make it (true is checked, false is unchecked)
void s_Monster_Get_Nuke(script_instruction inst)
{
}

//op1: The monster to edit (this is global for all monsters of this type)
//op2: The class to make friend or enemy
//op3: What to make it (true is checked, false is unchecked)
void s_Monster_Set_Nuke(script_instruction inst)
{
}
