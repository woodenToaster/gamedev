#ifndef GAMEDEV_MATH_H
#define GAMEDEV_MATH_H

#undef near
#undef far

#include <math.h>

// TODO(chogan): More digits
const f32 PI = 3.14159f;

struct Rect
{
    int x;
    int y;
    int w;
    int h;
};

inline b32 isZeroRect(Rect rect)
{
    return !(rect.x || rect.y || rect.w || rect.h);
}


struct Point
{
    i32 x;
    i32 y;
};

#pragma warning(disable:4201)
union Vec2
{
    struct
    {
        f32 x;
        f32 y;
    };

    f32 data[2];
};

struct Rect2
{
    Vec2 minP;
    Vec2 maxP;
};

union Vec3
{
    struct
    {
        f32 x;
        f32 y;
        f32 z;
    };

    struct
    {
        f32 r;
        f32 g;
        f32 b;
    };

    f32 data[3];
};

struct Vec3u8
{
    u8 r;
    u8 g;
    u8 b;
};

union Vec4
{
    struct
    {
        f32 x;
        f32 y;
        f32 z;
        f32 w;
    };

    struct
    {
        f32 r;
        f32 g;
        f32 b;
        f32 a;
    };

    f32 data[4];
};

union Mat4
{
    struct
    {
        Vec4 col1;
        Vec4 col2;
        Vec4 col3;
        Vec4 col4;
    };

    f32 data[16];
};
#pragma warning(default:4201)

inline f32 radians(f32 angleInDegrees)
{
    return angleInDegrees * PI / 180.0f;
}

Mat4 identityMat4()
{
    Mat4 result = {};
    result.col1.x = 1.0f;
    result.col2.y = 1.0f;
    result.col3.z = 1.0f;
    result.col4.w = 1.0f;

    return result;
}

Mat4 makeTranslationMat4(f32 x, f32 y, f32 z)
{
    Mat4 result = {};
    result.col1 = {1.0f, 0.0f, 0.0f, 0.0f};
    result.col2 = {0.0f, 1.0f, 0.0f, 0.0f};
    result.col3 = {0.0f, 0.0f, 1.0f, 0.0f};
    result.col4 = {x, y, z, 1.0f};

    return result;
}

Mat4 makeTranslationMat4(Vec4 v)
{
    Mat4 result = makeTranslationMat4(v.x, v.y, v.z);

    return result;
}

Mat4 makeRotationMat4(f32 angle, f32 x, f32 y, f32 z)
{
    Mat4 result = {};

    f32 x2 = x * x;
    f32 y2 = y * y;
    f32 z2 = z * z;
    f32 rads = f32(angle) * 0.0174532925f;
    f32 c = cosf(rads);
    f32 s = sinf(rads);
    f32 omc = 1.0f - c;

    result.col1 = {x2 * omc + c, y * x * omc + z * s, x * z * omc - y * s, 0.0f};
    result.col2 = {x * y * omc - z * s, y2 * omc + c, y * z * omc + x * s, 0.0f};
    result.col3 = {x * z * omc + y * s, y * z * omc - x * s, z2 * omc + c, 0.0f};
    result.col4 = {0.0f, 0.0f, 0.0f, 1.0f};

    return result;
}

Mat4 makePerspectiveMat4(f32 fovy, f32 aspect, f32 n, f32 f)
{
    Mat4 result = {};

    f32 q = 1.0f / tanf(radians(0.5f * fovy));
    f32 A = q / aspect;
    f32 B = (n + f) / (n - f);
    f32 C = (2.0f * n * f) / (n - f);

    result.col1 = {A, 0.0f, 0.0f, 0.0f};
    result.col2 = {0.0f, q, 0.0f, 0.0f};
    result.col3 = {0.0f, 0.0f, B, -1.0f};
    result.col4 = {0.0f, 0.0f, C, 0.0f};

    return result;
}

Mat4 makeFrustum(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far)
{
    Mat4 result = {};

    f32 w = right - left;
    f32 h = top - bottom;

    result.col1 = {(2.0f * near) / w, 0.0f, 0.0f, 0.0f};
    result.col2 = {0, (2.0f * near) / h, 0, 0};
    result.col3 = {(2 * (left + right)) / w, (2 * (top + bottom)) / h,
                       -((far + near) / (far - near)), -1.0f};
    result.col4 = {0, 0, 2 * far * near / (far - near), 0};

    return result;
}

// Mat4 lookAt(Vec3 position, Vec3 target, Vec3 up)
// {
    //
// }

