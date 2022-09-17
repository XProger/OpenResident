#ifndef H_ROOM
#define H_ROOM

#include "common.h"

#ifdef USE_ADT
#include "lzss.h"
#endif

#ifdef USE_BSS
#include "mdec.h"
#endif

#ifdef _DEBUG
    #define DEBUG_CAMERA_SWITCHES
    //#define DEBUG_FLOORS
    #define DEBUG_COLLISIONS
    #define DEBUG_DOORS
#endif

#define MAX_COLLISIONS          64
#define MAX_CAMERAS             16
#define MAX_CAMERA_SWITCHES     64
#define MAX_FLOORS              16
#define MAX_ENEMIES             34
#define MAX_DOORS               8
#define MAX_MASK_CHUNKS         1024
#define MAX_MASKS               8

#define DOOR_RADIUS             600

void scriptRun(Stream* stream);

struct Camera
{
    uint16 flags;
    uint16 persp;
    vec3i pos;
    vec3i target;

    MaskChunk* maskChunks;
    uint16 maskChunksCount;
};

struct CameraSwitch
{
    uint8 flags;
    uint8 floor;
    uint8 from;
    uint8 to;
    vec2s a, b, c, d;

    bool intersect(int32 x, int32 y) const
    {
        int32 px, py;

        // check ABD
        px = x - a.x;
        py = y - a.y;
        if ((b.y - a.y) * px < (b.x - a.x) * py)
            return false;
        if ((d.x - a.x) * py < (d.y - a.y) * px)
            return false;

        // check CDB
        px = x - c.x;
        py = y - c.y;
        if ((b.x - c.x) * py < (b.y - c.y) * px)
            return false;
        if ((d.y - c.y) * px < (d.x - c.x) * py)
            return false;

        return true;
    }
};

struct Floor
{
    Shape shape;
    uint16 soundIdx;
    uint16 y;
};

struct Door
{
    Shape shape;
    vec3s pos;
    int16 angle;
    uint8 stageIdx;
    uint8 roomIdx;
    uint8 cameraIdx;
    uint8 floor;
    uint8 texId;
    uint8 type;
    uint8 sndId;
    uint8 keyId;
    uint8 keyType;
    uint8 unlocked;

    bool intersect(int32 x, int32 z) const
    {
        return ((x >= shape.x) && (x <= shape.x + shape.sx) &&
                (z >= shape.z) && (z <= shape.z + shape.sz));
    }
};

#include "debug.h"

struct Room
{
    Player player;
    Enemy enemies[MAX_ENEMIES];
    Door doors[MAX_DOORS];

    int32 collisionsCount;
    int32 floorsCount;

    Collision collisions[MAX_COLLISIONS + 1 + MAX_ENEMIES]; // + player + enemies
    Camera cameras[MAX_CAMERAS];
    CameraSwitch cameraSwitches[MAX_CAMERA_SWITCHES];
    Floor floors[MAX_FLOORS];
    MaskChunk maskChunks[MAX_MASK_CHUNKS];

    Texture background;
    Texture masks;

    int32 stageIndex;
    int32 roomIndex;
    int32 cameraIndex;
    int32 playerIndex;

    CameraSwitch* cameraSwitchStart;

    void init(ModelID modelId)
    {
        playerIndex = player.modelId;

        player.init(modelId);
        player.setWeapon(WEAPON_NONE);
        player.pos.x = 18800;
        player.pos.y = 0;
        player.pos.z = -3160;
        player.angle = -0x8000;

        memset(enemies, 0, sizeof(enemies));

        cameraSwitchStart = cameraSwitches;
    }

    void free()
    {
        player.free();

        for (int32 i = 0; i < MAX_ENEMIES; i++)
        {
            if (enemies[i].active)
            {
                enemies[i].free();
            }
        }
    }

