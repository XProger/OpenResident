#ifndef H_INPUT
#define H_INPUT

enum InputKey
{
    IN_NONE     = (0 << 0),
    IN_LEFT     = (1 << 0),
    IN_DOWN     = (1 << 1),
    IN_RIGHT    = (1 << 2),
    IN_UP       = (1 << 3),
    IN_X        = (1 << 4),
    IN_A        = (1 << 5),
    IN_B        = (1 << 6),
    IN_Y        = (1 << 7),
    IN_L        = (1 << 8),
    IN_R        = (1 << 9),
    IN_LB       = (1 << 10),
    IN_LT       = (1 << 11),
    IN_RB       = (1 << 12),
    IN_RT       = (1 << 13),
    IN_SELECT   = (1 << 14),
    IN_START    = (1 << 15),
    IN_HOME     = (1 << 16)
};

#endif