/* script_instructions.h
	2/16/00 Created -  Chris Pruett
	
*/


#ifndef _SCRIPT_INSTRUCTION_DEF

#define _SCRIPT_INSTRUCTION_DEF

/* define out our three possible return states in english for greater readability */
#define script_TRUE 1
#define script_FALSE 0
#define script_ERROR -1

#include "cstypes.h"
#include "world.h"

struct script_instruction
{
	short opcode;		
	float op1;		/*operands 1 thru 4 */
	float op2;
	float op3;
	float op4;

};

struct script_camera
{
	short index;
	world_point3d position;
	short polygon_index;
	angle yaw;
	angle pitch;

};


 

 
 
enum /* instruction defs */
{
	Camera_Move = 0x01,
	Camera_Look = 0x02,

	Wait_Ticks = 0x03,

	Inflict_Dammage = 0x04,

	Jump = 0x05,

	Enable_Player = 0x06,

	Disable_Player = 0x07,

	Script_End = 0x08,

	Hide_Interface = 0x09,

	Show_Interface = 0x0A,

	Set_Tag_State = 0x0B,

	Get_Tag_State = 0x0C,

	Define = 0x0D,

	sAdd = 0x0E,

	sSubtract = 0x0F,

	If_Equal = 0x10,

	Set = 0x11,

	Call = 0x12,

	Return = 0x13,

	If_Greater = 0x14,

	If_Less = 0x15,

	If_Not_Equal = 0x16,

	Get_Life = 0x17,

	Set_Life = 0x18,

	Get_Oxygen = 0x19,

	Set_Oxygen = 0x1A,

	On_Init = 0x1B,

	End_Init = 0x1C,

	Add_Item = 0x1D,

	Select_Weapon = 0x1E,

	Block_Start = 0x1F,

	Block_End = 0x20,
	
	Init_Cameras = 0x21,
	
	Select_Camera = 0x22,
	
	Set_Camera_Poly = 0x23,
	
	Set_Camera_Pos = 0x24,
	
	Set_Camera_YP = 0x25,
	
	Activate_Camera = 0x26,
	
	Use_Camera = 0x27,
	
	

	NUMBER_OF_INSTRUCTIONS
};

 
 




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

void s_On_Init(script_instruction inst);
void s_End_Init(script_instruction inst);
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
#endif