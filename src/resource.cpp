#include "resource.h"
#include <stdexcept>
#include <sstream>
#include "Platform/platform.h"
#include "tiles.h"
#include "file_formats.h"
#include <fstream>
#include <atomic>

namespace Resource
{
    std::map<std::string, Sound*> sounds;
    std::map<std::string, MultiSound> multiSounds;
    std::map<std::string, Tileset> tilesets;
    std::map<std::string, Model> models;
    std::map<std::string, Sprite> sprites;
    std::map<std::string, Texture*> textures;
    std::map<std::string, Music*> musics;
    std::map<std::string, ParticleSystemPrefab> particleSystems;
    //std::list<TempImage> tempImages;

    Texture *error;
    Texture *noNormal;
    Texture *noLightmap;
}

void Resource::init()
{
    error = platform_load_texture("res/textures/error.png", true);
    noNormal = platform_load_texture("res/textures/normal_none.png", false);
    noLightmap = platform_load_texture("res/textures/lightmap_none.png", false);
}

void Resource::deinit()
{
    //delete textures
    for (auto& x : textures)
        platform_delete_texture(x.second);
    //delete sounds
    for (auto& x : sounds)
        platform_delete_sound(x.second);
    //delete musics
    /*
    for (auto& x : textures)
        platform_delete_texture(x.second);*/
    //delete tilesets
    //for (auto& x : tilesets)
        //delete x.second;
}

Sound *Resource::get_sound(std::string handle)
{
    auto found = sounds.find(handle);
    if (sounds.find(handle) != sounds.end())
        return (*found).second;
    else return NULL;
}

Music *Resource::get_music(std::string handle)
{
    auto found = musics.find(handle);
    if (musics.find(handle) != musics.end())
        return (*found).second;
    else return NULL;
}

Tileset *Resource::get_tileset(std::string handle)
{
    auto found = tilesets.find(handle);
    if (found != tilesets.end())
        return &(*found).second;
    else return NULL;
}

Model *Resource::get_model(std::string handle)
{
    auto found = models.find(handle);
    if (found != models.end())
        return &(*found).second;
    else return NULL;
}

Sprite *Resource::get_sprite(std::string handle)
{
    auto found = sprites.find(handle);
    if (found != sprites.end())
        return &(*found).second;
    else return NULL;
}

Texture *Resource::get_texture(std::string handle)
{
    auto found = textures.find(handle);
    if (found != textures.end())
        return (*found).second;
    else return NULL;
}

ParticleSystemPrefab *Resource::get_particle_system(std::string handle)
{
    auto found = particleSystems.find(handle);
    if (found != particleSystems.end())
        return &(*found).second;
    else return NULL;
}

MultiSound *Resource::get_multisound(std::string handle)
{
    auto found = multiSounds.find(handle);
    if (found != multiSounds.end())
        return &(*found).second;
    else return NULL;
}

//////////////////////////////////


Sound *Resource::Loader::load_sound(std::string fname)
{
    //process filename
    std::stringstream file;
    file << "res/sounds/" << fname << ".wav";
    Sound *sound = platform_load_sound(file.str().c_str());
    if (sound)
        sounds.emplace(fname, sound);
    return sound;
}

Music *Resource::Loader::load_music(std::string fname)
{
    //process filename
    std::stringstream file;
    file << "res/music/" << fname;
    Music *music = platform_load_music(file.str().c_str());
    musics.emplace(fname, music);
    return music;
}

Tileset *Resource::Loader::load_tileset(std::string fname)
{
    //process filename
    std::stringstream file;
    file << "res/tilesets/" << fname << ".rtil";
    return &((*tilesets.emplace(fname, file.str()).first).second);
}
/*
Model *ResourceLoader::load_model(std::string fname)
{
    //process filename
    std::stringstream file;
    file << "res/models/" << fname << ".mdl";
    return &((*models.emplace(fname, file.str()).first).second);
}

Sprite *ResourceLoader::load_sprite(std::string fname)
{
    //process filename
    std::stringstream file;
    file << "res/sprites/" << fname << ".spr";
    return &((*sprites.emplace(fname,file.str()).first).second);
}
*/
Texture *Resource::Loader::load_texture(std::string fname, bool srgb)
{
    //process filename
    std::stringstream file;
    file << "res/textures/" << fname << ".png";
    //rImage image;
    //platform_load_image(file.str().c_str(), &image);
    Texture *texture = platform_load_texture(file.str().c_str(), srgb);
    //delete[] image.pixels;
    textures.emplace(fname, texture);
    return texture;
}

