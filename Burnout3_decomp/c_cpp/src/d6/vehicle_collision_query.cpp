#include "vehicle_physics.h"

extern u8 D_004E2704;
extern u32 D_0052267C[];
extern u8 D_00522660[];

extern "C" s32 func_00251740(void *, const Vec4 *);
extern "C" s32 func_00301B00(void *, s32, const Vec4 *,
                            s32 (*)(const VehicleCollisionTriangle *, VehiclePhysics *),
                            VehiclePhysics *);

/* Keep the retail VU0 operations: only w is defined by these speed copies. */
static inline f32 querySpeed(register const Vec4 *speed, register u32 timeBits)
{
    VehicleVector scaled;
    register Vec4 *destination = &scaled.value;
    asm volatile {
        lqc2 vf1, 0(speed)
        qmtc2.ni timeBits, vf2
        vmulx.w vf1, vf1, vf2x
        sqc2 vf1, 0(destination)
    }
    return scaled.value.w;
}

static inline f32 querySpeed(register const Vec4 *speed, register u32 timeBits,
                             register f32 steps)
{
    VehicleVector scaled;
    register Vec4 *destination = &scaled.value;
    asm volatile {
        lqc2 vf1, 0(speed)
        qmtc2.ni timeBits, vf2
        vmulx.w vf2, vf1, vf2x
        mfc1 v1, steps
        qmtc2.ni v1, vf1
        vmulx.w vf1, vf2, vf1x
        sqc2 vf1, 0(destination)
    }
    return scaled.value.w;
}

/* Sphere radius = VU0 length(extents) + the predicted travel distance.
 * xyz comes from the transform's translation; w is set separately. */
static inline void querySphere(register Vec4 *sphere, register const Vec4 *position,
                               register const Vec4 *extents, register f32 travel)
{
    asm volatile {
        vaddw.x vf1, vf0, vf0w
        qmfc2.ni v1, vf1
        lqc2 vf2, 0(sphere)
        lqc2 vf1, 0(position)
        vadd.xyz vf2, vf0, vf1
        sqc2 vf2, 0(sphere)
        lqc2 vf1, 0(extents)
        vmul.xyz vf2, vf1, vf1
        vadday.x ACC, vf2, vf2y
        qmtc2.ni v1, vf1
        vmaddz.x vf1, vf1, vf2z
        qmfc2.ni v0, vf1
        qmtc2.ni v0, vf1
        vsqrt Q, vf1x
        vaddw.x vf1, vf0, vf0w
        qmfc2.ni v1, vf1
        vwaitq
        qmtc2.ni v1, vf1
        vmulq.x vf1, vf1, Q
        qmfc2.ni v1, vf1
        mfc1 v0, travel
        qmtc2.ni v1, vf1
        qmtc2.ni v0, vf2
        vaddx.x vf1, vf1, vf2x
        lqc2 vf2, 0(sphere)
        vmulx.w vf2, vf0, vf1x
        sqc2 vf2, 0(sphere)
    }
}

extern "C" void func_001C5080(VehiclePhysics *self, s32 steps)
{
    f32 travel;
    if (self->raceCar->state1914) {
        if (steps > 0) {
            travel = querySpeed(&self->speedSample.value, D_0052267C[0], (f32)steps);
        } else {
            travel = 0.0f;
        }
    } else {
        travel = querySpeed(&self->speedSample.value, D_0052267C[0]);
    }

    Vec4 sphere;
    querySphere(&sphere, &self->transform[3], &self->queryExtents.value, travel);
    self->triangleCache->count = 0;
    D_004E2704 = 0;
    self->currentUnit = func_00251740(D_00522660 + 0x12A348 + self->collisionWorld * 0x1CC,
                                     &self->transform[3]);
    func_00301B00(D_00522660 + 0x12A348 + self->collisionWorld * 0x1CC,
                  self->currentUnit, &sphere, func_001C5330, self);

    if (self->extraTrianglesEnabled) {
        if (self->triangleCache->count < 90) {
            for (s32 i = 0; i < 6; ++i) {
                VehicleCachedTriangle *destination =
                    &self->triangleCache->triangles[self->triangleCache->count];
                destination->vertices = self->extraTriangles[i].vertices;
                destination->normal.bits = self->extraTriangles[i].normal.bits;
                self->triangleCache->tags[self->triangleCache->count] = 0;
                if (i < 2) {
                    u16 *tag = &self->triangleCache->tags[self->triangleCache->count];
                    *tag &= 0xFF00;
                    *tag |= 0x26;
                } else {
                    u16 *tag = &self->triangleCache->tags[self->triangleCache->count];
                    *tag &= 0xFF00;
                    *tag |= 0x1A;
                }
                ++self->triangleCache->count;
            }
        } else {
            self->extraTrianglesEnabled = 0;
        }
    }
}
