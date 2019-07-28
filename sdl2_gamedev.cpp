#define _CRT_SECURE_NO_WARNINGS

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// #define SDL_MAIN_HANDLED

#include "SDL.h"
#include "SDL_opengl.h"
#include "SDL_mixer.h"

// TODO(cjh): Remove standard library dependency
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "gamedev_platform.h"
#include "gamedev_math.h"
#include "gamedev.h"
#include "gamedev_renderer.h"

#include "gamedev_font.h"
#include "gamedev_sprite_sheet.h"
#include "gamedev_tilemap.h"
#include "gamedev_entity.h"

#include "gamedev_renderer.cpp"
#include "gamedev_font.cpp"
#include "gamedev_sound.cpp"
#include "gamedev_asset_loading.cpp"
#include "gamedev_sprite_sheet.cpp"
#include "gamedev_entity.cpp"
#include "gamedev.cpp"
#include "gamedev_tilemap.cpp"

#define RENDERER_HANDLE_TO_SDL(r) ((SDL_Renderer*)((r).renderer))

internal SDL_Texture* SDLCreateTextureFromPng(const char* fname, RendererHandle renderer)
{
    SDL_Renderer *sdlRenderer = RENDERER_HANDLE_TO_SDL(renderer);
    unsigned char *img_data;
    int width;
    int height;
    int channels_in_file;
    img_data = stbi_load(fname, &width, &height, &channels_in_file, 0);

    if (!img_data)
    {
        printf("Loading image failed: %s\n", stbi_failure_reason());
        exit(1);
    }
    u32 rmask;
    u32 gmask;
    u32 bmask;
    u32 amask;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    bool bigEndian = true;
    int shift = (channels_in_file == STBI_rgb) ? 8 : 0;
    rmask = 0xff000000 >> shift;
    gmask = 0x00ff0000 >> shift;
    bmask = 0x0000ff00 >> shift;
    amask = 0x000000ff >> shift;
#else
    bool bigEndian = false;
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = (channels_in_file == STBI_rgb) ? 0 : 0xff000000;
#endif

    int depth;
    int pitch;
    u32 pixelFormat;
    if (channels_in_file == STBI_rgb)
    {
        depth = 24;
        pitch = 3 * width;
        pixelFormat = bigEndian ? SDL_PIXELFORMAT_RGB888 : SDL_PIXELFORMAT_BGR888;
    }
    else
    {
        depth = 32;
        pitch = 4 * width;
        pixelFormat = bigEndian ? SDL_PIXELFORMAT_RGBA8888 : SDL_PIXELFORMAT_ABGR8888;
    }

    SDL_Texture *texture = SDL_CreateTexture(sdlRenderer, pixelFormat, SDL_TEXTUREACCESS_STATIC, width, height);
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    SDL_UpdateTexture(texture, NULL, (void*)img_data, pitch);

    stbi_image_free(img_data);

    return texture;
}

void SDLInitColors(u32 *colors, SDL_PixelFormat *pixelFormat)
{
    colors[Color_None] = 0xFFFFFFFF;
    colors[Color_White] = SDL_MapRGB(pixelFormat, 255, 255, 255);
    colors[Color_DarkGreen] = SDL_MapRGB(pixelFormat, 37, 71, 0);
    colors[Color_Blue] = SDL_MapRGB(pixelFormat, 0, 0, 255);
    colors[Color_Yellow] = SDL_MapRGB(pixelFormat, 235, 245, 65);
    colors[Color_Brown] = SDL_MapRGB(pixelFormat, 153, 102, 0);
    colors[Color_Rust] = SDL_MapRGB(pixelFormat, 153, 70, 77);
    colors[Color_Magenta] = SDL_MapRGB(pixelFormat, 255, 0, 255);
    colors[Color_Black] = SDL_MapRGB(pixelFormat, 0, 0, 0);
    colors[Color_Red] = SDL_MapRGB(pixelFormat, 255, 0, 0);
    colors[Color_Grey] = SDL_MapRGB(pixelFormat, 135, 135, 135);
    colors[Color_DarkBlue] = SDL_MapRGB(pixelFormat, 0, 51, 102);
    colors[Color_DarkOrange] = SDL_MapRGB(pixelFormat, 255, 140, 0);
    colors[Color_BabyBlue] = SDL_MapRGB(pixelFormat, 137, 207, 240);
    colors[Color_LimeGreen] = SDL_MapRGB(pixelFormat, 106, 190, 48);
}


