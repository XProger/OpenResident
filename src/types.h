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

#define COUNT(arr)      int32(sizeof(arr) / sizeof(arr[0]))
#define BITS_MASK(n)    ((1 << (n)) - 1)

#ifdef _DEBUG
    #define LOG(...)        printf(__VA_ARGS__)
    #define ASSERT(expr)    if (!(expr)) { LOG("ASSERT:\n  %s:%d\n  %s => %s\n", __FILE__, __LINE__, __FUNCTION__, #expr); __debugbreak(); }
#else
    #define LOG(...)
    #define ASSERT(x)
#endif

#endif
