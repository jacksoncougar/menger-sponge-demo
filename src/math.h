#ifndef MATH_H
#define MATH_H

#include <vector>
#include <iostream>
#include <random>
#include <gl/GLU.h>

#define M_PI 3.141592653589793238462643

constexpr long double operator"" _deg(long double value)
{
    return value * M_PI / 180.;
}

constexpr double rads(float degree)
{
    return degree * M_PI / 180.;
}

struct Vector4f
{
    float x, y, z, w;

public:
    Vector4f(const float x, const float y, const float z, const float w)
        : x(x), y(y), z(z), w(w) {}

    float operator[](int index)
    {
        return ((float *)(&x))[index];
    }
};

struct mat4x4
{
    float m00, m10, m20, m30,
        m01, m11, m21, m31,
        m02, m12, m22, m32,
        m03, m13, m23, m33;

    mat4x4(float m00, float m01, float m02, float m03,
           float m10, float m11, float m12, float m13,
           float m20, float m21, float m22, float m23,
           float m30, float m31, float m32, float m33)
        : m00(m00), m01(m01), m02(m02), m03(m03),
          m10(m10), m11(m11), m12(m12), m13(m13),
          m20(m20), m21(m21), m22(m22), m23(m23),
          m30(m30), m31(m31), m32(m32), m33(m33) {}

    mat4x4 operator*(const mat4x4 right)
    {
        mat4x4 result(*this);
        result.multiply(right);
        return result;
    }

    void multiply(Vector4f &vector)
    {
        vector.x = m00 * vector.x + m01 * vector.y + m02 * vector.z + m03 * vector.w;
        vector.x = m10 * vector.x + m11 * vector.y + m12 * vector.z + m13 * vector.w;
        vector.x = m20 * vector.x + m21 * vector.y + m22 * vector.z + m23 * vector.w;
        vector.x = m30 * vector.x + m31 * vector.y + m32 * vector.z + m33 * vector.w;
    }

    void multiply(const mat4x4 &o)
    {
        float t0, t1, t2, t3;
        t0 = m00 * o.m00 + m01 * o.m10 + m02 * o.m20 + m03 * o.m30;
        t1 = m00 * o.m01 + m01 * o.m11 + m02 * o.m21 + m03 * o.m31;
        t2 = m00 * o.m02 + m01 * o.m12 + m02 * o.m22 + m03 * o.m32;
        t3 = m00 * o.m03 + m01 * o.m13 + m02 * o.m23 + m03 * o.m33;
        m00 = t0;
        m01 = t1;
        m02 = t2;
        m03 = t3;

        t0 = m10 * o.m00 + m11 * o.m10 + m12 * o.m20 + m13 * o.m30;
        t1 = m10 * o.m01 + m11 * o.m11 + m12 * o.m21 + m13 * o.m31;
        t2 = m10 * o.m02 + m11 * o.m12 + m12 * o.m22 + m13 * o.m32;
        t3 = m10 * o.m03 + m11 * o.m13 + m12 * o.m23 + m13 * o.m33;
        m10 = t0;
        m11 = t1;
        m12 = t2;
        m13 = t3;

        t0 = m20 * o.m00 + m21 * o.m10 + m22 * o.m20 + m23 * o.m30;
        t1 = m20 * o.m01 + m21 * o.m11 + m22 * o.m21 + m23 * o.m31;
        t2 = m20 * o.m02 + m21 * o.m12 + m22 * o.m22 + m23 * o.m32;
        t3 = m20 * o.m03 + m21 * o.m13 + m22 * o.m23 + m23 * o.m33;
        m20 = t0;
        m21 = t1;
        m22 = t2;
        m23 = t3;

        t0 = m30 * o.m00 + m31 * o.m10 + m32 * o.m20 + m33 * o.m30;
        t1 = m30 * o.m01 + m31 * o.m11 + m32 * o.m21 + m33 * o.m31;
        t2 = m30 * o.m02 + m31 * o.m12 + m32 * o.m22 + m33 * o.m32;
        t3 = m30 * o.m03 + m31 * o.m13 + m32 * o.m23 + m33 * o.m33;
        m30 = t0;
        m31 = t1;
        m32 = t2;
        m33 = t3;
    }
};

struct Vertex
{
    Vector4f position;
    Vector4f color = {1, 0, 0, 1};

    Vertex(const float x, const float y, const float z, const float w,
           const Vector4f color) : position(x, y, z, w), color(color) {}
};

Vector4f hexToColor(unsigned int color)
{
    return Vector4f(
        ((color >> 24) & 0xFF) / 255.,
        ((color >> 16) & 0xFF) / 255.,
        ((color >> 8) & 0xFF) / 255.,
        (color & 0xFF) / 255.);
}

struct Cube
{
    GLuint vertexbufferid;

    std::vector<Vertex> vertices;

    Cube()
    {
        auto color0 = hexToColor(0xDE9151FF);
        auto color1 = hexToColor(0xF34213FF);
        auto color2 = hexToColor(0x2E2E3AFF);
        auto color3 = hexToColor(0xBC5D2EFF);
        auto color4 = hexToColor(0xBBB8B2FF);
        auto color5 = hexToColor(0x7CE577FF);

        vertices = {
            {-0.5, -0.5, 0.5, 1, color0},
            {-0.5, 0.5, 0.5, 1, color0},
            {-0.5, 0.5, -0.5, 1, color0},
            {-0.5, -0.5, -0.5, 1, color0},
            {-0.5, -0.5, -0.5, 1, color1},
            {-0.5, 0.5, -0.5, 1, color1},
            {0.5, 0.5, -0.5, 1, color1},
            {0.5, -0.5, -0.5, 1, color1},
            {0.5, -0.5, -0.5, 1, color2},
            {0.5, 0.5, -0.5, 1, color2},
            {0.5, 0.5, 0.5, 1, color2},
            {0.5, -0.5, 0.5, 1, color2},
            {0.5, -0.5, 0.5, 1, color3},
            {0.5, 0.5, 0.5, 1, color3},
            {-0.5, 0.5, 0.5, 1, color3},
            {-0.5, -0.5, 0.5, 1, color3},
            {-0.5, -0.5, -0.5, 1, color4},
            {0.5, -0.5, -0.5, 1, color4},
            {0.5, -0.5, 0.5, 1, color4},
            {-0.5, -0.5, 0.5, 1, color4},
            {0.5, 0.5, -0.5, 1, color5},
            {-0.5, 0.5, -0.5, 1, color5},
            {-0.5, 0.5, 0.5, 1, color5},
            {0.5, 0.5, 0.5, 1, color5},
        };
    }
};

#endif