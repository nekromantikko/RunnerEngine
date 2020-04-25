#include "player.h"
#include <iostream>
#include <cstdlib>
#include <cmath>
#include "input.h"
#include "level.h"
#include "collision.h"
#include "units.h"
#include "entitymanager.h"
#include "renderer.h"
#include "debug.h"

const f32 Player::speedThreshold = 6;

Player::Player() : tempSpeedTimer(std::bind(&Player::reset_temp_speed, this)), speedThresholdTimer(std::bind(&Player::handle_dynamic_music, this))
{
    //stuff
}

void Player::init()
{
    if (state)
        delete state;
    state = new PlayerStand;

    //init data
    hSpeed = 0;
    vSpeed = 0;
    hSpeedTemp = 0;
    hSpeed = 0;
    health = 3;
    direction = RIGHT;
    jumpLeg = 0;
    inAir = false;
    duck = false;
    collisionDisabled = false;
    invincible = false;
    dead = false;
    playDrums = false;

    dmgInvinciTimer.reset();
    grabTimer.reset();
    jumpPressedTimer.reset();
    tempSpeedTimer.reset();

    movingPlatform = NULL;
}

void Player::deinit()
{
    if (state)
    {
        delete state;
        state = NULL;
    }
}

void Player::reset_temp_speed()
{
    hSpeedTemp = 0;
}

void Player::handle_dynamic_music()
{
    if (playDrums)
        CurrentLevel::unmute_drums();
    else CurrentLevel::mute_drums();
}

void Player::get_input()
{
    TIMED_BLOCK;
    input.xAxis = InputManager::axis(AXIS_RIGHT);
    input.yAxis = InputManager::axis(AXIS_UP);
    //jump pressed
    jumpPressedTimer.update();
    if (InputManager::button_down(JUMP))
        jumpPressedTimer = 3;

    input.keyJumpPressed = (jumpPressedTimer.is_running());
    input.keyJumpReleased = InputManager::button_up(JUMP);
    input.keyJumpHeld = InputManager::button(JUMP);
    input.keyDashHeld = InputManager::button(DASH);
    input.keyShootPressed = InputManager::button_down(SHOOT);


}

void Player::update()
{
    TIMED_BLOCK;

    if (parent->transform.position.y > (CurrentLevel::get_height() * TILE_SIZE) && !dead)
        die();

    //parent->transform.rotation.y += 0.01;

    //get input
    get_input();

    PlayerState *nextState = state->update(*this);

    if (nextState != state)
    {
        delete state;
        state = nextState;
    }

    if (parent->render)
    {
        if (parent->render->type == RENDER_SPRITE)
        {
            SpriteComponent *sprite = (SpriteComponent*)parent->render;
            sprite->hFlip = (direction == LEFT);
            state->setup_sprite(*this, sprite);
        }
    }

    r32 minCos = std::min(cos(collision.bottomTileAngle), cos(collision.topTileAngle));
    f32 hDist = hSpeed * minCos;

    collision.reset();
    tempSpeedTimer.update();
    grabTimer.update();
    dmgInvinciTimer.update();
    speedThresholdTimer.update();
    if (!dmgInvinciTimer.is_running())
    {
        invincible = false;
        parent->render->color.w = 1.0;
    }
    else parent->render->color.w = 0.5;


    //transform->set_rotation(0,0,-floorAngle);
    if (!collisionDisabled && parent->collider->type == COLLIDER_AABB)
    {
        if (movingPlatform)
        {
            movingPlatform->add_event({UNSTICK, (intptr_t)parent});
            movingPlatform = NULL;
        }
        AABBCollider *hitbox = (AABBCollider*)parent->collider;
        update_hitbox(hitbox);
        parent->transform.translate({hDist, 0});
        handle_collisions(hitbox);
    }
    else parent->transform.translate({hDist, vSpeed});

    //dynamic music stuff
    if (absolute_value(hSpeed) >= speedThreshold && !playDrums)
    {
        playDrums = true;
        if (!speedThresholdTimer.is_running())
            speedThresholdTimer = 60;
        else speedThresholdTimer.stop_timer();
    }
    else if (absolute_value(hSpeed) < speedThreshold && playDrums)
    {
        playDrums = false;
        if (!speedThresholdTimer.is_running())
            speedThresholdTimer = 60;
        else speedThresholdTimer.stop_timer();
    }
}

