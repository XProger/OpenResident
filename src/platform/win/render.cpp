#include <math.h>

#include "render.h"

#ifdef __WIN32__
    #include <windows.h>
    #include <gl/GL.h>
    #include <gl/glu.h>
    #include <gl/glext.h>
    #include <gl/wglext.h>
#elif __LINUX__
    #include <GL/gl.h>
    #include <GL/glext.h>
    #include <GL/glx.h>
#endif

#define MAX_UI_PRIMS    128
#define MAX_CLUTS       4

#define PI              3.14159265358979323846f
#define DEG2RAD         (PI / 180.0f)
#define RAD2DEG         (180.0f / PI)

#define PROJ_Z_NEAR     256.0f
#define PROJ_Z_FAR      (64 * 1024.0f)
#define PROJ_Z_CLIP     ((PROJ_Z_NEAR + PROJ_Z_FAR) / (PROJ_Z_NEAR - PROJ_Z_FAR))
#define PROJ_W_CLIP     (2.0f * PROJ_Z_FAR * PROJ_Z_NEAR / (PROJ_Z_NEAR - PROJ_Z_FAR))

#ifdef __WIN32__
    extern HWND hWnd;
    HDC hDC;
    HGLRC hRC;
#elif __LINUX__
    extern Display* dpy;
    extern Window wnd;
#endif

int32 gWidth, gHeight;

// Textures
PFNGLGENERATEMIPMAPPROC             glGenerateMipmap;
// Shader
PFNGLCREATEPROGRAMPROC              glCreateProgram;
PFNGLDELETEPROGRAMPROC              glDeleteProgram;
PFNGLLINKPROGRAMPROC                glLinkProgram;
PFNGLUSEPROGRAMPROC                 glUseProgram;
PFNGLGETPROGRAMINFOLOGPROC          glGetProgramInfoLog;
PFNGLCREATESHADERPROC               glCreateShader;
PFNGLDELETESHADERPROC               glDeleteShader;
PFNGLSHADERSOURCEPROC               glShaderSource;
PFNGLATTACHSHADERPROC               glAttachShader;
PFNGLCOMPILESHADERPROC              glCompileShader;
PFNGLGETSHADERINFOLOGPROC           glGetShaderInfoLog;
PFNGLGETUNIFORMLOCATIONPROC         glGetUniformLocation;
PFNGLUNIFORM1IVPROC                 glUniform1iv;
PFNGLUNIFORM1FVPROC                 glUniform1fv;
PFNGLUNIFORM2FVPROC                 glUniform2fv;
PFNGLUNIFORM3FVPROC                 glUniform3fv;
PFNGLUNIFORM4FVPROC                 glUniform4fv;
PFNGLUNIFORMMATRIX4FVPROC           glUniformMatrix4fv;
PFNGLBINDATTRIBLOCATIONPROC         glBindAttribLocation;
PFNGLENABLEVERTEXATTRIBARRAYPROC    glEnableVertexAttribArray;
PFNGLDISABLEVERTEXATTRIBARRAYPROC   glDisableVertexAttribArray;
PFNGLVERTEXATTRIBPOINTERPROC        glVertexAttribPointer;
PFNGLGETPROGRAMIVPROC               glGetProgramiv;
// Model
PFNGLGENBUFFERSARBPROC              glGenBuffers;
PFNGLDELETEBUFFERSARBPROC           glDeleteBuffers;
PFNGLBINDBUFFERARBPROC              glBindBuffer;
PFNGLBUFFERDATAARBPROC              glBufferData;
PFNGLBUFFERSUBDATAARBPROC           glBufferSubData;
// Vertex Arrays
PFNGLGENVERTEXARRAYSPROC            glGenVertexArrays;
PFNGLDELETEVERTEXARRAYSPROC         glDeleteVertexArrays;
PFNGLBINDVERTEXARRAYPROC            glBindVertexArray;

// vectors =============================================
struct vec3
{
    float x, y, z;
};

struct vec4
{
    float x, y, z, w;
};

