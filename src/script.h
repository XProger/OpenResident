#ifndef H_SCRIPT
#define H_SCRIPT

#include "common.h"

enum ScriptCmd
{
    CMD_NOP,
    CMD_RET,
    CMD_WAIT,
    CMD_CHAIN,
    CMD_EXEC,
    CMD_KILL,
    CMD_IF,
    CMD_IF_ELSE,
    CMD_IF_END,
    CMD_RESET_SLEEP,
    CMD_SLEEP,
    CMD_RESET_WSLEEP,
    CMD_WSLEEP,
    CMD_FOR,
    CMD_FOR_END,
    CMD_WHILE,
    CMD_WHILE_END,
    CMD_DO,
    CMD_DO_END,
    CMD_SWITCH,
    CMD_SWITCH_CASE,
    CMD_SWITCH_DEFAULT,
    CMD_SWITCH_END,
    CMD_GOTO,
    CMD_SUB,
    CMD_SUB_RET,
    CMD_BREAK,
    CMD_FOR_2,
    CMD_BREAKPOINT,
    CMD_WORK_COPY,
    CMD_NOP_1E,
    CMD_NOP_1F,
    CMD_NOP_20,
    CMD_BIT_TST,
    CMD_BIT_CHG,
    CMD_CMP,
    CMD_SAVE,
    CMD_COPY,
    CMD_CALC_IMM,
    CMD_CALC_VAR,
    CMD_RND,
    CMD_CAM_SET,
    CMD_CAM_PREV,
    CMD_MSG,
    CMD_AOT,
    CMD_MDL_SET,
    CMD_WORK_SET,
    CMD_SPEED_SET,
    CMD_ADD_SPEED,
    CMD_ADD_ACC,
    CMD_POS_SET,
    CMD_DIR_SET,
    CMD_MEM_SET,
    CMD_MEM_SET2,
    CMD_SE_ON,
    CMD_COL_ID_SET,
    CMD_FLOOR_SET,
    CMD_DIR_TST,
    CMD_ESPR_ON,
    CMD_DOOR_SET,
    CMD_CUT_AUTO,
    CMD_MEM_COPY,
    CMD_MEM_CMP,
    CMD_PLC_ANIM,
    CMD_PLC_DEST,
    CMD_PLC_NECK,
    CMD_PLC_RET,
    CMD_PLC_FLG,
    CMD_EM_SET,
    CMD_COL_CHG_SET,
    CMD_AOT_RESET,
    CMD_AOT_ON,
    CMD_SUPER_SET,
    CMD_SUPER_RESET,
    CMD_PLC_GUN,
    CMD_CAM_SWP,
    CMD_ESPR_KILL,
    CMD_DOOR_MDL_SET,
    CMD_ITEM_AOT_SET,
    CMD_KEY_TST,
    CMD_TRG_TST,
    CMD_BGM_CTRL,
    CMD_ESPR_CTRL,
    CMD_FADE_SET,
    CMD_ESPR_3D_ON,
    CMD_MEM_CALC,
    CMD_MEM_CALC2,
    CMD_BGM_SET,
    CMD_PLC_ROT,
    CMD_XA_ON,
    CMD_WPN_CHG,
    CMD_PLC_CNT,
    CMD_SHAKE_ON,
    CMD_DIV_SET,
    CMD_ITEM_TST,
    CMD_XA_VOL,
    CMD_KAGE_SET,
    CMD_CAM_BE_SET,
    CMD_ITEM_LOST,
    CMD_GUN_FX,
    CMD_ESPR_ON2,
    CMD_ESPR_KILL2,
    CMD_PLC_STOP,
    CMD_AOT_SET_4P,
    CMD_DOOR_SET_4P,
    CMD_ITEM_SET_4P,
    CMD_LIGHT_POS,
    CMD_LIGHT_LUM,
    CMD_OBJ_RESET,
    CMD_SCR_SCROLL,
    CMD_PARTS_SET,
    CMD_MOVIE_ON
};

enum CompareFunc
{
    CMP_EQ,
    CMP_GT,
    CMP_GE,
    CMP_LT,
    CMP_LE,
    CMP_NE,
    CMP_UNKNOWN
};

#define MAX_SCRIPT_SUBS 16

uint16 gScriptSubs[MAX_SCRIPT_SUBS];

