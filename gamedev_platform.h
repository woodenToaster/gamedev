#ifndef GAMEDEV_PLATFORM_H
#define GAMEDEV_PLATFORM_H

// TODO(cjh): Remove standard library dependencies
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX_CONTROLLERS 2

#define internal static
#define local_persist static
#define global_variable static

#define InvalidCodePath assert(!"InvalidCodePath")

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef float f32;
typedef double f64;
typedef int32_t b32;

#define MEGABYTES(n) ((n) * 1024 * 1024)
#define ArrayCount(arr) (sizeof(arr) / sizeof((arr)[0]))

enum Colors
{
    Color_None,
    Color_White,
    Color_DarkGreen,
    Color_Blue,
    Color_Yellow,
    Color_Brown,
    Color_Rust,
    Color_Magenta,
    Color_Black,
    Color_Red,
    Color_Grey,
    Color_DarkBlue,
    Color_BabyBlue,
    Color_DarkOrange,
    Color_LimeGreen,
    Color_Count
};

struct EntireFile
{
    u8 *contents;
    u64 size;
};

struct Game;
typedef  EntireFile PlatformReadEntireFile(char *);
typedef void PlatformFreeFileMemory(EntireFile *);
typedef u32 PlatformGetTicks();

struct PlatformAPI
{
    PlatformReadEntireFile *readEntireFile;
    PlatformFreeFileMemory *freeFileMemory;
    PlatformGetTicks *getTicks;
};

struct RendererHandle
{
    void *renderer;
};

struct TextureHandle
{
    void *texture;
};

struct TextureDims
{
    i32 width;
    i32 height;
};

struct Rect
{
    int x;
    int y;
    int w;
    int h;
};

typedef TextureDims (GetTextureDims)(TextureHandle texture);
typedef void (DestroyTexture)(TextureHandle t);
typedef void (SetRenderDrawColor)(RendererHandle renderer, u32 color);
typedef void (RenderRect)(RendererHandle renderer, Rect dest, u32 color, u8 alpha);
typedef void (RenderFilledRect)(RendererHandle renderer, Rect dest, u32 color, u8 alpha);
typedef void (RenderSprite)(RendererHandle renderer, TextureHandle texture, Rect source, Rect dest);
typedef TextureHandle (CreateTextureFromPng)(const char *fname, RendererHandle renderer);
typedef TextureHandle (CreateTextureFromGreyscaleBitmap)(RendererHandle renderer, u8 *bitmap, i32 width, i32 height);

struct RendererAPI
{
    GetTextureDims *getTextureDims;
    DestroyTexture *destroyTexture;
    SetRenderDrawColor *setRenderDrawColor;
    RenderRect *renderRect;
    RenderFilledRect *renderFilledRect;
    RenderSprite *renderSprite;
    CreateTextureFromPng *createTextureFromPng;
    CreateTextureFromGreyscaleBitmap *createTextureFromGreyscaleBitmap;
};

struct FontInfoHandle
{
    void *info;
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

struct FontMetadata
{
    i32 baseline;
    f32 size;
    f32 scale;
    i32 ascent;
    i32 descent;
    i32 lineGap;
    FontInfoHandle info;
    TextureHandle textures[128];
    CodepointMetadata codepointMetadata[128];

};

typedef void (GenerateFontData)(FontMetadata *fontMetadata, RendererHandle renderer);
typedef int (GetKernAdvancement)(FontInfoHandle info, char a, char b);

struct FontAPI
{
    GenerateFontData *generateFontData;
    GetKernAdvancement *getKernAdvancement;
};

struct SoundChunkHandle
{
    void *chunk;
};

struct Sound
{
    u8 is_playing;
    u32 delay;
    u64 last_play_time;
    SoundChunkHandle chunk;
};

typedef SoundChunkHandle (LoadWav)(const char *fname);
typedef void (PlaySound)(Sound *s, u64 now);

struct AudioAPI
{
    LoadWav *loadWav;
    PlaySound *playSound;
};

struct GameMemory
{
    u64 permanentStorageSize;
    u64 transientStorageSize;
    u32 currentTickCount;
    u32 dt;
    u32 targetMsPerFrame;
    b32 isInitialized;
    void *permanentStorage;
    void *transientStorage;
    PlatformAPI platformAPI;
    RendererAPI rendererAPI;
    FontAPI fontAPI;
    AudioAPI audioAPI;
    u32 colors[Color_Count];
};

enum Key
{
    KEY_UP,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_ESCAPE,
    KEY_LSHIFT,
    KEY_RSHIFT,
    KEY_A,
    KEY_B,
    KEY_C,
    KEY_D,
    KEY_E,
    KEY_F,
    KEY_G,
    KEY_H,
    KEY_I,
    KEY_J,
    KEY_K,
    KEY_L,
    KEY_M,
    KEY_N,
    KEY_O,
    KEY_P,
    KEY_Q,
    KEY_R,
    KEY_S,
    KEY_T,
    KEY_U,
    KEY_V,
    KEY_W,
    KEY_X,
    KEY_Y,
    KEY_Z,
    KEY_SPACE,
    KEY_COUNT
};

enum Buttons
{
    BUTTON_A,
    BUTTON_B,
    BUTTON_X,
    BUTTON_Y,
    BUTTON_UP,
    BUTTON_DOWN,
    BUTTON_LEFT,
    BUTTON_RIGHT,
    BUTTON_COUNT
};

struct Input
{
    b32 keyPressed[KEY_COUNT];
    b32 keyDown[KEY_COUNT];
    b32 buttonPressed[BUTTON_COUNT];
    b32 buttonDown[BUTTON_COUNT];
    f32 stickX;
    f32 stickY;
};
typedef void (GameUpdateAndRender)(GameMemory *memory, Input *input, TextureHandle outputTarget, Rect *viewport,
                                   RendererHandle renderer);
#endif
