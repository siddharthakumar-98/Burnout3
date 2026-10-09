/* Preview-mode load and init (0x24FC20-0x24FDE0). 82.19% load: cold blocks and VU0 zero register differ. */
#include "preview_flow.h"
#include "memmgr.h"

/* The original vector setters use VU0 macro mode. Keep these hardware operations inline; all lifecycle
 * decisions, scalar defaults and state transitions remain ordinary C++. */
static inline void resetPreviewBasis(register PreviewMode *self, register s32 one, register s32 zero)
{
    asm {
        lqc2 vf1, 0x240(self)
        qmtc2.ni one, vf3
        qmtc2.ni zero, vf2
        vaddx.x vf1, vf0, vf3x
        vaddx.y vf1, vf0, vf2x
        vaddx.z vf1, vf0, vf2x
        sqc2 vf1, 0x240(self)
        lqc2 vf1, 0x250(self)
        vaddx.x vf1, vf0, vf2x
        vaddx.y vf1, vf0, vf3x
        vaddx.z vf1, vf0, vf2x
        sqc2 vf1, 0x250(self)
        lqc2 vf1, 0x260(self)
        vaddx.x vf1, vf0, vf2x
        vaddx.y vf1, vf0, vf2x
        vaddx.z vf1, vf0, vf3x
        sqc2 vf1, 0x260(self)
    }
}

extern "C" {
extern u8 theMemMgr[];
extern u8 D_00516E34[];
extern u8 D_00516E10[];
extern s32 D_004E29C8;
extern s32 D_004E197C;

s32 func_0024FC20(PreviewMode *self)
{
    s32 i;
    switch (self->state) {
    case 1:
    case 24:
        if (!func_003E8760((Heap *)theMemMgr, 1)) { return 0; }
        self->state = 2;
        D_00516E34[0] = 0;
        D_00516E10[0] = 0;
        if (D_004E29C8 == 0) {
            self->yaw = 20.0f;
            self->pitch = -135.0f;
        }
        self->turn = 0.0f;
        self->distance = 5.0f;
        self->index = 0;
        self->reset = 1;
        self->speed = 1.0f;
        self->unk228 = 2;
        resetPreviewBasis(self, 0x3F800000, 0);
        self->unk234 = 0;
        self->unk230 = 0;
        self->unk274 = 0;
        self->unk270 = 0;
        self->unk22D = 0;
        self->unk22C = 0;
        self->flags[4] = 0;
        /* Fall through once the initial state has been prepared. */
    case 2:
        func_001346C0(&self->base);
        self->flags[0] = 0;
        self->flags[1] = 0;
        self->flags[2] = 0;
        self->flags[3] = 0;
        self->flags[4] = 0;
        for (i = 0; i < 6; i++) { self->previous[i] = 0.0f; }
done:
        D_004E197C = 1;
        return 1;
    default:
        goto done;
    }
}

void func_0024FDA0(PreviewMode *self)
{
    func_00134910(&self->base);
    ((PreviewControls *)self->controls)->Init();
    self->state = 1;
}
}