void Resource::Loader::load_sprites()
{
    std::ifstream file("res/sprites.rspr", std::ios::binary);

    if (file)
    {
        SpriteHeader header;
        file.read((char*)&header, sizeof(header));

        if (header.signature != SPRITE_SIGNATURE)
            throw std::runtime_error("Sprite file is not a proper rspr-file!");
        else if (header.version != SPRITE_VERSION)
            throw std::runtime_error("Sprite file is not the right version!");
        else
        {
            //for each sprite
            for (u32 i = 0; i < header.spriteCount; i++)
            {
                //read sprite info
                SpriteInfo sInfo;
                file.read((char*)&sInfo, sizeof(sInfo));

                //read name
                u32 spriteNameLength = sInfo.textureOffset - sInfo.nameOffset;
                char *spriteNameBuffer = new char[spriteNameLength];
                file.read(spriteNameBuffer, spriteNameLength);
                std::string spriteName(spriteNameBuffer, spriteNameLength);

                //read texture name
                u32 texNameLength = sInfo.lightmapOffset - sInfo.textureOffset;
                char *texNameBuffer = new char[texNameLength];
                file.read(texNameBuffer, texNameLength);
                std::string texName(texNameBuffer, texNameLength);

                //read lightmap name
                u32 lmapNameLength = sInfo.normalOffset - sInfo.lightmapOffset;
                char *lmapNameBuffer = new char[lmapNameLength];
                file.read(lmapNameBuffer, lmapNameLength);
                std::string lmapName(lmapNameBuffer, lmapNameLength);

                //read normal name
                u32 normNameLength = sInfo.animsOffset - sInfo.normalOffset;
                char *normNameBuffer = new char[normNameLength];
                file.read(normNameBuffer, normNameLength);
                std::string normName(normNameBuffer, normNameLength);

                delete[] spriteNameBuffer;
                delete[] texNameBuffer;
                delete[] lmapNameBuffer;
                delete[] normNameBuffer;

                //create sprite
                Sprite *sprite = &sprites[spriteName];
                Texture *tex = NULL;
                if (texName.size() > 0)
                    tex = Resource::get_texture(texName);
                if (tex)
                    sprite->texture = tex;
                else sprite->texture = error;

                Texture *lmap = NULL;
                if (lmapName.size() > 0)
                    lmap = Resource::get_texture(lmapName);
                if (lmap)
                    sprite->lightmap = lmap;
                else sprite->lightmap = noLightmap;

                Texture *norm = NULL;
                if (normName.size() > 0)
                    norm = Resource::get_texture(normName);
                if (norm)
                    sprite->normal = norm;
                else sprite->normal = noNormal;

                sprite->width = sInfo.width;
                sprite->height = sInfo.height;
                sprite->xOffset = sInfo.xoffset;
                sprite->yOffset = sInfo.yoffset;
                sprite->glow = sInfo.glow;

                sprite->create_clipframes();

                //loop thru anims
                for (u32 j = 0; j < sInfo.animCount; j++)
                {
                    SpriteAnimationInfo animInfo;
                    file.read((char*)&animInfo, sizeof(animInfo));

                    //read animation name
                    u32 animNameLength = animInfo.framesOffset - animInfo.nameOffset;
                    char *animNameBuffer = new char[animNameLength];
                    file.read(animNameBuffer, animNameLength);
                    std::string animName(animNameBuffer, animNameLength);

                    delete[] animNameBuffer;

                    sprite->animations.emplace_back(animName);
                    Animation *anim = &sprite->animations.back();

                    //read frames
                    for (u32 k = 0; k < animInfo.frameCount; k++)
                    {
                        u32 index;
                        file.read((char*)&index, sizeof(index));
                        anim->frames.push_back(index);
                    }
                }
            }
        }

        file.close();
    }
    else throw std::runtime_error("can't open sprite file!");
}

