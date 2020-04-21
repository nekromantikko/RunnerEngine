#include "playerstate.h"
#include "player.h"
#include "units.h"
#include "resource.h"
#include "entity.h"

//BASIC MOVEMENT
const f32 PlayerMovementSuperState::maxWalkSpd = 4, PlayerMovementSuperState::maxDashSpd = 10, PlayerMovementSuperState::acceleration = 0.1875, PlayerMovementSuperState::deceleration = 0.25;

void PlayerMovementSuperState::slow_down(Player &player)
{
    if (player.hSpeed > 0)
    {
        if (player.hSpeed < acceleration)
            player.hSpeed = 0;
        else player.hSpeed -= acceleration;
    }
    else
    {
        if (player.hSpeed > -acceleration)
            player.hSpeed = 0;
        else player.hSpeed += acceleration;
    }
}

void PlayerMovementSuperState::brake(Player &player)
{
    if (player.hSpeed < 0)
    {
        player.direction = RIGHT;
        if (player.hSpeed > -deceleration)
            player.hSpeed = 0;
        else player.hSpeed += deceleration;
    }
    else
    {
        player.direction = LEFT;
        if (player.hSpeed < deceleration)
            player.hSpeed = 0;
        else player.hSpeed -= deceleration;
    }
}

//GROUNDED

void PlayerGrounded::jump(Player &player)
{
    //set jumpforce based on hSpeed
    f32 jumpForce = 8.5 + absolute_value(player.hSpeed)/7.5;

    //play sound
    player.parent->play_sound("player_jump", 0);
    player.vSpeed = -jumpForce;
    player.hSpeed = player.hSpeed;
    player.jumpLeg = player.jumpLeg;
}

//STAND
PlayerStand::PlayerStand() : fallTimer(std::bind(&PlayerStand::fall, this))
{

}

PlayerState *PlayerStand::update(Player &player)
{
    //get input
    r64 xAxis = player.input.xAxis;
    r64 yAxis = player.input.yAxis;
    bool jumpPressed = player.input.keyJumpPressed;

    //update fallTimer
    fallTimer.update();
    //set fallTimer if no bottom collision is found
    if (!player.collision.bottom)
    {
        if (!fallTimer.is_running())
            fallTimer = 3;
    }
    else fallTimer.stop_timer();

    //set direction
    if (xAxis)
    {
        if (xAxis > 0)
            player.direction = RIGHT;
        else player.direction = LEFT;
    }

    //reset pushing
    pushing = false;

    //move

    //if not touching the stick, but still moving, slow down
    if (xAxis == 0)
    {
        if (player.hSpeed != 0)
            slow_down(player);
    }
    else
    {
        //if walking against a wall, push:
        if ( (xAxis > 0 && player.collision.right) || (xAxis < 0 && player.collision.left) )
        {
            pushing = true;
        }
        else
        {
            //if facing backwards, brake
            if (player.hSpeed != 0 && player.hSpeed / absolute_value(player.hSpeed) != player.direction)
                brake(player);

            //otherwise, run
            else move(player);
        }

    }

    //change states
    //jump if pressing jump
    if (jumpPressed)
    {
        jump(player);
        player.inAir = true;

        //reset the jumpPressed-timer to avoid doing a walljump immediately after jumping next to a wall
        player.jumpPressedTimer.stop_timer();

        return new PlayerAir;
    }

    //fall if timer has run out
    if (falling)
    {
        player.inAir = true;
        return new PlayerAir;
    }

    //duck if pressing down
    if (yAxis < 0 || (player.collision.top && player.collision.bottom))
    {
        player.duck = true;
        return new PlayerDuck;
    }

    return this;
}

void PlayerStand::move(Player &player)
{
    bool keyDashHeld = player.input.keyDashHeld;

    f32 maxSpeed;
    //set maximum velocity based on whether the dash key is being held
    if (keyDashHeld)
        maxSpeed = maxDashSpd;
    else maxSpeed = maxWalkSpd;

    if (player.direction == RIGHT)
    {
        if (player.collision.bottomTileAngle < 0)
            player.hSpeed += 0.125*sin(player.collision.bottomTileAngle);

        if (player.hSpeed < maxSpeed)
            player.hSpeed += acceleration;
        else if (!keyDashHeld && player.collision.bottomTileAngle == 0)
            player.hSpeed -= acceleration;
    }
    else
    {
        if (player.collision.bottomTileAngle > 0)
            player.hSpeed -= 0.125*sin(player.collision.bottomTileAngle);

        if (player.hSpeed > -maxSpeed)
            player.hSpeed -= acceleration;
        else if (!keyDashHeld && player.collision.bottomTileAngle == 0)
            player.hSpeed += acceleration;
    }
}