    void load(int32 stageIdx, int32 roomIdx, int32 cameraIdx)
    {
        memset(doors, 0, sizeof(doors));
        memset(enemies, 0, sizeof(enemies));

        stageIndex = stageIdx;
        roomIndex = roomIdx;
        loadInfo();
        setCameraIndex(cameraIdx);

        player.collision = collisions + collisionsCount;
        for (int32 i = 0; i < MAX_ENEMIES; i++)
        {
            enemies[i].collision = collisions + collisionsCount + 1 + i;
        }

        for (int32 i = collisionsCount; i < collisionsCount + 1 + MAX_ENEMIES; i++)
        {
            Collision* collision = collisions + i;
            collision->flags = SHAPE_CIRCLE | COL_FLAG_ENABLE;
            collision->shape.x =
            collision->shape.z =
            collision->shape.sx =
            collision->shape.sz = 0;
        }
    }

    void loadInfo()
    {
        char path[64];
        strcpy(path, "PL0/RDT/ROOM____.RDT");
        path[12] = '0' + stageIndex;
        addSuffix(path + 13, roomIndex);
        path[15] = '0' + playerIndex;

        LOG("load [%d %d %d] %s\n", stageIndex, roomIndex, playerIndex, path);

        if (loadRDT(path))
            return;

        path[6] = 'U';

        if (loadRDT(path))
            return;

        ASSERT(0 && "RDT not found");
    }

