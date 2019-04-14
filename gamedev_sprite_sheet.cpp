#include "gamedev_sprite_sheet.h"

void initSpriteSheet(SpriteSheet* ss, SDL_Texture *texture, int x_sprites, int y_sprites)
{
    ss->sheet = texture;
    SDL_QueryTexture(ss->sheet, NULL, NULL, &ss->width, &ss->height);
    ss->sprite_width = ss->width / x_sprites;
    ss->sprite_height = ss->height / y_sprites;
    ss->num_x = x_sprites;
    ss->num_y = y_sprites;
}
