#include <windows.h>
#include <math.h>

#define global_variable static
#define internal static

global_variable HDC globalDeviceContext;
global_variable int globalRunning;
global_variable BITMAPINFO globalBitmapInfo;
global_variable VOID *globalBitmapMemory;
global_variable HBITMAP globalBitmapHandle;
global_variable HDC globalBitmapDeviceContext;

internal void win32ResizeDIBSection(int width, int height)
{
    if (globalBitmapHandle)
    {
        DeleteObject(globalBitmapHandle);
    }

    if (!globalBitmapDeviceContext)
    {
        globalBitmapDeviceContext = CreateCompatibleDC(0);
    }

    globalBitmapInfo.bmiHeader.biSize = sizeof(globalBitmapInfo.bmiHeader);
    globalBitmapInfo.bmiHeader.biWidth = width;
    globalBitmapInfo.bmiHeader.biHeight = height;
    globalBitmapInfo.bmiHeader.biPlanes = 1;
    globalBitmapInfo.bmiHeader.biBitCount = 32;
    globalBitmapInfo.bmiHeader.biCompression = BI_RGB;

    globalBitmapHandle = CreateDIBSection(globalBitmapDeviceContext, &globalBitmapInfo, DIB_RGB_COLORS,
                                          &globalBitmapMemory, 0, 0);
}

internal void win32UpdateWindow(HDC deviceContext, int x, int y, int width, int height)
{
    StretchDIBits(deviceContext, x, y, width, height, x, y, width, height, &globalBitmapMemory,
                  &globalBitmapInfo, DIB_RGB_COLORS, SRCCOPY);
}

LRESULT CALLBACK win32WindowProc(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        PIXELFORMATDESCRIPTOR pixelFormatDescriptor = {0};
        pixelFormatDescriptor.nSize = sizeof(PIXELFORMATDESCRIPTOR);
        pixelFormatDescriptor.nVersion = 1;
        pixelFormatDescriptor.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        pixelFormatDescriptor.dwLayerMask = PFD_MAIN_PLANE;
        pixelFormatDescriptor.iPixelType = PFD_TYPE_RGBA;
        pixelFormatDescriptor.cColorBits = 24;
        pixelFormatDescriptor.cRedBits = 8;
        pixelFormatDescriptor.cRedShift = 16;
        pixelFormatDescriptor.cGreenBits = 8;
        pixelFormatDescriptor.cGreenShift = 8;
        pixelFormatDescriptor.cBlueBits = 8;
        pixelFormatDescriptor.cBlueShift = 0;
        pixelFormatDescriptor.cDepthBits = 16;
        pixelFormatDescriptor.cAccumBits = 0;
        pixelFormatDescriptor.cStencilBits = 0;

        globalDeviceContext = GetDC(windowHandle);

        int pixelFormat = ChoosePixelFormat(globalDeviceContext, &pixelFormatDescriptor);

        if (!pixelFormat)
        {
            MessageBox(NULL, "ChoosePixelFormat failed", "Error", MB_OK);
            PostQuitMessage(0);
            return 0;
        }

        if (SetPixelFormat(globalDeviceContext, pixelFormat, &pixelFormatDescriptor) == FALSE)
        {
            MessageBox(NULL, "SetPixelFormat failed", "Error", MB_OK);
            PostQuitMessage(0);
            return 0;
        }

        RECT clientRect = {0};
        GetClientRect(windowHandle, &clientRect);

        return 0;
    }
    case WM_SIZE:
    {
        RECT clientRect;
        GetClientRect(windowHandle, &clientRect);

        int width = clientRect.right - clientRect.left;
        int height = clientRect.bottom - clientRect.top;
        win32ResizeDIBSection(width, height);
        return 0;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_CLOSE:
        globalRunning = 0;
        return 0;
    case WM_PAINT:
    {
        PAINTSTRUCT paint;
        HDC deviceContext = BeginPaint(windowHandle, &paint);
        int x = paint.rcPaint.left;
        int y = paint.rcPaint.top;
        int width = paint.rcPaint.right - paint.rcPaint.left;
        int height = paint.rcPaint.bottom - paint.rcPaint.top;
        win32UpdateWindow(deviceContext, x, y, width, height);
        EndPaint(windowHandle, &paint);
        return 0;
    }
    }

    return DefWindowProc(windowHandle, message, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance, PSTR cmdLine, int cmdShow)
{
    (void)cmdLine;
    (void)prevInstance;

    LARGE_INTEGER countsPerSecond;
    QueryPerformanceFrequency(&countsPerSecond);

    WNDCLASS windowClass = {0};
    windowClass.lpfnWndProc = win32WindowProc;
    windowClass.hInstance = instance;
    windowClass.lpszClassName = "OpenGL";
    windowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);

    RegisterClass(&windowClass);

    HWND windowHandle = CreateWindowEx(0, windowClass.lpszClassName, "OpenGL", WS_OVERLAPPEDWINDOW,
                                       CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                                       0, 0, instance, 0);
    if (windowHandle)
    {
        ShowWindow(windowHandle, cmdShow);
        UpdateWindow(windowHandle);
        LARGE_INTEGER elapsedMicroseconds = {0};
        globalRunning = 1;
        while (globalRunning)
        {
            LARGE_INTEGER startTime;
            QueryPerformanceCounter(&startTime);

            MSG message = {0};
            while (PeekMessage(&message, NULL, 0, 0, PM_NOREMOVE) == TRUE)
            {
                if (GetMessage(&message, NULL, 0, 0))
                {
                    TranslateMessage(&message);
                    DispatchMessage(&message);
                }
            }

            SwapBuffers(globalDeviceContext);

            LARGE_INTEGER endTime;
            QueryPerformanceCounter(&endTime);

            elapsedMicroseconds.QuadPart += (endTime.QuadPart - startTime.QuadPart) * 1000000 /
                countsPerSecond.QuadPart / 100;

            if (elapsedMicroseconds.QuadPart > 16)
            {
                elapsedMicroseconds.QuadPart = 0;
            }
        }
    }
    else
    {
        // TODO(cjh): logging
    }

    return 0;
}