    bool loadRDT(const char* path)
    {
        FileStream stream(path);
        if (!stream.isValid())
            return false;

        struct Header
        {
            uint8 sprites;
            uint8 cameras;
            uint8 models;
            uint8 items;
            uint8 doors;
            uint8 unknown5;
            uint8 reverb;
            uint8 unknown7;
        } header;

        struct Offsets
        {
            uint32 soundTable;
            uint32 roomVH;
            uint32 roomVB;
            uint32 modelsVH;
            uint32 modelsVB;
            uint32 unknown0;
            uint32 collision;
            uint32 cameras;
            uint32 cameraSwitches;
            uint32 lights;
            uint32 items;
            uint32 floors;
            uint32 blocks;
            uint32 text1;
            uint32 text2;
            uint32 unknown1;
            uint32 scriptInit;
            uint32 scriptExec;
            uint32 spriteAnims;
            uint32 spriteAnimOffsets;
            uint32 sprites;
            uint32 textures;
            uint32 unknown2;
        } offset;

        for (int32 i = 0; i < sizeof(header); i++)
        {
            ((uint8*)&header)[i] = stream.u8();
        }

        for (int32 i = 0; i < sizeof(offset) >> 2; i++)
        {
            ((uint32*)&offset)[i] = stream.u32();
        }

        ASSERT(header.cameras <= MAX_CAMERAS);

        { // collisions
            stream.setPos(offset.collision);
            stream.skip(4);
            //int16 cx = stream.s16(); // TODO ceiling
            //int16 cz = stream.s16();
            collisionsCount = stream.u32() - 1;
            ASSERT(collisionsCount <= MAX_COLLISIONS);
            stream.skip(4);
            // int32 ceiling = stream.s32(); // TODO ceiling
            stream.skip(4);

            for (int32 i = 0; i < collisionsCount; i++)
            {
                Collision* collision = collisions + i;
                collision->shape.x = stream.s16();
                collision->shape.z = stream.s16();
                collision->shape.sx = stream.u16();
                collision->shape.sz = stream.u16();
                collision->flags = stream.u16();
                collision->type = stream.u16();
                collision->floor = stream.u32();
            }
        }

        { // cameras
            int32 maskOffsets[MAX_CAMERAS];
            
            struct MaskInfo
            {
                vec2s pos;
                uint16 count;
            };

            MaskInfo maskInfo[MAX_MASKS];

            stream.setPos(offset.cameras);
            for (int32 i = 0; i < header.cameras; i++)
            {
                Camera* camera = cameras + i;
                camera->flags = stream.u16();
                camera->persp = stream.u16() >> 7;
                camera->pos.x = stream.s32();
                camera->pos.y = stream.s32();
                camera->pos.z = stream.s32();
                camera->target.x = stream.s32();
                camera->target.y = stream.s32();
                camera->target.z = stream.s32();
                maskOffsets[i] = stream.s32();
            }

            // masks
            MaskChunk* chunk = maskChunks;
            for (int32 i = 0; i < header.cameras; i++)
            {
                Camera* camera = cameras + i;

                camera->maskChunks = NULL;
                camera->maskChunksCount = 0;

                if (maskOffsets[i] == 0xFFFFFFFF)
                {
                    continue;
                }

                stream.setPos(maskOffsets[i]);
                uint16 masksCount = stream.u16();
                uint16 chunksCount = stream.u16();

                if (masksCount == 0xFFFF)
                    continue;
                ASSERT(masksCount <= MAX_MASKS);
                ASSERT(chunksCount > 0 && chunksCount != 0xFFFF);

                for (uint32 j = 0; j < masksCount; j++)
                {
                    maskInfo[j].count = stream.u16();
                    stream.skip(2); // unknown
                    maskInfo[j].pos.x = stream.u16();
                    maskInfo[j].pos.y = stream.u16();
                }

                camera->maskChunks = chunk;
                camera->maskChunksCount = chunksCount;

                for (uint32 j = 0; j < masksCount; j++)
                {
                    for (uint32 k = 0; k < maskInfo[j].count; k++, chunk++)
                    {
                        ASSERT(chunk - maskChunks < MAX_MASK_CHUNKS);

                        chunk->src.x = stream.u8();
                        chunk->src.y = stream.u8();
                        chunk->dst.x = stream.u8() + maskInfo[j].pos.x;
                        chunk->dst.y = stream.u8() + maskInfo[j].pos.y;
                        chunk->depth = stream.u16();
                        uint16 size = stream.u16();
                        if (size)
                        {
                            chunk->size.x = chunk->size.y = size;
                        }
                        else
                        {
                            chunk->size.x = stream.u16();
                            chunk->size.y = stream.u16();
                        }
                    }
                }

            }
        }

        // camera switches
        stream.setPos(offset.cameraSwitches);
        int32 cameraSwitchesCount = 0;
        while (1)
        {
            ASSERT(cameraSwitchesCount < MAX_CAMERA_SWITCHES);
            CameraSwitch* cameraSwitch = cameraSwitches + cameraSwitchesCount++;

            uint8 b0 = stream.u8();
            uint8 b1 = stream.u8();
            uint8 b2 = stream.u8();
            uint8 b3 = stream.u8();

            cameraSwitch->flags = b0;
            cameraSwitch->floor = b1;
            cameraSwitch->from = b2;
            cameraSwitch->to = b3;

            if ((b0 & b1 & b2 & b3) == 0xFF)
                break;

            cameraSwitch->a.x = stream.s16();
            cameraSwitch->a.y = stream.s16();
            cameraSwitch->b.x = stream.s16();
            cameraSwitch->b.y = stream.s16();
            cameraSwitch->c.x = stream.s16();
            cameraSwitch->c.y = stream.s16();
            cameraSwitch->d.x = stream.s16();
            cameraSwitch->d.y = stream.s16();
        }

        // floors
        stream.setPos(offset.floors);
        floorsCount = stream.u16();
        ASSERT(floorsCount <= MAX_FLOORS);
        for (int32 i = 0; i < floorsCount; i++)
        {
            Floor* floor = floors + i;
            floor->shape.x = stream.s16();
            floor->shape.z = stream.s16();
            floor->shape.sx = stream.u16();
            floor->shape.sz = stream.u16();
            floor->soundIdx = stream.u16();
            floor->y = stream.u16();
        }

        stream.setPos(offset.scriptInit);
        ASSERT(offset.scriptInit != 0xFFFFFFFF);
        scriptRun(&stream);

        return true;
    }

    void loadBG()
    {
    #ifdef USE_ADT
        if (loadADT())
            return;
    #endif

    #ifdef USE_BSS
        if (loadBSS())
            return;
    #endif

        ASSERT(0);
    }

#ifdef USE_ADT
    #define MAX_BG_BUFFER_SIZE ((320 * 256 * 2) * 2)