Mat4 makeScaleMat4(f32 x, f32 y, f32 z)
{
    Mat4 result = {};
    result.col1.x = x;
    result.col2.y = y;
    result.col3.z = z;
    result.col4.w = 1.0f;

    return result;
}

Mat4 multiplyMat4(Mat4 *m1, Mat4 *m2)
{
    Mat4 result = {};

    result.col1 =
    {
        (m1->col1.x * m2->col1.x +
         m1->col2.x * m2->col1.y +
         m1->col3.x * m2->col1.z +
         m1->col4.x * m2->col1.w),
        (m1->col1.y * m2->col1.x +
         m1->col2.y * m2->col1.y +
         m1->col3.y * m2->col1.z +
         m1->col4.y * m2->col1.w),
        (m1->col1.z * m2->col1.x +
         m1->col2.z * m2->col1.y +
         m1->col3.z * m2->col1.z +
         m1->col4.z * m2->col1.w),
        (m1->col1.w * m2->col1.x +
         m1->col2.w * m2->col1.y +
         m1->col3.w * m2->col1.z +
         m1->col4.w * m2->col1.w)
    };
    result.col2 =
    {
        (m1->col1.x * m2->col2.x +
         m1->col2.x * m2->col2.y +
         m1->col3.x * m2->col2.z +
         m1->col4.x * m2->col2.w),
        (m1->col1.y * m2->col2.x +
         m1->col2.y * m2->col2.y +
         m1->col3.y * m2->col2.z +
         m1->col4.y * m2->col2.w),
        (m1->col1.z * m2->col2.x +
         m1->col2.z * m2->col2.y +
         m1->col3.z * m2->col2.z +
         m1->col4.z * m2->col2.w),
        (m1->col1.w * m2->col2.x +
         m1->col2.w * m2->col2.y +
         m1->col3.w * m2->col2.z +
         m1->col4.w * m2->col2.w)
    };
    result.col3 =
    {
        (m1->col1.x * m2->col3.x +
         m1->col2.x * m2->col3.y +
         m1->col3.x * m2->col3.z +
         m1->col4.x * m2->col3.w),
        (m1->col1.y * m2->col3.x +
         m1->col2.y * m2->col3.y +
         m1->col3.y * m2->col3.z +
         m1->col4.y * m2->col3.w),
        (m1->col1.z * m2->col3.x +
         m1->col2.z * m2->col3.y +
         m1->col3.z * m2->col3.z +
         m1->col4.z * m2->col3.w),
        (m1->col1.w * m2->col3.x +
         m1->col2.w * m2->col3.y +
         m1->col3.w * m2->col3.z +
         m1->col4.w * m2->col3.w)
    };
    result.col4 =
    {
        (m1->col1.x * m2->col4.x +
         m1->col2.x * m2->col4.y +
         m1->col3.x * m2->col4.z +
         m1->col4.x * m2->col4.w),
        (m1->col1.y * m2->col4.x +
         m1->col2.y * m2->col4.y +
         m1->col3.y * m2->col4.z +
         m1->col4.y * m2->col4.w),
        (m1->col1.z * m2->col4.x +
         m1->col2.z * m2->col4.y +
         m1->col3.z * m2->col4.z +
         m1->col4.z * m2->col4.w),
        (m1->col1.w * m2->col4.x +
         m1->col2.w * m2->col4.y +
         m1->col3.w * m2->col4.z +
         m1->col4.w * m2->col4.w)
    };

    return result;
}

struct Vec4u8
{
    u8 r;
    u8 g;
    u8 b;
    u8 a;
};

struct Circle
{
    Vec2 center;
    f32 radius;
};

Vec2 vec2(f32 x, f32 y)
{
    Vec2 v = {x, y};
    return v;
}

f32 vec2_magnitude(Vec2* v)
{
    return sqrtf(powf(v->x, 2) + powf(v->y, 2));
}

f32 vec2_distance(Vec2 *v1, Vec2 *v2)
{
    f32 dx = v2->x - v1->x;
    f32 dy = v2->y - v1->y;
    return sqrtf(dx * dx + dy * dy);
}

f32 vec2_dot(Vec2* v1, Vec2* v2)
{
    return v1->x * v2->x + v1->y * v2->y;
}

Vec2 vec2_abs(Vec2 *v)
{
    Vec2 result = {};
    result.x = v->x < 0 ? -1 * v->x : v->x;
    result.y = v->y < 0 ? -1 * v->y : v->y;
    return result;
}

Vec2 operator*(Vec2 v1, Vec2 v2)
{
    Vec2 result = {};
    result.x = v1.x * v2.x;
    result.y = v1.y * v2.y;
    return result;
}

