/*
 *  vr_openxr.cpp  --  OpenXR (Meta Quest) VR backend for Aleph One. See vr_openxr.h.
 *
 *  SDL-hosted: binds OpenXR to SDL's EGL context rather than creating its own (the QuestZDoom/TBXR
 *  approach owns the activity+thread+EGL, which collides with Aleph One's SDL platform layer).
 *  Adapted from TBXR_Common.cpp (DrBeef), restructured to run on SDL's context + main thread.
 *
 *  Increment 1: bring up session + swapchains + the stereo frame loop and render a head-tracked
 *  colored room to both eyes. This proves the whole immersive pipeline (session lifecycle, per-eye
 *  swapchain FBOs, projection/view matrices, head tracking, compositor submit) before the Marathon
 *  scene is wired in per-eye (Increment 2).
 */

#if defined(__ANDROID__)

#define XR_USE_GRAPHICS_API_OPENGL_ES 1
#define XR_USE_PLATFORM_ANDROID 1

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>
#include <jni.h>
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>
#include <android/log.h>
#include <SDL2/SDL.h>
#include <cstring>
#include <cmath>

#include "vr_openxr.h"

#define A1VR_LOG(...) __android_log_print(ANDROID_LOG_INFO, "A1VR", __VA_ARGS__)
#define XR_CHECK(call) do { XrResult _r = (call); if (XR_FAILED(_r)) A1VR_LOG("%s -> %d", #call, _r); } while (0)

// EXT_sRGB_write_control: lets us store our already-sRGB content verbatim into the sRGB eye swapchain
// (without GL re-encoding linear->sRGB, which over-brightens the legacy renderer's output + the 2D UI).
#ifndef GL_FRAMEBUFFER_SRGB_EXT
#define GL_FRAMEBUFFER_SRGB_EXT 0x8DB9
#endif

namespace {

constexpr int kEyes = 2;

XrInstance  s_instance = XR_NULL_HANDLE;
XrSystemId  s_systemId = XR_NULL_SYSTEM_ID;
bool        s_active   = false;          // instance + system created

// Session-level state (created lazily once the GL context exists).
XrSession      s_session = XR_NULL_HANDLE;
XrSpace        s_stageSpace = XR_NULL_HANDLE;   // world reference (metres)
XrSpace        s_headSpace  = XR_NULL_HANDLE;   // VIEW space (for locating eyes)
XrSessionState s_sessionState = XR_SESSION_STATE_UNKNOWN;
bool           s_sessionRunning = false;
bool           s_sessionCreated = false;

XrViewConfigurationView s_viewConfig[kEyes] = {};
int s_eyeW = 0, s_eyeH = 0;

struct EyeFB {
	XrSwapchain                  swapchain = XR_NULL_HANDLE;
	uint32_t                     imageCount = 0;
	XrSwapchainImageOpenGLESKHR* images = nullptr;
	GLuint*                      fbos = nullptr;
	GLuint                       depth = 0;   // shared depth renderbuffer (eye-sized)
};
EyeFB s_eye[kEyes];

XrFrameState s_frameState = {};
XrView       s_views[kEyes] = {};
XrPosef      s_stageFromHead = {};
bool         s_headPoseValid = false;   // s_stageFromHead orientation has been located at least once

// Room-scale head tracking: s_lastHead* is the "absorbed" head origin (stage metres, horizontal) --
// the point the player body has been moved up to. The render uses the head/eye position RELATIVE to
// this (the residual), and each physics tick the residual is fed to the body and this advances to the
// current head, so the body follows the head without the render double-counting the movement.
float        s_lastHeadX = 0, s_lastHeadZ = 0;
bool         s_headLatchInit = false;
float        s_headMoveX = 0, s_headMoveY = 0;   // latched per-tick body delta, world units (map x,y)
// Recenter reference (stage metres, horizontal): the head's position is reported RELATIVE to this, so
// the view lean/walk offset starts at 0 wherever the player is standing when recentered.
float        s_headRefX = 0, s_headRefZ = 0;
bool         s_headRefInit = false;

// Engine-owned headless EGL context (no window surface).
EGLDisplay s_eglDpy  = EGL_NO_DISPLAY;
EGLContext s_eglCtx  = EGL_NO_CONTEXT;
EGLSurface s_eglSurf = EGL_NO_SURFACE;
bool       s_eglReady = false;

int        s_srgbWriteControl = -1;   // -1 unknown, 0 absent, 1 present (EXT_sRGB_write_control)

bool hasGLExtension(const char* name) {
	GLint n = 0; glGetIntegerv(GL_NUM_EXTENSIONS, &n);
	for (GLint i = 0; i < n; ++i) {
		const char* e = (const char*)glGetStringi(GL_EXTENSIONS, i);
		if (e && std::strcmp(e, name) == 0) return true;
	}
	return false;
}

// --- test-scene GL objects ---
GLuint s_prog = 0, s_vao = 0, s_vbo = 0;
GLint  s_uProj = -1, s_uView = -1;
int    s_vertCount = 0;

// ------------------------------------------------------------------ math ----
// Column-major 4x4, GL convention.
void mat_identity(float* m) { std::memset(m, 0, 16 * sizeof(float)); m[0]=m[5]=m[10]=m[15]=1.0f; }

void mat_mul(float* out, const float* a, const float* b) {
	float r[16];
	for (int c = 0; c < 4; ++c)
		for (int row = 0; row < 4; ++row) {
			float s = 0;
			for (int k = 0; k < 4; ++k) s += a[k*4+row] * b[c*4+k];
			r[c*4+row] = s;
		}
	std::memcpy(out, r, sizeof r);
}

// OpenXR off-axis projection (tangent-based) -> column-major GL matrix, clip z in [-1,1].
void proj_from_fov(float* m, const XrFovf& fov, float n, float f) {
	const float l = std::tan(fov.angleLeft),  r = std::tan(fov.angleRight);
	const float d = std::tan(fov.angleDown),  u = std::tan(fov.angleUp);
	const float w = r - l, h = u - d;
	std::memset(m, 0, 16 * sizeof(float));
	m[0]  = 2.0f / w;
	m[5]  = 2.0f / h;
	m[8]  = (r + l) / w;
	m[9]  = (u + d) / h;
	m[10] = -(f + n) / (f - n);
	m[11] = -1.0f;
	m[14] = -(2.0f * f * n) / (f - n);
}

// Rigid transform from an XrPosef (quat + translation) -> column-major matrix.
void mat_from_pose(float* m, const XrPosef& p) {
	const float x = p.orientation.x, y = p.orientation.y, z = p.orientation.z, w = p.orientation.w;
	const float xx=x*x, yy=y*y, zz=z*z, xy=x*y, xz=x*z, yz=y*z, wx=w*x, wy=w*y, wz=w*z;
	m[0]=1-2*(yy+zz); m[1]=2*(xy+wz);   m[2]=2*(xz-wy);   m[3]=0;
	m[4]=2*(xy-wz);   m[5]=1-2*(xx+zz); m[6]=2*(yz+wx);   m[7]=0;
	m[8]=2*(xz+wy);   m[9]=2*(yz-wx);   m[10]=1-2*(xx+yy);m[11]=0;
	m[12]=p.position.x; m[13]=p.position.y; m[14]=p.position.z; m[15]=1;
}

// Inverse of a rigid (rotation+translation) matrix.
void mat_rigid_inverse(float* out, const float* m) {
	// transpose rotation
	out[0]=m[0]; out[1]=m[4]; out[2]=m[8];  out[3]=0;
	out[4]=m[1]; out[5]=m[5]; out[6]=m[9];  out[7]=0;
	out[8]=m[2]; out[9]=m[6]; out[10]=m[10];out[11]=0;
	// -R^T * t
	const float tx=m[12], ty=m[13], tz=m[14];
	out[12] = -(out[0]*tx + out[4]*ty + out[8]*tz);
	out[13] = -(out[1]*tx + out[5]*ty + out[9]*tz);
	out[14] = -(out[2]*tx + out[6]*ty + out[10]*tz);
	out[15] = 1;
}

XrPosef pose_mul(const XrPosef& a, const XrPosef& b) {
	// a * b for quaternion+translation poses
	XrPosef o;
	const auto& qa = a.orientation; const auto& qb = b.orientation;
	o.orientation.w = qa.w*qb.w - qa.x*qb.x - qa.y*qb.y - qa.z*qb.z;
	o.orientation.x = qa.w*qb.x + qa.x*qb.w + qa.y*qb.z - qa.z*qb.y;
	o.orientation.y = qa.w*qb.y - qa.x*qb.z + qa.y*qb.w + qa.z*qb.x;
	o.orientation.z = qa.w*qb.z + qa.x*qb.y - qa.y*qb.x + qa.z*qb.w;
	// rotate b.position by a.orientation, add a.position
	float m[16]; mat_from_pose(m, a);
	const float px=b.position.x, py=b.position.y, pz=b.position.z;
	o.position.x = m[0]*px + m[4]*py + m[8]*pz  + a.position.x;
	o.position.y = m[1]*px + m[5]*py + m[9]*pz  + a.position.y;
	o.position.z = m[2]*px + m[6]*py + m[10]*pz + a.position.z;
	return o;
}

// ----------------------------------------------------------- test scene ----
GLuint compile(GLenum type, const char* src) {
	GLuint s = glCreateShader(type);
	glShaderSource(s, 1, &src, nullptr);
	glCompileShader(s);
	GLint ok = 0; glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
	if (!ok) { char log[512]; glGetShaderInfoLog(s, sizeof log, nullptr, log); A1VR_LOG("test shader: %s", log); }
	return s;
}

void buildTestScene() {
	if (s_prog) return;
	static const char* vs =
		"#version 300 es\n"
		"layout(location=0) in vec3 aPos;\n"
		"layout(location=1) in vec3 aColor;\n"
		"uniform mat4 uProj; uniform mat4 uView;\n"
		"out vec3 vColor;\n"
		"void main(){ gl_Position = uProj * uView * vec4(aPos,1.0); vColor = aColor; }\n";
	static const char* fs =
		"#version 300 es\n"
		"precision mediump float;\n"
		"in vec3 vColor; out vec4 o;\n"
		"void main(){ o = vec4(vColor,1.0); }\n";
	GLuint v = compile(GL_VERTEX_SHADER, vs), f = compile(GL_FRAGMENT_SHADER, fs);
	s_prog = glCreateProgram();
	glAttachShader(s_prog, v); glAttachShader(s_prog, f); glLinkProgram(s_prog);
	glDeleteShader(v); glDeleteShader(f);
	s_uProj = glGetUniformLocation(s_prog, "uProj");
	s_uView = glGetUniformLocation(s_prog, "uView");

	// A 6m room (metres, Y up) centred on the player at eye height ~1.6m, each wall a distinct
	// colour so head rotation is unambiguous; floor a grid-ish dark, ceiling light.
	const float R = 3.0f, F = 0.0f, C = 3.0f;     // half-extent, floor y, ceiling y
	auto quad = [&](float* p, const float* a, const float* b, const float* c, const float* d, float r, float g, float bl) {
		const float verts[6][3] = { {a[0],a[1],a[2]},{b[0],b[1],b[2]},{c[0],c[1],c[2]},
									{a[0],a[1],a[2]},{c[0],c[1],c[2]},{d[0],d[1],d[2]} };
		for (int i = 0; i < 6; ++i) { *p++=verts[i][0]; *p++=verts[i][1]; *p++=verts[i][2]; *p++=r; *p++=g; *p++=bl; }
	};
	float data[6 * 6 * 6]; float* p = data;
	const float fnl[3]={-R,F,-R}, fnr[3]={ R,F,-R}, ffr[3]={ R,F, R}, ffl[3]={-R,F, R};
	const float cnl[3]={-R,C,-R}, cnr[3]={ R,C,-R}, cfr[3]={ R,C, R}, cfl[3]={-R,C, R};
	quad(p+0*36,  fnl,fnr,ffr,ffl, 0.12f,0.12f,0.16f);  // floor
	quad(p+1*36,  cnl,cfl,cfr,cnr, 0.55f,0.55f,0.60f);  // ceiling
	quad(p+2*36,  fnl,cnl,cnr,fnr, 0.80f,0.20f,0.20f);  // -Z wall (red, front)
	quad(p+3*36,  ffr,cfr,cfl,ffl, 0.20f,0.80f,0.20f);  // +Z wall (green, behind)
	quad(p+4*36,  ffl,cfl,cnl,fnl, 0.20f,0.30f,0.85f);  // -X wall (blue, left)
	quad(p+5*36,  fnr,cnr,cfr,ffr, 0.85f,0.75f,0.20f);  // +X wall (yellow, right)
	s_vertCount = 6 * 6;

	glGenVertexArrays(1, &s_vao);
	glGenBuffers(1, &s_vbo);
	glBindVertexArray(s_vao);
	glBindBuffer(GL_ARRAY_BUFFER, s_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof data, data, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));
	glBindVertexArray(0);
}

