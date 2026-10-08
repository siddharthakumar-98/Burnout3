/* vu0.h: inline VU0 macro-mode operations on 16-byte vectors.
 *
 * The game has no out-of-line math library: these compile into their callers the way a vector class's inline
 * methods do. Each helper is a static inline function around an MW `asm { }` block; the block's instructions are
 * scheduled with the caller's code, and every call emits its block again (nothing is shared between calls).
 * vf1 and vf2 are scratch; vf0 is the constant (0, 0, 0, 1).
 */
#ifndef VU0_H
#define VU0_H

#include "types.h"

/* A VU0 quadword: lqc2/sqc2 need 16-byte alignment. */
typedef struct {
    float x, y, z, w;
} Vec4 __attribute__((aligned(16)));

#ifdef __cplusplus
extern "C" {
#endif

/* v->w = w, keeping xyz. The float goes through a GPR (mfc1; qmtc2); for a constant the compiler folds the mfc1
 * into a move but keeps the hazard nop after it. */
static inline void vu0SetW(register Vec4 *v, register float w)
{
    asm {
        lqc2 vf1, 0(v)
        mfc1 v0, w
        qmtc2.ni v0, vf2
        vsubw.w vf1, vf0, vf0w
        vaddx.w vf1, vf1, vf2x
        sqc2 vf1, 0(v)
    }
}

/* v->xyz = 0. w gets whatever vf1.w holds (the original does this too). */
static inline void vu0ZeroXYZ(register Vec4 *v)
{
    asm {
        vadd.xyz vf1, vf0, vf0
        sqc2 vf1, 0(v)
    }
}

#ifdef __cplusplus
}
#endif

#endif /* VU0_H */
