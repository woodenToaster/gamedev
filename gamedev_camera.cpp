#include "gamedev_camera.h"

void centerCameraOverPoint(Camera* c, Vec2 pos)
{
    c->viewport.x = (int)pos.x - c->viewport.w / 2;
    c->viewport.y = (int)pos.y - c->viewport.h / 2;
}

void updateCamera(Camera* c, Vec2 centerPos)
{
    centerCameraOverPoint(c, centerPos);
    c->viewport.x = clamp(c->viewport.x, 0, c->max_x);
    c->viewport.y = clamp(c->viewport.y, 0, c->max_y);
}
