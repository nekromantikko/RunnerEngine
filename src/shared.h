#ifndef SHARED_H
#define SHARED_H

#include "signal.h"
#include <vector>
#include "units.h"

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 576
#define TILE_SIZE 32
#define SCREEN_MARGINAL 128

//DEFINITIONS FOR STUFF SHARED BETWEEN THE PLATFORM LAYER AND GAME

struct rImage
{
    u8 *pixels;
    u32 width, height;
};

struct Rectangle2
{
    r32 x1, x2, y1, y2;
};

struct Triangle
{
    u32 index[3];

    Triangle() {}
    Triangle(u32 a, u32 b, u32 c)
    {
        index[0] = a;
        index[1] = b;
        index[2] = c;
    }
};

struct Transform
{
    Transform() : position({0,0}), rotation({0,0,0}), scale({1,1,1})
    {}
    void reset()
    {
        position = {0,0};
        rotation = {0,0,0};
        scale = {1,1,1};
    }
    Transform lerp(const Transform &other, r32 alpha)
    {
        Transform temp;

        temp.position.x = other.position.x * alpha + position.x * (1.0 - alpha);
        temp.position.y = other.position.y * alpha + position.y * (1.0 - alpha);

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
    void translate(fvec2 pos)
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
    fvec2 position;
    v3 rotation;
    v3 scale;
};

struct Texture;

struct Mesh
{
    std::vector<v3> positions;
    std::vector<v2> texCoords;
    std::vector<v3> normals;
    std::vector<v4> tangents;
    std::vector<Triangle> tris;
};

struct VertexArrayHandle;
struct ParticleVertexArrayHandle;

struct ParticlePosition
{
    v2 current;
    v2 previous;
};

struct ParticleRotation
{
    v3 current;
    v3 previous;
};

struct ParticleScale
{
    v3 current;
    v3 previous;
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

struct Sound;
struct Music;

struct MultiSound
{
    Sound **sounds;
    u32 soundCount;
};

struct ControllerInput
{
    Signal aButton;
    Signal bButton;
    Signal xButton;
    Signal yButton;
    Signal startButton;

    Signal xAxisLeft;
    Signal yAxisLeft;
    Signal triggerAxisRight;
};

struct KeyboardInput
{
    Signal keyLeft;
    Signal keyRight;
    Signal keyUp;
    Signal keyDown;
    Signal keySpace;
    Signal keyLeftShift;
    Signal keyEscape;
    Signal keyF1;
    Signal keyR;
    Signal keyD;
};

/*struct MappableDeviceInput
{
    Signal up, down, left, right;
    Signal jump, dash;
};*/

struct Input
{
    KeyboardInput keyboard;
    ControllerInput controller;
    bool exit;
};

#endif // SHARED_H
