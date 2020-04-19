#include "entitymanager.h"
#include "entity.h"
#include "resource.h"
#include "Platform/platform.h"
#include <sstream>

namespace EntityManager
{
    Pool<Entity> entities;
    std::vector<Entity*> killList;
    Entity *listener;
}

void EntityManager::allocate(uint32_t size)
{
    entities.allocate(size);
}

Entity *EntityManager::create_entity()
{
    return entities.init();
}

//deinit components
void EntityManager::deinit_all()
{
    for (auto it = entities.begin(); it != entities.partition(); it++)
        it->deinit();

    entities.deinit_all();
}

void EntityManager::deinit_entity(Entity *entity)
{
    entity->deinit();
    entities.deinit(entity);
}

void EntityManager::update_entities(bool physics)
{
    //"physics" update
    if (physics)
    {
        for (auto it = entities.begin(); it != entities.partition(); it++)
            it->update();
    }

    //deinit entities that have been queued for death
    for (Entity *e : killList)
        deinit_entity(e);

    killList.clear();

    //now, draw what's left, in correct positions!
    for (auto it = entities.begin(); it != entities.partition(); it++)
        it->update_render_component();

    //platform_set_listener_attributes(&listener->transform.position, NULL);
}

void EntityManager::kill(Entity *e)
{
    killList.push_back(e);
}

Entity *EntityManager::find_entity(std::string name)
{
    Entity *result = NULL;
    for (auto it = entities.begin(); it != entities.partition(); it++)
    {
        Entity &entity = *it;
        if (entity.name.compare(name) == 0)
            result = &entity;
    }
    return result;
}

Pool<Entity> &EntityManager::get_entities()
{
    return entities;
}

/////////////////////////////

Entity *EntityManager::create_some_entity(s32 x, s32 y, s32 w, s32 h, r32 depth, std::string name, std::string type, std::vector<EntityProperty> properties)
{
    Entity *createdEntity = NULL;

    if (type.compare("Player") == 0)
    {
        createdEntity = create_player(name.c_str(), x, y, depth);
    }
    else if (type.compare("Camera") == 0)
    {
        std::string target = find_property(properties, "target");
        createdEntity = create_camera(name.c_str(), target.c_str());
    }
    else if (type.compare("Laser") == 0)
    {
        bool state = atoi(find_property(properties, "state").c_str());
        bool type = atoi(find_property(properties, "type").c_str());
        r32 angle = atof(find_property(properties, "angle").c_str());
        createdEntity = create_laser_beam(name.c_str(), x, y, depth, state, type, angle);
    }
    else if (type.compare("Switch") == 0)
    {
        bool state = atoi(find_property(properties, "state").c_str());
        u32 time = atoi(find_property(properties, "time").c_str());
        std::string target = find_property(properties, "target").c_str();
        createdEntity = create_static_player_switch(name.c_str(), x, y, depth, state, time, target);
    }
    else if (type.compare("StaticEnemy") == 0)
    {
        createdEntity = create_static_enemy(name.c_str(), x, y, depth);
    }
    else if (type.compare("Angel") == 0)
    {
        createdEntity = create_angel();
    }
    else if (type.compare("AngelShield") == 0)
    {
        std::string target = find_property(properties, "target");
        u32 count = atoi(find_property(properties, "count").c_str());
        createdEntity = create_angel_shield(count, target);
    }
    else if (type.compare("2x1Platform") == 0)
    {
        createdEntity = create_2x1_platform(name, x, y, depth);
    }
    else if (type.compare("RotatingPlatforms") == 0)
    {
        r32 speed = atof(find_property(properties, "speed").c_str());
        u32 count = atoi(find_property(properties, "count").c_str());
        u32 radius = atoi(find_property(properties, "radius").c_str());

        createdEntity = create_rotating_platforms(name, x, y, depth, count, radius, speed);
    }
    else if (type.compare("StaticLight") == 0)
    {
        v4 color = {1,1,1,1};
        std::stringstream ss;
        ss << find_property(properties, "color");
        s32 r, g, b;
        ss >> r >> g >> b;

        color.x = r / 255.f;
        color.y = g / 255.f;
        color.z = b / 255.f;
        color.w = atof(find_property(properties, "intensity").c_str());

        createdEntity = create_static_light(name, x, y, depth, color);
    }
    else if (type.compare("Medkit") == 0)
    {
        createdEntity = create_medkit(name, x, y, depth);
    }
    else if (type.compare("StaticProp") == 0)
    {
        std::stringstream ss;
        ss << find_property(properties, "rotation");
        v3 rot = {0,0,0};
        ss >> rot.x >> rot.y >> rot.z;

        std::string mdlName = find_property(properties, "model");
        Model *mdl = Resource::get_model(mdlName);

        createdEntity = create_static_prop(name, x, y, depth, mdl, rot);
    }
    else if (type.compare("SlidingDoor1x4") == 0)
    {
        createdEntity = create_sliding_1x4_door(name, x, y, depth, 30, 0, 0);
    }
    else if (type.compare("ReflectiveSurface") == 0)
    {
        createdEntity = create_reflective_surface(name, x, y, h, w);
    }

    if (createdEntity)
         createdEntity->init();
    return createdEntity;
}

