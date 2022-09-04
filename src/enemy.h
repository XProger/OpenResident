#ifndef H_ENEMY
#define H_ENEMY

#include "common.h"

struct Enemy
{
    vec3i pos;
    Model model;

    int32 animFrame;
    int32 frameIndex;
    int32 angle;

    void init(int32 id)
    {
        char path[32];
        strcpy(path, "PL0/EMD0/EM0");
        addSuffix(path + strlen(path), id);
        strcat(path, ".EMD");

        {
            FileStream stream(path);
            model.load(&stream);
        }

        {
            char* str = path + strlen(path);
            str[-3] = 'T';
            str[-2] = 'I';
            str[-1] = 'M';

            FileStream stream(path);
            model.texture.load(&stream);
        }

        animFrame = 0;
        frameIndex = 0;
        angle = -8192;
    }

    void free()
    {
        model.free();
    }

    void update()
    {
        frameIndex = model.animation.getFrameIndex(animFrame % model.animation.totalFrames);
        animFrame++;
    }

    void render()
    {
        model.render(pos, angle, frameIndex, &model.texture, &model.skeleton, &model.skeleton);
    }
};

#endif