void PlayerStand::setup_sprite(Player &player, SpriteComponent *sprite)
{
    sprite->animSpeed = (absolute_value(player.hSpeed) / 22.5);

    if (player.hSpeed == 0)
    {
        if (pushing)
            sprite->set_animation(8);
        else sprite->set_animation(0);
    }
    else
    {
        //if trying to move against the movement speed, brake
        if (player.hSpeed / absolute_value(player.hSpeed) != player.direction)
            sprite->set_animation(5);
        //normal running animation
        else sprite->set_animation(4);
    }
}

void PlayerStand::fall()
{
    falling = true;
}

////////////////////////////////////////////////////////////////////////
//DUCK
PlayerState *PlayerDuck::update(Player &player)
{
    //get input
    r64 xAxis = player.input.xAxis;
    r64 yAxis = player.input.yAxis;
    bool jumpPressed = player.input.keyJumpPressed;

    //set direction
    if (xAxis)
    {
        if (xAxis > 0)
            player.direction = RIGHT;
        else player.direction = LEFT;
    }

    //move

    //if not touching the stick, but still moving, slow down
    if (xAxis == 0)
    {
        if (player.hSpeed != 0)
            slow_down(player);
    }
    else move(player);

    //change states
    //jump if pressing jump
    if (jumpPressed && !player.collision.top)
    {
        jump(player);
        player.inAir = true;
        player.duck = false;
        return new PlayerAir;
    }

    //fall if no bottom collision
    if (!player.collision.bottom)
    {
        player.inAir = true;
        player.duck = false;
        return new PlayerAir;
    }

    //stand if NOT pressing down
    if (yAxis >= 0 && !player.collision.top)
    {
        player.duck = false;
        return new PlayerStand;
    }

    return this;
}

void PlayerDuck::move(Player &player)
{
    bool keyDashHeld = player.input.keyDashHeld;

    f32 maxSpeed;
    //set maximum velocity based on whether the dash key is being held
    if (keyDashHeld)
        maxSpeed = maxDashSpd / 4;
    else maxSpeed = maxWalkSpd / 4;

    if (player.direction == RIGHT)
    {
        if (player.hSpeed < maxSpeed)
            player.hSpeed += acceleration;
        else player.hSpeed -= fixed_min(player.hSpeed - maxSpeed, acceleration);
    }
    else
    {
        if (player.hSpeed > -maxSpeed)
            player.hSpeed -= acceleration;
        else player.hSpeed -= fixed_max(maxSpeed + player.hSpeed, -acceleration);
    }
}

void PlayerDuck::setup_sprite(Player &player, SpriteComponent *sprite)
{
    sprite->animSpeed = (absolute_value(player.hSpeed) / 22.5);

    if (player.hSpeed == 0)
        sprite->set_animation(7);
    else sprite->set_animation(11);
}

///////////////////////////////////////////////////////////
//AIR
const f32 PlayerAir::maxFallSpd = 13.3125, PlayerAir::gravity = 0;//0.3125;

PlayerState *PlayerAir::update(Player &player)
{
    //get input
    r64 xAxis = player.input.xAxis;
    bool jumpReleased= player.input.keyJumpReleased;

    //set direction
    if (xAxis)
    {
        if (xAxis > 0)
            player.direction = RIGHT;
        else player.direction = LEFT;
    }

    //apply gravity
    player.vSpeed += gravity;

    if (player.vSpeed > maxFallSpd)
        player.vSpeed = maxFallSpd;

    //half the jump speed if the button is released early
    if (jumpReleased && player.vSpeed < 0)
        player.vSpeed /=2;

    //move
    if (xAxis != 0)
    {
        //if facing backwards, brake
        if (player.hSpeed != 0 && player.hSpeed / absolute_value(player.hSpeed) != player.direction)
            brake(player);

        //otherwise, run
        else move(player);
    }

    //change states

    //if there's a ledge
    if (player.collision.hang)
        return new PlayerHang;

    //if bottom is colliding, stand
    if (player.collision.bottom)
    {
        player.inAir = false;
        return new PlayerStand;
    }

    //if there's a wall, get on it (if possible)
    if (player.collision.right && (xAxis > 0 || player.hSpeedTemp > 0))
    {
        player.direction = LEFT;
        player.hSpeed = 0;
        return new PlayerOnWall;
    }
    if (player.collision.left && (xAxis < 0 || player.hSpeedTemp < 0))
    {
        player.direction = RIGHT;
        player.hSpeed = 0;
        return new PlayerOnWall;
    }

    return this;
}

