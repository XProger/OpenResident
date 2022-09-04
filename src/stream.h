#ifndef H_STREAM
#define H_STREAM

#include <stdio.h>
#include "types.h"

struct Stream
{
    virtual bool isValid() = 0;
    virtual int32 getPos() = 0;
    virtual void setPos(int32 pos) = 0;
    virtual int32 getSize() = 0;
    virtual void skip(int32 bytes) = 0;
    virtual int32 read(void* dst, int32 size) = 0;

    inline int32 s32()
    {
        int32 value;
        read(&value, sizeof(value));
        return value;
    }

    inline uint32 u32()
    {
        return (uint32)s32();
    }

    inline int16 s16()
    {
        int16 value;
        read(&value, sizeof(value));
        return value;
    }

    inline uint16 u16()
    {
        return (uint16)s16();
    }

    inline int8 s8()
    {
        int8 value;
        read(&value, sizeof(value));
        return value;
    }

    inline uint8 u8()
    {
        return (uint8)s8();
    }
};


struct FileStream : Stream
{
    FILE* f;

    FileStream(const char* fileName)
    {
        f = fopen(fileName, "rb");
    }

    ~FileStream()
    {
        if (isValid())
        {
            fclose(f);
        }
    }

    virtual bool isValid()
    {
        return f != 0;
    }

    virtual int32 getPos()
    {
        return ftell(f);
    }

    virtual void setPos(int32 pos)
    {
        fseek(f, pos, SEEK_SET);
    }

    virtual int32 getSize()
    {
        int32 pos = getPos();
        fseek(f, 0, SEEK_END);
        int32 size = ftell(f);
        fseek(f, pos, SEEK_SET);
        return size;
    }

    virtual void skip(int32 bytes)
    {
        fseek(f, bytes, SEEK_CUR);
    }

    virtual int32 read(void* dst, int32 bytes)
    {
        int32 res = (int32)fread(dst, 1, bytes, f);
        ASSERT(bytes == res);
        return bytes;
    }
};


struct MemoryStream : Stream
{
    uint8* data;
    int32 size;
    int32 pos;
    bool useScratch;

    MemoryStream(uint8* data, int32 size) : data(data), size(size), pos(0), useScratch(false) {}
    
    virtual bool isValid()
    {
        return data != NULL;
    }

    virtual int32 getPos()
    {
        return pos;
    }

    virtual void setPos(int32 pos)
    {
        this->pos = pos;
    }

    virtual int32 getSize()
    {
        return size;
    }

    virtual void skip(int32 bytes)
    {
        pos += bytes;
    }

    virtual int32 read(void* dst, int32 bytes)
    {
        memcpy(dst, data + pos, bytes);
        pos += bytes;
        return bytes;
    }
};

#endif
