/*
 *  gl_es_compat.cpp  --  stateful fixed-function shim for the Aleph One renderer on OpenGL ES 3.
 *
 *  Implements the CPU matrix stack and immediate-mode colour/normal state declared in
 *  gl_es_compat.h. See that header for the bigger picture. Android-only.
 *
 *  IMPORTANT: this file must NOT include gl_es_compat.h, because that header #defines glGetFloatv
 *  (etc.) to our shims and we need to call the REAL GLES functions here. We include only the GLES
 *  headers and re-declare the few tokens we need.
 */
#if defined(__ANDROID__)

#include <GLES3/gl3.h>
#include <android/log.h>
#include "vr_openxr.h"
#include <array>
#include <vector>
#include <unordered_map>
#include <cstring>
#include <cmath>

/* GLES has no GLdouble; the matrix entry points take double arrays. */
#ifndef A1_HAS_GLDOUBLE
#define A1_HAS_GLDOUBLE
typedef double GLdouble;
typedef double GLclampd;
#endif

/* GL_DOUBLE is absent from the GLES headers but appears as a client-array type (the world
 * renderer submits GLdouble vertex/texcoord arrays); needed by the flush's type handling. */
#ifndef GL_DOUBLE
#define GL_DOUBLE 0x140A
#endif

namespace {

// Matrix-mode tokens (must match gl_es_compat.h).
constexpr GLenum kModelview        = 0x1700;
constexpr GLenum kProjection       = 0x1701;
constexpr GLenum kTextureMatrixMode= 0x1702;
constexpr GLenum kModelviewMatrix  = 0x0BA6;
constexpr GLenum kProjectionMatrix = 0x0BA7;
constexpr GLenum kTextureMatrix    = 0x0BA8;

using Mat = std::array<float, 16>;   // column-major, GL convention: m[col*4 + row]

Mat identity() {
    Mat m{};
    m[0] = m[5] = m[10] = m[15] = 1.0f;
    return m;
}

// C = A * B  (both column-major)
Mat multiply(const Mat& a, const Mat& b) {
    Mat c{};
    for (int col = 0; col < 4; ++col)
        for (int row = 0; row < 4; ++row) {
            float s = 0.0f;
            for (int k = 0; k < 4; ++k)
                s += a[k * 4 + row] * b[col * 4 + k];
            c[col * 4 + row] = s;
        }
    return c;
}

struct Stack {
    std::vector<Mat> s{ identity() };
    Mat& top() { return s.back(); }
};

Stack g_modelview;
Stack g_projection;
Stack g_texture;
Stack* g_current = &g_modelview;

float g_color[4]  = { 1.0f, 1.0f, 1.0f, 1.0f };
float g_normal[3] = { 0.0f, 0.0f, 1.0f };

// Recorded client-array state (consumed later by the draw-flush + rewritten shaders).
struct ClientArray {
    bool        enabled = false;
    GLint       size    = 4;
    GLenum      type     = GL_FLOAT;
    GLsizei     stride   = 0;
    const void* ptr      = nullptr;
};
ClientArray g_vertexArray;
ClientArray g_colorArray;
ClientArray g_normalArray;
ClientArray g_texCoordArray[2];
int   g_clientActiveTex = 0;                 // 0 or 1
float g_constTexCoord[2][4] = { {0,0,0,1}, {0,0,0,1} };  // glMultiTexCoord when array disabled

// post-multiply the current matrix by m: current = current * m
void mult(const Mat& m) { g_current->top() = multiply(g_current->top(), m); }

} // namespace

