#include "audio.h"
#include "Platform/platform_audio.h"

void AudioManager::play_menu_sfx(Sound *sound, s32 loops)
{
    //pick a random sound
    u32 soundIndex = 0;
    if (sound->soundCount == 0)
        return;
    else if (sound->soundCount > 1)
        soundIndex = rand() % (sound->soundCount - 1);

    platform_play_menu_sound(sound->sounds[soundIndex], loops);
}
void AudioManager::play_world_sfx(Sound *sound, s32 loops, fvec2 *pos, fvec2 *vel)
{
    //pick a random sound
    u32 soundIndex = 0;
    if (sound->soundCount == 0)
        return;
    else if (sound->soundCount > 1)
        soundIndex = rand() % (sound->soundCount - 1);

    platform_play_world_sound(sound->sounds[soundIndex], loops, pos, vel);
}

void AudioManager::pause_world_sounds()
{
    platform_pause_world_sounds();
}
void AudioManager::resume_world_sounds()
{
    platform_resume_world_sounds();
}
void AudioManager::stop_world_sounds()
{
    platform_stop_world_sounds();
}
void AudioManager::stop_menu_sounds()
{
    platform_stop_menu_sounds();
}
