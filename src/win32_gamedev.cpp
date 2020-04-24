#include <windows.h>
#include <GL/gl.h>
#include "glext.h"

#include "gamedev_platform.h"
#include "gamedev_memory.h"
#include "win32_gamedev.h"

#include "gamedev_opengl.cpp"

global i64 globalPerfFrequency;
global WINDOWPLACEMENT globalWindowPosition = {sizeof(globalWindowPosition)};

global Win32State globalWin32State;
global Win32BackBuffer globalBackBuffer;

enum RenderingStyle
{
    RenderingStyle_OpenGLRenderAndDisplay,
    RenderingStyle_SoftwareRender_OpenGLDisplay,
    RenderingStyle_SofwareRender_GDIDisplay,
};

global RenderingStyle globalRenderingStyle;

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

void *win32GetOpenGLProcAddress(const char *name)
{
    void *proc = (void *)wglGetProcAddress(name);
    if (proc == 0 || (proc == (void*)0x1) || (proc == (void*)0x2) || (proc == (void*)0x3) ||
        (proc == (void*)-1))
    {
        HMODULE module = LoadLibraryA("opengl32.dll");
        proc = (void *)GetProcAddress(module, name);
    }

    return proc;
}

#define LOAD_GL_FUNC(name, uppername) name = (PFNGL##uppername##PROC)win32GetOpenGLProcAddress(#name)

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

internal void win32InitOpenGL(HWND window)
{
    PIXELFORMATDESCRIPTOR desiredPixelFormat = {};
    desiredPixelFormat.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    desiredPixelFormat.nVersion = 1;
    desiredPixelFormat.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    desiredPixelFormat.iPixelType = PFD_TYPE_RGBA;
    desiredPixelFormat.cColorBits = 32;
    desiredPixelFormat.cAlphaBits = 8;

    HDC deviceContext = GetDC(window);

    int suggestedPixelFormatIndex = ChoosePixelFormat(deviceContext, &desiredPixelFormat);
    if (!suggestedPixelFormatIndex)
    {
        win32ErrorMessage(PlatformError_Fatal, "ChoosePixelFormat failed");
    }

    PIXELFORMATDESCRIPTOR suggestedPixelFormat = {};
    if (!DescribePixelFormat(deviceContext, suggestedPixelFormatIndex,
                             sizeof(suggestedPixelFormat), &suggestedPixelFormat))
    {
        win32ErrorMessage(PlatformError_Fatal, "DescribePixelFormat failed");
    }

    if (SetPixelFormat(deviceContext, suggestedPixelFormatIndex, &suggestedPixelFormat) == FALSE)
    {
        win32ErrorMessage(PlatformError_Fatal, "SetPixelFormat failed");
    }

    HGLRC renderingContext = wglCreateContext(deviceContext);
    wglMakeCurrent(deviceContext, renderingContext);

    loadOpenGLFunctions();

    // NOTE(chogan): Enable vsync
    // TODO(chogan): Check for extension
    wglSwapInterval = (PFNWGLSWAPINTERVALEXTPROC)win32GetOpenGLProcAddress("wglSwapIntervalEXT");
    wglSwapInterval(1);

    ReleaseDC(window, deviceContext);
}

u32 safeU64ToU32(u64 val)
{
    assert(val <= 0xFFFFFFFF);
    u32 result = (u32)val;

    return result;
}