float dot(const vec3& a, const vec3& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

vec3 cross(const vec3& a, const vec3& b)
{
    vec3 r;
    r.x = a.y * b.z - a.z * b.y;
    r.y = a.z * b.x - a.x * b.z;
    r.z = a.x * b.y - a.y * b.x;
    return r;
}

void normalize(vec3& v)
{
    float dist = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
    if (dist > 0.00001)
    {
        dist = 1.0f / dist;
        v.x *= dist;
        v.y *= dist;
        v.z *= dist;
    }
}

// matrix ==============================================
struct mat4
{
    float e00, e10, e20, e30,
          e01, e11, e21, e31,
          e02, e12, e22, e32,
          e03, e13, e23, e33;

    void identity() {
        e10 = e20 = e30 = e01 = e21 = e31 = e02 = e12 = e32 = e03 = e13 = e23 = 0.0f;
        e00 = e11 = e22 = e33 = 1.0f;
    }

    void ortho(float l, float r, float b, float t, float znear, float zfar)
    {
        identity();
        e00 = 2.0f / (r - l);
        e11 = 2.0f / (t - b);
        e22 = 2.0f / (znear - zfar);
        e33 = 1.0f;
        e03 = (l + r) / (l - r);
        e13 = (t + b) / (b - t);
        e23 = (znear + zfar) / (znear - zfar);
    }

    void perspective(float tan, float aspect, float znear, float zfar)
    {
        // tan = tanf(fov * 0.5f)
        float y = tan * znear;
        float x = y;

        y /= aspect;

        identity();
        e00 = znear / x;
        e11 = znear / y;
        e22 = (znear + zfar) / (znear - zfar);
        e33 = 0.0f;
        e02 = 0.0f;
        e12 = 0.0f;
        e32 = -1.0f;
        e23 = 2.0f * zfar * znear / (znear - zfar);
    }

    mat4 operator * (const mat4 &m) const {
        mat4 r;
        r.e00 = e00 * m.e00 + e01 * m.e10 + e02 * m.e20 + e03 * m.e30;
        r.e10 = e10 * m.e00 + e11 * m.e10 + e12 * m.e20 + e13 * m.e30;
        r.e20 = e20 * m.e00 + e21 * m.e10 + e22 * m.e20 + e23 * m.e30;
        r.e30 = e30 * m.e00 + e31 * m.e10 + e32 * m.e20 + e33 * m.e30;
        r.e01 = e00 * m.e01 + e01 * m.e11 + e02 * m.e21 + e03 * m.e31;
        r.e11 = e10 * m.e01 + e11 * m.e11 + e12 * m.e21 + e13 * m.e31;
        r.e21 = e20 * m.e01 + e21 * m.e11 + e22 * m.e21 + e23 * m.e31;
        r.e31 = e30 * m.e01 + e31 * m.e11 + e32 * m.e21 + e33 * m.e31;
        r.e02 = e00 * m.e02 + e01 * m.e12 + e02 * m.e22 + e03 * m.e32;
        r.e12 = e10 * m.e02 + e11 * m.e12 + e12 * m.e22 + e13 * m.e32;
        r.e22 = e20 * m.e02 + e21 * m.e12 + e22 * m.e22 + e23 * m.e32;
        r.e32 = e30 * m.e02 + e31 * m.e12 + e32 * m.e22 + e33 * m.e32;
        r.e03 = e00 * m.e03 + e01 * m.e13 + e02 * m.e23 + e03 * m.e33;
        r.e13 = e10 * m.e03 + e11 * m.e13 + e12 * m.e23 + e13 * m.e33;
        r.e23 = e20 * m.e03 + e21 * m.e13 + e22 * m.e23 + e23 * m.e33;
        r.e33 = e30 * m.e03 + e31 * m.e13 + e32 * m.e23 + e33 * m.e33;
        return r;
    }

    void translate(float x, float y, float z) {
        mat4 m;
        m.identity();
        m.e03 = x;
        m.e13 = y;
        m.e23 = z;
        *this = *this * m;
    };

    void scale(float x, float y, float z) {
        mat4 m;
        m.identity();
        m.e00 = x;
        m.e11 = y;
        m.e22 = z;
        *this = *this * m;
    }

    void rotateX(float angle) {
        mat4 m;
        m.identity();
        float s = sinf(angle);
        float c = cosf(angle);
        m.e11 = c;  m.e21 = s;
        m.e12 = -s; m.e22 = c;
        *this = *this * m;
    }

    void rotateY(float angle) {
        mat4 m;
        m.identity();
        float s = sinf(angle);
        float c = cosf(angle);
        m.e00 = c;  m.e20 = -s;
        m.e02 = s;  m.e22 = c;
        *this = *this * m;
    }

    void rotateZ(float angle) {
        mat4 m;
        m.identity();
        float s = sinf(angle);
        float c = cosf(angle);
        m.e00 = c;  m.e01 = -s;
        m.e10 = s;  m.e11 = c;
        *this = *this * m;
    }
};

float dbg_clear;

mat4 gViewProjMatrix;
mat4 gModelMatrix;

mat4 gMatrixStack[8];
mat4* gMatrixPtr = gMatrixStack;

// lighting
vec4 gAmbient;
vec4 gLightColor[MAX_LIGHTS];
vec4 gLightPos[MAX_LIGHTS];

void matrixPush()
{
    *gMatrixPtr++ = gModelMatrix;
}

void matrixPop()
{
    gModelMatrix = *--gMatrixPtr;
}


#ifdef _DEBUG
void dumpBitmap(const char* fileName, int32 width, int32 height, uint8* data32)
{
    struct BITMAPFILEHEADER {
        uint32  bfSize;
        uint16  bfReserved1;
        uint16  bfReserved2;
        uint32  bfOffBits;
    } fhdr;

    struct BITMAPINFOHEADER{
        uint32 biSize;
        uint32 biWidth;
        uint32 biHeight;
        uint16 biPlanes;
        uint16 biBitCount;
        uint32 biCompression;
        uint32 biSizeImage;
        uint32 biXPelsPerMeter;
        uint32 biYPelsPerMeter;
        uint32 biClrUsed;
        uint32 biClrImportant;
    } ihdr;

    memset(&fhdr, 0, sizeof(fhdr));
    memset(&ihdr, 0, sizeof(ihdr));

    ihdr.biSize      = sizeof(ihdr);
    ihdr.biWidth     = width;
    ihdr.biHeight    = height;
    ihdr.biPlanes    = 1;
    ihdr.biBitCount  = 32;
    ihdr.biSizeImage = width * height * 4;

    fhdr.bfOffBits   = 2 + sizeof(fhdr) + ihdr.biSize;
    fhdr.bfSize      = fhdr.bfOffBits + ihdr.biSizeImage;

    FILE* f = fopen(fileName, "wb");
    ASSERT(f);

    uint16 tag = 'B' + ('M' << 8);
    fwrite(&tag, sizeof(tag), 1, f);
    fwrite(&fhdr, sizeof(fhdr), 1, f);
    fwrite(&ihdr, sizeof(ihdr), 1, f);

    data32 += width * height * 4;
    for (int32 i = 0; i < height; i++)
    {
        data32 -= width * 4;
        fwrite(data32, 1, width * 4, f);
    }

    fclose(f);
}
#endif


// shader ==============================================
enum VertexAttrib
{
    aCoord,
    aNormal,
    aTexCoord,
    aColor,
    aMAX
};

enum UniformType
{
    uViewProjMatrix,
    uModelMatrix,
    uTexParam,
    uAmbient,
    uLightColor,
    uLightPos,
    uMAX
};

struct Shader
{
    GLuint id;
    GLint uid[uMAX];

    void setMatrix(UniformType type, const mat4* m)
    {
        if (uid[type] != -1)
        {
            glUniformMatrix4fv(uid[type], 1, GL_FALSE, (GLfloat*)m);
        }
    }

    void setVector(UniformType type, const vec4* v, int32 count)
    {
        if (uid[type] != -1)
        {
            glUniform4fv(uid[type], count, (GLfloat*)v);
        }
    }

    void bind()
    {
        glUseProgram(id);
    }
};

const char* GLSL_HEADER_VERT_GL3 = 
    "#version 140\n"
    "#define VERTEX\n"
    "#define varying out\n"
    "#define attribute in\n"
    "#define texture2D texture\n";

const char* GLSL_HEADER_FRAG_GL3 = 
    "#version 140\n"
    "#define FRAGMENT\n"
    "#define varying in\n"
    "#define texture2D texture\n"
    "out vec4 fragColor;\n";

Shader shaderModel;

const char* sh_model =
    "varying vec2 vTexCoord;\n"
    "varying vec4 vColor;\n"

    "#ifdef VERTEX\n"
        "#define MAX_LIGHTS 3\n"
        "uniform mat4 uViewProjMatrix;\n"
        "uniform mat4 uModelMatrix;\n"
        "uniform vec4 uTexParam;\n"
        "uniform vec4 uAmbient;\n"
        "uniform vec4 uLightColor[MAX_LIGHTS];\n"
        "uniform vec4 uLightPos[MAX_LIGHTS];\n"

        "attribute vec3 aCoord;\n"
        "attribute vec3 aNormal;\n"
        "attribute vec4 aTexCoord;\n"

        "void main() {\n"
            "vTexCoord.x = aTexCoord.x * uTexParam.x + aTexCoord.z * uTexParam.z;\n"
            "vTexCoord.y = aTexCoord.y * uTexParam.y;\n"

            "vec4 c = uModelMatrix * vec4(aCoord, 1.0);\n"
            "vec4 n = uModelMatrix * vec4(aNormal.xyz, 0.0);\n"
            "n = normalize(n);\n"

            "vec3 light = uAmbient.xyz;\n"
            "for (int i = 0; i < MAX_LIGHTS; i++) {\n"
                "vec3 lightVec = (uLightPos[i].xyz - c.xyz) * uLightPos[i].w;\n"
                "float att = 1.0f - dot(lightVec, lightVec);\n"
                "float NdotL = dot(n.xyz, normalize(lightVec));\n"
                "light += max(0.0, NdotL) * max(0.0, att) * uLightColor[i].xyz;\n"
            "}\n"

            "vColor = vec4(light, 1.0);\n"
            "gl_Position = uViewProjMatrix * c;\n"
        "}\n"

    "#else\n"

        "uniform sampler2D sDiffuse;\n"

        "void main() {\n"
            "fragColor = texture2D(sDiffuse, vTexCoord) * vColor;\n"
            "if (fragColor.a < 0.5) discard;\n"
        "}\n"

    "#endif\n";

Shader shaderBackground;

const char* sh_background =
    "varying vec2 vTexCoord;\n"

    "#ifdef VERTEX\n"
        "uniform mat4 uViewProjMatrix;\n"
        "uniform vec4 uTexParam;\n"

        "attribute vec4 aCoord;\n"
        "attribute vec4 aTexCoord;\n"

        "void main() {\n"
            "vTexCoord.xy = aTexCoord.xy * uTexParam.xy;\n"
            "gl_Position = uViewProjMatrix * aCoord;\n"
        "}\n"

    "#else\n"

        "uniform sampler2D sDiffuse;\n"

        "void main() {\n"
            "fragColor = texture2D(sDiffuse, vTexCoord);\n"
        "}\n"

    "#endif\n";

Shader shaderBackgroundMask;

const char* sh_background_mask =
    "varying vec2 vTexCoord;\n"

    "#ifdef VERTEX\n"
        "uniform mat4 uViewProjMatrix;\n"
        "uniform vec4 uTexParam;\n"

        "attribute vec4 aCoord;\n"
        "attribute vec4 aTexCoord;\n"

        "void main() {\n"
            "vTexCoord.xy = aTexCoord.xy * uTexParam.xy;\n"
            "float z = -32.0 * aCoord.z;\n"
            "vec4 p = uViewProjMatrix * aCoord;\n"
            "gl_Position = vec4(p.xy, -(z * uTexParam.z + uTexParam.w) / z, 1.0);\n"
        "}\n"

    "#else\n"

        "uniform sampler2D sDiffuse;\n"

        "void main() {\n"
            "fragColor = texture2D(sDiffuse, vTexCoord);\n"
            "if (fragColor.a < 0.5) discard;\n"
        "}\n"

    "#endif\n";


#ifdef _DEBUG

#define MAX_DEBUG_VERTICES  1024
#define MAX_DEBUG_INDICES   MAX_DEBUG_VERTICES * 3

struct DebugVertex
{
    vec3s coord;
    uint32 color;
};

Index gDebugIndices[MAX_DEBUG_INDICES];
DebugVertex gDebugVertices[MAX_DEBUG_VERTICES];
int32 gDebugIndicesCount;
int32 gDebugVerticesCount;
GLenum gDebugTopology;
GLuint gDebugVAO;
GLuint gDebugVBO[2];

Shader shaderDebug;

const char* sh_debug =
    "varying vec4 vColor;\n"

    "#ifdef VERTEX\n"
        "uniform mat4 uViewProjMatrix;\n"

        "attribute vec3 aCoord;\n"
        "attribute vec4 aColor;\n"

        "void main() {\n"
            "vColor = aColor;\n"
            "gl_Position = uViewProjMatrix * vec4(aCoord.xyz, 1.0);\n"
        "}\n"

    "#else\n"

        "void main() {\n"
            "fragColor = vColor;\n"
        "}\n"

    "#endif\n";
#endif

GLuint uiVAO;
GLuint uiVBO[2];

void compileShader(Shader* shader, const char* text)
{
    int32 i;
    GLchar info[1024];
    GLuint obj;
    const GLenum type[2] = { GL_VERTEX_SHADER, GL_FRAGMENT_SHADER };

    const GLchar *code[2][3] = {
        { GLSL_HEADER_VERT_GL3, "#line 0\n", text },
        { GLSL_HEADER_FRAG_GL3, "#line 0\n", text }
    };

    shader->id = glCreateProgram();

    for (i = 0; i < 2; i++)
    {
        obj = glCreateShader(type[i]);
        glShaderSource(obj, 3, code[i], NULL);
        glCompileShader(obj);

        glGetShaderInfoLog(obj, sizeof(info), NULL, info);
        if (info[0] && strlen(info) > 8)
        {
            LOG(info);
            ASSERT(0);
        }

        glAttachShader(shader->id, obj);
        glDeleteShader(obj);
    }

    glBindAttribLocation(shader->id, aCoord, "aCoord");
    glBindAttribLocation(shader->id, aNormal, "aNormal");
    glBindAttribLocation(shader->id, aTexCoord, "aTexCoord");
    glBindAttribLocation(shader->id, aColor, "aColor");

    glLinkProgram(shader->id);

    glGetProgramInfoLog(shader->id, sizeof(info), NULL, info);
    if (info[0] && strlen(info) > 8)
    {
        LOG(info);
        ASSERT(0);
    }

    i = 0;
    shader->bind();
    glUniform1iv(glGetUniformLocation(shader->id, "sDiffuse"), 1, &i);

    shader->uid[uViewProjMatrix] = glGetUniformLocation(shader->id, "uViewProjMatrix");
    shader->uid[uModelMatrix] = glGetUniformLocation(shader->id, "uModelMatrix");
    shader->uid[uTexParam] = glGetUniformLocation(shader->id, "uTexParam");
    shader->uid[uAmbient] = glGetUniformLocation(shader->id, "uAmbient");
    shader->uid[uLightColor] = glGetUniformLocation(shader->id, "uLightColor");
    shader->uid[uLightPos] = glGetUniformLocation(shader->id, "uLightPos");
}


// texture ==============================================
void Texture::load(Stream* stream)
{
    enum TextureFormat
    {
        TEX_FMT_4,
        TEX_FMT_8,
        TEX_FMT_16,
        TEX_FMT_24,
        TEX_FMT_32,
        TEX_FMT_MAX
    };

    uint32 version = stream->u32();
    ASSERT(version == 0x10);

    TextureFormat fmt = TextureFormat(stream->u32() & 7);

    uint16 cluts[256 * MAX_CLUTS];

    stream->skip(8); // offset, x, y
    uint32 colors = stream->u16();
    count = stream->u16();

    ASSERT(count <= MAX_CLUTS);
    stream->read(cluts, count * colors * sizeof(uint16)); 

    stream->skip(4); // texture size
    x = stream->s16();
    y = stream->s16();
    int32 w = stream->s16();
    int32 h = stream->s16();

    uint8* data = new uint8[w * h * sizeof(uint16)];
    stream->read(data, w * h * sizeof(uint16));

    // convert width from shorts to pixels
    if (fmt == TEX_FMT_4)
    {
        w *= 4;
    }
    else if (fmt == TEX_FMT_8)
    {
        w *= 2;
    }

    uint8* data32 = new uint8[w * h * 4];

    int32 pageWidth = w / count;

    uint8* src = data;
    uint8* dst = data32;

    switch (fmt)
    {
        case TEX_FMT_4:
        {
            for (int32 j = 0; j < w * h; j++)
            {
                uint16* clut = cluts + ((j / pageWidth) % count) * colors;
                uint8 index = *src++;
                uint16 value1 = clut[index & 15];
                uint16 value2 = clut[(index >> 4) & 15];

                *dst++ = (value1 & 31) << 3;
                *dst++ = ((value1 >> 5) & 31) << 3;
                *dst++ = ((value1 >> 10) & 31) << 3;
                *dst++ = 255;

                *dst++ = (value2 & 31) << 3;
                *dst++ = ((value2 >> 5) & 31) << 3;
                *dst++ = ((value2 >> 10) & 31) << 3;
                *dst++ = 255;

                j -= 2;
            }
            break;
        }

        case TEX_FMT_8:
        {
            for (int32 j = 0; j < w * h; j++)
            {
                uint16* clut = cluts + ((j / pageWidth) % count) * colors;
                uint8 index = *src++;
                uint16 value = clut[index];

                *dst++ = (value & 31) << 3;
                *dst++ = ((value >> 5) & 31) << 3;
                *dst++ = ((value >> 10) & 31) << 3;
                *dst++ = index ? 255 : 0;
            }
            break;
        }

        case TEX_FMT_16:
        {
            for (int32 j = 0; j < w * h; j++)
            {
                uint16 value = *(uint16*)src;
                src += 2;

                *dst++ = (value & 31) << 3;
                *dst++ = ((value >> 5) & 31) << 3;
                *dst++ = ((value >> 10) & 31) << 3;
                *dst++ = 255;
            }
            break;
        }

        case TEX_FMT_24:
        {
            for (int32 j = 0; j < w * h; j++)
            {
                *dst++ = *src++;
                *dst++ = *src++;
                *dst++ = *src++;
                *dst++ = 255;
            }
            break;
        }

        default: ASSERT(0);
    }

    init(data32, w, h);

#if 0
#ifdef _DEBUG
    char buf[32];
    sprintf(buf, "tex_%d.bmp", *(GLuint*)res);
    dumpBitmap(buf, width, height, data32);
#endif
#endif

    delete[] data32;
    delete[] data;
}

void Texture::init(uint8* data32, int32 w, int32 h)
{
    if (res && (w > width || h > height))
    {
        free();
    }

    if (!res)
    {
        width = w;
        height = h;

        res = new GLuint();

        glGenTextures(1, (GLuint*)res);
        glBindTexture(GL_TEXTURE_2D, *(GLuint*)res);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data32);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, *(GLuint*)res);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, data32);
    }
}

