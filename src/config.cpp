#include "config.h"
#include "Platform/platform.h"
#include <fstream>

namespace Config
{
    bool fullscreen = false;
    r32 sfxVolume = 1.0, musicVolume = 1.0;
}

void Config::load_config()
{
    std::ifstream file("res/settings.cfg");
    if (!file.is_open())
    {
        save_config();
        return;
    }

    file.read((char*)&fullscreen, sizeof(bool));
    file.read((char*)&sfxVolume, sizeof(r32));
    file.read((char*)&musicVolume, sizeof(r32));

    file.close();

    if (fullscreen)
        platform_set_fullscreen(true);
    platform_set_sfx_volume(sfxVolume);
    platform_set_music_volume(musicVolume);
}
void Config::save_config()
{
    std::ofstream file("res/settings.cfg");

    file.write((char*)&fullscreen, sizeof(bool));
    file.write((char*)&sfxVolume, sizeof(r32));
    file.write((char*)&musicVolume, sizeof(r32));

    file.close();
}
void Config::set_fullscreen(bool a)
{
    fullscreen = a;
    platform_set_fullscreen(a);
}
void Config::toggle_fullscreen()
{
    fullscreen = !fullscreen;
    platform_set_fullscreen(fullscreen);
}
bool Config::get_fullscreen()
{
    return fullscreen;
}
void Config::set_sfx_volume(r32 volume)
{
    sfxVolume = volume;
    platform_set_sfx_volume(volume);
}
void Config::set_music_volume(r32 volume)
{
    musicVolume = volume;
    platform_set_music_volume(volume);
}
r32 Config::get_sfx_volume()
{
    return sfxVolume;
}
r32 Config::get_music_volume()
{
    return musicVolume;
}