TextureHandle SDLCreateTextureFromGreyscaleBitmap(RendererHandle renderer, u8 *bitmap, i32 width, i32 height)
{
    TextureHandle result = {};

    SDL_Surface* surface = SDL_CreateRGBSurface(0, width, height, 32,
                                                0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
    if (!surface)
    {
        printf("%s\n", SDL_GetError());
        exit(1);
    }

    SDL_LockSurface(surface);
    u8 *srcPixel = bitmap;
    u32 *destPixel = (u32*)surface->pixels;

    for (int i = 0; i < height * width; ++i)
    {
        u8 val = *srcPixel++;
        *destPixel++ = ((val << 24) | (val << 16) | (val << 8) | (val << 0));
    }
    SDL_UnlockSurface(surface);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(RENDERER_HANDLE_TO_SDL(renderer), surface);
    result.texture = texture;
    stbtt_FreeBitmap(bitmap, 0);
    SDL_FreeSurface(surface);

    return result;
}

EntireFile SDLReadEntireFile(char *filename)
{
    EntireFile result = {};
    SDL_RWops *file = SDL_RWFromFile(filename, "rb");

    if (file)
    {
        Sint64 fileSize = SDL_RWseek(file, 0, RW_SEEK_END);
        if (fileSize >= 0)
        {
            result.size = (u64)fileSize;
            if (SDL_RWseek(file, 0, RW_SEEK_SET) >= 0)
            {
                result.contents = (u8*)malloc(result.size);
                SDL_RWread(file, (void*)result.contents, 1, result.size);
                SDL_RWclose(file);
            }
            else
            {
                // TODO(cjh):
                // printf(stderr, "%s\n", SDL_GetError());
            }
        }
        else
        {
            // TODO(cjh):
            // printf("%s\n", SDL_GetError());
        }
    }
    else
    {
        // TODO(cjh):
        // fprintf(stderr, "%s\n", SDL_GetError());
    }
    return result;
}

void SDLFreeFileMemory(EntireFile *file)
{
    if (file->contents)
    {
        free(file->contents);
    }
}

internal void SDLInitControllers(SDL_GameController **handles)
{
    // TODO(cjh): Handle SDL_CONTROLLERDEVICEADDED, SDL_CONTROLLERDEVICEREMOVED, and SDL_CONTROLLERDEVICEREMAPPED
    int maxJoysticks = SDL_NumJoysticks();
    for (int joystickIndex = 0; joystickIndex < maxJoysticks; ++joystickIndex)
    {
        if (joystickIndex >= MAX_CONTROLLERS)
        {
            break;
        }
        if (!SDL_IsGameController(joystickIndex))
        {
            continue;
        }
        handles[joystickIndex] = SDL_GameControllerOpen(joystickIndex);
    }
}

internal void SDLDestroyControllers(SDL_GameController **handles)
{
    for(int controllerIndex = 0; controllerIndex < MAX_CONTROLLERS; ++controllerIndex)
    {
        if (handles[controllerIndex])
        {
            SDL_GameControllerClose(handles[controllerIndex]);
        }
    }
}

internal void SDLUpdateInput(Input* input, SDL_Scancode key, b32 isDown)
{
    SDL_Keymod keyMod = SDL_GetModState();
    if (keyMod & KMOD_LSHIFT)
    {
        setKeyState(input, KEY_LSHIFT, isDown);
    }
    if (keyMod & KMOD_RSHIFT)
    {
        setKeyState(input, KEY_RSHIFT, isDown);
    }

    switch (key)
    {
    case SDL_SCANCODE_RIGHT:
    case SDL_SCANCODE_L:
    case SDL_SCANCODE_D:
        setKeyState(input, KEY_RIGHT, isDown);
        break;
    case SDL_SCANCODE_UP:
    case SDL_SCANCODE_K:
    case SDL_SCANCODE_W:
        setKeyState(input, KEY_UP, isDown);
        break;
    case SDL_SCANCODE_DOWN:
    case SDL_SCANCODE_J:
    case SDL_SCANCODE_S:
        setKeyState(input, KEY_DOWN, isDown);
        break;
    case SDL_SCANCODE_LEFT:
    case SDL_SCANCODE_H:
    case SDL_SCANCODE_A:
        setKeyState(input, KEY_LEFT, isDown);
        break;
    case SDL_SCANCODE_ESCAPE:
        setKeyState(input, KEY_ESCAPE, isDown);
        break;
    case SDL_SCANCODE_F:
        setKeyState(input, KEY_F, isDown);
        break;
    case SDL_SCANCODE_C:
        setKeyState(input, KEY_C, isDown);
        break;
    case SDL_SCANCODE_P:
        setKeyState(input, KEY_P, isDown);
        break;
    case SDL_SCANCODE_SPACE:
        setKeyState(input, KEY_SPACE, isDown);
        break;
    case SDL_SCANCODE_I:
        setKeyState(input, KEY_I, isDown);
        break;
    case SDL_SCANCODE_V:
        setKeyState(input, KEY_V, isDown);
        break;
    case SDL_SCANCODE_X:
        setKeyState(input, KEY_X, isDown);
        break;
    case SDL_SCANCODE_Z:
        setKeyState(input, KEY_Z, isDown);
        break;
    default:
        // char* action = pressed ? "pressed" : "released";
        // printf("Key %s: %s\n", action, SDL_GetKeyName(SDL_GetKeyFromScancode(key)));
        break;
    }
}

internal void SDLPollInput(Input *input, SDL_GameController **handles)
{
    // Reset all button presses
    // TODO(cjh): This seems wasteful
    for (u32 key = 0; key < KEY_COUNT; ++key)
    {
        input->keyPressed[key] = 0;
    }
    for (u32 button = 0; button < BUTTON_COUNT; ++button)
    {
        input->buttonPressed[button] = 0;
    }

    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_KEYUP:
            SDLUpdateInput(input, event.key.keysym.scancode, 0);
            break;
        case SDL_QUIT:
            globalRunning = 0;
            break;
        case SDL_KEYDOWN:
            SDLUpdateInput(input, event.key.keysym.scancode, true);
            break;
            // case SDL_MOUSEMOTION:
            //     input_handle_mouse(&input);
            //     break;
        }
    }
    // Check controller input
    // TODO(cjh): Should we use events for this? What's the difference?
    for (int controllerIndex = 0; controllerIndex < MAX_CONTROLLERS; ++controllerIndex)
    {
        SDL_GameController *controller = handles[controllerIndex];
        if(controller && SDL_GameControllerGetAttached(controller))
        {
            // TODO(cjh): Handle remaining buttons
            // TODO(cjh): Handle keyPressed as distinct from keyDown
            b32 *pressed = input->buttonPressed;
            pressed[BUTTON_UP] = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_UP);
            pressed[BUTTON_DOWN] = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN);
            pressed[BUTTON_LEFT] = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT);
            pressed[BUTTON_RIGHT] = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
            // bool start = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_START);
            // bool back = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_BACK);
            // bool leftShoulder = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
            // bool rightShoulder = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
            pressed[BUTTON_A] = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_A);
            pressed[BUTTON_B] = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_B);
            pressed[BUTTON_X] = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_X);
            pressed[BUTTON_Y] = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_Y);

            short sdlStickX = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX);
            short sdlStickY = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY);
            f32 stickX = normalizeStickInput(sdlStickX);
            f32 stickY = normalizeStickInput(sdlStickY);

            // Account for dead zone
            input->stickX = (stickX > -0.1f && stickX < 0.1f) ? 0.0f : stickX;
            input->stickY = (stickY > -0.1f && stickY < 0.1f) ? 0.0f : stickY;
        }
        else
        {
            // TODO: This controller is not plugged in.
        }
    }
}