void Texture::free()
{
    glDeleteTextures(1, (GLuint*)res);
    delete (GLuint*)res;
    res = NULL;
}

void Texture::bind() const
{
    glBindTexture(GL_TEXTURE_2D, *(GLuint*)res);
}

// animation ==============================================
void Animation::load(Stream* stream)
{
    clips[0].start = 0;
    clips[0].count = stream->u16();

    uint32 basePos = stream->u16();

    clipsCount = basePos >> 2;

    ASSERT(clipsCount <= MAX_ANIMATION_CLIPS);

    totalFrames = clips[0].count;
    for (uint32 i = 1; i < clipsCount; i++)
    {
        clips[i].count = stream->u16();
        clips[i].start = (stream->u16() - basePos) >> 2;
        totalFrames += clips[i].count;
    }
    ASSERT(totalFrames <= MAX_ANIMATION_FRAMES);

    for (uint32 i = 0; i < totalFrames; i++)
    {
        framesInfo[i] = stream->u32();
    }
}

void Animation::free()
{
    //
}

int32 Animation::getFrameIndex(int32 curIndex) const
{
    return ANIM_FRAME_INDEX(framesInfo[curIndex]);
}

int32 Animation::getFrameFlags(int32 curIndex) const
{
    return ANIM_FRAME_FLAGS(framesInfo[curIndex]);
}


