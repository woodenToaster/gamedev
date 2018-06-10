#ifndef GAMEDEV_MATH_H
#define GAMEDEV_MATH_H

const float PI = 3.14159f;

struct Point
{
    i32 x;
    i32 y;
};

struct Vec2
{
    float x;
    float y;
};

Vec2 vec2(float x, float y)
{
    Vec2 v = {x, y};
    return v;
}

float vec2_magnitude(Vec2* v)
{
    return sqrtf(powf(v->x, 2) + powf(v->y, 2));
}

float vec2_dot(Vec2* v1, Vec2* v2)
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

int max(int a, int b)
{
    return a > b ? a : b;
}

int min(int a, int b)
{
    return a < b ? a : b;
}

int clamp(int val, int min, int max)
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
Direction get_direction_from_angle(float angle)
{
    float direction_increment = (2.0f * PI) / 8.0f;
    float half_increment = 0.5f * direction_increment;
    Direction result;

    if (angle >= (3.0f * PI) / 2.0f - half_increment &&
        angle < (3.0f * PI) / 2.0f + half_increment)
    {
        result = UP;
    }
    else if (angle >= (3.0f * PI) / 2.0f + half_increment &&
             angle < 2.0f * PI - half_increment)
    {
        result = UP_RIGHT;
    }
    else if (angle >= 2.0f * PI - half_increment ||
             angle < half_increment)
    {
        result = RIGHT;
    }
    else if (angle >= half_increment &&
             angle < PI / 2.0f - half_increment)
    {
        result = DOWN_RIGHT;
    }
    else if (angle >= PI / 2.0f - half_increment &&
             angle < PI / 2.0f + half_increment)
    {
        result = DOWN;
    }
    else if (angle >= PI / 2.0f + half_increment &&
             angle < PI - half_increment)
    {
        result = DOWN_LEFT;
    }
    else if (angle >= PI - half_increment &&
             angle < PI + half_increment)
    {
        result = LEFT;
    }
    else if (angle >= PI + half_increment &&
             angle < (3.0f * PI) / half_increment)
    {
        result = UP_LEFT;
    }
    else
    {
        result = DOWN;
    }
    return result;
}

f32 square(f32 x)
{
    return x * x;
}

#endif
