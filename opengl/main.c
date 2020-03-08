#include <assert.h>
#include <stdio.h>
#include <stdint.h>

#include <windows.h>
#include <GL/gl.h>
#include "glext.h"

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

PFNGLATTACHSHADERPROC glAttachShader;
PFNGLBINDBUFFERPROC glBindBuffer;
PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
PFNGLBUFFERSUBDATAPROC glBufferSubData;
PFNGLCLEARBUFFERFVPROC glClearBufferfv;
PFNGLCOMPILESHADERPROC glCompileShader;
PFNGLCREATEBUFFERSPROC glCreateBuffers;
PFNGLCREATEPROGRAMPROC glCreateProgram;
PFNGLCREATESHADERPROC glCreateShader;
PFNGLCREATEVERTEXARRAYSPROC glCreateVertexArrays;
PFNGLDELETEPROGRAMPROC glDeleteProgram;
PFNGLDELETESHADERPROC glDeleteShader;
PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;
PFNGLENABLEVERTEXARRAYATTRIBPROC glEnableVertexArrayAttrib;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
PFNGLGETSHADERIVPROC glGetShaderiv;
PFNGLLINKPROGRAMPROC glLinkProgram;
PFNGLNAMEDBUFFERSTORAGEPROC glNamedBufferStorage;
PFNGLSHADERSOURCEPROC glShaderSource;
PFNGLUSEPROGRAMPROC glUseProgram;
PFNGLVERTEXARRAYATTRIBBINDINGPROC glVertexArrayAttribBinding;
PFNGLVERTEXARRAYATTRIBFORMATPROC glVertexArrayAttribFormat;
PFNGLVERTEXARRAYVERTEXBUFFERPROC glVertexArrayVertexBuffer;
PFNGLVERTEXATTRIB4FVPROC glVertexAttrib4fv;

#define ARRAY_COUNT(arr) (sizeof(arr) / sizeof(arr[0]))
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
        }

        GlobalRenderingContext = wglCreateContext(GlobalDeviceContext);
        wglMakeCurrent(GlobalDeviceContext, GlobalRenderingContext);

        RECT ClientRect = {0};
        GetClientRect(WindowHandle, &ClientRect);

        return 0;
    }
    case WM_SIZE:
    {
        // RECT ClientRect;
        // GetClientRect(WindowHandle, &ClientRect);

        // int Width = ClientRect.right;
        // int Height = ClientRect.bottom;
        // glViewport(0, 0, (GLsizei)Width, (GLsizei)Height);
        // glMatrixMode(GL_PROJECTION);
        // glLoadIdentity();
        // glOrtho(0, (GLdouble)Width, 0, (GLdouble)Height, -1, 1);
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
    "#version 450 core                                                         \n",
    // "layout (location = 1) in vec4 my_offset;           \n",
    // "layout (location = 2) in vec4 color;                                      \n",
    "layout (location = 1) in vec4 position;                                   \n",
    // "out vec4 vs_color;                                                        \n",
    "void main(void)                                                           \n",
    "{                                                                         \n",
    // "    const vec4 vertices[3] = vec4[3](vec4(0.25, -0.25, 0.5, 1.0),         \n",
    // "                                     vec4(-0.25, -0.25, 0.5, 1.0),        \n",
    // "                                     vec4(0.25, 0.25, 0.5, 1.0));         \n",
    // "    gl_Position = vertices[gl_VertexID] + my_offset; \n",
    // "    gl_Position = vertices[gl_VertexID];                                  \n",
    "    gl_Position = position;                                               \n",
    // "    vs_color = color;                                                     \n"
    "}                                                                         \n",
};

static const char *tessControlShaderSource[] =
{
    "#version 450 core                                       \n",
    "layout (vertices = 3) out;                              \n",
    "void main(void)                                         \n",
    "{                                                       \n",
        "if (gl_InvocationID == 0)                           \n",
        "{                                                   \n",
            "gl_TessLevelInner[0] = 5.0;                     \n",
            "gl_TessLevelOuter[0] = 5.0;                     \n",
            "gl_TessLevelOuter[1] = 5.0;                     \n",
            "gl_TessLevelOuter[2] = 5.0;                     \n",
        "}                                                   \n",
        "gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position; \n",
    "}                                                       \n",
};