// ---- exposed shim API (C linkage to match the header declarations) ----
extern "C" {

void a1ffMatrixMode(GLenum mode) {
    switch (mode) {
        case kProjection:        g_current = &g_projection; break;
        case kTextureMatrixMode: g_current = &g_texture;    break;
        case kModelview:
        default:                 g_current = &g_modelview;  break;
    }
}

void a1ffLoadIdentity(void) { g_current->top() = identity(); }

void a1ffPushMatrix(void) { g_current->s.push_back(g_current->top()); }

void a1ffPopMatrix(void)  { if (g_current->s.size() > 1) g_current->s.pop_back(); }

void a1ffLoadMatrixf(const GLfloat* m) { std::memcpy(g_current->top().data(), m, 16 * sizeof(float)); }

void a1ffLoadMatrixd(const GLdouble* m) {
    Mat& t = g_current->top();
    for (int i = 0; i < 16; ++i) t[i] = (float)m[i];
}

void a1ffMultMatrixf(const GLfloat* m) {
    Mat mm; std::memcpy(mm.data(), m, 16 * sizeof(float)); mult(mm);
}

void a1ffMultMatrixd(const GLdouble* m) {
    Mat mm; for (int i = 0; i < 16; ++i) mm[i] = (float)m[i]; mult(mm);
}

void a1ffTranslatef(GLfloat x, GLfloat y, GLfloat z) {
    Mat m = identity();
    m[12] = x; m[13] = y; m[14] = z;
    mult(m);
}

void a1ffScalef(GLfloat x, GLfloat y, GLfloat z) {
    Mat m = identity();
    m[0] = x; m[5] = y; m[10] = z;
    mult(m);
}

void a1ffRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z) {
    float len = std::sqrt(x * x + y * y + z * z);
    if (len < 1e-8f) return;
    x /= len; y /= len; z /= len;
    float rad = angle * 3.14159265358979323846f / 180.0f;
    float c = std::cos(rad), s = std::sin(rad), omc = 1.0f - c;
    Mat m = identity();
    // column-major rotation matrix
    m[0] = x*x*omc + c;     m[1] = y*x*omc + z*s;   m[2]  = x*z*omc - y*s;
    m[4] = x*y*omc - z*s;   m[5] = y*y*omc + c;     m[6]  = y*z*omc + x*s;
    m[8] = x*z*omc + y*s;   m[9] = y*z*omc - x*s;   m[10] = z*z*omc + c;
    mult(m);
}

void a1ffOrtho(GLdouble l, GLdouble r, GLdouble b, GLdouble t, GLdouble n, GLdouble f) {
    Mat m = identity();
    m[0]  = (float)(2.0 / (r - l));
    m[5]  = (float)(2.0 / (t - b));
    m[10] = (float)(-2.0 / (f - n));
    m[12] = (float)(-(r + l) / (r - l));
    m[13] = (float)(-(t + b) / (t - b));
    m[14] = (float)(-(f + n) / (f - n));
    mult(m);
}

void a1ffFrustum(GLdouble l, GLdouble r, GLdouble b, GLdouble t, GLdouble n, GLdouble f) {
    Mat m{};
    m[0]  = (float)(2.0 * n / (r - l));
    m[5]  = (float)(2.0 * n / (t - b));
    m[8]  = (float)((r + l) / (r - l));
    m[9]  = (float)((t + b) / (t - b));
    m[10] = (float)(-(f + n) / (f - n));
    m[11] = -1.0f;
    m[14] = (float)(-2.0 * f * n / (f - n));
    mult(m);
}

void a1ffGetFloatv(GLenum pname, GLfloat* params) {
    switch (pname) {
        case kModelviewMatrix:  std::memcpy(params, g_modelview.top().data(),  16 * sizeof(float)); break;
        case kProjectionMatrix: std::memcpy(params, g_projection.top().data(), 16 * sizeof(float)); break;
        case kTextureMatrix:    std::memcpy(params, g_texture.top().data(),    16 * sizeof(float)); break;
        default:                glGetFloatv(pname, params); break;   // real GLES glGetFloatv
    }
}

void a1ffColor4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
    g_color[0] = r; g_color[1] = g; g_color[2] = b; g_color[3] = a;
}

void a1ffNormal3f(GLfloat x, GLfloat y, GLfloat z) {
    g_normal[0] = x; g_normal[1] = y; g_normal[2] = z;
}

// ---- client vertex arrays ----
static ClientArray* arrayFor(GLenum cap) {
    switch (cap) {
        case 0x8074: return &g_vertexArray;                  // GL_VERTEX_ARRAY
        case 0x8075: return &g_normalArray;                  // GL_NORMAL_ARRAY
        case 0x8076: return &g_colorArray;                   // GL_COLOR_ARRAY
        case 0x8078: return &g_texCoordArray[g_clientActiveTex]; // GL_TEXTURE_COORD_ARRAY
        default:     return nullptr;
    }
}

void a1ffEnableClientState(GLenum cap)  { if (ClientArray* a = arrayFor(cap)) a->enabled = true;  }
void a1ffDisableClientState(GLenum cap) { if (ClientArray* a = arrayFor(cap)) a->enabled = false; }

