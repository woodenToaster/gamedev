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

#include "gamedev_math.h"

PFNGLATTACHSHADERPROC glAttachShader;
PFNGLBINDBUFFERPROC glBindBuffer;
PFNGLBINDTEXTUREUNITPROC glBindTextureUnit;
PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
PFNGLBUFFERDATAPROC glBufferData;
PFNGLBUFFERSUBDATAPROC glBufferSubData;
PFNGLCLEARBUFFERFVPROC glClearBufferfv;
PFNGLCOMPILESHADERPROC glCompileShader;
PFNGLCREATEBUFFERSPROC glCreateBuffers;
PFNGLCREATEPROGRAMPROC glCreateProgram;
PFNGLCREATESHADERPROC glCreateShader;
PFNGLCREATETEXTURESPROC glCreateTextures;
PFNGLCREATEVERTEXARRAYSPROC glCreateVertexArrays;
PFNGLDELETEPROGRAMPROC glDeleteProgram;
PFNGLDELETESHADERPROC glDeleteShader;
PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;
PFNGLENABLEVERTEXARRAYATTRIBPROC glEnableVertexArrayAttrib;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
PFNGLGETSHADERIVPROC glGetShaderiv;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
PFNGLLINKPROGRAMPROC glLinkProgram;
PFNGLNAMEDBUFFERSTORAGEPROC glNamedBufferStorage;
PFNGLSHADERSOURCEPROC glShaderSource;
PFNGLTEXTUREPARAMETERIPROC glTextureParameteri;
PFNGLTEXTUREPARAMETERIVPROC glTextureParameteriv;
PFNGLTEXTURESTORAGE2DPROC glTextureStorage2D;
PFNGLTEXTURESUBIMAGE2DPROC glTextureSubImage2D;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
PFNGLUSEPROGRAMPROC glUseProgram;
PFNGLVERTEXARRAYATTRIBBINDINGPROC glVertexArrayAttribBinding;
PFNGLVERTEXARRAYATTRIBFORMATPROC glVertexArrayAttribFormat;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
PFNGLVERTEXARRAYVERTEXBUFFERPROC glVertexArrayVertexBuffer;
PFNGLVERTEXATTRIB4FVPROC glVertexAttrib4fv;


#define ARRAY_COUNT(arr) (sizeof(arr) / sizeof(arr[0]))
#define global static

global int GlobalRunning;
global i64 globalPerfFrequency;

static const GLfloat cubeVertexPositions[] =
{
    -0.25f,  0.25f, -0.25f,
    -0.25f, -0.25f, -0.25f,
    0.25f, -0.25f, -0.25f,

    0.25f, -0.25f, -0.25f,
    0.25f,  0.25f, -0.25f,
    -0.25f,  0.25f, -0.25f,

    0.25f, -0.25f, -0.25f,
    0.25f, -0.25f,  0.25f,
    0.25f,  0.25f, -0.25f,

    0.25f, -0.25f,  0.25f,
    0.25f,  0.25f,  0.25f,
    0.25f,  0.25f, -0.25f,

    0.25f, -0.25f,  0.25f,
    -0.25f, -0.25f,  0.25f,
    0.25f,  0.25f,  0.25f,

    -0.25f, -0.25f,  0.25f,
    -0.25f,  0.25f,  0.25f,
    0.25f,  0.25f,  0.25f,

    -0.25f, -0.25f,  0.25f,
    -0.25f, -0.25f, -0.25f,
    -0.25f,  0.25f,  0.25f,

    -0.25f, -0.25f, -0.25f,
    -0.25f,  0.25f, -0.25f,
    -0.25f,  0.25f,  0.25f,

    -0.25f, -0.25f,  0.25f,
    0.25f, -0.25f,  0.25f,
    0.25f, -0.25f, -0.25f,

    0.25f, -0.25f, -0.25f,
    -0.25f, -0.25f, -0.25f,
    -0.25f, -0.25f,  0.25f,

    -0.25f,  0.25f, -0.25f,
    0.25f,  0.25f, -0.25f,
    0.25f,  0.25f,  0.25f,

    0.25f,  0.25f,  0.25f,
    -0.25f,  0.25f,  0.25f,
    -0.25f,  0.25f, -0.25f
};

static const GLfloat rectVertexPositions[] =
{
    -0.5f, -0.5f, 0.0f,
    -0.5f, 0.5f, 0.0f,
    0.5f, 0.5f, 0.0f,

    -0.5f, -0.5f, 0.0f,
    0.5f, 0.5f, 0.0f,
    0.5f, -0.5f, 0.0f
};

