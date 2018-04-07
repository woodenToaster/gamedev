#include "entity.h"

Entity::~Entity()
{
    if (sprite_sheet.sheet)
    {
        SDL_FreeSurface(sprite_sheet.sheet);
        sprite_sheet = NULL;
    }
}

void Entity::draw(SDL_Surface* map)
{
    if (active)
    {
        SDL_BlitSurface(sprite_sheet.sheet, &sprite_rect, map, &dest_rect);
    }
}