void a1ffVertexPointer(GLint size, GLenum type, GLsizei stride, const void* ptr) {
    g_vertexArray = { true, size, type, stride, ptr };
}
void a1ffColorPointer(GLint size, GLenum type, GLsizei stride, const void* ptr) {
    g_colorArray = { true, size, type, stride, ptr };
}
void a1ffNormalPointer(GLenum type, GLsizei stride, const void* ptr) {
    g_normalArray = { true, 3, type, stride, ptr };
}
void a1ffTexCoordPointer(GLint size, GLenum type, GLsizei stride, const void* ptr) {
    g_texCoordArray[g_clientActiveTex] = { true, size, type, stride, ptr };
}
void a1ffClientActiveTexture(GLenum tex) {
    int unit = (int)tex - (int)GL_TEXTURE0;
    g_clientActiveTex = (unit == 1) ? 1 : 0;
}
void a1ffMultiTexCoord4f(GLenum tex, GLfloat s, GLfloat t, GLfloat r, GLfloat q) {
    int unit = (int)tex - (int)GL_TEXTURE0;
    if (unit < 0 || unit > 1) unit = 0;
    g_constTexCoord[unit][0] = s; g_constTexCoord[unit][1] = t;
    g_constTexCoord[unit][2] = r; g_constTexCoord[unit][3] = q;
}

void a1ffGetDoublev(GLenum pname, GLdouble* params) {
    float tmp[16];
    switch (pname) {
        case kModelviewMatrix:  std::memcpy(tmp, g_modelview.top().data(),  sizeof tmp); break;
        case kProjectionMatrix: std::memcpy(tmp, g_projection.top().data(), sizeof tmp); break;
        case kTextureMatrix:    std::memcpy(tmp, g_texture.top().data(),    sizeof tmp); break;
        default: return;   // no real glGetDoublev in GLES; other queries are unsupported
    }
    for (int i = 0; i < 16; ++i) params[i] = (GLdouble)tmp[i];
}

} // extern "C"

// Internal accessors for the upcoming draw-flush (Phase 1b continuation).
namespace a1ff {
    const float* currentColor()  { return g_color; }
    const float* currentNormal() { return g_normal; }
    void getMVP(float* out16) {
        Mat mvp = multiply(g_projection.top(), g_modelview.top());
        std::memcpy(out16, mvp.data(), 16 * sizeof(float));
    }
    void getTexMatrix(float* out16) { std::memcpy(out16, g_texture.top().data(), 16 * sizeof(float)); }
}

