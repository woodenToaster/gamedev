#ifndef GAMEDEV_H
#define GAMEDEV_H

#include "gamedev_platform.h"


enum Direction
{
    DIR_UP,
    DIR_UP_RIGHT,
    DIR_RIGHT,
    DIR_DOWN_RIGHT,
    DIR_DOWN,
    DIR_DOWN_LEFT,
    DIR_LEFT,
    DIR_UP_LEFT,
    DIR_COUNT
};

struct Camera
{
    i32 maxX;
    i32 maxY;
    i32 yPixelMovementThreshold;
    i32 xPixelMovementThreshold;
    Rect viewport;
    Point startingPos;
};

struct Animation
{
    int totalFrames;
    int currentFrame;
    u32 delay;
    u32 elapsed;
};
#endif