static const char *tessEvaluationShaderSource[] =
{
    "#version 450 core                                            \n",
    "layout (triangles, equal_spacing, cw) in;                    \n",
    "void main(void)                                              \n",
    "{                                                            \n",
    "    gl_Position = (gl_TessCoord.x * gl_in[0].gl_Position +   \n",
    "                   gl_TessCoord.y * gl_in[1].gl_Position +   \n",
    "                   gl_TessCoord.z * gl_in[2].gl_Position);   \n",
    "}                                                            \n",
};

static const char *geometryShaderSource[] =
{
    "#version 450 core                                 \n",
    "layout (triangles) in;                            \n",
    "layout (points, max_vertices = 3) out;            \n",
    "void main(void)                                   \n",
    "{                                                 \n",
        "int i;                                        \n",
        "for (i = 0; i < gl_in.length(); i++)          \n",
        "{                                             \n",
            "gl_Position = gl_in[i].gl_Position;       \n",
            "EmitVertex();                             \n",
    "    }                                             \n",
    "}                                                 \n",
};

static const char *fragmentShaderSource[] =
{
    "#version 450 core                     \n",
    // "in vec4 vs_color;                     \n",
    "out vec4 color;                       \n",
    "void main(void)                       \n",
    "{                                     \n",
    // "    color = vs_color;                 \n",
    "    color = vec4(0.0, 0.8, 0.2, 1.0); \n",
    "}                                     \n"
};

GLuint compileShader(GLenum shaderType, const GLchar *shaderSource[], GLsizei numLines)
{
    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, numLines, shaderSource, NULL);
    glCompileShader(shader);
    GLint compileResult;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileResult);
    if (compileResult != GL_TRUE)
    {
        char info[256];
        GLsizei length;
        glGetShaderInfoLog(shader, 256, &length, info);
        OutputDebugString(info);
        exit(1);
    }

    return shader;
}

GLuint compileShaders()
{
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource,
                                        ARRAY_COUNT(vertexShaderSource));
    GLuint tessControlShader = compileShader(GL_TESS_CONTROL_SHADER, tessControlShaderSource,
                                             ARRAY_COUNT(tessControlShaderSource));
    GLuint tessEvaluationShader = compileShader(GL_TESS_EVALUATION_SHADER, tessEvaluationShaderSource,
                                                ARRAY_COUNT(tessEvaluationShaderSource));
    GLuint geometryShader = compileShader(GL_GEOMETRY_SHADER, geometryShaderSource,
                                          ARRAY_COUNT(geometryShaderSource));
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource,
                                          ARRAY_COUNT(fragmentShaderSource));

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, tessControlShader);
    glAttachShader(program, tessEvaluationShader);
    // glAttachShader(program, geometryShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    glDeleteShader(vertexShader);
    glDeleteShader(tessControlShader);
    glDeleteShader(tessEvaluationShader);
    glDeleteShader(geometryShader);
    glDeleteShader(fragmentShader);

    return program;
}

#define LOAD_GL_FUNC(name, uppername) name = (PFNGL##uppername##PROC)GetAnyGLFuncAddress(#name)

