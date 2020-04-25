#include <iostream>
#include <algorithm>
#include <cmath>
#include <sstream>
#include "level.h"
#include "world.h"
#include "resource.h"
#include <sstream>
#include "Platform/platform.h"
#include "renderer.h"
#include "entitymanager.h"
#include "file_formats.h"
#include <fstream>
#include <atomic>

//LEVEL FUNCTIONS

Level::Level() : width(0), height(0), ambientColor({1,1,1}), state(LEVEL_START)
{
    //bgColor[0] = glm::vec3(0, 0.0625, 0.125);
    //bgColor[1] = glm::vec3(0.016, 0.26, 0.5);

    std::cout << "Level <" << this << "> created!" << std::endl;
}

Level::~Level()
{
}

bool Level::is_loaded()
{
    return loaded;
}

void Level::unload()
{
    loaded = false;
    //don't stop music if realoading!
    if (!reload)
    {
        musicPlaying = false;
        platform_stop_music();

        musicBase = 0;
        musicDrums = 0;
    }
}

void Level::end_level(bool r)
{
    state = LEVEL_ENDING;
    reload = r;

    //fade drums
    if (!drumsMuted)
    {
        platform_fade_channel(musicDrums, 1.0, 0.0, 1);
        drumsMuted = true;
    }
    //fade music if not reloading
    if (!reload)
    {
        platform_fade_channel(musicBase, 1.0, 0.0, 1);
        transitionTimer = 60;
    }
    else transitionTimer = 30;
}

void Level::draw(r32 drawRatio)
{
    ///auto get_time = []() {return SDL_GetPerformanceCounter() / (SDL_GetPerformanceFrequency() / 1000);};
    ///s32 drawBegin = get_time();

    if (state == LEVEL_ERROR)
        return;

    platform_bind_framebuffer_post_process();
    platform_render_background(bgColor);

    Renderer::set_up_lights(drawRatio);
    Renderer::draw_tiles(drawRatio, &ambientColor);
    Renderer::draw_models(drawRatio, &ambientColor);
    Renderer::draw_sprites(drawRatio, &ambientColor);

    //platform_render_to_pp();

    /*
    platform_bind_framebuffer_reflect();
    Renderer::draw_reflections(drawRatio);
    platform_use_ui_shader();
    platform_render_reflections();*/

    //platform_bind_framebuffer_post_process();
    platform_render_bloom();

    Renderer::draw_ui(drawRatio);
    platform_render_circle_transition({512, 288}, transitionRadius);
    platform_render_final_image();

    ///s32 endTime = get_time();
    ///std::cout << setupTime - drawBegin << ", " << tileDrawTime - setupTime << ", " << entityDrawTime - tileDrawTime << ", " << endTime - entityDrawTime << std::endl;
}

void Level::update()
{
    if (state == LEVEL_ERROR)
        return;

    if (!musicPlaying)
    {
        musicBase = platform_play_music(Resource::get_music("dynatest_idle.ogg"));
        musicDrums = platform_play_music(Resource::get_music("dynatest_drums.ogg"), true);
        musicPlaying = true;
    }

    timer.update();

    bool physicsUpdate = true;

    if (state == LEVEL_START)
    {
        if (transitionRadius < 600)
            transitionRadius += 30;
        else
        {
            state = LEVEL_PLAYING;
            transitionRadius = 600;
        }
    }
    else if (state == LEVEL_ENDING)
    {
        transitionTimer.update();

        if (transitionRadius > 0)
            transitionRadius -= 30;

        if (transitionTimer.is_stopped())
        {
            state = LEVEL_ENDED;

            World::clear_index_maps();
            EntityManager::deinit_all();
            platform_stop_world_sounds();
            unload();

            //if (reload)
                //load_level(fileName);

            transitionRadius = 0;
        }
        physicsUpdate = false;
    }

    EntityManager::update_entities(physicsUpdate);
    World::update();
    draw_hud();
}