void Player::handle_collisions(AABBCollider *hitbox)
{
    TIMED_BLOCK;
    u32 iterations = 2;

    Transform &xform = parent->transform;

    Rectangle2 rect = {hitbox->x1 + xform.position.x - TILE_SIZE * 2,
                        hitbox->x2 + xform.position.x + TILE_SIZE * 2,
                        hitbox->y1 + xform.position.y - TILE_SIZE * 2,
                        hitbox->y2 + xform.position.y + TILE_SIZE *2};

    std::vector<TileHit> tiles = Collision::box_tile_collision_multiple(rect, ANY);

    for (s32 i = 0; i < iterations; i++)
        for (TileHit &tile : tiles)
            handle_h_collisions(hitbox, tile);

    xform.translate({0, vSpeed});

    for (s32 i = 0; i < iterations; i++)
        for (TileHit &tile : tiles)
            handle_v_collisions(hitbox, tile);

    //Collision::AABB_entity_collision(hitbox, xform, COLLISION_SPIKES);
    /*if (hit.hit)
        std::cout << "entity collision with <" << hit.entity->name << ">!\n";*/

    std::vector<EntityHit> enemies = Collision::rect_entity_collision_multiple(rect, COLLISION_ENEMY);
    for (EntityHit enemy : enemies)
        if (enemy.hit)
            handle_enemy_collision(hitbox, enemy.entity);

    std::vector<EntityHit> platforms = Collision::rect_entity_collision_multiple(rect, COLLISION_WALL);
    for (EntityHit platform : platforms)
        if (platform.hit)
            handle_platform_collision(hitbox, platform.entity);
}

void Player::handle_h_collisions(AABBCollider *hitbox,  TileHit &tile)
{
    //only collide with solids
    TileType tType = (TileType)tile.tile->type;
    if (tType != SOLID && tType != LEDGE)
        return;

    Transform &xform = parent->transform;

    f32 aLeft = hitbox->x1 + xform.position.x;
    f32 aRight = hitbox->x2 + xform.position.x;
    f32 aTop = hitbox->y1 + xform.position.y;
    f32 aBottom = hitbox->y2 + xform.position.y;

    f32 bLeft = tile.pos.x;
    f32 bRight = tile.pos.x + TILE_SIZE;
    f32 bTop = tile.pos.y;
    f32 bBottom = tile.pos.y + TILE_SIZE;

    //check if there's a solid tile one pixel left or right of the player
    if ((aRight < bRight) && (aRight + 1 > bLeft) && (aTop < bBottom) && (aBottom > bTop))
    {
        collision.right = true;
        //record hSpeedTemp
        if (hSpeedTemp == 0 && hSpeed > 0)
        {
            hSpeedTemp = hSpeed;
            tempSpeedTimer = 60;
        }

        if (tType == LEDGE && input.xAxis > 0 && grabTimer.is_stopped())
        {
            //collision.walljump = false;
            if (aTop >= bTop && vSpeed > 0 && vSpeed >= aTop - bTop)
            {
                f32 difference = bTop - aTop;
                xform.position.y += difference;
                aTop += difference;
                aBottom += difference;

                collision.hang = true;
                direction = RIGHT;
                vSpeed = 0;
            }
        }
    }

    else if ((aLeft - 1 < bRight) && (aLeft > bLeft) && (aTop < bBottom) && (aBottom > bTop))
    {
        collision.left = true;
        //record hSpeedTemp
        if (hSpeedTemp == 0 && hSpeed < 0)
        {
            hSpeedTemp = hSpeed;
            tempSpeedTimer = 60;
        }

        if (tType == LEDGE && input.xAxis < 0 && grabTimer.is_stopped())
        {
            //collision.walljump = false;
            if (aTop >= bTop && vSpeed > 0 && vSpeed >= aTop - bTop)
            {
                f32 difference = bTop - aTop;
                xform.position.y += difference;
                aTop += difference;
                aBottom += difference;

                collision.hang = true;
                direction = LEFT;
                vSpeed = 0;
            }
        }
    }

    //If there's no collision, return
    if ((aBottom <= bTop) || (aTop >= bBottom) || (aRight <= bLeft) || (aLeft >= bRight))
        return;

    //but if there is:

    //if the left side of the player's hitbox is inside the wall
    if (aRight >= bRight)
    {
        f32 difference = bRight - aLeft;
        xform.position.x += difference;
        aLeft += difference;
        aRight += difference;

        hSpeed = 0;
    }

    //if the right side of the hitbox is inside the wall
    else
    {
        f32 difference = bLeft - aRight;
        xform.position.x += difference;
        aLeft += difference;
        aRight += difference;

        hSpeed = 0;
    }

}

