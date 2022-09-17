#ifndef H_ENEMY
#define H_ENEMY

#include <math.h> // temporary

#include "common.h"

#define ENEMY_RADIUS    500

enum EnemyAnimationID
{
    ENEMY_ANIM_WALK,
    ENEMY_ANIM_WALK_FAST,
    ENEMY_ANIM_RUN,
    ENEMY_ANIM_RUN_2,
    ENEMY_ANIM_WALK_RAISED,
    ENEMY_ANIM_WALK_FAST_RAISED,
    ENEMY_ANIM_RUN_RAISED,
    ENEMY_ANIM_RUN_2_RAISED
};

struct Enemy
{
    vec3i pos;
    Model model;

    int32 animId;
    int32 animFrame;
    int32 frameIndex;
    int32 health;
    int32 floor;

    int16 angle;
    int16 turn;

    bool  active;

    vec3s offset;

    Collision* collision;

    void init(int32 id)
    {
        // TODO add CDEMD0.EMS & CDEMD1.EMS loader for PSX
        char path[32];
        strcpy(path, "PL0/EMD0/EM0");
        addSuffix(path + strlen(path), id);
        strcat(path, ".EMD");

        {
            FileStream stream(path);
            ASSERT(stream.isValid());
            model.load(&stream);
        }

        {
            char* str = path + strlen(path);
            str[-3] = 'T';
            str[-2] = 'I';
            str[-1] = 'M';

            FileStream stream(path);
            ASSERT(stream.isValid());
            model.texture.load(&stream);
        }

        animFrame = 0;
        frameIndex = 0;
        angle = -8192;
        floor = 0;

        animId = rand() & 7;

        collision = NULL;
    }

    void free()
    {
        model.free();
        active = false;
    }

    void setTarget(const vec3i& target)
    {
        int32 dx = target.x - pos.x;
        int32 dz = target.z - pos.z;
        turn = int16(atan2(dz, dx) / 3.14f * 0x7FFF) - angle;
        turn = x_clamp(turn, -512, 512);
    }

    void update()
    {
        angle += turn;

        const Animation::Clip* clip = model.animation.clips + animId;

        frameIndex = model.animation.getFrameIndex(clip->start + (animFrame % clip->count));
        const Skeleton::Frame* frame = model.skeleton.frames + frameIndex;

        if (animFrame == 0)
        {
            offset.x = offset.y = offset.z = 0;
        }

        animFrame++;

        animFrame %= clip->count;

        vec3i speed;
        speed.x = frame->offset.x - offset.x;
        speed.y = frame->offset.y - offset.y;
        speed.z = frame->offset.z - offset.z;

        offset = frame->offset;

        if (speed.x || speed.z)
        {
            int32 s, c;
            x_sincos(angle, s, c);

            pos.x += (c * speed.x - s * speed.z) >> FIXED_SHIFT;
            pos.z += (s * speed.x + c * speed.z) >> FIXED_SHIFT;
        }

        pos.y += speed.y;

        collision->shape.x = pos.x - (ENEMY_RADIUS >> 1);
        collision->shape.z = pos.z - (ENEMY_RADIUS >> 1);
        collision->shape.sx = ENEMY_RADIUS;
        collision->shape.sz = ENEMY_RADIUS;
    }

    void render()
    {
        model.render(pos, angle, frameIndex, &model.texture, &model.skeleton, &model.skeleton);
    }
};

#endif