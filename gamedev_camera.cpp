#include "gamedev_camera.h"

void camera_center_over_point(Camera* c, SDL_Rect* rect)
{
    int center_x = rect->x + (rect->w / 2);
    int center_y = rect->y + (rect->h / 2);
    c->viewport.x = center_x - c->viewport.w / 2;
    c->viewport.y = center_y - c->viewport.y / 2;
}
