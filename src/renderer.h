#ifndef RENDERER_H
#define RENDERER_H
#include "Rendering/mesh.h"
#include "Rendering/shader.h"
#include <functional>

class SpriteSheet;

enum DrawOrigin
{
    ORIGIN_TOPLEFT,
    ORIGIN_TOPRIGHT,
    ORIGIN_BOTTOMLEFT,
    ORIGIN_BOTTOMRIGHT,
    ORIGIN_CENTER
};

enum DrawCallType
{
    DRAWCALL_SINGLE,
    DRAWCALL_INSTANCED
};

struct ShaderPropertyMeta;

struct DrawCall
{
    //ID for sorting. Consists of 8 byte indices to textures, shader and mesh, in the following layout:
    //SHADER TEX0 TEX1 TEX2 TEX3 TEX4 TEX5 MESH
    u64 id;
    DrawCallData *data;

    u8 shader_index()
    {
        return id >> 56;
    }
    u8 tex0_index()
    {
        return (id >> 48) % 0x100;
    }
    u8 tex1_index()
    {
        return (id >> 40) % 0x100;
    }
    u8 tex2_index()
    {
        return (id >> 32) % 0x100;
    }
    u8 tex3_index()
    {
        return (id >> 24) % 0x100;
    }
    u8 tex4_index()
    {
        return (id >> 16) % 0x100;
    }
    u8 tex5_index()
    {
        return (id >> 8) % 0x100;
    }
    u8 mesh_index()
    {
        return id % 0x100;
    }
};

struct DrawCallData
{
    u32 instanceCount;
    glm::mat4 *matrix;

    u32 propertyCount;
    ShaderPropertyMeta *property;

    s32 renderID;
};

//temporary struct for sending data to renderer
//pointers don't need to persist because the data will be copied into buffers in the renderer!
struct TemporaryShaderPropertyBlock
{
    u32 propertyCount;
    u32 *index; // use Shader.get_index to get this
    ShaderPropertyType *type; //how the renderer should interpret the data
    u32 *arraySize;
    void **data;
};

struct SpriteInstance;

namespace Renderer
{
    void init();
    void deinit();

    void swap_queues();
    void sort_queue();
    void clear_queue();

    s32 reserve_persistent_block(u32 instanceCount);
    void free_persistent_block(s32 renderID);

    void add_to_render_queue(u32 instanceCount, u32 *index, u8 *shader, u8 *tex0, u8 *tex1, u8 *tex2, u8 *tex3, u8 *mesh, glm::mat4 *matrix, TemporaryShaderPropertyBlock *properties);

    void render();
}

#endif // RENDERER_H
