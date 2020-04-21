#ifndef VECTOR_H
#define VECTOR_H

template <typename T>
struct vec2
{
    vec2() : x(0), y(0) {}
    template <typename X>
    vec2(const vec2<X> &a) : x(a.x), y(a.y) {}
    vec2(T x, T y) : x(x), y(y) {}
    inline vec2 operator*(T a) const
    {
        vec2 result;
        result.x = x * a;
        result.y = y * a;
        return result;
    }
    inline vec2 operator*(vec2<T> a) const
    {
        vec2 result;
        result.x = x * a.x;
        result.y = y * a.y;
        return result;
    }
    inline vec2 &operator*=(T a)
    {
        x *= a;
        y *= a;
        return *this;
    }
    inline vec2 operator/(T a) const
    {
        vec2 result;
        result.x = x / a;
        result.y = y / a;
        return result;
    }
    inline vec2 &operator/=(T a)
    {
        x /= a;
        y /= a;
        return *this;
    }
    inline vec2 operator-()
    {
        vec2 result;
        result.x = -x;
        result.y = -y;
        return result;
    }
    template <typename C>
    inline vec2 &operator=(vec2<C> a)
    {
        x = a.x;
        y = a.y;
        return *this;
    }
    template <typename C>
    inline vec2 operator+(vec2<C> a) const
    {
        vec2 result;
        result.x = x + a.x;
        result.y = y + a.y;
        return result;
    }
    template <typename C>
    inline vec2 &operator+=(vec2<C> a)
    {
        x += a.x;
        y += a.y;
        return *this;
    }
    template <typename C>
    inline vec2 operator-(vec2<C> a) const
    {
        vec2 result;
        result.x = x - a.x;
        result.y = y - a.y;
        return result;
    }
    template <typename C>
    inline vec2 &operator-=(vec2<C> a)
    {
        x -= a.x;
        y -= a.y;
        return *this;
    }
    template <typename C>
    inline bool operator==(vec2<C> a) const
    {
        return (x == a.x && y == a.y);
    }
    template <typename C>
    inline bool operator!=(vec2<C> a) const
    {
        return !(*this == a);
    }

    T x, y;
};

typedef vec2<r32> v2;
typedef vec2<s32> ivec2;
typedef vec2<f32> fvec2;

template <typename T>
struct vec3
{
    vec3() : x(0), y(0), z(0) {}
    template <typename X>
    vec3(const vec3<X> &a) : x(a.x), y(a.y), z(a.z) {}
    vec3(T x, T y, T z) : x(x), y(y), z(z) {}
    inline vec3 operator*(T a) const
    {
        vec3 result;
        result.x = x * a;
        result.y = y * a;
        result.z = z * a;
        return result;
    }
    inline vec3 &operator*=(T a)
    {
        x *= a;
        y *= a;
        z *= a;
        return *this;
    }
    inline vec3 operator/(T a) const
    {
        vec3 result;
        result.x = x / a;
        result.y = y / a;
        result.z = z / a;
        return result;
    }
    inline vec3 &operator/=(T a)
    {
        x /= a;
        y /= a;
        z /= a;
        return *this;
    }
    inline vec3 operator-() const
    {
        vec3 result;
        result.x = -x;
        result.y = -y;
        result.z = -z;
        return result;
    }
    template <typename C>
    inline vec3 &operator=(vec3<C> a) const
    {
        x = a.x;
        y = a.y;
        z = a.z;
        return *this;
    }
    template <typename C>
    inline vec3 operator+(vec3<C> a) const
    {
        vec3 result;
        result.x = x + a.x;
        result.y = y + a.y;
        result.z = z + a.z;
        return result;
    }
    template <typename C>
    inline vec3 &operator+=(vec3<C> a)
    {
        x += a.x;
        y += a.y;
        z += a.z;
        return *this;
    }
    template <typename C>
    inline vec3 operator-(vec3<C> a) const
    {
        vec3 result;
        result.x = x - a.x;
        result.y = y - a.y;
        result.z = z - a.z;
        return result;
    }
    template <typename C>
    inline vec3 &operator-=(vec3<C> a)
    {
        x -= a.x;
        y -= a.y;
        z -= a.z;
        return *this;
    }
    template <typename C>
    inline bool operator==(vec3<C> a) const
    {
        return (x == a.x && y == a.y && z == a.z);
    }
    template <typename C>
    inline bool operator!=(vec3<C> a) const
    {
        return !(*this == a);
    }

