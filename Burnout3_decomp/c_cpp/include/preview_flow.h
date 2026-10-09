#ifndef PREVIEW_FLOW_H
#define PREVIEW_FLOW_H
#include "gamemode.h"
#include "vu0.h"

#ifdef __cplusplus
/* Model-preview mode embedded at Game+0x2C9A0. Geometry/input updates are scheduled for D6/D10. */
class PreviewControls {
public:
    virtual void Init();
    virtual void unk0C();
    virtual void unk10();
    virtual void Shutdown();
};
struct PreviewMode {
    GameMode base;
    u8 controls[0x40];             /* 0x1C0 */
    f32 yaw;                      /* 0x200 */
    f32 pitch;
    f32 turn;
    f32 distance;
    u8 flags[5];                  /* 0x210 */
    u8 pad215[3];
    s32 index;                    /* 0x218 */
    s32 reset;
    f32 speed;
    s32 state;                    /* 0x224 */
    s32 unk228;
    u8 unk22C;
    u8 unk22D;
    u8 pad22E[2];
    u32 unk230;
    u32 unk234;
    u8 pad238[8];
    Vec4 basis[3];                /* 0x240; xyz axes, w is preserved */
    u32 unk270;
    u32 unk274;
    u8 pad278[0x300 - 0x278];
    f32 previous[6];              /* 0x300 */
};

extern "C" {
#else
typedef struct PreviewMode PreviewMode;
#endif
void func_0024D4D0(PreviewMode *self);
s32 func_0024FC20(PreviewMode *self);
void func_0024FDA0(PreviewMode *self);
#ifdef __cplusplus
}
#endif
#endif
