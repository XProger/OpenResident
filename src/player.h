#ifndef H_PLAYER
#define H_PLAYER

#include <windows.h>
#include "common.h"

#define PLAYER_TURN_ANGLE   1280

#define PLAYER_RADIUS_MAIN  450
#define PLAYER_RADIUS_AIDA  400

enum ModelID
{
    MODEL_LEON,
    MODEL_CLAIRE
};

enum WeaponID
{
    WEAPON_NONE,
    WEAPON_KNIFE,
    WEAPON_GUN,
    WEAPON_UNKNOWN_1,
    WEAPON_MAX
};

enum AnimationID
{
// PLD
    ANIM_BACK,
    ANIM_BACK_SCARED,
    ANIM_DEATH,
    ANIM_HIT_SIDE,
    ANIM_HIT_BACK,
    ANIM_HIT_FRONT,
    ANIM_INSPECT,
    ANIM_PUSH_START,
    ANIM_PUSH,
    ANIM_HURT_BACK,
// PLW
    ANIM_WALK,
    ANIM_RUN,
    ANIM_IDLE,
    ANIM_HURT_WALK,
    ANIM_HURT_RUN,
    ANIM_HURT_IDLE,
    ANIM_BADLY_WALK,
    ANIM_BADLY_RUN,
    ANIM_BADLY_IDLE,
    ANIM_AIM_START,
    ANIM_FIRE,
    ANIM_AIM,
    ANIM_FIRE_UP,
    ANIM_AIM_UP,
    ANIM_FIRE_DOWN,
    ANIM_AIM_DOWN,
    ANIM_RELOAD
};

enum StateID
{
    STATE_IDLE,
    STATE_WALK,
    STATE_RUN,
    STATE_BACK,
    STATE_TURN,
    STATE_AIM,
    STATE_DEATH
};

extern int32 gPad;

struct Player
{
    ModelID modelId;
    WeaponID weaponId;
    AnimationID animId;
    StateID stateId;

    vec3i pos;
    int16 angle;

    Model model;
    Model weapon;

    int32 animFrame;
    int32 frameIndex;
    int32 health;
    int32 floor;

    vec3s offset;

    Collision* collision;

    void init(ModelID id)
    {
        pos.x = pos.y = pos.z = 0;
        angle = 0;

        modelId = id;

        weaponId = WEAPON_MAX;
        animId = ANIM_IDLE;
        stateId = STATE_IDLE;
        animFrame = 0;
        frameIndex = 0;
        health = 100;
        floor = 0;

        char path[32];
        strcpy(path, "PL0/PLD/PL");
        addSuffix(path + strlen(path), modelId);
        strcat(path, ".PLD");

        FileStream stream(path);
        model.load(&stream);

        setWeapon(WEAPON_NONE);
    }

    void setWeapon(WeaponID id)
    {
        if (weaponId != WEAPON_MAX)
        {
            weapon.free();
        }

        weaponId = id;

        char path[32];
        strcpy(path, "PL0/PLD/PL");
        addSuffix(path + strlen(path), modelId);
        strcat(path, "W");
        addSuffix(path + strlen(path), weaponId);
        strcat(path, ".PLW");

        FileStream stream(path);
        weapon.load(&stream);
    }

    void free()
    {
        model.free();
        weapon.free();
    }

    void setAnim(AnimationID id)
    {
        if (animId == id)
            return;

        animId = id;
        animFrame = 0;
    }

    void setState(StateID id)
    {
        stateId = id;
    }

