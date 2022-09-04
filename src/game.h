#ifndef H_GAME
#define H_GAME

#include "common.h"

int32 gFrames;

Player player;
Enemy enemy;
Room room;

void gameInit()
{
    room.loadBG(1, 0, 0);

    player.init(MODEL_LEON);
    player.setWeapon(WEAPON_NONE);
    player.pos.x = 1500;
    player.pos.y = 4000;
    player.pos.z = 6000;
    player.angle = -0x4000;

    enemy.init(0x12);
    enemy.pos.x = -1600;
    enemy.pos.y = 4300;
    enemy.pos.z = 3500;
    enemy.angle = -0x2000;
}

void gameFree()
{
    player.free();
}

void gameTick()
{
    player.update();
    enemy.update();
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
    player.render();
    enemy.render();
}

#endif
