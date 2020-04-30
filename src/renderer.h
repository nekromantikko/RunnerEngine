#ifndef RENDERER_H
#define RENDERER_H
#include "Rendering/mesh.h"
#include "Rendering/shader.h"
#include "Rendering/rendering_util.h"
#include <functional>

class Sprite;

enum DrawOrigin
{
    ORIGIN_TOPLEFT,
    ORIGIN_TOPRIGHT,
    ORIGIN_BOTTOMLEFT,
    ORIGIN_BOTTOMRIGHT,
    ORIGIN_CENTER
};

struct DrawCall
{
    u64 renderID;
    Mesh *mesh;
    ShaderInstance *shader;
    Transform transform;
};

struct DrawCallInstanced
{

};

#define MAX_RENDER_QUEUE_SIZE 1024

struct RenderQueue
{
    DrawCall data[MAX_RENDER_QUEUE_SIZE];
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
    void draw_texture(InternalTexture *tex, s32 x, s32 y, DrawOrigin *originPos, bool32 *hFlip, bool32 *vFlip, v4 *color, s32 priority = 0);
    void draw_texture(const char *name, s32 x, s32 y, DrawOrigin *originPos, bool32 *hFlip, bool32 *vFlip, v4 *color, s32 priority = 0);
    void draw_mirror(Transform &a, Transform &b, s32 w, s32 h);
    void draw_sprite_instance(SpriteInstance* inst, Transform &a, Transform &b, bool32 hFlip =0, bool32 vFlip = 0, v4 *color = NULL, s32 priority = 0);
    void draw_sprite_instance_hud(SpriteInstance* inst, s32 x, s32 y, bool32 hFlip = 0, bool32 vFlip = 0, v4 *color = NULL, s32 priority = 0);
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

    void update();

}

#endif // RENDERER_H
