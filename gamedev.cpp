#include "gamedev.h"

internal void setKeyState(Input* input, Key key, b32 isDown)
{
    if (input->keyDown[key] && !isDown)
    {
        input->keyPressed[key] = true;
    }
    input->keyDown[key] = isDown;
}

internal f32 normalizeStickInput(short unnormalizedStick)
{
    f32 result = 0.0f;
    if (unnormalizedStick < 0)
    {
        result = (f32)unnormalizedStick / 32768.0f;
    }
    else
    {
        result = (f32)unnormalizedStick / 32767.0f;
    }
    return result;
}

internal void initAnimation(Animation* a, int frames, int ms_delay)
{
    a->totalFrames = frames;
    a->delay = ms_delay;
}

internal void updateAnimation(Animation* a, u32 elapsed_last_frame, b32 active)
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

internal void centerCameraOverPoint(Camera* c, Vec2 pos)
{
    c->viewport.x = (int)pos.x - c->viewport.w / 2;
    c->viewport.y = (int)pos.y - c->viewport.h / 2;
}

internal void updateCamera(Camera* c, Vec2 centerPos)
{
    centerCameraOverPoint(c, centerPos);
    c->viewport.x = clampInt32(c->viewport.x, 0, c->maxX);
    c->viewport.y = clampInt32(c->viewport.y, 0, c->maxY);
}

#if 0
void gameUpdateAndRender(GameMemory *memory, Input *input, OffscreenBuffer buffer)
{
    platform = memory->platformAPI;
}
#endif
