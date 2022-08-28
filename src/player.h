#ifndef H_PLAYER
#define H_PLAYER

#include <windows.h>
#include "common.h"

#define PLAYER_SPEED_WALK           80
#define PLAYER_SPEED_RUN            200
#define PLAYER_SPEED_BACK           65
#define PLAYER_SPEED_HURT_WALK      65
#define PLAYER_SPEED_HURT_RUN       190
#define PLAYER_SPEED_HURT_BACK      45
#define PLAYER_SPEED_BADLY_WALK     30
#define PLAYER_SPEED_BADLY_RUN      80
#define PLAYER_SPEED_BADLY_BACK     25
#define PLAYER_TURN_ANGLE           1024

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
    int16 speed;
    int16 angle;

    Model model;
    Model weapon;

    int32 frameCounter;
    int32 frameIndex;
    int32 animFrame;
    int32 health;

    void init(ModelID id)
    {
        pos.x = pos.y = pos.z = 0;
        speed = 0;
        angle = 0;

        modelId = id;

        weaponId = WEAPON_MAX;
        animId = ANIM_IDLE;
        stateId = STATE_IDLE;
        animFrame = 0;
        frameCounter = 0;
        frameIndex = 0;
        health = 100;

        char path[32];
        strcpy(path, "Pl0/PLD/PL");
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
        strcpy(path, "Pl0/PLD/PL");
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
        frameCounter = 0;
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
            case STATE_AIM:
                update_AIM();
                break;
            case STATE_DEATH:
                update_DEATH();
                break;
        }

        if (animId >= ANIM_WALK)
        {
            if (!frameCounter)
            {
                const Animation::Clip& clip = weapon.animation.clips[animId - ANIM_WALK];

                frameIndex = weapon.animation.getFrameIndex(clip.start + (animFrame % clip.count));

                int32 sx = weapon.skeleton.frames[frameIndex].speed.x;

                char buf[256];
                sprintf(buf, "%d %d\n", sx & 0x3F, sx >> 6);
                OutputDebugString(buf);

                frameCounter = 0;// (sx < 0 ? -sx : sx) & 0x3F;
                animFrame++;
            }
            else
            {
                frameCounter--;
            }

            //speed = 0;
            if (speed)
            {
                int32 s, c;
                sincos(angle, s, c);

                pos.x += c * speed >> 14;
                pos.z += s * speed >> 14;
            }
        }
    }

    bool checkTurn()
    {
        if (!((gPad & IN_LEFT) ^ (gPad & IN_RIGHT)))
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
        if (checkTurn())
        {
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
        else
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
        speed = 0;
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
        speed = PLAYER_SPEED_WALK;
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
        speed = PLAYER_SPEED_RUN;
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
        speed = -PLAYER_SPEED_BACK;
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
                setAnim(ANIM_AIM_DOWN); // TODO aim down
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
        speed = 0;
    }

    void update_DEATH()
    {
        setAnim(ANIM_DEATH);
        speed = 0;
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
