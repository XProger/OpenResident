#ifndef H_TYPES
#define H_TYPES

#define USE_ADT
#define USE_BSS

#include <memory.h>

typedef signed char         int8;
typedef unsigned char       uint8;
typedef short int           int16;
typedef unsigned short int  uint16;
typedef int                 int32;
typedef unsigned int        uint32;

typedef uint16 Index;

struct vec2s
{
    int16 x, y;
};

struct vec3s
{
    int16 x, y, z;
};

struct vec4s
{
    int16 x, y, z, w;
};

struct vec3i
{
    int32 x, y, z;
};

struct vec4i
{
    int32 x, y, z, w;
};

struct MaskChunk
{
    vec2s src;
    vec2s dst;
    vec2s size;
    uint16 depth;
};

enum ShapeType
{
    SHAPE_RECT,
    SHAPE_TRI_1,
    SHAPE_TRI_2,
    SHAPE_TRI_3,
    SHAPE_TRI_4,
    SHAPE_RHOMBUS,
    SHAPE_CIRCLE,
    SHAPE_OBROUND_X,
    SHAPE_OBROUND_Z,
    SHAPE_CLIMB_UP,
    SHAPE_CLIMB_DOWN,
    SHAPE_SLOPE,
    SHAPE_STAIRS,
    SHAPE_CURVE,
    SHAPE_MAX
};

struct Shape
{
    int16 x;
    int16 z;
    uint16 sx;
    uint16 sz;
};

#define COUNT(arr)      int32(sizeof(arr) / sizeof(arr[0]))
#define BITS_MASK(n)    ((1 << (n)) - 1)

#ifdef _DEBUG
    #if __LINUX__
        #include <signal.h>
        #define __debugbreak() raise(SIGTRAP)
    #endif

    #define LOG(...)        printf(__VA_ARGS__)
    #define ASSERT(expr)    if (!(expr)) { LOG("ASSERT:\n  %s:%d\n  %s => %s\n", __FILE__, __LINE__, __FUNCTION__, #expr); __debugbreak(); }
#else
    #define LOG(...)
    #define ASSERT(x)
#endif

#define FLOOR_HEIGHT    (-1800)
#define FLOOR_STEP      (-200)

#endif
