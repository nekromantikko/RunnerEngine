#ifndef COMPONENT_H
#define COMPONENT_H
#include "collision.h"
#include "sprite.h"

enum RenderComponentType
{
    RENDER_SPRITE,
    RENDER_MODEL,
    RENDER_MIRROR
};

enum ColliderComponentType
{
    COLLIDER_AABB,
    COLLIDER_SATRECT,
    COLLIDER_CIRCLE
};

//rendercomponents
class RenderComponent
{
public:
    RenderComponent(RenderComponentType type);
    const RenderComponentType type;
    virtual void update(Transform &transform, r32 depth) = 0;

    Transform previous;
    bool visible = true;
    v4 color;
};

class SpriteComponent : public RenderComponent
{
public:
    SpriteComponent();
    void init(const char *spriteName, u32 anim=0, r32 speed=0, v4 c={1,1,1,1});
    void update(Transform &transform, r32 depth);
    void set_animation (u32 animIndex);
    void set_frame(u32 frame);

    SpriteInstance sInstance;
    r32 animSpeed;
    bool32 hFlip = false, vFlip = false;
};

class Model;

class ModelComponent : public RenderComponent
{
public:
    ModelComponent();
    void update(Transform &transform, r32 depth);

    Model *model;
};

class MirrorComponent : public RenderComponent
{
public:
    MirrorComponent();
    void update(Transform &transform, r32 depth);
    f32 width, height;
};

//collision components
class ColliderComponent
{
public:
    ColliderComponent(ColliderComponentType type);
    virtual void draw(Transform &transform) = 0;
    const ColliderComponentType type;
    static bool32 visible;
    bool32 state = true;
    CollisionFlag flags;
};

class AABBCollider : public ColliderComponent
{
public:
    AABBCollider();
    void draw(Transform &transform);
    f32 x1, y1, x2, y2;
};

class SATRectCollider : public ColliderComponent
{
public:
    SATRectCollider();
    void draw(Transform &transform);
    fvec2 corners[4];
};

class CircleCollider : public ColliderComponent
{
public:
    CircleCollider();
    void draw(Transform &transform);
    u32 radius;
};

enum EntityEventType
{
    TURN_OFF = 0,
    TURN_ON = 1,
    CLOSE = 0,
    OPEN = 1,
    TAKE_DAMAGE,
    DIE,
    STICK,
    UNSTICK,
    HEAL
};

struct EntityEvent
{
    EntityEventType type;
    u64 value;
};

//behaviour components
class BehaviourComponent
{
public:
    virtual void update() = 0;
    virtual void process_event(EntityEvent e) = 0;
    virtual void init() = 0;
    virtual void deinit() = 0;

    Entity *parent;
};

class Entity;

class Follower final : public BehaviourComponent
{
public:
    void update();
    void process_event(EntityEvent e);
    void init();
    void deinit();

    std::string targetName;
private:
    Entity *target = NULL;
};

class CameraComponent final : public BehaviourComponent
{
public:
    void update();
    void process_event(EntityEvent e);
    void init();
    void deinit();

    std::string targetName;
private:
    Entity *target = NULL;
};

class RotatingTrack final : public BehaviourComponent
{
public:
    void update();
    void process_event(EntityEvent e);
    void init();
    void deinit();
    void add_entity(Entity *entity);
    //radial speed in degrees per frame
    r32 speed;
    u32 radius;
private:
    std::vector<Entity*> targets;
    //angle in degreees
    r32 angle;
};

class PlatformController final : public BehaviourComponent
{
public:
    void update();
    void process_event(EntityEvent e);
    void init();
    void deinit();
private:
    std::vector<Entity*> targets;
    fvec2 previousPos;
    s32 test;
};

class HealthPickupComponent : public BehaviourComponent
{
public:
    void update();
    void process_event(EntityEvent e);
    void init();
    void deinit();

    CollisionFlag collectors;
    u32 healAmount;
};

enum SlidingDoorDirection
{
    UP = 0,
    DOWN = 1
};

enum SlidingDoorState
{
    DOOR_CLOSED,
    DOOR_CLOSING,
    DOOR_OPEN,
    DOOR_OPENING
};

class SlidingDoorComponent : public BehaviourComponent
{
public:
    void update();
    void process_event(EntityEvent e);
    void init();
    void deinit();

    SlidingDoorState state;
    SlidingDoorDirection dir;
    u32 height;
    //time to open door in frames
    r32 openTime;
private:
    r32 accumulator;
};

#endif // COMPONENT_H
