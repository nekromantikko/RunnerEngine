#include "particles.h"
#include "renderer.h"
#include "entitymanager.h"
#include "sprite.h"
#include "resource.h"
#include "Platform/platform.h"
#include "debug.h"
#include "particlemanager.h"

void ParticleEmitter::allocate(u32 size)
{
    ParticleArray pArray;
    pArray = ParticleManager::allocate(size);

    particleCount = pArray.pCount;
    particleOffset = pArray.offset;
    indices.allocate(particleCount);

    pData = ParticleManager::get_data(particleOffset);
    pPosition = ParticleManager::get_position(particleOffset);
    pRotation = ParticleManager::get_rotation(particleOffset);
    pScale = ParticleManager::get_scale(particleOffset);
    pColor = ParticleManager::get_color(particleOffset);
    pStatus = ParticleManager::get_status(particleOffset);

    //vao = platform_create_particle_vertex_array(particleCount, pPosition, pRotation, pScale, pColor, pStatus);
}

ParticleEmitter::~ParticleEmitter()
{
    reset();
}

void ParticleEmitter::reset()
{
    indices.deinit_all();
    ParticleManager::deallocate(particleOffset);
    deinitBuffer.clear();
    //platform_delete_particle_vertex_array(vao);
}

bool ParticleEmitter::update(Transform xform)
{
    TIMED_BLOCK;
    //if looped enough times, don't update anymore
    bool doneLooping = false;
    bool noParticles = false;

    time += 1.0 / 60;

    while (time >= prefab->duration)
    {
        time -= prefab->duration;
        looped++;
    }

    if (prefab->loops == 0 || looped < prefab->loops)
    {
        spawnTime += prefab->spawnRate / 60;

        while (spawnTime >= 1)
        {
            spawnTime -= 1;
            spawn(1, (v2)xform.position);
        }

    }
    else doneLooping = true;

    if (indices.active_size() != 0)
    {
        Sprite *sprite = prefab->sprite;
        u32 animation = prefab->animation;

        for (auto it = indices.begin(); it != indices.partition(); it++)
        {
            u32 index = *it;

            update_particle(index);
        }

        //send drawcall
        ParticleCall call;

        call.vao = vao;
        call.texture = sprite->texture;
        call.lightmap = sprite->lightmap;
        call.normal = sprite->normal;
        call.offset = {sprite->xOffset, sprite->yOffset};
        call.flip = {0, 0};
        call.glow = sprite->glow;
        call.priority = 1;
        call.clipRect = {0,0,1,1};

        Renderer::add_particles(call);
    }
    else noParticles = true;

    deinit_buffer();

    return (!noParticles || !doneLooping);
}
void ParticleEmitter::spawn(u32 amount, v2 pos)
{
    while (amount > 0)
    {
        s32 index = indices.init();

        if (index != -1)
        {
            ParticleData &p = pData[index];
            ParticlePosition &position = pPosition[index];
            ParticleRotation &rot = pRotation[index];
            ParticleScale &scale = pScale[index];
            float &state = pStatus[index];
            v4 &color = pColor[index];

            state = true;

            p = {};
            p.initialVelocity = random_vec2(prefab->pInitialVelocity.min.x, prefab->pInitialVelocity.max.x,
                                                           prefab->pInitialVelocity.min.y, prefab->pInitialVelocity.max.y);
            position.current = get_spawn_point() + pos;
            position.previous = position.current;
            rot.current = {0,0,degrees_to_radians(real_random(prefab->pInitialRotation.min, prefab->pInitialRotation.max))};
            rot.previous = rot.current;
            p.deltaRotation = degrees_to_radians(real_random(prefab->pDeltaRotation.min, prefab->pDeltaRotation.max));
            //
            r32 scl = real_random(prefab->pInitialScale.min, prefab->pInitialScale.max);
            scale.current = {scl,scl,scl};
            scale.previous = scale.current;
            p.lifetime = seconds_to_frames(real_random(prefab->pLifetime.min, prefab->pLifetime.max));
            p.animSpeed = real_random(prefab->pAnimSpeed.min, prefab->pAnimSpeed.max);
            //
            color = random_vec4(prefab->pInitialColor.min.x, prefab->pInitialColor.max.x,
                                                 prefab->pInitialColor.min.y, prefab->pInitialColor.max.y,
                                                 prefab->pInitialColor.min.z, prefab->pInitialColor.max.z,
                                                 prefab->pInitialColor.min.w, prefab->pInitialColor.max.w);
            v4 endColor = random_vec4(prefab->pEndColor.min.x, prefab->pEndColor.max.x,
                                                      prefab->pEndColor.min.y, prefab->pEndColor.max.y,
                                                      prefab->pEndColor.min.z, prefab->pEndColor.max.z,
                                                      prefab->pEndColor.min.w, prefab->pEndColor.max.w);
            p.deltaColor = (endColor - color) / p.lifetime;
        }

        amount--;
    }
}
void ParticleEmitter::update_particle(u32 index)
{
    ParticleData &p = pData[index];
    ParticlePosition &position = pPosition[index];
    ParticleRotation &rot = pRotation[index];
    ParticleScale &scale = pScale[index];
    float &state = pStatus[index];
    v4 &color = pColor[index];

    position.previous = position.current;
    rot.previous = rot.current;
    scale.previous = scale.current;

    p.lifetime--;
    color += p.deltaColor;

    if (p.lifetime <= 0)
    {
        state = false;
        deinitBuffer.push_back(index);
        return;
    }

    position.current += p.initialVelocity;
    rot.current.z += p.deltaRotation;
    p.initialVelocity.y += prefab->gravity;

    p.a += p.animSpeed;
    while (p.a >= 1)
    {
        p.a -= 1;
        p.frame++;
        if (p.frame >= prefab->sprite->get_anim(prefab->animation)->frames.size())
            p.frame = 0;
    }
}

