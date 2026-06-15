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
void VR_GetTurn(float* x);             // right thumbstick X: snap/smooth turn
bool VR_GetFire(void);                 // right trigger
bool VR_GetSecondaryFire(void);        // left trigger
bool VR_GetAction(void);               // A button (use terminals/switches)

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
int      VR_ScreenLayerWidth(void);
int      VR_ScreenLayerHeight(void);

#ifdef __cplusplus
}
#endif

#endif // VR_OPENXR_H
