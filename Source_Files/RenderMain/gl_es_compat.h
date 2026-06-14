/*
 *  gl_es_compat.h  --  OpenGL ES 3 compatibility shims for the Aleph One renderer (Android/Quest).
 *
 *  Included only on Android (from OGL_Headers.h, after <GLES3/gl3.h>). Desktop builds never
 *  see this file, so the existing desktop GL paths are completely unaffected.
 *
 *  It bridges the parts of the legacy renderer that use APIs absent from OpenGL ES:
 *    - ARB shader-object API            -> core GLSL shader API
 *    - EXT framebuffer-object API       -> core FBO API
 *    - desktop sized texture formats    -> GLES-valid sized formats
 *    - GL_TEXTURE_RECTANGLE_ARB         -> GL_TEXTURE_2D (GLES3 has NPOT 2D)
 *    - GL_POLYGON / GL_QUADS            -> GL_TRIANGLE_FAN (Marathon polys are convex; a
 *                                          single 4-vertex quad is a valid 2-triangle fan)
 *    - fixed-function state setters     -> no-ops (TEMPORARY: alpha test / clip planes / fog /
 *                                          texenv / logic-op will move into shaders in Phase 1b)
 *    - GLU helpers                      -> small inline implementations
 *
 *  NOT handled here (needs the stateful shim in Phase 1b): the matrix stack (glMatrixMode,
 *  glLoadMatrix*, glRotate*, glTranslate*, glPushMatrix...), client vertex arrays
 *  (glVertexPointer/glColorPointer/...), and immediate colour (glColor*). Those carry state and
 *  must be emulated with a CPU matrix stack + VBOs + a generic shader.
 */
#ifndef _GL_ES_COMPAT_H
#define _GL_ES_COMPAT_H

#if defined(__ANDROID__)

#include <GLES3/gl3.h>

/* GLES has no double-precision GL types, but the legacy renderer uses GLdouble/GLclampd
 * (e.g. Screen_2_Clip, glLoadMatrixd, glClipPlane). Provide them. */
#ifndef A1_HAS_GLDOUBLE
#define A1_HAS_GLDOUBLE
typedef double GLdouble;
typedef double GLclampd;
#endif

/* ---- ARB shader-object API -> core ---- */
typedef GLuint GLhandleARB;
typedef char   GLcharARB;
#define glCreateShaderObjectARB   glCreateShader
#define glCreateProgramObjectARB  glCreateProgram
#define glShaderSourceARB         glShaderSource
#define glCompileShaderARB        glCompileShader
#define glAttachObjectARB         glAttachShader
#define glLinkProgramARB          glLinkProgram
#define glUseProgramObjectARB     glUseProgram
#define glGetUniformLocationARB   glGetUniformLocation
#define glUniform1iARB            glUniform1i
#define glUniform1fARB            glUniform1f
#define glUniform2fARB            glUniform2f
#define glUniform3fARB            glUniform3f
#define glUniform4fARB            glUniform4f
#define glUniformMatrix4fvARB     glUniformMatrix4fv
#define GL_VERTEX_SHADER_ARB      GL_VERTEX_SHADER
#define GL_FRAGMENT_SHADER_ARB    GL_FRAGMENT_SHADER
#define GL_OBJECT_COMPILE_STATUS_ARB GL_COMPILE_STATUS
#define GL_OBJECT_LINK_STATUS_ARB    GL_LINK_STATUS

/* ARB unified one function where core has two. glGetObjectParameterivARB is only ever called
 * for a shader's compile status here, so route it to glGetShaderiv. glDeleteObjectARB is used
 * on shaders (OK) and on one program (OGL_Shader.cpp:362, edited to glDeleteProgram). */
static inline void glGetObjectParameterivARB(GLuint obj, GLenum pname, GLint* params) {
    glGetShaderiv(obj, pname, params);
}
#define glDeleteObjectARB         glDeleteShader