std::string EntityManager::find_property(std::vector<EntityProperty> properties, std::string name)
{
    std::string result;

    for (EntityProperty &property : properties)
    {
        if (property.name.compare(name) == 0)
            result = property.value;
    }
    return result;
}

Entity *EntityManager::create_player(std::string name, s32 x, s32 y, r32 depth)
{
    Entity *newPlayer = create_entity();
    newPlayer->name = name;
    newPlayer->transform.position = {x,y};
    newPlayer->depth = depth;

    SpriteComponent *sprite = &newPlayer->components.sprite;
    sprite->init("spr_player");
    sprite->color = {1,1,1,1};
    newPlayer->render = sprite;

    /*ModelComponent *model = &newPlayer->components.model;
    model->model = Resource::get_model("mdl_spikething");
    newPlayer->render = model;*/

    Player *player = &newPlayer->components.player;
    newPlayer->add_behaviour(player);

    AABBCollider *hitbox = &newPlayer->components.hitbox;
    hitbox->x1 = -14;
    hitbox->y1 = -28;
    hitbox->x2 = 14;
    hitbox->y2 = 32;
    hitbox->flags = COLLISION_PLAYER;
    newPlayer->collider = hitbox;

    /*ParticleSystem *particle = &newPlayer->components.particleSystem;
    particle->prefab = Resource::get_particle_system("debug");
    newPlayer->add_behaviour(particle);*/

    PlayerHUD *hud = &newPlayer->components.pHud;
    hud->health = &player->health;
    newPlayer->add_behaviour(hud);

    listener = newPlayer;

    std::cout << "player <"<< newPlayer << "> created, name " << name << ")\n";
    return newPlayer;
}

Entity *EntityManager::create_camera(std::string name, std::string targetName)
{
    Entity *camera = create_entity();

    CameraComponent *cam = &camera->components.camera;
    cam->targetName = targetName;
    camera->add_behaviour(cam);

    AABBCollider *hitbox = &camera->components.hitbox;
    hitbox->x1 = -48;
    hitbox->y1 = -64;
    hitbox->x2 = 48;
    hitbox->y2 = 64;
    hitbox->flags = COLLISION_NONE;
    camera->collider = hitbox;

    std::cout << "camera <"<< camera << "> created, name " << name << ")\n";
    return camera;
}

Entity *EntityManager::create_laser_beam(std::string name, s32 x, s32 y, r32 depth, bool state, bool type, r32 angle)
{
    Entity *beam = create_entity();
    beam->name = name;
    beam->transform.position = {x,y};
    beam->transform.rotation.z = degrees_to_radians(angle);
    beam->depth = depth;

    Laser *laser = &beam->components.laserbeam;
    if (state)
        laser->state = LASER_ON;
    if (type)
        laser->type = LASER_BLUE;

    beam->add_behaviour(laser);

    SpriteComponent *sprite = &beam->components.sprite;
    sprite->init("spr_laser", 2, 1);
    beam->render = sprite;

    SATRectCollider *hitbox = &beam->components.satrect;
    hitbox->corners[0] = {0, -16};
    hitbox->corners[1] = {32, -16};
    hitbox->corners[2] = {32, 16};
    hitbox->corners[3] = {0, 16};
    hitbox->flags = COLLISION_SPIKES;
    beam->collider = hitbox;

    std::cout << "laser beam <"<< beam << "> created, name " << name << ")\n";
    return beam;
}

