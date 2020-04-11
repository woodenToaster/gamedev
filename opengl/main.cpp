#include <assert.h>
#include <stdio.h>
#include <stdint.h>

#include <windows.h>
#include <GL/gl.h>
#include "glext.h"

#include "gamedev_platform.h"
#include "win32_gamedev.h"
#include "map_data.h"

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
PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;
PFNGLENABLEVERTEXARRAYATTRIBPROC glEnableVertexArrayAttrib;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
PFNGLGENBUFFERSPROC glGenBuffers;
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
PFNGLUNIFORM4FVPROC glUniform4fv;
PFNGLUNIFORM4FPROC glUniform4f;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
PFNGLUSEPROGRAMPROC glUseProgram;
PFNGLVERTEXARRAYATTRIBBINDINGPROC glVertexArrayAttribBinding;
PFNGLVERTEXARRAYATTRIBFORMATPROC glVertexArrayAttribFormat;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
PFNGLVERTEXARRAYVERTEXBUFFERPROC glVertexArrayVertexBuffer;
PFNGLVERTEXATTRIB4FVPROC glVertexAttrib4fv;

typedef BOOL (*PFNWGLSWAPINTERVALEXTPROC)(int interval);
PFNWGLSWAPINTERVALEXTPROC wglSwapInterval;

global Win32State globalWin32State;
global Win32BackBuffer globalBackBuffer;

global int globalRunning;
global i64 globalPerfFrequency;

enum Direction
{
    Direction_Up,
    Direction_Up_Right,
    Direction_Right,
    Direction_Down_Right,
    Direction_Down,
    Direction_Down_Left,
    Direction_Left,
    Direction_Up_Left,

    Direction_COUNT
};

struct Rect2
{
    Vec2 minP;
    Vec2 maxP;
};

struct Animation
{
    int totalFrames;
    int currentFrame;
    u32 delay;
    u64 elapsed;
};

struct Player
{
    Vec2 position;
    Vec2 size;
    Direction direction;
    Animation animation;
};

struct Camera
{
    Vec3 position;
    Vec3 up;
    Vec3 right;
    Vec3 direction;
};

enum PlatformErrorType
{
    PlatformError_Fatal,
    PlatformError_Warning
};

static void updateAnimation(Animation* a, u64 elapsed_last_frame, b32 active)
{
    a->elapsed += elapsed_last_frame;
    if (a->elapsed > a->delay && active) {
        a->currentFrame++;
        if (a->currentFrame > a->totalFrames - 1) {
            a->currentFrame = 0;
        }
        a->elapsed = 0;
    }
}

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
            globalRunning = 0;
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

void *getOpenGLProcAddress(const char *name)
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
    "out vec4 vs_color;                                          \n",
    "uniform mat4 model;                                         \n",
    "uniform mat4 view;                                          \n",
    "uniform mat4 projection;                                    \n",
    "uniform vec4 ucolor;                                        \n",
    "void main(void)                                             \n",
    "{                                                           \n",
    "    gl_Position = projection * view * model * in_position;  \n",
    "    vs_color = ucolor;                                      \n",
    "    vs_tex_coord = in_tex_coord;                            \n",
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
    "#version 450 core                                  \n",
    // "#version 330 core                                 \n",
    "uniform sampler2D tex;                             \n",
    "out vec4 color;                                    \n",
    "in vec2 vs_tex_coord;                              \n",
    "in vec4 vs_color;                                  \n",
    "void main(void)                                    \n",
    "{                                                  \n",
    "    color = texture(tex, vs_tex_coord) + vs_color; \n",
    "}                                                  \n"
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

#define LOAD_GL_FUNC(name, uppername) name = (PFNGL##uppername##PROC)getOpenGLProcAddress(#name)

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
    LOAD_GL_FUNC(glGenBuffers, GENBUFFERS);
    LOAD_GL_FUNC(glUniform4fv, UNIFORM4FV);
    LOAD_GL_FUNC(glUniform4f, UNIFORM4F);
    LOAD_GL_FUNC(glDisableVertexAttribArray, DISABLEVERTEXATTRIBARRAY);
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