// ============================================================================
//  Draw flush: turn the recorded fixed-function state into real GLES draws.
//  When no engine shader is bound (the 2D interface / OGL_Blitter path), a tiny
//  built-in program reproduces fixed-function textured/coloured drawing.
// ============================================================================
namespace {

GLboolean g_texture2DEnabled = GL_FALSE;
GLuint    g_currentProgram   = 0;

GLuint g_vao = 0, g_vbo = 0, g_ibo = 0;
GLuint g_builtinProg = 0;
GLint  u_mvp = -1, u_texmat = -1, u_useTex = -1, u_useVColor = -1, u_color = -1, u_tex = -1;

// Recorded fog state (set via glFogf/glFogfv; uploaded as the a1_Fog uniform to engine shaders).
float g_fogColor[4]  = { 0.0f, 0.0f, 0.0f, 1.0f };
float g_fogDensity   = 1.0f;
float g_fogStart     = 0.0f;
float g_fogEnd       = 1.0f;

// Recorded alpha-test state. GLES has no fixed-function alpha test, so shaders discard fragments
// below the reference value when enabled (replicating glEnable(GL_ALPHA_TEST)+glAlphaFunc).
GLboolean g_alphaTestEnabled = GL_FALSE;
float     g_alphaTestRef     = 0.5f;

// Cached a1_* uniform locations for each engine shader program.
struct EngineLocs {
    GLint mvp, mv, mvInv, normalMat, texMat0, texMat1;
    GLint fogColor, fogDensity, fogStart, fogEnd;
    GLint alphaTest, alphaRef;
    GLint brightness;
};
std::unordered_map<GLuint, EngineLocs> g_engineLocs;

const EngineLocs& engineLocsFor(GLuint prog) {
    auto it = g_engineLocs.find(prog);
    if (it != g_engineLocs.end()) return it->second;
    EngineLocs L;
    L.mvp        = glGetUniformLocation(prog, "a1_MVP");
    L.mv         = glGetUniformLocation(prog, "a1_MV");
    L.mvInv      = glGetUniformLocation(prog, "a1_MVInverse");
    L.normalMat  = glGetUniformLocation(prog, "a1_NormalMatrix");
    L.texMat0    = glGetUniformLocation(prog, "a1_TextureMatrix[0]");
    L.texMat1    = glGetUniformLocation(prog, "a1_TextureMatrix[1]");
    L.fogColor   = glGetUniformLocation(prog, "a1_Fog.color");
    L.fogDensity = glGetUniformLocation(prog, "a1_Fog.density");
    L.fogStart   = glGetUniformLocation(prog, "a1_Fog.start");
    L.fogEnd     = glGetUniformLocation(prog, "a1_Fog.end");
    L.alphaTest  = glGetUniformLocation(prog, "a1_AlphaTest");
    L.alphaRef   = glGetUniformLocation(prog, "a1_AlphaRef");
    L.brightness = glGetUniformLocation(prog, "a1_Brightness");
    return g_engineLocs.emplace(prog, L).first->second;
}

// 4x4 inverse (column-major). Returns identity if singular.
Mat invert4(const Mat& m) {
    Mat inv{};
    inv[0]  =  m[5]*m[10]*m[15] - m[5]*m[11]*m[14] - m[9]*m[6]*m[15] + m[9]*m[7]*m[14] + m[13]*m[6]*m[11] - m[13]*m[7]*m[10];
    inv[4]  = -m[4]*m[10]*m[15] + m[4]*m[11]*m[14] + m[8]*m[6]*m[15] - m[8]*m[7]*m[14] - m[12]*m[6]*m[11] + m[12]*m[7]*m[10];
    inv[8]  =  m[4]*m[9]*m[15]  - m[4]*m[11]*m[13] - m[8]*m[5]*m[15] + m[8]*m[7]*m[13] + m[12]*m[5]*m[11] - m[12]*m[7]*m[9];
    inv[12] = -m[4]*m[9]*m[14]  + m[4]*m[10]*m[13] + m[8]*m[5]*m[14] - m[8]*m[6]*m[13] - m[12]*m[5]*m[10] + m[12]*m[6]*m[9];
    inv[1]  = -m[1]*m[10]*m[15] + m[1]*m[11]*m[14] + m[9]*m[2]*m[15] - m[9]*m[3]*m[14] - m[13]*m[2]*m[11] + m[13]*m[3]*m[10];
    inv[5]  =  m[0]*m[10]*m[15] - m[0]*m[11]*m[14] - m[8]*m[2]*m[15] + m[8]*m[3]*m[14] + m[12]*m[2]*m[11] - m[12]*m[3]*m[10];
    inv[9]  = -m[0]*m[9]*m[15]  + m[0]*m[11]*m[13] + m[8]*m[1]*m[15] - m[8]*m[3]*m[13] - m[12]*m[1]*m[11] + m[12]*m[3]*m[9];
    inv[13] =  m[0]*m[9]*m[14]  - m[0]*m[10]*m[13] - m[8]*m[1]*m[14] + m[8]*m[2]*m[13] + m[12]*m[1]*m[10] - m[12]*m[2]*m[9];
    inv[2]  =  m[1]*m[6]*m[15]  - m[1]*m[7]*m[14]  - m[5]*m[2]*m[15] + m[5]*m[3]*m[14] + m[13]*m[2]*m[7]  - m[13]*m[3]*m[6];
    inv[6]  = -m[0]*m[6]*m[15]  + m[0]*m[7]*m[14]  + m[4]*m[2]*m[15] - m[4]*m[3]*m[14] - m[12]*m[2]*m[7]  + m[12]*m[3]*m[6];
    inv[10] =  m[0]*m[5]*m[15]  - m[0]*m[7]*m[13]  - m[4]*m[1]*m[15] + m[4]*m[3]*m[13] + m[12]*m[1]*m[7]  - m[12]*m[3]*m[5];
    inv[14] = -m[0]*m[5]*m[14]  + m[0]*m[6]*m[13]  + m[4]*m[1]*m[14] - m[4]*m[2]*m[13] - m[12]*m[1]*m[6]  + m[12]*m[2]*m[5];
    inv[3]  = -m[1]*m[6]*m[11]  + m[1]*m[7]*m[10]  + m[5]*m[2]*m[11] - m[5]*m[3]*m[10] - m[9]*m[2]*m[7]   + m[9]*m[3]*m[6];
    inv[7]  =  m[0]*m[6]*m[11]  - m[0]*m[7]*m[10]  - m[4]*m[2]*m[11] + m[4]*m[3]*m[10] + m[8]*m[2]*m[7]   - m[8]*m[3]*m[6];
    inv[11] = -m[0]*m[5]*m[11]  + m[0]*m[7]*m[9]   + m[4]*m[1]*m[11] - m[4]*m[3]*m[9]  - m[8]*m[1]*m[7]   + m[8]*m[3]*m[5];
    inv[15] =  m[0]*m[5]*m[10]  - m[0]*m[6]*m[9]   - m[4]*m[1]*m[10] + m[4]*m[2]*m[9]  + m[8]*m[1]*m[6]   - m[8]*m[2]*m[5];
    float det = m[0]*inv[0] + m[1]*inv[4] + m[2]*inv[8] + m[3]*inv[12];
    if (det == 0.0f) return identity();
    float idet = 1.0f / det;
    for (int i = 0; i < 16; ++i) inv[i] *= idet;
    return inv;
}

// Normal matrix = transpose(inverse(upper-left 3x3 of modelview)), written column-major (9 floats).
void normalMatrix3(const Mat& mv, float out9[9]) {
    float a = mv[0], b = mv[1], c = mv[2];
    float d = mv[4], e = mv[5], f = mv[6];
    float g = mv[8], h = mv[9], i = mv[10];
    float det = a*(e*i - f*h) - b*(d*i - f*g) + c*(d*h - e*g);
    if (det == 0.0f) { out9[0]=out9[4]=out9[8]=1.0f; out9[1]=out9[2]=out9[3]=out9[5]=out9[6]=out9[7]=0.0f; return; }
    float id = 1.0f / det;
    // inverse of the 3x3
    float i00=(e*i-f*h)*id, i01=(c*h-b*i)*id, i02=(b*f-c*e)*id;
    float i10=(f*g-d*i)*id, i11=(a*i-c*g)*id, i12=(c*d-a*f)*id;
    float i20=(d*h-e*g)*id, i21=(b*g-a*h)*id, i22=(a*e-b*d)*id;
    // transpose, column-major out[col*3+row]
    out9[0]=i00; out9[1]=i01; out9[2]=i02;
    out9[3]=i10; out9[4]=i11; out9[5]=i12;
    out9[6]=i20; out9[7]=i21; out9[8]=i22;
}

GLuint compileSh(GLenum type, const char* src) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    GLint ok = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetShaderInfoLog(s, sizeof log, nullptr, log);
        __android_log_print(ANDROID_LOG_ERROR, "A1FF", "builtin shader compile failed: %s", log);
    }
    return s;
}

