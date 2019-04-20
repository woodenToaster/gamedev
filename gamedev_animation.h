#ifndef GD_ANIMATION_H
#define GD_ANIMATION_H

struct Animation
{
    int totalFrames;
    int currentFrame;
    bool32 skipFrame;
    int frameToSkip;
    u32 delay;
    u32 elapsed;
};
#endif