void Level::draw_hud()
{
    //draw time
    u32 mins, secs, cents;

    mins = timer.get_minutes();
    secs = timer.get_seconds();
    cents = timer.get_centiseconds();

    std::stringstream ss;

    ss << "\x80 ";

    if (mins < 10)
        ss << "0";

    ss << mins << ":";

    if (secs < 10)
        ss << "0";

    ss << secs << ":";

    if (cents < 10)
        ss << "0";

    ss << cents;

    //v4 bg = {0.43, 0.0, 0.0, 1.0};
    //DrawOrigin textOrigin = ORIGIN_CENTER;
    //r32 textScale = 8;

    Renderer::draw_string(ss.str(), 16, 56, NULL, NULL, NULL, NULL);
    //Renderer::draw_rectangle(0, 0, 1024, 576, NULL, &bg);
    //Renderer::draw_string("WORLD 1-1", 512, 240, &textOrigin, NULL, &textScale, NULL);
    //Renderer::draw_string(levelName, 512, 288, &textOrigin, NULL, NULL, NULL);
}

bool Level::load_level(std::string fname)
{
    std::cout << "Trying to load level " << fname << std::endl;
    return World::load_map(fname.c_str());
    /*
    bool result = false;

    std::ifstream levelIn(fname, std::ios::binary);
    timer.init();

    if (levelIn)
    {
        fileName = fname;
        LevelHeader header;
        levelIn.read((char*)&header, sizeof(header));

        if (header.signature != LEVEL_SIGNATURE)
            throw std::runtime_error("Level is not a proper rlvl-file!");
        else if (header.version != LEVEL_VERSION)
            throw std::runtime_error("Level file is not the right version!");
        else
        {
            //copy basic info
            width = header.width;
            height = header.height;
            ambientColor = {header.ambientColor[0], header.ambientColor[1], header.ambientColor[2]};
            bgColor[0] = {header.bgColor1[0], header.bgColor1[1], header.bgColor1[2]};
            bgColor[1] = {header.bgColor2[0], header.bgColor2[1], header.bgColor2[2]};

            //read level name
            u32 levelNameLength = header.tilesetOffset - header.nameOffset;
            char *levelNameBuffer = new char[levelNameLength];
            levelIn.read(levelNameBuffer, levelNameLength);
            levelName = std::string(levelNameBuffer, levelNameLength);

            //load tilesets
            TileManager::clear_layers();
            std::vector<TilesetAndGid> tilesets;
            for (u32 i = 0; i < header.tilesetAmount; i++)
            {
                TilesetInfo tilesetInfo;
                levelIn.read((char*)&tilesetInfo, sizeof(tilesetInfo));

                //set first gid for tileset
                TilesetAndGid tileset;
                tileset.gid = tilesetInfo.firstGid;

                //get filename for tileset
                u32 tilesetFilenameLength = tilesetInfo.onePastFilenameOffset - tilesetInfo.filenameOffset;
                char *tilesetFilenameBuffer = new char[tilesetFilenameLength];
                levelIn.read(tilesetFilenameBuffer, tilesetFilenameLength);
                std::string tilesetFilename(tilesetFilenameBuffer, tilesetFilenameLength);
                //get pointer to tileset
                tileset.tileset = Resource::get_tileset(tilesetFilename);
                if (!tileset.tileset)
                    throw std::runtime_error("Tileset " + tilesetFilename + " not found!");

                tilesets.push_back(tileset);

                delete[] tilesetFilenameBuffer;
            }

            //load tileLayers
            for (u32 i = 0; i < header.tileLayerAmount; i++)
            {
                TileLayerInfo layerInfo;
                levelIn.read((char*)&layerInfo, sizeof(layerInfo));

                u32 *tiles = new u32[layerInfo.tileAmount];
                levelIn.read((char*)tiles, layerInfo.tileAmount * sizeof(u32));

                TileManager::create_layer(tilesets, layerInfo, tiles);

                delete[] tiles;
            }

            //load entities
            for (u32 i = 0; i < header.entityAmount; i++)
            {
                EntityInfo entity;
                levelIn.read((char*)&entity, sizeof(entity));

                //read entity name
                u32 entityNameLength = entity.typeOffset - entity.nameOffset;
                char *entityNameBuffer = new char[entityNameLength];
                levelIn.read(entityNameBuffer, entityNameLength);
                std::string entityName(entityNameBuffer, entityNameLength);

                //read entity type
                u32 entityTypeLength = entity.propertyOffset - entity.typeOffset;
                char *entityTypeBuffer = new char[entityTypeLength];
                levelIn.read(entityTypeBuffer, entityTypeLength);
                std::string entityType(entityTypeBuffer, entityTypeLength);

                //read properties
                std::vector<EntityProperty> entityProperties;

                for (u32 j = 0; j < entity.propertyAmount; j++)
                {
                    EntityPropertyInfo propertyInfo;
                    levelIn.read((char*)&propertyInfo, sizeof(propertyInfo));

                    EntityProperty property;

                    //read property name
                    u32 propertyNameLength = propertyInfo.valueOffset - propertyInfo.nameOffset;
                    char *propertyNameBuffer = new char[propertyNameLength];
                    levelIn.read(propertyNameBuffer, propertyNameLength);
                    property.name = std::string(propertyNameBuffer, propertyNameLength);

                    //read property value
                    u32 propertyValueLength = propertyInfo.onePastValueOffset - propertyInfo.valueOffset;
                    char *propertyValueBuffer = new char[propertyValueLength];
                    levelIn.read(propertyValueBuffer, propertyValueLength);
                    property.value = std::string(propertyValueBuffer, propertyValueLength);

                    entityProperties.push_back(property);

                    delete[] propertyNameBuffer;
                    delete[] propertyValueBuffer;
                }

                EntityManager::create_some_entity(entity.x, entity.y, entity.w, entity.h, entity.depth, entityName, entityType, entityProperties);

                delete[] entityNameBuffer;
                delete[] entityTypeBuffer;
            }

            result = true;

            delete[] levelNameBuffer;
        }
        levelIn.close();
    }

    loaded = result;

    if (loaded)
    {
        state = LEVEL_START;
        transitionRadius = 0;
    }
    else state = LEVEL_ERROR;

    return result;*/
}

