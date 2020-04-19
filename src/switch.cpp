#include "switch.h"
#include "entitymanager.h"
#include "resource.h"
#include "Platform/platform.h"

void Switch::update()
{
    if (time > 0 && state == true)
    {
        stateChangeTimer.update();

        if (stateChangeTimer == 0)
        {
            state = false;
        }
    }

    if (parent->render->type != RENDER_SPRITE)
        return;
    SpriteComponent *sprite = (SpriteComponent*)parent->render;
    switch (state)
    {
    case true:
        sprite->set_animation(1);
        break;
    case false:
        sprite->set_animation(0);
        break;
    default:
        break;
    }

    //collision
    EntityHit hit = Collision::entity_collision(parent->collider, parent->transform, COLLISION_PLAYER);
    if (hit.hit)
    {
        if (!colliding)
        {
            colliding = true;
            state = !state;
            if (state && time)
                stateChangeTimer.set_timer(time);

            Entity *target = EntityManager::find_entity(targetName);
            if (target)
            {
                parent->play_sound("sfx_beep", 0);

                if (state)
                    target->add_event({TURN_ON, 0});
                else target->add_event({TURN_OFF, 0});
            }
        }
    }
    else colliding = false;
}
void Switch::process_event(EntityEvent e)
{
}
void Switch::init()
{
}
void Switch::deinit()
{
}