void drawTestScene(const float* proj, const float* view) {
	buildTestScene();
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);   // we're inside the room
	glUseProgram(s_prog);
	glUniformMatrix4fv(s_uProj, 1, GL_FALSE, proj);
	glUniformMatrix4fv(s_uView, 1, GL_FALSE, view);
	glBindVertexArray(s_vao);
	glDrawArrays(GL_TRIANGLES, 0, s_vertCount);
	glBindVertexArray(0);
}

// --------------------------------------------------------- session setup ----
EGLConfig currentEglConfig(EGLDisplay dpy, EGLContext ctx) {
	EGLint cfgId = 0;
	eglQueryContext(dpy, ctx, EGL_CONFIG_ID, &cfgId);
	EGLConfig cfg = nullptr; EGLint n = 0;
	const EGLint attribs[] = { EGL_CONFIG_ID, cfgId, EGL_NONE };
	eglChooseConfig(dpy, attribs, &cfg, 1, &n);
	return cfg;
}

bool createSwapchains() {
	uint32_t count = 0;
	for (int i = 0; i < kEyes; ++i) s_viewConfig[i].type = XR_TYPE_VIEW_CONFIGURATION_VIEW;
	XR_CHECK(xrEnumerateViewConfigurationViews(s_instance, s_systemId,
		XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, kEyes, &count, s_viewConfig));
	s_eyeW = s_viewConfig[0].recommendedImageRectWidth;
	s_eyeH = s_viewConfig[0].recommendedImageRectHeight;
	A1VR_LOG("eye resolution %dx%d", s_eyeW, s_eyeH);

	// Pick an sRGB colour format (GL_SRGB8_ALPHA8), matching TBXR/QuestZDoom. The Quest compositor
	// interprets the swapchain in this colour space; handing it a linear RGBA8 made the world far too
	// bright (the runtime applied an extra linear->sRGB brighten). With an sRGB swapchain the
	// compositor samples correctly. Fall back to RGBA8 only if sRGB isn't offered.
	uint32_t fmtCount = 0;
	xrEnumerateSwapchainFormats(s_session, 0, &fmtCount, nullptr);
	int64_t* fmts = new int64_t[fmtCount];
	xrEnumerateSwapchainFormats(s_session, fmtCount, &fmtCount, fmts);
	int64_t chosen = fmts[0];
	bool haveSrgb = false, haveRgba8 = false;
	for (uint32_t i = 0; i < fmtCount; ++i) {
		if (fmts[i] == GL_SRGB8_ALPHA8) haveSrgb = true;
		if (fmts[i] == GL_RGBA8)        haveRgba8 = true;
	}
	if (haveSrgb)       chosen = GL_SRGB8_ALPHA8;
	else if (haveRgba8) chosen = GL_RGBA8;
	delete[] fmts;

	for (int e = 0; e < kEyes; ++e) {
		XrSwapchainCreateInfo ci = { XR_TYPE_SWAPCHAIN_CREATE_INFO };
		ci.usageFlags = XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT | XR_SWAPCHAIN_USAGE_SAMPLED_BIT;
		ci.format = chosen;
		ci.sampleCount = 1;
		ci.width = s_eyeW; ci.height = s_eyeH;
		ci.faceCount = 1; ci.arraySize = 1; ci.mipCount = 1;
		if (XR_FAILED(xrCreateSwapchain(s_session, &ci, &s_eye[e].swapchain))) { A1VR_LOG("xrCreateSwapchain eye %d failed", e); return false; }

		xrEnumerateSwapchainImages(s_eye[e].swapchain, 0, &s_eye[e].imageCount, nullptr);
		s_eye[e].images = new XrSwapchainImageOpenGLESKHR[s_eye[e].imageCount];
		for (uint32_t i = 0; i < s_eye[e].imageCount; ++i) s_eye[e].images[i].type = XR_TYPE_SWAPCHAIN_IMAGE_OPENGL_ES_KHR;
		xrEnumerateSwapchainImages(s_eye[e].swapchain, s_eye[e].imageCount, &s_eye[e].imageCount,
								   (XrSwapchainImageBaseHeader*)s_eye[e].images);

		glGenRenderbuffers(1, &s_eye[e].depth);
		glBindRenderbuffer(GL_RENDERBUFFER, s_eye[e].depth);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, s_eyeW, s_eyeH);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		s_eye[e].fbos = new GLuint[s_eye[e].imageCount];
		glGenFramebuffers(s_eye[e].imageCount, s_eye[e].fbos);
		for (uint32_t i = 0; i < s_eye[e].imageCount; ++i) {
			glBindFramebuffer(GL_FRAMEBUFFER, s_eye[e].fbos[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, s_eye[e].images[i].image, 0);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, s_eye[e].depth);
			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
				A1VR_LOG("eye %d fbo %u incomplete", e, i);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	return true;
}

void initActions();   // defined below (input action set); attached once the session exists

bool startSession() {
	if (s_sessionCreated) return true;

	EGLDisplay dpy = eglGetCurrentDisplay();
	EGLContext ctx = eglGetCurrentContext();
	if (dpy == EGL_NO_DISPLAY || ctx == EGL_NO_CONTEXT) {
		// SDL's GL context isn't current yet (window not created); try again next frame.
		return false;
	}

	// OpenXR requires the graphics requirements call before creating the session.
	PFN_xrGetOpenGLESGraphicsRequirementsKHR pfnReq = nullptr;
	xrGetInstanceProcAddr(s_instance, "xrGetOpenGLESGraphicsRequirementsKHR", (PFN_xrVoidFunction*)&pfnReq);
	if (pfnReq) {
		XrGraphicsRequirementsOpenGLESKHR req = { XR_TYPE_GRAPHICS_REQUIREMENTS_OPENGL_ES_KHR };
		pfnReq(s_instance, s_systemId, &req);
	}

	XrGraphicsBindingOpenGLESAndroidKHR gb = { XR_TYPE_GRAPHICS_BINDING_OPENGL_ES_ANDROID_KHR };
	gb.display = dpy;
	gb.config  = currentEglConfig(dpy, ctx);
	gb.context = ctx;

	XrSessionCreateInfo sci = { XR_TYPE_SESSION_CREATE_INFO };
	sci.next = &gb;
	sci.systemId = s_systemId;
	if (XR_FAILED(xrCreateSession(s_instance, &sci, &s_session))) { A1VR_LOG("xrCreateSession failed"); return false; }

	XrReferenceSpaceCreateInfo head = { XR_TYPE_REFERENCE_SPACE_CREATE_INFO };
	head.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_VIEW;
	head.poseInReferenceSpace.orientation.w = 1.0f;
	XR_CHECK(xrCreateReferenceSpace(s_session, &head, &s_headSpace));

	// Prefer STAGE (floor-level world); fall back to LOCAL.
	XrReferenceSpaceCreateInfo stage = { XR_TYPE_REFERENCE_SPACE_CREATE_INFO };
	stage.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_STAGE;
	stage.poseInReferenceSpace.orientation.w = 1.0f;
	if (XR_FAILED(xrCreateReferenceSpace(s_session, &stage, &s_stageSpace))) {
		stage.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_LOCAL;
		XR_CHECK(xrCreateReferenceSpace(s_session, &stage, &s_stageSpace));
		A1VR_LOG("using LOCAL space (no stage)");
	}

	if (!createSwapchains()) return false;
	initActions();
	s_sessionCreated = true;
	A1VR_LOG("session + swapchains created");
	return true;
}

void pollEvents() {
	XrEventDataBuffer ev = { XR_TYPE_EVENT_DATA_BUFFER };
	while (xrPollEvent(s_instance, &ev) == XR_SUCCESS) {
		if (ev.type == XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED) {
			auto* ssc = reinterpret_cast<XrEventDataSessionStateChanged*>(&ev);
			s_sessionState = ssc->state;
			A1VR_LOG("session state -> %d", s_sessionState);
			if (s_sessionState == XR_SESSION_STATE_READY) {
				XrSessionBeginInfo bi = { XR_TYPE_SESSION_BEGIN_INFO };
				bi.primaryViewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
				XR_CHECK(xrBeginSession(s_session, &bi));
				s_sessionRunning = true;
			} else if (s_sessionState == XR_SESSION_STATE_STOPPING) {
				XR_CHECK(xrEndSession(s_session));
				s_sessionRunning = false;
			}
		}
		ev.type = XR_TYPE_EVENT_DATA_BUFFER;
	}
}

} // namespace

