/* script_instructions.c
	2/16/00 Created - Chris Pruett

	July 7, 2000 (Loren Petrich):
	Changed _full_screen to SizeWithoutHUD(old_size)
	so as to be more general -- one wants to be able to change to more than just the
	640*480 size.
*/

 

  
  
#include <math.h>
#include <stdlib.h>
#include <string.h>

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

#include "script_instructions.h"

/*extern'd references to the rest of the mara code */
 
extern struct view_data *world_view;
extern int current_offset;
extern boolean ready_weapon(short player_index, short weapon_index);
extern void draw_panels(void);

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

extern bool is_startup;
extern short camera_count;
extern short current_camera;
extern script_camera *cameras;

bool s_camera_Control;
short old_size;

/*-------------------------------------------*/

bool script_Camera_Active(void)
{
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
			
			world_view->origin_polygon_index= cameras[current_camera].polygon_index;
			world_view->yaw = cameras[current_camera].yaw;
			world_view->pitch = cameras[current_camera].pitch;
			world_view->origin = cameras[current_camera].position;
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
 //instruction_decay = machine_tick_count() + inst.op1;
 
 	float temp;
	
	temp = inst.op1;
	
	if (inst.op2 != 0)
		if (inst.op2 == 1)
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
	
	if (inst.op2 != 0)
		if (inst.op2 == 1)
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
	
	if (inst.op2 != 0)
		if (inst.op2 == 1)
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
	// LP change: using more general way to temporarily get rid of the HUD
	the_mode->size = SizeWithoutHUD(old_size);
	// the_mode->size = _full_screen;
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
	
	if (inst.op3 != 0)
		switch((int)floor(inst.op3))
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
	
	boolean changed= FALSE;
	float temp,temp2;
	int tag;
	
	
	temp = inst.op1;
	temp2 = inst.op2;
	
	if (inst.op3 != 0)
		switch((int)floor(inst.op3))
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
				changed= TRUE;
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
					changed= TRUE;
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
	
	if (inst.op3 == 0)
		return;
	
	switch((int)floor(inst.op3))
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
	
	if (inst.op3 == 0)
		return;
	
	switch((int)floor(inst.op3))
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
	
	if (inst.op4 > 3)
		temp = get_variable(inst.op3);
	else
		temp = inst.op3;
		
	switch((int)floor(inst.op4))
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
	
	if (inst.op3 == 0)
		return;
	
	switch((int)floor(inst.op3))
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
	stack_push(current_offset);
	s_Jump(inst);

}


void s_Return(script_instruction inst)
{
	int new_offset;
	
	new_offset = stack_pop();
	if (new_offset != -1)
		current_offset = new_offset;
}


void s_If_Greater(script_instruction inst)
{
	float temp;
	
	if (inst.op4 > 3)
		temp = get_variable(inst.op3);
	else
		temp = inst.op3;
		
	switch((int)floor(inst.op4))
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
	
	if (inst.op4 > 3)
		temp = get_variable(inst.op3);
	else
		temp = inst.op3;
		
	switch((int)floor(inst.op4))
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
	
	if (inst.op4 > 3)
		temp = get_variable(inst.op3);
	else
		temp = inst.op3;
		
	switch((int)floor(inst.op4))
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
	if (inst.op2 == 0)
		return;
		
	set_variable(inst.op1,current_player->suit_energy);

}

void s_Set_Life(script_instruction inst)
{
		
	switch((int)floor(inst.op2))
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
	if (inst.op2 == 0)
		return;
		
	set_variable(inst.op1,current_player->suit_oxygen);

}

void s_Set_Oxygen(script_instruction inst)
{
	switch((int)floor(inst.op2))
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


void s_On_Init(script_instruction inst)
{
	is_startup = true;
}

void s_End_Init(script_instruction inst)
{
	is_startup = false;
}

void s_Add_Item(script_instruction inst)
{
	switch((int)floor(inst.op2))
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
	
	switch((int)floor(inst.op2))
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
	
	switch((int)floor(inst.op2))
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
	
	memset(cameras,0,sizeof(cameras));
	
	cameras[0].polygon_index = world_view->origin_polygon_index;
	cameras[0].yaw = world_view->yaw;
	cameras[0].pitch = world_view->pitch;
	cameras[0].position = world_view->origin;
	cameras[0].index = 0;
	
}

void s_Select_Camera(script_instruction inst)
{
	int temp;
	
	switch((int)floor(inst.op2))
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
		if (cameras[current_camera].index == 0 && temp != 0)
		{
			camera_count++;
			cameras[current_camera].index = camera_count;
		}
	}
	
}

void s_Set_Camera_Poly(script_instruction inst)
{
	int temp;
	
	switch((int)floor(inst.op2))
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
		cameras[current_camera].polygon_index = temp;
}

void s_Set_Camera_Pos(script_instruction inst)
{
	float x,y,z;
		
	x= inst.op1;
	y= inst.op2;
	z= inst.op3;
				
	switch((int)floor(inst.op4))
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
		cameras[current_camera].position.x = x*WORLD_ONE;
		cameras[current_camera].position.y = y*WORLD_ONE;
		cameras[current_camera].position.z = z*WORLD_ONE;
	}
	
}

void s_Set_Camera_YP(script_instruction inst)
{
	float temp,temp2;
	
	const float AngleConvert = 360/float(FULL_CIRCLE);
	
	temp = inst.op1;
	temp2 = inst.op2;
	
	if (inst.op3 != 0)
		switch((int)floor(inst.op3))
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
		cameras[current_camera].yaw = normalize_angle(temp/AngleConvert);
		
		cameras[current_camera].pitch = normalize_angle(temp2/AngleConvert);
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