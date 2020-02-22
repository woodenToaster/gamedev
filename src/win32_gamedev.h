#ifndef WIN32_GAMEDEV_H
#define WIN32_GAMEDEV_H

struct Win32State
{
    HWND window;
    b32 isFullscreen;
    b32 isRunning;
};

struct Win32BackBuffer
{
    BITMAPINFO bitmapInfo;
    VOID *memory;
    int width;
    int height;
    int bytesPerPixel;
};

#endif  // WIN32_GAMEDEV_H