// skeleton ==============================================
void Skeleton::load(Stream* stream, const Animation* anim)
{
    int32 basePos = stream->getPos();

    uint32 offsetLinks = stream->u16();
    uint32 offsetFrames = stream->u16();

    if (offsetLinks == 0 && offsetFrames == 0)
        return;

    count = stream->u16();
    uint32 size = stream->u16();

    ASSERT(size <= sizeof(Frame));

    if (offsetLinks > 0)
    {
        ASSERT(count <= MAX_RANGES);

        for (uint32 i = 0; i < count; i++)
        {
            offsets[i].x = stream->s16();
            offsets[i].y = stream->s16();
            offsets[i].z = stream->s16();
        }

        stream->setPos(offsetLinks + basePos);
        for (uint32 i = 0; i < count; i++)
        {
            links[i].count = stream->u16();
            links[i].offset = stream->u16();
            links[i].parent = -1;
            ASSERT(links[i].count <= MAX_CHILDS);
        }

        for (uint32 i = 0; i < count; i++)
        {
            stream->setPos(offsetLinks + basePos + links[i].offset);
            for (uint32 j = 0; j < links[i].count; j++)
            {
                links[i].childs[j] = stream->u8();
                ASSERT(links[i].childs[j] < count);
                links[links[i].childs[j]].parent = i;
            }
        }
    }

// animation frames
    dataFramesCount = -1;

    for (uint32 i = 0; i < anim->totalFrames; i++)
    {
        int32 frameIndex = anim->getFrameIndex(i);

        if (frameIndex > dataFramesCount)
        {
            dataFramesCount = frameIndex;
        }
    }

    dataFramesCount += 1;

    ASSERT(dataFramesCount > 0);
    ASSERT(offsetFrames > 0);

    stream->setPos(offsetFrames + basePos);
    for (int32 i = 0; i < dataFramesCount; i++)
    {
        Frame* f = frames + i;
        f->pos.x = stream->s16();
        f->pos.y = stream->s16();
        f->pos.z = stream->s16();
        f->offset.x = stream->s16();
        f->offset.y = stream->s16();
        f->offset.z = stream->s16();
        stream->read(f->angles, size - sizeof(vec3s) * 2);
    }
}

void Skeleton::free()
{
    //
}

void Skeleton::getAngles(int32 frameIndex, int32 jointIndex, int32& x, int32& y, int32& z) const
{
    const Frame* f = frames + frameIndex;
    int32 idx = (jointIndex >> 1) * 9;

    if (jointIndex & 1)
    {
        idx += 4;
    }

    uint8 a = f->angles[idx + 0];
    uint8 b = f->angles[idx + 1];
    uint8 c = f->angles[idx + 2];
    uint8 d = f->angles[idx + 3];
    uint8 e = f->angles[idx + 4];

    if (jointIndex & 1)
    {
        x = ((a & 0xF0) >> 4) | (b << 4);
        y = c | ((d & 0x0F) << 8);
        z = ((d & 0xF0) >> 4) | (e << 4);
    }
    else
    {
        x = a | ((b & 0x0F) << 8);
        y = ((b & 0xF0) >> 4) | (c << 4);
        z = d | ((e & 0x0F) << 8);
    }
}


// model ==============================================
struct MeshData
{
    GLuint VAO;
    GLuint VBO[2];
};

struct Coord
{
    int16 x, y, z, pad;
};

struct Prim
{
    uint16 cIndex[4];
    uint16 nIndex[4];
};

