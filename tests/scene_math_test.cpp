#include "scene_math.h"

#include <array>
#include <cstdlib>
#include <iostream>

std::array<float, 4> transform(Mat4 matrix, std::array<float, 4> vector)
{
    std::array<float, 4> result{};
    for (int row = 0; row < 4; ++row)
        for (int column = 0; column < 4; ++column)
            result[row] += matrix.values[column * 4 + row] * vector[column];
    return result;
}

void expectNear(float actual, float expected, const char* description)
{
    if (!std::isfinite(actual) || std::abs(actual - expected) > 0.0001f) {
        std::cerr << description << ": expected " << expected << ", got " << actual << '\n';
        std::exit(1);
    }
}

int main()
{
    auto bottom = transform(translation({0, 0.5f, 0}), {0, -0.5f, 0, 1});
    expectNear(bottom[1], 0, "Cube bottom rests on ground");

    Vec3 eye{3, 2, 5};
    Vec3 target{0, 0.5f, 0};
    Mat4 view = lookAt(eye, target);
    auto cameraOrigin = transform(view, {eye.x, eye.y, eye.z, 1});
    for (int axis = 0; axis < 3; ++axis)
        expectNear(cameraOrigin[axis], 0, "Eye maps to camera origin");
    auto centered = transform(view, {target.x, target.y, target.z, 1});
    expectNear(centered[0], 0, "Target centered horizontally");
    expectNear(centered[1], 0, "Target centered vertically");
    expectNear(centered[2], -std::sqrt(dot(target - eye, target - eye)), "Target in front of camera");

    Mat4 projection = perspective(0.785398163f, 2, 0.1f, 50);
    auto nearPoint = transform(projection, {0, 0, -0.1f, 1});
    auto farPoint = transform(projection, {0, 0, -50, 1});
    expectNear(nearPoint[2] / nearPoint[3], -1, "Near depth mapping");
    expectNear(farPoint[2] / farPoint[3], 1, "Far depth mapping");
    auto point = transform(projection, {1, 1, -3, 1});
    expectNear(point[0] * 2, point[1], "Aspect ratio preserves equal pixel lengths");
    std::cout << "Scene math checks passed\n";
}
