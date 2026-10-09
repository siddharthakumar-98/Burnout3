#ifndef FRONTEND_FLOW_H
#define FRONTEND_FLOW_H

#include "gamemode.h"

#ifdef __cplusplus
/* Frontend/movie mode at Game+0x2C668. The mode and embedded movie object keep their original vtables in assembly. */
struct FrontendMode {
    GameMode base;
    void *uiBuffer;            /* 0x1C0 */
    void *stageBuffer;         /* 0x1C4 */
    u8 movie[0x120];           /* 0x1C8 */
    s32 font;                 /* 0x2E8 */
    u8 pad2EC[4];
    u8 enabled;               /* 0x2F0 */
    u8 pad2F1[3];
    s32 state;                /* 0x2F4 */
    s32 movieState;           /* 0x2F8 */
    s32 movieId;              /* 0x2FC */
    u64 sceneHash;            /* 0x300 */
    s32 unk308;
    s32 sound;                /* 0x30C */
    void *movieBuffer;         /* 0x310 */
    u8 unk314;
    bool uiLoaded;            /* 0x315 */
    bool stageLoaded;         /* 0x316 */
    u8 unk317;
    u8 stream[4];             /* 0x318; only its address is used */
};
extern "C" {
#else
typedef struct FrontendMode FrontendMode;
#endif

void func_00130C20(FrontendMode *self, u64 hash);
void func_00130C30(FrontendMode *self, s32 movie);
void func_00130C40(FrontendMode *self);
void func_00130C70(FrontendMode *self);
void func_00130D00(FrontendMode *self);
s32 func_00130D50(FrontendMode *self);
s32 func_00130EE0(FrontendMode *self);
void func_00131100(FrontendMode *self);
void func_001312D0(FrontendMode *self);
void func_001313C0(FrontendMode *self);
s32 func_00131480(FrontendMode *self);
void func_00131990(FrontendMode *self);

#ifdef __cplusplus
}
#endif
#endif
