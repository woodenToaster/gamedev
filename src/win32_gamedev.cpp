#include <windows.h>

#include "gamedev_platform.h"
#include "gamedev_memory.h"

global BITMAPINFO globalBitmapInfo;
global VOID *globalBitmapMemory;
global int globalBitmapWidth;
global int globalBitmapHeight;
global int globalBytesPerPixel;
global i64 globalPerfFrequency;

struct Win32State
{
    HWND window;
    b32 isFullscreen;
    b32 isRunning;
};

global Win32State win32State;

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

#if 0
internal LARGE_INTEGER win32GetTicks()
{
    LARGE_INTEGER result;
    QueryPerformanceCounter(&result);

    return result;
}
#endif

internal void win32ResizeDIBSection(int width, int height)
{
    if (globalBitmapMemory)
    {
        VirtualFree(globalBitmapMemory, 0, MEM_RELEASE);
    }

    globalBitmapWidth = width;
    globalBitmapHeight = height;

    globalBitmapInfo.bmiHeader.biSize = sizeof(globalBitmapInfo.bmiHeader);
    globalBitmapInfo.bmiHeader.biWidth = globalBitmapWidth;
    // NOTE(cjh): Negative height is a top-down bitmap
    globalBitmapInfo.bmiHeader.biHeight = -globalBitmapHeight;
    globalBitmapInfo.bmiHeader.biPlanes = 1;
    globalBitmapInfo.bmiHeader.biBitCount = 32;
    globalBitmapInfo.bmiHeader.biCompression = BI_RGB;

    globalBytesPerPixel = 4;
    int bitmapMemorySize = globalBitmapWidth * globalBitmapHeight * globalBytesPerPixel;
    globalBitmapMemory = VirtualAlloc(0, bitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
}

internal void win32UpdateWindow(HDC deviceContext, RECT *windowRect, int x, int y, int width, int height)
{
    (void)x;
    (void)y;
    (void)width;
    (void)height;

    int windowWidth = windowRect->right - windowRect->left;
    int windowHeight = windowRect->bottom - windowRect->top;

    StretchDIBits(deviceContext,
                  /* x, y, width, height, */
                  /* x, y, width, height, */
                  0, 0, globalBitmapWidth, globalBitmapHeight,
                  0, 0, windowWidth, windowHeight, globalBitmapMemory,
                  &globalBitmapInfo, DIB_RGB_COLORS, SRCCOPY);
}

LRESULT CALLBACK win32MainWindowCallback(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;
    switch (message)
    {
        case WM_SIZE:
        {
            RECT clientRect;
            GetClientRect(windowHandle, &clientRect);

            int width = clientRect.right - clientRect.left;
            int height = clientRect.bottom - clientRect.top;
            win32ResizeDIBSection(width, height);
        } break;
        case WM_DESTROY:
        {
            win32State.isRunning = false;
        } break;
        case WM_PAINT:
        {
            PAINTSTRUCT paint;
            HDC deviceContext = BeginPaint(windowHandle, &paint);
            int x = paint.rcPaint.left;
            int y = paint.rcPaint.top;
            int width = paint.rcPaint.right - paint.rcPaint.left;
            int height = paint.rcPaint.bottom - paint.rcPaint.top;

            RECT clientRect;
            GetClientRect(windowHandle, &clientRect);
            win32UpdateWindow(deviceContext, &clientRect, x, y, width, height);
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
                win32State.isRunning = false;
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
        win32State.isRunning = false;
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
#if 0

    memory.rendererAPI.getTextureDims = SDLGetTextureDims;
    memory.rendererAPI.destroyTexture = SDLDestroyTexture;
    memory.rendererAPI.setRenderDrawColor = SDLSetRenderDrawColor;
    memory.rendererAPI.renderRect = SDLRenderRect;
    memory.rendererAPI.renderFilledRect = SDLRenderFilledRect;
    memory.rendererAPI.renderSprite = SDLRenderSprite;
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

    WNDCLASSA windowClass = {};

    win32ResizeDIBSection(screenWidth, screenHeight);

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

    win32State.window = CreateWindowExA(0, windowClass.lpszClassName, "Gamedev",
                                        WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT,
                                        CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, instance, 0);

    if (!win32State.window)
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

    u8 modx = 1;
    u8 mody = 1;
    u8 modr = 1;
    win32State.isRunning = true;
    while (win32State.isRunning)
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

        modx++;
        mody++;
        modr++;
        if (modx == 0) modx = 1;
        if (mody == 0) mody = 1;
        if (modr == 0) modr = 1;
        updateAndRender(&memory, &input, (u8 *)globalBitmapMemory, globalBitmapWidth, globalBitmapHeight,
                        globalBytesPerPixel);

        HDC deviceContext = GetDC(win32State.window);
        RECT clientRect;
        GetClientRect(win32State.window, &clientRect);
        int width = clientRect.right - clientRect.left;
        int height = clientRect.bottom - clientRect.top;
        win32UpdateWindow(deviceContext, &clientRect, 0, 0, width, height);
        ReleaseDC(win32State.window, deviceContext);

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
