#ifndef UNITS_H
#define UNITS_H
#include "typedef.h"
#include <cmath>

//distance
/*
inline r64 units_to_pixels(s32 s)
{
    return s / 16.f;
}
inline r64 units_to_meters(s32 s)
{
    return s / 512.f;
}

inline s32 pixels_to_units(r64 s)
{
    return ceil(s * 16);
}*/
inline r64 pixels_to_meters(r64 s)
{
    return s / 32;
}
/*
inline s32 meters_to_units(r64 s)
{
    return ceil(s * 512);
}*/
inline r64 meters_to_pixels(r64 s)
{
    return s * 32;
}

//time
inline r64 frames_to_seconds(u32 t)
{
    return t / 60;
}
inline u32 seconds_to_frames(r64 t)
{
    return t * 60;
}

//frequency
inline r64 hertz_to_perFrame(r64 f)
{
    return f / 60;
}
inline r64 perFrame_to_hertz(r64 f)
{
    return f * 60;
}


//velocity
/*
inline r64 unitsPerFrame_to_metersPerSecond(s32 v)
{
    return (15 * v) / 128.f;
}*/

inline r64 metersPerSecond_to_kmPerHour(r64 v)
{
    return v * 3.6;
}
/*
inline r64 unitsPerFrame_to_kmPerHour(s32 v)
{
    return metersPerSecond_to_kmPerHour(unitsPerFrame_to_metersPerSecond(v));
}

inline s32 metersPerSecond_to_unitsPerFrame(r64 v)
{
    return (128 * v) / 15.f;
}
//acceleration
inline r64 unitsPerFrame2_to_metersPerSecond2(s32 a)
{
    return (225 * a) / 32.f;
}
inline s32 metersPerSecond2_to_unitsPerFrame2(r64 a)
{
    return (32 * a) / 225.f;
}*/

//color
inline r32 color_8bit_to_float(s32 c)
{
    return c / 255.f;
}

inline s32 color_float_to_8bit(r32 c)
{
    return c * 255;
}

//angles
inline r64 slope_to_angle(r64 s)
{
    return atan(s);
}

inline r64 angle_to_slope(r64 a)
{
    return tan(a);
}

#define PI 3.14159265358979f

inline r64 degrees_to_radians(r64 deg)
{
    r64 rad = deg * (PI / 180.f);
    return rad;
}

inline r64 radians_to_degrees(r64 rad)
{
    r64 deg = rad / (PI / 180.f);
    return deg;
}

#endif // UNITS_H
