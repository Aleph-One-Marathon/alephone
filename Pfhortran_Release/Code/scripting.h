/* script.h
   Monday, Febuary 14th, 2000 (Chris Pruett)
   
 */
 #ifndef _SCRIPT_H
 #define _SCRIPT_H


 int load_script(int text_id);
 void free_script(void);
 
 void script_init(void);
 
 bool script_in_use(void);
 
 bool instruction_finished(void);

 void do_next_instruction(void);
  
 bool script_Camera_Active(void);
 
 
 #endif