void Player::handle_v_collisions(AABBCollider *hitbox, TileHit &tile)
{
    TileType tType = (TileType)tile.tile->type;

    Transform &xform = parent->transform;

    f32 aLeft = hitbox->x1 + xform.position.x;
    f32 aRight = hitbox->x2 + xform.position.x;
    f32 aTop = hitbox->y1 + xform.position.y;
    f32 aBottom = hitbox->y2 + xform.position.y;

    f32 bLeft = tile.pos.x;
    f32 bRight = tile.pos.x + TILE_SIZE;
    f32 bTop = tile.pos.y;
    f32 bBottom = tile.pos.y + TILE_SIZE;

    bool top = false;
    bool bottom = false;

    if (tType != SOLID && tType != LEDGE)
    {
        f32 xPos = xform.position.x;
        aLeft = xPos - 1;
        aRight = xPos + 1;

        f32 tileOffset = xPos - bLeft;
        if (tileOffset < 0 || tileOffset >= TILE_SIZE)
            return;

        f32 tileHeight = TileManager::get_tile_height(tile.tile, tileOffset);
        //std::cout << tileHeight << std::endl;
        if (tType == PASS_THROUGH_FLIP)
            bBottom = bTop + tileHeight;
        else bTop = bBottom - tileHeight;

        if (bTop == bBottom)
            return;
    }

    if ((aRight <= bLeft) || (aLeft >= bRight))
        return;

    f32 bTopCheck = bTop;
    //if on the floor, detect collision 16 pixels above the tile
    if (!inAir && !collision.bottom && vSpeed >= 0)
        bTopCheck = bTop - 16;

    //register top collision with the top of the (standing) hitbox
    f32 yPos = xform.position.y;
    if (yPos - 28 >= bTop && yPos - 28 < bBottom && tType != JUMP_THROUGH)
        top = true;

    if (aBottom > bTopCheck && aBottom - vSpeed <= bBottom)
    {
        if (tType  == JUMP_THROUGH && vSpeed < aBottom - bTop)
            return;
        bottom = true;
    }

    //If there's no collision, return
    if ((aBottom <= bTopCheck) || (aTop >= bBottom))
    {
        if (top)
            collision.top = true;

        if (bottom)
            collision.bottom = true;

        return;
    }

    //but if there is:
    //if we're approaching the obstacle from the bottom |<<<----|
    if (bottom)
    {
        //TODO: DO SOMETHING ABOUT THIS HSPEEDTEMP SITUATION
        //MAYBE A TIMER THAT COUNTS DOWN LIKE 4 OR SO FRAMES?
        hSpeedTemp = 0;

        f32 difference = bTop - aBottom;
        xform.position.y += difference;
        aTop += difference;
        aBottom += difference;

        if (vSpeed > 0)
            vSpeed = 0;
        collision.bottomTileAngle = slope_to_angle(tile.tile->slope);
        collision.bottom = true;
    }
    //if from the top |---->>>|
    else if (top)
    {
        f32 difference = bBottom - aTop;
        xform.position.y += difference;
        aTop += difference;
        aBottom += difference;

        if (vSpeed < 0)
            vSpeed = 0;
        collision.topTileAngle = slope_to_angle(tile.tile->slope);
        collision.top = true;
    }

    //take damage if touching spikes
    //if (tile->type() == COLLISION_SPIKES && !invincible)
        //dmgColl = true;

    return;
}

