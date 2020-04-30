#ifndef SHADER_H
#define SHADER_H

#include "../vector.h"

#define SHADER_PROPERTY_MAX_NAME_LENGTH 32

struct InternalShader;
struct InternalTexture;

enum ShaderPropertyType
{
    SHADER_PROPERTY_UNKNOWN = 0,
    SHADER_PROPERTY_FLOAT = 1,
    SHADER_PROPERTY_VEC2 = 2,
    SHADER_PROPERTY_VEC3 = 3,
    SHADER_PROPERTY_VEC4 = 4,
    SHADER_PROPERTY_INT = 9,
    SHADER_PROPERTY_UINT = 13,
    SHADER_PROPERTY_TEXTURE2D = 40,
};

struct ShaderPropertyValue
{
    union
    {
        r32 floatValue;
        v2 vec2Value;
        v3 vec3Value;
        v4 vec4Value;
        s32 intValue;
        u32 uintValue;
        InternalTexture *texture2DValue;
    };
};

struct Shader
{
    InternalShader *internal;

    u32 propertyCount;
    char **propertyName;
    ShaderPropertyType *propertyType;
};

struct ShaderInstance
{
    Shader *shader;
    ShaderPropertyValue *propertyValue;
};

#endif // SHADER_H
