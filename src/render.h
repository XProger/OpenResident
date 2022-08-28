#ifndef H_RENDER
#define H_RENDER

#include "types.h"
#include "stream.h"

struct Texture
{
    void* res;
    int16 x;
    int16 y;
    int16 width;
    int16 height;
    int32 count;

    void load(Stream* stream);
    void free();
};

#define MAX_RANGES  20

struct MeshRange
{
    uint16 iStart;
    uint16 iCount;
};

#define MAX_ANIMATION_FRAMES    512
#define MAX_ANIMATION_CLIPS     20

#define ANIM_FRAME_INDEX(x)     ((x) & BITS_MASK(12))
#define ANIM_FRAME_FLAGS(x)     (((x) >> 12) & BITS_MASK(20))

struct Animation
{
    struct Clip
    {
        uint16 count;
        uint16 start;
    };

    uint32 framesInfo[MAX_ANIMATION_FRAMES];
    Clip clips[MAX_ANIMATION_CLIPS];
    uint32 clipsCount;
    uint32 totalFrames;

    void load(Stream* stream);
    void free();
    int32 getFrameIndex(int32 curIndex) const;
    int32 getFrameFlags(int32 curIndex) const;
};


#define MAX_CHILDS 15

struct Skeleton
{
    struct Offset
    {
        int16 x, y, z;
    };

    struct Link
    {
        uint16 count;
        uint16 offset;
        uint8 childs[MAX_CHILDS];
        int8 parent;
    };

    struct Frame
    {
        vec3s pos;
        vec3s speed;
        uint8 angles[80 - 12];

    };

    Offset offsets[MAX_RANGES];
    Link links[MAX_RANGES];
    uint32 count;
    Frame frames[MAX_ANIMATION_FRAMES];
    int32 dataFramesCount;

    void load(Stream* stream, const Animation* anim);
    void free();
    void getAngles(int32 frameIndex, int32 jointIndex, int32& x, int32& y, int32& z) const;
};

struct Model
{
    Animation animation;
    Skeleton skeleton;
    Texture texture;

    void* res;
    uint32 rangesCount;
    MeshRange ranges[MAX_RANGES];

    void load(Stream* stream);
    void free();
    void render(const vec3i& pos, int32 angle, uint16 frameIndex, const Texture* texture, const Skeleton* skeleton, const Skeleton* animSkeleton);
    void renderMesh(uint32 meshIndex, uint32 frameIndex, const Skeleton* skeleton, const Skeleton* animSkeleton);
};

void renderInit();
void renderFree();
void renderResize(int32 width, int32 height);
void renderSwap();
void renderClear();

#endif
