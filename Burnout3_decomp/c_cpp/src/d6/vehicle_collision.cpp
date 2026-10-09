#include "vehicle_physics.h"

extern __int128 D_004B7DA0[];
extern __int128 D_004B7DB0[];
extern "C" void func_001CC580(VehiclePhysics *, u32, const Vec4 *, void *);

extern "C" void func_001CC4C0(VehiclePhysics *self, u32 flags, void *context)
{
    __int128 direction;
    if ((flags & 0x40) || (flags & 0x20)) {
        direction = D_004B7DA0[0];
    } else {
        direction = D_004B7DB0[0];
    }
    func_001CC580(self, flags, (const Vec4 *)&direction, context);
}

extern "C" void func_001CC520(VehiclePhysics *self, u32 flags, void *context)
{
    const Vec4 *direction;
    if ((flags & 0x40) || (flags & 0x20)) {
        direction = self->transform;
    } else if (flags & 0x80) {
        direction = &self->transform[2];
    } else {
        direction = &self->transform[1];
    }
    func_001CC580(self, flags, direction, context);
}
