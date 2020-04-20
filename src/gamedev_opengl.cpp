#include "gamedev_renderer.h"
#include "gamedev.h"
#include "gamedev_entity.h"

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

struct OpenGLState
{
    GLuint quadBufferHandle;
    GLint colorUniformLocation;
    GLint viewUniformLocation;
};

void drawOpenGLBitmap(RenderEntryTexture *entry, GLuint quadBuffer, GLint ucolorLocation)
{
    f32 spriteWidth = entry->spriteWidth;
    f32 spriteHeight = entry->spriteHeight;
    f32 sheetWidth = entry->sheetWidth;
    f32 sheetHeight = entry->sheetHeight;

    Rect2 spriteRect = {};
    spriteRect.minP = entry->position;
    spriteRect.maxP = spriteRect.minP + entry->size;

    int frame = entry->currentFrame;

    f32 texMinX = (spriteWidth * frame) / sheetWidth;
    f32 texMaxX = (spriteWidth * (frame + 1)) / sheetWidth;

    int sheetRow;
    switch (entry->direction)
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

void drawOpenGLFilledRect(OpenGLState *glState, Rect2 rect, Vec4u8 color)
{
    f32 z = -9.0f;
    f32 w = 1.0f;
    f32 quad[] =
    {
        rect.minP.x, rect.minP.y, z, w,
        rect.maxP.x, rect.minP.y, z, w,
        rect.maxP.x, rect.maxP.y, z, w,
        rect.minP.x, rect.maxP.y, z, w,
    };

    glBindBuffer(GL_ARRAY_BUFFER, glState->quadBufferHandle);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
    glUniform4f(glState->colorUniformLocation, color.r / 255.0f, color.g / 255.0f, color.b / 255.0f,
                color.a / 255.0f);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

static const char *vertexShaderSource[] =
{
    "#version 450 core                                           \n",
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

static const char *fragmentShaderSource[] =
{
    "#version 450 core                                  \n",
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
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource,
                                          ARRAY_COUNT(fragmentShaderSource));

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}


internal void updateOpenGLViewMatrix(RenderCommands *commands)
{
    OpenGLState *glState = (OpenGLState *)commands->renderer;
    Camera *camera = &commands->camera;

    Mat4 lookAtRot = {};
    lookAtRot.col1 = {camera->right.x, camera->up.x, camera->direction.x, 0};
    lookAtRot.col2 = {camera->right.y, camera->up.y, camera->direction.y, 0};
    lookAtRot.col3 = {camera->right.z, camera->up.z, camera->direction.z, 0};
    lookAtRot.col4 = {0, 0, 0, 1};

    Mat4 lookAtTrans = makeTranslationMat4(-camera->position.x, -camera->position.y,
                                           -camera->position.z);

    Mat4 view = multiplyMat4(&lookAtRot, &lookAtTrans);
    glUniformMatrix4fv(glState->viewUniformLocation, 1, GL_FALSE, view.data);
}

internal void drawOpenGLRenderGroup(RenderCommands *commands)
{
    OpenGLState *glState = (OpenGLState *)commands->renderer;

    glClearColor(1.0f, 0.0f, 1.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    for (int layerIndex = 0; layerIndex < RenderLayer_Count; ++layerIndex)
    {
        for (u32 baseAddress = 0; baseAddress < commands->bufferSize;)
        {
            RenderEntryHeader *header = (RenderEntryHeader*)(commands->bufferBase + baseAddress);
            baseAddress += sizeof(*header);
            
            void *data = (u8*)header + sizeof(*header);
            switch (header->type)
            {
                case RenderEntryType_RenderEntryRect:
                {
                    RenderEntryRect *entry = (RenderEntryRect*)data;
                    if (entry->layer == layerIndex)
                    {
                        // rendererAPI.renderRect(renderer, entry->dest, entry->color);
                        // TODO(chogan): drawOpenGLRect
                    }
                    baseAddress += sizeof(*entry);
                } break;
                case RenderEntryType_RenderEntryFilledRect:
                {
                    RenderEntryFilledRect *entry = (RenderEntryFilledRect*)data;
                    if (entry->layer == layerIndex)
                    {
                        // rendererAPI.renderFilledRect(renderer, entry->dest, entry->color);
                        drawOpenGLFilledRect(glState, entry->dest, entry->color); 
                    }
                    baseAddress += sizeof(*entry);
                } break;
                
                case RenderEntryType_RenderEntrySprite:
                {
                    RenderEntrySprite *entry = (RenderEntrySprite*)data;
                    if (entry->layer == layerIndex)
                    {
                        // TODO(chogan):
                        // rendererAPI.renderSprite(renderer, entry->sheet, entry->source, entry->dest);
                    }
                    baseAddress += sizeof(*entry);
                } break;
                case RenderEntryType_RenderEntryLoadedBitmap:
                {
                    RenderEntryLoadedBitmap *entry = (RenderEntryLoadedBitmap *)data;
                    if (entry->layer == layerIndex)
                    {
                        // rendererAPI.renderBitmap(renderer, entry->bitmap, entry->source, entry->dest);
                    }
                    baseAddress += sizeof(*entry);
                } break;
                case RenderEntryType_RenderEntryTexture:
                {
                    RenderEntryTexture *entry = (RenderEntryTexture *)data;
                    drawOpenGLBitmap(entry, glState->quadBufferHandle, glState->colorUniformLocation);
                    baseAddress += sizeof(*entry);
                } break;

                default:
                {
                    InvalidCodePath;
                }
            }
        }
    }
}
OpenGLState initOpenGLState()
{
    OpenGLState result = {};
    GLuint program = compileShaders();
    glUseProgram(program);

    u32 quadBuffer;
    glGenBuffers(1, &quadBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, quadBuffer);
    result.quadBufferHandle = quadBuffer;

    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid *)(16 * sizeof(GLfloat)));

    const GLuint quadIndices[] = {0, 1, 2, 0, 2, 3};

    GLuint ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, GL_STATIC_DRAW);

    GLint ucolorLocation = glGetUniformLocation(program, "ucolor");
    GLint modelLocation = glGetUniformLocation(program, "model");
    GLint viewLocation = glGetUniformLocation(program, "view");
    GLint projectionLocation = glGetUniformLocation(program, "projection");

    result.colorUniformLocation = ucolorLocation;
    result.viewUniformLocation = viewLocation;

    Mat4 model = identityMat4();
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, model.data);

    int windowWidth = 1920 / 2;
    int windowHeight = 1080 / 2;
    f32 pixelsToMeters = 1.0f / 60.0f;
    f32 viewportWidthInMeters = windowWidth * pixelsToMeters;
    f32 viewportHeightInMeters = windowHeight * pixelsToMeters;
    f32 aspect = viewportWidthInMeters / viewportHeightInMeters;
    Mat4 projection = makePerspectiveMat4(45, aspect, 0.1f, 100.0f);
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, projection.data);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    return result;
}

