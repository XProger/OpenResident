#ifndef H_DEBUG
#define H_DEBUG

#ifdef _DEBUG

#include "room.h"

void debugDrawCameraSwitches(const CameraSwitch* cameraSwitch, int32 cameraIndex, int32 floor)
{
    //renderDebugBegin();

    while (1)
    {
        cameraSwitch++;

        if (cameraSwitch->from != cameraIndex)
            break;

        if (cameraSwitch->floor != floor && cameraSwitch->floor != 0xFF)
            continue;

        vec3s p[] = {
            { cameraSwitch->a.x, 0, cameraSwitch->a.y },
            { cameraSwitch->b.x, 0, cameraSwitch->b.y },
            { cameraSwitch->c.x, 0, cameraSwitch->c.y },
            { cameraSwitch->d.x, 0, cameraSwitch->d.y },
        };
        renderDebugRectangle(p, 0x80802020);
    }
}

void debugDrawFloors(const Floor* floor, int32 count)
{
    for (int32 i = 0; i < count; i++, floor++)
    {
        int16 y = floor->y;
        int16 minX = floor->shape.x;
        int16 minZ = floor->shape.z;
        int16 maxX = minX + floor->shape.sx;
        int16 maxZ = minZ + floor->shape.sz;

        vec3s v[] = {
            { minX, y, maxZ },
            { maxX, y, maxZ },
            { maxX, y, minZ },
            { minX, y, minZ },
        };

        renderDebugRectangle(v, 0x8020FF20);
    }
}

void debugDrawCollisions(const Collision* collision, int32 count)
{
    renderDebugBegin();

    for (int32 i = 0; i < count; i++, collision++)
    {
        int16 minX = collision->shape.x;
        int16 minZ = collision->shape.z;
        int16 maxX = minX + collision->shape.sx;
        int16 maxZ = minZ + collision->shape.sz;

        //int16 y = (collision->type >> 11) * 100;
        //int16 y = -1800 * ((collision->type >> 6) & 0x1F);
        //y -= (collision->type >> 11) * 100;
        int16 y = 0;

        vec3s p[] = {
            { minX, y, maxZ },
            { maxX, y, maxZ },
            { maxX, y, minZ },
            { minX, y, minZ },
            // extra dup for triangles
            { minX, y, maxZ },
            { maxX, y, maxZ }
        };

        int32 shape = collision->getShape();

        switch (shape)
        {
            case SHAPE_RECT:
            {
                renderDebugRectangle(p, 0x40FFFFFF);
                break;
            }
            case SHAPE_TRI_1:
            case SHAPE_TRI_2:
            case SHAPE_TRI_3:
            case SHAPE_TRI_4:
            {
                const int32 startIdx[] = { 0, 3, 1, 2 };
                renderDebugTriangle(p + startIdx[shape - SHAPE_TRI_1], 0x40FFFFFF);
                break;
            }
            case SHAPE_RHOMBUS:
            {
                int16 cx = (minX + maxX) >> 1;
                int16 cz = (minZ + maxZ) >> 1;

                vec3s r[] = {
                    { cx,   y, maxZ },
                    { maxX, y, cz   },
                    { cx,   y, minZ },
                    { minX, y, cz   }
                };

                renderDebugRectangle(r, 0x40FFFFFF);
                break;
            }
            case SHAPE_CIRCLE:
            {
                ASSERT((maxX - minX) == (maxZ - minZ));
                int32 R = (maxX - minX) >> 1;
                vec3s c;
                c.x = minX + R;
                c.y = y;
                c.z = minZ + R;
                renderDebugRounded(c, R, 0, 0, 0x40FFFFFF);
                break;
            }
            case SHAPE_OBROUND_X:
            {
                int32 R = (maxZ - minZ) >> 1;
                vec3s c;
                c.x = (minX + maxX) >> 1;
                c.y = y;
                c.z = (minZ + maxZ) >> 1;
                renderDebugRounded(c, R, ((maxX - minX) >> 1) - R, 0, 0x40FFFFFF);
                break;
            }
            case SHAPE_OBROUND_Z:
            {
                int32 R = (maxX - minX) >> 1;
                vec3s c;
                c.x = (minX + maxX) >> 1;
                c.y = y;
                c.z = (minZ + maxZ) >> 1;
                renderDebugRounded(c, R, 0, ((maxZ - minZ) >> 1) - R, 0x40FFFFFF);
                break;
            }
            case SHAPE_CLIMB_UP:
            case SHAPE_CLIMB_DOWN:
            case SHAPE_SLOPE:
            case SHAPE_STAIRS:
            {
                renderDebugRectangle(p, 0x40CCFFCC);
                break;
            }

            case SHAPE_CURVE:
                break;
            default:;
                ASSERT(0);
        }
    }
}

void debugDrawDoors(const Door* door, int32 count)
{
    renderDebugBegin();

    for (int32 i = 0; i < count; i++, door++)
    {
        int16 minX = door->shape.x;
        int16 minZ = door->shape.z;
        int16 maxX = minX + door->shape.sx;
        int16 maxZ = minZ + door->shape.sz;

        vec3s p[] = {
            { minX, 0, maxZ },
            { maxX, 0, maxZ },
            { maxX, 0, minZ },
            { minX, 0, minZ }
        };

        renderDebugRectangle(p, 0x4000FF00);
    }
}

#endif

#endif