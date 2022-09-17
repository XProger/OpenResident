#include <string.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <linux/input.h>
#include <X11/Xutil.h>
#include <linux/joystick.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <pwd.h>
#include <pthread.h>
#include <dirent.h>

//#include <pulse/pulseaudio.h>
//#include <pulse/simple.h>

#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glx.h>

#include "game.h"

int32 gPad;
int32 gPadStickX;
int32 gPadStickY;

int32 gFrameIndex;
int32 gLastFrameIndex;

bool isQuit;

Display* dpy;
Window wnd;

// timing
unsigned int gTimerStart;

uint32 osGetSystemTimeMS()
{
    timeval t;
    gettimeofday(&t, NULL);
    return int(t.tv_sec * 1000 + t.tv_usec / 1000 - gTimerStart);
}

void osQuit()
{
    isQuit = true;
}

#define WND_TITLE   "OpenResident"

// input
void setDown(InputKey key, bool isDown)
{
    if (isDown)
    {
        gPad |= key;
    }
    else
    {
        gPad &= ~key;
    }
}

#define INPUT_JOY_COUNT         1
#define JOY_DEAD_ZONE_STICK     8192
#define JOY_DEAD_ZONE_TRIGGER   8192
#define JOY_MIN_UPDATE_FX_TIME  50.0f

struct JoyDevice
{
    int32 fd;     // device file descriptor
    int32 fe;     // event file descriptor
    int32 Lx, Ly; // left stick axes values
    int32 vL, vR; // current value for left/right motor vibration
    int32 oL, oR; // last applied value
    int32 time;   // time when we can send effect update
    ff_effect fx; // effect structure
    uint8_t axismap[ABS_CNT]; // axis mapping
} joyDevice[INPUT_JOY_COUNT];

bool osJoyReady(int index)
{
    return joyDevice[index].fd != -1;
}

void osJoyVibrate(int32 index, int32 L, int32 R)
{
    if (!osJoyReady(index))
        return;
    joyDevice[index].vL = L;
    joyDevice[index].vR = R;
}

int32 joyAxisValue(int32 value)
{
    if (value > -JOY_DEAD_ZONE_STICK && value < JOY_DEAD_ZONE_STICK)
        return 0;
    return value;
}

void joyRumble(JoyDevice& joy)
{
    if (joy.fe == -1)
        return;

    if ((joy.oL | joy.vL | joy.oR | joy.vR) == 0)
        return;

    if (osGetSystemTimeMS() <= joy.time)
        return;

    input_event event;
    event.type = EV_FF;

    if (joy.vL != 0 || joy.vR != 0)
    {
        // update effect
        joy.fx.u.rumble.strong_magnitude = joy.vL << 8;
        joy.fx.u.rumble.weak_magnitude = joy.vR << 8;
        joy.fx.replay.length = JOY_MIN_UPDATE_FX_TIME;

        if (ioctl(joy.fe, EVIOCSFF, &joy.fx) == -1)
        {
            LOG("! joy update fx\n");
        }

        // play effect
        event.value = 1;
        event.code = joy.fx.id;
        if (write(joy.fe, &event, sizeof(event)) == -1)
        {
            LOG("! joy play fx\n");
        }
    }
    else if (joy.oL != 0 || joy.oR != 0)
    {
        // stop effect
        event.value = 0;
        event.code = joy.fx.id;
        if (write(joy.fe, &event, sizeof(event)) == -1)
        {
            LOG("! joy stop fx\n");
        }
    }

    joy.oL = joy.vL;
    joy.oR = joy.vR;

    joy.time = osGetSystemTimeMS() + joy.fx.replay.length;
}

#define TEST_BIT(arr, bit) ((arr[bit / 32] >> (bit % 32)) & 1)