void ensureBuffers() {
    if (g_vao) return;
    glGenVertexArrays(1, &g_vao);
    glGenBuffers(1, &g_vbo);
    glGenBuffers(1, &g_ibo);
}

void ensureBuiltin() {
    if (g_builtinProg) return;

    static const char* kVert =
        "#version 300 es\n"
        "layout(location=0) in vec4 aPos;\n"
        "layout(location=1) in vec2 aTex0;\n"
        "layout(location=3) in vec4 aColor;\n"
        "uniform mat4 uMVP;\n"
        "uniform mat4 uTexMat;\n"
        "uniform int  uUseVColor;\n"
        "uniform vec4 uColor;\n"
        "out vec2 vTex;\n"
        "out vec4 vColor;\n"
        "void main() {\n"
        "  gl_Position = uMVP * aPos;\n"
        "  vTex   = (uTexMat * vec4(aTex0, 0.0, 1.0)).xy;\n"
        "  vColor = (uUseVColor != 0) ? aColor : uColor;\n"
        "}\n";
    static const char* kFrag =
        "#version 300 es\n"
        "precision mediump float;\n"
        "in vec2 vTex;\n"
        "in vec4 vColor;\n"
        "uniform int uUseTex;\n"
        "uniform sampler2D uTex;\n"
        "out vec4 fragColor;\n"
        "void main() {\n"
        "  vec4 c = vColor;\n"
        "  if (uUseTex != 0) c *= texture(uTex, vTex);\n"
        "  fragColor = c;\n"
        "}\n";

    GLuint v = compileSh(GL_VERTEX_SHADER, kVert);
    GLuint f = compileSh(GL_FRAGMENT_SHADER, kFrag);
    g_builtinProg = glCreateProgram();
    glAttachShader(g_builtinProg, v);
    glAttachShader(g_builtinProg, f);
    glLinkProgram(g_builtinProg);
    GLint linked = 0;
    glGetProgramiv(g_builtinProg, GL_LINK_STATUS, &linked);
    if (!linked) {
        char log[1024];
        glGetProgramInfoLog(g_builtinProg, sizeof log, nullptr, log);
        __android_log_print(ANDROID_LOG_ERROR, "A1FF", "builtin program link failed: %s", log);
    }
    glDeleteShader(v);
    glDeleteShader(f);

    u_mvp       = glGetUniformLocation(g_builtinProg, "uMVP");
    u_texmat    = glGetUniformLocation(g_builtinProg, "uTexMat");
    u_useTex    = glGetUniformLocation(g_builtinProg, "uUseTex");
    u_useVColor = glGetUniformLocation(g_builtinProg, "uUseVColor");
    u_color     = glGetUniformLocation(g_builtinProg, "uColor");
    u_tex       = glGetUniformLocation(g_builtinProg, "uTex");

    ensureBuffers();
}