int32 scriptProcess(Stream* stream)
{
    while (1)
    {
        ScriptCmd cmd = (ScriptCmd)stream->u8();
        switch (cmd)
        {
            case CMD_NOP:
            {
                break;
            }

            case CMD_RET:
            {
                return stream->u8();
            }

            case CMD_WAIT:
            case CMD_CHAIN:
                ASSERT(0);
                break;

            case CMD_EXEC:
            {
                stream->skip(2); // TODO
                uint8 sub = stream->u8();
                stream->setPos(gScriptSubs[sub]);
                break;
            }

            case CMD_KILL:
                ASSERT(0);
                break;

            case CMD_IF:
            {
                stream->skip(1);
                uint16 length = stream->u16();
                break;
            }

            case CMD_IF_ELSE:
            {
                stream->skip(1);
                uint16 length = stream->u16();
                break;
            }

            case CMD_IF_END:
            {
                // TODO
                break;
            }

            case CMD_RESET_SLEEP:
            case CMD_SLEEP:
            case CMD_RESET_WSLEEP:
            case CMD_WSLEEP:
            case CMD_FOR:
            case CMD_FOR_END:
            case CMD_WHILE:
            case CMD_WHILE_END:
            case CMD_DO:
            case CMD_DO_END:
            case CMD_SWITCH:
            case CMD_SWITCH_CASE:
            case CMD_SWITCH_DEFAULT:
            case CMD_SWITCH_END:
            case CMD_GOTO:
            case CMD_SUB:
            case CMD_SUB_RET:
            case CMD_BREAK:
            case CMD_FOR_2:
            case CMD_BREAKPOINT:
            case CMD_WORK_COPY:
                ASSERT(0);
                break;

            case CMD_NOP_1E:
            case CMD_NOP_1F:
            case CMD_NOP_20:
                break;

            case CMD_BIT_TST:
            {
                uint8 bits = stream->u8();
                uint8 index = stream->u8();
                uint8 value = stream->u8();
                // TODO
                break;
            }

            case CMD_BIT_CHG:
            {
                stream->skip(3); // TODO
                break;
            }

            case CMD_CMP:
            {
                uint8 padding = stream->u8();
                uint8 index = stream->u8();
                uint8 func = stream->u8();
                int16 value = stream->s16();
                ASSERT(func >= CMP_EQ && func < CMP_UNKNOWN);
                // TODO
                break;
            }

            case CMD_SAVE:
            case CMD_COPY:
            case CMD_CALC_IMM:
            case CMD_CALC_VAR:
            case CMD_RND:
                ASSERT(0);

            case CMD_CAM_SET:
            {
                room.setCameraIndex(stream->u8());
                break;
            }

            case CMD_CAM_PREV:
            case CMD_MSG:
                ASSERT(0);
                break;

            case CMD_AOT:
            {
                stream->skip(19); // TODO
                break;
            }

            case CMD_MDL_SET:
            {
                stream->skip(37); // TODO
                break;
            }

            case CMD_WORK_SET:
            {
                stream->skip(2); // TODO
                break;
            }

            case CMD_SPEED_SET:
            case CMD_ADD_SPEED:
            case CMD_ADD_ACC:
                ASSERT(0);

            case CMD_POS_SET:
            {
                vec3i pos;
                stream->skip(1); // padding?
                pos.x = stream->s16();
                pos.y = stream->s16();
                pos.z = stream->s16();
                room.player.pos = pos;
                break;
            }

            case CMD_DIR_SET:
            case CMD_MEM_SET:
            case CMD_MEM_SET2:
            case CMD_SE_ON:
                ASSERT(0);

            case CMD_COL_ID_SET:
            {
                stream->skip(3); // TODO
                break;
            }

            case CMD_FLOOR_SET:
            case CMD_DIR_TST:
                ASSERT(0);

            case CMD_ESPR_ON:
            {
                stream->skip(15); // TODO
                break;
            }

            case CMD_DOOR_SET:
            {
                Door door;
                uint8 id = stream->u8();
                stream->u16(); // TODO
                stream->u16(); // TODO
                door.shape.x = stream->s16();
                door.shape.z = stream->s16();
                door.shape.sx = stream->u16();
                door.shape.sz = stream->u16();
                door.pos.x = stream->s16();
                door.pos.y = stream->s16();
                door.pos.z = stream->s16();
                door.angle = stream->s16();
                door.stageIdx = stream->u8();
                door.roomIdx = stream->u8();
                door.cameraIdx = stream->u8();
                door.floor = stream->u8();
                door.texId = stream->u8();
                door.type = stream->u8();
                door.sndId = stream->u8();
                door.keyId = stream->u8();
                door.keyType = stream->u8();
                door.unlocked = stream->u8();
                room.setDoor(id, &door);
                break;
            }

            case CMD_CUT_AUTO:
            {
                stream->skip(1); // TODO 0:item, 1:map
                break;
            }

            case CMD_MEM_COPY:
                ASSERT(0);

            case CMD_MEM_CMP:
            {
                stream->skip(5); // TODO
                break;
            }

            case CMD_PLC_ANIM:
            case CMD_PLC_DEST:
            case CMD_PLC_NECK:
            case CMD_PLC_RET:
            case CMD_PLC_FLG:
                ASSERT(0);
                break;

            case CMD_EM_SET:
            {
                stream->skip(1); // TODO
                uint8 id = stream->u8();
                uint8 model = stream->u8();
                uint8 state = stream->u8();
                stream->skip(5);
                int16 x = stream->s16();
                int16 y = stream->s16();
                int16 z = stream->s16();
                int16 angle = stream->s16();
                stream->skip(4); // TODO
                room.setEnemy(id, model, x, y, z, angle);
                break;
            }

            case CMD_COL_CHG_SET:
                ASSERT(0);
            
            case CMD_AOT_RESET:
            {
                stream->skip(9); // TODO
                break;
            }

            case CMD_AOT_ON:
            case CMD_SUPER_SET:
            case CMD_SUPER_RESET:
            case CMD_PLC_GUN:
                ASSERT(0);

            case CMD_CAM_SWP:
            {
                uint8 from = stream->u8();
                uint8 to = stream->u8();
                room.swapCameraSwitch(from, to);
                break;
            }

            case CMD_ESPR_KILL:
            case CMD_DOOR_MDL_SET:
                ASSERT(0);

            case CMD_ITEM_AOT_SET:
            {
                stream->skip(21); // TODO
                break;
            }

            case CMD_KEY_TST:
            case CMD_TRG_TST:
                ASSERT(0);

            case CMD_BGM_CTRL:
            {
                stream->skip(5);
                break;
            }

            case CMD_ESPR_CTRL:
            case CMD_FADE_SET:
            case CMD_ESPR_3D_ON:
            case CMD_MEM_CALC:
            case CMD_MEM_CALC2:
            case CMD_BGM_SET:
            case CMD_PLC_ROT:
            case CMD_XA_ON:
            case CMD_WPN_CHG:
            case CMD_PLC_CNT:
            case CMD_SHAKE_ON:
            case CMD_DIV_SET:
            case CMD_ITEM_TST:
            case CMD_XA_VOL:
            case CMD_KAGE_SET:
            case CMD_CAM_BE_SET:
            case CMD_ITEM_LOST:
            case CMD_GUN_FX:
            case CMD_ESPR_ON2:
            case CMD_ESPR_KILL2:
            case CMD_PLC_STOP:
                ASSERT(0);
                break;

            case CMD_AOT_SET_4P:
            {
                stream->skip(27); // TODO
                break;
            }

            case CMD_DOOR_SET_4P:
            case CMD_ITEM_SET_4P:
            case CMD_LIGHT_POS:
            case CMD_LIGHT_LUM:
                ASSERT(0);

            case CMD_OBJ_RESET:
                // TODO
                break;

            case CMD_SCR_SCROLL:
            case CMD_PARTS_SET:
            case CMD_MOVIE_ON:
                ASSERT(0);

            default:
                ASSERT(0);
                break;
        }
    }
}

void scriptRun(Stream* stream)
{
    int32 basePos = stream->getPos();

    int32 count = stream->u16();
    gScriptSubs[0] = count + basePos;

    count >>= 1;
    ASSERT(count <= MAX_SCRIPT_SUBS);

    for (int32 i = 1; i < count; i++)
    {
        gScriptSubs[i] = stream->u16() + basePos;
    }

    stream->setPos(gScriptSubs[0]);
    scriptProcess(stream);
}

#endif