    T x, y, z;
    vec2<T> &xy()
    {
        return *(vec2<T>*)&x;
    }
    vec2<T> &yz()
    {
        return *(vec2<T>*)&y;
    }
};

typedef vec3<r32> v3;
typedef vec3<s32> ivec3;
typedef vec3<f32> fvec3;

template <typename T>
struct vec4
{
    vec4() : x(0), y(0), z(0), w(0) {}
    template <typename X>
    vec4(const vec4<X> &a) : x(a.x), y(a.y), z(a.z), w(a.w) {}
    vec4(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) {}
    inline vec4 operator*(T a) const
    {
        vec4 result;
        result.x = x * a;
        result.y = y * a;
        result.z = z * a;
        result.w = w * a;
        return result;
    }
    inline vec4 &operator*=(T a)
    {
        x *= a;
        y *= a;
        z *= a;
        w *= a;
        return *this;
    }
    inline vec4 operator/(T a) const
    {
        vec4 result;
        result.x = x / a;
        result.y = y / a;
        result.z = z / a;
        result.w = w / a;
        return result;
    }
    inline vec4 &operator/=(T a)
    {
        x /= a;
        y /= a;
        z /= a;
        w /= a;
        return *this;
    }
    inline vec4 operator-() const
    {
        vec4 result;
        result.x = -x;
        result.y = -y;
        result.z = -z;
        result.w = -w;
        return result;
    }
    template <typename C>
    inline vec4 &operator=(vec4<C> a) const
    {
        x = a.x;
        y = a.y;
        z = a.z;
        w = a.w;
        return *this;
    }
    template <typename C>
    inline vec4 operator+(vec4<C> a) const
    {
        vec4 result;
        result.x = x + a.x;
        result.y = y + a.y;
        result.z = z + a.z;
        result.w = w + a.w;
        return result;
    }
    template <typename C>
    inline vec4 &operator+=(vec4<C> a)
    {
        x += a.x;
        y += a.y;
        z += a.z;
        w += a.w;
        return *this;
    }
    template <typename C>
    inline vec4 operator-(vec4<C> a) const
    {
        vec4 result;
        result.x = x - a.x;
        result.y = y - a.y;
        result.z = z - a.z;
        result.w = w - a.w;
        return result;
    }
    template <typename C>
    inline vec4 &operator-=(vec4<C> a)
    {
        x -= a.x;
        y -= a.y;
        z -= a.z;
        w -= a.w;
        return *this;
    }
    template <typename C>
    inline bool operator==(vec4<C> a) const
    {
        return (x == a.x && y == a.y && z == a.z && w == a.w);
    }
    template <typename C>
    inline bool operator!=(vec4<C> a) const
    {
        return !(*this == a);
    }

    T x, y, z, w;
    vec3<T> &xyz()
    {
        return *(vec3<T>*)&x;
    }
    vec2<T> &xy()
    {
        return *(vec2<T>*)&x;
    }
    vec2<T> &yz()
    {
        return *(vec2<T>*)&y;
    }
    vec2<T> &zw()
    {
        return *(vec2<T>*)&z;
    }
};

typedef vec4<r32> v4;
typedef vec4<s32> ivec4;
typedef vec4<f32> fvec4;

template <typename T>
std::ostream& operator<< (std::ostream& stream, const vec2<T> v)
{
    return stream << v.x << ", " << v.y;
}

template <typename T>
std::ostream& operator<< (std::ostream& stream, const vec3<T> v)
{
    return stream << v.x << ", " << v.y << ", " << v.z;
}

template <typename T>
std::ostream& operator<< (std::ostream& stream, const vec4<T> v)
{
    return stream << v.x << ", " << v.y << ", " << v.z << ", " << v.w;
}

#endif // VECTOR_H
