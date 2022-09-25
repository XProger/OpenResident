#ifndef H_DEBUG
#define H_DEBUG

#ifdef _DEBUG

#include "room.h"

static const Index gIndicesRectangle[] = {
    0, 1, 1, 2, 2, 3, 3, 0,
    0+4, 1+4, 1+4, 2+4, 2+4, 3+4, 3+4, 0+4,
    0, 0+4, 1, 1+4, 2, 2+4, 3, 3+4
};

static const Index gIndicesTriangle[] = {
    0, 1, 1, 2, 2, 0,
    0+3, 1+3, 1+3, 2+3, 2+3, 0+3,
    0, 0+3, 1, 1+3, 2, 2+3
};

static const Index gIndicesObround[] = {
    0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 0,
    0+12, 1+12, 1+12, 2+12, 2+12, 3+12, 3+12, 4+12, 4+12, 5+12, 5+12, 6+12, 6+12, 7+12, 7+12, 8+12, 8+12, 9+12, 9+12, 10+12, 10+12, 11+12, 11+12, 0+12,
    0, 0+12, 1, 1+12, 2, 2+12, 3, 3+12, 4, 4+12, 5, 5+12, 6, 6+12, 7, 7+12, 8, 8+12, 9, 9+12, 10, 10+12, 11, 11+12
};

void debugDrawRectangle(const Shape& shape, int16 minY, int16 maxY, uint32 color)
{
    int16 minX = shape.x;
    int16 minZ = shape.z;
    int16 maxX = minX + shape.sx;
    int16 maxZ = minZ + shape.sz;

    const vec3s vertices[] = {
        { minX, minY, minZ },
        { maxX, minY, minZ },
        { maxX, minY, maxZ },
        { minX, minY, maxZ },
        { minX, maxY, minZ },
        { maxX, maxY, minZ },
        { maxX, maxY, maxZ },
        { minX, maxY, maxZ }
    };

    renderDebugLines(gIndicesRectangle, COUNT(gIndicesRectangle), vertices, COUNT(vertices), color);
};

void debugDrawTriangle(int32 index, const Shape& shape, int16 minY, int16 maxY, uint32 color)
{
    vec3s vertices[6];

    int16 minX = shape.x;
    int16 minZ = shape.z;
    int16 maxX = minX + shape.sx;
    int16 maxZ = minZ + shape.sz;

    switch (index)
    {
        case 0:
        {
            vertices[0] = vertices[3] = { minX, minY, maxZ };
            vertices[1] = vertices[4] = { maxX, minY, maxZ };
            vertices[2] = vertices[5] = { maxX, minY, minZ };
            break;
        }
        case 1:
        {
            vertices[0] = vertices[3] = { minX, minY, minZ };
            vertices[1] = vertices[4] = { minX, minY, maxZ };
            vertices[2] = vertices[5] = { maxX, minY, maxZ };
            break;
        }
        case 2:
        {
            vertices[0] = vertices[3] = { maxX, minY, maxZ };
            vertices[1] = vertices[4] = { maxX, minY, minZ };
            vertices[2] = vertices[5] = { minX, minY, minZ };
            break;
        }
        case 3:
        {
            vertices[0] = vertices[3] = { maxX, minY, minZ };
            vertices[1] = vertices[4] = { minX, minY, minZ };
            vertices[2] = vertices[5] = { minX, minY, maxZ };
            break;
        }
        default: ASSERT(0);
    }

    vertices[3].y = maxY;
    vertices[4].y = maxY;
    vertices[5].y = maxY;

    renderDebugLines(gIndicesTriangle, COUNT(gIndicesTriangle), vertices, COUNT(vertices), color);
};

void debugDrawRhombus(const Shape& shape, int16 minY, int16 maxY, uint32 color)
{
    int16 minX = shape.x;
    int16 minZ = shape.z;
    int16 maxX = minX + shape.sx;
    int16 maxZ = minZ + shape.sz;
    int16 cx = (minX + maxX) >> 1;
    int16 cz = (minZ + maxZ) >> 1;

    const vec3s vertices[] = {
        {   cx, minY, minZ },
        { maxX, minY,   cz },
        {   cx, minY, maxZ },
        { minX, minY,   cz },
        {   cx, maxY, minZ },
        { maxX, maxY,   cz },
        {   cx, maxY, maxZ },
        { minX, maxY,   cz },
    };

    renderDebugLines(gIndicesRectangle, COUNT(gIndicesRectangle), vertices, COUNT(vertices), color);
};