void Resource::Loader::load_models()
{
    std::ifstream file("res/models.rmdl", std::ios::binary);

    if (file)
    {
        ModelHeader header;
        file.read((char*)&header, sizeof(header));

        if (header.signature != MODEL_SIGNATURE)
            throw std::runtime_error("Model file is not a proper rmdl-file!");
        else if (header.version != MODEL_VERSION)
            throw std::runtime_error("Model file is not the right version!");
        else
        {
            //for each model
            for (u32 i = 0; i < header.modelCount; i++)
            {
                //read sprite info
                ModelInfo mInfo;
                file.read((char*)&mInfo, sizeof(mInfo));

                //read name
                u32 modelNameLength = mInfo.textureOffset - mInfo.nameOffset;
                char *modelNameBuffer = new char[modelNameLength];
                file.read(modelNameBuffer, modelNameLength);
                std::string modelName(modelNameBuffer, modelNameLength);

                //read texture name
                u32 texNameLength = mInfo.lightmapOffset - mInfo.textureOffset;
                char *texNameBuffer = new char[texNameLength];
                file.read(texNameBuffer, texNameLength);
                std::string texName(texNameBuffer, texNameLength);

                //read lightmap name
                u32 lmapNameLength = mInfo.normalOffset - mInfo.lightmapOffset;
                char *lmapNameBuffer = new char[lmapNameLength];
                file.read(lmapNameBuffer, lmapNameLength);
                std::string lmapName(lmapNameBuffer, lmapNameLength);

                //read normal name
                u32 normNameLength = mInfo.vertexOffset - mInfo.normalOffset;
                char *normNameBuffer = new char[normNameLength];
                file.read(normNameBuffer, normNameLength);
                std::string normName(normNameBuffer, normNameLength);

                delete[] modelNameBuffer;
                delete[] texNameBuffer;
                delete[] lmapNameBuffer;
                delete[] normNameBuffer;

                //create model
                Model *model = &models[modelName];
                Texture *tex = NULL;
                if (texName.size() > 0)
                    tex = Resource::get_texture(texName);
                if (tex)
                    model->texture = tex;
                else model->texture = error;

                Texture *lmap = NULL;
                if (lmapName.size() > 0)
                    lmap = Resource::get_texture(lmapName);
                if (lmap)
                    model->lightmap = lmap;
                else model->lightmap = noLightmap;

                Texture *norm = NULL;
                if (normName.size() > 0)
                    norm = Resource::get_texture(normName);
                if (norm)
                    model->normal = norm;
                else model->normal = noNormal;

                model->glow = mInfo.glow;

                //get mesh
                Mesh mesh;
                u64 currentPos = file.tellg();

                //get vertices
                while (currentPos < mInfo.indexOffset + sizeof(header))
                {
                    ModelVertInfo vertex;
                    file.read((char*)&vertex, sizeof(vertex));

                    v3 pos = {vertex.x, vertex.y, vertex.z};
                    v2 uv = {vertex.u, vertex.v};
                    v3 normal = {vertex.nx, vertex.ny, vertex.nz};

                    mesh.positions.push_back(pos);
                    mesh.texCoords.push_back(uv);
                    mesh.normals.push_back(normal);
                    currentPos = file.tellg();
                }

                //get index
                while (currentPos < mInfo.onePastIndexOffset + sizeof(header))
                {
                    Triangle tri;
                    file.read((char*)&tri, sizeof(tri));

                    mesh.tris.push_back(tri);
                    currentPos = file.tellg();
                }

                platform_calculate_tangent(&mesh);
                model->vbuffer = platform_create_vertex_array(&mesh);
            }
        }
        file.close();
    }
    else throw std::runtime_error("can't open model file!");
}

void Resource::Loader::load_particles()
{
    std::ifstream file("res/particles.rpar", std::ios::binary);

    if (file)
    {
        ParticleHeader header;
        file.read((char*)&header, sizeof(header));

        if (header.signature != PARTICLE_SIGNATURE)
            throw std::runtime_error("Particle file is not a proper rmdl-file!");
        else if (header.version != PARTICLE_VERSION)
            throw std::runtime_error("Particle file is not the right version!");
        else
        {
            //for each system
            for (u32 i = 0; i < header.systemCount; i++)
            {
                //read system info
                ParticleSystemInfo sInfo;
                file.read((char*)&sInfo, sizeof(sInfo));

                //read name
                u32 systemNameLength = sInfo.emitterOffset - sInfo.nameOffset;
                char *systemNameBuffer = new char[systemNameLength];
                file.read(systemNameBuffer, systemNameLength);
                std::string systemName(systemNameBuffer, systemNameLength);
                delete[] systemNameBuffer;

                ParticleSystemPrefab *system = &particleSystems[systemName];
                system->emitterCount = sInfo.emitterCount;
                system->emitters = new ParticleEmitterPrefab[sInfo.emitterCount];

                //read emitters
                for (u32 j = 0; j < sInfo.emitterCount; j++)
                {
                    ParticleEmitterInfo eInfo;
                    file.read((char*)&eInfo, sizeof(eInfo));

                    u32 spriteNameLength = eInfo.onePastSpriteOffset - eInfo.spriteOffset;
                    char *spriteNameBuffer = new char[spriteNameLength];
                    file.read(spriteNameBuffer, spriteNameLength);
                    std::string spriteName(spriteNameBuffer, spriteNameLength);
                    delete[] spriteNameBuffer;

                    ParticleEmitterPrefab *emitter = &system->emitters[j];

                    emitter->sprite = get_sprite(spriteName);
                    emitter->spawnRate = eInfo.spawnRate;
                    emitter->duration = eInfo.duration;
                    emitter->loops = eInfo.loops;
                    emitter->spawnAreaType = (ParticleSpawnAreaType)eInfo.spawnAreaType;
                    memcpy(&emitter->spawnRectDimensions, eInfo.spawnAreaData, 8);
                    emitter->pLifetime = {eInfo.lifeTime[0], eInfo.lifeTime[1]};
                    emitter->pInitialScale = {eInfo.initialScale[0], eInfo.initialScale[1]};
                    emitter->pInitialVelocity = {{eInfo.initialVelocity[0], eInfo.initialVelocity[1]}, {eInfo.initialVelocity[2], eInfo.initialVelocity[3]}};
                    emitter->pInitialRotation = {eInfo.initialRotation[0], eInfo.initialRotation[1]};
                    emitter->pDeltaRotation = 0.025;
                    emitter->pAnimSpeed = {eInfo.animSpeed[0], eInfo.animSpeed[1]};
                    emitter->pInitialColor = {1,1,1,1};
                    emitter->pEndColor = {1,1,1,0};
                    emitter->animation = eInfo.animation;
                    emitter->gravity = eInfo.gravity;
                }
            }
        }
        file.close();
    }
    else throw std::runtime_error("can't open particle file!");
}

