#include "renderer.h"
#include "Platform/platform.h"
#include <algorithm>
#include "units.h"
#include "resource.h"
#include "debug.h"
#include "level.h"

namespace Renderer
{
    //std::list<Drawcall> worldBuffer;
    std::list<UICall> uiBuffer;
    std::list<MirrorCall> mirrorBuffer;
    std::list<SpriteCall> spriteBuffer;
    std::list<ModelCall> modelBuffer;
    std::list<TileCall> tileBuffer;
    std::list<ParticleCall> particleBuffer;
    std::vector<LerpLight> lightBuffer;

    v3 lightPos[MAX_LIGHTS];
    v4 lightColor[MAX_LIGHTS];

    fvec2 camPos;
    fvec2 prevCamPos;
    VertexArrayHandle *rectangle;
}

void Renderer::init()
{
    Mesh box;

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

    rectangle = platform_create_vertex_array(&box);

    camPos = {0,0};
    prevCamPos = camPos;
}
void Renderer::deinit()
{
    platform_delete_vertex_array(rectangle);
}

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

void Renderer::draw_string(std::string str, s32 x, s32 y, DrawOrigin *originPos, v4 *color, r32 *scale, Sprite *sprite, s32 priority)
{
    DrawOrigin origin = ORIGIN_TOPLEFT;
    if (originPos)
        origin = *originPos;

    r32 scl = 2;
    if (scale)
        scl = *scale;

    if (!sprite)
        sprite = Resource::get_sprite("spr_ascii");
    u32 wSpr = sprite->width * scl;

    ivec2 topLeft = get_topleft(x, y, scl * sprite->width * str.length(), scl * sprite->height, origin);

    for (memory_index i = 0; i < str.length(); i++)
    {
        //clipping
        u8 frame = str.at(i);

        Transform xform;
        xform.position = {topLeft.x + i * wSpr, topLeft.y};
        xform.scale = {scl * sprite->width, scl * sprite->height, 1};

        UICall call;
        call.a = xform;
        call.b = xform;
        call.clipRect = sprite->get_clip(frame);

        if (color)
            call.color = *color;
        else call.color = {1,1,1,1};

        call.offset = {0,0};
        call.flip = {0,0};
        call.priority = priority;
        call.texture = sprite->texture;

        add_ui_element(call);
    }
}

void Renderer::draw_rectangle(s32 x, s32 y, s32 w, s32 h, DrawOrigin *originPos, v4 *color, s32 priority)
{
    DrawOrigin origin = ORIGIN_TOPLEFT;
    if (originPos)
        origin = *originPos;

    ivec2 topLeft = get_topleft(x, y, w, h, origin);

    Transform xform;
    xform.position = {topLeft.x, topLeft.y};
    xform.scale = {w, h, 1};

    UICall call;
    call.a = xform;
    call.b = xform;
    call.clipRect = {0,0,1,1};

    if (color)
        call.color = *color;
    else call.color = {1,1,1,1};

    call.offset = {0,0};
    call.flip = {0,0};
    call.priority = priority;
    call.texture = NULL;

    add_ui_element(call);
}

void Renderer::draw_texture(Texture *tex, s32 x, s32 y, DrawOrigin *originPos, bool32 *hFlip, bool32 *vFlip, v4 *color, s32 priority)
{
    DrawOrigin origin = ORIGIN_TOPLEFT;
    if (originPos)
        origin = *originPos;

    u32 texWidth, texHeight;
    platform_get_texture_width(tex, &texWidth);
    platform_get_texture_height(tex, &texHeight);

    ivec2 topLeft = get_topleft(x, y, texWidth, texHeight, origin);

    Transform xform;
    xform.position = {topLeft.x, topLeft.y};
    xform.scale = {texWidth, texHeight, 1};

    UICall call;
    call.a = xform;
    call.b = xform;
    call.clipRect = {0,0,1,1};

    if (color)
        call.color = *color;
    else call.color = {1,1,1,1};

    call.offset = {0,0};

    if (hFlip)
    {
        if (vFlip)
            call.flip = {*hFlip, *vFlip};
        else call.flip = {*hFlip, false};
    }
    else if (vFlip)
        call.flip = {false, *vFlip};
    else call.flip = {false, false};

    call.priority = priority;
    call.texture = tex;

    add_ui_element(call);
}

