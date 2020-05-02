#include "gamedev_sprite_sheet.h"

void initSpriteSheet(Entity *entity, int xSprites, int ySprites)
{
    // ss->sheet = texture;
    // TextureDims textureDims = rendererAPI.getTextureDims(texture);
    // ss->width = textureDims.width;
    // ss->height = textureDims.height;
    entity->spriteSheet.width = entity->texture.bitmap.width;
    entity->spriteSheet.height = entity->texture.bitmap.height;
    entity->spriteSheet.spriteWidth = entity->spriteSheet.width / xSprites;
    entity->spriteSheet.spriteHeight = entity->spriteSheet.height / ySprites;
    entity->spriteSheet.numX = xSprites;
    entity->spriteSheet.numY = ySprites;
}
