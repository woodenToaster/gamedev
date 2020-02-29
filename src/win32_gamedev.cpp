#include <windows.h>

#include "gamedev_platform.h"
#include "gamedev_memory.h"
#include "win32_gamedev.h"

global i64 globalPerfFrequency;
global WINDOWPLACEMENT globalWindowPosition = {sizeof(globalWindowPosition)};

global Win32State globalWin32State;
global Win32BackBuffer globalBackBuffer;

enum PlatformErrorType
{
    PlatformError_Fatal,
    PlatformError_Warning
};

internal void win32ErrorMessage(PlatformErrorType type, char *message)
{
    char *caption = "Gamedev Warning";

    UINT msgBoxType = MB_OK;
    if(type == PlatformError_Fatal)
    {
        caption = "Gamedev Fatal Error";
        msgBoxType |= MB_ICONSTOP;
    }
    else
    {
        msgBoxType |= MB_ICONWARNING;
    }

    // TODO(chogan): Use our window for this when it's created, but NULL if
    // creation fails
    // TODO(chogan): Include GetLastError in the message
    MessageBoxExA(NULL, message, caption, msgBoxType, 0);

    if(type == PlatformError_Fatal)
    {
        ExitProcess(1);
    }
}

u32 safeU64ToU32(u64 val)
{
    assert(val <= 0xFFFFFFFF);
    u32 result = (u32)val;

    return result;
}

