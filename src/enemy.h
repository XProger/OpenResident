#ifndef H_ENEMY
#define H_ENEMY

#include "common.h"

struct Enemy
{
    vec3i pos;
    Model model;

    void init(int32 id)
    {
        char path[32];
        strcpy(path, "PL0/EMD0/EM0");
        addSuffix(path + strlen(path), id);
        strcat(path, ".EMD");

        FileStream stream(path);
        model.load(&stream);
    }

    void free()
    {
        model.free();
    }

    void update()
    {
        //
    }

    void render()
    {
        int32 frameIndex = 0;

        model.render(pos, 0, frameIndex, &model.texture, &model.skeleton, &model.skeleton);
    }
};

#endif