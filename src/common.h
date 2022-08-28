#ifndef H_COMMON
#define H_COMMON

#include <memory.h>

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

#include "tables.h"
#include "stream.h"
#include "input.h"
#include "render.h"
#include "player.h"
#include "enemy.h"



#endif