void PlayerAir::move(Player &player)
{
    if (player.direction == RIGHT)
    {
        if (player.hSpeed < maxWalkSpd)
            player.hSpeed += acceleration;
    }
    else
    {
        if (player.hSpeed > -maxWalkSpd)
            player.hSpeed -= acceleration;
    }
}

void PlayerAir::setup_sprite(Player &player, SpriteComponent *sprite)
{
    //animation speed is zero when in air
    sprite->animSpeed = 0;
    //instead, set frame based on vSpeed
    if (std::round((r32)player.vSpeed) < -3)
        sprite->set_frame(0);
    else if (std::round((r32)player.vSpeed) > 3)
        sprite->set_frame(6);
    else sprite->set_frame(std::round((r32)player.vSpeed) + 3);

    if (player.jumpLeg)
        sprite->set_animation(3);//instead, set frame based on vSpeed
    else sprite->set_animation(9);
}

/////////////////////////////////////////////////////////////////////////////
//ON WALL
PlayerOnWall::PlayerOnWall() : releaseTimer(std::bind(&PlayerOnWall::release, this))
{

}

PlayerState *PlayerOnWall::update(Player &player)
{
    //get input
    r64 xAxis = player.input.xAxis;
    bool jumpReleased = player.input.keyJumpReleased;
    bool jumpPressed = player.input.keyJumpPressed;

    //update timer
    if ((xAxis > 0 && player.direction == RIGHT) || (xAxis < 0 && player.direction == LEFT))
    {
        if (!releaseTimer.is_running())
            releaseTimer = 15;
        else releaseTimer.update();
    }

    //apply gravity
    player.vSpeed += gravity;

    if (player.vSpeed > maxFallSpd)
        player.vSpeed = maxFallSpd;

    //half the jump speed if the button is released early
    if (jumpReleased && player.vSpeed < 0)
        player.vSpeed /=2;

    //change states
    //if there's a ledge
    if (player.collision.hang)
    {
        return new PlayerHang;
    }

    //if bottom is colliding, stand
    if (player.collision.bottom)
    {
        player.inAir = false;
        return new PlayerStand;
    }
    //cancel
    if (released || (!player.collision.right && player.direction == LEFT) || (!player.collision.left && player.direction == RIGHT))
    {
        player.hSpeedTemp = 0;
        return new PlayerAir;
    }
    //do a walljump
    if (jumpPressed)
    {
        walljump(player);
        return new PlayerWallJump;
    }

    return this;
}

void PlayerOnWall::setup_sprite(Player &player, SpriteComponent *sprite)
{
    //animation speed is zero when in air
    sprite->animSpeed = 0;
    sprite->set_animation(2);
}

void PlayerOnWall::walljump(Player &player)
{
    //play sound
    player.parent->play_sound("player_jump", 0);

    r32 sqrt3per2 = 0.87;
    f32 vel = fixed_abs(player.hSpeedTemp) / 1.6 + 5.3125;
    player.hSpeed = (vel / 2) * player.direction;
    player.vSpeed = -vel * sqrt3per2;

    player.tempSpeedTimer = 60;
    player.jumpLeg = !player.jumpLeg;
}

void PlayerOnWall::release()
{
    released = true;
}

/////////////////////////////////////////////////////////////////////////////
//WALLJUMP
PlayerWallJump::PlayerWallJump() : walljumpTimer(std::bind(&PlayerWallJump::end_walljump, this))
{

}