/* ---- EXT framebuffer-object API -> core (identical semantics) ---- */
#define glBindFramebufferEXT          glBindFramebuffer
#define glBindRenderbufferEXT         glBindRenderbuffer
#define glGenFramebuffersEXT          glGenFramebuffers
#define glGenRenderbuffersEXT         glGenRenderbuffers
#define glDeleteFramebuffersEXT       glDeleteFramebuffers
#define glDeleteRenderbuffersEXT      glDeleteRenderbuffers
#define glCheckFramebufferStatusEXT   glCheckFramebufferStatus
#define glFramebufferRenderbufferEXT  glFramebufferRenderbuffer
#define glRenderbufferStorageEXT      glRenderbufferStorage
#define GL_FRAMEBUFFER_EXT            GL_FRAMEBUFFER
#define GL_RENDERBUFFER_EXT           GL_RENDERBUFFER
#define GL_DEPTH_ATTACHMENT_EXT       GL_DEPTH_ATTACHMENT
#define GL_FRAMEBUFFER_COMPLETE_EXT   GL_FRAMEBUFFER_COMPLETE
/* glFramebufferTexture2DEXT / color attachment if present elsewhere */
#define glFramebufferTexture2DEXT     glFramebufferTexture2D
#define GL_COLOR_ATTACHMENT0_EXT      GL_COLOR_ATTACHMENT0

/* sRGB write control: no core GLES equivalent. Provide the token; glEnable/glDisable of it is a
 * harmless no-op via the wrapper below. */
#ifndef GL_FRAMEBUFFER_SRGB_EXT
#define GL_FRAMEBUFFER_SRGB_EXT       0x8DB9
#endif
#ifndef GL_FRAMEBUFFER_SRGB
#define GL_FRAMEBUFFER_SRGB           0x8DB9
#endif

/* ---- Desktop sized texture formats -> GLES-valid sized formats ----
 * The legacy code requested reduced bit depths to save VRAM; on a modern tiled GPU we just use
 * full 8-bit. Mapping the internalformat argument of glTexImage2D. */
#define GL_RGB4     GL_RGB8
#define GL_RGB5     GL_RGB8
#define GL_RGB8_OES GL_RGB8
#define GL_RGB10    GL_RGB8
#define GL_RGB12    GL_RGB8
#define GL_RGB16    GL_RGB8
#define GL_R3_G3_B2 GL_RGB8
#define GL_RGBA2    GL_RGBA8
#define GL_RGBA4_OES GL_RGBA4
#define GL_RGBA12   GL_RGBA8
#define GL_RGBA16   GL_RGBA8

/* Rectangle textures don't exist in GLES; GLES3 supports NPOT GL_TEXTURE_2D. */
#define GL_TEXTURE_RECTANGLE_ARB  GL_TEXTURE_2D

/* ---- ARB multitexture -> core ---- */
#define glActiveTextureARB        glActiveTexture
#define GL_TEXTURE0_ARB           GL_TEXTURE0
#define GL_TEXTURE1_ARB           GL_TEXTURE1
#define GL_TEXTURE2_ARB           GL_TEXTURE2
#define GL_TEXTURE3_ARB           GL_TEXTURE3
/* glClientActiveTextureARB and glMultiTexCoord*ARB are fixed-function vertex submission and are
 * handled by the stateful shim (Phase 1b), not here. */

/* Texture wrap: GLES has no GL_CLAMP (clamp-to-border); GL_CLAMP_TO_EDGE is the equivalent. */
#ifndef GL_CLAMP
#define GL_CLAMP  GL_CLAMP_TO_EDGE
#endif

/* ---- Primitive types absent from GLES ----
 * Marathon's world/sprite polygons are convex and drawn one at a time, so a triangle fan
 * reproduces both GL_POLYGON and a single 4-vertex GL_QUADS exactly. */
#ifndef GL_POLYGON
#define GL_POLYGON  GL_TRIANGLE_FAN
#endif
#ifndef GL_QUADS
#define GL_QUADS    GL_TRIANGLE_FAN
#endif

/* ---- Fixed-function state tokens (TEMPORARY no-ops) ----
 * These pipeline features move into the shaders in Phase 1b. For now define the tokens so the
 * code compiles, no-op the setter functions, and swallow them in glEnable/glDisable so they
 * don't raise GL_INVALID_ENUM. Rendering will be missing alpha-test/clip/fog until Phase 1b. */