Entity *EntityManager::create_static_player_switch(std::string name, s32 x, s32 y, r32 depth, bool state, u32 time, std::string target)
{
    Entity *swtch = create_entity();
    swtch->name = name;
    swtch->transform.position = {x,y};
    swtch->depth = depth;

    Switch *switchComponent = &swtch->components.swtch;
    switchComponent->targetName = target;
    switchComponent->time = time;
    switchComponent->state = state;
    switchComponent->flags = COLLISION_PLAYER;
    swtch->add_behaviour(switchComponent);

    SpriteComponent *sprite = &swtch->components.sprite;
    sprite->init("spr_switch");
    swtch->render = sprite;

    AABBCollider *hitbox = &swtch->components.hitbox;
    hitbox->x1 = -16;
    hitbox->y1 = -16;
    hitbox->x2 = 16;
    hitbox->y2 = 16;
    hitbox->flags = COLLISION_NONE;
    swtch->collider = hitbox;

    std::cout << "static player switch <"<< swtch << "> created, name " << name << ")\n";
    return swtch;
}

//ENEMIES

Entity *EntityManager::create_static_enemy(std::string name, s32 x, s32 y, r32 depth)
{
    Entity *enemy = create_entity();
    enemy->name = name;
    enemy->transform.position = {x,y};
    enemy->depth = depth;

    SpriteComponent *sprite = &enemy->components.sprite;
    sprite->init("spr_player");
    enemy->render = sprite;

    AABBCollider *hitbox = &enemy->components.hitbox;
    hitbox->x1 = -14;
    hitbox->y1 = -28;
    hitbox->x2 = 14;
    hitbox->y2 = 32;
    hitbox->flags = COLLISION_ENEMY;
    enemy->collider = hitbox;

    std::cout << "static enemy <"<< enemy << "> created, name " << name << ")\n";
    return enemy;
}

//POWERUPS

Entity *EntityManager::create_angel()
{
    Entity *entity = create_entity();

    SpriteComponent *sprite = &entity->components.sprite;
    sprite->init("spr_angel");
    entity->render = sprite;

    std::cout << "Angel created\n";

    return entity;
}

Entity *EntityManager::create_angel_shield(u32 angelCount, std::string targetName)
{
    Entity *entity = create_entity();

    Follower *follower = &entity->components.follower;
    follower->targetName = targetName;
    entity->add_behaviour(follower);

    RotatingTrack *track = &entity->components.rotTrack;
    track->speed = 2;
    track->radius = 48;

    for (int i = 0; i < angelCount; i++)
    {
        Entity *angel = create_angel();
        angel->init();
        track->add_entity(angel);
    }

    entity->add_behaviour(track);

    std::cout << "AngelShield created\n";

    return entity;
}

Entity *EntityManager::create_2x1_platform(std::string name, s32 x, s32 y, r32 depth)
{
    Entity *entity = create_entity();
    entity->name = name;
    entity->transform.position = {x,y};
    entity->depth = depth;

    AABBCollider *hitbox = &entity->components.hitbox;
    hitbox->x1 = -32;
    hitbox->y1 = -16;
    hitbox->x2 = 32;
    hitbox->y2 = 16;
    hitbox->flags = COLLISION_WALL;
    entity->collider = hitbox;

    SpriteComponent *sprite = &entity->components.sprite;
    sprite->init("spr_platform");
    entity->render = sprite;

    /*ParticleSystem *particle = &entity->components.particleSystem;
    particle->prefab = Resource::get_particle_system("debug");
    entity->add_behaviour(particle);*/

    PlatformController *controller = &entity->components.platformController;
    entity->add_behaviour(controller);

    std::cout << "2x1 platform <"<< entity << "> created, name " << name << ")\n";
    return entity;
}