inline static u8 *win32AllocateMemory(size_t bytes)
{
    u8 *result = 0;
    result = (u8 *)VirtualAlloc(0, bytes, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    return result;
}

inline static void win32FreeMemory(void *memory)
{
    VirtualFree(memory, 0, MEM_RELEASE);
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
            result.contents = win32AllocateMemory(fileSize32);
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
    if (file)
    {
        win32FreeMemory(file->contents);
    }
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

TextureDims win32GetTextureDims(TextureHandle texture)
{
    TextureDims result = {};
    result.width = texture.bitmap.width;
    result.height = texture.bitmap.height;

    return result;
}


#if 0
internal LARGE_INTEGER win32GetTicks()
{
    LARGE_INTEGER result;
    QueryPerformanceCounter(&result);

    return result;
}

internal void win32ResizeDIBSection(Win32BackBuffer *buffer, int width, int height)
{
    if (buffer->memory)
    {
        VirtualFree(buffer->memory, 0, MEM_RELEASE);
    }

    buffer->width = width;
    buffer->height = height;

    buffer->bitmapInfo.bmiHeader.biSize = sizeof(buffer->bitmapInfo.bmiHeader);
    buffer->bitmapInfo.bmiHeader.biWidth = width;
    // NOTE(cjh): Negative height is a top-down bitmap
    buffer->bitmapInfo.bmiHeader.biHeight = -height;
    buffer->bitmapInfo.bmiHeader.biPlanes = 1;
    buffer->bitmapInfo.bmiHeader.biBitCount = 32;
    buffer->bitmapInfo.bmiHeader.biCompression = BI_RGB;

    buffer->bytesPerPixel = 4;
    int bitmapMemorySize = width * height * buffer->bytesPerPixel;
    buffer->memory = VirtualAlloc(0, bitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
}
#endif

void win32RenderFilledRect(void *renderer, Rect dest, Vec4u8 color)
{
    // TODO(chogan): Clipping
    Win32BackBuffer *backBuffer = (Win32BackBuffer *)renderer;

    u8 r = color.r;
    u8 g = color.g;
    u8 b = color.b;
    u8 a = color.a;

    if (isZeroRect(dest))
    {
        // NOTE(chogan): Fill the whole back buffer
        dest.w = backBuffer->width;
        dest.h = backBuffer->height;
    }

    int bytesPerPixel = backBuffer->bytesPerPixel;
    int pitch = backBuffer->width * bytesPerPixel;
    u8 *start = (u8 *)backBuffer->memory + (dest.y * pitch) + (dest.x * bytesPerPixel);

    int yMax = clampInt32(dest.y + dest.h, 0, backBuffer->height);
    int xMax = clampInt32(dest.x + dest.w, 0, backBuffer->width);
    for (int y = dest.y; y < yMax; ++y)
    {
        u32 *pixel = (u32 *)start;
        for (int x = dest.x; x < xMax; ++x)
        {
            *pixel++ = ((a << 24) | (r << 16) | (g << 8) | (b << 0));
        }
        start += pitch;
    }
}

void win32RenderRect(void *renderer, Rect dest, Vec4u8 color)
{
    // TODO(chogan): Clipping
    Win32BackBuffer *backBuffer = (Win32BackBuffer *)renderer;

    u8 r = color.r;
    u8 g = color.g;
    u8 b = color.b;
    u8 a = color.a;

    if (isZeroRect(dest))
    {
        // NOTE(chogan): Fill the whole back buffer
        dest.w = backBuffer->width;
        dest.h = backBuffer->height;
    }

    int bytesPerPixel = backBuffer->bytesPerPixel;
    int pitch = backBuffer->width * bytesPerPixel;
    u8 *start = (u8 *)backBuffer->memory + (dest.y * pitch) + (dest.x * bytesPerPixel);

    int yMax = clampInt32(dest.y + dest.h, 0, backBuffer->height);
    int xMax = clampInt32(dest.x + dest.w, 0, backBuffer->width);
    for (int y = dest.y; y < yMax; ++y)
    {
        u32 *pixel = (u32 *)start;

        if (y == dest.y || y == yMax - 1)
        {
            for (int x = dest.x; x < xMax; ++x)
            {
                *pixel++ = ((a << 24) | (r << 16) | (g << 8) | (b << 0));
            }
        }
        else
        {
            for (int x = dest.x; x < xMax; ++x)
            {
                if (x != dest.x || x != xMax - 1)
                {
                    continue;
                }
                *pixel++ = ((a << 24) | (r << 16) | (g << 8) | (b << 0));
            }
        }
        start += pitch;
    }
}

void win32RenderBitmap(void *renderer, LoadedBitmap bitmap, Rect sourceRect, Rect destRect)
{
    Win32BackBuffer *backBuffer = (Win32BackBuffer *)renderer;

    sourceRect.x = clampInt32(sourceRect.x, 0, bitmap.width - 1);

    if (sourceRect.y < 0)
    {
        sourceRect.y = 0;
    }
    if (sourceRect.y + sourceRect.h > (int)bitmap.height - 1)
    {
        sourceRect.y = bitmap.height - 1 - sourceRect.h;
    }

    int maxDestX = (backBuffer->width - 1) * backBuffer->bytesPerPixel;
    destRect.x = clampInt32(destRect.x, 0, maxDestX);

    if (destRect.y < 0)
    {
        destRect.y = 0;
    }

    size_t destOffset = (destRect.y * backBuffer->width + destRect.x) * backBuffer->bytesPerPixel;
    u8 *destRow = (u8 *)backBuffer->memory + destOffset;
    u8 *srcRow = (u8 *)bitmap.pixels + bitmap.width * (bitmap.height - 1) * backBuffer->bytesPerPixel;

    for (int y = 0; y < destRect.h; ++y)
    {
        u32 *dest = (u32 *)destRow;
        u32 *src = (u32 *)srcRow;

        if (destRect.y + y > backBuffer->height - 1)
        {
            break;
        }

        for (int x = 0; x < destRect.w; ++x)
        {
            if (destRect.x + x > backBuffer->width - 1)
            {
                break;
            }
            u8 a = (*src >> 24);
            f32 alpha = a / 255.0f;
            u8 bSrc = ((u8 *)src)[0];
            u8 gSrc = ((u8 *)src)[1];
            u8 rSrc = ((u8 *)src)[2];

            u8 bDest = ((u8 *)dest)[0];
            u8 gDest = ((u8 *)dest)[1];
            u8 rDest = ((u8 *)dest)[2];

            u8 rFinal = (u8)(alpha * rSrc + (1.0f - alpha) * rDest);
            u8 gFinal = (u8)(alpha * gSrc + (1.0f - alpha) * gDest);
            u8 bFinal = (u8)(alpha * bSrc + (1.0f - alpha) * bDest);
            *dest++ = ((a << 24) | (rFinal << 16) | (gFinal << 8) | (bFinal << 0));
            src++;
        }
        destRow += backBuffer->bytesPerPixel * backBuffer->width;
        srcRow -= backBuffer->bytesPerPixel * bitmap.width;
    }
}

void win32RenderSprite(void *renderer, TextureHandle sheet, Rect source, Rect dest)
{
    win32RenderBitmap(renderer, sheet.bitmap, source, dest);
}

internal void win32UpdateWindow(Win32BackBuffer *buffer, HDC deviceContext, int windowWidth,
                                int windowHeight, RenderCommands *renderCommands)
{
    int destWidth = buffer->width;
    int destHeight = buffer->height;

    if (windowWidth >= 2 * buffer->width && windowHeight >= 2 * buffer->height)
    {
        destWidth = 2 * buffer->width;
        destHeight = 2 * buffer->height;
    }

    if (globalRenderingStyle == RenderingStyle_OpenGLRenderAndDisplay)
    {
        drawOpenGLRenderGroup(renderCommands);
        SwapBuffers(deviceContext);
    }
    else
    {
        // TODO(chogan): Delete this path
        if (globalRenderingStyle == RenderingStyle_SoftwareRender_OpenGLDisplay)
        {
            glViewport(0, 0, destWidth, destHeight);
            glBindTexture(GL_TEXTURE_2D, 1);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, destWidth, destHeight, 0, GL_BGRA_EXT,
                         GL_UNSIGNED_BYTE, buffer->memory);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

            glEnable(GL_TEXTURE_2D);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glMatrixMode(GL_TEXTURE);
            glLoadIdentity();

            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();

            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();

            glBegin(GL_TRIANGLES);

            glTexCoord2f(0.0f, 0.0f);
            glVertex2f(-0.9f, -0.9f);

            glTexCoord2f(1.0f, 0.0);
            glVertex2f(0.9f, -0.9f);

            glTexCoord2f(1.0f, 1.0f);
            glVertex2f(0.9f, 0.9f);

            glTexCoord2f(0.0f, 0.0f);
            glVertex2f(-0.9f, -0.9f);

            glTexCoord2f(1.0f, 1.0f);
            glVertex2f(0.9f, 0.9f);

            glTexCoord2f(0.0f, 1.0f);
            glVertex2f(-0.9f, 0.9f);

            glEnd();

            SwapBuffers(deviceContext);
        }
        else
        {
            StretchDIBits(deviceContext,
                          0, 0, destWidth, destHeight,
                          0, 0, buffer->width, buffer->height,
                          buffer->memory,
                          &buffer->bitmapInfo, DIB_RGB_COLORS, SRCCOPY);
        }
    }
}

LRESULT CALLBACK win32MainWindowCallback(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;
    switch (message)
    {
        case WM_DESTROY:
        {
            globalWin32State.isRunning = false;
        } break;
        case WM_PAINT:
        {
            PAINTSTRUCT paint;
            BeginPaint(windowHandle, &paint);
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

internal void toggleFullscreen(HWND window)
{
    // NOTE(chogan): This follows Raymond Chen's prescription
    // for fullscreen toggling, see:
    // http://blogs.msdn.com/b/oldnewthing/archive/2010/04/12/9994016.aspx

    DWORD style = GetWindowLong(window, GWL_STYLE);
    if(style & WS_OVERLAPPEDWINDOW)
    {
        MONITORINFO monitorInfo = {sizeof(monitorInfo)};
        if(GetWindowPlacement(window, &globalWindowPosition) &&
           GetMonitorInfo(MonitorFromWindow(window, MONITOR_DEFAULTTOPRIMARY), &monitorInfo))
        {
            SetWindowLong(window, GWL_STYLE, style & ~WS_OVERLAPPEDWINDOW);
            SetWindowPos(window, HWND_TOP,
                         monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.top,
                         monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
                         monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    }
    else
    {
        SetWindowLong(window, GWL_STYLE, style | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(window, &globalWindowPosition);
        SetWindowPos(window, 0, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                     SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
}

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
                globalWin32State.isRunning = false;
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
        globalWin32State.isRunning = false;
    }

    if (input->keyPressed[Key_F5])
    {
        toggleFullscreen(message.hwnd);
    }
}

int CALLBACK WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR commandLine, int showCode)
{
    (void)prevInstance;
    (void)commandLine;
    (void)showCode;

    GameMemory memory = {};
    memory.permanentStorageSize = (size_t)GIGABYTES(2);
    memory.transientStorageSize = (size_t)MEGABYTES(32);
    memory.permanentStorage = VirtualAlloc(0, memory.permanentStorageSize + memory.transientStorageSize,
                                           MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    memory.transientStorage = (u8*)memory.permanentStorage + memory.permanentStorageSize;

    memory.platformAPI.readEntireFile = win32ReadEntireFile;
    memory.platformAPI.freeFileMemory = win32FreeFileMemory;
    // memory.platformAPI.getTicks = win32GetTicks;

    memory.rendererAPI.renderFilledRect = win32RenderFilledRect;
    memory.rendererAPI.renderRect = win32RenderRect;
    memory.rendererAPI.loadBitmap = win32LoadBitmap;
    memory.rendererAPI.renderBitmap = win32RenderBitmap;
    memory.rendererAPI.renderSprite = win32RenderSprite;
    memory.rendererAPI.getTextureDims = win32GetTextureDims;

#if 0
    memory.rendererAPI.destroyTexture = SDLDestroyTexture;
    memory.rendererAPI.setRenderDrawColor = SDLSetRenderDrawColor;
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

    i32 windowWidth = 1920 / 2;
    i32 windowHeight = 1080 / 2;

    // win32ResizeDIBSection(&globalBackBuffer, windowWidth, windowHeight);

    WNDCLASSA windowClass = {};
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

    RECT desiredClientRect = {};
    desiredClientRect.top = 0;
    desiredClientRect.bottom = windowHeight;
    desiredClientRect.left = 0;
    desiredClientRect.right = windowWidth;
    AdjustWindowRectEx(&desiredClientRect, windowClass.style | WS_OVERLAPPEDWINDOW | WS_VISIBLE, FALSE,
                       0);

    globalWin32State.window = CreateWindowExA(0, windowClass.lpszClassName, "Gamedev",
                                              WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT,
                                              CW_USEDEFAULT,
                                              desiredClientRect.right - desiredClientRect.left,
                                              desiredClientRect.bottom - desiredClientRect.top,
                                              // CW_USEDEFAULT, CW_USEDEFAULT,
                                              0, 0,
                                              instance, 0);

    if (!globalWin32State.window)
    {
        win32ErrorMessage(PlatformError_Fatal, "Cannot create window");
    }

    win32InitOpenGL(globalWin32State.window);

    u32 targetFps = 60;
    // TODO(chogan): This gets set when memory is initialized in gameUpdateAndRender.
    // Remove this once we're calling that function.
    u32 targetMsPerFrame = (u32)(1000.0f / (f32)targetFps);

    // TODO(cjh):
    // SDL_GameController *controllerHandles[MAX_CONTROLLERS] = {};
    // SDLInitControllers(&controllerHandles[0]);

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

    LoadedBitmap heroLoadedBitmap = win32LoadBitmap("sprites/link_walking.bmp");
    GLuint heroTexture;
    glCreateTextures(GL_TEXTURE_2D, 1, &heroTexture);
    glBindTexture(GL_TEXTURE_2D, heroTexture);
    glTextureStorage2D(heroTexture, 1, GL_RGBA8, heroLoadedBitmap.width, heroLoadedBitmap.height);
    glTextureSubImage2D(heroTexture, 0, 0, 0, heroLoadedBitmap.width, heroLoadedBitmap.height, GL_BGRA,
                        GL_UNSIGNED_INT_8_8_8_8_REV, heroLoadedBitmap.pixels);

    f32 metersToPixels = 60.0f;
    Input oldInput = {};
    OpenGLState glState = initOpenGLState();

    globalWin32State.isRunning = true;
    while (globalWin32State.isRunning)
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

        Input newInput = {};
        newInput.dt = targetMsPerFrame;
        for (int i = 0; i < Key_Count; ++i)
        {
            newInput.keyDown[i] = oldInput.keyDown[i];
        }
        win32GetInput(&newInput);
        oldInput = newInput;

        HDC deviceContext = GetDC(globalWin32State.window);
        RECT clientRect;
        GetClientRect(globalWin32State.window, &clientRect);
        int clientWidth = clientRect.right - clientRect.left;
        int clientHeight = clientRect.bottom - clientRect.top;
        int fringe = 0;
        int viewportWidth = clientWidth - (2 * fringe);
        int viewportHeight = clientHeight - (2 * fringe);
        glViewport(fringe, fringe, viewportWidth, viewportHeight);

        RenderCommands renderCommands = {};
        renderCommands.metersToPixels = metersToPixels;
        renderCommands.maxBufferSize = MEGABYTES(2);
        // TODO(chogan): Keep this around as a temporary Arena
        renderCommands.bufferBase = win32AllocateMemory(renderCommands.maxBufferSize);
        renderCommands.windowWidth = viewportWidth;
        renderCommands.windowHeight = viewportHeight;
        renderCommands.renderer = &glState;
        updateAndRender(&memory, &newInput, &renderCommands);

        updateOpenGLViewMatrix(&renderCommands);

        win32UpdateWindow(&globalBackBuffer, deviceContext, clientWidth, clientHeight, &renderCommands);
        ReleaseDC(globalWin32State.window, deviceContext);

        win32FreeMemory(renderCommands.bufferBase);

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