#ifndef GL_ALPHA_TEST
#define GL_ALPHA_TEST       0x0BC0
#endif
#ifndef GL_CLIP_PLANE0
#define GL_CLIP_PLANE0      0x3000
#define GL_CLIP_PLANE1      0x3001
#define GL_CLIP_PLANE2      0x3002
#define GL_CLIP_PLANE3      0x3003
#define GL_CLIP_PLANE4      0x3004
#define GL_CLIP_PLANE5      0x3005
#endif
#ifndef GL_FOG
#define GL_FOG              0x0B60
#define GL_FOG_DENSITY      0x0B62
#define GL_FOG_START        0x0B63
#define GL_FOG_END          0x0B64
#define GL_FOG_MODE         0x0B65
#define GL_FOG_COLOR        0x0B66
#endif
#ifndef GL_COLOR_LOGIC_OP
#define GL_COLOR_LOGIC_OP   0x0BF2
#endif
#ifndef GL_XOR
#define GL_XOR              0x1506
#endif
#ifndef GL_VIEWPORT_BIT
#define GL_VIEWPORT_BIT     0x00000800
#endif
static inline void a1es_noop_logicop(GLenum) {}
#define glLogicOp  a1es_noop_logicop
#ifndef GL_POLYGON_STIPPLE
#define GL_POLYGON_STIPPLE  0x0B42
#endif
static inline void a1es_noop_polygonstipple(const GLubyte*) {}
#define glPolygonStipple  a1es_noop_polygonstipple
#ifndef GL_TEXTURE_ENV
#define GL_TEXTURE_ENV      0x2300
#define GL_TEXTURE_ENV_MODE 0x2200
#define GL_MODULATE         0x2100
#endif

static inline void a1es_noop_alphafunc(GLenum, GLclampf) {}
static inline void a1es_noop_clipplane(GLenum, const GLdouble*) {}
static inline void a1es_noop_fogf(GLenum, GLfloat) {}
static inline void a1es_noop_fogi(GLenum, GLint) {}
static inline void a1es_noop_fogfv(GLenum, const GLfloat*) {}
static inline void a1es_noop_texenvi(GLenum, GLenum, GLint) {}
static inline void a1es_noop_texenvf(GLenum, GLenum, GLfloat) {}
static inline void a1es_noop_pushattrib(GLbitfield) {}
static inline void a1es_noop_popattrib(void) {}
#define glAlphaFunc   a1es_noop_alphafunc
#define glClipPlane   a1es_noop_clipplane
#define glFogf        a1es_noop_fogf
#define glFogi        a1es_noop_fogi
#define glFogfv       a1es_noop_fogfv
#define glTexEnvi     a1es_noop_texenvi
#define glTexEnvf     a1es_noop_texenvf
#define glPushAttrib  a1es_noop_pushattrib
#define glPopAttrib   a1es_noop_popattrib

/* GL_TEXTURE_2D is a fixed-function enable in desktop GL; in GLES texturing is decided by the
 * bound shader. Track its state in the shim (consumed by the draw-flush) and never forward it to
 * the GLES driver (which would raise GL_INVALID_ENUM). */
#ifdef __cplusplus
extern "C" {
#endif
void a1ffSetTexture2D(GLboolean enabled);
#ifdef __cplusplus
}
#endif

/* Wrap glEnable/glDisable so the fixed-function tokens above are silently ignored while real
 * GLES capabilities (GL_BLEND, GL_DEPTH_TEST, GL_CULL_FACE, ...) pass through unchanged. */
static inline bool a1es_is_ff_cap(GLenum cap) {
    switch (cap) {
        case GL_ALPHA_TEST: case GL_COLOR_LOGIC_OP: case GL_FOG:
        case GL_TEXTURE_ENV: case GL_FRAMEBUFFER_SRGB_EXT:
        case GL_CLIP_PLANE0: case GL_CLIP_PLANE1: case GL_CLIP_PLANE2:
        case GL_CLIP_PLANE3: case GL_CLIP_PLANE4: case GL_CLIP_PLANE5:
            return true;
        default:
            return false;
    }
}
static inline void a1es_enable(GLenum cap)  { if (cap == GL_TEXTURE_2D) { a1ffSetTexture2D(GL_TRUE);  return; } if (!a1es_is_ff_cap(cap)) glEnable(cap); }
static inline void a1es_disable(GLenum cap) { if (cap == GL_TEXTURE_2D) { a1ffSetTexture2D(GL_FALSE); return; } if (!a1es_is_ff_cap(cap)) glDisable(cap); }
#define glEnable   a1es_enable
#define glDisable  a1es_disable

