#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>

#define global_variable static

global_variable HDC GlobalDeviceContext;
global_variable HGLRC GlobalRenderingContext;
global_variable int GlobalRunning;

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
        PixelFormatDescriptor.iPixelType = PFD_TYPE_COLORINDEX;
        PixelFormatDescriptor.cColorBits = 8;
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
        glClearColor(0, 0, 0, 0);
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

        GlobalRunning = 1;
        while (GlobalRunning)
        {
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
            glColor3f(1.0f, 0.0f, 0.0f);
            glBegin(GL_POLYGON);
              glVertex3f(0.25f, 0.25, 0.0f);
              glVertex3f(0.75f, 0.25, 0.0f);
              glVertex3f(0.75f, 0.75, 0.0f);
              glVertex3f(0.25f, 0.75, 0.0f);
            glEnd();

            SwapBuffers(GlobalDeviceContext);
        }
    }
    else
    {
        // TODO(chj): logging
    }

    return 0;
}
