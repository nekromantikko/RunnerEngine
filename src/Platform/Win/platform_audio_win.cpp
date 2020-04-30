#include "../../platform_audio.h"

#define OGG_MUSIC
#define OGG_USE_TREMOR

//FMOD
FMOD_SYSTEM *soundSystem;

FMOD_CHANNELGROUP *masterChannelGroup;
FMOD_CHANNELGROUP *sfxChannelGroup;
FMOD_CHANNELGROUP *menuChannelGroup;
FMOD_CHANNELGROUP *worldChannelGroup;
FMOD_CHANNELGROUP *musicChannelGroup;
FMOD_CHANNEL *track1;
FMOD_CHANNEL *track2;

void init_FMOD()
{
    //init sound
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    FMOD_System_Create(&soundSystem);
    FMOD_System_Init(soundSystem, 256, FMOD_INIT_NORMAL, 0);

    //create channel groups
    FMOD_System_CreateChannelGroup(soundSystem, "SFXGroup", &sfxChannelGroup);
    FMOD_System_CreateChannelGroup(soundSystem, "WorldGroup", &worldChannelGroup);
    FMOD_System_CreateChannelGroup(soundSystem, "MenuGroup", &menuChannelGroup);
    FMOD_System_CreateChannelGroup(soundSystem, "MusicGroup", &musicChannelGroup);

    //set groups as children of master group
    FMOD_System_GetMasterChannelGroup(soundSystem, &masterChannelGroup);
    FMOD_ChannelGroup_AddGroup(sfxChannelGroup, worldChannelGroup, 1, 0);
    FMOD_ChannelGroup_AddGroup(sfxChannelGroup, menuChannelGroup, 1, 0);
    FMOD_ChannelGroup_AddGroup(masterChannelGroup, sfxChannelGroup, 1, 0);
    FMOD_ChannelGroup_AddGroup(masterChannelGroup, musicChannelGroup, 1, 0);

    //set listener attributes
    FMOD_VECTOR vector_forward = {0, 0, -1};
    FMOD_VECTOR vector_up = {0, -1, 0};
    FMOD_System_Set3DListenerAttributes(soundSystem, 0, NULL, NULL, &vector_forward, &vector_up);
}

void close_FMOD()
{
    //FMOD quit
    FMOD_System_Release(soundSystem);
    CoUninitialize();
}

/////////////////////////////////////////////////////////////////////

struct InternalSound
{
    //Mix_Chunk *chunk;
    FMOD_SOUND *sound;
};

struct InternalMusic
{
    FMOD_SOUND *sound;
};