/* ---- GLU helpers ---- */
/* Upload base level then let the driver build the mip chain. */
static inline int gluBuild2DMipmaps(GLenum target, GLint internalformat, GLsizei width,
                                    GLsizei height, GLenum format, GLenum type, const void* data) {
    glTexImage2D(target, 0, internalformat, width, height, 0, format, type, data);
    glGenerateMipmap(target);
    return 0;
}
/* GLES3 supports NPOT textures, so rescaling is rarely needed. Provide a simple nearest-neighbour
 * resampler for the GL_UNSIGNED_BYTE case the renderer uses. */
static inline int gluScaleImage(GLenum format, GLsizei wIn, GLsizei hIn, GLenum typeIn,
                                const void* dataIn, GLsizei wOut, GLsizei hOut, GLenum typeOut,
                                void* dataOut) {
    (void)typeIn; (void)typeOut;
    int comps = (format == GL_RGBA) ? 4 : (format == GL_RGB) ? 3 : 1;
    const unsigned char* src = (const unsigned char*)dataIn;
    unsigned char* dst = (unsigned char*)dataOut;
    for (int y = 0; y < hOut; ++y) {
        int sy = (hOut > 1) ? (y * (hIn - 1)) / (hOut - 1) : 0;
        for (int x = 0; x < wOut; ++x) {
            int sx = (wOut > 1) ? (x * (wIn - 1)) / (wOut - 1) : 0;
            const unsigned char* sp = src + (sy * wIn + sx) * comps;
            unsigned char* dp = dst + (y * wOut + x) * comps;
            for (int c = 0; c < comps; ++c) dp[c] = sp[c];
        }
    }
    return 0;
}

/* ============================================================================
 * Stateful fixed-function shim (Phase 1b) -- implemented in gl_es_compat.cpp.
 *
 * Emulates the legacy fixed-function pipeline the renderer still uses:
 *   - a CPU matrix stack (modelview / projection / texture)
 *   - "current" colour and normal (immediate-mode glColor / glNormal3f)
 * A later step adds client-array recording + a flush-on-glDraw* that uploads the
 * current matrices/colour to the bound shader. The renderer's GLSL shaders are being
 * rewritten to read these as explicit uniforms/attributes (no gl_* built-ins on GLES).
 * ============================================================================ */

/* Matrix-mode + matrix-query tokens (classic values; absent from GLES). */
#ifndef GL_MODELVIEW
#define GL_MODELVIEW         0x1700
#define GL_PROJECTION        0x1701
#define GL_TEXTURE_MATRIX_MODE 0x1702   /* GL_TEXTURE collides with a sampler enum; use our own */
#endif
#ifndef GL_MODELVIEW_MATRIX
#define GL_MODELVIEW_MATRIX  0x0BA6
#define GL_PROJECTION_MATRIX 0x0BA7
#define GL_TEXTURE_MATRIX    0x0BA8
#endif
#ifndef GL_ALL_ATTRIB_BITS
#define GL_ALL_ATTRIB_BITS   0x000FFFFF
#define GL_ENABLE_BIT        0x00002000
#endif