Entity *EntityManager::create_rotating_platforms(std::string name, s32 x, s32 y, r32 depth, u32 count, u32 radius, r32 speed)
{
    Entity *entity = create_entity();
    entity->name = name;
    entity->transform.position = {x,y};
    entity->depth = depth;

    RotatingTrack *track = &entity->components.rotTrack;
    track->speed = speed;
    track->radius = radius;

    for (int i = 0; i < count; i++)
    {
        Entity *platform = create_2x1_platform("", 0, 0, depth);
        platform->name = name + "_platform" + std::to_string(i);
        platform->init();
        track->add_entity(platform);
    }
    entity->add_behaviour(track);

    std::cout << "RotatingPlatforms created\n";

    return entity;
}
//LIGHTS
Entity *EntityManager::create_static_light(std::string name, s32 x, s32 y, r32 depth, v4 color)
{
    Entity *entity = create_entity();
    entity->name = name;
    entity->transform.position = {x,y};
    entity->depth = depth;

    LightComponent *light = &entity->components.light;
    light->color = color;
    light->depth = depth;
    light->state = true;

    entity->add_behaviour(light);
    std::cout << "StaticLight created\n";
    return entity;
}

//PICKUPS
Entity *EntityManager::create_medkit(std::string name, s32 x, s32 y, r32 depth)
{
    Entity *entity = create_entity();
    entity->name = name;
    entity->transform.position = {x,y};
    entity->depth = depth;

    ModelComponent *model = &entity->components.model;
    model->model = Resource::get_model("mdl_medkit");
    entity->render = model;

    AABBCollider *hitbox = &entity->components.hitbox;
    hitbox->x1 = -16;
    hitbox->y1 = -16;
    hitbox->x2 = 16;
    hitbox->y2 = 16;
    hitbox->flags = COLLISION_NONE;
    entity->collider = hitbox;

    HealthPickupComponent *hp = &entity->components.healthPickup;
    hp->healAmount = 1;
    hp->collectors = COLLISION_PLAYER;
    entity->add_behaviour(hp);

    return entity;
}

//SCENERY
Entity *EntityManager::create_static_prop(std::string name, s32 x, s32 y, r32 depth, Model *mdl, v3 rotation)
{
    Entity *entity = create_entity();
    entity->name = name;
    entity->transform.position = {x,y};
    entity->transform.rotation = rotation;
    entity->depth = depth;

    ModelComponent *model = &entity->components.model;
    model->model = mdl;
    entity->render = model;

    return entity;
}

Entity *EntityManager::create_reflective_surface(std::string name, s32 x, s32 y, s32 h, s32 w)
{
    Entity *entity = create_entity();
    entity->name = name;
    entity->transform.position = {x,y};

    MirrorComponent *mirror = &entity->components.mirror;
    mirror->width = w;
    mirror->height = h;
    entity->render = mirror;

    return entity;
}

//DOORS
Entity *EntityManager::create_sliding_1x4_door(std::string name, s32 x, s32 y, r32 depth, r32 openTime, bool32 state, bool32 dir)
{
    Entity *entity = create_entity();
    entity->name = name;
    entity->transform.position = {x,y};
    entity->depth = depth;

    SpriteComponent *sprite = &entity->components.sprite;
    sprite->init("spr_door_1x4");
    entity->render = sprite;

    SlidingDoorComponent *door = &entity->components.slidingDoor;
    door->state = DOOR_CLOSED;
    door->dir = UP;
    door->height = 128;
    door->openTime = openTime;
    entity->add_behaviour(door);

    AABBCollider *hitbox = &entity->components.hitbox;
    hitbox->x1 = 0;
    hitbox->y1 = 0;
    hitbox->x2 = 32;
    hitbox->y2 = 128;
    hitbox->flags = COLLISION_WALL;
    entity->collider = hitbox;

    return entity;
}