struct Tile
{
    uint16 clut;
    uint16 page;
    uint8 u[4];
    uint8 v[4];
};

struct Vertex
{
    vec4s coord;
    vec4s normal;
    uint8 u, v, page, zero;
};

struct VertexUI
{
    vec4s coord;
    vec2s uv;
    uint32 color;
};

Index addVertex(int32 idx, const Prim* prim, const Coord* coords, const Coord* normals, const Tile* tile, Vertex* vertices, int32& vCount)
{
    Vertex* v = vertices + vCount;

    v->coord.x = coords[prim->cIndex[idx]].x;
    v->coord.y = coords[prim->cIndex[idx]].y;
    v->coord.z = coords[prim->cIndex[idx]].z;
    v->coord.w = 0;
    v->normal.x = normals[prim->nIndex[idx]].x;
    v->normal.y = normals[prim->nIndex[idx]].y;
    v->normal.z = normals[prim->nIndex[idx]].z;
    v->normal.w = 0;
    v->u = tile->u[idx];
    v->v = tile->v[idx];
    v->page = tile->page & 3;
    v->zero = 0;

    // search for an existing vertex
    for (int32 i = 0; i < vCount; i++)
    {
        if (memcmp(vertices + i, v, sizeof(Vertex)) == 0)
        {
            return i; // found
        }
    }

    // not found, return new index
    return vCount++;
}

void Model::load(Stream* stream)
{
    struct MeshHeader
    {
        uint32 coordOffset;
        uint32 coordCount;
        uint32 normOffset;
        uint32 normCount;
        uint32 primOffset;
        uint32 primCount;
        uint32 tileOffset;
    };

    uint32 offset = stream->u32();
    uint32 numOffsets = stream->u32();
    ASSERT((numOffsets == 4) || (numOffsets == 8));

    stream->setPos(offset);
    uint32 offsetAnimation0 = 0;
    uint32 offsetSkeleton0 = 0;
    uint32 offsetAnimation1 = 0;
    uint32 offsetSkeleton1 = 0;
    uint32 offsetAnimation2 = 0;
    uint32 offsetSkeleton2 = 0;
    uint32 offsetMesh = 0;
    uint32 offsetTexture = 0;

    if (numOffsets == 4)
    {
        offsetAnimation0 = stream->u32();
        offsetSkeleton0 = stream->u32();
        offsetMesh = stream->u32();
        offsetTexture = stream->u32();
    }
    else if (numOffsets == 8)
    {
        stream->skip(4);
        offsetAnimation0 = stream->u32();
        offsetSkeleton0 = stream->u32();
        offsetAnimation1 = stream->u32();
        offsetSkeleton1 = stream->u32();
        offsetAnimation2 = stream->u32();
        offsetSkeleton2 = stream->u32();
        offsetMesh = stream->u32();
    }
    else
    {
        ASSERT(0);
    }

    stream->setPos(offsetAnimation0);
    animation.load(stream);

    stream->setPos(offsetSkeleton0);
    skeleton.load(stream, &animation);

    if (offsetTexture)
    {
        stream->setPos(offsetTexture);
        texture.load(stream);
    }

    stream->setPos(offsetMesh);

    stream->skip(8); // length, unknown
    uint32 count = stream->u32();

    MeshHeader* headers = new MeshHeader[count];

    uint32 primCount = 0;

    // read prim headers
    int32 basePos = stream->getPos();
    for (uint32 i = 0; i < count; i++)
    {
        MeshHeader* h = headers + i;

        h->coordOffset = stream->u32() + basePos;
        h->coordCount = stream->u32();
        h->normOffset = stream->u32() + basePos;
        h->normCount = stream->u32();
        h->primOffset = stream->u32() + basePos;
        h->primCount = stream->u32();
        h->tileOffset = stream->u32() + basePos;

        //ASSERT(h->coordCount == h->normCount);
        primCount += h->primCount;
    }

    Coord* coords = new Coord[primCount * 4];
    Coord* normals = new Coord[primCount * 4];
    Prim* prims = new Prim[primCount];
    Tile* tiles = new Tile[primCount];

    Coord* coordPtr = coords;
    Coord* normPtr = normals;
    Prim* primPtr = prims;
    Tile* tilePtr = tiles;

    for (uint32 i = 0; i < count; i++)
    {
        MeshHeader* h = headers + i;

        int32 coordOffset = coordPtr - coords;
        int32 normOffset = normPtr - normals;

        bool isQuad = (i & 1);

        // coords
        stream->setPos(h->coordOffset);
        for (uint32 j = 0; j < h->coordCount; j++, coordPtr++)
        {
            coordPtr->x = stream->s16();
            coordPtr->y = stream->s16();
            coordPtr->z = stream->s16();
            stream->skip(2); // padding
        }
            
        // normals
        stream->setPos(h->normOffset);
        for (uint32 j = 0; j < h->normCount; j++, normPtr++)
        {
            normPtr->x = stream->s16();
            normPtr->y = stream->s16();
            normPtr->z = stream->s16();
            normPtr->pad = 0;
            stream->skip(2); // padding
        }

        // primitives
        stream->setPos(h->primOffset);
        for (uint32 j = 0; j < h->primCount; j++, primPtr++)
        {
            primPtr->nIndex[0] = stream->u16() + normOffset;
            primPtr->cIndex[0] = stream->u16() + coordOffset;
                
            primPtr->nIndex[1] = stream->u16() + normOffset;
            primPtr->cIndex[1] = stream->u16() + coordOffset;
                
            primPtr->nIndex[2] = stream->u16() + normOffset;
            primPtr->cIndex[2] = stream->u16() + coordOffset;

            if (isQuad)
            {
                primPtr->nIndex[3] = stream->u16() + normOffset;
                primPtr->cIndex[3] = stream->u16() + coordOffset;
            }
            else
            {
                primPtr->nIndex[3] = 0xFFFF;
                primPtr->cIndex[3] = 0xFFFF;
            }
        }
            
        // tiles
        stream->setPos(h->tileOffset);
        for (uint32 j = 0; j < h->primCount; j++, tilePtr++) // tileCount == primCount
        {
            tilePtr->u[0] = stream->u8();
            tilePtr->v[0] = stream->u8();
            tilePtr->clut = stream->u16();
            tilePtr->u[1] = stream->u8();
            tilePtr->v[1] = stream->u8();
            tilePtr->page = stream->u16();
            tilePtr->u[2] = stream->u8();
            tilePtr->v[2] = stream->u8();
            stream->skip(2); // padding
            if (isQuad)
            {
                tilePtr->u[3] = stream->u8();
                tilePtr->v[3] = stream->u8();
                stream->skip(2); // padding
            }
        }
    }

    // build index & vertex buffers
    Index* indices = new Index[primCount * 6];
    Vertex* vertices = new Vertex[primCount * 4];
    int32 iCount = 0;
    int32 vCount = 0;

    for (uint32 i = 0; i < primCount; i++)
    {
        Prim* prim = prims + i;
        Tile* tile = tiles + i;

        Index i0, i1, i2, i3;

        i0 = addVertex(0, prim, coords, normals, tile, vertices, vCount);
        i1 = addVertex(1, prim, coords, normals, tile, vertices, vCount);
        i2 = addVertex(2, prim, coords, normals, tile, vertices, vCount);

        indices[iCount++] = i0;
        indices[iCount++] = i1;
        indices[iCount++] = i2;

        if (prim->cIndex[3] != 0xFFFF) // quad
        {
            i3 = addVertex(3, prim, coords, normals, tile, vertices, vCount);
            indices[iCount++] = i1;
            indices[iCount++] = i3;
            indices[iCount++] = i2;
        }
    }


    MeshData* data = new MeshData();
    res = data;

    glGenVertexArrays(1, &data->VAO);
    glGenBuffers(2, data->VBO);

    glBindVertexArray(data->VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data->VBO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, data->VBO[1]);

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, iCount * sizeof(Index), indices, GL_STATIC_DRAW);
    glBufferData(GL_ARRAY_BUFFER, vCount * sizeof(Vertex), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(aCoord);
    glEnableVertexAttribArray(aNormal);
    glEnableVertexAttribArray(aTexCoord);

    Vertex* v = NULL;
    glVertexAttribPointer(aCoord, 3, GL_SHORT, GL_FALSE, sizeof(*v), &v->coord);
    glVertexAttribPointer(aNormal, 3, GL_SHORT, GL_FALSE, sizeof(*v), &v->normal);
    glVertexAttribPointer(aTexCoord, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(*v), &v->u);

    glBindVertexArray(0);

    // init ranges of model parts
    rangesCount = count >> 1;
    ASSERT(rangesCount <= MAX_RANGES);

    MeshRange* range = ranges;
    int32 indexOffset = 0;

    for (uint32 i = 0; i < count; i += 2, range++)
    {
        range->iStart = indexOffset;
        range->iCount = (headers[i].primCount * 3) + (headers[i + 1].primCount * 6); // triangles + quads
        indexOffset += range->iCount;
    }

    delete[] vertices;
    delete[] indices;
    delete[] tiles;
    delete[] prims;
    delete[] normals;
    delete[] coords;
    delete[] headers;
}