void inputInit()
{
    LOG("init gamepads:\n");

    char name[128];
    for (int32 i = 0; i < INPUT_JOY_COUNT; i++)
    {
        JoyDevice& joy = joyDevice[i];

        // open device
        sprintf(name, "/dev/input/js%d", i);
        joy.fd = open(name, O_RDONLY | O_NONBLOCK);
        if (joy.fd == -1)
            continue;

        // skip init messages
        js_event event;
        while (read(joy.fd, &event, sizeof(event)) != -1 && (event.type & JS_EVENT_INIT)) {};

        // get gamepad info
        int8 axes, buttons;
        ioctl(joy.fd, JSIOCGAXES, &axes);
        ioctl(joy.fd, JSIOCGBUTTONS, &buttons);
        ioctl(joy.fd, JSIOCGAXMAP, joy.axismap);

        if (axes < 4 || buttons < 11) // is it really a gamepad?
        {
            close(joy.fd);
            joy.fd = -1;
            continue;
        }

        if (ioctl(joy.fd, JSIOCGNAME(sizeof(name)), name) < 0)
        {
            strcpy(name, "Unknown");
        }

        LOG("gamepad %d\n", i + 1);
        LOG(" name : %s\n", name);
        LOG(" btns : %d\n", int(buttons));
        LOG(" axes : %d\n", int(axes));

        joy.fe = -1;
        for (int j = 0; j < 99; j++)
        {
            sprintf(name, "/sys/class/input/js%d/device/event%d", i, j);
            DIR* dir = opendir(name);
            if (!dir)
                continue;

            closedir(dir);
            sprintf(name, "/dev/input/event%d", j);
            joy.fe = open(name, O_RDWR);
            break;
        }

        uint32 features[4];
        if (joy.fe > -1 && (ioctl(joy.fe, EVIOCGBIT(EV_FF, sizeof(features)), features) == -1 || !TEST_BIT(features, FF_RUMBLE)))
        {
            close(joy.fe);
            joy.fe = -1;
        }

        if (joy.fe > -1)
        {
            int n_effects;
            if (ioctl(joy.fe, EVIOCGEFFECTS, &n_effects) != -1)
            {
                LOG(" vibration feature %d\n", n_effects);
                joy.fx.id = -1;
                joy.fx.type = FF_RUMBLE;
                joy.fx.replay.delay = 0;
                joy.vL = joy.oL = joy.vR = joy.oR = 0.0f;
                joy.time = osGetSystemTimeMS();
            }
        }
    }
}

void inputFree()
{
    for (int i = 0; i < INPUT_JOY_COUNT; i++)
    {
        JoyDevice &joy = joyDevice[i];
        if (joy.fd == -1) continue;
        close(joy.fd);
        if (joy.fe == -1) continue;
    }
}

void inputReset()
{
    gPad = 0;
}

void inputUpdate()
{
    static const InputKey buttons[] = {
        IN_A, IN_B, IN_X, IN_Y, IN_LB, IN_RB, IN_SELECT, IN_START, IN_HOME, IN_L, IN_R
    };

    gPadStickX = gPadStickY = 256;    

    for (int i = 0; i < INPUT_JOY_COUNT; i++)
    {
        JoyDevice &joy = joyDevice[i];
    
        if (joy.fd == -1)
            continue;

        joyRumble(joy);
        
        js_event event;
        while (read(joy.fd, &event, sizeof(event)) != -1)
        {
        // buttons
            if (event.type & JS_EVENT_BUTTON)
            {
                setDown(event.number >= COUNT(buttons) ? IN_NONE : buttons[event.number], event.value == 1);
            }

        // axes
            if (event.type & JS_EVENT_AXIS)
            {
                switch (joy.axismap[event.number])
                {
                // Left stick
                    case ABS_X  : joy.Lx = joyAxisValue(event.value); break;
                    case ABS_Y  : joy.Ly = joyAxisValue(event.value); break;
                // Right stick
                //    case ABS_RX : joy.Rx = joyAxisValue(event.value); break;
                //    case ABS_RY : joy.Ry = joyAxisValue(event.value); break;
                // Left trigger
                //    case ABS_Z  : setJoyPos(i, jkLT, joyTrigger(event.value)); break;
                // Right trigger
                //    case ABS_RZ : setJoyPos(i, jkRT, joyTrigger(event.value)); break;
                // D-PAD
                    case ABS_HAT0X    :
                    case ABS_THROTTLE :
                        setDown(IN_LEFT,  event.value < -0x4000);
                        setDown(IN_RIGHT, event.value >  0x4000);
                        break;
                    case ABS_HAT0Y    :
                    case ABS_RUDDER   :
                        setDown(IN_UP,    event.value < -0x4000);
                        setDown(IN_DOWN,  event.value >  0x4000);
                        break;
                }
                
                gPadStickX = (joy.Lx + 0x8000) >> 8;
                gPadStickY = (joy.Ly + 0x8000) >> 8;

                if (gPadStickX > 256)
                    gPadStickX = 256;
                if (gPadStickY > 256)
                    gPadStickY = 256;
            }
        }
    }
}

