#include "leveltimer.h"
#include <iostream>
#include "Platform/platform.h"

LevelTimer::LevelTimer()
{
    startTime = platform_time_in_ms();
    time = 0;

    std::cout << "LevelTimer <" << this << "> created!" << std::endl;
}

LevelTimer::~LevelTimer()
{
    std::cout << "LevelTimer <" << this << "> destroyed!" << std::endl;
}

void LevelTimer::update()
{
    r64 currentTime = platform_time_in_ms();
    time = currentTime - startTime;
}

s32 LevelTimer::get_minutes()
{
    u32 roundedTime = (u32)time;
    return roundedTime / 60000;
}

s32 LevelTimer::get_seconds()
{
    u32 roundedTime = (u32)time;
    return (roundedTime % 60000) / 1000;
}

s32 LevelTimer::get_centiseconds()
{
    u32 roundedTime = (u32)time;
    return ((roundedTime % 60000) % 1000) / 10;
}

void LevelTimer::init()
{
    startTime = platform_time_in_ms();
    time = 0;
}