struct EntireFile
{
    u8 *contents;
    u64 size;
};
struct LoadedBitmap
{
    u32 *pixels;
    u32 width;
    u32 height;
};

enum PlatformErrorType
{
    PlatformError_Fatal,
    PlatformError_Warning
};

static void win32ErrorMessage(PlatformErrorType type, char *message)
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
            PIXELFORMATDESCRIPTOR desiredPixelFormat = {};
            desiredPixelFormat.nSize = sizeof(PIXELFORMATDESCRIPTOR);
            desiredPixelFormat.nVersion = 1;
            desiredPixelFormat.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
            desiredPixelFormat.iPixelType = PFD_TYPE_RGBA;
            desiredPixelFormat.cColorBits = 32;
            desiredPixelFormat.cAlphaBits = 8;

            HDC deviceContext = GetDC(WindowHandle);

            int suggestedPixelFormatIndex = ChoosePixelFormat(deviceContext, &desiredPixelFormat);
            if (!suggestedPixelFormatIndex)
            {
                MessageBox(NULL, "ChoosePixelFormat failed", "Error", MB_OK);
                PostQuitMessage(0);
                return 0;
            }

            PIXELFORMATDESCRIPTOR suggestedPixelFormat = {};
            if (!DescribePixelFormat(deviceContext, suggestedPixelFormatIndex,
                                     sizeof(suggestedPixelFormat), &suggestedPixelFormat))
            {
                MessageBox(NULL, "DescribePixelFormat failed", "Error", MB_OK);
                PostQuitMessage(0);
                return 0;
            }

            if (SetPixelFormat(deviceContext, suggestedPixelFormatIndex, &suggestedPixelFormat) == FALSE)
            {
                MessageBox(NULL, "SetPixelFormat failed", "Error", MB_OK);
                PostQuitMessage(0);
            }

            HGLRC renderingContext = wglCreateContext(deviceContext);
            wglMakeCurrent(deviceContext, renderingContext);
            
            ReleaseDC(WindowHandle, deviceContext);

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
        {
            PostQuitMessage(0);
            return 0;
        }
        case WM_CLOSE:
        {
            GlobalRunning = 0;
            return 0;
        }
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
    "#version 450 core                                           \n",
    // "#version 330 core                                           \n",
    "layout (location = 1) in vec4 in_position;                  \n",
    "layout (location = 2) in vec2 in_tex_coord;                 \n",
    "out vec2 vs_tex_coord;                                      \n",
    "uniform mat4 modelView;                                     \n",
    "uniform mat4 projection;                                    \n",
    "void main(void)                                             \n",
    "{                                                           \n",
    "    vec4 pos = projection * modelView * in_position;        \n",
    // "    gl_PointSize = (1.0 - pos.z / pos.w) * 64.0;            \n",
    "    gl_Position = in_position;                                      \n",
    "    vs_tex_coord = in_tex_coord;                            \n",
    // "    gl_Position = pos;                                      \n",
    // "    vs_out.color = position * 2 + vec4(0.5, 0.5, 0.5, 0.0); \n",
    "}                                                           \n",
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
    "#version 450 core                            \n",
    // "#version 330 core                            \n",
    "uniform sampler2D tex;                       \n",
    "out vec4 color;                              \n",
    "in vec2 vs_tex_coord;                        \n",
    "void main(void)                              \n",
    "{                                            \n",
    // "    color = fs_in.color;                  \n",
    "    color = texture(tex, vs_tex_coord);      \n",
    "}                                            \n"
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
    // glAttachShader(program, tessControlShader);
    // glAttachShader(program, tessEvaluationShader);
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
    LOAD_GL_FUNC(glBufferData, BUFFERDATA);
    LOAD_GL_FUNC(glVertexAttribPointer, VERTEXATTRIBPOINTER);
    LOAD_GL_FUNC(glEnableVertexAttribArray, ENABLEVERTEXATTRIBARRAY);
    LOAD_GL_FUNC(glUniformMatrix4fv, UNIFORMMATRIX4FV);
    LOAD_GL_FUNC(glGetUniformLocation, GETUNIFORMLOCATION);
    LOAD_GL_FUNC(glCreateTextures, CREATETEXTURES);
    LOAD_GL_FUNC(glTextureStorage2D, TEXTURESTORAGE2D);
    LOAD_GL_FUNC(glTextureSubImage2D, TEXTURESUBIMAGE2D);
    LOAD_GL_FUNC(glBindTextureUnit, BINDTEXTUREUNIT);
    LOAD_GL_FUNC(glTextureParameteri, TEXTUREPARAMETERI);
    LOAD_GL_FUNC(glTextureParameteriv, TEXTUREPARAMETERIV);
}

