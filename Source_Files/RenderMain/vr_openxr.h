/*
 *  vr_openxr.h  --  OpenXR (Meta Quest) VR backend for Aleph One. See vr_openxr.cpp.
 *
 *  Phase 2 of the Quest port. SDL-hosted: OpenXR binds to the EGL context SDL already created, and
 *  the engine keeps SDL for the main loop, input, audio and timing. The stereo frame loop replaces
 *  SDL_GL_SwapWindow. Public API is plain C; everything is a no-op off Android.
 */
#ifndef VR_OPENXR_H
#define VR_OPENXR_H

#ifdef __cplusplus
extern "C" {
#endif

// Create the OpenXR instance + system. Call once SDL video is initialised (the GL context need not
// exist yet -- the session is created lazily on the first frame). Returns true if VR is available.
bool VR_InitOpenXR(void);

// True once VR_InitOpenXR has succeeded (i.e. we are running as a VR app).
bool VR_IsActive(void);

// ---- Tunable VR comfort/scale settings (the VR preferences menu binds to this) ----
// One global, sane defaults at startup. Centralises the knobs scattered across the render seam so a
// prefs dialog can drive them without touching the renderer. Modeled on QuestZDoom's VR cvars.
typedef struct {
	int   disableBob;       // 1 = suppress camera view-bob (nausea); weapon bob unaffected
	float screenDistanceM;  // distance of the 2D UI panel (menus/terminals) in metres
	float screenHeightM;    // height of the 2D UI panel in metres (width follows its aspect)
	float worldScaleWUM;    // Marathon world-units per metre (bigger = world feels smaller)
	float eyeHeightM;       // standing HMD eye height mapped to the Marathon eye
	int   snapTurn;         // 1 = snap turning, 0 = smooth (locomotion comfort)
	float turnDegrees;      // snap: degrees per snap flick; smooth: degrees/sec at full deflection
	float brightness;       // world brightness multiply (1=unchanged; <1 dims the over-bright world)
	int   roomScale;        // 1 = body follows the head's physical movement (room-scale); 0 = head is
	                        //     a free 6DOF camera over a static body (no positional locomotion)
	int   dominantHand;     // 0 = right-handed, 1 = left-handed. The dominant hand MOVES (stick) + fires
	                        //     the primary weapon (trigger); the off-hand TURNS (stick) + secondary-fires.
	int   switchSticks;     // 1 = swap the move/turn thumbsticks (turn on the dominant hand instead)
	float aimPitchAdjust;   // degrees added to the controller aim pitch (the OpenXR aim pose sits higher
	                        //   than a held-gun barrel; negative tilts the ray DOWN). QZD's vr_weaponRotate.
} vr_settings_t;

vr_settings_t* VR_Settings(void);

// ---- Locomotion yaw offset (QuestZDoom model) ----
// The HMD yaw IS the player's facing; this offset is the extra rotation added by snap/smooth turning
// the right stick (so you can turn without physically turning your body). The engine drives the
// player's simulation facing to (this offset + head yaw), and the per-eye render rotates by this
// offset only (head yaw rides in via the eye pose). Both in Marathon angle units (512 = full circle).
float VR_GetYawOffset(void);                 // current locomotion yaw offset, angle units
void  VR_UpdateTurn(float rightStickX, float dtSeconds);  // advance snap/smooth turn from the stick
void  VR_SetYawOffset(float angleUnits);     // recenter: pin the offset (e.g. to face a start dir)

// Request the player face `facingAngleUnits` when the head is neutral (called at level entry so the
// start orientation is the level's, not wherever the headset points). Consumed once by the input
// code, which sets the yaw offset accordingly.
void VR_RequestYawRecenter(int facingAngleUnits);
bool VR_TakeYawRecenter(int* targetAngleUnits);

// Dim the currently-bound eye buffer by VR_Settings()->brightness (a fullscreen multiply pass). Call
// after the world is rendered into the eye FBO. No-op at brightness >= 1.
void VR_DimCurrentEye(void);

// Create and make-current a headless (pbuffer) GLES3 EGL context that the engine renders with.
// Immersive VR apps get no SurfaceView Surface, so SDL can't make a window GL context; we own one
// instead (the QuestZDoom/TBXR approach). Call on the engine/render thread before any GL use.
bool VR_InitEGL(void);

// Eye (per-view) framebuffer resolution chosen by the runtime; valid after the first frame. Used as
// the engine's logical screen size in VR. Returns false until known.
bool VR_GetEyeResolution(int* w, int* h);

// Head orientation from the last located HMD pose, in Marathon angle units (512 = full circle),
// signed (may be negative). yaw is the WORLD-frame azimuth contribution to ADD to the body yaw
// (derived to match the per-eye modelview's camera direction exactly); pitch is elevation (+ = up).
// The engine folds these into view->yaw/pitch (yaw = bodyYaw + this) so the visibility tree, polygon
// sort and sprite billboarding follow the head, consistent with what each eye renders. Returns false
// until a head pose has been located (output 0,0 in that case). Valid after the first VR_BeginFrame.
bool VR_GetHmdYawPitch(float* yawAngleUnits, float* pitchAngleUnits);

// ---- Controller input (OpenXR action set; Touch controllers) ----
// Latched each frame in VR_BeginFrame. Components are normalised [-1,1] / pressed booleans.
void VR_GetMove(float* x, float* y);   // left thumbstick: x=strafe, y=forward(+)

// Analog locomotion + room-scale head tracking (QuestZDoom model). VR_GetAnalogMove returns the
// deadzoned/rescaled left stick (partial deflection -> proportionally slower). VR_LatchHeadMove
// captures the head's physical movement since the last call as a world-space body delta (call once
// per real physics tick); VR_GetHeadMove returns it. The engine adds these to the player's velocity
// and position (with collision) so the body follows the head and the stick gives analog speed.
void VR_GetAnalogMove(float* strafe, float* forward);
void VR_LatchHeadMove(void);
void VR_GetHeadMove(float* x, float* y);

// Head horizontal position relative to the recenter reference, in Marathon world units (map x,y).
// The engine applies this to the VIEW ORIGIN (clamped against walls) so leaning/walking moves the
// camera + visibility origin without touching the physics position (render-side -> can't fly).
// VR_RecenterHead pins the reference to the current head (offset becomes 0 from there).
void VR_GetHeadOffset(float* wx, float* wy);
void VR_RecenterHead(void);

// Vertical eye offset from neutral standing height, in Marathon world units (negative when seated).
// Added to view->origin.z so the visibility tree uses the true eye height; subtracted in the
// Rasterizer so the rendered camera is unchanged.
float VR_GetEyeZOffset(void);
void VR_GetTurn(float* x);             // right thumbstick X: snap/smooth turn
bool VR_GetFire(void);                 // right trigger
bool VR_GetSecondaryFire(void);        // left trigger
bool VR_GetAction(void);               // A button (use terminals/switches)
bool VR_GetAdvance(void);              // A or X: advance terminal / skip cutscene
bool VR_GetBack(void);                 // Y or B: terminal page back
bool VR_GetButtonX(void);              // X button alone (in-game: previous weapon)
bool VR_GetButtonY(void);              // Y button alone (in-game: next weapon)
bool VR_GetMoveStickClick(void);       // press of the move thumbstick (in-game: run, honors toggle pref)

// Increment 1: render one head-tracked stereo test frame (a colored room) to the headset and
// submit it. Drives the OpenXR session lifecycle internally. Returns true if a VR frame was
// presented (the caller should then skip SDL_GL_SwapWindow). Returns false if VR isn't ready yet.
// Used as the per-frame fallback for non-3D frames (menus/loading) in Increment 2+.
bool VR_RenderTestFrame(void);

// ---- Increment 2: per-eye frame loop the engine's render_view drives for the real world ----
// Begin a VR frame: advance the session, wait/begin the OpenXR frame, and locate the head pose +
// per-eye views. Returns true if the world should be rendered this frame (the engine then loops
// the eyes); the frame MUST be closed with VR_SubmitFrame regardless. Returns false if VR isn't
// rendering yet (no frame begun -> don't submit).
bool VR_BeginFrame(void);

// Bind + clear eye `eye`'s swapchain framebuffer and set the viewport. Between this and
// VR_FinishEye the engine renders the world for that eye.
void VR_BeginEye(int eye);
void VR_FinishEye(int eye);

// Per-eye matrices for the bound eye. Projection is an off-axis frustum from the runtime FOV
// (near/far in metres). View is eyeFromStage (metres, Y-up) -- the engine composes the Marathon
// world transform (origin/yaw/scale, Z-up world units) onto it.
void VR_GetEyeProjection(int eye, float* proj16, float zNearMetres, float zFarMetres);
void VR_GetEyeViewMetres(int eye, float* view16);

// Submit the frame to the compositor (xrEndFrame). Pairs with VR_BeginFrame.
void VR_SubmitFrame(void);

int  VR_EyeWidth(void);
int  VR_EyeHeight(void);
unsigned VR_CurrentEyeFramebuffer(void);   // GL FBO id bound for the current eye (for the blit target)
int  VR_CurrentEye(void);                  // eye index of the most recent VR_BeginEye (for SetView)

// One VR frame must be presented per engine frame: render_view presents the world frame and marks
// it; MainScreenSwap presents the menu/loading test frame only if the world frame didn't.
void VR_MarkWorldFramePresented(void);
bool VR_TakeWorldFramePresented(void);     // returns + clears the mark

// Screen layer: the engine renders its 2D UI (menus, terminals, loading) into this offscreen
// framebuffer; VR_PresentScreenLayer then shows it to both eyes as a flat head-locked panel. This
// is how menus/UI become visible+navigable in VR (the real SurfaceView is gone).
unsigned VR_ScreenLayerFramebuffer(void);
void     VR_PresentScreenLayer(void);

// Controller menu pointer: the screen-layer pixel an aiming controller is hitting on the (world-
// locked) 2D panel, and whether to click. The engine injects SDL mouse motion/clicks from these so
// the 2D menus/terminals become interactable. Computed during VR_PresentScreenLayer.
bool VR_GetPointerScreen(int* x, int* y);
bool VR_GetPointerClick(void);
int      VR_ScreenLayerWidth(void);
int      VR_ScreenLayerHeight(void);

// Controller aim pose this frame, in STAGE space (metres, Y-up): origin (pos3) + unit forward (fwd3,
// the controller's -Z). hand 0=left, 1=right. Returns false if the pose isn't tracked. Used to draw
// the 3D-gun aim debug (controller marker + ray + hit dot) and, later, to aim weapons. The engine
// maps stage->Marathon world with the same transform Rasterizer_Shader::SetView uses, anchoring at
// the head (VR_GetHeadPosStage) so the player's absolute position in the play space cancels out.
bool VR_GetAimPoseStage(int hand, float pos3[3], float fwd3[3]);

// Head position this frame in STAGE space (metres, Y-up). The aim-debug uses controller-minus-head so
// it doesn't double-count the head offset (which the renderer applies to view.origin, not the eye matrix).
bool VR_GetHeadPosStage(float pos3[3]);

// The dominant controller's aim direction as a Marathon-world unit vector (x/y horizontal, z up) --
// the same direction the aim-debug ray uses. Weapons fire along this instead of the player facing.
// Returns false if the controller pose isn't tracked.
bool VR_GetWeaponAim(float dir[3]);

// Left-controller menu (hamburger) button press, consumed once -> the main loop opens the in-game
// quit-with-confirmation dialog.
bool VR_TakeMenuButton(void);

// True only while the OpenXR session is FOCUSED. Goes false when the user presses the Meta/home button
// and drops to the system overlay -> the engine pauses the game while unfocused.
bool VR_HasFocus(void);

#ifdef __cplusplus
}
#endif

#endif // VR_OPENXR_H