InputKey keyToInputKey(Display *dpy, XKeyEvent event)
{
    static const struct {
        KeySym code;
        InputKey key;
    } keyMapping[] = {
        { XK_Return,    IN_A },
        { XK_c,         IN_A },
        { XK_v,         IN_X },
        { XK_z,         IN_B },
        { XK_x,         IN_RB },
        { XK_Tab,       IN_Y },
        { XK_Control_L, IN_START },
        { XK_Escape,    IN_SELECT },
        { XK_Left,      IN_LEFT },
        { XK_Right,     IN_RIGHT },
        { XK_Up,        IN_UP },
        { XK_Down,      IN_DOWN }
    };

    KeySym code = XLookupKeysym(&event, 0);

    if (code == XK_Shift_R)   code = XK_Shift_L;
    if (code == XK_Control_R) code = XK_Control_L;
    if (code == XK_Alt_R)     code = XK_Alt_L;

    for (int i = 0; i < COUNT(keyMapping); i++)
    {
        if (XKeysymToKeycode(dpy, keyMapping[i].code) == event.keycode)
        {
            return keyMapping[i].key;
        }
    }

    return IN_NONE;
}


// sound
#if 0
#define SND_FRAME_SIZE 4
#define SND_DATA_SIZE (2352 * SND_FRAME_SIZE)

pa_simple *sndOut;
pthread_t sndThread;

Sound::Frame *sndData;
#endif

void* soundFill(void* arg)
{
#if 0
    while (1)
    {
        Sound::fill(sndData, SND_DATA_SIZE / SND_FRAME_SIZE);
        pa_simple_write(sndOut, sndData, SND_DATA_SIZE, NULL);
    }
#endif
    return NULL;
}

void soundInit()
{
#if 0
    static const pa_sample_spec spec = {
        .format   = PA_SAMPLE_S16LE,
        .rate     = 44100,
        .channels = 2
    };

    static const pa_buffer_attr attr = {
        .maxlength  = SND_DATA_SIZE * 4,
        .tlength    = 0xFFFFFFFF,
        .prebuf     = 0xFFFFFFFF,
        .minreq     = SND_DATA_SIZE,
        .fragsize   = 0xFFFFFFFF,
    };

    int error;
    if (!(sndOut = pa_simple_new(NULL, WND_TITLE, PA_STREAM_PLAYBACK, NULL, "game", &spec, NULL, &attr, &error)))
    {
        LOG("pa_simple_new() failed: %s\n", pa_strerror(error));
        sndData = NULL;
        return;
    }

    sndData = new Sound::Frame[SND_DATA_SIZE / SND_FRAME_SIZE];
    pthread_create(&sndThread, NULL, sndFill, NULL);
#endif
}

void soundFree()
{
#if 0
    if (sndOut) {
        pthread_cancel(sndThread);
    //    pa_simple_flush(sndOut, NULL);
    //    pa_simple_free(sndOut);
        delete[] sndData;
    }
#endif
}

void toggle_fullscreen(Display* dpy, Window win)
{
    const size_t _NET_WM_STATE_TOGGLE = 2;

    XEvent xev;
    Atom wm_state = XInternAtom(dpy, "_NET_WM_STATE", False);
    Atom scr_full = XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", False);

    memset(&xev, 0, sizeof(xev));
    xev.type = ClientMessage;
    xev.xclient.window = win;
    xev.xclient.message_type = wm_state;
    xev.xclient.format = 32;
    xev.xclient.data.l[0] = _NET_WM_STATE_TOGGLE;
    xev.xclient.data.l[1] = scr_full;

    XSendEvent(dpy, DefaultRootWindow(dpy), False, SubstructureNotifyMask, &xev);
}

void WndProc(const XEvent& e, Display* dpy, Window wnd)
{
    switch (e.type)
    {
        case ConfigureNotify:
        {
            renderResize(e.xconfigure.width, e.xconfigure.height);
            break;
        }
        case KeyPress:
        case KeyRelease:
        {
            if (e.type == KeyPress && (e.xkey.state & Mod1Mask) && e.xkey.keycode == 36)
            {
                toggle_fullscreen(dpy, wnd);
                break;
            }
            setDown(keyToInputKey(dpy, e.xkey), e.type == KeyPress);
            break;
        }
    }
}

