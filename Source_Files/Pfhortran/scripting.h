/* script.h
   Monday, Febuary 14th, 2000 (Chris Pruett)
   
 */
 #ifndef _SCRIPT_H
 #define _SCRIPT_H


 int load_script(int text_id);
 void free_script(void);
 
 void script_init(void);
 
 bool script_in_use(void);
 
 /*bool instruction_finished(void);*/

 void do_next_instruction(void);
  
 bool script_Camera_Active(void);
 
 void activate_tag_switch_trap(int which);
 void activate_light_switch_trap(int which);
 void activate_platform_switch_trap(int which);
 void activate_terminal_enter_trap(int which);
 void activate_terminal_exit_trap(int which);
 void activate_pattern_buffer_trap(int which);
 void activate_got_item_trap(int which);
 
 #endif