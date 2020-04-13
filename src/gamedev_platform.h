#ifndef GAMEDEV_PLATFORM_H
#define GAMEDEV_PLATFORM_H

// TODO(cjh): Remove standard library dependencies
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

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


#include "gamedev_math.h"

#define MAX_CONTROLLERS 2

#define internal static
#define local_persist static
#define global static

#define InvalidCodePath assert(!"InvalidCodePath")

#define KILOBYTES(n) ((n) * 1024)
#define MEGABYTES(n) ((n) * 1024 * 1024)
#define GIGABYTES(n) ((n) * 1024 * 1024 * 1024)

#define ARRAY_COUNT(arr) (sizeof(arr) / sizeof((arr)[0]))

struct Camera
{
    Vec3 position;
    Vec3 up;
    Vec3 right;
    Vec3 direction;
};

struct RenderCommands
{
    u8 *bufferBase;
    void *renderer;
    int windowWidth;
    int windowHeight;
    u32 maxBufferSize;
    u32 bufferSize;
    f32 metersToPixels;
    Camera camera;
};

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

struct LoadedBitmap
{
    u32 *pixels;
    u32 width;
    u32 height;
};

struct TextureHandle
{
    union
    {
        void *texture;
        LoadedBitmap bitmap;
    };
};

struct TextureDims
{
    i32 width;
    i32 height;
};

typedef TextureDims (GetTextureDims)(TextureHandle texture);
typedef void (DestroyTexture)(TextureHandle t);
typedef void (SetRenderDrawColor)(void *renderer, Vec4u8 color);
typedef void (RenderRect)(void *renderer, Rect dest, Vec4u8 color);
typedef void (RenderFilledRect)(void *renderer, Rect dest, Vec4u8 color);
typedef void (RenderSprite)(void *renderer, TextureHandle texture, Rect source, Rect dest);
typedef void (RenderBitmap)(void *renderer, LoadedBitmap bitmap, Rect source, Rect dest);
typedef TextureHandle (CreateTextureFromPng)(const char *fname, void *renderer);
typedef TextureHandle (CreateTextureFromGreyscaleBitmap)(void *renderer, u8 *bitmap, i32 width,
                                                         i32 height);
typedef LoadedBitmap (LoadBitmapFunc)(char *path);

struct RendererAPI
{
    GetTextureDims *getTextureDims;
    DestroyTexture *destroyTexture;
    SetRenderDrawColor *setRenderDrawColor;
    RenderRect *renderRect;
    RenderFilledRect *renderFilledRect;
    RenderSprite *renderSprite;
    RenderBitmap *renderBitmap;
    CreateTextureFromPng *createTextureFromPng;
    CreateTextureFromGreyscaleBitmap *createTextureFromGreyscaleBitmap;
    LoadBitmapFunc *loadBitmap;
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

typedef void (GenerateFontData)(FontMetadata *fontMetadata, void *renderer);
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
typedef void (PlatformPlaySound)(Sound *s, u64 now);
typedef void (PlatformDestroySound)(Sound *s);

struct AudioAPI
{
    LoadWav *loadWav;
    PlatformPlaySound *playSound;
    PlatformDestroySound *destroySound;
};

struct GameMemory
{
#if 0
    u64 sdlStorageSize;
    void *sdlStorage;
#endif
    u64 permanentStorageSize;
    void *permanentStorage;
    u64 transientStorageSize;
    void *transientStorage;

    u64 currentTickCount;
    b32 isInitialized;
    PlatformAPI platformAPI;
    RendererAPI rendererAPI;
    FontAPI fontAPI;
    AudioAPI audioAPI;
};

enum Key
{
    Key_Up,
    Key_Down,
    Key_Left,
    Key_Right,
    Key_Escape,
    Key_LShift,
    Key_RShift,
    Key_LAlt,
    Key_RAlt,
    Key_LCtrl,
    Key_RCtrl,
    Key_F5,
    Key_A,
    Key_B,
    Key_C,
    Key_D,
    Key_E,
    Key_F,
    Key_G,
    Key_H,
    Key_I,
    Key_J,
    Key_K,
    Key_L,
    Key_M,
    Key_N,
    Key_O,
    Key_P,
    Key_Q,
    Key_R,
    Key_S,
    Key_T,
    Key_U,
    Key_V,
    Key_W,
    Key_X,
    Key_Y,
    Key_Z,
    Key_Space,

    Key_Count,
};

enum Button
{
    Button_A,         // 'x' on PS4 dualshock
    Button_B,         // 'o' on PS4 dualshock
    Button_X,         // 'square' on PS4 dualshock
    Button_Y,         // 'triangle' on PS4 dualshock
    Button_Up,        // dpad up
    Button_Down,      // dpad down
    Button_Left,      // dpad left
    Button_Right,     // dpad right
    Button_Start,     // 'options' on PS4 dualshock
    Button_Back,      // 'share' on PS4 dualshock
    Button_RShoulder, // 'R1' on PS4 dualshock
    Button_LShoulder, // 'L1' on PS4 dualshock
    Button_RTrigger,  // 'R2' on PS4 dualshock
    Button_LTrigger,  // 'L2' on PS4 dualshock

    Button_Count,
};

struct Input
{
    u64 dt;
    b32 keyPressed[Key_Count];
    b32 keyDown[Key_Count];
    b32 buttonPressed[Button_Count];
    b32 buttonDown[Button_Count];
    f32 stickX;
    f32 stickY;
};
#if GAMEDEV_SDL
typedef void (GameUpdateAndRender)(GameMemory *memory, Input *input, TextureHandle outputTarget,
                                   Rect *viewport, void *rendererState);
#else
typedef void (GameUpdateAndRender)(GameMemory *memory, Input *input, RenderCommands *renderCommands);
#endif

#endif  // GAMEDEV_PLATFORM_H
