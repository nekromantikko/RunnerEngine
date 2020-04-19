#ifndef PLAYER_H
#define PLAYER_H
#include "playerstate.h"
#include "component.h"

#include <memory>

class Tile;

struct PlayerCollision
{
    bool32 bottom = 0;
    bool32 top = 0;
    bool32 right = 0;
    bool32 left = 0;
    //true if colliding with a ledge in such a way that
    //if hanging should be forced
    bool32 hang = 0;

    //angle of the tile we're standing on
    r32 bottomTileAngle = 0, topTileAngle = 0;
    void reset()
    {
        bottom = 0;
        top = 0;
        right = 0;
        left = 0;
        hang = 0;
        ////
        bottomTileAngle = 0;
        topTileAngle = 0;
    }
};

struct PlayerInput
{
    //input switches
    r64 xAxis = 0;
    r64 yAxis = 0;
    bool32 keyJumpPressed = false;
    bool32 keyJumpReleased = false;
    bool32 keyJumpHeld = false;
    bool32 keyDashHeld = false;
    bool32 keyShootPressed = false;
};

class Player final : public BehaviourComponent
{
public:
    Player();
    void update();
    void process_event(EntityEvent e);
    void init();
    void deinit();
    void reset_temp_speed();
    void handle_dynamic_music();

    //data
    f32 hSpeed;
    f32 vSpeed;
    f32 hSpeedTemp;
    u32 health;
    //what direction the character is facing (1 = right, -1 = left)
    PlayerDirection direction = RIGHT;
    //self-explanatory
    PlayerCollision collision;
    //inputs
    PlayerInput input;
    //switches
    bool32 jumpLeg;
    bool32 inAir;
    bool32 duck;
    bool32 collisionDisabled;
    bool32 invincible;
    bool32 dead;
    bool32 playDrums;
    //global timers
    Timer tempSpeedTimer;
    Timer jumpPressedTimer;
    Timer grabTimer;
    Timer dmgInvinciTimer;
    Timer speedThresholdTimer;

    static const f32 speedThreshold;
private:
    PlayerState *state;

    //moving platform the player is stuck to!
    Entity *movingPlatform;

    inline void get_input();
    inline void handle_collisions(AABBCollider *hitbox);
    inline void handle_h_collisions(AABBCollider *hitbox, TileHit &tile);
    inline void handle_v_collisions(AABBCollider *hitbox, TileHit &tile);
    inline void handle_enemy_collision(AABBCollider *hitbox, Entity *entity);
    inline void handle_platform_collision(AABBCollider *hitbox, Entity *entity);
    inline void update_hitbox(AABBCollider *hitbox);
    void take_damage(u32 amount);
    void die();
};

#endif


