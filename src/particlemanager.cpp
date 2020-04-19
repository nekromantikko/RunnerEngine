#include "particlemanager.h"
#include <algorithm>
#include <list>
#include "renderer.h"

namespace ParticleManager
{
    ParticleData data[MAX_PARTICLE_AMOUNT];
    ParticlePosition position[MAX_PARTICLE_AMOUNT];
    ParticleRotation rotation[MAX_PARTICLE_AMOUNT];
    ParticleScale scale[MAX_PARTICLE_AMOUNT];
    v4 color[MAX_PARTICLE_AMOUNT];
    float status[MAX_PARTICLE_AMOUNT];

    std::list<ParticleArray> allocations;
}

ParticleArray ParticleManager::allocate(u32 pcount)
{
    ParticleArray p;
    p.offset = 0;
    p.pCount = 0;

    if (allocations.empty())
    {
        ParticleArray newAlloc;
        newAlloc.offset = 0;
        newAlloc.pCount = std::min(pcount, (u32)MAX_PARTICLE_AMOUNT);

        if (newAlloc.pCount == pcount)
            newAlloc.status = ALLOC_EQUAL;
        else if (newAlloc.pCount > pcount)
            newAlloc.status = ALLOC_BIGGER;
        else newAlloc.status = ALLOC_SMALLER;

        if (newAlloc.pCount > 0)
            allocations.push_back(newAlloc);
        p = newAlloc;
    }
    else
    {
        allocations.sort();
        u32 previousAllocEnd = 0;

        s32 closestNegative = -1;
        s32 negativeOffset = -1;
        s32 closestPositive = -1;
        s32 positiveOffset = -1;

        auto try_closest = [&](u32 emptySpaceSize)
        {
            if (emptySpaceSize < pcount)
            {
                if ((emptySpaceSize > closestNegative) || (closestNegative == -1))
                {
                    closestNegative = emptySpaceSize;
                    negativeOffset = previousAllocEnd;
                }
            }
            else
            {
                if ((emptySpaceSize < closestPositive) || (closestPositive == -1))
                {
                    closestPositive = emptySpaceSize;
                    positiveOffset = previousAllocEnd;
                }
            }
        };
        //find empty slot closest to the requested size
        for (ParticleArray &alloc : allocations)
        {
            //size of empty space between allocations
            u32 emptySpaceSize = alloc.offset - previousAllocEnd;

            try_closest(emptySpaceSize);

            previousAllocEnd = alloc.offset + alloc.pCount;
        }

        u32 remainingSpace = MAX_PARTICLE_AMOUNT - previousAllocEnd;
        try_closest(remainingSpace);

        ParticleArray newAlloc;

        {
            s32 negativeDistance = pcount - closestNegative;
            s32 positiveDistance = closestPositive - pcount;
            bool posIsBigger;

            if (negativeDistance < positiveDistance || positiveDistance < 0)
                posIsBigger = true;
            else posIsBigger = false;

            if (posIsBigger && (negativeDistance < MAX_PARTICLE_SLACK))
            {
                newAlloc.offset = negativeOffset;
                newAlloc.pCount = closestNegative;
            }
            else if (!posIsBigger && (positiveDistance < MAX_PARTICLE_SLACK))
            {
                newAlloc.offset = positiveOffset;
                newAlloc.pCount = closestPositive;
            }
            else
            {
                if (positiveDistance >= 0)
                {
                    newAlloc.offset = positiveOffset;
                    if (positiveDistance <= MIN_PARTICLE_SLOT_SIZE)
                        newAlloc.pCount = closestPositive;
                    else newAlloc.pCount = pcount;
                }
                else
                {
                    newAlloc.offset = negativeOffset;
                    newAlloc.pCount = closestNegative;
                }
            }
        }

        if (newAlloc.pCount == pcount)
            newAlloc.status = ALLOC_EQUAL;
        else if (newAlloc.pCount > pcount)
            newAlloc.status = ALLOC_BIGGER;
        else newAlloc.status = ALLOC_SMALLER;

        if (newAlloc.pCount > 0)
            allocations.push_back(newAlloc);

        p = newAlloc;
    }

    return p;
}
void ParticleManager::deallocate(u32 offset)
{
    for (auto it = allocations.begin(); it != allocations.end(); it++)
    {
        ParticleArray &alloc = *it;
        if (alloc.offset != offset)
            continue;

        allocations.erase(it);
        return;
    }
}
void ParticleManager::clear_particles()
{
    allocations.clear();
}
ParticlePosition *ParticleManager::get_position(u32 offset)
{
    return position + offset;
}
ParticleRotation *ParticleManager::get_rotation(u32 offset)
{
    return rotation + offset;
}
ParticleScale *ParticleManager::get_scale(u32 offset)
{
    return scale + offset;
}
v4 *ParticleManager::get_color(u32 offset)
{
    return color + offset;
}
ParticleData *ParticleManager::get_data(u32 offset)
{
    return data + offset;
}
float *ParticleManager::get_status(u32 offset)
{
    return status + offset;
}
void ParticleManager::draw_debug()
{
    v4 black = {0,0,0,1};
    Renderer::draw_rectangle(0,0,1024, 32, NULL, &black, 1);

    r32 barRatio = 1024.f / MAX_PARTICLE_AMOUNT;

    for (ParticleArray &alloc : allocations)
    {
        r32 offsetInPx = std::floor(alloc.offset * barRatio);
        r32 lengthInPx = std::ceil(alloc.pCount * barRatio);
        v4 color;
        if (alloc.status == ALLOC_EQUAL)
            color = {0,1,0,1};
        else if (alloc.status == ALLOC_SMALLER)
            color = {1,0,0,1};
        else color = {0,0,1,1};

        Renderer::draw_rectangle(offsetInPx,0,lengthInPx, 32, NULL, &color, 2);
    }
}
