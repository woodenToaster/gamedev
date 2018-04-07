#ifndef GAMEDEV_MATH_H
#define GAMEDEV_MATH_H

const float PI = 3.14159f;

struct Point
{
    float x;
    float y;

    Point() {}
    Point(float x, float y);
};

Point::Point(float x, float y): x(x), y(y) {}

struct Vec2
{
    float x;
    float y;
};

float vec2_magnitude(Vec2* v)
{
    return sqrtf(powf(v->x, 2) + powf(v->y, 2));
}

float vec2_dot(Vec2* v1, Vec2* v2)
{
    return v1->x * v2->x + v1->y * v2->y;
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
#endif
