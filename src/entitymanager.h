#ifndef COMPONENTMANAGER_H
#define COMPONENTMANAGER_H
#include <queue>
#include <map>
#include "entity.h"

namespace EntityManager
{
    void allocate(uint32_t size);
    Entity *create_entity();
    void deinit_all();
    void deinit_entity(Entity *entity);
    void update_entities(bool step);
    void kill(Entity *e);
    Entity *find_entity(std::string name);
    Pool<Entity> &get_entities();

    //entityfactory
    Entity *create_some_entity(s32 x, s32 y, s32 w, s32 h, r32 depth, std::string name, std::string type, std::vector<EntityProperty> properties);
    std::string find_property(std::vector<EntityProperty> properties, std::string name);
    Entity *create_player(std::string name, s32 x, s32 y, r32 depth);
    Entity *create_camera(std::string name, std::string targetName);
    //HAZARDS
    Entity *create_laser_beam(std::string name, s32 x, s32 y, r32 depth, bool state, bool type, r32 angle);
    //SWITCHES
    Entity *create_static_player_switch(std::string name, s32 x, s32 y, r32 depth, bool state, u32 time, std::string target);
    //ENEMIES
    Entity *create_static_enemy(std::string name, s32 x, s32 y, r32 depth);
    //POWERUPS
    Entity *create_angel();
    Entity *create_angel_shield(u32 angelCount, std::string targetName);
    //COLLISION_WALLS
    Entity *create_2x1_platform(std::string name, s32 x, s32 y, r32 depth);
    Entity *create_rotating_platforms(std::string name, s32 x, s32 y, r32 depth, u32 count, u32 radius, r32 speed);
    //LIGHTS
    Entity *create_static_light(std::string name, s32 x, s32 y, r32 depth, v4 color);
    //PICKUPS
    Entity *create_medkit(std::string name, s32 x, s32 y, r32 depth);
    //SCENERY
    Entity *create_static_prop(std::string name, s32 x, s32 y, r32 depth, Model *model, v3 rotation);
    Entity *create_reflective_surface(std::string name, s32 x, s32 y, s32 h, s32 w);
    //DOORS
    Entity *create_sliding_1x4_door(std::string name, s32 x, s32 y, r32 depth, r32 openTime, bool32 state, bool32 dir);
}

#endif // COMPONENTMANAGER_H