// ============================================================================
extern "C" bool VR_InitOpenXR(void)
{
	if (s_active) return true;

	JNIEnv* env       = static_cast<JNIEnv*>(SDL_AndroidGetJNIEnv());
	jobject actLocal  = static_cast<jobject>(SDL_AndroidGetActivity());
	JavaVM* vm        = nullptr;
	if (env) env->GetJavaVM(&vm);
	if (!vm || !actLocal) { A1VR_LOG("VR_InitOpenXR: no JavaVM/activity"); return false; }

	// SDL_AndroidGetActivity returns a LOCAL ref; hold a GLOBAL ref for the runtime's lifetime.
	// IMPORTANT: this must be the actual Activity (not the application Context) -- the runtime uses
	// XrInstanceCreateInfoAndroidKHR.applicationActivity to track the activity lifecycle; passing the
	// app context instead leaves the session stuck in XR_SESSION_STATE_IDLE.
	static jobject s_activityGlobal = nullptr;
	if (!s_activityGlobal) s_activityGlobal = env->NewGlobalRef(actLocal);
	jobject activity = s_activityGlobal;

	PFN_xrInitializeLoaderKHR xrInitializeLoaderKHR = nullptr;
	xrGetInstanceProcAddr(XR_NULL_HANDLE, "xrInitializeLoaderKHR",
						  reinterpret_cast<PFN_xrVoidFunction*>(&xrInitializeLoaderKHR));
	if (xrInitializeLoaderKHR) {
		XrLoaderInitInfoAndroidKHR lii{};
		lii.type = XR_TYPE_LOADER_INIT_INFO_ANDROID_KHR;
		lii.applicationVM = vm;
		lii.applicationContext = activity;
		XrResult lr = xrInitializeLoaderKHR(reinterpret_cast<const XrLoaderInitInfoBaseHeaderKHR*>(&lii));
		if (XR_FAILED(lr)) A1VR_LOG("VR_InitOpenXR: xrInitializeLoaderKHR failed (%d)", lr);
	} else {
		A1VR_LOG("VR_InitOpenXR: xrInitializeLoaderKHR not found");
	}

	const char* extensions[] = {
		XR_KHR_ANDROID_CREATE_INSTANCE_EXTENSION_NAME,
		XR_KHR_OPENGL_ES_ENABLE_EXTENSION_NAME,
	};
	XrInstanceCreateInfoAndroidKHR androidInfo{};
	androidInfo.type = XR_TYPE_INSTANCE_CREATE_INFO_ANDROID_KHR;
	androidInfo.applicationVM = vm;
	androidInfo.applicationActivity = activity;

	XrInstanceCreateInfo ici{};
	ici.type = XR_TYPE_INSTANCE_CREATE_INFO;
	ici.next = &androidInfo;
	std::strcpy(ici.applicationInfo.applicationName, "Aleph One");
	std::strcpy(ici.applicationInfo.engineName, "Aleph One");
	ici.applicationInfo.apiVersion = XR_CURRENT_API_VERSION;
	ici.enabledExtensionCount = sizeof(extensions) / sizeof(extensions[0]);
	ici.enabledExtensionNames = extensions;

	if (XR_FAILED(xrCreateInstance(&ici, &s_instance))) {
		A1VR_LOG("VR_InitOpenXR: xrCreateInstance failed (runtime unavailable?)");
		return false;
	}
	XrInstanceProperties ip{}; ip.type = XR_TYPE_INSTANCE_PROPERTIES;
	xrGetInstanceProperties(s_instance, &ip);
	A1VR_LOG("VR_InitOpenXR: runtime '%s'", ip.runtimeName);

	XrSystemGetInfo sgi{}; sgi.type = XR_TYPE_SYSTEM_GET_INFO;
	sgi.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;
	if (XR_FAILED(xrGetSystem(s_instance, &sgi, &s_systemId))) { A1VR_LOG("VR_InitOpenXR: xrGetSystem failed"); return false; }

	s_active = true;
	A1VR_LOG("VR_InitOpenXR: OK");
	return true;
}

extern "C" bool VR_IsActive(void) { return s_active; }

namespace {
	vr_settings_t s_settings = {
		/* disableBob      */ 1,
		/* screenDistanceM */ 2.5f,
		/* screenHeightM   */ 2.0f,
		/* worldScaleWUM   */ 512.0f,
		/* eyeHeightM      */ 1.6f,
		/* snapTurn        */ 1,
		/* turnDegrees     */ 30.0f,
		/* brightness      */ 1.0f,   // neutral; real fix is the sRGB write-control below
		/* roomScale       */ 0,      // OFF: body-follows-head conflicts with Marathon's tick/interpolation
		                            //      (camera lurches per tick). Stable 6DOF head view + stick for now.
		/* dominantHand    */ 0,      // right-handed
		/* switchSticks    */ 0,
		/* aimPitchAdjust  */ -20.0f, // aim pose sits ~20deg above a held-gun barrel; tilt down
	};

	// Locomotion yaw offset (snap/smooth turn), in Marathon angle units (512 = full circle).
	float s_yawOffset = 0.0f;
	bool  s_turnArmed = true;   // snap-turn edge latch (re-armed when the stick recentres)
	const float kAngleUnitsPerDeg = 512.0f / 360.0f;

	// Yaw recenter request (set at level entry): make the player face `s_yawRecenterTarget` when the
	// head is neutral, regardless of where the headset is pointing at spawn.
	bool s_yawRecenterPending = false;
	int  s_yawRecenterTarget  = 0;
}

extern "C" void VR_RequestYawRecenter(int facingAngleUnits)
{
	s_yawRecenterTarget = facingAngleUnits;
	s_yawRecenterPending = true;
}

// If a yaw recenter is pending, return its target facing (angle units) and clear it; else false.
extern "C" bool VR_TakeYawRecenter(int* targetAngleUnits)
{
	if (!s_yawRecenterPending) return false;
	s_yawRecenterPending = false;
	if (targetAngleUnits) *targetAngleUnits = s_yawRecenterTarget;
	return true;
}
extern "C" vr_settings_t* VR_Settings(void) { return &s_settings; }

extern "C" float VR_GetYawOffset(void)        { return s_yawOffset; }
extern "C" void  VR_SetYawOffset(float a)     { s_yawOffset = a; }

extern "C" void VR_UpdateTurn(float stickX, float dt)
{
	// Right stick turns WITHOUT physically turning your body. Snap right (+X) = turn the view right,
	// which in Marathon's CCW angle convention means DECREASING yaw. (Flip the sign here if turning
	// goes the wrong way on device.)
	if (s_settings.snapTurn) {
		const float thresh = 0.6f, release = 0.3f;
		if (s_turnArmed && stickX > thresh)  { s_yawOffset += s_settings.turnDegrees * kAngleUnitsPerDeg; s_turnArmed = false; }
		else if (s_turnArmed && stickX < -thresh) { s_yawOffset -= s_settings.turnDegrees * kAngleUnitsPerDeg; s_turnArmed = false; }
		else if (stickX > -release && stickX < release) s_turnArmed = true;
	} else {
		// Smooth turn: turnDegrees is deg/sec at full deflection, with a small deadzone.
		const float dead = 0.2f;
		if (stickX > dead || stickX < -dead)
			s_yawOffset += stickX * s_settings.turnDegrees * kAngleUnitsPerDeg * dt;
	}
	// keep bounded to one turn for numeric tidiness
	while (s_yawOffset >= 512.0f) s_yawOffset -= 512.0f;
	while (s_yawOffset <  0.0f)   s_yawOffset += 512.0f;
}

extern "C" bool VR_GetEyeResolution(int* w, int* h)
{
	if (s_eyeW <= 0 || s_eyeH <= 0) return false;
	if (w) *w = s_eyeW; if (h) *h = s_eyeH;
	return true;
}

extern "C" bool VR_InitEGL(void)
{
	if (s_eglReady) return true;

	s_eglDpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	if (s_eglDpy == EGL_NO_DISPLAY) { A1VR_LOG("VR_InitEGL: no display"); return false; }
	EGLint major = 0, minor = 0;
	if (!eglInitialize(s_eglDpy, &major, &minor)) { A1VR_LOG("VR_InitEGL: eglInitialize failed"); return false; }

	const EGLint cfgAttribs[] = {
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR,
		EGL_SURFACE_TYPE,    EGL_PBUFFER_BIT,
		EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8, EGL_ALPHA_SIZE, 8,
		EGL_DEPTH_SIZE, 24, EGL_STENCIL_SIZE, 8,
		EGL_NONE
	};
	EGLConfig cfg = nullptr; EGLint n = 0;
	if (!eglChooseConfig(s_eglDpy, cfgAttribs, &cfg, 1, &n) || n < 1) { A1VR_LOG("VR_InitEGL: no config"); return false; }

	const EGLint ctxAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE };
	s_eglCtx = eglCreateContext(s_eglDpy, cfg, EGL_NO_CONTEXT, ctxAttribs);
	if (s_eglCtx == EGL_NO_CONTEXT) { A1VR_LOG("VR_InitEGL: eglCreateContext failed"); return false; }

	const EGLint pbAttribs[] = { EGL_WIDTH, 16, EGL_HEIGHT, 16, EGL_NONE };
	s_eglSurf = eglCreatePbufferSurface(s_eglDpy, cfg, pbAttribs);
	if (s_eglSurf == EGL_NO_SURFACE) { A1VR_LOG("VR_InitEGL: eglCreatePbufferSurface failed"); return false; }

	if (!eglMakeCurrent(s_eglDpy, s_eglSurf, s_eglSurf, s_eglCtx)) { A1VR_LOG("VR_InitEGL: eglMakeCurrent failed"); return false; }

	s_eglReady = true;
	A1VR_LOG("VR_InitEGL: pbuffer GLES3 context current (EGL %d.%d, GL_VERSION=%s)", major, minor,
			 (const char*)glGetString(GL_VERSION));
	return true;
}

