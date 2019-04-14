#include "gamedev_camera.h"

void centerCameraOverPoint(Camera* c, Vec2 pos)
{
    c->viewport.x = (int)pos.x - c->viewport.w / 2;
    c->viewport.y = (int)pos.y - c->viewport.h / 2;
}

void updateCamera(Camera* c, Vec2 centerPos)
{
    centerCameraOverPoint(c, centerPos);
    c->viewport.x = clampInt32(c->viewport.x, 0, c->maxX);
    c->viewport.y = clampInt32(c->viewport.y, 0, c->maxY);
}