#define MAX_FILES 4096
char* gFiles[MAX_FILES];
int32 gFilesCount;

void addDir(char* path)
{
    dirent* e;
    DIR* dir = opendir(path);

    int32 pathLen = strlen(path);
    path[pathLen] = '/';

    while ((e = readdir(dir)))
    {
        if (e->d_type == DT_DIR)
        {
            if (e->d_name[0] != '.')
            {
                strcpy(path + 1 + pathLen, e->d_name);
                addDir(path);
            }
        }
        else
        {
            ASSERT(gFilesCount < MAX_FILES);
            if (gFilesCount < MAX_FILES)
            {
                strcpy(path + 1 + pathLen, e->d_name);
                char* fileName = new char[strlen(path) + 1 - 2];
                gFiles[gFilesCount++] = strcpy(fileName, path + 2);
            }
        }
    }

    path[pathLen] = '\0';
}

void streamInit()
{
    char path[1024];
    strcpy(path, ".");
    addDir(path);
    LOG("scan %d files\n", gFilesCount);
}

void streamFree()
{
    for (int32 i = 0; i < gFilesCount; i++)
    {
        delete[] gFiles[i];
    }
}

FileStream::FileStream(const char* fileName)
{
    for (int32 i = 0; i < gFilesCount; i++)
    {
        if (!strcasecmp(fileName, gFiles[i]))
        {
            LOG("open file %s -> %s\n", fileName, gFiles[i]);
            f = fopen(gFiles[i], "rb");
            return;
        }
    }
    LOG("file not found %s\n", fileName);
    f = NULL;
}

int main(int argc, char **argv)
{
    static int XGLAttr[] = {
        GLX_RGBA,
        GLX_DOUBLEBUFFER,
        GLX_DEPTH_SIZE, 24,
        GLX_RED_SIZE, 8,
        GLX_GREEN_SIZE, 8,
        GLX_BLUE_SIZE, 8,
        GLX_ALPHA_SIZE, 8,
        0
    };

    dpy = XOpenDisplay(NULL);
    XVisualInfo* vis = glXChooseVisual(dpy, XDefaultScreen(dpy), XGLAttr);

    XSetWindowAttributes attr;
    attr.colormap = XCreateColormap(dpy, RootWindow(dpy, vis->screen), vis->visual, AllocNone);
    attr.border_pixel = 0;
    attr.event_mask = KeyPressMask | KeyReleaseMask | StructureNotifyMask;

    wnd = XCreateWindow(
        dpy, RootWindow(dpy, vis->screen),
        0, 0, 960, 720, 0,
        vis->depth, InputOutput, vis->visual,
        CWColormap | CWBorderPixel | CWEventMask, &attr);
    XStoreName(dpy, wnd, WND_TITLE);

    GLXContext ctx = glXCreateContext(dpy, vis, NULL, true);
    glXMakeCurrent(dpy, wnd, ctx);
    XMapWindow(dpy, wnd);

    Atom WM_DELETE_WINDOW = XInternAtom(dpy, "WM_DELETE_WINDOW", 0);
    XSetWMProtocols(dpy, wnd, &WM_DELETE_WINDOW, 1);

    gTimerStart = osGetSystemTimeMS();
    srand(gTimerStart);

    streamInit();
    inputInit();
    soundInit();
    renderInit();
    gameInit();

    while (!isQuit)
    {
        if (XPending(dpy))
        {
            XEvent event;
            XNextEvent(dpy, &event);
            if (event.type == ClientMessage && *event.xclient.data.l == WM_DELETE_WINDOW)
            {
                osQuit();
            }
            WndProc(event, dpy, wnd);
        }
        else
        {
            gLastFrameIndex = gFrameIndex;
            gFrameIndex = osGetSystemTimeMS() * 60 / 1000;

            inputUpdate();

            gameUpdate();
            gameRender();

            renderSwap();
        }
    };

    gameFree();
    renderFree();
    soundFree();
    inputFree();
    streamFree();

    glXMakeCurrent(dpy, 0, 0);
    XCloseDisplay(dpy);
    return 0;
}
