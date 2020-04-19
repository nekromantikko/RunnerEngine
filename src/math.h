#ifndef MATH_H
#define MATH_H
#include <cmath>
#include <cstdlib>
#include "vector.h"

inline f32
fixed_min(f32 a, f32 b)
{
    if (b < a)
        return b;
    else return a;
}

inline f32
fixed_max(f32 a, f32 b)
{
    if (b > a)
        return b;
    else return a;
}

inline f32
fixed_abs(f32 a)
{
    if (a >= 0)
        return a;
    else return -a;
}

inline int32
sign_of(int32 Value)
{
    int32 Result = (Value >= 0) ? 1 : -1;
    return(Result);
}

inline real32
sign_of(real32 Value)
{
    real32 Result = (Value >= 0) ? 1.0f : -1.0f;
    return(Result);
}

inline real32
square_root(real32 Real32)
{
    real32 Result = sqrtf(Real32);
    return(Result);
}

inline real32
absolute_value(real32 Real32)
{
    real32 Result = fabs(Real32);
    return(Result);
}

inline r32
real_random(r32 fMin, r32 fMax)
{
    r32 f = (r32)rand() / RAND_MAX;
    return fMin + f * (fMax - fMin);
}

inline f32
round_fixed(f32 fixed)
{
    f32 result = f32(std::round(fixed.to_double()));
    return result;
}

inline s32
random_integer(s32 iMin, s32 iMax)
{
    // x is in [0,1[
    double x = rand()/static_cast<double>(RAND_MAX+1);

    // [0,1[ * (max - min) + min is in [min,max[
    int that = iMin + static_cast<int>( x * (iMax - iMin) );

    return that;
}

#define PI 3.14159265358979f

inline v2
random_point_on_circle(r32 radius)
{
    //generate random angle
    r32 angle = real_random(0, 2*PI);

    r32 a, b;
    a = real_random(0, radius);
    b = real_random(0, radius);
    r32 d = a + b;
    if (d > radius)
        d -= radius;
    return {d*cos(angle), d*sin(angle)};
}

inline v2
random_point_on_rect(v2 d)
{
    r32 a = real_random(-d.x / 2, d.x / 2);
    r32 b = real_random(-d.y / 2, d.y / 2);
    return {a, b};
}

inline v2
random_vec2(r32 xMin, r32 xMax, r32 yMin, r32 yMax)
{
    v2 result;
    result.x = real_random(xMin, xMax);
    result.y = real_random(yMin, yMax);
    return result;
}

inline v2
random_vec2(r32 aMin, r32 aMax)
{
    v2 result;
    result.x = real_random(aMin, aMax);
    result.y = real_random(aMin, aMax);
    return result;
}

inline v3
random_vec3(r32 xMin, r32 xMax, r32 yMin, r32 yMax, r32 zMin, r32 zMax)
{
    v3 result;
    result.x = real_random(xMin, xMax);
    result.y = real_random(yMin, yMax);
    result.z = real_random(zMin, zMax);
    return result;
}

inline v3
random_vec3(r32 aMin, r32 aMax)
{
    v3 result;
    result.x = real_random(aMin, aMax);
    result.y = real_random(aMin, aMax);
    result.z = real_random(aMin, aMax);
    return result;
}

inline v4
random_vec4(r32 xMin, r32 xMax, r32 yMin, r32 yMax, r32 zMin, r32 zMax, r32 wMin, r32 wMax)
{
    v4 result;
    result.x = real_random(xMin, xMax);
    result.y = real_random(yMin, yMax);
    result.z = real_random(zMin, zMax);
    result.w = real_random(wMin, wMax);
    return result;
}

inline v4
random_vec4(r32 aMin, r32 aMax)
{
    v4 result;
    result.x = real_random(aMin, aMax);
    result.y = real_random(aMin, aMax);
    result.z = real_random(aMin, aMax);
    result.w = real_random(aMin, aMax);
    return result;
}