void loadOpenGLFunctions()
{
    LOAD_GL_FUNC(glClearBufferfv, CLEARBUFFERFV);
    LOAD_GL_FUNC(glCreateShader, CREATESHADER);
    LOAD_GL_FUNC(glShaderSource, SHADERSOURCE);
    LOAD_GL_FUNC(glCompileShader, COMPILESHADER);
    LOAD_GL_FUNC(glCreateProgram, CREATEPROGRAM);
    LOAD_GL_FUNC(glAttachShader, ATTACHSHADER);
    LOAD_GL_FUNC(glLinkProgram, LINKPROGRAM);
    LOAD_GL_FUNC(glDeleteShader, DELETESHADER);
    LOAD_GL_FUNC(glCreateVertexArrays, CREATEVERTEXARRAYS);
    LOAD_GL_FUNC(glBindVertexArray, BINDVERTEXARRAY);
    LOAD_GL_FUNC(glDeleteVertexArrays, DELETEVERTEXARRAYS);
    LOAD_GL_FUNC(glDeleteProgram, DELETEPROGRAM);
    LOAD_GL_FUNC(glUseProgram, USEPROGRAM);
    LOAD_GL_FUNC(glGetShaderInfoLog, GETSHADERINFOLOG);
    LOAD_GL_FUNC(glGetShaderiv, GETSHADERIV);
    LOAD_GL_FUNC(glVertexAttrib4fv, VERTEXATTRIB4FV);
    LOAD_GL_FUNC(glCreateBuffers, CREATEBUFFERS);
    LOAD_GL_FUNC(glNamedBufferStorage, NAMEDBUFFERSTORAGE);
    LOAD_GL_FUNC(glBufferSubData, BUFFERSUBDATA);
    LOAD_GL_FUNC(glVertexArrayAttribBinding, VERTEXARRAYATTRIBBINDING);
    LOAD_GL_FUNC(glVertexArrayVertexBuffer, VERTEXARRAYVERTEXBUFFER);
    LOAD_GL_FUNC(glVertexArrayAttribFormat, VERTEXARRAYATTRIBFORMAT);
    LOAD_GL_FUNC(glEnableVertexArrayAttrib, ENABLEVERTEXARRAYATTRIB);
    LOAD_GL_FUNC(glBindBuffer, BINDBUFFER);
}

void render(f32 dt, GLuint program)
{
    (void)dt;
    // const GLfloat color[] = {(f32)sinf(dt) * 0.5f + 0.5f, (f32)cosf(dt) * 0.5f + 0.5f, 0.0f, 1.0f};
    const GLfloat red[] = {1.0f, 0.0f, 0.0f, 1.0f};
    glClearBufferfv(GL_COLOR, 0, red);
    glUseProgram(program);
    // GLfloat offset_attrib[] = {(f32)sinf(dt) * 0.5f, (f32)cosf(dt) * 0.6f, 0.0f, 0.0f};
    // glVertexAttrib4fv(1, offset_attrib);
    // const GLfloat triangleColor[] = {0.0f, 0.8f, 0.2f, 1.0f};
    // glVertexAttrib4fv(2, triangleColor);

    // glPointSize(5.0f);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawArrays(GL_PATCHES, 0, 3);
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

    GLuint gpuBuffer;
    glCreateBuffers(1, &gpuBuffer);
    glNamedBufferStorage(gpuBuffer, 1024 * 1024, NULL, GL_MAP_WRITE_BIT);
    glBindBuffer(GL_ARRAY_BUFFER, gpuBuffer);

    const f32 triangleVertices[] =
    {
        0.25f, -0.25f, 0.5f, 1.0f,
        -0.25f, -0.25f, 0.5f, 1.0f,
        0.25f, 0.25f, 0.5f, 1.0f
    };

    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(triangleVertices), triangleVertices);
    glVertexArrayVertexBuffer(vertexArrayObject, 0, gpuBuffer, 0, 4 * sizeof(f32));
    glVertexArrayAttribFormat(vertexArrayObject, 1, 4, GL_FLOAT, GL_FALSE, 0);
    glEnableVertexArrayAttrib(vertexArrayObject, 1);

    LARGE_INTEGER start = win32GetTicks();
    if (WindowHandle)
    {
        ShowWindow(WindowHandle, CmdShow);
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

            f32 seconds_elapsed = win32GetSecondsElapsed(start, currentTick);
            render(seconds_elapsed, program);

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
            // char dt_str[32];
            // snprintf(dt_str, 32, "MS per frame: %f\n", dt);
            // OutputDebugString(dt_str);
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