void Renderer::draw_texture(const char *name, s32 x, s32 y, DrawOrigin *originPos, bool32 *hFlip, bool32 *vFlip, v4 *color, s32 priority)
{
    Texture *texture = Resource::get_texture(name);
    if (!texture)
        texture = Resource::no_texture();
    draw_texture(texture, x, y, originPos, hFlip, vFlip, color, priority);
}

void Renderer::draw_mirror(Transform &a, Transform &b, s32 w, s32 h)
{
    MirrorCall call;
    call.a = a;
    call.a.scale = {w, h, 1};
    call.b = b;
    call.b.scale = {w, h, 1};
    call.clipA = {(r32)a.position.x/runnerScreenWidth, (r32)(runnerScreenHeight - a.position.y ) /(runnerScreenHeight + runnerScreenMarginal), (r32)w/runnerScreenWidth, (r32)h/(runnerScreenHeight + runnerScreenMarginal)};
    call.clipB = {(r32)b.position.x/runnerScreenWidth, (r32)(runnerScreenHeight - b.position.y ) /(runnerScreenHeight + runnerScreenMarginal), (r32)w/runnerScreenWidth, (r32)h/(runnerScreenHeight + runnerScreenMarginal)};

    call.priority = 0;

    add_mirror(call);
}
void Renderer::draw_sprite_instance(SpriteInstance* inst, Transform &a, Transform &b, bool32 hFlip, bool32 vFlip, v4 *color, s32 priority)
{
    //send callback function
    SpriteCall call;
    Sprite *sprite = inst->sprite;

    v4 c = {1,1,1,1};
    if (color)
        c = *color;

    call.a = a;
    call.b = b;
    call.texture = sprite->texture;
    call.lightmap = sprite->lightmap;
    call.normal = sprite->normal;
    call.clipRect = sprite->get_clip(inst->currentAnim, inst->currentFrame);
    //std::cout << call.clipRect.x << ", " << call.clipRect.y << ", " << call.clipRect.z << ", " << call.clipRect.w << std::endl;
    call.offset = {sprite->xOffset, sprite->yOffset};
    call.color = c;
    call.flip = {hFlip, vFlip};
    call.glow = sprite->glow;
    call.priority = priority;

    add_sprite(call);

}
void Renderer::draw_sprite_instance_hud(SpriteInstance* inst, s32 x, s32 y, bool32 hFlip, bool32 vFlip, v4 *color, s32 priority)
{
    //send callback function
    UICall call;
    Sprite *sprite = inst->sprite;

    if (!sprite)
        return;

    Transform xform;
    xform.position = {x,y};
    xform.scale = {sprite->width, sprite->height, 1};

    v4 c = {1,1,1,1};
    if (color)
        c = *color;

    call.a = xform;
    call.b = xform;
    call.texture = sprite->texture;
    call.clipRect = sprite->get_clip(inst->currentAnim, inst->currentFrame);
    call.offset = {sprite->xOffset, sprite->yOffset};
    call.color = c;
    call.flip = {hFlip, vFlip};
    call.priority = priority;

    add_ui_element(call);
}
/*
void Renderer::draw_world(r64 a, v4 ambient)
{
    TIMED_BLOCK;

    Drawcall call;

    call.depth = -1;
    Sprite *sprite = Resource::get_sprite("spr_particle_debug");
    call.vbuffer = sprite->vbuffer;
    call.clipBuffer = NO_CLIP;
    call.texture = sprite->texture;
    call.lightmap = sprite->lightmap;
    call.normal = sprite->normal;
    call.glow = sprite->glow;
    call.blend = true;

    call.instanceData = test.data();
    call.instanceColor = colorTest.data();
    call.instanceFrame = frameTest.data();
    call.instanceCount = test.size();

    add_world_element(call);

    u32 lightAmount = lightBuffer.size();
    if (lightAmount > MAX_LIGHTS)
        lightAmount = MAX_LIGHTS;

    std::vector<Light> lerpedLights;
    lerpedLights.reserve(lightAmount);
    for (u32 i = 0; i < lightAmount; i++)
    {
        Light &aLight = lightBuffer.at(i).a;
        Light &bLight = lightBuffer.at(i).b;

        Light temp = bLight.lerp(aLight, a);
        lerpedLights.push_back(temp);
    }

    for (Drawcall &call : worldBuffer)
    {
        if (!call.instanceData)
            continue;

        Texture *lightmap = call.lightmap;
        if (!lightmap)
            lightmap = Resource::get_texture("lightmap_none");
        Texture *normal = call.normal;
        if (!normal)
            normal = Resource::get_texture("normal_none", false);

        InstanceDataProcessed *processedData = platform_process_instance_data(call.instanceData, call.instanceCount, a, call.depth);
        platform_render_world_element(processedData, call.instanceColor, call.instanceFrame, call.instanceCount, call.texture, lightmap, normal, call.vbuffer, call.clipBuffer, call.glow, call.blend, ambient, lightAmount, lerpedLights.data());
        delete[] processedData;
    }
}
*/
void Renderer::set_up_lights(r64 a)
{

}
void Renderer::draw_tiles(r64 a, v3 *ambientColor)
{
    u32 lightCount = lightBuffer.size();

    platform_use_tile_shader();

    if (lightCount)
    {
        std::vector<Light> lerpedLights;
        lerpedLights.reserve(lightCount);
        for (u32 i = 0; i < lightCount; i++)
        {
            Light &aLight = lightBuffer.at(i).a;
            Light &bLight = lightBuffer.at(i).b;

            Light temp = bLight.lerp(aLight, a);
            lerpedLights.push_back(temp);
        }
        platform_set_lights(lerpedLights.data(), lightCount);
    }

    Texture *palette = Resource::get_texture("palette_01");

    platform_set_ambient_color(ambientColor);
    for (TileCall &call : tileBuffer)
    {
        v2 c = Lerp(call.b, a, call.a);
        v3 pos = {c.x, c.y, call.z};
        platform_render_tiles(&pos, palette, call.layout, call.texture, call.lightmap, call.normal);
    }
}