/*
inline v2
V2i(int32 X, int32 Y)
{
    v2 Result = {(real32)X, (real32)Y};

    return(Result);
}

inline v2
V2i(uint32 X, uint32 Y)
{
    v2 Result = {(real32)X, (real32)Y};

    return(Result);
}

inline v2
V2(real32 X, real32 Y)
{
    v2 Result;

    Result.x = X;
    Result.y = Y;

    return(Result);
}

inline v3
V3(real32 X, real32 Y, real32 Z)
{
    v3 Result;

    Result.x = X;
    Result.y = Y;
    Result.z = Z;

    return(Result);
}

inline v3
V3(v2 XY, real32 Z)
{
    v3 Result;

    Result.x = XY.x;
    Result.y = XY.y;
    Result.z = Z;

    return(Result);
}

inline v4
V4(real32 X, real32 Y, real32 Z, real32 W)
{
    v4 Result;

    Result.x = X;
    Result.y = Y;
    Result.z = Z;
    Result.w = W;

    return(Result);
}

inline v4
V4(v3 XYZ, real32 W)
{
    v4 Result;

    Result.xyz = XYZ;
    Result.w = W;

    return(Result);
}*/

//
// NOTE(casey): Scalar operations
//

inline real32
Square(real32 A)
{
    real32 Result = A*A;

    return(Result);
}

inline real32
Lerp(real32 A, real32 t, real32 B)
{
    real32 Result = (1.0f - t)*A + t*B;

    return(Result);
}

inline real32
Clamp(real32 Min, real32 Value, real32 Max)
{
    real32 Result = Value;

    if(Result < Min)
    {
        Result = Min;
    }
    else if(Result > Max)
    {
        Result = Max;
    }

    return(Result);
}

inline real32
Clamp01(real32 Value)
{
    real32 Result = Clamp(0.0f, Value, 1.0f);

    return(Result);
}

inline real32
Clamp01MapToRange(real32 Min, real32 t, real32 Max)
{
    real32 Result = 0.0f;

    real32 Range = Max - Min;
    if(Range != 0.0f)
    {
        Result = Clamp01((t - Min) / Range);
    }

    return(Result);
}

inline real32
SafeRatioN(real32 Numerator, real32 Divisor, real32 N)
{
    real32 Result = N;

    if(Divisor != 0.0f)
    {
        Result = Numerator / Divisor;
    }

    return(Result);
}

inline real32
SafeRatio0(real32 Numerator, real32 Divisor)
{
    real32 Result = SafeRatioN(Numerator, Divisor, 0.0f);

    return(Result);
}

inline real32
SafeRatio1(real32 Numerator, real32 Divisor)
{
    real32 Result = SafeRatioN(Numerator, Divisor, 1.0f);

    return(Result);
}

//
// NOTE(casey): v2 operations
//

template <typename T>
inline vec2<T>
operator*(T A, vec2<T> B)
{
    vec2<T> Result;

    Result.x = A*B.x;
    Result.y = A*B.y;

    return(Result);
}

template <typename T>
inline vec2<T>
Perp(vec2<T> A)
{
    vec2<T> Result = {-A.y, A.x};
    return(Result);
}

template <typename T>
inline vec2<T>
operator*(vec2<T> B, T A)
{
    vec2<T> Result = A*B;

    return(Result);
}

template <typename T, typename C>
inline vec2<T>
Hadamard(vec2<T> A, vec2<C> B)
{
    vec2<T> Result = {A.x*B.x, A.y*B.y};

    return(Result);
}

template <typename T, typename C>
inline r32
Inner(vec2<T> A, vec2<C> B)
{
    r32 Result = A.x*B.x + A.y*B.y;

    return(Result);
}

template <typename T>
inline r32
LengthSq(vec2<T> A)
{
    r32 Result = Inner(A, A);

    return(Result);
}

template <typename T>
inline r32
Length(vec2<T> A)
{
    r32 Result = square_root(LengthSq(A));
    return(Result);
}

template <typename T>
inline vec2<T>
Normalize(vec2<T> A)
{
    vec2<T> Result = A * (1.0f / Length(A));

    return(Result);
}

template <typename T>
inline vec2<T>
Clamp01(vec2<T> Value)
{
    vec2<T> Result;

    Result.x = Clamp01(Value.x);
    Result.y = Clamp01(Value.y);

    return(Result);
}

inline v2
Arm2(r32 Angle)
{
    v2 Result = {cos(Angle), sin(Angle)};

    return(Result);
}

template <typename T>
inline vec2<T> Rotate(vec2<T> vec, r32 angle)
{
    r32 ca = cos(angle);
    r32 sa = sin(angle);
    vec2<T> result = {ca * vec.x - sa * vec.y, sa * vec.x + ca * vec.y};
    return result;
}

