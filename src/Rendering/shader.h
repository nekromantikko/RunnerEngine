#ifndef SHADER_H
#define SHADER_H

#include "../vector.h"

#define SHADER_PROPERTY_MAX_NAME_LENGTH 32

struct InternalShader;
struct InternalTexture;

enum ShaderPropertyType
{
    SHADER_PROPERTY_UNKNOWN = 0,

    SHADER_PROPERTY_FLOAT,
    SHADER_PROPERTY_VEC2,
    SHADER_PROPERTY_VEC3,
    SHADER_PROPERTY_VEC4,
    SHADER_PROPERTY_INT,
    SHADER_PROPERTY_UINT,

    SHADER_PROPERTY_TEXTURE2D,
};

struct Shader
{
    InternalShader *internal;

    u32 propertyCount;
    char **propertyName;

    u32 get_index(const char* name);
};

#endif // SHADER_H
