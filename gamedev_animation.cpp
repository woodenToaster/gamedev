#include "gamedev_animation.h"

void initAnimation(Animation* a, int frames, int ms_delay)
{
    a->totalFrames = frames;
    a->delay = ms_delay;
}

void updateAnimation(Animation* a, u32 elapsed_last_frame, b32 active)
{
    a->elapsed += elapsed_last_frame;
    if (a->elapsed > a->delay && active) {
        a->currentFrame++;
        if (a->currentFrame > a->totalFrames - 1) {
            a->currentFrame = 0;
        }
        a->elapsed = 0;
    }
}
