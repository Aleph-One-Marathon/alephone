typedef struct GDSpec {
	short slot;
	short flags;
	short bit_depth;
	short width;
	short height;
} GDSpec,*GDSpecPtr;

extern GDHandle BestDevice(
	GDSpecPtr spec);
extern GDHandle MatchGDSpec(
	GDSpecPtr spec);
extern void SetDepthGDSpec(
	GDSpecPtr spec);
extern void BuildGDSpec(
	GDSpecPtr spec,
	GDHandle dev);
extern Boolean HasDepthGDSpec(
	GDSpecPtr spec);
extern Boolean EqualGDSpec(
	GDSpecPtr spec1,
	GDSpecPtr spec2);

extern short GetSlotFromGDevice(
	GDHandle dev);

extern void display_device_dialog(
	GDSpecPtr spec);
