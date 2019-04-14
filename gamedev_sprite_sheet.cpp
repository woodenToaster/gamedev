#include "gamedev_sprite_sheet.h"

void initSpriteSheet(SpriteSheet* ss, SDL_Texture *texture, int xSprites, int ySprites)
{
    ss->sheet = texture;
    SDL_QueryTexture(ss->sheet, NULL, NULL, &ss->width, &ss->height);
    ss->spriteWidth = ss->width / xSprites;
    ss->spriteHeight = ss->height / ySprites;
    ss->numX = xSprites;
    ss->numY = ySprites;
}