void Renderer::draw_sprites(r64 a, v3 *ambientColor)
{
    u32 lightCount = lightBuffer.size();

    platform_use_sprite_shader();

    if (lightCount)
    {
        std::vector<Light> lerpedLights;
        lerpedLights.reserve(lightCount);
        for (u32 i = 0; i < lightCount; i++)
        {
            Light &aLight = lightBuffer.at(i).a;
            Light &bLight = lightBuffer.at(i).b;

            Light temp = bLight.lerp(aLight, a);
            lerpedLights.push_back(temp);
        }
        platform_set_lights(lerpedLights.data(), lightCount);
    }

    platform_bind_vao(rectangle);
    platform_set_projection();
    platform_set_ambient_color(ambientColor);

    Texture *palette = Resource::get_texture("palette_01");

    for (SpriteCall &call : spriteBuffer)
    {
        Transform c = call.b.lerp(call.a, a);
        c.position.x = std::round((r32)c.position.x);
        c.position.y = std::round((r32)c.position.y);

        Texture *tex = call.texture;
        if (!tex)
            tex = Resource::no_texture();
        Texture *lightmap = call.lightmap;
        if (!lightmap)
            lightmap = Resource::no_lightmap();
        Texture *normal = call.normal;
        if (!normal)
            normal = Resource::no_normal();

        platform_render_sprite(c, palette, tex, normal, &call.clipRect, &call.offset, &call.flip, &call.color, call.glow);
    }
    platform_bind_vao(NULL);
}

void Renderer::draw_models(r64 a, v3 *ambientColor)
{
    u32 lightCount = lightBuffer.size();

    platform_enable_depth_test();
    platform_use_model_shader();

    if (lightCount)
    {
        std::vector<Light> lerpedLights;
        lerpedLights.reserve(lightCount);
        for (u32 i = 0; i < lightCount; i++)
        {
            Light &aLight = lightBuffer.at(i).a;
            Light &bLight = lightBuffer.at(i).b;

            Light temp = bLight.lerp(aLight, a);
            lerpedLights.push_back(temp);
        }
        platform_set_lights(lerpedLights.data(), lightCount);
    }

    platform_set_projection();
    platform_set_ambient_color(ambientColor);
    for (ModelCall &call : modelBuffer)
    {
        Transform c = call.b.lerp(call.a, a);
        c.position.x = std::round((r32)c.position.x);
        c.position.y = std::round((r32)c.position.y);

        Texture *tex = call.texture;
        if (!tex)
            tex = Resource::no_texture();
        Texture *lightmap = call.lightmap;
        if (!lightmap)
            lightmap = Resource::no_lightmap();
        Texture *normal = call.normal;
        if (!normal)
            normal = Resource::no_normal();

        platform_render_model(c, call.vao, tex, lightmap, normal, call.glow);
    }
    platform_disable_depth_test();
}

