/* script_instructions.c
	2/16/00 Created - Chris Pruett
	
	Aug 10, 2000 (Loren Petrich): renamed dynamic_fog_depth to FogDepth
*/

 

  
  
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

#include "script_parser.h"
#include "script_instructions.h"

#define sign(a) (a) < 0 ? -1 : 1


/*extern'd references to the rest of the mara code */
 
extern struct view_data *world_view;
extern bool ready_weapon(short player_index, short weapon_index);
extern void draw_panels(void);
extern struct monster_data *get_monster_data(short monster_index);

// LP: added complete set of fog variables: whether present, the depth, and the color
// The depth is in internal units (1024 = 1 world unit),
// and the color is RGBA, with color channels from 0 to 1.
extern bool FogPresent;
extern GLfloat FogDepth;
extern GLfloat FogColor[4];



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
extern void do_next_instruction(void);
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
short offset_polygon_index;
float offset_yaw;
float offset_pitch;
short offset_count;
short roll_count;

//float current_yaw;
//float current_pitch;

bool s_camera_Control;
short old_size;
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


/*-------------------------------------------*/

/*init_instructions sets up the instruction_lookup array so that
functions can be called by casting back the pointer.  This is only
called once.*/
void init_instructions(void)
{

	

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
	


}

#pragma mark -

/* ----------------------------------------- */