size_t typeSize(GLenum t) {
    switch (t) {
        case GL_DOUBLE:                       return 8;
        case GL_FLOAT: case GL_INT: case GL_UNSIGNED_INT: return 4;
        case GL_SHORT: case GL_UNSIGNED_SHORT:return 2;
        case GL_BYTE:  case GL_UNSIGNED_BYTE: return 1;
        default:                              return 4;
    }
}

float readComp(const void* base, GLenum type, int i, bool normalize) {
    switch (type) {
        case GL_FLOAT:          return ((const float*)base)[i];
        case GL_DOUBLE:         return (float)((const double*)base)[i];
        case GL_INT:            return (float)((const int*)base)[i];
        case GL_UNSIGNED_INT:   return (float)((const unsigned int*)base)[i];
        case GL_SHORT:          { float v = ((const short*)base)[i];          return normalize ? v / 32767.0f : v; }
        case GL_UNSIGNED_SHORT: { float v = ((const unsigned short*)base)[i]; return normalize ? v / 65535.0f : v; }
        case GL_BYTE:           { float v = ((const signed char*)base)[i];    return normalize ? v / 127.0f   : v; }
        case GL_UNSIGNED_BYTE:  { float v = ((const unsigned char*)base)[i];  return normalize ? v / 255.0f   : v; }
        default:                return 0.0f;
    }
}

// Append the attribute for a single (absolute) vertex index to buf; returns component count.
int appendVertex(std::vector<float>& buf, const ClientArray& a, int vertexIndex, bool normalize) {
    size_t stride = a.stride ? (size_t)a.stride : (size_t)a.size * typeSize(a.type);
    const char* vp = (const char*)a.ptr + (size_t)vertexIndex * stride;
    for (int j = 0; j < a.size; ++j) buf.push_back(readComp(vp, a.type, j, normalize));
    return a.size;
}

void setConstAttrib(int loc, const float* v, int n) {
    glDisableVertexAttribArray(loc);
    switch (n) {
        case 2:  glVertexAttrib2fv(loc, v); break;
        case 3:  glVertexAttrib3fv(loc, v); break;
        default: glVertexAttrib4fv(loc, v); break;
    }
}

