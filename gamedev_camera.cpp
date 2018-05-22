#include "gamedev_camera.h"

void camera_center_over_point(Camera* c, SDL_Rect* rect)
{
    int center_x = rect->x + (rect->w / 2);
    int center_y = rect->y + (rect->h / 2);
    c->viewport.x = center_x - c->viewport.w / 2;
    c->viewport.y = center_y - c->viewport.y / 2;
}

void camera_update(Camera* c)
{
    c->viewport.x = clamp(c->viewport.x, 0, c->max_x);
    c->viewport.y = clamp(c->viewport.y, 0, c->max_y);
}