// Rendering

SDL_Rect SDLRectFromRect(Rect rect)
{
    SDL_Rect result = {};
    result.x = rect.x;
    result.y = rect.y;
    result.w = rect.w;
    result.h = rect.h;

    return result;
}

TextureDims SDLGetTextureDims(TextureHandle texture)
{
    SDL_Texture *sdlTexture = (SDL_Texture*)texture.texture;
    TextureDims result = {};
    SDL_QueryTexture(sdlTexture, NULL, NULL, &result.width, &result.height);

    return result;
}

void SDLDestroyTexture(TextureHandle t)
{
    SDL_DestroyTexture((SDL_Texture*)t.texture);
}

void SDLSetRenderDrawColor(RendererHandle renderer, u32 color)
{
    SDL_SetRenderDrawColor(RENDERER_HANDLE_TO_SDL(renderer), getRedFromU32(color), getGreenFromU32(color),
                           getBlueFromU32(color), 255);
}

void SDLRenderRect(RendererHandle renderer, Rect dest, u32 color, u8 alpha=255)
{
    u8 r = (u8)((color & 0x00FF0000) >> 16);
    u8 g = (u8)((color & 0x0000FF00) >> 8);
    u8 b = (u8)((color & 0x000000FF) >> 0);

    SDL_Renderer *sdlRenderer = RENDERER_HANDLE_TO_SDL(renderer);
    SDL_Rect sdl_dest = SDLRectFromRect(dest);
    SDL_Rect *sdl_dest_ptr = isZeroRect(dest) ? NULL : &sdl_dest;
    SDL_BlendMode blendMode;
    SDL_GetRenderDrawBlendMode(sdlRenderer, &blendMode);
    SDL_SetRenderDrawColor(sdlRenderer, r, g, b, alpha);
    SDL_SetRenderDrawBlendMode(sdlRenderer, SDL_BLENDMODE_BLEND);
    SDL_RenderDrawRect(sdlRenderer, sdl_dest_ptr);
    SDL_SetRenderDrawBlendMode(sdlRenderer, blendMode);
}

