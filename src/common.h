#ifndef H_COMMON
#define H_COMMON

#include "types.h"

const char* HEX = "0123456789ABCDEF";

void addSuffix(char* str, int32 value)
{
    *str++ = HEX[value / 16];
    *str++ = HEX[value % 16];
    *str++ = '\0';
}

extern int32 gFrameIndex;
extern int32 gLastFrameIndex;

#define x_sqrt(x) sqrt((uint32)x)

#define x_clamp(x,a,b) ((x) < (a)) ? (a) : (((x) > (b)) ? (b) : (x))

#include "tables.h"
#include "stream.h"
#include "input.h"
#include "render.h"
#include "collision.h"
#include "player.h"
#include "enemy.h"
#include "room.h"
#include "script.h"

#endif
