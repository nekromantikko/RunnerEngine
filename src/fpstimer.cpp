#include "fpstimer.h"
#include <iostream>

FPSTimer::FPSTimer()
{
    std::cout << "FPSTimer <" << this << "> created!" << std::endl;
}

FPSTimer::~FPSTimer()
{
    std::cout << "FPSTimer <" << this << "> destroyed!" << std::endl;
}

void FPSTimer::add_time(r64 t)
{
    dt = t;
    if (fcount == 30)
    {
        fps = 30000/time30f;
        time30f = 0;
        fcount = 0;
    }
    else
    {
        time30f += t;
        fcount++;
    }
}

r32 FPSTimer::get_fps() const
{
    return fps;
}

r64 FPSTimer::get_dt() const
{
    return dt;
}