// ---- frame-scoped state (between VR_BeginFrame and VR_SubmitFrame) ----
namespace {
	XrCompositionLayerProjectionView s_layerViews[kEyes] = {};
	XrPosef  s_stageFromEye[kEyes] = {};
	bool     s_frameBegun       = false;
	bool     s_frameShouldRender = false;
	uint32_t s_curEyeImageIdx   = 0;
	int      s_curEye           = 0;

	// ---- input (OpenXR action set, Touch controllers) ----
	XrActionSet s_actionSet  = XR_NULL_HANDLE;
	// Per-hand sticks/triggers (index 0=left, 1=right) so handedness/stick-switch can be routed in
	// software each frame without re-attaching the action set when the setting changes.
	XrAction    s_stickAction[2]   = { XR_NULL_HANDLE, XR_NULL_HANDLE };
	XrAction    s_stickClickAction[2] = { XR_NULL_HANDLE, XR_NULL_HANDLE };   // thumbstick press (L3/R3)
	XrAction    s_triggerAction[2] = { XR_NULL_HANDLE, XR_NULL_HANDLE };
	XrAction    s_actionAction = XR_NULL_HANDLE;
	XrAction    s_bAction = XR_NULL_HANDLE, s_xAction = XR_NULL_HANDLE, s_yAction = XR_NULL_HANDLE;
	XrAction    s_menuAction = XR_NULL_HANDLE;   // left controller hamburger/menu button -> in-game quit
	XrAction    s_aimAction[2] = { XR_NULL_HANDLE, XR_NULL_HANDLE };  // 0=left, 1=right, pointing pose
	XrSpace     s_aimSpace[2]  = { XR_NULL_HANDLE, XR_NULL_HANDLE };
	XrPosef     s_aimStage[2]  = {};   // aim pose in stage space, this frame
	bool        s_aimValid[2]  = { false, false };
	bool        s_actionsReady = false;
	float       s_stickX[2] = {0,0}, s_stickY[2] = {0,0}, s_trigger[2] = {0,0};   // raw per-hand
	bool        s_stickClick[2] = {false, false};   // raw per-hand thumbstick press
	float       s_moveX = 0, s_moveY = 0, s_turnX = 0;   // routed (handedness/switch-sticks applied)
	bool        s_fire = false, s_altFire = false, s_action = false;
	bool        s_bBtn = false, s_xBtn = false, s_yBtn = false;
	bool        s_advancePrev = false;   // edge latch for cutscene-skip key injection
	bool        s_menuBtn = false, s_menuPrev = false;   // menu button + edge latch
	bool        s_menuLatch = false;     // set on menu-button press, consumed by VR_TakeMenuButton

	XrPath path(const char* s) { XrPath p = XR_NULL_PATH; xrStringToPath(s_instance, s, &p); return p; }

	void initActions() {
		if (s_actionsReady) return;

		XrActionSetCreateInfo asci = { XR_TYPE_ACTION_SET_CREATE_INFO };
		std::strcpy(asci.actionSetName, "gameplay");
		std::strcpy(asci.localizedActionSetName, "Gameplay");
		if (XR_FAILED(xrCreateActionSet(s_instance, &asci, &s_actionSet))) { A1VR_LOG("createActionSet failed"); return; }

		auto mkAction = [&](const char* name, XrActionType type, XrAction* out) {
			XrActionCreateInfo aci = { XR_TYPE_ACTION_CREATE_INFO };
			std::strcpy(aci.actionName, name);
			std::strcpy(aci.localizedActionName, name);
			aci.actionType = type;
			xrCreateAction(s_actionSet, &aci, out);
		};
		mkAction("stickleft",  XR_ACTION_TYPE_VECTOR2F_INPUT, &s_stickAction[0]);
		mkAction("stickright", XR_ACTION_TYPE_VECTOR2F_INPUT, &s_stickAction[1]);
		mkAction("stickclickleft",  XR_ACTION_TYPE_BOOLEAN_INPUT, &s_stickClickAction[0]);
		mkAction("stickclickright", XR_ACTION_TYPE_BOOLEAN_INPUT, &s_stickClickAction[1]);
		mkAction("triggerleft",  XR_ACTION_TYPE_FLOAT_INPUT,  &s_triggerAction[0]);
		mkAction("triggerright", XR_ACTION_TYPE_FLOAT_INPUT,  &s_triggerAction[1]);
		mkAction("use", XR_ACTION_TYPE_BOOLEAN_INPUT,   &s_actionAction);
		mkAction("bbtn", XR_ACTION_TYPE_BOOLEAN_INPUT,  &s_bAction);
		mkAction("xbtn", XR_ACTION_TYPE_BOOLEAN_INPUT,  &s_xAction);
		mkAction("ybtn", XR_ACTION_TYPE_BOOLEAN_INPUT,  &s_yAction);
		mkAction("menu", XR_ACTION_TYPE_BOOLEAN_INPUT,  &s_menuAction);
		mkAction("aimleft",  XR_ACTION_TYPE_POSE_INPUT, &s_aimAction[0]);
		mkAction("aimright", XR_ACTION_TYPE_POSE_INPUT, &s_aimAction[1]);

		XrActionSuggestedBinding binds[] = {
			{ s_stickAction[0],   path("/user/hand/left/input/thumbstick") },
			{ s_stickAction[1],   path("/user/hand/right/input/thumbstick") },
			{ s_stickClickAction[0], path("/user/hand/left/input/thumbstick/click") },
			{ s_stickClickAction[1], path("/user/hand/right/input/thumbstick/click") },
			{ s_triggerAction[1], path("/user/hand/right/input/trigger/value") },
			{ s_triggerAction[0], path("/user/hand/left/input/trigger/value") },
			{ s_actionAction,  path("/user/hand/right/input/a/click") },
			{ s_bAction,       path("/user/hand/right/input/b/click") },
			{ s_xAction,       path("/user/hand/left/input/x/click") },
			{ s_yAction,       path("/user/hand/left/input/y/click") },
			{ s_menuAction,    path("/user/hand/left/input/menu/click") },
			{ s_aimAction[0],  path("/user/hand/left/input/aim/pose") },
			{ s_aimAction[1],  path("/user/hand/right/input/aim/pose") },
		};
		XrInteractionProfileSuggestedBinding sb = { XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING };
		sb.interactionProfile = path("/interaction_profiles/oculus/touch_controller");
		sb.countSuggestedBindings = sizeof(binds) / sizeof(binds[0]);
		sb.suggestedBindings = binds;
		if (XR_FAILED(xrSuggestInteractionProfileBindings(s_instance, &sb))) A1VR_LOG("suggestBindings failed");

		XrSessionActionSetsAttachInfo ai = { XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO };
		ai.countActionSets = 1;
		ai.actionSets = &s_actionSet;
		if (XR_FAILED(xrAttachSessionActionSets(s_session, &ai))) { A1VR_LOG("attachActionSets failed"); return; }

		for (int h = 0; h < 2; ++h) {
			XrActionSpaceCreateInfo sci = { XR_TYPE_ACTION_SPACE_CREATE_INFO };
			sci.action = s_aimAction[h];
			sci.poseInActionSpace.orientation.w = 1.0f;
			if (XR_FAILED(xrCreateActionSpace(s_session, &sci, &s_aimSpace[h]))) A1VR_LOG("createActionSpace %d failed", h);
		}

		s_actionsReady = true;
		A1VR_LOG("input actions attached");
	}