// 2D / interface path: no engine shader bound -> drive the built-in program.
void flushBuiltin(GLenum mode, const std::vector<int>& verts) {
    ensureBuiltin();

    const bool useTex    = (g_texture2DEnabled && g_texCoordArray[0].enabled && g_texCoordArray[0].ptr);
    const bool useVColor = (g_colorArray.enabled && g_colorArray.ptr);
    const bool colNorm   = (g_colorArray.type != GL_FLOAT && g_colorArray.type != GL_DOUBLE);

    std::vector<float> buf;
    buf.reserve(verts.size() * 8);

    int posComps = g_vertexArray.size;
    size_t posOff = 0;
    for (int v : verts) appendVertex(buf, g_vertexArray, v, false);

    int texComps = 0;
    size_t texOff = buf.size() * sizeof(float);
    if (useTex) { texComps = g_texCoordArray[0].size; for (int v : verts) appendVertex(buf, g_texCoordArray[0], v, false); }

    int colComps = 0;
    size_t colOff = buf.size() * sizeof(float);
    if (useVColor) { colComps = g_colorArray.size; for (int v : verts) appendVertex(buf, g_colorArray, v, colNorm); }

    glBindVertexArray(g_vao);
    glBindBuffer(GL_ARRAY_BUFFER, g_vbo);
    glBufferData(GL_ARRAY_BUFFER, buf.size() * sizeof(float), buf.data(), GL_STREAM_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, posComps, GL_FLOAT, GL_FALSE, 0, (const void*)posOff);

    if (useTex) { glEnableVertexAttribArray(1); glVertexAttribPointer(1, texComps, GL_FLOAT, GL_FALSE, 0, (const void*)texOff); }
    else        { glDisableVertexAttribArray(1); }

    if (useVColor) { glEnableVertexAttribArray(3); glVertexAttribPointer(3, colComps, GL_FLOAT, GL_FALSE, 0, (const void*)colOff); }
    else           { glDisableVertexAttribArray(3); }

    glUseProgram(g_builtinProg);
    float mvp[16];
    { Mat m = multiply(g_projection.top(), g_modelview.top()); std::memcpy(mvp, m.data(), sizeof mvp); }
    glUniformMatrix4fv(u_mvp, 1, GL_FALSE, mvp);
    glUniformMatrix4fv(u_texmat, 1, GL_FALSE, g_texture.top().data());
    glUniform1i(u_useTex, useTex ? 1 : 0);
    glUniform1i(u_useVColor, useVColor ? 1 : 0);
    glUniform4fv(u_color, 1, g_color);
    glUniform1i(u_tex, 0);

    glDrawArrays(mode, 0, (GLsizei)verts.size());
    glUseProgram(0);
}

// 3D world path: an engine shader is bound. Bind all five fixed-function attributes (with
// constant fallbacks) and feed the matrix/fog uniforms the rewritten shaders read via a1_*.
void flushEngine(GLenum mode, const std::vector<int>& verts) {
    ensureBuffers();

    struct A { int loc; ClientArray* arr; const float* cst; int cstN; bool norm; bool isArray; size_t off; int comps; };
    bool colNorm = (g_colorArray.type != GL_FLOAT && g_colorArray.type != GL_DOUBLE);
    A attrs[5] = {
        { 0, &g_vertexArray,       g_normal,           3, false,   false, 0, 0 },  // position (no sane constant)
        { 1, &g_texCoordArray[0],  g_constTexCoord[0], 4, false,   false, 0, 0 },
        { 2, &g_texCoordArray[1],  g_constTexCoord[1], 4, false,   false, 0, 0 },
        { 3, &g_colorArray,        g_color,            4, colNorm, false, 0, 0 },
        { 4, &g_normalArray,       g_normal,           3, false,   false, 0, 0 },
    };
    for (auto& a : attrs) a.isArray = (a.arr->enabled && a.arr->ptr);
    attrs[0].isArray = true;   // position array is required (checked by caller)

    std::vector<float> buf;
    buf.reserve(verts.size() * 16);
    for (auto& a : attrs) {
        if (!a.isArray) continue;
        a.off = buf.size() * sizeof(float);
        a.comps = a.arr->size;
        for (int v : verts) appendVertex(buf, *a.arr, v, a.norm);
    }

    glBindVertexArray(g_vao);
    glBindBuffer(GL_ARRAY_BUFFER, g_vbo);
    glBufferData(GL_ARRAY_BUFFER, buf.size() * sizeof(float), buf.data(), GL_STREAM_DRAW);

    for (auto& a : attrs) {
        if (a.isArray) {
            glEnableVertexAttribArray(a.loc);
            glVertexAttribPointer(a.loc, a.comps, GL_FLOAT, GL_FALSE, 0, (const void*)a.off);
        } else {
            setConstAttrib(a.loc, a.cst, a.cstN);
        }
    }

    const EngineLocs& L = engineLocsFor(g_currentProgram);
    Mat mv  = g_modelview.top();
    Mat mvp = multiply(g_projection.top(), mv);
    if (L.mvp   >= 0) glUniformMatrix4fv(L.mvp,   1, GL_FALSE, mvp.data());
    if (L.mv    >= 0) glUniformMatrix4fv(L.mv,    1, GL_FALSE, mv.data());
    if (L.mvInv >= 0) { Mat inv = invert4(mv); glUniformMatrix4fv(L.mvInv, 1, GL_FALSE, inv.data()); }
    if (L.normalMat >= 0) { float nm[9]; normalMatrix3(mv, nm); glUniformMatrix3fv(L.normalMat, 1, GL_FALSE, nm); }
    if (L.texMat0 >= 0) glUniformMatrix4fv(L.texMat0, 1, GL_FALSE, g_texture.top().data());
    if (L.texMat1 >= 0) glUniformMatrix4fv(L.texMat1, 1, GL_FALSE, g_texture.top().data());
    if (L.fogColor   >= 0) glUniform4fv(L.fogColor, 1, g_fogColor);
    if (L.fogDensity >= 0) glUniform1f(L.fogDensity, g_fogDensity);
    if (L.fogStart   >= 0) glUniform1f(L.fogStart, g_fogStart);
    if (L.fogEnd     >= 0) glUniform1f(L.fogEnd, g_fogEnd);
    if (L.alphaTest  >= 0) glUniform1i(L.alphaTest, g_alphaTestEnabled ? 1 : 0);
    if (L.alphaRef   >= 0) glUniform1f(L.alphaRef, g_alphaTestRef);
    if (L.brightness >= 0) glUniform1f(L.brightness, VR_IsActive() ? VR_Settings()->brightness : 1.0f);

    glDrawArrays(mode, 0, (GLsizei)verts.size());
}

