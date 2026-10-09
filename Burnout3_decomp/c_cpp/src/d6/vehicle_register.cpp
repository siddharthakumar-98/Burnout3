#include "vehicle_physics.h"

extern u8 D_0064A8F0[];
extern u8 D_0064B940[];
extern "C" void func_00354CF0(void *, VehiclePhysics *);

/* The retail translation getter copies one quadword before reading y.
 * Keep the matrix pointer at the verified +0x204 offset. */
static inline VehicleVector vehiclePosition(register VehiclePhysics *self)
{
    VehicleVector position;
    register VehicleVector *destination = &position;
    asm {
        lw a0, 0x204(self)
        lq a0, 0x30(a0)
        sq a0, 0(destination)
    }
    return position;
}

extern "C" void func_001CBC70(VehiclePhysics *self, u8 state)
{
    if (self->flag210) {
        return;
    }
    func_001ABE40(self, state);
    func_00354CF0(D_0064A8F0, self);
    func_00354CF0(D_0064B940, self);
    self->flag116B = 0;
    self->initialHeight = vehiclePosition(self).value.y;
}