void update_path_camera(void)
{
	path_point old_point;
	const float AngleConvert = 360/float(FULL_CIRCLE);
	
	/* copy over old location info so we can calculate new polygon index later */
	
	old_point.yaw = camera_point.location.yaw;
	old_point.pitch = camera_point.location.pitch;
	old_point.position.x = camera_point.location.position.x;
	old_point.position.y = camera_point.location.position.y;
	old_point.position.z = camera_point.location.position.z;
	old_point.polygon_index = camera_point.location.polygon_index;

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
			camera_point.location.pitch = normalize_angle(camera_point.location.pitch + offset_pitch);
		}

		if (normalize_angle(camera_point.location.pitch) > normalize_angle(script_paths[cameras[current_camera].path].the_path[cameras[current_camera].point].location.pitch))
		{
			camera_point.location.pitch = normalize_angle(camera_point.location.pitch - offset_pitch);
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
	
	world_point2d CurrentPosition, TargetPosition;
	short CurrentPolygonIndex;
	short NextPolygonIndex;
	
	// Set CurrentPosition to the old position of the camera
	// Set TargetPosition to the new position of the camera
	// Set CurrentPolygonIndex to the old polygon membership of the camera
	
/*	TargetPosition.x = camera_point.location.position.x;
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
						cameras[current_camera].point = 0;
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
		temp = get_variable(inst.op1);
	}
		
		
	set_instruction_decay(machine_tick_count() + temp);
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
		temp = get_variable(inst.op1);
	}
				

	damage.flags= _alien_damage;
	damage.type= _damage_crushing;
	damage.base= temp;
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
		temp = get_variable(inst.op1);
	}
		
		
	jump_to_line((int)floorf(temp));

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
	the_mode->size = _full_screen;
	change_screen_mode(the_mode,true);
}

void s_Show_Interface(script_instruction inst)
{
	screen_mode_data *the_mode;
	the_mode = get_screen_mode();
	the_mode->size =old_size;
	change_screen_mode(the_mode,true);
	draw_panels();
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
				temp = get_variable(inst.op1);
				break;
			case 2:
				temp2 = get_variable(inst.op2);
				break;
			case 3:
				temp = get_variable(inst.op1);
				temp2 = get_variable(inst.op2);
				break;
		}
		
		
	if (set_tagged_light_statuses(temp, temp2));
	if (try_and_change_tagged_platform_states(temp, temp2)); 
	
	assume_correct_switch_position(_panel_is_tag_switch, temp, temp2);
	
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
				temp2 = get_variable(inst.op2);
				break;
			case 3:
				temp = get_variable(inst.op1);
				temp2 = get_variable(inst.op2);
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
		set_variable(inst.op2,1);
	else
		set_variable(inst.op2,0);
	

}

void s_Define(script_instruction inst)
{
	add_variable(inst.op1);
}

void s_sAdd(script_instruction inst)
{
	float temp;
	
	if (inst.mode == 0)
		return;
	
	switch(inst.mode)
	{
		case 1:
				temp = get_variable(inst.op1);
				set_variable(inst.op1,temp+inst.op2);
				break;
		case 3:
				temp = get_variable(inst.op1);
				set_variable(inst.op1,temp+get_variable(inst.op2));
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
				temp = get_variable(inst.op1);
				set_variable(inst.op1,temp-inst.op2);
				break;
		case 3:
				temp = get_variable(inst.op1);
				set_variable(inst.op1,temp-get_variable(inst.op2));
				break;
		default:
				break;
	}
 	 
	 
}

void s_If_Equal(script_instruction inst)
{
	float temp;
	
	if (inst.mode > 3)
		temp = get_variable(inst.op3);
	else
		temp = inst.op3;
		
	switch(inst.mode)
	{
		case 0:
		case 4:
			if (inst.op1 == inst.op2)
				jump_to_line((int)floorf(temp));
			break;
		case 1:
		case 5:
			if (get_variable(inst.op1) == inst.op2)
				jump_to_line((int)floorf(temp));
			
			break;
		case 2:
		case 6:
			if (get_variable(inst.op2) == inst.op1)
				jump_to_line((int)floorf(temp));
			
			break;
		case 3:
		case 7:
			if (get_variable(inst.op1) == get_variable(inst.op2))
				jump_to_line((int)floorf(temp));
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
				temp = get_variable(inst.op1);
				set_variable(inst.op1,inst.op2);
				break;
		case 3:
				temp = get_variable(inst.op1);
				set_variable(inst.op1,get_variable(inst.op2));
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
		temp = get_variable(inst.op3);
	else
		temp = inst.op3;
		
	switch(inst.mode)
	{
		case 0:
		case 4:
			if (inst.op1 > inst.op2)
				jump_to_line((int)floorf(temp));
			break;
		case 1:
		case 5:
			if (get_variable(inst.op1) > inst.op2)
				jump_to_line((int)floorf(temp));
			
			break;
		case 2:
		case 6:
			if (get_variable(inst.op2) > inst.op1)
				jump_to_line((int)floorf(temp));
			
			break;
		case 3:
		case 7:
			if (get_variable(inst.op1) > get_variable(inst.op2))
				jump_to_line((int)floorf(temp));
			break;
		default:
				break;
	}
 	 
	 
}

void s_If_Less(script_instruction inst)
{
	float temp;
	
	if (inst.mode > 3)
		temp = get_variable(inst.op3);
	else
		temp = inst.op3;
		
	switch(inst.mode)
	{
		case 0:
		case 4:
			if (inst.op1 < inst.op2)
				jump_to_line((int)floorf(temp));
			break;
		case 1:
		case 5:
			if (get_variable(inst.op1) < inst.op2)
				jump_to_line((int)floorf(temp));
			
			break;
		case 2:
		case 6:
			if (get_variable(inst.op2) < inst.op1)
				jump_to_line((int)floorf(temp));
			
			break;
		case 3:
		case 7:
			if (get_variable(inst.op1) < get_variable(inst.op2))
				jump_to_line((int)floorf(temp));
			break;
		default:
				break;
	}
 	 
	 
}

void s_If_Not_Equal(script_instruction inst)
{
	float temp;
	
	if (inst.mode > 3)
		temp = get_variable(inst.op3);
	else
		temp = inst.op3;
		
	switch(inst.mode)
	{
		case 0:
		case 4:
			if (inst.op1 != inst.op2)
				jump_to_line((int)floorf(temp));
			break;
		case 1:
		case 5:
			if (get_variable(inst.op1) != inst.op2)
				jump_to_line((int)floorf(temp));
			
			break;
		case 2:
		case 6:
			if (get_variable(inst.op2) != inst.op1)
				jump_to_line((int)floorf(temp));
			
			break;
		case 3:
		case 7:
			if (get_variable(inst.op1) != get_variable(inst.op2))
				jump_to_line((int)floorf(temp));
			break;
		default:
				break;
	}
 	 
	 
}

void s_Get_Life(script_instruction inst)
{		
	if (inst.mode == 0)
		return;
		
	set_variable(inst.op1,current_player->suit_energy);

}

void s_Set_Life(script_instruction inst)
{
		
	switch(inst.mode)
	{
		case 0:
				current_player->suit_energy = (int)floor(inst.op1);
				break;
		case 1:
				current_player->suit_energy = (int)floor(get_variable(inst.op1));
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
		
	set_variable(inst.op1,current_player->suit_oxygen);

}

void s_Set_Oxygen(script_instruction inst)
{
	switch(inst.mode)
	{
		case 0:
				current_player->suit_oxygen = (int)floor(inst.op1);
				break;
		case 1:
				current_player->suit_oxygen = (int)floor(get_variable(inst.op1));
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
		
	set_variable(inst.op1,get_trap_value(current_trap));
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
				if (!try_and_add_player_item(player_identifier_to_player_index(current_player->identifier), (int)floor(get_variable(inst.op1))))
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
				temp= (int)floor(get_variable(inst.op1));
				break;
		default:
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
		
	}	
	
	if (!ready_weapon(player_identifier_to_player_index(current_player->identifier), weapon_index))
		; /* this sucks */
		
	

}


