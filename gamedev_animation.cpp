#include "gamedev_animation.h"

void initAnimation(Animation* a, int frames, int ms_delay)
{
    a->total_frames = frames;
    a->delay = ms_delay;
}

void updateAnimation(Animation* a, u32 elapsed_last_frame, bool32 active)
{
    a->elapsed += elapsed_last_frame;
    if (a->elapsed > a->delay && active) {
        a->current_frame++;
        if (a->current_frame > a->total_frames - 1) {
            a->current_frame = 0;
        }
        a->elapsed = 0;
    }
}
