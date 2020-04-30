#ifndef PLATFORM_RENDERING_H
#define PLATFORM_RENDERING_H

#include "../Rendering/shader.h"
#include "../Rendering/texture.h"
#include "../Rendering/mesh.h"
#include "../Rendering/rendering_util.h"

#define MAX_LIGHTS 16

enum SubPaletteIndex
{
    SUB_PALETTE_TILESET0 = 0,
    SUB_PALETTE_TILESET1 = 1,
    SUB_PALETTE_TILESET2 = 2,
    SUB_PALETTE_TILESET3 = 3,
    SUB_PALETTE_TILESET4 = 4,
    SUB_PALETTE_TILESET5 = 5,
    SUB_PALETTE_TILESET6 = 6,
    SUB_PALETTE_TILESET7 = 7,
    SUB_PALETTE_SPRITE0 = 8,
    SUB_PALETTE_SPRITE1 = 9,
    SUB_PALETTE_SPRITE2 = 10,
    SUB_PALETTE_SPRITE3 = 11,
    SUB_PALETTE_SPRITE4 = 12,
    SUB_PALETTE_SPRITE5 = 13,
    //ui colors
    SUB_PALETTE_SPRITE6 = 14,
    //player colors
    SUB_PALETTE_SPRITE7 = 15
};

struct SubPalette
{
    //RGB565
    u16 colors[256];
};

struct Palette
{
    SubPalette subPalette[16];
};

struct rImage
{
    u8 *pixels;
    u32 width, height;
};

//loading
void platform_load_image(rImage *image, const char* fname);
void platform_delete_image(rImage *image);

void platform_load_palette(const char *fname, void *output, int s);

void platform_load_mesh(MeshData *mesh, const char* fname);
void platform_calculate_tangent(MeshData *mesh);

//Shader stuff
InternalShader *platform_create_shader(const char* vert, const char* frag);
void platform_get_shader_property_count(InternalShader *shader, u32 *count);
void platform_get_shader_properties(InternalShader *shader, u32 count, char **names, ShaderPropertyType *types);
void platform_delete_shader(InternalShader *shader);

void platform_use_shader(InternalShader *shader);

InternalTexture *platform_create_indexed_sprite_sheet(rImage *image);
InternalTexture *platform_create_texture(rImage *image, bool srgb = true);
void platform_delete_texture(InternalTexture* texture);
void platform_get_texture_width(InternalTexture *texture, u32 *w);
void platform_get_texture_height(InternalTexture *texture, u32 *h);
InternalTexture *platform_create_empty_texture(s32 w, s32 h);

InternalTexture *platform_create_tile_index_map(u32 w, u32 h);
void platform_populate_tile_index_map(InternalTexture *texture, u32 w, u32 h, u8 *pixels);

InternalTexture *platform_create_palette_texture(Palette *palette);
void platform_update_palette_texture(InternalTexture *texture, Palette *palette);

void platform_shader_set_texture(const char* propertyName, InternalTexture *tex, TextureType type);
void platform_shader_set_float(const char* propertyName, float f);
void platform_shader_set_vector(const char* propertyName, v2 vec);
void platform_shader_set_vector(const char* propertyName, v3 vec);
void platform_shader_set_vector(const char* propertyName, v4 vec);

void platform_set_lights(Light *lights, u32 lightCount);
void platform_set_ambient_color(v3 *color);

void platform_render(Transform xform);
void platform_blit();

void platform_clear_buffer();
void platform_swap_buffer();

InternalMesh *platform_create_mesh(MeshData *mesh);
void platform_delete_mesh(InternalMesh *buf);

void platform_use_mesh(InternalMesh *mesh);

void platform_set_projection();

void platform_enable_depth_test();
void platform_disable_depth_test();

#endif // PLATFORM_MAIN_H
