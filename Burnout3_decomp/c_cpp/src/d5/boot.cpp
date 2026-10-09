/* Retail entry point (0x12EB30-0x12EEF0). Startup, asynchronous boot loading, the game frame loop and shutdown. */
#include "game.h"

class BootManager {
public:
    virtual void unk08();
    virtual void unk0C();
    virtual void unk10();
    virtual void unk14();
    virtual void unk18(void *next, s32 flags);
    virtual void unk1C(void *previous, s32 flags);
};

struct BootScene {
    u8 pad0[0x20];
    f32 time;
};
struct BootItem {
    u8 pad0[8];
    f32 time;
};

extern "C" {
extern Game D_004EE040;
extern u8 D_0051BA90[];
extern u8 D_0051BAD7[];
extern u8 D_00665EC0[];
extern char D_0066E980[];
extern u8 D_01E32AE0[];
extern s32 D_01E900B0[];
extern u8 D_01EA2970[];
extern BootItem *D_01EA2974[];
extern BootScene *D_01EA2980[];
extern f32 D_01EA2984[];
extern char D_004B4298[];
extern char D_004B42A0[];
extern char D_004B42A8[];
extern char D_004B42B0[];
extern u8 D_10000[];
extern u8 D_1FF0000[];

void func_00100230(void);
void func_00100240(void);
char *func_00127BF8(const char *text, const char *pattern);
void func_00129198(char *text);
void func_00131AC0(Game *game);
void func_0019DD00(void *self, f32 time);
void func_001D3D70(Game *game);
BootItem *func_00217690(void *self, u64 hash);
void func_002270B0(void *self);
void func_00237120(void);
void func_0026F860(void *self);
void *memset(void *dst, s32 value, u32 size);
s32 snprintf(char *dst, u32 size, const char *format, ...);
s32 GetThreadId(void);
s32 ChangeThreadPriority(s32 thread, s32 priority);

static inline bool BootLoopActive(void)
{
    return !D_0051BA90[0];
}

s32 func_0012EB30(s32 argc, char **argv)
{
    char path[128];
    BootItem *next;
    BootItem *previous;

    func_00100240();
    memset(D_1FF0000, 0x65, (u32)D_10000 - 0x1000);
    func_00133BB0(&D_004EE040);
    snprintf(path, 128, D_004B4298, argv[0]);
    func_00129198(path);
    if (func_00127BF8(path, D_004B42A0) == path) {
        snprintf(D_0066E980, 128, D_004B4298, path);
    } else if (func_00127BF8(path, D_004B42A8) == path) {
        path[0] = 'c';
        path[1] = 'd';
        path[2] = 'r';
        path[3] = 'o';
        path[4] = 'm';
        snprintf(D_0066E980, 128, D_004B4298, path);
    } else {
        snprintf(D_0066E980, 128, D_004B42B0, path);
    }
    ChangeThreadPriority(GetThreadId(), 10);
    func_0019DD00(D_00665EC0, -1.0f);
    func_002270B0(D_00665EC0);
    func_0019DD00(D_00665EC0, -1.0f);
    func_002270B0(D_00665EC0);
    func_0019DD00(D_00665EC0, -1.0f);
    func_002270B0(D_00665EC0);
    func_0019DD00(D_00665EC0, -1.0f);
    func_002270B0(D_00665EC0);
    while (!func_00133190(&D_004EE040)) {
        func_00237120();
        func_0019DD00(D_00665EC0, -1.0f);
        func_002270B0(D_00665EC0);
    }
    if (BootLoopActive()) {
        while (BootLoopActive()) {
            func_00132600(&D_004EE040);
        }
    }
    if (D_0051BAD7[0]) {
        D_01E900B0[0] = 1;
        next = func_00217690(D_01EA2970, 0x94413FA737AAA797ull);
        if (D_01EA2974[0] != next) {
            ((BootManager *)D_01EA2970)->unk18(next, 0);
            previous = D_01EA2974[0];
            D_01EA2974[0] = next;
            BootScene *scene = D_01EA2980[0];
            if (scene != 0) {
                D_01EA2984[0] = scene->time;
                if (next != 0) {
                    next->time = scene->time;
                }
            }
            ((BootManager *)D_01EA2970)->unk1C(previous, 0);
        }
        func_00131AC0(&D_004EE040);
        func_00131AC0(&D_004EE040);
        func_00131AC0(&D_004EE040);
        func_00131AC0(&D_004EE040);
        func_00131AC0(&D_004EE040);
        func_00131AC0(&D_004EE040);
        func_00131AC0(&D_004EE040);
        func_00131AC0(&D_004EE040);
    }
    func_00132560(&D_004EE040);
    func_00132420(&D_004EE040);
    if (D_0051BAD7[0]) {
        func_001D3D70(&D_004EE040);
    }
    func_0026F860(D_01E32AE0);
    func_00100230();
    return 0;
}
}
