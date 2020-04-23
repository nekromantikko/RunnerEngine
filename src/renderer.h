#ifndef RENDERER_H
#define RENDERER_H
#include "shared.h"
#include <functional>

class Sprite;

enum DrawLayerType
{
    DRAWLAYER_TILES = 0,
    DRAWLAYER_SPRITES = 1
};

enum DrawOrigin
{
    ORIGIN_TOPLEFT,
    ORIGIN_TOPRIGHT,
    ORIGIN_BOTTOMLEFT,
    ORIGIN_BOTTOMRIGHT,
    ORIGIN_CENTER
};

struct LerpLight
{
    Light a, b;
};

class TransformComponent;

struct UICall
{
    Transform a;
    Transform b;
    Texture *texture;
    v4 clipRect;
    v2 offset;
    v4 color;
    v2 flip;
    s32 priority;

    bool operator< (const UICall &b)
    {
        return (priority < b.priority);
    }
};

struct MirrorCall
{
    Transform a;
    Transform b;
    v4 clipA;
    v4 clipB;

    s32 priority;

    bool operator< (const MirrorCall &b)
    {
        return (priority < b.priority);
    }
};

struct SpriteCall : UICall
{
    Texture *lightmap;
    Texture *normal;
    r32 glow;
};

struct ModelCall
{
    Transform a;
    Transform b;
    VertexArrayHandle *vao;
    Texture *texture;
    Texture *lightmap;
    Texture *normal;
    r32 glow;

    bool operator< (const ModelCall &b)
    {
        return ((intptr_t)vao < (intptr_t)b.vao);
    }
};

struct TileCall
{
    v2 a;
    v2 b;
    Texture *texture;
    Texture *lightmap;
    Texture *normal;
    Texture *layout;

    s32 z;

    bool operator< (const TileCall &b)
    {
        return (z < b.z);
    }
};

struct ParticleCall
{
    ParticleVertexArrayHandle *vao;

    Texture *texture;
    Texture *lightmap;
    Texture *normal;

    v4 clipRect;
    v2 offset;
    v2 flip;
    r32 glow;

    s32 priority;

    bool operator< (const ParticleCall &b)
    {
        return (priority < b.priority);
    }
};

//////////////////////////////////////////////

enum ShaderPropertyType
{
    SAMPLER,
    FLOAT1,
    FLOAT2,
    FLOAT3,
    FLOAT4
};

struct ShaderProperty
{
    ShaderPropertyType type;
    void *payload;
};

#define MAX_SHADER_PROPERTIES

struct RenderData
{
    Transform xform;
    ShaderProperty properties[MAX_SHADER_PROPERTIES];
};

#define MAX_RENDER_QUEUE_SIZE 1024

struct RenderQueue
{
    RenderData data[MAX_RENDER_QUEUE_SIZE];
};

struct SpriteInstance;

namespace Renderer
{
    void init();
    void deinit();

    void swap_queues();

    ivec2 get_topleft(s32 x, s32 y, s32 w, s32 h, DrawOrigin origin);
    void draw_string(std::string str, s32 x, s32 y, DrawOrigin *originPos, v4 *color, r32 *scale, Sprite *sprite, s32 priority = 0);
    void draw_rectangle(s32 x, s32 y, s32 w, s32 h, DrawOrigin *originPos, v4 *color, s32 priority = 0);
    void draw_texture(Texture *tex, s32 x, s32 y, DrawOrigin *originPos, bool32 *hFlip, bool32 *vFlip, v4 *color, s32 priority = 0);
    void draw_texture(const char *name, s32 x, s32 y, DrawOrigin *originPos, bool32 *hFlip, bool32 *vFlip, v4 *color, s32 priority = 0);
    void draw_mirror(Transform &a, Transform &b, s32 w, s32 h);
    void draw_sprite_instance(SpriteInstance* inst, Transform &a, Transform &b, bool32 hFlip =0, bool32 vFlip = 0, v4 *color = NULL, s32 priority = 0);
    void draw_sprite_instance_hud(SpriteInstance* inst, s32 x, s32 y, bool32 hFlip = 0, bool32 vFlip = 0, v4 *color = NULL, s32 priority = 0);
    //void draw_world(r64 a, v4 ambient);
    ////////////////////////////////////////////////
    void set_up_lights(r64 a);
    void draw_tiles(r64 a, v3 *ambientColor);
    void draw_sprites(r64 a, v3 *ambientColor);
    void draw_models(r64 a, v3 *ambientColor);
    void draw_ui(r64 a);
    void draw_particles(r64 a, v3 *ambientColor);
    void draw_reflections(r64 a);
    /////////////////////////////////////////////////////
    void sort_buffers();
    void clear_buffers();
    //void add_world_element(Drawcall call);
    void add_ui_element(UICall &call);
    void add_mirror(MirrorCall &call);
    void add_sprite(SpriteCall &call);
    void add_model(ModelCall &call);
    void add_tile_layer(TileCall &call);
    void add_particles(ParticleCall &call);
    void add_light(Light current, Light previous);
    void set_camera_position(fvec2 pos);
    fvec2 get_camera_position();
    void update();
    //void get_sprite_mesh(Sprite *sprite, u32 frame, bool hFlip, bool vFlip, Mesh &mesh);
    //void get_texture_portion_mesh(Texture *tex, Rectangle2 src, Rectangle2 out, Mesh &mesh);
    //void get_light_mesh(r32 radius, Mesh &mesh);
    //void add_rect_to_buffer(v2 a, v2 b, v2 c, v2 d, v4 color, Mesh *mesh, Transform transform);
}

#endif // RENDERER_H
