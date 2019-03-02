#ifndef GD_FONT_H
#define GD_FONT_H

struct FontMetadata
{
    i32 baseline;
    f32 size;
    f32 scale;
    i32 ascent;
    i32 descent;
    i32 lineGap;
    stbtt_fontinfo info;
};

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

struct TTFFile
{
    char buf[1 << 25];
    char* fname;
    u8 initialized;
};

struct TTFFont
{
    stbtt_fontinfo font;
    char* text;
    f32 size;
    f32 scale;
    int width;
    int height;
    u32* bitmap;
    SDL_Texture* texture;
    TTFFile* ttf_file;
    SDL_Rect dest;
};
#endif
