#ifndef __ISp_Support_File
#define __ISp_Support_File
/*
	Loren Petrich: This code was originally written by Ben Thopmson;
	
July 29, 2000 (Loren Petrich):
	Rewrote it a tiny bit.
*/

// Public Function Prototypes
void initialize_ISp(void);
void ShutDown_ISp(void);
void Start_ISp(void);
void Stop_ISp(void);
long InputSprocketTestElements(void);
void ConfigureMarathonISpControls(void);

#endif