u32 platform_play_world_sound(Sound *sound, s32 loops, fvec2 *pos, fvec2 *vel)
{
    if (!sound)
        return NULL;
    /*Mix_VolumeChunk(sound->chunk, MIX_MAX_VOLUME * volume);
    Mix_PlayChannel(channel, sound->chunk, loops);
    Mix_ChannelFinished(channel_finished);*/
    FMOD_CHANNEL *chan;
    FMOD_System_PlaySound(soundSystem, sound->sound, worldChannelGroup, 1, &chan);

    FMOD_Channel_Set3DLevel(chan, 1.0f);

    FMOD_VECTOR position = {0, 0, 0};
    FMOD_VECTOR velocity = {0, 0, 0};

    if (pos)
    {
        position.x = pixels_to_meters(pos->x);
        position.y = pixels_to_meters(pos->y);
    }
    if (vel)
    {
        velocity.x = vel->x;
        velocity.y = vel->y;
    }

    FMOD_Channel_Set3DAttributes(chan, &position, &velocity, NULL);
    FMOD_Channel_SetLoopCount(chan, loops);
    FMOD_Channel_Set3DMinMaxDistance(chan, 1.0, 32.0);

    FMOD_Channel_SetPaused(chan, false);

    return (intptr_t)chan;

}
u32 platform_play_menu_sound(Sound *sound, s32 loops)
{
    if (!sound)
        return NULL;

    FMOD_CHANNEL *chan;
    FMOD_System_PlaySound(soundSystem, sound->sound, menuChannelGroup, 1, &chan);

    FMOD_Channel_Set3DLevel(chan, 0.0f);
    FMOD_Channel_SetLoopCount(chan, loops);
    FMOD_Channel_SetPaused(chan, false);

    return (intptr_t)chan;
}
void platform_stop_channel(u32 channel)
{
    //Mix_HaltChannel(channel);
    if (channel)
    {
        FMOD_Channel_Stop((FMOD_CHANNEL*)channel);
    }
}
void platform_pause_channel(u32 channel)
{
    //Mix_Pause(channel);
    if (channel)
    {
        FMOD_Channel_SetPaused((FMOD_CHANNEL*)channel, true);
    }
}
void platform_resume_channel(u32 channel)
{
    //Mix_Resume(channel);
    if (channel)
    {
        FMOD_Channel_SetPaused((FMOD_CHANNEL*)channel, false);
    }
}
u32 platform_play_music(Music *music, bool32 mute, s32 fade, r64 pos)
{
    if (!music)
        return NULL;

    FMOD_CHANNEL *chan;
    FMOD_System_PlaySound(soundSystem, music->sound, musicChannelGroup, 1, &chan);

    //fade in
    /*if (fade)
    {
        unsigned long long dspclock;
        int rate;

        FMOD_System_GetSoftwareFormat(soundSystem, &rate, NULL, NULL);

        FMOD_ChannelGroup_GetDSPClock(musicChannelGroup, &dspclock, NULL);
        FMOD_Channel_AddFadePoint(track1, dspclock, 0.0f);
        FMOD_Channel_AddFadePoint(track1, dspclock + (fade * rate), 1.0f);
    }*/

    FMOD_Channel_SetMute(chan, mute);
    FMOD_Channel_SetLoopCount(chan, -1);
    FMOD_Channel_SetPaused(chan, false);

    return (intptr_t)chan;
}
void platform_stop_music(s32 fade)
{
    /*if (fade)
    {
        unsigned long long dspclock;
        int rate;

        FMOD_System_GetSoftwareFormat(soundSystem, &rate, NULL, NULL);

        FMOD_ChannelGroup_GetDSPClock(musicChannelGroup, &dspclock, NULL);
        FMOD_Channel_AddFadePoint(track1, dspclock, 1.0f);
        FMOD_Channel_AddFadePoint(track1, dspclock + (fade * rate), 0.0f);
        FMOD_Channel_SetDelay(track1, 0, dspclock + (fade * rate), true);
    }*/
    FMOD_ChannelGroup_Stop(musicChannelGroup);
}
void platform_set_sfx_volume(r32 volume)
{
    //Mix_Volume(-1, MIX_MAX_VOLUME * volume);
    FMOD_ChannelGroup_SetVolume(sfxChannelGroup, volume);
}
void platform_set_music_volume(r32 volume)
{
    //Mix_VolumeMusic(MIX_MAX_VOLUME * volume);
    FMOD_ChannelGroup_SetVolume(musicChannelGroup, volume);
}
bool platform_music_is_playing()
{
    //return Mix_PlayingMusic();
    bool32 result;
    FMOD_ChannelGroup_IsPlaying(musicChannelGroup, &result);
    return result;
}

//FMOD
void platform_update_audio()
{
    FMOD_System_Update(soundSystem);
}
void platform_set_listener_attributes(fvec2 *pos, fvec2 *vel)
{
    FMOD_VECTOR position = {0, 0, 5};
    FMOD_VECTOR velocity = {0, 0, 0};

    if (pos)
    {
        position.x = pixels_to_meters(pos->x);
        position.y = pixels_to_meters(pos->y);
    }
    if (vel)
    {
        velocity.x = vel->x;
        velocity.y = vel->y;
    }

    FMOD_System_Set3DListenerAttributes(soundSystem, 0, &position, &velocity, NULL, NULL);
}
void platform_pause_world_sounds()
{
    FMOD_ChannelGroup_SetPaused(worldChannelGroup, true);
}
void platform_resume_world_sounds()
{
    FMOD_ChannelGroup_SetPaused(worldChannelGroup, false);
}
void platform_stop_world_sounds()
{
    FMOD_ChannelGroup_Stop(worldChannelGroup);
}
void platform_stop_menu_sounds()
{
    FMOD_ChannelGroup_Stop(menuChannelGroup);
}

void platform_mute_channel(s32 channel, bool32 mute)
{
    FMOD_CHANNEL *chan = (FMOD_CHANNEL*)channel;
    FMOD_Channel_SetMute(chan, mute);
}
void platform_fade_channel(s32 channel, r32 startVol, r32 endVol, r32 length)
{
    FMOD_CHANNEL *chan = (FMOD_CHANNEL*)channel;
    unsigned long long dspclock;
    int rate;

    FMOD_System_GetSoftwareFormat(soundSystem, &rate, NULL, NULL);

    FMOD_ChannelGroup_GetDSPClock(musicChannelGroup, &dspclock, NULL);
    FMOD_Channel_AddFadePoint(chan, dspclock, startVol);
    FMOD_Channel_AddFadePoint(chan, dspclock + (length * rate), endVol);
}
