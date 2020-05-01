#include "renderer.h"
#include <algorithm>
#include "units.h"
#include "resource.h"
#include "debug.h"

#define RENDER_QUEUE_SIZE 1024
#define SHADER_PROPERTY_METADATA_SIZE 8192 // actual size ~ 130KB
#define SHADER_PROPERTY_DATA_SIZE 16385 // 16KB for data

#define PERSISTENT_MATRIX_BLOCK_SIZE 4096

//metadata for a shader property
struct ShaderPropertyMeta
{
    u32 index;
    ShaderPropertyType type;
    u32 arraySize;
    u8 *data;
};

namespace Renderer
{
    //render queues
    DrawCall renderQueue[RENDER_QUEUE_SIZE];
    u32 nextFreeIndex;


    //persistens block
    //
    glm::mat4 frontPersistentMatrix[PERSISTENT_MATRIX_BLOCK_SIZE];
    glm::mat4 backPersistentMatrix[PERSISTENT_MATRIX_BLOCK_SIZE];

    ///////////////////////////

    ShaderPropertyMeta propertyMeta[SHADER_PROPERTY_METADATA_SIZE];
    u32 nextFreePropertyIndex;

    u8 propertyData[SHADER_PROPERTY_DATA_SIZE];
    u32 nextFreeDataIndex;

    ///////////////////////////

    InternalMesh *rectangle;

    Palette palette;
    InternalTexture *paletteTexture;
}

void Renderer::init()
{
    MeshData box;

    box.positions.emplace_back(v3{0, 0, 0});
    box.positions.emplace_back(v3{0, 1, 0});
    box.positions.emplace_back(v3{1, 0, 0});
    box.positions.emplace_back(v3{1, 1, 0});

    box.texCoords.emplace_back(v2{0, 0});
    box.texCoords.emplace_back(v2{0, 1});
    box.texCoords.emplace_back(v2{1, 0});
    box.texCoords.emplace_back(v2{1, 1});

    box.normals.emplace_back(v3{0, 0, 1});
    box.normals.emplace_back(v3{0, 0, 1});
    box.normals.emplace_back(v3{0, 0, 1});
    box.normals.emplace_back(v3{0, 0, 1});

    box.tangents.emplace_back(v4{1, 0, 0, 1});
    box.tangents.emplace_back(v4{1, 0, 0, 1});
    box.tangents.emplace_back(v4{1, 0, 0, 1});
    box.tangents.emplace_back(v4{1, 0, 0, 1});

    //indices
    box.tris.emplace_back(0, 1, 2);
    box.tris.emplace_back(1, 3, 2);
    box.tris.emplace_back(2, 1, 0);
    box.tris.emplace_back(2, 3, 1);

    platform_create_mesh(rectangle, &box);

    camera[0] = {0,0};
    camera[1] = {0,0};

    frontIndex = 0;

    nextPropertyIndex = 0;

    platform_load_palette("res/palettes/palette01.pal", &palette, 256);
    platform_create_palette_texture(paletteTexture, &palette);
}
void Renderer::deinit()
{
    platform_delete_mesh(rectangle);
    platform_delete_texture(paletteTexture);
}

void Renderer::swap_queues()
{
    frontIndex = 1 - frontIndex;
}

void Renderer::sort_queue()
{
    //TODO!
}

void Renderer::clear_queue()
{
    renderQueue[frontIndex].count = 0;
    nextPropertyIndex = 0;
}

////////////////////////////////////////////////////

ivec2 Renderer::get_topleft(s32 x, s32 y, s32 w, s32 h, DrawOrigin origin)
{
    ivec2 newPos = {x, y};
    switch (origin)
    {
    case ORIGIN_TOPLEFT:
        break;
    case ORIGIN_TOPRIGHT:
        {
            newPos.x -= w;
            break;
        }
    case ORIGIN_BOTTOMLEFT:
        {
            newPos.y -= h;
            break;
        }
    case ORIGIN_BOTTOMRIGHT:
        {
            newPos.x -= w;
            newPos.y -= h;
            break;
        }
    case ORIGIN_CENTER:
        {
            newPos.x -= (w / 2);
            newPos.y -= (h / 2);
            break;
        }
    default:
        break;
    }

    return newPos;
}


void add_to_render_queue(u32 instanceCount, u32 *id, u8 *shader, u8 *tex0, u8 *tex1, u8 *tex2, u8 *tex3, u8 *mesh, glm::mat4 *matrix, TemporaryShaderPropertyBlock *properties)
{

}

void Renderer::render(r64 a)
{
    u32 backIndex = 1 - frontIndex;

    //sTODO: set up lights here

    //TODO: sort render queue before rendering

    fvec2 lerpedCamera = lerp(camera[frontIndex], camera[backIndex], a);

    for (int i = 0; i < renderQueue[frontIndex].count; i++)
    {
        DrawCall current = renderQueue[backIndex].data[i];
        Transform lerpedTransform;

        if (current.previousIndex >= 0)
        {
            DrawCall previous = renderQueue[backIndex].data[current.previousIndex];

            lerpedTransform = current.transform.lerp(previous.transform, a);
        }
        else lerpedTransform = current.transform;


        platform_use_shader(shader->internal);

        int textureIndex = 0;

        for (int p = 0; p < current.propertyCount; p++)
        {
            ShaderProperty property = current.property[p];

            switch (property.type)
            {
            case SHADER_PROPERTY_FLOAT:
                platform_shader_set_float(propertyName, property.floatValue);
            case SHADER_PROPERTY_VEC2:
                platform_shader_set_vectorpropertyName, property.vec2Value);
            case SHADER_PROPERTY_VEC3:
                platform_shader_set_vector(propertyName, property.vec3Value);
            case SHADER_PROPERTY_VEC4:
                platform_shader_set_vector(propertyName, property.vec4Value);
            case SHADER_PROPERTY_TEXTURE2D:
                platform_shader_set_texture(propertyName, property.texture2DValue, textureIndex++);
            case SHADER_PROPERTY_INT:
            case SHADER_PROPERTY_UINT:
            case SHADER_PROPERTY_UNKNOWN:
            case default:
                break;

            }
        }

        platform_use_mesh(current.mesh->internal);

        fvec2 cam = {0,0};
        if (current.type == DRAWCALL_WORLDSPACE)
        {
            cam = lerpedCamera;
        }

        platform_render(lerpedTransform, cam);

    }

    swap_queues();
    clear_queue();
}
