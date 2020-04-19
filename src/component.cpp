#include "component.h"
#include "sprite.h"
#include "renderer.h"
#include "entitymanager.h"
#include "resource.h"

RenderComponent::RenderComponent(RenderComponentType t) : type(t)
{
}

SpriteComponent::SpriteComponent() : RenderComponent(RENDER_SPRITE)
{
}

void SpriteComponent::init(const char *spriteName, u32 anim, r32 speed, v4 c)
{
    sInstance.sprite = Resource::get_sprite(spriteName);
    sInstance.currentAnim = anim;
    animSpeed = speed;
    color = c;
}

void SpriteComponent::update(Transform &transform, r32 depth)
{
    sInstance.update(animSpeed);

    ///////////////////////////////////////////
    Sprite *sprite = sInstance.sprite;

    Transform currentTransform = transform;
    currentTransform.translate(-Renderer::get_camera_position());
    currentTransform.do_scale({sprite->width, sprite->height, 1});

    if (visible)
        Renderer::draw_sprite_instance(&sInstance, currentTransform, previous, hFlip, vFlip, &color, -depth);

    previous = currentTransform;
}

void SpriteComponent::set_animation(u32 animIndex)
{
    sInstance.set_animation(animIndex);
}

void SpriteComponent::set_frame(u32 frame)
{
    sInstance.currentFrame = frame;
}

ModelComponent::ModelComponent() : RenderComponent(RENDER_MODEL)
{
}

void ModelComponent::update(Transform &transform, r32 depth)
{
    Transform currentTransform = transform;
    currentTransform.translate(-Renderer::get_camera_position());

    //send callback function
    ModelCall call;

    call.a = currentTransform;
    call.b = previous;

    call.vao = model->vbuffer;
    call.texture = model->texture;
    call.lightmap = model->lightmap;
    call.normal = model->normal;
    call.glow = model->glow;

    previous = currentTransform;

    if (visible)
        Renderer::add_model(call);
}

MirrorComponent::MirrorComponent() : RenderComponent(RENDER_MIRROR)
{
}

void MirrorComponent::update(Transform &transform, r32 depth)
{
    Transform currentTransform = transform;
        currentTransform.translate(-Renderer::get_camera_position());

    if (visible)
        Renderer::draw_mirror(currentTransform, previous, width, height);

    previous = currentTransform;
}

void Follower::update()
{
    if (target)
        parent->transform = target->transform;
}

void Follower::process_event(EntityEvent e)
{

}
void Follower::init()
{
    target = EntityManager::find_entity(targetName);
}
void Follower::deinit()
{

}

/////////////////////////////////////////////////////

void CameraComponent::update()
{
    if (!target || !parent->collider)
        return;
    if (parent->collider->type != COLLIDER_AABB)
        return;

    AABBCollider *aabb = (AABBCollider*)parent->collider;
    fvec2 targetPos = target->transform.position;

    Transform &xform = parent->transform;

    f32 left, right, top, bottom;
    right = aabb->x2 + xform.position.x;
    left = aabb->x1 + xform.position.x;
    bottom = aabb->y2 + xform.position.y;
    top = aabb->y1 + xform.position.y;

    if (left > targetPos.x)
        xform.translate({targetPos.x - left, 0});
    else if (right < targetPos.x)
        xform.translate({targetPos.x - right, 0});

    if (top > targetPos.y)
        xform.translate({0, targetPos.y - top});
    else if (bottom < targetPos.y)
        xform.translate({0, targetPos.y - bottom});

    Renderer::set_camera_position(xform.position);
}
void CameraComponent::process_event(EntityEvent e)
{

}
void CameraComponent::init()
{
    target = EntityManager::find_entity(targetName);
}

void CameraComponent::deinit()
{

}

////////////////////////////////////////////

void RotatingTrack::update()
{
    angle += speed;

    u32 targetCount = targets.size();
    if (!targetCount)
        return;

    r32 angleInRad = degrees_to_radians(angle);
    r32 anglePerEntity = degrees_to_radians(360 / targetCount);
    u32 counter = 1;
    for (Entity *entity : targets)
    {
        entity->transform.position = {radius * cos(anglePerEntity * counter + angleInRad), radius * sin(anglePerEntity * counter + angleInRad)};
        entity->transform.position += parent->transform.position;
        counter++;
    }
}
void RotatingTrack::process_event(EntityEvent e)
{

}

void RotatingTrack::init()
{

}

void RotatingTrack::deinit()
{
    targets.clear();
}

void RotatingTrack::add_entity(Entity *entity)
{
    targets.push_back(entity);
}

////////////////////////////

void PlatformController::update()
{
    fvec2 deltaPos = parent->transform.position - previousPos;
    for (Entity *target : targets)
        target->transform.translate(deltaPos);

    previousPos = parent->transform.position;
    //targets.clear();

    //if (!test)
        //test = parent->play_sound("Randomize19", -1);
}
void PlatformController::process_event(EntityEvent e)
{
    switch (e.type)
    {
    case STICK:
        {
            bool exists = false;
            Entity *newTarget = (Entity*)e.value;
            for (Entity *entity : targets)
            {
                if (entity == newTarget)
                    exists = true;
            }
            if (!exists)
                targets.push_back(newTarget);
            break;
        }
    case UNSTICK:
        {
            Entity *targetToRemove = (Entity*)e.value;
            targets.erase(std::find(targets.begin(), targets.end(), targetToRemove));
        }
    default:
        break;
    }
}
void PlatformController::init()
{
    test = 0;
}
void PlatformController::deinit()
{
    targets.clear();
}

