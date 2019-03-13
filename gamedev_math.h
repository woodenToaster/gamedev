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

Vec2 vec2(f32 x, f32 y)
{
    Vec2 v = {x, y};
    return v;
}

f32 vec2_magnitude(Vec2* v)
{
    return sqrtf(powf(v->x, 2) + powf(v->y, 2));
}

f32 vec2_dot(Vec2* v1, Vec2* v2)
{
    return v1->x * v2->x + v1->y * v2->y;
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

i32 max(i32 a, i32 b)
{
    return a > b ? a : b;
}

i32 min(i32 a, i32 b)
{
    return a < b ? a : b;
}

i32 clamp(i32 val, i32 min, i32 max)
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

bool overlaps(SDL_Rect* r1, SDL_Rect* r2)
{
    bool x_overlap = r1->x + r1->w > r2->x && r1->x < r2->x + r2->w;
    bool y_overlap = r1->y + r1->h > r2->y && r1->y < r2->y + r2->h;
    return x_overlap && y_overlap;
}

bool32 pointInRect(Point point, SDL_Rect *rect)
{
    SDL_Rect pointRect = {};
    pointRect.x = point.x;
    pointRect.y = point.y;
    return overlaps(&pointRect, rect)
}
#endif