namespace CurrentLevel
{
    Level level;
    std::string fname;
}

void CurrentLevel::set_fname(std::string f)
{
    fname = f;
}

void CurrentLevel::load_level(std::atomic<s32> *done)
{
    try
    {
        level.load_level(fname);
    }
    catch (std::runtime_error error)
    {
        platform_show_error(error.what());
        //loading failed!
        *done = -1;
        return;
    }

    //loading done!
    *done = 1;
}
void CurrentLevel::end_level(bool reload)
{
    level.end_level(reload);
}
bool CurrentLevel::level_loaded()
{
    return level.is_loaded();
}
void CurrentLevel::clear_entities_and_tiles()
{
    //TileManager::clear_layers();
    World::unload_map();
    EntityManager::deinit_all();
    platform_stop_world_sounds();
    //SoundManager::clear_queue();
}
bool CurrentLevel::update_level()
{
    if (!level_loaded())
        return false;

    level.update();
    return true;
}
bool CurrentLevel::draw_level(r32 timeRatio)
{
    if (!level_loaded())
        return false;

    level.draw(timeRatio);
    return true;
}
u32 CurrentLevel::get_width()
{
    return level.width;
}
u32 CurrentLevel::get_height()
{
    return level.height;
}

LevelState CurrentLevel::get_state()
{
    return level.get_state();
}

void CurrentLevel::unmute_drums()
{
    platform_mute_channel(level.get_drum_channel(), false);
    platform_fade_channel(level.get_drum_channel(), 0.0, 1.0, 1);
    level.drumsMuted = false;
}
void CurrentLevel::mute_drums()
{
    //platform_mute_channel(level.get_drum_channel(), true);
    platform_fade_channel(level.get_drum_channel(), 1.0, 0.0, 1);
    level.drumsMuted = true;
}
