#include <windows.h>

#include "gamedev_platform.h"
#include "gamedev_memory.h"

global b32 globalRunning;
global BITMAPINFO globalBitmapInfo;
global VOID *globalBitmapMemory;
global int globalBitmapWidth;
global int globalBitmapHeight;
global int globalBytesPerPixel;
global LARGE_INTEGER globalPerfFrequency;

struct Win32State
{
    HWND window;
    b32 isFullscreen;
    b32 isRunning;
};

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

internal void renderTest(u8 modx, u8 mody, u8 modr)
{
    u8 *row = (u8 *)globalBitmapMemory;
    int pitch = globalBitmapWidth * globalBytesPerPixel;
    for (int y = 0; y < globalBitmapHeight; ++y)
    {
        u32 *pixel = (u32 *)row;
        for (int x = 0; x < globalBitmapWidth; ++x)
        {
            u8 r = (x + y) % modr;
            u8 g = y % mody;
            u8 b = x % modx;
            u8 a = 0xFF;

            // RR GG BB xx
            // xx BB GG RR
            // xx RR GG BB
            *pixel++ = ((a << 24) | (r << 16) | (g << 8) | (b << 0));
        }
        row += pitch;
    }
}

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
            break;
        }
        case WM_DESTROY:
            globalRunning = false;
            break;
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
            break;
        }
        default:
            result = DefWindowProc(windowHandle, message, wParam, lParam);
            break;
    }

    return result;
}

int CALLBACK WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR commandLine, int showCode)
{
    (void)prevInstance;
    (void)commandLine;
    (void)showCode;

    // NOTE(chogan): Counts per second
    if (!QueryPerformanceFrequency(&globalPerfFrequency))
    {
        // TODO(chogan): No high resolution performance counter available.
        // Fall back to something else?
    }

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

    // NOTE(chogan): Set the Windows scheduler granularity to 1ms so that our
    // Sleep() can be more granular.
    // UINT desiredSchedulerMS = 1;
    // b32 sleepIsGranular = timeBeginPeriod(desiredSchedulerMS) == TIMERR_NOERROR;

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

    Win32State state = {};
    state.window = CreateWindowExA(0, windowClass.lpszClassName, "Gamedev",
                                   WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT,
                                   CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, instance, 0);

    if (!state.window)
    {
        win32ErrorMessage(PlatformError_Fatal, "Cannot create window");
    }

    u32 targetFps = 60;
    memory.dt = (i32)((1.0f / (f32)targetFps) * 1000);

    // Input
    Input input = {};
    // TODO(cjh):
    // SDL_GameController *controllerHandles[MAX_CONTROLLERS] = {};
    // SDLInitControllers(&controllerHandles[0]);

    Rect viewport = {0, 0, screenWidth, screenHeight};

    HMODULE gamedevDLL = LoadLibraryA("gamedev.dll");
    if (!gamedevDLL)
    {
        win32ErrorMessage(PlatformError_Warning, "Cannot create window");
        exit(1);
    }
    GameUpdateAndRender *updateAndRender = (GameUpdateAndRender*)GetProcAddress(gamedevDLL, "gameUpdateAndRender");

    if (!updateAndRender)
    {
        // TODO(cjh): @win32
        OutputDebugString("Failed to load gameUpdateAndRender\n");
        exit(1);
    }

    // TODO(cjh): @win32 specific code
    WIN32_FILE_ATTRIBUTE_DATA attributeData;
    GetFileAttributesExA("w:\\gamedev\\build\\gamedev.dll", GetFileExInfoStandard, &attributeData);
    FILETIME lastWriteTime = attributeData.ftLastWriteTime;

    u8 modx = 1;
    u8 mody = 1;
    u8 modr = 1;
    globalRunning = true;
    while (globalRunning)
    {
        MSG message;
        while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
        {
            if (message.message == WM_QUIT)
            {
                globalRunning = false;
            }

            TranslateMessage(&message);
            DispatchMessageA(&message);
        }

        modx++;
        mody++;
        modr++;
        if (modx == 0) modx = 1;
        if (mody == 0) mody = 1;
        if (modr == 0) modr = 1;
        renderTest(modx, mody, modr);

        HDC deviceContext = GetDC(state.window);
        RECT clientRect;
        GetClientRect(state.window, &clientRect);
        int width = clientRect.right - clientRect.left;
        int height = clientRect.bottom - clientRect.top;
        win32UpdateWindow(deviceContext, &clientRect, 0, 0, width, height);
        ReleaseDC(state.window, deviceContext);
    }

    return 0;
}