void Player::handle_enemy_collision(AABBCollider *hitbox, Entity *entity)
{
    if (entity->collider->type != COLLIDER_AABB)
        return;
    AABBCollider *enemyHitbox = (AABBCollider*)entity->collider;
    Transform entityXform = entity->transform;

    Transform &xform = parent->transform;

    f32 aLeft = hitbox->x1 + xform.position.x;
    f32 aRight = hitbox->x2 + xform.position.x;
    f32 aTop = hitbox->y1 + xform.position.y;
    f32 aBottom = hitbox->y2 + xform.position.y;

    f32 bLeft = enemyHitbox->x1 + entityXform.position.x;
    f32 bRight = enemyHitbox->x2 + entityXform.position.x;
    f32 bTop = enemyHitbox->y1 + entityXform.position.y;
    f32 bBottom = enemyHitbox->y2 + entityXform.position.y;

    //If there's no collision, return
    if ((aBottom <= bTop) || (aTop >= bBottom) || (aRight <= bLeft) || (aLeft >= bRight))
        return;

    //if approaching from above
    if (vSpeed > 0 && aBottom - bTop <= vSpeed)
    {
        //kill enemy
        entity->add_event({TAKE_DAMAGE, 1});
        parent->play_sound("sfx_kick", 0);
        if (input.keyJumpHeld)
        {
            f32 jumpForce = 9.375 + fixed_abs(hSpeed)/7.5;
            vSpeed = -jumpForce;
        }
        else vSpeed = -6.25;
        //change leg again
        jumpLeg = !jumpLeg;
        return;
    }

    //else just take damage:
    //if (!invincible)
        //dmgColl = true;
}