    bool loadADT()
    {
        FileStream stream("COMMON/BIN/ROOMCUT.BIN");
        if (!stream.isValid())
            return false;

        int32 index = (stageIndex - 1) * 512 + roomIndex * 16 + cameraIndex;

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

        uint8* buffer = new uint8[MAX_BG_BUFFER_SIZE];
        buffer[320 * 256 * 2] = 0xFF; // special mark to override by masks data

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
        background.count = 1;
        background.init(data32, 320, 240);

        if (buffer[320 * 256 * 2] != 0xFF) // has masks
        {
            MemoryStream masksStream(buffer, MAX_BG_BUFFER_SIZE);
            masksStream.setPos(320 * 256 * 2);
            masks.load(&masksStream);
        }

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
    // based on Patrice Mandin code https://github.com/pmandin/reevengi-tools/wiki/.BSS
    uint8* bss_tim_re2(uint8* src, int32& size)
    {
        if (*(uint16*)(src + 4) != 0xFFFF)
        {
            size = 0;
            return NULL;
        }
        size = (*((uint32*)src)); // TODO BE support
        src += 6;

        uint8* dst = new uint8[size];
        uint8* ret = src;

        int count;
        while (1)
        {
            while (!(*src & 0x10))
            {
                int32 prev = *src & 0x0F;
                int32 offset = ((*src++ & 0xE0) - 256) << 3;
                offset |= *src++;

                if (prev == 0x0F)
                {
                    prev += *src++;
                }
                prev += 3;

                while (prev--)
                {
                    dst[0] = dst[offset];
                    dst++;
                }
            }

            if (*src == 0xff)
                break;

            int32 next = ((*src++ | 0xFFE0) ^ 0xFFFF) + 1;
            if (next == 0x10)
            {
                next += *src++;
            }

            memcpy(dst, src, next);
            dst += next;
            src += next;
        }

        return dst - size;
    }

    bool loadBSS()
    {
        char path[32];
        strcpy(path, "COMMON/BSS/ROOM");
        path[15] = '0' + stageIndex;
        addSuffix(path + 16, roomIndex);
        strcat(path, ".BSS");

        FileStream stream(path);
        if (!stream.isValid())
            return false;

        int32 sectionSize = 64 << 10; // TODO: 32 for RE1
        stream.skip(cameraIndex * sectionSize);

        int32 length = stream.u16();
        int32 id = stream.u16();
        ASSERT(id == 0x3800);
        int32 qscale = stream.u16();
        int32 version = stream.u16();

        int32 bufSize = stream.getSize() - stream.getPos();
        if (bufSize > sectionSize)
            bufSize = sectionSize;

        uint8* buffer = new uint8[bufSize];
        stream.read(buffer, bufSize);

        uint8* data32 = new uint8[320 * 240 * 4];
        
        int32 maskOffset = mdec_decode(buffer, version, 320, 240, qscale, data32);

        // TODO proper calc of maskOffset
        maskOffset += 3;
        while (1)
        {
            uint32 mask = buffer[maskOffset + 1] | (buffer[maskOffset + 2] << 8);
            if ((buffer[maskOffset] == 0) && (mask == 0xFFFF || mask == 0x0000))
            {
                maskOffset -= 3;
                break;
            }
            maskOffset++;
        }

        background.x = 0;
        background.y = 0;
        background.count = 1;
        background.init(data32, 320, 240);

        int32 timSize;
        uint8* timData = bss_tim_re2(buffer + maskOffset, timSize);
        if (timData)
        {
            MemoryStream masksStream(timData, timSize);
            masks.load(&masksStream);
            delete[] timData;
        }

        delete[] data32;
        delete[] buffer;

        return true;
    }
#endif

    void setEnemy(int32 id, int32 model, int32 x, int32 y, int32 z, int32 angle)
    {
        ASSERT(id < MAX_ENEMIES);

        Enemy* enemy = enemies + id;
        if (enemy->active)
        {
            enemy->free();
        }

        enemy->init(model);
        enemy->pos.x = x;
        enemy->pos.y = y;
        enemy->pos.z = z;
        enemy->angle = angle << 4; // 4096 -> 65536
        enemy->active = true;
    }

    void setDoor(int32 id, const Door* door)
    {
        ASSERT(id < MAX_DOORS);

        doors[id] = *door;
    }

    void setCameraIndex(int32 cameraIdx)
    {
        cameraIndex = cameraIdx;

        cameraSwitchStart = cameraSwitches;
        while (cameraSwitchStart->from != cameraIndex)
            cameraSwitchStart++;
        ASSERT(cameraSwitchStart->to == 0);

        loadBG();
    }

    void swapCameraSwitch(uint8 from, uint8 to)
    {
        CameraSwitch* cameraSwitch = cameraSwitches;
        while (1)
        {
            if (cameraSwitch->from == 0xFF && cameraSwitch->to == 0xFF)
                break;

            if (cameraSwitch->from == from && cameraSwitch->to == to)
            {
                cameraSwitch->from = to;
                cameraSwitch->to = from;
            }
            cameraSwitch++;
        }
    }

    void updateCameraSwitch()
    {
        const CameraSwitch* cameraSwitch = cameraSwitchStart;

        while (1)
        {
            cameraSwitch++;

            if (cameraSwitch->from != cameraIndex)
                break;

            if (cameraSwitch->floor != player.floor && cameraSwitch->floor != 0xFF)
                continue;

            if (cameraSwitch->intersect(player.pos.x, player.pos.z))
            {
                setCameraIndex(cameraSwitch->to);
                break;
            }
        }
    }

    void collide(int32 floor, int32 r, vec3i& pos)
    {
        for (int32 i = 0; i < collisionsCount + 1 + MAX_ENEMIES; i++)
        {
            const Collision* collision = collisions + i;
            if (collision->collide(floor, r, pos))
            {
                switch (collision->getShape())
                {
                    case SHAPE_SLOPE:
                        break;
                    case SHAPE_CLIMB_UP:
                    case SHAPE_CLIMB_DOWN:
                    case SHAPE_STAIRS:
                        player.stairs = collision;
                        break;
                }
            }
        }
    }

    void update()
    {
        player.stairs = NULL;

        for (int32 i = 0; i < MAX_ENEMIES; i++)
        {
            Enemy* enemy = enemies + i;
            if (enemy->active)
            {
                enemy->setTarget(player.pos);
                enemy->update();
                enemy->collision->flags &= ~COL_FLAG_ENABLE;
                collide(enemy->floor, ENEMY_RADIUS, enemy->pos);
                enemy->collision->flags |= COL_FLAG_ENABLE;
            }
        }

        player.update();
        player.collision->flags &= ~COL_FLAG_ENABLE;
        collide(player.floor, PLAYER_RADIUS_MAIN, player.pos);
        player.collision->flags |= COL_FLAG_ENABLE;

        player.updateStairs();

        updateCameraSwitch();

        if (gPad & IN_A)
        {
            // check doors
            int32 px = player.pos.x + ((player.dir.x * DOOR_RADIUS) >> FIXED_SHIFT);
            int32 pz = player.pos.z + ((player.dir.z * DOOR_RADIUS) >> FIXED_SHIFT);

            const Door* door = doors;
            for (int32 i = 0; i < MAX_DOORS; i++, door++)
            {
                if (door->intersect(px, pz))
                {
                    player.pos.x = door->pos.x;
                    player.pos.y = door->pos.y;
                    player.pos.z = door->pos.z;
                    player.angle = -door->angle << 4;
                    player.floor = door->floor;

                    load(door->stageIdx + 1, door->roomIdx, door->cameraIdx);
                    break;
                }
            }

            // check items
            // TODO
        }
    }

    void render()
    {
        const Camera* camera = cameras + cameraIndex;
        
        renderBackground(&background, &masks, camera->maskChunks, camera->maskChunksCount);

        renderSetCamera(camera->pos, camera->target, camera->persp);

        for (int32 i = 0; i < MAX_ENEMIES; i++)
        {
            Enemy* enemy = enemies + i;
            if (enemy->active)
            {
                enemy->render();
            }
        }

        player.render();

    #ifdef DEBUG_CAMERA_SWITCHES
        debugDrawCameraSwitches(cameraSwitchStart, cameraIndex, player.floor);
    #endif

    #ifdef DEBUG_FLOORS
        debugDrawFloors(floors, floorsCount);
    #endif

    #ifdef DEBUG_COLLISIONS
        debugDrawCollisions(collisions, collisionsCount + 1 + MAX_ENEMIES, player.pos.y);
    #endif

    #ifdef DEBUG_DOORS
        debugDrawDoors(doors, MAX_DOORS);
    #endif
    }
};

Room room;

#endif
