#include "gamedev_sprite_sheet.h"

void destroySpriteSheet(SpriteSheet* ss)
{
    if (ss)
    {
        SDL_DestroyTexture(ss->sheet);
        ss->sheet = NULL;
    }
}

void loadSpriteSheet(SpriteSheet* ss, const char* path, int x_sprites, int y_sprites, SDL_Renderer* renderer)
{
    ss->sheet = create_texture_from_png(path, renderer);
    SDL_QueryTexture(ss->sheet, NULL, NULL, &ss->width, &ss->height);
    ss->sprite_width = ss->width / x_sprites;
    ss->sprite_height = ss->height / y_sprites;
    ss->num_x = x_sprites;
    ss->num_y = y_sprites;
}