    void update()
    {
        if (health == 0)
        {
            setState(STATE_DEATH);
        }
        else if (gPad & IN_RB)
        {
            setState(STATE_AIM);
        }
        else if (gPad & IN_UP)
        {
            if (gPad & IN_X)
            {
                setState(STATE_RUN);
            }
            else
            {
                setState(STATE_WALK);
            }
        }
        else if (gPad & IN_DOWN)
        {
            setState(STATE_BACK);
        }
        else if (((gPad & IN_LEFT) != 0) ^ ((gPad & IN_RIGHT) != 0))
        {
            setState(STATE_TURN);
        }
        else
        {
            setState(STATE_IDLE);
        }

        switch (stateId)
        {
            case STATE_IDLE:
                update_IDLE();
                break;
            case STATE_WALK:
                update_WALK();
                break;
            case STATE_RUN:
                update_RUN();
                break;
            case STATE_BACK:
                update_BACK();
                break;
            case STATE_TURN:
                update_TURN();
                break;
            case STATE_AIM:
                update_AIM();
                break;
            case STATE_DEATH:
                update_DEATH();
                break;
        }

        const Animation::Clip* clip;
        const Skeleton::Frame* frame;

        if (animId >= ANIM_WALK)
        {
            clip = weapon.animation.clips + animId - ANIM_WALK;

            frameIndex = weapon.animation.getFrameIndex(clip->start + (animFrame % clip->count));
            frame = weapon.skeleton.frames + frameIndex;
        }
        else
        {
            clip = model.animation.clips + animId;

            frameIndex = model.animation.getFrameIndex(clip->start + (animFrame % clip->count));
            frame = model.skeleton.frames + frameIndex;
        }

        if (animFrame == 0)
        {
            offset.x = offset.y = offset.z = 0;

            // TODO hack
            if (stateId == STATE_IDLE || stateId == STATE_AIM)
            {
                offset = frame->offset;
            }
        }

        animFrame++;

        // play death animation once
        if (animId == ANIM_DEATH && animFrame >= clip->count)
        {
            animFrame = clip->count - 1;
        }

        animFrame %= clip->count;

        vec3i speed;
        if (stateId == STATE_TURN)
        {
            speed.x = speed.y = speed.z = 0;
        }
        else
        {
            speed.x = frame->offset.x - offset.x;
            speed.y = frame->offset.y - offset.y;
            speed.z = frame->offset.z - offset.z;
        }

        offset = frame->offset;

        if (speed.x || speed.z)
        {
            int32 s, c;
            sincos(angle, s, c);

            pos.x += (c * speed.x - s * speed.z) >> FIXED_SHIFT;
            pos.z += (s * speed.x + c * speed.z) >> FIXED_SHIFT;
        }

        pos.y += speed.y;

        collision->x = pos.x - (PLAYER_RADIUS_MAIN >> 1);
        collision->z = pos.z - (PLAYER_RADIUS_MAIN >> 1);
        collision->sx = PLAYER_RADIUS_MAIN;
        collision->sz = PLAYER_RADIUS_MAIN;
    }

    bool checkTurn()
    {
        if (!(((gPad & IN_LEFT) != 0) ^ ((gPad & IN_RIGHT) != 0)))
            return false;

        if (gPad & IN_LEFT)
        {
            angle += PLAYER_TURN_ANGLE;
        }
        else
        {
            angle -= PLAYER_TURN_ANGLE;
        }

        return true;
    }

    void update_IDLE()
    {
        if (health <= 25)
        {
            setAnim(ANIM_BADLY_IDLE);
        }
        else if (health <= 50)
        {
            setAnim(ANIM_HURT_IDLE);
        }
        else
        {
            setAnim(ANIM_IDLE);
        }
    }

    void update_WALK()
    {
        checkTurn();

        if (health <= 25)
        {
            setAnim(ANIM_BADLY_WALK);
        }
        else if (health <= 50)
        {
            setAnim(ANIM_HURT_WALK);
        }
        else
        {
            setAnim(ANIM_WALK);
        }
    }

    void update_RUN()
    {
        checkTurn();

        if (health <= 25)
        {
            setAnim(ANIM_BADLY_RUN);
        }
        else if (health <= 50)
        {
            setAnim(ANIM_HURT_RUN);
        }
        else
        {
            setAnim(ANIM_RUN);
        }
    }

    void update_BACK()
    {
        checkTurn();

        if (health <= 50)
        {
            setAnim(ANIM_HURT_BACK);
        }
        else
        {
            setAnim(ANIM_BACK);
        }
    }

    void update_TURN()
    {
        checkTurn();

        if (health <= 25)
        {
            setAnim(ANIM_BADLY_WALK);
        }
        else if (health <= 50)
        {
            setAnim(ANIM_HURT_WALK);
        }
        else
        {
            setAnim(ANIM_WALK);
        }
    }

    void update_AIM()
    {
        checkTurn();

        if (gPad & IN_UP)
        {
            if (gPad & IN_A)
            {
                setAnim(ANIM_FIRE_UP);
            }
            else
            {
                setAnim(ANIM_AIM_UP);
            }
        }
        else if (gPad & IN_DOWN)
        {
            if (gPad & IN_A)
            {
                setAnim(ANIM_FIRE_DOWN);
            }
            else
            {
                setAnim(ANIM_AIM_DOWN);
            }
        }
        else
        {
            if (gPad & IN_A)
            {
                setAnim(ANIM_FIRE);
            }
            else
            {
                setAnim(ANIM_AIM);
            }
        }
    }

    void update_DEATH()
    {
        pos.y = 1800;
        setAnim(ANIM_DEATH);
    }

    void render()
    {
        if (animId < ANIM_WALK)
        {
            model.render(pos, angle, frameIndex, &model.texture, &model.skeleton, &model.skeleton);
        }
        else
        {
            model.render(pos, angle, frameIndex, &model.texture, &model.skeleton, &weapon.skeleton);
        }
    }
};

#endif