void Resource::Loader::load_multisounds()
{
    std::ifstream file("res/multisounds.rsnd", std::ios::binary);

    if (file)
    {
        MultiSoundHeader header;
        file.read((char*)&header, sizeof(header));

        if (header.signature != MULTISOUND_SIGNATURE)
            throw std::runtime_error("Multisound file is not a proper rmdl-file!");
        else if (header.version != MULTISOUND_VERSION)
            throw std::runtime_error("Multisound file is not the right version!");
        else
        {
            //for each system
            for (u32 i = 0; i < header.multiSoundCount; i++)
            {
                //read system info
                MultiSoundInfo sInfo;
                file.read((char*)&sInfo, sizeof(sInfo));

                //read name
                u32 systemNameLength = sInfo.soundsOffset - sInfo.nameOffset;
                char *systemNameBuffer = new char[systemNameLength];
                file.read(systemNameBuffer, systemNameLength);
                std::string systemName(systemNameBuffer, systemNameLength);
                delete[] systemNameBuffer;

                MultiSound *sound = &multiSounds[systemName];
                sound->soundCount = sInfo.soundCount;
                sound->sounds = new Sound*[sInfo.soundCount];

                //read sounds
                for (u32 j = 0; j < sInfo.soundCount; j++)
                {
                    MultiSoundSoundInfo eInfo;
                    file.read((char*)&eInfo, sizeof(eInfo));

                    u32 spriteNameLength = eInfo.onePastNameOffset - eInfo.nameOffset;
                    char *spriteNameBuffer = new char[spriteNameLength];
                    file.read(spriteNameBuffer, spriteNameLength);
                    std::string spriteName(spriteNameBuffer, spriteNameLength);
                    delete[] spriteNameBuffer;

                    Sound *penis = Resource::get_sound(spriteName);
                    sound->sounds[j] = penis;
                }
            }
        }
        file.close();
    }
    else throw std::runtime_error("can't open multisound file!");
}

void Resource::Loader::load_assets()
{
    load_texture("tex_ascii", true);
    load_texture("fireball", true);
    load_texture("medkit", true);
    load_texture("medkit_normal", false);
    load_texture("player", true);
    load_texture("tex_beam", true);
    load_texture("tex_beam_lightmap", false);
    load_texture("tex_door", true);
    load_texture("tex_platform", true);
    load_texture("tex_particle_debug", true);
    load_texture("tex_logo", true);
    load_texture("tex_sewer", true);
    load_texture("tex_sewer_normal", false);
    load_texture("tex_switch", true);
    load_texture("loading", true);

    load_texture("tilesets/tileset_city", true);
    load_texture("tilesets/tileset_city_light", false);
    load_texture("tilesets/tileset_city_normal", false);
    load_texture("tilesets/tileset_debug", true);
    load_texture("tilesets/tileset_debug_light", false);
    load_texture("tilesets/tileset_debug_normal", false);

    load_sound("sfx_jump1");
    load_sound("sfx_jump2");
    load_sound("sfx_jump3");
    load_sound("sfx_jump4");
    load_sound("sfx_death");
    load_sound("sfx_beep");
    load_sound("sfx_menu1");
    load_sound("sfx_hurt");
    load_sound("medkit_pickup");

    load_music("dynatest_drums.ogg");
    load_music("dynatest_idle.ogg");

    load_sprites();
    load_models();
    load_particles();
    load_multisounds();

    load_tileset("tileset_city");
    load_tileset("debug");
}

//////////////////////////////////////////////

void Resource::animate_tilesets()
{
    for (auto& x : tilesets)
        x.second.animate();
}

Texture *Resource::no_texture()
{
    return error;
}
Texture *Resource::no_normal()
{
    return noNormal;
}
Texture *Resource::no_lightmap()
{
    return noLightmap;
}

void Resource::Loader::throw_file_error(std::string msg, std::string fname)
{
    std::stringstream errormsg;
    errormsg << msg << " " << fname;
    throw std::runtime_error(errormsg.str());
}

