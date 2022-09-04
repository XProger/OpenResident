#ifndef H_ROOM
#define H_ROOM

#include "common.h"

#ifdef USE_ADT
#include "lzss.h"
#endif

#ifdef USE_BSS
#include "mdec.h"
#endif

struct Room
{
    Texture background;

    Room() {}

    Room(Stream* stream)
    {
        //
    }

    void loadBG(int32 stageIdx, int32 roomIdx, int32 cameraIdx)
    {
    #ifdef USE_ADT
        if (loadADT(stageIdx, roomIdx, cameraIdx))
            return;
    #endif

    #ifdef USE_BSS
        if (loadBSS(stageIdx, roomIdx, cameraIdx))
            return;
    #endif

        ASSERT(false);
    }

#ifdef USE_ADT
    bool loadADT(int32 stageIdx, int32 roomIdx, int32 cameraIdx)
    {
        FileStream stream("COMMON/BIN/ROOMCUT.BIN");
        if (!stream.isValid())
            return false;

        int32 index = (stageIdx - 1) * 512 + roomIdx * 16 + cameraIdx;

        int32 off = stream.u32();
        int32 count = off / 4;
        ASSERT(index < count);
        
        stream.setPos(index * 4);
        int32 offset = stream.s32();
        int32 size;
        if (index == count - 1)
        {
            size = stream.getSize() - offset;
        }
        else
        {
            size = stream.s32() - offset;
        }
        ASSERT(size > 0);
        stream.setPos(offset);

        uint8 tmpData[128 << 10];
        ASSERT(size < sizeof(tmpData));
        
        stream.read(tmpData, size);

        uint8* buffer = new uint8[256 << 10];

        unpackImage(tmpData + 4, size - 4, buffer); // skip magic

        uint8* data32 = new uint8[320 * 240 * 4];
        uint16* src = (uint16*)buffer;
        uint8* dst = data32;

        for (int32 y = 0; y < 240; y++)
        {
            for (int32 x = 0; x < 256; x++)
            {
                uint16 value = *src++;

                *dst++ = (value & 31) << 3;
                *dst++ = ((value >> 5) & 31) << 3;
                *dst++ = ((value >> 10) & 31) << 3;
                *dst++ = 255;
            }

            uint16* part = (uint16*)buffer + 256 * 256 + y * 128;

            if (y >= 128)
            {
                part -= 128 * 128 - 64;
            }

            for (int32 x = 0; x < 64; x++)
            {
                uint16 value = *part++;

                *dst++ = (value & 31) << 3;
                *dst++ = ((value >> 5) & 31) << 3;
                *dst++ = ((value >> 10) & 31) << 3;
                *dst++ = 255;
            }
        }

        background.x = 0;
        background.y = 0;
        background.width = 320;
        background.height = 240;
        background.count = 1;
        background.init(data32);

    #if 0
    #ifdef _DEBUG
        dumpBitmap("rooms.bmp", 320, 240, data32);
    #endif
    #endif

        delete[] data32;
        delete[] buffer;

        return true;
    }
#endif

#ifdef USE_BSS
    bool loadBSS(int32 stageIdx, int32 roomIdx, int32 cameraIdx)
    {
        char path[32];
        strcpy(path, "COMMON/BSS/ROOM___.BSS");
        path[15] = '0' + stageIdx;
        path[16] = '0' + (roomIdx / 10);
        path[17] = '0' + (roomIdx % 10);

        FileStream stream(path);
        if (!stream.isValid())
            return false;

        int32 sectionSize = 64 << 10; // TODO: 32 for RE1
        stream.skip(cameraIdx * sectionSize);

        int32 length = stream.u16();
        int32 id = stream.u16();
        ASSERT(id == 0x3800);
        int32 qscale = stream.u16();
        int32 version = stream.u16();
        ASSERT(version = 3);

        uint8* buffer = new uint8[length * 2];
        stream.read(buffer, length * 2);

        uint8* data32 = new uint8[320 * 240 * 4];

        mdec_decode(buffer, version, 320, 240, qscale, data32);

        background.x = 0;
        background.y = 0;
        background.width = 320;
        background.height = 240;
        background.count = 1;
        background.init(data32);

        delete[] data32;

        return true;
    }
#endif

    void render()
    {
        renderBackground(&background);
    }
};

#endif