void SDLRenderFilledRect(RendererHandle renderer, Rect dest, u32 color, u8 alpha=255)
{
    // Color_None is 0xFFFFFFFF. Set a tile's background color to Color_None to avoid
    // extra rendering.
    if (color != 0xFFFFFFFF)
    {
        u8 r = (u8)((color & 0x00FF0000) >> 16);
        u8 g = (u8)((color & 0x0000FF00) >> 8);
        u8 b = (u8)((color & 0x000000FF) >> 0);

        SDL_Renderer *sdlRenderer = RENDERER_HANDLE_TO_SDL(renderer);
        SDL_Rect sdl_dest = SDLRectFromRect(dest);
        SDL_Rect *sdl_dest_ptr = isZeroRect(dest) ? NULL : &sdl_dest;
        SDL_BlendMode blendMode;
        SDL_GetRenderDrawBlendMode(sdlRenderer, &blendMode);
        SDL_SetRenderDrawColor(sdlRenderer, r, g, b, alpha);
        SDL_SetRenderDrawBlendMode(sdlRenderer, SDL_BLENDMODE_BLEND);
        SDL_RenderFillRect(sdlRenderer, sdl_dest_ptr);
        SDL_SetRenderDrawBlendMode(sdlRenderer, blendMode);
    }
}