void Renderer::draw_ui(r64 a)
{
    platform_use_ui_shader();
    platform_bind_vao(rectangle);
    platform_set_projection();

    Texture *palette = Resource::get_texture("palette_01");

    for (UICall &call : uiBuffer)
    {
        Transform c = call.b.lerp(call.a, a);
        c.position.x = std::round((r32)c.position.x);
        c.position.y = std::round((r32)c.position.y);

        Texture *tex = call.texture;
        if (!tex)
            tex = Resource::no_texture();

        platform_render_hud_element(c, palette, tex, &call.clipRect, &call.offset, &call.flip, &call.color);
    }
    platform_bind_vao(NULL);
}

void Renderer::draw_particles(r64 a, v3 *ambientColor)
{
    platform_use_particle_shader(0);
    platform_set_camera_pos(camPos, prevCamPos);
    platform_set_projection();
    platform_set_ambient_color(ambientColor);
    for (ParticleCall &call : particleBuffer)
    {
        Texture *tex = call.texture;
        if (!tex)
            tex = Resource::no_texture();
        Texture *lightmap = call.lightmap;
        if (!lightmap)
            lightmap = Resource::no_lightmap();
        Texture *normal = call.normal;
        if (!normal)
            normal = Resource::no_normal();

        platform_update_particle_vertex_array(call.vao);
        platform_render_particles(call.vao, a, tex, lightmap, normal, &call.clipRect, &call.offset, &call.flip, call.glow);
    }
}

void Renderer::draw_reflections(r64 a)
{
    platform_use_mirror_shader();
    platform_bind_vao(rectangle);
    platform_set_projection();
    for (MirrorCall &call : mirrorBuffer)
    {
        Transform c = call.b.lerp(call.a, a);
        c.position.x = std::round((r32)c.position.x);
        c.position.y = std::round((r32)c.position.y);

        v4 clipInterpolated = Lerp(call.clipB, a, call.clipA);

        platform_render_mirror(c, &clipInterpolated);
    }
    platform_bind_vao(NULL);
}

void Renderer::sort_buffers()
{
    //worldBuffer.sort();
    tileBuffer.sort();
    uiBuffer.sort();
    mirrorBuffer.sort();
    spriteBuffer.sort();
    modelBuffer.sort();
    particleBuffer.sort();
}

void Renderer::clear_buffers()
{
    //worldBuffer.clear();
    tileBuffer.clear();
    uiBuffer.clear();
    mirrorBuffer.clear();
    spriteBuffer.clear();
    modelBuffer.clear();
    particleBuffer.clear();
    lightBuffer.clear();
}
/*
void Renderer::add_world_element(Drawcall call)
{
    worldBuffer.push_back(call);
}*/

void Renderer::add_ui_element(UICall &call)
{
    uiBuffer.push_back(call);
}

void Renderer::add_mirror(MirrorCall &call)
{
    mirrorBuffer.push_back(call);
}

void Renderer::add_sprite(SpriteCall &call)
{
    spriteBuffer.push_back(call);
}

void Renderer::add_model(ModelCall &call)
{
    modelBuffer.push_back(call);
}

void Renderer::add_tile_layer(TileCall &call)
{
    tileBuffer.push_back(call);
}

void Renderer::add_particles(ParticleCall &call)
{
    particleBuffer.push_back(call);
}

void Renderer::add_light(Light current, Light previous)
{
    LerpLight light;
    light.a = current;
    light.b = previous;

    lightBuffer.push_back(light);
}

void Renderer::set_camera_position(fvec2 pos)
{
    camPos.x = std::round((r32)pos.x - (runnerScreenWidth / 2));
    camPos.y = std::round((r32)pos.y - (runnerScreenHeight / 2));

    f32 a, b, c, d;
    a = -camPos.x;
    b = -camPos.y;
    c = (camPos.x + runnerScreenWidth) - (CurrentLevel::get_width() * runnerTileSize);
    d = (camPos.y + runnerScreenHeight) - (CurrentLevel::get_height() * runnerTileSize);

    if (a > 0)
        camPos.x += a;
    else if (c > 0)
        camPos.x -= c;

    if (b > 0)
        camPos.y += b;
    else if (d > 0)
        camPos.y -= d;

    //listener
    fvec2 lisp;
    lisp.x = camPos.x + (runnerScreenWidth / 2);
    lisp.y = camPos.y + (runnerScreenHeight / 2);
    platform_set_listener_attributes(&lisp, NULL);
}

fvec2 Renderer::get_camera_position()
{
    return camPos;
}

void Renderer::update()
{
    prevCamPos = camPos;
}
