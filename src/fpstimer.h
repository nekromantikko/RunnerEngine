#ifndef FPSTIMER_H
#define FPSTIMER_H
#include "typedef.h"

//a class to keep track of passed time in a level, and also fps
class FPSTimer
{
public:
    FPSTimer();
    ~FPSTimer();
    void add_time(r64);
    r32 get_fps() const;
    r64 get_dt() const;
private:
    //amount of frames since last fps update
    u32 fcount = 0;
    //last delta time
    r64 dt = 0;
    //current fps count
    r32 fps = 0;
    //average time to complete a frame, calculated from the last 30 frames
    r32 time30f = 0;
};

#endif // FPSTIMER_H
