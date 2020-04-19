#ifndef CONFIG_H
#define CONFIG_H
#include "typedef.h"

namespace Config
{
    void load_config();
    void save_config();
    void set_fullscreen(bool);
    void toggle_fullscreen();
    bool get_fullscreen();
    void set_sfx_volume(r32 volume);
    void set_music_volume(r32 volume);
    r32 get_sfx_volume();
    r32 get_music_volume();
}

#endif // CONFIG_H