#ifdef __cplusplus
extern "C" {
#endif
void a1ffMatrixMode(GLenum mode);
void a1ffLoadIdentity(void);
void a1ffPushMatrix(void);
void a1ffPopMatrix(void);
void a1ffLoadMatrixf(const GLfloat* m);
void a1ffLoadMatrixd(const GLdouble* m);
void a1ffMultMatrixf(const GLfloat* m);
void a1ffMultMatrixd(const GLdouble* m);
void a1ffRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
void a1ffTranslatef(GLfloat x, GLfloat y, GLfloat z);
void a1ffScalef(GLfloat x, GLfloat y, GLfloat z);
void a1ffOrtho(GLdouble l, GLdouble r, GLdouble b, GLdouble t, GLdouble n, GLdouble f);
void a1ffFrustum(GLdouble l, GLdouble r, GLdouble b, GLdouble t, GLdouble n, GLdouble f);
void a1ffGetFloatv(GLenum pname, GLfloat* params);   /* intercepts the *_MATRIX queries */
void a1ffColor4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
void a1ffNormal3f(GLfloat x, GLfloat y, GLfloat z);
#ifdef __cplusplus
}
#endif

/* double-argument convenience wrappers */
static inline void a1ffRotated(GLdouble a, GLdouble x, GLdouble y, GLdouble z) { a1ffRotatef((GLfloat)a,(GLfloat)x,(GLfloat)y,(GLfloat)z); }
static inline void a1ffTranslated(GLdouble x, GLdouble y, GLdouble z) { a1ffTranslatef((GLfloat)x,(GLfloat)y,(GLfloat)z); }
static inline void a1ffScaled(GLdouble x, GLdouble y, GLdouble z) { a1ffScalef((GLfloat)x,(GLfloat)y,(GLfloat)z); }
static inline void a1ffDepthRange(GLdouble n, GLdouble f) { glDepthRangef((GLfloat)n,(GLfloat)f); }
/* immediate-colour variants -> a1ffColor4f */
static inline void a1ffColor3f(GLfloat r, GLfloat g, GLfloat b) { a1ffColor4f(r,g,b,1.0f); }
static inline void a1ffColor4fv(const GLfloat* v) { a1ffColor4f(v[0],v[1],v[2],v[3]); }
static inline void a1ffColor3fv(const GLfloat* v) { a1ffColor4f(v[0],v[1],v[2],1.0f); }
static inline void a1ffColor4ub(GLubyte r, GLubyte g, GLubyte b, GLubyte a) { a1ffColor4f(r/255.0f,g/255.0f,b/255.0f,a/255.0f); }
static inline void a1ffColor3ub(GLubyte r, GLubyte g, GLubyte b) { a1ffColor4f(r/255.0f,g/255.0f,b/255.0f,1.0f); }
static inline void a1ffColor3us(GLushort r, GLushort g, GLushort b) { a1ffColor4f(r/65535.0f,g/65535.0f,b/65535.0f,1.0f); }
static inline void a1ffColor4us(GLushort r, GLushort g, GLushort b, GLushort a) { a1ffColor4f(r/65535.0f,g/65535.0f,b/65535.0f,a/65535.0f); }
static inline void a1ffColor4usv(const GLushort* v) { a1ffColor4f(v[0]/65535.0f,v[1]/65535.0f,v[2]/65535.0f,v[3]/65535.0f); }
static inline void a1ffColor3usv(const GLushort* v) { a1ffColor4f(v[0]/65535.0f,v[1]/65535.0f,v[2]/65535.0f,1.0f); }

#define glMatrixMode   a1ffMatrixMode
#define glLoadIdentity a1ffLoadIdentity
#define glPushMatrix   a1ffPushMatrix
#define glPopMatrix    a1ffPopMatrix
#define glLoadMatrixf  a1ffLoadMatrixf
#define glLoadMatrixd  a1ffLoadMatrixd
#define glMultMatrixf  a1ffMultMatrixf
#define glMultMatrixd  a1ffMultMatrixd
#define glRotatef      a1ffRotatef
#define glRotated      a1ffRotated
#define glTranslatef   a1ffTranslatef
#define glTranslated   a1ffTranslated
#define glScalef       a1ffScalef
#define glScaled       a1ffScaled
#define glOrtho        a1ffOrtho
#define glFrustum      a1ffFrustum
#define glDepthRange   a1ffDepthRange
#define glGetFloatv    a1ffGetFloatv
#define glColor4f      a1ffColor4f
#define glColor3f      a1ffColor3f
#define glColor4fv     a1ffColor4fv
#define glColor3fv     a1ffColor3fv
#define glColor4ub     a1ffColor4ub
#define glColor3ub     a1ffColor3ub
#define glColor3us     a1ffColor3us
#define glColor4us     a1ffColor4us
#define glColor4usv    a1ffColor4usv
#define glColor3usv    a1ffColor3usv
#define glNormal3f     a1ffNormal3f