template <typename T, typename C>
inline vec2<T>
Lerp(vec2<T> A, real32 t, vec2<C> B)
{
    vec2<T> Result = (1.0f - t)*A + t*B;

    return(Result);
}

//
// NOTE(casey): v3 operations
//

template <typename T>
inline vec3<T>
operator*(T A, vec3<T> B)
{
    vec3<T> Result;

    Result.x = A*B.x;
    Result.y = A*B.y;
    Result.z = A*B.z;

    return(Result);
}

template <typename T, typename C>
inline vec3<T>
Hadamard(vec3<T> A, vec3<C> B)
{
    vec3<T> Result = {A.x*B.x, A.y*B.y, A.z*B.z};

    return(Result);
}

template <typename T, typename C>
inline r32
Inner(vec3<T> A, vec3<C> B)
{
    r32 Result = A.x*B.x + A.y*B.y + A.z*B.z;

    return(Result);
}

template <typename T>
inline vec3<T>
Cross(vec3<T> A, vec3<T> B)
{
    vec3<T> result = {(A.y*B.z - A.z*B.y), (A.z*B.x - A.x*B.z), (A.x*B.y - A.y*B.x)};

    return result;
}

template <typename T>
inline r32
LengthSq(vec3<T> A)
{
    r32 Result = Inner(A, A);

    return(Result);
}

template <typename T>
inline r32
Length(vec3<T> A)
{
    r32 Result = square_root(LengthSq(A));
    return(Result);
}

template <typename T>
inline vec3<T>
Normalize(vec3<T> A)
{
    vec3<T> Result = A * (1.0f / Length(A));

    return(Result);
}

template <typename T>
inline vec3<T>
Clamp01(vec3<T> Value)
{
    vec3<T> Result;

    Result.x = Clamp01(Value.x);
    Result.y = Clamp01(Value.y);
    Result.z = Clamp01(Value.z);

    return(Result);
}

template <typename T, typename C>
inline vec3<T>
Lerp(vec3<T> A, real32 t, vec3<C> B)
{
    vec3<T> Result = (1.0f - t)*A + t*B;

    return(Result);
}

//
// NOTE(casey): v4 operations
//

template <typename T>
inline vec4<T>
operator*(T A, vec4<T> B)
{
    vec4<T> Result;

    Result.x = A*B.x;
    Result.y = A*B.y;
    Result.z = A*B.z;
    Result.w = A*B.w;

    return(Result);
}

template <typename T, typename C>
inline vec4<T>
Hadamard(vec4<T> A, vec4<C> B)
{
    vec4<T> Result = {A.x*B.x, A.y*B.y, A.z*B.z, A.w*B.w};

    return(Result);
}

template <typename T, typename C>
inline r32
Inner(vec4<T> A, vec4<C> B)
{
    r32 Result = A.x*B.x + A.y*B.y + A.z*B.z + A.w*B.w;

    return(Result);
}

template <typename T>
inline r32
LengthSq(vec4<T> A)
{
    r32 Result = Inner(A, A);

    return(Result);
}

template <typename T>
inline T
Length(vec4<T> A)
{
    real32 Result = square_root(LengthSq(A));
    return(Result);
}

template <typename T>
inline vec4<T>
Clamp01(vec4<T> Value)
{
    vec4<T> Result;

    Result.x = Clamp01(Value.x);
    Result.y = Clamp01(Value.y);
    Result.z = Clamp01(Value.z);
    Result.w = Clamp01(Value.w);

    return(Result);
}

template <typename T, typename C>
inline vec4<T>
Lerp(vec4<T> A, real32 t, vec4<C> B)
{
    vec4<T> Result = (1.0f - t)*A + t*B;

    return(Result);
}