void drawOpenGLBitmap(Player *player, GLuint quadBuffer, GLint ucolorLocation)
{
    f32 spriteWidth = 24.0f;
    f32 spriteHeight = 32.0f;
    f32 sheetWidth = 264.0f;
    f32 sheetHeight = 160.0f;

    Rect2 spriteRect = {};
    spriteRect.minP = player->position; //  - camera.viewport.minP;
    spriteRect.maxP = spriteRect.minP + player->size;

    int frame = player->animation.currentFrame;

    f32 texMinX = (spriteWidth * frame) / sheetWidth;
    f32 texMaxX = (spriteWidth * (frame + 1)) / sheetWidth;

    int sheetRow;
    switch (player->direction)
    {
        case Direction_Up:
        {
            sheetRow = 3;
        } break;
        case Direction_Down:
        {
            sheetRow = 0;
        } break;
        case Direction_Left:
        {
            sheetRow = 1;
        } break;
        case Direction_Right:
        {
            sheetRow = 4;
        } break;
        default:
        {
            sheetRow = 0;
        }
    }

    f32 texMinY = (sheetRow * spriteHeight) / sheetHeight;
    // TODO(chogan): Not sure why I have to subtract from the spriteHeight.
    // If I don't, I get a small sliver of the sprite above.
    f32 texMaxY = texMinY + ((spriteHeight - 2) / sheetHeight);

    f32 z = -9.0f;
    f32 w = 1.0f;
    f32 quad[] =
    {
        // Positions
        spriteRect.minP.x, spriteRect.minP.y, z, w,
        spriteRect.maxP.x, spriteRect.minP.y, z, w,
        spriteRect.maxP.x, spriteRect.maxP.y, z, w,
        spriteRect.minP.x, spriteRect.maxP.y, z, w,
        // Texture coordinates
        texMinX, texMinY,
        texMaxX, texMinY,
        texMaxX, texMaxY,
        texMinX, texMaxY,
    };

    glEnableVertexAttribArray(2);
    {
        glBindBuffer(GL_ARRAY_BUFFER, quadBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
        glUniform4f(ucolorLocation, 0.0f, 0.0f, 0.0f, 0.0f);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }
    glDisableVertexAttribArray(2);
}

#if 0
void drawOpenGLFilledRect(Rect2 rect, Vec3u8 color)
{
    glBegin(GL_TRIANGLES);

    glColor3f(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f);

    glVertex4f(rect.minP.x, rect.minP.y, 0, 1);
    glVertex4f(rect.maxP.x, rect.minP.y, 0, 1);
    glVertex4f(rect.maxP.x, rect.maxP.y, 0, 1);
    glVertex4f(rect.minP.x, rect.minP.y, 0, 1);
    glVertex4f(rect.maxP.x, rect.maxP.y, 0, 1);
    glVertex4f(rect.minP.x, rect.maxP.y, 0, 1);

    glEnd();
}
#else
void drawOpenGLFilledRect(Rect2 rect, Vec3u8 color, GLuint quadBuffer, GLint ucolorLocation)
{
    f32 z = -9.0f;
    f32 quad[] =
    {
        rect.minP.x, rect.minP.y, z, 1.0f,
        rect.maxP.x, rect.minP.y, z, 1.0f,
        rect.maxP.x, rect.maxP.y, z, 1.0f,
        rect.minP.x, rect.maxP.y, z, 1.0f,
    };

    glBindBuffer(GL_ARRAY_BUFFER, quadBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
    glUniform4f(ucolorLocation, color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, 1.0f);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}
#endif

internal void win32SetKeyState(Input *input, Key key, b32 isDown)
{
    if (input->keyDown[key] && !isDown)
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
        case VK_UP:
        {
            win32SetKeyState(input, Key_Up, isDown);
        } break;
        case 'A':
        case VK_LEFT:
        {
            win32SetKeyState(input, Key_Left, isDown);
        } break;
        case 'S':
        case VK_DOWN:
        {
            win32SetKeyState(input, Key_Down, isDown);
        } break;
        case 'D':
        case VK_RIGHT:
        {
            win32SetKeyState(input, Key_Right, isDown);
        } break;
        case 'X':
        {
            win32SetKeyState(input, Key_X, isDown);
        } break;
        case 'Z':
        {
            win32SetKeyState(input, Key_Z, isDown);
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
    MSG message;
    while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
    {
        switch (message.message)
        {
            case WM_QUIT:
            {
                globalRunning = false;
            } break;
            case WM_SYSKEYUP:
            case WM_KEYUP:
            case WM_SYSKEYDOWN:
            case WM_KEYDOWN:
            {
                // b32 altWasDown = message.lParam & (1 << 29);
                // b32 shiftWasDown = message.lParam & (1 << 15);

                b32 wasDown = (message.lParam & (1 << 30)) != 0;
                b32 isDown = (message.lParam & (1UL << 31)) == 0;

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
        globalRunning = false;
    }

    if (input->keyPressed[Key_F5])
    {
        // toggleFullscreen(message.hwnd);
    }
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
    WindowClass.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClass(&WindowClass);

    int windowWidth = 1920 / 2;
    int windowHeight = 1080 / 2;
    HWND WindowHandle = CreateWindowEx(0, WindowClass.lpszClassName, "OpenGL", WS_OVERLAPPEDWINDOW,
                                       CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                                       0, 0, Instance, 0);

    f32 tileWidthMeters = 1.0f;
    f32 tileHeightMeters = 1.0f;
    f32 metersToPixels = 60.f;
    f32 pixelsToMeters = 1.0f / metersToPixels;
    f32 viewportWidthInMeters = windowWidth * pixelsToMeters;
    f32 viewportHeightInMeters = windowHeight * pixelsToMeters;

    loadOpenGLFunctions();
    GLuint program = compileShaders();
    glUseProgram(program);

    // GLuint vao;
    // glCreateVertexArrays(1, &vao);
    // glBindVertexArray(vao);

    u32 quadBuffer;
    glGenBuffers(1, &quadBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, quadBuffer);

    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid *)(16 * sizeof(GLfloat)));

    static const GLuint quadIndices[] = {0, 1, 2, 0, 2, 3};

    GLuint ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, GL_STATIC_DRAW);

    GLint ucolorLocation = glGetUniformLocation(program, "ucolor");
    GLint modelLocation = glGetUniformLocation(program, "model");
    GLint viewLocation = glGetUniformLocation(program, "view");
    GLint projectionLocation = glGetUniformLocation(program, "projection");

    Mat4 model = identityMat4();
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, model.data);

    Mat4 projection = makePerspectiveMat4(45, viewportWidthInMeters / viewportHeightInMeters, 0.1f,
                                          100.0f);
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, projection.data);

    RECT ClientRect;
    GetClientRect(WindowHandle, &ClientRect);

    int clientWidth = ClientRect.right;
    int clientHeight = ClientRect.bottom;

    int fringe = 5;
    glViewport(fringe, fringe, clientWidth - (2 * fringe), clientHeight - (2 * fringe));
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

    // TODO(chogan): Check for extension
    wglSwapInterval = (PFNWGLSWAPINTERVALEXTPROC)getOpenGLProcAddress("wglSwapIntervalEXT");
    wglSwapInterval(1);

    Camera camera = {};
    camera.position = vec3(0.0f, 0.0f, 1.75f);
    camera.up = vec3(0.0f, 1.0f, 0.0f);
    camera.right = vec3(1.0f, 0.0f, 0.0f);
    camera.direction = vec3(0.0f, 0.0f, 1.0);

    Input oldInput = {};
    Player player = {};
    player.position = vec2(0, 0);
    player.size = vec2(pixelsToMeters * 24, pixelsToMeters * 32);
    player.direction = Direction_Down;
    player.animation.totalFrames = 8;
    player.animation.delay = 80;

    LARGE_INTEGER start = win32GetTicks();
    if (WindowHandle)
    {
        HDC deviceContext = GetDC(WindowHandle);
        ShowWindow(WindowHandle, CmdShow);
        LARGE_INTEGER ElapsedMicroseconds = {0};
        globalRunning = 1;
        while (globalRunning)
        {
            LARGE_INTEGER StartTime;
            QueryPerformanceCounter(&StartTime);
            LARGE_INTEGER currentTick = win32GetTicks();

            // input
            Input newInput = {};

            for (int i = 0; i < Key_Count; ++i)
            {
                newInput.keyDown[i] = oldInput.keyDown[i];
            }

            newInput.dt = targetMsPerFrame;
            win32GetInput(&newInput);
            oldInput = newInput;

            // update
            f32 speed = 0.2f;
            if (newInput.keyDown[Key_Up])
            {
                player.position.y += speed;
                player.direction = Direction_Up;
            }
            if (newInput.keyDown[Key_Down])
            {
                player.position.y -= speed;
                player.direction = Direction_Down;
            }
            if (newInput.keyDown[Key_Left])
            {
                player.position.x -= speed;
                player.direction = Direction_Left;
            }
            if (newInput.keyDown[Key_Right])
            {
                player.position.x += speed;
                player.direction = Direction_Right;
            }
            if (newInput.keyPressed[Key_Z])
            {
                if (camera.position.z < 2.0f)
                {
                    camera.position.z = 90.0f;
                }
                else
                {
                    camera.position.z = 1.75f;
                }
            }

            f32 minCameraPx = 0.5f * viewportWidthInMeters;
            f32 minCameraPy = 0.5f * viewportHeightInMeters;
            f32 maxCameraPx = worldWidth - 0.5f * viewportWidthInMeters;
            f32 maxCameraPy = worldHeight - 0.5f * viewportHeightInMeters;
            f32 maxPlayerPx = worldWidth - player.size.x;
            f32 maxPlayerPy = worldHeight - player.size.y;

            player.position.x = clampFloat(player.position.x, 0, maxPlayerPx);
            player.position.y = clampFloat(player.position.y, 0, maxPlayerPy);

            updateAnimation(&player.animation, newInput.dt, true);

            // Center camera over player
            camera.position.x = player.position.x + 0.5f * player.size.x;
            camera.position.y = player.position.y + 0.5f * player.size.y;
            camera.position.x = clampFloat(camera.position.x, minCameraPx, maxCameraPx);
            camera.position.y = clampFloat(camera.position.y, minCameraPy, maxCameraPy);

            // render
            glClearColor(1.0f, 0.0f, 1.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            Mat4 lookAtRot = {};
            lookAtRot.col1 = {camera.right.x, camera.up.x, camera.direction.x, 0};
            lookAtRot.col2 = {camera.right.y, camera.up.y, camera.direction.y, 0};
            lookAtRot.col3 = {camera.right.z, camera.up.z, camera.direction.z, 0};
            lookAtRot.col4 = {0, 0, 0, 1};

            Mat4 lookAtTrans = makeTranslationMat4(-camera.position.x, -camera.position.y,
                                                   -camera.position.z);

            Mat4 view = multiplyMat4(&lookAtRot, &lookAtTrans);
            glUniformMatrix4fv(viewLocation, 1, GL_FALSE, view.data);

            // World floor
            Rect2 floor =  {};
            floor.minP = vec2(0, 0);
            floor.maxP = vec2(worldWidth, worldHeight);
            drawOpenGLFilledRect(floor, vec3u8(135, 135, 135), quadBuffer, ucolorLocation);

            int rowStart = 0;
            int colStart = 0;
            int rowEnd = worldHeight;
            int colEnd = worldWidth;

            // TODO(chogan): @optimization Only process visible parts of the map
            // int rowStart = maxInt32((int)camera.viewport.minP.y - 1, 0);
            // int rowEnd = minInt32((int)camera.viewport.maxP.y + 2, worldHeight);
            // int colStart = maxInt32((int)camera.viewport.minP.x - 1, 0);
            // int colEnd = minInt32((int)camera.viewport.maxP.x + 2, worldWidth);

            for (int row = rowStart; row < rowEnd; ++row)
            {
                for (int col = colStart; col < colEnd; ++col)
                {
                    if (globalMapData[row][col])
                    {
                        Rect2 rect = {};
                        rect.minP = vec2((f32)col, (f32)row); // - camera.viewport.minP;
                        rect.maxP = rect.minP + vec2(tileWidthMeters, tileHeightMeters);
                        Vec3u8 tileColor = vec3u8(37, 71, 0);
                        drawOpenGLFilledRect(rect, tileColor, quadBuffer, ucolorLocation);
                    }
                }
            }

            drawOpenGLBitmap(&player, quadBuffer, ucolorLocation);

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
            // snprintf(dt_str, 32, "MS per frame: %f\n", (f32)dt);
            // OutputDebugString(dt_str);
        }
    }
    else
    {
        // TODO(cjh): logging
    }

    return 0;
}