void SDLRenderSprite(RendererHandle renderer, TextureHandle texture, Rect source, Rect dest)
{
    SDL_Renderer *sdlRenderer = RENDERER_HANDLE_TO_SDL(renderer);
    SDL_Rect sdl_source = SDLRectFromRect(source);
    SDL_Rect sdl_dest = SDLRectFromRect(dest);
    SDL_Rect *sdl_source_ptr = isZeroRect(source) ? NULL : &sdl_source;
    SDL_Rect *sdl_dest_ptr = isZeroRect(dest) ? NULL : &sdl_dest;
    SDL_RenderCopy(sdlRenderer, (SDL_Texture*)texture.texture, sdl_source_ptr, sdl_dest_ptr);
}

#if 0
void SDLRenderCircle(SDL_Renderer *renderer, i32 _x, i32 _y, i32 radius)
{
    i32 x = radius - 1;
    if (radius == 0)
    {
        return;
    }
    i32 y = 0;
    i32 tx = 1;
    i32 ty = 1;
    i32 err = tx - (2 * radius);
    while (x >= y)
    {
        //  Each of the following renders an octant of the circle
        SDL_RenderDrawPoint(renderer, _x + x, _y - y);
        SDL_RenderDrawPoint(renderer, _x + x, _y + y);
        SDL_RenderDrawPoint(renderer, _x - x, _y - y);
        SDL_RenderDrawPoint(renderer, _x - x, _y + y);
        SDL_RenderDrawPoint(renderer, _x + y, _y - x);
        SDL_RenderDrawPoint(renderer, _x + y, _y + x);
        SDL_RenderDrawPoint(renderer, _x - y, _y - x);
        SDL_RenderDrawPoint(renderer, _x - y, _y + x);

        if (err <= 0)
        {
            y++;
            err += ty;
            ty += 2;
        }
        if (err > 0)
        {
            x--;
            tx += 2;
            err += tx - (radius << 1);
        }
    }
}
#endif

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    GameMemory memory = {};
    memory.permanentStorageSize = (size_t)MEGABYTES(1);
    memory.transientStorageSize = (size_t)MEGABYTES(4);
    memory.permanentStorage = calloc(memory.permanentStorageSize + memory.transientStorageSize, sizeof(u8));
    memory.transientStorage = (u8*)memory.permanentStorage + memory.permanentStorageSize;

    memory.platformAPI.readEntireFile = SDLReadEntireFile;
    memory.platformAPI.freeFileMemory = SDLFreeFileMemory;
    memory.platformAPI.getTicks = SDL_GetTicks;

    memory.rendererAPI.getTextureDims = SDLGetTextureDims;
    memory.rendererAPI.destroyTexture = SDLDestroyTexture;
    memory.rendererAPI.setRenderDrawColor = SDLSetRenderDrawColor;
    memory.rendererAPI.renderRect = SDLRenderRect;
    memory.rendererAPI.renderFilledRect = SDLRenderFilledRect;
    memory.rendererAPI.renderSprite = SDLRenderSprite;
    memory.rendererAPI.createTextureFromGreyscaleBitmap = SDLCreateTextureFromGreyscaleBitmap;

    platform = memory.platformAPI;
    rendererAPI = memory.rendererAPI;

    u32 targetFps = 60;
    memory.dt = (i32)((1.0f / (f32)targetFps) * 1000);

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) < 0)
    {
        printf("SDL failed to initialize: %s\n", SDL_GetError());
        exit(1);
    }

    if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048 ) < 0 )
    {
        printf( "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError() );
        exit(1);
    }

    i32 screenWidth = 960;
    i32 screenHeight = 540;
    SDL_Window *window = SDL_CreateWindow("gamedev",
                                          30,
                                          50,
                                          screenWidth,
                                          screenHeight,
                                          SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

    if (window == NULL)
    {
        fprintf(stderr, "Could not create window: %s\n", SDL_GetError());
        exit(1);
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    RendererHandle rendererHandle = {renderer};

    if (renderer == NULL)
    {
        fprintf(stderr, "Could not create renderer: %s\n", SDL_GetError());
        exit(1);
    }

    SDL_PixelFormat *pixelFormat = SDL_GetWindowSurface(window)->format;
    SDLInitColors(memory.colors, pixelFormat);

    // OpenGL
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_GLContext ogl_context = SDL_GL_CreateContext(window);

    if (ogl_context == NULL)
    {
        fprintf(stderr, "Failed to create opengl context: %s\n", SDL_GetError());
        exit(1);
    }

    // Asset loading
    // TODO(cjh): Packed asset file
    // TODO(cjh): asset streaming

    // PNGs
    // TODO(cjh): Don't store assets in GameMemory
    memory.linkTexture.texture = SDLCreateTextureFromPng("sprites/link_walking.png", rendererHandle);
    memory.harvestableTreeTexture.texture = SDLCreateTextureFromPng("sprites/harvestable_tree.png", rendererHandle);
    // memory.harlodTexture = SDLCreateTextureFromPng("sprites/Harlod_the_caveman.png", game->renderer);
    // memory.knightTexture = SDLCreateTextureFromPng("sprites/knight_alligned.png", game->renderer);
    memory.flameTexture.texture = SDLCreateTextureFromPng("sprites/flame.png", rendererHandle);
    memory.firePitTexture.texture = SDLCreateTextureFromPng("sprites/fire_pit.png", rendererHandle);
    memory.glowTreeTexture.texture = SDLCreateTextureFromPng("sprites/glow_tree.png", rendererHandle);

    // Sounds
    memory.mudSound.delay = 250;
    memory.mudSound.chunk = loadWav("sounds/mud_walk.wav");

    // Fonts
    generateFontData(&memory.fontMetadata, rendererHandle);

    // Input
    Input input = {};
    SDL_GameController *controllerHandles[MAX_CONTROLLERS] = {};
    SDLInitControllers(&controllerHandles[0]);

    u32 tileWidth = 80;
    u32 tileHeight = 80;
    u32 rows = 10;
    u32 cols = 12;
    TextureHandle backBuffer = {};
    backBuffer.texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
                                           cols * tileWidth, rows * tileHeight);

    globalRunning = true;
    while(globalRunning)
    {
        memory.currentTickCount = SDL_GetTicks();
        SDLPollInput(&input, &controllerHandles[0]);
        SDL_SetRenderTarget(renderer, (SDL_Texture*)backBuffer.texture);
        Rect viewport = {};
        gameUpdateAndRender(&memory, &input, backBuffer, &viewport, rendererHandle, screenWidth, screenHeight);

        // Hero interaction region
        // SDL_SetRenderDrawColor(game->renderer, 255, 255, 0, 255);
        // drawCircle(game->renderer, (i32)heroInteractionRegion.center.x,
        //                 (i32)heroInteractionRegion.center.y, (i32)heroInteractionRegion.radius);

        SDL_SetRenderTarget(renderer, NULL);
        SDL_Rect sdlViewport = {viewport.x, viewport.y, viewport.w, viewport.h};
        SDL_RenderCopy(renderer, (SDL_Texture*)backBuffer.texture, &sdlViewport, NULL);
        u32 dt = SDL_GetTicks() - memory.currentTickCount;

        if (dt < memory.targetMsPerFrame)
        {
            while (dt < memory.targetMsPerFrame)
            {
                u32 sleep_ms = memory.targetMsPerFrame - dt;
                dt += sleep_ms;
                SDL_Delay(sleep_ms);
            }
        }
        memory.dt = dt;

        SDL_RenderPresent(renderer);
    }

    destroyFontMetadata(&memory.fontMetadata);
    SDLDestroyControllers(&controllerHandles[0]);

    // TODO(cjh): This needs work
    Game* game = (Game*)memory.permanentStorage;
    destroyMap(game->currentMap);
    destroyGame(game);
    //

    Mix_Quit();
    SDL_DestroyRenderer(renderer);
    free(memory.permanentStorage);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
