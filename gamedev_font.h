#ifndef GD_FONT_H
#define GD_FONT_H

struct CodepointMetadata
{
    i32 advance;
    i32 leftSideBearing;
    i32 x0;
    i32 y0;
    i32 x1;
    i32 y1;
    f32 xShift;
};

struct FontMetadata
{
    i32 baseline;
    f32 size;
    f32 scale;
    i32 ascent;
    i32 descent;
    i32 lineGap;
    stbtt_fontinfo info;
    SDL_Texture *textures[128];
    CodepointMetadata codepointMetadata[128];
};

struct EntireFile
{
    u8 *contents;
    u64 size;
};
#endif