	void syncInput() {
		if (!s_actionsReady || !s_sessionRunning) return;
		XrActiveActionSet aas = { s_actionSet, XR_NULL_PATH };
		XrActionsSyncInfo si = { XR_TYPE_ACTIONS_SYNC_INFO };
		si.countActiveActionSets = 1;
		si.activeActionSets = &aas;
		xrSyncActions(s_session, &si);

		XrActionStateGetInfo gi = { XR_TYPE_ACTION_STATE_GET_INFO };
		XrActionStateVector2f v2 = { XR_TYPE_ACTION_STATE_VECTOR2F };
		XrActionStateFloat f = { XR_TYPE_ACTION_STATE_FLOAT };
		XrActionStateBoolean bs = { XR_TYPE_ACTION_STATE_BOOLEAN };
		for (int h = 0; h < 2; ++h) {
			gi.action = s_stickAction[h];   xrGetActionStateVector2f(s_session, &gi, &v2);
			s_stickX[h] = v2.currentState.x; s_stickY[h] = v2.currentState.y;
			gi.action = s_triggerAction[h]; xrGetActionStateFloat(s_session, &gi, &f);
			s_trigger[h] = f.currentState;
			gi.action = s_stickClickAction[h]; xrGetActionStateBoolean(s_session, &gi, &bs);
			s_stickClick[h] = bs.currentState;
		}

		// Route the raw per-hand sticks/triggers into the logical move/turn/fire actions. The DOMINANT
		// hand MOVES (stick) + FIRES the primary weapon (trigger); the OFF hand TURNS (stick) +
		// secondary-fires (trigger). So a left-handed player moves+fires with the left controller.
		// (QuestZDoom's default is the reverse -- dominant turns/aims, off-hand moves -- and its
		// vr_switch_sticks flips that; switchSticks here is the same move/turn swap.) (index 0=left, 1=right)
		const int domIdx = VR_Settings()->dominantHand ? 0 : 1;   // left-handed -> dominant is the left hand
		const int offIdx = 1 - domIdx;
		const int moveIdx = VR_Settings()->switchSticks ? offIdx : domIdx;
		const int turnIdx = VR_Settings()->switchSticks ? domIdx : offIdx;
		s_moveX = s_stickX[moveIdx]; s_moveY = s_stickY[moveIdx];
		s_turnX = s_stickX[turnIdx];
		s_fire    = s_trigger[domIdx] > 0.5f;
		s_altFire = s_trigger[offIdx] > 0.5f;

		XrActionStateBoolean b = { XR_TYPE_ACTION_STATE_BOOLEAN };
		gi.action = s_actionAction;  xrGetActionStateBoolean(s_session, &gi, &b); s_action = b.currentState;
		gi.action = s_bAction;       xrGetActionStateBoolean(s_session, &gi, &b); s_bBtn   = b.currentState;
		gi.action = s_xAction;       xrGetActionStateBoolean(s_session, &gi, &b); s_xBtn   = b.currentState;
		gi.action = s_yAction;       xrGetActionStateBoolean(s_session, &gi, &b); s_yBtn   = b.currentState;
		gi.action = s_menuAction;    xrGetActionStateBoolean(s_session, &gi, &b); s_menuBtn = b.currentState;

		// Menu (hamburger) button edge -> request the in-game quit-with-confirmation (consumed in the
		// main loop, which is where the modal confirmation dialog can run).
		if (s_menuBtn && !s_menuPrev) s_menuLatch = true;
		s_menuPrev = s_menuBtn;

		// Cutscene/intro skip: A or X edge -> inject Enter so the chapter screens advance (terminals
		// are handled via action flags in vbl.cpp). Lightweight stopgap until proper VR button mapping.
		const bool advance = s_action || s_xBtn;
		if (advance && !s_advancePrev) {
			SDL_Event e{};
			e.type = SDL_KEYDOWN; e.key.state = SDL_PRESSED;
			e.key.keysym.sym = SDLK_RETURN; e.key.keysym.scancode = SDL_SCANCODE_RETURN;
			SDL_PushEvent(&e);
			e.type = SDL_KEYUP; e.key.state = SDL_RELEASED;
			SDL_PushEvent(&e);
		}
		s_advancePrev = advance;
	}
}

extern "C" bool VR_BeginFrame(void)
{
	s_frameBegun = false;
	s_frameShouldRender = false;
	if (!s_active) return false;
	if (!startSession()) return false;   // waits for the GL context
	pollEvents();
	if (!s_sessionRunning) return false; // session not running yet -> no frame begun
	syncInput();

	XrFrameWaitInfo wfi = { XR_TYPE_FRAME_WAIT_INFO };
	s_frameState = XrFrameState{ XR_TYPE_FRAME_STATE };
	XR_CHECK(xrWaitFrame(s_session, &wfi, &s_frameState));
	XrFrameBeginInfo fbi = { XR_TYPE_FRAME_BEGIN_INFO };
	XR_CHECK(xrBeginFrame(s_session, &fbi));
	s_frameBegun = true;

	if (s_frameState.shouldRender) {
		XrSpaceLocation hl = { XR_TYPE_SPACE_LOCATION };
		xrLocateSpace(s_headSpace, s_stageSpace, s_frameState.predictedDisplayTime, &hl);
		s_stageFromHead = hl.pose;
		if (hl.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT)
			s_headPoseValid = true;
		// Seed the absorbed head origin on the first valid pose so the render starts at ~zero residual
		// (avoids a one-tick position jump when the first VR_LatchHeadMove runs).
		if (!s_headLatchInit && (hl.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT)) {
			s_lastHeadX = s_stageFromHead.position.x;
			s_lastHeadZ = s_stageFromHead.position.z;
			s_headLatchInit = true;
		}

		XrViewLocateInfo vli = { XR_TYPE_VIEW_LOCATE_INFO };
		vli.viewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
		vli.displayTime = s_frameState.predictedDisplayTime;
		vli.space = s_headSpace;
		XrViewState vs = { XR_TYPE_VIEW_STATE };
		uint32_t got = 0;
		for (int e = 0; e < kEyes; ++e) s_views[e].type = XR_TYPE_VIEW;
		xrLocateViews(s_session, &vli, &vs, kEyes, &got, s_views);
		for (int e = 0; e < kEyes; ++e)
			s_stageFromEye[e] = pose_mul(s_stageFromHead, s_views[e].pose);

		// Controller aim poses (for the menu pointer), in stage space.
		for (int h = 0; h < 2; ++h) {
			s_aimValid[h] = false;
			if (s_aimSpace[h] != XR_NULL_HANDLE) {
				XrSpaceLocation al = { XR_TYPE_SPACE_LOCATION };
				xrLocateSpace(s_aimSpace[h], s_stageSpace, s_frameState.predictedDisplayTime, &al);
				if ((al.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT) &&
					(al.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT)) {
					s_aimStage[h] = al.pose;
					s_aimValid[h] = true;
				}
			}
		}

		s_frameShouldRender = true;
	}
	return s_frameShouldRender;
}

extern "C" void VR_BeginEye(int eye)
{
	s_curEye = eye;
	EyeFB& fb = s_eye[eye];
	XrSwapchainImageAcquireInfo ai = { XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO };
	xrAcquireSwapchainImage(fb.swapchain, &ai, &s_curEyeImageIdx);
	XrSwapchainImageWaitInfo wi = { XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO };
	wi.timeout = XR_INFINITE_DURATION;
	xrWaitSwapchainImage(fb.swapchain, &wi);

	glBindFramebuffer(GL_FRAMEBUFFER, fb.fbos[s_curEyeImageIdx]);
	glDisable(GL_SCISSOR_TEST);   // the engine's 2D view scissor must not clip the full-eye render

	// The eye swapchain is sRGB so the compositor interprets it correctly, but the engine already
	// outputs sRGB-encoded colour; disable the GL linear->sRGB write conversion so our bytes are
	// stored verbatim (else the whole image -- world AND the 2D UI panel -- is over-brightened).
	if (s_srgbWriteControl < 0) {
		s_srgbWriteControl = hasGLExtension("GL_EXT_sRGB_write_control") ? 1 : 0;
		A1VR_LOG("EXT_sRGB_write_control: %s", s_srgbWriteControl ? "present (disabling sRGB encode)" : "ABSENT");
	}
	if (s_srgbWriteControl == 1)
		glDisable(GL_FRAMEBUFFER_SRGB_EXT);

	glViewport(0, 0, s_eyeW, s_eyeH);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

extern "C" unsigned VR_CurrentEyeFramebuffer(void) { return s_eye[s_curEye].fbos[s_curEyeImageIdx]; }

extern "C" void VR_GetEyeProjection(int eye, float* proj16, float zn, float zf)
{
	proj_from_fov(proj16, s_views[eye].fov, zn, zf);
}

extern "C" void VR_GetEyeViewMetres(int eye, float* view16)
{
	// Use the eye position RELATIVE to the absorbed head origin (horizontal only) so the render shows
	// only the residual head movement since the last tick -- the body carries the rest. Without this
	// the body-follows-head locomotion would double-count and movement would feel ~2x.
	// The head's HORIZONTAL position is applied to the VIEW ORIGIN engine-side (VR_GetHeadOffset,
	// clamped against walls), so here the camera keeps only the per-eye IPD separation horizontally
	// (eye relative to head). Keep full vertical (duck) + full orientation. This is render-side only
	// (never the physics position), so it can't fly.
	XrPosef p = s_stageFromEye[eye];
	p.position.x = s_stageFromEye[eye].position.x - s_stageFromHead.position.x;
	p.position.z = s_stageFromEye[eye].position.z - s_stageFromHead.position.z;
	float eyeM[16]; mat_from_pose(eyeM, p);
	mat_rigid_inverse(view16, eyeM);
}

extern "C" void VR_FinishEye(int eye)
{
	EyeFB& fb = s_eye[eye];
	glBindFramebuffer(GL_FRAMEBUFFER, fb.fbos[s_curEyeImageIdx]);
	const GLenum disc[1] = { GL_DEPTH_ATTACHMENT };
	glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, disc);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	XrSwapchainImageReleaseInfo ri = { XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO };
	xrReleaseSwapchainImage(fb.swapchain, &ri);

	s_layerViews[eye].type = XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW;
	s_layerViews[eye].pose = s_stageFromEye[eye];
	s_layerViews[eye].fov  = s_views[eye].fov;
	s_layerViews[eye].subImage.swapchain = fb.swapchain;
	s_layerViews[eye].subImage.imageRect.offset = { 0, 0 };
	s_layerViews[eye].subImage.imageRect.extent = { s_eyeW, s_eyeH };
	s_layerViews[eye].subImage.imageArrayIndex = 0;
}

extern "C" void VR_SubmitFrame(void)
{
	if (!s_frameBegun) return;
	XrCompositionLayerProjection layer = { XR_TYPE_COMPOSITION_LAYER_PROJECTION };
	layer.space = s_stageSpace;
	layer.viewCount = kEyes;
	layer.views = s_layerViews;
	const XrCompositionLayerBaseHeader* layers[1] = { (XrCompositionLayerBaseHeader*)&layer };

	XrFrameEndInfo fei = { XR_TYPE_FRAME_END_INFO };
	fei.displayTime = s_frameState.predictedDisplayTime;
	fei.environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
	fei.layerCount = s_frameShouldRender ? 1 : 0;
	fei.layers     = s_frameShouldRender ? layers : nullptr;
	XR_CHECK(xrEndFrame(s_session, &fei));
	s_frameBegun = false;
}

extern "C" int VR_EyeWidth(void)  { return s_eyeW; }
extern "C" int VR_EyeHeight(void) { return s_eyeH; }
extern "C" int VR_CurrentEye(void) { return s_curEye; }

