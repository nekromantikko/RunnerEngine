#ifndef ENTITY_H
#define ENTITY_H
#include "player.h"
#include "laser.h"
#include "switch.h"
#include "hudcomponent.h"
#include "light.h"
#include "particles.h"

//////////////////////////////////////////////
struct EntityProperty
{
    std::string name;
    std::string value;
};
/*
struct EntityFlag
{
    u32 x;
    u32 y;

    std::string name;
    std::string type;
    std::vector<EntityProperty> properties;
};*/

//this is a fun little struct with every possible entity component in it
struct AllEntityComponents
{
    SpriteComponent sprite;
    ModelComponent model;
    MirrorComponent mirror;
    AABBCollider hitbox;
    SATRectCollider satrect;
    CircleCollider circle;
    Player player;
    Follower follower;
    Laser laserbeam;
    Switch swtch;
    CameraComponent camera;
    RotatingTrack rotTrack;
    PlatformController platformController;
    PlayerHUD pHud;
    LightComponent light;
    HealthPickupComponent healthPickup;
    ParticleSystem particleSystem;
    SlidingDoorComponent slidingDoor;
};

class Entity
{
public:
    void update();
    void update_render_component();
    void init();
    void deinit();

    std::string name = "";
    bool32 persistent = false;
    AllEntityComponents components;

    Transform transform;
    fvec2 velocity;
    r32 depth;
    ColliderComponent *collider = NULL;
    RenderComponent *render = NULL;

    void add_event(EntityEvent e);
    void add_behaviour(BehaviourComponent *b);
    u32 play_sound(std::string fname, s32 loops);
private:
    std::vector<BehaviourComponent*> behaviours;
    std::vector<EntityEvent> eventBuffer;
};

#endif // ENTITY_H