/* ---- Client vertex arrays (fixed-function) -> recorded for the draw flush ---- */
#ifndef GL_VERTEX_ARRAY
#define GL_VERTEX_ARRAY        0x8074
#define GL_NORMAL_ARRAY        0x8075
#define GL_COLOR_ARRAY         0x8076
#define GL_TEXTURE_COORD_ARRAY 0x8078
#endif
#ifndef GL_DOUBLE
#define GL_DOUBLE              0x140A
#endif
#ifndef GL_TEXTURE
#define GL_TEXTURE             0x1702   /* used as a glMatrixMode() target by the renderer */
#endif

#ifdef __cplusplus
extern "C" {
#endif
void a1ffEnableClientState(GLenum cap);
void a1ffDisableClientState(GLenum cap);
void a1ffVertexPointer(GLint size, GLenum type, GLsizei stride, const void* ptr);
void a1ffColorPointer(GLint size, GLenum type, GLsizei stride, const void* ptr);
void a1ffNormalPointer(GLenum type, GLsizei stride, const void* ptr);
void a1ffTexCoordPointer(GLint size, GLenum type, GLsizei stride, const void* ptr);
void a1ffClientActiveTexture(GLenum tex);
void a1ffMultiTexCoord4f(GLenum tex, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
void a1ffGetDoublev(GLenum pname, GLdouble* params);
#ifdef __cplusplus
}
#endif
#define glEnableClientState      a1ffEnableClientState
#define glDisableClientState     a1ffDisableClientState
#define glVertexPointer          a1ffVertexPointer
#define glColorPointer           a1ffColorPointer
#define glNormalPointer          a1ffNormalPointer
#define glTexCoordPointer        a1ffTexCoordPointer
#define glClientActiveTexture    a1ffClientActiveTexture
#define glClientActiveTextureARB a1ffClientActiveTexture
#define glMultiTexCoord4f        a1ffMultiTexCoord4f
#define glMultiTexCoord4fARB     a1ffMultiTexCoord4f
#define glGetDoublev             a1ffGetDoublev

/* ---- Draw flush ----
 * glDrawArrays/glDrawElements are intercepted so the recorded matrix stack, immediate colour and
 * client arrays are turned into real GLES draws. When no shader program is bound (the 2D
 * interface/blitter path, which relied on fixed-function), a built-in GLES program emulates it.
 * glUseProgram is tracked so the flush can tell the FF path from the engine's own shaders. */
#ifdef __cplusplus
extern "C" {
#endif
void a1ffUseProgram(GLuint program);
void a1ffDrawArrays(GLenum mode, GLint first, GLsizei count);
void a1ffDrawElements(GLenum mode, GLsizei count, GLenum type, const void* indices);
#ifdef __cplusplus
}
#endif
#define glUseProgram   a1ffUseProgram
#define glDrawArrays   a1ffDrawArrays
#define glDrawElements a1ffDrawElements

/* Display lists don't exist in GLES (FontHandler). No-op for now; fonts will be reworked to
 * render textured quads in Phase 1b. */
#ifndef GL_COMPILE
#define GL_COMPILE        0x1300
#define GL_COMPILE_AND_EXECUTE 0x1301
#endif
static inline GLuint a1ffGenLists(GLsizei) { return 0; }
static inline void a1ffNewList(GLuint, GLenum) {}
static inline void a1ffEndList(void) {}
static inline void a1ffCallList(GLuint) {}
static inline void a1ffDeleteLists(GLuint, GLsizei) {}
static inline void a1ffListBase(GLuint) {}
#define glGenLists    a1ffGenLists
#define glNewList     a1ffNewList
#define glEndList     a1ffEndList
#define glCallList    a1ffCallList
#define glDeleteLists a1ffDeleteLists
#define glListBase    a1ffListBase

#endif /* __ANDROID__ */
#endif /* _GL_ES_COMPAT_H */
