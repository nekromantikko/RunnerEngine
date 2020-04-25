#ifndef DATABASES_H
#define DATABASES_H

#include <vector>
#include <cstddef>
#include <iostream>
#include <map>
#include "sprite.h"
#include "model.h"
#include "shared.h"
#include "particles.h"

struct TempImage
{
    rImage image;
    std::string textureName;
};

class Tileset;

namespace Resource
{
    void init();
    void deinit();

    Sound *get_sound(std::string handle);
    MultiSound *get_multisound(std::string handle);
    Tileset *get_tileset(std::string handle);
    Model *get_model(std::string handle);
    Sprite *get_sprite(std::string handle);
    Texture *get_texture(std::string handle);
    Music *get_music(std::string handle);
    ParticleSystemPrefab *get_particle_system(std::string handle);
    ////////////////////////////////

    void animate_tilesets();
    Texture *no_texture();
    Texture *no_normal();
    Texture *no_lightmap();

    //this should only be accessed by a loading thread
    namespace Loader
    {
        Sound *load_sound(std::string fname);
        bool load_tileset(const char *fname, Tileset *tileset);
        //Model *load_model(std::string fname);
        //Sprite *load_sprite(std::string fname);
        Texture *load_texture(std::string fname, bool srgb = true);
        Music *load_music(std::string fname);
        void load_sprites();
        void load_models();
        void load_particles();
        void load_multisounds();

        void load_assets();

        void throw_file_error(std::string msg, std::string fname);
    }
}


#endif // DATABASES_H

