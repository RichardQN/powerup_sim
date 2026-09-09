#pragma once

#include <cmath>

struct Vec3 {
    float x, y, z;
};

inline Vec3 operator-(Vec3 a, Vec3 b)
{
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}

inline float dot(Vec3 a, Vec3 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline Vec3 cross(Vec3 a, Vec3 b)
{
    return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x};
}

// Callers supply nonzero vectors; the camera's pitch limits keep its basis valid.
inline Vec3 normalized(Vec3 v)
{
    float length = std::sqrt(dot(v, v));
    return {v.x / length, v.y / length, v.z / length};
}

// Column-major storage matches OpenGL's glUniformMatrix4fv with transpose=false.
// Coordinates use a right-handed world with +Y up; the camera looks along -Z.
struct Mat4 {
    float values[16];
};

inline Mat4 translation(Vec3 offset)
{
    return {{1, 0, 0, 0,
             0, 1, 0, 0,
             0, 0, 1, 0,
             offset.x, offset.y, offset.z, 1}};
}

inline Mat4 lookAt(Vec3 eye, Vec3 target)
{
    Vec3 forward = normalized(target - eye);
    Vec3 right = normalized(cross(forward, {0, 1, 0}));
    Vec3 up = cross(right, forward);
    return {{right.x, up.x, -forward.x, 0,
             right.y, up.y, -forward.y, 0,
             right.z, up.z, -forward.z, 0,
             -dot(right, eye), -dot(up, eye), dot(forward, eye), 1}};
}

// Vertical field of view is in radians; aspect > 0 and 0 < nearPlane < farPlane.
// Maps camera-space depth to OpenGL's normalized depth range [-1, +1].
inline Mat4 perspective(float verticalFov, float aspect, float nearPlane, float farPlane)
{
    float scale = 1.0f / std::tan(verticalFov * 0.5f);
    return {{scale / aspect, 0, 0, 0,
             0, scale, 0, 0,
             0, 0, (farPlane + nearPlane) / (nearPlane - farPlane), -1,
             0, 0, 2 * farPlane * nearPlane / (nearPlane - farPlane), 0}};
}