void ParticleEmitter::deinit_buffer()
{
    for (u32 index : deinitBuffer)
        indices.deinit(index);

    deinitBuffer.clear();
}

v2 ParticleEmitter::get_spawn_point()
{
    v2 point = {};

    switch (prefab->spawnAreaType)
    {
    case PSAT_POINT:
        point = {0,0};
        break;
    case PSAT_CIRCLE:
        point = random_point_on_circle(prefab->spawnCircleRadius);
        break;
    case PSAT_RECTANGLE:
        point = random_point_on_rect({prefab->spawnRectDimensions.x, prefab->spawnRectDimensions.y});
        break;
    default:
        break;
    }

    return point;
}

///////////////////////////////////////////////////////////
ParticleSystemPrefab::ParticleSystemPrefab()
{
    //TODO
}

ParticleSystemPrefab::~ParticleSystemPrefab()
{
    delete[] emitters;
}

void ParticleSystem::init()
{
    state = false;

    if (prefab)
    {
        emitterCount = prefab->emitterCount;
        emitters = new ParticleEmitter[emitterCount];
    }
    else emitterCount = 0;

    for (memory_index i = 0; i < emitterCount; i++)
    {
        emitters[i].allocate(20000);
        emitters[i].prefab = &prefab->emitters[i];

        //and then do some shiz
    }
}
void ParticleSystem::deinit()
{
    delete[] emitters;
}
void ParticleSystem::update()
{
    for (memory_index i = 0; i < emitterCount; i++)
        emitters[i].update(parent->transform);
}

void ParticleSystem::process_event(EntityEvent e)
{

}

void ParticleSystem::play()
{
    state = true;
}
void ParticleSystem::pause()
{
    state = false;
}
void ParticleSystem::stop()
{
    state = false;

    for (memory_index i = 0; i < emitterCount; i++)
        emitters[i].reset();
}