void render(f32 dt, GLuint program, GLint projectionLocation, GLint modelViewLocation, f32 aspect)
{
    (void)dt;
    const GLfloat darkGreen[] = {0.0f, 0.25f, 0.0f, 1.0f};
    GLfloat one = 1.0f;
    glClearBufferfv(GL_COLOR, 0, darkGreen);
    glClearBufferfv(GL_DEPTH, 0, &one);
    glUseProgram(program);

    f32 f = (f32)dt * 0.3f;
    Mat4 zTrans = makeTranslationMat4(0.0f, 0.0f, -4.0f);
    Mat4 ovalTrans = makeTranslationMat4(sinf(2.1f * f) * 0.5f, cosf(1.7f * f) * 0.5f,
                                         sinf(1.3f * f) * cosf(1.5f * f) * 2.0f);
    Mat4 rotY = makeRotationMat4(dt * 45.0f, 0.0f, 1.0f, 0.0f);
    Mat4 rotX = makeRotationMat4(dt * 81.0f, 1.0f, 0.0f, 0.0f);

    Mat4 temp1 = multiplyMat4(&rotY, &rotX);
    Mat4 temp2 = multiplyMat4(&ovalTrans, &temp1);
    // Mat4 modelView = multiplyMat4(&zTrans, &temp2);
    Mat4 identity = identityMat4();
    Mat4 modelView = multiplyMat4(&identity, &zTrans);
    Mat4 projection = makePerspectiveMat4(50.0f, aspect, 0.1f, 1000.0f);
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, projection.data);
    glUniformMatrix4fv(modelViewLocation, 1, GL_FALSE, modelView.data);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
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

    int WindowWidth = 800;
    int WindowHeight = 600;
    HWND WindowHandle = CreateWindowEx(0, WindowClass.lpszClassName, "OpenGL", WS_OVERLAPPEDWINDOW,
                                       CW_USEDEFAULT, CW_USEDEFAULT, WindowWidth, WindowHeight,
                                       0, 0, Instance, 0);

    loadOpenGLFunctions();
    GLuint program = compileShaders();

    GLuint vertexArrayObject;
    glCreateVertexArrays(1, &vertexArrayObject);
    glBindVertexArray(vertexArrayObject);

    static const GLfloat quadData[] =
    {
        // Vertex positions
        -1.0f, -1.0f, 0.0f, 1.0f,
        1.0f, -1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 0.0f, 1.0f,
        -1.0f, 1.0f, 0.0f, 1.0f,
        // Texture coordinates
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f
    };

    GLuint cubeBuffer;
    glCreateBuffers(1, &cubeBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, cubeBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadData), quadData, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid *)(16 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    GLint projectionLocation = glGetUniformLocation(program, "projection");
    GLint modelViewLocation = glGetUniformLocation(program, "modelView");
    glViewport(0, 0, WindowWidth, WindowHeight);
    // glEnable(GL_CULL_FACE);
    // glFrontFace(GL_CW);
    // glEnable(GL_DEPTH_TEST);
    // glDepthFunc(GL_LEQUAL);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    LoadedBitmap linkLoadedBitmap = win32LoadBitmap("../../data/sprites/link_walking.bmp");
    GLuint linkTexture;
    glCreateTextures(GL_TEXTURE_2D, 1, &linkTexture);
    glBindTexture(GL_TEXTURE_2D, linkTexture);
    glTextureStorage2D(linkTexture, 1, GL_RGBA8, linkLoadedBitmap.width, linkLoadedBitmap.height);
    glTextureSubImage2D(linkTexture, 0, 0, 0, linkLoadedBitmap.width, linkLoadedBitmap.height, GL_BGRA,
                        GL_UNSIGNED_INT_8_8_8_8_REV, linkLoadedBitmap.pixels);

    LARGE_INTEGER start = win32GetTicks();
    if (WindowHandle)
    {
        HDC deviceContext = GetDC(WindowHandle);
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
            f32 aspect = WindowWidth / (f32)WindowHeight;
            render(seconds_elapsed, program, projectionLocation, modelViewLocation, aspect);

            SwapBuffers(deviceContext);

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
    glDeleteTextures(1, &linkTexture);
    glDeleteProgram(program);

    return 0;
}
