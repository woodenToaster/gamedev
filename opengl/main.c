#include <assert.h>
#include <stdio.h>
#include <stdint.h>

#include <windows.h>
#include <GL/gl.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef float f32;
typedef double f64;
typedef int32_t b32;

#define GL_VERTEX_SHADER 0x8B31
#define GL_FRAGMENT_SHADER 0x8B30

typedef void (*PFNGLCLEARBUFFERFVPROC)(GLenum buffer, GLint drawbuffer, const GLfloat *value);
PFNGLCLEARBUFFERFVPROC glClearBufferfv;
typedef GLuint (*PFNGLCREATESHADERPROC)(GLenum type);
PFNGLCREATESHADERPROC glCreateShader;
typedef void (*PFNGLSHADERSOURCEPROC)(GLuint shader, GLsizei count, const char *const*string,
                                      const GLint *length);
PFNGLSHADERSOURCEPROC glShaderSource;
typedef void (*PFNGLCOMPILESHADERPROC) (GLuint shader);
PFNGLCOMPILESHADERPROC glCompileShader;
typedef GLuint (*PFNGLCREATEPROGRAMPROC)();
PFNGLCREATEPROGRAMPROC glCreateProgram;
typedef void (*PFNGLATTACHSHADERPROC)(GLuint program, GLuint shader);
PFNGLATTACHSHADERPROC glAttachShader;
typedef void (*PFNGLLINKPROGRAMPROC)(GLuint program);
PFNGLLINKPROGRAMPROC glLinkProgram;
typedef void (*PFNGLDELETESHADERPROC)(GLuint shader);
PFNGLDELETESHADERPROC glDeleteShader;
typedef void (*PFNGLCREATEVERTEXARRAYSPROC)(GLsizei n, GLuint *arrays);
PFNGLCREATEVERTEXARRAYSPROC glCreateVertexArrays;
typedef void (*PFNGLBINDVERTEXARRAYPROC)(GLuint array);
PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
typedef void (*PFNGLDELETEVERTEXARRAYSPROC)(GLsizei n, const GLuint *arrays);
PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;
typedef void (*PFNGLDELETEPROGRAMPROC)(GLuint program);
PFNGLDELETEPROGRAMPROC glDeleteProgram;
typedef void (*PFNGLUSEPROGRAMPROC)(GLuint program);
PFNGLUSEPROGRAMPROC glUseProgram;

#define global static

global HDC GlobalDeviceContext;
global HGLRC GlobalRenderingContext;
global int GlobalRunning;
global i64 globalPerfFrequency;

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
        glOrtho(0, (GLdouble)Width, 0, (GLdouble)Height, -1, 1);
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

void *GetAnyGLFuncAddress(const char *name)
{
    void *p = (void *)wglGetProcAddress(name);
    if(p == 0 ||
       (p == (void*)0x1) || (p == (void*)0x2) || (p == (void*)0x3) ||
       (p == (void*)-1) )
    {
        HMODULE module = LoadLibraryA("opengl32.dll");
        p = (void *)GetProcAddress(module, name);
    }

    return p;
}

static const char *vertexShaderSource[] =
{
    "#version 450 core \n",
    "void main(void) \n",
    "{ \n",
    "    gl_Position = vec4(0.0, 0.0, 0.5, 1.0); \n",
    "}",
};

static const char *fragmentShaderSource[] =
{
    "#version 450 core \n",
    "out vec4 color \n",
    "void main(void) \n",
    "{ \n",
    "    color = vec4(0.0, 0.8, 1.0, 1.0); \n",
    "}"
};

