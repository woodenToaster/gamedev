#ifndef GD_CAMERA_H
#define GD_CAMERA_H

struct Camera
{
    i32 max_x;
    i32 max_y;
    i32 y_pixel_movement_threshold;
    i32 x_pixel_movement_threshold;
    SDL_Rect viewport;
    Point starting_pos;
};
#endif