EntireFile win32ReadEntireFile(char *filename)
{
    EntireFile result = {};
    HANDLE fileHandle = CreateFileA(filename, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
                                    0);
    if (fileHandle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER fileSize;
        if (GetFileSizeEx(fileHandle, &fileSize))
        {
            u32 fileSize32 = safeU64ToU32(fileSize.QuadPart);
            result.contents = (u8 *)VirtualAlloc(0, fileSize32, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
            if (result.contents)
            {
                DWORD bytesRead;
                // TODO(chogan): Support files larger than 4GB?
                BOOL success = ReadFile(fileHandle, result.contents, fileSize32, &bytesRead, 0);
                if (success && bytesRead == fileSize32)
                {
                    result.size = fileSize32;
                }
                else
                {
                    win32ErrorMessage(PlatformError_Fatal, "Failed to read file");
                }

            }
            else
            {
                // TODO(chogan): @error
            }
        }
        else
        {
            // TODO(chogan): @error
        }

        CloseHandle(fileHandle);
    }
    else
    {
        win32ErrorMessage(PlatformError_Fatal, "Failed to open file");
    }

    return result;
}

internal void win32FreeFileMemory(EntireFile *file)
{
    if (file->contents)
    {
        VirtualFree(file->contents, 0, MEM_RELEASE);
    }
}

LoadedBitmap win32LoadBitmap(char *path)
{
    // TODO(chogan): Save bitmap in game memory and delete file data
    EntireFile file_data = win32ReadEntireFile(path);
    u8 *bitmapData = (u8 *)file_data.contents;

    LoadedBitmap result = {};
    if (bitmapData && (char)bitmapData[0] == 'B' && (char)bitmapData[1] == 'M')
    {
        BITMAPFILEHEADER *bitmapHeader = (BITMAPFILEHEADER *)bitmapData;
        if (bitmapHeader->bfReserved1 == 0 && bitmapHeader->bfReserved2 == 0)
        {
            DWORD bitsOffset = bitmapHeader->bfOffBits;
            bitmapData += sizeof(BITMAPFILEHEADER);

            BITMAPINFOHEADER *bmpInfoHeader = (BITMAPINFOHEADER *)bitmapData;

            if (bmpInfoHeader->biCompression != BI_BITFIELDS)
            {
                assert(!"Expected an uncompressed bitmap");
            }

            if (bmpInfoHeader->biBitCount != 32)
            {
                assert(!"Expected a bitmap with 32 bits per pixel");
            }

            result.width = bmpInfoHeader->biWidth;
            // NOTE(chogan): Positive height means 0,0 is in lower left corner
            b32 isBottomUp = bmpInfoHeader->biHeight > 0;
            result.height = isBottomUp ? bmpInfoHeader->biHeight : -bmpInfoHeader->biHeight;
            result.pixels = (u32 *)((u8 *)bitmapHeader + bitsOffset);
        }
        else
        {
            win32ErrorMessage(PlatformError_Warning, "Not a .bmp file");
        }
    }
    else
    {
        win32ErrorMessage(PlatformError_Warning, "Not a .bmp file");
    }

    return result;
}

TextureDims win32GetTextureDims(TextureHandle texture)
{
    TextureDims result = {};
    result.width = texture.bitmap.width;
    result.height = texture.bitmap.height;

    return result;
}


#if 0
internal LARGE_INTEGER win32GetTicks()
{
    LARGE_INTEGER result;
    QueryPerformanceCounter(&result);

    return result;
}
#endif

internal void win32ResizeDIBSection(Win32BackBuffer *buffer, int width, int height)
{
    if (buffer->memory)
    {
        VirtualFree(buffer->memory, 0, MEM_RELEASE);
    }

    buffer->width = width;
    buffer->height = height;

    buffer->bitmapInfo.bmiHeader.biSize = sizeof(buffer->bitmapInfo.bmiHeader);
    buffer->bitmapInfo.bmiHeader.biWidth = width;
    // NOTE(cjh): Negative height is a top-down bitmap
    buffer->bitmapInfo.bmiHeader.biHeight = -height;
    buffer->bitmapInfo.bmiHeader.biPlanes = 1;
    buffer->bitmapInfo.bmiHeader.biBitCount = 32;
    buffer->bitmapInfo.bmiHeader.biCompression = BI_RGB;

    buffer->bytesPerPixel = 4;
    int bitmapMemorySize = width * height * buffer->bytesPerPixel;
    buffer->memory = VirtualAlloc(0, bitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
}

void win32RenderFilledRect(void *renderer, Rect dest, Vec4u8 color)
{
    // TODO(chogan): Clipping
    Win32BackBuffer *backBuffer = (Win32BackBuffer *)renderer;

    u8 r = color.r;
    u8 g = color.g;
    u8 b = color.b;
    u8 a = color.a;

    if (isZeroRect(dest))
    {
        // NOTE(chogan): Fill the whole back buffer
        dest.w = backBuffer->width;
        dest.h = backBuffer->height;
    }

    int bytesPerPixel = backBuffer->bytesPerPixel;
    int pitch = backBuffer->width * bytesPerPixel;
    u8 *start = (u8 *)backBuffer->memory + (dest.y * pitch) + (dest.x * bytesPerPixel);

    int yMax = clampInt32(dest.y + dest.h, 0, backBuffer->height);
    int xMax = clampInt32(dest.x + dest.w, 0, backBuffer->width);
    for (int y = dest.y; y < yMax; ++y)
    {
        u32 *pixel = (u32 *)start;
        for (int x = dest.x; x < xMax; ++x)
        {
            *pixel++ = ((a << 24) | (r << 16) | (g << 8) | (b << 0));
        }
        start += pitch;
    }
}

void win32RenderRect(void *renderer, Rect dest, Vec4u8 color)
{
    // TODO(chogan): Clipping
    Win32BackBuffer *backBuffer = (Win32BackBuffer *)renderer;

    u8 r = color.r;
    u8 g = color.g;
    u8 b = color.b;
    u8 a = color.a;

    if (isZeroRect(dest))
    {
        // NOTE(chogan): Fill the whole back buffer
        dest.w = backBuffer->width;
        dest.h = backBuffer->height;
    }

    int bytesPerPixel = backBuffer->bytesPerPixel;
    int pitch = backBuffer->width * bytesPerPixel;
    u8 *start = (u8 *)backBuffer->memory + (dest.y * pitch) + (dest.x * bytesPerPixel);

    int yMax = clampInt32(dest.y + dest.h, 0, backBuffer->height);
    int xMax = clampInt32(dest.x + dest.w, 0, backBuffer->width);
    for (int y = dest.y; y < yMax; ++y)
    {
        u32 *pixel = (u32 *)start;

        if (y == dest.y || y == yMax - 1)
        {
            for (int x = dest.x; x < xMax; ++x)
            {
                *pixel++ = ((a << 24) | (r << 16) | (g << 8) | (b << 0));
            }
        }
        else
        {
            for (int x = dest.x; x < xMax; ++x)
            {
                if (x != dest.x || x != xMax - 1)
                {
                    continue;
                }
                *pixel++ = ((a << 24) | (r << 16) | (g << 8) | (b << 0));
            }
        }
        start += pitch;
    }
}

void win32RenderBitmap(void *renderer, LoadedBitmap bitmap, Rect sourceRect, Rect destRect)
{
    // TODO(chogan): Clamp
    (void)sourceRect;
    Win32BackBuffer *backBuffer = (Win32BackBuffer *)renderer;
    u8 *destRow = (u8 *)backBuffer->memory + (destRect.y * backBuffer->width + destRect.x);
    u8 *srcRow = (u8 *)bitmap.pixels + bitmap.width * (bitmap.height - 1) * backBuffer->bytesPerPixel;
    for (int y = 0; y < destRect.h; ++y)
    {
        u32 *dest = (u32 *)destRow;
        u32 *src = (u32 *)srcRow;
        for (int x = 0; x < destRect.w; ++x)
        {
            u8 a = (*src >> 24);
            f32 alpha = a / 255.0f;
            u8 bSrc = ((u8 *)src)[0];
            u8 gSrc = ((u8 *)src)[1];
            u8 rSrc = ((u8 *)src)[2];

            u8 bDest = ((u8 *)dest)[0];
            u8 gDest = ((u8 *)dest)[1];
            u8 rDest = ((u8 *)dest)[2];

            u8 rFinal = (u8)(alpha * rSrc + (1.0f - alpha) * rDest);
            u8 gFinal = (u8)(alpha * gSrc + (1.0f - alpha) * gDest);
            u8 bFinal = (u8)(alpha * bSrc + (1.0f - alpha) * bDest);
            *dest++ = ((a << 24) | (rFinal << 16) | (gFinal << 8) | (bFinal << 0));
            src++;
        }
        destRow += backBuffer->bytesPerPixel * backBuffer->width;
        srcRow -= backBuffer->bytesPerPixel * bitmap.width;
    }
}

void win32RenderSprite(void *renderer, TextureHandle sheet, Rect source, Rect dest)
{
    win32RenderBitmap(renderer, sheet.bitmap, source, dest);
}

internal void win32UpdateWindow(Win32BackBuffer *buffer, HDC deviceContext, int windowWidth,
                                int windowHeight)
{
    int destWidth = buffer->width;
    int destHeight = buffer->height;

    if (windowWidth >= 2 * buffer->width && windowHeight >= 2 * buffer->height)
    {
        destWidth = 2 * buffer->width;
        destHeight = 2 * buffer->height;
    }
    StretchDIBits(deviceContext,
                  0, 0, destWidth, destHeight,
                  0, 0, buffer->width, buffer->height,
                  buffer->memory,
                  &buffer->bitmapInfo, DIB_RGB_COLORS, SRCCOPY);
}

LRESULT CALLBACK win32MainWindowCallback(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;
    switch (message)
    {
        case WM_DESTROY:
        {
            globalWin32State.isRunning = false;
        } break;
        case WM_PAINT:
        {
            PAINTSTRUCT paint;
            HDC deviceContext = BeginPaint(windowHandle, &paint);
            int width = paint.rcPaint.right - paint.rcPaint.left;
            int height = paint.rcPaint.bottom - paint.rcPaint.top;

            RECT clientRect;
            GetClientRect(windowHandle, &clientRect);
            win32UpdateWindow(&globalBackBuffer, deviceContext, width, height);
            EndPaint(windowHandle, &paint);
        } break;
        default:
        {
            result = DefWindowProc(windowHandle, message, wParam, lParam);
        } break;
    }

    return result;
}

inline LARGE_INTEGER win32GetTicks()
{
    LARGE_INTEGER result;
    QueryPerformanceCounter(&result);

    return result;
}

inline f32 win32GetSecondsElapsed(LARGE_INTEGER start, LARGE_INTEGER end)
{
    f32 result = ((f32)(end.QuadPart - start.QuadPart)) / (f32)globalPerfFrequency;

    return result;
}
inline u32 win32GetMillisecondsElapsed(LARGE_INTEGER start, LARGE_INTEGER end)
{
    f32 seconds = win32GetSecondsElapsed(start, end);
    u32 result = (u32)(seconds * 1000.0f);

    return result;
}

internal void toggleFullscreen(HWND window)
{
    // NOTE(chogan): This follows Raymond Chen's prescription
    // for fullscreen toggling, see:
    // http://blogs.msdn.com/b/oldnewthing/archive/2010/04/12/9994016.aspx

    DWORD style = GetWindowLong(window, GWL_STYLE);
    if(style & WS_OVERLAPPEDWINDOW)
    {
        MONITORINFO monitorInfo = {sizeof(monitorInfo)};
        if(GetWindowPlacement(window, &globalWindowPosition) &&
           GetMonitorInfo(MonitorFromWindow(window, MONITOR_DEFAULTTOPRIMARY), &monitorInfo))
        {
            SetWindowLong(window, GWL_STYLE, style & ~WS_OVERLAPPEDWINDOW);
            SetWindowPos(window, HWND_TOP,
                         monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.top,
                         monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
                         monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    }
    else
    {
        SetWindowLong(window, GWL_STYLE, style | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(window, &globalWindowPosition);
        SetWindowPos(window, 0, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                     SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
}

internal void win32SetKeyState(Input *input, Key key, b32 isDown)
{
    if (!isDown)
    {
        input->keyPressed[key] = true;
    }
    input->keyDown[key] = isDown;
}

internal void win32UpdateKeyboardInput(Input* input, u64 vkCode, b32 isDown)
{
    // TODO(chogan): Modifier keys
    switch(vkCode)
    {
        case 'W':
        {
            win32SetKeyState(input, Key_Up, isDown);
        } break;
        case 'A':
        {
            win32SetKeyState(input, Key_Left, isDown);
        } break;
        case 'S':
        {
            win32SetKeyState(input, Key_Down, isDown);
        } break;
        case 'D':
        {
            win32SetKeyState(input, Key_Right, isDown);
        } break;
        case VK_ESCAPE:
        {
            win32SetKeyState(input, Key_Escape, isDown);
        } break;
        case VK_F5:
        {
            win32SetKeyState(input, Key_F5, isDown);
        } break;
        default:
        {
        } break;
    }
}

internal void win32GetInput(Input *input)
{
    memset(input->keyPressed, 0, sizeof(b32) * Key_Count);
    memset(input->buttonPressed, 0, sizeof(b32) * Button_Count);

    MSG message;
    while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
    {
        switch (message.message)
        {
            case WM_QUIT:
            {
                globalWin32State.isRunning = false;
            } break;
            case WM_SYSKEYUP:
            case WM_KEYUP:
            case WM_SYSKEYDOWN:
            case WM_KEYDOWN:
            {
                b32 wasDown = message.lParam >> 30 != 0;
                b32 isDown = message.lParam >> 31 == 0;

                if (wasDown != isDown)
                {
                    win32UpdateKeyboardInput(input, (u64)message.wParam, isDown);
                }
            } break;
            default:
            {
                TranslateMessage(&message);
                DispatchMessageA(&message);
            } break;
        }
    }

    if (input->keyPressed[Key_Escape])
    {
        globalWin32State.isRunning = false;
    }

    if (input->keyPressed[Key_F5])
    {
        toggleFullscreen(message.hwnd);
    }
}

int CALLBACK WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR commandLine, int showCode)
{
    (void)prevInstance;
    (void)commandLine;
    (void)showCode;

    GameMemory memory = {};
    memory.permanentStorageSize = (size_t)MEGABYTES(1);
    memory.transientStorageSize = (size_t)MEGABYTES(4);
    memory.permanentStorage = VirtualAlloc(0, memory.permanentStorageSize + memory.transientStorageSize,
                                           MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    memory.transientStorage = (u8*)memory.permanentStorage + memory.permanentStorageSize;

    memory.platformAPI.readEntireFile = win32ReadEntireFile;
    memory.platformAPI.freeFileMemory = win32FreeFileMemory;
    // memory.platformAPI.getTicks = win32GetTicks;

    memory.rendererAPI.renderFilledRect = win32RenderFilledRect;
    memory.rendererAPI.renderRect = win32RenderRect;
    memory.rendererAPI.loadBitmap = win32LoadBitmap;
    memory.rendererAPI.renderBitmap = win32RenderBitmap;
    memory.rendererAPI.renderSprite = win32RenderSprite;
    memory.rendererAPI.getTextureDims = win32GetTextureDims;

#if 0
    memory.rendererAPI.destroyTexture = SDLDestroyTexture;
    memory.rendererAPI.setRenderDrawColor = SDLSetRenderDrawColor;
    memory.rendererAPI.createTextureFromPng = SDLCreateTextureFromPng;
    memory.rendererAPI.createTextureFromGreyscaleBitmap = SDLCreateTextureFromGreyscaleBitmap;

    memory.fontAPI.getKernAdvancement = SDLGetKernAdvancement;
    memory.fontAPI.generateFontData = SDLGenerateFontData;

    memory.audioAPI.playSound = SDLPlaySound;
    memory.audioAPI.loadWav = SDLLoadWav;
#endif

    // NOTE(chogan): Counts per second
    LARGE_INTEGER perfCountFreqResult;
    if (!QueryPerformanceFrequency(&perfCountFreqResult))
    {
        // TODO(chogan): No high resolution performance counter available.
        // Fall back to something else?
    }
    globalPerfFrequency = perfCountFreqResult.QuadPart;

    // NOTE(chogan): Set the Windows scheduler granularity to the minimum that
    // the device allows.
    TIMECAPS timeCaps = {};
    UINT minimumResolution = 0;
    if(timeGetDevCaps(&timeCaps, sizeof(TIMECAPS)) != MMSYSERR_NOERROR)
    {
        win32ErrorMessage(PlatformError_Warning, "Could not determine device TIMECAPS. Using default.");
        minimumResolution = 5;
    }
    else
    {
        minimumResolution = timeCaps.wPeriodMin;
    }

    // TODO(chogan): This should technically be matched with a call to
    // timeEndPeriod(minimumResolution).
    b32 sleepIsGranular = (minimumResolution == 1 &&
                           (timeBeginPeriod(minimumResolution) == TIMERR_NOERROR));

    i32 screenWidth = 960;
    i32 screenHeight = 540;
    // i32 screenWidth = 1920;
    // i32 screenHeight = 1080;

    WNDCLASSA windowClass = {};

    win32ResizeDIBSection(&globalBackBuffer, screenWidth, screenHeight);

    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = win32MainWindowCallback;
    windowClass.hInstance = instance;
    windowClass.hCursor = LoadCursor(0, IDC_ARROW);
    windowClass.lpszClassName = "GamedevWindowClass";

    if (!RegisterClassA(&windowClass))
    {
        // TODO(cjh): Use GetLastError() in message
        win32ErrorMessage(PlatformError_Fatal, "Cannot register window class");
    }

    globalWin32State.window = CreateWindowExA(0, windowClass.lpszClassName, "Gamedev",
                                              WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT,
                                              CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0,
                                              instance, 0);

    if (!globalWin32State.window)
    {
        win32ErrorMessage(PlatformError_Fatal, "Cannot create window");
    }

    u32 targetFps = 60;
    // TODO(chogan): This gets set when memory is initialized in gameUpdateAndRender.
    // Remove this once we're calling that function.
    u32 targetMsPerFrame = (u32)(1000.0f / (f32)targetFps);

    // TODO(cjh):
    // SDL_GameController *controllerHandles[MAX_CONTROLLERS] = {};
    // SDLInitControllers(&controllerHandles[0]);

    // u32 tileWidth = 80;
    // u32 tileHeight = 80;
    // u32 rows = 10;
    // u32 cols = 12;
    // TextureHandle backBuffer = {};
    // backBuffer.texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
                                           // cols * tileWidth, rows * tileHeight);
    Rect viewport = {0, 0, screenWidth, screenHeight};

    char *dllName = "gamedev.dll";
    char *tempDllName = "gamedev_temp.dll";
    char *dllPath = "w:\\gamedev\\build\\gamedev.dll";
    char *tempPath = "w:\\gamedev\\build\\gamedev_temp.dll";

    CopyFile(dllPath, tempPath, FALSE);
    HMODULE gamedevDLL = LoadLibraryA(tempDllName);
    if (!gamedevDLL)
    {
        char errBuffer[64];
        _snprintf_s(errBuffer, sizeof(errBuffer), "Cannot load %s\n", dllName);
        win32ErrorMessage(PlatformError_Fatal, errBuffer);
    }
    char *procName = "gameUpdateAndRender";
    GameUpdateAndRender *updateAndRender = (GameUpdateAndRender*)GetProcAddress(gamedevDLL, procName);

    if (!updateAndRender)
    {
        char errBuffer[64];
        _snprintf_s(errBuffer, sizeof(errBuffer), "Failed to find %s\n", procName);
        win32ErrorMessage(PlatformError_Fatal, errBuffer);
    }


    WIN32_FILE_ATTRIBUTE_DATA attributeData;
    GetFileAttributesExA(dllPath, GetFileExInfoStandard, &attributeData);
    FILETIME lastWriteTime = attributeData.ftLastWriteTime;

    LARGE_INTEGER lastTickCount = win32GetTicks();

    globalWin32State.isRunning = true;
    while (globalWin32State.isRunning)
    {
        WIN32_FILE_ATTRIBUTE_DATA w32FileAttributData;
        GetFileAttributesExA(dllPath, GetFileExInfoStandard, &w32FileAttributData);
        FILETIME newWriteTime = w32FileAttributData.ftLastWriteTime;
        if (CompareFileTime(&newWriteTime, &lastWriteTime) != 0)
        {
            WIN32_FILE_ATTRIBUTE_DATA ignored;
            char *lockFileName = "lock.tmp";
            if (!GetFileAttributesExA(lockFileName, GetFileExInfoStandard, &ignored))
            {
                FreeLibrary(gamedevDLL);
                lastWriteTime = newWriteTime;
                CopyFile(dllPath, tempPath, FALSE);
                gamedevDLL = LoadLibraryA(tempDllName);
                updateAndRender = (GameUpdateAndRender*)GetProcAddress(gamedevDLL, procName);
                memory.isInitialized = false;
            }
        }
        LARGE_INTEGER currentTick = win32GetTicks();
        memory.currentTickCount = currentTick.QuadPart;

        Input input = {};
        // TODO(chogan): Should this be based on the actual elapsed ms instead of
        // the target?
        input.dt = targetMsPerFrame;
        win32GetInput(&input);

        updateAndRender(&memory, &input, &globalBackBuffer);

        HDC deviceContext = GetDC(globalWin32State.window);
        RECT clientRect;
        GetClientRect(globalWin32State.window, &clientRect);
        int width = clientRect.right - clientRect.left;
        int height = clientRect.bottom - clientRect.top;
        win32UpdateWindow(&globalBackBuffer, deviceContext, width, height);
        ReleaseDC(globalWin32State.window, deviceContext);

        u32 dt = win32GetMillisecondsElapsed(currentTick, win32GetTicks());
        if (dt < targetMsPerFrame)
        {
            if (sleepIsGranular)
            {
                while (dt < targetMsPerFrame)
                {
                    u32 sleep_ms = targetMsPerFrame - dt;
                    dt += sleep_ms;
                    Sleep(sleep_ms);
                }
            }
        }
        else if (dt > targetMsPerFrame)
        {
            OutputDebugStringA("Missed frame\n");
        }

        char fpsBuffer[64];
        u32 msPerFrame = dt;
        _snprintf_s(fpsBuffer, sizeof(fpsBuffer), "%d ms/f\n", msPerFrame);
        // OutputDebugStringA(fpsBuffer);
    }

    return 0;
}
