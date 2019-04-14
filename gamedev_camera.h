#ifndef GD_CAMERA_H
#define GD_CAMERA_H

struct Camera
{
    i32 maxX;
    i32 maxY;
    i32 yPixelMovementThreshold;
    i32 xPixelMovementThreshold;
    SDL_Rect viewport;
    Point startingPos;
};
#endif