//
// NOTE(casey): Rectangle2
//
/*
inline rectangle2
InvertedInfinityRectangle2(void)
{
    rectangle2 Result;

    Result.Min.x = Result.Min.y = Real32Maximum;
    Result.Max.x = Result.Max.y = -Real32Maximum;

    return(Result);
}

inline rectangle2
Union(rectangle2 A, rectangle2 B)
{
    rectangle2 Result;

    Result.Min.x = (A.Min.x < B.Min.x) ? A.Min.x : B.Min.x;
    Result.Min.y = (A.Min.y < B.Min.y) ? A.Min.y : B.Min.y;
    Result.Max.x = (A.Max.x > B.Max.x) ? A.Max.x : B.Max.x;
    Result.Max.y = (A.Max.y > B.Max.y) ? A.Max.y : B.Max.y;

    return(Result);
}

inline v2
GetMinCorner(rectangle2 Rect)
{
    v2 Result = Rect.Min;
    return(Result);
}

inline v2
GetMaxCorner(rectangle2 Rect)
{
    v2 Result = Rect.Max;
    return(Result);
}

inline v2
GetDim(rectangle2 Rect)
{
    v2 Result = Rect.Max - Rect.Min;
    return(Result);
}

inline v2
GetCenter(rectangle2 Rect)
{
    v2 Result = 0.5f*(Rect.Min + Rect.Max);
    return(Result);
}

inline rectangle2
RectMinMax(v2 Min, v2 Max)
{
    rectangle2 Result;

    Result.Min = Min;
    Result.Max = Max;

    return(Result);
}

inline rectangle2
RectMinDim(v2 Min, v2 Dim)
{
    rectangle2 Result;

    Result.Min = Min;
    Result.Max = Min + Dim;

    return(Result);
}

inline rectangle2
RectCenterHalfDim(v2 Center, v2 HalfDim)
{
    rectangle2 Result;

    Result.Min = Center - HalfDim;
    Result.Max = Center + HalfDim;

    return(Result);
}

inline rectangle2
AddRadiusTo(rectangle2 A, v2 Radius)
{
    rectangle2 Result;
    Result.Min = A.Min - Radius;
    Result.Max = A.Max + Radius;

    return(Result);
}

inline rectangle2
Offset(rectangle2 A, v2 Offset)
{
    rectangle2 Result;

    Result.Min = A.Min + Offset;
    Result.Max = A.Max + Offset;

    return(Result);
}

inline rectangle2
RectCenterDim(v2 Center, v2 Dim)
{
    rectangle2 Result = RectCenterHalfDim(Center, 0.5f*Dim);

    return(Result);
}

inline bool32
IsInRectangle(rectangle2 Rectangle, v2 Test)
{
    bool32 Result = ((Test.x >= Rectangle.Min.x) &&
                     (Test.y >= Rectangle.Min.y) &&
                     (Test.x < Rectangle.Max.x) &&
                     (Test.y < Rectangle.Max.y));

    return(Result);
}

inline v2
GetBarycentric(rectangle2 A, v2 P)
{
    v2 Result;

    Result.x = SafeRatio0(P.x - A.Min.x, A.Max.x - A.Min.x);
    Result.y = SafeRatio0(P.y - A.Min.y, A.Max.y - A.Min.y);

    return(Result);
}

//
// NOTE(casey): Rectangle3
//

inline v3
GetMinCorner(rectangle3 Rect)
{
    v3 Result = Rect.Min;
    return(Result);
}

inline v3
GetMaxCorner(rectangle3 Rect)
{
    v3 Result = Rect.Max;
    return(Result);
}

inline v3
GetDim(rectangle3 Rect)
{
    v3 Result = Rect.Max - Rect.Min;
    return(Result);
}

inline v3
GetCenter(rectangle3 Rect)
{
    v3 Result = 0.5f*(Rect.Min + Rect.Max);
    return(Result);
}

inline rectangle3
RectMinMax(v3 Min, v3 Max)
{
    rectangle3 Result;

    Result.Min = Min;
    Result.Max = Max;

    return(Result);
}

inline rectangle3
RectMinDim(v3 Min, v3 Dim)
{
    rectangle3 Result;

    Result.Min = Min;
    Result.Max = Min + Dim;

    return(Result);
}

inline rectangle3
RectCenterHalfDim(v3 Center, v3 HalfDim)
{
    rectangle3 Result;

    Result.Min = Center - HalfDim;
    Result.Max = Center + HalfDim;

    return(Result);
}

inline rectangle3
AddRadiusTo(rectangle3 A, v3 Radius)
{
    rectangle3 Result;

    Result.Min = A.Min - Radius;
    Result.Max = A.Max + Radius;

    return(Result);
}

inline rectangle3
Offset(rectangle3 A, v3 Offset)
{
    rectangle3 Result;

    Result.Min = A.Min + Offset;
    Result.Max = A.Max + Offset;

    return(Result);
}

inline rectangle3
RectCenterDim(v3 Center, v3 Dim)
{
    rectangle3 Result = RectCenterHalfDim(Center, 0.5f*Dim);

    return(Result);
}

inline bool32
IsInRectangle(rectangle3 Rectangle, v3 Test)
{
    bool32 Result = ((Test.x >= Rectangle.Min.x) &&
                     (Test.y >= Rectangle.Min.y) &&
                     (Test.z >= Rectangle.Min.z) &&
                     (Test.x < Rectangle.Max.x) &&
                     (Test.y < Rectangle.Max.y) &&
                     (Test.z < Rectangle.Max.z));

    return(Result);
}

inline bool32
RectanglesIntersect(rectangle3 A, rectangle3 B)
{
    bool32 Result = !((B.Max.x <= A.Min.x) ||
                      (B.Min.x >= A.Max.x) ||
                      (B.Max.y <= A.Min.y) ||
                      (B.Min.y >= A.Max.y) ||
                      (B.Max.z <= A.Min.z) ||
                      (B.Min.z >= A.Max.z));
    return(Result);
}

inline v3
GetBarycentric(rectangle3 A, v3 P)
{
    v3 Result;

    Result.x = SafeRatio0(P.x - A.Min.x, A.Max.x - A.Min.x);
    Result.y = SafeRatio0(P.y - A.Min.y, A.Max.y - A.Min.y);
    Result.z = SafeRatio0(P.z - A.Min.z, A.Max.z - A.Min.z);

    return(Result);
}

inline rectangle2
ToRectangleXY(rectangle3 A)
{
    rectangle2 Result;

    Result.Min = A.Min.xy;
    Result.Max = A.Max.xy;

    return(Result);
}

//
//
//

struct rectangle2i
{
    int32 MinX, MinY;
    int32 MaxX, MaxY;
};

inline rectangle2i
Intersect(rectangle2i A, rectangle2i B)
{
    rectangle2i Result;

    Result.MinX = (A.MinX < B.MinX) ? B.MinX : A.MinX;
    Result.MinY = (A.MinY < B.MinY) ? B.MinY : A.MinY;
    Result.MaxX = (A.MaxX > B.MaxX) ? B.MaxX : A.MaxX;
    Result.MaxY = (A.MaxY > B.MaxY) ? B.MaxY : A.MaxY;

    return(Result);
}

inline rectangle2i
Union(rectangle2i A, rectangle2i B)
{
    rectangle2i Result;

    Result.MinX = (A.MinX < B.MinX) ? A.MinX : B.MinX;
    Result.MinY = (A.MinY < B.MinY) ? A.MinY : B.MinY;
    Result.MaxX = (A.MaxX > B.MaxX) ? A.MaxX : B.MaxX;
    Result.MaxY = (A.MaxY > B.MaxY) ? A.MaxY : B.MaxY;

    return(Result);
}

inline int32
GetClampedRectArea(rectangle2i A)
{
    int32 Width = (A.MaxX - A.MinX);
    int32 Height = (A.MaxY - A.MinY);
    int32 Result = 0;
    if((Width > 0) && (Height > 0))
    {
        Result = Width*Height;
    }

    return(Result);
}

inline bool32
HasArea(rectangle2i A)
{
    bool32 Result = ((A.MinX < A.MaxX) && (A.MinY < A.MaxY));

    return(Result);
}

inline rectangle2i
InvertedInfinityRectangle2i(void)
{
    rectangle2i Result;

    Result.MinX = Result.MinY = INT_MAX;
    Result.MaxX = Result.MaxY = -INT_MAX;

    return(Result);
}*/

inline v4
SRGB255ToLinear1(v4 C)
{
    v4 Result;

    real32 Inv255 = 1.0f / 255.0f;

    Result.x = Square(Inv255*C.x);
    Result.y = Square(Inv255*C.y);
    Result.z = Square(Inv255*C.z);
    Result.w = Inv255*C.w;

    return(Result);
}

inline v4
Linear1ToSRGB255(v4 C)
{
    v4 Result;

    real32 One255 = 255.0f;

    Result.x = One255*square_root(C.x);
    Result.y = One255*square_root(C.y);
    Result.z = One255*square_root(C.z);
    Result.w = One255*C.w;

    return(Result);
}

#endif // MATH_H