void flushIndexed(GLenum mode, const std::vector<int>& verts) {
    if (!g_vertexArray.enabled || !g_vertexArray.ptr || verts.empty()) return;
    if (g_currentProgram != 0) flushEngine(mode, verts);
    else                       flushBuiltin(mode, verts);
}

} // namespace

extern "C" {

void a1ffSetTexture2D(GLboolean enabled) { g_texture2DEnabled = enabled; }

void a1ffSetAlphaTest(GLboolean enabled) { g_alphaTestEnabled = enabled; }
void a1ffAlphaFunc(GLenum /*func*/, GLclampf ref) { g_alphaTestRef = ref; }

void a1ffFrontFace(GLenum mode) {
	// The VR modelview now matches the engine's handedness (det=-1 remap), so pass the winding
	// through unchanged.
	glFrontFace(mode);
}

void a1ffUseProgram(GLuint program) { g_currentProgram = program; glUseProgram(program); }

// Fog parameter capture (GL_FOG_* tokens; values per classic glFog semantics).
void a1ffFogf(GLenum pname, GLfloat param) {
    switch (pname) {
        case 0x0B62: g_fogDensity = param; break;  // GL_FOG_DENSITY
        case 0x0B63: g_fogStart   = param; break;  // GL_FOG_START
        case 0x0B64: g_fogEnd     = param; break;  // GL_FOG_END
        default: break;
    }
}
void a1ffFogi(GLenum pname, GLint param) { a1ffFogf(pname, (GLfloat)param); }
void a1ffFogfv(GLenum pname, const GLfloat* params) {
    if (pname == 0x0B66) {                         // GL_FOG_COLOR
        g_fogColor[0] = params[0]; g_fogColor[1] = params[1];
        g_fogColor[2] = params[2]; g_fogColor[3] = params[3];
    } else {
        a1ffFogf(pname, params[0]);
    }
}

void a1ffDrawArrays(GLenum mode, GLint first, GLsizei count) {
    if (count <= 0) return;
    std::vector<int> verts;
    verts.reserve(count);
    for (GLsizei i = 0; i < count; ++i) verts.push_back(first + i);
    flushIndexed(mode, verts);
}

void a1ffDrawElements(GLenum mode, GLsizei count, GLenum type, const void* indices) {
    if (count <= 0 || !indices) return;
    std::vector<int> verts;
    verts.reserve(count);
    for (GLsizei i = 0; i < count; ++i) {
        int idx = 0;
        switch (type) {
            case GL_UNSIGNED_BYTE:  idx = ((const unsigned char*)indices)[i];  break;
            case GL_UNSIGNED_SHORT: idx = ((const unsigned short*)indices)[i]; break;
            case GL_UNSIGNED_INT:   idx = (int)((const unsigned int*)indices)[i]; break;
            default: return;
        }
        verts.push_back(idx);
    }
    flushIndexed(mode, verts);
}

} // extern "C"

#endif /* __ANDROID__ */
