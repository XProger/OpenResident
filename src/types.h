#ifndef H_TYPES
#define H_TYPES

typedef signed char         int8;
typedef unsigned char       uint8;
typedef short int           int16;
typedef unsigned short int  uint16;
typedef int                 int32;
typedef unsigned int        uint32;

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

#define BITS_MASK(n)        ((1 << (n)) - 1)

#ifdef _DEBUG
    #define ASSERT(x) if (!(x)) __debugbreak()
#else
    #define ASSERT(x)
#endif

#endif
