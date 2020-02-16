#ifndef WIN32_GAMEDEV_H
#define WIN32_GAMEDEV_H

struct Win32State
{
    HWND window;
    b32 isFullscreen;
    b32 isRunning;
};

struct Win32RendererState
{
    VOID *backbufferMemory;
    int backbufferWidth;
    int backbufferHeight;
    int backbufferBytesPerPixel;
};

#endif  // WIN32_GAMEDEV_H