/////////////////////////////////////////////

void HealthPickupComponent::update()
{
    parent->transform.rotate({0,0.05,0});

    if (parent->collider)
    {
        EntityHit collector = Collision::entity_collision(parent->collider, parent->transform, collectors);
        if (collector.hit)
        {
            EntityEvent e;
            e.type = HEAL;
            e.value = healAmount;

            collector.entity->add_event(e);

            parent->play_sound("medkit_pickup", 0);
            EntityManager::kill(parent);
        }
    }
}
void HealthPickupComponent::process_event(EntityEvent e)
{

}
void HealthPickupComponent::init()
{

}
void HealthPickupComponent::deinit()
{

}

//////////////////////////////////////////////

void SlidingDoorComponent::update()
{
    int32 multiplier = 1;

    if (dir == DOWN)
       multiplier = -1;

    r32 moveStep = height / openTime;

    switch (state)
    {
    case DOOR_OPENING:
        parent->transform.translate({0, -moveStep * multiplier});
        accumulator += moveStep;
        if (accumulator >= height)
        {
            state = DOOR_OPEN;
            accumulator = 0;
        }
        break;
    case DOOR_CLOSING:
        parent->transform.translate({0, moveStep * multiplier});
        accumulator += moveStep;
        if (accumulator >= height)
        {
            state = DOOR_CLOSED;
            accumulator = 0;
        }
        break;
    default:
        break;
    }

}
void SlidingDoorComponent::process_event(EntityEvent e)
{
    switch (e.type)
    {
    case OPEN:
        if (state == DOOR_CLOSED)
            state = DOOR_OPENING;
        break;
    case CLOSE:
        if (state == DOOR_OPEN)
            state = DOOR_CLOSING;
        break;
    default:
        break;
    }
}
void SlidingDoorComponent::init()
{
    accumulator = 0;
}
void SlidingDoorComponent::deinit()
{

}

//////////////////////////////////////////////

bool32 ColliderComponent::visible = false;

ColliderComponent::ColliderComponent(ColliderComponentType t) : type(t)
{
}

AABBCollider::AABBCollider() : ColliderComponent(COLLIDER_AABB)
{
}

void AABBCollider::draw(Transform &transform)
{
    /*
    mesh = {};
    mesh.vertices.reserve(4);

    s32 x1p, x2p, y1p, y2p;
    x1p = units_to_pixels(x1);
    x2p = units_to_pixels(x2);
    y1p = units_to_pixels(y1);
    y2p = units_to_pixels(y2);

    //front faces
    mesh.vertices.emplace_back(x1p, y1p, 0, 0, 0);
    mesh.vertices.emplace_back(x1p, y2p, 0, 0, 0);
    mesh.vertices.emplace_back(x2p, y2p, 0, 0, 0);
    mesh.vertices.emplace_back(x2p, y1p, 0, 0, 0);

    //indices
    mesh.indices.reserve(4);
    mesh.indices.push_back(0);
    mesh.indices.push_back(1);
    mesh.indices.push_back(2);
    mesh.indices.push_back(3);

    //send callback function
    Drawcall call;
    call.a = transform;
    call.a.position = {units_to_pixels(call.a.position.x), units_to_pixels(call.a.position.y)};
    call.a.translate(-Renderer::get_camera_position());
    call.b = call.a;
    call.mesh = &mesh;
    call.texture = NULL;
    call.lightmap = NULL;
    call.color = {0,1,0,0.5};
    call.glow = 0;
    call.depth = -500;

    Renderer::add_world_element(call);*/
}

SATRectCollider::SATRectCollider() : ColliderComponent(COLLIDER_SATRECT)
{
}

void SATRectCollider::draw(Transform &transform)
{
    /*
    mesh = {};
    mesh.vertices.reserve(4);

    v2 ap, bp, cp, dp;
    ap = {units_to_pixels(corners[0].x), units_to_pixels(corners[0].y)};
    bp = {units_to_pixels(corners[1].x), units_to_pixels(corners[1].y)};
    cp = {units_to_pixels(corners[2].x), units_to_pixels(corners[2].y)};
    dp = {units_to_pixels(corners[3].x), units_to_pixels(corners[3].y)};

    //front faces
    mesh.vertices.emplace_back(ap.x, ap.y, 0, 0, 0);
    mesh.vertices.emplace_back(dp.x, dp.y, 0, 0, 0);
    mesh.vertices.emplace_back(cp.x, cp.y, 0, 0, 0);
    mesh.vertices.emplace_back(bp.x, bp.y, 0, 0, 0);

    //indices
    mesh.indices.reserve(4);
    mesh.indices.push_back(0);
    mesh.indices.push_back(1);
    mesh.indices.push_back(2);
    mesh.indices.push_back(3);

    //send callback function
    Drawcall call;
    call.a = transform;
    call.a.position = {units_to_pixels(call.a.position.x), units_to_pixels(call.a.position.y)};
    call.a.translate(-Renderer::get_camera_position());
    call.b = call.a;
    call.mesh = &mesh;
    call.texture = NULL;
    call.lightmap = NULL;
    call.color = {0,1,0,0.5};
    call.glow = 0;
    call.depth = -500;

    Renderer::add_world_element(call);
    */
}

CircleCollider::CircleCollider() : ColliderComponent(COLLIDER_CIRCLE)
{
}

void CircleCollider::draw(Transform &transform)
{

}
