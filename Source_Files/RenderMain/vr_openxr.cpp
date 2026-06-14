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

// Engine-owned headless EGL context (no window surface).
EGLDisplay s_eglDpy  = EGL_NO_DISPLAY;
EGLContext s_eglCtx  = EGL_NO_CONTEXT;
EGLSurface s_eglSurf = EGL_NO_SURFACE;
bool       s_eglReady = false;

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

	// Pick a colour format (prefer plain RGBA8 to avoid sRGB conversions for now).
	uint32_t fmtCount = 0;
	xrEnumerateSwapchainFormats(s_session, 0, &fmtCount, nullptr);
	int64_t* fmts = new int64_t[fmtCount];
	xrEnumerateSwapchainFormats(s_session, fmtCount, &fmtCount, fmts);
	int64_t chosen = fmts[0];
	for (uint32_t i = 0; i < fmtCount; ++i) if (fmts[i] == GL_RGBA8) { chosen = GL_RGBA8; break; }
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

extern "C" bool VR_RenderTestFrame(void)
{
	if (!s_active) return false;
	if (!startSession()) return false;   // waits for the GL context

	pollEvents();
	if (!s_sessionRunning) return true;   // VR owns the frame; just don't present yet

	XrFrameWaitInfo wfi = { XR_TYPE_FRAME_WAIT_INFO };
	s_frameState = XrFrameState{ XR_TYPE_FRAME_STATE };
	XR_CHECK(xrWaitFrame(s_session, &wfi, &s_frameState));

	XrFrameBeginInfo fbi = { XR_TYPE_FRAME_BEGIN_INFO };
	XR_CHECK(xrBeginFrame(s_session, &fbi));

	XrCompositionLayerProjectionView layerViews[kEyes] = {};
	XrCompositionLayerProjection layer = { XR_TYPE_COMPOSITION_LAYER_PROJECTION };
	bool haveLayer = false;

	if (s_frameState.shouldRender) {
		// Head pose (for the layer) + per-eye views.
		XrSpaceLocation hl = { XR_TYPE_SPACE_LOCATION };
		xrLocateSpace(s_headSpace, s_stageSpace, s_frameState.predictedDisplayTime, &hl);
		s_stageFromHead = hl.pose;

		XrViewLocateInfo vli = { XR_TYPE_VIEW_LOCATE_INFO };
		vli.viewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
		vli.displayTime = s_frameState.predictedDisplayTime;
		vli.space = s_headSpace;
		XrViewState vs = { XR_TYPE_VIEW_STATE };
		uint32_t got = 0;
		for (int e = 0; e < kEyes; ++e) s_views[e].type = XR_TYPE_VIEW;
		xrLocateViews(s_session, &vli, &vs, kEyes, &got, s_views);

		for (int e = 0; e < kEyes; ++e) {
			EyeFB& fb = s_eye[e];
			uint32_t idx = 0;
			XrSwapchainImageAcquireInfo ai = { XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO };
			xrAcquireSwapchainImage(fb.swapchain, &ai, &idx);
			XrSwapchainImageWaitInfo wi = { XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO };
			wi.timeout = XR_INFINITE_DURATION;
			xrWaitSwapchainImage(fb.swapchain, &wi);

			glBindFramebuffer(GL_FRAMEBUFFER, fb.fbos[idx]);
			glViewport(0, 0, s_eyeW, s_eyeH);
			glClearColor(0.05f, 0.05f, 0.08f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// view = inverse(stageFromEye); eye pose is in head space -> compose with head.
			XrPosef stageFromEye = pose_mul(s_stageFromHead, s_views[e].pose);
			float eyeM[16]; mat_from_pose(eyeM, stageFromEye);
			float view[16];  mat_rigid_inverse(view, eyeM);
			float proj[16];  proj_from_fov(proj, s_views[e].fov, 0.05f, 100.0f);
			drawTestScene(proj, view);

			const GLenum disc[1] = { GL_DEPTH_ATTACHMENT };
			glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, disc);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			XrSwapchainImageReleaseInfo ri = { XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO };
			xrReleaseSwapchainImage(fb.swapchain, &ri);

			layerViews[e].type = XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW;
			layerViews[e].pose = stageFromEye;
			layerViews[e].fov  = s_views[e].fov;
			layerViews[e].subImage.swapchain = fb.swapchain;
			layerViews[e].subImage.imageRect.offset = { 0, 0 };
			layerViews[e].subImage.imageRect.extent = { s_eyeW, s_eyeH };
			layerViews[e].subImage.imageArrayIndex = 0;
		}
		layer.space = s_stageSpace;
		layer.viewCount = kEyes;
		layer.views = layerViews;
		haveLayer = true;
	}

	XrFrameEndInfo fei = { XR_TYPE_FRAME_END_INFO };
	fei.displayTime = s_frameState.predictedDisplayTime;
	fei.environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
	const XrCompositionLayerBaseHeader* layers[1] = { (XrCompositionLayerBaseHeader*)&layer };
	fei.layerCount = haveLayer ? 1 : 0;
	fei.layers = haveLayer ? layers : nullptr;
	XR_CHECK(xrEndFrame(s_session, &fei));
	return true;
}

#else // !__ANDROID__

extern "C" bool VR_InitOpenXR(void)     { return false; }
extern "C" bool VR_IsActive(void)       { return false; }
extern "C" bool VR_RenderTestFrame(void){ return false; }
extern "C" bool VR_InitEGL(void)        { return false; }
extern "C" bool VR_GetEyeResolution(int* w, int* h) { (void)w; (void)h; return false; }

#endif
