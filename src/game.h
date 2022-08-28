#ifndef H_GAME
#define H_GAME

#include "common.h"

int32 gFrames;

Player player;
Enemy enemy;

void gameInit()
{
    player.init(MODEL_LEON);
    player.setWeapon(WEAPON_NONE);

    player.pos.x = 0;
    player.pos.y = 3000;
    player.pos.z = 3000;

//    player.init(0);
}

void gameFree()
{
    player.free();
}

void gameTick()
{
    player.update();
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

    player.render();
}

#endif