void s_Block_Start(script_instruction inst)
{
	
	set_trap_instruction(current_trap, current_instruction);
	
	is_startup = true;
	
	do
		do_next_instruction();
	while (is_startup && script_in_use());
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
				temp= (int)floor(get_variable(inst.op1));
				break;
		default:
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
				temp= (int)floor(get_variable(inst.op1));
				break;
		default:
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
				temp= (int)floor(get_variable(inst.op1));
				break;
		default:
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
				x= get_variable(inst.op1);
				break;
		case 2:
				y= get_variable(inst.op2);
				break;
		case 3:
				x= get_variable(inst.op1);
				y= get_variable(inst.op2);
				break;
		case 4:
				z= get_variable(inst.op3);
				break;
		case 5:
				x= get_variable(inst.op1);
				z= get_variable(inst.op3);
				break;
		case 6:
				y= get_variable(inst.op2);
				z= get_variable(inst.op3);
				break;
		case 7:
				x= get_variable(inst.op1);
				y= get_variable(inst.op2);
				z= get_variable(inst.op3);
				break;
		default:
				break;
	}
	
	if (cameras)
	{
		cameras[current_camera].location.position.x = x*WORLD_ONE;
		cameras[current_camera].location.position.y = y*WORLD_ONE;
		cameras[current_camera].location.position.z = z*WORLD_ONE;
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
				temp = get_variable(inst.op1);
				break;
			case 2:
				temp2 = get_variable(inst.op2);
				break;
			case 3:
				temp = get_variable(inst.op1);
				temp2 = get_variable(inst.op2);
				break;
		}
		
	if (cameras)
	{
		cameras[current_camera].location.yaw = normalize_angle(temp/AngleConvert);
		
		cameras[current_camera].location.pitch = normalize_angle(temp2/AngleConvert);
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
				temp= (int)floor(get_variable(inst.op1));
				break;
		default:
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
				path_num= (int)floor(get_variable(inst.op1));
				path_length= (int)floor(inst.op2);
				break;
		case 2:
				path_num= (int)floor(inst.op1);
				path_length= (int)floor(get_variable(inst.op2));
				break;
		case 3:
				path_num= (int)floor(get_variable(inst.op1));
				path_length= (int)floor(get_variable(inst.op2));
				break;
		default:
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
				temp= (int)floor(get_variable(inst.op1));
				break;
		default:
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
				temp= (int)floor(get_variable(inst.op1));
				break;
		default:
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
				temp= get_variable(inst.op1);
				break;
		default:
				break;
	}
	
	if (script_paths)
		script_paths[current_path].move_speed = temp;
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
				temp= get_variable(inst.op1);
				break;
		default:
				break;
	}
	
	if (script_paths)
		script_paths[current_path].roll_speed = temp;
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
				temp= (int)floor(get_variable(inst.op1));
				break;
		default:
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
				temp= (int)floor(get_variable(inst.op1));
				break;
		default:
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
				x= get_variable(inst.op1);
				break;
		case 2:
				y= get_variable(inst.op2);
				break;
		case 3:
				x= get_variable(inst.op1);
				y= get_variable(inst.op2);
				break;
		case 4:
				z= get_variable(inst.op3);
				break;
		case 5:
				x= get_variable(inst.op1);
				z= get_variable(inst.op3);
				break;
		case 6:
				y= get_variable(inst.op2);
				z= get_variable(inst.op3);
				break;
		case 7:
				x= get_variable(inst.op1);
				y= get_variable(inst.op2);
				z= get_variable(inst.op3);
				break;
		default:
				break;
	}
	
	if (script_paths)
	{
		script_paths[current_path].the_path[current_path_point].location.position.x = x*WORLD_ONE;
		script_paths[current_path].the_path[current_path_point].location.position.y = y*WORLD_ONE;
		script_paths[current_path].the_path[current_path_point].location.position.z = z*WORLD_ONE;
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
				temp = get_variable(inst.op1);
				break;
			case 2:
				temp2 = get_variable(inst.op2);
				break;
			case 3:
				temp = get_variable(inst.op1);
				temp2 = get_variable(inst.op2);
				break;
		}
		
	if (script_paths)
	{
		script_paths[current_path].the_path[current_path_point].location.yaw = normalize_angle(temp/AngleConvert);
		
		script_paths[current_path].the_path[current_path_point].location.pitch = normalize_angle(temp2/AngleConvert);
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
				temp= (int)floor(get_variable(inst.op1));
				break;
		default:
				break;
	}
	
	if (cameras && script_paths)
	{
		cameras[current_camera].path = temp-1;
		cameras[current_camera].point = 0;
		
	/*	cameras[current_camera].location.position = script_paths[cameras[current_camera].path].the_path[0].location.position;
		cameras[current_camera].location.polygon_index = script_paths[cameras[current_camera].path].the_path[0].location.polygon_index;
		cameras[current_camera].location.yaw = script_paths[cameras[current_camera].path].the_path[0].location.yaw;
		cameras[current_camera].location.pitch = script_paths[cameras[current_camera].path].the_path[0].location.pitch;
	*/
		current_path_point = 0;
		
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
			temp= (int)floor(get_variable(inst.op1));
			break;
		default:
			break;
 	}
	FogDepth = temp;
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
			r= get_variable(inst.op1);
			break;
		case 2:
			g= get_variable(inst.op2);
			break;
		case 3:
			r= get_variable(inst.op1);
			g= get_variable(inst.op2);
			break;
		case 4:
			b= get_variable(inst.op3);
			break;
		case 5:
			r= get_variable(inst.op1);
			b= get_variable(inst.op3);
			break;
		case 6:
			g= get_variable(inst.op2);
			b= get_variable(inst.op3);
			break;
		case 7:
			r= get_variable(inst.op1);
			g= get_variable(inst.op2);
			b= get_variable(inst.op3);
			break;
		default:
			break;
	}

	FogColor[0] = r;
	FogColor[1] = g;
	FogColor[2] = b;
}

void s_Get_Fog_Depth(script_instruction inst)
{
	if (inst.mode != 1)
		return;

	set_variable(inst.op1,FogDepth);

}

void s_Get_Fog_Color(script_instruction inst)
{
	if (inst.mode != 7)
		return;

	set_variable(inst.op1,FogColor[0]);
	set_variable(inst.op2,FogColor[1]);
	set_variable(inst.op3,FogColor[2]);

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
				dest = (int)floor(get_variable(inst.op1));
				break;
		default:
				break;
	}
	
	
	
	SET_PLAYER_TELEPORTING_STATUS(current_player, true);
	monster->action= _monster_is_teleporting;
	current_player->teleporting_phase= 0;
	current_player->delay_before_teleport= 0;
	
	current_player->teleporting_destination= dest;
	start_teleporting_effect(true);
	play_object_sound(current_player->object_index, _snd_teleport_out);
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

