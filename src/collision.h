#ifndef H_COLLISION
#define H_COLLISION

#include "math.h"
#include "common.h"

enum ShapeType
{
    SHAPE_RECT,
    SHAPE_TRI_1,
    SHAPE_TRI_2,
    SHAPE_TRI_3,
    SHAPE_TRI_4,
    SHAPE_RHOMBUS,
    SHAPE_CIRCLE,
    SHAPE_OBROUND_X,
    SHAPE_OBROUND_Z,
    SHAPE_CLIMB_UP,
    SHAPE_CLIMB_DOWN,
    SHAPE_SLOPE,
    SHAPE_STAIRS,
    SHAPE_CURVE,
    SHAPE_MAX
};

struct Shape
{
    int16 x;
    int16 z;
    uint16 sx;
    uint16 sz;
};

#define COL_FLAG_ENABLE (1 << 15)

struct Collision
{
    Shape shape;
    uint16 flags;
    uint16 type;
    uint32 floor;

    inline int32 getShape() const
    {
        return (flags & 0x0F);
    }

    bool collide(int32 r, int32& px, int32& pz) const
    {
        if (!(flags & COL_FLAG_ENABLE))
            return false;

        int32 minX = shape.x;
        int32 maxX = minX + shape.sx;
        int32 minZ = shape.z;
        int32 maxZ = minZ + shape.sz;

        // fast check first
        if (px + r < minX || px - r > maxX || pz + r < minZ || pz - r > maxZ)
            return false;

        switch (getShape())
        {
            case SHAPE_RECT:
            {
                return rect(minX, maxX, minZ, maxZ, r, px, pz);
            }

            case SHAPE_TRI_1:
            {
                if (px > maxX || pz > maxZ)
                    return rect(minX, maxX, minZ, maxZ, r, px, pz);
                return line(maxX, minZ, minX, maxZ, r, px, pz);
            }

            case SHAPE_TRI_2:
            {
                if (px < minX || pz > maxZ)
                    return rect(minX, maxX, minZ, maxZ, r, px, pz);
                return line(maxX, maxZ, minX, minZ, r, px, pz);
            }

            case SHAPE_TRI_3:
            {
                if (px > maxX || pz < minZ)
                    return rect(minX, maxX, minZ, maxZ, r, px, pz);
                return line(minX, minZ, maxX, maxZ, r, px, pz);
            }

            case SHAPE_TRI_4:
            {
                if (px < minX || pz < minZ)
                    return rect(minX, maxX, minZ, maxZ, r, px, pz);
                return line(minX, maxZ, maxX, minZ, r, px, pz);
            }

            case SHAPE_RHOMBUS:
            {
                int32 cx = (minX + maxX) >> 1;
                int32 cz = (minZ + maxZ) >> 1;

                if (px < cx)
                {
                    if (pz < cz)
                        return line(cx, minZ, minX, cz, r, px, pz);
                    else
                        return line(minX, cz, cx, maxZ, r, px, pz);
                }
                else
                {
                    if (pz < cz)
                        return line(maxX, cz, cx, minZ, r, px, pz);
                    else
                        return line(cx, maxZ, maxX, cz, r, px, pz);
                }
            }

            case SHAPE_CIRCLE:
            {
                int32 cr = (maxX - minX) >> 1;
                int32 cx = minX + cr;
                int32 cz = minZ + cr;
                return circle(cr, cx, cz, r, px, pz);
            }

            case SHAPE_OBROUND_X:
            {
                int32 cr = (maxZ - minZ) >> 1;
                if (px < minX + cr)
                    return circle(cr, minX + cr, minZ + cr, r, px, pz);
                if (px > maxX - cr)
                    return circle(cr, maxX - cr, minZ + cr, r, px, pz);
                return rect(minX + cr, maxX - cr, minZ, maxZ, r, px, pz);
            }

            case SHAPE_OBROUND_Z:
            {
                int32 cr = (maxX - minX) >> 1;
                if (pz < minZ + cr)
                    return circle(cr, minX + cr, minZ + cr, r, px, pz);
                if (pz > maxZ - cr)
                    return circle(cr, minX + cr, maxZ - cr, r, px, pz);
                return rect(minX, maxX, minZ + cr, maxZ - cr, r, px, pz);
            }

            case SHAPE_CLIMB_UP:
            case SHAPE_CLIMB_DOWN:
            case SHAPE_SLOPE:
            case SHAPE_STAIRS:
            {
                return rect(minX, maxX, minZ, maxZ, r, px, pz);
            }

            case SHAPE_CURVE:
                // TODO
                break;
            default: ASSERT(0 && "unknown collision shape");
        }

        return false;
    }

    static bool circle(int32 cr, int32 cx, int32 cz, int32 r, int32& px, int32& pz)
    {
        int32 dx = px - cx;
        int32 dz = pz - cz;

        int32 minR = r + cr;
        int32 dist = dx * dx + dz * dz;

        if (dist > minR * minR)
            return false;

        dist = x_sqrt(dist);

        if (dist <= 0)
            return false;

        int32 delta = minR - dist;

        px += (dx + 8) * delta / dist;
        pz += (dz + 8) * delta / dist;

        return true;
    }

    static bool line(int32 ax, int32 az, int32 bx, int32 bz, int32 r, int32& px, int32& pz)
    {
        int32 dx = bx - ax;
        int32 dz = bz - az;

        int32 c = x_sqrt(dx * dx + dz * dz);

        if (c <= 0)
            return false;

        int32 dist = (dx * (az - pz) - dz * (ax - px)) / c + r;

        if (dist <= 0)
            return false;

        px += (-dz + 8) * dist / c;
        pz += ( dx + 8) * dist / c;

        return true;
    }

    static bool rect(int32 minX, int32 maxX, int32 minZ, int32 maxZ, int32 r, int32& px, int32& pz)
    {
        int32 closestX = x_clamp(px, minX, maxX);
        int32 closestZ = x_clamp(pz, minZ, maxZ);
        return circle(0, closestX, closestZ, r, px, pz);
    }
};

#endif