PlayerState *PlayerWallJump::update(Player &player)
{
    //get input
    bool jumpReleased= player.input.keyJumpReleased;

    //update timer
    if (!walljumpTimer.is_running())
        walljumpTimer = 20;
    else walljumpTimer.update();

    //apply gravity
    player.vSpeed += gravity;

    if (player.vSpeed > maxFallSpd)
        player.vSpeed = maxFallSpd;

    //half the jump speed if the button is released early
    if (jumpReleased && player.vSpeed < 0)
        player.vSpeed /=2;

    //change states
    //if there's a wall, get on it
    if (player.collision.right)
    {
        player.direction = LEFT;
        player.hSpeed = 0;
        return new PlayerOnWall;
    }
    if (player.collision.left)
    {
        player.direction = RIGHT;
        player.hSpeed = 0;
        return new PlayerOnWall;
    }
    //if bottom is colliding, stand
    if (player.collision.bottom)
    {
        player.inAir = false;
        return new PlayerStand;
    }
    if (endJump)
    {
        //player.hSpeedTemp = 0;
        return new PlayerAir;
    }

    return this;
}

void PlayerWallJump::end_walljump()
{
    endJump = true;
}

///////////////////////////////////////////////////////////
//HURT:
PlayerHurt::PlayerHurt()
{
    hurtTimer = 20;
}
PlayerState *PlayerHurt::update(Player &player)
{
    //apply gravity
    player.vSpeed += gravity;

    if (player.vSpeed > maxFallSpd)
        player.vSpeed = maxFallSpd;

    hurtTimer.update();

    if (!hurtTimer.is_running())
        return new PlayerAir;
    else return this;
}

///////////////////////////////////////////////////////////
//DEAD:
PlayerDead::PlayerDead()
{
}
PlayerState *PlayerDead::update(Player &player)
{
    player.hSpeed = 0;

    //apply gravity
    player.vSpeed += gravity;

    if (player.vSpeed > maxFallSpd)
        player.vSpeed = maxFallSpd;

    return this;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//HANG:
PlayerState *PlayerHang::update(Player &player)
{
    player.vSpeed = 0;
    //get input
    r64 yAxis = player.input.yAxis;
    r64 xAxis = player.input.xAxis;
    bool jumpPressed = player.input.keyJumpPressed;

    //change state
    if (jumpPressed)
    {
        //release grip
        if (yAxis < 0)
        {
            player.grabTimer = 5;
            player.jumpPressedTimer.stop_timer();
            return new PlayerAir;
        }
        //walljump
        else if (xAxis > 0 && player.direction == LEFT)
        {
            player.direction = RIGHT;
            walljump(player);
            return new PlayerWallJump;
        }
        else if (xAxis < 0 && player.direction == RIGHT)
        {
            player.direction = LEFT;
            walljump(player);
            return new PlayerWallJump;
        }
        //climb
        else
        {
            player.collisionDisabled = true;
            player.hSpeed = 2 * player.direction;
            player.vSpeed = -4;
            return new PlayerClimb;
        }
    }

    return this;
}

void PlayerHang::setup_sprite(Player &player, SpriteComponent *sprite)
{
    sprite->set_animation(10);
}

void PlayerHang::walljump(Player &player)
{
    //play sound
    player.parent->play_sound("player_jump", 0);

    r32 sqrt3per2 = 0.87;
    player.hSpeed = 5.3125 / 2 * player.direction;
    player.vSpeed = -5.3125 * sqrt3per2;

    player.jumpLeg = !player.jumpLeg;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//CLIMB:
PlayerClimb::PlayerClimb() : climbTimer(std::bind(&PlayerClimb::end_climb, this))
{
    climbTimer = 15;
}

PlayerState *PlayerClimb::update(Player &player)
{
    climbTimer.update();

    if (climbTimer == 8)
        player.duck = true;

    if (endClimb)
    {
        player.collisionDisabled = false;
        player.inAir = false;
        player.hSpeed = 0;
        player.vSpeed = 0;
        return new PlayerDuck;
    }

    return this;
}

void PlayerClimb::setup_sprite(Player &player, SpriteComponent *sprite)
{
    sprite->set_animation(12);
    sprite->set_frame((15 - climbTimer.get_time()) / 2);
}

void PlayerClimb::end_climb()
{
    endClimb = true;
}
