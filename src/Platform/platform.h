#ifndef PLATFORM_H
#define PLATFORM_H

#include "../shared.h"

#define MAX_LIGHTS 16

void *platform_get_pp_texture();

//rendering
void platform_clear_buffer();
void platform_swap_buffer();
void platform_render_background(v3 *colors);
void platform_render_circle_transition(v2 pos, r32 radius);
/*
void platform_render_world_element(InstanceDataProcessed *instanceData,
                                   v4 *color,
                                   u32 *frame,
                                   u32 instanceCount,
                            Texture *diffuse,
                            Texture *lightmap,
                            Texture *normal,
                            VertexArrayHandle *vbuffer,
                            ClipBufferHandle *clipBuffer,
                            r32 glow,
                            bool32 blend,
                            v4 ambient,
                            u32 lightAmount,
                            Light *lights);
void platform_render_world(InstanceDataProcessed *instanceData,
                           v4 *color,
                            u32 *frame,
                           u32 instanceCount,
                           Texture *texture,
                            Texture *lightmap,
                            VertexArrayHandle *vbuffer,
                            ClipBufferHandle *clipBuffer,
                            r32 glow,
                            v4 ambient);
void platform_render_world_light(InstanceDataProcessed *instanceData,
                                 v4 *color,
                                   u32 *frame,
                                 u32 instanceCount,
                                 Texture *texture,
                                    Texture *lightmap,
                                    Texture *normal,
                                    VertexArrayHandle *vbuffer,
                                    ClipBufferHandle *clipBuffer,
                                    r32 glow,
                                    v4 ambient,
                                    u32 lightAmount,
                                    Light *lights);
void platform_render_textured(VertexArrayHandle *vao, v4 *color,
                              v4 *color,
                                   u32 *frame,
                              u32 instanceCount,
                              Texture *texture,
                              VertexArrayHandle *vbuffer);
void platform_render_shape(InstanceDataProcessed *instanceData,
                           v4 *color,
                           u32 instanceCount,
                           VertexArrayHandle *vbuffer,
                           bool32 fill = false);*/

void platform_bind_vao(VertexArrayHandle *vao);

void platform_use_ui_shader();
void platform_set_projection();
void platform_render_hud_element(Transform xform, Texture *texture, v4 *clipRect, v2 *offset, v2 *flip, v4 *color);

void platform_use_mirror_shader();
void platform_render_mirror(Transform xform, v4 *clipRect);
void platform_render_reflections();

void platform_use_sprite_shader();
void platform_set_lights(Light *lights, u32 lightCount);
void platform_set_ambient_color(v3 *color);
void platform_render_sprite(Transform xform, Texture *texture, Texture *lightmap, Texture *normal, v4 *clipRect, v2 *offset, v2 *flip, v4 *color, r32 glow);

void platform_enable_depth_test();
void platform_disable_depth_test();
void platform_use_model_shader();
void platform_render_model(Transform xform, VertexArrayHandle *vao, Texture *texture, Texture *lightmap, Texture *normal, r32 glow);

void platform_use_tile_shader();
void platform_render_tiles(v3 *position, Texture *layout, Texture *texture, Texture *lightmap, Texture *normal);

void platform_set_camera_pos(v2 current, v2 previous);
void platform_use_particle_shader(u32 lightCount);
void platform_render_particles(ParticleVertexArrayHandle *vao, float a, Texture *texture, Texture *lightmap, Texture *normal, v4 *clipRect, v2 *offset, v2 *flip, r32 glow);

void platform_bind_framebuffer_reflect();
void platform_bind_framebuffer_post_process();
void platform_unbind_framebuffer();
void platform_render_bloom();
void platform_render_final_image();
void platform_take_screenshot();
Texture *platform_get_screenshot();
void platform_set_fullscreen(bool);


//audio
#define WORLD_CHANNEL_COUNT 8
#define MENU_CHANNEL_COUNT 2

u32 platform_play_world_sound(Sound *sound, s32 loops, fvec2 *pos, fvec2 *vel);
u32 platform_play_menu_sound(Sound *sound, s32 loops);
void platform_stop_channel(u32 channel);
void platform_pause_channel(u32 channel);
void platform_resume_channel(u32 channel);
u32 platform_play_music(Music *music, bool32 mute = false, s32 fade = 0, r64 pos = 0.f);
void platform_stop_music(s32 fade = 0);
void platform_set_sfx_volume(r32 volume);
void platform_set_music_volume(r32 volume);
bool platform_music_is_playing();

//FMOD
void platform_update_audio();
void platform_set_listener_attributes(fvec2 *pos, fvec2 *vel);
void platform_pause_world_sounds();
void platform_resume_world_sounds();
void platform_stop_world_sounds();
void platform_stop_menu_sounds();
void platform_mute_channel(s32 channel, bool32 mute);
void platform_fade_channel(s32 channel, r32 startVol, r32 endVol, r32 length);

//input
void platform_init_controller();
void platform_deinit_controller();
bool platform_controller_exists();

void platform_poll_input(Input *input);

//haptic
void platform_controller_rumble(r32 strength, u32 lengthInMs);

//resource loading
Texture *platform_load_texture(const char* fname, bool srgb = true);
void platform_load_image(const char* fname, rImage *image);
void platform_delete_texture(Texture* texture);
void platform_get_texture_width(Texture *texture, u32 *w);
void platform_get_texture_height(Texture *texture, u32 *h);
Texture *platform_create_empty_texture(s32 w, s32 h);

Texture *platform_create_tile_layer_texture(v3 *data, u32 w, u32 h);
void platform_update_tile_layer_texture(Texture *texture, v3 *data, u32 w, u32 h);

Mesh platform_load_mesh(const char* fname);
void platform_calculate_tangent(Mesh *mesh);

VertexArrayHandle *platform_create_vertex_array(Mesh *mesh);
void platform_delete_vertex_array(VertexArrayHandle *buf);

ParticleVertexArrayHandle *platform_create_particle_vertex_array(u32 pcount,
                                           ParticlePosition *pPosition,
                                           ParticleRotation *pRotation,
                                           ParticleScale *pScale,
                                           v4 *pColor,
                                           float *pStatus);
void platform_update_particle_vertex_array(ParticleVertexArrayHandle *buf);
void platform_delete_particle_vertex_array(ParticleVertexArrayHandle *buf);

//void platform_delete_texture(Texture* texture);

Sound *platform_load_sound(const char* fname);
void platform_delete_sound(Sound *sound);
Music *platform_load_music(const char* fname);
void platform_delete_music(Music *music);

void platform_init();
void platform_quit();
void platform_show_error(const char* error);

//time
r64 platform_time_in_ms();
u64 platform_get_ticks();
u64 platform_ms_to_ticks(r64 t);
r64 platform_ticks_to_ms(u64 t);
void platform_disable_vsync();
void platform_enable_vsync();

#endif // COLLISION_WALL_H
