#ifndef H_GAME
#define H_GAME

#include "common.h"

int32 gFrames;

void gameInit()
{
    room.init(MODEL_LEON);
    room.load(1, 0, 0);
}

void gameFree()
{
    room.free();
}

void gameTick()
{
    room.update();
}

void gameUpdate()
{
    gFrames += gFrameIndex - gLastFrameIndex;
    int32 count = gFrames >> 1; // 30 Hz
    gFrames -= count << 1;

    for (int32 i = 0; i < count; i++)
    {
        gameTick();
    }
}

void gameRender()
{
    renderClear();

    room.render();
}

#endif