void Model::free()
{
    texture.free();
    glDeleteVertexArrays(1, &((MeshData*)res)->VAO);
    glDeleteBuffers(2, ((MeshData*)res)->VBO);
}

void Model::render(const vec3i& pos, int32 angle, uint16 frameIndex, const Texture* texture, const Skeleton* skeleton, const Skeleton* animSkeleton)
{
    Texture* pTexture = (Texture*)texture;
    Shader* pShader = &shaderModel;

    gModelMatrix.identity();
    gModelMatrix.translate(pos.x, pos.y, pos.z);
    gModelMatrix.rotateY(-angle * PI / 32768.0f);

    vec3s framePos = animSkeleton->frames[frameIndex].pos;
    gModelMatrix.translate(framePos.x, framePos.y - FLOOR_HEIGHT, framePos.z);

    glBindTexture(GL_TEXTURE_2D, *((GLuint*)pTexture->res));
    vec4 texParam = { 1.0f / pTexture->width, 1.0f / pTexture->height, 1.0f / pTexture->count, 0.0f };

    pShader->bind();
    pShader->setMatrix(uViewProjMatrix, &gViewProjMatrix);
    pShader->setVector(uTexParam, &texParam, 1);
    pShader->setVector(uAmbient, &gAmbient, 1);
    pShader->setVector(uLightColor, gLightColor, MAX_LIGHTS);
    pShader->setVector(uLightPos, gLightPos, MAX_LIGHTS);

    renderMesh(0, frameIndex, skeleton, animSkeleton);
}

void Model::renderMesh(uint32 meshIndex, uint32 frameIndex, const Skeleton* skeleton, const Skeleton* animSkeleton)
{
    const Skeleton::Offset& offset = skeleton->offsets[meshIndex];
    gModelMatrix.translate(offset.x, offset.y, offset.z);

    int32 rx, ry, rz;
    animSkeleton->getAngles(frameIndex, meshIndex, rx, ry, rz);

    gModelMatrix.rotateX(rx * (DEG2RAD * 360.0f / 4096.0f));
    gModelMatrix.rotateY(ry * (DEG2RAD * 360.0f / 4096.0f));
    gModelMatrix.rotateZ(rz * (DEG2RAD * 360.0f / 4096.0f));

    shaderModel.setMatrix(uModelMatrix, &gModelMatrix);

    ASSERT(meshIndex < rangesCount);
    MeshRange* range = ranges + meshIndex;

    glBindVertexArray(((MeshData*)res)->VAO);
    glDrawElements(GL_TRIANGLES, range->iCount, GL_UNSIGNED_SHORT, (Index*)(range->iStart * sizeof(Index)));

    uint32 childsCount = skeleton->links[meshIndex].count;

    for (uint32 i = 0; i < childsCount; i++)
    {
        if (childsCount > 1)
        {
            matrixPush();
        }

        renderMesh(skeleton->links[meshIndex].childs[i], frameIndex, skeleton, animSkeleton);

        if (childsCount > 1)
        {
            matrixPop();
        }
    }
}

// render ==============================================
void* GetProc(const char *name)
{
#ifdef __WIN32__
    return (void*)wglGetProcAddress(name);
#elif __LINUX__
    return (void*)glXGetProcAddress((GLubyte*)name);
#endif
    return NULL;
}

#define GetProcOGL(x) x=(decltype(x))GetProc(#x)

