// LP: not sure who originally wrote these cseries files: Bo Lindbergh?
typedef struct myTMTask myTMTask,*myTMTaskPtr;

extern myTMTaskPtr myTMSetup(
	long time,
	boolean (*func)(void));
extern myTMTaskPtr myXTMSetup(
	long time,
	boolean (*func)(void));
extern myTMTaskPtr myTMRemove(
	myTMTaskPtr task);
extern void myTMReset(
	myTMTaskPtr task);

