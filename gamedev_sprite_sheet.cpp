#include "gamedev_sprite_sheet.h"

void initSpriteSheet(SpriteSheet* ss, TextureHandle texture, int xSprites, int ySprites)
{
    ss->sheet = texture;
    TextureDims textureDims = rendererAPI.getTextureDims(texture);
    ss->width = textureDims.width;
    ss->height = textureDims.height;
    ss->spriteWidth = ss->width / xSprites;
    ss->spriteHeight = ss->height / ySprites;
    ss->numX = xSprites;
    ss->numY = ySprites;
}
