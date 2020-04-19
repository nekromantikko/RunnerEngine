#include "soundmanager.h"
#include "Platform/platform.h"
#include "pool.h"
#include "resource.h"

#include <list>
/*
namespace SoundManager
{
    std::list<SoundInfo> menuSoundQueue;
    Pool<SoundInfo> menuSounds; //channels [0, MENU_CHANNEL_COUNT[

    std::list<SoundInfo> worldSoundQueue;
    Pool<SoundInfo> worldSounds; //channels [MENU_CHANNEL_COUNT, WORLD_CHANNEL_COUNT[

    std::list<u32> channelFreeBuffer;

    fvec2 micPosition;
}

void SoundManager::init()
{
    menuSounds.allocate(MENU_CHANNEL_COUNT);
    worldSounds.allocate(WORLD_CHANNEL_COUNT);
}

void SoundManager::add_world_sound(Sound *sound, r32 volume, s32 loops, fvec2 *position, SoundStatus *status)
{
    if (!sound)
        return;

    SoundInfo temp;

    temp.sound = sound;
    temp.loops = loops;
    temp.volume = volume;
    temp.position = position;

    temp.status = status;
    if (status)
        *status = SOUND_IN_QUEUE;

    worldSoundQueue.push_back(temp);
}

void SoundManager::add_world_sound(const char *name, r32 volume, s32 loops, fvec2 *position, SoundStatus *status)
{
    Sound *sound = ResourceDatabase::get_sound(name);
    add_world_sound(sound, volume, loops, position, status);
}

void SoundManager::add_world_multisound(const char *name, r32 volume, s32 loops, fvec2 *position, SoundStatus *status)
{
    MultiSound *mSound = ResourceDatabase::get_multisound(name);

    if (!mSound)
        return;
    if (mSound->soundCount == 0)
        return;

    u32 random = random_integer(0, mSound->soundCount - 1);
    Sound *randomSound = mSound->sounds[random];

    add_world_sound(randomSound, volume, loops, position, status);


}

r32 SoundManager::get_distance(fvec2 position)
{
    fvec2 AB = micPosition - position;
    r32 distance = pixels_to_meters(Length(AB));

    r32 result = distance / (distance + 1);
    return result;
}

void SoundManager::add_menu_sound(Sound *sound, r32 volume, s32 loops, SoundStatus *status)
{
    if (!sound)
        return;

    SoundInfo temp;

    temp.sound = sound;
    temp.loops = loops;
    temp.volume = volume;
    temp.position = NULL;

    temp.status = status;
    if (status)
        *status = SOUND_IN_QUEUE;

    menuSoundQueue.push_back(temp);
}

void SoundManager::add_menu_sound(const char *name, r32 volume, s32 loops, SoundStatus *status)
{
    Sound *sound = ResourceDatabase::get_sound(name);
    add_menu_sound(sound, volume, loops, status);
}

void SoundManager::update()
{
    deinit_channels();

    u32 channel;
    while (menuSounds.has_room() && menuSoundQueue.size() > 0)
    {
        SoundInfo &sound = menuSoundQueue.front();

        SoundInfo *newSound = menuSounds.init();
        *newSound = sound;

        channel = menuSounds.get_index(newSound);

        platform_set_channel_distance(channel, 0);
        platform_play_world_sound(sound.sound, sound.volume, sound.loops, channel, &free_channel);

        if (newSound->status)
            *newSound->status = (SoundStatus)channel;

        menuSoundQueue.pop_front();
    }

    while (worldSounds.has_room() && worldSoundQueue.size() > 0)
    {
        SoundInfo &sound = worldSoundQueue.front();

        r32 distance = 0;
        if (sound.position)
            distance = get_distance(*sound.position);

        if (distance <= 0.97)
        {
            SoundInfo *newSound = worldSounds.init();
            *newSound = sound;

            channel = worldSounds.get_index(newSound) + MENU_CHANNEL_COUNT;

            platform_set_channel_distance(channel, distance);
            platform_play_world_sound(sound.sound, sound.volume, sound.loops, channel, &free_channel);

            if (newSound->status)
                *newSound->status = (SoundStatus)channel;
        }
        else
        {
            if (sound.status)
                *sound.status = SOUND_STOPPED;
        }

        worldSoundQueue.pop_front();
    }

    //update playing sounds
    for (auto it = worldSounds.begin(); it != worldSounds.partition(); it++)
    {
        SoundInfo *info = &*it;
        if (!info->position)
            continue;

        u32 channel = worldSounds.get_index(info) + MENU_CHANNEL_COUNT;
        r32 distance = get_distance(*info->position);

        if (distance > 0.97)
            stop_sound(channel);
        else platform_set_channel_distance(channel, distance);
    }
}

void SoundManager::set_mic_position(fvec2 pos)
{
    micPosition = pos;
}

void SoundManager::free_channel(int channel)
{
    channelFreeBuffer.push_back(channel);
}

void SoundManager::deinit_channels()
{
    while (!channelFreeBuffer.empty())
    {
        u32 channel = channelFreeBuffer.front();

        if (channel < MENU_CHANNEL_COUNT)
        {
            SoundInfo *sound = menuSounds.get(channel);

            if (sound->status)
                *sound->status = SOUND_STOPPED;

            menuSounds.deinit(channel);
        }
        else
        {
            SoundInfo *sound = worldSounds.get(channel - MENU_CHANNEL_COUNT);

            if (sound->status)
                *sound->status = SOUND_STOPPED;

            worldSounds.deinit(channel - MENU_CHANNEL_COUNT);
        }

        channelFreeBuffer.pop_front();
    }
}

void SoundManager::pause_world_sounds()
{
    platform_pause_world_sounds();
}
void SoundManager::resume_world_sounds()
{
    platform_resume_world_sounds();
}
void SoundManager::stop_world_sounds()
{
    platform_stop_world_sounds();
}

void SoundManager::pause_sound(u32 channel)
{
    platform_pause_channel(channel);

    if (channel < MENU_CHANNEL_COUNT)
    {
        SoundInfo *sound = menuSounds.get(channel);

        if (sound->status)
            *sound->status = SOUND_PAUSED;
    }
    else
    {
        SoundInfo *sound = worldSounds.get(channel - MENU_CHANNEL_COUNT);

        if (sound->status)
            *sound->status = SOUND_PAUSED;
    }
}
void SoundManager::resume_sound(u32 channel)
{
    platform_resume_channel(channel);

    if (channel < MENU_CHANNEL_COUNT)
    {
        SoundInfo *sound = menuSounds.get(channel);

        if (sound->status)
            *sound->status = (SoundStatus)channel;
    }
    else
    {
        SoundInfo *sound = worldSounds.get(channel - MENU_CHANNEL_COUNT);

        if (sound->status)
            *sound->status = (SoundStatus)channel;
    }
}
void SoundManager::stop_sound(u32 channel)
{
    //platform_stop_channel(channel);
}
void SoundManager::clear_queue()
{
    worldSoundQueue.clear();
}*/

void SoundManager::play_world_sound(Sound *sound, s32 loops, fvec2 *pos, fvec2 *vel)
{
    platform_play_world_sound(sound, loops, pos, vel);
}
