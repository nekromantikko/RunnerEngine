#ifndef AUDIO_H
#define AUDIO_H

#include "vector.h"

struct InternalSound;
struct InternalMusic;

struct Sound
{
    InternalSound **sounds;
    u32 soundCount;
};

struct Music
{
    InternalMusic *baseChannel;
    InternalMusic *drumsChannel;
};

namespace AudioManager
{
    void play_menu_sfx(Sound *sound, s32 loops);
    void play_world_sfx(Sound *sound, s32 loops, fvec2 *pos, fvec2 *vel);

    void pause_world_sounds();
    void resume_world_sounds();
    void stop_world_sounds();
    void stop_menu_sounds();
}

#endif // AUDIO_H
