#include "laser.h"
#include "renderer.h"
#include "entitymanager.h"
#include "resource.h"
#include "Platform/platform.h"

void Laser::update()
{
    switch (type)
    {
    case LASER_YELLOW:
        update_yellow(parent);
        break;
    case LASER_BLUE:
        update_blue(parent);
        break;
    default:
        break;
    }

    r32 length = 0;

    RaycastHit hit = Collision::cast_ray(parent->transform.position, Arm2(parent->transform.rotation.z), 1024);
    if (hit.hit)
        length = hit.length;
    else length = 1024;

    parent->transform.scale.x = length / 32.f;

    if (state != LASER_ON)
        parent->collider->state = false;
    else
    {
        parent->collider->state = true;

        EntityHit hit;

        hit = Collision::entity_collision(parent->collider, parent->transform, COLLISION_PLAYER);
        if (hit.hit)
        hit.entity->add_event({TAKE_DAMAGE, 1});

    }
}

void Laser::init()
{

}

void Laser::deinit()
{

}

void Laser::update_yellow(Entity *parent)
{
    stateChangeTimer.update();

    if (stateChangeTimer == 0)
    {
        if (state == LASER_OFF)
        {
            stateChangeTimer.set_timer(3);
            state = LASER_TURNING_ON;
        }
        else if (state == LASER_ON)
        {
            stateChangeTimer.set_timer(3);
            state = LASER_TURNING_OFF;
        }
        else if (state == LASER_TURNING_OFF)
        {
            stateChangeTimer.set_timer(57);
            state = LASER_OFF;
        }
        else if (state == LASER_TURNING_ON)
        {
            stateChangeTimer.set_timer(57);
            state = LASER_ON;
        }
    }

    if (parent->render->type != RENDER_SPRITE)
        return;
    SpriteComponent *sprite = (SpriteComponent*)parent->render;
    switch (state)
    {
    case LASER_OFF:
        sprite->visible = false;
        break;
    case LASER_TURNING_ON:
        sprite->visible = true;
        sprite->set_animation(1);
        break;
    case LASER_ON:
        sprite->visible = true;
        sprite->set_animation(0);
        break;
    case LASER_TURNING_OFF:
        sprite->visible = true;
        sprite->set_animation(2);
        break;
    default:
        break;
    }
}

void Laser::update_blue(Entity *parent)
{
    stateChangeTimer.update();

    if (stateChangeTimer == 0)
    {
        if (state == LASER_TURNING_OFF)
        {
            state = LASER_OFF;
        }
        else if (state == LASER_TURNING_ON)
        {
            state = LASER_ON;
        }
    }

    if (parent->render->type != RENDER_SPRITE)
        return;
    SpriteComponent *sprite = (SpriteComponent*)parent->render;
    switch (state)
    {
    case LASER_OFF:
        sprite->visible = false;
        break;
    case LASER_TURNING_ON:
        sprite->visible = true;
        sprite->set_animation(4);
        break;
    case LASER_ON:
        sprite->visible = true;
        sprite->set_animation(3);
        break;
    case LASER_TURNING_OFF:
        sprite->visible = true;
        sprite->set_animation(5);
        break;
    default:
        break;
    }
}

void Laser::process_event(EntityEvent e)
{
    switch (e.type)
    {
    case TURN_ON:
        stateChangeTimer.set_timer(3);
        state = LASER_TURNING_ON;
        break;
    case TURN_OFF:
        stateChangeTimer.set_timer(3);
        state = LASER_TURNING_OFF;
        break;
    default:
        break;
    }
}