Vec2 operator+(Vec2 v1, Vec2 v2)
{
    Vec2 result = {};
    result.x = v1.x + v2.x;
    result.y = v1.y + v2.y;

    return result;
}

Vec2& operator+=(Vec2& v1, Vec2 v2)
{
    v1 = v1 + v2;
    return v1;
}

Vec2 operator-(Vec2 v1, Vec2 v2)
{
    Vec2 result = {};
    result.x = v1.x - v2.x;
    result.y = v1.y - v2.y;
    return result;
}

Vec2& operator-=(Vec2& v1, Vec2 v2)
{
    v1 = v1 - v2;
    return v1;
}

Vec2 operator*(f32 s, Vec2 v)
{
    Vec2 result = {};
    result.x = v.x * s;
    result.y = v.y * s;

    return result;
}

Vec2 operator*(Vec2 v, f32 s)
{
    Vec2 result = s * v;
    return result;
}

Vec2& operator*=(Vec2& v1, f32 s)
{
    v1 = s * v1;

    return v1;
}

Vec3 vec3(f32 x, f32 y, f32 z)
{
    Vec3 v = {x, y, z};
    return v;
}

f32 magnitudeV3(Vec3* v)
{
    return sqrtf(powf(v->x, 2) + powf(v->y, 2) + powf(v->z, 2));
}

Vec3 crossV3(Vec3 *a, Vec3 *b)
{
    Vec3 result = {a->y * b->z - a->z * b->y,
                   a->z * b->x - a->x * b->z,
                   a->x * b->y - a->y * b->x};

    return result;
}

Vec3 normalizeV3(Vec3 *v)
{
    f32 length = magnitudeV3(v);
    Vec3 result = {v->x / length, v->y / length, v->z / length};

    return result;
}

Vec3 operator-(Vec3 v1, Vec3 v2)
{
    Vec3 result = {};
    result.x = v1.x - v2.x;
    result.y = v1.y - v2.y;
    result.z = v1.z - v2.z;

    return result;
}

Vec3 operator*(f32 s, Vec3 v)
{
    Vec3 result = {};
    result.x = v.x * s;
    result.y = v.y * s;
    result.z = v.z * s;

    return result;
}

Vec3& operator*=(Vec3& v1, f32 s)
{
    v1 = s * v1;

    return v1;
}

Vec3u8 vec3u8(u8 r, u8 g, u8 b)
{
    Vec3u8 v = {r, g, b};
    return v;
}

Vec4 vec4(f32 x, f32 y, f32 z, f32 w)
{
    Vec4 v = {x, y, z, w};
    return v;
}

Vec4u8 vec4u8(u8 r, u8 g, u8 b, u8 a)
{
    Vec4u8 v = {r, g, b, a};
    return v;
}

Vec4u8 vec4u8(Vec3u8 v3, u8 a)
{
    Vec4u8 v = {v3.r, v3.g, v3.b, a};
    return v;
}

i32 maxInt32(i32 a, i32 b)
{
    return a > b ? a : b;
}

i32 minInt32(i32 a, i32 b)
{
    return a < b ? a : b;
}

i32 maxUInt32(u32 a, u32 b)
{
    return a > b ? a : b;
}

i32 minUInt32(u32 a, u32 b)
{
    return a < b ? a : b;
}

i32 absInt32(i32 x)
{
    i32 result = x;
    if (x < 0)
    {
        result *= -1;
    }
    return result;
}

f32 maxFloat32(f32 a, f32 b)
{
    return a > b ? a : b;
}

f32 minFloat32(f32 a, f32 b)
{
    return a < b ? a : b;
}

f32 absFloat32(f32 x)
{
    f32 result = x;
    if (x < 0)
    {
        result *= -1;
    }
    return result;
}

i32 clampInt32(i32 val, i32 min, i32 max)
{
    if (val < min)
    {
        return min;
    }
    else if (val > max)
    {
        return max;
    }
    else
    {
        return val;
    }
}

u32 clampU32(u32 val, u32 min, u32 max)
{
    if (val < min)
    {
        return min;
    }
    else if (val > max)
    {
        return max;
    }
    else
    {
        return val;
    }
}

f32 clampFloat(f32 val, f32 min, f32 max)
{
    if (val < min)
    {
        return min;
    }
    else if (val > max)
    {
        return max;
    }
    else
    {
        return val;
    }
}

void clamp_size_t(size_t* val, size_t min, size_t max)
{
    if (*val < min)
    {
        *val = min;
    }
    else if (*val > max)
    {
        *val = max;
    }
}