GLuint compileShaders()
{
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

#define LOAD_GL_FUNC(name, uppername) (name = (PFNGL##uppername##PROC)GetAnyGLFuncAddress(#name))

void loadOpenGLFunctions()
{
    glClearBufferfv = (PFNGLCLEARBUFFERFVPROC)GetAnyGLFuncAddress("glClearBufferfv");
    glCreateShader = (PFNGLCREATESHADERPROC)GetAnyGLFuncAddress("glCreateShader");
    glShaderSource = (PFNGLSHADERSOURCEPROC)GetAnyGLFuncAddress("glShaderSource");
    glCompileShader = (PFNGLCOMPILESHADERPROC)GetAnyGLFuncAddress("glCompileShader");
    glCreateProgram = (PFNGLCREATEPROGRAMPROC)GetAnyGLFuncAddress("glCreateProgram");
    glAttachShader = (PFNGLATTACHSHADERPROC)GetAnyGLFuncAddress("glAttachShader");
    glLinkProgram = (PFNGLLINKPROGRAMPROC)GetAnyGLFuncAddress("glLinkProgram");
    glDeleteShader = (PFNGLDELETESHADERPROC)GetAnyGLFuncAddress("glDeleteShader");
    glCreateVertexArrays = (PFNGLCREATEVERTEXARRAYSPROC)GetAnyGLFuncAddress("glCreateVertexArrays");
    glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)GetAnyGLFuncAddress("glBindVertexArray");
    LOAD_GL_FUNC(glDeleteVertexArrays, DELETEVERTEXARRAYS);
    LOAD_GL_FUNC(glDeleteProgram, DELETEPROGRAM);
    LOAD_GL_FUNC(glUseProgram, USEPROGRAM);
}

int WINAPI WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, PSTR CmdLine, int CmdShow)
{
    (void)CmdLine;
    (void)PrevInstance;

    // NOTE(chogan): Counts per second
    LARGE_INTEGER perfCountFreqResult;
    QueryPerformanceFrequency(&perfCountFreqResult);
    globalPerfFrequency = perfCountFreqResult.QuadPart;

    u32 targetFps = 60;
    u32 targetMsPerFrame = (u32)(1000.0f / (f32)targetFps);

    b32 sleepIsGranular = timeBeginPeriod(1) == TIMERR_NOERROR;

    LARGE_INTEGER CountsPerSecond;
    QueryPerformanceFrequency(&CountsPerSecond);

    WNDCLASS WindowClass = {0};
    WindowClass.lpfnWndProc = WindowProc;
    WindowClass.hInstance = Instance;
    WindowClass.lpszClassName = "OpenGL";
    WindowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);

    RegisterClass(&WindowClass);

    int WindowWidth = 512;
    int WindowHeight = 512;
    HWND WindowHandle = CreateWindowEx(0, WindowClass.lpszClassName, "OpenGL", WS_OVERLAPPEDWINDOW,
                                       CW_USEDEFAULT, CW_USEDEFAULT, WindowWidth, WindowHeight,
                                       0, 0, Instance, 0);

    loadOpenGLFunctions();
    GLuint program = compileShaders();

    GLuint vertexArrayObject;
    glCreateVertexArrays(1, &vertexArrayObject);
    glBindVertexArray(vertexArrayObject);

    if (WindowHandle)
    {
        ShowWindow(WindowHandle, CmdShow);
        UpdateWindow(WindowHandle);
        glEnable(GL_BLEND);
        LARGE_INTEGER ElapsedMicroseconds = {0};
        GlobalRunning = 1;
        while (GlobalRunning)
        {
            LARGE_INTEGER StartTime;
            QueryPerformanceCounter(&StartTime);

            LARGE_INTEGER currentTick = win32GetTicks();

            MSG Message = {0};
            while (PeekMessage(&Message, NULL, 0, 0, PM_NOREMOVE) == TRUE)
            {
                if (GetMessage(&Message, NULL, 0, 0))
                {
                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                }
            }

            const GLfloat red[] = {1.0f, 0.0f, 0.0f, 1.0f};
            glClearBufferfv(GL_COLOR, 0, red);

            glUseProgram(program);
            glPointSize(40.0f);
            glDrawArrays(GL_POINTS, 0, 1);

            SwapBuffers(GlobalDeviceContext);

            LARGE_INTEGER EndTime;
            QueryPerformanceCounter(&EndTime);

            ElapsedMicroseconds.QuadPart += (EndTime.QuadPart - StartTime.QuadPart) * 1000000 /
                CountsPerSecond.QuadPart / 100;

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
            char dt_str[32];
            snprintf(dt_str, 32, "MS per frame: %d\n", dt);
            OutputDebugString(dt_str);
        }
    }
    else
    {
        // TODO(cjh): logging
    }

    glDeleteVertexArrays(1, &vertexArrayObject);
    glDeleteProgram(program);

    return 0;
}