void renderInit()
{
#ifdef __WIN32__
    PIXELFORMATDESCRIPTOR pfd;
    memset(&pfd, 0, sizeof(pfd));
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.cColorBits = 32;
    pfd.cRedBits = 8;
    pfd.cGreenBits = 8;
    pfd.cBlueBits = 8;
    pfd.cAlphaBits = 8;
    pfd.cDepthBits = 24;

    PFNWGLCHOOSEPIXELFORMATARBPROC    wglChoosePixelFormatARB    = NULL;
    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = NULL;

    {
        HWND fWnd = CreateWindow("static", NULL, WS_POPUP, 0, 0, 0, 0, 0, 0, 0, 0);
        HDC fDC = GetDC(fWnd);

        int format = ChoosePixelFormat(fDC, &pfd);
        SetPixelFormat(fDC, format, &pfd);
        HGLRC fRC = wglCreateContext(fDC);
        wglMakeCurrent(fDC, fRC);

        wglChoosePixelFormatARB    = GetProcOGL(wglChoosePixelFormatARB);
        wglCreateContextAttribsARB = GetProcOGL(wglCreateContextAttribsARB);

        wglMakeCurrent(0, 0);
        ReleaseDC(fWnd, fDC);
        wglDeleteContext(fRC);
        DestroyWindow(fWnd);
    }

    hDC = GetDC(hWnd);

    if (wglChoosePixelFormatARB && wglCreateContextAttribsARB)
    {
        const int pixelAttribs[] = {
            WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
            WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
            WGL_DOUBLE_BUFFER_ARB,  GL_TRUE,
            WGL_PIXEL_TYPE_ARB,     WGL_TYPE_RGBA_ARB,
            WGL_ACCELERATION_ARB,   WGL_FULL_ACCELERATION_ARB,
            WGL_COLOR_BITS_ARB,     32,
            WGL_ALPHA_BITS_ARB,     8,
            WGL_DEPTH_BITS_ARB,     24,
            0
        };

        int format;
        UINT numFormats;
        int status = wglChoosePixelFormatARB(hDC, pixelAttribs, NULL, 1, &format, &numFormats);
        ASSERT(status && numFormats > 0);

        DescribePixelFormat(hDC, format, sizeof(pfd), &pfd);
        SetPixelFormat(hDC, format, &pfd);

        int contextAttribs[] = {
            WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
            WGL_CONTEXT_MINOR_VERSION_ARB, 2,
            WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
            0
        };

        hRC = wglCreateContextAttribsARB(hDC, 0, contextAttribs);
        LOG("OpenGL 3.2\n");
    } else {
        int format = ChoosePixelFormat(hDC, &pfd);
        SetPixelFormat(hDC, format, &pfd);
        hRC = wglCreateContext(hDC);
        LOG("OpenGL 2.0\n");
    }

    wglMakeCurrent(hDC, hRC);
#endif

    LOG("Vendor   : %s\n", (char*)glGetString(GL_VENDOR));
    LOG("Renderer : %s\n", (char*)glGetString(GL_RENDERER));
    LOG("Version  : %s\n", (char*)glGetString(GL_VERSION));

    GetProcOGL(glGenerateMipmap);

    GetProcOGL(glCreateProgram);
    GetProcOGL(glDeleteProgram);
    GetProcOGL(glLinkProgram);
    GetProcOGL(glUseProgram);
    GetProcOGL(glGetProgramInfoLog);
    GetProcOGL(glCreateShader);
    GetProcOGL(glDeleteShader);
    GetProcOGL(glShaderSource);
    GetProcOGL(glAttachShader);
    GetProcOGL(glCompileShader);
    GetProcOGL(glGetShaderInfoLog);
    GetProcOGL(glGetUniformLocation);
    GetProcOGL(glUniform1iv);
    GetProcOGL(glUniform1fv);
    GetProcOGL(glUniform2fv);
    GetProcOGL(glUniform3fv);
    GetProcOGL(glUniform4fv);
    GetProcOGL(glUniformMatrix4fv);
    GetProcOGL(glBindAttribLocation);
    GetProcOGL(glEnableVertexAttribArray);
    GetProcOGL(glDisableVertexAttribArray);
    GetProcOGL(glVertexAttribPointer);
    GetProcOGL(glGetProgramiv);

    GetProcOGL(glGenBuffers);
    GetProcOGL(glDeleteBuffers);
    GetProcOGL(glBindBuffer);
    GetProcOGL(glBufferData);
    GetProcOGL(glBufferSubData);

    GetProcOGL(glGenVertexArrays);
    GetProcOGL(glDeleteVertexArrays);
    GetProcOGL(glBindVertexArray);

    compileShader(&shaderModel, sh_model);
    compileShader(&shaderBackground, sh_background);
    compileShader(&shaderBackgroundMask, sh_background_mask);

#ifdef _DEBUG
    compileShader(&shaderDebug, sh_debug);

    glGenVertexArrays(1, &gDebugVAO);
    glGenBuffers(2, gDebugVBO);
    glBindVertexArray(gDebugVAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gDebugVBO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, gDebugVBO[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Index) * MAX_DEBUG_INDICES, NULL, GL_DYNAMIC_DRAW);
    glBufferData(GL_ARRAY_BUFFER, sizeof(DebugVertex) * MAX_DEBUG_VERTICES, NULL, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(aCoord);
    glEnableVertexAttribArray(aColor);

    DebugVertex* dv = NULL;
    glVertexAttribPointer(aCoord, 3, GL_SHORT, GL_FALSE, sizeof(*dv), &dv->coord);
    glVertexAttribPointer(aColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(*dv), &dv->color);

    glBindVertexArray(0);
#endif

    glClearColor(0.2f + dbg_clear, 0.2f, 0.2f, 1.0f);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    glEnable(GL_DEPTH_TEST);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // init dynamic UI buffer
    glGenVertexArrays(1, &uiVAO);
    glGenBuffers(2, uiVBO);

    glBindVertexArray(uiVAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, uiVBO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, uiVBO[1]);

    // pre-fill quads index buffer
    Index indicesUI[MAX_UI_PRIMS * 6];
    for (int32 i = 0; i < MAX_UI_PRIMS; i++)
    {
        indicesUI[i * 6 + 0] = i * 4 + 0;
        indicesUI[i * 6 + 1] = i * 4 + 1;
        indicesUI[i * 6 + 2] = i * 4 + 2;
        indicesUI[i * 6 + 3] = i * 4 + 0;
        indicesUI[i * 6 + 4] = i * 4 + 2;
        indicesUI[i * 6 + 5] = i * 4 + 3;
    }

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_UI_PRIMS * 6 * sizeof(Index), indicesUI, GL_STATIC_DRAW);
    glBufferData(GL_ARRAY_BUFFER, MAX_UI_PRIMS * 4 * sizeof(VertexUI), NULL, GL_DYNAMIC_DRAW);
    
    glEnableVertexAttribArray(aCoord);
    glEnableVertexAttribArray(aTexCoord);
    glEnableVertexAttribArray(aColor);

    VertexUI* v = NULL;
    glVertexAttribPointer(aCoord, 4, GL_SHORT, GL_FALSE, sizeof(*v), &v->coord);
    glVertexAttribPointer(aTexCoord, 4, GL_SHORT, GL_FALSE, sizeof(*v), &v->uv);
    glVertexAttribPointer(aColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(*v), &v->color);

    glBindVertexArray(0);
}

void renderFree()
{
#ifdef __WIN32__
    wglMakeCurrent(0, 0);
    wglDeleteContext(hRC);
    ReleaseDC(hWnd, hDC);
#endif
}

void renderResize(int32 width, int32 height)
{
    gWidth = width;
    gHeight = height;
    glViewport(0, 0, width, height);
}

void renderSwap()
{
#ifdef __WIN32__
    SwapBuffers(hDC);
#elif __LINUX__
    glXSwapBuffers(dpy, wnd);
#endif
}

void renderClear()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void renderSetCamera(const vec3i& pos, const vec3i& target, int32 persp)
{
    vec3 P, R, U, D;
    D.x = (float)(pos.x - target.x);
    D.y = (float)(pos.y - target.y);
    D.z = (float)(pos.z - target.z);
    normalize(D);

    U.x = 0.0f;
    U.y = 1.0f;
    U.z = 0.0f;

    P.x = (float)pos.x;
    P.y = (float)pos.y;
    P.z = (float)pos.z;

    R = cross(D, U);
    normalize(R);

    U = cross(D, R);
    normalize(U);

    mat4 mView, mProj;
    mView.e00 = R.x;
    mView.e01 = R.y;
    mView.e02 = R.z;
    mView.e03 = -dot(P, R);
    mView.e10 = U.x;
    mView.e11 = U.y;
    mView.e12 = U.z;
    mView.e13 = -dot(P, U);
    mView.e20 = D.x;
    mView.e21 = D.y;
    mView.e22 = D.z;
    mView.e23 = -dot(P, D);
    mView.e30 = 0.0f;
    mView.e31 = 0.0f;
    mView.e32 = 0.0f;
    mView.e33 = 1.0f;

    mProj.perspective(160.0f / persp, (float)gWidth / (float)gHeight, PROJ_Z_NEAR, PROJ_Z_FAR);

    gViewProjMatrix = mProj * mView;
}

