#ifndef PARTICLEMANAGER_H
#define PARTICLEMANAGER_H

#include "particles.h"

#define MAX_PARTICLE_AMOUNT 200000
#define MAX_PARTICLE_SLACK 32
#define MIN_PARTICLE_SLOT_SIZE 64

enum AllocationStatus
{
    ALLOC_EQUAL = 0,
    ALLOC_SMALLER = 1,
    ALLOC_BIGGER = 2
};

struct ParticleArray
{
    u32 offset;
    u32 pCount;

    //some metadata
    AllocationStatus status;

    bool operator< (const ParticleArray &b)
    {
        return (offset < b.offset);
    }
};

namespace ParticleManager
{
    ParticleArray allocate(u32 pcount);
    void deallocate (u32 offset);
    void clear_particles();
    ParticlePosition *get_position(u32 offset);
    ParticleRotation *get_rotation(u32 offset);
    ParticleScale *get_scale(u32 offset);
    v4 *get_color(u32 offset);
    ParticleData *get_data(u32 offset);
    float *get_status(u32 offset);
    void draw_debug();
}

#endif // PARTICLEMANAGER_H
