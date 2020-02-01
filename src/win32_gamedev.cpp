#include <windows.h>

#include "gamedev_platform.h"
#include "gamedev_memory.h"

static b32 globalRunning;
static BITMAPINFO globalBitmapInfo;
static VOID *globalBitmapMemory;
static int globalBitmapWidth;
static int globalBitmapHeight;

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

void win32ErrorMessage(PlatformErrorType type, char *message)
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

    // TODO(cjh): Use our window for this when it's created, but NULL if
    // creation fails
    MessageBoxExA(NULL, message, caption, msgBoxType, 0);

    if(type == PlatformError_Fatal)
    {
        ExitProcess(1);
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

    int bytesPerPixel = 4;
    int bitmapMemorySize = globalBitmapWidth * globalBitmapHeight * bytesPerPixel;
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

    GameMemory memory = {};
    memory.permanentStorageSize = (size_t)MEGABYTES(1);
    memory.transientStorageSize = (size_t)MEGABYTES(4);
    memory.permanentStorage = calloc(memory.permanentStorageSize + memory.transientStorageSize, sizeof(u8));
    memory.transientStorage = (u8*)memory.permanentStorage + memory.permanentStorageSize;

#if 0
    memory.platformAPI.readEntireFile = SDLReadEntireFile;
    memory.platformAPI.freeFileMemory = SDLFreeFileMemory;
    memory.platformAPI.getTicks = SDL_GetTicks;

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
    memory.audioAPI.destroySound = SDLDestroySound;
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

    globalRunning = true;
    while (globalRunning)
    {
        MSG message = {};
        BOOL messageResult = GetMessageA(&message, 0, 0, 0);
        if (messageResult > 0)
        {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }
        else
        {
            break;
        }

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