void renderSetAmbient(uint8 r, uint8 g, uint8 b)
{
    gAmbient.x = r / 255.0f;
    gAmbient.y = g / 255.0f;
    gAmbient.z = b / 255.0f;
    gAmbient.w = 1.0f;
}

void renderSetLight(int32 index, const vec3s& pos, uint8 r, uint8 g, uint8 b, uint16 intensity)
{
    gLightColor[index].x = r / 255.0f;
    gLightColor[index].y = g / 255.0f;
    gLightColor[index].z = b / 255.0f;
    gLightColor[index].w = 1.0f;

    gLightPos[index].x = (float)pos.x;
    gLightPos[index].y = (float)pos.y;
    gLightPos[index].z = (float)pos.z;
    gLightPos[index].w = 1.0f / intensity;
}

int32 uiAddQuad(VertexUI* vertices, const vec2s& src, const vec2s& dst, const vec2s& size, int32 z)
{
    ASSERT(z < 1024);

    vertices->coord.x = dst.x;
    vertices->coord.y = dst.y;
    vertices->coord.z = z;
    vertices->coord.w = 1;
    vertices->uv.x = src.x;
    vertices->uv.y = src.y;
    vertices->color = 0xFFFFFFFF;
    vertices++;

    vertices->coord.x = dst.x + size.x;
    vertices->coord.y = dst.y;
    vertices->coord.z = z;
    vertices->coord.w = 1;
    vertices->uv.x = src.x + size.x;
    vertices->uv.y = src.y;
    vertices->color = 0xFFFFFFFF;
    vertices++;

    vertices->coord.x = dst.x + size.x;
    vertices->coord.y = dst.y + size.y;
    vertices->coord.z = z;
    vertices->coord.w = 1;
    vertices->uv.x = src.x + size.x;
    vertices->uv.y = src.y + size.y;
    vertices->color = 0xFFFFFFFF;
    vertices++;

    vertices->coord.x = dst.x;
    vertices->coord.y = dst.y + size.y;
    vertices->coord.z = z;
    vertices->coord.w = 1;
    vertices->uv.x = src.x;
    vertices->uv.y = src.y + size.y;
    vertices->color = 0xFFFFFFFF;
    vertices++;

    return 4;
}

void renderBackground(const Texture* texture, const Texture* masks, const MaskChunk* chunks, uint32 chunksCount)
{
    ASSERT(chunksCount + 1 <= MAX_UI_PRIMS);

    static const vec2s bgPos = { 0, 0 };
    static const vec2s bgSize = { 320, 240 };

    VertexUI vertices[MAX_UI_PRIMS * 4];
    VertexUI* vptr = vertices;

    // add background
    vptr += uiAddQuad(vptr, bgPos, bgPos, bgSize, 0);

    // add background masks
    for (uint32 i = 0; i < chunksCount; i++, chunks++)
    {
        vptr += uiAddQuad(vptr, chunks->src, chunks->dst, chunks->size, chunks->depth);
    }

    int32 vCount = (vptr - vertices);

    glBindBuffer(GL_ARRAY_BUFFER, uiVBO[1]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(VertexUI) * vCount, vertices);

    glBindVertexArray(uiVAO);

    mat4 mProj;
    float h = int(320 * (float)gHeight / (float)gWidth * 0.5f);
    mProj.ortho(0, 320, 120 + h, 120 - h, 0, 1);

    { // draw background
        Shader* pShader = &shaderBackground;
        pShader->bind();
        texture->bind();
        vec4 texParam = { 1.0f / texture->width, 1.0f / texture->height, 0.0f, 0.0f };
        pShader->setMatrix(uViewProjMatrix, &mProj);
        pShader->setVector(uTexParam, &texParam, 1);

        glDisable(GL_DEPTH_TEST);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);
        glEnable(GL_DEPTH_TEST);
    }

    // draw background masks
    if (chunksCount > 0)
    {
        Shader* pShader = &shaderBackgroundMask;
        pShader->bind();
        masks->bind();
        vec4 texParam = { 1.0f / masks->width, 1.0f / masks->height, PROJ_Z_CLIP, PROJ_W_CLIP };
        pShader->setMatrix(uViewProjMatrix, &mProj);
        pShader->setVector(uTexParam, &texParam, 1);

        glDrawElements(GL_TRIANGLES, (vCount/ 4 - 1) * 6, GL_UNSIGNED_SHORT, (GLvoid*)(sizeof(Index) * 6)); // offset from background quad
    }
}

#ifdef _DEBUG
void renderDebugFlush()
{
    if (!gDebugIndicesCount)
        return;

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gDebugVBO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, gDebugVBO[1]);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(Index) * gDebugIndicesCount, gDebugIndices);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(DebugVertex) * gDebugVerticesCount, gDebugVertices);
    glDrawElements(gDebugTopology, gDebugIndicesCount, GL_UNSIGNED_SHORT, NULL);

    gDebugIndicesCount = 0;
    gDebugVerticesCount = 0;
}

void renderDebugBegin(bool planar)
{
    Shader* pShader = &shaderDebug;
    pShader->bind();

    if (planar)
    {
        mat4 mView, mProj, mViewProj;
        mView.identity();
        mView.rotateX(-PI * 0.5f);
        mProj.identity();
        mProj.ortho(-0x8000, 0x8000, -0x8000, 0x8000, -0x8000, 0x8000);
        mViewProj = mProj * mView;
        pShader->setMatrix(uViewProjMatrix, &mViewProj);
    }
    else
    {
        pShader->setMatrix(uViewProjMatrix, &gViewProjMatrix);
    }

    // color = AABBGGRR
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    glBindVertexArray(gDebugVAO);
}

void renderDebugEnd()
{
    renderDebugFlush();

    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

void renderDebugLines(const Index* indices, int32 iCount, const vec3s* vertices, int32 vCount, uint32 color)
{
    if ((gDebugIndicesCount + iCount > MAX_DEBUG_INDICES) || (gDebugVerticesCount + vCount > MAX_DEBUG_VERTICES) || (gDebugTopology != GL_LINES))
    {
        renderDebugFlush();
    }

    Index* idx = gDebugIndices + gDebugIndicesCount;
    for (int32 i = 0; i < iCount; i++, idx++)
    {
        *idx = indices[i] + gDebugVerticesCount;
    }

    DebugVertex* vert = gDebugVertices + gDebugVerticesCount;
    for (int32 i = 0; i < vCount; i++, vert++)
    {
        vert->coord = vertices[i];
        vert->color = color;
    }

    gDebugIndicesCount += iCount;
    gDebugVerticesCount += vCount;
    gDebugTopology = GL_LINES;
}
#endif
