#ifndef OPENXR_HELPERS_H_
#define OPENXR_HELPERS_H_

#include <xr_linear.h>

static inline XrTime ToXrTime(const double timeInSeconds) {
    return (timeInSeconds * 1e9);
}

static inline double FromXrTime(const XrTime time) {
    return (time * 1e-9);
}

static inline XrQuaternionf XrQuaternionf_Identity() {
    XrQuaternionf r;
    r.x = r.y = r.z = 0.0;
    r.w = 1.0f;
    return r;
}

static inline XrVector3f XrVector3f_Zero() {
    XrVector3f r;
    r.x = r.y = r.z = 0.0f;
    return r;
}

static inline float XrVector3f_LengthSquared(const XrVector3f v) {
    return XrVector3f_Dot(&v, &v);
}

static inline XrVector3f XrVector3f_ScalarMultiply(const XrVector3f v, float scale) {
    XrVector3f u;
    u.x = v.x * scale;
    u.y = v.y * scale;
    u.z = v.z * scale;
    return u;
}

static inline XrVector3f XrVector3f_Normalized(const XrVector3f v) {
    float rcpLen = 1.0f / XrVector3f_Length(&v);
    return XrVector3f_ScalarMultiply(v, rcpLen);
}

static inline XrQuaternionf XrQuaternionf_Inverse(const XrQuaternionf q) {
    XrQuaternionf r;
    r.x = -q.x;
    r.y = -q.y;
    r.z = -q.z;
    r.w = q.w;
    return r;
}

static inline XrVector3f XrQuaternionf_Rotate(const XrQuaternionf a, const XrVector3f v) {
    XrVector3f r;
    XrQuaternionf q = {v.x, v.y, v.z, 0.0f};
    XrQuaternionf aq;
    XrQuaternionf_Multiply(&aq, &q, &a);
    XrQuaternionf aInv = XrQuaternionf_Inverse(a);
    XrQuaternionf aqaInv;
    XrQuaternionf_Multiply(&aqaInv, &aInv, &aq);
    r.x = aqaInv.x;
    r.y = aqaInv.y;
    r.z = aqaInv.z;
    return r;
}

static inline XrQuaternionf XrQuaternionf_CreateFromVectorAngle(
        const XrVector3f axis,
        const float angle) {
    XrQuaternionf r;
    if (XrVector3f_LengthSquared(axis) == 0.0f) {
        return XrQuaternionf_Identity();
    }

    XrVector3f unitAxis = XrVector3f_Normalized(axis);
    float sinHalfAngle = sinf(angle * 0.5f);

    r.w = cosf(angle * 0.5f);
    r.x = unitAxis.x * sinHalfAngle;
    r.y = unitAxis.y * sinHalfAngle;
    r.z = unitAxis.z * sinHalfAngle;
    return r;
}

static inline XrPosef XrPosef_Identity() {
    XrPosef r;
    r.orientation = XrQuaternionf_Identity();
    r.position = XrVector3f_Zero();
    return r;
}

static inline XrPosef XrPosef_Inverse(const XrPosef a) {
    XrPosef b;
    b.orientation = XrQuaternionf_Inverse(a.orientation);
    b.position = XrQuaternionf_Rotate(b.orientation, XrVector3f_ScalarMultiply(a.position, -1.0f));
    return b;
}

static inline XrVector3f XrPosef_Transform(const XrPosef a, const XrVector3f v) {
    XrVector3f r0 = XrQuaternionf_Rotate(a.orientation, v);
    XrVector3f result;
    XrVector3f_Add(&result, &r0, &a.position);
    return result;
}

static inline XrPosef XrPosef_Multiply(const XrPosef a, const XrPosef b) {
    XrPosef c;
    XrQuaternionf_Multiply(&c.orientation, &b.orientation, &a.orientation);
    c.position = XrPosef_Transform(a, b.position);
    return c;
}

#endif //OPENXR_HELPERS_H_