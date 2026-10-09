#include "vehicle_physics.h"

static inline f32 collisionDot(register const Vec4 *a, register const Vec4 *b)
{
    register f32 result;
    asm volatile {
        lqc2 vf3, 0(a)
        lqc2 vf2, 0(b)
        vaddw.x vf1, vf0, vf0w
        vmul.xyz vf2, vf3, vf2
        qmfc2.ni v0, vf1
        vadday.x ACC, vf2, vf2y
        qmtc2.ni v0, vf1
        vmaddz.x vf1, vf1, vf2z
        qmfc2.ni v0, vf1
        mtc1 v0, result
    }
    return result;
}

extern "C" s32 func_001C5330(const VehicleCollisionTriangle *triangle, VehiclePhysics *self)
{
    u16 tag = triangle->surface->tag;
    s32 surface = tag & 0xFF;
    if (surface == 0x23 || surface == 0x22 || (tag & 0x1000) != 0) {
        return 1;
    }
    s32 directional;
    if (surface < 0x15 || surface > 0x20) {
        directional = 0;
    } else {
        directional = 1;
    }
    if (directional) {
        if (collisionDot(&self->speedSample.value, &triangle->normal.value) > 0.5f) {
            return 1;
        }
    }
    VehicleVector normal;
    normal.bits = triangle->normal.bits;
    if (normal.value.y < -0.7f) {
        return 1;
    }
    return func_001AB090(self, triangle);
}
