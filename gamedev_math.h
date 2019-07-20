#ifndef GAMEDEV_MATH_H
#define GAMEDEV_MATH_H

const f32 PI = 3.14159f;

struct Point
{
    i32 x;
    i32 y;
};

struct Vec2
{
    f32 x;
    f32 y;
};

struct Circle
{
    Vec2 center;
    f32 radius;
};

struct Rect
{
    int x;
    int y;
    int w;
    int h;
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

bool32 isClose(f32 a, f32 b, f32 tol=0.001f)
{
    f32 diff = fabsf(a - b);
    bool32 result = diff < tol;
    return result;
}

// One piece of a circle split in 8 sections
// The radians start at 2*PI on (1, 0) and go to zero counter-clockwise
Direction get_direction_from_angle(f32 angle)
{
    f32 direction_increment = (2.0f * PI) / 8.0f;
    f32 half_increment = 0.5f * direction_increment;
    Direction result;

    if (angle >= (3.0f * PI) / 2.0f - half_increment &&
        angle < (3.0f * PI) / 2.0f + half_increment)
    {
        result = DIR_UP;
    }
    else if (angle >= (3.0f * PI) / 2.0f + half_increment &&
             angle < 2.0f * PI - half_increment)
    {
        result = DIR_UP_RIGHT;
    }
    else if (angle >= 2.0f * PI - half_increment ||
             angle < half_increment)
    {
        result = DIR_RIGHT;
    }
    else if (angle >= half_increment &&
             angle < PI / 2.0f - half_increment)
    {
        result = DIR_DOWN_RIGHT;
    }
    else if (angle >= PI / 2.0f - half_increment &&
             angle < PI / 2.0f + half_increment)
    {
        result = DIR_DOWN;
    }
    else if (angle >= PI / 2.0f + half_increment &&
             angle < PI - half_increment)
    {
        result = DIR_DOWN_LEFT;
    }
    else if (angle >= PI - half_increment &&
             angle < PI + half_increment)
    {
        result = DIR_LEFT;
    }
    else if (angle >= PI + half_increment &&
             angle < (3.0f * PI) / half_increment)
    {
        result = DIR_UP_LEFT;
    }
    else
    {
        result = DIR_DOWN;
    }
    return result;
}

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

bool32 pointIsInRect(Point point, Rect *rect)
{
    Rect pointRect = {};
    pointRect.x = point.x;
    pointRect.y = point.y;
    return rectsOverlap(&pointRect, rect);
}

bool32 positionIsInRect(Vec2 pos, Rect *rect)
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

bool32 lineIntersectsCircle(Point a, Point b, Circle *c)
{
    Vec2 u = {(f32)a.x, (f32)a.y};
    Vec2 v = {(f32)b.x, (f32)b.y};
    Vec2 p = {c->center.x, c->center.y};
    return minimumDistanceBetweenPointAndLine(u, v, p) <= c->radius;
}

bool32 circleOverlapsRect(Circle *circle, Rect *rect)
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