extern "C" bool VR_GetHmdYawPitch(float* yawAngleUnits, float* pitchAngleUnits)
{
	if (yawAngleUnits)   *yawAngleUnits   = 0;
	if (pitchAngleUnits) *pitchAngleUnits = 0;
	if (!s_headPoseValid) return false;

	// Head-forward (head space -Z) rotated into stage space (Y-up, -Z forward) by the HMD quaternion.
	const XrQuaternionf& q = s_stageFromHead.orientation;
	const float fx = -2.0f * (q.x * q.z + q.w * q.y);
	const float fy = -2.0f * (q.y * q.z - q.w * q.x);
	const float fz = -(1.0f - 2.0f * (q.x * q.x + q.y * q.y));

	const float kUnitsPerRad = 512.0f / (2.0f * 3.14159265358979f);   // NUMBER_OF_ANGLES / 2pi
	// World-frame azimuth contribution to ADD to the body yaw, derived so view->yaw matches the
	// per-eye modelview's camera direction exactly: modelview maps head-forward (fx,fy,fz) through
	// M^-1 (zUpToYUp inverse: stage(sx,sy,sz)->world(-sx,-sz,sy)) giving world dir (-fx,-fz,fy), whose
	// Marathon azimuth is atan2(j,i) = atan2(-fz,-fx). (Using atan2(fx,-fz) before put the visibility
	// cone ~90 deg off the camera even at neutral -> the world looked back-to-front.)
	const float yawRad   = std::atan2(-fz, -fx);
	float s = fy; if (s > 1.0f) s = 1.0f; else if (s < -1.0f) s = -1.0f;
	const float pitchRad = std::asin(s);                             // + = looking up (world elevation)

	if (yawAngleUnits)   *yawAngleUnits   = yawRad   * kUnitsPerRad;
	if (pitchAngleUnits) *pitchAngleUnits = pitchRad * kUnitsPerRad;
	return true;
}

extern "C" void VR_GetMove(float* x, float* y) { if (x) *x = s_moveX; if (y) *y = s_moveY; }
extern "C" void VR_GetTurn(float* x)           { if (x) *x = s_turnX; }

// Deadzoned + rescaled thumbstick so partial deflection gives proportionally slower movement.
extern "C" void VR_GetAnalogMove(float* strafe, float* forward)
{
	const float dead = 0.15f;
	auto curve = [&](float v) -> float {
		const float a = std::fabs(v);
		if (a <= dead) return 0.0f;
		const float t = (a - dead) / (1.0f - dead);   // just past deadzone -> ~0, full -> 1
		return (v < 0 ? -t : t);
	};
	if (strafe)  *strafe  = curve(s_moveX);
	if (forward) *forward = curve(s_moveY);
}

// Capture the head's physical movement since the last call as a world-space body delta (map x,y),
// so the engine can move the player to follow the head (with collision). Call once per real tick.
extern "C" void VR_LatchHeadMove(void)
{
	const float hx = s_stageFromHead.position.x;
	const float hz = s_stageFromHead.position.z;
	if (!s_headLatchInit || !s_headPoseValid) {
		s_lastHeadX = hx; s_lastHeadZ = hz;
		s_headLatchInit = s_headPoseValid;
		s_headMoveX = s_headMoveY = 0;
		return;
	}
	float dhx = hx - s_lastHeadX;
	float dhz = hz - s_lastHeadZ;
	s_lastHeadX = hx; s_lastHeadZ = hz;

	// Safety clamp (catastrophe guard only -- normal lean/walk is well under this; the old takeoff was
	// the residual/rate bug, now removed, not large deltas). ~0.15 m/tick = ~4.5 m/s head speed.
	const float kMax = 0.15f;
	if (dhx >  kMax) dhx =  kMax; else if (dhx < -kMax) dhx = -kMax;
	if (dhz >  kMax) dhz =  kMax; else if (dhz < -kMax) dhz = -kMax;

	// World body delta = Rz(yawOffset) * (-dhx, -dhz) * worldScale (matches the render's head-offset:
	// head_world = origin + Rz(yawOffset) * (-hx, -hz) * W). Stage deltas keep snap-turns from
	// registering as movement (a snap rotates the play space, not the head's physical position).
	const float W  = s_settings.worldScaleWUM;
	const float yr = s_yawOffset * (2.0f * 3.14159265358979f / 512.0f);
	const float c  = std::cos(yr), s = std::sin(yr);
	s_headMoveX = W * (-c * dhx + s * dhz);
	s_headMoveY = W * (-s * dhx - c * dhz);

	// Diagnostic (throttled): raw head delta (m/tick, post-clamp) + resulting body delta (world units).
	static int s_dbg = 0;
	if ((s_dbg++ % 15) == 0)
		A1VR_LOG("headmove: pos=(%.3f,%.3f) d=(%.4f,%.4f) -> body=(%.2f,%.2f) wu", hx, hz, dhx, dhz, s_headMoveX, s_headMoveY);
}

extern "C" void VR_GetHeadMove(float* x, float* y) { if (x) *x = s_headMoveX; if (y) *y = s_headMoveY; }

// Vertical offset of the actual HMD eye from the neutral standing eye height, in Marathon world
// units (negative when seated/ducking). The engine adds this to view->origin.z so the visibility
// tree clips floors/ceilings at the TRUE eye height (fixes step-top floors vanishing when seated);
// the Rasterizer subtracts it back so the rendered camera is unchanged.
extern "C" float VR_GetEyeZOffset(void)
{
	if (!s_headPoseValid) return 0.0f;
	return (s_stageFromHead.position.y - s_settings.eyeHeightM) * s_settings.worldScaleWUM;
}

extern "C" void VR_RecenterHead(void)
{
	if (s_headPoseValid) { s_headRefX = s_stageFromHead.position.x; s_headRefZ = s_stageFromHead.position.z; s_headRefInit = true; }
}

// The head's HORIZONTAL position relative to the recenter reference, mapped to Marathon world units
// (map x,y). The engine applies this to the VIEW ORIGIN (clamped against walls) so leaning/walking
// moves the camera + visibility origin -- render-side only, never the physics position.
extern "C" void VR_GetHeadOffset(float* wx, float* wy)
{
	if (wx) *wx = 0; if (wy) *wy = 0;
	if (!s_headPoseValid) return;
	if (!s_headRefInit) {   // auto-recenter on first valid pose -> offset starts at 0
		s_headRefX = s_stageFromHead.position.x; s_headRefZ = s_stageFromHead.position.z;
		s_headRefInit = true; return;
	}
	const float dhx = s_stageFromHead.position.x - s_headRefX;
	const float dhz = s_stageFromHead.position.z - s_headRefZ;
	const float W  = s_settings.worldScaleWUM;
	const float yr = s_yawOffset * (2.0f * 3.14159265358979f / 512.0f);
	const float c  = std::cos(yr), s = std::sin(yr);
	if (wx) *wx = W * (-c * dhx + s * dhz);
	if (wy) *wy = W * (-s * dhx - c * dhz);
}
extern "C" bool VR_GetFire(void)               { return s_fire; }
extern "C" bool VR_GetSecondaryFire(void)      { return s_altFire; }
extern "C" bool VR_GetAction(void)             { return s_action; }
extern "C" bool VR_GetAdvance(void)            { return s_action || s_xBtn; }   // A or X
extern "C" bool VR_GetBack(void)               { return s_yBtn || s_bBtn; }     // Y or B
extern "C" bool VR_GetButtonX(void)            { return s_xBtn; }
extern "C" bool VR_GetButtonY(void)            { return s_yBtn; }
// Press of the thumbstick you MOVE with (routed by handedness/switch-sticks, like the move stick).
extern "C" bool VR_GetMoveStickClick(void)
{
	const int domIdx = s_settings.dominantHand ? 0 : 1;
	const int offIdx = 1 - domIdx;
	const int moveIdx = s_settings.switchSticks ? offIdx : domIdx;
	return s_stickClick[moveIdx];
}

namespace {
	bool s_worldFramePresented = false;
	// World-locked 2D panel (placed in stage space when a menu/terminal is shown). Vectors in stage m.
	float s_panelC[3] = {0,0,0}, s_panelR[3] = {1,0,0}, s_panelU[3] = {0,1,0}, s_panelN[3] = {0,0,1};
	float s_panelHalfW = 1, s_panelHalfH = 1;
	bool  s_panelPlaced = false;
	// Pointer result (computed in VR_PresentScreenLayer): which hand hit the panel + screen pixel + UV.
	bool  s_ptrActive = false; int s_ptrHand = 1;
	int   s_ptrX = 0, s_ptrY = 0; float s_ptrU = 0.5f, s_ptrV = 0.5f;
}
extern "C" void VR_MarkWorldFramePresented(void) { s_worldFramePresented = true; s_panelPlaced = false; /* re-place the 2D panel next time a menu is shown */ }
extern "C" bool VR_TakeWorldFramePresented(void) { bool v = s_worldFramePresented; s_worldFramePresented = false; return v; }

// Menu pointer: the screen-layer pixel the active controller ray is hitting (false = no hit), and
// whether that controller's trigger is pressed (for a click). Computed in VR_PresentScreenLayer.
extern "C" bool VR_GetPointerScreen(int* x, int* y) { if (s_ptrActive) { if (x) *x = s_ptrX; if (y) *y = s_ptrY; } return s_ptrActive; }
extern "C" bool VR_GetPointerClick(void) { if (!s_ptrActive) return false; return s_trigger[s_ptrHand] > 0.5f; }   // raw per-hand trigger (s_fire is the routed dominant one)

// ---------------------------------------------------------- screen layer ----
namespace {
	constexpr int kScreenW = 1280, kScreenH = 1024;   // matches change_screen_mode's VR vmode default
	GLuint s_screenFBO = 0, s_screenTex = 0, s_screenDepth = 0;
	GLuint s_quadProg = 0, s_quadVAO = 0, s_quadVBO = 0;
	GLint  s_quadTexLoc = -1, s_quadMVPLoc = -1;
	GLuint s_curProg = 0; GLint s_curMVPLoc = -1, s_curColorLoc = -1;   // pointer cursor (solid colour)

