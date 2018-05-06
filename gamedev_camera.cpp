#include "gamedev_camera.h"

void camera_init(Camera* c, Game* game, Map* map)
{
    c->viewport = {};
    c->viewport.w = game->screen_width;
    c->viewport.h = game->screen_height;
    c->starting_pos = c->viewport;
    c->max_x = map->width_pixels - c->viewport.w;
    c->max_y = map->height_pixels - c->viewport.h;
    c->y_pixel_movement_threshold = game->screen_height / 2;
    c->x_pixel_movement_threshold = game->screen_width / 2;
}

void camera_center_over_point(Camera* c, SDL_Rect* rect)
{
    int center_x = rect->x + (rect->w / 2);
    int center_y = rect->y + (rect->h / 2);
    c->viewport.x = center_x - c->viewport.w / 2;
    c->viewport.y = center_y - c->viewport.y / 2;
}