void Player::handle_platform_collision(AABBCollider *hitbox, Entity *entity)
{
    if (entity->collider->type != COLLIDER_AABB)
        return;
    AABBCollider *enemyHitbox = (AABBCollider*)entity->collider;
    Transform entityXform = entity->transform;

    Transform &xform = parent->transform;

    f32 aLeft = hitbox->x1 + xform.position.x;
    f32 aRight = hitbox->x2 + xform.position.x;
    f32 aTop = hitbox->y1 + xform.position.y;
    f32 aBottom = hitbox->y2 + xform.position.y;

    f32 bLeft = enemyHitbox->x1 + entityXform.position.x;
    f32 bRight = enemyHitbox->x2 + entityXform.position.x;
    f32 bTop = enemyHitbox->y1 + entityXform.position.y;
    f32 bBottom = enemyHitbox->y2 + entityXform.position.y;

    if ((aBottom + 16 <= bTop) || (aTop >= bBottom) || (aRight + 1 <= bLeft) || (aLeft - 1 >= bRight))
    {
        return;
    }

    f32 x1, x2, y1, y2;

    if (aLeft > bLeft)
        x1 = aLeft;
    else x1 = bLeft;

    if (aRight < bRight)
        x2 = aRight;
    else x2 = bRight;

    if (aTop > bTop)
        y1 = aTop;
    else y1 = bTop;

    if (aBottom < bBottom)
        y2 = aBottom;
    else y2 = bBottom;

    if (x2 - x1 < y2 - y1)
    {

        //check if there's a solid tile one pixel left or right of the player
        if ((aRight < bRight) && (aRight + 1 > bLeft) && (aTop < bBottom) && (aBottom > bTop))
        {
            collision.right = true;
            //record hSpeedTemp
            if (hSpeedTemp == 0)
            {
                hSpeedTemp = hSpeed;
                tempSpeedTimer = 60;
            }

            if (input.xAxis > 0 && grabTimer.is_stopped())
            {
                //collision.walljump = false;
                if (aTop >= bTop && vSpeed > 0 && vSpeed >= aTop - bTop)
                {
                    f32 difference = bTop - aTop;
                    xform.position.y += difference;
                    aTop += difference;
                    aBottom += difference;

                    collision.hang = true;
                    direction = RIGHT;
                    vSpeed = 0;

                }
            }
        }

        else if ((aLeft - 1 < bRight) && (aLeft > bLeft) && (aTop < bBottom) && (aBottom > bTop))
        {
            collision.left = true;
            //record hSpeedTemp
            if (hSpeedTemp == 0)
            {
                hSpeedTemp = hSpeed;
                tempSpeedTimer = 60;
            }

            if (input.xAxis < 0 && grabTimer.is_stopped())
            {
                //collision.walljump = false;
                if (aTop >= bTop && vSpeed > 0 && vSpeed >= aTop - bTop)
                {
                    f32 difference = bTop - aTop;
                    xform.position.y += difference;
                    aTop += difference;
                    aBottom += difference;

                    collision.hang = true;
                    direction = LEFT;
                    vSpeed = 0;
                }
            }
        }

        //THIS STICKS THE PLAYER TO WALLS EVEN WHEN NOT DESIRED
        //FIX THIS
        if (inAir)
        {
            entity->add_event({STICK, (intptr_t)parent});
            if (movingPlatform)
                movingPlatform->add_event({UNSTICK, (intptr_t)parent});
            movingPlatform = entity;
        }


        //If there's no collision, return
        if ((aBottom <= bTop) || (aTop >= bBottom) || (aRight <= bLeft) || (aLeft >= bRight))
            return;

        //but if there is:

        //if the left side of the player's hitbox is inside the wall
        if (aRight >= bRight)
        {
            f32 difference = bRight - aLeft;
            xform.position.x += difference;
            aLeft += difference;
            aRight += difference;

            hSpeed = 0;
        }

        //if the right side of the hitbox is inside the wall
        else
        {
            f32 difference = bLeft - aRight;
            xform.position.x += difference;
            aLeft += difference;
            aRight += difference;

            hSpeed = 0;
        }
    }

    //////////////////////////////////////////
    else
    {
        bool top = false;
        bool bottom = false;

        if ((aRight <= bLeft) || (aLeft >= bRight))
        {
            //entity->add_event({REMOVE_PLATFORM_TARGET, (int)this});
            return;
        }

        f32 bTopCheck = bTop;
        //if on the floor, detect collision 16 pixels above the tile
        if (!inAir && !collision.bottom && vSpeed >= 0)
            bTopCheck = bTop - 16;

        //register top collision with the top of the (standing) hitbox
        f32 yPos = xform.position.y;
        if (yPos - 28 >= bTop && yPos - 28 < bBottom)
            top = true;

        if (aBottom > bTopCheck && aBottom - vSpeed <= bBottom)
            bottom = true;

        //If there's no collision, return
        if ((aBottom <= bTopCheck) || (aTop >= bBottom))
        {
            if (top)
                collision.top = true;

            if (bottom)
                collision.bottom = true;

            //entity->add_event({REMOVE_PLATFORM_TARGET, (int)this});
            return;
        }

        //entity->add_event({ADD_COLLISION_WALL_TARGET, (int)this});

        //but if there is:
        //if we're approaching the obstacle from the bottom |<<<----|
        if (bottom)
        {
            entity->add_event({STICK, (intptr_t)parent});
            if (movingPlatform)
                movingPlatform->add_event({UNSTICK, (intptr_t)parent});
            movingPlatform = entity;
            //TODO: DO SOMETHING ABOUT THIS HSPEEDTEMP SITUATION
            //MAYBE A TIMER THAT COUNTS DOWN LIKE 4 OR SO FRAMES?
            hSpeedTemp = 0;

            f32 difference = bTop - aBottom;
            xform.position.y += difference;
            aTop += difference;
            aBottom += difference;

            if (vSpeed > 0)
                vSpeed = 0;
            collision.bottomTileAngle = 0;
            collision.bottom = true;
        }
        //if from the top |---->>>|
        else if (top)
        {
            f32 difference = bBottom - aTop;
            xform.position.y += difference;
            aTop += difference;
            aBottom += difference;

            if (vSpeed < 0)
                vSpeed = 0;
            collision.topTileAngle = 0;
            collision.top = true;
        }
    }
}

void Player::update_hitbox(AABBCollider *hitbox)
{
    if (duck)
        hitbox->y1 = 0;
    else hitbox->y1 = -28;
}

void Player::take_damage(u32 amount)
{
    if (invincible || dead)
        return;

    if (amount > health)
        health = 0;
    else health -= amount;

    if (health == 0)
    {
        parent->play_sound("sfx_hurt", 0);
        platform_controller_rumble(1, 1000);

        die();
    }
    else
    {
        parent->play_sound("sfx_hurt", 0);
        platform_controller_rumble(1, 250);

        if (!collision.top)
        {
            hSpeed = -2.5 * direction;
            vSpeed = -5;
        }
        dmgInvinciTimer = 120;
        invincible = true;

        delete state;
        state = new PlayerHurt();
    }

}

void Player::die()
{
    parent->play_sound("sfx_death", 0);
    dead = true;
    collisionDisabled = true;
    delete state;
    state = new PlayerDead();

    vSpeed = -7.5;
}

void Player::process_event(EntityEvent e)
{
    switch (e.type)
    {
    case TAKE_DAMAGE:
        take_damage(e.value);
        break;
    case HEAL:
        if (health < 3)
            health += e.value;
        break;
    default:
        break;
    }
}