	void ensureScreenLayer() {
		if (s_screenFBO) return;
		glGenTextures(1, &s_screenTex);
		glBindTexture(GL_TEXTURE_2D, s_screenTex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, kScreenW, kScreenH, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glGenRenderbuffers(1, &s_screenDepth);
		glBindRenderbuffer(GL_RENDERBUFFER, s_screenDepth);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, kScreenW, kScreenH);
		glGenFramebuffers(1, &s_screenFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, s_screenFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, s_screenTex, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, s_screenDepth);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			A1VR_LOG("screen layer FBO incomplete");
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		static const char* vs =
			"#version 300 es\n"
			"layout(location=0) in vec2 aPos;\n"
			"uniform mat4 uMVP;\n"
			"out vec2 vUV;\n"
			"void main(){ vUV = aPos*0.5+0.5; gl_Position = uMVP * vec4(aPos,0.0,1.0); }\n";
		static const char* fs =
			"#version 300 es\n"
			"precision mediump float;\n"
			"in vec2 vUV; out vec4 o; uniform sampler2D uTex;\n"
			"void main(){ o = texture(uTex, vUV); }\n";
		GLuint v = compile(GL_VERTEX_SHADER, vs), f = compile(GL_FRAGMENT_SHADER, fs);
		s_quadProg = glCreateProgram();
		glAttachShader(s_quadProg, v); glAttachShader(s_quadProg, f); glLinkProgram(s_quadProg);
		glDeleteShader(v); glDeleteShader(f);
		s_quadTexLoc = glGetUniformLocation(s_quadProg, "uTex");
		s_quadMVPLoc = glGetUniformLocation(s_quadProg, "uMVP");
		const float quad[] = { -1,-1,  1,-1,  1,1,  -1,-1,  1,1,  -1,1 };
		glGenVertexArrays(1, &s_quadVAO);
		glGenBuffers(1, &s_quadVBO);
		glBindVertexArray(s_quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, s_quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof quad, quad, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glBindVertexArray(0);

		// Cursor program: a solid-coloured quad (reuses the same VAO) drawn at the pointer hit.
		static const char* cvs =
			"#version 300 es\n"
			"layout(location=0) in vec2 aPos;\n"
			"uniform mat4 uMVP;\n"
			"void main(){ gl_Position = uMVP * vec4(aPos,0.0,1.0); }\n";
		static const char* cfs =
			"#version 300 es\n"
			"precision mediump float;\n"
			"uniform vec4 uColor; out vec4 o;\n"
			"void main(){ o = uColor; }\n";
		GLuint cv = compile(GL_VERTEX_SHADER, cvs), cf = compile(GL_FRAGMENT_SHADER, cfs);
		s_curProg = glCreateProgram();
		glAttachShader(s_curProg, cv); glAttachShader(s_curProg, cf); glLinkProgram(s_curProg);
		glDeleteShader(cv); glDeleteShader(cf);
		s_curMVPLoc   = glGetUniformLocation(s_curProg, "uMVP");
		s_curColorLoc = glGetUniformLocation(s_curProg, "uColor");
	}
}

// ----------------------------------------------------------- dim pass ----
namespace {
	GLuint s_dimProg = 0, s_dimVAO = 0, s_dimVBO = 0;
	GLint  s_dimColorLoc = -1;

	void ensureDim() {
		if (s_dimProg) return;
		static const char* vs =
			"#version 300 es\n"
			"layout(location=0) in vec2 aPos;\n"
			"void main(){ gl_Position = vec4(aPos,0.0,1.0); }\n";
		static const char* fs =
			"#version 300 es\n"
			"precision mediump float;\n"
			"uniform vec4 uColor; out vec4 o;\n"
			"void main(){ o = uColor; }\n";
		GLuint v = compile(GL_VERTEX_SHADER, vs), f = compile(GL_FRAGMENT_SHADER, fs);
		s_dimProg = glCreateProgram();
		glAttachShader(s_dimProg, v); glAttachShader(s_dimProg, f); glLinkProgram(s_dimProg);
		glDeleteShader(v); glDeleteShader(f);
		s_dimColorLoc = glGetUniformLocation(s_dimProg, "uColor");
		const float quad[] = { -1,-1,  1,-1,  1,1,  -1,-1,  1,1,  -1,1 };
		glGenVertexArrays(1, &s_dimVAO);
		glGenBuffers(1, &s_dimVBO);
		glBindVertexArray(s_dimVAO);
		glBindBuffer(GL_ARRAY_BUFFER, s_dimVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof quad, quad, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glBindVertexArray(0);
	}
}

extern "C" void VR_DimCurrentEye(void)
{
	const float b = s_settings.brightness;
	if (b >= 0.999f) return;
	ensureDim();
	glBindFramebuffer(GL_FRAMEBUFFER, s_eye[s_curEye].fbos[s_curEyeImageIdx]);
	glViewport(0, 0, s_eyeW, s_eyeH);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_SCISSOR_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ZERO, GL_SRC_COLOR);   // result = dst * srcColor  -> multiply the eye by (b,b,b)
	glUseProgram(s_dimProg);
	glUniform4f(s_dimColorLoc, b, b, b, 1.0f);
	glBindVertexArray(s_dimVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	glDisable(GL_BLEND);
}

extern "C" unsigned VR_ScreenLayerFramebuffer(void) { ensureScreenLayer(); return s_screenFBO; }
extern "C" int VR_ScreenLayerWidth(void)  { return kScreenW; }
extern "C" int VR_ScreenLayerHeight(void) { return kScreenH; }

namespace {
	void vsub(const float* a, const float* b, float* o){ o[0]=a[0]-b[0]; o[1]=a[1]-b[1]; o[2]=a[2]-b[2]; }
	float vdot(const float* a, const float* b){ return a[0]*b[0]+a[1]*b[1]+a[2]*b[2]; }
	void vcross(const float* a, const float* b, float* o){ o[0]=a[1]*b[2]-a[2]*b[1]; o[1]=a[2]*b[0]-a[0]*b[2]; o[2]=a[0]*b[1]-a[1]*b[0]; }
	void vnorm(float* v){ float l=std::sqrt(vdot(v,v)); if(l>1e-6f){ v[0]/=l; v[1]/=l; v[2]/=l; } }
	void poseFwd(const XrPosef& p, float* d){ float m[16]; mat_from_pose(m,p); d[0]=-m[8]; d[1]=-m[9]; d[2]=-m[10]; }

	// Place the 2D panel fixed in stage space, in front of the head's horizontal facing (so it stays
	// put while you point at it / look around). Called once each time a 2D screen is (re)entered.
	void placePanel() {
		float hp[3] = { s_stageFromHead.position.x, s_stageFromHead.position.y, s_stageFromHead.position.z };
		float hf[3]; poseFwd(s_stageFromHead, hf); hf[1]=0; vnorm(hf);
		if (hf[0]==0 && hf[2]==0) { hf[2]=-1; }
		const float D = s_settings.screenDistanceM;
		s_panelC[0]=hp[0]+hf[0]*D; s_panelC[1]=hp[1]; s_panelC[2]=hp[2]+hf[2]*D;
		s_panelN[0]=-hf[0]; s_panelN[1]=0; s_panelN[2]=-hf[2];           // faces the head
		const float up[3]={0,1,0};
		vcross(up, s_panelN, s_panelR); vnorm(s_panelR);
		vcross(s_panelN, s_panelR, s_panelU); vnorm(s_panelU);
		s_panelHalfH = 0.5f*s_settings.screenHeightM;
		s_panelHalfW = s_panelHalfH * (float)kScreenW/(float)kScreenH;
		s_panelPlaced = true;
	}

