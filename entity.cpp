#include "entity.h"

Entity::Entity(const char* sprite_path,
               int x_sprites,
               int y_sprites,
               int speed,
               float starting_x,
               float starting_y,
               bool active):
    speed(speed),
    starting_pos(starting_x, starting_y),
    active(active),
    sprite_sheet(),
    direction(DOWN)

{
    sprite_sheet.load(sprite_path, x_sprites, y_sprites);
    sprite_rect.w = sprite_sheet.sprite_width;
    sprite_rect.h = sprite_sheet.sprite_height;
    dest_rect.x = (int)starting_pos.x;
    dest_rect.y = (int)starting_pos.y;
    dest_rect.w = sprite_sheet.sprite_width;
    dest_rect.h = sprite_sheet.sprite_height;
}

Entity::~Entity()
{}

void Entity::draw(SDL_Surface* map)
{
    if (active)
    {
        SDL_BlitSurface(sprite_sheet.sheet, &sprite_rect, map, &dest_rect);
    }
}