b32 isClose(f32 a, f32 b, f32 tol=0.001f)
{
    f32 diff = fabsf(a - b);
    b32 result = diff < tol;
    return result;
}

// One piece of a circle split in 8 sections
// The radians start at 2*PI on (1, 0) and go to zero counter-clockwise
#if 0
Direction get_direction_from_angle(f32 angle)
{
    f32 direction_increment = (2.0f * PI) / 8.0f;
    f32 half_increment = 0.5f * direction_increment;
    Direction result;

    if (angle >= (3.0f * PI) / 2.0f - half_increment &&
        angle < (3.0f * PI) / 2.0f + half_increment)
    {
        result = Direction_Up;
    }
    else if (angle >= (3.0f * PI) / 2.0f + half_increment &&
             angle < 2.0f * PI - half_increment)
    {
        result = Direction_Up_Right;
    }
    else if (angle >= 2.0f * PI - half_increment ||
             angle < half_increment)
    {
        result = Direction_Right;
    }
    else if (angle >= half_increment &&
             angle < PI / 2.0f - half_increment)
    {
        result = Direction_Down_Right;
    }
    else if (angle >= PI / 2.0f - half_increment &&
             angle < PI / 2.0f + half_increment)
    {
        result = Direction_Down;
    }
    else if (angle >= PI / 2.0f + half_increment &&
             angle < PI - half_increment)
    {
        result = Direction_Down_Left;
    }
    else if (angle >= PI - half_increment &&
             angle < PI + half_increment)
    {
        result = Direction_Left;
    }
    else if (angle >= PI + half_increment &&
             angle < (3.0f * PI) / half_increment)
    {
        result = Direction_Up_Left;
    }
    else
    {
        result = Direction_Down;
    }
    return result;
}
#endif

f32 square(f32 x)
{
    return x * x;
}

Circle circle(Vec2 center, f32 radius)
{
    Circle result = {};
    result.center = center;
    result.radius = radius;
    return result;
}

bool rectsOverlap(Rect* r1, Rect* r2)
{
    bool x_overlap = r1->x + r1->w > r2->x && r1->x < r2->x + r2->w;
    bool y_overlap = r1->y + r1->h > r2->y && r1->y < r2->y + r2->h;
    return x_overlap && y_overlap;
}

b32 pointIsInRect(Point point, Rect *rect)
{
    Rect pointRect = {};
    pointRect.x = point.x;
    pointRect.y = point.y;
    return rectsOverlap(&pointRect, rect);
}

b32 positionIsInRect(Vec2 pos, Rect *rect)
{
    Point point = {(i32)pos.x, (i32)pos.y};
    return pointIsInRect(point, rect);
}

f32 minimumDistanceBetweenPointAndLine(Vec2 a, Vec2 b, Vec2 p) {
    // Return minimum distance between line segment ab and point p
    Vec2 diff = a - b;
    f32 l2 = vec2_magnitude(&diff) * vec2_magnitude(&diff);
    if (l2 == 0.0f)
    {
        return vec2_distance(&p, &a);
    }
    // Consider the line extending the segment, parameterized as v + t (w - v).
    // We find projection of point p onto the line.
    // It falls where t = [(p-v) . (w-v)] / |w-v|^2
    // We clamp t from [0,1] to handle points outside the segment vw.
    Vec2 pMinusA = p - a;
    Vec2 bMinusA = b - a;
    f32 t = maxFloat32(0, minFloat32(1, vec2_dot(&pMinusA, &bMinusA) / l2));
    Vec2 projection = a + t * (b - a);
    return vec2_distance(&p, &projection);
}

b32 lineIntersectsCircle(Point a, Point b, Circle *c)
{
    Vec2 u = {(f32)a.x, (f32)a.y};
    Vec2 v = {(f32)b.x, (f32)b.y};
    Vec2 p = {c->center.x, c->center.y};
    return minimumDistanceBetweenPointAndLine(u, v, p) <= c->radius;
}

b32 circleOverlapsRect(Circle *circle, Rect *rect)
{
    Point p = {};
    p.x = (i32)(circle->center.x + 0.5f);
    p.y = (i32)(circle->center.y + 0.5f);
    Point a = {rect->x, rect->y};
    Point b = {rect->x + rect->w, rect->y};
    Point c = {rect->x + rect->w, rect->y + rect->h};
    Point d = {rect->x, rect->y + rect->h};
    return (pointIsInRect(p, rect) ||
            lineIntersectsCircle(a, b, circle) ||
            lineIntersectsCircle(b, c, circle) ||
            lineIntersectsCircle(c, d, circle) ||
            lineIntersectsCircle(d, a, circle));
}
#endif