	// Intersect each controller aim ray with the panel; pick the closest hit -> screen pixel + UV.
	void updatePointer() {
		s_ptrActive = false;
		float bestT = 1e9f;
		for (int h=0; h<2; ++h) {
			if (!s_aimValid[h]) continue;
			float O[3] = { s_aimStage[h].position.x, s_aimStage[h].position.y, s_aimStage[h].position.z };
			float Dd[3]; poseFwd(s_aimStage[h], Dd);
			float denom = vdot(Dd, s_panelN);
			if (denom > -1e-4f) continue;                 // ray must point at the panel's front face
			float oc[3]; vsub(s_panelC, O, oc);
			float t = vdot(oc, s_panelN)/denom;
			if (t <= 0 || t >= bestT) continue;
			float hit[3] = { O[0]+Dd[0]*t, O[1]+Dd[1]*t, O[2]+Dd[2]*t };
			float loc[3]; vsub(hit, s_panelC, loc);
			float u = vdot(loc, s_panelR)/s_panelHalfW;
			float v = vdot(loc, s_panelU)/s_panelHalfH;
			if (u<-1||u>1||v<-1||v>1) continue;
			bestT = t; s_ptrActive = true; s_ptrHand = h; s_ptrU = u; s_ptrV = v;
			s_ptrX = (int)((u*0.5f+0.5f) * kScreenW);
			s_ptrY = (int)((1.0f-(v*0.5f+0.5f)) * kScreenH);
		}
	}
}

extern "C" bool VR_GetAimPoseStage(int hand, float pos3[3], float fwd3[3])
{
	if (hand < 0 || hand > 1 || !s_aimValid[hand]) return false;
	pos3[0] = s_aimStage[hand].position.x;
	pos3[1] = s_aimStage[hand].position.y;
	pos3[2] = s_aimStage[hand].position.z;
	// Forward = the pose's -Z, pitched by aimPitchAdjust about the pose's local right axis (the OpenXR
	// aim pose points higher than a held-gun barrel; negative tilts down). m: col0=right, col1=up,
	// col2=back. fwd = sin(th)*up - cos(th)*back  (th=0 -> -back -> plain poseFwd).
	float m[16]; mat_from_pose(m, s_aimStage[hand]);
	const float th = s_settings.aimPitchAdjust * (3.14159265358979f / 180.0f);
	const float st = std::sin(th), ct = std::cos(th);
	fwd3[0] = st*m[4] - ct*m[8];
	fwd3[1] = st*m[5] - ct*m[9];
	fwd3[2] = st*m[6] - ct*m[10];
	return true;
}

extern "C" bool VR_GetHeadPosStage(float pos3[3])
{
	if (!s_headPoseValid) return false;
	pos3[0] = s_stageFromHead.position.x;
	pos3[1] = s_stageFromHead.position.y;
	pos3[2] = s_stageFromHead.position.z;
	return true;
}

// The DOMINANT controller's aim direction as a Marathon-world unit vector (x/y horizontal, z up) --
// the same mapping the aim-debug ray uses, so weapons fire exactly where the ray points. Returns
// false if the controller pose isn't tracked. dir = Rz(yawOffset) * Z^T * aimForward(stage).
extern "C" bool VR_GetWeaponAim(float dir[3])
{
	const int hand = s_settings.dominantHand ? 0 : 1;   // dominant hand holds the weapon
	float pos[3], fwd[3];
	if (!VR_GetAimPoseStage(hand, pos, fwd)) return false;   // fwd already has aimPitchAdjust applied
	// stage(metres,Y-up) forward -> world(WU,Z-up): Z^T maps (sx,sy,sz)->(-sx,-sz,sy), then Rz(yawOffset).
	const float zx = -fwd[0], zy = -fwd[2], zz = fwd[1];
	const float yr = s_yawOffset * (2.0f * 3.14159265358979f / 512.0f);   // 512 = Marathon FULL_CIRCLE
	const float cw = std::cos(yr), sw = std::sin(yr);
	dir[0] = zx*cw - zy*sw;
	dir[1] = zx*sw + zy*cw;
	dir[2] = zz;
	const float l = std::sqrt(dir[0]*dir[0] + dir[1]*dir[1] + dir[2]*dir[2]);
	if (l > 1e-6f) { dir[0]/=l; dir[1]/=l; dir[2]/=l; }
	return true;
}

// Menu (hamburger) button press, consumed once. The main loop turns this into the in-game quit dialog.
extern "C" bool VR_TakeMenuButton(void) { bool v = s_menuLatch; s_menuLatch = false; return v; }

// True only while the OpenXR session is FOCUSED (the app is the active immersive app). Drops when the
// user presses the Meta/home button and goes to the system overlay -> the engine pauses on that.
extern "C" bool VR_HasFocus(void) { return s_sessionState == XR_SESSION_STATE_FOCUSED; }

extern "C" void VR_PresentScreenLayer(void)
{
	if (!s_active) return;
	ensureScreenLayer();
	const bool render = VR_BeginFrame();
	if (render) {
		if (!s_panelPlaced && s_headPoseValid) placePanel();
		updatePointer();   // ray-cast both controllers onto the world-locked panel

		// World-locked panel: local quad (-1..1 XY) -> stage via panelRight/Up/Center.
		const float hw = s_panelHalfW, hh = s_panelHalfH;
		float model[16] = {
			s_panelR[0]*hw, s_panelR[1]*hw, s_panelR[2]*hw, 0,
			s_panelU[0]*hh, s_panelU[1]*hh, s_panelU[2]*hh, 0,
			s_panelN[0],    s_panelN[1],    s_panelN[2],    0,
			s_panelC[0],    s_panelC[1],    s_panelC[2],    1 };
		// Cursor: small quad at the hit point, a hair toward the head so it sits in front of the panel.
		const float cs = 0.012f;
		const float cc[3] = { s_panelC[0] + s_ptrU*hw*s_panelR[0] + s_ptrV*hh*s_panelU[0] + s_panelN[0]*0.01f,
		                      s_panelC[1] + s_ptrU*hw*s_panelR[1] + s_ptrV*hh*s_panelU[1] + s_panelN[1]*0.01f,
		                      s_panelC[2] + s_ptrU*hw*s_panelR[2] + s_ptrV*hh*s_panelU[2] + s_panelN[2]*0.01f };
		float curModel[16] = {
			s_panelR[0]*cs, s_panelR[1]*cs, s_panelR[2]*cs, 0,
			s_panelU[0]*cs, s_panelU[1]*cs, s_panelU[2]*cs, 0,
			s_panelN[0],    s_panelN[1],    s_panelN[2],    0,
			cc[0],          cc[1],          cc[2],          1 };

		for (int e = 0; e < kEyes; ++e) {
			VR_BeginEye(e);
			float proj[16];
			VR_GetEyeProjection(e, proj, 0.05f, 50.0f);
			float stageFromEye[16]; mat_from_pose(stageFromEye, s_stageFromEye[e]);
			float eyeFromStage[16]; mat_rigid_inverse(eyeFromStage, stageFromEye);
			float vp[16]; mat_mul(vp, proj, eyeFromStage);   // proj * eyeFromStage

			glDisable(GL_DEPTH_TEST);
			glDisable(GL_CULL_FACE);
			// panel
			float mvp[16]; mat_mul(mvp, vp, model);
			glUseProgram(s_quadProg);
			glUniformMatrix4fv(s_quadMVPLoc, 1, GL_FALSE, mvp);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, s_screenTex);
			glUniform1i(s_quadTexLoc, 0);
			glBindVertexArray(s_quadVAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			// cursor
			if (s_ptrActive) {
				float cmvp[16]; mat_mul(cmvp, vp, curModel);
				glUseProgram(s_curProg);
				glUniformMatrix4fv(s_curMVPLoc, 1, GL_FALSE, cmvp);
				glUniform4f(s_curColorLoc, 1.0f, 0.85f, 0.1f, 1.0f);
				glDrawArrays(GL_TRIANGLES, 0, 6);
			}
			glBindVertexArray(0);
			VR_FinishEye(e);
		}
	}
	VR_SubmitFrame();
}

// Per-frame fallback for non-3D frames (menus/loading): render the test room to both eyes.
extern "C" bool VR_RenderTestFrame(void)
{
	if (!s_active) return false;
	const bool render = VR_BeginFrame();
	if (render) {
		for (int e = 0; e < kEyes; ++e) {
			VR_BeginEye(e);
			float proj[16], view[16];
			VR_GetEyeProjection(e, proj, 0.05f, 100.0f);
			VR_GetEyeViewMetres(e, view);
			drawTestScene(proj, view);
			VR_FinishEye(e);
		}
	}
	VR_SubmitFrame();   // no-ops if no frame was begun (session not running)
	return s_active;
}

#else // !__ANDROID__

extern "C" bool VR_InitOpenXR(void)     { return false; }
extern "C" bool VR_IsActive(void)       { return false; }
extern "C" vr_settings_t* VR_Settings(void) {
	static vr_settings_t s = { 1, 2.5f, 2.0f, 512.0f, 1.6f, 1, 30.0f, 1.0f, 0, 0, 0, -20.0f };
	return &s;
}
extern "C" float VR_GetYawOffset(void)   { return 0.0f; }
extern "C" void  VR_UpdateTurn(float, float) {}
extern "C" void  VR_SetYawOffset(float)  {}
extern "C" void  VR_RequestYawRecenter(int) {}
extern "C" bool  VR_TakeYawRecenter(int*) { return false; }
extern "C" void  VR_DimCurrentEye(void)  {}
extern "C" bool VR_RenderTestFrame(void){ return false; }
extern "C" bool VR_InitEGL(void)        { return false; }
extern "C" bool VR_GetEyeResolution(int* w, int* h) { (void)w; (void)h; return false; }
extern "C" bool VR_GetHmdYawPitch(float* y, float* p) { if (y) *y = 0; if (p) *p = 0; return false; }
extern "C" void VR_GetMove(float* x, float* y) { if (x) *x = 0; if (y) *y = 0; }
extern "C" void VR_GetTurn(float* x)           { if (x) *x = 0; }
extern "C" void VR_GetAnalogMove(float* s, float* f) { if (s) *s = 0; if (f) *f = 0; }
extern "C" void VR_LatchHeadMove(void) {}
extern "C" void VR_GetHeadMove(float* x, float* y) { if (x) *x = 0; if (y) *y = 0; }
extern "C" void VR_RecenterHead(void) {}
extern "C" void VR_GetHeadOffset(float* x, float* y) { if (x) *x = 0; if (y) *y = 0; }
extern "C" float VR_GetEyeZOffset(void) { return 0.0f; }
extern "C" bool VR_GetFire(void)               { return false; }
extern "C" bool VR_GetSecondaryFire(void)      { return false; }
extern "C" bool VR_GetAction(void)             { return false; }
extern "C" bool VR_GetAdvance(void)            { return false; }
extern "C" bool VR_GetBack(void)               { return false; }
extern "C" bool VR_GetButtonX(void)            { return false; }
extern "C" bool VR_GetButtonY(void)            { return false; }
extern "C" bool VR_GetMoveStickClick(void)     { return false; }
extern "C" bool VR_BeginFrame(void)     { return false; }
extern "C" void VR_BeginEye(int)        {}
extern "C" void VR_FinishEye(int)       {}
extern "C" void VR_GetEyeProjection(int, float*, float, float) {}
extern "C" void VR_GetEyeViewMetres(int, float*) {}
extern "C" void VR_SubmitFrame(void)    {}
extern "C" int  VR_EyeWidth(void)       { return 0; }
extern "C" int  VR_EyeHeight(void)      { return 0; }
extern "C" unsigned VR_CurrentEyeFramebuffer(void) { return 0; }
extern "C" int  VR_CurrentEye(void)     { return 0; }
extern "C" void VR_MarkWorldFramePresented(void) {}
extern "C" bool VR_TakeWorldFramePresented(void) { return false; }
extern "C" unsigned VR_ScreenLayerFramebuffer(void) { return 0; }
extern "C" void VR_PresentScreenLayer(void) {}
extern "C" bool VR_GetPointerScreen(int* x, int* y) { (void)x; (void)y; return false; }
extern "C" bool VR_GetPointerClick(void) { return false; }
extern "C" bool VR_GetAimPoseStage(int, float*, float*) { return false; }
extern "C" bool VR_GetHeadPosStage(float*) { return false; }
extern "C" bool VR_GetWeaponAim(float*) { return false; }
extern "C" bool VR_TakeMenuButton(void) { return false; }
extern "C" bool VR_HasFocus(void) { return false; }
extern "C" int  VR_ScreenLayerWidth(void)  { return 0; }
extern "C" int  VR_ScreenLayerHeight(void) { return 0; }

#endif
