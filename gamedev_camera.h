#ifndef GD_CAMERA_H
#define GD_CAMERA_H

struct Camera
{
    int max_x;
    int max_y;
    int y_pixel_movement_threshold;
    int x_pixel_movement_threshold;
    SDL_Rect viewport;
    SDL_Rect starting_pos;
};
#endif
