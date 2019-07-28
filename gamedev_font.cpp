#include "gamedev_font.h"

void destroyFontMetadata(FontMetadata *fmd)
{
    for (int i = '!'; i <= '~'; ++i)
    {
        rendererAPI.destroyTexture(fmd->textures[i]);
    }
}

