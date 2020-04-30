#ifndef RENDER_UTIL_H
#define RENDER_UTIL_H

#include "../math.h"

struct Transform
{
    Transform() : position({0,0,0}), rotation({0,0,0}), scale({1,1,1})
    {}
    void reset()
    {
        position = {0,0,0};
        rotation = {0,0,0};
        scale = {1,1,1};
    }
    Transform lerp(const Transform &other, r32 alpha)
    {
        Transform temp;

        temp.position.x = other.position.x * alpha + position.x * (1.0 - alpha);
        temp.position.y = other.position.y * alpha + position.y * (1.0 - alpha);
        temp.position.z = other.position.z * alpha + position.z * (1.0 - alpha);

        temp.rotation.x = other.rotation.x * alpha + rotation.x * (1.0 - alpha);
        temp.rotation.y = other.rotation.y * alpha + rotation.y * (1.0 - alpha);
        temp.rotation.z = other.rotation.z * alpha + rotation.z * (1.0 - alpha);

        temp.scale.x = other.scale.x * alpha + scale.x * (1.0 - alpha);
        temp.scale.y = other.scale.y * alpha + scale.y * (1.0 - alpha);
        temp.scale.z = other.scale.z * alpha + scale.z * (1.0 - alpha);

        return temp;
    }
    Transform operator+(const Transform& other)
    {
        Transform result;
        result.position = position + other.position;
        result.rotation = rotation + other.rotation;
        result.scale = Hadamard(scale, other.scale);
        return result;
    }
    Transform operator-(const Transform& other)
    {
        Transform result;
        result.position = position - other.position;
        result.rotation = rotation - other.rotation;
        result.scale.x = scale.x / other.scale.x;
        result.scale.y = scale.y / other.scale.y;
        result.scale.z = scale.z / other.scale.z;
        return result;
    }
    Transform &operator+=(const Transform &other)
    {
        position += other.position;
        rotation += other.rotation;
        scale = Hadamard(scale, other.scale);
        return *this;
    }
    Transform &operator-=(const Transform &other)
    {
        position -= other.position;
        rotation -= other.rotation;
        scale.x /= other.scale.x;
        scale.y /= other.scale.y;
        scale.z /= other.scale.z;
        return *this;
    }
    void translate(fvec3 pos)
    {
        position += pos;
    }
    void rotate(v3 rot)
    {
        rotation += rot;
    }
    void do_scale(v3 scl)
    {
        scale = Hadamard(scale, scl);
    }
///////////////////////////
    fvec3 position;
    v3 rotation;
    v3 scale;
};

struct Light
{
    v4 position;
    v4 color;
    Light lerp(const Light &other, r32 alpha)
    {
        Light result;
        result.position.x = other.position.x * alpha + position.x * (1.0 - alpha);
        result.position.y = other.position.y * alpha + position.y * (1.0 - alpha);
        result.position.z = other.position.z * alpha + position.z * (1.0 - alpha);

        result.color.x = other.color.x * alpha + color.x * (1.0 - alpha);
        result.color.y = other.color.y * alpha + color.y * (1.0 - alpha);
        result.color.z = other.color.z * alpha + color.z * (1.0 - alpha);
        result.color.w = other.color.w * alpha + color.w * (1.0 - alpha);

        return result;
    }
};

#endif // RENDER_UTIL_H
