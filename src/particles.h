#ifndef PARTICLES_H
#define PARTICLES_H
#include "shared.h"
#include "pool.h"
#include "component.h"

struct ParticleData
{
    r32 deltaRotation;

    //lifetime in frames
    u32 lifetime = 0;
    fvec2 initialVelocity;

    r32 animSpeed;
    r32 a = 0;
    u32 frame = 0;

    v4 deltaColor;
};

template <class T>
struct Uniform
{
    T min;
    T max;

    Uniform() : min(T()), max(T())
    {
    }
    Uniform(T min, T max) : min(min), max(max)
    {
    }
    Uniform(T a) : min(a), max(a)
    {
    }
    Uniform<T> &operator= (const Uniform<T> &other)
    {
        min = other.min;
        max = other.max;
        return *this;
    }
    Uniform<T> &operator= (const T &a)
    {
        min = a;
        max = a;
        return *this;
    }
};

enum ParticleSpawnAreaType
{
    PSAT_POINT = 0,
    PSAT_RECTANGLE = 1,
    PSAT_CIRCLE = 2
};

enum ParticleVisualType
{
    PVT_SPRITE = 0,
    PVT_MODEL = 1
};

struct ParticleEmitterPrefab
{
    Sprite *sprite;
    //spawn rate in hz (1/s)
    r32 spawnRate = 20;
    //duration in seconds
    r32 duration = 1;
    u32 loops = 0;
    ParticleSpawnAreaType spawnAreaType = PSAT_POINT;
    union
    {
        struct
        {
            r32 x, y;
        } spawnRectDimensions;
        r64 spawnCircleRadius;
    };

    Uniform<r32> pLifetime;
    Uniform<r32> pInitialScale;
    Uniform<v2> pInitialVelocity;
    Uniform<r32> pInitialRotation;
    Uniform<r32> pDeltaRotation;
    Uniform<r32> pAnimSpeed;
    Uniform<v4> pInitialColor;
    Uniform<v4> pEndColor;

    u32 animation;
    u32 gravity = 0;
};

class ParticleEmitter
{
public:
    ~ParticleEmitter();
    void allocate(u32 size);
    void reset();
    //returns false when emitter no longer does anything
    //(that is, when no more particles are spawned, or exist)
    bool update(Transform xform);
    void spawn(u32 amount, v2 pos);
    void update_particle(u32 index);
    void deinit_buffer();
    inline v2 get_spawn_point();

    ParticleEmitterPrefab *prefab;
private:
    IndexPool indices;
    u32 particleOffset;
    std::vector<u32> deinitBuffer;

    //////////////////////////////////////////////
    u32 particleCount;
    ParticleData *pData;
    ParticlePosition *pPosition;
    ParticleRotation *pRotation;
    ParticleScale *pScale;
    v4 *pColor;
    float *pStatus;

    ParticleVertexArrayHandle *vao;
    //////////////////////////////////////////////

    r32 spawnTime = 0;
    r32 time = 0;
    u32 looped = 0;
};

class ParticleSystemPrefab
{
public:
    ParticleSystemPrefab();
    ~ParticleSystemPrefab();

    u32 emitterCount;
    ParticleEmitterPrefab *emitters;
};

class ParticleSystem : public BehaviourComponent
{
public:
    void update();
    void process_event(EntityEvent e);
    void init();
    void deinit();

    void play();
    void pause();
    void stop();

    ParticleSystemPrefab *prefab;
private:
    u32 emitterCount;
    ParticleEmitter *emitters;
    bool32 state;
};

#endif // PARTICLES_H