void debugDrawObround(const Shape& shape, int16 minY, int16 maxY, uint32 color)
{
    int16 r = x_min(shape.sx, shape.sz) >> 1;
    int16 x = shape.x + (shape.sx >> 1);
    int16 z = shape.z + (shape.sz >> 1);
    int16 t = int16(r * 0.7);
    int16 hx = (shape.sx >> 1) - r;
    int16 hz = (shape.sz >> 1) - r;

    const vec3s vertices[] = {
        { x - hx,     minY, z - hz - r },
        { x + hx,     minY, z - hz - r },
        { x + hx + t, minY, z - hz - t },
        { x + hx + r, minY, z - hz     },
        { x + hx + r, minY, z + hz     },
        { x + hx + t, minY, z + hz + t },
        { x + hx,     minY, z + hz + r },
        { x - hx,     minY, z + hz + r },
        { x - hx - t, minY, z + hz + t },
        { x - hx - r, minY, z + hz     },
        { x - hx - r, minY, z - hz     },
        { x - hx - t, minY, z - hz - t },

        { x - hx,     maxY, z - hz - r },
        { x + hx,     maxY, z - hz - r },
        { x + hx + t, maxY, z - hz - t },
        { x + hx + r, maxY, z - hz     },
        { x + hx + r, maxY, z + hz     },
        { x + hx + t, maxY, z + hz + t },
        { x + hx,     maxY, z + hz + r },
        { x - hx,     maxY, z + hz + r },
        { x - hx - t, maxY, z + hz + t },
        { x - hx - r, maxY, z + hz     },
        { x - hx - r, maxY, z - hz     },
        { x - hx - t, maxY, z - hz - t },
    };

    renderDebugLines(gIndicesObround, COUNT(gIndicesObround), vertices, COUNT(vertices), color);
};

void debugDrawShape(ShapeType type, const Shape& shape, int16 minY, int16 maxY, uint32 color)
{
    switch (type)
    {
        case SHAPE_RECT:
            debugDrawRectangle(shape, minY, maxY, color);
            break;

        case SHAPE_TRI_1:
        case SHAPE_TRI_2:
        case SHAPE_TRI_3:
        case SHAPE_TRI_4:
            debugDrawTriangle(type - SHAPE_TRI_1, shape, minY, maxY, color);
            break;

        case SHAPE_RHOMBUS:
            debugDrawRhombus(shape, minY, maxY, color);
            break;

        case SHAPE_CIRCLE:
        case SHAPE_OBROUND_X:
        case SHAPE_OBROUND_Z:
            debugDrawObround(shape, minY, maxY, color);
            break;

        case SHAPE_CLIMB_UP:
        case SHAPE_CLIMB_DOWN:
        case SHAPE_SLOPE:
        case SHAPE_STAIRS:
        case SHAPE_CURVE:
            debugDrawRectangle(shape, minY, maxY, color);
            break;
    }
}

void debugDrawCameraSwitches(const CameraSwitch* cameraSwitch, int32 cameraIndex, int32 floor)
{
    { // visibility area
        vec3s vertices[] = {
            { cameraSwitch->a.x, 0, cameraSwitch->a.y },
            { cameraSwitch->b.x, 0, cameraSwitch->b.y },
            { cameraSwitch->c.x, 0, cameraSwitch->c.y },
            { cameraSwitch->d.x, 0, cameraSwitch->d.y },
        };
        renderDebugLines(gIndicesRectangle, COUNT(gIndicesRectangle) / 3, vertices, COUNT(vertices), 0xFF0000FF);
    }

    while (1)
    {
        cameraSwitch++;

        if (cameraSwitch->from != cameraIndex)
            break;

        if (cameraSwitch->floor != floor && cameraSwitch->floor != 0xFF)
            continue;

        vec3s vertices[] = {
            { cameraSwitch->a.x, 0, cameraSwitch->a.y },
            { cameraSwitch->b.x, 0, cameraSwitch->b.y },
            { cameraSwitch->c.x, 0, cameraSwitch->c.y },
            { cameraSwitch->d.x, 0, cameraSwitch->d.y },
        };
        renderDebugLines(gIndicesRectangle, COUNT(gIndicesRectangle) / 3, vertices, COUNT(vertices), 0xFF00FF00);
    }
}

void debugDrawFloors(const Floor* floor, int32 count)
{
    for (int32 i = 0; i < count; i++, floor++)
    {
        int16 y = floor->y;
        debugDrawShape(SHAPE_RECT, floor->shape, y, y, 0x8020FF20);
    }
}

void debugDrawCollisions(const Collision* collision, int32 count, int32 y, uint32 floorsMask, uint32 flagsMask)
{
    for (int32 i = 0; i < count; i++, collision++)
    {
        ShapeType type = collision->getShape();
        int16 minY = collision->getHeight();
        int16 maxY = collision->getGround();

        debugDrawShape(type, collision->shape, minY, maxY, 0xFF808080);
    }
}

void debugDrawDoors(const Door* door, int32 count)
{
    for (int32 i = 0; i < count; i++, door++)
    {
        int16 y = door->floor * FLOOR_HEIGHT;
        debugDrawShape(SHAPE_RECT, door->shape, y, y, 0x4000FF00);
    }
}
#endif

#endif