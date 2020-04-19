#ifndef SOUND_H
#define SOUND_H
#include "shared.h"


/*enum SoundStatus
{
    SOUND_STOPPED = -3,
    SOUND_IN_QUEUE = -2,
    SOUND_PAUSED = -1,
    SOUND_CHANNEL0 = 0,
    SOUND_CHANNEL1 = 1,
    SOUND_CHANNEL2 = 2,
    SOUND_CHANNEL3 = 3,
    SOUND_CHANNEL4 = 4,
    SOUND_CHANNEL5 = 5,
    SOUND_CHANNEL6 = 6,
    SOUND_CHANNEL7 = 7,
    SOUND_CHANNEL8 = 8,
    SOUND_CHANNEL9 = 9,
    SOUND_CHANNEL10 = 10,
    SOUND_CHANNEL11 = 11,
    SOUND_CHANNEL12 = 12,
    SOUND_CHANNEL13 = 13,
    SOUND_CHANNEL14 = 14,
    SOUND_CHANNEL15 = 15,
};

struct SoundInfo
{
    Sound *sound;
    s32 loops;
    //channel is -1 if the sound is not playing
    SoundStatus *status;
    fvec2 *position;
    r32 volume;
};*/

namespace SoundManager
{
    /*void init();
    void add_world_sound(Sound *sound, r32 volume = 1, s32 loops = 0, fvec2 *position = NULL, SoundStatus *status = NULL);
    void add_world_sound(const char *name, r32 volume = 1, s32 loops = 0, fvec2 *position = NULL, SoundStatus *status = NULL);
    void add_world_multisound(const char *name, r32 volume = 1, s32 loops = 0, fvec2 *position = NULL, SoundStatus *status = NULL);
    r32 get_distance(fvec2 position);
    void add_menu_sound(Sound *sound, r32 volume = 1, s32 loops = 0, SoundStatus *status = NULL);
    void add_menu_sound(const char *name, r32 volume = 1, s32 loops = 0, SoundStatus *status = NULL);
    void update();
    void set_mic_position(fvec2 pos);
    void free_channel(int channel);
    void deinit_channels();

    void pause_world_sounds();
    void resume_world_sounds();
    void stop_world_sounds();

    void pause_sound(u32 channel);
    void resume_sound(u32 channel);
    void stop_sound(u32 channel);

    void clear_queue();*/
    void play_world_sound(Sound *sound, s32 loops, fvec2 *pos, fvec2 *vel);
}

#endif // SOUND_H
