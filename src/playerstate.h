#ifndef STATE_H
#define STATE_H

#include "timer.h"

struct PlayerData;
class Player;
class SpriteComponent;

enum PlayerDirection
{
    LEFT = -1,
    RIGHT = 1
};

struct PlayerState
{
    virtual PlayerState *update(Player &player) = 0;
    virtual ~PlayerState() {}
    virtual void setup_sprite(Player &player, SpriteComponent *sprite) = 0;
};

struct PlayerMovementSuperState : PlayerState
{
    virtual void move(Player &player) = 0;
    inline void slow_down(Player &player);
    inline void brake(Player &player);

    static const f32 maxWalkSpd, maxDashSpd, acceleration, deceleration;
};

//SUPER STATES:

//grounded parent state
struct PlayerGrounded : PlayerMovementSuperState
{
    inline void jump(Player &player);
};

//air state
struct PlayerAir : PlayerMovementSuperState
{
    PlayerState *update(Player &player);

    virtual inline void setup_sprite(Player &player, SpriteComponent *sprite);
    inline void move(Player &player);

    static const f32 maxFallSpd, gravity;
};

//SUB/REGULAR STATES:

//standing state
struct PlayerStand : PlayerGrounded
{
    PlayerStand();
    PlayerState *update(Player &player);

    inline void setup_sprite(Player &player, SpriteComponent *sprite);
    inline void move(Player &player);

    void fall();
    Timer fallTimer;
    bool falling = false;
    bool pushing = false;
};

//ducking/crawling state
struct PlayerDuck : PlayerGrounded
{
    PlayerState *update(Player &player);

    inline void setup_sprite(Player &player, SpriteComponent *sprite);
    inline void move(Player &player);
};

//on wall
struct PlayerOnWall : PlayerAir
{
    PlayerOnWall();
    PlayerState *update(Player &player);

    inline void setup_sprite(Player &player, SpriteComponent *sprite);
    inline void walljump(Player &player);
    void release();

    Timer releaseTimer;
    bool released = false;
};

//walljump
struct PlayerWallJump : PlayerAir
{
    PlayerWallJump();
    PlayerState *update(Player &player);
    void end_walljump();

    Timer walljumpTimer;
    bool endJump = false;
};

//hurt
struct PlayerHurt : PlayerAir
{
    PlayerHurt();
    PlayerState *update(Player &player);
    Timer hurtTimer;
};

struct PlayerDead : PlayerAir
{
    PlayerDead();
    PlayerState *update(Player &player);
};

//hang
struct PlayerHang : PlayerState
{
    PlayerState *update(Player &player);
    inline void setup_sprite(Player &player, SpriteComponent *sprite);
    inline void walljump(Player &player);
};

//climb
struct PlayerClimb : PlayerState
{
    PlayerClimb();
    PlayerState *update(Player &player);
    inline void setup_sprite(Player &player, SpriteComponent *sprite);
    void end_climb();

    Timer climbTimer;
    bool endClimb = false;
};


#endif // STATE_H
