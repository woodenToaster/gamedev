#include <windows.h>
#include <GL/gl.h>

#define global_variable static

global_variable HDC GlobalDeviceContext;
global_variable HGLRC GlobalRenderingContext;
global_variable int GlobalRunning;
global_variable GLfloat GlobalSpin;

LRESULT CALLBACK WindowProc(HWND WindowHandle, UINT Message, WPARAM WParam, LPARAM LParam)
{
    switch (Message)
    {
    case WM_CREATE:
    {
        PIXELFORMATDESCRIPTOR PixelFormatDescriptor = {0};
        PixelFormatDescriptor.nSize = sizeof(PIXELFORMATDESCRIPTOR);
        PixelFormatDescriptor.nVersion = 1;
        PixelFormatDescriptor.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        PixelFormatDescriptor.dwLayerMask = PFD_MAIN_PLANE;
        PixelFormatDescriptor.iPixelType = PFD_TYPE_RGBA;
        PixelFormatDescriptor.cColorBits = 24;
        PixelFormatDescriptor.cRedBits = 8;
        PixelFormatDescriptor.cRedShift = 16;
        PixelFormatDescriptor.cGreenBits = 8;
        PixelFormatDescriptor.cGreenShift = 8;
        PixelFormatDescriptor.cBlueBits = 8;
        PixelFormatDescriptor.cBlueShift = 0;
        PixelFormatDescriptor.cDepthBits = 16;
        PixelFormatDescriptor.cAccumBits = 0;
        PixelFormatDescriptor.cStencilBits = 0;

        GlobalDeviceContext = GetDC(WindowHandle);

        int PixelFormat = ChoosePixelFormat(GlobalDeviceContext, &PixelFormatDescriptor);

        if (!PixelFormat)
        {
            MessageBox(NULL, "ChoosePixelFormat failed", "Error", MB_OK);
            PostQuitMessage(0);
            return 0;
        }

        if (SetPixelFormat(GlobalDeviceContext, PixelFormat, &PixelFormatDescriptor) == FALSE)
        {
            MessageBox(NULL, "SetPixelFormat failed", "Error", MB_OK);
            PostQuitMessage(0);
            return 0;
        }

        GlobalRenderingContext = wglCreateContext(GlobalDeviceContext);
        wglMakeCurrent(GlobalDeviceContext, GlobalRenderingContext);

        RECT ClientRect = {0};
        GetClientRect(WindowHandle, &ClientRect);

        glEnable(GL_DEPTH_TEST);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glShadeModel(GL_FLAT);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, 1, 0, 1, -1, 1);
        return 0;
    }
    case WM_SIZE:
    {
        RECT ClientRect;
        GetClientRect(WindowHandle, &ClientRect);

        int Width = ClientRect.right;
        int Height = ClientRect.bottom;
        glViewport(0, 0, (GLsizei)Width, (GLsizei)Height);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, 1, 0, 1, -1, 1);
        return 0;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_CLOSE:
        GlobalRunning = 0;
        return 0;
    case WM_PAINT:
    {
        PAINTSTRUCT Paint;
        BeginPaint(WindowHandle, &Paint);
        EndPaint(WindowHandle, &Paint);
        return 0;
    }
    }

    return DefWindowProc(WindowHandle, Message, WParam, LParam);
}

int WINAPI WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, PSTR CmdLine, int CmdShow)
{
    (void)CmdLine;
    (void)PrevInstance;

    LARGE_INTEGER CountsPerSecond;
    QueryPerformanceFrequency(&CountsPerSecond);

    WNDCLASS WindowClass = {0};
    WindowClass.lpfnWndProc = WindowProc;
    WindowClass.hInstance = Instance;
    WindowClass.lpszClassName = "OpenGL";
    WindowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);

    RegisterClass(&WindowClass);

    int WindowWidth = 250;
    int WindowHeight = 250;
    HWND WindowHandle = CreateWindowEx(0, WindowClass.lpszClassName, "OpenGL", WS_OVERLAPPEDWINDOW,
                                       CW_USEDEFAULT, CW_USEDEFAULT, WindowWidth, WindowHeight,
                                       0, 0, Instance, 0);
    if (WindowHandle)
    {
        ShowWindow(WindowHandle, CmdShow);
        UpdateWindow(WindowHandle);

        LARGE_INTEGER ElapsedMicroseconds = {0};
        GlobalRunning = 1;
        while (GlobalRunning)
        {
            LARGE_INTEGER StartTime;
            QueryPerformanceCounter(&StartTime);

            MSG Message = {0};
            while (PeekMessage(&Message, NULL, 0, 0, PM_NOREMOVE) == TRUE)
            {
                if (GetMessage(&Message, NULL, 0, 0))
                {
                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                }
            }

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glPushMatrix();
            glTranslatef(0.375f, 0.375f, 0.0f);
            glRotatef(GlobalSpin, 0.0f, 0.0f, 1.0f);
            glTranslatef(-0.375f, -0.375f, 0.0f);
            glColor3f(1.0f, 0.0f, 0.0f);
            glRectf(0.25f, 0.25f, 0.5f, 0.5f);
            glPopMatrix();
            glFlush();
            SwapBuffers(GlobalDeviceContext);

            LARGE_INTEGER EndTime;
            QueryPerformanceCounter(&EndTime);

            ElapsedMicroseconds.QuadPart += (EndTime.QuadPart - StartTime.QuadPart) * 1000000 /
                CountsPerSecond.QuadPart / 100;

            if (ElapsedMicroseconds.QuadPart > 16)
            {
                GlobalSpin += 1.0f;
                GlobalSpin = fmodf(GlobalSpin, 360);
                ElapsedMicroseconds.QuadPart = 0;
            }
        }
    }
    else
    {
        // TODO(chj): logging
    }

    return 0;
}
