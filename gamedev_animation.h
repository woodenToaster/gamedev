#ifndef GD_ANIMATION_H
#define GD_ANIMATION_H

struct Animation
{
    int total_frames;
    int current_frame;
    u32 delay;
    u32 elapsed;
};
#endif