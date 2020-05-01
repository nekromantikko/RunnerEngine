#include "resource.h"
#include <stdexcept>
#include <sstream>
#include "Platform/platform.h"
#include "file_formats.h"
#include <fstream>
#include <atomic>

#define MAX_TEXTURE_COUNT 256
#define MAX_SHADER_COUNT 256
#define MAX_MESH_COUNT 256


namespace Resource
{
    char* textureName[MAX_TEXTURE_COUNT];
    InternalTexture internalTexture[MAX_TEXTURE_COUNT];
    char* shaderName[MAX_TEXTURE_COUNT];
    InternalShader internalShader[MAX_SHADER_COUNT];
    char* meshName[MAX_TEXTURE_COUNT];
    InternalMesh internalMesh[MAX_MESH_COUNT];
}

void Resource::load_assets()
{
    //load assets here
}

void Resource::free_assets()
{

}

