#ifndef UTIL_TRANSFORM_H
#define UTIL_TRANSFORM_H

#include <cmath>
#include "util/geometry.h"

///////////////////////////////////////////////////////////////////////////////
// class Matrix33
//================
//A 3x3 matrix, stored in column-major order. Intended for use by
//..class Transform
///////////////////////////////////////////////////////////////////////////////

class Matrix33;
inline Matrix33 operator*(float scalar, Matrix33 matrix);

class Matrix33
{
public:
    Matrix33();
    Matrix33(float a00, float a01, float a02,
             float a10, float a11, float a12,
             float a20, float a21, float a22);
    float getDeterminant() const;
    Matrix33 getAdjugate() const;
    Matrix33 getInverse() const;
    Matrix33 &invert();
    Matrix33 operator*(const Matrix33 &o) const;
    Matrix33 &operator*=(const Matrix33 &other);
    Vector2D<float> operator*(const Vector2D<float> &vec) const;
    Matrix33 operator*(float scalar) const;
private:
    //The matrix is in row major order
    float m_[3][3];
};

inline float det2x2(float a, float b, float c, float d)
{
    return a * d - b * c;
}

inline Matrix33 operator*(float scalar, Matrix33 matrix)
{
    return matrix * scalar;
}

///////////////////////////////////////////////////////////////////////////////
// class Transform
//=================
//A class encapsulating a linear transformation.
///////////////////////////////////////////////////////////////////////////////

class Transform
{
public:
    Transform();
    Transform operator+(const Transform &other) const;
    Transform &append(const Transform &other);
    Transform getInverse() const;
    Transform &invert();
    Vector2D<float> applyTo(Vector2D<float> source) const;
    Geometry<float> applyTo(const Geometry<float> &source) const;
protected:
    Transform(const Matrix33 &matrix, float angle, const Vector2D<float> &scale);
    Matrix33 matrix_;
    float angle_;
    Vector2D<float> scale_;
};

class Translation : public Transform
{
public:
    Translation(float x, float y);
    Translation(Vector2D<float> offset);
};

class Rotation : public Transform
{
public:
    Rotation(float angle);
};

class Scaling : public Transform
{
public:
    Scaling(float xScale, float yScale);
    Scaling(Vector2D<float> factors);
    Scaling(float factor);
};

#endif // UTIL_TRANSFORM_H
