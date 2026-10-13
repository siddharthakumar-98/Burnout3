#include "vehicle_physics.h"

extern VehicleVector D_004C9CD0[];

static inline void copyZero(register VehicleVector *destination)
{
    register __int128 bits = D_004C9CD0[0].bits;
    asm { sq bits, 0(destination) }
}

/* The radius remains in vf1 until sphereCenter finishes the sphere update.
 * The intervening quadword copy uses only the CPU register file. */
static inline void sphereRadius(register f32 radius)
{
    register s32 bits;
    asm {
        mfc1 bits, radius
        qmtc2.ni bits, vf1
    }
}

static inline void sphereCenter(register VehicleVector *sphere, register const VehicleVector *center)
{
    register __int128 bits = center->bits;
    asm {
        qmtc2.ni bits, vf2
        lqc2 vf3, 0(sphere)
        vadd.xyz vf3, vf0, vf2
        vmulx.w vf3, vf0, vf1x
        sqc2 vf3, 0(sphere)
    }
}

extern "C" void func_001CFD20(VehiclePhysics *self)
{
    func_001AD850(self);
    for (s32 i = 0; i < 6; ++i) {
        self->wheels[i].radius = 29.0f;
        self->wheels[i].radiusScale = 1.0f;
        self->wheels[i].widthScale = 1.0f;
        self->wheelMatrices[i] = 0;
    }
    self->state1169 = 0;
    self->state116A = 0;
    self->suspension[0].attachHeight = 0.0f;
    self->suspension[1].attachHeight = 0.0f;
    self->suspension[0].springLength = 0.2f;
    self->suspension[0].springConstant = 60000.0f;
    self->suspension[0].dampingConstant = 5000.0f;
    self->suspension[1].springLength = 0.2f;
    self->suspension[1].springConstant = 60000.0f;
    self->suspension[1].dampingConstant = 5000.0f;
    self->steeringAngle = 0.0f;
    self->state1168 = 0;
    self->flag1352 = 1;
    self->collisionTag &= 0xFF00;
    func_002BAEE0(&self->shape);
    self->boundingVolume = 2;
    self->collisionShape = &self->shape;
    copyZero(&self->bounds1E0);
    sphereRadius(2.0f);
    copyZero(&self->queryExtents);
    sphereCenter(&self->boundingSphere, D_004C9CD0);
}
