#ifndef PLATFORM_AUDIO_H
#define PLATFORM_AUDIO_H

#include "../vector.h"

#define WORLD_CHANNEL_COUNT 8
#define MENU_CHANNEL_COUNT 2

struct InternalSound;
struct InternalMusic;

bool platform_load_sound(InternalSound *sound, const char* fname);
void platform_delete_sound(InternalSound *sound);
bool platform_load_music(InternalMusic *music, const char* fname);
void platform_delete_music(InternalMusic *music);

u32 platform_play_world_sound(InternalSound *sound, s32 loops, fvec2 *pos, fvec2 *vel);
u32 platform_play_menu_sound(InternalSound *sound, s32 loops);
void platform_stop_channel(u32 channel);
void platform_pause_channel(u32 channel);
void platform_resume_channel(u32 channel);
u32 platform_play_music(InternalMusic *music, bool32 mute = false, s32 fade = 0, r64 pos = 0.f);
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

#endif // PLATFORM_AUDIO_H
